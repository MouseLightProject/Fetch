#pragma once
#include<QOBJECT>
#include<QSettings>

namespace fetch{
namespace ui{
	class simpleUiUpdater :public QObject
	{ QSettings settings;
		Q_OBJECT
	public:
		template<typename T> void updateSettings(QString settingString, T newSetting){settings.setValue(settingString,newSetting);}
	signals:
		void signal_somethingChanged();
		void signal_somethingChanged(QString val);
	};
}
}