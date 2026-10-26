#include "Effect/Shaders/Constants.h"
#include "../vsCommon.h"
#include "../../../../../render/include/render/shaders/psColour.h"

float4x4	g_mViewProj			: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
float4		g_vViewPosition		: register(CCV_DISPLAY_VIEW_POSITION);

float4		g_vSettings			: register(CCV_RENDER_SETTINGS);
float3		g_vA				: register(CCV_RENDER_SETTINGS_1);
float3		g_vB				: register(CCV_RENDER_SETTINGS_2);
float3		g_vC				: register(CCV_RENDER_SETTINGS_3);
float3		g_vD				: register(CCV_RENDER_SETTINGS_4);
float3		g_vE				: register(CCV_RENDER_SETTINGS_5);
float3		g_vSunDirection		: register(CCV_RENDER_SETTINGS_6);
float2		g_vSunTheta			: register(CCV_RENDER_SETTINGS_7);
float3		g_vZenithColour		: register(CCV_RENDER_SETTINGS_8);

#define		g_fSize				g_vSettings.x
#define		g_fHeight			g_vSettings.y
#define		g_fOffset			g_vSettings.z

struct VS_INPUT
{
	float3	m_Position		: POSITION;
	float2	m_TexCoord		: TEXCOORD0;
};

struct VS_OUTPUT
{
	#if defined(BLUR_DX11) // [Blur1PCDX11] D3D11 links stages by register: interpolants from zero, SV_Position last.

	float4	m_Colour		: TEXCOORD0;
	float4	m_Position		: SV_Position;
	#else
	float4	m_Position		: POSITION;
	float4	m_Colour		: TEXCOORD0;
	#endif
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	
	float3	vObjectPosition	= input.m_Position;
	vObjectPosition.xyz	*= g_fSize;
	vObjectPosition.y	*= g_fHeight;
	vObjectPosition.y	+= g_fSize * g_fOffset;

	float4	vWorldPosition = float4(vObjectPosition.xyz + g_vViewPosition.xyz, 1.0f);
	output.m_Position	= mul(vWorldPosition, g_mViewProj);
	
	float3	vDirection	= normalize(input.m_Position.xyz);
	float3	vZenith		= float3(0.0f, 1.0f, 0.0f);

	vDirection.y = abs(vDirection.y);
	
	float	fCosTheta	= dot(vZenith, vDirection);
	float	fCosGamma	= dot(g_vSunDirection, vDirection);
	float	fCos2Gamma	= fCosGamma * fCosGamma;
	float	fGamma		= acos(fCosGamma);
	
	float3	vNum		= (1.0f + g_vA * exp(g_vB / fCosTheta)) * (1.0f + g_vC * exp(g_vD * fGamma) + g_vE * fCos2Gamma);
	float3	vDen		= (1.0f + g_vA * exp(g_vB)) * (1.0f + g_vC * exp(g_vD * g_vSunTheta.x) + g_vE * g_vSunTheta.y);
	float3	xyY			= vNum / vDen * g_vZenithColour;
	
	output.m_Colour.rgb	= xyYToRGB(xyY);
	output.m_Colour.a	= 1.0f;
	
	return output;
}
