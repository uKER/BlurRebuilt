#include <Core.h>
#include <Globals.h>

#include <Common/Defines.h>
#include <Shader/Common/HDR.h>
#include <Shader/Misc/Resolve.h>

#include "Effects/LightFlareParameters.h"

struct C_VertexInput
{
	float3	m_vPosition	: POSITION annotation(xyzw(spriteposition) | format(F32_4));
	float4	m_vTexCoord	: TEXCOORD0 annotation(xy(uv:0) | zw(spriteoffset:0) | format(F16_4));
};

struct C_VertexOutput
{
	float4	m_vPosition	: POSITION;
	float2	m_vTexCoord	: TEXCOORD0;
	real3	m_vColour	: TEXCOORD1; //Colour
	#if defined(SOFT) && !defined(LOD) 
		float4	m_vScreen	: TEXCOORD2;
	#endif
};

struct C_PixelOutput
{
	float4	m_vColour	: COLOR0;	
};

C_VertexOutput LightFlareVS(in C_VertexInput input)
{
	C_VertexOutput output = (C_VertexOutput) 0;

    float3	vPosition	= input.m_vPosition.xyz;
	float2	vPivot		= input.m_vTexCoord.zw;

#if !defined(CFG_CONVERTER)
	vPosition.xy -= vPivot.xy;

	float4	vWorldPosition	= mul(float4(vPosition, 1.0f), g_mWorld);
	
	float3  view			= (g_vViewPosition.xyz - vWorldPosition.xyz);
	float distant			= length(view);
	view = normalize(view);

#if !defined(LOD)
	//Position in screen
	float4 screenPos = mul(vWorldPosition, g_mViewProj);

	float screenPosX = screenPos.x/screenPos.w;
	float screenPosY = screenPos.y/screenPos.w;

	float centerPos = saturate(screenPosX * screenPosX + screenPosY * screenPosY);

	float2 originalPivot = vPivot;
	float angle = (screenPosX + screenPosY);
	vPivot.x = cos(angle) * originalPivot.x + sin(angle) * originalPivot.y;
	vPivot.y = -sin(angle) * originalPivot.x + cos(angle) * originalPivot.y;
#else
	float centerPos = 0.f;
#endif

#else
	float centerPos = 0.f;
	float4	vWorldPosition	= mul(float4(vPosition, 1.0f), g_mWorld);

#endif

	float fade	= max(0.f, 1.f + distant * g_vNearFarLightFlareColour.w) * ((1.f - saturate((distant - g_vNearFarLightFlareColour.x) / (g_vNearFarLightFlareColour.y - g_vNearFarLightFlareColour.x))) * lerp(1.f, g_fLightFlareBorderIntensityBias, centerPos));

#if defined(CAMERAFADE)
	fade *= dot(g_mWorld[2].xyz, view); 
#endif

#if defined(FLASHING)
	float	fFlash	= 0.5f + 0.5f * cos(6.2831853f * g_fFrequency * g_fTime);
	fFlash	= smoothstep(g_fThresholdOff, g_fThresholdOn, fFlash);
	fade *= fFlash;
#endif

	float3 flareColour = g_vLightFlareColour * g_fLightFlareIntensity * g_vGlobalLightFlareColour * fade;
	//Remove pivot in back faced (degenerated flare)
	vPivot = (dot(flareColour,flareColour) <= 0.01f)?float2(0.f, 0.f):vPivot;

	// grab vectors from view matrix
	//float3		vRight		= float3(g_mView._11, g_mView._21, g_mView._31);
	//float3		vUp			= float3(g_mView._12, g_mView._22, g_mView._32);
	float3			vRight		= normalize(cross(view, float3(0.f, 1.f, 0.f)));
	float3			vUp			= normalize(cross(vRight, view ));
	//float3		vForward	= float3(g_mView._13, g_mView._23, g_mView._33);

#if !defined(CFG_CONVERTER)   
	vWorldPosition.xyz	+= vRight * vPivot.x * g_fGlobalLightFlareSizeX * g_fGlobalLightFlareSize * g_fLightFlareSize * lerp(1.f, g_fLightFlareBorderSizeBias, centerPos) * max(0.f, 1.f + distant * g_vNearFarLightFlareColour.z);
	vWorldPosition.xyz	+= vUp * vPivot.y * g_fGlobalLightFlareSizeY * g_fGlobalLightFlareSize * g_fLightFlareSize * lerp(1.f, g_fLightFlareBorderSizeBias, centerPos)  * max(0.f, 1.f + distant * g_vNearFarLightFlareColour.z);
	vWorldPosition.xyz	+= view * g_fLightFlareDepthBias;
#endif
	
	output.m_vPosition			= mul(vWorldPosition, g_mViewProj);
	output.m_vTexCoord.xy		= input.m_vTexCoord.xy;	
	output.m_vColour.rgb		= flareColour;

#if defined(SOFT) && !defined(LOD)
	output.m_vScreen.xy			= output.m_vPosition.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f) * output.m_vPosition.w;
	output.m_vScreen.zw			= output.m_vPosition.zw;
