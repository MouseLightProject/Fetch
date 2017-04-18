/** \file
    Histogram Dock Widget
    
    \todo never need to rescale cdf axis, always 0 to 1
    \todo options for scaling on x-axis...full range, zoom to range (and fix)
    /todo size x axis by cdf? e.g where cdf is between 0.05 to 0.95? 
    \todo inspection by hover over cdf
*/
#include "HistogramDockWidget.h"
#include "qcustomplot.h"
#include "common.h"
#include <cmath>  

namespace mylib {
#include "image.h"
}

#include "HistogramUtilities.h"

#define ENDL "\r\n"
#define PANIC(e) do{if(!(e)){qFatal("%s(%d)"ENDL "\tExpression evalatuated as false."ENDL "\t%s"ENDL,__FILE__,__LINE__,#e);           }}while(0)
#define TRY(e)   do{if(!(e)){qDebug("%s(%d)"ENDL "\tExpression evalatuated as false."ENDL "\t%s"ENDL,__FILE__,__LINE__,#e);goto Error;}}while(0)

#define HINT_NBINS (1<<12)

namespace fetch{
namespace ui {

  HistogramDockWidget::HistogramDockWidget(QWidget *parent)
    : QDockWidget("Image Display and Histogram",parent) //DGA: Changed widget name
	, dataType_(mylib::UINT16_TYPE) //DGA: Initialize it to uint16 type
	, didScalingChange_(false)
    , plot_(0)
    , ichan_(0)
    , last_(0)
	, currentImagePointerAccordingToUI_(0) //DGA: Initialize currentImagePointerAccordingToUI_ to 0
    , x_(HINT_NBINS)
    , pdf_(HINT_NBINS)
    , cdf_(HINT_NBINS)
	, yForPlottingCutoffsVector_({0,1}) //DGA: Initialize yForPlottingCutoffsVector_ to {0,1}
    , minX_(DBL_MAX)
    , maxX_(0)
    , perct_(0.5)
	, minimumCutoffPrevious_(-1) //DGA: Initialize minimum and maximum previous cutoffs to -1
	, maximumCutoffPrevious_(-1)
    , leMin_(0)
    , leMax_(0)
    , lePerct_(0)
  { 
    QBoxLayout *layout = new QVBoxLayout;
    {
      QWidget *w = new QWidget(this);    
      w->setLayout(layout);
      setWidget(w);
    }

	QFormLayout *form = new QFormLayout;
	//DGA: Channel display checkboxes
	{
	  QHBoxLayout* displayChannelCheckBoxRow = new QHBoxLayout();
	  QLabel * undersaturatedPercentileLabel = new QLabel("Select Channels To Display:");
	  displayChannelCheckBoxRow->addWidget(undersaturatedPercentileLabel);
	  displayChannelSignalMapper_ = new QSignalMapper(this);
	  for (int i = 0; i < 4; i++){
		  //DGA: Initialize each display channel checkbox, by default set to true
		  displayChannelCheckBoxes_[i] = new QCheckBox(); 
		  displayChannelCheckBoxes_[i]->setText(QStringLiteral("%1").arg(i));
		  displayChannelCheckBoxes_[i]->setChecked(true);
		  //DGA: Connect the checkboxes clicked signal to the signalmapper's map slot, set the mapping to equal the channel index and add the chanel (map emits mapped signal)
		  PANIC(connect(displayChannelCheckBoxes_[i], SIGNAL(clicked()),
			  displayChannelSignalMapper_, SLOT(map())));
		  displayChannelSignalMapper_->setMapping(displayChannelCheckBoxes_[i], i);
		  displayChannelCheckBoxRow->addWidget(displayChannelCheckBoxes_[i]);
	  }
	  //DGA: Connect the mapped signal of the signal mapper to the slot set_displayChannel, so that set_displayChannel gets passed the index of the clicked channel
	  PANIC(connect(displayChannelSignalMapper_, SIGNAL(mapped(int)),
		    this, SLOT(set_displayChannel(int))));
	  form->addRow(displayChannelCheckBoxRow); //DGA: Add the checkboxes
	}

    // channel selector
    { 
      QComboBox *c=new QComboBox;
      for(int i=0;i<4;++i) // initially add 4 channels to select
        c->addItem(QString("%1").arg(i),QVariant());
      PANIC(connect(   c,SIGNAL(currentIndexChanged(int)),
        this,SLOT(set_ichan(int))));
      PANIC(connect(this,SIGNAL(change_chan(int)),
        c,SLOT(setCurrentIndex(int))));
      form->addRow(tr("Channel:"),c);
    }

    { 
      QHBoxLayout *row = new QHBoxLayout();
      QPushButton *b;
      // Live update button
      b=new QPushButton("Live update");
      b->setCheckable(true);      
      PANIC(connect(  b,SIGNAL(toggled(bool)),
        this,SLOT(set_live(bool))));
      b->setChecked(true);
      row->addWidget(b);

      // Rescale button
      b=new QPushButton("Rescale");     
      PANIC(connect(   b,SIGNAL(pressed()),
        this,SLOT(rescale_axes())));
      b->setChecked(true);
      row->addWidget(b);
      form->addRow(row);
    }

    {
      QHBoxLayout *row = new QHBoxLayout();
      QPushButton *b;
      //percentile label
      QLabel *perctLabel = new QLabel(tr("Percentile:"));
      row->addWidget(perctLabel);
      ///// Line Edit Control - Gamma
      lePerct_= new QLineEdit("0.5");    
      QCompleter *c = new QCompleter(new QStringListModel(lePerct_),lePerct_);// Add completion based on history
      c->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
      c->setModelSorting(QCompleter::UnsortedModel);
      lePerct_->setCompleter(c);
      lePerct_->setValidator(new QDoubleValidator(0.0001,1.0000,4,lePerct_));      // Add validated input
      PANIC(connect(lePerct_,SIGNAL(textEdited(QString)),
        this,  SLOT(set_percentile(QString))));
      row->addWidget(lePerct_);

      // Reset button
      b=new QPushButton("Reset");     
      PANIC(connect(   b,SIGNAL(pressed()),
        this,SLOT(reset_minmax())));
      row->addWidget(b);
      form->addRow(row);
    }

    {
      QHBoxLayout *row = new QHBoxLayout();
 
      QLabel *lbMin = new QLabel(tr("Minimum:"));
      row->addWidget(lbMin);
      ///// Line Edit Control - Minimum
      leMin_= new QLineEdit("DBL_MAX");
      leMin_->setReadOnly(true);
      row->addWidget(leMin_);

 
      QLabel *lbMax = new QLabel(tr("Maximum:"));
      row->addWidget(lbMax);
      ///// Line Edit Control - Maximum
      leMax_= new QLineEdit("0");
      leMax_->setReadOnly(true);;
      row->addWidget(leMax_);
	  form->addRow(row);
    }

    layout->addLayout(form);

    // plot
    plot_=new QCustomPlot();
    plot_->setMinimumHeight(100);
    plot_->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    plot_->addGraph();
    plot_->graph(0)->setPen(QPen(Qt::blue));
    plot_->addGraph(plot_->xAxis2,plot_->yAxis2); //DGA: Made CDF on second x axis
    plot_->graph(1)->setPen(QPen(Qt::red)); 
	plot_->addGraph(plot_->xAxis2,plot_->yAxis2); //DGA: Minimum cutoff line
	plot_->graph(2)->setPen(QPen(Qt::black));
	plot_->addGraph(plot_->xAxis2,plot_->yAxis2); //DGA: Maximum cutoff line
	plot_->graph(3)->setPen(QPen(Qt::black));
    plot_->xAxis->setLabel("Intensity");
    plot_->xAxis2->setVisible(true);
    plot_->yAxis->setLabel("PDF");
    plot_->yAxis2->setLabelColor(Qt::blue);
    plot_->yAxis2->setVisible(true);
    plot_->yAxis2->setLabel("CDF");
    plot_->yAxis2->setLabelColor(Qt::red);
	plot_->yAxis2->setRange(0, 1); //DGA: CDF range set to 0-1

    layout->addWidget(plot_);

	// DGA: Contrast controls
	{ QFormLayout *contrastForm = new QFormLayout; //DGA: Form layout for contrast and display
	  intensitySlider_ = new MySliderWithMultipleHandles(channelHistogramInformation, &ichan_, parent); //DGA: Create intensity slider
	  
	  //DGA: Intensity slider
	  QGridLayout* cutoffRow = new QGridLayout();
	  //DGA: Add the minimum and maximum cutoff labels, by default assuming max is 65535
	  minimumCutoffLabel_ = new QLabel(QString("Minimum: %1").arg("0", 6));
	  maximumCutoffLabel_ = new QLabel(QString("Maximum: %1").arg(QString::number(USHRT_MAX), 6));
	  PANIC(connect(intensitySlider_, SIGNAL(minimumMaximumCutoffValuesChanged(void)), //DGA: connect the minimumMaximumCutoffValuesChanged signal of the intensity slider to the updateMinimumMaximumCutoffValuesReplotAndRedisplay slot
		    this, SLOT(updateMinimumMaximumCutoffValuesReplotAndRedisplay(void))));
	  //DGA: Add the cutoff labels on the left/right ends, and disable the slider by default and add the slider and cutoff labels to the form
	  cutoffRow->addWidget(minimumCutoffLabel_, 0, 0, Qt::AlignLeft);
	  cutoffRow->addWidget(maximumCutoffLabel_, 0, 1, Qt::AlignRight);
	  intensitySlider_->setEnabled(false);
	  intensitySlider_->setMinimum(0);
	  intensitySlider_->setMaximum(USHRT_MAX);
	  contrastForm->addRow(intensitySlider_);
	  contrastForm->addRow(cutoffRow);

	  //DGA: Create autoscale group, which is checked by default
	  autoscaleGroupCheckBox_ = new QGroupBox("Autoscale"); 
	  autoscaleGroupCheckBox_->setCheckable(true);
	  autoscaleGroupCheckBox_->setChecked(true);
	  PANIC(connect(autoscaleGroupCheckBox_, SIGNAL(clicked(bool)), //DGA: Connect the group checkbox clicked signal to the autoscale slot
		    this, SLOT(set_autoscale(bool))));

	  percentileSignalMapper_ = new QSignalMapper(this); //DGA: Create signal mapper
	  QHBoxLayout* percentileRow = new QHBoxLayout(); //DGA: Percentile row for changing cutoffs
	  //DGA: Labels and default values for under/oversaturated percentages (10/90 respectively)
	  QLabel * undersaturatedPercentileLabel = new QLabel("Undersaturated %:"); 
	  undersaturatedPercentile_ = new QLineEdit("10");
	  QLabel * oversaturatedPercentileLabel = new QLabel("Oversaturated %:");
	  oversaturatedPercentile_ = new QLineEdit("90");
	  //DGA: connect the editing finished signals of the under/oversaturated percentiles to the signal mapper's map slot, mapping the signals respectively
	  PANIC(connect(undersaturatedPercentile_, SIGNAL(editingFinished()),
		    percentileSignalMapper_, SLOT(map())));
	  percentileSignalMapper_->setMapping(undersaturatedPercentile_, "undersaturatedPercentile");
	  PANIC(connect(oversaturatedPercentile_, SIGNAL(editingFinished()),
		    percentileSignalMapper_, SLOT(map())));
	  percentileSignalMapper_->setMapping(oversaturatedPercentile_, "oversaturatedPercentile");
	  PANIC(connect(percentileSignalMapper_, SIGNAL(mapped(QString)), this, SLOT(percentileValuesEntered(QString)))); //DGA: Connect signal mapper's mapped signal to the percentileValuesEntered slot so that when either percentile is changed, this signal is called with the appropriate string identifier

	  //DGA: Add the percentile labels and edit boxes to the autoscale group
	  percentileRow->addWidget(undersaturatedPercentileLabel);
	  percentileRow->addWidget(undersaturatedPercentile_);
	  percentileRow->addWidget(oversaturatedPercentileLabel);
	  percentileRow->addWidget(oversaturatedPercentile_);
	  autoscaleGroupCheckBox_->setLayout(percentileRow);
	  contrastForm->addWidget(autoscaleGroupCheckBox_);

	  layout->addLayout(contrastForm);
	}
  }
  
void HistogramDockWidget::showEvent( QShowEvent* event ) { //DGA: Rescale axes when the widget is shown
    QWidget::showEvent( event );
	rescale_axes();
} 

