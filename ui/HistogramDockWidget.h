#pragma once
#include <QtWidgets>
namespace mylib{
#include "array.h"
}
#include "channelHistogramInformationStruct.h"
#include "MySliderWithMultipleHandles.h"

class QCustomPlot;

namespace fetch {
namespace ui {

class HistogramDockWidget: public QDockWidget
{ Q_OBJECT
	QCustomPlot      *plot_;
	MySliderWithMultipleHandles *intensitySlider_;
	size_t            ichan_;
	mylib::Array     *last_, *currentImagePointerAccordingToUI_;
	bool              is_live_;
	QVector<double>   x_,pdf_,cdf_,minimumCutoffVector_, maximumCutoffVector_, yForPlottingCutoffsVector_;
	double            minX_,maxX_,perct_,minimumCutoffPrevious_,maximumCutoffPrevious_;
	QLineEdit         *leMin_, *leMax_, *lePerct_, *undersaturatedPercentile_, *oversaturatedPercentile_;
	QCheckBox		  *autoscaleCheckBox_, *displayChannelCheckBox_;
	QGroupBox		  *autoscaleGroupCheckBox_;
	QLabel			  *minimumCutoffLabel_, *maximumCutoffLabel_;

	public:
		HistogramDockWidget(QWidget *parent=NULL);
		channelHistogramInformationStruct channelHistogramInformation[4];
		size_t * channelIndex = &ichan_;

	signals:
		void change_chan(int ichan);
		void redisplayImage(mylib::Array*, mylib::Array*, bool fromSlider);

	public slots:
		void set(mylib::Array *im);
		void set_ichan(int ichan);
		void set_live(bool is_live);
		void set_autoscale(bool is_autoscale);
		void set_displayChannel(int is_displayChannel);
		void rescale_axes();
		void updateMinimumMaximumCutoffValues();
	void reset_minmax();
	void set_percentile(QString);
	private:
		void swap(mylib::Array *im);
		int  check_chan(mylib::Array *im);
		void compute(mylib::Array *im);

	protected:
      void showEvent(QShowEvent *ev);

};

}} // namespace fetch::ui
