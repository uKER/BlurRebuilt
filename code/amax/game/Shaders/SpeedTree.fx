#include <Core.h>
#include <Globals.h>

#define NO_LIGHTMAP

#define ENABLE_ALBEDOMAP
#define ENABLE_ALPHA_ENHANCE
#define ENABLE_MIPMAP_ALPHA_ENHANCE
#define ALPHA_TO_COVERAGE
#define ENABLE_WIND

#include <Common/Defines.h>
#include <Common/Parameters.h>
#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// vertex shader

#include <Common/Vertex.h>

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
