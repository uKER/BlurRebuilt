#include <fx\shaders\core.h>
#include <fx\shaders\global.h>

// shared variables can't have default values on them - Xenon ignores them and PS3 will break
texture g_MeshTexture	  < string source = "Diffuse"; bool AlphaDetect=true; >;				// Color texture for mesh
texture g_LightmapTexture < string source = "<Lightmap>"; >;			// Lightmap
texture g_UnusedTexture   < string source = "SetThisIfYouLike"; >;

float4 TestPixelShaderConstantA < 	string source = "MyShaderConstantA"; > = { 0, 1, 0, 0 };
float4 TestPixelShaderConstantB < 	string source = "HelloHello"; > = { 0, 1, 0, 0 };

float3 gvLightDirection = {0.7,0.7,-0.7};

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
	float4 vTexCoord : TEXCOORD0 annotation(xy(uv:0) | zw(uv:luxlightmap) | format(F16_4));
	float4 vColour : COLOR0;
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
	
	position = mul(position, g_WorldMatrix);
	position = mul(position, g_ViewProjectionMatrix);

	Output.Position = position;
    
//	Output.Diffuse.rgb = input.vColour;
//	Output.Diffuse.a = 1.0f;
	
	Output.Diffuse.xyz = saturate(dot(input.vNormal, gvLightDirection)) + float3(0.3, 0.3, 0.3);
	Output.Diffuse.w = 1;
	
	Output.TextureUV0 = input.vTexCoord; 

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

	Output.RGBColor = tex2D(MeshTextureSampler, In.TextureUV0.xy) * In.Diffuse;

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
