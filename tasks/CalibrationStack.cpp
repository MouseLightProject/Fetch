/**
  \file
  Microscope task.  Take a calibration stack at specific xyz.
  Copied/modified from AdaptiveTiledAcquisition.cpp by David Ackerman, otherwise:
  \author Nathan Clack <clackn@janelia.hhmi.org>

  \copyright
  Copyright 2010 Howard Hughes Medical Institute.
  All rights reserved.
  Use is subject to Janelia Farm Research Campus Software Copyright 1.1
  license terms (http://license.janelia.org/license/jfrc_copyright_1_1.html).
 */


#include "common.h"
#include "CalibrationStack.h"
#include "StackAcquisition.h"
#include "Video.h"
#include "frame.h"
#include "devices\digitizer.h"
#include "devices\Microscope.h"
#include "devices\tiling.h"
#include "tasks\SurfaceFind.h"


#if 1 // PROFILING
#define TS_OPEN(name)   timestream_t ts__=timestream_open(name)
#define TS_TIC          timestream_tic(ts__)
#define TS_TOC          timestream_toc(ts__)
#define TS_CLOSE        timestream_close(ts__)
#else
#define TS_OPEN(name)
#define TS_TIC
#define TS_TOC
#define TS_CLOSE
#endif

#define CHKJMP(expr) if(!(expr)) {warning("%s(%d)"ENDL"\tExpression indicated failure:"ENDL"\t%s"ENDL,__FILE__,__LINE__,#expr); goto Error;}

namespace fetch
{

  namespace task
  {

    //
    // AdaptiveTiledAcquisition -  microscope task
    //

    namespace microscope {

      //Upcasting
      unsigned int CalibrationStack::config(IDevice *d) {return config(dynamic_cast<device::Microscope*>(d));}
      unsigned int CalibrationStack::run   (IDevice *d) {return run   (dynamic_cast<device::Microscope*>(d));}

      unsigned int CalibrationStack::config(device::Microscope *d)
      {
        static task::scanner::ScanStack<u16> grabstack;
        std::string filename;

        Guarded_Assert(d);

        //Assemble pipeline here
        IDevice *cur;
        cur = d->configPipeline();

        CHKJMP(d->file_series.ensurePathExists());
        d->file_series.inc();
        filename = d->stack_filename();
        IDevice::connect(&d->disk,0,cur,0);
        Guarded_Assert( d->disk.close()==0 );
        //Guarded_Assert( d->disk.open(filename,"w")==0);

        d->__scan_agent.disarm(10000/*timeout_ms*/);
        int isok=d->__scan_agent.arm(&grabstack,&d->scanner)==0;
        d->stage()->tiling()->resetCursor();          // this is here so that run/stop cycles will pick up where they left off

        return isok; //success
Error:
        return 0;
      }

      static int _handle_wait_for_result(DWORD result, const char *msg)
      {
          return_val_if( result == WAIT_OBJECT_0  , 0 );
          return_val_if( result == WAIT_OBJECT_0+1, 1 );
          Guarded_Assert_WinErr( result != WAIT_FAILED );
          if(result == WAIT_ABANDONED_0)   warning("%s(%d)"ENDL "\tCalibrationStack: Wait 0 abandoned"ENDL "\t%s"ENDL, __FILE__, __LINE__, msg);
          if(result == WAIT_ABANDONED_0+1) warning("%s(%d)"ENDL "\tCalibrationStack: Wait 1 abandoned"ENDL "\t%s"ENDL, __FILE__, __LINE__, msg);
          if(result == WAIT_TIMEOUT)       warning("%s(%d)"ENDL "\tCalibrationStack: Wait timeout"ENDL     "\t%s"ENDL, __FILE__, __LINE__, msg);

          Guarded_Assert_WinErr( result !=WAIT_FAILED );

          return -1;
      }

	  unsigned int reset_to_original_values( device::Microscope *dc, Vector3f curpos, float * original_pockels_v_open){ //DGA: Reset pockel values to their non-calibration-stack values and reset the stage position
		  device::Pockels * pockels1 = &(dc->scanner._scanner2d._pockels1);
		  device::Pockels * pockels2 = &(dc->scanner._scanner2d._pockels2);
		  CHKJMP(pockels1->setOpenPercentNoWait(original_pockels_v_open[0]));
		  CHKJMP(pockels2->setOpenPercentNoWait(original_pockels_v_open[1]));
		  dc->moveToNewPosThroughSafeZ(curpos);
	  Error:
		  return 0;
	  }

