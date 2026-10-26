#define ALPHA_LUMINANCE

#include <Core.h>
#include <Globals.h>

#define REPLACE_MATERIAL
#define REPLACE_LIGHTING_DIFFUSE
#define REPLACE_SURFACE

#define ENABLE_LIGHTMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#define ENABLE_BUMPMAP
#define ENABLE_SPECULAR
#define ENABLE_SCREENCOORD
#define ENABLE_WORLD

#include <Common/Defines.h>

#if defined(ENABLE_TANGENTNORMAL)
#	undef ENABLE_TANGENTNORMAL
#endif

#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// water surface shader
#include <Special/Water.h>

#include <Shader/Misc/Resolve.h>

float2		g_vNormalScale0		< string Source = "NormalScale0"; string Default = "1, 1"; string Type = "Float2"; >			= float2(1.0f, 1.0f);
float2		g_vNormalScale1		< string Source = "NormalScale1"; string Default = "1, 1"; string Type = "Float2"; >			= float2(1.0f, 1.0f);
float2		g_vNormalVelocity0	< string Source = "NormalVelocity0"; string Default = "0.01, 0.01"; string Type = "Float2"; >	= float2(0.01f, 0.01f);
float2		g_vNormalVelocity1	< string Source = "NormalVelocity1"; string Default = "0.01, 0.00"; string Type = "Float2"; >	= float2(0.01f, 0.00f);

texture2D	g_tBlendNormal		< string Source = "BlendNormal"; string Default = "Black"; >;

sampler BlendNormalSampler = 
sampler_state
{
    Texture = <g_tBlendNormal>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

#if defined(PLATFORM_PS3) && defined(__PIXEL__)
#pragma texformat g_tBlendNormal COMPRESSED_RGBA_S3TC_DXT1
#endif

real	g_fWaterSpecularIntensity	< string Source = "SpecularIntensity"; string Default = "1.0"; string Type = "Float"; >			= 1.00h;
real	g_fWaterReflection			< string Source = "Reflection"; string Default = "0.10"; string Type = "Float"; >				= 0.10h;

real3	g_vWaterColour				< string Source = "WaterColour"; string Default = "0.30, 0.73, 0.63"; string Type = "Float3"; >	= real3(0.30h, 0.73h, 0.63h);
real	g_fWaterRefractAmount		< string Source = "RefractAmount"; string Default = "0.1"; string Type = "Float"; >				= 0.1h;
real	g_fWaterExtinctionScale		< string Source = "ExtinctionScale"; string Default = "1.0"; string Type = "Float"; >			= 1.0h;
real	g_fWaterDiffuseScale		< string Source = "DiffuseScale"; string Default = "0.2"; string Type = "Float"; >				= 0.2h;

C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData = (C_Material) 0;
	
	mtlData.m_vAlbedo				= 1.0h;
	mtlData.m_fAlpha				= 1.0h;

	real2	vTexCoord0				= vtxData.m_vTexCoord.xy * g_vNormalScale0 + g_fTime * g_vNormalVelocity0;
	real2	vTexCoord1				= vtxData.m_vTexCoord.xy * g_vNormalScale1 + g_fTime * g_vNormalVelocity1;
	real3	vTangentNormal0			= tex2D(NormalSampler, vTexCoord0).xzy;
	real3	vTangentNormal1			= tex2D(BlendNormalSampler, vTexCoord1).xzy;
	mtlData.m_vTangentNormal		= normalize(vTangentNormal0 + vTangentNormal1 - 1.0h);

	mtlData.m_fSpecularIntensity	= g_fWaterSpecularIntensity;
	mtlData.m_fHalfSmoothness		= g_fWaterSurfaceSmoothness;
	mtlData.m_fFresnelMidBrightness	= 0.0h;
	mtlData.m_vEmissive				= 0.0h;

	return mtlData;
}

