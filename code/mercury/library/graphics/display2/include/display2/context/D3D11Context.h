// [Blur1PCDX11] Native Display2 declarations; legacy enum values describe engine data only.
#ifndef DISPLAY2_D3D11_CONTEXT_H
#define DISPLAY2_D3D11_CONTEXT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/Vector.h>
#include "Display2/Display2Defines.h"
#include "Display2/Context/D3D9Formats.h"
// [Blur1PCDX11] Existing asset and engine enums use D3D9 numeric values.
// This header supplies those constants; rendering uses ID3D11Device only.
#include <d3d9.h>
#include <d3d11.h>
#include <dxgi.h>
#include <vector>
#include "Display2/Device/D3D11TextureFormats.h"

namespace Display2
{

// [Blur1PCDX11] Call after a GPU copy, resolve or update writes a resource the
// game may lock, so its persistent staging mirror follows the new contents.
void D3D11ResourceWritten(ID3D11Resource* resource, ID3D11DeviceContext* context);
	
//------------------------------------------------------------------------------
// Defines

#define ALIGNMENT_SURFACE						D3DSURFACE_ALIGNMENT
#define ALIGNMENT_TEXTURE2D						D3DTEXTURE_ALIGNMENT
#define ALIGNMENT_TEXTURE3D						D3DVOLUME_ALIGNMENT
#define ALIGNMENT_TEXTURECUBE					D3DTEXTURE_ALIGNMENT
#define ALIGNMENT_VERTEXBUFFER					D3DVERTEXBUFFER_ALIGNMENT
#define ALIGNMENT_INDEXBUFFER					D3DINDEXBUFFER_ALIGNMENT
#define ALIGNMENT_VERTEXSHADER					VERTEXSHADER_ALIGNMENT
#define ALIGNMENT_PIXELSHADER					PIXELSHADER_ALIGNMENT


//------------------------------------------------------------------------------
// Macros

#define GPU_U8_4_TO_U32(r, g, b, a) ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff)

#define GPU_SAFE_RELEASE(_ptr) if(_ptr) { const ULONG refCount = _ptr->Release(); assert(refCount == 0); _ptr = NULL; }

//------------------------------------------------------------------------------
// Types

typedef IDXGIFactory1								GPUObject;
typedef ID3D11Device						GPUDevice;
typedef ID3D11InputLayout				GPUVertexDeclaration;
typedef ID3D11Buffer					GPUVertexBuffer;
typedef ID3D11Buffer					GPUIndexBuffer;
typedef ID3D11VertexShader					GPUVertexShader;
typedef ID3D11PixelShader					GPUPixelShader;
typedef ID3D11Texture2D						GPUSurface;
typedef ID3D11Resource					GPUTexture;
typedef ID3D11Texture2D						GPUTexture2D;
typedef ID3D11Texture3D					GPUTexture3D;
typedef ID3D11Texture2D					GPUTextureCube;
typedef	D3DVERTEXELEMENT9						GPUVertexElement;
typedef D3D11_VIEWPORT							GPUViewport;

//------------------------------------------------------------------------------
// Forward declarations

struct C_Texture;

//------------------------------------------------------------------------------
// Enums

//! E_AspectRatio

enum E_AspectRatio
{
	E_AspectRatio_4_3							= 0,
	E_AspectRatio_5_4,
	E_AspectRatio_16_9,
	E_AspectRatio_16_10,
	E_AspectRatio_Unknown,
};

//! E_Bool

enum E_Bool
{
	E_Bool_False								= FALSE,
	E_Bool_True									= TRUE,
	E_Bool_Unknown								= UINT_MAX,
};

//! E_Blend

enum E_Blend
{
	E_Blend_Zero								= D3DBLEND_ZERO,
	E_Blend_One									= D3DBLEND_ONE,
	E_Blend_SrcColour							= D3DBLEND_SRCCOLOR,
	E_Blend_InvSrcColour						= D3DBLEND_INVSRCCOLOR,
	E_Blend_SrcAlpha							= D3DBLEND_SRCALPHA,
	E_Blend_InvSrcAlpha							= D3DBLEND_INVSRCALPHA,
	E_Blend_DstAlpha							= D3DBLEND_DESTALPHA,
	E_Blend_InvDstAlpha							= D3DBLEND_INVDESTALPHA,
	E_Blend_DstColour							= D3DBLEND_DESTCOLOR,
	E_Blend_InvDstColour						= D3DBLEND_INVDESTCOLOR,
	E_Blend_Unknown								= UINT_MAX,
};

//! E_BlendMode

enum E_BlendMode
{
	E_BlendMode_Solid							= 0,
	E_BlendMode_SolidAlphaTest,
	E_BlendMode_Alpha,
	E_BlendMode_AlphaPreMultiply,
	E_BlendMode_Additive,
	E_BlendMode_AlphaAdditive,
	E_BlendMode_AlphaSubtractive,
	E_BlendMode_Subtractive,
	E_BlendMode_Multiply,
	E_BlendMode_Count,
};

//! E_BlendOp

enum E_BlendOp
{
	E_BlendOp_Add								= D3DBLENDOP_ADD,
	E_BlendOp_Subtract							= D3DBLENDOP_SUBTRACT,
	E_BlendOp_RevSubtract						= D3DBLENDOP_REVSUBTRACT,
	E_BlendOp_Min								= D3DBLENDOP_MIN,
	E_BlendOp_Max								= D3DBLENDOP_MAX,
	E_BlendOp_Unknown							= UINT_MAX,
};

//! E_Clear

enum E_Clear
{
	E_Clear_Colour								= D3DCLEAR_TARGET,
	E_Clear_Depth								= D3DCLEAR_ZBUFFER,
	E_Clear_Stencil								= D3DCLEAR_STENCIL,
};

//! E_CmpFunc

enum E_CmpFunc
{
	E_CmpFunc_Never								= D3DCMP_NEVER,
	E_CmpFunc_Less								= D3DCMP_LESS,
	E_CmpFunc_Equal								= D3DCMP_EQUAL,
	E_CmpFunc_LessEqual							= D3DCMP_LESSEQUAL,
	E_CmpFunc_Greater							= D3DCMP_GREATER,
	E_CmpFunc_NotEqual							= D3DCMP_NOTEQUAL,
	E_CmpFunc_GreaterEqual						= D3DCMP_GREATEREQUAL,
	E_CmpFunc_Always							= D3DCMP_ALWAYS,
	E_CmpFunc_Unknown							= UINT_MAX,
};

//! E_ColourWriteEnable

enum E_ColourWriteEnable
{
	E_ColourWriteEnable_Red						= 1L<<0,
	E_ColourWriteEnable_Green					= 1L<<1,
	E_ColourWriteEnable_Blue					= 1L<<2,
	E_ColourWriteEnable_Alpha					= 1L<<3,
	E_ColourWriteEnable_All						= 0xf,
	E_ColourWriteEnable_Unknown 				= UINT_MAX,
};

//! E_CubeMapFace

enum E_CubeMapFace
{
	E_CubeMapFace_PosX							= D3DCUBEMAP_FACE_POSITIVE_X,
	E_CubeMapFace_NegX							= D3DCUBEMAP_FACE_NEGATIVE_X,
	E_CubeMapFace_PosY							= D3DCUBEMAP_FACE_POSITIVE_Y,
	E_CubeMapFace_NegY							= D3DCUBEMAP_FACE_NEGATIVE_Y,
	E_CubeMapFace_PosZ							= D3DCUBEMAP_FACE_POSITIVE_Z,
	E_CubeMapFace_NegZ							= D3DCUBEMAP_FACE_NEGATIVE_Z,
};

//! E_CullMode

enum E_CullMode
{
	E_CullMode_None								= D3DCULL_NONE,
	E_CullMode_Cw								= D3DCULL_CW,
	E_CullMode_Ccw								= D3DCULL_CCW,
	E_CullMode_Unknown							= UINT_MAX,
	E_CullMode_Default							= E_CullMode_Unknown-1,
	E_CullMode_Reversed							= E_CullMode_Unknown-2,
};

//! E_DeviceType

enum E_DeviceType
{
	E_DeviceType_HAL							= D3DDEVTYPE_HAL,
	#if DEVICE_D3D11_PC
	E_DeviceType_REF							= D3DDEVTYPE_REF,
	#endif
	E_DeviceType_Unknown						= UINT_MAX,
};

//! E_FillMode

enum E_FillMode
{
	E_FillMode_Point							= D3DFILL_POINT,
	E_FillMode_Wireframe						= D3DFILL_WIREFRAME,
	E_FillMode_Solid							= D3DFILL_SOLID,
	E_FillMode_Unknown							= UINT_MAX,
};

//! E_Flags

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

