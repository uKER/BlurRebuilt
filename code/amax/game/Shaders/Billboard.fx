#include <Core.h>
#include <Globals.h>

#define REPLACE_VERTEXSHADER_INPUT

#define NO_LIGHTMAP

#define ENABLE_ALBEDOMAP

#define VS_POSITION_ANNOTATION annotation(xyzw(spriteposition) | format(F32_4))

// foliage defines
#define ENABLE_ALPHA_ENHANCE
#define ENABLE_MIPMAP_ALPHA_ENHANCE
#define ALPHA_TO_COVERAGE
#define REDUCE_DYNAMIC_LIGHTING

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


shared float3 g_vHeroCarPos : register(CCV_SHADER_USER_1);

#ifdef CFG_DEBUG
	float		g_fGrassFactor		: register(CCV_SHADER_USER_2) < string DebugVariable = "GrassFactor"; string Range = "(0,4)"; string Type = "Float"; > = 2.f;
	float		g_fGrassOffset		: register(CCV_SHADER_USER_3) < string DebugVariable = "GrassOffset"; string Range = "(0,1)"; string Type = "Float"; > = 0.3f;
#else
	const real		g_fGrassFactor = 2.h;
	const real		g_fGrassOffset = 0.3h;
#endif

C_VertexOutput MainSpriteVS(in C_VertexInput input)
{
#if !defined(CFG_CONVERTER)
	input.m_vPosition.x -= input.m_vPivot.x;
#endif	
	
	C_VertexData vtxData = (C_VertexData) 0;
	vtxData	= EvaluateBindPosition(vtxData, input);
	vtxData	= EvaluateObjectPosition(vtxData, input);
	// Crunch the grass down.
	float factor = (input.m_vPivot.y * 0.5f + 0.5f) * saturate(input.m_vPivot.y * g_fGrassFactor - g_fGrassOffset);
	vtxData	= EvaluateWorldPosition(vtxData, input);
	float distance = length(g_vHeroCarPos.xyz - vtxData.m_vPosition.xyz);
		  distance = saturate(3.0f - distance);	
	vtxData.m_vPosition.xz += factor * distance * normalize(vtxData.m_vPosition.xyz - g_vHeroCarPos.xyz);
	vtxData.m_vPosition.y  -= factor * distance;


#if !defined(CFG_CONVERTER)
	float3	vView		= g_vViewPosition.xyz - vtxData.m_vPosition.xyz;
	float3	vRight		= normalize(float3(-vView.z, 0.0f, vView.x));
	float3	vForward	= normalize(float3(vView.x, 0.0f, vView.z));
	vtxData.m_vPosition.xyz	+= vRight * input.m_vPivot.x;
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

	// Crunch the grass down.
	float factor = (input.m_vPivot.y * 0.5f + 0.5f) * saturate(input.m_vPivot.y * g_fGrassFactor - g_fGrassOffset);
	vtxData	= EvaluateWorldPosition(vtxData, input);
	float distance = length(g_vHeroCarPos.xyz - vtxData.m_vPosition.xyz);
		  distance = saturate(3.0f - distance);	
	vtxData.m_vPosition.xz += factor * distance * normalize(vtxData.m_vPosition.xyz - g_vHeroCarPos.xyz);
	vtxData.m_vPosition.y  -= factor * distance;


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
#if !defined(CFG_CONVERTER)
    float fLodFade = EvaluateLodFadeNoSaturate(output.m_vPosition.w);
    float fLodSelect = 1.0f - step(fLodFade, 0.0f);
    output.m_vPosition *= fLodSelect;
    output.m_vNormal.a = fLodFade;
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

#if defined(BLUR_DX11)
// [Blur2PC][Blur1PCDX11][retail-recovered] Loaded PC foliage draw 7047732
// leaves depth z/w at zero. D3D9's zero multiply yields zero after rcp(0),
// whereas SM5 yields NaN. Preserve the measured PC result for this path.
#define RETAIL_ZERO_BILLBOARD_PREPASS_DEPTH
#endif
#include <Common/Pixel.h>

// technique

#define FLAG_EXCLUDE_FROM_CUBEMAP_MIRROR_REFLECTION_PREPASS 15

#ifdef SHOW_IN_CUBEMAP_REFLECTION_MIRROR
technique Material
#else
technique Material < int Flags = FLAG_EXCLUDE_FROM_CUBEMAP_MIRROR_REFLECTION_PREPASS; >
#endif
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
