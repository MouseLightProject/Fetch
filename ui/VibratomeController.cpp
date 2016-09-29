#include "VibratomeController.h"
#include <QtWidgets>

QLineEdit*
  fetch::ui::VibratomeController::
  createThicknessCorrectionUmLineEdit(QWidget *parent)
{	QSettings settings;
	thicknessCorrectionUmLineEdit = new QLineEdit(parent);
	currentThicknessCorrectionUm = new QLabel(parent);
	connect(thicknessCorrectionUmLineEdit, SIGNAL(editingFinished()), this, SLOT(setSliceThicknessCorrection()));
	connect(&(vibratome_->thicknessUpdater), SIGNAL(signal_somethingChanged(QString)),thicknessCorrectionUmLineEdit, SLOT(setText(QString)));
	connect(&(vibratome_->thicknessUpdater), SIGNAL(signal_somethingChanged(QString)),currentThicknessCorrectionUm, SLOT(setText(QString)));
	thicknessCorrectionUmLineEdit->setText(QString::number(settings.value("thicknessCorrectionUm").toFloat()));
	thicknessCorrectionUmLineEdit->editingFinished();
	return thicknessCorrectionUmLineEdit;
}

void 
  fetch::ui::VibratomeController::
  setSliceThicknessCorrection()
{	QSettings settings;
	bool ok;
	float newThicknessCorrection = thicknessCorrectionUmLineEdit->text().toFloat(&ok);
	if (ok && abs(newThicknessCorrection)<1000){ //DGA: Conversion was successful and is a valid number
		vibratome_->setThicknessCorrection_um(newThicknessCorrection);
	}
	else{
		thicknessCorrectionUmLineEdit->setText(QString::number(getSliceThicknessCorrection()));
	}
}

void 
  fetch::ui::VibratomeController::
  setSliceThicknessCorrection(float um)
{
	vibratome_->setThicknessCorrection_um(um);
}

float 
  fetch::ui::VibratomeController::
  getSliceThicknessCorrection()
{
	return vibratome_->getThicknessCorrection_um();
}