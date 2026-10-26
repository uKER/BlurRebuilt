#include "Effect/Shaders/Constants.h"
#include "..\vsCommon.h"

float4x4 g_mWorldViewProj		: register(CCV_DISPLAY_VIEWPROJ_MATRIX);

float4x4 g_mWorldViewProjOrigin		: register(CCV_RENDER_VIEWPROJORIGIN_MATRIX);
float4x4 g_mWorldViewProjOriginPrev	: register(CCV_RENDER_VIEWPROJORIGINPREV_MATRIX);

float4	 g_vViewPosition		: register(CCV_DISPLAY_VIEW_POSITION);
float4	 g_vPositionOffset		: register(CCV_EFFECT_RAIN_POSITIONOFFSET);
float4   g_vVelocity			: register(CCV_EFFECT_RAIN_RAINVELOCITY);
float4   g_vColourScale			: register(CCV_EFFECT_RAIN_COLOURSCALE);
float4   g_vSizeScale			: register(CCV_EFFECT_RAIN_SIZESCALE);
float4	 g_vLighting			: register(CCV_EFFECT_RAIN_LIGHTING);

float4   g_vForwardVector		: register(CCV_EFFECT_RAIN_FORWARD);
float4   g_vForwardOffset		: register(CCV_EFFECT_RAIN_FORWARDOFFSET);

float4	 g_vFadeIn				: register(CCV_RENDER_FADEIN_SETTINGS);

struct VS_OUTPUT
{
    float4 position		: POSITION;		//Pixel position (world-view-proj space)
    float2 uv			: TEXCOORD0;
    float4 colour		: COLOR0;		//Diffuse texture
    
#ifdef OCCLUSION
	float3 worldPosition : TEXCOORD1;
#endif
};

struct VS_INPUT
{
	float3 position		: POSITION;
	float2 uv			: TEXCOORD0;
	#ifndef SIMPLE
	float2 scale		: TEXCOORD1;
	#endif
};

#define PARTICLE_BOX 30.0f

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	
	output.uv = input.uv;
	
	float3 position = input.position;
		
	// work out position of rain drop
	
	// subtract the animation, view position and forward offset fmoded into (-PARTICLE_BOX, PARTICLE_BOX)
	position.xyz -= g_vPositionOffset.xyz;
	position.xyz = fmod(position.xyz, PARTICLE_BOX);
	position.xyz -= (PARTICLE_BOX/2);
	position.xyz += g_vForwardVector.xyz;
	
	float4 worldPosition = float4(position.xyz + g_vViewPosition.xyz, 1.0f);
	
	output.colour = g_vLighting;
	
	#ifdef OCCLUSION
	output.worldPosition = worldPosition.xyz;
	#endif
	
	// calculate colour based on distance from origin
	float subtractColour = length(position.xyz) / PARTICLE_BOX;
	float distanceColourScale = 1.0f - subtractColour * subtractColour;
	float size = 0.5f - output.uv.x;
	
	#ifdef SIMPLE
	float4 worldPositionTop = float4(worldPosition.xyz + g_vVelocity.xyz * g_vSizeScale.y, 1.0f);
	#else
	float4 worldPositionTop = float4(worldPosition.xyz + g_vVelocity.xyz * g_vSizeScale.y * input.scale.x, 1.0f);
	#endif
	
	// projection method
	// project other end of drop to screen with previous matrix
	float4 start = mul(worldPosition, g_mWorldViewProjOrigin);		
	float4 end = mul(worldPositionTop, g_mWorldViewProjOrigin);
	float4 endPrev = mul(worldPositionTop, g_mWorldViewProjOriginPrev);
	
	// get a 2d vector in screenspace between start and end of raindrop
	float2 dir = (end.xy/end.w) - (start.xy/start.w);
	float2 dirPrev = (endPrev.xy/endPrev.w) - (start.xy/start.w);
	
	float len = length(dir);
	float lenPrev = length(dirPrev);
	
	// scale colour depending on length
	float lenColourScale = saturate(len/lenPrev);
	
	float2 swap;
	swap.x = -dirPrev.y;
	swap.y = dirPrev.x;
	dirPrev = normalize(swap.xy);

	output.position = lerp(endPrev, start, output.uv.y);
	#ifdef SIMPLE
		output.position.xy += size * dirPrev * g_vSizeScale.x;
		output.colour = output.colour * distanceColourScale * lenColourScale * g_vColourScale.x;
	#else
		output.position.xy += size * dirPrev * g_vSizeScale.x * input.scale.x;
		output.colour = output.colour * distanceColourScale * lenColourScale * g_vColourScale.x * input.scale.y;
	#endif
	
	output.colour *= saturate(length(position.xyz) * g_vFadeIn.x + g_vFadeIn.y);
	
	return output;
}


