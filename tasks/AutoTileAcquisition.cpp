/** \file
  Task: Full-automatic 3D tiling acquisition.

  \author: Nathan Clack <clackn@janelia.hhmi.org>

  \copyright
  Copyright 2010 Howard Hughes Medical Institute.
  All rights reserved.
  Use is subject to Janelia Farm Research Campus Software Copyright 1.1
  license terms (http://license.janelia.org/license/jfrc_copyright_1_1.html).
 */


#include "common.h"
#include "AutoTileAcquisition.h"
#include "TiledAcquisition.h"
#include "StackAcquisition.h"
#include "Video.h"
#include "frame.h"
#include "devices\digitizer.h"
#include "devices\Microscope.h"
#include "devices\tiling.h"
#include "AdaptiveTiledAcquisition.h"
#include "CalibrationStack.h"

#define CHKJMP(expr) if(!(expr)) {warning("%s(%d)"ENDL"\tExpression indicated failure:"ENDL"\t%s"ENDL,__FILE__,__LINE__,#expr); goto Error;}
#define WARN(msg)    warning("%s(%d)"ENDL"\t%s"ENDL,__FILE__,__LINE__,msg)
#define DBG(...)     do{debug("%s(%d)"ENDL "\t",__FILE__,__LINE__); debug(__VA_ARGS__);}while(0)

namespace fetch
{

  namespace task
  {

    //
    // AutoTileAcquisition -  microscope task
    //

    namespace microscope {

      /** \class AutoTileAcquisition AutoTileAcquisition.h

      Microscope task fro automatic 3d tiling of a volume.

      The task operates in three phases:

      While zpos in bounds:
      -#  Explore the current slice to determine which tiles to acquire.
         - foreach tile in zone:
           - move to tile
           - acquire single image at probe depth (could use stage system to offset...just need to ensure correct tile is marked)
           - classify image
           - mark tile
         - postprocess marked tiles
           - close
           - dilate1
      -#  Run the TiledAcquisition task to collect those tiles.
         - since TiledAcquisition is a microscope task, it's run function will be directly
           invoked rather than running it asynchronously.
      -#  Run the Cut (vibratome) task to cut the imaged slice off.

      Questions:
      -#  How to define exploration zone? Z limits.
          How to iterate over tiles for exploration.
      -#  Feedback on marking.
          - can see tiles getting marked in view as it happens...view linked to current stage zpos
      -#  User interuption.
      -#  Speed?  How fast can I explore a given area?
          - initially assume I don't need to be efficient (take simple approach)

      */

      //Upcasting
      unsigned int AutoTileAcquisition::config(IDevice *d) {return config(dynamic_cast<device::Microscope*>(d));} 
      unsigned int AutoTileAcquisition::run   (IDevice *d) {return run   (dynamic_cast<device::Microscope*>(d));}

      unsigned int AutoTileAcquisition::config(device::Microscope *d)
      {
        return 1; //success
Error:
        return 0;
      }

      static int _handle_wait_for_result(DWORD result, const char *msg)
      {
          return_val_if( result == WAIT_OBJECT_0  , 0 );
          return_val_if( result == WAIT_OBJECT_0+1, 1 );
          Guarded_Assert_WinErr( result != WAIT_FAILED );
          switch(result)
          { case WAIT_ABANDONED_0:   warning("TiledAcquisition: Wait 0 abandoned\r\n\t%s\r\n", msg); break;
            case WAIT_ABANDONED_0+1: warning("TiledAcquisition: Wait 1 abandoned\r\n\t%s\r\n", msg); break;
            case WAIT_TIMEOUT:       warning("TiledAcquisition: Wait timeout\r\n\t%s\r\n", msg);     break;
            default:
            ;
          }
          return -1;
      }

