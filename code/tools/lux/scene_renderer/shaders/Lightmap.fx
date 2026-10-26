//--------------------------------------------------------------------------------------
// File: BasicHLSL.fx
//
// The effect file for the BasicHLSL sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
float4 g_MaterialAmbientColor;      // Material's ambient color
float4 g_MaterialDiffuseColor;      // Material's diffuse color
int g_nNumLights;

float3 g_LightDir[3];               // Light's direction in world space
float4 g_LightDiffuse[3];           // Light's diffuse color
float4 g_LightAmbient;              // Light's ambient color

texture g_MeshTexture;              // Color texture for mesh
texture g_LightmapTexture;              // Color texture for mesh
texture g_BumpTexture;				// bumpmap texture for testing SH

float4x4 g_mWorld;                  // World matrix for object
float4x4 g_mWorldViewProjection;    // World * View * Projection matrix
float4x4 g_mView;

float3 gvLightDirection = {0.7,0.7,-0.7};

float4 g_vSH;

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

sampler LightmapTextureSampler = 
sampler_state
{
    Texture = <g_LightmapTexture>;
    MipFilter = NONE;			// Lightmaps shouldn't mipmap
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    
};

sampler BumpyTextureSampler = 
sampler_state
{
	Texture = <g_BumpTexture>;
	MipFilter = NONE;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float4 Diffuse    : COLOR0;     // vertex diffuse color (note that COLOR0 is clamped from 0..1)
    float4 Diffuse2	  : COLOR1;
    float2 TextureUV0  : TEXCOORD0;  // vertex texture coords 
    float2 TextureUV1  : TEXCOORD1;  // vertex texture coords 
    float2 TextureUV2  : TEXCOORD2;  // vertex texture coords 
};


//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderSceneVS( float4 vPos : POSITION, 
                         float3 vNormal : NORMAL,
                         float2 vTexCoord0 : TEXCOORD0,
                         float2 vTexCoord1 : TEXCOORD1,
