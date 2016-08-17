/** \file
DavidTestGraph Dock Widget

\todo never need to rescale cdf axis, always 0 to 1
\todo options for scaling on x-axis...full range, zoom to range (and fix)
/todo size x axis by cdf? e.g where cdf is between 0.05 to 0.95?
\todo inspection by hover over cdf
*/
#include "DavidTestDockWidget.h"//Inlclude necessary headers
#include "qcustomplot.h"
#include "common.h"
#include <cmath>  

namespace mylib {
#include "image.h"//Include image.h in namespace mylib
}

//Below, the (from wiki) "restirct keyword is a declaration of intent given by the programmer to the compiler. It says that for the lifetime of the pointer, only the pointer itself or a value directly derived from it (such as pointer +1) will be used to access the object to which it points.
#ifdef _MSC_VER
#define restrict __restrict 
#else
#define restrict __restrict__ 
#endif

//Defining things, including macros that can take in arguments. PANIC and FAIL call the message handler with the supplied fatal message, printed to stderr, presumably returning the file and line, # sign is the stringizing operator. Similar for debug
//Do while 0 is useful for multistatement macros since their text replaces anywhere the macro is called, which can lead to weird errors with semicolons etc
#define ENDL "\r\n"
#define PANIC(e) do{if(!(e)){qFatal("%s(%d)"ENDL "\tExpression evalatuated as false."ENDL "\t%s"ENDL,__FILE__,__LINE__,#e);           }}while(0)
#define FAIL     do{         qFatal("%s(%d)"ENDL "\tExecution should not reach here."ENDL,__FILE__,__LINE__);                                 }while(0)
#define TRY(e)   do{if(!(e)){qDebug("%s(%d)"ENDL "\tExpression evalatuated as false."ENDL "\t%s"ENDL,__FILE__,__LINE__,#e);goto Error;}}while(0)
#define HERE     qDebug("%s(%d). HERE."ENDL,__FILE__,__LINE__)

//1 bit shifted by 12 so 1 changes to 00010000 00000000 (4096)
#define HINT_NBINS (1<<12) 

