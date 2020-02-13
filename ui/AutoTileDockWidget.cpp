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

	QHBoxLayout* autotileScheduleStopAfterNthCutRow = new QHBoxLayout(); //DGA: Row for setting schedule stop
	QCheckBox * scheduleStopAfterNthCutCheckBox = parent->_autotile_schedule_stop_after_nth_cut_control->createCheckBox();
	connect(&(dc->cutCompletedSoStopSignaler), SIGNAL(signaler(bool)), scheduleStopAfterNthCutCheckBox, SLOT(setChecked(bool)), Qt::QueuedConnection); //DGA: toggle checkbox off, since cutCompletedSoStop is only called when a cut was scheduled.
	connect(scheduleStopAfterNthCutCheckBox, SIGNAL(clicked(bool)), parent->_microscopeController, SLOT(updateScheduleStopAfterNthCutProperties(bool)), Qt::QueuedConnection); //DGA: When checkbox is clicked, update properties relevant to schedule stop

	scheduleStopAfterNthCutCheckBox->setText("Schedule stop after cut number: ");
	autotileScheduleStopAfterNthCutRow->addWidget(scheduleStopAfterNthCutCheckBox);
	autotileScheduleStopAfterNthCutRow->addWidget(parent->_autotile_nth_cut_to_stop_after_control->createLineEdit());
	form->addRow("", autotileScheduleStopAfterNthCutRow);

	QCheckBox * acquireCalibrationStackCheckBox = parent->_microscopeController->createAcquireCalibrationStackCheckBox(); //DGA: create acquireCalibrationStackCheckBox
	acquireCalibrationStackCheckBox->setText("Acquire Calibration Stack"); //DGA: Set the text then add the checkbox so that is aligned properly with other widgets
	form->addRow("", acquireCalibrationStackCheckBox);

    AgentControllerButtonPanel *btns = new AgentControllerButtonPanel(&parent->_scope_state_controller,&dc->auto_tile_task);
    form->addRow(btns);
  }
}}//fetch::ui