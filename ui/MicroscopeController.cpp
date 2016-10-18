#include "MicroscopeController.h"
#include <QtWidgets>

QCheckBox *
  fetch::ui::MicroscopeController::
  createSkipSurfaceFindOnImageResumeCheckBox(QWidget *parent) //DGA: Definition of function from header
{	QCheckBox *skipSurfaceFindOnImageResumeCheckBox = new QCheckBox(parent);  //DGA: Dynamically allocates check box
	connect(skipSurfaceFindOnImageResumeCheckBox, SIGNAL(clicked(bool)), this, SLOT(setSkipSurfaceFindOnImageResume(bool)),Qt::QueuedConnection); //DGA: Connects check box's clicked signal to the setSkipSurfaceFindOnImageResume slot of this class instance, so that skipSurfaceFindOnImageResume_ of microscope_ will be updated when the checkbox is clicked
	connect(&(microscope_->skipSurfaceFindOnImageResumeCheckBoxUpdater), SIGNAL(signal_valueSet(bool)),skipSurfaceFindOnImageResumeCheckBox, SLOT(setChecked(bool)),Qt::QueuedConnection); //DGA: Connect signal _valueSet of microscope_'s skipSurfaceFindOnImageResumeCheckBoxUpdater to setChecked slot so that changing skipSurfaceFindOnImageResume_ will update the checkbox
	skipSurfaceFindOnImageResumeCheckBox->clicked(false);//DGA: Initialize the checkbox to off
	return skipSurfaceFindOnImageResumeCheckBox; //DGA: Returns the checkbox
}

QCheckBox *
  fetch::ui::MicroscopeController::
  createScheduleStopAfterNextCutCheckBox(QWidget *parent) //DGA: Definition of function from header
{	QCheckBox *scheduleStopAfterNextCutCheckBox = new QCheckBox(parent);  //DGA: Dynamically allocates check box
	connect(scheduleStopAfterNextCutCheckBox, SIGNAL(clicked(bool)), this, SLOT(setScheduleStopAfterNextCut(bool)),Qt::QueuedConnection); //DGA: Connects check box's clicked signal to the scheduleStopAfterNextCut slot of this class instance, so that scheduleStopAfterNextCut_ of microscope_ will be updated when the checkbox is clicked
	connect(&(microscope_->scheduleStopAfterNextCutCheckBoxUpdater), SIGNAL(signal_valueSet(bool)),scheduleStopAfterNextCutCheckBox, SLOT(setChecked(bool)),Qt::QueuedConnection); //DGA: Connect signal _valueSet of microscope_'s scheduleStopAfterNextCutCheckBoxUpdater to setChecked slot so that changing skipSurfaceFindOnImageResume_ will update the checkbox
	scheduleStopAfterNextCutCheckBox->clicked(false);//DGA: Initialize the checkbox to off
	return scheduleStopAfterNextCutCheckBox; //DGA: Returns the checkbox
}

void 
  fetch::ui::MicroscopeController::
  setSkipSurfaceFindOnImageResume(bool setValue) //DGA: setSkipSurfaceFindOnImageResume slot that takes in the setValue bool
{	
  microscope_->setSkipSurfaceFindOnImageResume(setValue); //DGA: sets micrsocope_->skipSurfaceFindOnImageResume_ to setValue
}

void 
  fetch::ui::MicroscopeController::
  setScheduleStopAfterNextCut(bool setValue) //DGA: setSkipSurfaceFindOnImageResume slot that takes in the setValue bool
{	
  microscope_->setScheduleStopAfterNextCut(setValue); //DGA: sets micrsocope_->skipSurfaceFindOnImageResume_ to setValue
}