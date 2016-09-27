#pragma once
#include <QtWidgets>
#include <devices/vibratome.h>

namespace fetch {
namespace ui {

  class VibratomeController:public QObject
  {
    Q_OBJECT
  public:
	VibratomeController(device::Vibratome *vibratome, QObject *parent=0): QObject(parent), vibratome_(vibratome){}

	QLineEdit * createThicknessCorrectionUmLineEdit(QWidget * parent=0);
	QLineEdit * thicknessCorrectionUmLineEdit;
	void setSliceThicknessCorrection(float um);
	float getSliceThicknessCorrection();

  public slots:
    void setSliceThicknessCorrection();//                                     {vibratome_->setThicknessCorrection_um(4.0);}

  private:
    device::Vibratome       *vibratome_;
  };

}} //end fetch::ui
