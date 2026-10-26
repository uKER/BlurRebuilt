#ifndef DISPLAY2_GCM_CONTEXT_SPU_H
#define DISPLAY2_GCM_CONTEXT_SPU_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/Maths/Storage.h>
#include <Btl/Containers/Vector.h>

namespace Display2
{

//---------------------------------------------------------------------------
// Defines

#define ALIGNMENT_SURFACE						128
#define ALIGNMENT_TEXTURE						128
#define ALIGNMENT_TEXTURE2D						128
#define ALIGNMENT_TEXTURE3D						128
#define ALIGNMENT_TEXTURECUBE					128
#define ALIGNMENT_VERTEXBUFFER					16
#define ALIGNMENT_INDEXBUFFER					16
#define ALIGNMENT_VERTEXSHADER					64
#define ALIGNMENT_PIXELSHADER					64

//---------------------------------------------------------------------------
// Macros

#define ALIGN_SIZE(size, align)					(( (size)+(align)-1)/(align)*(align))

//------------------------------------------------------------------------------
// Types

typedef CellGcmSurface							GPUSurface;
typedef CellGcmTexture							GPUTexture;
typedef CellGcmTexture							GPUTexture2D;
typedef CellGcmTexture							GPUTexture3D;
typedef CellGcmTexture							GPUTextureCube;

//---------------------------------------------------------------------------
// Enums

enum E_Bool
{
	E_Bool_False								= CELL_GCM_FALSE,
	E_Bool_True									= CELL_GCM_TRUE,
	E_Bool_Unknown								= UINT_MAX,
};

enum E_Blend
{
	E_Blend_Zero								= CELL_GCM_ZERO,
	E_Blend_One									= CELL_GCM_ONE,
	E_Blend_SrcColour							= CELL_GCM_SRC_COLOR,
	E_Blend_InvSrcColour						= CELL_GCM_ONE_MINUS_SRC_COLOR,
	E_Blend_SrcAlpha							= CELL_GCM_SRC_ALPHA,
	E_Blend_InvSrcAlpha							= CELL_GCM_ONE_MINUS_SRC_ALPHA,
	E_Blend_DstAlpha							= CELL_GCM_DST_ALPHA,
	E_Blend_InvDstAlpha							= CELL_GCM_ONE_MINUS_DST_ALPHA,
	E_Blend_DstColour							= CELL_GCM_DST_COLOR,
	E_Blend_InvDstColour						= CELL_GCM_ONE_MINUS_DST_COLOR,
	E_Blend_Unknown								= UINT_MAX,
};

enum E_BlendOp
{
	E_BlendOp_Add								= CELL_GCM_FUNC_ADD,
	E_BlendOp_Subtract							= CELL_GCM_FUNC_SUBTRACT,
	E_BlendOp_RevSubtract						= CELL_GCM_FUNC_REVERSE_SUBTRACT,
	E_BlendOp_Min								= CELL_GCM_MIN,
	E_BlendOp_Max								= CELL_GCM_MAX,
	E_BlendOp_Unknown							= UINT_MAX,
};

enum E_Clear
{
	E_Clear_Colour								= CELL_GCM_CLEAR_R|CELL_GCM_CLEAR_G|CELL_GCM_CLEAR_B|CELL_GCM_CLEAR_A,
	E_Clear_Depth								= CELL_GCM_CLEAR_Z,
	E_Clear_Stencil								= CELL_GCM_CLEAR_S,
		
	E_Clear_Colour_Red							= CELL_GCM_CLEAR_R,
	E_Clear_Colour_Green						= CELL_GCM_CLEAR_G,
	E_Clear_Colour_Blue							= CELL_GCM_CLEAR_B,
	E_Clear_Colour_Alpha						= CELL_GCM_CLEAR_A,

};

enum E_CmpFunc
{
	E_CmpFunc_Never								= CELL_GCM_NEVER,
	E_CmpFunc_Less								= CELL_GCM_LESS,
	E_CmpFunc_Equal								= CELL_GCM_EQUAL,
	E_CmpFunc_LessEqual							= CELL_GCM_LEQUAL,
	E_CmpFunc_Greater							= CELL_GCM_GREATER,
	E_CmpFunc_NotEqual							= CELL_GCM_NOTEQUAL,
	E_CmpFunc_GreaterEqual						= CELL_GCM_GEQUAL,
	E_CmpFunc_Always							= CELL_GCM_ALWAYS,
	E_CmpFunc_Unknown							= UINT_MAX,
};

enum E_ColourWriteEnable
{
	E_ColourWriteEnable_Red						= CELL_GCM_COLOR_MASK_R,
	E_ColourWriteEnable_Green					= CELL_GCM_COLOR_MASK_G,
	E_ColourWriteEnable_Blue					= CELL_GCM_COLOR_MASK_B,
	E_ColourWriteEnable_Alpha					= CELL_GCM_COLOR_MASK_A,
	E_ColourWriteEnable_All						= CELL_GCM_COLOR_MASK_R|CELL_GCM_COLOR_MASK_G|CELL_GCM_COLOR_MASK_B|CELL_GCM_COLOR_MASK_A,
	E_ColourWriteEnable_Unknown 				= UINT_MAX,
};

enum E_CullMode
{
	E_CullMode_None								= 0,
	E_CullMode_Cw								= CELL_GCM_CW,
	E_CullMode_Ccw								= CELL_GCM_CCW,
	E_CullMode_Unknown							= UINT_MAX,
	E_CullMode_Default							= E_CullMode_Unknown-1,
	E_CullMode_Reversed							= E_CullMode_Unknown-2,
};

enum E_CubeMapFace
{
	E_CubeMapFace_PosX							= 0,
	E_CubeMapFace_NegX							= 1,
	E_CubeMapFace_PosY							= 2,
	E_CubeMapFace_NegY							= 3,
	E_CubeMapFace_PosZ							= 4,
	E_CubeMapFace_NegZ							= 5,
};

enum E_FillMode
{
	E_FillMode_Point							= CELL_GCM_POLYGON_MODE_POINT,
	E_FillMode_Wireframe						= CELL_GCM_POLYGON_MODE_LINE,
	E_FillMode_Solid							= CELL_GCM_POLYGON_MODE_FILL,
	E_FillMode_Unknown							= UINT_MAX,
};

enum E_Flags
{
	E_Flags_Video								= 0x00000001,
	E_Flags_System								= 0x00000002,
	E_Flags_Device								= 0x00000004,
	
	E_Flags_Indirect							= 0x00000010,
	E_Flags_AlignSizeToBlockSize				= 0x00000020,

	E_Flags_Dynamic								= 0x00000100,

	E_Flags_Colour								= 0x00001000,
	E_Flags_DepthStencil						= 0x00002000,
	
	E_Flags_Raw									= 0x00010000,
	E_Flags_Tiled								= 0x00020000,
	E_Flags_Linear								= 0x00040000,
	E_Flags_Swizzle								= 0x00080000,
	
	E_Flags_SRGB								= 0x00100000,

	E_Flags_DoNotTrack							= 0x01000000,
	E_Flags_DoNotReload							= 0x02000000,
	
	E_Flags_ReadWrite							= 0x10000000,
};

enum E_IndexFormat
{
	E_IndexFormat_16							= CELL_GCM_DRAW_INDEX_ARRAY_TYPE_16,
	E_IndexFormat_32							= CELL_GCM_DRAW_INDEX_ARRAY_TYPE_32,
	E_IndexFormat_Unknown						= UINT_MAX,
};

enum E_Location
{
	E_Location_Local							= CELL_GCM_LOCATION_LOCAL,
	E_Location_Main								= CELL_GCM_LOCATION_MAIN,
};

enum E_Multisample
{
	E_Multisample_None							= CELL_GCM_SURFACE_CENTER_1,
	E_Multisample_2								= CELL_GCM_SURFACE_DIAGONAL_CENTERED_2,
	E_Multisample_4								= CELL_GCM_SURFACE_SQUARE_ROTATED_4,
	E_Multisample_Unknown						= UINT_MAX,
};

enum E_PresentInterval
{
	E_PresentInterval_Immediate					= 0,
	E_PresentInterval_One,
	E_PresentInterval_Two,
	E_PresentInterval_Three,
	E_PresentInterval_Four,
	E_PresentInterval_Default,
	E_PresentInterval_Unknown					= UINT_MAX,
};

enum E_PrimitiveType
{
	// Cross-platform
	E_PrimitiveType_PointList					= CELL_GCM_PRIMITIVE_POINTS,
	E_PrimitiveType_LineList					= CELL_GCM_PRIMITIVE_LINES,
	E_PrimitiveType_LineStrip					= CELL_GCM_PRIMITIVE_LINE_STRIP,
	E_PrimitiveType_TriangleList				= CELL_GCM_PRIMITIVE_TRIANGLES,
	E_PrimitiveType_TriangleStrip				= CELL_GCM_PRIMITIVE_TRIANGLE_STRIP,
	E_PrimitiveType_TriangleFan					= CELL_GCM_PRIMITIVE_TRIANGLE_FAN,

