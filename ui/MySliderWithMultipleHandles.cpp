#include "MySliderWithMultipleHandles.h"

//https://forum.qt.io/topic/73019/slider-with-two-handles-styles-support/2
MySliderWithMultipleHandles::MySliderWithMultipleHandles(QWidget * parent)
	: QSlider(Qt::Horizontal, parent),
	  minValue(0),
	  maxValue(65535),
	  currentlySelected(-1),
	  minDistanceBetweenSliders(1000)
{ installEventFilter(this);
}

void MySliderWithMultipleHandles::paintEvent(QPaintEvent *ev)
{
	Q_UNUSED(ev);
	QPainter p(this);
	QStyleOptionSlider opt;
	setMaximum(65535);
	setMinimum(0);
	setSingleStep(1);
	setPageStep(0);
	//Groove
	initStyleOption(&opt);
	opt.subControls = QStyle::SC_SliderGroove;
	opt.rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
	style()->drawComplexControl(QStyle::CC_Slider, &opt, &p, this);

	//First handle.
	initStyleOption(&opt);
					opt.sliderPosition = minValue;
	opt.subControls = QStyle::SC_SliderHandle;
	opt.rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
	sliderWidthInSliderCoordinates = ((maximum() - minimum()) * 1.0*opt.rect.width()) / width();
	style()->drawComplexControl(QStyle::CC_Slider, &opt, &p, this);
		//Second handle
	initStyleOption(&opt);
		opt.sliderPosition = maxValue;
	opt.subControls = QStyle::SC_SliderHandle;
	opt.rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
	style()->drawComplexControl(QStyle::CC_Slider, &opt, &p, this);
}



void MySliderWithMultipleHandles::mouseMoveEvent(QMouseEvent *ev)
{ Q_UNUSED(ev);
QPoint p = mapFromGlobal(QCursor::pos());
	int mousePositionInSliderCoordinates = minimum() + ((maximum() - minimum()) * p.x()) / width();
	if ( minValue - sliderWidthInSliderCoordinates*minValue/maximum() < mousePositionInSliderCoordinates
		&& minValue + sliderWidthInSliderCoordinates*(1-minValue/maximum()) > mousePositionInSliderCoordinates
		&& currentlySelected == -1
		&& justPushed)
	{
		setSliderPosition(minValue);
		currentlySelected = 0;
	}
	else if ( maxValue - sliderWidthInSliderCoordinates*maxValue/maximum() < mousePositionInSliderCoordinates
		&& maxValue + sliderWidthInSliderCoordinates*(1-maxValue/maximum()) > mousePositionInSliderCoordinates
		&& currentlySelected == -1
		&& justPushed)
	{
		setSliderPosition(maxValue);
		currentlySelected = 1;
	}

	switch (currentlySelected)
	{
	case 0:
		if (mousePositionInSliderCoordinates > (maxValue - minDistanceBetweenSliders)){
			(mousePositionInSliderCoordinates + minDistanceBetweenSliders)<maximum() ? maxValue = mousePositionInSliderCoordinates+minDistanceBetweenSliders : maxValue = maximum();
			minValue = maxValue - minDistanceBetweenSliders;
		}
		else minValue = mousePositionInSliderCoordinates;
		if (minValue < minimum()) minValue = minimum();
		setSliderPosition(minValue);
		break;
	case 1:
		if (mousePositionInSliderCoordinates < (minValue + minDistanceBetweenSliders)){
			(mousePositionInSliderCoordinates - minDistanceBetweenSliders)>minimum() ? minValue = mousePositionInSliderCoordinates-minDistanceBetweenSliders : minValue = minimum();
			maxValue = minValue + minDistanceBetweenSliders;
		}
		else maxValue = mousePositionInSliderCoordinates;
		if (maxValue > maximum()) maxValue = maximum();
		setSliderPosition(maxValue);
		break;
	}
	justPushed = false;
	printf("%d %d \n", minValue, maxValue);
}

void MySliderWithMultipleHandles::mouseReleaseEvent(QMouseEvent *ev)
{ Q_UNUSED(ev);
	currentlySelected = -1;
	justPushed = true;
}