 void HistogramDockWidget::set(mylib::Array *im)
 { //DGA: Gets called when player signals an image is ready
   TRY(check_chan(im));
   swap(im);
   currentImagePointerAccordingToUI_ = im; //DGA: Sets the currentImagePointerAccordingToUI_ equal to im
   if(!maximumValueForImageDataType_) //DGA: If the maximum value for the data type has not yet been determined, then determine it
   { determineMaximumValueForDataType(im->type, maximumValueForImageDataType_);
	 if(maximumValueForImageDataType_ != USHRT_MAX) //DGA: If the max is not the default 65535
	 { //DGA: If the min or max values exceed the new maximumValueForImageDataType_, then set the minValue to 0 and the maxValue to maximumValueForImageDataType_, and set didScalingChange_ to true
	   for(size_t tempChannelIndex = 0; tempChannelIndex < 3; tempChannelIndex++)
	   { if (channelHistogramInformation[tempChannelIndex].minValue > maximumValueForImageDataType_) { channelHistogramInformation[tempChannelIndex].minValue = 0; didScalingChange_ = true; }
	     if (channelHistogramInformation[tempChannelIndex].maxValue > maximumValueForImageDataType_) { channelHistogramInformation[tempChannelIndex].maxValue = maximumValueForImageDataType_; didScalingChange_ = true; }
	   }
	 }
	 intensitySlider_->maximumValueForImageDataType = maximumValueForImageDataType_; //DGA: Set intensity slider's maximumValueForImageDataType
   }
   if (!is_live_) return;
   compute(im);
Error:
   ; // bad input, ignore 
 }

static int g_inited=0;

