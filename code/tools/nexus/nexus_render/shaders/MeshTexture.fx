//--------------------------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------------------------

	//#include "MeshTextureCommon.fx"
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


	//#include "Structures.fx"
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

//--------------------------------------------------------------------------------------
// [posNormTex1] Vertex Declaration
//--------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------
	// Static Mesh Vertex Shaders
	//--------------------------------------------------------------------------------------
	POSCOLTEX1_VS_OUTPUT Static_posNormTex1_VS(	
		float4 vPos : POSITION, 
		float2 vTexCoord0 : TEXCOORD0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
	    
		// Transform the position from object space to homogeneous projection space
		Output.Position = mul(vPos, g_mWorldViewProjection);

		//No lights - just add vertex colour contribution.
		Output.Diffuse.rgb = 1;
		Output.Diffuse.a = 1;
		
		//Offset and copy texture coordinates
		Output.TextureUV0[0] = vTexCoord0[0];
		Output.TextureUV0[1] = vTexCoord0[1];
		return Output;    
	}

	POSCOLTEX1_VS_OUTPUT Static_posNormTex1_CM1_VS(	
		float4 vPos : POSITION, 
		float2 vTexCoord0 : TEXCOORD0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
	 
		// Transform the position from object space to homogeneous projection space
		Output.Position = mul(vPos, g_mWorldViewProjection);
	    
		//Display black as we have no vertex weights.
		Output.Diffuse.rgb = float3(0.0f,0.0f,0.0f);
		Output.Diffuse.a = 1;
		
		//Offset and copy texture coordinates
		Output.TextureUV0[0] = vTexCoord0[0];
		Output.TextureUV0[1] = vTexCoord0[1];
		return Output;    
	}

	POSCOLTEX1_VS_OUTPUT Static_posNormTex1_CM2_VS(	
		float4 vPos : POSITION, 
		float2 vTexCoord0 : TEXCOORD0)
	{
		POSCOLTEX1_VS_OUTPUT Output;

		// Transform the position from object space to homogeneous projection space
		Output.Position = mul(vPos, g_mWorldViewProjection);
	    
		//Display diffuse UV set	
		Output.Diffuse.r = vTexCoord0[0];
		Output.Diffuse.g = 0;
		Output.Diffuse.b = vTexCoord0[1];
		Output.Diffuse.a = 1;
		
		//Offset and copy texture coordinates
		Output.TextureUV0[0] = vTexCoord0[0];
		Output.TextureUV0[1] = vTexCoord0[1];
		return Output;    
	}

	POSCOLTEX1_VS_OUTPUT Static_posNormTex1_CM3_VS(	
		float4 vPos : POSITION, 
		float2 vTexCoord0 : TEXCOORD0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
	   
		// Transform the position from object space to homogeneous projection space
		Output.Position = mul(vPos, g_mWorldViewProjection);
	    
		Output.Diffuse.r = 0;
		Output.Diffuse.g = 0;
		Output.Diffuse.b = 0;
		Output.Diffuse.a = 1;
		
		//Offset and copy texture coordinates
		Output.TextureUV0[0] = vTexCoord0[0];
		Output.TextureUV0[1] = vTexCoord0[1];
		return Output;    
	}

	POSCOLTEX1_VS_OUTPUT Static_posNormTex1_RTL1_VS(	
		float4 vPos : POSITION, 
		float3 vNormal : NORMAL,
		float2 vTexCoord0 : TEXCOORD0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
		float3 vNormalViewSpace;
	  
		// Transform the position from object space to homogeneous projection space
		Output.Position = mul(vPos, g_mWorldViewProjection);
	    
		// Transform the normal from object space to world space 
		vNormalViewSpace = mul(vNormal, (float3x3)g_mWorldTransposedInverse); // normal (world space)
		vNormalViewSpace = mul(vNormalViewSpace, (float3x3)g_mView); // normal (view space)
		vNormalViewSpace = normalize(vNormalViewSpace);

		//Add diffuse lighting contribution, and vertex colour contribution.
		float lambert = dot(g_lightDirection, vNormalViewSpace);
		lambert = saturate(lambert);
		Output.Diffuse.rgb = lambert;
		Output.Diffuse.rgb += 0.3f;
		Output.Diffuse.a = 1;
		
		//Offset and copy texture coordinates
		Output.TextureUV0[0] = vTexCoord0[0];
		Output.TextureUV0[1] = vTexCoord0[1];
		return Output;    
	}
	//--------------------------------------------------------------------------------------
	// END Static Mesh Vertex Shaders
	//--------------------------------------------------------------------------------------	
//--------------------------------------------------------------------------------------
// END [posNormTex1] Vertex Declaration
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// [posNormColTex1] Vertex Declaration
//--------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------
	// [posNormColTex1] Static Mesh Vertex Shaders
	//--------------------------------------------------------------------------------------
	POSCOLTEX1_VS_OUTPUT Static_posNormColTex1_VS(	
		float4 vPos : POSITION, 
		float3 vColour : COLOR0,
		float2 vTexCoord0 : TEXCOORD0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
		// Animation the vertex based on time and the vertex's object space position
	    
		// Transform the position from object space to homogeneous projection space
		Output.Position = mul(vPos, g_mWorldViewProjection);

		//No lights - just add vertex colour contribution.
		Output.Diffuse.rgb = vColour;
		Output.Diffuse.a = 1;
		
		//Offset and copy texture coordinates
		Output.TextureUV0[0] = vTexCoord0[0];
		Output.TextureUV0[1] = vTexCoord0[1];		
		return Output;    
	}


	POSCOLTEX2_VS_OUTPUT Static_posNormTex2_CM3_VS(	
		float4 vPos : POSITION,
		float2 vTexCoord0 : TEXCOORD0,
		float2 vTexCoord1 : TEXCOORD1)
	{
		POSCOLTEX2_VS_OUTPUT Output;
	  
		// Animation the vertex based on time and the vertex's object space position
	    
		// Transform the position from object space to homogeneous projection space
		Output.Position = mul(vPos, g_mWorldViewProjection);
	    
		//Display diffuse UV set	
		Output.Diffuse.r = vTexCoord1[0];
		Output.Diffuse.g = 0;
		Output.Diffuse.b = vTexCoord1[1];
		Output.Diffuse.a = 1;
		
		//Offset and copy texture coordinates
		Output.TextureUV0[0] = vTexCoord0[0];
		Output.TextureUV0[1] = vTexCoord0[1];	
		//Offset and copy texture coordinates
		Output.TextureUV1[0] = vTexCoord1[0];
		Output.TextureUV1[1] = vTexCoord1[1];					
		
		return Output;    
	}	

	POSCOLTEX1_VS_OUTPUT Static_posNormColTex1_RTL1_VS(	
		float4 vPos : POSITION, 
		float3 vNormal : NORMAL,
		float3 vColour : COLOR0,
		float2 vTexCoord0 : TEXCOORD0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
		float3 vNormalViewSpace;
	  
		// Animation the vertex based on time and the vertex's object space position
	    
		// Transform the position from object space to homogeneous projection space
		Output.Position = mul(vPos, g_mWorldViewProjection);
	    
		// Transform the normal from object space to world space 
		vNormalViewSpace = mul(vNormal, (float3x3)g_mWorldTransposedInverse); // normal (world space)
		vNormalViewSpace = mul(vNormalViewSpace, (float3x3)g_mView); // normal (view space)
		vNormalViewSpace = normalize(vNormalViewSpace);

		//Add diffuse lighting contribution, and vertex colour contribution.
		float lambert = dot(g_lightDirection, vNormalViewSpace);
		lambert = saturate(lambert);
		Output.Diffuse.rgb = vColour * lambert;
		Output.Diffuse.rgb += 0.3f;
		Output.Diffuse.a = 1;
		
		Output.TextureUV0 = vTexCoord0;
		return Output;    
	}
	//--------------------------------------------------------------------------------------
	// END [posNormColTex1] Static Mesh Vertex Shaders
	//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
