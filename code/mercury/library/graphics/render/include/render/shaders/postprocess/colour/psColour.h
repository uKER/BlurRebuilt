#include "../../psColour.h"

real3 ApplyVignette(real3 colour, float2 uv, real3 noise, real edgeBrightness)
{
	float2 uvOffset = uv - float2(0.5f, 0.5f);
	float dist = dot(uvOffset, uvOffset) * 2.0f;
	colour += noise;
	{
		float3 middleColour = colour;
		float3 edgeColour = colour * edgeBrightness;
		
		colour = lerp(middleColour, edgeColour, dist);
	}
	return colour;
}

float3 ApplyVignetteWeight(float3 colour, float2 uv, float3 noise, float edgeBrightness, float weight)
{
	float2 uvOffset = uv - float2(0.5f, 0.5f);
	float dist = dot(uvOffset, uvOffset) * 2.0f * weight; // times 2 to bring into 0->1 range
	colour += noise;
	{
		float3 middleColour = colour;
		float3 edgeColour = colour * edgeBrightness;
		
		colour = lerp( middleColour, edgeColour, dist );			
	}	
	
	return colour;
}

float3 ApplyRootVignetteWeight(float3 colour, float2 uv, float3 noise, float edgeBrightness, float weight)
{
	float2 uvOffset = uv - float2(0.5f, 0.5f);
	float dist = dot(uvOffset, uvOffset) * 2.0f * weight; // times 2 to bring into 0->1 range
	colour += noise;
	{
		float3 middleColour = colour;
		float3 edgeColour = colour * edgeBrightness;
		
		colour = lerp( middleColour, edgeColour, sqrt(dist) );			
	}	
	
	return colour;
}

float3 ApplyVignette(float3 colour, float2 uv, float3 noise, float edgeBrightness, float edgeSaturation)
{
	float2 uvOffset = uv - float2(0.5f, 0.5f);
	float3 dist = dot(uvOffset, uvOffset) * 2.0f; // times 2 to bring into 0->1 range
	float3 invDist = 1.0f - dist;

	colour += noise;

	{
		float3 middleColour = colour;
		float3 edgeColour = colour * edgeBrightness;
		
		colour.r = lerp( middleColour.r, edgeColour.r, dist.x );
		colour.g = lerp( middleColour.g, edgeColour.g, dist.y );
		colour.b = lerp( middleColour.b, edgeColour.b, dist.z );
	}	
		
	{
		float3 edgeColour = colour;
		
		// saturate edges
		float lum = Greyscale(colour);
		edgeColour = lum + (edgeColour - lum) * edgeSaturation;
		colour.r = lerp( edgeColour.r, colour.r, invDist.x );
		colour.g = lerp( edgeColour.g, colour.g, invDist.y );
		colour.b = lerp( edgeColour.b, colour.b, invDist.z );
	}

	return colour;
}

float3 ApplyVignette(float3 colour, float2 uv, float noise, float edgeBrightness, float edgeSaturation, float middleSaturation)
{
	float2 uvOffset = uv - float2(0.5f, 0.5f);
	float dist = dot(uvOffset, uvOffset) * 2.0f; // times 2 to bring into 0->1 range
	float invDist = 1.0f - dist;

	{
		float3 middleColour = colour;
		float3 edgeColour = colour * edgeBrightness;
		
		colour = lerp( middleColour, edgeColour, dist );			
	}	
		
	{
		float3 middleColour = colour;
		float3 edgeColour = colour;
		
		// saturate edges
		float lum = Greyscale(colour);
		edgeColour = lum + (edgeColour - lum) * edgeSaturation;
		
		// saturate middle
		middleColour = lum + (middleColour - lum) * middleSaturation;
		colour = lerp( edgeColour, middleColour, invDist );			
	}

	return colour;
}

float GetVignetteShape(float2 uv, float3 shape)
{
	float2 uvOffset = uv - float2(0.5f, 0.5f);
	float dist = dot(uvOffset, uvOffset) * 2.0f; // times 2 to bring into 0->1 range
	float invDist = 1.0f - dist;

	return saturate( shape.z * (pow(invDist, shape.x) - shape.y) );
}

float GetVignetteShape(float2 uv, float3 shape, float noise)
{
	float2 uvOffset = uv - float2(0.5f, 0.5f);
	float dist = dot(uvOffset, uvOffset) * 2.0f; // times 2 to bring into 0->1 range
	float invDist = 1.0f - dist * (1.0f - noise);

	return saturate( shape.z * (pow(invDist, shape.x) - shape.y) );
}
