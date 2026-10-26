#include "dynamic\shaders\vsCommon.h"
#include "dynamic\shaders\constants.h"
#include "render\shaders\vsPointLights.h"
#include "Particle.h"

float4	 g_vClipVector			: register(CCV_DISPLAY_CLIP_VECTOR);
float4x4 g_mWorld				: register(CCV_DISPLAY_WORLD_MATRIX);
float4x4 g_mWorldViewProj		: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
float4x4 g_mWorldView			: register(CCV_DISPLAY_VIEW_MATRIX);
float4	 g_vViewPosition		: register(CCV_DISPLAY_VIEW_POSITION);
float4	 g_vLightDirection		: register(CCV_RENDER_LIGHT_DIRECTION);

float4	 g_vUvScale				: register(CCV_RENDER_SETTINGS);
float4	 g_vLodFade				: register(CCV_RENDER_SETTINGS_1);

struct VS_INPUT
{
	float3 position : POSITION0;
	float4 colour : TEXCOORD0;
	
#ifdef UVW
	float3 uv : TEXCOORD1;
#else
	#ifdef NORMALMAP
		float3 uv : TEXCOORD1;
	#else
		#ifdef SOFT
			float4 uv : TEXCOORD1;
		#else
			float2 uv : TEXCOORD1;
		#endif
	#endif
#endif

#ifdef NDOTL
	float3 normal	: NORMAL0;
#endif

#ifdef DECAL
	float3 normal	: NORMAL0;
#endif

};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	
	float4 position = float4(input.position, 1.0f);
	output.position = 0;
	
	// transformation
	{
		#ifdef TWOD
		output.position = float4((position.xy * g_vClipVector.xy) + g_vClipVector.zw, 0.f, 1.f);
		#else
		output.position = mul(position, g_mWorldViewProj);
		#endif

		#ifdef POINTSPRITE
		output.position = mul(position, g_mWorldViewProj);
		output.pointsize = (input.uv.x * g_vUvScale.x) / output.position.w;
		#endif
	
		#ifdef DEPTH
		output.positionUV = output.position * float4(0.5f, -0.5f, 1.0f, 1.0f) + float4(0.5f, 0.5f, 0.0f, 0.0f) * output.position.w;
		#endif
	}
	
	// uvs
	{
		#ifndef POINTSPRITE
		output.uv = float4(input.uv.xy, 1.0f, 1.0f);
			
		#ifdef UVW
		output.uv = input.uv.xyzx;
		#endif
		
		#ifdef SOFT
		output.uv = input.uv;
		#endif
		
		// decompress UVs and apply blend scale
		output.uv *= g_vUvScale;
		#else
		output.uv = float4(0.0f, 0.0f, g_vUvScale.z, 1.0f);
		#endif
	}

	// colour and lighting
	{
		output.colour = input.colour;
	
		#ifdef POINTLIGHTS
		float4 lightColour = float4(PointLight4(position.xyz), 0.0f);
		output.colour += lightColour;
		#endif
	
		#ifdef NDOTL
		output.colour.rgb *= saturate(dot(input.normal, g_vLightDirection.xyz)) * 0.75f + 0.25f;
		#endif
	}
	
	// normalmapping
	{
		#ifdef NORMALMAP
		float sinTheta;
		float cosTheta;
		sincos(input.uv.z, sinTheta, cosTheta);
		
		float3 vRight = float3(g_mWorldView._11, g_mWorldView._21, g_mWorldView._31);
		float3 vUp = float3(g_mWorldView._12, g_mWorldView._22, g_mWorldView._32);
		float3 vNormal = -float3(g_mWorldView._13, g_mWorldView._23, g_mWorldView._33);
					
		float3 vTangent = vRight * cosTheta + vUp * -sinTheta;
		float3 vBinormal = vRight * sinTheta + vUp * cosTheta;
		
		float3x3 mTrans = float3x3(vTangent.xyz, vBinormal.xyz, vNormal.xyz);
		
		output.light = mul(mTrans, g_vLightDirection.xyz);
		#endif
	}
		
	// fog
	float4 worldPos = position;
	float3 posOffset = worldPos.xyz - g_vViewPosition.xyz; 
	output.worldPositionAndOffset = float4(worldPos.xyz, sqrt(dot(posOffset, posOffset)));

	float3 vForward = float3(g_mWorldView._13, g_mWorldView._23, g_mWorldView._33);
	float alphaScale = saturate(g_vLodFade.x * dot(posOffset, vForward) + g_vLodFade.y);
	output.colour.a *= alphaScale;
		
    return output;
}




