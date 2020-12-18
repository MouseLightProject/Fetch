/*
 * Microscope.h
 *
 * Author: Nathan Clack <clackn@janelia.hhmi.org>
 *   Date: Apr 28, 2010
 */
/*
 * Copyright 2010 Howard Hughes Medical Institute.
 * All rights reserved.
 * Use is subject to Janelia Farm Research Campus Software Copyright 1.1
 * license terms (http://license.janelia.org/license/jfrc_copyright_1_1.html).
 */
#pragma once

#include "agent.h"
#include "task.h"
#include "microscope.pb.h"

#include "workers/Pipeline.h"
#include "workers/Terminator.h"
#include "workers/TripDetect.h"
#include "workers/SurfaceFindWorker.h"

#include "devices/scanner3D.h"
#include "devices/DiskStream.h"
#include "devices/LinearScanMirror.h"
#include "devices/pockels.h"
#include "devices/Stage.h"
#include "devices/FieldOfViewGeometry.h"
#include "devices/Vibratome.h"
#include "devices/Probe.h"
#include "tasks/microscope-interaction.h"
#include "tasks/StackAcquisition.h"
#include "tasks/TiledAcquisition.h"
#include "tasks/Vibratome.h"
#include "tasks/AutoTileAcquisition.h"
#include "tasks/TiledSurfaceScan.h"
#include "tasks/TimeSeries.h"

#include <string>
#include <set>

#define MICROSCOPE_MAX_WORKERS     10
#define MICROSCOPE_DEFAULT_TIMEOUT INFINITE
#include "pmt.h"


namespace fetch
{

  bool operator==(const cfg::device::Microscope& a, const cfg::device::Microscope& b);
  bool operator!=(const cfg::device::Microscope& a, const cfg::device::Microscope& b);

  namespace device
  {

    //TODO: move this into it's own header and out of namespace fetch::device (move to fetch::)
    struct FileSeriesListener
    {
      virtual void update(const std::string& path) = 0;
    };

    class FileSeries
    {
      typedef std::set<FileSeriesListener*> TListeners;
    public:
      FileSeries() :_desc(&__default_desc), _is_valid(false) {};
      FileSeries(cfg::FileSeries *desc) :_desc(desc), _is_valid(false) { Guarded_Assert(_desc != NULL); }

      FileSeries& inc(bool increment = true); //DGA: Added boolean for setting whether or not to increment seriesno; if not, then just checking date for resetting seriesno
      const std::string getFullPath(const std::string& prefix, const std::string& ext);
      const std::string getPath();
      bool updateDesc(cfg::FileSeries *desc);
      bool ensurePathExists();
      inline bool is_valid() { return _is_valid; };

      void addListener(FileSeriesListener *x) { _listeners.insert(x); }

    private:
      void renderSeriesNo(char * strSeriesNo, int maxbytes);
      void tryCreateDirectory(LPCTSTR root_date, const char* description, LPCTSTR root);
      void updateDate(void);
      std::string _lastpath;
      cfg::FileSeries __default_desc;
      bool _is_valid;

      void notify();

    public: // pseudo-private
      cfg::FileSeries *_desc;
      TListeners _listeners;
    };

    class Microscope : public IConfigurableDevice<cfg::device::Microscope>
    {
    public:
      // Will on_attach and arm the default task on construction
      Microscope();
      Microscope(const Config &cfg);
      Microscope(Config *cfg);

      void __common_setup();

      ~Microscope();

      unsigned int on_attach(void);
      unsigned int on_detach(void);

      unsigned int on_disarm();

      virtual void _set_config(Config IN *cfg);
      virtual void _set_config(const Config& cfg);
      virtual void onUpdate();

      IDevice* configPipeline();                                           // returns the end of the pipeline
      unsigned int runPipeline();
      unsigned int stopPipeline();

      // inline IDevice* pipelineEnd() {return &wrap;}

    public:
      const std::string stack_filename();                                  // get the current file
      const std::string config_filename();                                 // get the current file
      const std::string metadata_filename();
      void write_stack_metadata();

    public:
      device::Scanner3D                     scanner;
      device::Stage                         stage_;
      device::Vibratome                     vibratome_;
      device::PMT                           pmt_;
      device::FieldOfViewGeometry           fov_;
      device::Probe                         surface_probe_;

      worker::PipelineAgent                 pipeline;
      worker::TripDetectWorkerAgent         trip_detect;
      worker::SurfaceFindWorkerAgent        surface_finder;

      worker::TerminalAgent		            trash;
      device::TiffGroupStream               disk;

