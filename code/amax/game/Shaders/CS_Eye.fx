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

#define ENABLE_TANGENTNORMAL
#define ENABLE_TANGENTVIEW

//#define ENABLE_BUMPMAP_OPTIMISATION
//#define ENABLE_MATERIALMAP_OPTIMISATION
//#define ENABLE_LIGHTMAP_OPTIMISATION
//#define ENABLE_SPECULARMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#ifndef DAY_LIGHTING
#define REPLACE_LIGHTING_DIFFUSE
#endif

#define REPLACE_LIGHTING_HIGHLIGHT
#define REPLACE_MATERIAL

#define REPLACE_VERTEXSHADER_OUTPUT

#include <Common/Defines.h>
#include <Common/Parameters.h>

#ifdef REPLACE_VERTEXSHADER_OUTPUT
struct C_VertexOutput
{
	VS_OUTPUT_BASE
	float3	m_vHighlightOffset	: TEXCOORD8;
};
#endif


#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

float		g_fHighlightPower < string Source = "HighlightPower"; string Default = "100.0f"; string Type = "Float"; string range = "0.0, 1000.0"; >		= 100.0f;
float		g_fEyeHighlightIntensity < string Source = "EyeHighlightIntensity"; string Default = "2.0f"; string Type = "Float"; string range = "0.0, 200.0"; >		= 2.0f;
float		g_fSpecularIntensity < string Source = "SpecularIntensity"; string Default = "0.25f"; string Type = "Float"; string range = "0.0, 1.0"; >		= 0.25f;
float		g_fFresnelMidBrightness < string Source = "FresnelMidBrightness"; string Default = "0.3f"; string Type = "Float"; string range = "0.0, 1.0"; >		= 0.3f;
float		g_fCubemapSmoothness < string Source = "CubemapSmoothness"; string Default = "50.0f"; string Type = "Float"; string range = "0.0, 1.0"; >		= 1.0f;

float g_fRefractiveIndex		< string Source = "RefractiveIndex"; string Default = "2.0"; string Type = "Float"; string range = "1.0, 4.0"; >				= 2.0f;
float g_fDisplacement			< string Source = "Displacement"; string Default = "0.0"; string Type = "Float"; string range = "0.0, 1.0"; >					= 0.0f;

#include <Lighting/KeyFillLighting.h>

#ifdef REPLACE_LIGHTING_DIFFUSE

real3 EvaluateDiffuse(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
	real3	vStaticLighting		= lightData.m_vLighting * g_vLightColour.rgb;
	real3	vDynamicLighting	= EvaluateDynamicLighting(vtxData);	
	real3	vKeyFillLighting	= EvaluateKeyFillLighting(mtlData, surfaceData, lightData, vtxData) * g_vLightColour.rgb;
	return (vStaticLighting + vDynamicLighting + vKeyFillLighting) * mtlData.m_vAlbedo.rgb;
}
#endif // REPLACE_LIGHTING_DIFFUSE

#ifdef REPLACE_LIGHTING_HIGHLIGHT
/*
real3 EvaluateHighlight(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{

	//real3	vLightDirOffset = vtxData.m_vHighlightOffset;
	//normalize(lightData.m_vDirection + vLightDirOffset);
	//real3	vLightDirection	= normalize( float3(1.0f, 0.5f, 1.0f) );
	
	real3	vLightDirection	= normalize(float3(0.5f, 0.25f, 0.5f) + vtxData.m_vHighlightOffset);
	
	real3	vHighlight		= 0.0f;
	
	float fHighlightPower = 10.0f;
	
#if defined(ENABLE_SPECULAR)
	vHighlight += Phong(vLightDirection, surfaceData.m_vNormal, surfaceData.m_vView, g_fHighlightPower) * g_fEyeHighlightIntensity;
#endif

	return vHighlight;
}
*/

real3 EvaluateHighlight(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
	//real3	vDir0				= normalize(g_vCutSceneLightDir0.xyz + vtxData.m_vHighlightOffset);
	//real3	vHighlight0			= Phong(vDir0, surfaceData.m_vNormal, surfaceData.m_vView, g_fHighlightPower);
	//		vHighlight0			*= g_vCutSceneLightColour0.rgb;
	real3	vDir1				= normalize(g_vCutSceneLightDir1.xyz + vtxData.m_vHighlightOffset);
	real3	vHighlight1			= Phong(vDir1, surfaceData.m_vNormal, surfaceData.m_vView, g_fHighlightPower);
			vHighlight1			*= g_vCutSceneLightColour1.rgb;
	real3	vDir2				= normalize(g_vCutSceneLightDir2.xyz + vtxData.m_vHighlightOffset);
	real3	vHighlight2			= Phong(vDir2, surfaceData.m_vNormal, surfaceData.m_vView, g_fHighlightPower);
			vHighlight2			*= g_vCutSceneLightColour2.rgb;
	//real3	vHighlight			= (vHighlight0 + vHighlight1) * mtlData.m_fSpecularIntensity * (2.0f + g_fHighlightPower) * 0.5f;
	real3	vHighlight			= (vHighlight1 + vHighlight2) * mtlData.m_fSpecularIntensity * (2.0f + g_fHighlightPower) * 0.5f;

	//real3	vLightDirection	= normalize(vtxData.m_vHighlightOffset);
	//vHighlight += Phong(vLightDirection, surfaceData.m_vNormal, surfaceData.m_vView, g_fHighlightPower);

	return vHighlight * g_fEyeHighlightIntensity;
}
#endif // REPLACE_LIGHTING_HIGHLIGHT



