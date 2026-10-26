
struct VS_INPUT
{
	float3 Position				: POSITION;

#ifdef NORM
	float3 Normal				: NORMAL;
#endif

#ifdef COLOUR
	float4 Colour				: COLOR0;
#endif

#ifdef UV
	float2 Texcoord0			: TEXCOORD0;
	float2 Texcoord1			: TEXCOORD1;
#endif

#ifdef TANGENTS
	float3 Tangent				: TANGENT;
	float3 Binormal				: BINORMAL;
#endif

#ifdef SKINNED
	float4 Blendweight			: BLENDWEIGHT;
	int4 Blendindices			: BLENDINDICES;
#endif
};


struct VS_OUTPUT
{
	float4 Position				: POSITION;

	float4 Colour				: COLOR0;

	float4 Texcoord0			: TEXCOORD0;
	float4 Texcoord1			: TEXCOORD1;	//dot (world normal, view direction)
	float4 Texcoord2			: TEXCOORD2;	//world normal
};


struct PS_OUTPUT
{
	float4 Colour				: COLOR0;	
};

