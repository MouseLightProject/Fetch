#pragma once
#include <QMouseEvent>
#include <QSlider>
#include <QPainter>
#include <QStyleOptionSlider>
#include "channelHistogramInformationStruct.h"
#include "HistogramUtilities.h"

namespace fetch{ namespace ui{
class MySliderWithMultipleHandles : public QSlider
{
	Q_OBJECT
protected:
	void mousePressEvent(QMouseEvent *ev);
	void mouseReleaseEvent(QMouseEvent *ev);
public:
	MySliderWithMultipleHandles(channelHistogramInformationStruct *channelHistogramInformationInput, size_t *currentIndexInput, QWidget *parent);
	void paintEvent(QPaintEvent *ev);
	void mouseMoveEvent(QMouseEvent *ev);
	double newSliderValue(double distanceFromLeftEdge);
	double slidersLeftEdge(int position);
	double maxValue = 100;
	double minValue = 0;
	double sliderWidthInSliderCoordinates;
	int currentlySelected, mostRecentlySelected;
	bool justPushed = true;
	bool justGotFocus = true;
	double mousePositionInSliderCoordinates, mousePositionInSliderCoordinatesForSliderSelection;
	double minDistanceBetweenSliders;
	channelHistogramInformationStruct *channelHistogramInformation;
	size_t *currentIndex, currentIndexPrevious=0;
	double distanceToCurrentlySelectedSlidersLeftEdge;
	double maximumValueForImageDataType=65535, convertToSliderCoordinates;
	size_t dataType;
	signals:
	void minimumMaximumCutoffValuesChanged(void);
};
}
}