#include "Display/Device/D3D11/D3D11States.h"

namespace Display { namespace D3D11 {

namespace {
bool Blend(D3DBLEND source, D3D11_BLEND* result)
{
    switch (source)
    {
    case D3DBLEND_ZERO: *result = D3D11_BLEND_ZERO; break;
    case D3DBLEND_ONE: *result = D3D11_BLEND_ONE; break;
    case D3DBLEND_SRCCOLOR: *result = D3D11_BLEND_SRC_COLOR; break;
    case D3DBLEND_INVSRCCOLOR: *result = D3D11_BLEND_INV_SRC_COLOR; break;
    case D3DBLEND_SRCALPHA: *result = D3D11_BLEND_SRC_ALPHA; break;
    case D3DBLEND_INVSRCALPHA: *result = D3D11_BLEND_INV_SRC_ALPHA; break;
    case D3DBLEND_DESTALPHA: *result = D3D11_BLEND_DEST_ALPHA; break;
    case D3DBLEND_INVDESTALPHA: *result = D3D11_BLEND_INV_DEST_ALPHA; break;
    case D3DBLEND_DESTCOLOR: *result = D3D11_BLEND_DEST_COLOR; break;
    case D3DBLEND_INVDESTCOLOR: *result = D3D11_BLEND_INV_DEST_COLOR; break;
    case D3DBLEND_SRCALPHASAT: *result = D3D11_BLEND_SRC_ALPHA_SAT; break;
    case D3DBLEND_BLENDFACTOR: *result = D3D11_BLEND_BLEND_FACTOR; break;
    case D3DBLEND_INVBLENDFACTOR: *result = D3D11_BLEND_INV_BLEND_FACTOR; break;
    default: return false;
    }
    return true;
}

bool BlendOp(D3DBLENDOP source, D3D11_BLEND_OP* result)
{
    switch (source)
    {
    case D3DBLENDOP_ADD: *result = D3D11_BLEND_OP_ADD; break;
    case D3DBLENDOP_SUBTRACT: *result = D3D11_BLEND_OP_SUBTRACT; break;
    case D3DBLENDOP_REVSUBTRACT: *result = D3D11_BLEND_OP_REV_SUBTRACT; break;
    case D3DBLENDOP_MIN: *result = D3D11_BLEND_OP_MIN; break;
    case D3DBLENDOP_MAX: *result = D3D11_BLEND_OP_MAX; break;
    default: return false;
    }
    return true;
}

bool Compare(D3DCMPFUNC source, D3D11_COMPARISON_FUNC* result)
{
    switch (source)
    {
    case D3DCMP_NEVER: *result = D3D11_COMPARISON_NEVER; break;
    case D3DCMP_LESS: *result = D3D11_COMPARISON_LESS; break;
    case D3DCMP_EQUAL: *result = D3D11_COMPARISON_EQUAL; break;
    case D3DCMP_LESSEQUAL: *result = D3D11_COMPARISON_LESS_EQUAL; break;
    case D3DCMP_GREATER: *result = D3D11_COMPARISON_GREATER; break;
    case D3DCMP_NOTEQUAL: *result = D3D11_COMPARISON_NOT_EQUAL; break;
    case D3DCMP_GREATEREQUAL: *result = D3D11_COMPARISON_GREATER_EQUAL; break;
    case D3DCMP_ALWAYS: *result = D3D11_COMPARISON_ALWAYS; break;
    default: return false;
    }
    return true;
}

bool StencilOp(D3DSTENCILOP source, D3D11_STENCIL_OP* result)
{
    switch (source)
    {
    case D3DSTENCILOP_KEEP: *result = D3D11_STENCIL_OP_KEEP; break;
    case D3DSTENCILOP_ZERO: *result = D3D11_STENCIL_OP_ZERO; break;
    case D3DSTENCILOP_REPLACE: *result = D3D11_STENCIL_OP_REPLACE; break;
    case D3DSTENCILOP_INCRSAT: *result = D3D11_STENCIL_OP_INCR_SAT; break;
    case D3DSTENCILOP_DECRSAT: *result = D3D11_STENCIL_OP_DECR_SAT; break;
    case D3DSTENCILOP_INVERT: *result = D3D11_STENCIL_OP_INVERT; break;
    case D3DSTENCILOP_INCR: *result = D3D11_STENCIL_OP_INCR; break;
    case D3DSTENCILOP_DECR: *result = D3D11_STENCIL_OP_DECR; break;
    default: return false;
    }
    return true;
}

bool Address(D3DTEXTUREADDRESS source, D3D11_TEXTURE_ADDRESS_MODE* result)
{
    switch (source)
    {
    case D3DTADDRESS_WRAP: *result = D3D11_TEXTURE_ADDRESS_WRAP; break;
    case D3DTADDRESS_MIRROR: *result = D3D11_TEXTURE_ADDRESS_MIRROR; break;
    case D3DTADDRESS_CLAMP: *result = D3D11_TEXTURE_ADDRESS_CLAMP; break;
    case D3DTADDRESS_BORDER: *result = D3D11_TEXTURE_ADDRESS_BORDER; break;
    case D3DTADDRESS_MIRRORONCE: *result = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE; break;
    default: return false;
    }
    return true;
}
}

D3D11_BLEND AlphaBlendFactor(D3D11_BLEND factor)
{
    // [Blur1PCDX11] D3D9 colour factors contain an alpha component. D3D11
    // rejects *_COLOR in the alpha equation; select that exact component.
    switch (factor)
    {
    case D3D11_BLEND_SRC_COLOR: return D3D11_BLEND_SRC_ALPHA;
    case D3D11_BLEND_INV_SRC_COLOR: return D3D11_BLEND_INV_SRC_ALPHA;
    case D3D11_BLEND_DEST_COLOR: return D3D11_BLEND_DEST_ALPHA;
    case D3D11_BLEND_INV_DEST_COLOR: return D3D11_BLEND_INV_DEST_ALPHA;
    case D3D11_BLEND_SRC_ALPHA_SAT: return D3D11_BLEND_ONE;
    default: return factor;
    }
}

bool MakeBlendDesc(D3DBLEND source, D3DBLEND destination, D3DBLENDOP operation,
                   D3DBLEND sourceAlpha, D3DBLEND destinationAlpha,
                   D3DBLENDOP alphaOperation, bool enabled, UINT8 writeMask,
                   D3D11_BLEND_DESC* result)
{
    if (!result) return false;
    D3D11_RENDER_TARGET_BLEND_DESC target = {};
    if (!Blend(source, &target.SrcBlend) || !Blend(destination, &target.DestBlend) ||
        !BlendOp(operation, &target.BlendOp) ||
        !Blend(sourceAlpha, &target.SrcBlendAlpha) ||
        !Blend(destinationAlpha, &target.DestBlendAlpha) ||
        !BlendOp(alphaOperation, &target.BlendOpAlpha)) return false;
    target.BlendEnable = enabled ? TRUE : FALSE;
    target.SrcBlendAlpha = AlphaBlendFactor(target.SrcBlendAlpha);
    target.DestBlendAlpha = AlphaBlendFactor(target.DestBlendAlpha);
    target.RenderTargetWriteMask = writeMask & D3D11_COLOR_WRITE_ENABLE_ALL;
    D3D11_BLEND_DESC desc = {};
    desc.RenderTarget[0] = target;
    *result = desc;
    return true;
}

void ResolveStencilFaces(D3D11_DEPTH_STENCIL_DESC& desc, bool twoSided,
                         bool frontCounterClockwise, bool triangles)
{
    // [Blur1PCDX11] D3D9 ignores the CCW bank for one-sided stencil, lines
    // and points. Keep the stored banks intact when producing a GPU state.
    if (!twoSided || !triangles) desc.BackFace = desc.FrontFace;
    else if (frontCounterClockwise)
    {
        const D3D11_DEPTH_STENCILOP_DESC clockwise = desc.FrontFace;
        desc.FrontFace = desc.BackFace;
        desc.BackFace = clockwise;
    }
}

bool MakeDepthStencilDesc(bool depthEnabled, bool depthWrite, D3DCMPFUNC depthFunction,
                          bool stencilEnabled, D3DCMPFUNC stencilFunction,
                          D3DSTENCILOP stencilFail, D3DSTENCILOP depthFail,
                          D3DSTENCILOP stencilPass, UINT8 stencilReadMask,
                          UINT8 stencilWriteMask, D3D11_DEPTH_STENCIL_DESC* result)
{
    if (!result) return false;
    D3D11_DEPTH_STENCIL_DESC desc = {};
    if (!Compare(depthFunction, &desc.DepthFunc) ||
        !Compare(stencilFunction, &desc.FrontFace.StencilFunc) ||
        !StencilOp(stencilFail, &desc.FrontFace.StencilFailOp) ||
        !StencilOp(depthFail, &desc.FrontFace.StencilDepthFailOp) ||
        !StencilOp(stencilPass, &desc.FrontFace.StencilPassOp)) return false;
    desc.DepthEnable = depthEnabled ? TRUE : FALSE;
    desc.DepthWriteMask = depthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    desc.StencilEnable = stencilEnabled ? TRUE : FALSE;
    desc.StencilReadMask = stencilReadMask;
    desc.StencilWriteMask = stencilWriteMask;
    desc.BackFace = desc.FrontFace;
    *result = desc;
    return true;
}

bool MakeRasterizerDesc(D3DFILLMODE fill, D3DCULL cull, bool scissor,
                        D3D11_RASTERIZER_DESC* result)
{
    if (!result) return false;
    D3D11_RASTERIZER_DESC desc = {};
    switch (fill)
    {
    case D3DFILL_SOLID: desc.FillMode = D3D11_FILL_SOLID; break;
    case D3DFILL_WIREFRAME: desc.FillMode = D3D11_FILL_WIREFRAME; break;
    default: return false; // D3D9 point fill has no D3D11 rasterizer equivalent.
    }
    switch (cull)
    {
    case D3DCULL_NONE: desc.CullMode = D3D11_CULL_NONE; break;
    case D3DCULL_CW: desc.CullMode = D3D11_CULL_BACK; desc.FrontCounterClockwise = TRUE; break;
    case D3DCULL_CCW: desc.CullMode = D3D11_CULL_BACK; desc.FrontCounterClockwise = FALSE; break;
    default: return false;
    }
    desc.DepthClipEnable = TRUE;
    desc.ScissorEnable = scissor ? TRUE : FALSE;
    *result = desc;
    return true;
}

bool MakeSamplerDesc(D3DTEXTUREFILTERTYPE minFilter, D3DTEXTUREFILTERTYPE magFilter,
                     D3DTEXTUREFILTERTYPE mipFilter, D3DTEXTUREADDRESS addressU,
                     D3DTEXTUREADDRESS addressV, D3DTEXTUREADDRESS addressW,
                     UINT maxAnisotropy, D3D11_SAMPLER_DESC* result,
                     UINT borderColour, int encodedLodBias)
{
    if (!result) return false;
    D3D11_SAMPLER_DESC desc = {};
    if (!Address(addressU, &desc.AddressU) || !Address(addressV, &desc.AddressV) ||
        !Address(addressW, &desc.AddressW)) return false;
    const bool anisotropic = minFilter == D3DTEXF_ANISOTROPIC || magFilter == D3DTEXF_ANISOTROPIC;
    if (anisotropic)
    {
        desc.Filter = D3D11_FILTER_ANISOTROPIC;
        if (!maxAnisotropy || maxAnisotropy > D3D11_MAX_MAXANISOTROPY) return false;
        desc.MaxAnisotropy = maxAnisotropy;
    }
    else
    {
        if ((minFilter != D3DTEXF_POINT && minFilter != D3DTEXF_LINEAR) ||
            (magFilter != D3DTEXF_POINT && magFilter != D3DTEXF_LINEAR) ||
            (mipFilter != D3DTEXF_NONE && mipFilter != D3DTEXF_POINT &&
             mipFilter != D3DTEXF_LINEAR)) return false;
        const bool minLinear = minFilter == D3DTEXF_LINEAR;
        const bool magLinear = magFilter == D3DTEXF_LINEAR;
        const bool mipLinear = mipFilter == D3DTEXF_LINEAR;
        desc.Filter = static_cast<D3D11_FILTER>((minLinear ? 0x10 : 0) |
                                             (magLinear ? 0x04 : 0) |
                                             (mipLinear ? 0x01 : 0));
        desc.MaxAnisotropy = 1;
    }
    // [Blur2PC] PC Display2 sends its signed bias to D3D9 as bias / 128 * 8.
    // Border colour is the original D3DCOLOR ARGB value, independent of SRGB.
    desc.MipLODBias = static_cast<float>(encodedLodBias) / 16.f;
    desc.BorderColor[0] = ((borderColour >> 16) & 255) / 255.f;
    desc.BorderColor[1] = ((borderColour >> 8) & 255) / 255.f;
    desc.BorderColor[2] = (borderColour & 255) / 255.f;
    desc.BorderColor[3] = ((borderColour >> 24) & 255) / 255.f;
    desc.MinLOD = 0.f;
    desc.MaxLOD = mipFilter == D3DTEXF_NONE ? 0.f : D3D11_FLOAT32_MAX;
    desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    *result = desc;
    return true;
}

} }