	E_Flags_SRGB								= 0x00100000,

	E_Flags_DoNotTrack							= 0x01000000,
	E_Flags_DoNotReload							= 0x02000000,

	E_Flags_ReadWrite							= 0x10000000,
};

//! E_IndexFormat

enum E_IndexFormat
{
	E_IndexFormat_16 							= D3DFMT_INDEX16,
	E_IndexFormat_32 							= D3DFMT_INDEX32,
	E_IndexFormat_Unknown						= UINT_MAX,
};

//! E_Multisample

enum E_Multisample
{
	E_Multisample_None							= D3DMULTISAMPLE_NONE,
	E_Multisample_2								= D3DMULTISAMPLE_2_SAMPLES,
	E_Multisample_4								= D3DMULTISAMPLE_4_SAMPLES,
	E_Multisample_Unknown						= UINT_MAX,
};

//! E_PresentInterval

enum E_PresentInterval
{
	E_PresentInterval_Immediate					= D3DPRESENT_INTERVAL_IMMEDIATE,
	E_PresentInterval_One						= D3DPRESENT_INTERVAL_ONE,
	E_PresentInterval_Two						= D3DPRESENT_INTERVAL_TWO,
	E_PresentInterval_Three						= D3DPRESENT_INTERVAL_THREE,
	E_PresentInterval_Four						= D3DPRESENT_INTERVAL_FOUR,
	E_PresentInterval_Default					= D3DPRESENT_INTERVAL_DEFAULT,
	E_PresentInterval_Unknown					= UINT_MAX,
};

//! E_PrimitiveType

enum E_PrimitiveType
{
	E_PrimitiveType_PointList					= D3DPT_POINTLIST,
	E_PrimitiveType_LineList					= D3DPT_LINELIST,
	E_PrimitiveType_LineStrip					= D3DPT_LINESTRIP,
	E_PrimitiveType_TriangleList				= D3DPT_TRIANGLELIST,
	E_PrimitiveType_TriangleStrip				= D3DPT_TRIANGLESTRIP,
	E_PrimitiveType_TriangleFan					= D3DPT_TRIANGLEFAN,
	E_PrimitiveType_Unknown						= UINT_MAX,
};

//! E_RefreshRate

enum E_RefreshRate
{
	E_RefreshRate_30HZ							= 0,
	E_RefreshRate_50HZ,
	E_RefreshRate_60HZ,
	E_RefreshRate_Unknown,
};

//! E_RenderState

enum E_RenderState
{
	E_RenderState_AlphaBlendEnable				= D3DRS_ALPHABLENDENABLE,
	E_RenderState_AlphaFunc						= D3DRS_ALPHAFUNC,
	E_RenderState_AlphaRef						= D3DRS_ALPHAREF,
	E_RenderState_AlphaTestEnable				= D3DRS_ALPHATESTENABLE,
	E_RenderState_BlendOp						= D3DRS_BLENDOP,
	E_RenderState_ColorWriteEnable				= D3DRS_COLORWRITEENABLE,
	E_RenderState_CullMode						= D3DRS_CULLMODE,
	E_RenderState_DepthBias						= D3DRS_DEPTHBIAS,
	E_RenderState_DestBlend						= D3DRS_DESTBLEND,
	E_RenderState_FillMode						= D3DRS_FILLMODE,
	E_RenderState_SlopeScaleDepthBias			= D3DRS_SLOPESCALEDEPTHBIAS,
	E_RenderState_SrcBlend						= D3DRS_SRCBLEND,
	E_RenderState_StencilEnable					= D3DRS_STENCILENABLE,
	E_RenderState_StencilFail					= D3DRS_STENCILFAIL,
	E_RenderState_StencilFunc					= D3DRS_STENCILFUNC,
	E_RenderState_StencilMask					= D3DRS_STENCILMASK,
	E_RenderState_StencilPass					= D3DRS_STENCILPASS,
	E_RenderState_StencilRef					= D3DRS_STENCILREF,
	E_RenderState_StencilWriteMask				= D3DRS_STENCILWRITEMASK,
	E_RenderState_StencilZFail					= D3DRS_STENCILZFAIL,
	E_RenderState_ZEnable						= D3DRS_ZENABLE,
	E_RenderState_ZFunc							= D3DRS_ZFUNC,
	E_RenderState_ZWriteEnable					= D3DRS_ZWRITEENABLE,
	E_RenderState_Unknown						= D3DRS_FORCE_DWORD,
};

//! E_Resolve

enum E_Resolve
{
	E_Resolve_RenderTarget0,
	E_Resolve_RenderTarget1,
	E_Resolve_RenderTarget2,
	E_Resolve_RenderTarget3,
	E_Resolve_DepthStencil,
	E_Resolve_AllFragments,
};

//! E_ResourceType

enum E_ResourceType
{
	E_ResourceType_Surface						= D3DRTYPE_SURFACE,
	E_ResourceType_Texture2D					= D3DRTYPE_TEXTURE,
	E_ResourceType_Texture3D					= D3DRTYPE_VOLUMETEXTURE,
	E_ResourceType_TextureCube					= D3DRTYPE_CUBETEXTURE,
	E_ResourceType_VertexBuffer					= D3DRTYPE_VERTEXBUFFER,
	E_ResourceType_IndexBuffer					= D3DRTYPE_INDEXBUFFER,
	E_ResourceType_VertexShader,
	E_ResourceType_PixelShader,
	E_ResourceType_Unknown						= D3DRTYPE_FORCE_DWORD,
};

//! E_Sampler

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

//! E_SamplerState

