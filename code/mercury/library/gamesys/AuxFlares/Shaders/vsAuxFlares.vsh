#include "../../graphics/display/include/display/shaders/vsCommon.h"
#include "../../graphics/display/include/display/shaders/vsConstants.h"

float4x4			g_mViewProj	: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
float4x4			g_mView		: register(CCV_DISPLAY_VIEW_MATRIX);
float4				g_vCameraPosition : register(CCV_DISPLAY_VIEW_POSITION);
#if PLATFORM_PS3
float4				g_UserClipPlane : register(CCV_DISPLAY_USERCLIPPLANE);
#endif

struct VS_INPUT
{
	float3	m_Position	: POSITION;
	float4	m_Colour	: TEXCOORD0;  //Colour-> RGB, Size->A
	float2	m_TexCoord	: TEXCOORD1;
	float3	m_Normal	: NORMAL;
};

struct VS_OUTPUT
{
	float4	m_Position	: POSITION;
	float2	m_TexCoord	: TEXCOORD0;
	float3	m_Colour	: TEXCOORD1;
#if PLATFORM_PS3
	float	m_UserClipPlane : CLP0;
#endif
};

float4		g_FlareSize			: register(c200);
float4		g_FlareColour		: register(c201);


VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	
	// grab vectors from view matrix
	float3		vRight			= float3(g_mView._11, g_mView._21, g_mView._31);
	float3		vUp				= float3(g_mView._12, g_mView._22, g_mView._32);
	//float3		vForward		= float3(g_mView._13, g_mView._23, g_mView._33);
	
	//Pivot
	float2	vPivot			= input.m_TexCoord.xy;
	float3	view			= normalize(g_vCameraPosition.xyz - input.m_Position);
	float	fade			= dot(input.m_Normal, view); 
	
	//Remove pivot in back faced (degenerated flare)
	vPivot = (fade <= 0.f)?float2(0.f, 0.f):vPivot;
	
	//Calculate world position
	float4 vWorldPosition = float4(input.m_Position, 1.0f);
	
	vWorldPosition.xyz	+= vRight * vPivot.x * g_FlareSize.x * input.m_Colour.w;
	vWorldPosition.xyz	+= vUp * vPivot.y * g_FlareSize.y * input.m_Colour.w;
	vWorldPosition.xyz	+= view * 0.6f;
	
	//Calculate position
	float4  vProjPos = mul(vWorldPosition, g_mViewProj);
	output.m_Position = vProjPos;
#if PLATFORM_PS3
	output.m_UserClipPlane = dot(vWorldPosition, g_UserClipPlane);
#endif
	output.m_TexCoord = input.m_TexCoord * 0.5f + 0.5f;
	output.m_Colour = g_FlareColour.rgb * input.m_Colour.rgb * saturate(fade);
	
	return output;
}
