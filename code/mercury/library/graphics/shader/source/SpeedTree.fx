#include <Core.h>
#include <Globals.h>

#define NO_LIGHTMAP

#define ENABLE_BUMPMAP_OPTIMISATION

#define ENABLE_BUMPMAP

#define ENABLE_MIPMAP_ALPHA_ENHANCE

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

technique Material < bool PVSUsingOcclusionQuery = true; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}
