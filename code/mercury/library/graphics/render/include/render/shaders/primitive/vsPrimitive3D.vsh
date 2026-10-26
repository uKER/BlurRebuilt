#include "..\vsConstants.h"
#include "..\vsCommon.h"

float4x4 g_WorldMatrix		: register(CCV_DISPLAY_WORLD_MATRIX);
float4x4 g_ViewProjMatrix	: register(CCV_DISPLAY_VIEWPROJ_MATRIX);

struct VS_INPUT
{
	float3 Position			: POSITION;
	float4 Color0			: COLOR0;
	
#ifdef LIT
	float3 Normal			: TEXCOORD;
#endif
};

#ifdef LIT
float4 g_vLightDirection	: register(CCV_RENDER_LIGHT_DIRECTION);
#endif

struct VS_OUTPUT
{
	#if defined(BLUR_DX11) // [Blur1PCDX11] D3D11 links stages by register: interpolants from zero, SV_Position last.
	float4 Color0			: COLOR0;
	float4 Position			: SV_Position;
	#else
	float4 Position			: POSITION;
	float4 Color0			: COLOR0;
	#endif
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;
	
	float4 objectPosition = float4(IN.Position, 1.f);
	float4 worldPosition = mul(objectPosition, g_WorldMatrix);
	OUT.Position = mul(worldPosition, g_ViewProjMatrix);
	
#ifdef LIT
	float ndotl = saturate((dot(normalize(mul(IN.Normal.xyz, (float3x3)g_WorldMatrix)).xyz, g_vLightDirection.xyz) + 1.0f) * 0.5f);
	float4 lighting = float4(ndotl, ndotl, ndotl, 1.0f);
	OUT.Color0 = IN.Color0 * lighting;
#else
	OUT.Color0 = IN.Color0;
#endif
	
	return OUT;
}