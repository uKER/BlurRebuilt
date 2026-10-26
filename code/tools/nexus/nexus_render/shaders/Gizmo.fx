//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

//General transforms
float4x4 g_mWorld;             // World matrix for object
float4x4 g_mViewProjection;    // View * Projection matrix

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 position		: POSITION;   // vertex position 
    float4 colour		: COLOR0;
};


VS_OUTPUT GizmoVS(
	float4 vPos : POSITION, 
	float4 vColour : COLOR0)
{
    VS_OUTPUT Output;
	//Move position into projection space.
	Output.position = mul(vPos, g_mWorld);
    Output.position = mul(Output.position, g_mViewProjection);
    Output.colour = vColour;
    return Output;    
}

//--------------------------------------------------------------------------------------
// Pixel shader output structure
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
    float4 colour : COLOR0;  // Pixel color    
};


//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
//       color with diffuse material color
//--------------------------------------------------------------------------------------
PS_OUTPUT GizmoPS(VS_OUTPUT input) 
{ 
    PS_OUTPUT output;
    output.colour = input.colour;
    return output;    
}

//--------------------------------------------------------------------------------------
// Renders scene to render target
//--------------------------------------------------------------------------------------

technique Gizmo
{
    pass P0
    {          
        VertexShader = compile vs_2_0 GizmoVS();
        PixelShader  = compile ps_2_0 GizmoPS(); 
    }
}



