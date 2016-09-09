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

	///// Use Current Z Checkbox Control
	QCheckBox *b = new QCheckBox();
	//b->setEnabled(false);
	QStateMachine *useCurrentZmachine = new QStateMachine(this);
	QState *doUseCurrentZ = new QState(),
		*doNotUseCurrentZ = new QState();
	doUseCurrentZ->addTransition(b, SIGNAL(stateChanged(int)), doNotUseCurrentZ);
	doNotUseCurrentZ->addTransition(b, SIGNAL(stateChanged(int)), doUseCurrentZ);
	doUseCurrentZ->assignProperty(b, "text", "Using Current Z");
	doNotUseCurrentZ->assignProperty(b, "text", "Not Using Current Z");

	useCurrentZmachine->addState(doUseCurrentZ);
	useCurrentZmachine->addState(doNotUseCurrentZ);
	b->setCheckState(Qt::Unchecked);
	useCurrentZmachine->setInitialState(doNotUseCurrentZ);
	useCurrentZmachine->start();

    parent->_autotile_zoffum_control->createLineEditAndAddToLayout(form);
    parent->_autotile_zmaxmm_control->createLineEditAndAddToLayout(form);
    parent->_autotile_timeoutms_control->createLineEditAndAddToLayout(form);
    parent->_autotile_chan_control->createLineEditAndAddToLayout(form);
    parent->_autotile_intensity_thresh_control->createLineEditAndAddToLayout(form);
    parent->_autotile_area_thresh_control->createLineEditAndAddToLayout(form);
//	parent->_autotile_use_current_z->createCheckBoxAndAddToLayout(form);
//	form->addRow("Use Current Z: ", b);
//	parent->controller
//	QObject::connect(b, SIGNAL(stateChanged(int)), &parent->_scope_state_controller, SLOT());

	QObject::connect(b, SIGNAL(clicked(bool)), parent->_stageController, SLOT(toggleUseCurrentZ(bool)));
	form->addRow("Use Current Z: ", b);
    AgentControllerButtonPanel *btns = new AgentControllerButtonPanel(&parent->_scope_state_controller,&dc->auto_tile_task);
    form->addRow(btns);
  }
}}//fetch::ui