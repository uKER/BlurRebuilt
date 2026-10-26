#include <Core.h>
#include <Globals.h>

texture g_MeshTexture	  < string source = "<InvalidTexture>"; >;

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------
sampler MeshTextureSampler = 
sampler_state
{
    Texture = <g_MeshTexture>;
    MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

//--------------------------------------------------------------------------------------
// Vertex shader input structure
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 vPos : POSITION;
	float3 vNormal : NORMAL;
};

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Position   : POSITION;	// vertex position 
    float4 Diffuse    : COLOR0;		// vertex diffuse color (note that COLOR0 is clamped from 0..1)
	float4 TextureUV0  : TEXCOORD0;	// vertex texture coords 
};


//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderSceneVS( VS_INPUT input )
{
    VS_OUTPUT Output;
  
    // Transform the position from object space to homogeneous projection space
	float4 position = float4(input.vPos, 1.f);
	
	position = mul(position, g_mWorld);
	position = mul(position, g_mViewProj);

	Output.Position = position;
    
	Output.Diffuse = 1;
	
	Output.TextureUV0 = position;

    return Output;    
}

//--------------------------------------------------------------------------------------
// Pixel shader output structure
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
    float4 RGBColor : COLOR0;  // Pixel color    
};

//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
//       color with diffuse material color
//--------------------------------------------------------------------------------------
PS_OUTPUT RenderScenePS( VS_OUTPUT In ) 
{ 
    PS_OUTPUT Output;

	Output.RGBColor = tex2D(MeshTextureSampler, In.TextureUV0.xy);
					  
    return Output;
}

//--------------------------------------------------------------------------------------
// Renders scene to render target
//--------------------------------------------------------------------------------------


technique Textured <
	string Script = "Pass=p0;";
> {
    pass p0 <
	string Script = "Draw=geometry;";
> {		
		VertexShader = compile vs_2_0 RenderSceneVS();
		ZEnable = true;
		ZWriteEnable = true;
		CullMode = None;
		PixelShader = compile ps_2_0 RenderScenePS();
    }
}