namespace fetch{//Fetch namespace
	namespace ui {//Ui namespace in Fetch namespace
		//Defining constructor outside class itself, with initialization list (QDockWidget is the parent class of DavidTestDockWidget, so we are also calling parent constructor) giving this widget title DavidTest
		DavidTestDockWidget::DavidTestDockWidget(QWidget *parent)
			: QDockWidget("DavidTest", parent)
			, plot_(0)
			, ichan_(0)
			, last_(0)
			, x_(HINT_NBINS)
			, pdf_(HINT_NBINS)
			, cdf_(HINT_NBINS)
			, minX_(DBL_MAX)
			, maxX_(0)
			, perct_(0.5)
			, leMin_(0)
			, leMax_(0)
			, lePerct_(0)
		{ //This is the definition of the constructor
			QBoxLayout *layout = new QVBoxLayout; //QBoxLayout class lines up WIDGETS horizontally or vertically, QVBoxLayout makes them veritcal. layout is a pointer to data type QBoxLayout
			{//Brackets reduces scope of local variables, which would be destroyed outside of the scope. So create pointers, but don't need them since they are passed to the functions
				QWidget *w = new QWidget(this); //w is a pointer to a Qwidget data type (base class of all user interface objects), set to equal a new QWidget, which allocates and constructs it on the heap, this is the overall class address, used for member functions; here it is used as the parent of qwidget. New means it is allocated on the heap rather than stack.
				w->setLayout(layout); //Calls member function of *w (class pointed to by w), setting the layout to layout (pointer to QBoxLayout)
				setWidget(w); //Sets the widget of the dock widget to widget
			}

			QFormLayout *form = new QFormLayout; //form is a pointer to type QFormLayout, and new is allocationg a QFormLayout on the heap. It's children are layed out as two columns: left are labels, right are fields
			// channel selector
			{//First row
				QComboBox *c = new QComboBox; //c is a pointer to type QComboBox and new is allocating one on the heap. It is a button with popuplist
				for (int i = 0; i<4; ++i) // initially add 4 channels to select
					c->addItem(QString("%1").arg(i), QVariant()); //arg(i) will be the string, QVariant not set to anything
				PANIC(connect(c, SIGNAL(currentIndexChanged(int)),
					this, SLOT(set_ichan(int)))); //Try to connect: when index of combobox (with address c) changed, currentIndexChanged signal emitted and slot function (set_ichan(int)) is called, as a member of "this", where this is address of class
				PANIC(connect(this, SIGNAL(change_chan(int)),
					c, SLOT(setCurrentIndex(int)))); //Try to connect: "this" is address of class, and signal to be emitted is change_chan(int), and slot function to be called upon signal is c->setCurrentIndex(int)
				form->addRow(tr("Channel:"), c); //Add row labeled channel (tr is just translate) with field combobox c. Passed by const reference
			}

	{// Second row
		QHBoxLayout *row = new QHBoxLayout();//row is a pointer to type QHBoxLayout, new allocates QHBoxLayout on heap, lines up widgets horizontally
		QPushButton *b; //b is a pointer to a QPushButton type
		// Live update button
		b = new QPushButton("Live update"); //allocating on heap, with label "Live Update"
		b->setCheckable(true); //To toggle behavior
		PANIC(connect(b, SIGNAL(toggled(bool)),
			this, SLOT(set_live(bool))));//Try to connect. signal if button is toggled, then slot results in calling function this-> set_live()
		b->setChecked(true); //{resumably it is true by default
		row->addWidget(b); //Add pushbutton widget b to row

		// Rescale button
		b = new QPushButton("Rescale"); //allocating on heap, with label "Rescale"
		PANIC(connect(b, SIGNAL(pressed()),
			this, SLOT(rescale_axes())));//Try to connect. signal if button is pressed, then slot results in calling function this->rescale_axes()
		b->setChecked(true);//By default, it is checked, which is unnecessary since this is not checkable
		row->addWidget(b); //Add Rescale button to row
		form->addRow(row); //Add this now to the overall form so we will now have Channel: combobox \n |Live Update|   |Rescale|
	}

	{//Third row
		QHBoxLayout *row = new QHBoxLayout(); //Starting a new row
		QPushButton *b; //Another pushbutton
		//percentile label
		QLabel *perctLabel = new QLabel(tr("Percentile:"));//perctLAbel is pointer to QLabel type, with label Percentile:
		row->addWidget(perctLabel); //Add perctLabel to the row
		///// Line Edit Control - Gamma
		lePerct_ = new QLineEdit("0.5");//lePerct is a QlineEdit type which is a one line text editor, here with default value 0.5
		QCompleter *c = new QCompleter(new QStringListModel(lePerct_), lePerct_);// Add completion based on history
		c->setCompletionMode(QCompleter::UnfilteredPopupCompletion); // Most likely suggestion listed as current
		c->setModelSorting(QCompleter::UnsortedModel); // Unsorted
		lePerct_->setCompleter(c); // Set the completer
		lePerct_->setValidator(new QDoubleValidator(0.0001, 1.0000, 4, lePerct_));      // Add validated input
		PANIC(connect(lePerct_, SIGNAL(textEdited(QString)),
			this, SLOT(set_percentile(QString))));//Try to connect: textEdited(QString) signal is sent when text is edited, slot: this->set_percentile(Qstring) is then called
		row->addWidget(lePerct_); //Add this to row

		// Reset button
		b = new QPushButton("Reset"); // Reset button
		PANIC(connect(b, SIGNAL(pressed()),
			this, SLOT(reset_minmax()))); //Signal if the button is pressed, upon which this->reset_minmax() gets called
		row->addWidget(b); //Add this to row
		form->addRow(row); //Add this row to widget
	}

	{ //Fourth Row
		QHBoxLayout *row = new QHBoxLayout(); //Another row, which is pointer to type QHBoxLayout();

		QLabel *lbMin = new QLabel(tr("Minimum:")); //Minimum label
		row->addWidget(lbMin); //Add this to row
		///// Line Edit Control - Minimum
		leMin_ = new QLineEdit("DBL_MAX");
		leMin_->setReadOnly(true); //Sets it to read only
		row->addWidget(leMin_); //Adds this to row


		QLabel *lbMax = new QLabel(tr("Maximum:")); //Maximum label
		row->addWidget(lbMax); //Add this to row
		///// Line Edit Control - Maximum
		leMax_ = new QLineEdit("0"); //Edit text box, by default set to 0
		leMax_->setReadOnly(true);; //Set to read only
		row->addWidget(leMax_); //Add this to row
		form->addRow(row); //Add row to form
	}

	layout->addLayout(form); //Adds form to the layout, which says that the widgets should be stacked vertically

	// plot
	plot_ = new QCustomPlot;
	plot_->setMinimumHeight(100);
	plot_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); //Presumably that it can expand
	plot_->addGraph(); // Graph 0 uses xAxis and yAxis by default
	plot_->graph(0)->setPen(QPen(Qt::blue));
	plot_->addGraph(plot_->xAxis, plot_->yAxis2); //Graph 1 uses xAxis and yAxis2
	plot_->graph(1)->setPen(QPen(Qt::red));
	plot_->xAxis->setLabel("Intensity");
	plot_->xAxis2->setVisible(true);
	plot_->yAxis->setLabel("PDF");
	plot_->yAxis2->setLabelColor(Qt::blue);
	plot_->yAxis2->setVisible(true);
	plot_->yAxis2->setLabel("CDF");
	plot_->yAxis2->setLabelColor(Qt::red);
	layout->addWidget(plot_); //add plot to the layout
		}

