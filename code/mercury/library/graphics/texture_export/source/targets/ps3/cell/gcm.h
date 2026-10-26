/*   SCE CONFIDENTIAL                                       */
/*   PLAYSTATION(R)3 Programmer Tool Runtime Library 091.006 */
/*   Copyright (C) 2006 Sony Computer Entertainment Inc.    */
/*   All Rights Reserved.                                   */

#ifndef __CELL_GCM_H__
#define __CELL_GCM_H__

#ifndef __CELL_CELL_ERROR_H__
#include <cell/error.h>	// target/common/include/cell/error.h
#endif

/*
CELL_ERROR_FACILITY_GFX		0x021
libgcm			: 0x8021_0001 - 0x8021_00ff
*/
#define CELL_GCM_MAKE_ERROR(status) \
	CELL_ERROR_MAKE_ERROR(CELL_ERROR_FACILITY_GFX, status)

#define CELL_GCM_ERROR_FAILURE		CELL_GCM_MAKE_ERROR(0x00ff)

// Treyarch
#define __restrict
typedef int system_time_t;

#ifdef CELL_GCM_DEBUG
//#include <stdio.h>
#endif


#ifndef _STDINT
#include <stdint.h>
#endif

#ifndef _STDBOOL
//#include <stdbool.h>
#endif

#ifndef _cg_h
#include <Cg/cg.h>
#endif