// END [posNormColTex1] Vertex Declaration
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// [posNormTex1Weight4Idx4] Vertex Declaration
//--------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------
	// [posNormTex1Weight4Idx4] Skinned Mesh Vertex Shaders
	//--------------------------------------------------------------------------------------

	POSCOLTEX1_VS_OUTPUT Skinned_posNormTex1Weight4Idx4_VS(	
		float4 vPos : POSITION, 
		float2 vTexCoord0 : TEXCOORD0,
		float4 vWeights : BLENDWEIGHT0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
	 
		// Transform the position from object space to homogeneous projection space
		Output.Position = mul(vPos, g_mWorldViewProjection);

		Output.Diffuse.rgb = 1;
		Output.Diffuse.a = 1;
		
		//Offset and copy texture coordinates
		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];

		return Output;  
	}

	POSCOLTEX1_VS_OUTPUT Skinned_posNormTex1Weight4Idx4_CM1_VS(	
		float4 vPos : POSITION, 
		float2 vTexCoord0 : TEXCOORD0,
		float4 vWeights : BLENDWEIGHT0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
	 
		// Transform the position from object space to homogeneous projection space
		Output.Position = mul(vPos, g_mWorldViewProjection);
	    
		//Display vertex weights
		Output.Diffuse.r = vWeights[0];
		Output.Diffuse.g = vWeights[1];
		Output.Diffuse.b = vWeights[2];
		Output.Diffuse.a = 1;
		
		//Offset and copy texture coordinates
		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];

		return Output;    
	}

	POSCOLTEX1_VS_OUTPUT Skinned_posNormTex1Weight4Idx4_CM2_VS(	
		float4 vPos : POSITION, 
		float2 vTexCoord0 : TEXCOORD0,
		float4 vWeights : BLENDWEIGHT0)
	{
		POSCOLTEX1_VS_OUTPUT Output;

		// Transform the position from object space to homogeneous projection space
		Output.Position = mul(vPos, g_mWorldViewProjection);

		//Display diffuse UV set	
		Output.Diffuse.r = vTexCoord0[0];
		Output.Diffuse.g = 0;
		Output.Diffuse.b = vTexCoord0[1];
		Output.Diffuse.a = 1;
		
		//Offset and copy texture coordinates
		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];

		return Output;    
	}

	POSCOLTEX1_VS_OUTPUT Skinned_posNormTex1Weight4Idx4_CM3_VS(	
		float4 vPos : POSITION, 
		float2 vTexCoord0 : TEXCOORD0,
		float4 vWeights : BLENDWEIGHT0)
	{
		POSCOLTEX1_VS_OUTPUT Output;

		// Transform the position from object space to homogeneous projection space
		Output.Position = mul(vPos, g_mWorldViewProjection);
	    
		//Display lightmap UV set
		Output.Diffuse.r = 0;
		Output.Diffuse.g = 0;
		Output.Diffuse.b = 0;
		Output.Diffuse.a = 1;
		
		//Offset and copy texture coordinates
		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];

		return Output;    
	}
	
	POSCOLTEX2_VS_OUTPUT Skinned_posNormTex2Weight4Idx4_CM3_VS(	
		float4 vPos : POSITION, 
		float2 vTexCoord0 : TEXCOORD0,
		float2 vTexCoord1 : TEXCOORD1,
		float4 vWeights : BLENDWEIGHT0)
	{
		POSCOLTEX2_VS_OUTPUT Output;

		// Transform the position from object space to homogeneous projection space
		Output.Position = mul(vPos, g_mWorldViewProjection);
	    
		//Display lightmap UV set
		Output.Diffuse.r = 0;
		Output.Diffuse.g = 0;
		Output.Diffuse.b = 0;
		Output.Diffuse.a = 1;
		
		//Offset and copy texture coordinates
		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];		
		Output.TextureUV1[0] = vTexCoord1[0]; 
		Output.TextureUV1[1] = vTexCoord1[1];

		return Output;    
	}
	

	POSCOLTEX1_VS_OUTPUT Skinned_posNormTex1Weight4Idx4_RTL1_VS(	
		float4 vPos : POSITION, 
		float3 vNormal : NORMAL,
		float2 vTexCoord0 : TEXCOORD0,
		float4 vWeights : BLENDWEIGHT0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
		float3 vNormalViewSpace;
	    
		// Transform the position from object space to homogeneous projection space
		Output.Position = mul(vPos, g_mWorldViewProjection);
	    
		// Transform the normal from object space to world space 
		vNormalViewSpace = mul(vNormal, (float3x3)g_mWorldTransposedInverse); // normal (world space)
		vNormalViewSpace = mul(vNormalViewSpace, (float3x3)g_mView); // normal (view space)
		vNormalViewSpace = normalize(vNormalViewSpace);

		//Add diffuse lighting contribution, and vertex colour contribution.
		float lambert = dot(g_lightDirection, vNormalViewSpace);
		lambert = saturate(lambert);
		Output.Diffuse.rgb = lambert;
		Output.Diffuse.rgb += 0.3f;
		Output.Diffuse.a = 1;
		
		//Offset and copy texture coordinates
		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];

		return Output;    
	}
	
	//--------------------------------------------------------------------------------------
	// [posNormTex1Weight4Idx4] Deformed (Boned) Instance Shaders.
	//--------------------------------------------------------------------------------------
	POSCOLTEX1_VS_OUTPUT BonedInstance_posNormTex1Weight4Idx4_VS( 
		float4 vPos : POSITION, 
		float2 vTexCoord0 : TEXCOORD0,
		float4 vWeights : BLENDWEIGHT0,
		int4 vBoneIndices : BLENDINDICES0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
		float4 vPosResultWorldSpace = float4(0.0f,0.0f,0.0f,0.0f);
		float lastWeight = 0;
	  
		//Apply first bone
		lastWeight = vWeights[0];
		
		vPosResultWorldSpace += vWeights[0] * 
			mul(vPos, g_mPalette[vBoneIndices[0]]);
			
		if (vBoneIndices[3] > 1)
		{
			//Apply last bone
			lastWeight = 1.0f - lastWeight;
			vPosResultWorldSpace += lastWeight * 
				mul(vPos, g_mPalette[vBoneIndices[1]]);
		}
			
		//Apply the camera and projection matrix to our position.
		Output.Position = mul(vPosResultWorldSpace, g_mWorldViewProjection);

		//No lights - just add vertex colour contribution.
		Output.Diffuse.rgb = 1;
		Output.Diffuse.a = 1;
			
		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];
	    
		return Output;    
	}
	
	POSCOLTEX1_VS_OUTPUT BonedInstance_posNormTex1Weight4Idx4_CM1_VS( 
		float4 vPos : POSITION, 
		float2 vTexCoord0 : TEXCOORD0,
		float4 vWeights : BLENDWEIGHT0,
		int4 vBoneIndices : BLENDINDICES0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
		float4 vPosResultWorldSpace = float4(0.0f,0.0f,0.0f,0.0f);
		float lastWeight = 0;
	  
		//Apply first bone
		lastWeight = vWeights[0];
		
		vPosResultWorldSpace += vWeights[0] * 
			mul(vPos, g_mPalette[vBoneIndices[0]]);
			
		if (vBoneIndices[3] > 1)
		{
			//Apply last bone
			lastWeight = 1.0f - lastWeight;
			vPosResultWorldSpace += lastWeight * 
				mul(vPos, g_mPalette[vBoneIndices[1]]);
		}
			
		//Apply the camera and projection matrix to our position.
		Output.Position = mul(vPosResultWorldSpace, g_mWorldViewProjection);

		//Display vertex weights
		Output.Diffuse.r = vWeights[0];
		Output.Diffuse.g = vWeights[1];
		Output.Diffuse.b = vWeights[2];
		Output.Diffuse.a = 1;
			
		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];
	    
		return Output;    
	}
	
	POSCOLTEX1_VS_OUTPUT BonedInstance_posNormTex1Weight4Idx4_CM2_VS( 
		float4 vPos : POSITION, 
		float2 vTexCoord0 : TEXCOORD0,
		float4 vWeights : BLENDWEIGHT0,
		int4 vBoneIndices : BLENDINDICES0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
		float4 vPosResultWorldSpace = float4(0.0f,0.0f,0.0f,0.0f);
		float lastWeight = 0;
	  
		//Apply first bone
		lastWeight = vWeights[0];
		
		vPosResultWorldSpace += vWeights[0] * 
			mul(vPos, g_mPalette[vBoneIndices[0]]);
			
		if (vBoneIndices[3] > 1)
		{
			//Apply last bone
			lastWeight = 1.0f - lastWeight;
			vPosResultWorldSpace += lastWeight * 
				mul(vPos, g_mPalette[vBoneIndices[1]]);
		}
			
		//Apply the camera and projection matrix to our position.
		Output.Position = mul(vPosResultWorldSpace, g_mWorldViewProjection);

		//Display diffuse UV set	
		Output.Diffuse.r = vTexCoord0[0];
		Output.Diffuse.g = 0;
		Output.Diffuse.b = vTexCoord0[1];
		Output.Diffuse.a = 1;
		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];
	    
		return Output;    
	}
	
	POSCOLTEX1_VS_OUTPUT BonedInstance_posNormTex1Weight4Idx4_CM3_VS( 
		float4 vPos : POSITION, 
		float2 vTexCoord0 : TEXCOORD0,
		float4 vWeights : BLENDWEIGHT0,
		int4 vBoneIndices : BLENDINDICES0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
		float4 vPosResultWorldSpace = float4(0.0f,0.0f,0.0f,0.0f);
		float lastWeight = 0;
	  
		//Apply first bone
		lastWeight = vWeights[0];
		
		vPosResultWorldSpace += vWeights[0] * 
			mul(vPos, g_mPalette[vBoneIndices[0]]);
			
		if (vBoneIndices[3] > 1)
		{
			//Apply last bone
			lastWeight = 1.0f - lastWeight;
			vPosResultWorldSpace += lastWeight * 
				mul(vPos, g_mPalette[vBoneIndices[1]]);
		}
			
		//Apply the camera and projection matrix to our position.
		Output.Position = mul(vPosResultWorldSpace, g_mWorldViewProjection);

		//Display lightmap UV set
		Output.Diffuse.r = 0;
		Output.Diffuse.g = 0;
		Output.Diffuse.b = 0;
		Output.Diffuse.a = 1;
			
		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];
	    
		return Output;    
	}	
	
	POSCOLTEX2_VS_OUTPUT BonedInstance_posNormTex2Weight4Idx4_CM3_VS( 
		float4 vPos : POSITION, 
		float2 vTexCoord0 : TEXCOORD0,
		float2 vTexCoord1 : TEXCOORD1,
		float4 vWeights : BLENDWEIGHT0,
		int4 vBoneIndices : BLENDINDICES0)
	{
		POSCOLTEX2_VS_OUTPUT Output;
		float4 vPosResultWorldSpace = float4(0.0f,0.0f,0.0f,0.0f);
		float lastWeight = 0;
	  
		//Apply first bone
		lastWeight = vWeights[0];
		
		vPosResultWorldSpace += vWeights[0] * 
			mul(vPos, g_mPalette[vBoneIndices[0]]);
			
		if (vBoneIndices[3] > 1)
		{
			//Apply last bone
			lastWeight = 1.0f - lastWeight;
			vPosResultWorldSpace += lastWeight * 
				mul(vPos, g_mPalette[vBoneIndices[1]]);
		}
			
		//Apply the camera and projection matrix to our position.
		Output.Position = mul(vPosResultWorldSpace, g_mWorldViewProjection);

		//Display lightmap UV set
		Output.Diffuse.r = 0;
		Output.Diffuse.g = 0;
		Output.Diffuse.b = 0;
		Output.Diffuse.a = 1;

		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];
		Output.TextureUV1[0] = vTexCoord1[0]; 
		Output.TextureUV1[1] = vTexCoord1[1];
	    
		return Output;    
	}	
	
	POSCOLTEX1_VS_OUTPUT BonedInstance_posNormTex1Weight4Idx4_RTL1_VS( 
		float4 vPos : POSITION, 
		float3 vNormal : NORMAL,
		float2 vTexCoord0 : TEXCOORD0,
		float4 vWeights : BLENDWEIGHT0,
		int4 vBoneIndices : BLENDINDICES0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
		float3 vNormalViewSpace;
		float4 vPosResultWorldSpace = float4(0.0f,0.0f,0.0f,0.0f);
		float3 vNormResultWorldSpace = float3(0.0f,0.0f,0.0f);
		float lastWeight = 0;
	  
		//Apply first bone
		lastWeight = vWeights[0];
		
		vPosResultWorldSpace += vWeights[0] * 
			mul(vPos, g_mPalette[vBoneIndices[0]]);
			
		vNormResultWorldSpace += vWeights[0] * 
			mul(vNormal, g_mPaletteTransposedInverse[vBoneIndices[0]]);
			
		if (vBoneIndices[3] > 1)
		{
			//Apply last bone
			lastWeight = 1.0f - lastWeight;
			vPosResultWorldSpace += lastWeight * 
				mul(vPos, g_mPalette[vBoneIndices[1]]);
				
			vNormResultWorldSpace += lastWeight * 
				mul(vNormal, g_mPaletteTransposedInverse[vBoneIndices[1]]);
		}
		normalize(vNormal);
			
		//Apply the camera and projection matrix to our position.
		Output.Position = mul(vPosResultWorldSpace, g_mWorldViewProjection);

		// Transform the normal from object space to world space 
		vNormalViewSpace = mul(vNormal, (float3x3)g_mWorldTransposedInverse); // normal (world space)
		vNormalViewSpace = mul(vNormalViewSpace, (float3x3)g_mView); // normal (view space)
		vNormalViewSpace = normalize(vNormalViewSpace);
	    
		//Add diffuse lighting contribution, and vertex colour contribution.
		float lambert = dot(g_lightDirection, vNormalViewSpace);
		lambert = saturate(lambert);
		Output.Diffuse.rgb = lambert;
		Output.Diffuse.rgb += 0.3f;
		Output.Diffuse.a = 1;
			
		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];
	    
		return Output;    
	}	