	  // DGA: Function for simulating an ellipsoid (with circular cross sections) in 3D so that we can test, eg., if the explorable region shrinks as desired
	  static bool insideSimulationOfEllipse(float maxzUm, float targetZUm, Vector3f tilepos)
	  {
		  float distanceFromXYCenterUm, ellipseMajorAxisUm = 2000, crossSectionRadiusUm, ellipseMinorAxisUm, zEllipseCenterUm; // DGA: Variables used to create ellipsoid in 3D
		  ellipseMinorAxisUm = (0.5*(maxzUm - 10000)); //DGA: Assumes initial offset of 10000 um, so want minor axis to be equal to the total height/2
		  zEllipseCenterUm = 10000 + ellipseMinorAxisUm; //DGA: Center of ellipse in Z
		  crossSectionRadiusUm = ellipseMajorAxisUm*sqrt(1 - pow((zEllipseCenterUm - targetZUm) / ellipseMinorAxisUm, 2)); //DGA: Calculate the cross section radius
		  crossSectionRadiusUm = (crossSectionRadiusUm > 250) ? crossSectionRadiusUm : 250; //DGA: Set a minimum cross section since the program will stop if there aren't any detected tiles
		  distanceFromXYCenterUm = sqrt((tilepos[0] - 50000)*(tilepos[0] - 50000) + (tilepos[1] - 50000)*(tilepos[1] - 50000)); //DGA: Calculate if the current tile is within the circular cross section, centered at (50 mm, 50 mm)
		  return (distanceFromXYCenterUm < crossSectionRadiusUm); //DGA: If the tile is within the ellipse, return true
	  }
      ///// CLASSIFY //////////////////////////////////////////////////
      template<class T>
      static int _classify(mylib::Array *src, int ichan, double intensity_thresh, double area_thresh)
      { T* data;
        mylib::Array tmp=*src,*image=&tmp;
        if(ichan>=0)
        { image=mylib::Get_Array_Plane(&tmp,ichan);
        }
        data = (T*)image->data;
        size_t i,count=0;
        if(!image->size) return 0;
        for(i=0;i<image->size;++i)
          count+=(data[i]>intensity_thresh);
#if 0
        mylib::Write_Image("classify.tif",image,mylib::DONT_PRESS);
#endif
        DBG("Fraction above thresh: %f\n\t\tintensity thresh: %f\n\t\tarea_thresh: %f\n",
          count/((double)image->size),
          intensity_thresh,
          area_thresh);        
        return (count/((double)image->size))>area_thresh;
      }
      #define CLASSIFY(type_id,type) case type_id: return _classify<type>(image,ichan,intensity_thresh,area_thresh); break
      /**
      \returns 0 if background, 1 if foreground

      Image could be multiple channels.  Channels are assumed to plane-wise.
      */
      static int classify(mylib::Array *image, int ichan, double intensity_thresh, double area_thresh)
      {
        if(image->ndims<3)  // check that there are enough dimensions to select a channel
        { ichan=-1;
        } else if(ichan>=image->dims[image->ndims-1]) // is ichan sane?  If not, use chan 0.
        { ichan=0;
        }

        switch(image->type)
        {
          CLASSIFY( mylib::UINT8_TYPE   ,uint8_t );
          CLASSIFY( mylib::UINT16_TYPE  ,uint16_t);
          CLASSIFY( mylib::UINT32_TYPE  ,uint32_t);
          CLASSIFY( mylib::UINT64_TYPE  ,uint64_t);
          CLASSIFY( mylib::INT8_TYPE    , int8_t );
          CLASSIFY( mylib::INT16_TYPE   , int16_t);
          CLASSIFY( mylib::INT32_TYPE   , int32_t);
          CLASSIFY( mylib::INT64_TYPE   , int64_t);
          CLASSIFY( mylib::FLOAT32_TYPE ,float   );
          CLASSIFY( mylib::FLOAT64_TYPE ,double  );
          default:
            return 0;
        }
      }
      #undef CLASSIFY

      ///// EXPLORE  //////////////////////////////////////////////////

      /** Tests to make sure the cut/image cycle stays in z bounds.

      Only need to test max since stage only moves up as cuts progress.

      */
      static int PlaneInBounds(device::Microscope *dc,float maxz)
      { float x,y,z;
        dc->stage()->getPos(&x,&y,&z);
        return z<maxz;
      }

