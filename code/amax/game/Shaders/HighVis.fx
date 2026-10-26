#include <Core.h>
#include <Globals.h>

#define ENABLE_ALBEDOMAP
#define ENABLE_BUMPMAP
#define ENABLE_MATERIALMAP
#define ENABLE_SPECULAR
#define ENABLE_EMISSIVE

//#define DISABLE_VERTEXLIGHTING

#define ENABLE_BUMPMAP_OPTIMISATION
//#define ENABLE_MATERIALMAP_OPTIMISATION
#define ENABLE_LIGHTMAP_OPTIMISATION
//#define ENABLE_SPECULARMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#define REPLACE_MATERIAL

#define ENABLE_TEXTURE_NAMING_CONVENTIONS

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h> 

float g_fReflectorIntensity			< string Source = "ReflectorIntensity"; string Default = "20.0"; string Type = "Float"; string range = "0.0, 1.0"; >	= 20.0f;
float g_fEmissiveIntensity			< string Source = "EmissiveIntensity"; string Default = "1.0"; string Type = "Float"; string range = "0.0, 1.0"; >	= 1.0f;

#if defined(REPLACE_MATERIAL)
C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData		= (C_Material) 0;

	float2	vTexCoord		= vtxData.m_vTexCoord.xy;

	mtlData	= EvaluateAlbedo(mtlData, vTexCoord);
	mtlData	= EvaluateTangentNormal(mtlData, vTexCoord);
	mtlData	= EvaluateMaterialParameters(mtlData, vtxData, vTexCoord);
	
	// we will be calculating the dynamic lighting twice but for now this is ok
	real3 vDynamicLighting = EvaluateDynamicLighting(vtxData);
	mtlData.m_vEmissive *= g_fReflectorIntensity * vDynamicLighting + g_fEmissiveIntensity;
	
	return mtlData;
}
#endif

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

#if !defined(CFG_CONVERTER)
technique MaterialLights < string Define = "ENABLE_LIGHTING_PREPASS"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}
#endif

#if PLATFORM_PS3

technique MaterialAlphaLuminance < string Define = "ALPHA_LUMINANCE"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

technique MaterialAlphaLuminanceLights < string Define = "ALPHA_LUMINANCE; ENABLE_LIGHTING_PREPASS"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

#endif

#if !defined(CFG_CONVERTER)
technique MaterialPrepass < string Define = "ENABLE_PREPASS"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 PrepassVS();
		PixelShader = compile ps_3_0 PrepassPS();
	}
}
#endif

#if !defined(CFG_CONVERTER)
technique MaterialPrepassSolid < string Define = "OPAQUE_PASS; ENABLE_PREPASS"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 PrepassVS();
		PixelShader = compile ps_3_0 PrepassPS();
	}
}
#endif

