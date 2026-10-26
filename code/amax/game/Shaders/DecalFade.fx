#include <Core.h>
#include <Globals.h>

#define REPLACE_MATERIAL

#define ENABLE_LIGHTMAP_OPTIMISATION
#define ENABLE_SPECULARMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#define ENABLE_ALBEDOMAP
#define ENABLE_MATERIALMAP

#define DISABLE_LODFADE
#define DISABLE_VERTEXLIGHTING

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// replace material

C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData		= (C_Material) 0;

	float2	vTexCoord		= vtxData.m_vTexCoord.xy;

	mtlData	= EvaluateAlbedo(mtlData, vTexCoord);
	mtlData	= EvaluateTangentNormal(mtlData, vTexCoord);
	mtlData	= EvaluateMaterialParameters(mtlData, vtxData, vTexCoord);

#if defined(ENABLE_VERTEXCOLOURS)
	mtlData.m_fAlpha *= vtxData.m_vColour.a;
#endif

	return mtlData;
}

// vertex shader

#include <Common/Vertex.h>

// pixel shader

#include <Common/Pixel.h>

// technique

technique Material
{
	pass p0
	{
		AlphaBlendEnable = TRUE;
		AlphaTestEnable = TRUE;
		SrcBlend = One;
		DestBlend = InvSrcAlpha;
		ZWriteEnable = FALSE;
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

#if !defined(CFG_CONVERTER)
technique MaterialLights < string Define = "ENABLE_LIGHTING_PREPASS"; >
{
	pass p0
	{
		AlphaBlendEnable = TRUE;
		AlphaTestEnable = TRUE;
		SrcBlend = One;
		DestBlend = InvSrcAlpha;
		ZWriteEnable = FALSE;
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}
#endif
