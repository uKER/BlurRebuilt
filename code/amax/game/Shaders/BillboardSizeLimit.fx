#include <Core.h>
#include <Globals.h>

#define REPLACE_VERTEXSHADER_INPUT

#define NO_LIGHTMAP

#define ENABLE_ALBEDOMAP
#define ENABLE_VERTEXCOLOURS
#define DISABLE_VERTEXLIGHTING

#define VS_POSITION_ANNOTATION annotation(xyzw(spriteposition) | format(F32_4))

#include <Common/Defines.h>
#include <Common/Parameters.h>

struct C_VertexInput
{
	VS_INPUT_BASE
	float2	m_vPivot	: TEXCOORD3 annotation(xy(spriteoffset:0) | format(F16_2));
};

#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// vertex shader

#include <Common/Vertex.h>

float	g_fBeginDistance	< string Source = "BeginDistance"; string Default = "8.0"; string Type = "Float"; >	= 8.0f;
float	g_fEndDistance		< string Source = "EndDistance"; string Default = "4.0"; string Type = "Float"; >	= 4.0f;
float	g_fHeight			< string Source = "Height"; string Default = "1.0"; string Type = "Float"; >		= 1.0f;

C_VertexOutput MainSpriteVS(in C_VertexInput input)
{
#if !defined(CFG_CONVERTER)
	input.m_vPosition.x -= input.m_vPivot.x;
#endif	
	
	C_VertexData vtxData = (C_VertexData) 0;
	vtxData	= EvaluateBindPosition(vtxData, input);
	vtxData	= EvaluateObjectPosition(vtxData, input);
	vtxData	= EvaluateWorldPosition(vtxData, input);

#if !defined(CFG_CONVERTER)
	float3	vView		= g_vViewPosition.xyz - vtxData.m_vPosition.xyz;
	float3	vRight		= normalize(float3(-vView.z, 0.0f, vView.x));
	float3	vForward	= normalize(float3(vView.x, 0.0f, vView.z));
	vtxData.m_vPosition.xyz	+= vRight * input.m_vPivot.x;
	float	fDistance	= 1.0f - smoothstep(g_fEndDistance, g_fBeginDistance, length(vView));
	vtxData.m_vPosition.y	-= fDistance * g_fHeight * input.m_vColour.a;
	vtxData.m_vNormal.xyz	= vForward.xyz;
#endif

	C_VertexOutput output = (C_VertexOutput) 0;
	output	= EvaluatePosition(output, vtxData);
	output	= EvaluateTexCoord(output, input);
	output	= EvaluateColour(output, input, vtxData);

#if !defined(CFG_CONVERTER)
	float fLodFade = EvaluateLodFadeNoSaturate(output.m_vPosition.w);
	float fLodSelect = 1.0f - step(fLodFade, 0.0f);
	
	output.m_vPosition *= fLodSelect;
	output.m_vNormal.a = fLodFade;
#endif

    return output;
}

C_VertexPrepassOutput PrepassSpriteVS(in C_VertexInput input)
{
#if !defined(CFG_CONVERTER)
	input.m_vPosition.x -= input.m_vPivot.x;
#endif	
	
	C_VertexData vtxData = (C_VertexData) 0;
	vtxData	= EvaluateBindPosition(vtxData, input);
	vtxData	= EvaluateObjectPosition(vtxData, input);
	vtxData	= EvaluateWorldPosition(vtxData, input);

#if !defined(CFG_CONVERTER)
	float3	vView		= g_vViewPosition.xyz - vtxData.m_vPosition.xyz;
	float3	vRight		= normalize(float3(-vView.z, 0.0f, vView.x));
	float3	vForward	= normalize(float3(vView.x, 0.0f, vView.z));
	vtxData.m_vPosition.xyz	+= vRight * input.m_vPivot.x;
	vtxData.m_vNormal.xyz	= vForward.xyz;
#endif

	C_VertexPrepassOutput output = (C_VertexPrepassOutput) 0;
	output.m_vPosition		= mul(vtxData.m_vPosition, g_mViewProj); 
#if defined(ENABLE_PREPASS_VIEWSPACE)
	output.m_vNormal.xyz	= float3(0.0f, 0.0f, -1.0f);
#else
	output.m_vNormal.xyz	= vtxData.m_vNormal.xyz;
#endif
#if defined(ENABLE_TANGENTNORMAL)
	output.m_vTangent		= vRight;
	output.m_vBinormal		= cross(vRight, vForward);
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
		VertexShader = compile vs_3_0 MainSpriteVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

#if !defined(CFG_CONVERTER)
technique MaterialLights < string Define = "ENABLE_LIGHTING_PREPASS"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainSpriteVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}
#endif

#if !defined(CFG_CONVERTER)
technique MaterialPrepass < string Define = "ENABLE_PREPASS"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 PrepassSpriteVS();
		PixelShader = compile ps_3_0 PrepassPS();
	}
}
#endif
