//-----------------------------------------------------------------------------
// File: ShadowMap.fx
//
// Desc: Effect	file for high dynamic range	cube mapping sample.
//
// Copyright (c) Microsoft Corporation.	All	rights reserved.
//-----------------------------------------------------------------------------


#define	SMAP_SIZE g_fShadowMapSize
#define NUM_PASSES g_fPasses


#define SHADOW_EPSILON 0.000005f


float4x4 g_mWorldView;
float4x4 g_mProj;
float4x4 g_mLightProj;  // Transform to light projection space
float4	 g_vMaterial;
texture	 g_txScene;
texture	 g_txShadow;
texture	 g_txPositions, g_txNormals;
float3	 g_vLightPos;  // Light	position in	view space
float3	 g_vLightDir;  // Light	direction in view space
float4	 g_vLightDiffuse = float4( 1.0f, 1.0f, 1.0f, 1.0f );  // Light diffuse color
float4	 g_vLightAmbient = float4( 0.0f, 0.0f, 0.0f, 0.0f );  // Use an	ambient	light of 0.3
float	 g_fCosTheta = 0;  // Cosine of	theta of the spot light
float	 g_fPasses = 256.0f;
float	 g_fAtlasSize = 1024.0f;
float	 g_fShadowMapSize = 1024.0f;


float	 g_fBiasScale = 0.0001;



sampler2D g_samScene =
sampler_state
{
	Texture	= <g_txScene>;
	MinFilter =	Point;
	MagFilter =	Linear;
	MipFilter =	Linear;
};

// texture containing vertex positions
sampler2D g_samPosition =
sampler_state
{
	Texture	= <g_txPositions>;
	MinFilter =	Point;
	MagFilter =	Linear;
	MipFilter =	Linear;
};

// texture containing vertex positions
sampler2D g_samNormal =
sampler_state
{
	Texture	= <g_txNormals>;
	MinFilter =	Point;
	MagFilter =	Linear;
	MipFilter =	Linear;
};


sampler2D g_samShadow =
sampler_state
{
	Texture	= <g_txShadow>;
	MinFilter =	Point;
	MagFilter =	Point;
	MipFilter =	Point;
	AddressU = Clamp;
	AddressV = Clamp;
};



// some fudgy code to transform normal into light-space
float3 TransformNormalToLightSpace(float3 vNormal)
{
	float3 vNormalLight;
	
	vNormalLight = -vNormal;
//	vNormalLight.z = -vNormalLight.z;	// JAN: not sure why this needs to be here
	
	vNormalLight = mul( vNormalLight, (float3x3)g_mWorldView );
	vNormalLight = normalize( vNormalLight );
	
	return vNormalLight;
}

//-----------------------------------------------------------------------------
// Vertex Shader: VertScene
// Desc: Process vertex	for	scene
//-----------------------------------------------------------------------------
void VertScene(	float4 iPos	: POSITION,
				float3 iNormal : NORMAL,
				float3 iFaceNormal : TANGENT,
				float2 iTex	: TEXCOORD0,
				out	float4 oPos	: POSITION,
				out	float2 Tex : TEXCOORD0,
				out	float4 vPos	: TEXCOORD1,
				out	float4 vPosLight : TEXCOORD3,
				out float3 vNormalLight : TEXCOORD4,
				out float3 vFaceNormalLight : TEXCOORD2
				 )
{
	float4 vInPos = iPos;
	
	//
	// Transform position to view space
	//
	vPos = mul(	vInPos, g_mWorldView );

	//
	// Convert (expanded) UV coordinate to screen coord
	//
	oPos.x = iTex.x*2 - 1;
	oPos.y = 1 - iTex.y*2;
	oPos.z = 0.5;
	oPos.w = 1;

	// offset the UV coordinates to the middle of the texel	
	oPos.x -= 1.0f/(float)g_fAtlasSize;
	oPos.y += 1.0f/(float)g_fAtlasSize;

	//
	// Propagate texture coord
	//
	Tex	= iTex;

	//
	// Transform the position to light projection space, or	the
	// projection space	as if the camera is	looking	out	from
	// the spotlight.
	//
	vPosLight =	mul( iPos, g_mLightProj );


	// for doing N*L
	vNormalLight = TransformNormalToLightSpace( iNormal );
	
	// Transform the normal into light-space (so that z is pointing at the light)
	// then pass 1.0/z into the pixel shader. This is used to calculate the bias.
	// If the angle of the plane vs the light is low, 1.0/z will be high, which makes
	// the bias high.
	vFaceNormalLight = -iFaceNormal;
	vFaceNormalLight = mul( vFaceNormalLight, (float3x3)g_mWorldView );
	vFaceNormalLight = normalize( vFaceNormalLight );
	vFaceNormalLight.x = vFaceNormalLight.x/vFaceNormalLight.z;
	vFaceNormalLight.y = vFaceNormalLight.y/vFaceNormalLight.z;
	
	vFaceNormalLight.x *= g_fBiasScale;
	vFaceNormalLight.y *= g_fBiasScale;
	
	// TODO: scale z depending on the world-space scale of the shadowmap
}