	  /**
      calibration_stack

      Parameters to get from configuration:
      - calibration_stack --> target --> x,y,z
	  - calibration_stack --> v_open_percent

      \returns 0 if no tiles were targeted for imaging, otherwise 1.
      */
      unsigned int CalibrationStack::run(device::Microscope *dc)
	  {   
		  Vector3f pos, curpos;
		  unsigned int eflag = 0; // success
		  std::string filename;
		  //DGA: Create points for pockelToTurnOn, pockelToTurnOff and pockels1 and pockels2
		  device::Pockels * pockelToTurnOn, * pockelToTurnOff, * pockels1 = &(dc->scanner._scanner2d._pockels1), * pockels2 = &(dc->scanner._scanner2d._pockels2);
		  curpos = dc->stage()->getPos();
		  float original_pockels_v_open[2];
		  original_pockels_v_open[0] = pockels1->getOpenPercent();
		  original_pockels_v_open[1] = pockels2->getOpenPercent();
		  //DGA: If pockels1 has a calibration stack then that one will be turned on
		  if (!(pockels1->get_config().has_calibration_stack())) { pockelToTurnOn = pockels2; pockelToTurnOff = pockels1; }
		  else{ pockelToTurnOn = pockels1; pockelToTurnOff = pockels2; }
		  //DGA: Get position to take calibration stack and ensure it's within appropriate range
		  int target_num_el = pockelToTurnOn->get_config().calibration_stack().target_mm_size();
		  for (int i = 0; i < target_num_el ; i++){
			  pos[0] = pockelToTurnOn->get_config().calibration_stack().target_mm(0).x();
			  pos[1] = pockelToTurnOn->get_config().calibration_stack().target_mm(0).y();
			  pos[2] = pockelToTurnOn->get_config().calibration_stack().target_mm(0).z();
			  for (int i = 0; i < 3; i++){
				  pos[i] = (pos[i] < minXYZ[i]) ? minXYZ[i] : (pos[i] > maxXYZ[i] ? maxXYZ[i] : pos[i]);
			  }
			  CHKJMP(dc->__scan_agent.is_runnable());
			  CHKJMP(dc->moveToNewPosThroughSafeZ(pos));
			  //DGA: Set pockel percentages to appropriate values
			  CHKJMP(pockelToTurnOn->setOpenPercentNoWait(pockelToTurnOn->get_config().calibration_stack().v_open_percent()));
			  CHKJMP(pockelToTurnOff->setOpenPercentNoWait(0));
			  debug("%s(%d)"ENDL "\t[Calibration Stack Task] tilepos: %5.1f %5.1f %5.1f"ENDL, __FILE__, __LINE__, pos[0], pos[1], pos[2]);
			  filename = dc->stack_filename();
			  dc->file_series.ensurePathExists();
			  dc->disk.set_nchan(dc->scanner.get2d()->digitizer()->nchan());
			  eflag |= dc->disk.open(filename, "w");
			  if (eflag)
			  {
				  warning("Couldn't open file: %s"ENDL, filename.c_str());
				  return eflag;
			  }

			  eflag |= dc->runPipeline();
			  eflag |= dc->__scan_agent.run() != 1;

			  { // Wait for stack to finish
				  HANDLE hs[] = {
					  dc->__scan_agent._thread,
					  dc->__self_agent._notify_stop };
				  DWORD res;
				  int   t;

				  // wait for scan to complete (or cancel)
				  res = WaitForMultipleObjects(2, hs, FALSE, INFINITE);
				  t = _handle_wait_for_result(res, "CalibrationStack::run - Wait for scanner to finish.");
				  switch (t)
				  {
				  case 0:                            // in this case, the scanner thread stopped.  Nothing left to do.
					  eflag |= dc->__scan_agent.last_run_result(); // check the run result
					  eflag |= dc->__io_agent.last_run_result();
				  case 1:                            // in this case, the stop event triggered and must be propagated.
					  eflag |= dc->__scan_agent.stop(SCANNER2D_DEFAULT_TIMEOUT) != 1;
					  break;
				  default:                           // in this case, there was a timeout or abandoned wait
					  eflag |= 1;                      // failure
				  }
			  } // end waiting block

			  // Output and Increment files
			  dc->write_stack_metadata();          // write the metadata
			  eflag |= dc->disk.close();
			  dc->file_series.inc();               // increment regardless of completion status
			  eflag |= dc->stopPipeline();         // wait till everything stops
		  }
		  CHKJMP(reset_to_original_values(dc, curpos, original_pockels_v_open)); //DGA: Reset to original values
		  return eflag;
	  Error:
		  CHKJMP(reset_to_original_values(dc, curpos, original_pockels_v_open)); //DGA: Reset to original values
		  return 0;
	  }

    } // namespace microscope

  }   // namespace task
}     // namespace fetch
