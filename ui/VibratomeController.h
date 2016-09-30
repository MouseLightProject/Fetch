#pragma once
#include <QtWidgets>
#include <devices/vibratome.h>

//DGA: Controller class to interface between vibratome settings (that are not defined in cfg file) and ui
namespace fetch { //DGA: This class is in namespace fetch and ui
namespace ui {

  class VibratomeController: public QObject //DGA: Class inherits from QObject
  { Q_OBJECT //DGA: Makes it a Q_OBJECT so that it can have slots etc
    device::Vibratome       *vibratome_; //DGA: Creates vibratome_, which is a private pointer to a Vibratome device
  
   public:
	 //DGA: VibratomeController constructor that takes in a pointer to a vibratome device and a QObject pointer to the parent object, with default parameter 0. The base class QObject
	 //constructor is called with parent argument and vibratome_ is set equal to vibratome.
	 VibratomeController(device::Vibratome *vibratome, QObject *parent=0): QObject(parent), vibratome_(vibratome){}
	 void createSliceThicknessCorrectionUmWidgets(); //DGA: Function to create slice thickness correction widgets
	 QLineEdit * sliceThicknessCorrectionUmLineEdit; //DGA: Pointer to the slice thickness correction line edit
	 QLabel *  currentSliceThicknessCorrectionUmLabel; //DGA: Pointer to the slice thickness correction label
   public slots:
     void setSliceThicknessCorrectionUm(); //DGA: slot to be called when need to set sliceThicknessCorrectionUm_ in vibratome_
  };

}} //end fetch::ui