//#include <alloca.h>
//#include <sys/timer.h>
#include <cell/gcm/gcm_macros.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
	//	Enable
	CELL_GCM_FALSE	= (0),
	CELL_GCM_TRUE	= (1),

	// Location
	CELL_GCM_LOCATION_LOCAL	= (0),
	CELL_GCM_LOCATION_MAIN	= (1),


	// SetSurface
	CELL_GCM_SURFACE_X1R5G5B5_Z1R5G5B5	= (1),
	CELL_GCM_SURFACE_X1R5G5B5_O1R5G5B5	= (2),
	CELL_GCM_SURFACE_R5G6B5				= (3),
	CELL_GCM_SURFACE_X8R8G8B8_Z8R8G8B8	= (4),
	CELL_GCM_SURFACE_X8R8G8B8_O8R8G8B8	= (5),
	CELL_GCM_SURFACE_A8R8G8B8			= (8),
	CELL_GCM_SURFACE_B8					= (9),
	CELL_GCM_SURFACE_G8B8				= (10),
	CELL_GCM_SURFACE_F_W16Z16Y16X16		= (11),
	CELL_GCM_SURFACE_F_W32Z32Y32X32		= (12),
	CELL_GCM_SURFACE_F_X32				= (13),
	CELL_GCM_SURFACE_X8B8G8R8_Z8B8G8R8	= (14),
	CELL_GCM_SURFACE_X8B8G8R8_O8B8G8R8	= (15),
	CELL_GCM_SURFACE_A8B8G8R8			= (16),

	CELL_GCM_SURFACE_Z16	= (1),
	CELL_GCM_SURFACE_Z24S8	= (2),

	CELL_GCM_SURFACE_PITCH		= (1),
	CELL_GCM_SURFACE_SWIZZLE	= (2),

	CELL_GCM_SURFACE_CENTER_1				= (0),
	CELL_GCM_SURFACE_DIAGONAL_CENTERED_2	= (3),
	CELL_GCM_SURFACE_SQUARE_CENTERED_4		= (4),
	CELL_GCM_SURFACE_SQUARE_ROTATED_4		= (5),

	CELL_GCM_SURFACE_TARGET_NONE	= (0),
	CELL_GCM_SURFACE_TARGET_0		= (1),
	CELL_GCM_SURFACE_TARGET_1		= (2),
	CELL_GCM_SURFACE_TARGET_MRT1	= (0x13),
	CELL_GCM_SURFACE_TARGET_MRT2	= (0x17),
	CELL_GCM_SURFACE_TARGET_MRT3	= (0x1f),

	// SetClearSurface
	CELL_GCM_CLEAR_Z	= (1<<0),
	CELL_GCM_CLEAR_S	= (1<<1),
	CELL_GCM_CLEAR_R	= (1<<4),
	CELL_GCM_CLEAR_G	= (1<<5),
	CELL_GCM_CLEAR_B	= (1<<6),
	CELL_GCM_CLEAR_A	= (1<<7),
	CELL_GCM_CLEAR_M	= (0xf3),

	// SetVertexDataArray
	CELL_GCM_VERTEX_S1		= (1),
	CELL_GCM_VERTEX_F		= (2),
	CELL_GCM_VERTEX_SF		= (3),
	CELL_GCM_VERTEX_UB		= (4),
	CELL_GCM_VERTEX_S32K	= (5),
	CELL_GCM_VERTEX_CMP		= (6),
	CELL_GCM_VERTEX_UB256	= (7),

	CELL_GCM_VERTEX_S16_NR		            = (1),
	CELL_GCM_VERTEX_F32		                = (2),
	CELL_GCM_VERTEX_F16		                = (3),
	CELL_GCM_VERTEX_U8_NR		            = (4),
	CELL_GCM_VERTEX_S16_UN	                = (5),
	CELL_GCM_VERTEX_S11_11_10_NR		    = (6),
	CELL_GCM_VERTEX_U8_UN	                = (7),

	// SetTexture
	CELL_GCM_TEXTURE_B8						= (0x81),
	CELL_GCM_TEXTURE_A1R5G5B5				= (0x82),
	CELL_GCM_TEXTURE_A4R4G4B4				= (0x83),
	CELL_GCM_TEXTURE_R5G6B5					= (0x84),
	CELL_GCM_TEXTURE_A8R8G8B8				= (0x85),
	CELL_GCM_TEXTURE_COMPRESSED_DXT1		= (0x86),
	CELL_GCM_TEXTURE_COMPRESSED_DXT23		= (0x87),
	CELL_GCM_TEXTURE_COMPRESSED_DXT45		= (0x88),
	CELL_GCM_TEXTURE_G8B8					= (0x8B),
	CELL_GCM_TEXTURE_R6G5B5					= (0x8F),
	CELL_GCM_TEXTURE_DEPTH24_D8				= (0x90),
	CELL_GCM_TEXTURE_DEPTH24_D8_FLOAT		= (0x91),
	CELL_GCM_TEXTURE_DEPTH16				= (0x92),
	CELL_GCM_TEXTURE_DEPTH16_FLOAT			= (0x93),
	CELL_GCM_TEXTURE_X16					= (0x94),
	CELL_GCM_TEXTURE_Y16_X16				= (0x95),
	CELL_GCM_TEXTURE_R5G5B5A1				= (0x97),
	CELL_GCM_TEXTURE_COMPRESSED_HILO8		= (0x98),
	CELL_GCM_TEXTURE_COMPRESSED_HILO_S8		= (0x99),
	CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT	= (0x9A),
	CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT	= (0x9B),
	CELL_GCM_TEXTURE_X32_FLOAT				= (0x9C),
	CELL_GCM_TEXTURE_D1R5G5B5				= (0x9D),
	CELL_GCM_TEXTURE_D8R8G8B8				= (0x9E),
	CELL_GCM_TEXTURE_Y16_X16_FLOAT			= (0x9F),

	CELL_GCM_TEXTURE_SZ	= (0x00),
	CELL_GCM_TEXTURE_LN	= (0x20),
	CELL_GCM_TEXTURE_NR	= (0x00),
	CELL_GCM_TEXTURE_UN	= (0x40),

	CELL_GCM_TEXTURE_DIMENSION_1		= (1),
	CELL_GCM_TEXTURE_DIMENSION_2		= (2),
	CELL_GCM_TEXTURE_DIMENSION_3		= (3),

	CELL_GCM_TEXTURE_REMAP_ORDER_XYXY	= (0),
	CELL_GCM_TEXTURE_REMAP_ORDER_XXXY	= (1),
	CELL_GCM_TEXTURE_REMAP_FROM_A		= (0),
	CELL_GCM_TEXTURE_REMAP_FROM_R		= (1),
	CELL_GCM_TEXTURE_REMAP_FROM_G		= (2),
	CELL_GCM_TEXTURE_REMAP_FROM_B		= (3),
	CELL_GCM_TEXTURE_REMAP_ZERO			= (0),
	CELL_GCM_TEXTURE_REMAP_ONE			= (1),
	CELL_GCM_TEXTURE_REMAP_REMAP		= (2),

	// SetTextureFilter
	CELL_GCM_TEXTURE_NEAREST			= (1),
	CELL_GCM_TEXTURE_LINEAR				= (2),
	CELL_GCM_TEXTURE_NEAREST_NEAREST	= (3),
	CELL_GCM_TEXTURE_LINEAR_NEAREST		= (4),
	CELL_GCM_TEXTURE_NEAREST_LINEAR		= (5),
	CELL_GCM_TEXTURE_LINEAR_LINEAR		= (6),
	CELL_GCM_TEXTURE_CONVOLUTION_MIN	= (7),
	CELL_GCM_TEXTURE_CONVOLUTION_MAG	= (4),
	CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX		= (1),
	CELL_GCM_TEXTURE_CONVOLUTION_GAUSSIAN		= (2),
	CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX_ALT	= (3),

	// SetTextureAddress
	CELL_GCM_TEXTURE_WRAP						= (1),
	CELL_GCM_TEXTURE_MIRROR						= (2),
	CELL_GCM_TEXTURE_CLAMP_TO_EDGE				= (3),
	CELL_GCM_TEXTURE_BORDER						= (4),
	CELL_GCM_TEXTURE_CLAMP						= (5),
	CELL_GCM_TEXTURE_MIRROR_ONCE_CLAMP_TO_EDGE	= (6),
	CELL_GCM_TEXTURE_MIRROR_ONCE_BORDER			= (7),
	CELL_GCM_TEXTURE_MIRROR_ONCE_CLAMP			= (8),

	CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL	= (0),
	CELL_GCM_TEXTURE_UNSIGNED_REMAP_BIASED	= (1),

	CELL_GCM_TEXTURE_ZFUNC_NEVER	= (0),
	CELL_GCM_TEXTURE_ZFUNC_LESS		= (1),
	CELL_GCM_TEXTURE_ZFUNC_EQUAL	= (2),
	CELL_GCM_TEXTURE_ZFUNC_LEQUAL	= (3),
	CELL_GCM_TEXTURE_ZFUNC_GREATER	= (4),
	CELL_GCM_TEXTURE_ZFUNC_NOTEQUAL	= (5),
	CELL_GCM_TEXTURE_ZFUNC_GEQUAL	= (6),
	CELL_GCM_TEXTURE_ZFUNC_ALWAYS	= (7),

	CELL_GCM_TEXTURE_GAMMA_R	= (1<<0),
	CELL_GCM_TEXTURE_GAMMA_G	= (1<<1),
	CELL_GCM_TEXTURE_GAMMA_B	= (1<<2),
	CELL_GCM_TEXTURE_GAMMA_A	= (1<<3),

	// SetCylindricalWrap
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX0_U  = (1 << 0),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX0_V  = (1 << 1),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX0_P  = (1 << 2),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX0_Q  = (1 << 3),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX1_U  = (1 << 4),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX1_V  = (1 << 5),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX1_P  = (1 << 6),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX1_Q  = (1 << 7),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX2_U  = (1 << 8),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX2_V  = (1 << 9),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX2_P  = (1 << 10),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX2_Q  = (1 << 11),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX3_U  = (1 << 12),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX3_V  = (1 << 13),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX3_P  = (1 << 14),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX3_Q  = (1 << 15),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX4_U  = (1 << 16),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX4_V  = (1 << 17),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX4_P  = (1 << 18),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX4_Q  = (1 << 19),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX5_U  = (1 << 20),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX5_V  = (1 << 21),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX5_P  = (1 << 22),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX5_Q  = (1 << 23),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX6_U  = (1 << 24),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX6_V  = (1 << 25),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX6_P  = (1 << 26),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX6_Q  = (1 << 27),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX7_U  = (1 << 28),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX7_V  = (1 << 29),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX7_P  = (1 << 30),
	CELL_GCM_TEXTURE_CYLINDRICAL_WRAP_ENABLE_TEX7_Q  = (1 << 31),

	// SetTextureControl
	CELL_GCM_TEXTURE_MAX_ANISO_1	= (0),
	CELL_GCM_TEXTURE_MAX_ANISO_2	= (1),
	CELL_GCM_TEXTURE_MAX_ANISO_4	= (2),
	CELL_GCM_TEXTURE_MAX_ANISO_6	= (3),
	CELL_GCM_TEXTURE_MAX_ANISO_8	= (4),
	CELL_GCM_TEXTURE_MAX_ANISO_10	= (5),
	CELL_GCM_TEXTURE_MAX_ANISO_12	= (6),
	CELL_GCM_TEXTURE_MAX_ANISO_16	= (7),

	// SetDrawArrays, SetDrawIndexArray
	CELL_GCM_PRIMITIVE_POINTS			= (1),
	CELL_GCM_PRIMITIVE_LINES			= (2),
	CELL_GCM_PRIMITIVE_LINE_LOOP		= (3),
	CELL_GCM_PRIMITIVE_LINE_STRIP		= (4),
	CELL_GCM_PRIMITIVE_TRIANGLES		= (5),
	CELL_GCM_PRIMITIVE_TRIANGLE_STRIP	= (6),
	CELL_GCM_PRIMITIVE_TRIANGLE_FAN		= (7),
	CELL_GCM_PRIMITIVE_QUADS			= (8),
	CELL_GCM_PRIMITIVE_QUAD_STRIP		= (9),
	CELL_GCM_PRIMITIVE_POLYGON			= (10),

	// SetColorMask
	CELL_GCM_COLOR_MASK_B	= (1<<0),
	CELL_GCM_COLOR_MASK_G	= (1<<8),
	CELL_GCM_COLOR_MASK_R	= (1<<16),
	CELL_GCM_COLOR_MASK_A	= (1<<24),

	// SetColorMaskMrt
	CELL_GCM_COLOR_MASK_MRT1_A	= (1<<4),
	CELL_GCM_COLOR_MASK_MRT1_R	= (1<<5),
	CELL_GCM_COLOR_MASK_MRT1_G	= (1<<6),
	CELL_GCM_COLOR_MASK_MRT1_B	= (1<<7),
	CELL_GCM_COLOR_MASK_MRT2_A	= (1<<8),
	CELL_GCM_COLOR_MASK_MRT2_R	= (1<<9),
	CELL_GCM_COLOR_MASK_MRT2_G	= (1<<10),
	CELL_GCM_COLOR_MASK_MRT2_B	= (1<<11),
	CELL_GCM_COLOR_MASK_MRT3_A	= (1<<12),
	CELL_GCM_COLOR_MASK_MRT3_R	= (1<<13),
	CELL_GCM_COLOR_MASK_MRT3_G	= (1<<14),
	CELL_GCM_COLOR_MASK_MRT3_B	= (1<<15),

	// SetAlphaFunc, DepthFunc, StencilFunc
	CELL_GCM_NEVER		= (0x0200),
	CELL_GCM_LESS		= (0x0201),
	CELL_GCM_EQUAL		= (0x0202),
	CELL_GCM_LEQUAL		= (0x0203),
	CELL_GCM_GREATER	= (0x0204),
	CELL_GCM_NOTEQUAL	= (0x0205),
	CELL_GCM_GEQUAL		= (0x0206),
	CELL_GCM_ALWAYS		= (0x0207),

	// SetBlendFunc
	CELL_GCM_ZERO						= (0),
	CELL_GCM_ONE						= (1),
	CELL_GCM_SRC_COLOR					= (0x0300),
	CELL_GCM_ONE_MINUS_SRC_COLOR		= (0x0301),
	CELL_GCM_SRC_ALPHA					= (0x0302),
	CELL_GCM_ONE_MINUS_SRC_ALPHA		= (0x0303),
	CELL_GCM_DST_ALPHA					= (0x0304),
	CELL_GCM_ONE_MINUS_DST_ALPHA		= (0x0305),
	CELL_GCM_DST_COLOR					= (0x0306),
	CELL_GCM_ONE_MINUS_DST_COLOR		= (0x0307),
	CELL_GCM_SRC_ALPHA_SATURATE			= (0x0308),
	CELL_GCM_CONSTANT_COLOR				= (0x8001),
	CELL_GCM_ONE_MINUS_CONSTANT_COLOR	= (0x8002),
	CELL_GCM_CONSTANT_ALPHA				= (0x8003),
	CELL_GCM_ONE_MINUS_CONSTANT_ALPHA	= (0x8004),

	// SetBlendEquation
	CELL_GCM_BLEND_COLOR			= (0x8005),
	CELL_GCM_FUNC_ADD				= (0x8006),
	CELL_GCM_MIN					= (0x8007),
	CELL_GCM_MAX					= (0x8008),
	CELL_GCM_BLEND_EQUATION			= (0x8009),
	CELL_GCM_FUNC_SUBTRACT			= (0x800A),
	CELL_GCM_FUNC_REVERSE_SUBTRACT	= (0x800B),

	// SetCullFace
	CELL_GCM_FRONT			= (0x0404),
	CELL_GCM_BACK			= (0x0405),
	CELL_GCM_FRONT_AND_BACK	= (0x0408),

	// SetShadeMode
	CELL_GCM_FLAT	= (0x1D00),
	CELL_GCM_SMOOTH	= (0x1D01),

	// SetFrontFace
	CELL_GCM_CW		= (0x0900),
	CELL_GCM_CCW	= (0x0901),

	// SetLogicOp
	CELL_GCM_CLEAR			= (0x1500),
	CELL_GCM_AND			= (0x1501),
	CELL_GCM_AND_REVERSE	= (0x1502),
	CELL_GCM_COPY			= (0x1503),
	CELL_GCM_AND_INVERTED	= (0x1504),
	CELL_GCM_NOOP			= (0x1505),
	CELL_GCM_XOR			= (0x1506),
	CELL_GCM_OR				= (0x1507),
	CELL_GCM_NOR			= (0x1508),
	CELL_GCM_EQUIV			= (0x1509),
	CELL_GCM_INVERT			= (0x150A),
	CELL_GCM_OR_REVERSE		= (0x150B),
	CELL_GCM_COPY_INVERTED	= (0x150C),
	CELL_GCM_OR_INVERTED	= (0x150D),
	CELL_GCM_NAND			= (0x150E),
	CELL_GCM_SET			= (0x150F),

	// SetStencilOp
	CELL_GCM_KEEP		= (0x1E00),
	CELL_GCM_REPLACE	= (0x1E01),
	CELL_GCM_INCR		= (0x1E02),
	CELL_GCM_DECR		= (0x1E03),
	CELL_GCM_INCR_WRAP	= (0x8507),
	CELL_GCM_DECR_WRAP	= (0x8508),

	// SetDrawIndexArray
	CELL_GCM_DRAW_INDEX_ARRAY_TYPE_32	= (0),
	CELL_GCM_DRAW_INDEX_ARRAY_TYPE_16	= (1),

	// SetTransfer
	CELL_GCM_TRANSFER_LOCAL_TO_LOCAL	= (0),
	CELL_GCM_TRANSFER_MAIN_TO_LOCAL		= (1),
	CELL_GCM_TRANSFER_LOCAL_TO_MAIN		= (2),

	// SetInvalidateTextureCache
	CELL_GCM_INVALIDATE_TEXTURE			= (1),
	CELL_GCM_INVALIDATE_VERTEX_TEXTURE	= (2),

	// SetFrequencyDividerOperation
	CELL_GCM_FREQUENCY_MODULO	= (1), 
	CELL_GCM_FREQUENCY_DIVIDE	= (0),

	// SetTile, SetZCull
	CELL_GCM_COMPMODE_DISABLED					= (0),
	CELL_GCM_COMPMODE_C32_2X1					= (7),
	CELL_GCM_COMPMODE_C32_2X2					= (8),
	CELL_GCM_COMPMODE_Z32_SEPSTENCIL			= (9),
	CELL_GCM_COMPMODE_Z32_SEPSTENCIL_REG		= (10),
	CELL_GCM_COMPMODE_Z32_SEPSTENCIL_REGULAR	= (10),
	CELL_GCM_COMPMODE_Z32_SEPSTENCIL_DIAGONAL	= (11),
	CELL_GCM_COMPMODE_Z32_SEPSTENCIL_ROTATED	= (12),

	// SetZCull
	CELL_GCM_ZCULL_Z16		= (1),
	CELL_GCM_ZCULL_Z24S8	= (2),
	CELL_GCM_ZCULL_MSB		= (0),
	CELL_GCM_ZCULL_LONES	= (1),
	CELL_GCM_ZCULL_LESS		= (0),
	CELL_GCM_ZCULL_GREATER	= (1),

	// flip mode
	CELL_GCM_DISPLAY_HSYNC				= (1),
	CELL_GCM_DISPLAY_VSYNC				= (2),
	CELL_GCM_DISPLAY_HSYNC_WITH_NOISE	= (3),

	// vsync frequency
	CELL_GCM_DISPLAY_FREQUENCY_59_94HZ =(1),
	CELL_GCM_DISPLAY_FREQUENCY_SCANOUT =(2),
	CELL_GCM_DISPLAY_FREQUENCY_DISABLE =(3),

	CELL_GCM_TYPE_B		= (1),
	CELL_GCM_TYPE_C		= (2),
	CELL_GCM_TYPE_RSX	= (3),

	// MRT
	CELL_GCM_MRT_MAXCOUNT	= (4),

	// max display id
	CELL_GCM_DISPLAY_MAXID	= (8),

	// Debug output level
	CELL_GCM_DEBUG_LEVEL0		= (0),
	CELL_GCM_DEBUG_LEVEL1		= (1),
	CELL_GCM_DEBUG_LEVEL2		= (2),

	// SetRenderEnable
	CELL_GCM_CONDITIONAL		= (2),

	// SetClearReport, SetReport, GetReport
	CELL_GCM_ZPASS_PIXEL_CNT	= (1),
	CELL_GCM_ZCULL_STATS		= (2),
	CELL_GCM_ZCULL_STATS1		= (3),
	CELL_GCM_ZCULL_STATS2		= (4),
	CELL_GCM_ZCULL_STATS3		= (5),

	// SetPointSpriteControl
	CELL_GCM_POINT_SPRITE_RMODE_ZERO       = (0),
	CELL_GCM_POINT_SPRITE_RMODE_FROM_R     = (1),
	CELL_GCM_POINT_SPRITE_RMODE_FROM_S     = (2),

	CELL_GCM_POINT_SPRITE_TEX0             = (1<<8),
	CELL_GCM_POINT_SPRITE_TEX1             = (1<<9),
	CELL_GCM_POINT_SPRITE_TEX2             = (1<<10),
	CELL_GCM_POINT_SPRITE_TEX3             = (1<<11),
	CELL_GCM_POINT_SPRITE_TEX4             = (1<<12),
	CELL_GCM_POINT_SPRITE_TEX5             = (1<<13),
	CELL_GCM_POINT_SPRITE_TEX6             = (1<<14),
	CELL_GCM_POINT_SPRITE_TEX7             = (1<<15),
	CELL_GCM_POINT_SPRITE_TEX8             = (1<<16),
	CELL_GCM_POINT_SPRITE_TEX9             = (1<<17),

	// SetUserClipPlaneControl
	CELL_GCM_USER_CLIP_PLANE_DISABLE       = (0),
	CELL_GCM_USER_CLIP_PLANE_ENABLE_LT     = (1),
	CELL_GCM_USER_CLIP_PLANE_ENABLE_GE     = (2),

	// SetAttribOutputMask
	CELL_GCM_ATTRIB_OUTPUT_MASK_FRONTDIFFUSE    = (1<< 0),
	CELL_GCM_ATTRIB_OUTPUT_MASK_FRONTSPECULAR   = (1<< 1),
	CELL_GCM_ATTRIB_OUTPUT_MASK_BACKDIFFUSE     = (1<< 2),
	CELL_GCM_ATTRIB_OUTPUT_MASK_BACKSPECULAR    = (1<< 3),
	CELL_GCM_ATTRIB_OUTPUT_MASK_FOG             = (1<< 4),
	CELL_GCM_ATTRIB_OUTPUT_MASK_POINTSIZE       = (1<< 5),
	CELL_GCM_ATTRIB_OUTPUT_MASK_UC0             = (1<< 6),
	CELL_GCM_ATTRIB_OUTPUT_MASK_UC1             = (1<< 7),
	CELL_GCM_ATTRIB_OUTPUT_MASK_UC2             = (1<< 8),
	CELL_GCM_ATTRIB_OUTPUT_MASK_UC3             = (1<< 9),
	CELL_GCM_ATTRIB_OUTPUT_MASK_UC4             = (1<<10),
	CELL_GCM_ATTRIB_OUTPUT_MASK_UC5             = (1<<11),
	CELL_GCM_ATTRIB_OUTPUT_MASK_TEX8            = (1<<12),
	CELL_GCM_ATTRIB_OUTPUT_MASK_TEX9            = (1<<13),
	CELL_GCM_ATTRIB_OUTPUT_MASK_TEX0            = (1<<14),
	CELL_GCM_ATTRIB_OUTPUT_MASK_TEX1            = (1<<15),
	CELL_GCM_ATTRIB_OUTPUT_MASK_TEX2            = (1<<16),
	CELL_GCM_ATTRIB_OUTPUT_MASK_TEX3            = (1<<17),
	CELL_GCM_ATTRIB_OUTPUT_MASK_TEX4            = (1<<18),
	CELL_GCM_ATTRIB_OUTPUT_MASK_TEX5            = (1<<19),
	CELL_GCM_ATTRIB_OUTPUT_MASK_TEX6            = (1<<20),
	CELL_GCM_ATTRIB_OUTPUT_MASK_TEX7            = (1<<21),

}CellGcmEnum;

