#pragma once
#include <QtWidgets>
#include <devices/Microscope.h>
#include "AgentController.h"

//DGA: Controller class to interface between microscope settings (that are not defined in cfg file) and ui
namespace fetch { //DGA: This class is in namespace fetch and ui
namespace ui {

  class MicroscopeController: public QObject //DGA: Class inherits from QObject
  { Q_OBJECT //DGA: Makes it a Q_OBJECT so that it can have slots etc
    device::Microscope       *microscope_; //DGA: Creates microscope_, which is a private pointer to a Microscope device

   public:
	 //DGA: MicroscopeController constructor that takes in a pointer to a microscope device, a pointer to an agent controller object and a QObject pointer to the parent object, with default parameter 0. The base class QObject
	 //constructor is called with parent argument and microscope_ is set equal to microscope.
	 MicroscopeController(device::Microscope *microscope, AgentController *ac, QObject *parent=0);

	 QCheckBox * createSkipSurfaceFindOnImageResumeCheckBox(QWidget * parent=0); //DGA: Function to create skip surface find checkbox, which takes in parent, a pointer to type QWidget, defaulted to 0
	 QCheckBox * createScheduleStopAfterNextCutCheckBox(QWidget * parent=0); //DGA: Function to create scheduled stop checkbox, which takes in parent, a pointer to type QWidget, defaulted to 0

   public slots: 
     void setSkipSurfaceFindOnImageResume(bool); //DGA: slot to be called when need to set skipSurfaceFindOnImageResume_ in microscope_
	 void setScheduleStopAfterNextCut(bool); //DGA: slot to be called when need to set scheduleStopAfterNextCut_ in microscope_
  };

}} //end fetch::ui