	// Not cross-platform
	E_PrimitiveType_LineLoop					= CELL_GCM_PRIMITIVE_LINE_LOOP,
	E_PrimitiveType_QuadList					= CELL_GCM_PRIMITIVE_QUADS,
	E_PrimitiveType_QuadStrip					= CELL_GCM_PRIMITIVE_QUAD_STRIP,
	E_PrimitiveType_Polygon						= CELL_GCM_PRIMITIVE_POLYGON,
	E_PrimitiveType_Unknown						= UINT_MAX,
};

enum E_Report
{
	E_Report_TimingStart						= 1024,
	E_Report_TimingCount						= 1022,
};

enum E_Resolve
{
	E_Resolve_RenderTarget0						= 0,
	E_Resolve_RenderTarget1,
	E_Resolve_RenderTarget2,
	E_Resolve_RenderTarget3,
	E_Resolve_DepthStencil,
	E_Resolve_AllFragments,
};

enum E_ResourceType
{
	E_ResourceType_Surface						= 0,
	E_ResourceType_Texture,
	E_ResourceType_Texture2D,
	E_ResourceType_Texture3D,
	E_ResourceType_TextureCube,
	E_ResourceType_VertexBuffer,
	E_ResourceType_IndexBuffer,
	E_ResourceType_VertexShader,
	E_ResourceType_PixelShader,
	E_ResourceType_Unknown,
};

enum E_Sampler
{
	E_Sampler_0									= 0,
	E_Sampler_1,
	E_Sampler_2,
	E_Sampler_3,
	E_Sampler_4,
	E_Sampler_5,
	E_Sampler_6,
	E_Sampler_7,
	E_Sampler_8,
	E_Sampler_9,
	E_Sampler_10,
	E_Sampler_11,
	E_Sampler_12,
	E_Sampler_13,
	E_Sampler_14,
	E_Sampler_15,
	E_Sampler_Count,
};

enum E_StencilOp
{
	E_StencilOp_Keep							= CELL_GCM_KEEP,
	E_StencilOp_Zero							= CELL_GCM_ZERO,
	E_StencilOp_Replace							= CELL_GCM_REPLACE,
	E_StencilOp_IncrSat							= CELL_GCM_INCR_WRAP,
	E_StencilOp_DecrSat							= CELL_GCM_DECR_WRAP,
	E_StencilOp_Invert							= CELL_GCM_INVERT,
	E_StencilOp_Incr							= CELL_GCM_INCR,
	E_StencilOp_Decr							= CELL_GCM_DECR,
	E_StencilOp_Unknown							= UINT_MAX,
};

enum E_Stream
{
	E_Stream_0									= 0,
	E_Stream_1,
	E_Stream_2,
	E_Stream_3,
	E_Stream_4,
	E_Stream_5,
	E_Stream_6,
	E_Stream_7,
	E_Stream_Count,
};

enum E_SurfaceFormat
{
	// Cross-platform
	E_SurfaceFormat_Unknown						= UINT_MAX,
	E_SurfaceFormat_A8R8G8B8					= CELL_GCM_SURFACE_A8R8G8B8,
	E_SurfaceFormat_A16B16G16R16F 				= CELL_GCM_SURFACE_F_W16Z16Y16X16,
	E_SurfaceFormat_A32B32G32R32F 				= CELL_GCM_SURFACE_F_W32Z32Y32X32,
	E_SurfaceFormat_D16							= CELL_GCM_SURFACE_Z16,
	E_SurfaceFormat_D24S8						= CELL_GCM_SURFACE_Z24S8,
	E_SurfaceFormat_R32F						= CELL_GCM_SURFACE_F_X32,
	E_SurfaceFormat_L8							= CELL_GCM_SURFACE_B8,

	// Not cross-platform
	E_SurfaceFormat_X1R5G5B5_Z1R5G5B5 			= CELL_GCM_SURFACE_X1R5G5B5_Z1R5G5B5,
	E_SurfaceFormat_X1R5G5B5_O1R5G5B5 			= CELL_GCM_SURFACE_X1R5G5B5_O1R5G5B5,
	E_SurfaceFormat_R5G6B5						= CELL_GCM_SURFACE_R5G6B5,
	E_SurfaceFormat_X8R8G8B8_Z8R8G8B8 			= CELL_GCM_SURFACE_X8R8G8B8_Z8R8G8B8,
	E_SurfaceFormat_X8R8G8B8_O8R8G8B8 			= CELL_GCM_SURFACE_X8R8G8B8_O8R8G8B8,
	E_SurfaceFormat_A8B8G8R8					= CELL_GCM_SURFACE_A8B8G8R8,
	E_SurfaceFormat_X8B8G8R8_Z8B8G8R8 			= CELL_GCM_SURFACE_X8B8G8R8_Z8B8G8R8,
	E_SurfaceFormat_X8B8G8R8_O8B8G8R8 			= CELL_GCM_SURFACE_X8B8G8R8_O8B8G8R8,
};

enum E_TextureAddress
{
	E_TextureAddress_Wrap						= CELL_GCM_TEXTURE_WRAP,
	E_TextureAddress_Clamp						= CELL_GCM_TEXTURE_CLAMP,
	E_TextureAddress_ClampToEdge				= CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
	E_TextureAddress_Mirror						= CELL_GCM_TEXTURE_MIRROR,
	E_TextureAddress_Border						= CELL_GCM_TEXTURE_BORDER,
	E_TextureAddress_Unknown					= UINT_MAX,
};

enum E_TextureZFunc
{
	E_TextureZFunc_Never						= CELL_GCM_TEXTURE_ZFUNC_NEVER,
	E_TextureZFunc_Less							= CELL_GCM_TEXTURE_ZFUNC_LESS,
	E_TextureZFunc_Equal						= CELL_GCM_TEXTURE_ZFUNC_EQUAL,
	E_TextureZFunc_LessEqual					= CELL_GCM_TEXTURE_ZFUNC_LEQUAL,
	E_TextureZFunc_Greater						= CELL_GCM_TEXTURE_ZFUNC_GREATER,
	E_TextureZFunc_NotEqual						= CELL_GCM_TEXTURE_ZFUNC_NOTEQUAL,
	E_TextureZFunc_GreaterEqual					= CELL_GCM_TEXTURE_ZFUNC_GEQUAL,
	E_TextureZFunc_Always						= CELL_GCM_TEXTURE_ZFUNC_ALWAYS,
	E_TextureZFunc_Unknown						= UINT_MAX,
};

enum E_TextureFilter
{
	E_TextureFilter_None,
	E_TextureFilter_Point,
	E_TextureFilter_Linear,
	E_TextureFilter_Anisotropic,
	E_TextureFilter_Unknown,
};

enum E_TextureFilterMin
{
	E_TextureFilterMin_Nearest					= CELL_GCM_TEXTURE_NEAREST,
	E_TextureFilterMin_Linear					= CELL_GCM_TEXTURE_LINEAR,
	E_TextureFilterMin_NearestNearest			= CELL_GCM_TEXTURE_NEAREST_NEAREST,
	E_TextureFilterMin_LinearNearest			= CELL_GCM_TEXTURE_LINEAR_NEAREST,
	E_TextureFilterMin_NearestLinear			= CELL_GCM_TEXTURE_NEAREST_LINEAR,
	E_TextureFilterMin_LinearLinear				= CELL_GCM_TEXTURE_LINEAR_LINEAR,
	E_TextureFilterMin_ConvolutionMin			= CELL_GCM_TEXTURE_CONVOLUTION_MIN,
};

enum E_TextureFilterMag
{
	E_TextureFilterMag_Nearest					= CELL_GCM_TEXTURE_NEAREST,
	E_TextureFilterMag_Linear					= CELL_GCM_TEXTURE_LINEAR,
	E_TextureFilterMag_ConvolutionMag			= CELL_GCM_TEXTURE_CONVOLUTION_MAG,
};

enum E_TextureFilterConv
{
	E_TextureFilterConv_Quincunx				= CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX,
	E_TextureFilterConv_Gaussiab				= CELL_GCM_TEXTURE_CONVOLUTION_GAUSSIAN,
	E_TextureFilterConv_QuincunxAlt				= CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX_ALT,
};

enum E_TextureFormat
{
	// Cross-platform
	E_TextureFormat_Unknown						= UINT_MAX,
	E_TextureFormat_A8R8G8B8					= CELL_GCM_TEXTURE_A8R8G8B8,
	E_TextureFormat_DXT1						= CELL_GCM_TEXTURE_COMPRESSED_DXT1,
	E_TextureFormat_DXT3 						= CELL_GCM_TEXTURE_COMPRESSED_DXT23,
	E_TextureFormat_DXT5 						= CELL_GCM_TEXTURE_COMPRESSED_DXT45,
	E_TextureFormat_A16B16G16R16F 				= CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT,
	E_TextureFormat_A32B32G32R32F 				= CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT,
	E_TextureFormat_D16							= CELL_GCM_TEXTURE_DEPTH16,
	E_TextureFormat_D24S8						= CELL_GCM_TEXTURE_DEPTH24_D8,
	E_TextureFormat_R32F						= CELL_GCM_TEXTURE_X32_FLOAT,
	E_TextureFormat_L8							= CELL_GCM_TEXTURE_B8,
	E_TextureFormat_G16R16						= CELL_GCM_TEXTURE_Y16_X16,
	E_TextureFormat_G16R16F						= CELL_GCM_TEXTURE_Y16_X16_FLOAT,
	