      /**
      Explores the current plane searching for tiles to image.  A heuristic classifier
      is used to target a tile for imaging based on a single snapshot acquired at a
      given depth (\c dz_um).

      Preconditions:
      - tiles to explore have been labelled as such

      Parameters to get from configuration:
      - dz_um:               zpiezo offset
      - maxz                 stage units(mm)
      - timeout_ms
      - ichan:               channel to use for classification, -1 uses all channels
      - intensity_threshold: use the expected pixel units
      - area_threshold:      0 to 1. The fraction of pixels that must be brighter than intensity threshold.

      \returns 0 if no tiles were targeted for imaging, otherwise 1.
      */
      static int explore(device::Microscope *dc)
      { Vector3f tilepos;
        unsigned any_explorable=0,
                 any_active=0;
		cfg::tasks::AutoTile cfg=dc->get_config().autotile();
        size_t iplane=dc->stage()->getPosInLattice().z();

        device::StageTiling* tiling = dc->stage()->tiling();
        tiling->markAddressable(iplane); // make sure the current plane is marked addressable
        tiling->setCursorToPlane(iplane);

		device::Digitizer::Config digcfg = dc->scanner._scanner2d._digitizer.get_config(); //DGA: Get the configuration of the digitizer to know if it is simulated
        device::TileSearchContext *ctx=0;
        while(  !dc->_agent->is_stopping()
              && tiling->nextSearchPosition(iplane,cfg.search_radius()/*radius - tiles*/,tilepos,&ctx))
              //&& tiling->nextInPlaneExplorablePosition(tilepos))
        { mylib::Array *im;
          any_explorable=1;
          tilepos[2]=dc->stage()->getTarget().z()*1000.0; // convert mm to um
          DBG("Exploring tile: %6.1f %6.1f %6.1f",tilepos.x(),tilepos.y(),tilepos.z());
          CHKJMP(dc->stage()->setPos(tilepos*0.001)); // convert um to mm
          CHKJMP(im=dc->snapshot(cfg.z_um(),cfg.timeout_ms()));
          tiling->markExplored();
		  
          tiling->markDetected(classify(im,cfg.ichan(),cfg.intensity_threshold(),cfg.area_threshold()));
		  if (digcfg.kind() == cfg::device::Digitizer_DigitizerType_Simulated){	//DGA: If digitizer is simulated, then simulate an ellipsoidal volume
			  if (!insideSimulationOfEllipse(cfg.maxz_mm()*1000, dc->stage()->getTarget().z()*1000.0, tilepos)) tiling->markDetected(false); //DGA: If the tile is outside the simulated volume, mark as undetected (by default, simulation mode marks everything as detected)
		  }
          mylib::Free_Array(im);
        }
        if(!tiling->updateActive(iplane))
        { WARN("No tiles found to image.\n");
          goto Error;
        }
		tiling->fillHolesInActive(iplane);
		if(ctx) delete ctx;
        return 1;
      Error:
        if(ctx) delete ctx;
        return 0;
      }

	  static bool sanity_checks(device::Microscope *dc){//DGA: Sanity checks regarding z max, z step, frame average count, vibratome offset and vibratome geometry
		  bool eflag = false;
		  device::Microscope::Config current_cfg = dc->get_config(); //DGA: current state of cfg
		  device::Microscope::Config cfg_as_set_by_file = *dc->cfg_as_set_by_file; //DGA: cfg state from file
		  if (current_cfg.scanner3d().zpiezo().um_max() != current_cfg.fov().z_size_um()) {warning(ENDL"\tzpiezo's um_max (Stack Acquisition --> Z Max) does not equal fov's z_size_um"ENDL); eflag = true;}
		  if (current_cfg.scanner3d().zpiezo().um_step() > 1) {warning(ENDL"\tzpiezo's um_step (Stack Acquisition --> Z Step) is greater than 1"ENDL); eflag = true;}
		  if (current_cfg.pipeline().frame_average_count() != 1) {warning(ENDL"\tpipeline's frame_average_count (Video Acquisition --> Frame Average Count) does not equal 1"ENDL); eflag = true;};
		  if (dc->vibratome()->verticalOffset() != cfg_as_set_by_file.vibratome().geometry().dz_mm()){warning(ENDL"\tvibratome geometry's dz_mm does not equal Vibratome Geometry-->Offset"ENDL); eflag = true;};
		  if (dc->vibratome()->verticalOffset()<0.5 || dc->vibratome()->verticalOffset()>3.5) {warning(ENDL"\tvibratome geometry's dz_mm (Vibratome Geometry-->Offset) is not between 0.5 and 3.5"ENDL); eflag = true;}
		  return eflag;
	  }

