#ifndef DISPLAY2_WII_CONTEXT_INL
#define DISPLAY2_WII_CONTEXT_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display2
{
	
// C_Viewport

INLINE U32 C_Viewport::GetX() const
{
	return 0;
}

INLINE U32 C_Viewport::GetY() const
{
	return 0;
}

INLINE U32 C_Viewport::GetWidth() const
{
	return 0;
}

INLINE U32 C_Viewport::GetHeight() const
{
	return 0;
}

INLINE F32 C_Viewport::GetMinZ() const
{
	return 0.0f;
}

INLINE F32 C_Viewport::GetMaxZ() const
{
	return 0.0f;
}

INLINE void C_Viewport::SetX(const U32 x)
{
}

INLINE void C_Viewport::SetY(const U32 y)
{
}

INLINE void C_Viewport::SetWidth(const U32 width)
{
}

INLINE void C_Viewport::SetHeight(const U32 height)
{
}

INLINE void C_Viewport::SetMinZ(const F32 minZ)
{
}

INLINE void C_Viewport::SetMaxZ(const F32 maxZ)
{
}

// C_VertexElement

INLINE U16 C_VertexElement::GetStream() const
{
	return 0;
}

INLINE U32 C_VertexElement::GetOffset() const
{
	return 0;
}

INLINE U32 C_VertexElement::GetType() const
{
	return 0;
}

INLINE U8 C_VertexElement::GetUsage() const
{
	return 0;
}

INLINE U8 C_VertexElement::GetUsageIndex() const
{
	return 0;
}

INLINE U32 C_VertexElement::GetSize() const
{
	return 0;
}

INLINE U32 C_VertexElement::GetCount() const
{
	return 0;
}

INLINE C_VertexElement& C_VertexElement::operator=(const C_VertexElement& other)
{
	return *this;
}

INLINE bool C_VertexElement::operator==(const C_VertexElement& other) const
{
	return false;
}
	
INLINE bool C_VertexElement::operator!=(const C_VertexElement& other) const
{
	return false;
}

// C_VertexDeclaration

INLINE bool C_VertexDeclaration::IsCreated() const
{
	return false;
}

// C_VertexBuffer

INLINE bool C_VertexBuffer::IsCreated() const
{
	return false;
}

INLINE U32 C_VertexBuffer::GetSize() const
{
	return 0;
}

INLINE U32 C_VertexBuffer::GetFlags() const
{
	return 0;
}

// C_IndexBuffer

INLINE bool C_IndexBuffer::IsCreated() const
{
	return false;
}

INLINE U32 C_IndexBuffer::GetSize() const
{
	return 0;
}

INLINE U32 C_IndexBuffer::GetFlags() const
{
	return 0;
}

INLINE E_IndexFormat C_IndexBuffer::GetFormat() const
{
	return E_IndexFormat_Unknown;
}

// C_VertexShader

INLINE bool C_VertexShader::IsCreated() const
{
	return false;
}

INLINE U32 C_VertexShader::GetSize() const
{
	return 0;
}

INLINE U32 C_VertexShader::GetFlags() const
{
	return 0;
}

// C_PixelShader

INLINE bool C_PixelShader::IsCreated() const
{
	return false;
}

INLINE U32 C_PixelShader::GetSize() const
{
	return 0;
}

INLINE U32 C_PixelShader::GetFlags() const
{
	return 0;
}

// C_LockedRect

INLINE S32 C_LockedRect::GetPitch() const
{
	return pitch;
}

INLINE void* C_LockedRect::GetBits() const
{
	return pBits;
}

// C_LockedRect

INLINE S32 C_ConstLockedRect::GetPitch() const
{
	return pitch;
}

INLINE const void* C_ConstLockedRect::GetBits() const
{
	return pBits;
}

// C_LockedBox

INLINE S32 C_LockedBox::GetRowPitch() const
{
	return rowPitch;
}

INLINE S32 C_LockedBox::GetSlicePitch() const
{
	return slicePitch;
}

INLINE void* C_LockedBox::GetBits() const
{
	return pBits;
}

// C_ConstLockedBox

INLINE S32 C_ConstLockedBox::GetRowPitch() const
{
	return rowPitch;
}

INLINE S32 C_ConstLockedBox::GetSlicePitch() const
{
	return slicePitch;
}

INLINE const void* C_ConstLockedBox::GetBits() const
{
	return pBits;
}

// C_Surface

INLINE bool C_Surface::IsCreated() const
{
	return false;
}

INLINE U32 C_Surface::GetSize() const
{
	return 0;
}

INLINE U32 C_Surface::GetFlags() const
{
	return 0;
}

INLINE U32 C_Surface::GetWidth() const
{
	return 0;
}
	
INLINE U32 C_Surface::GetHeight() const
{
	return 0;
}

INLINE E_SurfaceFormat C_Surface::GetFormat() const
{
	return E_SurfaceFormat_Unknown;
}

INLINE E_Multisample C_Surface::GetMultisample() const
{
	return E_Multisample_Unknown;
}

// C_Texture

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
	return false;
}

INLINE E_TextureType C_Texture::GetType() const
{
	return E_TextureType_Unknown;
}

INLINE U32 C_Texture::GetSize() const
{
	return 0;
}

INLINE U32 C_Texture::GetFlags() const
{
	return 0;
}

INLINE U32 C_Texture::GetWidth(const U32 level) const
{
	return 0;
}