	// Not cross-platform
	E_TextureFormat_A1R5G5B5 					= CELL_GCM_TEXTURE_A1R5G5B5,
	E_TextureFormat_A4R4G4B4 					= CELL_GCM_TEXTURE_A4R4G4B4,
	E_TextureFormat_R5G6B5						= CELL_GCM_TEXTURE_R5G6B5,
	E_TextureFormat_G8B8						= CELL_GCM_TEXTURE_G8B8,
	E_TextureFormat_R6G5B5						= CELL_GCM_TEXTURE_R6G5B5,
	E_TextureFormat_DEPTH24_D8_FLOAT			= CELL_GCM_TEXTURE_DEPTH24_D8_FLOAT,
	E_TextureFormat_DEPTH16_FLOAT				= CELL_GCM_TEXTURE_DEPTH16_FLOAT,
	E_TextureFormat_X16							= CELL_GCM_TEXTURE_X16,
	E_TextureFormat_R5G5B5A1					= CELL_GCM_TEXTURE_R5G5B5A1,
	E_TextureFormat_COMPRESSED_HILO8			= CELL_GCM_TEXTURE_COMPRESSED_HILO8,
	E_TextureFormat_COMPRESSED_HILO_S8			= CELL_GCM_TEXTURE_COMPRESSED_HILO_S8,
	E_TextureFormat_D1R5G5B5					= CELL_GCM_TEXTURE_D1R5G5B5,
	E_TextureFormat_D8R8G8B8					= CELL_GCM_TEXTURE_D8R8G8B8,
	E_TextureFormat_COMPRESSED_B8R8_G8R8		= CELL_GCM_TEXTURE_COMPRESSED_B8R8_G8R8,
	E_TextureFormat_COMPRESSED_R8B8_R8G8		= CELL_GCM_TEXTURE_COMPRESSED_R8B8_R8G8,
};

enum E_TextureGamma
{
	E_TextureGamma_R							= CELL_GCM_TEXTURE_GAMMA_R,
	E_TextureGamma_G							= CELL_GCM_TEXTURE_GAMMA_G,
	E_TextureGamma_B							= CELL_GCM_TEXTURE_GAMMA_B,
	E_TextureGamma_A							= CELL_GCM_TEXTURE_GAMMA_A,
};

enum E_TextureMaxAniso
{
	E_TextureMaxAniso_1							= CELL_GCM_TEXTURE_MAX_ANISO_1,
	E_TextureMaxAniso_2							= CELL_GCM_TEXTURE_MAX_ANISO_2,
	E_TextureMaxAniso_4							= CELL_GCM_TEXTURE_MAX_ANISO_4,
	E_TextureMaxAniso_6							= CELL_GCM_TEXTURE_MAX_ANISO_6,
	E_TextureMaxAniso_8							= CELL_GCM_TEXTURE_MAX_ANISO_8,
	E_TextureMaxAniso_10						= CELL_GCM_TEXTURE_MAX_ANISO_10,
	E_TextureMaxAniso_12						= CELL_GCM_TEXTURE_MAX_ANISO_12,
	E_TextureMaxAniso_16						= CELL_GCM_TEXTURE_MAX_ANISO_16,
};

enum E_TextureOptimisationIso
{
	E_TextureOptimisationIso_Low				= CELL_GCM_TEXTURE_ISO_LOW,
	E_TextureOptimisationIso_High				= CELL_GCM_TEXTURE_ISO_HIGH,
};

enum E_TextureOptimisationAniso
{
	E_TextureOptimisationAniso_Low				= CELL_GCM_TEXTURE_ANISO_LOW,
	E_TextureOptimisationAniso_High				= CELL_GCM_TEXTURE_ANISO_HIGH,
};

enum E_TextureType
{
	E_TextureType_1D							= 0,
	E_TextureType_2D,
	E_TextureType_3D,
	E_TextureType_Cube,
	E_TextureType_Unknown,
};

enum E_TextureUnsignedRemap
{
	E_TextureUnsignedRemap_Normal				= CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL,
	E_TextureUnsignedRemap_Biased				= CELL_GCM_TEXTURE_UNSIGNED_REMAP_BIASED,
};

enum E_VertexElementType
{
	// Cross-platform
	E_VertexElementType_F32_1					= 0,
    E_VertexElementType_F32_2,
    E_VertexElementType_F32_3,
    E_VertexElementType_F32_4,
    E_VertexElementType_Colour,
    E_VertexElementType_U8_4,
	E_VertexElementType_U8_4N,
    E_VertexElementType_S16_2,
	E_VertexElementType_S16_4,
	E_VertexElementType_F16_2,
    E_VertexElementType_F16_4,
    E_VertexElementType_S16_2N,

	// Not cross-platform
	E_VertexElementType_SHHD_3,
	E_VertexElementType_SHHD_3N,

