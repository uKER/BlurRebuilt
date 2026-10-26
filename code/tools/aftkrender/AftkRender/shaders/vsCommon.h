#include "vsConstants.h"

#define PALETTE_SIZE 30

float4x4 g_WorldMatrix			: register(CCV_DISPLAY_WORLD_MATRIX);
float4x4 g_ViewProjMatrix		: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
float4x4 g_WorldInvTransMatrix  : register(CCV_AFTKRENDER_WORLD_INV_TRANS_MATRIX);

float3   g_WorldViewPosition	: register(CCV_DISPLAY_VIEW_POSITION);

float4x4 g_MatrixPalette[30]	: register(CCV_AFTKRENDER_MATRIX_PALETTE);


float4 SkinPosition(float4 position, float4x4 palette[PALETTE_SIZE],  int4 indices, float4 weights)
{
    int i;
    float4 skinnedPosition = 0;

    if(weights[0] > 0)
    {
		for(i=0; i<4; i++)
		{
			skinnedPosition  += mul( position, palette[indices[i]]) * weights[i];
        }
    }
    return skinnedPosition;
}


float3 SkinVector(float3 vec, float4x4 palette[PALETTE_SIZE], int4 indices, float4 weights)
{
    int i;
    float4 skinnedVector = 0;

    if(weights[0] > 0)
    {
		for(i=0; i<4; i++)
		{
			skinnedVector  += mul( vec, palette[indices[i]]) * weights[i];
        }
		skinnedVector = normalize(skinnedVector);
    }
    return skinnedVector;
}

float Lambert(float4 position, float3 normal, float4 lightPosition, out float4 lightVector)
{
	lightVector = normalize(lightPosition - position);
	return saturate( abs(dot(normal, lightVector)) );	//h4x0red abs() so that back faces are not black
}

float Specular( float3 normal, float3 halfAngle, float power, float gloss)
{
    float spec = 0;
    
    spec = saturate( dot(normal,halfAngle) );
    spec = pow( spec, power) * gloss;
    
    return spec;
}

float3 ToTangentSpace( float3 lightVector, float3 tangent, float3 binormal, float3 normal)
{
    float3   outVector;
    float3x3 tanSpace=float3x3(tangent,binormal,normal);

    outVector = mul( tanSpace,lightVector );
    return outVector;
}