#include <Core.h>
#include <Globals.h>

#define FLIP_NORMAL_MAP_X
#define ENABLE_SHNODE_IN_PIXEL_SHADER

#define ENABLE_ALBEDOMAP
#define ENABLE_BUMPMAP
#define ENABLE_MATERIALMAP
#define ENABLE_SPECULAR
#define NO_LIGHTMAP

#define DISABLE_VERTEXLIGHTING
#define DISABLE_LODFADE
#define DISABLE_ALPHA_DETECT

//#define ENABLE_BUMPMAP_OPTIMISATION
//#define ENABLE_MATERIALMAP_OPTIMISATION
//#define ENABLE_LIGHTMAP_OPTIMISATION
//#define ENABLE_SPECULARMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#ifndef DAY_LIGHTING
#define REPLACE_LIGHTING_DIFFUSE
#define REPLACE_LIGHTING_HIGHLIGHT
#endif

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

#ifdef REPLACE_LIGHTING_DIFFUSE
#include <Lighting/KeyFillLighting.h>

real3 EvaluateDiffuse(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
	real3	vStaticLighting		= lightData.m_vLighting * g_vLightColour.rgb;
	real3	vDynamicLighting	= EvaluateDynamicLighting(vtxData);	
	real3	vKeyFillLighting	= EvaluateKeyFillLighting(mtlData, surfaceData, lightData, vtxData) * g_vLightColour.rgb;
	return (vStaticLighting + vDynamicLighting + vKeyFillLighting) * mtlData.m_vAlbedo.rgb;
}
#endif // REPLACE_LIGHTING_DIFFUSE

#ifdef CFG_DEBUG
	float	g_fHighlightPower < string DebugVariable = "HighlightPower"; string Default = "100.0"; string Range = "(1.0, 100.0)"; string Type = "Float"; > = 100.0f;
	float	g_fHighlightIntensity < string DebugVariable = "HighlightIntensity"; string Default = "0.5"; string Range = "(0.0, 1.0)"; string Type = "Float"; > = 0.5f;
#else
	const float	g_fHighlightPower = 100.0f;
	const float	g_fHighlightIntensity = 0.5f;
#endif

#if defined(REPLACE_LIGHTING_HIGHLIGHT)
real3 EvaluateHighlight(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
	real3	vHighlight0			= Phong(g_vCutSceneLightDir0.xyz, surfaceData.m_vNormal, surfaceData.m_vView, g_fHighlightPower);
			vHighlight0			*= g_vCutSceneLightColour0.rgb;
	real3	vHighlight1			= Phong(g_vCutSceneLightDir1.xyz, surfaceData.m_vNormal, surfaceData.m_vView, g_fHighlightPower);
			vHighlight1			*= g_vCutSceneLightColour1.rgb;
	real3	vHighlight			= (vHighlight0 + vHighlight1) * mtlData.m_fSpecularIntensity * (2.0f + g_fHighlightPower) * 0.5f * g_fHighlightIntensity;
	return vHighlight;
}
#endif // REPLACE_LIGHTING_HIGHLIGHT

/*
real4 EvaluateGlassColour(in real3 vDiffuse, in real3 vSpecular, in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData)
{	
#if defined(ENABLE_SPECULAR)
	float fFresnel	= Schlick4TwoWay(surfaceData.m_fNDotV, mtlData.m_fFresnelMidBrightness) * mtlData.m_fFresnelIntensity;	
#else
	float fFresnel	= 0.0f;		
#endif

	float3 vColour = vDiffuse;
	vColour *= g_fGlobalIntensity;
	
	float fMaterialAlpha = mtlData.m_fAlpha;
	
	// make material less transparent to account for fake glass "thickness"
	
	// make less transparent for fresnel reflection
	float fAlpha = fMaterialAlpha * (1.0f - fFresnel) + fFresnel;
	
	// premultiplied alpha for semi transparent
	vColour *= fMaterialAlpha;
	
	// add specular
	vColour += vSpecular * fFresnel;
	
	return float4(vColour, fAlpha);
}
*/
#if defined(ENABLE_SPECULAR)
real Schlick4TwoWay(real fNDotV, real fFresnelMidBrightness)
{
	float fOmDot = saturate(1.0 - abs(fNDotV));
	float fDotSquared = fOmDot * fOmDot;
	float fDot4 = fDotSquared * fDotSquared;

	return fFresnelMidBrightness + (1.0f - fFresnelMidBrightness) * fDot4; // replace with lerp?
}
#endif

real4 EvaluateGlassColour(in real3 vDiffuse, in real3 vSpecular, in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData)
{	
#if defined(ENABLE_SPECULAR)
	float fReflectivity	= Schlick4TwoWay(surfaceData.m_fNDotV, mtlData.m_fFresnelMidBrightness) * mtlData.m_fSpecularIntensity;	
#else
	float fReflectivity	= 0.0f;		
#endif

	float3 vColour = vDiffuse;
		
	float fMaterialAlpha = mtlData.m_fAlpha;
	
	// make less transparent for fresnel reflection
	float fAlpha = fMaterialAlpha * (1.0f - fReflectivity) + fReflectivity;
	
	// premultiplied alpha for semi transparent
	vColour *= fMaterialAlpha;
	
	// add specular
	vColour += vSpecular * fReflectivity;
	
	return float4(vColour, fAlpha);
}

// vertex shader

#include <Common/Vertex.h>

// pixel shader

#include <Common/Pixel.h>

C_PixelOutput GlassPS(in C_VertexOutput vtxData) 
{ 
    C_PixelOutput output = (C_PixelOutput) 0;

	C_Material	mtlData		= EvaluateMaterial(vtxData);
	C_Surface	surfaceData	= EvaluateSurface(mtlData, vtxData);
	C_Lighting	lightData	= EvaluateLighting(mtlData, surfaceData, vtxData);

	real3	vDiffuse		= EvaluateDiffuse(mtlData, surfaceData, lightData, vtxData);
	real3	vSpecular		= EvaluateSpecular(mtlData, surfaceData, lightData, vtxData);

	output.m_vColour.xyzw	= EvaluateGlassColour(vDiffuse, vSpecular, mtlData, surfaceData, lightData);

	//output.m_vColour		= EvaluateFog(output.m_vColour, vtxData.m_vScreenPos.w, surfaceData.m_fDistance);
	output.m_vColour		= EvaluateFinalColour(output.m_vColour);

	 output.m_vColour		= DebugColour(output.m_vColour, mtlData, vtxData);

    return output;
}

// technique

technique Material < string SortType = "backfront";  bool TranslucentForPVS = true; >
{
	pass p0
	{
        AlphaBlendEnable = TRUE;
        AlphaTestEnable = FALSE;
        SrcBlend = One;
        DestBlend = InvSrcAlpha;

        ZWriteEnable = TRUE;
        //CullMode = NONE;
        
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 GlassPS();
	}
}


/*
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

*/
