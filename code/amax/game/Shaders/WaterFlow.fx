#define ALPHA_LUMINANCE

#include <Core.h>
#include <Globals.h>

#define REPLACE_MATERIAL
#define REPLACE_LIGHTING_DIFFUSE
#define REPLACE_SURFACE

#define ENABLE_LIGHTMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#define ENABLE_ALBEDOMAP
#define ENABLE_BUMPMAP
#define ENABLE_SPECULAR
#define ENABLE_SCREENCOORD

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// water surface shader
#include <Special/Water.h>

#include <Shader/Misc/Resolve.h>

float2		g_vNormalScale		< string Source = "NormalScale"; string Default = "1, 1"; string Type = "Float2"; >					= float2(1.0f, 1.0f);
float2		g_vAlbedoScale		< string Source = "AlbedoScale"; string Default = "1, 1"; string Type = "Float2"; >					= float2(1.0f, 1.0f);
float2		g_vNormalVelocity	< string Source = "NormalVelocity"; string Default = "0.01, 0.01"; string Type = "Float2"; >		= float2(0.01f, 0.01f);
float2		g_vAlbedoVelocity	< string Source = "AlbedoVelocity"; string Default = "0.01, 0.00"; string Type = "Float2"; >		= float2(0.01f, 0.00f);

real	g_fWaterSpecularIntensity	< string Source = "SpecularIntensity"; string Default = "1.0"; string Type = "Float"; >			= 1.00h;
real	g_fWaterReflection			< string Source = "Reflection"; string Default = "0.10"; string Type = "Float"; >				= 0.10h;

real3	g_vWaterColour				< string Source = "WaterColour"; string Default = "0.30, 0.73, 0.63"; string Type = "Float3"; >	= real3(0.30h, 0.73h, 0.63h);
real	g_fWaterRefractAmount		< string Source = "RefractAmount"; string Default = "0.1"; string Type = "Float"; >				= 0.1h;
real	g_fWaterExtinctionScale		< string Source = "ExtinctionScale"; string Default = "1.0"; string Type = "Float"; >			= 1.0h;
real	g_fWaterDiffuseScale		< string Source = "DiffuseScale"; string Default = "0.2"; string Type = "Float"; >				= 0.2h;

C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData = (C_Material) 0;
	
	real2	vAlbedoTexCoord			= vtxData.m_vTexCoord.xy * g_vAlbedoScale + g_fTime * g_vAlbedoVelocity;
	real4	vAlbedo					= h4tex2D(AlbedoSampler, vAlbedoTexCoord);
	mtlData.m_vAlbedo				= vAlbedo.rgb;
	mtlData.m_fAlpha				= vAlbedo.a;

	real2	vNormalTexCoord			= vtxData.m_vTexCoord.xy * g_vNormalScale + g_fTime * g_vNormalVelocity;
	mtlData.m_vTangentNormal		= normalize(h4tex2D(NormalSampler, vNormalTexCoord).xyz - 0.5h);

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
#if defined(ENABLE_TANGENTNORMAL)
	real3	vWorldNormal	= normalize(vtxData.m_vNormal.xyz);
	real3	vWorldTangent	= normalize(vtxData.m_vTangent.xyz);
	real3	vWorldBinormal	= normalize(vtxData.m_vBinormal.xyz);
	surfaceData.m_vNormal	= normalize(vWorldNormal * mtlData.m_vTangentNormal.z + vWorldTangent * mtlData.m_vTangentNormal.x + vWorldBinormal * mtlData.m_vTangentNormal.y);
	surfaceData.m_vTangent	= vWorldTangent;
	surfaceData.m_vBinormal	= vWorldBinormal;
#elif defined(ENABLE_NORMAL)
	surfaceData.m_vNormal	= normalize(vtxData.m_vNormal.xyz);
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

real3 EvaluateDiffuse(in out C_Material mtlData, in out C_Surface surfaceData, in C_Lighting lightData, in C_VertexOutput vtxData)
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

#if defined(SPLITSCREEN)
	real	fRefractAmount  = 0.0f;
#else
	real	fRefractAmount	= saturate(g_fWaterRefractAmount * fDistUnderwater / (1.0h + 0.01h * fWaterDepth));
#endif
	real2	vRefractCoords	= vScreenCoords.xy + surfaceData.m_vNormal.xz * surfaceData.m_fNDotV * fRefractAmount;
	
	real3	vRefractColour	= SampleColourBuffer(ColourBufferSampler, vRefractCoords);

	real3	vExtinction		= exp(-g_vWaterColour * (g_fWaterExtinctionScale * fDistUnderwater));
	real3	vInScattering	= vDiffuse * g_vWaterColour * g_fWaterDiffuseScale * saturate(fDistUnderwater);

	vDiffuse				= lerp(vRefractColour * vExtinction + vInScattering, mtlData.m_vAlbedo * vDiffuse, mtlData.m_fAlpha * saturate(fDistUnderwater * 16.0f));
	mtlData.m_fAlpha		= 1.0h;

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
