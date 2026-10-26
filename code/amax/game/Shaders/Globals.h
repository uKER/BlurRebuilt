#ifndef AMAX_SHADERS_COMMON_GLOBALS_H
#define AMAX_SHADERS_COMMON_GLOBALS_H

#include <Constants.h>

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
float4			g_vNearFar			= float4(0.1f, 500.0f, 0.0f, 0.0f);
#else
shared float4	g_vNearFar			: register(CCV_NEARFAR);
#endif

#if defined(CFG_CONVERTER)
shared float4x4	g_mViewPS			: VIEW;
static float4	g_vViewPositionPS	= float4(g_mViewInverse[3].x, g_mViewInverse[3].y, g_mViewInverse[3].z, 0.0f);
#else
shared real4	g_vViewMatrix0		: register(CCP_VIEW_MATRIX);
shared real4	g_vViewMatrix1		: register(CCP_VIEW_MATRIX_1);
shared real4	g_vViewMatrix2		: register(CCP_VIEW_MATRIX_2);
shared real4	g_vViewMatrix3		: register(CCP_VIEW_MATRIX_3);
shared real4	g_vViewProjMatrix0	: register(CCP_VIEWPROJ_MATRIX);
shared real4	g_vViewProjMatrix1	: register(CCP_VIEWPROJ_MATRIX_1);
shared real4	g_vViewProjMatrix2	: register(CCP_VIEWPROJ_MATRIX_2);
shared real4	g_vViewProjMatrix3	: register(CCP_VIEWPROJ_MATRIX_3);
shared float4	g_vViewPositionPS	: register(CCP_VIEW_POSITION);
#endif

#if defined(CFG_CONVERTER)
float			g_fTime				: TIME;
#else
shared float	g_fTime				: register(CCP_SHADER_TIME);
#endif

#if defined(CFG_CONVERTER)
float4			g_vDepthScale		= 0.0f;
#else
shared float4	g_vDepthScale		: register(CCP_SHADER_DEPTH_SCALE);
#endif

#if defined(CFG_CONVERTER)
float4			g_vHalfPixelOffset	= 0.0f;
#else
shared float4	g_vHalfPixelOffset	: register(CCP_SHADER_HALFPIXELOFFSET);
#endif

shared float4x4	g_mWind[4]			: register(CCV_WIND_MATRIX0);
shared float4x4	g_mLeaf[4]			: register(CCV_LEAF_MATRIX0);

#if !defined(CFG_CONVERTER)
shared real4	g_vDebugColour		: register(CCP_SHADER_DEBUG_COLOUR);
#endif

#ifndef CFG_SPEED
#endif

// shared defines
#define			SQRT2		1.4142135623730950488016887242097h
#define			SQRT3		1.7320508075688772935274463415059h
#define			SQRT6		2.4494897427831780981972840747059h
#define			SQRT2OVER3	0.81649658092772603273242802490196h
#define			PI			3.141592654h

#endif // AMAX_SHADERS_COMMON_GLOBALS_H