		// histogram utilities START 
		//define a macro to switch the types of values. CASE is defined in the next function
#define TYPECASES(ARRAYARG) do {\
    switch(ARRAYARG->type)  \
		    { CASE( UINT8_TYPE ,u8); \
      CASE( UINT16_TYPE,u16);\
      CASE( UINT32_TYPE,u32);\
      CASE( UINT64_TYPE,u64);\
      CASE(  INT8_TYPE ,i8); \
      CASE(  INT16_TYPE,i16);\
      CASE(  INT32_TYPE,i32);\
      CASE(  INT64_TYPE,i64);\
      CASE(FLOAT32_TYPE,f32);\
      CASE(FLOAT64_TYPE,f64);\
      default: \
        FAIL;  \
		    }}while(0)

		static double amin(mylib::Array *a) //takes in a, which points to type Array of namespace mylib
		{
			double out; //Below define first sets m to the first element of a->data. It will then loop through each element of a to find the smallest element of a
#define CASE(ID,T) case mylib::ID: {const T *d=(T*)a->data; T m=d[0]; for(size_t i=1;i<a->size;++i) m=(d[i]<m)?d[i]:m; out=(double)m;} break;
			TYPECASES(a);
#undef CASE
			return out;
		}

		static double amax(mylib::Array *a)
		{
			double out; //Same as above, but to find max
#define CASE(ID,T) case mylib::ID: {const T *d=(T*)a->data; T m=d[0]; for(size_t i=1;i<a->size;++i) m=(d[i]>m)?d[i]:m; out=(double)m;} break;    
			TYPECASES(a);
#undef CASE
			return out;
		}

		static unsigned nbins(mylib::Array *a, double min, double max) //takes in pointer a to mylib::Array type, min and max. returns nbins to use
		{
			unsigned n, lim = 1 << 12; // max number of bins
			switch (a->type)
			{
			case mylib::UINT8_TYPE:
			case mylib::INT8_TYPE:
				lim = 1 << 8;
			case mylib::UINT16_TYPE:
			case mylib::UINT32_TYPE:
			case mylib::UINT64_TYPE:
			case mylib::INT16_TYPE:
			case mylib::INT32_TYPE:
			case mylib::INT64_TYPE:
				n = max - min + 1;
				return (n<lim) ? n : lim;  //Will return the minimum of n and lim
			case mylib::FLOAT32_TYPE:
			case mylib::FLOAT64_TYPE:
				return lim;
			default:
				FAIL; //Will fail except for int8, int64 and float64
			}
		}

		static double binsize(unsigned n, double min, double max) // Number of bins n, min and max
		{
			return (n == 0) ? 0 : (((double)n) - 1) / (max - min); //returns bins/step
		}

		template<class T>
		static void count(double *restrict pdf, size_t nbins, T *restrict data, size_t nelem, T min, double dy)
		{//Template static function. Restrict means only pointer, or value derived from pointer (eg pointer+1) will be used to access object to which it points
			for (size_t i = 0; i<nelem; ++i) pdf[(size_t)((data[i] - min)*dy)]++; //loops through all the data to create a PDF by incrmenting bins by 1
			for (size_t i = 0; i<nbins; ++i) pdf[i] /= (double)nelem; //normalizes pdf
		}
		static void scan(double *restrict cdf, double *restrict pdf, size_t n)
		{
			memcpy(cdf, pdf, n*sizeof(double));//copies n*sizeof(double) bits from location pointed to by pdf to location pointed to by cdf
			for (size_t i = 1; i<n; ++i) cdf[i] += cdf[i - 1]; //creates cdf
		}
		static void setbins(double *x, size_t n, double min, double dy)
		{
			for (size_t i = 0; i<n; ++i) x[i] = i / dy + min; //x location of bins
		}

		static size_t findIndex(QVector<double> &cdf, double perct) //QVector is a template class
		{// Takes in reference to cdf which is of class QVector<double>, and perct, returns index of value closest to perct
			size_t index;
			double * cdfdata = cdf.data(); //pointer to cdf.data
			double diff = 1.0, minDiff = 1.0;

			for (size_t i = 0; i<cdf.size(); ++i)
			{
				diff = abs(cdfdata[i] - perct);
				if (diff <= minDiff)
				{
					minDiff = diff;
					index = i;
				}
			}

			return index; //Index of element closest to perct
		}

		static void histogram(QVector<double> &x, QVector<double> &pdf, QVector<double> &cdf, mylib::Array *a)
		{//References to x, pdf and cdf, as well as pointer a to type mylib::Array. Creates PDF and CDF
			double min, max, dy;
			unsigned n;
			min = amin(a); //minimum of a
			max = amax(a); //maximum of a
#if 0
			HERE; //Debug stuff
			mylib::Write_Image("histogram.tif", a, mylib::DONT_PRESS);
#endif
			//    debug("%s(%d) %s"ENDL "\tmin %6.1f\tmax %6.1f"ENDL,__FILE__,__LINE__,__FUNCTION__,min,max);
			n = nbins(a, min, max);
			dy = binsize(n, min, max); // value transform is  (data[i]-min)*dy --> for max this is (max-min)*(nbins-1)/(max-min)

			x.resize(n); //resized to n, so if X is bigger than n, elements after n are cut off. else elements are added with default constructed value
			pdf.resize(n);
			cdf.resize(n);

#define CASE(ID,T) case mylib::ID: count<T>(pdf.data(),n,(T*)a->data,a->size,min,dy); break;
			TYPECASES(a); //count is above function, creating PDF
#undef CASE
			scan(cdf.data(), pdf.data(), n);//creates CDF where .data is a pointer to the data stored in the vector
			setbins(x.data(), n, min, dy);// sets x bins
		}
		// histogram utilities END

		void DavidTestDockWidget::set(mylib::Array *im)
		{//set function in class DavidTestDockWidget
			if (!is_live_) return;//if it is not alive, then return from this function
			TRY(check_chan(im)); //If not true, go to Error
			swap(im); // make last_ now point to im
			compute(im);
		Error:
			; // bad input, ignore 
		}

		static int g_inited = 0;

		/** Presumes channels are on different planes */
		void DavidTestDockWidget::compute(mylib::Array *im)
		{//compute function in class DavidTestDockWidget
			mylib::Array t, *ch; //t is of type mylib::Array, ch is a pointer to type mylib::Array
			QString tempStr;
			TRY(ch = mylib::Get_Array_Plane(&(t = *im), ichan_)); //select channel    
			histogram(x_, pdf_, cdf_, ch); // Creates PDF and CDF
			plot_->graph(0)->setData(x_, pdf_); // Plots PDF
			plot_->graph(1)->setData(x_, cdf_); // Plots CDF
			if (!g_inited)  // Above this is set to 0
			{
				plot_->graph(0)->rescaleAxes(); // Rescale axes, I guess the first time
				plot_->graph(1)->rescaleAxes(); // Rescale axes
				g_inited = 1;
			}
			plot_->replot(); // Replot, so rescaling etc comes into effect

			//find intensity value for the input CDF percentile 
			int index;
			index = findIndex(cdf_, perct_); //Finds the index of the CDF element closest to perct_, initialized to 0.5
			double xValue;
			xValue = x_.data()[index]; // Get the x value corresponding to index
			if (xValue < minX_)  minX_ = xValue; // Resets minX_ and maxX_
			if (xValue > maxX_)  maxX_ = xValue;

			tempStr.setNum(minX_); // Relables xmin and xmax editlabels
			leMin_->setText(tempStr);
			tempStr.setNum(maxX_);
			leMax_->setText(tempStr);

		Error:
			; // memory error or oob channel, should never get here.    
		}

		void DavidTestDockWidget::rescale_axes()
		{ //rescale_axes function in class DavidTestDockWidget, to rescale both axes
			plot_->graph(0)->rescaleAxes();
			plot_->graph(1)->rescaleAxes();
		}

		void DavidTestDockWidget::set_ichan(int ichan)
		{//set_ichan function in class DavidTestDockWidget
			ichan_ = ichan; //ichan_ equal to ichan
			if (last_) // if last_ is pointing to something
			{
				check_chan(last_); //checks that last_ is valid
				compute(last_); //computes the value corresponding to given percentile for CDF
			}
		}

		void DavidTestDockWidget::set_live(bool is_live)
		{//set_live function in class DavidTestDockWidget
			is_live_ = is_live;
		}

		void DavidTestDockWidget::reset_minmax()
		{ //reset_minmax function in class DavidTestDockWidget
			minX_ = DBL_MAX;
			maxX_ = 0;
		}

		void DavidTestDockWidget::set_percentile(QString text)
		{//set_percentile function in class DavidTestDockWidget
			perct_ = text.toDouble();
			reset_minmax(); // Resets min max
		}

		/** updates the last_ array with a new one. */
		void DavidTestDockWidget::swap(mylib::Array *im)
		{//Make last_ now point to im
			mylib::Array *t = last_; //set t pointer equal to last_ pointer, which is initialized to zero
			TRY(last_ = mylib::Copy_Array(im)); //Try to make last_ point to im, else go to Error:
			if (t) mylib::Free_Array(t); //if t (last_) now points to a real address, then free_array(t)? maybe this gets rid of t
		Error:
			; // presumably a memory error...not sure what to do, should be rare    
		}

		int DavidTestDockWidget::check_chan(mylib::Array *im)
		{
			int oc = ichan_; //set oc to ichan_, initialized to 0
			TRY(im->ndims <= 3); //ensures ndmis of im is less than 3, else goes to error
			if ((im->ndims == 2) && (ichan_ != 0))
				ichan_ = 0; // if ichan_ does not equal 0 and the number of dimensions equals two, set ichan_ to 0
			if ((im->ndims == 3) && (ichan_ >= im->dims[2]))
				ichan_ = 0; //if im has 3 dimensions and ichan_ is greater than the value is dimension 3, then set ichan_ to 0
			if (ichan_ != oc)
				emit change_chan(ichan_); // after the above if statements, if ichan_ does not equal oc, then signal a channel changed
			return 1;
		Error:
			return 0;
		}

	}
} //namspace fetch::ui
