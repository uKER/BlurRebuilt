
struct VS_OUTPUT
{
	float4 position			: POSITION0;

#ifdef POINTSPRITE
	float  pointsize		: PSIZE;
	
		float4 psuv			: TEXCOORD0;
		float4 uv			: TEXCOORD1;
#else
	float4 uv : TEXCOORD0;
#endif

#ifdef NORMALMAP
	float3 light			: TEXCOORD1;
#endif

#ifdef REFRACT
	float4 screenUV			: TEXCOORD1;
#endif

#ifdef DEPTH
	float4 positionUV		: TEXCOORD1;
#endif

#if defined(BLUR_DX11) && !defined(POINTSPRITE) && !defined(NORMALMAP) && !defined(REFRACT) && !defined(DEPTH)
	// [Blur1PCDX11] Retail also pairs the soft VS with the ordinary PS. Keep
	// its TEXCOORD1 register so TEXCOORD2/3 link without enabling soft depth.
	float4 unusedPositionUV : TEXCOORD1;
#endif

	float4 worldPositionAndOffset	: TEXCOORD2;
	float4 colour					: TEXCOORD3;
};