enum E_SamplerState
{
	E_SamplerState_AddressU						= D3DSAMP_ADDRESSU,
	E_SamplerState_AddressV						= D3DSAMP_ADDRESSV,
	E_SamplerState_AddressW						= D3DSAMP_ADDRESSW,
	E_SamplerState_BorderColor					= D3DSAMP_BORDERCOLOR,
	E_SamplerState_MagFilter					= D3DSAMP_MAGFILTER,
	E_SamplerState_MinFilter					= D3DSAMP_MINFILTER,
	E_SamplerState_MipFilter					= D3DSAMP_MIPFILTER,
	E_SamplerState_Unknown						= D3DSAMP_FORCE_DWORD,
};

//! E_ShaderProfile

enum E_ShaderProfile
{
	E_ShaderProfile_2_0							= 0,
	E_ShaderProfile_3_0,
	E_ShaderProfile_Count,
};

//! E_StencilOp

enum E_StencilOp
{
	E_StencilOp_Keep							= D3DSTENCILOP_KEEP,
	E_StencilOp_Zero							= D3DSTENCILOP_ZERO,
	E_StencilOp_Replace							= D3DSTENCILOP_REPLACE,
	E_StencilOp_IncrSat							= D3DSTENCILOP_INCRSAT,
	E_StencilOp_DecrSat							= D3DSTENCILOP_DECRSAT,
	E_StencilOp_Invert							= D3DSTENCILOP_INVERT,
	E_StencilOp_Incr							= D3DSTENCILOP_INCR,
	E_StencilOp_Decr							= D3DSTENCILOP_DECR,
	E_StencilOp_Unknown							= UINT_MAX,
};

//! E_Stream

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

//! E_SurfaceFormat

enum E_SurfaceFormat
{
	// 8bpp:
	E_SurfaceFormat_A8 							= D3DFMT_A8,
	E_SurfaceFormat_L8 							= D3DFMT_L8,

	// 16bpp:
	E_SurfaceFormat_R5G6B5 						= D3DFMT_R5G6B5,
	E_SurfaceFormat_L6V5U5 						= D3DFMT_L6V5U5,
	E_SurfaceFormat_X1R5G5B5 					= D3DFMT_X1R5G5B5,
	E_SurfaceFormat_A1R5G5B5 					= D3DFMT_A1R5G5B5,
	E_SurfaceFormat_A4R4G4B4 					= D3DFMT_A4R4G4B4,
	E_SurfaceFormat_X4R4G4B4 					= D3DFMT_X4R4G4B4,
	E_SurfaceFormat_A8L8 						= D3DFMT_A8L8,
	E_SurfaceFormat_V8U8 						= D3DFMT_V8U8,
	E_SurfaceFormat_D16 						= D3DFMT_D16,
	E_SurfaceFormat_L16 						= D3DFMT_L16,
	E_SurfaceFormat_R16F						= D3DFMT_R16F,
	E_SurfaceFormat_UYVY						= D3DFMT_UYVY,
	E_SurfaceFormat_G8R8_G8B8 					= D3DFMT_G8R8_G8B8,
	E_SurfaceFormat_R8G8_B8G8 					= D3DFMT_R8G8_B8G8,
	E_SurfaceFormat_YUY2						= D3DFMT_YUY2,

	// 32bpp:
	E_SurfaceFormat_A8R8G8B8 					= D3DFMT_A8R8G8B8,
	E_SurfaceFormat_X8R8G8B8 					= D3DFMT_X8R8G8B8,
	E_SurfaceFormat_A8B8G8R8 					= D3DFMT_A8B8G8R8,
	E_SurfaceFormat_X8B8G8R8 					= D3DFMT_X8B8G8R8,
	E_SurfaceFormat_X8L8V8U8 					= D3DFMT_X8L8V8U8,
	E_SurfaceFormat_Q8W8V8U8 					= D3DFMT_Q8W8V8U8,
	E_SurfaceFormat_A2R10G10B10 				= D3DFMT_A2R10G10B10,
	E_SurfaceFormat_A2B10G10R10 				= D3DFMT_A2B10G10R10,
	E_SurfaceFormat_A2W10V10U10 				= D3DFMT_A2W10V10U10,
	E_SurfaceFormat_G16R16 						= D3DFMT_G16R16,
	E_SurfaceFormat_V16U16 						= D3DFMT_V16U16,
	E_SurfaceFormat_G16R16F						= D3DFMT_G16R16F,
	E_SurfaceFormat_R32F						= D3DFMT_R32F,
	E_SurfaceFormat_D24S8						= D3DFMT_D24S8,
	E_SurfaceFormat_D24X8						= D3DFMT_D24X8,
	E_SurfaceFormat_D24FS8						= D3DFMT_D24FS8,
	E_SurfaceFormat_D32							= D3DFMT_D32,

	// 64bpp:
	E_SurfaceFormat_A16B16G16R16 				= D3DFMT_A16B16G16R16,
	E_SurfaceFormat_Q16W16V16U16 				= D3DFMT_Q16W16V16U16,
	E_SurfaceFormat_A16B16G16R16F				= D3DFMT_A16B16G16R16F,
	E_SurfaceFormat_G32R32F						= D3DFMT_G32R32F,

	// 128bpp:
	E_SurfaceFormat_A32B32G32R32F				= D3DFMT_A32B32G32R32F,

	// EDRAM only:

	// Front buffer formats have to be little endian:

	E_SurfaceFormat_Unknown						= UINT_MAX,
};

//! E_TextureAddress

enum E_TextureAddress
{
	E_TextureAddress_Wrap						= D3DTADDRESS_WRAP,
	E_TextureAddress_Clamp						= D3DTADDRESS_CLAMP,
	E_TextureAddress_ClampToEdge				= D3DTADDRESS_CLAMP,
	E_TextureAddress_Mirror						= D3DTADDRESS_MIRROR,
	E_TextureAddress_Border						= D3DTADDRESS_BORDER,
	E_TextureAddress_Unknown					= UINT_MAX,
};

//! E_TextureFilter

enum E_TextureFilter
{
	E_TextureFilter_None						= D3DTEXF_NONE,
	E_TextureFilter_Point						= D3DTEXF_POINT,
	E_TextureFilter_Linear						= D3DTEXF_LINEAR,
	E_TextureFilter_Anisotropic					= D3DTEXF_ANISOTROPIC,
	E_TextureFilter_Unknown						= UINT_MAX,
};

//! E_TextureFormat

enum E_TextureFormat
{
	// DXT:
	E_TextureFormat_DXT1 						= D3DFMT_DXT1,
	E_TextureFormat_DXT2 						= D3DFMT_DXT2,
	E_TextureFormat_DXT3 						= D3DFMT_DXT3,
	E_TextureFormat_DXT4 						= D3DFMT_DXT4,
	E_TextureFormat_DXT5 						= D3DFMT_DXT5,

	// 8bpp:
	E_TextureFormat_A8 							= D3DFMT_A8,
	E_TextureFormat_L8 							= D3DFMT_L8,

