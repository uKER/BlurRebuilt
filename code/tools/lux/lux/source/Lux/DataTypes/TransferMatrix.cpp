#include "StdAfx.h"

#include "Lux\Sampler.h"

namespace LUX
{

namespace DataTypes
{

float3 TransferMatrix_GetSphereRand()
{
	return UniformSampleSphere(RandomFloat(),RandomFloat());
}

}

}
