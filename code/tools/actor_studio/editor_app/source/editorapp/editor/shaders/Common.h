#include "vsConstants.h"
#include "psConstants.h"

struct VS_INPUT
{
	float3 Position				: POSITION;
	float4 Colour				: COLOR0;

#ifdef SOLID
	float3 Normal				: NORMAL;
	float4 Texcoord0			: TEXCOORD0;
#endif

	float4 Texcoord1			: TEXCOORD1;
	float4 Texcoord2			: TEXCOORD2;
	float4 Texcoord3			: TEXCOORD3;
	float4 Texcoord4			: TEXCOORD4;
};


struct VS_OUTPUT
{
	float4 Position				: POSITION;
	float4 Colour0				: COLOR0;

#ifdef SOLID
	float4 Texcoord0			: TEXCOORD0;
	float4 Colour1				: COLOR1;
#endif

#ifdef DEPTHBIAS
	float Depth					: TEXCOORD1;
#endif
};


struct PS_OUTPUT
{
	float4 Colour				: COLOR0;
#ifdef DEPTHBIAS
	float Depth					: DEPTH;
#endif
};

