#ifndef DISPLAY_DEVICE_D3D11_FORMATS_H
#define DISPLAY_DEVICE_D3D11_FORMATS_H

#include <d3d9.h>
#include <d3d11.h>
#include <dxgiformat.h>
#include <vector>

namespace Display { namespace D3D11 {

struct C_FormatLayout
{
    DXGI_FORMAT m_Format;
    unsigned m_BlockWidth;
    unsigned m_BlockHeight;
    unsigned m_BlockBytes;
};

// [Blur1PCDX11] Describe serialized PC bytes; native luminance storage is
// selected separately and never changes the serializer's byte/pitch authority.
bool GetFormatLayout(D3DFORMAT source, C_FormatLayout* layout);
// Serialized/locked bytes keep their PC layout; GPU luminance storage expands.
DXGI_FORMAT GetPCTextureResourceFormat(D3DFORMAT source);
void PreparePCTextureData(D3DFORMAT source, unsigned width, unsigned height, unsigned depth,
                         const void* bytes, unsigned rowPitch, unsigned slicePitch,
                         std::vector<unsigned char>& converted, D3D11_SUBRESOURCE_DATA& data);
DXGI_FORMAT GetSRGBFormat(DXGI_FORMAT linearFormat);
// [Blur1PCDX11] PC gamma is per sampler, so colour storage supports both views.
DXGI_FORMAT GetTextureResourceFormat(DXGI_FORMAT format);
DXGI_FORMAT GetLinearFormat(DXGI_FORMAT format);
HRESULT CreateTextureView(ID3D11Device* device, ID3D11Resource* resource,
                          bool srgb, ID3D11ShaderResourceView** view,
                          ID3D11ShaderResourceView* existing = nullptr, bool* owned = nullptr);
// [Blur1PCDX11] Changes whenever an owned texture is released, so a caller may
// keep "this texture has this owned view" until the generation changes.
unsigned TextureViewOwnerGeneration();
// [Blur1PCDX11] Display2's texture owner pairs these at creation/destruction.
// Retained views are released before its resource; raw resources stay uncached.
void RegisterTextureViewOwner(ID3D11Resource* resource);
void ReleaseTextureViewOwner(ID3D11Resource* resource);
// [Blur1PCDX11] Render-target and depth views of an owned resource, kept with
// its owner like the shader views; other resources get a new view each call.
// A null desc means the resource's default view.
HRESULT GetRenderTargetView(ID3D11Device* device, ID3D11Resource* resource,
                            const D3D11_RENDER_TARGET_VIEW_DESC* desc, ID3D11RenderTargetView** view);
HRESULT GetDepthStencilView(ID3D11Device* device, ID3D11Resource* resource,
                            const D3D11_DEPTH_STENCIL_VIEW_DESC* desc, ID3D11DepthStencilView** view);

} }

#endif
