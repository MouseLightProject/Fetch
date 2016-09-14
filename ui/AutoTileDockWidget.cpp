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
    ///// Use Current Z Checkbox Control
	QCheckBox *useCurrentZCheckBox = new QCheckBox(); //DGA: Create checkbox for choosing whether or not to use current z when imaging resumes after being stopped in the middle
	useCurrentZCheckBox->setText("Use Current Z On Image Resume"); //DGA: The text for the checkbox
	QLabel * spacerLabel = new QLabel("                    "); //DGA: Spacer so that the checkbox is aligned with the text boxes
	QObject::connect(useCurrentZCheckBox, SIGNAL(clicked(bool)), parent->_stageController, SLOT(setUseCurrentZ(bool))); //DGA: When the checkbox is clicked, then the slot setUseCurrentZ function of parent->_stageController will be called with true (if checked) or false (if unchecked), which will update stage->useCurrentZ_
	QObject::connect(parent->_stageController->tiling()->listener(), SIGNAL(sig_useCurrentZSet(bool)),
					 useCurrentZCheckBox, SLOT(setChecked(bool)), Qt::DirectConnection);
	useCurrentZCheckBox->clicked(false); //DGA: Initiate the box to unchecked, this also has the benefit of initiating useCurrentZ_;
	form->addRow(spacerLabel, useCurrentZCheckBox); //DGA: Add the checkbox at the appropriate location in the widget
    AgentControllerButtonPanel *btns = new AgentControllerButtonPanel(&parent->_scope_state_controller,&dc->auto_tile_task);
    form->addRow(btns);
  }
}}//fetch::ui