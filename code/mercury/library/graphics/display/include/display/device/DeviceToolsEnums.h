#ifndef DISPLAY_DEVICE_TOOLS_ENUMS_H
#define DISPLAY_DEVICE_TOOLS_ENUMS_H

namespace Display
{

enum E_Bool
{
	E_Bool_False = 0,
	E_Bool_True,
	E_Bool_Count,
	E_Bool_Default,
	E_Bool_Unknown,
};

enum E_Blend
{
	E_Blend_Unknown = 0,
	E_Blend_Zero,
	E_Blend_One,
	E_Blend_SrcColour,
	E_Blend_InvSrcColour,
	E_Blend_SrcAlpha,
	E_Blend_InvSrcAlpha,
	E_Blend_DstAlpha,
	E_Blend_InvDstAlpha,
	E_Blend_DstColour,
	E_Blend_InvDstColour,
};

enum E_BlendMode
{
	E_BlendMode_Solid = 0,
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
	E_BlendOp_Unknown = 0,
	E_BlendOp_Add,
	E_BlendOp_Subtract,
	E_BlendOp_RevSubtract,
	E_BlendOp_Min,
	E_BlendOp_Max,
};


enum E_CmpFunc
{
	E_CmpFunc_Unknown = 0,
	E_CmpFunc_Default,
	E_CmpFunc_Never,
	E_CmpFunc_Less,
	E_CmpFunc_Equal,
	E_CmpFunc_LessEqual,
	E_CmpFunc_Greater,
	E_CmpFunc_NotEqual,
	E_CmpFunc_GreaterEqual,
	E_CmpFunc_Always,
};

enum E_CubeTextureFace
{
	E_CubeTextureFace_PosX = 0,
	E_CubeTextureFace_PosY,
	E_CubeTextureFace_PosZ,
	E_CubeTextureFace_NegX,
	E_CubeTextureFace_NegY,
	E_CubeTextureFace_NegZ,
	E_CubeTextureFace_Count,
};

enum E_CullMode
{
	E_CullMode_Unknown = 0,
	E_CullMode_Default,
	E_CullMode_None,
	E_CullMode_Cw,
	E_CullMode_Ccw,
	E_CullMode_Reversed,
	E_CullMode_Front,
	E_CullMode_Back,
};

enum E_FillMode
{
	E_FillMode_Unknown = 0,
	E_FillMode_Default,
	E_FillMode_Point,
	E_FillMode_Wireframe,
	E_FillMode_Solid,
};

enum E_Format
{
	E_Format_Unknown = 0,
	E_Format_DXT1,
	E_Format_DXT3,
	E_Format_DXT5,
	E_Format_DXN,
	E_Format_L8,
	E_Format_A8,
	E_Format_A8L8,
	E_Format_L16,
	E_Format_A8R8G8B8,
	E_Format_X8R8G8B8,
	E_Format_G16R16,
	E_Format_G16R16F,
	E_Format_A16B16G16R16,
	E_Format_A16B16G16R16F,
	E_Format_A2R10G10B10,
	E_Format_A2B10G10R10F,
	E_Format_R16F,
	E_Format_R32F,
	E_Format_A32B32G32R32F,
	E_Format_D16,
	E_Format_D24S8,
	E_Format_D24FS8,
	E_Format_Index16,
	E_Format_Index32,
};

enum E_Multisample
{
	E_Multisample_None = 0,
	E_Multisample_2,
	E_Multisample_4,
	E_Multisample_8,
	E_Multisample_16,
	E_Multisample_Count,
};

enum E_PrimitiveType
{
	E_PrimitiveType_Unknown = 0,
	E_PrimitiveType_PointList,
	E_PrimitiveType_LineList,
	E_PrimitiveType_LineStrip,
	E_PrimitiveType_TriangleList,
	E_PrimitiveType_TriangleStrip,
	E_PrimitiveType_TriangleFan,
	E_PrimitiveType_QuadList,
	E_PrimitiveType_RectList,
};


enum E_ResourceType
{
	E_ResourceType_Unknown = 0,
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

enum E_StencilOp
{
	E_StencilOp_Keep = 0,
	E_StencilOp_Zero,
	E_StencilOp_Replace,
	E_StencilOp_IncrSat,
	E_StencilOp_DecrSat,
	E_StencilOp_Invert,
	E_StencilOp_Incr,
	E_StencilOp_Decr,
	E_StencilOp_Count,
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
	E_TextureAddress_Wrap = 0,
	E_TextureAddress_Clamp,
	E_TextureAddress_Mirror,
	E_TextureAddress_Border,
	E_TextureAddress_Count,
	E_TextureAddress_Default,
	E_TextureAddress_Unknown,
};

enum E_TextureFilter
{
	E_TextureFilter_None = 0,
	E_TextureFilter_Point,
	E_TextureFilter_Linear,
	E_TextureFilter_Anisotropic,
	E_TextureFilter_Count,
	E_TextureFilter_Default,
	E_TextureFilter_Unknown,
};

enum E_Flags
{
	E_Usage_Video			= 0x00000001,
	E_Usage_System			= 0x00000002,
	E_Usage_Managed			= 0x00000004,
	E_Usage_Bundled			= 0x00000008,
	