typedef struct CellGcmControl{
	volatile uint32_t put;
	volatile uint32_t get;
	volatile uint32_t ref;
}CellGcmControl;

typedef struct CellGcmConfig{
	void		*localAddress;
	void		*ioAddress;
	uint32_t	localSize;
	uint32_t	ioSize;
	uint32_t	memoryFrequency;
	uint32_t	coreFrequency;
}CellGcmConfig;

typedef struct CellGcmSurface{
	uint8_t		type;
	uint8_t		antialias;

	uint8_t		colorFormat;
	uint8_t		colorTarget;
	uint8_t		colorLocation[CELL_GCM_MRT_MAXCOUNT];
	uint32_t	colorOffset[CELL_GCM_MRT_MAXCOUNT];
	uint32_t	colorPitch[CELL_GCM_MRT_MAXCOUNT];

	uint8_t		depthFormat;
	uint8_t		depthLocation;
	uint8_t		_padding[2];
	uint32_t	depthOffset;
	uint32_t	depthPitch;

	uint16_t	width;
	uint16_t	height;
	uint16_t	x;
	uint16_t	y;
}CellGcmSurface;

typedef struct CellGcmTexture{
	uint8_t		format;
	uint8_t		mipmap;
	uint8_t		dimension;
	uint8_t		cubemap;

	uint32_t	remap;

	uint16_t	width;
	uint16_t	height;
	uint16_t	depth;
	uint8_t		location;
	uint8_t		_padding;

	uint32_t	pitch;
	uint32_t	offset;
}CellGcmTexture;

