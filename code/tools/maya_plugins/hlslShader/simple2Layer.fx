//
// Simple Shader
//

int sas : SasGlobal
<
	bool SasUiVisible = false;
	int3 SasVersion= {1,1,0};
>;

// transformations
float4x4 World
<
	bool SasUiVisible = false;
	string SasBindAddress= "Sas.Skeleton.MeshToJointToWorld[0]";
>;

float4x4 View
<
	bool SasUiVisible = false;
	string SasBindAddress= "Sas.Camera.WorldToView";
>;

float4x4 Projection
<
	bool SasUiVisible = false;
	string SasBindAddress= "Sas.Camera.Projection";
>;

//textures
texture Diff0 
< 
	string SasUiLabel = "Diffuse Layer0";
	string SasUiControl= "FilePicker";
>;

texture Diff1 
< 
	string SasUiLabel = "Diffuse Layer1";
	string SasUiControl= "FilePicker";
>;

//variables
float  ScaleU
<
    string SasUiLabel = "U Scale Factor";
    string SasUiControl = "Slider"; 
    float SasUiMin = 0.0f; 
    float SasUiMax = 4.0f; 
    int SasUiSteps = 31;
>   = 1.0f;                        

float  ScaleV
<
    string SasUiLabel = "V Scale Factor";
    string SasUiControl = "Slider"; 
    float SasUiMin = 0.0f; 
    float SasUiMax = 4.0f; 
    int SasUiSteps = 31;
>   = 1.0f;                         

//-----------------------------------------------------------------------------
// Vertex shader input structure
//-----------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Position : POSITION;
    float3 Normal   : NORMAL;
    float3 Tex0 : TEXCOORD0;
    float3 Tex1 : TEXCOORD1;               
    float4 Color  : COLOR0;
};

//-----------------------------------------------------------------------------
// Vertex shader output structure
//-----------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos  : POSITION;
    float3 Tex0 : TEXCOORD0;               
    float3 Tex1 : TEXCOORD1;               
    float4 Diff : COLOR0;
};

VS_OUTPUT VS(VS_INPUT In)
{
    VS_OUTPUT Out = (VS_OUTPUT)0;

    float3 P = mul(In.Position, World);    // position (view space)
    P = mul(float4(P, 1), View);    // position (view space)
       
    Out.Pos  = mul(float4(P, 1), Projection);             // position (projected)
    Out.Diff = In.Color; 
    Out.Tex0 = In.Tex0;        
	Out.Tex1 = In.Tex1; 

    return Out;
}

// samplers
sampler DiffuseTex0Samp<bool SasUiVisible = false;> = sampler_state 
{
    texture = <Diff0>;
    AddressU  = WRAP;		
    AddressV  = WRAP;
    AddressW  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

sampler DiffuseTex1Samp<bool SasUiVisible = false;> = sampler_state 
{
    texture = <Diff1>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    AddressW  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

float4 PS_Blend(VS_OUTPUT In) : COLOR
{   
    float4 Color;
    
    float2 UVScale;
    UVScale.x = ScaleU;
    UVScale.y = ScaleV;
    
	float4 tex0 = tex2D(DiffuseTex0Samp, In.Tex0.xy*UVScale);
	float4 tex1 = tex2D(DiffuseTex1Samp, In.Tex1.xy*UVScale);
	float alpha = tex1.a * In.Diff.w;
	Color.xyz = lerp(tex0.xyz, tex1.xyz, alpha) * In.Diff.xyz;
	Color.w = tex0.a;

    return Color;
}  

float4 PS_Layer0(VS_OUTPUT In) : COLOR
{   
    float4 Color;
    
    float2 UVScale;
    UVScale.x = ScaleU;
    UVScale.y = ScaleV;
    
	float4 tex = tex2D(DiffuseTex0Samp, In.Tex0.xy*UVScale);
	Color.xyz = tex.xyz * In.Diff.xyz;
	Color.w = tex.a;
	
    return Color;
}  

float4 PS_Layer1(VS_OUTPUT In) : COLOR
{   
    float4 Color;
    
    float2 UVScale;
    UVScale.x = ScaleU;
    UVScale.y = ScaleV;
    
	float4 tex = tex2D(DiffuseTex1Samp, In.Tex1.xy*UVScale);
	Color.xyz = tex.xyz * In.Diff.xyz;
	Color.w = tex.a;
	
    return Color;
}  

float4 PS_NoTexture(VS_OUTPUT In) : COLOR
{   
    float4 Color;
    Color = In.Diff;
    return Color;
}  


technique Blend
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS_Blend();
    }
}

technique Layer0
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS_Layer0();
    }
}

technique Layer1
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS_Layer1();
    }
}

technique No_Texture
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS_NoTexture();
    }
}

