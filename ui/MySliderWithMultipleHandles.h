#pragma once
#include <QMouseEvent>
#include <QSlider>
#include <QPainter>
#include <QStyleOptionSlider>
#include "channelHistogramInformationStruct.h"
class MySliderWithMultipleHandles : public QSlider
{
	Q_OBJECT
protected:
	void mouseReleaseEvent(QMouseEvent * ev);
	void focusInEvent(QFocusEvent * ev);
public:
	MySliderWithMultipleHandles(channelHistogramInformationStruct *channelHistogramInformationInput, size_t *currentIndexInput, QWidget *parent);
	void paintEvent(QPaintEvent *ev);
	void mouseMoveEvent(QMouseEvent *ev);
	int maxValue = 100;
	int minValue = 0;
	float sliderWidthInSliderCoordinates;
	int currentlySelected;
	bool justPushed = true;
	bool justGotFocus = true;
	int mousePositionInSliderCoordinates;
	int minDistanceBetweenSliders;
	channelHistogramInformationStruct *channelHistogramInformation;
	size_t *currentIndex;
};