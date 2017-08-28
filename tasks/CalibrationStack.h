/** 
  \file
  Microscope task.  Take a calibration stack at specific xyz.

  Copied/modified from AdaptiveTiledAcquisition.h by David Ackerman, otherwise:
  \author Nathan Clack <clackn@janelia.hhmi.org> (DGA)

  \copyright
  Copyright 2010 Howard Hughes Medical Institute.
  All rights reserved.
  Use is subject to Janelia Farm Research Campus Software Copyright 1.1
  license terms (http://license.janelia.org/license/jfrc_copyright_1_1.html).
 */

#pragma once

#include "task.h"
#include "devices\Stage.h"

namespace fetch {
namespace device {
  class Scanner3D;
  class Microscope;
}
}

namespace fetch
{

  namespace task
  {
    namespace microscope {

      typedef Task MicroscopeTask;

      class CalibrationStack : public MicroscopeTask
      {        
        public:
          unsigned int config(IDevice *d);
          unsigned int    run(IDevice *d);

          unsigned int config(device::Microscope *agent);
          unsigned int    run(device::Microscope *agent);

	  private:
		  Vector3f minXYZ = {0.5, 0.5, 8}, maxXYZ = {100, 100, 45};
      };

    }  // namespace microscope

  }
}
