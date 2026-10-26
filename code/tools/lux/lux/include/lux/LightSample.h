#ifndef __LUX_LIGHTSAMPLE_H__
#define __LUX_LIGHTSAMPLE_H__

#include "Lux\RenderChannel.h"
#include "Lux\DataTypes\BaseType.h"
#include "Lux\DataTypes\PolymorphicType.h"

// a light sample is a piece of data (usually a colour), a direction and a target channel
// the light sample owns the SampleData, and deletes it when it's destroyed
class LightSample
{
public:
	union
	{
	LUX::RenderChannel::Target target;
	int iChannel;
	};
	int iSubChannel;
	LUX::DataTypes::PolymorphicType vColour;
//	float3 vColour;
	float3 vDir;
};

#endif
