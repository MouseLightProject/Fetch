#include "MicroscopeController.h"
#include <QtWidgets>

  fetch::ui::MicroscopeController::
  MicroscopeController(device::Microscope *microscope, AgentController *ac, QObject *parent): 
  QObject(parent), 
  microscope_(microscope)
{ //DGA: MicroscopeController constructor which initializes microscope_ to microscope and QObject with argument parent
    connect(&(microscope_->scheduledStopReachedSignaler),SIGNAL(signaler()),ac,SLOT(stop()), Qt::BlockingQueuedConnection); //DGA: Connects signal_setValue() of cutCompletedSoStopSignaler to ac
} 

QCheckBox *
  fetch::ui::MicroscopeController::
  createSkipSurfaceFindOnImageResumeCheckBox(QWidget *parent) //DGA: Definition of function from header
{	QCheckBox *skipSurfaceFindOnImageResumeCheckBox = new QCheckBox(parent);  //DGA: Dynamically allocates check box
	connect(skipSurfaceFindOnImageResumeCheckBox, SIGNAL(clicked(bool)), this, SLOT(setSkipSurfaceFindOnImageResume(bool)),Qt::QueuedConnection); //DGA: Connects check box's clicked signal to the setSkipSurfaceFindOnImageResume slot of this class instance, so that skipSurfaceFindOnImageResume_ of microscope_ will be updated when the checkbox is clicked
	connect(&(microscope_->skipSurfaceFindOnImageResumeCheckBoxUpdater), SIGNAL(signaler(bool)),skipSurfaceFindOnImageResumeCheckBox, SLOT(setChecked(bool)),Qt::QueuedConnection); //DGA: Connect signaler of microscope_'s skipSurfaceFindOnImageResumeCheckBoxUpdater to setChecked slot so that changing skipSurfaceFindOnImageResume_ will update the checkbox
	skipSurfaceFindOnImageResumeCheckBox->clicked(false);//DGA: Initialize the checkbox to off
	return skipSurfaceFindOnImageResumeCheckBox; //DGA: Returns the checkbox
}

QCheckBox *
  fetch::ui::MicroscopeController::
  createAcquireCalibrationStackCheckBox(QWidget *parent) //DGA: Definition of function from header
{	QCheckBox *acquireCalibrationStackCheckBox = new QCheckBox(parent);  //DGA: Dynamically allocates check box
	connect(acquireCalibrationStackCheckBox, SIGNAL(clicked(bool)), this, SLOT(setAcquireCalibrationStack(bool)),Qt::QueuedConnection); //DGA: Connects check box's clicked signal to the acquireCalibrationStack slot of this class instance, so that acquireCalibrationStack_ of microscope_ will be updated when the checkbox is clicked
	connect(&(microscope_->acquireCalibrationStackCheckBoxUpdater), SIGNAL(signaler(bool)),acquireCalibrationStackCheckBox, SLOT(setChecked(bool)),Qt::QueuedConnection); //DGA: Connect signaler of microscope_'s acquireCalibrationStackUpdater to setChecked slot so that changing acquireCalibrationStack_ will update the checkbox
	acquireCalibrationStackCheckBox->clicked(false);//DGA: Initialize the checkbox to off
	return acquireCalibrationStackCheckBox; //DGA: Returns the checkbox
}

void
fetch::ui::MicroscopeController::
checkCutCountParametersAfterConfigUpdated() //DGA: checkCutCountParametersAfterConfigUpdated slot that is called when config is updated
{
	microscope_->cutCountSinceScheduledStopChangedSoUpdateConfig(microscope_->get_config().autotile().cut_count_since_scheduled_stop()); //DGA: updates schedule stop after nth cut properties
}

void
fetch::ui::MicroscopeController::
scheduleStopCheckBoxToggledSoUpdateConfig(bool setValue) //DGA: scheduleStopCheckBoxToggledSoUpdateConfig slot that takes in the setValue bool
{
	microscope_->scheduleStopCheckBoxToggledSoUpdateConfig(setValue); //DGA: updates schedule stop after nth cut properties
}

void
fetch::ui::MicroscopeController::
cutCountSinceScheduledStopChangedSoUpdateConfig(int setValue) //DGA: cutCountSinceScheduledStopChangedSoUpdateConfig slot that takes in the setValue bool
{ 
	microscope_->cutCountSinceScheduledStopChangedSoUpdateConfig(setValue); //DGA: updates schedule stop after nth cut properties
}

void 
  fetch::ui::MicroscopeController::
  setSkipSurfaceFindOnImageResume(bool setValue) //DGA: setSkipSurfaceFindOnImageResume slot that takes in the setValue bool
{	
  microscope_->setSkipSurfaceFindOnImageResume(setValue); //DGA: sets microscope_->skipSurfaceFindOnImageResume_ to setValue
}

void 
  fetch::ui::MicroscopeController::
  setAcquireCalibrationStack(bool setValue) //DGA: setAcquireCalibrationStack slot that takes in the setValue bool
{	
  microscope_->setAcquireCalibrationStack(setValue); //DGA: sets microscope_->acquireCalibrationStack_ to setValue
}