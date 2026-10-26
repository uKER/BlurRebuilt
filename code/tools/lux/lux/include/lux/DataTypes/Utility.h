#ifndef __LUX_DATATYPES_UTILITY_H__
#define __LUX_DATATYPES_UTILITY_H__

namespace LUX
{

namespace DataTypes
{

/// convert RGB to alpha scaled RGBA
float4 GetAlphaScaled(const float3 &vColour);

/// clamp the xyz, such that they are all within [0,1] and their ratios are preserved
float3 LinearClamp(const float3 &v);

/// clamp the xyz components independantly
float3 Clamp(const float3 &v);

/// turn an SH vector into a vector that can be dotted with a normal to get a valid lighting value
float4 PreTransformSHVector(const float4 &_v);

};

};

#endif
