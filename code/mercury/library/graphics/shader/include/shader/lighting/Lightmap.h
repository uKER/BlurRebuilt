#ifndef SHADER_LIGHTING_LIGHTMAP_H
#define SHADER_LIGHTING_LIGHTMAP_H

void RGBLightmap(in sampler RGBSampler, in float2 vUV, inout real3 vColour)
{
	vColour *= h4tex2D(RGBSampler, vUV).rgb;
}

void RGBLightmapNormalised(in sampler RGBSampler, in float2 vUV, inout real3 vColour)
{
	real4 vSample = h4tex2D(RGBSampler, vUV);
	
	vColour *= vSample.rgb * vSample.a;
}

void RGBLightmapNormalisedGamma(in sampler RGBSampler, in float2 vUV, inout real3 vColour)
{
	real4 vSample = h4tex2D(RGBSampler, vUV);

	vSample.rgb *= vSample.a;

	vColour *= vSample.rgb * vSample.rgb;
}

void SunSkyLightmap(in sampler SunSkySampler, in float2 vUV, inout real fSunOcclusion, inout real fSkyOcclusion)
{
	real4 vSample = h4tex2D(SunSkySampler, vUV);
	fSkyOcclusion *= vSample.g;
	fSunOcclusion *= vSample.b;
}

void SunSkyLightmapGamma(in sampler SunSkySampler, in float2 vUV, inout real fSunOcclusion, inout real fSkyOcclusion)
{
	real4 vSample = h4tex2D(SunSkySampler, vUV);
	fSkyOcclusion *= vSample.g * vSample.g;
	fSunOcclusion *= vSample.b * vSample.b;
}

void DirectionalLightmap(in sampler DirectionalSampler, in float2 vUV, inout real3 vDirection)
{
	vDirection = normalize(vDirection + tex2D(DirectionalSampler, vUV).rgb * 2.0f - 1.0f);
}

void BasisLightmap(in sampler BasisSampler, in float2 vUV, in float3 vTangentNormal, inout float3 vColour, inout float fSunOcclusion, inout float fSkyOcclusion)
{
	float4	vTexCoord		= float4(vUV, vUV + 0.5f);

	real4	vSampleA		= h4tex2D(BasisSampler, vTexCoord.xy);
			vSampleA.a		*= vSampleA.a;
			vSampleA.rgb	*= vSampleA.a;
	real4	vSampleB		= h4tex2D(BasisSampler, vTexCoord.xw);
			vSampleB.a		*= vSampleB.a;
			vSampleB.rgb	*= vSampleB.a;
	real4	vSampleC		= h4tex2D(BasisSampler, vTexCoord.zy);
			vSampleC.a		*= vSampleC.a;
			vSampleC.rgb	*= vSampleC.a;
	real4	vSampleD		= h4tex2D(BasisSampler, vTexCoord.zw);
			vSampleD		*= vSampleD;
	
	real3 vBasisA 			= real3(SQRT2OVER3,		-1.0h * 0.0h,			1.0h / SQRT3);
	real3 vBasisB 			= real3(-1.0h / SQRT6,	-1.0h * 1.0h / SQRT2,	1.0h / SQRT3);
	real3 vBasisC 			= real3(-1.0h / SQRT6,	-1.0h * -1.0h / SQRT2,	1.0h / SQRT3);	

	real4	vColourA		= real4(vSampleA.rgb, vSampleD.r);
	real4	vColourB		= real4(vSampleB.rgb, vSampleD.g);
	real4	vColourC		= real4(vSampleC.rgb, vSampleD.b);

	#ifndef PLATFORM_PS3
	real4	vBasisLight		= dot(vTangentNormal, vBasisA) * vColourA
							+ dot(vTangentNormal, vBasisB) * vColourB
							+ dot(vTangentNormal, vBasisC) * vColourC;
	#else
	real4	vBasisLight		= saturate(dot(vTangentNormal, vBasisA)) * vColourA
							+ saturate(dot(vTangentNormal, vBasisB)) * vColourB
							+ saturate(dot(vTangentNormal, vBasisC)) * vColourC;
	#endif

	vColour			*= vBasisLight.rgb;
	fSkyOcclusion	*= vBasisLight.a;
	fSunOcclusion	*= vSampleD.a;
}

void BasisLightmapNoNormal(in sampler BasisSampler, in float2 vUV, inout real3 vColour, inout real fSunOcclusion, inout real fSkyOcclusion)
{
	float4	vTexCoord		= float4(vUV, vUV + 0.5f);

	real4	vSampleA		= h4tex2D(BasisSampler, vTexCoord.xy);
			vSampleA.a		*= vSampleA.a;
			vSampleA.rgb	*= vSampleA.a;
	real4	vSampleB		= h4tex2D(BasisSampler, vTexCoord.xw);
			vSampleB.a		*= vSampleB.a;
			vSampleB.rgb	*= vSampleB.a;
	real4	vSampleC		= h4tex2D(BasisSampler, vTexCoord.zy);
			vSampleC.a		*= vSampleC.a;
			vSampleC.rgb	*= vSampleC.a;
	real4	vSampleD		= h4tex2D(BasisSampler, vTexCoord.zw);
			vSampleD		*= vSampleD;

	vColour			*= (vSampleA.rgb + vSampleB.rgb + vSampleC.rgb) * (1.0h / SQRT3);
	fSkyOcclusion	*= dot(vSampleD.rgb, (1.0h / SQRT3));
	fSunOcclusion	*= vSampleD.a;
}

void BasisLightmapNoColour(in sampler BasisSampler, in float2 vUV, in real3 vTangentNormal, inout real fSunOcclusion, inout real fSkyOcclusion)
{
	real4	vSample	= h4tex2D(BasisSampler, vUV + 0.5f);
			vSample	*= vSample;

	real3 vBasisA	= real3(-1.0h * SQRT2OVER3,		0.0h,			+1.0h / SQRT3);
	real3 vBasisB	= real3(+1.0h * +1.0h / SQRT6,	+1.0h / SQRT2,	+1.0h / SQRT3);
	real3 vBasisC	= real3(+1.0h * +1.0h / SQRT6,	-1.0h / SQRT2,	+1.0h / SQRT3);

	#ifndef PLATFORM_PS3
	fSkyOcclusion	*= dot(vTangentNormal, vBasisA) * vSample.r
					+ dot(vTangentNormal, vBasisB) * vSample.g
					+ dot(vTangentNormal, vBasisC) * vSample.b;
	#else
	fSkyOcclusion	*= saturate(dot(vTangentNormal, vBasisA)) * vSample.r
					+ saturate(dot(vTangentNormal, vBasisB)) * vSample.g
					+ saturate(dot(vTangentNormal, vBasisC)) * vSample.b;
	#endif

	fSunOcclusion	*= vSample.a;
}

void BasisLightmapNoColourNoNormal(in sampler BasisSampler, in float2 vUV, inout real fSunOcclusion, inout real fSkyOcclusion)
{
	real4	vSample	= h4tex2D(BasisSampler, vUV + 0.5f);
			vSample	*= vSample;
	
	fSkyOcclusion	*= dot(vSample.rgb, 1.0h / SQRT3);
	fSunOcclusion	*= vSample.a;
}

#endif // SHADER_LIGHTING_LIGHTMAP_H