	// 16bpp:
	E_TextureFormat_R5G6B5 						= D3DFMT_R5G6B5,
	E_TextureFormat_L6V5U5 						= D3DFMT_L6V5U5,
	E_TextureFormat_X1R5G5B5 					= D3DFMT_X1R5G5B5,
	E_TextureFormat_A1R5G5B5 					= D3DFMT_A1R5G5B5,
	E_TextureFormat_A4R4G4B4 					= D3DFMT_A4R4G4B4,
	E_TextureFormat_X4R4G4B4 					= D3DFMT_X4R4G4B4,
	E_TextureFormat_A8L8 						= D3DFMT_A8L8,
	E_TextureFormat_V8U8 						= D3DFMT_V8U8,
	E_TextureFormat_D16 						= D3DFMT_D16,
	E_TextureFormat_L16 						= D3DFMT_L16,
	E_TextureFormat_R16F						= D3DFMT_R16F,
	E_TextureFormat_UYVY						= D3DFMT_UYVY,
	E_TextureFormat_G8R8_G8B8 					= D3DFMT_G8R8_G8B8,
	E_TextureFormat_R8G8_B8G8 					= D3DFMT_R8G8_B8G8,
	E_TextureFormat_YUY2						= D3DFMT_YUY2,

	// 32bpp:
	E_TextureFormat_A8R8G8B8 					= D3DFMT_A8R8G8B8,
	E_TextureFormat_X8R8G8B8 					= D3DFMT_X8R8G8B8,
	E_TextureFormat_A8B8G8R8 					= D3DFMT_A8B8G8R8,
	E_TextureFormat_X8B8G8R8 					= D3DFMT_X8B8G8R8,
	E_TextureFormat_X8L8V8U8 					= D3DFMT_X8L8V8U8,
	E_TextureFormat_Q8W8V8U8 					= D3DFMT_Q8W8V8U8,
	E_TextureFormat_A2R10G10B10 				= D3DFMT_A2R10G10B10,
	E_TextureFormat_A2B10G10R10 				= D3DFMT_A2B10G10R10,
	E_TextureFormat_A2W10V10U10 				= D3DFMT_A2W10V10U10,
	E_TextureFormat_G16R16 						= D3DFMT_G16R16,
	E_TextureFormat_V16U16 						= D3DFMT_V16U16,
	E_TextureFormat_G16R16F						= D3DFMT_G16R16F,
	E_TextureFormat_R32F						= D3DFMT_R32F,
	E_TextureFormat_D24S8						= D3DFMT_D24S8,
	E_TextureFormat_D24X8						= D3DFMT_D24X8,
	E_TextureFormat_D24FS8						= D3DFMT_D24FS8,
	E_TextureFormat_D32							= D3DFMT_D32,

	// 64bpp:
	E_TextureFormat_A16B16G16R16 				= D3DFMT_A16B16G16R16,
	E_TextureFormat_Q16W16V16U16 				= D3DFMT_Q16W16V16U16,
	E_TextureFormat_A16B16G16R16F				= D3DFMT_A16B16G16R16F,
	E_TextureFormat_G32R32F						= D3DFMT_G32R32F,

	// 128bpp:
	E_TextureFormat_A32B32G32R32F				= D3DFMT_A32B32G32R32F,
	
	// EDRAM only:

	// Front buffer formats have to be little endian:

	E_TextureFormat_Unknown						= UINT_MAX,
};

//! E_TextureType

enum E_TextureType
{
	E_TextureType_2D							= D3DRTYPE_TEXTURE,
	E_TextureType_3D							= D3DRTYPE_VOLUMETEXTURE,
	E_TextureType_Cube							= D3DRTYPE_CUBETEXTURE,
	E_TextureType_Unknown						= D3DRTYPE_FORCE_DWORD,
};

//! E_VertexElementType

enum E_VertexElementType
{
	// Cross-platform
	E_VertexElementType_F32_1					= D3DDECLTYPE_FLOAT1,
    E_VertexElementType_F32_2					= D3DDECLTYPE_FLOAT2,
    E_VertexElementType_F32_3					= D3DDECLTYPE_FLOAT3,
    E_VertexElementType_F32_4					= D3DDECLTYPE_FLOAT4,
	E_VertexElementType_Colour					= D3DDECLTYPE_D3DCOLOR,
	E_VertexElementType_U8_4					= D3DDECLTYPE_UBYTE4,
    E_VertexElementType_U8_4N					= D3DDECLTYPE_UBYTE4N,
	E_VertexElementType_S16_2					= D3DDECLTYPE_SHORT2,
	E_VertexElementType_S16_4					= D3DDECLTYPE_SHORT4,
	E_VertexElementType_F16_2					= D3DDECLTYPE_FLOAT16_2,
    E_VertexElementType_F16_4					= D3DDECLTYPE_FLOAT16_4,
	E_VertexElementType_S16_2N					= D3DDECLTYPE_SHORT2N,
	
	// Not cross-platform
	E_VertexElementType_S16_4N					= D3DDECLTYPE_SHORT4N,
#if DEVICE_D3D11_PC
	E_VertexElementType_U10_3					= D3DDECLTYPE_UDEC3,
	E_VertexElementType_S10_3N					= D3DDECLTYPE_DEC3N,
#endif

	// Cross-platform
	E_VertexElementType_Unknown					= D3DDECLTYPE_UNUSED,
};

//! E_VertexElementUsage

enum E_VertexElementUsage
{
	// Cross-platform
	E_VertexElementUsage_Position				= D3DDECLUSAGE_POSITION,
	E_VertexElementUsage_BlendWeight			= D3DDECLUSAGE_BLENDWEIGHT,
	E_VertexElementUsage_BlendIndices			= D3DDECLUSAGE_BLENDINDICES,
	E_VertexElementUsage_Normal					= D3DDECLUSAGE_NORMAL,
	E_VertexElementUsage_PSize					= D3DDECLUSAGE_PSIZE,
	E_VertexElementUsage_TexCoord				= D3DDECLUSAGE_TEXCOORD,
	E_VertexElementUsage_Tangent				= D3DDECLUSAGE_TANGENT,
	E_VertexElementUsage_Binormal				= D3DDECLUSAGE_BINORMAL,
	E_VertexElementUsage_Colour					= D3DDECLUSAGE_COLOR,
	E_VertexElementUsage_Fog					= D3DDECLUSAGE_FOG,

	// Not cross-platform
	E_VertexElementUsage_TessFactor				= D3DDECLUSAGE_TESSFACTOR,
	E_VertexElementUsage_Depth					= D3DDECLUSAGE_DEPTH,
	E_VertexElementUsage_Sample					= D3DDECLUSAGE_SAMPLE,
#if DEVICE_D3D11_PC
	E_VertexElementUsage_PositionT				= D3DDECLUSAGE_POSITIONT,
#endif

	// Cross-platform
	E_VertexElementUsage_Unknown				= MAXD3DDECLUSAGE+1,
};

//! E_VertexSampler

enum E_VertexSampler
{
	E_VertexSampler_0							= 0,
	E_VertexSampler_1,
	E_VertexSampler_2,
	E_VertexSampler_3,
	E_VertexSampler_Count,
};

//------------------------------------------------------------------------------
// Globals

//! g_pd3dObject

extern GPUObject* g_pd3dObject;

//! g_pd3dDevice

extern GPUDevice* g_pd3dDevice;

//------------------------------------------------------------------------------
// Asserts

#define GPU_ASSERT(_x) \
	BIZ_ASSERT( \
		(_x));

