#pragma once
#include <QMouseEvent>
#include <QSlider>
#include <QPainter>
#include <QStyleOptionSlider>
#include "channelHistogramInformationStruct.h"
#include "HistogramUtilities.h"

namespace fetch{ namespace ui{ //DGA: In namespace fetch::ui
class MySliderWithMultipleHandles : public QSlider
{
	Q_OBJECT //DGA: It is a Q_OBJECT and can have signals/slots
	void   updateAndEmitSignal(); //DGA: Update widget and cutoff values and emit the signal that the values have changed
	double newSliderValue(double distanceFromLeftEdge) { return (mousePositionInSliderCoordinates - distanceFromLeftEdge) / (1.0 - sliderWidthInSliderCoordinates / maximum()); }; //DGA: Function to return slider value based on cursor position
	double minValue, maxValue, //DGA: Variables to hold what will become the minimum and maximum slider values
	       sliderWidthInSliderCoordinates=-1, //DGA: Width of slider in slider coordinates
		   mousePositionInSliderCoordinates, //DGA: Mouse position in slider cooridnates
		   minDistanceBetweenSliders=0, //DGA: Minimum distance between sliders
		   distanceToMostRecentlySelectedSlidersLeftEdge, //DGA: Distance to sliders left edge
		   convertToSliderCoordinates; //DGA: The value need to convert to slider coordinates
	int    mostRecentlySelectedSlider; //DGA: Most recently selected slider
	bool   justPushed = true, //DGA: If slider was just pushed
		   calculatedConversion = false; //DGA: Calculated conversion to slider coordinates
	channelHistogramInformationStruct *channelHistogramInformation; //DGA: Pointer to structure used to store channel histogram information
	size_t *currentIndex; //DGA: Pointer to the currentIndex used for the channelHistogramInformation structure
protected:
	//DGA: Virtual functions for widgets that we want to define
	void mousePressEvent(QMouseEvent *ev); 
	void paintEvent(QPaintEvent *ev);
	void mouseMoveEvent(QMouseEvent *ev);
public:
	MySliderWithMultipleHandles(channelHistogramInformationStruct *channelHistogramInformationInput, size_t *currentIndexInput, QWidget *parent); //DGA: Constructor definition
	int dataType; //DGA: The data type for the images being collected
	double maximumValueForImageDataType=65535; //DGA: The maximum value for the image data type	
	signals:
	void minimumMaximumCutoffValuesChanged(void); //DGA: Signal emitted when minimum or maximum cutoff values were changed
};
}
}