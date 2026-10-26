#ifndef DISPLAY_CONTEXT_D3D11_H
#define DISPLAY_CONTEXT_D3D11_H

// [Blur1PCDX11] Game render context with native D3D11 bindings.
#include "Display/Context/Context_Base.h"

namespace Display
{
class C_ContextD3D11 : public C_ContextBase
{
public:
    C_ContextD3D11();
    void SetVertexDeclaration(const C_VertexDeclaration* declaration);
    void SetVertexBuffer(U32 stream, const C_VertexBuffer* buffer, U32 offset, U32 stride);
    void SetIndexBuffer(const C_IndexBuffer* buffer);
    void SetVertexShader(const C_VertexShader* shader);
    void SetPixelShader(const C_PixelShader* shader);
    void SetPixelShaderParameterF(C_PixelShaderParameter handle, const F32* data);
    void SetTexture(U32 sampler, const void* texture);
    void SetVertexTexture(U32 sampler, const void* texture);
    void SetAlphaTestFunc(E_CmpFunc func);
    void SetBlendHighPrecisionEnable(E_Bool enabled);
    void* BeginVertices(E_PrimitiveType type, U32 count, U32 stride,
                        const C_VertexDeclaration* declaration);
    void EndVertices();
};
}

#endif
