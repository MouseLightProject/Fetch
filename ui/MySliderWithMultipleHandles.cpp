#include "MySliderWithMultipleHandles.h"
namespace fetch{ namespace ui{
//https://forum.qt.io/topic/73019/slider-with-two-handles-styles-support/2
MySliderWithMultipleHandles::MySliderWithMultipleHandles(channelHistogramInformationStruct * channelHistogramInformationInput, size_t *currentIndexInput, QWidget * parent)
	: QSlider(Qt::Horizontal, parent),
	  currentlySelected(-1),
	  channelHistogramInformation(channelHistogramInformationInput),
	  currentIndex(currentIndexInput),
	  dataType(mylib::UINT16_TYPE)
{// installEventFilter(this);
}

void MySliderWithMultipleHandles::paintEvent(QPaintEvent *ev)
{ 
	Q_UNUSED(ev);
	QPainter p(this);
	QStyleOptionSlider opt;
	setSingleStep(1);
	setPageStep(0);
	//Groove
	initStyleOption(&opt);
	opt.subControls = QStyle::SC_SliderGroove;
	opt.rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
	style()->drawComplexControl(QStyle::CC_Slider, &opt, &p, this);

	convertToSliderCoordinates=maximum()/maximumValueForImageDataType;
	//First handle.
	initStyleOption(&opt);
	opt.sliderPosition = mostRecentlySelected==0 ? channelHistogramInformation[*currentIndex].maxValue*convertToSliderCoordinates : channelHistogramInformation[*currentIndex].minValue*convertToSliderCoordinates;
	opt.subControls = QStyle::SC_SliderHandle;
	opt.rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
	sliderWidthInSliderCoordinates = ((maximum() - minimum()) * 1.0*opt.rect.width()) / width();
	style()->drawComplexControl(QStyle::CC_Slider, &opt, &p, this);

    //Second handle
	initStyleOption(&opt);
	opt.sliderPosition = mostRecentlySelected==0 ? channelHistogramInformation[*currentIndex].minValue*convertToSliderCoordinates : channelHistogramInformation[*currentIndex].maxValue*convertToSliderCoordinates;
	opt.subControls = QStyle::SC_SliderHandle;
	opt.rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
	style()->drawComplexControl(QStyle::CC_Slider, &opt, &p, this);
}



void MySliderWithMultipleHandles::mouseMoveEvent(QMouseEvent *ev)
{	mousePositionInSliderCoordinates = minimum() + (double(maximum() - minimum()) * ev->x() )/ width();
	minValue = channelHistogramInformation[*currentIndex].minValue*convertToSliderCoordinates;
	maxValue = channelHistogramInformation[*currentIndex].maxValue*convertToSliderCoordinates;
	minDistanceBetweenSliders = 0;//sliderWidthInSliderCoordinates/2.0;
	switch (currentlySelected)
	{ 
	case 0:
		minValue = newSliderValue(distanceToCurrentlySelectedSlidersLeftEdge);
		if (minValue > maxValue - minDistanceBetweenSliders)
		{
		 minValue + minDistanceBetweenSliders < maximum() ? maxValue = minValue + minDistanceBetweenSliders : maxValue = maximum();
		  minValue = maxValue - minDistanceBetweenSliders;
		}
		if (minValue < minimum()) minValue = minimum();
		setSliderPosition(minValue);
		channelHistogramInformation[*currentIndex].minValue = minValue/convertToSliderCoordinates;
		channelHistogramInformation[*currentIndex].maxValue = maxValue/convertToSliderCoordinates;
		break;
	case 1:
		maxValue = newSliderValue(distanceToCurrentlySelectedSlidersLeftEdge);
		if (maxValue < minValue + minDistanceBetweenSliders){
			maxValue - minDistanceBetweenSliders > minimum()  ? minValue = maxValue - minDistanceBetweenSliders : minValue = minimum();
			maxValue = minValue + minDistanceBetweenSliders;
		}
		if (maxValue > maximum()) maxValue = maximum();
		setSliderPosition(maxValue);
		channelHistogramInformation[*currentIndex].minValue = minValue/convertToSliderCoordinates;
		channelHistogramInformation[*currentIndex].maxValue = maxValue/convertToSliderCoordinates;
		break;
	}
	if (dataType != mylib::FLOAT32_TYPE && dataType != mylib::FLOAT64_TYPE){
		channelHistogramInformation[*currentIndex].minValue = floor(channelHistogramInformation[*currentIndex].minValue);
		channelHistogramInformation[*currentIndex].maxValue = floor(channelHistogramInformation[*currentIndex].maxValue);
	}
	emit minimumMaximumCutoffValuesChanged();
	justPushed = false;
}

void MySliderWithMultipleHandles::mouseReleaseEvent(QMouseEvent *ev)
{ Q_UNUSED(ev);
	currentlySelected = -1;
	justPushed = true;
}

void MySliderWithMultipleHandles::mousePressEvent(QMouseEvent *ev)
{ convertToSliderCoordinates=maximum()/maximumValueForImageDataType;
mousePositionInSliderCoordinatesForSliderSelection = minimum() + (double(maximum() - minimum()) * ev->x()) / width();
  minValue = channelHistogramInformation[*currentIndex].minValue*convertToSliderCoordinates;
  maxValue = channelHistogramInformation[*currentIndex].maxValue*convertToSliderCoordinates;
  double topSliderValue, bottomSliderValue;
  if (mostRecentlySelected == 0) {topSliderValue = minValue; bottomSliderValue = maxValue;}
  else { topSliderValue = maxValue; bottomSliderValue = minValue;}
  if (topSliderValue - sliderWidthInSliderCoordinates*topSliderValue / maximum() < mousePositionInSliderCoordinatesForSliderSelection
		&& topSliderValue + sliderWidthInSliderCoordinates*(1 - (float)topSliderValue / maximum()) > mousePositionInSliderCoordinatesForSliderSelection
		&& currentlySelected == -1
		&& justPushed)
  {
	setSliderPosition(topSliderValue);
	currentlySelected = (minValue == maxValue ? mostRecentlySelected : (topSliderValue == maxValue) );
	mostRecentlySelected = currentlySelected;
	distanceToCurrentlySelectedSlidersLeftEdge = mousePositionInSliderCoordinatesForSliderSelection-(topSliderValue-sliderWidthInSliderCoordinates*topSliderValue / maximum());
  }
  else if (bottomSliderValue - sliderWidthInSliderCoordinates*bottomSliderValue / maximum() < mousePositionInSliderCoordinatesForSliderSelection
		&& bottomSliderValue + sliderWidthInSliderCoordinates*(1 - (float)bottomSliderValue / maximum()) > mousePositionInSliderCoordinatesForSliderSelection
		&& currentlySelected == -1
		&& justPushed)
  {
	setSliderPosition(bottomSliderValue);
	currentlySelected = (minValue == maxValue ? mostRecentlySelected : (bottomSliderValue == maxValue) );
	mostRecentlySelected = currentlySelected;
	distanceToCurrentlySelectedSlidersLeftEdge = mousePositionInSliderCoordinatesForSliderSelection - (bottomSliderValue - sliderWidthInSliderCoordinates*bottomSliderValue / maximum());
  }
}
double MySliderWithMultipleHandles::newSliderValue(double distanceFromLeftEdge){
	return (mousePositionInSliderCoordinates-distanceFromLeftEdge)/(1.0-sliderWidthInSliderCoordinates/maximum());
}

double MySliderWithMultipleHandles::slidersLeftEdge(int sliderPosition){
	return (sliderPosition - sliderWidthInSliderCoordinates*sliderPosition/maximum());
}

}
}