	E_Usage_Indirect		= 0x00000010,
	
	E_Usage_Dynamic			= 0x00000100,
	E_Usage_Static			= 0x00000200,

	E_Usage_Colour			= 0x00001000,
	E_Usage_DepthStencil	= 0x00002000,

	E_Format_Raw			= 0x00010000,
	E_Format_Tiled			= 0x00020000,
	E_Format_Linear			= 0x00040000,
	
	E_Format_SRGB			= 0x00100000,
	E_Usage_Expand			= 0x00200000,
	E_Usage_Uint			= 0x00400000,

	E_Usage_DoNotTrack		= 0x01000000,
	E_Usage_DoNotReload		= 0x02000000,

	E_Format_Cached			= 0x10000000,
	E_Usage_Streamed		= 0x20000000,
	E_Usage_ReadWrite		= 0x40000000,
};

enum E_VertexElementType
{
	E_VertexElementType_F32_1 = 0,
    E_VertexElementType_F32_2,
    E_VertexElementType_F32_3,
    E_VertexElementType_F32_4,
    E_VertexElementType_Colour,
    E_VertexElementType_U8_4,
    E_VertexElementType_U16_2,
    E_VertexElementType_U16_4,
	E_VertexElementType_S16_2,
	E_VertexElementType_S16_4,
    E_VertexElementType_U8_4N,
    E_VertexElementType_F16_2,
    E_VertexElementType_F16_4,
	E_VertexElementType_U10_3N,
	E_VertexElementType_U10_4N,
	E_VertexElementType_S16_2N,
	E_VertexElementType_SHHD_3,
	E_VertexElementType_SHHD_3N,
	E_VertexElementType_U32_1,
	E_VertexElementType_Count,
	E_VertexElementType_Unknown,
};

enum E_VertexElementUsage
{
	E_VertexElementUsage_Position = 0,
	E_VertexElementUsage_BlendWeight,
	E_VertexElementUsage_BlendIndices,
	E_VertexElementUsage_Normal,
	E_VertexElementUsage_PSize,
	E_VertexElementUsage_TexCoord,
	E_VertexElementUsage_Tangent,
	E_VertexElementUsage_Binormal,
	E_VertexElementUsage_TessFactor,
	E_VertexElementUsage_PositionT,
	E_VertexElementUsage_Colour,
	E_VertexElementUsage_Fog,
	E_VertexElementUsage_Depth,
	E_VertexElementUsage_Sample,
	E_VertexElementUsage_Count,
	E_VertexElementUsage_Unknown,
};

enum E_VertexSampler
{
	E_VertexSampler_0 = 0,
	E_VertexSampler_1,
	E_VertexSampler_2,
	E_VertexSampler_3,
	E_VertexSampler_Count,
};

} // namespace Display

#endif // DISPLAY_DEVICE_TOOLS_ENUMS_H
