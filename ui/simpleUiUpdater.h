#pragma once
#include<QOBJECT>

//DGA: Simple class to update the graphical user interface when corresponding device properties (not defined in cfg) are set
namespace fetch{ //DGA: This class is in namespace fetch and ui
namespace ui{

	class simpleUiUpdater: public QObject //DGA: Class inherits from QObject
	{ Q_OBJECT //DGA: Makes it a Q_OBJECT so that it can have signals etc
	  signals:
		//DGA: The following overloaded signals are to take into account of a few standard inputs that might be sent with a signal, including nothing, a bool and a QString
		void signal_valueSet();
		void signal_valueSet(bool setValue);
		void signal_valueSet(QString setValue);
	};

}}//DGA: end of ui and fetch namespaces