//--------------------------------------------------------------------------------------
// END [posNormTex1Weight4Idx4] Vertex Declaration
//--------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------
// [posNormColTex1Weight4Idx4] Vertex Declaration
//--------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------
	//posNormColTex1Weight4Idx4 Skinned Mesh Vertex Shaders
	//--------------------------------------------------------------------------------------
	POSCOLTEX1_VS_OUTPUT Skinned_posNormColTex1Weight4Idx4_VS(	
		float4 vPos : POSITION, 
		float2 vTexCoord0 : TEXCOORD0,
		float3 vColour : COLOR0,
		float4 vWeights : BLENDWEIGHT0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
	 
		// Transform the position from object space to homogeneous projection space
		Output.Position = mul(vPos, g_mWorldViewProjection);

		//No lights - just add vertex colour contribution.
		Output.Diffuse.rgb = vColour;
		Output.Diffuse.a = 1;
		
		//Offset and copy texture coordinates
		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];

		return Output;  
	}

	POSCOLTEX1_VS_OUTPUT Skinned_posNormColTex1Weight4Idx4_CM1_VS(	
		float4 vPos : POSITION, 
		float2 vTexCoord0 : TEXCOORD0,
		float4 vWeights : BLENDWEIGHT0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
	 
		// Transform the position from object space to homogeneous projection space
		Output.Position = mul(vPos, g_mWorldViewProjection);
	    
		//Display vertex weights
		Output.Diffuse.r = vWeights[0];
		Output.Diffuse.g = vWeights[1];
		Output.Diffuse.b = vWeights[2];
		Output.Diffuse.a = 1;
		
		//Offset and copy texture coordinates
		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];

		return Output;    
	}

	POSCOLTEX1_VS_OUTPUT Skinned_posNormColTex1Weight4Idx4_CM2_VS(	
		float4 vPos : POSITION, 
		float2 vTexCoord0 : TEXCOORD0,
		float4 vWeights : BLENDWEIGHT0)
	{
		POSCOLTEX1_VS_OUTPUT Output;

		// Transform the position from object space to homogeneous projection space
		Output.Position = mul(vPos, g_mWorldViewProjection);

		//Display diffuse UV set	
		Output.Diffuse.r = vTexCoord0[0];
		Output.Diffuse.g = 0;
		Output.Diffuse.b = vTexCoord0[1];
		Output.Diffuse.a = 1;
		
		//Offset and copy texture coordinates
		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];

		return Output;    
	}

	POSCOLTEX1_VS_OUTPUT Skinned_posNormColTex1Weight4Idx4_CM3_VS(	
		float4 vPos : POSITION, 
		float2 vTexCoord0 : TEXCOORD0,
		float4 vWeights : BLENDWEIGHT0)
	{
		POSCOLTEX1_VS_OUTPUT Output;

		// Transform the position from object space to homogeneous projection space
		Output.Position = mul(vPos, g_mWorldViewProjection);
	    
		//Display lightmap UV set
		Output.Diffuse.r = 0;
		Output.Diffuse.g = 0;
		Output.Diffuse.b = 0;
		Output.Diffuse.a = 1;
		
		//Offset and copy texture coordinates
		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];

		return Output;    
	}

	POSCOLTEX1_VS_OUTPUT Skinned_posNormColTex1Weight4Idx4_RTL1_VS(	
		float4 vPos : POSITION, 
		float3 vNormal : NORMAL,
		float2 vTexCoord0 : TEXCOORD0,
		float3 vColour : COLOR0,
		float4 vWeights : BLENDWEIGHT0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
		float3 vNormalViewSpace;
	    
		// Transform the position from object space to homogeneous projection space
		Output.Position = mul(vPos, g_mWorldViewProjection);
	    
		// Transform the normal from object space to world space 
		vNormalViewSpace = mul(vNormal, (float3x3)g_mWorldTransposedInverse); // normal (world space)
		vNormalViewSpace = mul(vNormalViewSpace, (float3x3)g_mView); // normal (view space)
		vNormalViewSpace = normalize(vNormalViewSpace);

		//Add diffuse lighting contribution, and vertex colour contribution.
		float lambert = dot(g_lightDirection, vNormalViewSpace);
		lambert = saturate(lambert);
		Output.Diffuse.rgb = vColour * lambert;
		Output.Diffuse.rgb += 0.3f;
		Output.Diffuse.a = 1;
		
		//Offset and copy texture coordinates
		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];

		return Output;    
	}
	//--------------------------------------------------------------------------------------
	// END [posNormColTex1Weight4Idx4] Skinned Mesh Vertex Shaders
	//--------------------------------------------------------------------------------------
	

	//--------------------------------------------------------------------------------------
	// [posNormColTex1Weight4Idx4] Deformed (Boned) Instance Shaders.
	//--------------------------------------------------------------------------------------
	POSCOLTEX1_VS_OUTPUT BonedInstance_posNormColTex1Weight4Idx4_VS( 
		float4 vPos : POSITION, 
		float3 vNormal : NORMAL,
		float2 vTexCoord0 : TEXCOORD0,
		float3 vColour : COLOR0,
		float4 vWeights : BLENDWEIGHT0,
		int4 vBoneIndices : BLENDINDICES0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
		float3 vNormalViewSpace;
		float4 vPosResultWorldSpace = float4(0.0f,0.0f,0.0f,0.0f);
		float3 vNormResultWorldSpace = float3(0.0f,0.0f,0.0f);
		float lastWeight = 0;
	  
		//Apply first bone
		lastWeight = vWeights[0];
		
		vPosResultWorldSpace += vWeights[0] * 
			mul(vPos, g_mPalette[vBoneIndices[0]]);
			
		vNormResultWorldSpace += vWeights[0] * 
			mul(vNormal, g_mPaletteTransposedInverse[vBoneIndices[0]]);
			
		if (vBoneIndices[3] > 1)
		{
			//Apply last bone
			lastWeight = 1.0f - lastWeight;
			vPosResultWorldSpace += lastWeight * 
				mul(vPos, g_mPalette[vBoneIndices[1]]);
				
			vNormResultWorldSpace += lastWeight * 
				mul(vNormal, g_mPaletteTransposedInverse[vBoneIndices[1]]);
		}
		normalize(vNormal);
			
		//Apply the camera and projection matrix to our position.
		Output.Position = mul(vPosResultWorldSpace, g_mWorldViewProjection);

		// Transform the normal from object space to world space 
		vNormalViewSpace = mul(vNormal, (float3x3)g_mWorldTransposedInverse); // normal (world space)
		vNormalViewSpace = mul(vNormalViewSpace, (float3x3)g_mView); // normal (view space)
		vNormalViewSpace = normalize(vNormalViewSpace);
	    
		//No lights - just add vertex colour contribution.
		Output.Diffuse.rgb = vColour;
		Output.Diffuse.a = 1;
			
		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];
	    
		return Output;    
	}
	
	POSCOLTEX1_VS_OUTPUT BonedInstance_posNormColTex1Weight4Idx4_CM1_VS( 
		float4 vPos : POSITION, 
		float2 vTexCoord0 : TEXCOORD0,
		float4 vWeights : BLENDWEIGHT0,
		int4 vBoneIndices : BLENDINDICES0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
		float4 vPosResultWorldSpace = float4(0.0f,0.0f,0.0f,0.0f);
		float lastWeight = 0;
	  
		//Apply first bone
		lastWeight = vWeights[0];
		
		vPosResultWorldSpace += vWeights[0] * 
			mul(vPos, g_mPalette[vBoneIndices[0]]);
			
		if (vBoneIndices[3] > 1)
		{
			//Apply last bone
			lastWeight = 1.0f - lastWeight;
			vPosResultWorldSpace += lastWeight * 
				mul(vPos, g_mPalette[vBoneIndices[1]]);
		}
			
		//Apply the camera and projection matrix to our position.
		Output.Position = mul(vPosResultWorldSpace, g_mWorldViewProjection);

		//Display vertex weights
		Output.Diffuse.r = vWeights[0];
		Output.Diffuse.g = vWeights[1];
		Output.Diffuse.b = vWeights[2];
		Output.Diffuse.a = 1;
		
		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];
	    
		return Output;    
	}
	
	POSCOLTEX1_VS_OUTPUT BonedInstance_posNormColTex1Weight4Idx4_CM2_VS( 
		float4 vPos : POSITION, 
		float2 vTexCoord0 : TEXCOORD0,
		float4 vWeights : BLENDWEIGHT0,
		int4 vBoneIndices : BLENDINDICES0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
		float4 vPosResultWorldSpace = float4(0.0f,0.0f,0.0f,0.0f);
		float lastWeight = 0;
	  
		//Apply first bone
		lastWeight = vWeights[0];
		
		vPosResultWorldSpace += vWeights[0] * 
			mul(vPos, g_mPalette[vBoneIndices[0]]);
			
		if (vBoneIndices[3] > 1)
		{
			//Apply last bone
			lastWeight = 1.0f - lastWeight;
			vPosResultWorldSpace += lastWeight * 
				mul(vPos, g_mPalette[vBoneIndices[1]]);
		}
			
		//Apply the camera and projection matrix to our position.
		Output.Position = mul(vPosResultWorldSpace, g_mWorldViewProjection);

		//Display diffuse UV set	
		Output.Diffuse.r = vTexCoord0[0];
		Output.Diffuse.g = 0;
		Output.Diffuse.b = vTexCoord0[1];
		Output.Diffuse.a = 1;
			
		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];
	    
		return Output;    
	}
	
	POSCOLTEX1_VS_OUTPUT BonedInstance_posNormColTex1Weight4Idx4_CM3_VS( 
		float4 vPos : POSITION, 
		float2 vTexCoord0 : TEXCOORD0,
		float4 vWeights : BLENDWEIGHT0,
		int4 vBoneIndices : BLENDINDICES0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
		float4 vPosResultWorldSpace = float4(0.0f,0.0f,0.0f,0.0f);
		float lastWeight = 0;
	  
		//Apply first bone
		lastWeight = vWeights[0];
		
		vPosResultWorldSpace += vWeights[0] * 
			mul(vPos, g_mPalette[vBoneIndices[0]]);
			
		if (vBoneIndices[3] > 1)
		{
			//Apply last bone
			lastWeight = 1.0f - lastWeight;
			vPosResultWorldSpace += lastWeight * 
				mul(vPos, g_mPalette[vBoneIndices[1]]);
		}
			
		//Apply the camera and projection matrix to our position.
		Output.Position = mul(vPosResultWorldSpace, g_mWorldViewProjection);
	   
		//Display lightmap UV set
		Output.Diffuse.r = 0;
		Output.Diffuse.g = 0;
		Output.Diffuse.b = 0;
		Output.Diffuse.a = 1;
			
		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];
	    
		return Output;    
	}
	
	POSCOLTEX1_VS_OUTPUT BonedInstance_posNormColTex1Weight4Idx4_RTL1_VS( 
		float4 vPos : POSITION, 
		float3 vNormal : NORMAL,
		float2 vTexCoord0 : TEXCOORD0,
		float3 vColour : COLOR0,
		float4 vWeights : BLENDWEIGHT0,
		int4 vBoneIndices : BLENDINDICES0)
	{
		POSCOLTEX1_VS_OUTPUT Output;
		float3 vNormalViewSpace;
		float4 vPosResultWorldSpace = float4(0.0f,0.0f,0.0f,0.0f);
		float3 vNormResultWorldSpace = float3(0.0f,0.0f,0.0f);
		float lastWeight = 0;
	  
		//Apply first bone
		lastWeight = vWeights[0];
		
		vPosResultWorldSpace += vWeights[0] * 
			mul(vPos, g_mPalette[vBoneIndices[0]]);
			
		vNormResultWorldSpace += vWeights[0] * 
			mul(vNormal, g_mPaletteTransposedInverse[vBoneIndices[0]]);
			
		if (vBoneIndices[3] > 1)
		{
			//Apply last bone
			lastWeight = 1.0f - lastWeight;
			vPosResultWorldSpace += lastWeight * 
				mul(vPos, g_mPalette[vBoneIndices[1]]);
				
			vNormResultWorldSpace += lastWeight * 
				mul(vNormal, g_mPaletteTransposedInverse[vBoneIndices[1]]);
		}
		normalize(vNormal);
			
		//Apply the camera and projection matrix to our position.
		Output.Position = mul(vPosResultWorldSpace, g_mWorldViewProjection);

		// Transform the normal from object space to world space 
		vNormalViewSpace = mul(vNormal, (float3x3)g_mWorldTransposedInverse); // normal (world space)
		vNormalViewSpace = mul(vNormalViewSpace, (float3x3)g_mView); // normal (view space)
		vNormalViewSpace = normalize(vNormalViewSpace);
	    
		//Add diffuse lighting contribution, and vertex colour contribution.
		float lambert = dot(g_lightDirection, vNormalViewSpace);
		lambert = saturate(lambert);
		Output.Diffuse.rgb = vColour * lambert;
		Output.Diffuse.rgb += 0.3f;
		Output.Diffuse.a = 1;
			
		Output.TextureUV0[0] = vTexCoord0[0]; 
		Output.TextureUV0[1] = vTexCoord0[1];
	    
		return Output;    
	}	
	
