#ifndef DISPLAY_DEVICE_DEVICE_TYPES_ENUMS_H
#define DISPLAY_DEVICE_DEVICE_TYPES_ENUMS_H

#include "Display/Device/DeviceDefines.h"

#include <Display2.h>

namespace Display
{

enum E_AspectRatio
{
	E_AspectRatio_4_3,
	E_AspectRatio_5_4,
	E_AspectRatio_16_9,
	E_AspectRatio_16_10,
	E_AspectRatio_Count,
	E_AspectRatio_Unknown,
};

enum E_Bool
{
	E_Bool_False							= Display2::E_Bool_False,
	E_Bool_True								= Display2::E_Bool_True,
	E_Bool_Count							= 2,
	E_Bool_Default							= Display2::E_Bool_Unknown-1,
	E_Bool_Unknown							= Display2::E_Bool_Unknown,
};

enum E_Blend
{
	E_Blend_Zero							= Display2::E_Blend_Zero,
	E_Blend_One								= Display2::E_Blend_One,
	E_Blend_SrcColour						= Display2::E_Blend_SrcColour,
	E_Blend_InvSrcColour					= Display2::E_Blend_InvSrcColour,
	E_Blend_SrcAlpha						= Display2::E_Blend_SrcAlpha,
	E_Blend_InvSrcAlpha						= Display2::E_Blend_InvSrcAlpha,
	E_Blend_DstAlpha						= Display2::E_Blend_DstAlpha,
	E_Blend_InvDstAlpha						= Display2::E_Blend_InvDstAlpha,
	E_Blend_DstColour						= Display2::E_Blend_DstColour,
	E_Blend_InvDstColour					= Display2::E_Blend_InvDstColour,
	E_Blend_Unknown							= Display2::E_Blend_Unknown,
};

enum E_BlendMode
{
	E_BlendMode_Solid,
	E_BlendMode_SolidAlphaTest,
	E_BlendMode_SolidAlphaMask,
	E_BlendMode_Alpha,
	E_BlendMode_AlphaPreMultiply,
	E_BlendMode_Additive,
	E_BlendMode_AlphaAdditive,
	E_BlendMode_AlphaSubtractive,
	E_BlendMode_Subtractive,
	E_BlendMode_Multiply,
	E_BlendMode_AdditiveSmooth,
	E_BlendMode_Count,
	E_BlendMode_Default,
	E_BlendMode_Unknown,
};

enum E_BlendOp
{
	E_BlendOp_Add							= Display2::E_BlendOp_Add,
	E_BlendOp_Subtract						= Display2::E_BlendOp_Subtract,
	E_BlendOp_RevSubtract					= Display2::E_BlendOp_RevSubtract,
	E_BlendOp_Min 							= Display2::E_BlendOp_Min,
	E_BlendOp_Max 							= Display2::E_BlendOp_Max,
	E_BlendOp_Unknown						= Display2::E_BlendOp_Unknown,
};

enum E_Clear
{
	E_Clear_Colour							= Display2::E_Clear_Colour,
	E_Clear_Depth							= Display2::E_Clear_Depth,
	E_Clear_Stencil							= Display2::E_Clear_Stencil,
};

enum E_CmpFunc
{
	E_CmpFunc_Never							= Display2::E_CmpFunc_Never,
	E_CmpFunc_Less							= Display2::E_CmpFunc_Less,
	E_CmpFunc_Equal							= Display2::E_CmpFunc_Equal,
	E_CmpFunc_LessEqual						= Display2::E_CmpFunc_LessEqual,
	E_CmpFunc_Greater						= Display2::E_CmpFunc_Greater,
	E_CmpFunc_NotEqual						= Display2::E_CmpFunc_NotEqual,
	E_CmpFunc_GreaterEqual					= Display2::E_CmpFunc_GreaterEqual,
	E_CmpFunc_Always						= Display2::E_CmpFunc_Always,
	E_CmpFunc_Default 						= Display2::E_CmpFunc_Unknown-1,
	E_CmpFunc_Unknown 						= Display2::E_CmpFunc_Unknown,
};

enum E_CubeTextureFace
{
	E_CubeTextureFace_PosX 					= Display2::E_CubeMapFace_PosX,
	E_CubeTextureFace_PosY 					= Display2::E_CubeMapFace_PosY,
	E_CubeTextureFace_PosZ 					= Display2::E_CubeMapFace_PosZ,
	E_CubeTextureFace_NegX 					= Display2::E_CubeMapFace_NegX,
	E_CubeTextureFace_NegY 					= Display2::E_CubeMapFace_NegY,
	E_CubeTextureFace_NegZ 					= Display2::E_CubeMapFace_NegZ,
	E_CubeTextureFace_Count					= 6,
};

enum E_CullMode
{
	E_CullMode_None							= Display2::E_CullMode_None,
	E_CullMode_Cw							= Display2::E_CullMode_Cw,
	E_CullMode_Ccw							= Display2::E_CullMode_Ccw,
	E_CullMode_Unknown						= Display2::E_CullMode_Unknown,
	E_CullMode_Default						= Display2::E_CullMode_Default, // Display2::E_CullMode_Unknown-1
	E_CullMode_Reversed						= Display2::E_CullMode_Reversed, // Display2::E_CullMode_Unknown-2
	E_CullMode_Front						= Display2::E_CullMode_Unknown-3,
	E_CullMode_Back							= Display2::E_CullMode_Unknown-4,
};

enum E_FillMode
{
	E_FillMode_Point						= Display2::E_FillMode_Point,
	E_FillMode_Wireframe					= Display2::E_FillMode_Wireframe,
	E_FillMode_Solid						= Display2::E_FillMode_Solid,
	E_FillMode_Default						= Display2::E_FillMode_Unknown-1,
	E_FillMode_Unknown						= Display2::E_FillMode_Unknown,
};

enum E_Format
{
	E_Format_Unknown						= Display2::E_TextureFormat_Unknown,
	E_Format_A8R8G8B8						= Display2::E_TextureFormat_A8R8G8B8,
	E_Format_DXT1							= Display2::E_TextureFormat_DXT1,
	E_Format_DXT3							= Display2::E_TextureFormat_DXT3,
	E_Format_DXT5							= Display2::E_TextureFormat_DXT5,
	E_Format_A16B16G16R16F					= Display2::E_TextureFormat_A16B16G16R16F,
	E_Format_A32B32G32R32F					= Display2::E_TextureFormat_A32B32G32R32F,
	E_Format_D16							= Display2::E_TextureFormat_D16,
	E_Format_D24S8							= Display2::E_TextureFormat_D24S8,
	E_Format_R32F							= Display2::E_TextureFormat_R32F,
	E_Format_L8								= Display2::E_TextureFormat_L8,
	E_Format_G16R16							= Display2::E_TextureFormat_G16R16,
	E_Format_G16R16F						= Display2::E_TextureFormat_G16R16F,

#if PLATFORM_PPU_SPU
	E_Format_D24FS8							= E_Format_D24S8,
	E_Format_A8								= E_Format_L8,
#else
	E_Format_D24FS8							= Display2::E_TextureFormat_D24FS8,
	E_Format_A8								= Display2::E_TextureFormat_A8,
#endif

#if !PLATFORM_PPU_SPU
	E_Format_A8L8							= Display2::E_TextureFormat_A8L8,
	E_Format_L16							= Display2::E_TextureFormat_L16,
	E_Format_X8R8G8B8						= Display2::E_TextureFormat_X8R8G8B8,
	E_Format_A16B16G16R16					= Display2::E_TextureFormat_A16B16G16R16,
	E_Format_A2R10G10B10					= Display2::E_TextureFormat_A2R10G10B10,
	E_Format_R16F							= Display2::E_TextureFormat_R16F,
#endif

