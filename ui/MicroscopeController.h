#pragma once
#include <QtWidgets>
#include <devices/Microscope.h>

namespace fetch {
namespace ui {

  class MicroscopeController: public QObject
  {
    Q_OBJECT
  public:
	MicroscopeController(device::Microscope *microscope, QObject *parent=0): QObject(parent), microscope_(microscope){}

	QCheckBox * createSkipSurfaceFindOnImageResumeCheckBox(QWidget * parent=0);
	QCheckBox * skipSurfaceFindOnImageResumeCheckBox;	
	public slots: 
		void setSkipSurfaceFindOnImageResume(bool);
  private:
    device::Microscope       *microscope_;
  };

}} //end fetch::ui
