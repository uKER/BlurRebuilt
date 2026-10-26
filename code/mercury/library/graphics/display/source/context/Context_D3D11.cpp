// [Blur1PCDX11] Bind the game render context directly to D3D11.
#include "StdAfx.h"
#include "Display/Context/Context_D3D11.h"
#include "Display/Device/Device.h"

namespace Display
{
C_ContextD3D11::C_ContextD3D11() {}

void C_ContextD3D11::SetVertexDeclaration(const C_VertexDeclaration* declaration)
{
    Device()->SetVertexDeclaration(declaration);
}

void C_ContextD3D11::SetVertexBuffer(U32 stream, const C_VertexBuffer* buffer,
                                     U32 offset, U32 stride)
{
    Device()->SetVertexBuffer(stream, buffer, offset, stride);
}

void C_ContextD3D11::SetIndexBuffer(const C_IndexBuffer* buffer)
{
    Device()->SetIndexBuffer(buffer);
}

void C_ContextD3D11::SetVertexShader(const C_VertexShader* shader)
{
    Device()->SetVertexShader(shader);
}

void C_ContextD3D11::SetPixelShader(const C_PixelShader* shader)
{
    Device()->SetPixelShader(shader);
}

void C_ContextD3D11::SetPixelShaderParameterF(C_PixelShaderParameter handle,
                                               const F32* data)
{
    if (handle.m_RegisterIndex != UINT_MAX && handle.m_RegisterCount)
        m_Context.SetPixelShaderConstantF(handle.m_RegisterIndex, data, 1);
}

void C_ContextD3D11::SetTexture(U32 sampler, const void* texture)
{
    // [Blur1PCDX11] Raw Display2 resources need the same SM5 register map as
    // Display textures, including retained views when the shader changes.
    Device()->SetNativeTexture(sampler, static_cast<ID3D11Resource*>(const_cast<void*>(texture)));
}

void C_ContextD3D11::SetVertexTexture(U32 sampler, const void* texture)
{
    Device()->SetNativeVertexTexture(sampler, static_cast<ID3D11Resource*>(const_cast<void*>(texture)));
}

void C_ContextD3D11::SetAlphaTestFunc(E_CmpFunc func) { Device()->SetAlphaTestFunc(func); }
void C_ContextD3D11::SetBlendHighPrecisionEnable(E_Bool) {}
void* C_ContextD3D11::BeginVertices(E_PrimitiveType, U32, U32,
                                    const C_VertexDeclaration*) { return nullptr; }
void C_ContextD3D11::EndVertices() {}

}
