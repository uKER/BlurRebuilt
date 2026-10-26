// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE		: The HLSL vertex and pixel shader programs required for the Xenon

#ifndef SHADERS_H
#define SHADERS_H

static const char* vertexShaderProgram =
" float4x4 matWorld : register(c0);            "  
" float4x4 matView : register(c4);             "
" float4x4 matProj : register(c8);             "
"                                              "  
" struct VS_IN                                 "  
"                                              "  
" {                                            " 
"     float4 ObjPos   : POSITION;              "  // Object space position 
"     float4 Color    : COLOR;                 "  // Vertex color                 
" };                                           " 
"                                              " 
" struct VS_OUT                                " 
" {                                            " 
"     float4 ProjPos  : POSITION;              "  // Projected space position 
"     float4 Color    : COLOR;                 "  
" };                                           "  
"                                              "  
" VS_OUT main( VS_IN In )                      "  
" {                                            "  
"     VS_OUT Out;                              "
"     float4x4 matWVP;                         "
"     matWVP = mul(matView, matWorld);         "
"     matWVP = mul(matProj, matWVP);           "
"     Out.ProjPos = mul( matWVP, In.ObjPos );  "  // Transform vertex into
"     Out.Color = In.Color;                    "  // Projected space and 
"     return Out;                              "  // Transfer color
" }                                            "
"                                              ";
/*
" technique Simple                             "
" {                                            "
"	pass P0                                    "
"	{                                          "
"		VertexShader = compile vs_2_0 main();  "
"	}                                          "
" }                                            ";
*/
static const char* pixelShaderProgram =
" struct PS_IN                                 "
" {                                            "
"     float4 Color : COLOR;                    "  // Interpolated color from                      
" };                                           "  // the vertex shader
"                                              "  
" float4 main( PS_IN In ) : COLOR              "  
" {                                            "  
"     return In.Color;                         "  // Output color
" }                                            "; 

#endif // SHADERS_H