C_Surface EvaluateSurface(in C_Material mtlData, in C_VertexOutput vtxData)
{
	C_Surface surfaceData	= (C_Surface) 0;
#if defined(ENABLE_WORLD)
	surfaceData.m_vPosition	= vtxData.m_vWorld.xyz;
#endif
#if defined(ENABLE_NORMAL)
	surfaceData.m_vNormal	= mtlData.m_vTangentNormal.xyz;
#endif
#if defined(ENABLE_VIEW)
	surfaceData.m_vView		= normalize(vtxData.m_vView.xyz);
#endif
#if !defined(LOD)
	surfaceData.m_fDistance	= vtxData.m_vScreenPos.z;
#endif
#if defined(ENABLE_SPECULAR)
	surfaceData.m_fNDotV	= dot(surfaceData.m_vNormal, surfaceData.m_vView);
	surfaceData.m_vReflect	= surfaceData.m_vView - 2.0h * surfaceData.m_fNDotV * surfaceData.m_vNormal;
	surfaceData.m_fFresnel	= Schlick4(surfaceData.m_fNDotV) * mtlData.m_fSpecularIntensity;
#endif
	return surfaceData;
}

real3 EvaluateDiffuse(in C_Material mtlData, in out C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
#if defined(DAY_LIGHTING)
	real3	vSunDiffuse	= Diffuse(g_vSunDirection.xyz, surfaceData.m_vNormal) * g_vSunColour.rgb * lightData.m_fSunOcclusion;
	real3	vSkyDiffuse	= g_vSkyColour.rgb * lightData.m_fSkyOcclusion;
	real3	vDiffuse	= (vSunDiffuse + vSkyDiffuse);
#elif defined(NIGHT_LIGHTING)
	real3	vDiffuse	= lightData.m_vLighting * g_vLightColour.rgb;
#else
	real3	vDiffuse	= 0.0h;
#endif
#if !defined(CFG_CONVERTER)
	real2	vScreenCoords	= vtxData.m_vScreenPos.xy / vtxData.m_vScreenPos.z;
#if !defined(PLATFORM_PS3)
			vScreenCoords	+= g_vHalfPixelOffset.xy;
#endif

#if defined(BLUR_DX11) && defined(WATER_NO_DEPTH)
	// [Blur2PC][Blur1PCDX11][retail-recovered] The shipped *NoDepth techniques
	// (refract pass at graphical detail 1) skip the depth read: distance is 1.
	float	fWaterDepth		= vtxData.m_vScreenPos.z;
	float	fDistUnderwater	= 1.0h;
#else
	float	fSceneDepth		= SampleDepthBuffer(DepthBufferSampler, vScreenCoords, g_vDepthScale);
	float	fWaterDepth		= vtxData.m_vScreenPos.z; 
	float	fDistUnderwater	= max(0.0h, fSceneDepth - fWaterDepth);
#endif

#ifdef SPLITSCREEN
	real	fRefractAmount  = 0;
#else
	real	fRefractAmount	= saturate(g_fWaterRefractAmount * fDistUnderwater / (1.0h + 0.01h * fWaterDepth));
#endif
	real2	vRefractCoords	= vScreenCoords.xy + surfaceData.m_vNormal.xz * surfaceData.m_fNDotV * fRefractAmount;
	
	real3	vRefractColour	= SampleColourBuffer(ColourBufferSampler, vRefractCoords);

	real3	vExtinction		= exp(-g_vWaterColour * (g_fWaterExtinctionScale * fDistUnderwater));
	real3	vInScattering	= vDiffuse * g_vWaterColour * g_fWaterDiffuseScale * saturate(fDistUnderwater);
	
	vDiffuse				= vRefractColour * vExtinction + vInScattering;

	surfaceData.m_fFresnel	*= saturate(fDistUnderwater);
#else
	vDiffuse				= g_vWaterColour.rgb;
#endif
	return vDiffuse;
}

// vertex shader

#include <Common/Vertex.h>

// pixel shader

#include <Common/Pixel.h>

// technique

technique Material < string SortType = "postresolve"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

technique MaterialLights <string SortType = "postresolve"; string Define = "ENABLE_LIGHTING_PREPASS";>
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

technique MaterialLSplitScreen <string SortType = "postresolve"; string Define = "SPLITSCREEN; ENABLE_LIGHTING_PREPASS";>
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

#if defined(BLUR_DX11)
// [Blur2PC][Blur1PCDX11][retail-recovered] Shipped NoDepth subtechniques.
technique MaterialNoDepth <string SortType = "postresolve"; string Define = "WATER_NO_DEPTH";>
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

technique MaterialLightsNoDepth <string SortType = "postresolve"; string Define = "ENABLE_LIGHTING_PREPASS; WATER_NO_DEPTH";>
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

technique MaterialLSplitScreenNoDepth <string SortType = "postresolve"; string Define = "SPLITSCREEN; ENABLE_LIGHTING_PREPASS; WATER_NO_DEPTH";>
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}
#endif
