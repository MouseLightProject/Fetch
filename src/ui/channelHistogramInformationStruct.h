#include "simpleSignalerClass.h"
#pragma once
//DGA: Structure used to store the min and max cutoff values for each channel's scaling, bools to determine whether autoscale is selected or display channel is selected and saturation percentiles
namespace fetch{
	namespace ui{
		struct channelHistogramInformationStruct{
			double minValue=0, maxValue=65535;
			bool autoscale=true, displayChannel=true;
			double undersaturatedPercentile=0.05, oversaturatedPercentile=0.9999;
		};
	}
}