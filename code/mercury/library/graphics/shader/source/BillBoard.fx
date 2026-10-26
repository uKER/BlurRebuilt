#include <Core.h>
#include <Globals.h>

#define REPLACE_VERTEXSHADER_INPUT

#define NO_LIGHTMAP

#define ENABLE_BUMPMAP_OPTIMISATION
#define ENABLE_BUMPMAP

#include <Common/Defines.h>
#include <Common/Parameters.h>

struct C_VertexInput
{
	VS_INPUT_BASE
	float2	m_vPivot		: TEXCOORD5 annotation(xy(spriteoffset:0) | format(F16_2));
};

#include <Common/Transform.h>
#include <Common/Material.h>
#include <Common/Surface.h>
#include <Common/Lighting.h>

// vertex shader

#include <Common/Vertex.h>

C_VertexOutput MainSpriteVS(C_VertexInput input)
{
    float4	vPosition	= float4(input.m_vPosition, 1.0f);

#ifndef MAYA    
	vPosition.x += input.m_vPivot.x;
	
	// Calculate right vector
	float3 viewVector	= g_vViewPosition.xyz - vPosition.xyz;
	float3 rightVector  = normalize(float3(-viewVector.z, 0, viewVector.x)); // cross product between up-vector and view-vector

	vPosition.xyz += rightVector * input.m_vPivot.x;
#endif

	input.m_vPosition = vPosition.xyz;

    return MainVS(input);    
}

// pixel shader

#include <Common/Pixel.h>

// technique

technique Material < bool PVSUsingOcclusionQuery = true; >
{
	pass p0
	{
		VertexShader = compile vs_3_0 MainSpriteVS();
		PixelShader = compile ps_3_0 MainPS();
	}
}