  /** Presumes channels are on different planes */
 void HistogramDockWidget::compute(mylib::Array *im)
  { 
    mylib::Array t,*ch;    
    QString tempStr;
    TRY(ch=mylib::Get_Array_Plane(&(t=*im),ichan_)); //select channel    
    histogram(x_,pdf_,cdf_,ch);
	plot_->graph(0)->setData(x_,pdf_);
    plot_->graph(1)->setData(x_,cdf_);
	//DGA: Only need plot visible if it is in range (if we don't do this, then we get an error)
	if(x_.first() <= plot_->xAxis->range().upper && x_.last() >= plot_->xAxis->range().lower)
	{ (pdf_.first() <= plot_->yAxis->range().upper && pdf_.last() >= plot_->yAxis->range().lower) ? plot_->graph(0)->setVisible(true) : plot_->graph(0)->setVisible(false);
	  plot_->graph(1)->setVisible(true); //DGA: CDF is always in range 0-1 on y-axis
	}
	else
	{ plot_->graph(0)->setVisible(false);
	  plot_->graph(1)->setVisible(false);
	}
    if(!g_inited) //DGA: Only done first time
    { rescale_axes();
	  //DGA: Set the number of channels and disable all the invalid channel checkboxes
	  numberOfChannels_ = (im->ndims == 3 ?  im->dims[2] : 1); 
	  for(int i = numberOfChannels_; i < 4; i++)
	  { displayChannelCheckBoxes_[i]->setEnabled(false);
	    displayChannelCheckBoxes_[i]->setChecked(false);
      }
	  //DGA: Determine the image data type and set the dataType variables for the widget and slider
	  dataType_ = im->type;
	  intensitySlider_->dataType = dataType_;
      g_inited=1;
    }
	updateMinimumMaximumCutoffValuesReplotAndRedisplay(); //DGA: Update widget even if haven't started collecting data


	//find intensity value for the input CDF percentile 
	int index;
	index = findIndex(cdf_, perct_);
	double xValue;
	xValue = x_.data()[index];
	if (xValue < minX_)  minX_ = xValue;
	if (xValue > maxX_)  maxX_ = xValue;

	tempStr.setNum(minX_);
	leMin_->setText(tempStr);
	tempStr.setNum(maxX_);
	leMax_->setText(tempStr);

Error:
    ; // memory error or oob channel, should never get here.    
  }