//--------------------------------------------------------------------------------------
// END [posNormColTex1Weight4Idx4] Vertex Declaration
//--------------------------------------------------------------------------------------

PS_OUTPUT CS1_PS(POSCOLTEX1_VS_OUTPUT In) 
{ 
    PS_OUTPUT Output;
    Output.RGBColor = SampleCropSheetColour(In.TextureUV0) * In.Diffuse;
    return Output;    
}

//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
//       color with diffuse material color
//--------------------------------------------------------------------------------------
PS_OUTPUT PS(POSCOLTEX1_VS_OUTPUT In) 
{ 
    PS_OUTPUT Output;
    Output.RGBColor = tex2D(ColourTextureSampler, In.TextureUV0) * In.Diffuse;
    return Output;    
}



//--------------------------------------------------------------------------------------
//Static techniques for meshes WITH vertex colours [posNormColTex1].
//--------------------------------------------------------------------------------------

technique Static_posNormColTex1
{
    pass P0
    {          
        VertexShader = compile vs_2_0 Static_posNormColTex1_VS();
        PixelShader  = compile ps_2_0 PS(); 
    }
}

technique Static_posNormColTex1_CS1
{
    pass P0
    {          
        VertexShader = compile vs_2_0 Static_posNormColTex1_VS();
        PixelShader  = compile ps_2_0 CS1_PS();
    }
}

