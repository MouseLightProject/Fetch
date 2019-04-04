# Fetch

Fetch is a program developed to image entire mouse brains by repeatedly alternating between imaging, and removing, the top brain layer. Fetch allows for control over the microscope, stage and brain slicing.


## System Requirements
64-bit Windows 7
Microsoft Visual Studio 12 2013
QT 5.5
NVIDIA CUDA 7.0 and higher
CMAKE
Currently, only the src directory is located in this repository; other directories are required but are too large to include in github.


## Installation Instructions
1.	Download opensource qt5.5 WITH tools selected as well, nvidia 7.5, visual studio 12 2013, protoc.exe and replace the old protoc.exe in C:\Users\ackermand\Google Drive\Janelia\ScientificComputing\src\3rdParty\protobuf-2.6.1\vsprojects\Debug
2.	Open cmake, turn off developer warnings CHECK ADVANCED BOX.
3.	Select source as /src/fetch/
4.	Select a clean directory to build the binaries
5.	Add below, by clicking add entry. Put the desired path etc in the value section after you enter the name.
	*	PROTOBUF_INCLUDE_DIR:PATH=/path/to/src/3rdParty/protobuf-2.6.1/src
	*	PROTOBUF_LIBRARY:FILEPATH=/path/to/src/3rdParty/protobuf-2.6.1/vsprojects/x64/Debug/libprotobuf.lib
	*	PROTOBUF_PROTOC_EXECUTABLE:FILEPATH=/path/to/src/3rdParty/protobuf-2.6.1/vsprojects/Debug/protoc.exe
	*	MYLIB_INCLUDE_DIR:PATH=/path/to/src/mylib/
6.	Suppress developer warnings, press Configure and select Windows Visual Studio 12 2013 Win64 and click finish
7.	An error will appear. Change directories as follows:
	*	Qt5Core_DIR:PATH=C:/Qt/5.5/msvc2013_64/lib/cmake/Qt5Core
	*	MYLIB_MYFFT_LIBRARY_D:FILEPATH=/path/to/src/mylib/Visual Studio 10 Win64/Debug/myfft.lib
	*	MYLIB_MYFFT_LIBRARY_R:STRING=/path/to/src/mylib/Visual Studio 10 Win64/Release/myfft.lib
	*	MYLIB_MYLIB_LIBRARY_D:FILEPATH=/path/to/src/mylib/Visual Studio 10 Win64/Debug/mylib.lib
	*	MYLIB_MYLIB_LIBRARY_R:STRING=/path/to/src/mylib/Visual Studio 10 Win64/Release/mylib.lib
	*	MYLIB_MYTIFF_LIBRARY_D:FILEPATH=/path/to/src/mylib/Visual Studio 10 Win64/Debug/mytiff.lib
	*	MYLIB_MYTIFF_LIBRARY_R:STRING=/path/to/src/mylib/Visual Studio 10 Win64/Release/mytiff.lib
	*	CUDA_TOOLKIT_INCLUDE:PATH=C:/
8.	Press configure again (it will find more directories). Then press it again. Then click generate.
9.	Open fetch.sln, and change fetch to startup project and build.
10.	Copy glew32.dll (can be from parent directory or build2/debug/ etc.) into Debug, and Qt5Cored.dll, Qt5Guid.dll, Qt5Networkd.dll, Qt5OpenGLd.dll, Qt5PrintSupportd.dll, Qt5Widgetsd.dll from C:\Qt\5.5\msvc2013_64\bin into Debug

## Authors
Fetch was developed at the HHMI Janelia Research Campus. It was written by Nathan Clack, and the original repository can be found on his github:(https://github.com/TeravoxelTwoPhotonTomography/fetch).

Fetch is currently developed by David Ackerman.

## Version History

* 1.0    Aug 5, 2016:    Initial commit, based on fully functional code from local
		      microscope machine, written by Nathan Clack.

* 1.01   Sep 7, 2016:    Changed folder numbering so that numbering always 
		      increases, resetting to 0 when a new day begins; seriesno 
		      in .microscope file is now unused. Updated simulation 
		      mode so that a complete autotile simulation (exploration, 
		      surface find and image stack acquisition) is possible. 
		      Fixed issue with distance-to-edge calculation in Tiling.cpp. 
	              Added this README, and LICENSE from Nathan Clack's 
		      repository.

* 1.011  Sep 7, 2016:    Fixed README formatting.

* 1.02   Sep 20, 2016:   Created a property and check box called "Skip Surface Find 
		      on Image Resume" in Auto Tile mode. This gives the user the 
		      option to stop imaging and restart without having to do 
		      another surface find. This property is outputted in 
		      .acquisition file. Also reformatted Stage's "Lock Controls" 
		      checkbox and added version number to main window.

* 1.021  Sep 21, 2016:   Updated "Skip Surface Find on Image Resume" check box so 
		      that its checked status updates from .microscope file, and 
		      removed the corresponding property from simulated.microscope 
		      file. Also changed z_tile_offset to 0 to prevent errors when 
		      resaving .microscope file.

* 1.022  Sep 21, 2016:   Updated version number in main window.

* 1.03-beta-1 Sep 23, 2016:    Enable user to enter slice thickness correction 
		            with accompanying locked checkbox. Updated checkbox
			    alignment for other widgets.

* 1.03   Sep 30, 2016:   Slice thickness correction and skip surface find are no 
		      longer configuration properties, but are instead QSettings. 
		      Added field that displays the current slice thickness 
		      correction setting.

* 1.04   Oct 17, 2016:   TCP server is started by default. The desired stage backup 
		      distance is settable in the .microscope file. The "done" 
		      tiles in one slice are used as the "explorable" tiles in 
		      the next slice. The color of the "detected" tiles is now 
		      dark blue.

* 1.1    Nov 11, 2016:   2D tiling is now possible and can be chosen via a .microscope 
		      setting. Surface find now skips tiles in two dimensions by 
		      the requisite amount. A stop can be scheduled to occur after 
	              the next cut. If imaging is stopped in 3D mode, it will 
                      resume at the current stage's z coordinate. The lattice z 
  		      position in the .acquisition file is replaced by the cut 
		      count. Tile selection now corresponds to tile centers, and 
		      explored tiles are colored purple.

* 1.2    Apr 18, 2017:   Image contrast can be controlled automatically or manually via
		      sliders. Restrictions are placed on how much the stage can 
		      lowered or raised during surface find. Tile dilation is performed
		      after surface find, and occurs once per slice every time an
		      explore occurs.