struct CellGcmContextData;
typedef int32_t (*CellGcmContextCallback)(struct CellGcmContextData *, uint32_t);

typedef struct CellGcmContextData{
	uint32_t *__restrict begin;
	uint32_t *__restrict end;
	uint32_t *__restrict current;
	CellGcmContextCallback callback;
}CellGcmContextData;

extern struct CellGcmContextData *gCellGcmCurrentContext;

#ifdef CELL_GCM_DEBUG
extern void (*gCellGcmDebugCallback)(struct CellGcmContextData*);
extern void cellGcmDebugFinish(struct CellGcmContextData*);
#endif 

/*	macros		*/
#define CELL_GCM_REMAP_MODE(order, inputA, inputR, inputG, inputB, outputA, outputR, outputG, outputB) \
	(((order)<<16)|((inputA))|((inputR)<<2)|((inputG)<<4)|((inputB)<<6)|((outputA)<<8)|((outputR)<<10)|((outputG)<<12)|((outputB)<<14))

/*	system functions	*/
int32_t cellGcmInit(const uint32_t cmdSize, const uint32_t ioSize, const void *ioAddress);
int32_t cellGcmAddressToOffset(const void *address, uint32_t *offset);
int32_t cellGcmMapMainMemory(const void *address, const uint32_t size, uint32_t *offset);
void cellGcmGetConfiguration(CellGcmConfig *config);
CellGcmControl *cellGcmGetControlRegister(void);
uint32_t cellGcmGetTiledPitchSize(const uint32_t size);