	// Cross-platform
	E_VertexElementType_Unknown,
};

enum E_VertexElementUsage
{
	// Cross-platform
	E_VertexElementUsage_Position				= 0,		// CG_ATTR0,
	E_VertexElementUsage_BlendWeight			= 1,		// CG_ATTR1,
	E_VertexElementUsage_BlendIndices			= 7,		// CG_ATTR7,
	E_VertexElementUsage_Normal					= 2,		// CG_ATTR2,
	E_VertexElementUsage_PSize					= 6,		// CG_ATTR6,
	E_VertexElementUsage_TexCoord				= 8,		// CG_ATTR8,
	E_VertexElementUsage_Tangent				= 14,		// CG_ATTR14,
	E_VertexElementUsage_Binormal				= 15,		// CG_ATTR15,
	E_VertexElementUsage_Colour					= 3,		// CG_ATTR3,
	E_VertexElementUsage_Fog					= 5,		// CG_ATTR5,
	E_VertexElementUsage_Unknown				= 16,
};

enum E_VertexSampler
{
	E_VertexSampler_0							= 0,
	E_VertexSampler_1,
	E_VertexSampler_2,
	E_VertexSampler_3,
	E_VertexSampler_Count,
};

//-----------------------------------------------------------------------------
// Asserts

#define GPU_ASSERT_X BIZ_ASSERT_SPEED

#define GPU_ASSERT(_x) \
	BIZ_ASSERT( \
		(_x));

#define GPU_ASSERT_BOOL(_bool) \
	BIZ_ASSERT( \
		(_bool) == E_Bool_False || \
		(_bool) == E_Bool_True );

#define GPU_ASSERT_BLEND(_blend) \
	BIZ_ASSERT( \
		(_blend) == E_Blend_Zero || \
		(_blend) == E_Blend_One || \
		(_blend) == E_Blend_SrcColour || \
		(_blend) == E_Blend_InvSrcColour || \
		(_blend) == E_Blend_SrcAlpha || \
		(_blend) == E_Blend_InvSrcAlpha || \
		(_blend) == E_Blend_DstAlpha || \
		(_blend) == E_Blend_InvDstAlpha || \
		(_blend) == E_Blend_DstColour || \
		(_blend) == E_Blend_InvDstColour);

#define GPU_ASSERT_BLEND_OP(_blendOp) \
	BIZ_ASSERT( \
		(_blendOp) == E_BlendOp_Add || \
		(_blendOp) == E_BlendOp_Subtract || \
		(_blendOp) == E_BlendOp_RevSubtract || \
		(_blendOp) == E_BlendOp_Min || \
		(_blendOp) == E_BlendOp_Max);

#define GPU_ASSERT_CMP_FUNC(_cmpFunc) \
	BIZ_ASSERT( \
		(_cmpFunc) == E_CmpFunc_Never || \
		(_cmpFunc) == E_CmpFunc_Less || \
		(_cmpFunc) == E_CmpFunc_LessEqual || \
		(_cmpFunc) == E_CmpFunc_Greater || \
		(_cmpFunc) == E_CmpFunc_GreaterEqual || \
		(_cmpFunc) == E_CmpFunc_Equal || \
		(_cmpFunc) == E_CmpFunc_NotEqual || \
		(_cmpFunc) == E_CmpFunc_Always);

#define GPU_ASSERT_CULL_MODE(_cullMode) \
	BIZ_ASSERT( \
		(_cullMode) == E_CullMode_Cw || \
		(_cullMode) == E_CullMode_Ccw);

#define GPU_ASSERT_FILL_MODE(_fillMode) \
	BIZ_ASSERT( \
		(_fillMode) == E_FillMode_Point || \
		(_fillMode) == E_FillMode_Wireframe || \
		(_fillMode) == E_FillMode_Solid);

#define GPU_ASSERT_INDEX_FORMAT(_indexFormat) \
	BIZ_ASSERT( \
		(_indexFormat) == E_IndexFormat_16 || \
		(_indexFormat) == E_IndexFormat_32);

#define GPU_ASSERT_LOCATION(_location) \
	BIZ_ASSERT( \
		(_location) == E_Location_Main || \
		(_location) == E_Location_Local);

#define GPU_ASSERT_PRIMITIVE_TYPE(_primitiveType) \
	BIZ_ASSERT( \
		(_primitiveType) == E_PrimitiveType_PointList || \
		(_primitiveType) == E_PrimitiveType_LineList || \
		(_primitiveType) == E_PrimitiveType_LineStrip || \
		(_primitiveType) == E_PrimitiveType_TriangleList || \
		(_primitiveType) == E_PrimitiveType_TriangleStrip || \
		(_primitiveType) == E_PrimitiveType_TriangleFan || \
		(_primitiveType) == E_PrimitiveType_LineLoop || \
		(_primitiveType) == E_PrimitiveType_QuadStrip || \
		(_primitiveType) == E_PrimitiveType_Polygon);

#define GPU_ASSERT_STENCIL_OP(_stencilOp) \
	BIZ_ASSERT( \
		(_stencilOp) == E_StencilOp_Keep || \
		(_stencilOp) == E_StencilOp_Zero || \
		(_stencilOp) == E_StencilOp_Replace || \
		(_stencilOp) == E_StencilOp_IncrSat || \
		(_stencilOp) == E_StencilOp_DecrSat || \
		(_stencilOp) == E_StencilOp_Invert || \
		(_stencilOp) == E_StencilOp_Incr || \
		(_stencilOp) == E_StencilOp_Decr);

#define GPU_ASSERT_TEXTURE(_texture) \
		DebugCheckTexture(_texture);

#define GPU_ASSERT_TEXTURE_ADDRESS(_textureAddress) \
	BIZ_ASSERT( \
		(_textureAddress) == E_TextureAddress_Wrap || \
		(_textureAddress) == E_TextureAddress_Clamp || \
		(_textureAddress) == E_TextureAddress_ClampToEdge || \
		(_textureAddress) == E_TextureAddress_Mirror || \
		(_textureAddress) == E_TextureAddress_Border);

#define GPU_ASSERT_TEXTURE_FILTER(_textureFilter) \
	BIZ_ASSERT( \
		(_textureFilter) == E_TextureFilter_None || \
		(_textureFilter) == E_TextureFilter_Point || \
		(_textureFilter) == E_TextureFilter_Linear || \
		(_textureFilter) == E_TextureFilter_Anisotropic);

#define GPU_ASSERT_TEXTURE_FILTER_MIN(_textureFilterMin) \
	BIZ_ASSERT( \
		(_textureFilterMin) == E_TextureFilterMin_Nearest || \
		(_textureFilterMin) == E_TextureFilterMin_Linear || \
		(_textureFilterMin) == E_TextureFilterMin_NearestNearest || \
		(_textureFilterMin) == E_TextureFilterMin_LinearNearest || \
		(_textureFilterMin) == E_TextureFilterMin_NearestLinear || \
		(_textureFilterMin) == E_TextureFilterMin_LinearLinear || \
		(_textureFilterMin) == E_TextureFilterMin_ConvolutionMin);

#define GPU_ASSERT_TEXTURE_FILTER_MAG(_textureFilterMag) \
	BIZ_ASSERT( \
		(_textureFilterMag) == E_TextureFilterMag_Nearest || \
		(_textureFilterMag) == E_TextureFilterMin_Linear || \
		(_textureFilterMag) == E_TextureFilterMag_ConvolutionMag);

#define GPU_ASSERT_TEXTURE_FILTER_CONV(_textureFilterConv) \
	BIZ_ASSERT( \
		(_textureFilterConv) == E_TextureFilterConv_Quincunx || \
		(_textureFilterConv) == E_TextureFilterConv_Gaussiab || \
		(_textureFilterConv) == E_TextureFilterConv_QuincunxAlt);

#define GPU_ASSERT_TEXTURE_MAX_ANISO(_textureMaxAniso) \
	BIZ_ASSERT( \
		(_textureMaxAniso) == E_TextureMaxAniso_1 || \
		(_textureMaxAniso) == E_TextureMaxAniso_2 || \
		(_textureMaxAniso) == E_TextureMaxAniso_4 || \
		(_textureMaxAniso) == E_TextureMaxAniso_6 || \
		(_textureMaxAniso) == E_TextureMaxAniso_8 || \
		(_textureMaxAniso) == E_TextureMaxAniso_10 || \
		(_textureMaxAniso) == E_TextureMaxAniso_12 || \
		(_textureMaxAniso) == E_TextureMaxAniso_16);

#define GPU_ASSERT_TEXTURE_Z_FUNC(_textureZFunc) \
	BIZ_ASSERT( \
		(_textureZFunc) == E_TextureZFunc_Never || \
		(_textureZFunc) == E_TextureZFunc_Less || \
		(_textureZFunc) == E_TextureZFunc_Equal || \
		(_textureZFunc) == E_TextureZFunc_LessEqual || \
		(_textureZFunc) == E_TextureZFunc_Greater || \
		(_textureZFunc) == E_TextureZFunc_NotEqual || \
		(_textureZFunc) == E_TextureZFunc_GreaterEqual || \
		(_textureZFunc) == E_TextureZFunc_Always);

#define GPU_ASSERT_VERTEX_ELEMENT_GCM_TYPE(_vertexElementGcmType) \
	BIZ_ASSERT( \
		(_vertexElementGcmType) == CELL_GCM_VERTEX_S1 || \
		(_vertexElementGcmType) == CELL_GCM_VERTEX_F || \
		(_vertexElementGcmType) == CELL_GCM_VERTEX_SF || \
		(_vertexElementGcmType) == CELL_GCM_VERTEX_UB || \
		(_vertexElementGcmType) == CELL_GCM_VERTEX_S32K || \
		(_vertexElementGcmType) == CELL_GCM_VERTEX_CMP || \
		(_vertexElementGcmType) == CELL_GCM_VERTEX_UB256);

//-----------------------------------------------------------------------------
// Functions

bool IsMainAddress(void* pAddress);

bool IsLocalAddress(void* pAddress);

bool IsMainOffset(const U32 offset);

bool IsLocalOffset(const U32 offset);

void* OffsetToAddress(U32 location, U32 offset);

U32 FlagsToLocation(const U32 flags);

U32 GetSurfacePitchFromWidth(U32 width, const U32 flags, const E_SurfaceFormat format, const E_Multisample multisample);

U32 GetSurfaceSizeFromHeightAndPitch(U32 height, const U32 pitch, const U32 flags, const E_SurfaceFormat format, const E_Multisample multisample);

U32 GetSurfaceSize(const U32 width, const U32 height, const U32 flags, const E_SurfaceFormat format, const E_Multisample multisample);

void GetSurfaceSizeAndPitch(const U32 width, const U32 height, const U32 flags, const E_SurfaceFormat format, const E_Multisample multisample, U32& size, U32& pitch);

U32 GetTexturePitchFromWidth(U32 width, const U32 flags, const E_TextureFormat format);

U32 GetTextureSizeFromHeightAndPitch(U32 height, const U32 pitch, const U32 flags, const E_TextureFormat format);

U32 GetTexturePitch(U32 width, const U32 flags, const E_TextureFormat format);

U32 GetTextureRowPitch(U32 width, const U32 flags, const E_TextureFormat format);

U32 GetTextureSlicePitch(U32 width, U32 height, U32 levels, const U32 flags, const E_TextureFormat format);

U32 GetTexture2DSize(const U32 width, const U32 height, const U32 flags, const E_TextureFormat format);

U32 GetTexture2DSize(U32 width, U32 height, const U32 levels, const U32 flags, const E_TextureFormat format);

void GetTexture2DSizeAndPitch(const U32 width, const U32 height, const U32 levels, const U32 flags, const E_TextureFormat format, U32& size, U32& pitch);

U32 GetTexture3DSize(const U32 width, const U32 height, const U32 depth, const U32 flags, const E_TextureFormat format);

U32 GetTexture3DSize(U32 width, U32 height, const U32 depth, const U32 levels, const U32 flags, const E_TextureFormat format);

void GetTexture3DSizeAndPitch(const U32 width, const U32 height, const U32 depth, const U32 levels, const U32 flags, const E_TextureFormat format, U32& size, U32& pitch);

U32 GetTextureCubeSize(const U32 width, const U32 flags, const E_TextureFormat format);

U32 GetTextureCubeSize(U32 width, const U32 levels, const U32 flags, const E_TextureFormat format);

void GetTextureCubeSizeAndPitch(const U32 width, const U32 levels, const U32 flags, const E_TextureFormat format, U32& size, U32& pitch);

U32 GetTextureWidthFromMultisample(const U32 width, const E_Multisample multisample);

U32 GetTextureHeightFromMultisample(const U32 height, const E_Multisample multisample);

U32 GetTexture2DLevelOffset(const U32 level, U32 width, U32 height, const U32 levels, const U32 flags, const E_TextureFormat format);

U32 GetTexture3DLevelOffset(const U32 level, U32 width, U32 height, U32 depth, const U32 levels, const U32 flags, const E_TextureFormat format);

U32 GetTextureCubeLevelOffset(const E_CubeMapFace face, const U32 level, U32 width, const U32 levels, const U32 flags, const E_TextureFormat format);

U32 SetTexture2DHeader(const U32 width, const U32 height, const U32 levels, const U32 flags, const E_TextureFormat format, const U32 pitch, GPUTexture* pTexture);

U32 SetTexture3DHeader(const U32 width, const U32 height, const U32 depth, const U32 levels, const U32 flags, const E_TextureFormat format, const U32 pitch, GPUTexture* pTexture);

U32 SetTextureCubeHeader(const U32 width, const U32 levels, const U32 flags, const E_TextureFormat format, const U32 pitch, GPUTexture* pTexture);

U32 GetTexture2DLevels(U32 width, U32 height);

bool DebugCheckTexture(const CellGcmTexture* pTexture);

void DebugPrintContextData(const CellGcmContextData* pContextData);

const Util::C_String GetFlagsAsString(const U32 flags);

const Util::C_String GetIndexFormatAsString(const E_IndexFormat indexFormat);

const Util::C_String GetMultisampleAsString(const E_Multisample multisample);

const Util::C_String GetSurfaceFormatAsString(const E_SurfaceFormat format);

const Util::C_String GetTextureFormatAsString(const E_TextureFormat format);

const Util::C_String GetTextureTypeAsString(const E_TextureType type);

//-----------------------------------------------------------------------------
// Structs

struct C_CommandChain
{
	C_CommandChain();

