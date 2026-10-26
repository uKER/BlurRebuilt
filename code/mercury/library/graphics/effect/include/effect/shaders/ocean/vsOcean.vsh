#define ANIMATION_HEIGHT_MAP
#define ANIMATION_HEIGHT_GRADIENT_MAP

#include "Effect/Shaders/Constants.h"
#include "..\vsCommon.h"
#include "vsOcean.h"

extern float4x4		g_mViewProj			: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
extern float4		g_vViewPosition		: register(CCV_DISPLAY_VIEW_POSITION);
extern float4		g_vGeometrySettings	: register(CCV_RENDER_GEOMETRY_SETTINGS);

struct VS_OUTPUT
{
	float4	m_Position	: POSITION;
	float4	m_TexCoord	: TEXCOORD0;
	float3	m_Normal	: TEXCOORD1;
	float3	m_Eye		: TEXCOORD2;
	float4	m_Screen	: TEXCOORD3;
};

struct VS_INPUT
{
	float4	m_Position	: POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	
	float4	vWorldPosition	= float4(input.m_Position.x * g_vGeometrySettings.w + g_vGeometrySettings.x, g_vGeometrySettings.y, input.m_Position.y * g_vGeometrySettings.w + g_vGeometrySettings.z, 1.0f);
	float2	vUV0			= vWorldPosition.xz * g_vHeightData.x;

	float3	vDisplacement, vNormal, vTangent, vBinormal;
	GetDisplacementAndNormal(vWorldPosition.xyz, vDisplacement, vNormal, vTangent, vBinormal);

	float	fDamping	= saturate(exp(-0.005f * mul(vWorldPosition, g_mViewProj).w));
	
	vWorldPosition.xyz	+= vDisplacement.xyz * fDamping;

	output.m_Position = mul(vWorldPosition, g_mViewProj);
	output.m_TexCoord = float4(vUV0, 0.0f, 0.0f);	
	output.m_Normal = vNormal;
	output.m_Eye = vWorldPosition.xyz - g_vViewPosition.xyz;
	output.m_Screen = output.m_Position;
	
	return output;
}
