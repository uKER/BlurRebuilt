#include <Core.h>
#include <Globals.h>

#define REPLACE_MATERIAL
#define REPLACE_LIGHTING_DIFFUSE

#define ENABLE_LIGHTMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#define ENABLE_BUMPMAP
#define ENABLE_SPECULARMAP
#define ENABLE_EMISSIVE

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

float fDisplacement;
float fFresnelMidBrightness;
float fRefractiveIndex;


float g_fRefractiveIndex		< string Source = "RefractiveIndex"; string Default = "1.0"; string Type = "Float"; >				= 1.0f;
float g_fDisplacement			< string Source = "Displacement"; string Default = "0.0"; string Type = "Float"; >					= 0.0f;


C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData = (C_Material) 0;

	float2 vTexCoord = vtxData.m_vTexCoord.xy;

	mtlData	= EvaluateTangentNormal(mtlData, vTexCoord);

	float3 vSpecNormalTS = mtlData.m_vTangentNormal;
	
	
	float3 vViewWS = vtxData.m_vView;
	float3 vViewTS;
	
	float3 vTangent = normalize(vtxData.m_vTangent);
	float3 vBinormal = normalize(vtxData.m_vBinormal);
	float3 vNormal = normalize(vtxData.m_vNormal);
	
	vViewTS = vViewWS.x * float3(vTangent.x, vBinormal.x, vNormal.x)
			+ vViewWS.y * float3(vTangent.y, vBinormal.y, vNormal.y)
			+ vViewWS.z * float3(vTangent.z, vBinormal.z, vNormal.z);
			
    float3 refractedRay = refract(vViewTS, vSpecNormalTS, 1.0f / g_fRefractiveIndex);
    float2 uvOffset = refractedRay.xy * (g_fDisplacement / refractedRay.z);	
    
    //float2 uvOffset = vViewTS.xy / g_fDisplacement * vViewTS.z;
    
	float2 vTexCoordRefract = vTexCoord + uvOffset;

	mtlData	= EvaluateAlbedo(mtlData, vTexCoordRefract);
	mtlData	= EvaluateSpecularParameters(mtlData, vTexCoord);
	mtlData	= EvaluateEmissive(mtlData, vTexCoordRefract);
	
	return mtlData;
}

real3 EvaluateDiffuse(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
	real3 vDiffuse = 0.0f;
	float3 vNormal = normalize(vtxData.m_vNormal.xyz);
	
#if !defined(CFG_CONVERTER)
	vDiffuse += lightData.m_vLighting * g_vBakedColour.rgb;
	vDiffuse += g_vSkyColour.rgb * lightData.m_fSkyOcclusion * saturate(vNormal.y * 0.5f + 0.5f);
	vDiffuse += g_vSunColour.rgb * saturate(dot(vNormal.xyz, g_vSunDirection.xyz)) * lightData.m_fSunOcclusion;
#else
	vDiffuse += float3(1.0f, 1.0f, 1.0f);
#endif
	vDiffuse *= mtlData.m_vAlbedo * g_fDiffuseIntensity;
	return vDiffuse;
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