	E_Format_A2B10G10R10F					= Display2::E_SurfaceFormat_Unknown-1,

	E_Format_Index16						= Display2::E_IndexFormat_16,
	E_Format_Index32						= Display2::E_IndexFormat_32,
};

INLINE E_Format GetFormatFromSurfaceFormat(const Display2::E_SurfaceFormat surfaceFormat)
{
	switch(surfaceFormat)
	{
	case Display2::E_SurfaceFormat_A8R8G8B8:				return E_Format_A8R8G8B8;
	case Display2::E_SurfaceFormat_A16B16G16R16F: 			return E_Format_A16B16G16R16F;
	case Display2::E_SurfaceFormat_A32B32G32R32F: 			return E_Format_A32B32G32R32F;
	case Display2::E_SurfaceFormat_D16:						return E_Format_D16;
	case Display2::E_SurfaceFormat_D24S8:					return E_Format_D24S8;
	case Display2::E_SurfaceFormat_R32F:					return E_Format_R32F;
	case Display2::E_SurfaceFormat_L8:						return E_Format_L8;
	case Display2::E_SurfaceFormat_Unknown:					return E_Format_Unknown;
	default:
		#ifndef CFG_GOLD
		CORE_PRINT("Unsupported surface format %s\n", Display2::GetSurfaceFormatAsString(surfaceFormat).Chars());
		#endif
		BIZ_ASSERT_FAIL("Unsupported surface format %i\n", surfaceFormat);
		return E_Format_Unknown;
	}
}

INLINE E_Format GetFormatFromTextureFormat(const Display2::E_TextureFormat textureFormat)
{
	switch(textureFormat)
	{
	case Display2::E_TextureFormat_A8R8G8B8:				return E_Format_A8R8G8B8;
	case Display2::E_TextureFormat_DXT1: 					return E_Format_DXT1;
	case Display2::E_TextureFormat_DXT3: 					return E_Format_DXT3;
	case Display2::E_TextureFormat_DXT5: 					return E_Format_DXT5;
	case Display2::E_TextureFormat_A16B16G16R16F: 			return E_Format_A16B16G16R16F;
	case Display2::E_TextureFormat_A32B32G32R32F: 			return E_Format_A32B32G32R32F;
	case Display2::E_TextureFormat_D16:						return E_Format_D16;
	case Display2::E_TextureFormat_D24S8:					return E_Format_D24S8;
	case Display2::E_TextureFormat_R32F:					return E_Format_R32F;
	case Display2::E_TextureFormat_L8:						return E_Format_L8;
	case Display2::E_TextureFormat_G16R16:					return E_Format_G16R16;
	case Display2::E_TextureFormat_G16R16F:					return E_Format_G16R16F;
	case Display2::E_TextureFormat_Unknown:					return E_Format_Unknown;
#if !PLATFORM_PPU_SPU
	case Display2::E_TextureFormat_X8R8G8B8:				return E_Format_X8R8G8B8;
#endif
	default:
		#ifndef CFG_GOLD
		CORE_PRINT("Unsupported texture format %s\n", Display2::GetTextureFormatAsString(textureFormat).Chars());
		#endif
		BIZ_ASSERT_FAIL("Unsupported texture format %i\n", textureFormat);
		return E_Format_Unknown;
	}
}

INLINE Display2::E_SurfaceFormat GetSurfaceFormatFromFormat(const E_Format format)
{
	switch(format)
	{
	case E_Format_A8R8G8B8:									return Display2::E_SurfaceFormat_A8R8G8B8;
	case E_Format_A16B16G16R16F: 							return Display2::E_SurfaceFormat_A16B16G16R16F;
	case E_Format_A32B32G32R32F: 							return Display2::E_SurfaceFormat_A32B32G32R32F;
	case E_Format_D16:										return Display2::E_SurfaceFormat_D16;
	case E_Format_D24S8:									return Display2::E_SurfaceFormat_D24S8;
	case E_Format_R32F:										return Display2::E_SurfaceFormat_R32F;
	case E_Format_L8:										return Display2::E_SurfaceFormat_L8;
	case E_Format_Unknown:									return Display2::E_SurfaceFormat_Unknown;
	default:
		#ifndef CFG_GOLD
		CORE_PRINT("Unsupported format %s when getting surface format\n", Display2::GetTextureFormatAsString((Display2::E_TextureFormat)format).Chars());
		#endif
		BIZ_ASSERT_FAIL("Unsupported format %i\n", format);
		return Display2::E_SurfaceFormat_Unknown;
	}
}

INLINE Display2::E_SurfaceFormat GetSurfaceFormat2(const E_Format format)
{
	return GetSurfaceFormatFromFormat(format);
}

INLINE Display2::E_TextureFormat GetTextureFormatFromFormat(const E_Format format)
{
	switch(format)
	{
	case E_Format_A8R8G8B8:									return Display2::E_TextureFormat_A8R8G8B8;
	case E_Format_DXT1: 									return Display2::E_TextureFormat_DXT1;
	case E_Format_DXT3: 									return Display2::E_TextureFormat_DXT3;
	case E_Format_DXT5: 									return Display2::E_TextureFormat_DXT5;
	case E_Format_A16B16G16R16F: 							return Display2::E_TextureFormat_A16B16G16R16F;
	case E_Format_A32B32G32R32F: 							return Display2::E_TextureFormat_A32B32G32R32F;
	case E_Format_D16:										return Display2::E_TextureFormat_D16;
	case E_Format_D24S8:									return Display2::E_TextureFormat_D24S8;
	case E_Format_R32F:										return Display2::E_TextureFormat_R32F;
	case E_Format_L8:										return Display2::E_TextureFormat_L8;
	case E_Format_G16R16:									return Display2::E_TextureFormat_G16R16;
	case E_Format_G16R16F:									return Display2::E_TextureFormat_G16R16F;
	case E_Format_Unknown:									return Display2::E_TextureFormat_Unknown;
#if !PLATFORM_PPU_SPU
	case E_Format_X8R8G8B8:									return Display2::E_TextureFormat_X8R8G8B8;
#endif
	default:
		#ifndef CFG_GOLD
		CORE_PRINT("Unsupported format %s when getting texture format\n", Display2::GetTextureFormatAsString((Display2::E_TextureFormat)format).Chars());
		#endif
		BIZ_ASSERT_FAIL("Unsupported format %i\n", format);
		return Display2::E_TextureFormat_Unknown;
	}
}

INLINE Display2::E_TextureFormat GetTextureFormat2(const E_Format format)
{
	return GetTextureFormatFromFormat(format);
}

enum E_LuminanceWrite
{
	E_LuminanceWrite_Enabled,
	E_LuminanceWrite_Disabled,
	E_LuminanceWrite_Default,
	E_LuminanceWrite_Unknown,
};

enum E_Multisample
{
	E_Multisample_None						= Display2::E_Multisample_None,
	E_Multisample_2							= Display2::E_Multisample_2,
	E_Multisample_4							= Display2::E_Multisample_4,
	E_Multisample_Count						= 3,
#ifdef PLATFORM_PC
	// [Blur2PC] retail Blur PC offers 8x and 16x and its values are the sample counts (retail
	// D3D9Manager::Initialise tests {0,2,4,8,16}; the AA option labels print the value with "%ix")
	E_Multisample_8							= 8,
	E_Multisample_16						= 16,
#else
	E_Multisample_8							= Display2::E_Multisample_Unknown-2,
	E_Multisample_16						= Display2::E_Multisample_Unknown-1,
#endif
	E_Multisample_Unknown					= Display2::E_Multisample_Unknown,
};

INLINE Display2::E_Multisample GetMultisample2(const E_Multisample multisample)
{
	switch(multisample)
	{
	case E_Multisample_None:				return Display2::E_Multisample_None;
	case E_Multisample_2:					return Display2::E_Multisample_2;
	case E_Multisample_4: 					return Display2::E_Multisample_4;
	default:
		BIZ_ASSERT_FAIL("Unsupported multisample %i\n", multisample);
		return Display2::E_Multisample_Unknown;
	}
}

INLINE E_Multisample GetMultisample(const Display2::E_Multisample multisample)
{
	switch(multisample)
	{
	case Display2::E_Multisample_None:		return E_Multisample_None;
	case Display2::E_Multisample_2:			return E_Multisample_2;
	case Display2::E_Multisample_4: 		return E_Multisample_4;
	default:
		BIZ_ASSERT_FAIL("Unsupported multisample2 %i\n", multisample);
		return E_Multisample_Unknown;
	}
}

enum E_PixelShaderProfile
{
	E_PixelShaderProfile_1_1,
	E_PixelShaderProfile_1_2,
	E_PixelShaderProfile_1_3,
	E_PixelShaderProfile_1_4,
	E_PixelShaderProfile_2_0,
	E_PixelShaderProfile_2_a,
	E_PixelShaderProfile_2_b,
	E_PixelShaderProfile_3_0,
	E_PixelShaderProfile_Count,
	E_PixelShaderProfile_Unknown,
};

enum E_PresentInterval
{
	E_PresentInterval_Immediate 			= Display2::E_PresentInterval_Immediate,
	E_PresentInterval_One					= Display2::E_PresentInterval_One,
	E_PresentInterval_Two					= Display2::E_PresentInterval_Two,
	E_PresentInterval_Three					= Display2::E_PresentInterval_Three,
	E_PresentInterval_Default				= Display2::E_PresentInterval_Default,
	E_PresentInterval_Count					= 6,
	E_PresentInterval_Unknown				= Display2::E_PresentInterval_Unknown,
};

enum E_PrimitiveType
{
	E_PrimitiveType_PointList				= Display2::E_PrimitiveType_PointList,
	E_PrimitiveType_LineList				= Display2::E_PrimitiveType_LineList,
	E_PrimitiveType_LineStrip				= Display2::E_PrimitiveType_LineStrip,
	E_PrimitiveType_TriangleList			= Display2::E_PrimitiveType_TriangleList,
	E_PrimitiveType_TriangleStrip			= Display2::E_PrimitiveType_TriangleStrip,
	E_PrimitiveType_TriangleFan				= Display2::E_PrimitiveType_TriangleFan,
	#if PLATFORM_PPU_SPU
	E_PrimitiveType_QuadList 				= Display2::E_PrimitiveType_QuadList,
	#else
	E_PrimitiveType_QuadList				= Display2::E_PrimitiveType_Unknown-2,
	#endif
	E_PrimitiveType_RectList				= Display2::E_PrimitiveType_Unknown-1,
	E_PrimitiveType_Unknown					= Display2::E_PrimitiveType_Unknown,
};

enum E_RefreshRate
{
	E_RefreshRate_50HZ,
	E_RefreshRate_60HZ,
};

enum E_ResourceType
{
	E_ResourceType_Unknown,
	E_ResourceType_Surface,
	E_ResourceType_Texture2D,
	E_ResourceType_Texture3D,
	E_ResourceType_TextureCube,
	E_ResourceType_VertexBuffer,
	E_ResourceType_IndexBuffer,
	E_ResourceType_VertexShader,
	E_ResourceType_PixelShader,
	E_ResourceType_Count,
};

enum E_Resolve
{
	E_Resolve_Colour						= Display2::E_Resolve_RenderTarget0,
	E_Resolve_Colour_1						= Display2::E_Resolve_RenderTarget1,
	E_Resolve_Colour_2						= Display2::E_Resolve_RenderTarget2,
	E_Resolve_Colour_3						= Display2::E_Resolve_RenderTarget3,
	E_Resolve_DepthStencil					= Display2::E_Resolve_DepthStencil,
};

enum E_Sampler
{
	E_Sampler_0 = 0,
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

enum E_ShaderProfile
{
	E_ShaderProfile_2_0 = 0,
	E_ShaderProfile_3_0,
	E_ShaderProfile_Count,
};

enum E_StencilOp
{
	E_StencilOp_Keep						= Display2::E_StencilOp_Keep,
	E_StencilOp_Zero						= Display2::E_StencilOp_Zero,
	E_StencilOp_Replace						= Display2::E_StencilOp_Replace,
	E_StencilOp_IncrSat						= Display2::E_StencilOp_IncrSat,
	E_StencilOp_DecrSat						= Display2::E_StencilOp_DecrSat,
	E_StencilOp_Invert						= Display2::E_StencilOp_Invert,
	E_StencilOp_Incr						= Display2::E_StencilOp_Incr,
	E_StencilOp_Decr						= Display2::E_StencilOp_Decr,
	E_StencilOp_Count						= 8,
};

enum E_Stream
{
	E_Stream_0 = 0,
	E_Stream_1,
	E_Stream_2,
	E_Stream_3,
	E_Stream_4,
	E_Stream_5,
	E_Stream_6,
	E_Stream_7,
	E_Stream_Count,
};

enum E_TextureAddress
{
	E_TextureAddress_Wrap					= Display2::E_TextureAddress_Wrap,
	E_TextureAddress_Clamp					= Display2::E_TextureAddress_Clamp,
	E_TextureAddress_Mirror					= Display2::E_TextureAddress_Mirror,
	E_TextureAddress_Border					= Display2::E_TextureAddress_Border,
	E_TextureAddress_Count					= 4,
	E_TextureAddress_Default				= Display2::E_TextureAddress_Unknown-1,
	E_TextureAddress_Unknown				= Display2::E_TextureAddress_Unknown,
};

enum E_TextureFilter
{
	E_TextureFilter_None					= Display2::E_TextureFilter_None,
	E_TextureFilter_Point					= Display2::E_TextureFilter_Point,
	E_TextureFilter_Linear					= Display2::E_TextureFilter_Linear,
	E_TextureFilter_Anisotropic				= Display2::E_TextureFilter_Anisotropic,
	E_TextureFilter_Count					= 4,
	E_TextureFilter_Default					= Display2::E_TextureFilter_Unknown-1,
	E_TextureFilter_Unknown					= Display2::E_TextureFilter_Unknown,
};

enum E_Flags
{
	E_Usage_Video							= Display2::E_Flags_Video,						// 	0x00000001
	E_Usage_System							= Display2::E_Flags_System,						// 	0x00000002
	E_Usage_Device							= Display2::E_Flags_Device,						// 	0x00000004
	
