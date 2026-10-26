#ifndef SHADER_GLOBALS_H
#define SHADER_GLOBALS_H

#include "Shader/Constants.h"

// shared constants
#if !defined(CFG_CONVERTER)
shared float4x4	g_mWorld			: register(CCV_DISPLAY_WORLD_MATRIX)	< string Source = "<WorldMatrix>"; string InstanceMode = "InstanceOnly"; >;
#endif

#if defined(CFG_CONVERTER)
#define g_mViewProj mul(g_mView, g_mProj)
#else
shared float4x4	g_mViewProj			: register(CCV_DISPLAY_VIEWPROJ_MATRIX);
#endif

#if defined(CFG_CONVERTER)
float4x4		g_mWorld			: WORLD;
float4x4		g_mView				: VIEW;
float4x4		g_mProj				: PROJECTION;
float4x4		g_mViewInverse		: VIEWINVERSE;
static float4	g_vViewPosition		= float4(g_mViewInverse[3].x, g_mViewInverse[3].y, g_mViewInverse[3].z, 0.0f);
#else
shared float4x4	g_mView				: register(CCV_DISPLAY_VIEW_MATRIX);
shared float4x4	g_mProj				: register(CCV_DISPLAY_PROJ_MATRIX);
shared float4	g_vViewPosition		: register(CCV_DISPLAY_VIEW_POSITION);
#endif

#if defined(CFG_CONVERTER)
float			g_fTime				: TIME;
#else
shared float	g_fTime				: register(CCP_SHADER_TIME);
#endif

#ifndef CFG_SPEED
#if !defined(CFG_CONVERTER)
shared float4	g_vDebugColour		: register(CCP_SHADER_DEBUG_COLOUR);
#endif
#endif

#ifndef CFG_SPEED
#endif

// shared defines
#define			SQRT2		1.4142135623730950488016887242097h
#define			SQRT3		1.7320508075688772935274463415059h
#define			SQRT6		2.4494897427831780981972840747059h
#define			SQRT2OVER3	0.81649658092772603273242802490196h
#define			PI			3.141592654h

#endif // SHADER_GLOBALS_H