      unsigned int AutoTileAcquisition::run(device::Microscope *dc)
      { unsigned eflag=0; //success
        cfg::tasks::AutoTile cfg=dc->get_config().autotile();
		TiledAcquisition         nonadaptive_tiling;
        AdaptiveTiledAcquisition adaptive_tiling;
        MicroscopeTask *tile=0;
        Cut cut;
		device::StageTiling * tiling = dc->stage()->tiling(); //DGA: Pointer to tiling object
		device::Pockels * pockels1 = &(dc->scanner._scanner2d._pockels1), * pockels2 = &(dc->scanner._scanner2d._pockels2);
        tile=cfg.use_adaptive_tiling()?((MicroscopeTask*)&adaptive_tiling):((MicroscopeTask*)&nonadaptive_tiling);
		CalibrationStack calibration_stack;
		CHKJMP(0==sanity_checks(dc)); //DGA: Perform sanity checks
		while (!dc->_agent->is_stopping() && PlaneInBounds(dc, cfg.maxz_mm()))
		{
		  if ( (pockels1->get_config().has_calibration_stack() || pockels2->get_config().has_calibration_stack()) && dc->getAcquireCalibrationStack()) //DGA: If one of the pockels has the calibration stack set and a calibration stack should be acquired
		  {
			//CHKJMP(calibration_stack.config(dc));//DGA: Make sure no errors occur
			//CHKJMP(0==calibration_stack.run(dc));
		  }
          if(cfg.use_explore())
            CHKJMP(explore(dc));       // will return an error if no explorable tiles found on the plane
		  CHKJMP(tile->config(dc));
          CHKJMP(0==tile->run(dc));

          /* Assert the trip detector hasn't gone off.  
           * Trip detector will signal acq task and microscope tasks to stop, but 
           * we double check here as extra insurance against any extra cuts.
           */
          CHKJMP(dc->trip_detect.ok());
		  
          CHKJMP(   cut.config(dc));
		  dc->cutButtonWasPressed = false; //DGA: Before a cut, set cutButtonWasPressed to false
          CHKJMP(0==cut.run(dc));
		  if(tiling->useTwoDimensionalTiling_) //DGA: If using two dimensional tiling
		  {
			if (PlaneInBounds(dc,cfg.maxz_mm())) tiling->useDoneTilesAsExplorableTilesForTwoDimensionalTiling(); //DGA: If the next position is in bounds (ie, not beyond the max z), then update the tiling, otherwise do nothing.
		  }
		  else tiling->useCurrentDoneTilesAsNextExplorableTiles(); //DGA: After imaging tiles, set the next explorable tiles equal to the current done tiles
		  
		  if(dc->get_config().autotile().schedule_stop_after_nth_cut() && dc->get_config().autotile().cut_count_since_scheduled_stop() == dc->get_config().autotile().nth_cut_to_stop_after()-1) //DGA: if a stop is scheduled. TODO: NOTE! UPDATING CFG SEEMS TO CAUSE BOUNCING INTO AND OUT OF FINALIZE/ERROR below, WHICH MEANS THAT THINGS AREN'T AS UP TO DATE AS THEY SHOULD BE, HENCE THE -1 HERE
			 dc->cutCompletedSoStop(); //DGA: Call function to stop autotile
        }


	Finalize:
		dc->cutButtonWasPressed = true; //DGA: Make sure that cutButtonWasPressed is true unless otherwise specified (above)
        return eflag;
	Error:
        eflag=1;
        goto Finalize;
      }

    } // namespace microscope

  }   // namespace task
}     // namespace fetch
