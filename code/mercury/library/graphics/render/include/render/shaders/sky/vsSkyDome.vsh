#include "..\\vsConstants.h"
#include "..\\vsCommon.h"

float4		g_SkySettings			: register(CCV_RENDER_SETTINGS);
float4		g_SkySettings2			: register(CCV_RENDER_SETTINGS_1);
float4		g_FogSettings			: register(CCV_RENDER_SETTINGS_3);
float4x4	g_WorldMatrix			: register(CCV_DISPLAY_WORLD_MATRIX);
float4x4	g_ViewProjMatrix		: register(CCV_DISPLAY_VIEWPROJ_MATRIX);

#define		g_SkySize				g_SkySettings.x
#define		g_SkyHeight				g_SkySettings.y
#define		g_SkyOffset				g_SkySettings.z
#define		g_SkyRotation			g_SkySettings2.x
#define		g_SkyIntensity			g_SkySettings2.y
#define		g_SkyBlackHeight		g_SkySettings2.z
#define		g_SkyBlackSize			g_SkySettings2.w

#define		g_FogAlpha				g_FogSettings.x
#define		g_FogTop				g_FogSettings.y
#define		g_FogPower				g_FogSettings.z

struct VS_INPUT
{
	float3 Position					: POSITION;
	float2 TexCoord0				: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 Position					: POSITION;
	float4 TexCoord0				: TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;
	
	float4 vObjectPosition = float4(IN.Position, 1.f);
	vObjectPosition.xyz *= g_SkySize;
    vObjectPosition.y *= g_SkyHeight;
    vObjectPosition.y += g_SkySize * g_SkyOffset;
	float4 vWorldPosition = mul(vObjectPosition, g_WorldMatrix);
	OUT.Position = mul(vWorldPosition, g_ViewProjMatrix);
	OUT.TexCoord0.xy = IN.TexCoord0;
	OUT.TexCoord0.x += g_SkyRotation;
	float fFogIntensity = saturate((OUT.TexCoord0.y + g_FogTop - 1.0f) / g_FogTop);	
	fFogIntensity = pow(fFogIntensity, g_FogPower);
	fFogIntensity = fFogIntensity * g_FogAlpha;
	OUT.TexCoord0.z = fFogIntensity;
	float fBlackBlend = saturate( (g_SkyBlackHeight - OUT.TexCoord0.y) * g_SkyBlackSize );
	OUT.TexCoord0.w = fBlackBlend;
	return(OUT);
}