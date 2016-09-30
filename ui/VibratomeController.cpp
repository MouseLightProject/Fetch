#include "VibratomeController.h"
#include <QtWidgets>

void
  fetch::ui::VibratomeController::
  createSliceThicknessCorrectionUmWidgets() //DGA: Definition of function from header
{	QSettings settings; //DGA: Create a QSettings variable
	sliceThicknessCorrectionUmLineEdit = new QLineEdit(); //DGA: Dynamically allocate slice thickness correction line edit
	currentSliceThicknessCorrectionUmLabel = new QLabel(); //DGA: Dynamically allocate current slice thickness correction label
	connect(sliceThicknessCorrectionUmLineEdit, SIGNAL(editingFinished()), this, SLOT(setSliceThicknessCorrectionUm()),Qt::QueuedConnection); //DGA: Connect the editingFinished signal from the line edit to the setSliceThicknessCorrection slot of this class instance. This will set the slice thickness variable of vibratome_ to that entered in the line edit
	connect(&(vibratome_->sliceThicknessCorrectionUmLineEditUpdater), SIGNAL(signal_valueSet(QString)),sliceThicknessCorrectionUmLineEdit, SLOT(setText(QString)),Qt::QueuedConnection); //DGA: Connect the signal signal_valueSet(Qstring) of the thickness correction line edit updater object of vibratome_ to the setText slot of the line edit so the displayed text matches the value of the slice thickness variable of vibratome_
	connect(&(vibratome_->sliceThicknessCorrectionUmLabelUpdater), SIGNAL(signal_valueSet(QString)),currentSliceThicknessCorrectionUmLabel, SLOT(setText(QString)),Qt::QueuedConnection); //DGA: Connect the signal signal_valueSet(QString) of the thickness correction label updater object of vibratome_ to the setText slot of the label so the displayed text matches the value of the slice thickness variable of vibratome_
}

void 
  fetch::ui::VibratomeController::
  setSliceThicknessCorrectionUm() //DGA: setSliceThicknessCorrectionUm slot
{	bool ok;
	float newSliceThicknessCorrection = sliceThicknessCorrectionUmLineEdit->text().toFloat(&ok); //Try to convert text to a float, if successful ok = true
	if (ok && abs(newSliceThicknessCorrection)<1000){ //DGA: Conversion was successful and is a valid number (between -1000 and 1000
		vibratome_->setSliceThicknessCorrectionUm(newSliceThicknessCorrection); //DGA: set slice thickness correction member of vibratome_
	}
	else{
		sliceThicknessCorrectionUmLineEdit->setText(QString::number(vibratome_->getSliceThicknessCorrectionUm())); //DGA: If conversion wasn't successful, then set the text of the line edit to the current sliceThicknessCorrection_um_
	    currentSliceThicknessCorrectionUmLabel->setText(QString::number(vibratome_->getSliceThicknessCorrectionUm())); //DGA: If conversion wasn't successful, then set the text of the line edit to the current sliceThicknessCorrection_um_
	}
}