//-----------------------------------------------------------------------------
// Pixel Shader: PixScene
// Desc: Process pixel (do per-pixel lighting) for enabled scene
//-----------------------------------------------------------------------------
float4 PixScene( float4	vPosLight :	TEXCOORD3,
				 float3 vNormalLight: TEXCOORD4,
				 float3 vFaceNormalLight : TEXCOORD2,
				 uniform bool bVertexColours
				  )	: COLOR
{
	float4 Diffuse;
	float LightAmount, epsilon, depth, depthX;

	// TODO: check whether this vPosLight is behind the light by looking at z?
	if ( 0 )
		return float4(0,0,0,0);
	
	// Pixel is	in lit area. Find out if it's
	// in shadow using 2x2 percentage closest filtering

	//transform	from RT	space to texture space.
	float2 ShadowTexC =	0.5	* vPosLight.xy + float2( 0.5, 0.5 );
	ShadowTexC.y = 1.0f	- ShadowTexC.y;

	// transform to	texel space
	float2 texelpos	= SMAP_SIZE	* ShadowTexC;
	
	// Determine the lerp amounts
	float2 lerps = frac( texelpos );

	depth = vPosLight.z;

	// anti-acne
	depth += vFaceNormalLight.x * lerps.x;
	depth += -vFaceNormalLight.y * lerps.y;
	
	// the steeper the angle is against the light, the bigger the epsilon may need to be
	epsilon = SHADOW_EPSILON*100;// + (1.0-vNormalLight.z) * 0.001;

	//read in bilerp stamp,	doing the shadow checks
	float sourcevals[4];
	sourcevals[0] =	(tex2D( g_samShadow, ShadowTexC	) +	epsilon < depth)? 0.0f: 1.0f;
	
	depthX = depth;
	depthX += -vFaceNormalLight.x;
	
	sourcevals[1] =	(tex2D( g_samShadow, ShadowTexC	+ float2(1.0/SMAP_SIZE,	0) ) + epsilon < depthX)? 0.0f: 1.0f;	 
	
	depthX = depth;
	depthX += vFaceNormalLight.y;
	
	sourcevals[2] =	(tex2D( g_samShadow, ShadowTexC	+ float2(0,	1.0/SMAP_SIZE) ) + epsilon < depthX)? 0.0f: 1.0f;	 

	depthX = depth;
	depthX += -vFaceNormalLight.x;
	depthX += vFaceNormalLight.y;
	
	sourcevals[3] =	(tex2D( g_samShadow, ShadowTexC	+ float2(1.0/SMAP_SIZE,	1.0/SMAP_SIZE) ) + epsilon < depthX)? 0.0f: 1.0f;	 
	
	// lerp	between	the	shadow values to calculate our light amount
	LightAmount =	lerp( lerp(	sourcevals[0], sourcevals[1], lerps.x ),
								lerp(	sourcevals[2], sourcevals[3], lerps.x ),
								lerps.y );
								
//	LightAmount = sourcevals[3];

	float zlookup = tex2D( g_samShadow, ShadowTexC );
	
	Diffuse.x = zlookup;
	Diffuse.y = depth;	// *3 -0.7;
//	Diffuse = (Diffuse.x - Diffuse.y) > -epsilon;
//	Diffuse = (zlookup - depth) +0.5;//> -epsilon;
//	Diffuse = Diffuse.y+0.5;
	Diffuse = Diffuse.y;
//	Diffuse = sourcevals[0];

	// Light it
//	Diffuse = LightAmount;
//	LightAmount = 1;

//	Diffuse = LightAmount * saturate( vNormalLight.z *100 );		// no N*L
	Diffuse = LightAmount * saturate( vNormalLight.z *3);			// slight N*L - looks nicer when acne is not there

	Diffuse *= (1.0/NUM_PASSES);
	Diffuse.w = 1;

	return Diffuse;
}



