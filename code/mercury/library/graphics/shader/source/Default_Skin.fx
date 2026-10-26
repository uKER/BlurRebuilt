#ifndef NO_LIGHTMAP
#define NO_LIGHTMAP
#endif

#include <Core.h>
#include <Globals.h>

#define ENABLE_LAMBERT_SHADER

#define ENABLE_BUMPMAP_OPTIMISATION
#define ENABLE_EMISSIVE_OPTIMISATION
#define ENABLE_LIGHTMAP_OPTIMISATION
#define ENABLE_SPECULARMAP_OPTIMISATION
#define ENABLE_VERTEXCOLOURS_OPTIMISATION

#define ENABLE_BUMPMAP

#ifndef LOD
#define ENABLE_SPECULARMAP
#define ENABLE_EMISSIVE
#endif

#define ENABLE_SKINNING

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

/*technique MaterialLOD < string Define = "LOD"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}*/

#if PLATFORM_PS3

technique MaterialAlphaLuminance < string Define = "ALPHA_LUMINANCE"; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

#endif
