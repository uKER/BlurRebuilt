#include <Core.h>
#include <Globals.h>

#define REPLACE_LIGHTING_HIGHLIGHT

#define ENABLE_BUMPMAP
#define ENABLE_SPECULARMAP

#define ENABLE_LIGHTMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// replace specular highlight

real3 EvaluateHighlight(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
	real3	vNormal			= surfaceData.m_vNormal.xyz;
#if defined(ENABLE_TANGENTNORMAL)
	real3	vBinormal		= surfaceData.m_vBinormal.xyz;
#else
	real3	vBinormal		= float3(0.0f, 1.0f, 0.0f);
#endif
	real3	vLightDirection	= normalize(lightData.m_vDirection + 2.0f * vNormal - 0.5f * vBinormal);
	real3	vHighlight		= 0.0f;
#if defined(ENABLE_SPECULAR)
#if defined(BLINN)
	vHighlight += lightData.m_vHighlight * Blinn(vLightDirection, surfaceData.m_vNormal, surfaceData.m_vView, mtlData.m_fHighlightPower) * mtlData.m_fHighlightIntensity * g_fHighlightIntensity;
#else
	vHighlight += lightData.m_vHighlight * Phong(vLightDirection, surfaceData.m_vNormal, surfaceData.m_vView, mtlData.m_fHighlightPower) * mtlData.m_fHighlightIntensity * g_fHighlightIntensity;
#endif
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