//-----------------------------------------------------------------------------
// Vertex Shader: VertShadow
// Desc: Process vertex	for	the	shadow map
//-----------------------------------------------------------------------------
void VertShadow( float4	Pos	: POSITION,
				 float3	Normal : NORMAL,
				 out float4	oPos : POSITION,
				 out float2	Depth :	TEXCOORD0 )
{
	//
	// Compute the projected coordinates
	//
	oPos = mul(	Pos, g_mWorldView );
	oPos = mul(	oPos, g_mProj );

	//
	// Store z and w in	our	spare texcoord
	//
	Depth.xy = oPos.zw;
}




//-----------------------------------------------------------------------------
// Pixel Shader: PixShadow
// Desc: Process pixel for the shadow map
//-----------------------------------------------------------------------------
void PixShadow(	float2 Depth : TEXCOORD0,
				out	float4 Color : COLOR )
{
	//
	// Depth is	z /	w
	//
	Color =	Depth.x;// / Depth.y;
}

//-----------------------------------------------------------------------------
// Pixel Shader: PixVertShadow
// Desc: render lighting into vertex colours
//-----------------------------------------------------------------------------
float4 PixTest( float3 TexCoord : TEXCOORD0) : COLOR 
{
	float3 vPos		= tex2D( g_samPosition, TexCoord );
	float3 vNormal  = tex2D( g_samNormal, TexCoord );
	float4 Out;

	float4 vPosLight	= mul( vPos, g_mLightProj );
	float3 vNormalLight = TransformNormalToLightSpace( vNormal );
	
	return PixScene(vPosLight, vNormalLight, vNormalLight, true);
}


//-----------------------------------------------------------------------------
// Technique: RenderScene
// Desc: Renders scene objects
//-----------------------------------------------------------------------------
technique RenderScene
{
	pass p0
	{
		CullMode = None;
		
        AlphaBlendEnable	= True;
        ColorOp[0]			= Add;
        SrcBlend			= One;
        DestBlend			= One;
	
		ZFunc = NotEqual;	// stop overdraw on the lightmap
		
		VertexShader = compile vs_1_1 VertScene();
		PixelShader	= compile ps_2_0 PixScene(false);
	}
}



//-----------------------------------------------------------------------------
// Technique: RenderShadow
// Desc: Renders the shadow	map
//-----------------------------------------------------------------------------
technique RenderShadow
{
	pass p0
	{
//		CullMode = CW;	// only backfaces emit shadows
		CullMode = None;// normal (CCW seems to bugger up)
		
		ColorWriteEnable	= RED | GREEN | BLUE | ALPHA;
		
		ZFunc = LessEqual;
		
		VertexShader = compile vs_1_1 VertShadow();
		PixelShader	= compile ps_2_0 PixShadow();
	}
}


//-----------------------------------------------------------------------------
// Technique: RenderScene
// Desc: Lights vertex position/normals and render result into framebuffer
//-----------------------------------------------------------------------------
technique RenderSceneToVertices
{
	pass p0
	{
		CullMode = None;
		
        AlphaBlendEnable	= True;
        ColorOp[0]			= Add;
        SrcBlend			= One;
        DestBlend			= One;
		
		ZFunc = NotEqual;	// stop overdraw on the lightmap
		
		PixelShader	= compile ps_2_0 PixTest();
	}
}
