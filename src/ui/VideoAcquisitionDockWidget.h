#include <QtWidgets>
#include "devices/Microscope.h"
#include "AgentController.h"
#include "MainWindow.h"
namespace fetch{

  namespace ui{

    class VideoAcquisitionDockWidget:public QDockWidget
    {
      Q_OBJECT

    public:
      typedef device::Microscope::Config Config;
      VideoAcquisitionDockWidget(device::Microscope *dc, MainWindow* parent);
	  QSpinBox * trigger_holdoff_spinbox; //DGA
        
    signals:
      void onRun();

	public slots:
		void setTriggerHoldoffStep(int v);
    //private:      
    //  void createForm();
    };

    //end namespace fetch::ui
  }
}
