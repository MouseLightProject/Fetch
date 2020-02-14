#pragma once

#include "task.h"

namespace fetch{

// Forward declarations
namespace device { 
  class Vibratome; 
  class Microscope;
}

namespace task {
namespace vibratome {

  // Just run the vibratome
  class Activate : public TTask<device::Vibratome>
  { public:
    virtual unsigned int run(device::Vibratome* dc);
  };

} // end fetch::task::vibratome

namespace microscope {  

  // Turn the vibratome on and feed using the stage.
  class Cut : public TTask<device::Microscope>
  { public:
    virtual unsigned int config(device::Microscope* dc);
    virtual unsigned int run(device::Microscope* dc);
  };

  static void save_cut_count(const int cut_count) { //DGA: Moved from Vibratome.cpp to here so that it can be accessible in VibratomeDockWidget.cpp
	  const char *path[] = { "Software","Howard Hughes Medical Institute","Fetch","Microscope" };
	  HKEY key = HKEY_CURRENT_USER;
	  for (int i = 0; i<_countof(path); ++i)
		  RegCreateKey(key, path[i], &key);
	  Guarded_Assert_WinErr(ERROR_SUCCESS == (
		  RegSetValueEx(key, "cut_count", 0, REG_DWORD,
		  (const BYTE*)&cut_count, sizeof(cut_count))));
  }

} // fetch::task::microscope

}}
