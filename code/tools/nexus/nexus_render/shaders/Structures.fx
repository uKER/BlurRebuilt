//--------------------------------------------------------------------------------------
// Vertex shader output structures
//--------------------------------------------------------------------------------------

struct POSCOLTEX1_VS_OUTPUT
{
    float4 Position   : POSITION;		// vertex position 
    float4 Diffuse    : COLOR0;			// vertex color (note that COLOR0 is clamped from 0..1)
    float2 TextureUV0  : TEXCOORD0;		// vertex texture coords 
};

struct POSCOLTEX2_VS_OUTPUT
{
    float4 Position   : POSITION;		// vertex position 
    float4 Diffuse    : COLOR0;			// vertex color (note that COLOR0 is clamped from 0..1)
    float2 TextureUV0  : TEXCOORD0;		// vertex texture coords 
    float2 TextureUV1  : TEXCOORD1;		// vertex texture coords 
};

struct POSCOL_VS_OUTPUT
{
    float4 Position   : POSITION;		// vertex position 
    float4 Diffuse    : COLOR0;			// vertex color (note that COLOR0 is clamped from 0..1)
};

//--------------------------------------------------------------------------------------
// Pixel shader output structure
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
    float4 RGBColor : COLOR0;  // Pixel color    
};