	E_Usage_Indirect						= Display2::E_Flags_Indirect,					// 	0x00000010
	E_Usage_AlignSizeToBlockSize			= Display2::E_Flags_AlignSizeToBlockSize,		// 	0x00000020
	E_Usage_Managed							= 													0x00000040,
	E_Usage_Bundled							= 													0x00000080,
	
	E_Usage_Dynamic							= Display2::E_Flags_Dynamic,					// 	0x00000100
	E_Usage_Static							=													0x00000200,

	E_Usage_Colour							= Display2::E_Flags_Colour,						// 	0x00001000
	E_Usage_DepthStencil					= Display2::E_Flags_DepthStencil,				// 	0x00002000

	E_Format_Raw							= Display2::E_Flags_Raw,						// 	0x00010000,
	E_Format_Tiled							= Display2::E_Flags_Tiled,						// 	0x00020000,
	E_Format_Linear							= Display2::E_Flags_Linear,						// 	0x00040000
	
	E_Format_SRGB							= Display2::E_Flags_SRGB,						//	0x00100000,
	E_Usage_Expand							= 													0x00200000,
	E_Usage_Uint							= 													0x00400000,

	E_Usage_DoNotTrack						= Display2::E_Flags_DoNotTrack,					//	0x01000000,
	E_Usage_DoNotReload						= Display2::E_Flags_DoNotReload,				//	0x02000000,