	bool IsCreated() const;

	U32 GetSize() const;
	U8* GetAddress() const;

	void Setup(CellGcmContextData* pContextData, CellGcmContextCallback pContextCallback = NULL, const S32 padding = 0);
	
	U32											m_Size;
	U32											m_Offset;
	U8*											m_pAddress;
};

// C_Viewport

struct C_Viewport
{
	C_Viewport();
	C_Viewport(const U32 x, const U32 y, const U32 width, const U32 height);
	C_Viewport(const U32 x, const U32 y, const U32 width, const U32 height, const F32 minZ, const F32 maxZ);
	~C_Viewport();

	U32 GetX() const;
	U32 GetY() const;
	U32 GetWidth() const;
	U32 GetHeight() const;
	F32 GetMinZ() const;
	F32 GetMaxZ() const;

	void SetX(const U32 x);
	void SetY(const U32 y);
	void SetWidth(const U32 width);
	void SetHeight(const U32 height);
	void SetMinZ(const F32 minZ);
	void SetMaxZ(const F32 maxZ);

	C_Viewport& operator=(const C_Viewport& other);
	bool operator==(const C_Viewport& other) const;
	bool operator!=(const C_Viewport& other) const;

	U32											m_X;
	U32											m_Y;
	U32											m_Width;
	U32											m_Height;
	F32											m_MinZ;
	F32											m_MaxZ;
};

struct C_VertexElement
{
	C_VertexElement();
	C_VertexElement(const U16 stream, const U32 offset, const U32 type, const U8 usage, const U8 usageIndex);
	C_VertexElement(const U16 stream, const U32 offset, const U32 type, const U8 usage, const U8 usageIndex, const U8 gcmSize, const U8 gcmType, const U8 gcmStride);
	~C_VertexElement();

	U16 GetStream() const;
	U32 GetOffset() const;
	U32 GetType() const;
	U8 GetUsage() const;
	U8 GetUsageIndex() const;
	
	void SetStream(const U16 stream);
	void SetOffset(const U32 offset);
	void SetType(const U32 type);
	void SetUsage(const U8 usage);
	void SetUsageIndex(const U8 usageIndex);

	U32 GetSize() const;
	U32 GetCount() const;

	C_VertexElement& operator=(const C_VertexElement& other);
	bool operator==(const C_VertexElement& other) const;
	bool operator!=(const C_VertexElement& other) const;

	U8											m_Stream;
	U32											m_Offset;
	U8											m_Type;
	U8											m_Usage;
	U8											m_UsageIndex;

	U8											m_gcmSize;
	U8											m_gcmType;
	U8											m_gcmStride;
};

#define GPU_VERTEX_ELEMENT_END() Display2::C_VertexElement(Display2::E_Stream_Count, 0, Display2::E_VertexElementType_Unknown, Display2::E_VertexElementUsage_Unknown, 0, 0, 0, 0)

struct STOREALIGN16 C_VertexDeclaration
{
	C_VertexDeclaration();

	bool IsCreated() const;

	S32 GetElementCount() const;
	const C_VertexElement* GetElements() const; 
	U32 GetStride(const U32 stream = 0) const;

	static const S32							m_MaxElementCount = 12;
	S32											m_ElementCount;
	C_VertexElement								m_ElementArray[m_MaxElementCount];
};

struct STOREALIGN16 C_VertexBuffer
{
	C_VertexBuffer();

	bool IsCreated() const;

	U32 GetSize() const;
	U32 GetFlags() const;
	U32 GetOffset() const;
	void* GetAddress() const;
	
