#pragma once
#include<QOBJECT>

//DGA: Simple class to send signals (particularly useful when interfacing with uis for properties not set in cfg file)
namespace fetch{ namespace ui{ //DGA: This class is in namespace fetch and ui
	class simpleSignalerClass: public QObject //DGA: Class inherits from QObject
	{ Q_OBJECT //DGA: Makes it a Q_OBJECT so that it can have signals etc
	  signals:
		//DGA: The following overloaded signals are to take into account of a few standard inputs that might be sent with a signal, including nothing, a bool, an int a float and a QString
		void signaler();
		void signaler(bool setValue);
		void signaler(int setValue);
		void signaler(float setValue);
		void signaler(QString setValue);
	};
}}//DGA: end of ui and fetch namespaces