void cellGcmSetVBlankHandler(void (*handler)(const uint32_t head));
void cellGcmSetSecondVHandler(void (*handler)(const uint32_t head));
void cellGcmSetGraphicsHandler(void (*handler)(const uint32_t val));
void cellGcmSetFlipHandler(void (*handler)(const uint32_t head));
void cellGcmSetQueueHandler(void (*handler)(const uint32_t head));
void cellGcmSetUserHandler(void (*handler)(const uint32_t cause));
void cellGcmSetDebugOutputLevel(int32_t level);

/*	display functions	*/
int32_t cellGcmSetDisplayBuffer(const uint8_t id, const uint32_t offset, const uint32_t pitch, const uint32_t width, const uint32_t height);
system_time_t cellGcmGetLastFlipTime(void);
void cellGcmSetFlipMode(const uint32_t mode);
int32_t cellGcmSetFlipImmediate(const uint8_t id);
void cellGcmResetFlipStatus(void);
void cellGcmSetFlipStatus(void);
uint32_t cellGcmGetFlipStatus(void);
void cellGcmSetSecondVFrequency(const uint32_t freq);
int32_t cellGcmGetDisplayBufferByFlipIndex(const uint32_t qid);
uint64_t cellGcmGetVBlankCount(void);

/* obsolete functions */
/* void cellGcmSetFlipVFrequency(const uint32_t freq); */
/* void cellGcmEmulateNewSwitch(void); */


