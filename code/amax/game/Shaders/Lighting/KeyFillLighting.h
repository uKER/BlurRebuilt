#ifndef SHADER_SCENE_KEY_FILL_LIGHTING_H
#define SHADER_SCENE_KEY_FILL_LIGHTING_H

shared float4			g_vCutSceneLightColour0	: register(CCP_LIGHTING_CUTSCENELIGHT_COLOUR0)	= float4(0.0f, 0.0f, 0.0f, 1.0f);
shared float4			g_vCutSceneLightColour1	: register(CCP_LIGHTING_CUTSCENELIGHT_COLOUR1)	= float4(0.0f, 0.0f, 0.0f, 1.0f);
shared float4			g_vCutSceneLightColour2	: register(CCP_LIGHTING_CUTSCENELIGHT_COLOUR2)	= float4(0.0f, 0.0f, 0.0f, 1.0f);
shared float4			g_vCutSceneLightColour3	: register(CCP_LIGHTING_CUTSCENELIGHT_COLOUR3)	= float4(0.0f, 0.0f, 0.0f, 1.0f);
shared float4			g_vCutSceneLightDir0	: register(CCP_LIGHTING_CUTSCENELIGHT_DIR0)		= float4(1.0f, 0.0f, 0.0f, 0.0f);
shared float4			g_vCutSceneLightDir1	: register(CCP_LIGHTING_CUTSCENELIGHT_DIR1)		= float4(1.0f, 0.0f, 0.0f, 0.0f);
shared float4			g_vCutSceneLightDir2	: register(CCP_LIGHTING_CUTSCENELIGHT_DIR2)		= float4(1.0f, 0.0f, 0.0f, 0.0f);
shared float4			g_vCutSceneLightDir3	: register(CCP_LIGHTING_CUTSCENELIGHT_DIR3)		= float4(1.0f, 0.0f, 0.0f, 0.0f);

real3 EvaluateKeyFillLighting(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
{
#if defined(CFG_CONVERTER)
	return real3(0.0f, 0.0f, 0.0f);
#else
	real4	vLightFactors;
	vLightFactors.x	= saturate(dot(g_vCutSceneLightDir0.xyz, surfaceData.m_vNormal.xyz) * g_vCutSceneLightColour0.w + (1.0f - g_vCutSceneLightColour0.w));
	vLightFactors.y	= saturate(dot(g_vCutSceneLightDir1.xyz, surfaceData.m_vNormal.xyz) * g_vCutSceneLightColour1.w + (1.0f - g_vCutSceneLightColour1.w));
	vLightFactors.z	= saturate(dot(g_vCutSceneLightDir2.xyz, surfaceData.m_vNormal.xyz) * g_vCutSceneLightColour2.w + (1.0f - g_vCutSceneLightColour2.w));
	vLightFactors.w	= saturate(dot(g_vCutSceneLightDir3.xyz, surfaceData.m_vNormal.xyz) * g_vCutSceneLightColour3.w + (1.0f - g_vCutSceneLightColour3.w));

	real3	vLightColour = 0.0h;
	vLightColour += g_vCutSceneLightColour0.rgb * vLightFactors.x;
	vLightColour += g_vCutSceneLightColour1.rgb * vLightFactors.y;
	vLightColour += g_vCutSceneLightColour2.rgb * vLightFactors.z;
	vLightColour += g_vCutSceneLightColour3.rgb * vLightFactors.w;

	return vLightColour * g_vLightColour.rgb;
#endif
}

real3 EvaluateKeyFillLighting(in C_Material mtlData, in C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData, real3 vDiffuseWrap)
{
#if defined(CFG_CONVERTER)
	return real3(0.0f, 0.0f, 0.0f);
#else

	real4	vLightFactors;
	vLightFactors.x	= dot(g_vCutSceneLightDir0.xyz, surfaceData.m_vNormal.xyz) * g_vCutSceneLightColour0.w + (1.0f - g_vCutSceneLightColour0.w);
	vLightFactors.y	= dot(g_vCutSceneLightDir1.xyz, surfaceData.m_vNormal.xyz) * g_vCutSceneLightColour1.w + (1.0f - g_vCutSceneLightColour1.w);
	vLightFactors.z	= dot(g_vCutSceneLightDir2.xyz, surfaceData.m_vNormal.xyz) * g_vCutSceneLightColour2.w + (1.0f - g_vCutSceneLightColour2.w);
	vLightFactors.w	= dot(g_vCutSceneLightDir3.xyz, surfaceData.m_vNormal.xyz) * g_vCutSceneLightColour3.w + (1.0f - g_vCutSceneLightColour3.w);

	real3	vLightColour = 0.0h;
	vLightColour += g_vCutSceneLightColour0.rgb * saturate(vLightFactors.x * (1.0f - vDiffuseWrap * g_vCutSceneLightColour0.w) + vDiffuseWrap * g_vCutSceneLightColour0.w);
	vLightColour += g_vCutSceneLightColour1.rgb * saturate(vLightFactors.y * (1.0f - vDiffuseWrap * g_vCutSceneLightColour1.w) + vDiffuseWrap * g_vCutSceneLightColour1.w);
	vLightColour += g_vCutSceneLightColour2.rgb * saturate(vLightFactors.z);
	vLightColour += g_vCutSceneLightColour3.rgb * saturate(vLightFactors.w);

	return vLightColour * g_vLightColour.rgb;
#endif
}

#endif