technique Static_posNormColTex1_RTL1
{
    pass P0
    {          
        VertexShader = compile vs_2_0 Static_posNormColTex1_RTL1_VS();
        PixelShader  = compile ps_2_0 PS(); 
    }
}

technique Static_posNormColTex1_RTL1_CS1
{
    pass P0
    {          
        VertexShader = compile vs_2_0 Static_posNormColTex1_RTL1_VS();
        PixelShader  = compile ps_2_0 CS1_PS(); 
    }
}

technique Static_posNormColTex1_CM1
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex1_CM1_VS();
        PixelShader  = compile ps_2_0 PS();	
	}
}

technique Static_posNormColTex1_CM1_CS1
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex1_CM1_VS();
        PixelShader  = compile ps_2_0 CS1_PS();
	}
}

technique Static_posNormColTex1_CM2
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex1_CM2_VS();
        PixelShader  = compile ps_2_0 PS();	
	}
}

technique Static_posNormColTex1_CM2_CS1
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex1_CM2_VS();
        PixelShader  = compile ps_2_0 CS1_PS();
	}
}

technique Static_posNormColTex1_CM3
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex1_CM3_VS();
        PixelShader  = compile ps_2_0 PS();	
	}
}

technique Static_posNormColTex1_CM3_CS1
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex1_CM3_VS();
        PixelShader  = compile ps_2_0 CS1_PS();
	}
}