#define GPU_ASSERT_BITCOUNT(_state, _bitcount) \
	BIZ_ASSERT(_state <= _bitcount<<_bitcount);

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

#define GPU_ASSERT_BLENDOP(_blendOp) \
	BIZ_ASSERT( \
		(_blendOp) == E_BlendOp_Add || \
		(_blendOp) == E_BlendOp_Subtract || \
		(_blendOp) == E_BlendOp_RevSubtract || \
		(_blendOp) == E_BlendOp_Min || \
		(_blendOp) == E_BlendOp_Max);

#define GPU_ASSERT_CMPFUNC(_cmpFunc) \
	BIZ_ASSERT( \
		(_cmpFunc) == E_CmpFunc_Never || \
		(_cmpFunc) == E_CmpFunc_Less || \
		(_cmpFunc) == E_CmpFunc_LessEqual || \
		(_cmpFunc) == E_CmpFunc_Greater || \
		(_cmpFunc) == E_CmpFunc_GreaterEqual || \
		(_cmpFunc) == E_CmpFunc_Equal || \
		(_cmpFunc) == E_CmpFunc_NotEqual || \
		(_cmpFunc) == E_CmpFunc_Always);

#define GPU_ASSERT_CULLMODE(_cullMode) \
	BIZ_ASSERT( \
		(_cullMode) == E_CullMode_None || \
		(_cullMode) == E_CullMode_Cw || \
		(_cullMode) == E_CullMode_Ccw);

#define GPU_ASSERT_FILLMODE(_fillMode) \
	BIZ_ASSERT( \
		(_fillMode) == E_FillMode_Point || \
		(_fillMode) == E_FillMode_Wireframe || \
		(_fillMode) == E_FillMode_Solid);

#define GPU_ASSERT_INDEXFORMAT(_indexFormat) \
	BIZ_ASSERT( \
		(_indexFormat) == E_IndexFormat_16 || \
		(_indexFormat) == E_IndexFormat_32);

#define D3D_ASSERT_PRIMITIVETYPE(_primitiveType) \
		(_primitiveType) == E_PrimitiveType_PointList || \
		(_primitiveType) == E_PrimitiveType_LineList || \
		(_primitiveType) == E_PrimitiveType_LineStrip || \
		(_primitiveType) == E_PrimitiveType_TriangleList || \
		(_primitiveType) == E_PrimitiveType_TriangleStrip \

#define GPU_ASSERT_PRIMITIVETYPE(_primitiveType) \
	BIZ_ASSERT( \
		D3D_ASSERT_PRIMITIVETYPE(_primitiveType));

#define GPU_ASSERT_STENCILOP(_stencilOp) \
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

#define GPU_ASSERT_TEXTUREADDRESS(_textureAddress) \
	BIZ_ASSERT( \
		(_textureAddress) == E_TextureAddress_Wrap || \
		(_textureAddress) == E_TextureAddress_Clamp || \
		(_textureAddress) == E_TextureAddress_Mirror || \
		(_textureAddress) == E_TextureAddress_Border);

#define GPU_ASSERT_TEXTUREFILTER(_textureFilter) \
	BIZ_ASSERT( \
		(_textureFilter) == E_TextureFilter_None || \
		(_textureFilter) == E_TextureFilter_Point || \
		(_textureFilter) == E_TextureFilter_Linear || \
		(_textureFilter) == E_TextureFilter_Anisotropic);

#define GPU_ASSERT_TEXTUREBORDERCOLOUR(_textureBorderColour)

//------------------------------------------------------------------------------
// Functions

E_SurfaceFormat GetSurfaceFormatFromTexture(const E_TextureFormat format);

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

U32 GetTexture2DLevelOffset(const U32 level, U32 width, U32 height, const U32 levels, const U32 flags, const E_TextureFormat format);

U32 GetTexture3DLevelOffset(const U32 level, U32 width, U32 height, U32 depth, const U32 levels, const U32 flags, const E_TextureFormat format);

U32 GetTextureCubeLevelOffset(const E_CubeMapFace face, const U32 level, U32 width, const U32 levels, const U32 flags, const E_TextureFormat format);

U32 SetTexture2DHeader(const U32 width, const U32 height, const U32 levels, const U32 flags, const E_TextureFormat format, const U32 pitch, GPUTexture2D* pTexture);

U32 SetTexture3DHeader(const U32 width, const U32 height, const U32 depth, const U32 levels, const U32 flags, const E_TextureFormat format, const U32 pitch, GPUTexture3D* pTexture);

U32 SetTextureCubeHeader(const U32 width, const U32 levels, const U32 flags, const E_TextureFormat format, const U32 pitch, GPUTextureCube* pTexture);

const Util::C_String GetFlagsAsString(const U32 flags);

const Util::C_String GetIndexFormatAsString(const E_IndexFormat indexFormat);

const Util::C_String GetMultisampleAsString(const E_Multisample multisample);

const Util::C_String GetSurfaceFormatAsString(const E_SurfaceFormat format);

const Util::C_String GetTextureFormatAsString(const E_TextureFormat format);

const Util::C_String GetTextureTypeAsString(const E_TextureType type);

bool DebugCheckTexture(const C_Texture* pTexture);

bool DebugCheckGPUTexture(const GPUTexture* pgpuTexture);

//------------------------------------------------------------------------------
// Direct3D functions

DWORD D3DUsage(const U32 flags);

DWORD D3DTextureUsage(const U32 flags);

D3DPOOL D3DPool(const U32 flags);

U32 D3DPrimitiveCount(const U32 primitiveType, const U32 indexCount);

const Util::C_String GetD3DFormatAsString(const D3DFORMAT format);

const Util::C_String GetD3DMultisampleTypeAsString(const D3DMULTISAMPLE_TYPE multisampleType);

const Util::C_String GetD3DSwapEffectAsString(const D3DSWAPEFFECT swapEffect);

const Util::C_String GetD3DPresentFlagsAsString(const DWORD presentFlags);

const Util::C_String GetD3DPresentIntervalAsString(const UINT presentInterval);

//------------------------------------------------------------------------------
// Structs

//! C_Viewport

struct C_Viewport : public GPUViewport
{
	C_Viewport();
	C_Viewport(const U32 x, const U32 y, const U32 width, const U32 height);
	C_Viewport(const U32 x, const U32 y, const U32 width, const U32 height, const F32 minZ, const F32 maxZ);

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
};

//! C_VertexElement

struct C_VertexElement : public GPUVertexElement
{
	C_VertexElement();
	C_VertexElement(const U16 stream, const U32 offset, const U32 type, const U8 usage, const U8 usageIndex);

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
};

#define GPU_VERTEX_ELEMENT_END() Display2::C_VertexElement(0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0)

//! C_VertexDeclaration

struct C_VertexDeclaration
{
	C_VertexDeclaration();
	~C_VertexDeclaration();

	bool IsCreated() const;

	S32 GetElementCount() const;
	bool GetElements(const U32 elementCount, C_VertexElement* pElements) const;
	U32 GetStride(const U32 stream = 0) const;

	GPUVertexDeclaration* GetGPUVertexDeclaration();
	const GPUVertexDeclaration* GetGPUVertexDeclaration() const;