/* flow control functions */
uint32_t *cellGcmGetCurrentBuffer(void);
void cellGcmSetCurrentBuffer(const uint32_t *addr, const size_t size);
void cellGcmSetDefaultCommandBuffer(void);
void cellGcmSetUserCallback(CellGcmContextCallback callback);
void cellGcmSetupContextData(CellGcmContextData *context, const uint32_t *addr, const uint32_t size, CellGcmContextCallback callback);


/* cg functions */
void cellGcmCgInitProgram(CGprogram prog);
void cellGcmCgGetUCode(CGprogram prog, void **pUCode, uint32_t *pUCodeSize);
CGparameter cellGcmCgGetNamedParameter(CGprogram prog, const char *name);
CGparameter cellGcmCgGetFirstLeafParameter(CGprogram prog, CGenum name_space);
CGparameter cellGcmCgGetNextLeafParameter(CGprogram prog, CGparameter current);
uint32_t cellGcmCgGetCountParameter(CGprogram prog);
uint32_t cellGcmCgGetInstructions(CGprogram prog);
CGparameter cellGcmCgGetIndexParameter(CGprogram prog, const uint32_t Index);
CGtype cellGcmCgGetParameterType(CGprogram prog, CGparameter param);
CGresource cellGcmCgGetParameterResource(CGprogram prog, CGparameter param);
const char *cellGcmCgGetParameterName(CGprogram prog, CGparameter param);
CGenum cellGcmCgGetParameterVariability(CGprogram prog, CGparameter param);
uint32_t cellGcmCgGetParameterResourceIndex(CGprogram prog, CGparameter param);
const float *cellGcmCgGetParameterValues(CGprogram prog, CGparameter param);
#define cellGcmCgGetParameterIndex cellGcmCgGetParameterOrdinalNumber
int32_t cellGcmCgGetParameterOrdinalNumber(CGprogram prog, CGparameter param);
CGenum cellGcmCgGetParameterDirection(CGprogram prog, CGparameter param);
const char *cellGcmCgGetParameterSemantic(CGprogram prog, CGparameter param);
CGprofile cellGcmCgGetProgramProfile(CGprogram prog);
CGbool cellGcmCgGetParameterReferenced(CGprogram prog, CGparameter param);
uint32_t cellGcmCgGetTotalBinarySize(CGprogram prog);
uint32_t cellGcmCgGetRegisterCount(const CGprogram prog);
void cellGcmCgSetRegisterCount(const CGprogram prog, const uint8_t registerCount);
uint32_t cellGcmCgGetAttribOutputMask(const CGprogram prog);
void cellGcmCgSetAttribOutputMask(const CGprogram prog, const uint32_t mask);

