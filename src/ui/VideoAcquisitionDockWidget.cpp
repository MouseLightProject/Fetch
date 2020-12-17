#include "VideoAcquisitionDockWidget.h"
#include "devices\microscope.h"
#include "AgentController.h"

namespace fetch {
namespace ui {



  VideoAcquisitionDockWidget::VideoAcquisitionDockWidget(device::Microscope *dc, MainWindow *parent)
    :QDockWidget("Video Acquisition",parent)
  {
    QWidget *formwidget = new QWidget(this);
    QFormLayout *form = new QFormLayout;
    formwidget->setLayout(form);
    setWidget(formwidget);
	
    parent->_resonant_turn_controller->createLineEditAndAddToLayout(form);
    parent->_vlines_controller->createLineEditAndAddToLayout(form);
    parent->_lsm_vert_range_controller->createLineEditAndAddToLayout(form);
    parent->_pockels_controllers[0]->createLineEditAndAddToLayout(form);
    parent->_pockels_controllers[1]->createLineEditAndAddToLayout(form);
	parent->_frame_average_count_controller->createLineEditAndAddToLayout(form);//DGA: Added this so user can modify frame average count from GUI

	// if simulated or vdaq, then add 
	int digitizer_kind = dc->scanner._scanner2d._digitizer.get_config().kind();
	if (digitizer_kind == cfg::device::Digitizer_DigitizerType_vDAQ || digitizer_kind == cfg::device::Digitizer_DigitizerType_Simulated) {	//DGA: if vDAQ or simulated
		QGridLayout *row = new QGridLayout();

		trigger_holdoff_spinbox = parent->_trigger_holdoff_controller->createSpinBox();
		trigger_holdoff_spinbox->setRange(0, 15000);
		row->addWidget(trigger_holdoff_spinbox, 0, 1);

		QSpinBox *s = new QSpinBox();
		s->setRange(1, 100.0);
		s->setValue(1); //initialize to step size 1s
		connect(s, SIGNAL(valueChanged(int)), this, SLOT(setTriggerHoldoffStep(int)));
		row->addWidget(s, 0, 2);

		form->addRow(new QLabel("Trigger Holdoff "),row);
	}
    AgentControllerButtonPanel *btns = new AgentControllerButtonPanel(&parent->_scope_state_controller,&dc->interaction_task);
    form->addRow(btns);      

    connect(btns->controller(),SIGNAL(onRun()),this,SIGNAL(onRun()));
  }

  void VideoAcquisitionDockWidget::setTriggerHoldoffStep(int v)
  {
	trigger_holdoff_spinbox->setSingleStep(v);
  }


//end namespace fetch::ui
}
}