	static const U32							m_MaxElementCount = 12;
	GPUVertexDeclaration*						m_pd3dVertexDeclaration;
	U32										m_ElementCount; // [Blur1PCDX11] Input layout metadata before a shader is bound.
	C_VertexElement							m_Elements[m_MaxElementCount];
};

//! C_VertexBuffer

struct C_VertexBuffer
{
	C_VertexBuffer();
	~C_VertexBuffer();

	bool IsCreated() const;
	
	U32 GetSize() const;
	U32 GetFlags() const;
	void* GetAddress() const;

	void SetSize(const U32 size);
	void SetFlags(const U32 flags);
	void SetAddress(void* pAddress);

	GPUVertexBuffer* GetGPUVertexBuffer();
	const GPUVertexBuffer* GetGPUVertexBuffer() const;

	void* Lock(const U32 offset, const U32 size, const U32 flags = 0);
	void* Lock();
	void Unlock();

	GPUVertexBuffer*							m_pd3dVertexBuffer;

	U32											m_Flags;
};

//! C_IndexBuffer

struct C_IndexBuffer
{
	C_IndexBuffer();
	~C_IndexBuffer();

	bool IsCreated() const;

	U32 GetSize() const;
	U32 GetFlags() const;
	void* GetAddress() const;
	E_IndexFormat GetFormat() const;

	void SetSize(const U32 size);
	void SetFlags(const U32 flags);
	void SetAddress(void* pAddress);
	
	GPUIndexBuffer* GetGPUIndexBuffer();
	const GPUIndexBuffer* GetGPUIndexBuffer() const;

	void* Lock(const U32 offset, const U32 size, const U32 flags = 0);
	void* Lock();
	void Unlock();

	GPUIndexBuffer*				m_pd3dIndexBuffer;

	U32							m_Flags;
	E_IndexFormat				m_Format; // [Blur1PCDX11] D3D11 index buffers do not encode element width.
};

//! C_ShaderSemantic

struct C_ShaderSemantic
{
	C_ShaderSemantic();

	U8							m_Usage;
	U8							m_UsageIndex;
};

//! C_ShaderParam

struct C_ShaderParam
{
	C_ShaderParam();
	C_ShaderParam(const U32 registerIndex, const U32 registerCount);
	
	void SetupFromFunc(const char* name, const U8* func, U32 size);

	U32							m_RegisterIndex;
	U32							m_RegisterCount;
};

typedef C_ShaderParam C_VertexShaderParam;
typedef C_ShaderParam C_PixelShaderParam;
typedef C_ShaderParam C_PixelShaderParamArray;

//! C_VertexShader

struct C_VertexShader
{
	C_VertexShader();
	~C_VertexShader();

	bool IsCreated() const;

	U32 GetSize() const;
	U32 GetFlags() const;
	void* GetAddress() const;

	void SetFlags(const U32 flags);

	U8 GetRegisterCount() const;
	void SetRegisterCount(const U8 registerCount);

	GPUVertexShader* GetGPUVertexShader();
	const GPUVertexShader* GetGPUVertexShader() const;

	const C_ShaderParam GetParamFromName(const char* name) const;
	const C_ShaderParam GetParameter(const S32 index, const char* name) const;

	bool GetSemantics(Btl::Vector<C_ShaderSemantic>& semantics);

	GPUVertexShader*			m_pd3dVertexShader;
	
	U32							m_Flags;
	U32							m_Size; // [Blur1PCDX11] Shader bytecode length is kept by the engine.
	std::vector<U8>					m_Bytecode;
};

//! C_PixelShader

struct C_PixelShader
{
	C_PixelShader();
	~C_PixelShader();

	bool IsCreated() const;

	U32 GetSize() const;
	U32 GetFlags() const;
	void* GetAddress() const;

	void SetFlags(const U32 flags);
	void SetAddress(U8* pAddress);

	U8 GetRegisterCount() const;
	void SetRegisterCount(const U8 registerCount);

	GPUPixelShader* GetGPUPixelShader();
	const GPUPixelShader* GetGPUPixelShader() const;

	const C_ShaderParam GetParamFromName(const char* name) const;
	const C_ShaderParam GetParameter(const S32 index, const char* name) const;
	const C_ShaderParam GetParameterArray(const S32 index, const char* name, const U32 count) const;

	GPUPixelShader*				m_pd3dPixelShader;

	U32							m_Flags;
	U32							m_Size; // [Blur1PCDX11] Shader bytecode length is kept by the engine.
	std::vector<U8>					m_Bytecode;

};


//! C_LockedRect

struct C_LockedRect
{
	C_LockedRect();

	S32 GetPitch() const;
    U8* GetBits() const;

	U8*							pBits;
	U32							pitch;
};

//! C_ConstLockedRect

struct C_ConstLockedRect
{
	C_ConstLockedRect();

	S32 GetPitch() const;
    const U8* GetBits() const;

	const U8*					pBits;
	U32							pitch;
};

//! C_LockedBox

struct C_LockedBox
{
	C_LockedBox();

	S32 GetRowPitch() const;
	S32 GetSlicePitch() const;
    U8* GetBits() const;

	U8*							pBits;
	U32							rowPitch;
	U32							slicePitch;
};

//! C_ConstLockedBox

struct C_ConstLockedBox
{
	C_ConstLockedBox();

	S32 GetRowPitch() const;
	S32 GetSlicePitch() const;
    const U8* GetBits() const;

	const U8*					pBits;
	U32							rowPitch;
	U32							slicePitch;
};

//! C_SurfaceParams

struct C_Surface;

struct C_SurfaceParams
{
	C_SurfaceParams();
	~C_SurfaceParams();

	void OverSurface(const C_Surface* pSurface);
	void AfterSurface(const C_Surface* pSurface);
	void BeforeSurface(const C_Surface* pSurface);
};

//! C_Surface

struct C_Surface
{
	C_Surface();
	~C_Surface();

	bool IsCreated() const;

	U32 GetSize() const;
	U32 GetFlags() const;
	U8* GetAddress() const;

	U32 GetWidth() const;
	U32 GetHeight() const;
	E_SurfaceFormat GetFormat() const;
	E_Multisample GetMultisample() const;

	void SetSize(const U32 size);
	void SetFlags(const U32 flags);
	void SetAddress(U8* pAddress);

	GPUSurface* GetGPUSurface();
	const GPUSurface* GetGPUSurface() const;
	
	C_LockedRect LockRect(const U32 flags = 0);
	void UnlockRect();

	GPUSurface*					m_pd3dSurface;
	U32							m_Flags;
	U32							m_Subresource; // [Blur1PCDX11] mip/array slice for native views.
};

//! C_Texture

struct C_Texture
{
	C_Texture();
	~C_Texture();

	bool IsCreated() const;

	E_TextureType GetType() const;
	U32 GetSize() const;
	U32 GetFlags() const;
	U8* GetAddress() const;
	
	U32 GetWidth(const U32 level = 0) const;
	U32 GetHeight(const U32 level = 0) const;
	U32 GetDepth(const U32 level = 0) const;
	U32 GetLevels() const;
	E_TextureFormat GetFormat() const;

	void SetSize(const U32 size);
	void SetFlags(const U32 flags);
	void SetAddress(U8* pAddress);
	void SetSRGB(const bool enable = true);

