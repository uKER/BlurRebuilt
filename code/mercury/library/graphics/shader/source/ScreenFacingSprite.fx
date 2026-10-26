#include <Core.h>
#include <Globals.h>

#define REPLACE_VERTEXSHADER_INPUT

#define NO_LIGHTMAP

#define ENABLE_BUMPMAP_OPTIMISATION
#define ENABLE_BUMPMAP

#define ENABLE_MIPMAP_ALPHA_ENHANCE

#define VS_POSITION_ANNOTATION annotation(xyzw(spriteposition) | format(F32_4))

#include <Common/Defines.h>
#include <Common/Parameters.h>

struct C_VertexInput
{
	VS_INPUT_BASE
	float2	m_vPivot		: TEXCOORD5 annotation(xy(spriteoffset) | format(F16_2));
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
	vPosition.xy -= input.m_vPivot.xy;
#endif

	// grab vectors from view matrix
	float4x4 mWorldView = mul(g_mWorld, g_mView);
	float4 vRightVector = float4(mWorldView._11, mWorldView._21, mWorldView._31, 1.0f);
	float4 vUpVector = float4(mWorldView._12, mWorldView._22, mWorldView._32, 1.0f);
	
#ifndef MAYA    
	// make screen-facing
	vPosition.xyz += vRightVector.xyz * input.m_vPivot.x;
	vPosition.xyz += vUpVector.xyz * input.m_vPivot.y;
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