INLINE U32 C_Texture::GetHeight(const U32 level) const
{
	return 0;
}

INLINE U32 C_Texture::GetDepth(const U32 level) const
{
	return 0;
}

INLINE E_TextureFormat C_Texture::GetFormat() const
{
	return E_TextureFormat_Unknown;
}

INLINE U32 C_Texture::GetLevels() const
{
	return 0;
}

// C_AlphaTestState

INLINE void C_AlphaTestState::SetDefaultStates()
{
}

INLINE void C_AlphaTestState::SetAlphaTestCmpFunc(const U32 alphaTestCmpFunc)
{
}

INLINE void C_AlphaTestState::SetAlphaTestRef(const U8 alphaTestRef)
{
}

// C_BlendState

INLINE void C_BlendState::SetDefaultStates()
{
}

INLINE void C_BlendState::SetBlendColourSrc(const U32 blendColourSrc)
{
}

INLINE void C_BlendState::SetBlendColourDst(const U32 blendColourDst)
{
}

INLINE void C_BlendState::SetBlendColourOp(const U32 blendColourOp)
{
}

INLINE void C_BlendState::SetBlendAlphaSrc(const U32 blendAlphaSrc)
{
}

INLINE void C_BlendState::SetBlendAlphaDst(const U32 blendAlphaDst)
{
}

INLINE void C_BlendState::SetBlendAlphaOp(const U32 blendAlphaOp)
{
}

// C_ColourState

INLINE void C_ColourState::SetDefaultStates()
{
}
	
INLINE void C_ColourState::SetColourWriteEnable(const U32 colourWriteEnable)
{

}

// C_CullState

INLINE void C_CullState::SetDefaultStates()
{
}

INLINE void C_CullState::SetCullMode(const U32 cullMode)
{
}

// C_DepthState

INLINE void C_DepthState::SetDefaultStates()
{
}

INLINE void C_DepthState::SetDepthFunc(const U32 depthFunc)
{
}

INLINE void C_DepthState::SetDepthEnable(const U32 depthEnable)
{
}

INLINE void C_DepthState::SetDepthWriteEnable(const U32 depthWriteEnable)
{
}

INLINE void C_DepthState::SetDepthBias(const F32 depthBias)
{
}

INLINE void C_DepthState::SetSlopeScaledBias(const F32 slopeScaledBias) 
{
}

// C_FillState

INLINE void C_FillState::SetDefaultStates()
{
}

INLINE void C_FillState::SetFillMode(const U32 fillMode)
{
}

// C_TextureState

INLINE void C_TextureState::SetDefaultStates()
{
}

INLINE void C_TextureState::SetTexture(const C_Texture* pTexture)
{
}

// C_SamplerState

INLINE void C_SamplerState::SetDefaultStates()
{
}

INLINE void C_SamplerState::SetTextureAddressU(const U32 textureAddressU)
{
}

INLINE void C_SamplerState::SetTextureAddressV(const U32 textureAddressV)
{
}

INLINE void C_SamplerState::SetTextureAddressW(const U32 textureAddressW)
{
}

INLINE void C_SamplerState::SetTextureAddress(const U32 textureAddressU, const U32 textureAddressV)
{
}

INLINE void C_SamplerState::SetTextureAddress(const U32 textureAddressU, const U32 textureAddressV, const U32 textureAddressW)
{
}

INLINE void C_SamplerState::SetTextureAddress(const U32 textureAddress)
{
}

INLINE void C_SamplerState::SetTextureFilterMag(const U32 textureFilterMag) 
{
}

INLINE void C_SamplerState::SetTextureFilterMin(const U32 textureFilterMin)
{
}

INLINE void C_SamplerState::SetTextureFilterMip(const U32 textureFilterMip)
{
}

INLINE void C_SamplerState::SetTextureFilter(const U32 textureFilterMag, const U32 textureFilterMin, const U32 textureFilterMip)
{
}

INLINE void C_SamplerState::SetTextureFilter(const U32 textureFilter)
{
}

INLINE void C_SamplerState::SetTextureBorderColour(const U32 textureBorderColour)
{
}

INLINE void C_SamplerState::SetTextureLodBias(const S8 textureLodBias)
{
}

// C_ScissorState

INLINE void C_ScissorState::SetDefaultStates()
{
}

INLINE void C_ScissorState::SetScissorEnable(const U32 scissorEnable)
{
}

// C_StencilState

INLINE void C_StencilState::SetDefaultStates()
{
}

INLINE void C_StencilState::SetStencilFunc(const U32 stencilFunc)
{
}

INLINE void C_StencilState::SetStencilFail(const U32 stencilFail)
{
}

INLINE void C_StencilState::SetStencilDepthFail(const U32 stencilDepthFail)
{
}

INLINE void C_StencilState::SetStencilDepthPass(const U32 stencilDepthPass)
{
}

INLINE void C_StencilState::SetStencilRef(const U32 stencilRef)
{
}

INLINE void C_StencilState::SetStencilMask(const U32 stencilMask)
{
}

INLINE void C_StencilState::SetStencilWriteMask(const U32 stencilWriteMask)
{
}

// C_ContextState

INLINE void C_ContextState::SetDefaultStates()
{
}

// C_BlendMode

INLINE void C_BlendMode::SetDefaultStates()
{
}

} // namespace Display2

#endif // DISPLAY2_WII_CONTEXT_INL