	void SetSize(const U32 size);
	void SetFlags(const U32 flags);
	void SetOffset(const U32 offset);

	void* Lock(const U32 offset, const U32 size, const U32 flags = 0);
	void* Lock(const U32 flags = 0);
	void Unlock();

	U32											m_Size;
	U32											m_Flags;
	U32											m_Offset;

	#if DISPLAY_DEBUG
	bool										m_Locked;
	#endif
};

struct STOREALIGN16 C_IndexBuffer
{
	C_IndexBuffer();

	bool IsCreated() const;

	U32 GetSize() const;
	U32 GetFlags() const;
	U32 GetOffset() const;
	E_IndexFormat GetFormat() const;
	void* GetAddress() const;

	void SetSize(const U32 size);
	void SetFlags(const U32 flags);
	void SetOffset(const U32 offset);
	void SetFormat(const E_IndexFormat format);

	void* Lock(const U32 offset, const U32 size, const U32 flags = 0);
	void* Lock(const U32 flags = 0);
	void Unlock();

	U32											m_Size;
	U32											m_Flags;
	U32											m_Offset;
	E_IndexFormat								m_Format;

	#if DISPLAY_DEBUG
	bool										m_Locked;
	#endif
};

struct C_ShaderSemantic
{
	C_ShaderSemantic();
	C_ShaderSemantic(const U8 usage, const U8 usageIndex);

	U8											m_Usage;
	U8											m_UsageIndex;
};

struct C_VertexShaderParam
{
	C_VertexShaderParam();

	U32											m_RegisterIndex;
	U32											m_RegisterCount;
};

struct STOREALIGN16 C_VertexShader
{
	C_VertexShader();

	bool IsCreated() const;

	U32 GetSize() const;
	U32 GetFlags() const;
	U32 GetOffset() const;
	U8* GetAddress() const;
	U8* GetAddressInMainMemory() const;

	void SetSize(const U32 size);
	void SetFlags(const U32 flags);
	
	U8 GetRegisterCount() const;
	void SetRegisterCount(const U8 registerCount);

	const C_VertexShaderParam GetParamFromName(const char* name) const;
	const C_VertexShaderParam GetParameter(const U32 index, const char* name) const;

	C_CommandChain* GetCommandChain();

	#if !PLATFORM_SPU
	CellCgbProgram *GetCgbProgram();
	const CellCgbProgram *GetCgbProgram() const;
	#endif

	bool GetSemantics(Btl::Vector<C_ShaderSemantic>& semantics);

	U32											m_Size;
	U32											m_Flags;
	U8*											m_pAddressInMainMemory;

	CellCgbVertexProgramConfiguration			m_ProgramConfig;
	
	C_CommandChain								m_CommandChain; // Shader is preset into this...

	#if !PLATFORM_SPU
	CellCgbProgram								m_Program;
	#endif
};

struct C_PixelShader;

struct C_PixelShaderParamBase
{
	C_PixelShaderParamBase();
	~C_PixelShaderParamBase();

	C_PixelShader*								m_pPixelShader;
	Btl::Vector<U16>							m_Offsets;

	#if TRACK_DISPLAY2_RESOURCES
	char										m_Name[32];
	#endif
};

typedef const C_PixelShaderParamBase*			C_PixelShaderParam;

typedef Btl::Vector<C_PixelShaderParam>			C_PixelShaderParamArray;

struct STOREALIGN16 C_PixelShader
{
	C_PixelShader();
	~C_PixelShader();

	bool IsCreated() const;

	U32 GetSize() const;
	U32 GetFlags() const;
	U32 GetOffset() const;
	U8* GetAddress() const;
	U8* GetAddressInMainMemory() const;
	
	void SetSize(const U32 size);
	void SetFlags(const U32 flags);
	void SetOffset(const U32 offset);

	U8 GetRegisterCount() const;
	void SetRegisterCount(const U8 registerCount);

	C_PixelShaderParam GetParamFromName(const char* name) const;
	C_PixelShaderParam GetParameter(const S32 index, const char* name) const; // Compatibility for Display
	C_PixelShaderParamArray GetParameterArray(const S32 index, const char* name, const U32 count) const; // Compatibility for Display

	void CreateParams();

	U32											m_Size;
	U32											m_Flags;
	U8*											m_pAddress;
	U8*											m_pAddressInMainMemory;

	CellCgbFragmentProgramConfiguration			m_ProgramConfig;

	Btl::Vector<C_PixelShaderParamBase>			m_Params;

	#if !PLATFORM_SPU
	CellCgbProgram								m_Program;
	#endif
};

struct C_LockedRect
{
	C_LockedRect() ;

	S32 GetPitch() const;
	U8* GetBits() const;

	C_LockedRect& operator=(const C_LockedRect& other);

	U8*											pBits;
	U32											pitch;
};

struct C_ConstLockedRect
{
	C_ConstLockedRect();

	S32 GetPitch() const;
    const U8* GetBits() const;

	C_ConstLockedRect& operator=(const C_ConstLockedRect& other);

	const U8*									pBits;
	U32											pitch;
};

struct C_LockedBox
{
	C_LockedBox();

	S32 GetRowPitch() const;
	S32 GetSlicePitch() const;
	U8* GetBits() const;

	C_LockedBox& operator=(const C_LockedBox& other);

	U8*											pBits;
	U32											rowPitch;
	U32											slicePitch;
};

struct C_ConstLockedBox
{
	C_ConstLockedBox();

	S32 GetRowPitch() const;
	S32 GetSlicePitch() const;
    const U8* GetBits() const;

	C_ConstLockedBox& operator=(const C_ConstLockedBox& other);

	const U8*									pBits;
	U32											rowPitch;
	U32											slicePitch;
};

struct C_Surface;

struct C_SurfaceParams
{
	void OverSurface(const C_Surface* pSurface);
	void AfterSurface(const C_Surface* pSurface);
	void BeforeSurface(const C_Surface* pSurface);
};

struct C_Surface
{
	C_Surface();

	bool IsCreated() const;

	U32 GetSize() const;
	U32 GetFlags() const;
	U32 GetOffset() const;
	U8* GetAddress() const;

	U32 GetWidth() const;
	U32 GetHeight() const;
	E_SurfaceFormat GetFormat() const;
	U32 GetMultisample() const;
	U32 GetPitch() const;

	void SetSize(const U32 size);
	void SetFlags(const U32 flags);
	void SetOffset(const U32 offset);
	void SetAddress(U8* pAddress);

	C_LockedRect LockRect(const U32 flags = 0);
	C_ConstLockedRect LockRect(const U32 flags = 0) const;
	void UnlockRect();

	U16											m_Width;
	U16											m_Height;
	E_SurfaceFormat								m_Format;
	U32											m_Multisample;
	U32											m_Size;
	U32											m_Pitch;
	U32											m_Offset;
	U32											m_Flags;

	U8*											m_pAddress; // Temp put this in for Rog's screenshot code...

	#if !CFG_GOLD
	mutable bool								m_Locked;
	#endif
};

struct STOREALIGN16 C_Texture
{
	C_Texture();

	bool IsCreated() const;

	U32 GetSize() const;
	U32 GetFlags() const;
	U32 GetOffset() const;
	U8* GetAddress() const;

	U32 GetWidth(const U32 level = 0) const;
	U32 GetHeight(const U32 level = 0) const;
	U32 GetDepth(const U32 level = 0) const;
	U32 GetLevels() const;
	E_TextureFormat GetFormat() const;
	U32 GetPitch() const;
	E_TextureType GetType() const;
	
	void SetSize(const U32 size);
	void SetFlags(const U32 flags);
	void SetOffset(const U32 offset);
	void SetSRGB(const bool enable = true);

	GPUTexture* GetGPUTexture();
	const GPUTexture* GetGPUTexture() const;
	
	C_LockedRect LockRect2D(const U32 level, const U32 flags = 0);
	C_ConstLockedRect LockRect2D(const U32 level, const U32 flags = 0) const;
	void UnlockRect2D(const U32 level) const;

	C_LockedBox LockBox3D(const U32 level, const U32 flags = 0);
	C_ConstLockedBox LockBox3D(const U32 level, const U32 flags = 0) const;
	void UnlockBox3D(const U32 level) const;

