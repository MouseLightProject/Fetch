#pragma once
#include <QMouseEvent>
#include <QSlider>
#include <QPainter>
#include <QStyleOptionSlider>
#include "channelHistogramInformationStruct.h"
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
	float newSliderValue(float distanceFromLeftEdge);
	float slidersLeftEdge(int position);
	double maxValue = 100;
	double minValue = 0;
	float sliderWidthInSliderCoordinates;
	int currentlySelected, mostRecentlySelected;
	bool justPushed = true;
	bool justGotFocus = true;
	double mousePositionInSliderCoordinates, mousePositionInSliderCoordinatesForSliderSelection;
	double minDistanceBetweenSliders;
	channelHistogramInformationStruct *channelHistogramInformation;
	size_t *currentIndex, currentIndexPrevious=0;
	float distanceToCurrentlySelectedSlidersLeftEdge;
	double maximumValueForImageDataType=65535, convertToSliderCoordinates;

	signals:
	void minimumMaximumCutoffValuesChanged(void);
};
}
}