#endif

    return output;
}

// pixel shader

C_PixelOutput LightFlarePS(in C_VertexOutput vtxData)
{
	C_PixelOutput output = (C_PixelOutput) 0;
	
	half	fFadeFactor		= 1.0h;
#if defined(SOFT) && !defined(CFG_CONVERTER) && !defined(LOD)
	float3	vScreenPosition	= vtxData.m_vScreen.xyz / vtxData.m_vScreen.w;
	float	fScreenDepth	= SampleDepthBuffer(DepthBufferSampler, vScreenPosition.xy, g_vDepthScale);
	float	fEffectDepth	= vtxData.m_vScreen.w;
	fFadeFactor				= saturate(fScreenDepth - fEffectDepth) * (1.0h - vScreenPosition.z);
#endif

#if defined(ENVIRONMENT)
	half2	offset = vtxData.m_vTexCoord.xy * 2.f - 1.f;
#if defined(BLUR_DX11)
	// [Blur2PC][Blur1PCDX11][retail-recovered] Shipped flares saturate the radius.
	half2 radius = saturate(abs(offset.xy));
#else
	half2 radius = abs(offset.xy);
#endif
	half factor = pow(saturate(1.f - dot(radius.xy,radius.xy)), 6.f); 
	half4	vColour = factor;
#else
	half4	vColour = h4tex2D(ColourSampler, vtxData.m_vTexCoord.xy);
#endif
	vColour.rgb	*= vtxData.m_vColour;
	vColour.rgb *= fFadeFactor;
	vColour.rgb *= vColour.a;
	
	vColour.a = 1.0h / 255.0h * sign(vColour.a);
	
	output.m_vColour = vColour;
		
	return output;
}

// technique

technique Material < string SortType = "backfront"; >
{
	pass p0
	{
		AlphaBlendEnable = TRUE;
		AlphaTestEnable = TRUE;
		SrcBlend = One;
		DestBlend = InvSrcAlpha;
		VertexShader = compile vs_3_0 LightFlareVS();
		PixelShader = compile ps_3_0 LightFlarePS();
	}
}

technique MaterialLOD < string Define = "LOD"; string SortType = "backfront"; >
{
	pass p0
	{
		AlphaBlendEnable = TRUE;
		AlphaTestEnable = TRUE;
		SrcBlend = One;
		DestBlend = InvSrcAlpha;
		VertexShader = compile vs_3_0 LightFlareVS();
		PixelShader = compile ps_3_0 LightFlarePS();
	}
}

technique MaterialEnvironment < string Define = "ENVIRONMENT; LOD"; string SortType = "backfront"; >
{
	pass p0
	{
		AlphaBlendEnable = TRUE;
		AlphaTestEnable = TRUE;
		SrcBlend = One;
		DestBlend = InvSrcAlpha;
		VertexShader = compile vs_3_0 LightFlareVS();
		PixelShader = compile ps_3_0 LightFlarePS();
	}
}