	C_LockedRect LockRectCube(const E_CubeMapFace face, const U32 level, const U32 flags = 0);
	C_ConstLockedRect LockRectCube(const E_CubeMapFace face, const U32 level, const U32 flags = 0) const;
	void UnlockRectCube(const E_CubeMapFace face, const U32 level) const;

	// Compat for Display
	GPUTexture* Get();
	const Util::C_String GetFileName() const;
	C_LockedRect Lock(const U32 level);
	C_ConstLockedRect Lock(const U32 level) const;
	void Unlock(const U32 level) const;
	
	U32											m_Size;
	U32											m_Flags;

	GPUTexture									m_gcmTexture;

	#if !CFG_GOLD
	mutable bool								m_Locked[6];
	#endif
};

struct C_RenderTexture2D
{
	C_RenderTexture2D();
	~C_RenderTexture2D();

	bool IsCreated() const;

	U32 GetFlags() const;

	C_Texture& GetTexture();
	const C_Texture& GetTexture() const;

	C_Surface& GetSurface(const U32 level = 0);
	const C_Surface& GetSurface(const U32 level = 0) const;

	mutable U16									m_TextureIndex;
	C_Texture									m_Texture[2];
	mutable U16									m_SurfaceIndex;
	Btl::Vector<C_Surface>						m_Surfaces[2];
	U32											m_Flags;
};

struct C_RenderTextureCube
{
	C_RenderTextureCube();
	~C_RenderTextureCube();

	bool IsCreated() const;

	U32 GetFlags() const;

	C_Texture& GetTexture();
	const C_Texture& GetTexture() const;

	C_Surface& GetSurface(const U32 face, const U32 level = 0);
	const C_Surface& GetSurface(const U32 face, const U32 level = 0) const;

	C_Texture									m_Texture;
	Btl::Vector<C_Surface>						m_Surfaces[6];
	U32											m_Flags;
};

//-----------------------------------------------------------------------------
// States

struct C_AlphaTestState
{
	void SetDefaultStates();
	void SetAlphaTestCmpFunc(const E_CmpFunc alphaTestCmpFunc);
	void SetAlphaTestRef(const U16 alphaTestRef);

	union
	{
		struct
		{
			U32									m_AlphaTestCmpFunc	: 16;	// E_CmpFunc
			U32									m_AlphaTestRef		: 16;	// U16
		};
		U32										m_AlphaTestBits;
	};
};

struct C_BlendState
{
	void SetDefaultStates();
	void SetBlendColourSrc(const E_Blend blendColourSrc);
	void SetBlendColourDst(const E_Blend blendColourDst);
	void SetBlendColourOp(const E_BlendOp blendColourOp);
	void SetBlendAlphaSrc(const E_Blend blendAlphaSrc);
	void SetBlendAlphaDst(const E_Blend blendAlphaDst);
	void SetBlendAlphaOp(const E_BlendOp blendAlphaOp);

	U32											m_BlendColourSrc;	// E_Blend
	U32											m_BlendColourDst;	// E_Blend
	U32											m_BlendColourOp;	// E_BlendOp
	U32											m_BlendAlphaSrc;	// E_Blend
	U32											m_BlendAlphaDst;	// E_Blend
	U32											m_BlendAlphaOp;		// E_BlendOp
};

struct C_ColourState
{
	void SetDefaultStates();
	void SetColourWriteEnable(const U32 colourWriteEnable);

	U32											m_ColourWriteEnable;
};

struct C_CullState
{
	void SetDefaultStates();
	void SetCullMode(const E_CullMode cullMode);

	union
	{
		struct
		{
			U32									m_CullEnable	: 1;	// E_Bool
			U32									m_CullMode		: 31;	// E_CullMode
		};
		U32										m_CullBits;
	};
};

struct C_DepthState
{
	void SetDefaultStates();
	void SetDepthFunc(const E_CmpFunc depthFunc);
	void SetDepthEnable(const E_Bool depthEnable);
	void SetDepthWriteEnable(const E_Bool depthWriteEnable);
	void SetDepthBias(const F32 depthBias);
	void SetSlopeScaledBias(const F32 slopeScaledBias);

	union
	{
		struct
		{
			U32									m_DepthFunc			: 30;	// E_CmpFunc
			U32									m_DepthEnable		: 1;	// E_Bool
			U32									m_DepthWriteEnable	: 1;	// E_Bool
		};
		U32										m_DepthBits;
	};
	F32											m_DepthBias;
	F32											m_SlopeScaledBias;
};

struct C_FillState
{
	void SetDefaultStates();
	void SetFillMode(const E_FillMode fillMode);

	U32											m_FillMode;
};

struct C_TextureState
{
	void SetDefaultStates();
	void SetTexture(const C_Texture* pTexture);
	void SetTextureMaxAniso(const E_TextureMaxAniso maxAniso);
	
	const C_Texture*							m_pTexture;

	union
	{
		struct
		{
			U16									m_TextureControlEnable			: 1; // E_Bool
			U16									m_TextureControlMinLod			: 5; // 0 to 12
			U16									m_TextureControlMaxLod			: 5; // 0 to 12
			U16									m_TextureControlMaxAniso		: 4; // E_TextureMaxAniso
		};
		U16										m_SetTextureControlBits;
	};

	U32											m_TextureRemap;
};

struct C_SamplerState
{
	void SetDefaultStates();
	bool IsSame(const C_SamplerState &other);
	void SetTextureAddressU(const E_TextureAddress textureAddressU);
	void SetTextureAddressV(const E_TextureAddress textureAddressV);
	void SetTextureAddressW(const E_TextureAddress textureAddressW);
	void SetTextureAddress(const E_TextureAddress textureAddressU, const E_TextureAddress textureAddressV, const E_TextureAddress textureAddressW);
	void SetTextureAddress(const E_TextureAddress textureAddress);
	void SetTextureFilter(const E_TextureFilter textureFilterMag, const E_TextureFilter textureFilterMin, const E_TextureFilter textureFilterMip);
	void SetTextureFilter(const E_TextureFilter textureFilter);
	void SetTextureBorderColour(const U32 textureBorderColour);
	void SetTextureGamma(const U32 textureGamma);
	void SetTextureLodBias(const S8 textureLodBias);
	
	union
	{
		struct
		{
			U32									m_TextureAddressU				: 4; // E_TextureAddress
			U32									m_TextureAddressV				: 4; // E_TextureAddress
			U32									m_TextureAddressW				: 4; // E_TextureAddress
			U32									m_TextureUnsignedRemap			: 1; // E_TextureUnsignedRemap
			U32									m_TextureZFunc					: 4; // E_TextureCmpFunc
			U32									m_TextureGamma					: 5; // E_TextureGamma
		};
		U32										m_SetTextureAddressBits;
	};

	U32 										m_TextureBorderColour;

	union
	{
		struct
		{
			S16									m_TextureFilterBias				: 8;
			U16									m_TextureFilterMin				: 3; // E_TextureFilterMin
			U16									m_TextureFilterMag				: 3; // E_TextureFilterMag
			U16									m_TextureFilterConv				: 2; // E_TextureFilterConv
		};
		U16										m_SetTextureFilterBits;
	};

	union
	{
		struct
		{
			U32									m_TextureOptimisationSlope		: 8;
			U32									m_TextureOptimisationIso		: 8; // E_TextureOptimisationIso
			U32									m_TextureOptimisationAniso		: 8; // E_TextureOptimisationAniso

		};
		U32										m_SetTextureOptimisationBits;
	};
};

struct C_ScissorState
{
	void SetDefaultStates();
	void SetScissorEnable(const E_Bool scissorEnable);
	void SetScissorRect(const U16 x, const U16 y, const U16 w, const U16 h);

	U16											m_ScissorRectX;
	U16											m_ScissorRectY;
	U16											m_ScissorRectW;
	U16											m_ScissorRectH;
};

struct C_StencilState
{
	void SetDefaultStates();
	void SetStencilFunc(const E_CmpFunc stencilFunc);
	void SetStencilFail(const E_StencilOp stencilFail);
	void SetStencilDepthFail(const E_StencilOp stencilDepthFail);
	void SetStencilDepthPass(const E_StencilOp stencilDepthPass);
	void SetStencilRef(const U32 stencilRef);
	void SetStencilMask(const U32 stencilMask);
	void SetStencilWriteMask(const U32 stencilWriteMask);

