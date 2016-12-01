#pragma once
namespace fetch{
	namespace ui{
		struct channelHistogramInformationStruct{
			int minValue=0;
			int maxValue=65535;
			bool autoscale=true;
		};
	}
}