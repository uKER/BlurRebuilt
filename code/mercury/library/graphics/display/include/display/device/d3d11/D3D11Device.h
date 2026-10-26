#ifndef DISPLAY_DEVICE_D3D11_D3D11_DEVICE_H
#define DISPLAY_DEVICE_D3D11_D3D11_DEVICE_H

// [Blur1PCDX11] First native D3D11 device path. The current game still uses
// Display::D3D9; this class becomes its replacement as resources are ported.
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <string>
#include <unordered_map>
#include "Display/Device/D3D11/D3D11Formats.h"

namespace Display { namespace D3D11 {

class C_D3D11Effect;

class C_D3D11Device
{
public:
    C_D3D11Device();

    HRESULT Initialise();
    HRESULT CreateOffscreenTarget(UINT width, UINT height);
    HRESULT CreateWindowTarget(HWND window, UINT width, UINT height);
    HRESULT ResizeWindowTarget(UINT width, UINT height);
    HRESULT Present(UINT syncInterval);
    HRESULT DrawProbeTriangle(const wchar_t* vertexShaderPath, const wchar_t* pixelShaderPath);
    HRESULT DrawColoured3D(const wchar_t* vertexShaderPath, const wchar_t* pixelShaderPath,
                           const float worldMatrix[16], const float viewProjMatrix[16],
                           const void* vertexData, UINT vertexBytes, UINT vertexStride,
                           const unsigned short* indexData, UINT indexCount);
    HRESULT DrawTexturedQuad(const wchar_t* vertexShaderPath, const wchar_t* pixelShaderPath,
                             const unsigned char rgba[4], bool srgb = false);
    HRESULT DrawTexturedQuadView(const wchar_t* vertexShaderPath,
                                 const wchar_t* pixelShaderPath,
                                 ID3D11ShaderResourceView* view,
                                 D3DTEXTUREFILTERTYPE filter = D3DTEXF_POINT);
    HRESULT DrawGameEffectIndexed(C_D3D11Effect& effect, const char* technique,
                                  UINT pass, const D3DVERTEXELEMENT9* elements,
                                  UINT elementCount, ID3D11Buffer* vertices,
                                  UINT vertexStride, ID3D11Buffer* indices,
                                  DXGI_FORMAT indexFormat, UINT indexCount,
                                  D3D11_PRIMITIVE_TOPOLOGY topology =
                                      D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    HRESULT CreateTexture2DFromPCData(UINT width, UINT height, UINT levels, D3DFORMAT format,
                                      const void* const* mipData, const UINT* mipSizes,
                                      Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture,
                                      Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& view,
                                      bool srgb = false);
    HRESULT CreateTexture3DFromPCData(UINT width, UINT height, UINT depth, UINT levels,
                                      D3DFORMAT format, const void* const* mipData,
                                      const UINT* mipSizes,
                                      Microsoft::WRL::ComPtr<ID3D11Texture3D>& texture,
                                      Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& view);
    HRESULT CreateTextureCubeFromPCData(UINT edge, UINT levels, D3DFORMAT format,
                                        const void* const* faceMipData,
                                        const UINT* faceMipSizes,
                                        Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture,
                                        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& view,
                                        bool srgb = false);
    HRESULT CreateRenderTexture2D(UINT width, UINT height, UINT levels, UINT samples,
                                  DXGI_FORMAT format,
                                  Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture,
                                  Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& target,
                                  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& view);
    HRESULT CreateDepthTexture2D(UINT width, UINT height, UINT samples,
                                 Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture,
                                 Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& target,
                                 Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& view);
    HRESULT CreateVertexBuffer(UINT bytes, const void* initialData, bool dynamic,
                               Microsoft::WRL::ComPtr<ID3D11Buffer>& buffer);
    HRESULT CreateIndexBuffer(UINT bytes, const void* initialData, bool dynamic,
                              Microsoft::WRL::ComPtr<ID3D11Buffer>& buffer);
    HRESULT UpdateDynamicBuffer(ID3D11Buffer* buffer, const void* data, UINT bytes);
    HRESULT CompileShader(const wchar_t* path, const char* profile,
                          Microsoft::WRL::ComPtr<ID3DBlob>& bytecode);
    HRESULT ReadPixel(UINT x, UINT y, unsigned char rgba[4]);

    D3D_FEATURE_LEVEL FeatureLevel() const { return m_FeatureLevel; }
    D3D_DRIVER_TYPE DriverType() const { return m_DriverType; }
    ID3D11Device* Device() const { return m_Device.Get(); }
    ID3D11DeviceContext* Context() const { return m_Context.Get(); }
    ID3D11RenderTargetView* RenderTargetView() const { return m_TargetView.Get(); }
    ID3D11DepthStencilView* DepthStencilView() const { return m_DepthView.Get(); }
    UINT Width() const { return m_Width; }
    UINT Height() const { return m_Height; }
    size_t InputLayoutCount() const { return m_InputLayouts.size(); }

private:
    Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_Context;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_Target;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_TargetView;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_Depth;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DepthView;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_DepthShaderView;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11InputLayout>> m_InputLayouts;
    D3D_FEATURE_LEVEL m_FeatureLevel;
    D3D_DRIVER_TYPE m_DriverType;
    UINT m_Width;
    UINT m_Height;

    HRESULT BindSwapChainBuffer(UINT width, UINT height);
    HRESULT CreateDepthTarget(UINT width, UINT height);
    HRESULT CreateBuffer(UINT bytes, UINT bindFlags, const void* initialData, bool dynamic,
                         Microsoft::WRL::ComPtr<ID3D11Buffer>& buffer);
};

} }

#endif