/*
render command functions
*/
uint32_t *cellGcmGetLabelAddress(const uint8_t index);
uint64_t cellGcmGetTimeStamp(const uint32_t index);
uint32_t cellGcmGetReport(const uint32_t type, const uint32_t index);
void cellGcmSetTile(const uint8_t index, const uint8_t location, const uint32_t offset, const uint32_t size, const uint32_t pitch, const uint8_t comp, const uint16_t base, const uint8_t bank);
void cellGcmSetInvalidateTile(const uint8_t index);
void cellGcmSetZcull(const uint8_t index, const uint32_t offset, const uint32_t width, const uint32_t height, const uint32_t cullStart, const uint32_t zFormat, const uint32_t aaFormat, const uint32_t zCullDir, const uint32_t zCullFormat, const uint32_t sFunc, const uint32_t sRef, const uint32_t sMask);

/*
data transfer and format conversion functions
*/
void cellGcmConvertSwizzleFormat(void *swizzledTexture, const void *texture, const uint32_t  dstx0, const uint32_t  dsty0, const uint32_t  dstz0, const uint32_t  dstWidth, const uint32_t  dstHeight, const uint32_t  dstDepth, const uint32_t  srcx0, const uint32_t  srcy0, const uint32_t  srcz0, const uint32_t  srcx1, const uint32_t  srcy1, const uint32_t  srcz1, const uint32_t  srcWidth, const uint32_t  srcHeight, const uint32_t  srcDepth, const uint32_t  dstLog2cdepth, const uint32_t  srcColordepth, const uint32_t  border, const uint32_t  dim, void (*copyTexel)(void *dst, const void *src));

//////////////////////////////////////////////////////////////
// C style functions

#define CELL_GCM_MODE CELL_GCM_C
#define CELL_GCM_THIS thisContext
#define CELL_GCM_CURRENT thisContext->current
#define CELL_GCM_END thisContext->end
#define CELL_GCM_CALLBACK thisContext->callback

#define CELL_GCM_MEASURE 0
#include <cell/gcm/gcm_methods.h>


// cellGcmFunction(CellGcmContext, ...);

#define CELL_GCM_INLINE 0
#define CELL_GCM_UNSAFE 0
#include <cell/gcm/gcm_prototypes.h>
#undef CELL_GCM_UNSAFE
#undef CELL_GCM_INLINE

// cellGcmFunctionUnsafe(CellGcmContext, ...);

#define CELL_GCM_INLINE 0
#define CELL_GCM_UNSAFE 1
#include <cell/gcm/gcm_prototypes.h>
#undef CELL_GCM_UNSAFE
#undef CELL_GCM_INLINE

// cellGcmFunctionInline(CellGcmContext, ...);

#define CELL_GCM_INLINE 1
#define CELL_GCM_UNSAFE 0
#include <cell/gcm/gcm_prototypes.h>
#include <cell/gcm/gcm_implementation.h>
#undef CELL_GCM_UNSAFE
#undef CELL_GCM_INLINE

// cellGcmFunctionInlineUnsafe(CellGcmContext, ...);

#define CELL_GCM_INLINE 1
#define CELL_GCM_UNSAFE 1
#include <cell/gcm/gcm_prototypes.h>
#include <cell/gcm/gcm_implementation.h>
#undef CELL_GCM_UNSAFE
#undef CELL_GCM_INLINE

#undef CELL_GCM_MEASURE

#undef CELL_GCM_MODE
#undef CELL_GCM_THIS
#undef CELL_GCM_CURRENT
#undef CELL_GCM_END
#undef CELL_GCM_CALLBACK

#ifdef __cplusplus
}
#endif /* __cplusplus */


//////////////////////////////////////////////////////////////
// C++ style functions
#ifdef __cplusplus

