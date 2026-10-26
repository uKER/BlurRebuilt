#ifndef SHADER_LIGHTING_DYNAMIC_H
#define SHADER_LIGHTING_DYNAMIC_H

real PointLight(float4 vPositionFalloff, float3 vWorldPosition, real3 vWorldNormal)
{
	float3	vLightPosition	= vPositionFalloff.xyz;
	real	fLightFalloff	= vPositionFalloff.w;

	float3	vLightToWorld	= vLightPosition - vWorldPosition;
	real	fDistance		= dot(vLightToWorld, vLightToWorld);
	real	fFalloff		= 1.0h - saturate(fDistance * fLightFalloff);

	real3	vLightDirection	= normalize(vLightToWorld);
	real	fNDotL			= saturate(dot(vLightDirection, vWorldNormal));

	return fNDotL;
}

real3 PointLight(float4 vPositionFalloff, real3 vColour, float3 vWorldPosition, real3 vWorldNormal)
{
	return vColour * PointLight(vPositionFalloff, vWorldPosition, vWorldNormal);
}

real3 PointLight4(float4 vPositionFalloffs[4], real4 vRed, real4 vGreen, real4 vBlue, float3 vWorldPosition, real3 vWorldNormal)
{
	real4 vFalloffConstants;
	vFalloffConstants.x = vPositionFalloffs[0].w;
	vFalloffConstants.y = vPositionFalloffs[1].w;
	vFalloffConstants.z = vPositionFalloffs[2].w;
	vFalloffConstants.w = vPositionFalloffs[3].w;

	float3 vLightToWorld0 = vPositionFalloffs[0].xyz - vWorldPosition;
	float3 vLightToWorld1 = vPositionFalloffs[1].xyz - vWorldPosition;
	float3 vLightToWorld2 = vPositionFalloffs[2].xyz - vWorldPosition;
	float3 vLightToWorld3 = vPositionFalloffs[3].xyz - vWorldPosition;

	real4 vDistances;
	vDistances.x = dot(vLightToWorld0, vLightToWorld0);
	vDistances.y = dot(vLightToWorld1, vLightToWorld1);
	vDistances.z = dot(vLightToWorld2, vLightToWorld2);
	vDistances.w = dot(vLightToWorld3, vLightToWorld3);
	real4 vFalloffs = 1.0h - saturate(vDistances * vFalloffConstants);

	real4x3 vDirections;
	vDirections[0] = normalize(vLightToWorld0);
	vDirections[1] = normalize(vLightToWorld1);
	vDirections[2] = normalize(vLightToWorld2);
	vDirections[3] = normalize(vLightToWorld3);

	real4 vNDotL = saturate(mul(vDirections, vWorldNormal));
	real4 vIntensities = saturate(vNDotL * vFalloffs);

	real3 vDiffuse;
	vDiffuse.r = dot(vIntensities, vRed);
	vDiffuse.g = dot(vIntensities, vGreen);
	vDiffuse.b = dot(vIntensities, vBlue);

	return vDiffuse;
}

#endif // SHADER_LIGHTING_DYNAMIC_H
