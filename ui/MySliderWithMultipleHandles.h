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
	int maxValue = 100;
	int minValue = 0;
	float sliderWidthInSliderCoordinates;
	int currentlySelected;
	bool justPushed = true;
	bool justGotFocus = true;
	int mousePositionInSliderCoordinates, mousePositionInSliderCoordinatesForSliderSelection;
	int minDistanceBetweenSliders;
	channelHistogramInformationStruct *channelHistogramInformation;
	size_t *currentIndex, currentIndexPrevious=0;

	signals:
	void minimumMaximumCutoffValuesChanged(void);
};
}
}