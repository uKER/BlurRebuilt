#include <Core.h>
#include <Globals.h>

#define REPLACE_VERTEXSHADER_INPUT

#define NO_LIGHTMAP

#define ENABLE_ALBEDOMAP
#define ENABLE_MIPMAP_ALPHA_ENHANCE
#define VS_POSITION_ANNOTATION annotation(xyzw(spriteposition) | format(F32_4))

#include <Common/Defines.h>
#include <Common/Parameters.h>

struct C_VertexInput
{
	VS_INPUT_BASE
	float2	m_vPivot		: TEXCOORD3 annotation(xy(spriteoffset) | format(F16_2));
};

#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// vertex shader

C_VertexOutput MainVS(in C_VertexInput input)
{
#if !defined(CFG_CONVERTER)
	input.m_vPosition.xy -= input.m_vPivot.xy;
#endif	
	
	C_VertexData vtxData = (C_VertexData) 0;
	vtxData	= EvaluateBindPosition(vtxData, input);
	vtxData	= EvaluateObjectPosition(vtxData, input);
	vtxData	= EvaluateWorldPosition(vtxData, input);

#if !defined(CFG_CONVERTER)
	float3	vRight		= float3(g_mView._11, g_mView._21, g_mView._31);
	float3	vUp			= float3(g_mView._12, g_mView._22, g_mView._32);
	float3	vForward	= float3(g_mView._13, g_mView._23, g_mView._33);
	float3	vCorner		= vRight * input.m_vPivot.x + vUp * input.m_vPivot.y;
#if defined(ENABLE_WIND)
	vCorner = lerp(vCorner, mul(vCorner, g_mLeaf[input.m_vTexCoord.z]), input.m_vTexCoord.w);
#endif
	vtxData.m_vPosition.xyz	+= vCorner;
	vtxData.m_vNormal.xyz	= -vForward.xyz;
#endif

	C_VertexOutput output = (C_VertexOutput) 0;
	output	= EvaluatePosition(output, vtxData);
	output	= EvaluateTexCoord(output, input);
	output	= EvaluateColour(output, input, vtxData);

    return output;    
}

C_VertexPrepassOutput PrepassVS(in C_VertexInput input)
{
#if !defined(CFG_CONVERTER)
	input.m_vPosition.xy -= input.m_vPivot.xy;
#endif	
	
	C_VertexData vtxData = (C_VertexData) 0;
	vtxData	= EvaluateBindPosition(vtxData, input);
	vtxData	= EvaluateObjectPosition(vtxData, input);
	vtxData	= EvaluateWorldPosition(vtxData, input);

#if !defined(CFG_CONVERTER)
	float3	vRight		= float3(g_mView._11, g_mView._21, g_mView._31);
	float3	vUp			= float3(g_mView._12, g_mView._22, g_mView._32);
	float3	vForward	= float3(g_mView._13, g_mView._23, g_mView._33);
	float3	vCorner		= vRight * input.m_vPivot.x + vUp * input.m_vPivot.y;
#if defined(ENABLE_WIND) && !defined(LOD)
	vCorner = lerp(vCorner, mul(vCorner, g_mLeaf[input.m_vTexCoord.z]), input.m_vTexCoord.w);
#endif
	vtxData.m_vPosition.xyz	+= vCorner;
	vtxData.m_vNormal.xyz	= -vForward.xyz;
#endif

	C_VertexPrepassOutput output = (C_VertexPrepassOutput) 0;
	output.m_vPosition		= mul(vtxData.m_vPosition, g_mViewProj); 
#if defined(ENABLE_PREPASS_VIEWSPACE)
	output.m_vNormal.xyz	= float3(0.0f, 0.0f, -1.0f);
#else
	output.m_vNormal.xyz	= vtxData.m_vNormal.xyz;
#endif
#if !defined(CFG_CONVERTER)
#if defined(ENABLE_TANGENTNORMAL)
	output.m_vTangent		= vRight;
	output.m_vBinormal		= vUp;
#endif
#endif
	output.m_vTexCoord.xy	= input.m_vTexCoord.xy;
#if defined(ENABLE_PREPASS_VIEWDEPTH)
	output.m_vTexCoord.z	= output.m_vPosition.w / g_vNearFar.w;
#endif

    return output;
}

// pixel shader

#include <Common/Pixel.h>

// technique

technique Material
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

#if !defined(CFG_CONVERTER)
technique MaterialLOD < string Define = "LOD"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0  MainPS();
	}
}
#endif

#if !defined(CFG_CONVERTER)
technique MaterialLights < string Define = "ENABLE_LIGHTING_PREPASS"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}
#endif

#if !defined(CFG_CONVERTER)
technique MaterialPrepass < string Define = "ENABLE_PREPASS"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 PrepassVS();
		PixelShader = compile ps_3_0 PrepassPS();
	}
}
#endif
