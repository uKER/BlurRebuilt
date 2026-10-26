#ifndef __FX_SHADERS_GLOBAL_H__
#define __FX_SHADERS_GLOBAL_H__

// this file defines the most common global shared parameters

shared float4x4 g_WorldMatrix :  register(c0) ;
#ifdef MAYA
#define g_ViewProjectionMatrix mul(g_ViewMatrix, g_ProjectionMatrix)
#else
shared float4x4 g_ViewProjectionMatrix : register(c4) ;
#endif
shared float4x4 g_ViewMatrix : register(c8) ;
shared float4x4 g_ProjectionMatrix : register(c12) ;

#endif
