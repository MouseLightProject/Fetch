#include "AutoTileDockWidget.h"
#include "devices\Microscope.h"
#include "MainWindow.h"
#include "AgentController.h"

namespace fetch{
namespace ui{
  AutoTileDockWidget::AutoTileDockWidget(device::Microscope *dc, MainWindow *parent)
    : QDockWidget("Auto Tile",parent)
  { QWidget *w = new QWidget(this);
    QFormLayout *form = new QFormLayout;
    w->setLayout(form);
    setWidget(w);

    parent->_autotile_zoffum_control->createLineEditAndAddToLayout(form);
    parent->_autotile_zmaxmm_control->createLineEditAndAddToLayout(form);
    parent->_autotile_timeoutms_control->createLineEditAndAddToLayout(form);
    parent->_autotile_chan_control->createLineEditAndAddToLayout(form);
    parent->_autotile_intensity_thresh_control->createLineEditAndAddToLayout(form);
	parent->_surface_find_intensity_thresh_control->createLineEditAndAddToLayout(form); //DGA: Added this new field
    parent->_autotile_area_thresh_control->createLineEditAndAddToLayout(form);
	
	QCheckBox * skipSurfaceFindOnImageResumeCheckBox = parent->_microscopeController->createSkipSurfaceFindOnImageResumeCheckBox(); //DGA: create skipSurfaceFindOnImageResumeCheckBox
	skipSurfaceFindOnImageResumeCheckBox->setText("Skip Surface Find On Image Resume"); //DGA: Set the text then add the checkbox so that is aligned properly with other widgets
	form->addRow("",skipSurfaceFindOnImageResumeCheckBox);

	QCheckBox * scheduleStopAfterNthCutCheckBox = parent->_autotile_schedule_stop_after_nth_cut_control->createCheckBox();
	connect(&(dc->scheduledStopReachedSignaler), SIGNAL(signaler(bool)), scheduleStopAfterNthCutCheckBox, SLOT(setChecked(bool)), Qt::QueuedConnection); //DGA: toggle checkbox off, since cutCompletedSoStop is only called when a cut was scheduled.
	connect(scheduleStopAfterNthCutCheckBox, SIGNAL(toggled(bool)), parent->_microscopeController, SLOT(scheduleStopCheckBoxToggledSoUpdateConfig(bool)), Qt::QueuedConnection); //DGA: When checkbox is setChecked, update properties relevant to schedule stop

	scheduleStopAfterNthCutCheckBox->setText("Schedule stop after cut number: ");

	QLineEdit * nthCutToStopAfterLineEdit = parent->_autotile_nth_cut_to_stop_after_control->createLineEdit(); //DGA: Create line edit that is disabled when stop is scheduled
	connect(scheduleStopAfterNthCutCheckBox, SIGNAL(toggled(bool)), nthCutToStopAfterLineEdit, SLOT(setDisabled(bool)), Qt::QueuedConnection);

	connect(&(dc->cutCountChangedSignaler), SIGNAL(signaler(int)), parent->_microscopeController, SLOT(cutCountSinceScheduledStopChangedSoUpdateConfig(int))); //DGA: Ensure cut count changing is propagated

	QLabel * cutsLeftQLabel = new QLabel("");//update cuts left
	connect(&(dc->updateScheduledStopCutCountProgressSignaler), SIGNAL(signaler(QString)), cutsLeftQLabel, SLOT(setText(QString)), Qt::QueuedConnection); //DGA: Update text for progress

	QHBoxLayout* autotileScheduleStopAfterNthCutRow = new QHBoxLayout(); //DGA: Row for setting schedule stop
	autotileScheduleStopAfterNthCutRow->addWidget(scheduleStopAfterNthCutCheckBox);
	autotileScheduleStopAfterNthCutRow->addWidget(nthCutToStopAfterLineEdit);
	autotileScheduleStopAfterNthCutRow->addWidget(cutsLeftQLabel);
	form->addRow("", autotileScheduleStopAfterNthCutRow);

	//DGA: initialize cut count params/UI
	nthCutToStopAfterLineEdit->setDisabled(scheduleStopAfterNthCutCheckBox->isChecked());
	dc->cutCountSinceScheduledStopChangedSoUpdateConfig(dc->get_config().autotile().cut_count_since_scheduled_stop());
	connect(parent, SIGNAL(configUpdated()), parent->_microscopeController, SLOT(checkCutCountParametersAfterConfigUpdated())); //DGA: Update parameters when config is updated including when initially loaded 

	QCheckBox * acquireCalibrationStackCheckBox = parent->_microscopeController->createAcquireCalibrationStackCheckBox(); //DGA: create acquireCalibrationStackCheckBox
	acquireCalibrationStackCheckBox->setText("Acquire Calibration Stack"); //DGA: Set the text then add the checkbox so that is aligned properly with other widgets
	form->addRow("", acquireCalibrationStackCheckBox);

    AgentControllerButtonPanel *btns = new AgentControllerButtonPanel(&parent->_scope_state_controller,&dc->auto_tile_task);
    form->addRow(btns);
  }
}}//fetch::ui