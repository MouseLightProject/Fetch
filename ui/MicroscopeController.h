#pragma once
#include <QtWidgets>
#include <devices/Microscope.h>

//DGA: Controller class to interface between vibratome settings (that are not defined in cfg file) and ui
namespace fetch { //DGA: This class is in namespace fetch and ui
namespace ui {

  class MicroscopeController: public QObject //DGA: Class inherits from QObject
  { Q_OBJECT //DGA: Makes it a Q_OBJECT so that it can have slots etc
    device::Microscope       *microscope_; //DGA: Creates vibratome_, which is a private pointer to a Vibratome device

   public:
	 //DGA: MicroscopeController constructor that takes in a pointer to a microscope device and a QObject pointer to the parent object, with default parameter 0. The base class QObject
	 //constructor is called with parent argument and microscope_ is set equal to microscope.
	 MicroscopeController(device::Microscope *microscope, QObject *parent=0): QObject(parent), microscope_(microscope){} 
	 QCheckBox * createSkipSurfaceFindOnImageResumeCheckBox(QWidget * parent=0); //DGA: Function to create skip surface find checkbox, which takes in parent, a pionter to type QWidget, defaulted to 0

   public slots: 
     void setSkipSurfaceFindOnImageResume(bool); //DGA: slot to be called when need to set skipSurfaceFindOnImageResume_ microscope_
  };

}} //end fetch::ui
