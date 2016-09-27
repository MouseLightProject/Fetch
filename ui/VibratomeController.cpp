#include "VibratomeController.h"
#include <QtWidgets>

QLineEdit*
  fetch::ui::VibratomeController::
  createThicknessCorrectionUmLineEdit(QWidget *parent)
{	QSettings settings;
	thicknessCorrectionUmLineEdit = new QLineEdit(parent);
	currentThicknessCorrectionUm = new QLabel(parent);
	connect(thicknessCorrectionUmLineEdit, SIGNAL(editingFinished()), this, SLOT(setSliceThicknessCorrection()));
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
	if (ok && abs(newThicknessCorrection)<1000){ //DGA: Conversion was success and is a valid number
		vibratome_->setThicknessCorrection_um(newThicknessCorrection);
		currentThicknessCorrectionUm->setText(thicknessCorrectionUmLineEdit->text());
		settings.setValue("thicknessCorrectionUm", getSliceThicknessCorrection());
	}
	else{
		thicknessCorrectionUmLineEdit->setText(QString::number(getSliceThicknessCorrection()));
	}
}

void 
  fetch::ui::VibratomeController::
  setSliceThicknessCorrection(float um)
{	QSettings settings;
	vibratome_->setThicknessCorrection_um(um);
	QString sliceThicknessCorrectionText = QString::number(getSliceThicknessCorrection());
	thicknessCorrectionUmLineEdit->setText(sliceThicknessCorrectionText);
	currentThicknessCorrectionUm->setText(sliceThicknessCorrectionText);
	settings.setValue("thicknessCorrectionUm",getSliceThicknessCorrection());
}

float 
  fetch::ui::VibratomeController::
  getSliceThicknessCorrection()
{
	return vibratome_->getThicknessCorrection_um();
}