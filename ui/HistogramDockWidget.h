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
	MySliderWithMultipleHandles *intensitySlider_; //DGA: Slider with two handles
	size_t            ichan_, numberOfChannels_, dataType_; //DGA: Added number of channels and dataType
	mylib::Array     *last_, *currentImagePointerAccordingToUI_; //DGA: currentImagePointerAccordingToUI_ stores the most recent pointer used by the widget
	bool              is_live_, didScalingChange_; //DGA: Added didScalingChange_, used to determine whether or not to update the image
	QVector<double>   x_,pdf_,cdf_,minimumCutoffVector_, maximumCutoffVector_, yForPlottingCutoffsVector_; //DGA: Added minimum/maximumCutoffs and yForPlotting which are used for plotting lines in histogram showing min/max cutoffs
	double            minX_,maxX_,perct_,minimumCutoffPrevious_,maximumCutoffPrevious_,maximumValueForImageDataType_=0; //DGA: Added previous cutoffs, used to check whether anything changed which determines if something should be updated; maximumValueForImageDataType_ is used 
	QCheckBox		  *displayChannelCheckBoxes_[4]; //DGA: Four checkboxes for the four possible channels
	QLineEdit         *leMin_, *leMax_, *lePerct_, *undersaturatedPercentile_, *oversaturatedPercentile_; //DGA: Added the under/oversaturatedPercentile_ line edit pointers
	QGroupBox		  *autoscaleGroupCheckBox_; //DGA: Autoscale group check box which houses the undersaturated and oversaturated percentiles
	QLabel			  *minimumCutoffLabel_, *maximumCutoffLabel_; //DGA: Cutoff labels that are displayed below slider
	QSignalMapper	  *displayChannelSignalMapper_, *percentileSignalMapper_; //DGA: Signal mapper for entering displaying channels and for changing percentiles

	public:
		HistogramDockWidget(QWidget *parent=NULL);
		channelHistogramInformationStruct channelHistogramInformation[4]; //DGA: channelHistogramInformation contains the information for up to 4 channels
		size_t * channelIndex = &ichan_; //DGA: channelIndex is set to the address of ichan_

	signals:
		void change_chan(int ichan);
		void redisplayImage(mylib::Array*, mylib::Array*, bool fromSlider);

	public slots:
		void set(mylib::Array *im);
		void set_ichan(int ichan);
		void set_live(bool is_live);
		void set_autoscale(bool is_autoscale); //DGA: Slot to set a channel to be autoscaled
		void set_displayChannel(int currentlyCheckedChannel); //DGA: Slot to set a channel to display or not
		void rescale_axes();
		void updateMinimumMaximumCutoffValuesReplotAndRedisplay(); //DGA: Slot to update the minimum/maximum values that are displayed, replot the graph and redisplay the image
		void percentileValuesEntered(QString); //DGA: Slot to update percentiles when new values entered
		//void autoscaleToggledInFigure(int ichan); //DGA: To update the widget if autoscaling was changed in the figure
		//void displayChannelChangedInFigure(int ichan); //DGA: To update the widget if display channels changed in figure
	void reset_minmax();
	void set_percentile(QString);
	private:
		void swap(mylib::Array *im);
		int  check_chan(mylib::Array *im);
		void compute(mylib::Array *im);

	protected:
      void showEvent(QShowEvent *ev); //DGA: showEvent that will be called when the widget is shown

};

}} // namespace fetch::ui