	U32											m_StencilFunc;			// E_CmpFunc
	U32											m_StencilFail;			// E_StencilOp
	U32											m_StencilDepthFail;		// E_StencilOp
	U32											m_StencilDepthPass;		// E_StencilOp
	U32											m_StencilRef;
	U32											m_StencilMask;
	U32											m_StencilWriteMask;
};

struct C_VertexTextureState
{
	void SetDefaultStates();
	void SetTexture(const C_Texture* pTexture);
	
	const C_Texture*							m_pTexture;

	union
	{
		struct
		{
			U16									m_TextureControlEnable			: 1; // E_Bool
			U16									m_TextureControlMinLod			: 5; // 0 to 12
			U16									m_TextureControlMaxLod			: 6; // 0 to 12
		};
		U16										m_SetTextureControlBits;
	};
};

struct C_VertexSamplerState
{
	void SetDefaultStates();
	void SetTextureAddressU(const E_TextureAddress textureAddressU);
	void SetTextureAddressV(const E_TextureAddress textureAddressV);
	void SetTextureAddress(const E_TextureAddress textureAddressU, const E_TextureAddress textureAddressV);
	void SetTextureAddress(const E_TextureAddress textureAddress);
	void SetTextureBorderColour(const U32 textureBorderColour);
	void SetTextureLodBias(const S8 textureLodBias);
	
	union
	{
		struct
		{
			U32									m_TextureAddressU				: 4; // E_TextureAddress
			U32									m_TextureAddressV				: 4; // E_TextureAddress
		};
		U32										m_SetTextureAddressBits;
	};

	U32 										m_TextureBorderColour;
	
	union
	{
		struct
		{
			S16									m_TextureFilterBias				: 8;
		};
		U16										m_SetTextureFilterBits;
	};
};

struct C_ContextState
{
	void SetDefaultStates();

	U32											m_AlphaTestEnable;
	U32											m_BlendEnable;
	U32											m_StencilEnable;
	U32											m_PointSpriteEnable;
	
	C_AlphaTestState							m_AlphaTestState;
	C_BlendState								m_BlendState;
	C_ColourState								m_ColourState;
	C_CullState									m_CullState;
	C_DepthState								m_DepthState;
	C_FillState									m_FillState;
	C_ScissorState								m_ScissorState;
	C_StencilState								m_StencilState;

	const C_VertexDeclaration*					m_pVertexDeclaration;
	const C_VertexBuffer*						m_pVertexBuffer[E_Stream_Count];
	U32											m_VertexOffset[E_Stream_Count];
	U32											m_VertexStride[E_Stream_Count];
	const C_IndexBuffer*						m_pIndexBuffer;

	const C_VertexShader*						m_pVertexShader;
	const C_PixelShader*						m_pPixelShader;

	C_TextureState								m_TextureState[E_Sampler_Count];
	C_SamplerState								m_SamplerState[E_Sampler_Count];
	
	C_VertexTextureState						m_VertexTextureState[E_VertexSampler_Count];
	C_VertexSamplerState						m_VertexSamplerState[E_VertexSampler_Count];
};

//-----------------------------------------------------------------------------
// Contexts

#define GPU_NO_ARGS()
#define GPU_ARGS(...)			__VA_ARGS__
#define GPU_VOID				void
#define GPU_RETURN_OK			return
#define GPU_RETURN_FAIL			return
#define GPU_RETURN_VOID			return
#define GPU_RETURN(_func)		_func;

typedef CellGcmContextData C_ContextData;

struct C_ContextBase
{
	C_ContextBase();
	C_ContextBase(C_ContextData* pData);

	C_ContextData* GetData();
	const C_ContextData* GetData() const;
	
	C_ContextData*		m_pData;
};

//-------------------------------------

#define GPU_CLASS		C_Context
#define GPU_CHECK		1
#define GPU_TRACK		1

struct C_Context : public C_ContextBase
{
	C_Context();
	C_Context(C_ContextData* pData);
	C_Context(C_ContextData* pData, C_ContextState* pState);

	#include "Display2/Context/GCMFunctions.h"

	C_ContextState*		m_pState;
};

#if !DONT_INLINE_DISPLAY2
#include "Display2/Context/GCMFunctions.inl"
#endif

#undef GPU_TRACK
#undef GPU_CHECK
#undef GPU_CLASS

//-------------------------------------

#define GPU_CLASS		C_ContextNoCheck
#define GPU_TRACK		1

struct C_ContextNoCheck : public C_ContextBase
{
	C_ContextNoCheck();
	C_ContextNoCheck(C_ContextData* pData);
	C_ContextNoCheck(C_ContextData* pData, C_ContextState* pState);

	#include "Display2/Context/GCMFunctions.h"

	C_ContextState*		m_pState;
};

#if !DONT_INLINE_DISPLAY2
#include "Display2/Context/GCMFunctions.inl"
#endif

#undef GPU_TRACK
#undef GPU_CLASS

//-------------------------------------

#define GPU_CLASS		C_ContextNoCheckNoTrack

struct C_ContextNoCheckNoTrack : public C_ContextBase
{
	C_ContextNoCheckNoTrack();
	C_ContextNoCheckNoTrack(C_ContextData* pData);

	#include "Display2/Context/GCMFunctions.h"
};

#if !DONT_INLINE_DISPLAY2
#include "Display2/Context/GCMFunctions.inl"
#endif

#undef GPU_CLASS

//-------------------------------------

#define GPU_CLASS		C_ContextNoCheckNoTrackMeasure
#define GPU_MEASURE		1

struct C_ContextNoCheckNoTrackMeasure : public C_ContextBase
{
	C_ContextNoCheckNoTrackMeasure();
	C_ContextNoCheckNoTrackMeasure(C_ContextData* pData);

	#include "Display2/Context/GCMFunctions.h"
};

#if !DONT_INLINE_DISPLAY2
#include "Display2/Context/GCMFunctions.inl"
#endif

#undef GPU_MEASURE
#undef GPU_CLASS

//-------------------------------------

#define GPU_CLASS		C_ContextNoCheckNoTrackUnsafeInline
#define GPU_UNSAFE		1
#define GPU_INLINE		1

struct C_ContextNoCheckNoTrackUnsafeInline : public C_ContextBase
{
	C_ContextNoCheckNoTrackUnsafeInline();
	C_ContextNoCheckNoTrackUnsafeInline(C_ContextData* pData);

	#include "Display2/Context/GCMFunctions.h"
};

#if !DONT_INLINE_DISPLAY2
#include "Display2/Context/GCMFunctions.inl"
#endif

#undef GPU_INLINE
#undef GPU_UNSAFE
#undef GPU_CLASS

//-------------------------------------

#define GPU_CLASS C_ContextNoCheckNoTrackUnsafe
#define GPU_UNSAFE 1

struct C_ContextNoCheckNoTrackUnsafe : public C_ContextBase
{
	C_ContextNoCheckNoTrackUnsafe();
	C_ContextNoCheckNoTrackUnsafe(C_ContextData* pData);

	#include "Display2/Context/GCMFunctions.h"
};

#if !DONT_INLINE_DISPLAY2
#include "Display2/Context/GCMFunctions.inl"
#endif

#undef GPU_UNSAFE
#undef GPU_CLASS

//-------------------------------------

#undef GPU_RETURN
#undef GPU_RETURN_VOID
#undef GPU_RETURN_FAIL
#undef GPU_RETURN_OK
#undef GPU_VOID
#undef GPU_ARGS
#undef GPU_NO_ARGS

//---------------------------------------------------------------------------
// Globals

extern CellGcmConfig g_gcmConfig;

#if USE_GPU_TIMINGS2
extern U32 g_FreeTiming;
extern U32 g_LastTiming;
extern U32 g_ReadTiming;
extern U32 g_WriteTiming;
static const U32 g_NumTimings = 4;
extern U32 g_HeadTiming[g_NumTimings];
extern U32 g_TailTiming[g_NumTimings];
extern U32 g_TimingKeys[E_Report_TimingCount / 2];
#endif

} // namespace Display2

#if !DONT_INLINE_DISPLAY2
#include "Display2/Context/GCMContextSPU.inl"
#endif

#endif // DISPLAY2_GCM_CONTEXT_SPU_H
