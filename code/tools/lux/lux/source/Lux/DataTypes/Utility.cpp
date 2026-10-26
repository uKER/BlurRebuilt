#include "stdafx.h"

namespace LUX
{

namespace DataTypes
{

// convert RGB to alpha scaled RGBA
float4 GetAlphaScaled(const float3 &vColour)
{
	// apply alpha scaling
	float fMax = 0;
	if (vColour.x > fMax) fMax = vColour.x;
	if (vColour.y > fMax) fMax = vColour.y;
	if (vColour.z > fMax) fMax = vColour.z;

	if (fMax > 1.f)
		fMax = 1.f;

	float fInv;
	if (fMax > 0)
		fInv = 1.0f / fMax;
	else
		return float4(0,0,0,0);

	return float4(vColour.x*fInv, vColour.y*fInv, vColour.z*fInv, fMax);
}

// clamp the xyz, such that they are all within [0,1] and their ratios are preserved
float3 LinearClamp(const float3 &v)
{
	float fMax = v.x;
	if (v.y>fMax) fMax = v.y;
	if (v.z>fMax) fMax = v.z;
	if (fMax<1.f)
		return v;

	float fInv = 1.f/fMax;

	return float3(v.x*fInv, v.y*fInv, v.z*fInv);
}

// clamp the xyz components independantly
float3 Clamp(const float3 &_v)
{
	float3 v(_v);

	if (v.x > 1.f) v.x = 1.f; else if (v.x < 0.f) v.x = 0.f;
	if (v.y > 1.f) v.y = 1.f; else if (v.y < 0.f) v.y = 0.f;
	if (v.z > 1.f) v.z = 1.f; else if (v.z < 0.f) v.z = 0.f;

	return v;
}

/// turn an SH vector into a vector that can be dotted with a normal to get a valid lighting value
float4 PreTransformSHVector(const float4 &_v)
{
	float4 v;

    const float fC0 = 0.282094777; //1.0f / (2.0f * sqrtf(PI));
    const float fC1 = 0.325734973; //sqrt(3.0f) / (3.0f * sqrtf(PI));

	v.w = fC0*_v.x;

	v.y = -fC1*_v.y;
	v.z =  fC1*_v.z;
	v.x = -fC1*_v.w;

	return v;
}

}

}