//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
//Component mode takes precidence.
technique Static_posNormColTex1_CM1_RTL1
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex1_CM1_VS();
        PixelShader  = compile ps_2_0 PS();	
	}
}

//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
//Component mode takes precidence.
technique Static_posNormColTex1_CM1_RTL1_CS1
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex1_CM1_VS();
        PixelShader  = compile ps_2_0 CS1_PS();
	}
}


//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
//Component mode takes precidence.
technique Static_posNormColTex1_CM2_RTL1
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex1_CM2_VS();
        PixelShader  = compile ps_2_0 PS();	
	}
}

technique Static_posNormColTex1_CM2_RTL1_CS1
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex1_CM2_VS();
        PixelShader  = compile ps_2_0 CS1_PS();
	}
}

//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
//Component mode takes precidence.
technique Static_posNormColTex1_CM3_RTL1
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex1_CM3_VS();
        PixelShader  = compile ps_2_0 PS();	
	}
}

//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
//Component mode takes precidence.
technique Static_posNormColTex1_CM3_RTL1_CS1
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex1_CM3_VS();
        PixelShader  = compile ps_2_0 CS1_PS();
	}
}

//--------------------------------------------------------------------------------------
// [posNormTex1] Vertex declaration
//--------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------
	// [posNormTex1] Static mesh techniques
	//--------------------------------------------------------------------------------------
	technique Static_posNormTex1
	{
		pass P0
		{          
			VertexShader = compile vs_2_0 Static_posNormTex1_VS();
			PixelShader  = compile ps_2_0 PS(); 
		}
	}
	
	technique Static_posNormTex1_CS1
	{
		pass P0
		{          
			VertexShader = compile vs_2_0 Static_posNormTex1_VS();
			PixelShader  = compile ps_2_0 CS1_PS(); 
		}
	}	
	
	technique Static_posNormTex1_RTL1
	{
		pass P0
		{          
			VertexShader = compile vs_2_0 Static_posNormTex1_RTL1_VS();
			PixelShader  = compile ps_2_0 PS(); 
		}
	}
	
	technique Static_posNormTex1_RTL1_CS1
	{
		pass P0
		{          
			VertexShader = compile vs_2_0 Static_posNormTex1_RTL1_VS();
			PixelShader  = compile ps_2_0 CS1_PS(); 
		}
	}
	

	technique Static_posNormTex1_CM1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex1_CM1_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}
	
	technique Static_posNormTex1_CM1_CS1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex1_CM1_VS();
			PixelShader  = compile ps_2_0 CS1_PS(); 
		}
	}
	
	technique Static_posNormTex1_CM2
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex1_CM2_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}
	
	technique Static_posNormTex1_CM2_CS1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex1_CM2_VS();
			PixelShader  = compile ps_2_0 CS1_PS(); 
		}
	}
	
	technique Static_posNormTex1_CM3
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex1_CM3_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}
	
	technique Static_posNormTex1_CM3_CS1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex1_CM3_VS();
			PixelShader  = compile ps_2_0 CS1_PS(); 
		}
	}
	

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Static_posNormTex1_CM1_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex1_CM1_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}
	
	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Static_posNormTex1_CM1_RTL1_CS1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex1_CM1_VS();
			PixelShader  = compile ps_2_0 CS1_PS(); 
		}
	}	

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Static_posNormTex1_CM2_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex1_CM2_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}
	
	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Static_posNormTex1_CM2_RTL1_CS1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex1_CM2_VS();
			PixelShader  = compile ps_2_0 CS1_PS(); 
		}
	}	

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Static_posNormTex1_CM3_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex1_CM3_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}
	
	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Static_posNormTex1_CM3_RTL1_CS1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex1_CM3_VS();
			PixelShader  = compile ps_2_0 CS1_PS(); 
		}
	}	

	//--------------------------------------------------------------------------------------
	// [posNormTex1] END static mesh techniques
	//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// END [posNormTex1] Vertex declaration
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// [posNormColTex1Weight4Idx4] Vertex declaration
//--------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------
	// Skinned mesh techniques
	//--------------------------------------------------------------------------------------
	technique Skinned_posNormColTex1Weight4Idx4
	{
		pass P0
		{          
			VertexShader = compile vs_2_0 Skinned_posNormColTex1Weight4Idx4_VS();
			PixelShader  = compile ps_2_0 PS(); 
		}
	}

	technique Skinned_posNormColTex1Weight4Idx4_RTL1
	{
		pass P0
		{          
			VertexShader = compile vs_2_0 Skinned_posNormColTex1Weight4Idx4_RTL1_VS();
			PixelShader  = compile ps_2_0 PS(); 
		}
	}

	technique Skinned_posNormColTex1Weight4Idx4_CM1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormColTex1Weight4Idx4_CM1_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	technique Skinned_posNormColTex1Weight4Idx4_CM2
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormColTex1Weight4Idx4_CM2_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	technique Skinned_posNormColTex1Weight4Idx4_CM3
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormColTex1Weight4Idx4_CM3_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Skinned_posNormColTex1Weight4Idx4_CM1_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormColTex1Weight4Idx4_CM1_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Skinned_posNormColTex1Weight4Idx4_CM2_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormColTex1Weight4Idx4_CM2_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Skinned_posNormColTex1Weight4Idx4_CM3_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormColTex1Weight4Idx4_CM3_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}
	//--------------------------------------------------------------------------------------
	// END Skinned mesh techniques
	//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// END [posNormColTex1Weight4Idx4] Vertex declaration
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// [posNormTex1Weight4Idx4] Vertex declaration
//--------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------
	// Skinned mesh techniques
	//--------------------------------------------------------------------------------------
	technique Skinned_posNormTex1Weight4Idx4
	{
		pass P0
		{          
			VertexShader = compile vs_2_0 Skinned_posNormTex1Weight4Idx4_VS();
			PixelShader  = compile ps_2_0 PS(); 
		}
	}

	technique Skinned_posNormTex1Weight4Idx4_RTL1
	{
		pass P0
		{          
			VertexShader = compile vs_2_0 Skinned_posNormTex1Weight4Idx4_RTL1_VS();
			PixelShader  = compile ps_2_0 PS(); 
		}
	}

	technique Skinned_posNormTex1Weight4Idx4_CM1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormTex1Weight4Idx4_CM1_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	technique Skinned_posNormTex1Weight4Idx4_CM2
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormTex1Weight4Idx4_CM2_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	technique Skinned_posNormTex1Weight4Idx4_CM3
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormTex1Weight4Idx4_CM3_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Skinned_posNormTex1Weight4Idx4_CM1_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormTex1Weight4Idx4_CM1_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Skinned_posNormTex1Weight4Idx4_CM2_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormTex1Weight4Idx4_CM2_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Skinned_posNormTex1Weight4Idx4_CM3_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormTex1Weight4Idx4_CM3_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}
	//--------------------------------------------------------------------------------------
	// END Skinned mesh techniques
	//--------------------------------------------------------------------------------------
	
	//--------------------------------------------------------------------------------------
	// Boned Instance mesh techniques
	//--------------------------------------------------------------------------------------
	technique BonedInstance_posNormTex1Weight4Idx4
	{
		pass P0
		{          
			VertexShader = compile vs_2_0 BonedInstance_posNormTex1Weight4Idx4_VS();
			PixelShader  = compile ps_2_0 PS(); 
		}
	}

	technique BonedInstance_posNormTex1Weight4Idx4_RTL1
	{
		pass P0
		{          
			VertexShader = compile vs_2_0 BonedInstance_posNormTex1Weight4Idx4_RTL1_VS();
			PixelShader  = compile ps_2_0 PS(); 
		}
	}

	technique BonedInstance_posNormTex1Weight4Idx4_CM1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 BonedInstance_posNormTex1Weight4Idx4_CM1_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	technique BonedInstance_posNormTex1Weight4Idx4_CM2
	{
		pass P0
		{
			VertexShader = compile vs_2_0 BonedInstance_posNormTex1Weight4Idx4_CM2_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	technique BonedInstance_posNormTex1Weight4Idx4_CM3
	{
		pass P0
		{
			VertexShader = compile vs_2_0 BonedInstance_posNormTex1Weight4Idx4_CM3_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique BonedInstance_posNormTex1Weight4Idx4_CM1_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 BonedInstance_posNormTex1Weight4Idx4_CM1_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique BonedInstance_posNormTex1Weight4Idx4_CM2_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 BonedInstance_posNormTex1Weight4Idx4_CM2_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique BonedInstance_posNormTex1Weight4Idx4_CM3_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 BonedInstance_posNormTex1Weight4Idx4_CM3_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}
	//--------------------------------------------------------------------------------------
	// END Boned Instance mesh techniques
	//--------------------------------------------------------------------------------------	

//--------------------------------------------------------------------------------------
// END [posNormTex1Weight4Idx4] Vertex declaration
//--------------------------------------------------------------------------------------













//--------------------------------------------------------------------------------------
//Static techniques for meshes WITH vertex colours [posNormColTex2].
//--------------------------------------------------------------------------------------

technique Static_posNormColTex2
{
    pass P0
    {          
        VertexShader = compile vs_2_0 Static_posNormColTex1_VS();
        PixelShader  = compile ps_2_0 PS(); 
    }
}

technique Static_posNormColTex2_CS1
{
    pass P0
    {          
        VertexShader = compile vs_2_0 Static_posNormColTex1_VS();
        PixelShader  = compile ps_2_0 CS1_PS();
    }
}

technique Static_posNormColTex2_RTL1
{
    pass P0
    {          
        VertexShader = compile vs_2_0 Static_posNormColTex1_RTL1_VS();
        PixelShader  = compile ps_2_0 PS(); 
    }
}

technique Static_posNormColTex2_RTL1_CS1
{
    pass P0
    {          
        VertexShader = compile vs_2_0 Static_posNormColTex1_RTL1_VS();
        PixelShader  = compile ps_2_0 CS1_PS(); 
    }
}

technique Static_posNormColTex2_CM1
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex1_CM1_VS();
        PixelShader  = compile ps_2_0 PS();	
	}
}

technique Static_posNormColTex2_CM1_CS1
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex1_CM1_VS();
        PixelShader  = compile ps_2_0 CS1_PS();
	}
}

