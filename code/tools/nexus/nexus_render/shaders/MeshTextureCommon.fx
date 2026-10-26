//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

//Lighting direction
float3 g_lightDirection = {0.7,0.7,-0.7};

texture g_colourTexture;			// Color texture for mesh
texture g_cropTexture;				// Color texture for mesh

//UV offset information for the two UV sets that are supported.
float4 g_offsetWidthHeight;
float4 g_clamping;
uniform int g_rotatedCS = 0;

//General transforms
float4x4 g_mWorldTransposedInverse;
float4x4 g_mWorldViewProjection;    // World * View * Projection matrix
float4x4 g_mView;

//Bone transforms for deformed instances.
float4x4 g_mPalette[2];
float4x4 g_mPaletteTransposedInverse[2];

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------
sampler ColourTextureSampler = 
sampler_state
{
    Texture = <g_colourTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    
};

sampler CropTextureSampler = 
sampler_state
{
    Texture = <g_cropTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    AddressU  = CLAMP;
    AddressV  = CLAMP;
};


float4 SampleCropSheetColour(float2 TextureUV0)
{
//	float ddxOrig = ddx(TextureUV0[0]);
//	float ddyOrig = ddy(TextureUV0[1]);
	
	TextureUV0[0] = frac(TextureUV0[0]);
	TextureUV0[1] = frac(TextureUV0[1]);

	//Image not rotated in crop sheet
	if (g_rotatedCS == 0)
	{
		TextureUV0[0] = TextureUV0[0] * g_offsetWidthHeight[2] + g_offsetWidthHeight[0]; 
		TextureUV0[1] = TextureUV0[1] * g_offsetWidthHeight[3] + g_offsetWidthHeight[1]; 	
	}
	//Image rotated in crop sheet
	else
	{
		float tmp = TextureUV0[0];
		TextureUV0[0] = TextureUV0[1];
		TextureUV0[1] = tmp;
		TextureUV0[1] = 1 - TextureUV0[1];
		TextureUV0[0] = TextureUV0[0] * g_offsetWidthHeight[3] + g_offsetWidthHeight[0]; 
		TextureUV0[1] = TextureUV0[1] * g_offsetWidthHeight[2] + g_offsetWidthHeight[1];
	}

	TextureUV0[0] = max(TextureUV0[0], g_clamping[0]);
	TextureUV0[1] = max(TextureUV0[1], g_clamping[1]);
		
	TextureUV0[0] = min(TextureUV0[0], g_clamping[2]);
	TextureUV0[1] = min(TextureUV0[1], g_clamping[3]);

	return tex2D(CropTextureSampler, TextureUV0);
//	return tex2D(CropTextureSampler, TextureUV0, ddxOrig, ddyOrig);
}