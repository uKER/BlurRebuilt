#ifndef DISPLAY2_GCM_CONTEXT_SPU_INL
#define DISPLAY2_GCM_CONTEXT_SPU_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display2
{

INLINE U32 FlagsToLocation(const U32 flags)
{
	return flags & E_Flags_System ? E_Location_Main : E_Location_Local;
}

INLINE bool C_CommandChain::IsCreated() const
{
	return m_Offset != UINT_MAX || m_pAddress != NULL ? true : false;
}

INLINE U32 C_CommandChain::GetSize() const
{
	return m_Size;
}

INLINE U8* C_CommandChain::GetAddress() const
{
	return m_pAddress;
}

INLINE U32 C_Viewport::GetX() const
{
	return m_X;
}

INLINE U32 C_Viewport::GetY() const
{
	return m_Y;
}

INLINE U32 C_Viewport::GetWidth() const
{
	return m_Width;
}

INLINE U32 C_Viewport::GetHeight() const
{
	return m_Height;
}

INLINE F32 C_Viewport::GetMinZ() const
{
	return m_MinZ;
}

INLINE F32 C_Viewport::GetMaxZ() const
{
	return m_MaxZ;
}

INLINE void C_Viewport::SetX(const U32 x)
{
	m_X = x;
}

INLINE void C_Viewport::SetY(const U32 y)
{
	m_Y = y;
}

INLINE void C_Viewport::SetWidth(const U32 width)
{
	m_Width = width;
}

INLINE void C_Viewport::SetHeight(const U32 height)
{
	m_Height = height;
}

INLINE void C_Viewport::SetMinZ(const F32 minZ)
{
	m_MinZ = minZ;
}

INLINE void C_Viewport::SetMaxZ(const F32 maxZ)
{
	m_MaxZ = maxZ;
}

INLINE C_Viewport& C_Viewport::operator=(const C_Viewport& other)
{
	m_X 			= other.m_X;
	m_Y 			= other.m_Y;
	m_Width			= other.m_Width;
	m_Height		= other.m_Height;
	m_MinZ			= other.m_MinZ;
	m_MaxZ			= other.m_MaxZ;

	return *this;
}

INLINE bool C_Viewport::operator==(const C_Viewport& other) const
{
	return 
		m_X 		== other.m_X		&&
		m_Y 		== other.m_Y		&&
		m_Width		== other.m_Width	&&
		m_Height	== other.m_Height	&&
		m_MinZ		== other.m_MinZ		&&
		m_MaxZ		== other.m_MaxZ;
}

INLINE bool C_Viewport::operator!=(const C_Viewport& other) const
{
	return 
		m_X 		!= other.m_X		||
		m_Y 		!= other.m_Y		||
		m_Width		!= other.m_Width	||
		m_Height	!= other.m_Height	||
		m_MinZ		!= other.m_MinZ		||
		m_MaxZ		!= other.m_MaxZ;
}

INLINE U16 C_VertexElement::GetStream() const
{
	return m_Stream;
}

INLINE U32 C_VertexElement::GetOffset() const
{
	return m_Offset;
}

INLINE U32 C_VertexElement::GetType() const
{
	return m_Type;
}

INLINE U8 C_VertexElement::GetUsage() const
{
	return m_Usage;
}

INLINE U8 C_VertexElement::GetUsageIndex() const
{
	return m_UsageIndex;
}

INLINE C_VertexElement& C_VertexElement::operator=(const C_VertexElement& other)
{
	m_Stream 				= other.m_Stream;
	m_Offset 				= other.m_Offset;
	m_Type					= other.m_Type;
	m_Usage					= other.m_Usage;
	m_UsageIndex			= other.m_UsageIndex;

	m_gcmSize 				= other.m_gcmSize;
	m_gcmType 				= other.m_gcmType;
	m_gcmStride				= other.m_gcmStride;

	return *this;
}

INLINE bool C_VertexElement::operator==(const C_VertexElement& other) const
{
	return
		m_Stream 			== other.m_Stream 		&&
		m_Offset 			== other.m_Offset 		&&
		m_Type				== other.m_Type			&&
		m_Usage				== other.m_Usage		&&
		m_UsageIndex		== other.m_UsageIndex;
}

INLINE bool C_VertexElement::operator!=(const C_VertexElement& other) const
{
	return
		m_Stream 			!= other.m_Stream 		||
		m_Offset 			!= other.m_Offset 		||
		m_Type				!= other.m_Type			||
		m_Usage				!= other.m_Usage		||
		m_UsageIndex		!= other.m_UsageIndex;
}

INLINE bool C_VertexDeclaration::IsCreated() const
{
	return m_ElementCount > 0 ? true : false;
}

INLINE S32 C_VertexDeclaration::GetElementCount() const
{
	return m_ElementCount;
}

INLINE const C_VertexElement* C_VertexDeclaration::GetElements() const
{
	return m_ElementArray;
}

INLINE bool C_VertexBuffer::IsCreated() const
{
	return m_Size > 0 ? true : false;
}

INLINE U32 C_VertexBuffer::GetSize() const
{
	return m_Size;
}

INLINE U32 C_VertexBuffer::GetFlags() const
{
	return m_Flags;
}

INLINE U32 C_VertexBuffer::GetOffset() const
{
	return m_Offset;
}

INLINE void* C_VertexBuffer::GetAddress() const
{
	return IsCreated() ? OffsetToAddress(FlagsToLocation(m_Flags), m_Offset) : NULL;
}

INLINE bool C_IndexBuffer::IsCreated() const
{
	return m_Size > 0 ? true : false;
}

INLINE U32 C_IndexBuffer::GetSize() const
{
	return m_Size;
}

INLINE U32 C_IndexBuffer::GetFlags() const
{
	return m_Flags;
}

INLINE U32 C_IndexBuffer::GetOffset() const
{
	return m_Offset;
}

INLINE E_IndexFormat C_IndexBuffer::GetFormat() const
{
	return m_Format;
}

INLINE void* C_IndexBuffer::GetAddress() const
{
	return IsCreated() ? OffsetToAddress(FlagsToLocation(m_Flags), m_Offset) : NULL;
}

INLINE bool C_VertexShader::IsCreated() const
{
	return m_Size > 0 ? true : false;
}

INLINE U32 C_VertexShader::GetSize() const
{
	return m_Size;
}

INLINE U32 C_VertexShader::GetFlags() const
{
	return m_Flags;
}

INLINE U32 C_VertexShader::GetOffset() const
{
	return m_CommandChain.m_Offset;
}

INLINE U8* C_VertexShader::GetAddress() const
{
	return NULL;
}

INLINE U8* C_VertexShader::GetAddressInMainMemory() const
{
	return m_pAddressInMainMemory;
}

INLINE U8 C_VertexShader::GetRegisterCount() const
{
	return m_ProgramConfig.registerCount;
}

INLINE void C_VertexShader::SetRegisterCount(const U8 registerCount)
{
	m_ProgramConfig.registerCount = registerCount;
}	

INLINE C_CommandChain* C_VertexShader::GetCommandChain()
{
	return &m_CommandChain;
}

#if !PLATFORM_SPU
INLINE CellCgbProgram *C_VertexShader::GetCgbProgram()
{
	return &m_Program;
}

INLINE const CellCgbProgram *C_VertexShader::GetCgbProgram() const
{
	return &m_Program;
}
#endif

INLINE bool C_PixelShader::IsCreated() const
{
	return m_Size > 0 ? true : false;
}

INLINE U32 C_PixelShader::GetSize() const
{
	return m_Size;
}

INLINE U32 C_PixelShader::GetFlags() const
{
	return m_Flags;
}

INLINE U32 C_PixelShader::GetOffset() const
{
	return m_ProgramConfig.offset;
}

INLINE U8* C_PixelShader::GetAddress() const
{
	return m_pAddress;
}

INLINE U8* C_PixelShader::GetAddressInMainMemory() const
{
	return m_pAddressInMainMemory;
}

INLINE U8 C_PixelShader::GetRegisterCount() const
{
	return m_ProgramConfig.registerCount;
}

INLINE void C_PixelShader::SetRegisterCount(const U8 registerCount)
{
	m_ProgramConfig.registerCount = registerCount;
}

INLINE S32 C_LockedRect::GetPitch() const
{
	return pitch;
}

INLINE U8* C_LockedRect::GetBits() const
{
	return pBits;
}

INLINE C_LockedRect& C_LockedRect::operator=(const C_LockedRect& other)
{
	pBits = other.pBits;
	pitch = other.pitch;
	return *this;
}

INLINE S32 C_ConstLockedRect::GetPitch() const
{
	return pitch;
}

INLINE const U8* C_ConstLockedRect::GetBits() const
{
	return pBits;
}

INLINE C_ConstLockedRect& C_ConstLockedRect::operator=(const C_ConstLockedRect& other)
{
	pBits = other.pBits;
	pitch = other.pitch;
	return *this;
}

INLINE S32 C_LockedBox::GetRowPitch() const
{
	return rowPitch;
}

INLINE S32 C_LockedBox::GetSlicePitch() const
{
	return slicePitch;
}

INLINE U8* C_LockedBox::GetBits() const
{
	return pBits;
}

INLINE C_LockedBox& C_LockedBox::operator=(const C_LockedBox& other)
{
	pBits		= other.pBits;
	rowPitch	= other.rowPitch;
	slicePitch	= other.slicePitch;
	return *this;
}

INLINE S32 C_ConstLockedBox::GetRowPitch() const
{
	return rowPitch;
}

INLINE S32 C_ConstLockedBox::GetSlicePitch() const
{
	return slicePitch;
}

INLINE const U8* C_ConstLockedBox::GetBits() const
{
	return pBits;
}

INLINE C_ConstLockedBox& C_ConstLockedBox::operator=(const C_ConstLockedBox& other)
{
	pBits		= other.pBits;
	rowPitch	= other.rowPitch;
	slicePitch	= other.slicePitch;
	return *this;
}

INLINE bool C_Surface::IsCreated() const
{
	return m_Size > 0 ? true : false;
}

INLINE U32 C_Surface::GetSize() const
{
	return m_Size;
}

INLINE U32 C_Surface::GetFlags() const
{
	return m_Flags;
}

INLINE U32 C_Surface::GetOffset() const
{
	return m_Offset;
}

INLINE U8* C_Surface::GetAddress() const
{
	if(!IsCreated())
		return NULL;

	return (U8*)OffsetToAddress(FlagsToLocation(m_Flags), m_Offset);
}

INLINE U32 C_Surface::GetWidth() const
{
	if(!IsCreated())
		return 0;

	return m_Width;
}

INLINE U32 C_Surface::GetHeight() const
{
	if(!IsCreated())
		return 0;

	return m_Height;
}

INLINE E_SurfaceFormat C_Surface::GetFormat() const
{
	if(!IsCreated())
		return E_SurfaceFormat_Unknown;

	return m_Format;
}

INLINE U32 C_Surface::GetMultisample() const
{
	if(!IsCreated())
		return E_Multisample_Unknown;

	return m_Multisample;
}

INLINE U32 C_Surface::GetPitch() const
{
	if(!IsCreated())
		return 0;

	return m_Pitch;
}

inline U32 GetWidth2D(const C_Texture& texture)
{
	return texture.GetWidth();
}

inline U32 GetHeight2D(const C_Texture& texture)
{
	return texture.GetHeight();
}

inline U32 GetLevels(const C_Texture& texture)
{
	return texture.GetLevels();
}

INLINE bool C_Texture::IsCreated() const
{
	return m_Size > 0 ? true : false;
}

INLINE U32 C_Texture::GetSize() const
{
	return m_Size;
}

INLINE U32 C_Texture::GetFlags() const
{
	return m_Flags;
}

INLINE U32 C_Texture::GetOffset() const
{
	return m_gcmTexture.offset;
}

INLINE U8* C_Texture::GetAddress() const
{
	if(!IsCreated())
		return NULL;

	return (U8*)OffsetToAddress(m_gcmTexture.location, m_gcmTexture.offset);
}

INLINE U32 C_Texture::GetWidth(const U32 level) const
{
	if(!IsCreated())
		return 0;
	
	BIZ_ASSERT_MSG(level < m_gcmTexture.mipmap, "level(%d) < m_gcmTexture.mipmap(%d)", level, m_gcmTexture.mipmap);
	U32 width = m_gcmTexture.width;
	for(U32 i=0; i<level; i++)
		if(width > 1)
			width /= 2;
	return width;
}

INLINE U32 C_Texture::GetHeight(const U32 level) const
{
	if(!IsCreated())
		return 0;
	
	BIZ_ASSERT_MSG(level < m_gcmTexture.mipmap, "level(%d) < m_gcmTexture.mipmap(%d)", level, m_gcmTexture.mipmap);
	U32 height = m_gcmTexture.height;
	for(U32 i=0; i<level; i++)
		if(height > 1)
			height /= 2;
	return height;
}

INLINE U32 C_Texture::GetDepth(const U32 level) const
{
	if(!IsCreated())
		return 0;
	
	BIZ_ASSERT_MSG(level < m_gcmTexture.mipmap, "level(%d) < m_gcmTexture.mipmap(%d)", level, m_gcmTexture.mipmap);
	U32 depth = m_gcmTexture.depth;
	for(U32 i=0; i<level; i++)
		if(depth > 1)
			depth /= 2;
	return depth;
}

INLINE U32 C_Texture::GetLevels() const
{
	if(!IsCreated())
		return 0;

	return m_gcmTexture.mipmap;
}

INLINE U32 C_Texture::GetPitch() const
{
	if(!IsCreated())
		return 0;

	return m_gcmTexture.pitch;
}

INLINE E_TextureFormat C_Texture::GetFormat() const
{
	if(!IsCreated())
		return E_TextureFormat_Unknown;

	U32 format = m_gcmTexture.format;

	format &= ~CELL_GCM_TEXTURE_LN;
	format &= ~CELL_GCM_TEXTURE_SZ;
	format &= ~CELL_GCM_TEXTURE_UN;

	return (E_TextureFormat)format;
}

INLINE E_TextureType C_Texture::GetType() const
{
	if(!IsCreated())
		return E_TextureType_Unknown;

	switch(m_gcmTexture.dimension)
	{
	case CELL_GCM_TEXTURE_DIMENSION_1:
		return E_TextureType_1D;
	case CELL_GCM_TEXTURE_DIMENSION_2:
		return m_gcmTexture.cubemap ? E_TextureType_Cube : E_TextureType_2D;
	case CELL_GCM_TEXTURE_DIMENSION_3:
		return E_TextureType_3D;
	default:
		BIZ_ASSERT(0 && "Unsupported texture type");
		return E_TextureType_Unknown;
	}
}

INLINE GPUTexture* C_Texture::GetGPUTexture()
{
	return IsCreated() ? &m_gcmTexture : NULL;
}

INLINE const GPUTexture* C_Texture::GetGPUTexture() const
{
	return IsCreated() ? &m_gcmTexture : NULL;
}

INLINE bool C_RenderTexture2D::IsCreated() const
{
	return m_Texture[0].IsCreated() && m_Surfaces[0].Size() > 0 && m_Surfaces[0][0].IsCreated();
}

INLINE U32 C_RenderTexture2D::GetFlags() const
{
	return m_Flags;
}

INLINE C_Texture& C_RenderTexture2D::GetTexture()
{
	BIZ_ASSERT(m_TextureIndex < 2);
	return m_Texture[m_TextureIndex];
}

INLINE const C_Texture& C_RenderTexture2D::GetTexture() const
{
	BIZ_ASSERT(m_TextureIndex < 2);
	return m_Texture[m_TextureIndex];
}

INLINE C_Surface& C_RenderTexture2D::GetSurface(const U32 level)
{
	BIZ_ASSERT(m_SurfaceIndex < 2);
	BIZ_ASSERT(level < (U32)m_Surfaces[m_SurfaceIndex].Size());
	return m_Surfaces[m_SurfaceIndex][level];
}

INLINE const C_Surface& C_RenderTexture2D::GetSurface(const U32 level) const
{
	BIZ_ASSERT(m_SurfaceIndex < 2);
	BIZ_ASSERT(level < (U32)m_Surfaces[m_SurfaceIndex].Size());
	return m_Surfaces[m_SurfaceIndex][level];
}

INLINE bool C_RenderTextureCube::IsCreated() const
{
	return m_Texture.IsCreated();
}

INLINE U32 C_RenderTextureCube::GetFlags() const
{
	return m_Flags;
}

INLINE C_Texture& C_RenderTextureCube::GetTexture()
{
	return m_Texture;
}

INLINE const C_Texture& C_RenderTextureCube::GetTexture() const
{
	return m_Texture;
}

INLINE C_Surface& C_RenderTextureCube::GetSurface(const U32 face, const U32 level)
{
	BIZ_ASSERT(face < 6);
	BIZ_ASSERT(level < (U32)m_Surfaces[face].Size());
	return m_Surfaces[face][level];
}

INLINE const C_Surface& C_RenderTextureCube::GetSurface(const U32 face, const U32 level) const
{
	BIZ_ASSERT(face < 6);
	BIZ_ASSERT(level < (U32)m_Surfaces[face].Size());
	return m_Surfaces[face][level];
}

INLINE void C_AlphaTestState::SetDefaultStates()
{
	m_AlphaTestCmpFunc	= E_CmpFunc_Always;
	m_AlphaTestRef		= 0;
}

INLINE void C_AlphaTestState::SetAlphaTestCmpFunc(const E_CmpFunc alphaTestCmpFunc)
{
	GPU_ASSERT_CMP_FUNC(alphaTestCmpFunc);

	m_AlphaTestCmpFunc = alphaTestCmpFunc;
}

INLINE void C_AlphaTestState::SetAlphaTestRef(const U16 alphaTestRef)
{
	m_AlphaTestRef = alphaTestRef;
}

INLINE void C_BlendState::SetDefaultStates()
{
	m_BlendColourSrc	= E_Blend_One;
	m_BlendColourDst	= E_Blend_Zero;
	m_BlendColourOp		= E_BlendOp_Add;
	m_BlendAlphaSrc		= E_Blend_One;
	m_BlendAlphaDst		= E_Blend_One;
	m_BlendAlphaOp		= E_BlendOp_Add;
}

INLINE void C_BlendState::SetBlendColourSrc(const E_Blend blendColourSrc)
{
	GPU_ASSERT_BLEND(blendColourSrc);

	m_BlendColourSrc = blendColourSrc;
}

INLINE void C_BlendState::SetBlendColourDst(const E_Blend blendColourDst)
{
	GPU_ASSERT_BLEND(blendColourDst);

	m_BlendColourDst = blendColourDst;
}

INLINE void C_BlendState::SetBlendColourOp(const E_BlendOp blendColourOp)
{
	GPU_ASSERT_BLEND_OP(blendColourOp);

	m_BlendColourOp = blendColourOp;
}

INLINE void C_BlendState::SetBlendAlphaSrc(const E_Blend blendAlphaSrc)
{
	GPU_ASSERT_BLEND(blendAlphaSrc);

	m_BlendAlphaSrc = blendAlphaSrc;
}

INLINE void C_BlendState::SetBlendAlphaDst(const E_Blend blendAlphaDst)
{
	GPU_ASSERT_BLEND(blendAlphaDst);

	m_BlendAlphaDst = blendAlphaDst;
}

INLINE void C_BlendState::SetBlendAlphaOp(const E_BlendOp blendAlphaOp)
{
	GPU_ASSERT_BLEND_OP(blendAlphaOp);

	m_BlendAlphaOp = blendAlphaOp;
}

INLINE void C_ColourState::SetDefaultStates()
{
	m_ColourWriteEnable = E_ColourWriteEnable_All;
}
	
INLINE void C_ColourState::SetColourWriteEnable(const U32 colourWriteEnable)
{
	m_ColourWriteEnable = colourWriteEnable;
}

INLINE void C_CullState::SetDefaultStates()
{
	m_CullEnable		= E_Bool_True;
	m_CullMode			= E_CullMode_Ccw;
}

INLINE void C_CullState::SetCullMode(const E_CullMode cullMode)
{
	switch(cullMode)
	{
	case E_CullMode_None:
		m_CullEnable	= E_Bool_False;
		m_CullMode		= E_CullMode_None;
		break;
	case E_CullMode_Cw:
		m_CullEnable	= E_Bool_True;
		m_CullMode		= E_CullMode_Cw;
		break;
	case E_CullMode_Ccw:
		m_CullEnable	= E_Bool_True;
		m_CullMode		= E_CullMode_Ccw;
		break;
	default:
		BIZ_ASSERT(0 && "Unsupported cull mode");
		break;
	}
}

INLINE void C_DepthState::SetDefaultStates()
{
	m_DepthFunc			= E_CmpFunc_LessEqual;
	m_DepthEnable		= E_Bool_True;
	m_DepthWriteEnable	= E_Bool_True;
	m_DepthBias			= 0.0f;
	m_SlopeScaledBias	= 0.0f;
}

INLINE void C_DepthState::SetDepthFunc(const E_CmpFunc depthFunc)
{
	GPU_ASSERT_CMP_FUNC(depthFunc);

	m_DepthFunc = depthFunc;
}

INLINE void C_DepthState::SetDepthEnable(const E_Bool depthEnable)
{
	GPU_ASSERT_BOOL(depthEnable);

	m_DepthEnable = depthEnable;
}

INLINE void C_DepthState::SetDepthWriteEnable(const E_Bool depthWriteEnable)
{
	GPU_ASSERT_BOOL(depthWriteEnable);

	m_DepthWriteEnable = depthWriteEnable;
}

INLINE void C_DepthState::SetDepthBias(const F32 depthBias)
{
	m_DepthBias = depthBias;
}

INLINE void C_DepthState::SetSlopeScaledBias(const F32 slopeScaledBias)
{
	m_SlopeScaledBias = slopeScaledBias;
}

INLINE void C_FillState::SetDefaultStates()
{
	m_FillMode = E_FillMode_Solid;
}

INLINE void C_FillState::SetFillMode(const E_FillMode fillMode)
{
	GPU_ASSERT_FILL_MODE(fillMode);

	m_FillMode = fillMode;
}

INLINE void C_TextureState::SetDefaultStates()
{
	// cellGcmSetTexture
	m_pTexture						= NULL;

	// cellGcmSetTextureControl
	m_TextureControlEnable			= E_Bool_False;
	m_TextureControlMinLod			= 0;	
	m_TextureControlMaxLod			= 12;	
	m_TextureControlMaxAniso		= E_TextureMaxAniso_1;

	// cellGcmSetTextureRemap
	m_TextureRemap					= 0;

}

INLINE void C_TextureState::SetTexture(const C_Texture* pTexture)
{
	if(pTexture)
	{
		GPU_ASSERT_TEXTURE(&pTexture->m_gcmTexture);

		m_TextureControlEnable		= E_Bool_True;
		m_TextureControlMinLod		= 0;
		m_TextureControlMaxLod		= pTexture->m_gcmTexture.mipmap-1;
		m_TextureRemap				= pTexture->m_gcmTexture.remap;
	}
	else
	{
		m_TextureControlEnable		= E_Bool_False;
		m_TextureControlMinLod		= 0;
		m_TextureControlMaxLod		= 0;
		m_TextureRemap				= 0;
	}
	m_pTexture = pTexture;
}

INLINE void C_TextureState::SetTextureMaxAniso(const E_TextureMaxAniso textureMaxAniso)
{
	GPU_ASSERT_TEXTURE_MAX_ANISO(textureMaxAniso);

	m_TextureControlMaxAniso = textureMaxAniso;
}

INLINE void C_SamplerState::SetDefaultStates()
{
	m_SetTextureOptimisationBits	= 0;
	m_SetTextureFilterBits			= 0;
	m_SetTextureAddressBits			= 0;

	// cellGcmSetTextureAddress
	m_TextureAddressU				= E_TextureAddress_Wrap;
	m_TextureAddressV				= E_TextureAddress_Wrap;
	m_TextureAddressW				= E_TextureAddress_ClampToEdge;
	m_TextureUnsignedRemap			= E_TextureUnsignedRemap_Normal;
	m_TextureZFunc					= E_TextureZFunc_Never;
	m_TextureGamma					= 0;

	// cellGcmSetTextureBorderColor
	m_TextureBorderColour			= 0x00000000;

	// cellGcmSetTextureFilter
	m_TextureFilterBias				= 0;		
	m_TextureFilterMin				= E_TextureFilterMin_NearestLinear;
	m_TextureFilterMag				= E_TextureFilterMag_Linear;
	m_TextureFilterConv				= E_TextureFilterConv_Quincunx;

	// cellGcmSetTextureOptimization
	m_TextureOptimisationSlope		= 0x8;
	m_TextureOptimisationIso		= E_TextureOptimisationIso_High;
	m_TextureOptimisationAniso		= E_TextureOptimisationAniso_High;
}

INLINE bool C_SamplerState::IsSame(const C_SamplerState &other)
{
	return	m_SetTextureOptimisationBits == other.m_SetTextureOptimisationBits &&
			m_SetTextureFilterBits == other.m_SetTextureFilterBits &&
			m_SetTextureAddressBits == other.m_SetTextureAddressBits;
}

INLINE void C_SamplerState::SetTextureAddressU(const E_TextureAddress textureAddressU)
{
	GPU_ASSERT_TEXTURE_ADDRESS(textureAddressU);

	m_TextureAddressU = textureAddressU;
}

INLINE void C_SamplerState::SetTextureAddressV(const E_TextureAddress textureAddressV)
{
	GPU_ASSERT_TEXTURE_ADDRESS(textureAddressV);

	m_TextureAddressV = textureAddressV;
}

INLINE void C_SamplerState::SetTextureAddressW(const E_TextureAddress textureAddressW)
{
	GPU_ASSERT_TEXTURE_ADDRESS(textureAddressW);

	m_TextureAddressW = textureAddressW;
}

INLINE void C_SamplerState::SetTextureAddress(const E_TextureAddress textureAddressU, const E_TextureAddress textureAddressV, const E_TextureAddress textureAddressW)
{
	GPU_ASSERT_TEXTURE_ADDRESS(textureAddressU);
	GPU_ASSERT_TEXTURE_ADDRESS(textureAddressV);
	GPU_ASSERT_TEXTURE_ADDRESS(textureAddressW);
	
	m_TextureAddressU = textureAddressU;
	m_TextureAddressV = textureAddressV;
	m_TextureAddressW = textureAddressW;
}

INLINE void C_SamplerState::SetTextureAddress(const E_TextureAddress textureAddress)
{
	GPU_ASSERT_TEXTURE_ADDRESS(textureAddress);
	
	m_TextureAddressU = textureAddress;
	m_TextureAddressV = textureAddress;
	m_TextureAddressW = textureAddress;
}

INLINE void C_SamplerState::SetTextureFilter(const E_TextureFilter textureFilterMag, const E_TextureFilter textureFilterMin, const E_TextureFilter textureFilterMip)
{
	GPU_ASSERT_TEXTURE_FILTER(textureFilterMag);
	GPU_ASSERT_TEXTURE_FILTER(textureFilterMin);
	GPU_ASSERT_TEXTURE_FILTER(textureFilterMip);

	switch(textureFilterMag)
	{
	case E_TextureFilter_Point:
		m_TextureFilterMag = E_TextureFilterMag_Nearest;
		m_TextureFilterMin = textureFilterMin != E_TextureFilter_None ? E_TextureFilterMin_NearestNearest : E_TextureFilterMin_Nearest;
		break;
	case E_TextureFilter_Linear:
	case E_TextureFilter_Anisotropic:
		m_TextureFilterMag = E_TextureFilterMag_Linear;
		m_TextureFilterMin = textureFilterMin != E_TextureFilter_None ? E_TextureFilterMin_LinearLinear : E_TextureFilterMin_Linear;
		break;
	default:
		m_TextureFilterMag = E_TextureFilterMag_Nearest;
		m_TextureFilterMin = E_TextureFilterMin_Nearest;
		break;
	}
}

INLINE void C_SamplerState::SetTextureFilter(const E_TextureFilter textureFilter)
{
	GPU_ASSERT_TEXTURE_FILTER(textureFilter);

	SetTextureFilter(textureFilter, textureFilter, textureFilter);
}

INLINE void C_SamplerState::SetTextureBorderColour(const U32 textureBorderColour)
{
	m_TextureBorderColour = textureBorderColour;
}

INLINE void C_SamplerState::SetTextureGamma(const U32 textureGamma)
{
	GPU_ASSERT(textureGamma <= (E_TextureGamma_R|E_TextureGamma_G|E_TextureGamma_B|E_TextureGamma_A));

	m_TextureGamma = textureGamma;
}

INLINE void C_SamplerState::SetTextureLodBias(const S8 textureLodBias)
{
	m_TextureFilterBias = textureLodBias;

	//BIZ_ASSERT((S32)m_TextureFilterBias >= CHAR_MIN && (S32)m_TextureFilterBias <= CHAR_MAX);
}

INLINE void C_ScissorState::SetDefaultStates()
{
	m_ScissorRectX = 0;
	m_ScissorRectY = 0;
	m_ScissorRectW = 4095;
	m_ScissorRectH = 4095;
}

INLINE void C_ScissorState::SetScissorEnable(const E_Bool scissorEnable)
{
	m_ScissorRectX = 0;
	m_ScissorRectY = 0;
	
	if(scissorEnable)
	{
		m_ScissorRectW = 0;
		m_ScissorRectH = 0;
	}
	else
	{
		m_ScissorRectW = 4095;
		m_ScissorRectH = 4095;
	}
}

INLINE void C_ScissorState::SetScissorRect(const U16 x, const U16 y, const U16 w, const U16 h)
{
	m_ScissorRectX = x;
	m_ScissorRectY = y;
	m_ScissorRectW = w;
	m_ScissorRectH = h;
}

INLINE void C_StencilState::SetDefaultStates()
{
	m_StencilFunc		= E_CmpFunc_Always;
	m_StencilFail		= E_StencilOp_Keep;
	m_StencilDepthFail	= E_StencilOp_Keep;
	m_StencilDepthPass	= E_StencilOp_Keep;
	m_StencilRef		= 0;
	m_StencilMask		= 0xFFFFFFFF;
	m_StencilWriteMask	= 0xFFFFFFFF;
}

INLINE void C_StencilState::SetStencilFunc(const E_CmpFunc stencilFunc)
{
	GPU_ASSERT_CMP_FUNC(stencilFunc);

	m_StencilFunc = stencilFunc;
}

INLINE void C_StencilState::SetStencilFail(const E_StencilOp stencilFail)
{
	GPU_ASSERT_STENCIL_OP(stencilFail);

	m_StencilFail = stencilFail;
}

INLINE void C_StencilState::SetStencilDepthFail(const E_StencilOp stencilDepthFail)
{
	GPU_ASSERT_STENCIL_OP(stencilDepthFail);

	m_StencilDepthFail = stencilDepthFail;
}

INLINE void C_StencilState::SetStencilDepthPass(const E_StencilOp stencilDepthPass)
{
	GPU_ASSERT_STENCIL_OP(stencilDepthPass);

	m_StencilDepthPass = stencilDepthPass;
}

INLINE void C_StencilState::SetStencilRef(const U32 stencilRef)
{
	m_StencilRef = stencilRef;
}

INLINE void C_StencilState::SetStencilMask(const U32 stencilMask)
{
	m_StencilMask = stencilMask;
}

INLINE void C_StencilState::SetStencilWriteMask(const U32 stencilWriteMask)
{
	m_StencilWriteMask = stencilWriteMask;
}

INLINE void C_VertexTextureState::SetDefaultStates()
{
	// cellGcmSetVertexTexture
	m_pTexture						= NULL;

	// cellGcmSetVertexTextureControl
	m_TextureControlEnable			= E_Bool_False;
	m_TextureControlMinLod			= 0;	
	m_TextureControlMaxLod			= 12;	
}

INLINE void C_VertexTextureState::SetTexture(const C_Texture* pTexture)
{
	if(pTexture)
	{
		GPU_ASSERT_TEXTURE(&pTexture->m_gcmTexture);

		m_TextureControlEnable		= E_Bool_True;
		m_TextureControlMinLod		= 0;
		m_TextureControlMaxLod		= pTexture->m_gcmTexture.mipmap-1;
	}
	else
	{
		m_TextureControlEnable		= E_Bool_False;
		m_TextureControlMinLod		= 0;
		m_TextureControlMaxLod		= 0;
	}
	m_pTexture = pTexture;
}

INLINE void C_VertexSamplerState::SetDefaultStates()
{
	// cellGcmSetVertexTextureAddress
	m_TextureAddressU				= E_TextureAddress_Wrap;
	m_TextureAddressV				= E_TextureAddress_Wrap;

	// cellGcmSetVertexTextureBorderColor
	m_TextureBorderColour			= 0x00000000;

	// cellGcmSetTextureVertexFilter
	m_TextureFilterBias				= 0;		
}

INLINE void C_VertexSamplerState::SetTextureAddressU(const E_TextureAddress textureAddressU)
{
	GPU_ASSERT_TEXTURE_ADDRESS(textureAddressU);

	m_TextureAddressU = textureAddressU;
}

INLINE void C_VertexSamplerState::SetTextureAddressV(const E_TextureAddress textureAddressV)
{
	GPU_ASSERT_TEXTURE_ADDRESS(textureAddressV);

	m_TextureAddressV = textureAddressV;
}

INLINE void C_VertexSamplerState::SetTextureAddress(const E_TextureAddress textureAddressU, const E_TextureAddress textureAddressV)
{
	GPU_ASSERT_TEXTURE_ADDRESS(textureAddressU);
	GPU_ASSERT_TEXTURE_ADDRESS(textureAddressV);

	m_TextureAddressU = textureAddressU;
	m_TextureAddressV = textureAddressV;
}

INLINE void C_VertexSamplerState::SetTextureAddress(const E_TextureAddress textureAddress)
{
	GPU_ASSERT_TEXTURE_ADDRESS(textureAddress);

	m_TextureAddressU = textureAddress;
	m_TextureAddressV = textureAddress;
}

INLINE void C_VertexSamplerState::SetTextureBorderColour(const U32 textureBorderColour)
{
	m_TextureBorderColour = textureBorderColour;
}

INLINE void C_VertexSamplerState::SetTextureLodBias(const S8 textureLodBias)
{
	m_TextureFilterBias = textureLodBias;
}

INLINE void C_ContextState::SetDefaultStates()
{
	m_AlphaTestEnable						= E_Bool_False;
	m_BlendEnable							= E_Bool_False;
	m_StencilEnable							= E_Bool_False;
	m_PointSpriteEnable						= E_Bool_False;
	
	m_AlphaTestState.SetDefaultStates();
	m_BlendState.SetDefaultStates();
	m_ColourState.SetDefaultStates();
	m_CullState.SetDefaultStates();
	m_DepthState.SetDefaultStates();
	m_FillState.SetDefaultStates();
	m_ScissorState.SetDefaultStates();
	m_StencilState.SetDefaultStates();

	m_pVertexDeclaration					= NULL;
	m_pIndexBuffer							= NULL;

	for(U32 i=0; i<E_Stream_Count; i++)
	{
		m_pVertexBuffer[i]					= NULL;
		m_VertexOffset[i]					= 0;
		m_VertexStride[i]					= 0;
	}

	m_pVertexShader							= NULL;
	m_pPixelShader							= NULL;

	for(U32 i=0; i<E_Sampler_Count; i++)
		m_TextureState[i].SetDefaultStates();
	for(U32 i=0; i<E_Sampler_Count; i++)
		m_SamplerState[i].SetDefaultStates();
	
	for(U32 i=0; i<E_VertexSampler_Count; i++)
		m_VertexTextureState[i].SetDefaultStates();
	for(U32 i=0; i<E_VertexSampler_Count; i++)
		m_VertexSamplerState[i].SetDefaultStates();
}

INLINE C_ContextData* C_ContextBase::GetData()
{
#if PLATFORM_SPU
	BIZ_ASSERT(m_pData);
	return m_pData;
#else
	return m_pData ? m_pData : gCellGcmCurrentContext;
#endif
}

INLINE const C_ContextData* C_ContextBase::GetData() const
{
#if PLATFORM_SPU
	BIZ_ASSERT(m_pData);
	return m_pData;
#else
	return m_pData ? m_pData : gCellGcmCurrentContext;
#endif
}

} // namespace Display2

#endif // DISPLAY2_GCM_CONTEXT_SPU_INL