technique Static_posNormColTex2_CM2
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex1_CM2_VS();
        PixelShader  = compile ps_2_0 PS();	
	}
}

technique Static_posNormColTex2_CM2_CS1
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex1_CM2_VS();
        PixelShader  = compile ps_2_0 CS1_PS();
	}
}

technique Static_posNormColTex2_CM3
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex2_CM3_VS();
        PixelShader  = compile ps_2_0 PS();	
	}
}

technique Static_posNormColTex2_CM3_CS1
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex2_CM3_VS();
        PixelShader  = compile ps_2_0 CS1_PS();
	}
}

//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
//Component mode takes precidence.
technique Static_posNormColTex2_CM1_RTL1
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex1_CM1_VS();
        PixelShader  = compile ps_2_0 PS();	
	}
}

//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
//Component mode takes precidence.
technique Static_posNormColTex2_CM1_RTL1_CS1
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex1_CM1_VS();
        PixelShader  = compile ps_2_0 CS1_PS();
	}
}


//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
//Component mode takes precidence.
technique Static_posNormColTex2_CM2_RTL1
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex1_CM2_VS();
        PixelShader  = compile ps_2_0 PS();	
	}
}

technique Static_posNormColTex2_CM2_RTL1_CS1
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex1_CM2_VS();
        PixelShader  = compile ps_2_0 CS1_PS();
	}
}

//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
//Component mode takes precidence.
technique Static_posNormColTex2_CM3_RTL1
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex2_CM3_VS();
        PixelShader  = compile ps_2_0 PS();	
	}
}

//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
//Component mode takes precidence.
technique Static_posNormColTex2_CM3_RTL1_CS1
{
	pass P0
	{
        VertexShader = compile vs_2_0 Static_posNormTex2_CM3_VS();
        PixelShader  = compile ps_2_0 CS1_PS();
	}
}

