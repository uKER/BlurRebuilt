#ifndef DISPLAY2_WII_CONTEXT_H
#define DISPLAY2_WII_CONTEXT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display2
{

//------------------------------------------------------------------------------
// Enums

//! E_AspectRatio

enum E_AspectRatio
{
	E_AspectRatio_4_3,
	E_AspectRatio_5_4,
	E_AspectRatio_16_9,
	E_AspectRatio_16_10,
	E_AspectRatio_Unknown,
};

//! E_Bool

enum E_Bool
{
	E_Bool_False,
	E_Bool_True,
	E_Bool_Unknown,
};

//! E_Blend

enum E_Blend
{
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
	E_Blend_Unknown,
};

//! E_BlendMode

enum E_BlendMode
{
	E_BlendMode_Solid,
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
	E_BlendOp_Add,
	E_BlendOp_Subtract,
	E_BlendOp_RevSubtract,
	E_BlendOp_Min,
	E_BlendOp_Max,
	E_BlendOp_Unknown,
};

//! E_Clear

enum E_Clear
{
	E_Clear_Colour,
	E_Clear_Depth,
	E_Clear_Stencil,
};

//! E_CmpFunc

enum E_CmpFunc
{
	E_CmpFunc_Never,
	E_CmpFunc_Less,
	E_CmpFunc_Equal,
	E_CmpFunc_LessEqual,
	E_CmpFunc_Greater,
	E_CmpFunc_NotEqual,
	E_CmpFunc_GreaterEqual,
	E_CmpFunc_Always,
	E_CmpFunc_Unknown,
};

//! E_ColourWriteEnable

enum E_ColourWriteEnable
{
	E_ColourWriteEnable_Red,
	E_ColourWriteEnable_Green,
	E_ColourWriteEnable_Blue,
	E_ColourWriteEnable_Alpha,
	E_ColourWriteEnable_All,
	E_ColourWriteEnable_Unknown,
};

//! E_CullMode

enum E_CullMode
{
	E_CullMode_None,
	E_CullMode_Cw,
	E_CullMode_Ccw,
	E_CullMode_Default,
	E_CullMode_Reversed,
	E_CullMode_Unknown,
};

//! E_CubeMapFace

enum E_CubeMapFace
{
	E_CubeMapFace_PosX,
	E_CubeMapFace_NegX,
	E_CubeMapFace_PosY,
	E_CubeMapFace_NegY,
	E_CubeMapFace_PosZ,
	E_CubeMapFace_NegZ,
};

//! E_FillMode

enum E_FillMode
{
	E_FillMode_Point,
	E_FillMode_Wireframe,
	E_FillMode_Solid,
	E_FillMode_Unknown,
};

//! E_Flags

enum E_Flags
{
	E_Flags_Video								= 0x00000001,
	E_Flags_System								= 0x00000002,

	E_Flags_Indirect							= 0x00000010,
	
	E_Flags_Dynamic								= 0x00000100,
	
	E_Flags_Colour								= 0x00001000,
	E_Flags_DepthStencil						= 0x00002000,
	
	E_Flags_Raw									= 0x00010000,
	E_Flags_Tiled								= 0x00020000,
	E_Flags_Linear								= 0x00040000,

	E_Flags_SRGB								= 0x00100000,

	E_Flags_DoNotTrack							= 0x01000000,
	E_Flags_DoNotReload							= 0x02000000,
};

//! E_IndexFormat

enum E_IndexFormat
{
	E_IndexFormat_16,
	E_IndexFormat_32,
	E_IndexFormat_Unknown,
};

//! E_Multisample

enum E_Multisample
{
	E_Multisample_None,
	E_Multisample_2,
	E_Multisample_4,
	E_Multisample_Unknown,
};

//! E_PresentInterval

enum E_PresentInterval
{
	E_PresentInterval_Immediate,
	E_PresentInterval_One,
	E_PresentInterval_Two,
	E_PresentInterval_Three,
	E_PresentInterval_Four,
	E_PresentInterval_Default,
	E_PresentInterval_Unknown,
};

//! E_PrimitiveType

enum E_PrimitiveType
{
	E_PrimitiveType_PointList,
	E_PrimitiveType_LineList,
	E_PrimitiveType_LineStrip,
	E_PrimitiveType_TriangleList,
	E_PrimitiveType_TriangleStrip,
	E_PrimitiveType_TriangleFan,
	E_PrimitiveType_QuadList,
	E_PrimitiveType_Unknown,
};

//! E_RefreshRate

enum E_RefreshRate
{
	E_RefreshRate_30HZ,
	E_RefreshRate_50HZ,
	E_RefreshRate_60HZ,
	E_RefreshRate_Unknown,
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
	E_ResourceType_Surface,
	E_ResourceType_Texture2D,
	E_ResourceType_Texture3D,
	E_ResourceType_TextureCube,
	E_ResourceType_VertexBuffer,
	E_ResourceType_IndexBuffer,
	E_ResourceType_VertexShader,
	E_ResourceType_PixelShader,
	E_ResourceType_Unknown,
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

//! E_StencilOp

enum E_StencilOp
{
	E_StencilOp_Keep,
	E_StencilOp_Zero,
	E_StencilOp_Replace,
	E_StencilOp_IncrSat,
	E_StencilOp_DecrSat,
	E_StencilOp_Invert,
	E_StencilOp_Incr,
	E_StencilOp_Decr,
	E_StencilOp_Unknown,
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
	E_SurfaceFormat_A8R8G8B8,
	E_SurfaceFormat_A16B16G16R16F,
	E_SurfaceFormat_A32B32G32R32F,
	E_SurfaceFormat_D16,
	E_SurfaceFormat_D24S8,
	E_SurfaceFormat_R32F,
	E_SurfaceFormat_L8,
	E_SurfaceFormat_Unknown,
};

//! E_TextureAddress

enum E_TextureAddress
{
	E_TextureAddress_Wrap,
	E_TextureAddress_Clamp,
	E_TextureAddress_Mirror,
	E_TextureAddress_Border,
	E_TextureAddress_Unknown,
};

//! E_TextureFilter

enum E_TextureFilter
{
	E_TextureFilter_None,
	E_TextureFilter_Point,
	E_TextureFilter_Linear,
	E_TextureFilter_Anisotropic,
	E_TextureFilter_Unknown,
};

//! E_TextureFormat

enum E_TextureFormat
{
	E_TextureFormat_A8R8G8B8,
	E_TextureFormat_DXT1,
	E_TextureFormat_DXT3,
	E_TextureFormat_DXT5,
	E_TextureFormat_A16B16G16R16F,
	E_TextureFormat_A32B32G32R32F,
	E_TextureFormat_D16,
	E_TextureFormat_D24S8,
	E_TextureFormat_R32F,
	E_TextureFormat_L8,
	E_TextureFormat_G16R16,
	E_TextureFormat_G16R16F,
	E_TextureFormat_Unknown,
};

//! E_TextureType

enum E_TextureType
{
	E_TextureType_2D,
	E_TextureType_3D,
	E_TextureType_Cube,
	E_TextureType_Unknown,
};

//! E_VertexElementType

enum E_VertexElementType
{
	E_VertexElementType_F32_1,
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
	E_VertexElementType_Unknown,
};

//! E_VertexElementUsage

enum E_VertexElementUsage
{
	E_VertexElementUsage_Position,
	E_VertexElementUsage_BlendWeight,
	E_VertexElementUsage_BlendIndices,
	E_VertexElementUsage_Normal,
	E_VertexElementUsage_PSize,
	E_VertexElementUsage_TexCoord,
	E_VertexElementUsage_Tangent,
	E_VertexElementUsage_Binormal,
	E_VertexElementUsage_Colour,
	E_VertexElementUsage_Fog,
	E_VertexElementUsage_Unknown,
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

const Util::C_String GetTextureFormatAsString(const E_TextureFormat format);

//------------------------------------------------------------------------------
// Structs

//! C_Viewport

struct C_Viewport
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

struct C_VertexElement
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

#define GPU_VERTEX_ELEMENT_END() Display2::C_VertexElement(Display2::E_Stream_Count, 0, Display2::E_VertexElementType_Unknown, Display2::E_VertexElementUsage_Unknown, 0, 0, 0, 0)

//! C_VertexDeclaration

struct C_VertexDeclaration
{
	C_VertexDeclaration();
	~C_VertexDeclaration();

	bool IsCreated() const;

	S32 GetElementCount() const;
	U32 GetStride(const U32 stream = 0) const;

	static const S32 m_MaxElementCount = 12;
};

//! C_VertexBuffer

struct C_VertexBuffer
{
	C_VertexBuffer();
	~C_VertexBuffer();

	bool IsCreated() const;
	
	U32 GetSize() const;
	U32 GetFlags() const;

	void SetFlags(const U32 flags);

	void* Lock(const U32 offset = 0, const U32 size = 0, const U32 flags = 0);
	void Unlock();
};

//! C_IndexBuffer

struct C_IndexBuffer
{
	C_IndexBuffer();
	~C_IndexBuffer();

	bool IsCreated() const;

	U32 GetSize() const;
	U32 GetFlags() const;
	E_IndexFormat GetFormat() const;

	void SetFlags(const U32 flags);
	
	void* Lock(const U32 offset = 0, const U32 size = 0, const U32 flags = 0);
	void Unlock();
};

//! C_ShaderParam

struct C_ShaderParam
{
	C_ShaderParam();
};

typedef C_ShaderParam C_VertexShaderParam;
typedef C_ShaderParam C_PixelShaderParam;

//! C_VertexShader

struct C_VertexShader
{
	C_VertexShader();
	~C_VertexShader();

	bool IsCreated() const;

	U32 GetSize() const;
	U32 GetFlags() const;

	void SetFlags(const U32 flags);

	const C_ShaderParam GetParamFromName(const char* name) const;
	const C_ShaderParam GetParameter(const S32 index, const char* name) const; // Compatibility for Display
};

//! C_PixelShader

struct C_PixelShader
{
	C_PixelShader();
	~C_PixelShader();

	bool IsCreated() const;

	U32 GetSize() const;
	U32 GetFlags() const;

	void SetFlags(const U32 flags);

	const C_ShaderParam GetParamFromName(const char* name) const;
	const C_ShaderParam GetParameter(const S32 index, const char* name) const; // Compatibility for Display
	const C_ShaderParam GetParameterArray(const S32 index, const char* name, const U32 count) const; // Compatibility for Display
};

//! C_LockedRect

struct C_LockedRect
{
	C_LockedRect();

	S32 GetPitch() const;
    void* GetBits() const;

	U8*			pBits;
	U32			pitch;
};

//! C_ConstLockedRect

struct C_ConstLockedRect
{
	C_ConstLockedRect();

	S32 GetPitch() const;
    const void* GetBits() const;

	const U8*	pBits;
	U32			pitch;
};

//! C_LockedBox

struct C_LockedBox
{
	C_LockedBox();

	S32 GetRowPitch() const;
	S32 GetSlicePitch() const;
    void* GetBits() const;

	U8*			pBits;
	U32			rowPitch;
	U32			slicePitch;
};

//! C_ConstLockedBox

struct C_ConstLockedBox
{
	C_ConstLockedBox();

	S32 GetRowPitch() const;
	S32 GetSlicePitch() const;
    const void* GetBits() const;

	const U8*	pBits;
	U32			rowPitch;
	U32			slicePitch;
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

	U32 GetWidth() const;
	U32 GetHeight() const;
	E_SurfaceFormat GetFormat() const;
	E_Multisample GetMultisample() const;

	C_LockedRect LockRect(const U32 flags = 0);
	void UnlockRect();
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
	
	U32 GetWidth(const U32 level = 0) const;
	U32 GetHeight(const U32 level = 0) const;
	U32 GetDepth(const U32 level = 0) const;
	U32 GetLevels() const;
	E_TextureFormat GetFormat() const;

	void SetSize(const U32 size);
	void SetFlags(const U32 flags);
	void SetSRGB(const bool enable = true);

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
	const Util::C_String GetFileName() const;
	C_LockedRect Lock(const U32 level);
	C_ConstLockedRect Lock(const U32 level) const;
	void Unlock(const U32 level) const;
};

//! C_VideoMode

struct C_VideoMode
{
};

//------------------------------------------------------------------------------
// States

//! C_AlphaTestState

struct C_AlphaTestState
{
	void SetDefaultStates();
	void SetAlphaTestCmpFunc(const U32 alphaTestCmpFunc);
	void SetAlphaTestRef(const U8 alphaTestRef);
};

//! C_BlendState

struct C_BlendState
{
	void SetDefaultStates();
	void SetBlendColourSrc(const U32 blendColourSrc);
	void SetBlendColourDst(const U32 blendColourDst);
	void SetBlendColourOp(const U32 blendColourOp);
	void SetBlendAlphaSrc(const U32 blendAlphaSrc);
	void SetBlendAlphaDst(const U32 blendAlphaDst);
	void SetBlendAlphaOp(const U32 blendAlphaOp);
};

//! C_ColourState

struct C_ColourState
{
	void SetDefaultStates();
	void SetColourWriteEnable(const U32 colourWriteEnable);
};

//! C_CullState

struct C_CullState
{
	void SetDefaultStates();
	void SetCullMode(const U32 cullMode);
};

//! C_DepthState

struct C_DepthState
{
	void SetDefaultStates();
	void SetDepthFunc(const U32 depthFunc);
	void SetDepthEnable(const U32 depthEnable);
	void SetDepthWriteEnable(const U32 depthWriteEnable);
	void SetDepthBias(const F32 depthBias);
	void SetSlopeScaledBias(const F32 slopeScaledBias);
};

//! C_FillState

struct C_FillState
{
	void SetDefaultStates();
	void SetFillMode(const U32 fillMode);
};

//! C_TextureState

struct C_TextureState
{
	void SetDefaultStates();
	void SetTexture(const C_Texture* pTexture);
};

//! C_SamplerState

struct C_SamplerState
{
	void SetDefaultStates();
	void SetTextureAddressU(const U32 textureAddressU);
	void SetTextureAddressV(const U32 textureAddressV);
	void SetTextureAddressW(const U32 textureAddressW);
	void SetTextureAddress(const U32 textureAddressU, const U32 textureAddressV);
	void SetTextureAddress(const U32 textureAddressU, const U32 textureAddressV, const U32 textureAddressW);
	void SetTextureAddress(const U32 textureAddress);
	void SetTextureFilterMag(const U32 textureFilterMag);
	void SetTextureFilterMin(const U32 textureFilterMin);
	void SetTextureFilterMip(const U32 textureFilterMip);
	void SetTextureFilter(const U32 textureFilterMag, const U32 textureFilterMin, const U32 textureFilterMip);
	void SetTextureFilter(const U32 textureFilter);
	void SetTextureBorderColour(const U32 textureBorderColour);
	void SetTextureLodBias(const S8 textureLodBias);
};

//! C_ScissorState

struct C_ScissorState
{
	void SetDefaultStates();
	void SetScissorEnable(const U32 scissorEnable);
};

//! C_StencilState

struct C_StencilState
{
	void SetDefaultStates();
	void SetStencilEnable(const U32 stencilEnable);
	void SetStencilFunc(const U32 stencilFunc);
	void SetStencilFail(const U32 stencilFail);
	void SetStencilDepthFail(const U32 stencilDepthFail);
	void SetStencilDepthPass(const U32 stencilDepthPass);
	void SetStencilRef(const U32 stencilRef);
	void SetStencilMask(const U32 stencilMask);
	void SetStencilWriteMask(const U32 stencilWriteMask);
};

//! C_VertexTextureState

typedef C_TextureState C_VertexTextureState;

//! C_VertexSamplerState

typedef C_SamplerState C_VertexSamplerState;

//! C_ContextState

struct C_ContextState
{
	void SetDefaultStates();

	C_AlphaTestState							m_AlphaTestState;
	C_BlendState								m_BlendState;
	C_ColourState								m_ColourState;
	C_CullState									m_CullState;
	C_DepthState								m_DepthState;
	C_FillState									m_FillState;
	C_ScissorState								m_ScissorState;
	C_StencilState								m_StencilState;
	C_TextureState								m_TextureState[E_Sampler_Count];
	C_SamplerState								m_SamplerState[E_Sampler_Count];
	C_VertexTextureState						m_VertexTextureState[E_VertexSampler_Count];
	C_VertexSamplerState						m_VertexSamplerState[E_VertexSampler_Count];
};

//! C_BlendMode

struct C_BlendMode
{
	void SetDefaultStates();
};

//------------------------------------------------------------------------------
// Context

#define GPU_NO_ARGS()
#define GPU_ARGS(...)					__VA_ARGS__
#define GPU_VOID						void
#define GPU_RETURN_OK					return
#define GPU_RETURN_FAIL					return
#define GPU_RETURN_VOID					return
#define GPU_RETURN(_func)				_func;

//! C_Context

#define GPU_CLASS 						C_Context
#define GPU_CHECK 						1
#define GPU_TRACK 						1

struct C_Context
{
	C_Context();

	#include "Display2/Context/WiiFunctions.h"

	C_ContextState*						m_pState;
};

#include "Display2/Context/WiiFunctions.inl"

#undef GPU_TRACK
#undef GPU_CHECK
#undef GPU_CLASS

//! C_ContextNoCheck

#define GPU_CLASS 						C_ContextNoCheck
#define GPU_TRACK 						1

struct C_ContextNoCheck
{
	C_ContextNoCheck();

	#include "Display2/Context/WiiFunctions.h"

	C_ContextState*						m_pState;
};

#include "Display2/Context/WiiFunctions.inl"

#undef GPU_TRACK
#undef GPU_CLASS

//! C_ContextNoCheckNoTrack

#define GPU_CLASS C_ContextNoCheckNoTrack

struct C_ContextNoCheckNoTrack
{
	C_ContextNoCheckNoTrack();

	#include "Display2/Context/WiiFunctions.h"
};

#include "Display2/Context/WiiFunctions.inl"

#undef GPU_CLASS

} // namespace Display2

#include "Display2/Context/WiiContext.inl"

#endif // DISPLAY2_WII_CONTEXT_H
