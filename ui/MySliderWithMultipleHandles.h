#include <QMouseEvent>
#include <QSlider>
#include <QPainter>
#include <QStyleOptionSlider>
class MySliderWithMultipleHandles: public QSlider{
	Q_OBJECT
protected:
	void mouseReleaseEvent (QMouseEvent * ev);
	public:
	MySliderWithMultipleHandles(QWidget *parent);
	void paintEvent(QPaintEvent *ev);
		void mouseMoveEvent(QMouseEvent *ev);
	int maxValue = 100;
	int minValue = 0;
	float sliderWidthInSliderCoordinates;
	int currentlySelected;
	bool justPushed = true;
	int minDistanceBetweenSliders;
	public slots:
//	void moveMinimumAndOrMaximum(int newValue);
//	void sliderWidgetActionTaken(int actionEnum);
};