      task::microscope::Interaction         interaction_task;
      task::microscope::StackAcquisition    stack_task;
      task::microscope::TiledAcquisition    tiling_task;
      task::microscope::Cut                 cut_task;
      task::microscope::AutoTileAcquisition auto_tile_task;
      task::microscope::TiledSurfacescan    surface_scan_task;
      task::microscope::TimeSeries          time_series_task;
      int _cut_count;

      mylib::Array* snapshot(float dz_um, unsigned timeout_ms);

      inline Chan*  getVideoChannel() { if (_end_of_pipeline && _end_of_pipeline->_out) return _end_of_pipeline->_out->contents[0]; else return NULL; }
      inline LinearScanMirror*  LSM() { return &scanner._scanner2d._LSM; }

      Pockels* pockels(const std::string& k) { return scanner.get2d()->pockels(k); }
      Pockels* pockels(unsigned k) { return scanner.get2d()->pockels(k); }
      Pockels* pockels(cfg::device::Pockels::LaserLineIdentifier k) { return scanner.get2d()->pockels(k); }

      inline ZPiezo*         zpiezo() { return &scanner._zpiezo; }
      inline Stage*           stage() { return &stage_; }
      inline Vibratome*   vibratome() { return &vibratome_; }
      inline Probe*    surfaceProbe() { return &surface_probe_; }

      int updateFovFromStackDepth(int nowait = 0);  // These also account for cut thickness, returns 0 if overlap is not positive.
      int updateStackDepthFromFov(int nowait = 0);

      unsigned int moveToNewPosThroughSafeZ(Vector3f pos); //DGA: Move to a new position by first moving the stage to a safe z location

      float safeZtoLowerTo_mm(float current_z); //DGA: This will output the z height for the stage to be lowered to based on the minimum z stage height and the desired backup distance

      void scheduleStopCheckBoxToggledSoUpdateConfig(bool setChecked); //DGA: Function setter prototype for updating schedule stop after nth cut properties
      void cutCountSinceScheduledStopChangedSoUpdateConfig(int cutCountSinceScheduledStop); //DGA: Function setter prototype for cut count since scheduled stop changed
      void updateScheduleStopCutCountProgress(device::Microscope::Config c); //DGA: Change cut count progress text

      bool getSkipSurfaceFindOnImageResume() { return skipSurfaceFindOnImageResume_; }; //DGA: Getter for skipSurfaceFindOnImageResume_
      void setSkipSurfaceFindOnImageResume(bool setValue); //DGA: Function setter prototype for skipSurfaceFindOnImageResume_

      bool getAcquireCalibrationStack() { return acquireCalibrationStack_; }; //DGA: Getter for acquireCalibrationStack_
      void setAcquireCalibrationStack(bool setValue); //DGA: Function setter prototype for acquireCalibrationStack_

      void scheduledStopReached() { scheduledStopReachedSignaler.signaler(false); scheduledStopReachedSignaler.signaler(); }; //DGA: Function to call signaler which stops the task
      void cutCountChanged(int cutCount, int cutCountSinceScheduledStop) { cutCountChangedSignaler.signaler(QString("Current Cut Count: %1. Reset Cut Count To 0?").arg(cutCount)); if (cutCountSinceScheduledStop > 0) { cutCountChangedSignaler.signaler(cutCountSinceScheduledStop); } }; //DGA: Function to call signaler when vibratome completes cut



    public:
      FileSeries file_series;

    public:
      IDevice* _end_of_pipeline;
	  ui::simpleSignalerClass skipSurfaceFindOnImageResumeCheckBoxUpdater, scheduledStopReachedSignaler, acquireCalibrationStackCheckBoxUpdater, cutCountChangedSignaler, updateScheduledStopCutCountProgressSignaler, updateCurrentLatticePositionSignaler; //DGA: Updater for skipSurfaceFindOnImageResumeCheckBox, acquireCalibrationStackCheckBox and signaler for scheduledStopReachedSignaler and cutCountChangedSignaler currentLatticePositionSignaler
	
      Agent __self_agent;
      Agent __scan_agent;
      Agent __io_agent;
      Agent __vibratome_agent;
      float    minimumBackupDistance_mm = 0.5, minimumSafeZHeightToDropTo_mm = 8;
      bool cutButtonWasPressed = true; //DGA: By default, set cutButtonWasPressed to true so that when it is pressed, this is correct; if the cut occurs during autotile cutBottonWasPressed will have been set to false
      fetch::cfg::device::Microscope*      cfg_as_set_by_file; //DGA: Keep track of configuration as set by file
    private:
      bool skipSurfaceFindOnImageResume_, acquireCalibrationStack_; //DGA: Private variables storing whether or not to skip surface find or schedule a stop or acquire a calibration stack
    };
    //end namespace fetch::device
  }
}