	GPUTexture* GetGPUTexture();
	const GPUTexture* GetGPUTexture() const;

	GPUTexture2D* GetGPUTexture2D();
	const GPUTexture2D* GetGPUTexture2D() const;

	GPUTexture3D* GetGPUTexture3D();
	const GPUTexture3D* GetGPUTexture3D() const;

	GPUTextureCube* GetGPUTextureCube();
	const GPUTextureCube* GetGPUTextureCube() const;

	C_LockedRect LockRect2D(const U32 level, const U32 flags = 0);
	C_ConstLockedRect LockRect2D(const U32 level, const U32 flags = 0) const;
	void UnlockRect2D(const U32 level) const;

	C_LockedBox LockBox3D(const U32 level, const U32 flags = 0);
	C_ConstLockedBox LockBox3D(const U32 level, const U32 flags = 0) const;
	void UnlockBox3D(const U32 level) const;

	C_LockedRect LockRectCube(const U32 face, const U32 level, const U32 flags = 0);
	C_ConstLockedRect LockRectCube(const U32 face, const U32 level, const U32 flags = 0) const;
	void UnlockRectCube(const U32 face, const U32 level) const;

	// Compat for Display
	GPUTexture* Get();
	const Util::C_String GetFileName() const;
	C_LockedRect Lock(const U32 level);
	C_ConstLockedRect Lock(const U32 level) const;
	void Unlock(const U32 level) const;
	
	U32							m_Size;
	U32							m_Flags;

	GPUTexture*					m_pd3dTexture;
};

//! C_RenderTexture2D

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

#if DEVICE_D3D11_PC // [Blur1PCDX11] Preserve PC read/write target slots.
	mutable U16						m_TextureIndex;
	mutable U16						m_SurfaceIndex;
	C_Texture						m_Texture[2];
	Btl::Vector<C_Surface>			m_Surfaces[2];
	// [Blur2PC] retail Blur PC (Blur.exe 1.2 0x005c5270): when multisampled, m_Surfaces holds a separate
	// multisampled render target and this the texture's own level 0 surface, which Resolve copies into
	Btl::Vector<C_Surface>			m_ResolveSurfaces[2];
#else
	C_Texture						m_Texture;
	Btl::Vector<C_Surface>			m_Surfaces;
#endif
	U32								m_Flags;
#if DEVICE_D3D11_PC
	U32								m_Multisample;	// [Blur2PC] retail +0x50: D3DMULTISAMPLE_TYPE of the render surface
#endif
};

//! C_RenderTextureCube

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

	C_Texture					m_Texture;
	Btl::Vector<C_Surface>		m_Surfaces[6];
	U32							m_Flags;
};

//! C_VideoMode

struct C_VideoMode
{
};

struct C_RenderBufferSetup
{
};

struct C_RenderBuffer
{
};

//------------------------------------------------------------------------------
// States

#define GPU_ALPHATEST_CMPFUNC_BITCOUNT	4
#define GPU_ALPHATEST_REF_BITCOUNT		8

//! C_AlphaTestState

struct C_AlphaTestState
{
	void SetDefaultStates();
	void SetAlphaTestCmpFunc(const E_CmpFunc alphaTestCmpFunc);
	void SetAlphaTestRef(const U8 alphaTestRef);

	union
	{
		struct
		{
			U32							m_AlphaTestCmpFunc	: GPU_ALPHATEST_CMPFUNC_BITCOUNT;	// E_CmpFunc
			U32							m_AlphaTestRef		: GPU_ALPHATEST_REF_BITCOUNT;		// U8
		};
		U32								m_AlphaTestBits;
	};
};

#define GPU_BLEND_COLORSRC_BITCOUNT		4
#define GPU_BLEND_COLORDST_BITCOUNT		4
#define GPU_BLEND_COLOROP_BITCOUNT		4
#define GPU_BLEND_ALPHASRC_BITCOUNT		4
#define GPU_BLEND_ALPHADST_BITCOUNT		4
#define GPU_BLEND_ALPHA_OPBITCOUNT		4

//! C_BlendState

struct C_BlendState
{
	void SetDefaultStates();
	void SetBlendColourSrc(const E_Blend blendColorSrc);
	void SetBlendColourDst(const E_Blend blendColorDst);
	void SetBlendColourOp(const E_BlendOp blendColorOp);
	void SetBlendAlphaSrc(const E_Blend blendAlphaSrc);
	void SetBlendAlphaDst(const E_Blend blendAlphaDst);
	void SetBlendAlphaOp(const E_BlendOp blendAlphaOp);

	union
	{
		struct
		{
			U32							m_BlendColorSrc		: GPU_BLEND_COLORSRC_BITCOUNT;	// E_Blend
			U32							m_BlendColorDst		: GPU_BLEND_COLORDST_BITCOUNT;	// E_Blend
			U32							m_BlendColorOp		: GPU_BLEND_COLOROP_BITCOUNT;	// E_BlendOp
			U32							m_BlendAlphaSrc		: GPU_BLEND_ALPHASRC_BITCOUNT;	// E_Blend
			U32							m_BlendAlphaDst		: GPU_BLEND_ALPHADST_BITCOUNT;	// E_Blend
			U32							m_BlendAlphaOp		: GPU_BLEND_ALPHA_OPBITCOUNT;	// E_BlendOp
		};
		U32								m_BlendBits;
	};
};

//! C_ColourState

struct C_ColourState
{
	void SetDefaultStates();
	void SetColourWriteEnable(const U32 colourWriteEnable);

	U32									m_ColourWriteEnable;
};

//! C_CullState

struct C_CullState
{
	void SetDefaultStates();
	void SetCullMode(const E_CullMode cullMode);

	U32									m_CullMode;
};

#define GPU_DEPTH_FUNC_BITCOUNT			4
#define GPU_DEPTH_ENABLE_BITCOUNT		1
#define GPU_DEPTH_WRITEENABLE_BITCOUNT	1

//! C_DepthState

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
			U32							m_DepthFunc			: GPU_DEPTH_FUNC_BITCOUNT; // E_CmpFunc
			U32							m_DepthEnable		: GPU_DEPTH_ENABLE_BITCOUNT; // E_Bool
			U32							m_DepthWriteEnable	: GPU_DEPTH_WRITEENABLE_BITCOUNT; // E_Bool
		};
		U32								m_DepthBits;
	};
	F32									m_DepthBias;
	F32									m_SlopeScaledBias;
};

//! C_FillState

struct C_FillState
{
	void SetDefaultStates();
	void SetFillMode(const U32 fillMode);

	U32									m_FillMode;
};

//! C_TextureState

struct C_TextureState
{
	void SetDefaultStates();
	void SetTexture(const C_Texture* pTexture);
	
	const C_Texture*					m_pTexture;
};

#define GPU_TEXTURE_ADDRESSU_BITCOUNT 	4
#define GPU_TEXTURE_ADDRESSV_BITCOUNT 	4
#define GPU_TEXTURE_ADDRESSW_BITCOUNT 	4
#define GPU_TEXTURE_FILTERMAG_BITCOUNT 	4
#define GPU_TEXTURE_FILTERMIN_BITCOUNT 	4
#define GPU_TEXTURE_FILTERMIP_BITCOUNT 	4
#define GPU_TEXTURE_GAMMA_BITCOUNT 		1

