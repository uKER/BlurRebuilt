#include <Core.h>
#include <Globals.h>

#define REPLACE_LIGHTING_HIGHLIGHT

#define ENABLE_TANGENTNORMAL
#define ENABLE_BUMPMAP
#define ENABLE_SPECULARMAP
#define ENABLE_VIEWVECTOR

#define ENABLE_BUMPMAP_OPTIMISATION
#define ENABLE_LIGHTMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// replace specular

float2		g_vUVScale	< string Source = "UVScale"; string Default = "4, 4"; >;

texture2D	g_tHair		< string Source = "Hair"; string Default = "Black"; >;

sampler HairSampler = 
sampler_state
{
    Texture = <g_tHair>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	SRGB_TEXTURE
};

float3	g_vRimLightDirection		< string Source = "RimLightDirection"; string Default = "-1.0, -0.3, 0.2"; >;
float3	g_vRimLightColour			< string Source = "RimLightColour"; string Default = "0.50, 0.29, 0.13"; >;
float	g_fRimLightFactor			< string Source = "RimLightFactor"; string Default = "0.8"; >;
float	g_fRootSpecularPower		< string Source = "RootSpecularPower"; string Default = "0.25"; >;
float	g_fRootSpecularShift		< string Source = "RootSpecularShift"; string Default = "-0.20"; >;
float	g_fRootSpecularIntensity	< string Source = "RootSpecularIntensity"; string Default = "40.0"; >;
float	g_fTipSpecularShift			< string Source = "TipSpecularShift"; string Default = "0.05"; >;

real EvaluateHairSpecular(real3 vLight, real3 vBias, real3 vTangent, real3 vView, real fSpecularPower)
{
	real3	vHalf		= normalize(vLight - vView);
	real	fFactor		= saturate(1.0f - abs(dot(vHalf, vTangent + vBias)));
	//[Ken/SteveM] TODO FIX THIS: Yields this error (win32): Hair.fx(50,2): error X4579: NaN and infinity literals not allowed by shader model
	//return pow(fFactor, fSpecularPower);
	return fFactor;
}

real3 EvaluateHighlight(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
	real3	vHighlight				= 0.0f;
#if defined(ENABLE_SPECULAR)
	real4	vHairSample				= tex2D(HairSampler, vtxData.m_vTexCoord.xy * g_vUVScale);
	real	fHighlightShift			= vHairSample.g;
	real	fHighlightModulate		= vHairSample.a;
	
	real3	vStrandTangent			= cross(surfaceData.m_vNormal, surfaceData.m_vTangent);
	
	real	fSpecularBrightnessRoot	= EvaluateHairSpecular(lightData.m_vDirection, surfaceData.m_vNormal * g_fRootSpecularShift * fHighlightShift, vStrandTangent, surfaceData.m_vView, mtlData.m_fHighlightPower * g_fRootSpecularPower);
	real	fSpecularBrightnessTip	= EvaluateHairSpecular(lightData.m_vDirection, surfaceData.m_vNormal * g_fTipSpecularShift * (0.5f + 0.5f * fHighlightShift), vStrandTangent, surfaceData.m_vView, mtlData.m_fHighlightPower);

	real3	vHighlightColour		= lightData.m_vHighlight;
	real3	vRootColour				= fHighlightModulate * g_fRootSpecularIntensity * mtlData.m_vAlbedo;
	
	vHighlight += (fSpecularBrightnessRoot * vRootColour + fSpecularBrightnessTip) * vHighlightColour * g_fHighlightIntensity;
#endif

	real3	vUp						= real3(0.0f, 1.0f, 0.0f);
	real3	vRight					= cross(surfaceData.m_vView, vUp);
	real3	vRimDirection			= normalize(g_vRimLightDirection.z * surfaceData.m_vView + g_vRimLightDirection.y * vUp + g_vRimLightDirection.x * vRight);
	real	fHairFactor				= smoothstep(g_fRimLightFactor, 1.0f, 1.0f - dot(vRimDirection, surfaceData.m_vNormal));
	real3	vRimColour				= lerp(mtlData.m_vAlbedo, g_vRimLightColour, 0.5f); 

#if defined(ENABLE_SPECULAR)
	vHighlight += fHairFactor * vRimColour;
#endif

	return vHighlight;
}

// vertex shader

#include <Common/Vertex.h>

// pixel shader

#include <Common/Pixel.h>

// technique

technique Material
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}
