#ifndef AMAX_SHADERS_COMMON_LODFADE
#define AMAX_SHADERS_COMMON_LODFADE

#if !defined(DISABLE_LODFADE)
#ifndef LODFADE_IN_VERTEX_CONSTANT
shared real4 g_lodFade : register(CCP_LOD_FADE) < string Source = "<LodFade>"; string InstanceMode = "ResetMtl"; >;
shared real4 g_lodDistanceScale : register(CCP_LOD_DISTANCE_SCALE);
#else
shared float4 g_lodFade : register(CCV_LOD_FADE) < string Source = "<LodFade>"; string InstanceMode = "ResetMtl"; >;
shared float4 g_lodDistanceScale : register(CCV_LOD_DISTANCE_SCALE);
#endif
#endif

#endif