//--------------------------------------------------------------------------------------
// [posNormTex2] Vertex declaration
//--------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------
	// [posNormTex2] Static mesh techniques
	//--------------------------------------------------------------------------------------
	technique Static_posNormTex2
	{
		pass P0
		{          
			VertexShader = compile vs_2_0 Static_posNormTex1_VS();
			PixelShader  = compile ps_2_0 PS(); 
		}
	}
	
	technique Static_posNormTex2_CS1
	{
		pass P0
		{          
			VertexShader = compile vs_2_0 Static_posNormTex1_VS();
			PixelShader  = compile ps_2_0 CS1_PS(); 
		}
	}	
	
	technique Static_posNormTex2_RTL1
	{
		pass P0
		{          
			VertexShader = compile vs_2_0 Static_posNormTex1_RTL1_VS();
			PixelShader  = compile ps_2_0 PS(); 
		}
	}
	
	technique Static_posNormTex2_RTL1_CS1
	{
		pass P0
		{          
			VertexShader = compile vs_2_0 Static_posNormTex1_RTL1_VS();
			PixelShader  = compile ps_2_0 CS1_PS(); 
		}
	}
	

	technique Static_posNormTex2_CM1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex1_CM1_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}
	
	technique Static_posNormTex2_CM1_CS1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex1_CM1_VS();
			PixelShader  = compile ps_2_0 CS1_PS(); 
		}
	}
	
	technique Static_posNormTex2_CM2
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex1_CM2_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}
	
	technique Static_posNormTex2_CM2_CS1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex1_CM2_VS();
			PixelShader  = compile ps_2_0 CS1_PS(); 
		}
	}
	
	technique Static_posNormTex2_CM3
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex2_CM3_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}
	
	technique Static_posNormTex2_CM3_CS1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex2_CM3_VS();
			PixelShader  = compile ps_2_0 CS1_PS(); 
		}
	}
	

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Static_posNormTex2_CM1_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex1_CM1_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}
	
	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Static_posNormTex2_CM1_RTL1_CS1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex1_CM1_VS();
			PixelShader  = compile ps_2_0 CS1_PS(); 
		}
	}	

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Static_posNormTex2_CM2_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex1_CM2_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}
	
	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Static_posNormTex2_CM2_RTL1_CS1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex1_CM2_VS();
			PixelShader  = compile ps_2_0 CS1_PS(); 
		}
	}	

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Static_posNormTex2_CM3_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex2_CM3_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}
	
	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Static_posNormTex2_CM3_RTL1_CS1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Static_posNormTex2_CM3_VS();
			PixelShader  = compile ps_2_0 CS1_PS(); 
		}
	}	

	//--------------------------------------------------------------------------------------
	// [posNormTex2] END static mesh techniques
	//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// END [posNormTex2] Vertex declaration
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// [posNormColTex2Weight4Idx4] Vertex declaration
//--------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------
	// Skinned mesh techniques
	//--------------------------------------------------------------------------------------
	technique Skinned_posNormColTex2Weight4Idx4
	{
		pass P0
		{          
			VertexShader = compile vs_2_0 Skinned_posNormColTex1Weight4Idx4_VS();
			PixelShader  = compile ps_2_0 PS(); 
		}
	}

	technique Skinned_posNormColTex2Weight4Idx4_RTL1
	{
		pass P0
		{          
			VertexShader = compile vs_2_0 Skinned_posNormColTex1Weight4Idx4_RTL1_VS();
			PixelShader  = compile ps_2_0 PS(); 
		}
	}

	technique Skinned_posNormColTex2Weight4Idx4_CM1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormColTex1Weight4Idx4_CM1_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	technique Skinned_posNormColTex2Weight4Idx4_CM2
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormColTex1Weight4Idx4_CM2_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	technique Skinned_posNormColTex2Weight4Idx4_CM3
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormTex2Weight4Idx4_CM3_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Skinned_posNormColTex2Weight4Idx4_CM1_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormColTex1Weight4Idx4_CM1_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Skinned_posNormColTex2Weight4Idx4_CM2_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormColTex1Weight4Idx4_CM2_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Skinned_posNormColTex2Weight4Idx4_CM3_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormTex2Weight4Idx4_CM3_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}
	//--------------------------------------------------------------------------------------
	// END Skinned mesh techniques
	//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// END [posNormColTex2Weight4Idx4] Vertex declaration
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// [posNormTex2Weight4Idx4] Vertex declaration
//--------------------------------------------------------------------------------------

	//--------------------------------------------------------------------------------------
	// Skinned mesh techniques
	//--------------------------------------------------------------------------------------
	technique Skinned_posNormTex2Weight4Idx4
	{
		pass P0
		{          
			VertexShader = compile vs_2_0 Skinned_posNormTex1Weight4Idx4_VS();
			PixelShader  = compile ps_2_0 PS(); 
		}
	}

	technique Skinned_posNormTex2Weight4Idx4_RTL1
	{
		pass P0
		{          
			VertexShader = compile vs_2_0 Skinned_posNormTex1Weight4Idx4_RTL1_VS();
			PixelShader  = compile ps_2_0 PS(); 
		}
	}

	technique Skinned_posNormTex2Weight4Idx4_CM1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormTex1Weight4Idx4_CM1_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	technique Skinned_posNormTex2Weight4Idx4_CM2
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormTex1Weight4Idx4_CM2_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	technique Skinned_posNormTex2Weight4Idx4_CM3
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormTex2Weight4Idx4_CM3_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Skinned_posNormTex2Weight4Idx4_CM1_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormTex1Weight4Idx4_CM1_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Skinned_posNormTex2Weight4Idx4_CM2_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormTex1Weight4Idx4_CM2_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique Skinned_posNormTex2Weight4Idx4_CM3_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 Skinned_posNormTex2Weight4Idx4_CM3_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}
	//--------------------------------------------------------------------------------------
	// END Skinned mesh techniques
	//--------------------------------------------------------------------------------------
	
	//--------------------------------------------------------------------------------------
	// Boned Instance mesh techniques
	//--------------------------------------------------------------------------------------
	technique BonedInstance_posNormTex2Weight4Idx4
	{
		pass P0
		{          
			VertexShader = compile vs_2_0 BonedInstance_posNormTex1Weight4Idx4_VS();
			PixelShader  = compile ps_2_0 PS(); 
		}
	}

	technique BonedInstance_posNormTex2Weight4Idx4_RTL1
	{
		pass P0
		{          
			VertexShader = compile vs_2_0 BonedInstance_posNormTex1Weight4Idx4_RTL1_VS();
			PixelShader  = compile ps_2_0 PS(); 
		}
	}

	technique BonedInstance_posNormTex2Weight4Idx4_CM1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 BonedInstance_posNormTex1Weight4Idx4_CM1_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	technique BonedInstance_posNormTex2Weight4Idx4_CM2
	{
		pass P0
		{
			VertexShader = compile vs_2_0 BonedInstance_posNormTex1Weight4Idx4_CM2_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	technique BonedInstance_posNormTex2Weight4Idx4_CM3
	{
		pass P0
		{
			VertexShader = compile vs_2_0 BonedInstance_posNormTex2Weight4Idx4_CM3_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique BonedInstance_posNormTex2Weight4Idx4_CM1_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 BonedInstance_posNormTex1Weight4Idx4_CM1_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique BonedInstance_posNormTex2Weight4Idx4_CM2_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 BonedInstance_posNormTex1Weight4Idx4_CM2_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}

	//Component modes other than 0 and Real-time-lighting are mutually exclusive. 
	//Component mode takes precidence.
	technique BonedInstance_posNormTex2Weight4Idx4_CM3_RTL1
	{
		pass P0
		{
			VertexShader = compile vs_2_0 BonedInstance_posNormTex2Weight4Idx4_CM3_VS();
			PixelShader  = compile ps_2_0 PS();	
		}
	}
	//--------------------------------------------------------------------------------------
	// END Boned Instance mesh techniques
	//--------------------------------------------------------------------------------------	

//--------------------------------------------------------------------------------------
// END [posNormTex2Weight4Idx4] Vertex declaration
//--------------------------------------------------------------------------------------
