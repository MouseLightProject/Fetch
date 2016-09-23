Fetch
=====

Fetch is a program developed to image entire mouse brains by repeatedly alternating between imaging, and removing,the top brain layer. Fetch allows for control over the microscope, stage and brain slicing.


System Requirements
___________________

64-bit Windows 7
Microsoft Visual Studio 12 2013
QT 5.5
NVIDIA CUDA 7.0 and higher
CMAKE
Currently, only the src directory is located in this repository; other directories are required but are too large to include in github.


Authors
_______

Fetch was developed at the HHMI Janelia Research Campus. It was written by Nathan Clack, and the original repository can be found on his github:(https://github.com/TeravoxelTwoPhotonTomography/fetch).

Fetch is currently developed by David Ackerman.


Version History
_______________

1.0    Aug 5, 2016    Initial commit, based on fully functional 
		      code from local microscope machine, written
		      by Nathan Clack.

1.01   Sep 7, 2016    Changed folder numbering so that numbering
		      always increases, resetting to 0 when a
		      new day begins; seriesno in .microscope
		      file is now unused. Updated simulation mode
		      so that a complete autotile simulation
		      (exploration, surface find and image stack
		      acquisition) is possible. Fixed issue with
		      distance-to-edge calculation in Tiling.cpp.
		      Added this README, and LICENSE from Nathan 
		      Clack's repository.

1.011  Sep 7, 2016    Fixed README formatting.

1.02   Sep 20, 2016   Created a property and check box called
		      "Skip Surface Find on Image Resume" in 
		      Auto Tile mode. This gives the user the 
		      option to stop imaging and restart without 
		      having to do another surface find. This 
		      property is outputted in .acquisition file. 
	              Also reformatted Stage's "Lock Controls" 
		      checkbox and added version number to main 
		      window.

1.021  Sep 21, 2016   Updated "Skip Surface Find on Image Resume"
		      check box so that its checked status 
                      updates from .microscope file, and removed 
                      the corresponding property from simulated.
                      microscope file. Also changed z_tile_offset 
                      to 0 to prevent errors when resaving 
                      .microscope file.

1.022  Sep 21, 2016   Updated version number in main window.