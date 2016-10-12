#include "devices/Vibratome.h"
#include "devices/Microscope.h"
#include "tasks/Vibratome.h"
#include "vibratome.pb.h"

namespace fetch {
namespace task {
namespace vibratome {

  unsigned int Activate::run(device::Vibratome* dc)
  { dc->start();
    while(!dc->_agent->is_stopping());
    dc->stop();
    return 1;
  }
} // end fetch::task::vibratome

namespace microscope {

  // TODO
  // [ ] emergency stop? Interuptable.
  //     - add a stage halt command
  //     - use setposnowait and then block till move completed or halted.
  //     * Too complicated

  // Notes
  // - Technically the vibratome consists of two subdevices.
  //   the stage and the motorized knife.  If I had it organized
  //   this way, the I wouldn't have to make this a Microscope Task.
  //   One ought to be able to cut and image at the same time.
  //   However, that's not really useful right now.  It's more
  //   expedient this way, and maybe it's for the best anyway.

  unsigned int Cut::config(device::Microscope* dc)
  {
    // validate
    float cx,cy,cz,ax,ay,bx,by;
    dc->stage()->getTarget(&cx,&cy,&cz);
    dc->vibratome()->feed_begin_pos_mm(&ax,&ay);
    dc->vibratome()->feed_end_pos_mm(&bx,&by);

    int a = dc->stage()->isPosValid(ax,ay,cz),
        b = dc->stage()->isPosValid(bx,by,cz);
    if(!a) warning("[Vibratome] [Task: Cut] Cut origin is out-of-bounds"ENDL);
    if(!b) warning("[Vibratome] [Task: Cut] Feed goes out of bound during cut."ENDL);
    return a&&b;
  }

  /*
     Motion:

     c --> a --> b --> c
       v0    v1    v0

     c is current point, typically where the imaging just stopped
     a is the cut start (x,y) point with a(z) = c(z) - dz
     b is the cut end
     v0 is the currently set stage velocity
     v1 is the cut velocity

  */
#define CHECK_INTERUPTED \
  if( dc->_agent->is_stopping() ) \
  { warning("Task [Vibratome::Cut] - Interupted."ENDL"\t%s(%d)"ENDL,__FILE__,__LINE__); \
    goto Error; \
  }
#define CHK(expr) \
  CHECK_INTERUPTED; \
  if(!(expr))       \
  { warning("Task [Vibratome::Cut] - Expression failed."ENDL"\t%s(%d)"ENDL"\t%s"ENDL,__FILE__,__LINE__,#expr); \
    goto Error;     \
  }

  static void save_cut_count(const int cut_count) {
      const char *path[]={"Software","Howard Hughes Medical Institute","Fetch","Microscope"};
      HKEY key=HKEY_CURRENT_USER;
      for(int i=0;i<_countof(path);++i)
        RegCreateKey(key,path[i],&key);      
      Guarded_Assert_WinErr(ERROR_SUCCESS==(
        RegSetValueEx(key,"cut_count",0,REG_DWORD,
                      (const BYTE*)&cut_count,sizeof(cut_count))));
  }

  /* MOTION
     ======                            (c*)<-- New Image Plane -----
     In-Plane:  (c)        Vertical:                               |
                / ^                    (c) Old Image Plane         | dz
               /   \                    |                          |
              /     \                dz |(a)--------------------->(b)
            (a)--->(b)                  |_| thick    ___ Knife
                Cut
  */
  /* MOTION v2
     ====== ==                         (c*)<-- New Image Plane -----          -
     In-Plane:  (c)        Vertical:                               |          |
                / ^                    (c) Old Image Plane         |<-> 0.5mm |dz
               /   \                    |                          | thick    |
              /     \                dz |(a)--------------------->(b)         -
            (a)--->(b)                  |_| thick    ___ Knife
                Cut
  */

  unsigned int Cut::run(device::Microscope* dc)
  {
    float cx,cy,cz,vx,vy,vz,ax,ay,bx,by,bz,v,dz,thick, thicknessCorrection; //DGA: Added thicknessCorrection float
	float desiredBackupDistance_mm = dc->vibratome()->backupDistanceMm();
	float minimumSafeZHeightToDropTo_mm = 8;
    // get current pos,vel
    CHK( dc->stage()->getTarget(&cx,&cy,&cz));
    CHK( dc->stage()->getVelocity(&vx,&vy,&vz));

	float actualZHeightToDropTo_mm = ((cz - desiredBackupDistance_mm) < minimumSafeZHeightToDropTo_mm) ? minimumSafeZHeightToDropTo_mm : (cz - desiredBackupDistance_mm);

    // Get parameters
    dc->vibratome()->feed_begin_pos_mm(&ax,&ay);
    dc->vibratome()->feed_end_pos_mm(&bx,&by);
    thick = dc->vibratome()->thickness_um()*0.001;      // um->mm
    dz = dc->vibratome()->verticalOffset();             // when image plan is lower than cutting plane, dz should be negative
	thicknessCorrection = dc->vibratome()->getSliceThicknessCorrectionUm()*0.001; //DGA: Get thickness correction in um
    CHK( (v = dc->vibratome()->feed_vel_mm_p_s())>0.0); // must be non-zero

    // Move to the start of the cut
    bz = cz-dz+thick + (thicknessCorrection);		// DGA: cut z position = Current Z - delta Z offset + requested slice thickness ( + thickness correction); the first subtraction gets the blade to the top of the sample
    CHK( dc->stage()->setPos(cx,cy,actualZHeightToDropTo_mm));           // Drop to safe z first
    CHK( dc->stage()->setPos(ax,ay,actualZHeightToDropTo_mm));           // Move on safe z plane to cut position
    CHK( dc->stage()->setPos(ax,ay,bz));            // Move to final plane (bz)

    // do the cut
    unsigned feedaxis=dc->vibratome()->getFeedAxis();
    CHK( dc->stage()->prepareForCut(feedaxis));     // adjust stage parameters to ensure stead motion during cut
    CHK( dc->vibratome()->start());
    CHK( dc->stage()->setVelocity(v));              // set feed velocity
    CHK( dc->stage()->setPos(bx,by,bz));            // feed (Move to end of cut position)
    CHK( dc->stage()->setPos(bx,by,actualZHeightToDropTo_mm));  // Drop to safe z first, come up at an angle -- Note: vibratome still running!
                                  //(0 to 12)   
    CHK( dc->stage()->setVelocity(vx,vy,vz));       // set back to default velocity
    CHK( dc->vibratome()->stop());					// turn off vibratome
    CHK( dc->stage()->doneWithCut(feedaxis));       // reset stage parameters

    // Move back
    CHK( dc->stage()->setPos(cx,cy,actualZHeightToDropTo_mm));           // Move on safe z plane
    CHK( dc->stage()->setPos(cx,cy,cz+thick)); //DGA: Moves the stage back to cz+thick (the desired thickness)
    
    dc->_cut_count++;
    save_cut_count(dc->_cut_count);

    return 0;
Error:
    dc->vibratome()->stop();
    return 1;
  }
    


} // end fetch::task::microscope

}} // end fetch::task::vibratome

