#include "MicroscopeController.h"
#include <QtWidgets>

QCheckBox *
  fetch::ui::MicroscopeController::
  createSkipSurfaceFindOnImageResumeCheckBox(QWidget *parent)
{	QSettings settings;
	skipSurfaceFindOnImageResumeCheckBox = new QCheckBox(parent);
	connect(skipSurfaceFindOnImageResumeCheckBox, SIGNAL(clicked(bool)), this, SLOT(setSkipSurfaceFindOnImageResume(bool)));
	connect(&(microscope_->skipSurfaceFindOnImageResumeUpdater), SIGNAL(signal_somethingChanged(bool)),skipSurfaceFindOnImageResumeCheckBox, SLOT(setChecked(bool)));
	return skipSurfaceFindOnImageResumeCheckBox;
}

void 
  fetch::ui::MicroscopeController::
  setSkipSurfaceFindOnImageResume(bool setValue)
{	
  microscope_->setSkipSurfaceFindOnImageResume(setValue);
}