//						 float4 vColour : COLOR0,
                         uniform int nNumLights,
                         uniform bool bTexture,
                         uniform bool bAnimate )
{
	float4 vColour = 1;

    VS_OUTPUT Output;
    float3 vNormalWorldSpace, vNormalViewSpace;
  
    // Animation the vertex based on time and the vertex's object space position
    
    // Transform the position from object space to homogeneous projection space
    Output.Position = mul(vPos, g_mWorldViewProjection);
    
    // Transform the normal from object space to world space    
    vNormalWorldSpace = mul(vNormal, (float3x3)g_mWorld); // normal (world space)
    vNormalViewSpace = mul(vNormal, (float3x3)g_mView); // normal (view space)
    vNormalViewSpace = normalize(vNormalViewSpace);
    
    // Compute simple directional lighting equation
/*    float3 vTotalLightDiffuse = float3(0,0,0);
    for(int i=0; i<nNumLights; i++ )
        vTotalLightDiffuse += g_LightDiffuse[i] * max(0,dot(vNormalWorldSpace, g_LightDir[i]));
        
    Output.Diffuse.rgb = g_MaterialDiffuseColor * vTotalLightDiffuse + 
                         g_MaterialAmbientColor * g_LightAmbient;   
    Output.Diffuse.a = 1.0f; 
  */  
  
	float lambert = dot(gvLightDirection, vNormalViewSpace);
  
    if (nNumLights == 0)
		lambert = 1;
  
	Output.Diffuse.rgb = vColour * lambert;
	Output.Diffuse.a = 1.0f;
	
	Output.Diffuse2 = vColour;
    // Just copy the texture coordinate through
    if( bTexture ) 
    {
        Output.TextureUV0 = vTexCoord0; 
        Output.TextureUV1 = vTexCoord1; 
        Output.TextureUV2 = vTexCoord0;
	}
    else
    {
        Output.TextureUV0 = 0; 
        Output.TextureUV1 = 0; 
        Output.TextureUV2 = 0;
	}
    
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
PS_OUTPUT RenderScenePS( VS_OUTPUT In,
                         uniform bool bTexture, uniform int lightmapmode ) 
{ 
    PS_OUTPUT Output;

    In.Diffuse = 1;

    // Lookup mesh texture and modulate it with diffuse
    float4 vLMap = tex2D(LightmapTextureSampler, In.TextureUV1); 
//	vLMap += tex2D(LightmapTextureSampler, (In.TextureUV1 + float2(0.5f,0)) ); 
//	vLMap += tex2D(LightmapTextureSampler, (In.TextureUV1 + float2(0,0.5f)) ); 
//	float4 tmp = tex2D(LightmapTextureSampler, In.TextureUV1 + float2(0.5,0.5)); 
//	vLMap += tmp.y + tmp.w;
    float4 l = 1;

    if (lightmapmode == 0)
    {
		float fMax = vLMap.w;
		
		l.x = fMax * vLMap.x;
		l.y = fMax * vLMap.y;
		l.z = fMax * vLMap.z;
		l.w = 1;
    }
    else if (lightmapmode == 1)
    {
		l = vLMap;
//		l = (vLMap.y + vLMap.w) * 0.5;
    }
    else if (lightmapmode == 2)
    {
		l = (vLMap.y + vLMap.a) * 0.5;
    }
    else if (lightmapmode == 3)
    {
		// l = (vLMap.y * vLMap.z) + vLMap.y*0.5;
		l = tex2D(LightmapTextureSampler, In.TextureUV1); 
		l = l * l.a;
    }
    else if (lightmapmode == 4)
    {
		l = vLMap;

		float2 grid = frac( (In.TextureUV1 * 1.0f) * 512.0f );

		if ((grid.x > 0.9) || (grid.y > 0.9f))
		{
			l = float4(0.0f, 0.0f, 1.0f, 0.0f);
		}
    }
    else if (lightmapmode == 5)
    {
		l = tex2D(LightmapTextureSampler, In.TextureUV1 + float2(0.5,0.5)); 
		
		l = (l.x+l.y+l.z)/3.f;
	}
	else if (lightmapmode == 6)
	{	
		// test shader to display car vtx colour SH transfer
		float4 vSH = In.Diffuse2;
		vSH.y = vSH.y*2.f-1.f;
		vSH.z = vSH.z*2.f-1.f;
		vSH.w = vSH.w*2.f-1.f;
		
		Output.RGBColor = saturate(dot(vSH, g_vSH) * 0.2f);
		
		return Output;	
	}
    
    if( bTexture )
        Output.RGBColor = In.Diffuse * l;
    else
        Output.RGBColor = In.Diffuse;

    return Output;
}

//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
//       color with diffuse material color
//--------------------------------------------------------------------------------------
PS_OUTPUT RenderSceneBumpyPS( VS_OUTPUT In )
{ 
    PS_OUTPUT Output;

    float4 vLMap = tex2D(LightmapTextureSampler, In.TextureUV1); 
    float4 l;
    
    float3 vBump = tex2D(BumpyTextureSampler, In.TextureUV2);
    
	l = vLMap;
    
	Output.RGBColor = tex2D(MeshTextureSampler, In.TextureUV0) * l * In.Diffuse; // TODO BUMPYNESS!

    return Output;
}

//--------------------------------------------------------------------------------------
// Renders scene to render target
//--------------------------------------------------------------------------------------
technique RenderSceneWithTexture1Light
{
    pass P0
    {          
    // this mode is ONLY used if we haven't rendered any lightmaps yet
        VertexShader = compile vs_2_0 RenderSceneVS( 1, true, true );
        PixelShader  = compile ps_2_0 RenderScenePS( false, 0 );
    }
}

// full colour lightmaps
technique RenderSceneMode0
{
    pass P0
    {
        VertexShader = compile vs_2_0 RenderSceneVS( 0, true, true );
		PixelShader  = compile ps_2_0 RenderScenePS( true, 0 );
    }
}

// DXT1 style lightmaps
technique RenderSceneMode1
{
    pass P0
    {
        VertexShader = compile vs_2_0 RenderSceneVS( 0, true, true );
		PixelShader  = compile ps_2_0 RenderScenePS( true, 1 );
    }
}

// A8L8 style lightmaps
technique RenderSceneMode2
{
    pass P0
    {
        VertexShader = compile vs_2_0 RenderSceneVS( 0, true, true );
		PixelShader  = compile ps_2_0 RenderScenePS( true, 2 );
    }
}

// multiply G*B lightmaps
technique RenderSceneMode3
{
    pass P0
    {
        VertexShader = compile vs_2_0 RenderSceneVS( 0, true, true );
		PixelShader  = compile ps_2_0 RenderScenePS( true, 3 );
    }
}

// debug unwrapping
technique RenderSceneMode4
{
    pass P0
    {
        VertexShader = compile vs_2_0 RenderSceneVS( 0, true, true );
		PixelShader  = compile ps_2_0 RenderScenePS( true, 4 );
    }
}

// bumpy lightmaps
technique RenderSceneMode5
{
    pass P0
    {
        VertexShader = compile vs_2_0 RenderSceneVS( 0, true, true);
		PixelShader  = compile ps_2_0 RenderScenePS( true, 5 );
		
//	    CullMode = None;
    }
}

// vtx SH
technique RenderSceneMode6
{
    pass P0
    {
        VertexShader = compile vs_2_0 RenderSceneVS( 0, true, true);
		PixelShader  = compile ps_2_0 RenderScenePS( true, 6 );
		
//	    CullMode = None;
    }
}

// ------------------------------- SPHERICAL HARMONICS RENDERING


float4 cAr;
float4 cAg;
float4 cAb;
float4 cBr;
float4 cBg;
float4 cBb;
float4 cC;

//-----------------------------------------------------------------------------
VS_OUTPUT SHIrradianceEnvironmentMapVS( float4 vPos : POSITION, 
                                        float4 vNormal : NORMAL,
                                        float2 vTexCoord0 : TEXCOORD0,
                                        float4 vColor : COLOR0)
{
    VS_OUTPUT Output;
    
    // Output the vetrex position in projection space
    Output.Position = mul(vPos, g_mWorldViewProjection);
    
    float3 x1, x2, x3;
    
    // Linear + constant polynomial terms
    x1.r = dot(cAr,vNormal);
    x1.g = dot(cAg,vNormal);
    x1.b = dot(cAb,vNormal);
    
    // 4 of the quadratic polynomials
    float4 vB = vNormal.xyzz * vNormal.yzzx;   
    x2.r = dot(cBr,vB);
    x2.g = dot(cBg,vB);
    x2.b = dot(cBb,vB);
   
    // Final quadratic polynomial
    float vC = vNormal.x*vNormal.x - vNormal.y*vNormal.y;
    x3 = cC.rgb * vC;    

    Output.Diffuse.rgb = x1 + x2 + x3;
    Output.Diffuse.a   = 1.0f; 
    Output.TextureUV0 = Output.TextureUV1 = Output.TextureUV2 = 0;
    Output.Diffuse2 = vColor;

    return Output;
}

//-----------------------------------------------------------------------------
// Trival pixel shader (could use FF if desired)
//-----------------------------------------------------------------------------
PS_OUTPUT StandardPS( VS_OUTPUT In,
                      uniform bool bTexture ) 
{ 
    PS_OUTPUT Output;

	Output.RGBColor = In.Diffuse;

    return Output;
}

//-----------------------------------------------------------------------------
technique RenderWithSHIrradEnvMap
{
    pass P0
    {          
        VertexShader = compile vs_1_1 SHIrradianceEnvironmentMapVS();
        PixelShader  = compile ps_1_1 StandardPS( false );
    }
}