//! C_SamplerState

struct C_SamplerState
{
	void SetDefaultStates();
	bool IsSame(const C_SamplerState &other);
	void SetTextureAddressU(const E_TextureAddress textureAddressU);
	void SetTextureAddressV(const E_TextureAddress textureAddressV);
	void SetTextureAddressW(const E_TextureAddress textureAddressW);
	void SetTextureAddress(const E_TextureAddress textureAddressU, const E_TextureAddress textureAddressV);
	void SetTextureAddress(const E_TextureAddress textureAddressU, const E_TextureAddress textureAddressV, const E_TextureAddress textureAddressW);
	void SetTextureAddress(const E_TextureAddress textureAddress);
	void SetTextureFilter(const E_TextureFilter textureFilterMag, const E_TextureFilter textureFilterMin, const E_TextureFilter textureFilterMip);
	void SetTextureFilter(const E_TextureFilter textureFilter);
	void SetTextureBorderColour(const U32 textureBorderColour);
	void SetTextureLodBias(const S8 textureLodBias);
	
	#if DEVICE_D3D11_PC
	void SetTextureGamma(const E_Bool textureGamma);
	#endif
	
	union
	{
		struct
		{
			U32							m_TextureAddressU		: GPU_TEXTURE_ADDRESSU_BITCOUNT; // E_TextureAddress
			U32							m_TextureAddressV		: GPU_TEXTURE_ADDRESSV_BITCOUNT; // E_TextureAddress
			U32							m_TextureAddressW		: GPU_TEXTURE_ADDRESSW_BITCOUNT; // E_TextureAddress
			U32							m_TextureFilterMag		: GPU_TEXTURE_FILTERMAG_BITCOUNT; // E_TextureFilter
			U32							m_TextureFilterMin		: GPU_TEXTURE_FILTERMIN_BITCOUNT; // E_TextureFilter
			U32							m_TextureFilterMip		: GPU_TEXTURE_FILTERMIP_BITCOUNT; // E_TextureFilter
			#if DEVICE_D3D11_PC
			U32							m_TextureGamma			: GPU_TEXTURE_GAMMA_BITCOUNT; // E_Bool
			#endif
		};
		U32								m_TextureBits;
	};
	U32 								m_TextureBorderColour;
	S32 								m_TextureLodBias;
};

//! C_ScissorState

struct C_ScissorState
{
	void SetDefaultStates();
	void SetScissorEnable(const E_Bool scissorEnable);
	void SetScissorRect(const RECT& scissorRect);

	U32									m_ScissorEnable;
	RECT								m_ScissorRect;
};

#define GPU_STENCIL_FUNC_BITCOUNT		4
#define GPU_STENCIL_FAIL_BITCOUNT		4
#define GPU_STENCIL_DEPTHFAIL_BITCOUNT	4
#define GPU_STENCIL_DEPTHPASS_BITCOUNT	4

//! C_StencilState

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

	union
	{
		struct
		{
			U32							m_StencilFunc			: GPU_STENCIL_FUNC_BITCOUNT; // E_CmpFunc
			U32							m_StencilFail			: GPU_STENCIL_FAIL_BITCOUNT; // E_StencilOp
			U32							m_StencilDepthFail		: GPU_STENCIL_DEPTHFAIL_BITCOUNT; // E_StencilOp
			U32							m_StencilDepthPass		: GPU_STENCIL_DEPTHPASS_BITCOUNT; // E_StencilOp
		};
		U32								m_StencilBits;
	};
	U32									m_StencilRef;
	U32									m_StencilMask;
	U32									m_StencilWriteMask;
};

//! C_VertexTextureState

typedef C_TextureState C_VertexTextureState;

//! C_VertexSamplerState

typedef C_SamplerState C_VertexSamplerState;

//! C_ContextState

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

//! C_BlendMode

struct C_BlendMode
{
	void SetDefaultStates();
	
	C_AlphaTestState					m_AlphaTestState;
	C_BlendState						m_BlendState;
};

//------------------------------------------------------------------------------
// Context

//#define GPU_USEVOIDS 1

#if GPU_USEVOIDS
    #define GPU_VOID					void
	#define GPU_CALL(_func)				_func;
    #define GPU_RETURN_OK				return
	#define GPU_RETURN_FAIL				return
	#define GPU_RETURN_VOID				return
#else
    #define GPU_VOID					HRESULT
	#define GPU_CALL(_func)				{ HRESULT hr; if(FAILED(hr = _func)) return hr; }
    #define GPU_RETURN_OK				return S_OK
	#define GPU_RETURN_FAIL				return E_FAIL
	#define GPU_RETURN_VOID				return S_OK
#endif

//! C_ContextBase

struct C_ContextBase
{
	C_ContextBase();
	C_ContextBase(GPUDevice* pd3dDevice);

	GPUDevice* GetD3DDevice();
	const GPUDevice* GetD3DDevice() const;
	void SetD3DDevice(GPUDevice* pd3dDevice);

	GPUDevice*							m_pd3dDevice;
};

//! C_Context

#define GPU_CLASS 						C_Context
#define GPU_CHECK 						1
#define GPU_TRACK 						1

struct C_Context : public C_ContextBase
{
	C_Context();
	C_Context(GPUDevice* pd3dDevice);

	C_ContextState* GetState();
	const C_ContextState* GetState() const;
	void SetState(C_ContextState* pd3dDevice);

	#include "Display2/Context/D3D9Functions.h"

	C_ContextState*						m_pState;
};

#if !DONT_INLINE_DISPLAY2
// [Blur1PCDX11] D3D11 methods are implemented by the native backend.
#endif

#undef GPU_TRACK
#undef GPU_CHECK
#undef GPU_CLASS

//! C_ContextNoCheck

#define GPU_CLASS 						C_ContextNoCheck
#define GPU_TRACK 						1

struct C_ContextNoCheck : public C_ContextBase
{
	C_ContextNoCheck();
	C_ContextNoCheck(GPUDevice* pd3dDevice);

	C_ContextState* GetState();
	const C_ContextState* GetState() const;
	void SetState(C_ContextState* pd3dDevice);

	#include "Display2/Context/D3D9Functions.h"

	C_ContextState*						m_pState;
};

#if !DONT_INLINE_DISPLAY2
// [Blur1PCDX11] D3D11 methods are implemented by the native backend.
#endif

#undef GPU_TRACK
#undef GPU_CLASS

//! C_ContextNoCheckNoTrack

#define GPU_CLASS						C_ContextNoCheckNoTrack

struct C_ContextNoCheckNoTrack : public C_ContextBase
{
	C_ContextNoCheckNoTrack();
	C_ContextNoCheckNoTrack(GPUDevice* pd3dDevice);

	#include "Display2/Context/D3D9Functions.h"
};

#if !DONT_INLINE_DISPLAY2
// [Blur1PCDX11] D3D11 methods are implemented by the native backend.
#endif

#undef GPU_CLASS

} // namespace Display2

#if !DONT_INLINE_DISPLAY2
#include "Display2/Context/D3D11Context.inl" // [Blur1PCDX11] Native accessor definitions.
#endif

#endif // DISPLAY2_D3D11_CONTEXT_H
