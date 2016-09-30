#include "MicroscopeController.h"
#include <QtWidgets>

QCheckBox *
  fetch::ui::MicroscopeController::
  createSkipSurfaceFindOnImageResumeCheckBox(QWidget *parent) //DGA: Definition of function from header
{	skipSurfaceFindOnImageResumeCheckBox = new QCheckBox(parent);  //DGA: Dynamically allocates check box
	connect(skipSurfaceFindOnImageResumeCheckBox, SIGNAL(clicked(bool)), this, SLOT(setSkipSurfaceFindOnImageResume(bool))); //DGA: Connects check box's clicked signal to the setSkipSurfaceFindOnImageResume slot of this class instance, so that skipSurfaceFindOnImageResume_ of microscope_ will be updated when the checkbox is clicked
	connect(&(microscope_->skipSurfaceFindOnImageResumeCheckBoxUpdater), SIGNAL(signal_valueSet(bool)),skipSurfaceFindOnImageResumeCheckBox, SLOT(setChecked(bool))); //DGA: Connect signal_valueSet of microscope_'s skipSurfaceFindOnImageResumeUpdater to setChecked slot so that changing skipSurfaceFindOnImageResume_ will update the checkbox
	skipSurfaceFindOnImageResumeCheckBox->clicked(false);//DGA: Initialize the checkbox to off
	return skipSurfaceFindOnImageResumeCheckBox; //DGA: Returns the checkbox
}

void 
  fetch::ui::MicroscopeController::
  setSkipSurfaceFindOnImageResume(bool setValue) //DGA: setSkipSurfaceFindOnImageResume slot that takes in the setValue bool
{	
  microscope_->setSkipSurfaceFindOnImageResume(setValue); //DGA: sets micrsocope_->skipSurfaceFindOnImageResume_ to setValue
}