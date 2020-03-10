#include "MySliderWithMultipleHandles.h"
namespace fetch{ namespace ui{
//DGA: Code and idea taken from https://forum.qt.io/topic/73019/slider-with-two-handles-styles-support/2
MySliderWithMultipleHandles::MySliderWithMultipleHandles(channelHistogramInformationStruct * channelHistogramInformationInput, size_t *currentIndexInput, QWidget * parent)
	: QSlider(Qt::Horizontal, parent), //DGA: Constructor to make it a slider and initialize some properties
	  channelHistogramInformation(channelHistogramInformationInput),
	  currentIndex(currentIndexInput),
	  mostRecentlySelectedSlider(0),
	  dataType(mylib::UINT16_TYPE)
{
}

void MySliderWithMultipleHandles::paintEvent(QPaintEvent *ev)
{ Q_UNUSED(ev); //DGA: Don't use the event
  QPainter painter(this); //DGA: Creates painter that begins to paint device
  QStyleOptionSlider opt; //DGA: Options for drawing a slider
  setSingleStep(1);
  setPageStep(0);
  //DGA: Draw the groove first
  initStyleOption(&opt);
  opt.subControls = QStyle::SC_SliderGroove;
  opt.rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
  style()->drawComplexControl(QStyle::CC_Slider, &opt, &painter, this);

  if(!calculatedConversion) convertToSliderCoordinates = maximum() / maximumValueForImageDataType; //DGA: Used to convert slider values to slider coordinates

  //First handle.
  initStyleOption(&opt);
  //DGA: The first one should be the one on top which is the one that was most recently selected
  opt.sliderPosition = mostRecentlySelectedSlider == 0 ? channelHistogramInformation[*currentIndex].maxValue*convertToSliderCoordinates : channelHistogramInformation[*currentIndex].minValue*convertToSliderCoordinates;
  opt.subControls = QStyle::SC_SliderHandle;
  opt.rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
  if (sliderWidthInSliderCoordinates = -1) sliderWidthInSliderCoordinates = (double(maximum() - minimum())*opt.rect.width()) / width(); //DGA: Define the slider width in slider coordinates for later use
  style()->drawComplexControl(QStyle::CC_Slider, &opt, &painter, this);

  //Second handle
  initStyleOption(&opt);
  opt.sliderPosition = mostRecentlySelectedSlider == 0 ? channelHistogramInformation[*currentIndex].minValue*convertToSliderCoordinates : channelHistogramInformation[*currentIndex].maxValue*convertToSliderCoordinates;
  opt.subControls = QStyle::SC_SliderHandle;
  opt.rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
  style()->drawComplexControl(QStyle::CC_Slider, &opt, &painter, this);
}

void MySliderWithMultipleHandles::mousePressEvent(QMouseEvent *ev) //DGA: Used when the widget is first clicked
{ if(!calculatedConversion) convertToSliderCoordinates = maximum() / maximumValueForImageDataType; //DGA: Used to convert slider values to slider coordinates
  mousePositionInSliderCoordinates = minimum() + (double(maximum() - minimum()) * ev->x()) / width();
  //DGA: Determine which was the topSliderValue
  minValue = channelHistogramInformation[*currentIndex].minValue*convertToSliderCoordinates;
  maxValue = channelHistogramInformation[*currentIndex].maxValue*convertToSliderCoordinates;
  double topSliderValue, bottomSliderValue;
  if (mostRecentlySelectedSlider == 0) {topSliderValue = minValue; bottomSliderValue = maxValue;}
  else { topSliderValue = maxValue; bottomSliderValue = minValue;}
  //DGA: Determine the most recently selected slider and set mosRecentlySelected to the appropriate slider
  //The subtractions/addtion factors of SliderValue/max is so we know where the edges are since the slider value does not always correspond to the center.
  if (topSliderValue - sliderWidthInSliderCoordinates*topSliderValue / maximum() < mousePositionInSliderCoordinates
	  && topSliderValue + sliderWidthInSliderCoordinates*(1 - (float)topSliderValue / maximum()) > mousePositionInSliderCoordinates)
  {	mostRecentlySelectedSlider = (minValue == maxValue ? mostRecentlySelectedSlider : (topSliderValue == maxValue) );
	distanceToMostRecentlySelectedSlidersLeftEdge = mousePositionInSliderCoordinates-(topSliderValue-sliderWidthInSliderCoordinates*topSliderValue / maximum());
  }
  else if (bottomSliderValue - sliderWidthInSliderCoordinates*bottomSliderValue / maximum() < mousePositionInSliderCoordinates
		   && bottomSliderValue + sliderWidthInSliderCoordinates*(1 - (float)bottomSliderValue / maximum()) > mousePositionInSliderCoordinates)
  {	mostRecentlySelectedSlider = (minValue == maxValue ? mostRecentlySelectedSlider : (bottomSliderValue == maxValue) );
	distanceToMostRecentlySelectedSlidersLeftEdge = mousePositionInSliderCoordinates - (bottomSliderValue - sliderWidthInSliderCoordinates*bottomSliderValue / maximum());
  }
  else{ mostRecentlySelectedSlider=-1;} //DGA: Then it wasn't close enough either slider
  this->update(); //DGA: Update widget so appropriate one is on top
}

void MySliderWithMultipleHandles::mouseMoveEvent(QMouseEvent *ev) //DGA: Function declaring what to do when the mouse is moved
{ mousePositionInSliderCoordinates = minimum() + (double(maximum() - minimum()) * ev->x() )/ width(); //DGA: The mouse position in slider coordinates
  //DGA: The min and max cutoff values
  minValue = channelHistogramInformation[*currentIndex].minValue*convertToSliderCoordinates;
  maxValue = channelHistogramInformation[*currentIndex].maxValue*convertToSliderCoordinates;
  switch (mostRecentlySelectedSlider) //DGA: Switch based on most recently selected, wherein it is ensured that the sliders do not overlap too much or go beyond the borders of the slider.
  {
  case 0:
	  minValue = newSliderValue(distanceToMostRecentlySelectedSlidersLeftEdge);
	  if (minValue > maxValue - minDistanceBetweenSliders)
	  { minValue + minDistanceBetweenSliders < maximum() ? maxValue = minValue + minDistanceBetweenSliders : maxValue = maximum();
		minValue = maxValue - minDistanceBetweenSliders;
	  }
	  if (minValue < minimum()) minValue = minimum();
	  updateAndEmitSignal();
	  break;
  case 1:
	  maxValue = newSliderValue(distanceToMostRecentlySelectedSlidersLeftEdge);
	  if(maxValue < minValue + minDistanceBetweenSliders)
	  { maxValue - minDistanceBetweenSliders > minimum() ? minValue = maxValue - minDistanceBetweenSliders : minValue = minimum();
		maxValue = minValue + minDistanceBetweenSliders;
	  }
	  if (maxValue > maximum()) maxValue = maximum();
	  updateAndEmitSignal();
	  break;
  }
}


void MySliderWithMultipleHandles::updateAndEmitSignal() //DGA: Update widget and update the channel cutoff and emit a signal that the values changed
{ this->update();
  if (dataType != mylib::FLOAT32_TYPE && dataType != mylib::FLOAT64_TYPE) //DGA: If the datatype is not a float, then floor the value
  {
	  channelHistogramInformation[*currentIndex].minValue = floor(minValue / convertToSliderCoordinates);
	  channelHistogramInformation[*currentIndex].maxValue = floor(maxValue / convertToSliderCoordinates);
  }
  else
  {
	  channelHistogramInformation[*currentIndex].minValue = minValue / convertToSliderCoordinates;
	  channelHistogramInformation[*currentIndex].maxValue = maxValue / convertToSliderCoordinates;
  }
  emit minimumMaximumCutoffValuesChanged();
}

}
}