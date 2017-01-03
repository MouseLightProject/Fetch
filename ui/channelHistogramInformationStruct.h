#pragma once
namespace fetch{
	namespace ui{
		struct channelHistogramInformationStruct{
			double minValue=0, maxValue=65535;
			bool autoscale=true, displayChannel=true;
			double undersaturatedPercentile=0.1, oversaturatedPercentile=0.9;
		};
	}
}