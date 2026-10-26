#include <Core.h>
#include <Globals.h>

#define ENABLE_ALBEDOMAP
#define ENABLE_BUMPMAP
#define ENABLE_MATERIALMAP
#define ENABLE_SPECULAR
#define ENABLE_EMISSIVE

//#define DISABLE_VERTEXLIGHTING

//#define ENABLE_BUMPMAP_OPTIMISATION
//#define ENABLE_MATERIALMAP_OPTIMISATION
#define ENABLE_LIGHTMAP_OPTIMISATION
//#define ENABLE_SPECULARMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

//#define REPLACE_MATERIAL

#define ENABLE_TEXTURE_NAMING_CONVENTIONS

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h> 

real	g_fReflectorIntensity	< string Source = "ReflectorIntensity"; string Default = "20.0"; string Type = "Float"; string range = "0.0, 1.0"; >	= 20.0f;
real	g_fNeonFarIntensity		< string Source = "NeonFarIntensity"; string Default = "0.0"; string Type = "Float"; string range = "0.0, 1.0"; >	= 0.0f;
real	g_fNeonNearIntensity	< string Source = "NeonNearIntensity"; string Default = "0.0"; string Type = "Float"; string range = "0.0, 1.0"; >	= 0.0f;
real	g_fNeonRange			< string Source = "NeonRange"; string Default = "100.0"; string Type = "Float"; string range = "0.0, 1000.0"; >	= 100.0f;
real3	g_fTint					< string Source = "Tint"; string Default = "1, 1, 1"; >								= float3(1.0f, 1.0f, 1.0f);

// vertex shader

#include <Common/Vertex.h>

// pixel shader

#include <Common/Pixel.h>

#include <Shader/Common/Debug.h>

C_PixelOutput HighVisPS(in C_VertexOutput vtxData) 
{ 
    C_PixelOutput output = (C_PixelOutput) 0;

	C_Material	mtlData		= EvaluateMaterial(vtxData);
	C_Surface	surfaceData	= EvaluateSurface(mtlData, vtxData);
	C_Lighting	lightData	= EvaluateLighting(mtlData, surfaceData, vtxData);

	real3	vDiffuse		= EvaluateDiffuse(mtlData, surfaceData, lightData, vtxData);
	real3	vSpecular		= EvaluateSpecular(mtlData, surfaceData, lightData, vtxData);

	// apply high vis glow
	// we will be calculating the dynamic lighting twice but for now this is ok
	real3 vDynamicLighting = EvaluateDynamicLighting(vtxData);
	real fDistanceFactor = saturate(surfaceData.m_fDistance / g_fNeonRange);
	mtlData.m_vEmissive *= g_fReflectorIntensity * vDynamicLighting + lerp(g_fNeonNearIntensity, g_fNeonFarIntensity, fDistanceFactor) * g_fTint;

	output.m_vColour.xyz	= EvaluateColour(vDiffuse, vSpecular, mtlData, surfaceData);
	output.m_vColour.w		= mtlData.m_fAlpha;

#if defined(ENABLE_FOG) && !defined(LOD)
	output.m_vColour		= EvaluateDepthFog(output.m_vColour, surfaceData.m_fDistance);
#endif
	output.m_vColour		= EvaluateFinalColour(output.m_vColour);

#if !defined(CFG_CONVERTER)
#if !defined(DISABLE_LODFADE)
#if !defined(LODFADE_IN_VERTEX_CONSTANT)
	output.m_vColour.a		*= EvaluateLodFade(surfaceData);
#else
	output.m_vColour.a		*= saturate(vtxData.m_vNormal.w);
#endif
#endif
#endif

	 output.m_vColour		= DebugColour(output.m_vColour, mtlData, vtxData);

    return output;
}

// technique

technique Material
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 HighVisPS();
	}
}

#if !defined(CFG_CONVERTER)
technique MaterialLights < string Define = "ENABLE_LIGHTING_PREPASS"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 HighVisPS();
	}
}
#endif

#if PLATFORM_PS3

technique MaterialAlphaLuminance < string Define = "ALPHA_LUMINANCE"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 HighVisPS();
	}
}

technique MaterialAlphaLuminanceLights < string Define = "ALPHA_LUMINANCE; ENABLE_LIGHTING_PREPASS"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 HighVisPS();
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

