#pragma once
#include<QOBJECT>

namespace fetch{
namespace ui{
	class simpleUiUpdater :public QObject
	{
		Q_OBJECT
	public:
		void somethingChanged(){ emit signal_somethingChanged(); }
	signals:
		void signal_somethingChanged();
	};
}
}