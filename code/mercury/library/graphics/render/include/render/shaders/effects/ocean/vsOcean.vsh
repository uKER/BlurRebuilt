#include "..\..\vsConstants.h"
#include "..\..\vsCommon.h"
#include "OceanConstants.h"



struct C_Waves
{
	float4	m_Heights[2];
	float4	m_Frequencies[2];
	float4	m_DirectionsX[2];
	float4	m_DirectionsY[2];
	float4	m_Offsets[2];
	float4	m_Speeds[2];
	float4	m_Choppiness[2];
}; // size = 14



struct C_Position
{
	float3	m_BottomLeft;
	float3	m_TopLeft;
	float3	m_TopRight;
	float3	m_BottomRight;
}; // size = 4



extern float4x4		g_ViewProj :			register(CCV_DISPLAY_VIEWPROJ_MATRIX);
extern float4		g_ViewPosition :		register(CCV_DISPLAY_VIEW_POSITION);
extern float4x4		g_ProjWorld :			register(CCV_OCEAN_PROJWORLD_MATRIX);
extern float2		g_MinMaxZ :				register(CCV_OCEAN_MINMAXZ);
extern float		g_Time :				register(CCV_OCEAN_TIME);
extern float2		g_WorldToUV :			register(CCV_OCEAN_WORLDTOUV);
extern C_Waves		g_Waves :				register(CCV_OCEAN_WAVES);
extern C_Position	g_Position :			register(CCV_OCEAN_POSITION);
extern float4		g_Damping :				register(CCV_OCEAN_DAMPING);



struct VS_OUTPUT
{
	float4	m_Pos :				POSITION;
	float4	m_TexCoord :		TEXCOORD0;
	float3	m_View :			TEXCOORD1;
	float4	m_Normal :			TEXCOORD2;
	float3	m_Tangent :			TEXCOORD3;
	float3	m_Binormal :		TEXCOORD4;
};



struct VS_INPUT
{
	float4	m_Pos :			POSITION;
};



VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output	=	(VS_OUTPUT) 0;
	
	// calculate the projected position
	float4	bottomLeft	=	float4(g_Position.m_BottomLeft / g_Position.m_BottomLeft.z, 1.0f / g_Position.m_BottomLeft.z);
	float4	topLeft		=	float4(g_Position.m_TopLeft / g_Position.m_TopLeft.z, 1.0f / g_Position.m_TopLeft.z);
	float4	topRight	=	float4(g_Position.m_TopRight / g_Position.m_TopRight.z, 1.0f / g_Position.m_TopRight.z);
	float4	bottomRight	=	float4(g_Position.m_BottomRight / g_Position.m_BottomRight.z, 1.0f / g_Position.m_BottomRight.z);
	
	float4	leftPos		=	lerp(bottomLeft, topLeft, input.m_Pos.y);
	float4	rightPos	=	lerp(bottomRight, topRight, input.m_Pos.y);
	float4	projPos		=	lerp(leftPos, rightPos, input.m_Pos.x);
	
	float3	position	=	projPos.xyz / projPos.w;
	
	float	w			=	( (g_MinMaxZ.y - g_MinMaxZ.x) * position.z / g_MinMaxZ.y) + g_MinMaxZ.x;
	float4	worldPos	=	mul(float4(position, w), g_ProjWorld);

	float	damping		=	exp(-g_Damping.x * w * w);
	float4	dampedHeights[2];
	dampedHeights[0] = g_Waves.m_Heights[0] * damping;
	dampedHeights[1] = g_Waves.m_Heights[1] * damping;

	// waves
	float4	frequency0		=	g_Waves.m_DirectionsX[0] * worldPos.x + g_Waves.m_DirectionsY[0] * worldPos.z;
			frequency0		*=	g_Waves.m_Frequencies[0];
			frequency0		+=	g_Waves.m_Speeds[0] * g_Time + g_Waves.m_Offsets[0];
	float4	frequency1		=	g_Waves.m_DirectionsX[1] * worldPos.x + g_Waves.m_DirectionsY[1] * worldPos.z;
			frequency1		*=	g_Waves.m_Frequencies[1];
			frequency1		+=	g_Waves.m_Speeds[1] * g_Time + g_Waves.m_Offsets[1];
	float4	heights0, directions0, heights1, directions1;
	sincos(frequency0, heights0, directions0);
	sincos(frequency1, heights1, directions1);

	float	displacement		=	dot( dampedHeights[0], heights0 ) + dot( dampedHeights[1], heights1 );
	//float3 displacement;
	//displacement.x			=	dot( g_Waves.m_Choppiness[0] * dampedHeights[0] * g_Waves.m_DirectionsX[0], directions0 ) + dot( g_Waves.m_Choppiness[1] * dampedHeights[1] * g_Waves.m_DirectionsX[1], directions1 );
	//displacement.y			=	dot( dampedHeights[0], heights0 ) + dot( dampedHeights[1], heights1 );
	//displacement.z			=	dot( g_Waves.m_Choppiness[0] * dampedHeights[0] * g_Waves.m_DirectionsY[0], directions0 ) + dot( g_Waves.m_Choppiness[1] * dampedHeights[1] * g_Waves.m_DirectionsY[1], directions1 );
	
	float	maxHeight			=	dot( float4(1.0f, 1.0f, 1.0f, 1.0f), dampedHeights[0] ) + dot( float4(1.0f, 1.0f, 1.0f, 1.0f), dampedHeights[1] );

	//worldPos.xyz				+=	displacement;
	worldPos.y					+=	displacement;

	float4	normalSin0, normalCos0;
	float4	normalSin1, normalCos1;
	normalSin0				=	heights0;
	normalCos0				=	directions0;
	normalSin1				=	heights1;
	normalCos1				=	directions1;
		
	float3	normal;
	normal.x				=	-dot( g_Waves.m_DirectionsX[0] * dampedHeights[0] * g_Waves.m_Frequencies[0], normalCos0 ) - dot( g_Waves.m_DirectionsX[1] * dampedHeights[1] * g_Waves.m_Frequencies[1], normalCos1 );
	normal.y				=	1.0f - dot( g_Waves.m_Choppiness[0] * dampedHeights[0] * g_Waves.m_Frequencies[0], normalSin0 ) - dot( g_Waves.m_Choppiness[1] * dampedHeights[1] * g_Waves.m_Frequencies[1], normalSin1 );
	normal.z				=	-dot( g_Waves.m_DirectionsY[0] * dampedHeights[0] * g_Waves.m_Frequencies[0], normalCos0 ) - dot( g_Waves.m_DirectionsY[1] * dampedHeights[1] * g_Waves.m_Frequencies[1], normalCos1 );
	
	output.m_Pos			=	mul( worldPos, g_ViewProj );	
	output.m_TexCoord.xy	=	worldPos.xz * g_WorldToUV.x;
	output.m_TexCoord.zw	=	worldPos.xz * g_WorldToUV.y;
	output.m_Normal.xyz		=	normalize(normal);
	output.m_Normal.w		=	output.m_Pos.w;
	output.m_Tangent		=	normalize(cross(float3(1.0f, 0.0f, 0.0f), normal));
	output.m_Binormal		=	normalize(cross(normal, float3(0.0f, 0.0f, 1.0f)));
	output.m_View			=	normalize(worldPos.xyz - g_ViewPosition.xyz);
	
	return output;
}