 void HistogramDockWidget::rescale_axes()
 { //DGA: Rescale the axes, meaning they should both be visible, and then replot
   plot_->graph(0)->rescaleAxes();
   plot_->xAxis2->setRange(plot_->xAxis->range().lower, plot_->xAxis->range().upper);
   plot_->graph(0)->setVisible(true);
   plot_->graph(1)->setVisible(true);
   updateMinimumMaximumCutoffValuesReplotAndRedisplay();
  }

void HistogramDockWidget::set_ichan(int ichan)
  { ichan_=ichan;   
    if(last_)
    { check_chan(last_);
      compute(last_);
    }
	else{ updateMinimumMaximumCutoffValuesReplotAndRedisplay();} //DGA: Update widget even if haven't started collecting data}
	//DGA: Set the autoscale checkbox and the percentile values to their respective values from the current channels
	autoscaleGroupCheckBox_->setChecked(channelHistogramInformation[ichan_].autoscale);
	undersaturatedPercentile_->setText(QString("%1").arg(channelHistogramInformation[ichan_].undersaturatedPercentile*100.0));
	oversaturatedPercentile_->setText(QString("%1").arg(channelHistogramInformation[ichan_].oversaturatedPercentile*100.0));
  }

void HistogramDockWidget::set_live(bool is_live)
  { is_live_=is_live;
  }

void HistogramDockWidget::set_autoscale(bool is_autoscale)
  { //DGA: Called when autoscale checkbox has been clicked
    channelHistogramInformation[ichan_].autoscale = is_autoscale; //DGA: Set the channel's autoscale value equal to the checkbox state
	intensitySlider_->setEnabled(!is_autoscale); //DGA: Disable slider if autoscale enabled
	if(last_ && is_autoscale) emit redisplayImage(last_,currentImagePointerAccordingToUI_, true); //DGA: If there is a valid frame and autoscale has been checked, signal to redisplay the image, true indicating it is from UI
	updateMinimumMaximumCutoffValuesReplotAndRedisplay(); //DGA: Update the minimum and maximum cutoff values if necessary
  }

void HistogramDockWidget::percentileValuesEntered(QString whichPercentile) //DGA: Called when a percentile value is entered in the text edit box
  { bool conversionWorked;
	if(QString::compare(whichPercentile,"undersaturatedPercentile")==0) //DGA: Then the newly entered value was for the undersaturated percentile
    { double possibleNewUndersaturatedPercentile = undersaturatedPercentile_->text().toDouble(&conversionWorked); 
	  if(possibleNewUndersaturatedPercentile >= 0 && possibleNewUndersaturatedPercentile <= oversaturatedPercentile_->text().toDouble() //DGA: Set the new percentile value if valid, otherwise reset it to current valid value
		  && possibleNewUndersaturatedPercentile/100.0 != channelHistogramInformation[ichan_].undersaturatedPercentile && conversionWorked) 
	  {	channelHistogramInformation[ichan_].undersaturatedPercentile = possibleNewUndersaturatedPercentile/100.0;
		didScalingChange_=true;
	  }
	  else undersaturatedPercentile_->setText(QString("%1").arg(channelHistogramInformation[ichan_].undersaturatedPercentile*100.0));
	}
    else //DGA: Then the newly entered value was for the oversaturated percentile
	{ double possibleNewOversaturatedPercentile = oversaturatedPercentile_->text().toDouble(&conversionWorked);
	  if(possibleNewOversaturatedPercentile <=100 && possibleNewOversaturatedPercentile >= undersaturatedPercentile_->text().toDouble() //DGA: Set the new percentile value if valid, otherwise reset it to current valid value
		 && possibleNewOversaturatedPercentile/100.0 != channelHistogramInformation[ichan_].oversaturatedPercentile && conversionWorked) 
	  {	channelHistogramInformation[ichan_].oversaturatedPercentile = possibleNewOversaturatedPercentile/100.0;
		didScalingChange_ = true;
	  }
	  else oversaturatedPercentile_->setText(QString("%1").arg(channelHistogramInformation[ichan_].oversaturatedPercentile*100.0));
    }
    if(didScalingChange_ && last_) emit redisplayImage(last_, currentImagePointerAccordingToUI_, true); //DGA: If scaling changed and the last frame was valid, signal to redisplay the image with true indicating it is from UI
    didScalingChange_ = false; //DGA: Reset scaling to false and update the cutoff values
    updateMinimumMaximumCutoffValuesReplotAndRedisplay();
  }

/*void HistogramDockWidget::autoscaleToggledInFigure(int ichan)
  { channelHistogramInformation[ichan].autoscale = !channelHistogramInformation[ichan].autoscale;
	if (ichan==ichan_) { intensitySlider_->setEnabled(!channelHistogramInformation[ichan].autoscale); autoscaleGroupCheckBox_->setChecked(channelHistogramInformation[ichan].autoscale);}
  }*/

void HistogramDockWidget::set_displayChannel(int currentlyCheckedChannel) //DGA: Called when a display channel checkbox is toggled
  { channelHistogramInformation[currentlyCheckedChannel].displayChannel = displayChannelCheckBoxes_[currentlyCheckedChannel]->isChecked(); //DGA: Update the displayChannel property and redisplay the image
	if (last_) emit redisplayImage(last_,currentImagePointerAccordingToUI_, true);
  }

void HistogramDockWidget::updateMinimumMaximumCutoffValuesReplotAndRedisplay() //DGA: This function updates the minimum and maximum cutoff values for the slider widget, replots them and signals to redisplay the image if necessary
  {	//DGA: For minimum and maximum cutoffs, check if the current value is different from the previous. If so, update the current label and cutoff vector used for plotting and set didScalingChange_ equal to true. Then if the line would be in range, make it visible
	if(channelHistogramInformation[ichan_].minValue != minimumCutoffPrevious_)
	{ minimumCutoffLabel_->setText(QString("Minimum: %1").arg(channelHistogramInformation[ichan_].minValue,6));
	  minimumCutoffVector_ = { channelHistogramInformation[ichan_].minValue, channelHistogramInformation[ichan_].minValue };
	  minimumCutoffPrevious_ = channelHistogramInformation[ichan_].minValue;
	  didScalingChange_ = true;
	}
	plot_->graph(2)->setVisible(false);
	if (plot_->xAxis2->range().lower <= minimumCutoffPrevious_ && plot_->xAxis2->range().upper >= minimumCutoffPrevious_){
		plot_->graph(2)->setData(minimumCutoffVector_, yForPlottingCutoffsVector_);
		plot_->graph(2)->setVisible(true);
	}
	if(channelHistogramInformation[ichan_].maxValue != maximumCutoffPrevious_)
	{ maximumCutoffLabel_->setText(QString("Maximum: %1").arg(channelHistogramInformation[ichan_].maxValue,6));
	  maximumCutoffVector_ = { channelHistogramInformation[ichan_].maxValue, channelHistogramInformation[ichan_].maxValue };
	  maximumCutoffPrevious_ = channelHistogramInformation[ichan_].maxValue;
	  didScalingChange_ = true;
	}
	plot_->graph(3)->setVisible(false);
	if(plot_->xAxis2->range().lower <= maximumCutoffPrevious_ && plot_->xAxis2->range().upper >= maximumCutoffPrevious_)
	{ plot_->graph(3)->setData(maximumCutoffVector_, yForPlottingCutoffsVector_);
	  plot_->graph(3)->setVisible(true);
	}
	plot_->replot(); //DGA: Replot the histogram
	if (didScalingChange_)
	{ if (!channelHistogramInformation[ichan_].autoscale && last_) emit redisplayImage(last_, currentImagePointerAccordingToUI_, true); //DGA: If scaling changed and if the channel is not currently being autoscaled and if the last_ frame was valid, emit redisplay signal where true indicates it is coming from UI
	  intensitySlider_->update(); //DGA: Update the intensitySlider_
	}
	didScalingChange_ = false; //DGA: Reset didScalingChange_ to false
  }
void HistogramDockWidget::reset_minmax()
  {
    minX_ = DBL_MAX;
    maxX_ = 0;
  }

void HistogramDockWidget::set_percentile(QString text)
  {
    perct_ = text.toDouble();
    reset_minmax();
  }

  /** updates the last_ array with a new one. */
void HistogramDockWidget::swap(mylib::Array *im)
  { 
    mylib::Array *t=last_;
    TRY(last_=mylib::Copy_Array(im));
    if(t) mylib::Free_Array(t);
  Error:
    ; // presumably a memory error...not sure what to do, should be rare    
  }

 int HistogramDockWidget::check_chan(mylib::Array *im)
  { int oc=ichan_;
    TRY(im->ndims<=3);
    if( (im->ndims==2) && (ichan_!=0) )
      ichan_=0;
    if( (im->ndims==3) && (ichan_>=im->dims[2]) )
      ichan_=0;
    if(ichan_!=oc)
      emit change_chan(ichan_);
    return 1;
Error:
    return 0;      
  }

}} //namspace fetch::ui
