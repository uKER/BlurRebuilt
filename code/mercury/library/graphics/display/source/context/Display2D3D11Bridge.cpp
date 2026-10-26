// [Blur1PCDX11] Display2 state calls use the native game D3D11 device.
#include "stdafx.h"
#include "Display/Device/Device.h"
#include "Display2/Context/D3D11Context.h"
namespace Display2 {
namespace {
Display::C_Device* Game() { return Display::Device(); }
}
HRESULT C_ContextNoCheckNoTrack::SetAlphaTestState(const C_AlphaTestState& state)
{ Game()->SetAlphaTestFunc(static_cast<Display::E_CmpFunc>(state.m_AlphaTestCmpFunc));Game()->SetAlphaTestRef(state.m_AlphaTestRef);return S_OK; }
HRESULT C_ContextNoCheckNoTrack::SetBlendState(const C_BlendState& state)
{ Game()->SetBlend(static_cast<Display::E_Blend>(state.m_BlendColorSrc),static_cast<Display::E_Blend>(state.m_BlendColorDst),
    static_cast<Display::E_BlendOp>(state.m_BlendColorOp),static_cast<Display::E_Blend>(state.m_BlendAlphaSrc),
    static_cast<Display::E_Blend>(state.m_BlendAlphaDst),static_cast<Display::E_BlendOp>(state.m_BlendAlphaOp));return S_OK; }
HRESULT C_ContextNoCheckNoTrack::SetCullState(const C_CullState& state)
{ Game()->SetCullMode(static_cast<Display::E_CullMode>(state.m_CullMode));return S_OK; }
HRESULT C_ContextNoCheckNoTrack::SetSamplerState(U32 index,const C_SamplerState& state)
{ Game()->SetTextureAddressUVW(index,static_cast<Display::E_TextureAddress>(state.m_TextureAddressU),
    static_cast<Display::E_TextureAddress>(state.m_TextureAddressV),static_cast<Display::E_TextureAddress>(state.m_TextureAddressW));
  Game()->SetTextureFilterMagMinMip(index,static_cast<Display::E_TextureFilter>(state.m_TextureFilterMag),
    static_cast<Display::E_TextureFilter>(state.m_TextureFilterMin),static_cast<Display::E_TextureFilter>(state.m_TextureFilterMip));
  Game()->SetTextureBorderColour(index,state.m_TextureBorderColour);Game()->SetTextureLodBias(index,state.m_TextureLodBias);
  Game()->SetTextureGamma(index,static_cast<Display::E_Bool>(state.m_TextureGamma));return S_OK; }
HRESULT C_ContextNoCheckNoTrack::SetVertexSamplerState(U32 index,const C_SamplerState& state)
{ Game()->SetVertexTextureAddressUVW(index,static_cast<Display::E_TextureAddress>(state.m_TextureAddressU),
    static_cast<Display::E_TextureAddress>(state.m_TextureAddressV),static_cast<Display::E_TextureAddress>(state.m_TextureAddressW));
  Game()->SetVertexTextureFilterMagMinMip(index,static_cast<Display::E_TextureFilter>(state.m_TextureFilterMag),
    static_cast<Display::E_TextureFilter>(state.m_TextureFilterMin),static_cast<Display::E_TextureFilter>(state.m_TextureFilterMip));
  Game()->SetVertexTextureBorderColour(index,state.m_TextureBorderColour);Game()->SetVertexTextureLodBias(index,state.m_TextureLodBias);
  Game()->SetVertexTextureGamma(index,static_cast<Display::E_Bool>(state.m_TextureGamma));return S_OK; }
HRESULT C_ContextNoCheckNoTrack::SetStencilState(const C_StencilState& state)
{ Game()->SetStencilFunc(static_cast<Display::E_CmpFunc>(state.m_StencilFunc));
  Game()->SetStencilFail(static_cast<Display::E_StencilOp>(state.m_StencilFail));
  Game()->SetStencilDepthFail(static_cast<Display::E_StencilOp>(state.m_StencilDepthFail));
  Game()->SetStencilPass(static_cast<Display::E_StencilOp>(state.m_StencilDepthPass));
  Game()->SetStencilRef(state.m_StencilRef);Game()->SetStencilMask(state.m_StencilMask);
  Game()->SetStencilWriteMask(state.m_StencilWriteMask);return S_OK; }
HRESULT C_ContextNoCheckNoTrack::SetVertexDeclaration(const C_VertexDeclaration* value)
{Game()->SetVertexDeclaration(value);return S_OK;}
HRESULT C_ContextNoCheckNoTrack::SetVertexBuffer(U32 stream,const C_VertexBuffer* value,U32 offset,U32 stride,const C_VertexDeclaration* decl)
{Game()->SetVertexBuffer(stream,value,offset,stride);if(decl)Game()->SetVertexDeclaration(decl);return S_OK;}
HRESULT C_ContextNoCheckNoTrack::SetIndexBuffer(const C_IndexBuffer* value)
{Game()->SetIndexBuffer(value);return S_OK;}
HRESULT C_ContextNoCheckNoTrack::SetVertexShader(const C_VertexShader* value)
{Game()->SetVertexShader(value);return S_OK;}
HRESULT C_ContextNoCheckNoTrack::SetPixelShader(const C_PixelShader* value)
{Game()->SetPixelShader(value);return S_OK;}
HRESULT C_ContextNoCheckNoTrack::DrawIndexedPrimitive(E_PrimitiveType type,U32 base,U32 minIndex,U32 vertices,U32 start,U32 count,const C_IndexBuffer* index)
{
    // [Blur1PCDX11] The D3D9 overload ignores a null optional index pointer and
    // draws with the buffer already bound on the device. Preserve that state.
    if (index) Game()->SetIndexBuffer(index);
    Game()->DrawIndexedPrimitive(static_cast<Display::E_PrimitiveType>(type),base,minIndex,vertices,start,count);
    return S_OK;
}
HRESULT C_ContextNoCheckNoTrack::DrawPrimitive(E_PrimitiveType type,U32 start,U32 count)
{Game()->DrawPrimitive(static_cast<Display::E_PrimitiveType>(type),start,count);return S_OK;}
HRESULT C_ContextNoCheckNoTrack::SetAlphaTestEnable(E_Bool value)
{Game()->SetAlphaTestEnable(static_cast<Display::E_Bool>(value));return S_OK;}
HRESULT C_ContextNoCheckNoTrack::SetBlendEnable(E_Bool value)
{Game()->SetBlendEnable(static_cast<Display::E_Bool>(value));return S_OK;}
HRESULT C_ContextNoCheckNoTrack::SetColourWriteEnable(U32 value)
{Game()->SetColourWriteEnable(value);return S_OK;}
HRESULT C_ContextNoCheckNoTrack::SetDepthEnable(E_Bool value)
{Game()->SetDepthEnable(static_cast<Display::E_Bool>(value));return S_OK;}
HRESULT C_ContextNoCheckNoTrack::SetDepthFunc(E_CmpFunc value)
{Game()->SetDepthFunc(static_cast<Display::E_CmpFunc>(value));return S_OK;}
HRESULT C_ContextNoCheckNoTrack::SetDepthWriteEnable(E_Bool value)
{Game()->SetDepthWriteEnable(static_cast<Display::E_Bool>(value));return S_OK;}
HRESULT C_ContextNoCheckNoTrack::SetFillMode(E_FillMode value)
{Game()->SetFillMode(static_cast<Display::E_FillMode>(value));return S_OK;}
HRESULT C_ContextNoCheckNoTrack::SetPixelShaderConstantF(U32 start,const F32* data,U32 count)
{Game()->SetPixelShaderConstantF(start,data,count);return S_OK;}
HRESULT C_ContextNoCheckNoTrack::SetVertexShaderConstantF(U32 start,const F32* data,U32 count)
{Game()->SetVertexShaderConstantF(start,data,count);return S_OK;}
// [Blur1PCDX11] D3D9 uploads the reflected parameter span, including matrix
// arrays. The optional caller count defaults to one and is not the span.
HRESULT C_ContextNoCheckNoTrack::SetPixelShaderParamF(const C_PixelShaderParam& param,const F32* data,U32)
{
    if (param.m_RegisterIndex == UINT_MAX || !param.m_RegisterCount) return E_FAIL;
    Game()->SetPixelShaderConstantF(param.m_RegisterIndex,data,param.m_RegisterCount);
    return S_OK;
}
HRESULT C_ContextNoCheckNoTrack::SetPixelShaderParamArrayF(const C_PixelShaderParam& param,const F32* data,U32)
{
    if (param.m_RegisterIndex == UINT_MAX || !param.m_RegisterCount) return E_FAIL;
    Game()->SetPixelShaderConstantF(param.m_RegisterIndex,data,param.m_RegisterCount);
    return S_OK;
}
HRESULT C_ContextNoCheckNoTrack::SetPointSpriteEnable(E_Bool value)
{Game()->SetPointSpriteEnable(static_cast<Display::E_Bool>(value));return S_OK;}
HRESULT C_ContextNoCheckNoTrack::SetStencilEnable(E_Bool value)
{Game()->SetStencilEnable(static_cast<Display::E_Bool>(value));return S_OK;}
}