namespace cell {
namespace Gcm {

// Member methods and data types
#define CELL_GCM_MODE CELL_GCM_CPP
#define CELL_GCM_CPP_MEMBER 1
#define CELL_GCM_THIS this
#define CELL_GCM_CURRENT current
#define CELL_GCM_END end
#define CELL_GCM_CALLBACK callback

#define CELL_GCM_MEASURE 0
#include <cell/gcm/gcm_methods.h>

// CellGcmContext->Function(...);

#define CELL_GCM_INLINE 0
#define CELL_GCM_UNSAFE 0
struct CellGcmContext : public CellGcmContextData
{
#include <cell/gcm/gcm_global_implementation.h>
};
#undef CELL_GCM_INLINE
#undef CELL_GCM_UNSAFE


// Unsafe::CellGcmContext->Function(...);

namespace Unsafe
{
#define CELL_GCM_INLINE 0
#define CELL_GCM_UNSAFE 1
	struct CellGcmContext : public CellGcmContextData
	{
#include <cell/gcm/gcm_global_implementation.h>
	};
#undef CELL_GCM_INLINE
#undef CELL_GCM_UNSAFE
}

// Inline::CellGcmContext->Function(...);

namespace Inline
{
#define CELL_GCM_INLINE 1
#define CELL_GCM_UNSAFE 0
	struct CellGcmContext : public CellGcmContextData
	{
#include <cell/gcm/gcm_global_implementation.h>
	};
#undef CELL_GCM_INLINE
#undef CELL_GCM_UNSAFE
}

// UnsafeInline::CellGcmContext->Function(...);

namespace UnsafeInline
{
#define CELL_GCM_INLINE 1
#define CELL_GCM_UNSAFE 1
	struct CellGcmContext : public CellGcmContextData
	{
#include <cell/gcm/gcm_global_implementation.h>
	};
#undef CELL_GCM_INLINE
#undef CELL_GCM_UNSAFE
}

#undef CELL_GCM_MEASURE

// Space Measurements using actual code.
#define CELL_GCM_MEASURE 1
#include <cell/gcm/gcm_methods.h>

// Measure::CellGcmContext->Function(...);

namespace Measure
{
#define CELL_GCM_INLINE 1
#define CELL_GCM_UNSAFE 0
	struct CellGcmContext : public CellGcmContextData
	{
#include <cell/gcm/gcm_global_implementation.h>
	};
#undef CELL_GCM_INLINE
#undef CELL_GCM_UNSAFE
}

#undef CELL_GCM_MEASURE

#undef CELL_GCM_MODE
#undef CELL_GCM_CPP_MEMBER
#undef CELL_GCM_THIS
#undef CELL_GCM_CURRENT
#undef CELL_GCM_END
#undef CELL_GCM_CALLBACK

//////////////////////////////////////////////////////////////

#ifndef CELL_GCM_DENY_IMPLICIT_ARG

#define CELL_GCM_MODE CELL_GCM_CPP
#define CELL_GCM_CPP_MEMBER 0
#define CELL_GCM_THIS_IS_ARG 0
#define CELL_GCM_THIS gCellGcmCurrentContext

// cellGcmFunction(...);

#define CELL_GCM_INLINE 0
#define CELL_GCM_UNSAFE 0
#include <cell/gcm/gcm_global_implementation.h>
#undef CELL_GCM_UNSAFE
#undef CELL_GCM_INLINE

// Unsafe::cellGcmFunction(...);

namespace Unsafe
{
#define CELL_GCM_INLINE 0
#define CELL_GCM_UNSAFE 1
#include <cell/gcm/gcm_global_implementation.h>
#undef CELL_GCM_UNSAFE
#undef CELL_GCM_INLINE
}

// Inline::cellGcmFunction(...);

namespace Inline
{
#define CELL_GCM_INLINE 1
#define CELL_GCM_UNSAFE 0
#include <cell/gcm/gcm_global_implementation.h>
#undef CELL_GCM_UNSAFE
#undef CELL_GCM_INLINE
}

// UnsafeInline::cellGcmFunction(...);

namespace UnsafeInline
{
#define CELL_GCM_INLINE 1
#define CELL_GCM_UNSAFE 1
#include <cell/gcm/gcm_global_implementation.h>
#undef CELL_GCM_UNSAFE
#undef CELL_GCM_INLINE
}

#undef CELL_GCM_THIS
#undef CELL_GCM_THIS_IS_ARG
#undef CELL_GCM_CPP_MEMBER
#undef CELL_GCM_MODE

#endif /* CELL_GCM_DENY_IMPLICIT_ARG */

//////////////////////////////////////////////////////////////


#define CELL_GCM_MODE CELL_GCM_CPP
#define CELL_GCM_CPP_MEMBER 0
#define CELL_GCM_THIS_IS_ARG 1
#define CELL_GCM_THIS thisContext

// Unsafe::cellGcmFunction(CellGcmContext, ...);

namespace Unsafe
{
#define CELL_GCM_INLINE 0
#define CELL_GCM_UNSAFE 1
#include <cell/gcm/gcm_global_implementation.h>
#undef CELL_GCM_UNSAFE
#undef CELL_GCM_INLINE
}

// Inline::cellGcmFunction(CellGcmContext, ...);

namespace Inline
{
#define CELL_GCM_INLINE 1
#define CELL_GCM_UNSAFE 0
#include <cell/gcm/gcm_global_implementation.h>
#undef CELL_GCM_UNSAFE
#undef CELL_GCM_INLINE
}

// UnsafeInline::cellGcmFunction(CellGcmContext, ...);

namespace UnsafeInline
{
#define CELL_GCM_INLINE 1
#define CELL_GCM_UNSAFE 1
#include <cell/gcm/gcm_global_implementation.h>
#undef CELL_GCM_UNSAFE
#undef CELL_GCM_INLINE
}

#undef CELL_GCM_THIS
#undef CELL_GCM_THIS_IS_ARG
#undef CELL_GCM_CPP_MEMBER
#undef CELL_GCM_MODE

} /* namespace Ccm */
} /* namespace Cell */

#endif /* __cplusplus */


#endif	/* __CELL_GCM_H__ */
