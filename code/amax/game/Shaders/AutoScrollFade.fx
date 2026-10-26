#include <Core.h>
#include <Globals.h>

#define ENABLE_LIGHTMAP_OPTIMISATION

#define ENABLE_ALBEDOMAP
#define ENABLE_MATERIALMAP
#define ENABLE_VERTEXCOLOURS

#define DISABLE_LODFADE
#define DISABLE_VERTEXLIGHTING

#define REPLACE_MATERIAL

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// vertex shader

#include <Common/Vertex.h>

float	g_fScrollSpeedX	< string Source = "ScrollSpeedX"; string Default = "0.0"; >	= 0.0f;
float	g_fScrollSpeedY	< string Source = "ScrollSpeedY"; string Default = "0.0"; >	= 0.0f;

C_VertexOutput ScrollFadeVS(in C_VertexInput input)
{
    C_VertexOutput output = MainVS(input);
    output.m_vTexCoord.xy += g_fTime * float2(g_fScrollSpeedX, g_fScrollSpeedY);
    return output;    
}

C_Material EvaluateMaterial(in C_VertexOutput vtxData)
{
	C_Material mtlData		= (C_Material) 0;

	float2	vTexCoord		= vtxData.m_vTexCoord.xy;

	mtlData	= EvaluateAlbedo(mtlData, vTexCoord);
	mtlData	= EvaluateTangentNormal(mtlData, vTexCoord);
	mtlData	= EvaluateMaterialParameters(mtlData, vtxData, vTexCoord);

	mtlData.m_fAlpha *= vtxData.m_vColour.a;

	return mtlData;
}

// pixel shader

#include <Common/Pixel.h>

// technique

technique Material < string SortType = "backfront"; >
{
	pass p0
	{
		AlphaBlendEnable = TRUE;
		AlphaTestEnable = TRUE;
		SrcBlend = One;
		DestBlend = InvSrcAlpha;
		VertexShader = compile vs_3_0 ScrollFadeVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}
