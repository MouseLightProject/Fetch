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
    parent->_autotile_area_thresh_control->createLineEditAndAddToLayout(form);

	QCheckBox * skipSurfaceFindOnImageResumeCheckBox = parent->_microscopeController->createSkipSurfaceFindOnImageResumeCheckBox(); //DGA: create skipSurfaceFindOnImageResumeCheckBox
	skipSurfaceFindOnImageResumeCheckBox->setText("Skip Surface Find On Image Resume"); //DGA: Set the text then add the checkbox so that is aligned properly with other widgets
	form->addRow("",skipSurfaceFindOnImageResumeCheckBox);

	QCheckBox * scheduleStopAfterNextCutCheckBox = parent->_microscopeController->createScheduleStopAfterNextCutCheckBox(); //DGA: create skipSurfaceFindOnImageResumeCheckBox
	scheduleStopAfterNextCutCheckBox->setText("Schedule Stop After Next Cut"); //DGA: Set the text then add the checkbox so that is aligned properly with other widgets
	form->addRow("", scheduleStopAfterNextCutCheckBox);

    AgentControllerButtonPanel *btns = new AgentControllerButtonPanel(&parent->_scope_state_controller,&dc->auto_tile_task);
    form->addRow(btns);
  }
}}//fetch::ui