	E_Usage_ReadWrite						= Display2::E_Flags_ReadWrite,					//	0x10000000,
	E_Format_Cached							= 													0x20000000,
	E_Usage_Streamed						=													0x40000000,
};

enum E_VertexElementType
{
	E_VertexElementType_Unknown				= Display2::E_VertexElementType_Unknown,
	E_VertexElementType_F32_1 				= Display2::E_VertexElementType_F32_1,
    E_VertexElementType_F32_2 				= Display2::E_VertexElementType_F32_2,
    E_VertexElementType_F32_3 				= Display2::E_VertexElementType_F32_3,
    E_VertexElementType_F32_4 				= Display2::E_VertexElementType_F32_4,
    E_VertexElementType_Colour				= Display2::E_VertexElementType_Colour,
    E_VertexElementType_U8_4				= Display2::E_VertexElementType_U8_4,
    E_VertexElementType_U8_4N				= Display2::E_VertexElementType_U8_4N,
	E_VertexElementType_S16_2				= Display2::E_VertexElementType_S16_2,
	E_VertexElementType_S16_4				= Display2::E_VertexElementType_S16_4,
    E_VertexElementType_F16_2				= Display2::E_VertexElementType_F16_2,
    E_VertexElementType_F16_4				= Display2::E_VertexElementType_F16_4,
#if PLATFORM_PC
	E_VertexElementType_S16_2N 				= Display2::E_VertexElementType_S16_2N,
	E_VertexElementType_U16_2				= Display2::E_VertexElementType_Unknown+1,
	E_VertexElementType_U16_4				= Display2::E_VertexElementType_Unknown+2,
	E_VertexElementType_U10_3N 				= Display2::E_VertexElementType_Unknown+3,
	E_VertexElementType_U10_4N 				= Display2::E_VertexElementType_Unknown+4,
	E_VertexElementType_U32_1				= Display2::E_VertexElementType_Unknown+5,
	E_VertexElementType_SHHD_3				= Display2::E_VertexElementType_Unknown+6,
	E_VertexElementType_SHHD_3N				= Display2::E_VertexElementType_Unknown+7,
#elif PLATFORM_PPU_SPU
	E_VertexElementType_S16_2N 				= Display2::E_VertexElementType_Unknown+1,
	E_VertexElementType_U16_2				= Display2::E_VertexElementType_Unknown+2,
	E_VertexElementType_U16_4				= Display2::E_VertexElementType_Unknown+3,
	E_VertexElementType_U10_3N 				= Display2::E_VertexElementType_Unknown+4,
	E_VertexElementType_U10_4N 				= Display2::E_VertexElementType_Unknown+5,
	E_VertexElementType_U32_1				= Display2::E_VertexElementType_Unknown+6,
	E_VertexElementType_SHHD_3				= Display2::E_VertexElementType_SHHD_3,
	E_VertexElementType_SHHD_3N				= Display2::E_VertexElementType_SHHD_3N,
#endif
};

enum E_VertexElementUsage
{
	E_VertexElementUsage_Unknown			= Display2::E_VertexElementUsage_Unknown,
	E_VertexElementUsage_Position			= Display2::E_VertexElementUsage_Position,
	E_VertexElementUsage_BlendWeight		= Display2::E_VertexElementUsage_BlendWeight,
	E_VertexElementUsage_BlendIndices		= Display2::E_VertexElementUsage_BlendIndices,
	E_VertexElementUsage_Normal				= Display2::E_VertexElementUsage_Normal,
	E_VertexElementUsage_PSize				= Display2::E_VertexElementUsage_PSize,
	E_VertexElementUsage_TexCoord			= Display2::E_VertexElementUsage_TexCoord,
	E_VertexElementUsage_Tangent			= Display2::E_VertexElementUsage_Tangent,
	E_VertexElementUsage_Binormal			= Display2::E_VertexElementUsage_Binormal,
	E_VertexElementUsage_Colour				= Display2::E_VertexElementUsage_Colour,
	E_VertexElementUsage_Fog				= Display2::E_VertexElementUsage_Fog,
#if PLATFORM_PC
	E_VertexElementUsage_TessFactor			= Display2::E_VertexElementUsage_TessFactor,
	E_VertexElementUsage_Depth				= Display2::E_VertexElementUsage_Depth,
	E_VertexElementUsage_Sample				= Display2::E_VertexElementUsage_Sample,
	E_VertexElementUsage_PositionT			= Display2::E_VertexElementUsage_PositionT,
#elif PLATFORM_PPU_SPU
	E_VertexElementUsage_TessFactor			= Display2::E_VertexElementUsage_Unknown+1,
	E_VertexElementUsage_Depth				= Display2::E_VertexElementUsage_Unknown+2,
	E_VertexElementUsage_Sample				= Display2::E_VertexElementUsage_Unknown+3,
	E_VertexElementUsage_PositionT			= Display2::E_VertexElementUsage_Unknown+4,
#endif
};

enum E_VertexSampler
{
	E_VertexSampler_0,
	E_VertexSampler_1,
	E_VertexSampler_2,
	E_VertexSampler_3,
	E_VertexSampler_Count,
};

enum E_VertexShaderProfile
{
	E_VertexShaderProfile_1_1,
	E_VertexShaderProfile_2_0,
	E_VertexShaderProfile_2_a,
	E_VertexShaderProfile_3_0,
	E_VertexShaderProfile_Count,
	E_VertexShaderProfile_Unknown,
};

} // namespace Display

#endif // DISPLAY_DEVICE_DEVICE_TYPES_ENUMS_H
