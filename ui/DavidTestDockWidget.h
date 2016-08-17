#pragma once // Ensures that DavidTestDockWidget.h only included once in a compilation
#include <QtWidgets>
namespace mylib{
#include "array.h" // Include array.h from mylib, associate it with namespace mylib. ie, copied array.h contents into namespace mylib
}

class QCustomPlot;

namespace fetch { //Namespace fetch
	namespace ui {//Namespace ui in namespace fetch

		class DavidTestDockWidget : public QDockWidget
		{//DavidTestDockWidget class is a derived class of QDockWidget. The following are private, by default
			Q_OBJECT //MOC tool reads c++ header file and if it sees Q_OBJECT macro, produces c++ source file for meta-object code; basically says it has gui elements.http://stackoverflow.com/questions/1368584/what-does-the-q-object-macro-do-why-do-all-qt-objects-need-this-macro
				QCustomPlot      *plot_; //plot_ is a pointer to a QCustomPlot data type
			size_t            ichan_; //ichan_ is an unsigned data type
			mylib::Array     *last_; //last_ is a pointer to an Array data type of namespace mylib
			bool              is_live_; //is_live_ is a boolean
			QVector<double>   x_, pdf_, cdf_; //QVector stores items in adjacent memory locations for fast index-based access. Here three QVectors are created that store doubles.
			double            minX_, maxX_, perct_; //Doubles
			QLineEdit         *leMin_, *leMax_, *lePerct_; //Pointers to QLineEdit data types, which allow for one line text editing
		public:
			DavidTestDockWidget(QWidget *parent = NULL);//Constructor, takes in a pointer to a Qwidget data type, initialized to NULL.

		signals: //Emitted by object when state has changed (Signals must use voide, can't return)
			void change_chan(int ichan); //When necessary, this will signal to a slot with the argument ichan

		public slots: //These are fucntions that can receive signals
			void set(mylib::Array *im); //Function takes in a pointer to an Array data type that is in the namespace mylib
			void set_ichan(int ichan);  //Function that takes in ichan
			void set_live(bool is_live); //Function that sets axes
			void rescale_axes(); //Function to rescale axes
			void reset_minmax(); //Function to reset minmax
			void set_percentile(QString); //Function to set percentile, takes in Qstring
		private: //Other private functions that take in im, which is a pointer to an Array datatype in namespace mylib
			void swap(mylib::Array *im);
			int  check_chan(mylib::Array *im);
			void compute(mylib::Array *im);

		};

	}
} // namespace fetch::ui
