//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

//General transforms
float4x4 g_mWorldViewProjection;    // World * View * Projection matrix
float4x4 g_mView;

//Bone transforms for deformed instances.
float4x4 g_mPalette[2];

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Position   : POSITION;   // vertex position 
};


VS_OUTPUT StaticVS(	
	float4 vPos : POSITION)
{
    VS_OUTPUT Output;
    // Transform the position from object space to homogeneous projection space
    Output.Position = mul(vPos, g_mWorldViewProjection);
    return Output;    
}

//--------------------------------------------------------------------------------------
// This is the vertex shader for skinned meshes
//--------------------------------------------------------------------------------------
VS_OUTPUT SkinnedVS( 
	float4 vPos : POSITION, 
	float4 vWeights : BLENDWEIGHT0)
{
    VS_OUTPUT Output;
    // Transform the position from object space to homogeneous projection space
    Output.Position = mul(vPos, g_mWorldViewProjection);
    return Output;    
}

//--------------------------------------------------------------------------------------
// This is the vertex shader for deformed instances, such as barriers.
//--------------------------------------------------------------------------------------
VS_OUTPUT BonedInstanceVS( 
	float4 vPos : POSITION, 
	float4 vWeights : BLENDWEIGHT0,
	int4 vBoneIndices : BLENDINDICES0)
{
    VS_OUTPUT Output;
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
    return Output;    
}

//--------------------------------------------------------------------------------------
// Pixel shader output structure
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
    float4 RGBColor : COLOR0;  // Pixel color    
};


PS_OUTPUT SelectionPS(VS_OUTPUT In) 
{ 
    PS_OUTPUT Output;
    Output.RGBColor = float4(0.262f,1.0f,0.639f, 1.0f);
    return Output;    
}

technique SelectionBonedInstance
{
    pass P0
    {          
        VertexShader = compile vs_2_0 BonedInstanceVS();
        PixelShader  = compile ps_2_0 SelectionPS(); 
    }
}

technique SelectionSkinned
{
    pass P0
    {          
        VertexShader = compile vs_2_0 SkinnedVS();
        PixelShader  = compile ps_2_0 SelectionPS(); 
    }
}

technique SelectionStatic
{
    pass P0
    {          
        VertexShader = compile vs_2_0 StaticVS();
        PixelShader  = compile ps_2_0 SelectionPS(); 
    }
}



