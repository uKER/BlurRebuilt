#include "render\shaders\psCommon.h"

real2 ConvertNormalCartesianToSpherical(real3 vCartesian)
{
	real2 vSpherical;
	vSpherical.x = atan2(vCartesian.y, vCartesian.x) * (1.0h / 3.14159h);
	vSpherical.y = vCartesian.z;
	return vSpherical * 0.5h + 0.5h;
}

real3 ConvertNormalSphericalToCartesian(real2 vSpherical)
{
	real2 vSinCosTheta, vSinCosPhi;
	vSpherical = vSpherical * 2.0h - 1.0h;
	sincos(vSpherical.x * 3.14159h, vSinCosTheta.x, vSinCosTheta.y);
	vSinCosPhi = real2(sqrt(1.0h - vSpherical.y * vSpherical.y), vSpherical.y);
	return real3(vSinCosTheta.y * vSinCosPhi.x, vSinCosTheta.x * vSinCosPhi.x, vSinCosPhi.y);
}

real3 UnpackViewSpaceNormal16(real2 vPackedNormal)
{
	real3 vViewNormal;;
	vViewNormal.xy = vPackedNormal.xy * 2.0h - 1.0h;
	vViewNormal.z = -sqrt(1.0h - dot(vViewNormal.xy, vViewNormal.xy));
	return vViewNormal;
}

real3 UnpackViewSpaceSphericalNormal16(real2 vPackedNormal)
{
	return ConvertNormalSphericalToCartesian(vPackedNormal);
}

float UnpackViewSpaceDepth16(real2 vPackedDepth)
{
	return vPackedDepth.x + (1.0f / 255.0f) * vPackedDepth.y;
}

float3 GetViewPosition(sampler2D depth, float2 uv, float fDepthScale, float3 vViewScale)
{
      float fZ = SampleDepthBuffer(depth, uv);
      return (1.0f / (fZ - fDepthScale)) * vViewScale;
}

float3 GetViewPosition(sampler2D depth, float2 uv, float4 vXYScale, float4 vZScale)
{
	float	fDepth = SampleDepthBuffer(depth, uv, vZScale);
	float3	vViewPosition;
	vViewPosition.xy = uv * vXYScale.xy + vXYScale.zw;
	vViewPosition.xy = vViewPosition.xy * fDepth;
	vViewPosition.z = fDepth;
	return vViewPosition;
}

void GetViewPositionAndNormal(sampler2D normalDepth, float2 uv, float3 vInterpolated, out float3 vViewPosition, out real3 vViewNormal)
{
	real4 vPackedNormalDepth = tex2D(normalDepth, uv);
	vViewNormal = UnpackViewSpaceNormal16(vPackedNormalDepth.xy);
	vViewPosition = vInterpolated * UnpackViewSpaceDepth16(vPackedNormalDepth.zw);
}

void GetViewNormal(sampler2D normalDepth, float2 uv, float3 vInterpolated, out real3 vViewNormal)
{
	real4 vPackedNormalDepth = tex2D(normalDepth, uv);
	vViewNormal = UnpackViewSpaceNormal16(vPackedNormalDepth.xy);
}

void GetViewPositionAndNormal(sampler2D normalDepth, float2 uv, real4 vInvProjScale, float fFarClip, out float3 vViewPosition, out real3 vViewNormal)
{
	real4 vPackedNormalDepth = tex2D(normalDepth, uv);
	float fDepth = UnpackViewSpaceDepth16(vPackedNormalDepth.zw) * fFarClip;
	vViewPosition.xy = uv * vInvProjScale.xy + vInvProjScale.zw;
	vViewPosition.xy = vViewPosition.xy * fDepth;
	vViewPosition.z = fDepth;
	vViewNormal = UnpackViewSpaceNormal16(vPackedNormalDepth.xy);
}