#ifdef REPLACE_MATERIAL
C_Material EvaluateEyeMaterialParameters(in C_Material mtlData)
{
#if defined(DAY_LIGHTING)

#else
	mtlData.m_fSpecularIntensity	= g_fSpecularIntensity;
	mtlData.m_fFresnelMidBrightness	= g_fFresnelMidBrightness;
	mtlData.m_fHalfSmoothness		= g_fCubemapSmoothness;
	mtlData.m_vEmissive				= 0.0f;
#endif
	return mtlData;
}

C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData		= (C_Material) 0;

	float2	vTexCoord		= vtxData.m_vTexCoord.xy;

	mtlData	= EvaluateTangentNormal(mtlData, vTexCoord);

	// TODO : optimise this into same texture sample as tangent normal
    float fDisplacement = tex2D(NormalSampler, vTexCoord).a;    	

	float3 vSpecNormalTS = float3(0.0f, 0.0f, 1.0f);
	//float3 vSpecNormalTS = mtlData.m_vTangentNormal.xyz;
		
	float3 vViewTS = normalize(vtxData.m_vTangentView.xyz);
			
    float3 vRefractedRay = refract(vViewTS, vSpecNormalTS, 1.0f / g_fRefractiveIndex);
    float fDist = (g_fDisplacement * fDisplacement / vRefractedRay.z);
    float2 uvOffset = vRefractedRay.xy * fDist;	
    
	float2 vTexCoordRefract = vTexCoord + uvOffset;

	mtlData	= EvaluateAlbedo(mtlData, vTexCoordRefract);
		
	mtlData	= EvaluateEyeMaterialParameters(mtlData);
	
	//mtlData.m_vAlbedo = lerp( mtlData.m_vAlbedo, 1.0f, (1.0f - tex2D(NormalSampler, vTexCoordRefract).a) * fDisplacement);
	
	/*
	float3 vTest = normalize(vtxData.m_vHighlightOffset);
	float fTestDot = dot(vtxData.m_vNormal, vTest);
	mtlData.m_vAlbedo = 0.5f;
	if(fTestDot > 0.85f)
	{
		mtlData.m_vAlbedo = float3(1.0f, 0.0f, 0.0f);
	}
	*/
	return mtlData;
}

#endif // REPLACE_MATERIAL



// vertex shader

#include <Common/Vertex.h>

float	g_fHighlightOffset	< string Source = "HighlightOffset"; string Default = "1.0"; string Type = "Float"; >	= 1.0f;

#ifdef REPLACE_VERTEXSHADER_OUTPUT
C_VertexOutput EyeVS(C_VertexInput input)
{
    C_VertexOutput output = MainVS(input);

#if !defined(CFG_CONVERTER)
	float3	vBindPoseHighlightOffset = float3(0.0f, 0.0f, -g_fHighlightOffset);
	
#if defined(ENABLE_SKINNING)
	float3	vObjectHighlightOffset	= 0.0f;
	for (int i = 0; i < 4; i++)
    {
		float fBlendWeight = input.m_vBlendWeights[i];
		if (fBlendWeight > 0.0f)
		{
			int iBlendIndex = input.m_vBlendIndices[i];

			float3x4 mBone			= g_mPalette[iBlendIndex];
			float3x3 mBoneNormal	= (float3x3) mBone;

			vObjectHighlightOffset.xyz	+= mul(mBoneNormal, vBindPoseHighlightOffset).xyz * fBlendWeight;
		}
    }
#else
	float3	vObjectHighlightOffset	= vBindPoseHighlightOffset;
#endif

	output.m_vHighlightOffset = mul(vObjectHighlightOffset, (float3x3) g_mWorld);
#endif

    return output;    
}
#endif

// pixel shader

#include <Common/Pixel.h>

// technique

technique Material
{
	pass p0
	{
		VertexShader = compile vs_3_0 EyeVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

#if !defined(CFG_CONVERTER)
technique MaterialLights < string Define = "ENABLE_LIGHTING_PREPASS"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 EyeVS();
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

