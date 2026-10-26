#ifndef DISPLAY_DEVICE_D3D11_STATES_H
#define DISPLAY_DEVICE_D3D11_STATES_H

// [Blur1PCDX11] Map the renderer's existing state values onto native D3D11
// state objects. These are value mappings; no D3D9 device or API calls occur.
#include <d3d9.h>
#include <d3d11.h>

namespace Display { namespace D3D11 {

// [Blur1PCDX11] Select the alpha component of a PC RGBA blend factor.
D3D11_BLEND AlphaBlendFactor(D3D11_BLEND factor);

// [Blur1PCDX11] Input faces are PC clockwise/counterclockwise state banks.
// D3D11 faces follow rasterizer orientation; one-sided PC state covers both.
void ResolveStencilFaces(D3D11_DEPTH_STENCIL_DESC& desc, bool twoSided,
                         bool frontCounterClockwise, bool triangles = true);

bool MakeBlendDesc(D3DBLEND source, D3DBLEND destination, D3DBLENDOP operation,
                   D3DBLEND sourceAlpha, D3DBLEND destinationAlpha,
                   D3DBLENDOP alphaOperation, bool enabled, UINT8 writeMask,
                   D3D11_BLEND_DESC* result);

bool MakeDepthStencilDesc(bool depthEnabled, bool depthWrite, D3DCMPFUNC depthFunction,
                          bool stencilEnabled, D3DCMPFUNC stencilFunction,
                          D3DSTENCILOP stencilFail, D3DSTENCILOP depthFail,
                          D3DSTENCILOP stencilPass, UINT8 stencilReadMask,
                          UINT8 stencilWriteMask, D3D11_DEPTH_STENCIL_DESC* result);

bool MakeRasterizerDesc(D3DFILLMODE fill, D3DCULL cull, bool scissor,
                        D3D11_RASTERIZER_DESC* result);

bool MakeSamplerDesc(D3DTEXTUREFILTERTYPE minFilter,
                     D3DTEXTUREFILTERTYPE magFilter,
                     D3DTEXTUREFILTERTYPE mipFilter,
                     D3DTEXTUREADDRESS addressU, D3DTEXTUREADDRESS addressV,
                     D3DTEXTUREADDRESS addressW, UINT maxAnisotropy,
                     D3D11_SAMPLER_DESC* result,
                     UINT borderColour = 0, int encodedLodBias = 0);

} }

#endif
