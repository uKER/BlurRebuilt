#include "Display/Device/D3D11/D3D11Device.h"
#include "Display/Device/D3D11/D3D11Effect.h"
#include "Display/Device/D3D11/D3D11InputLayout.h"
#include "Display/Device/D3D11/D3D11Constants.h"
#include "Display/Device/D3D11/D3D11States.h"
#include "Display2/Device/D3D11TextureFormats.h"

#include <d3dcompiler.h>
#include <d3d11sdklayers.h>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <map>
#include <string>
#include <vector>

namespace Display { namespace D3D11 {

namespace {
// D3D_COMPILE_STANDARD_FILE_INCLUDE does not resolve the engine's nested
// #includes relative to their own source files. Track each include's parent.
class C_ShaderInclude : public ID3DInclude
{
public:
    explicit C_ShaderInclude(const wchar_t* source) : m_Source(source) {}

    HRESULT STDMETHODCALLTYPE Open(D3D_INCLUDE_TYPE, LPCSTR fileName, LPCVOID parent,
                                   LPCVOID* data, UINT* bytes) override
    {
        if (!fileName || !data || !bytes) return E_INVALIDARG;
        const std::wstring& from = parent && m_Paths.count(parent) ? m_Paths[parent] : m_Source;
        const size_t slash = from.find_last_of(L"/\\");
        const std::wstring path = from.substr(0, slash + 1) +
                                  std::wstring(fileName, fileName + std::strlen(fileName));
        FILE* input = nullptr;
        if (_wfopen_s(&input, path.c_str(), L"rb") != 0 || !input) return E_FAIL;
        if (std::fseek(input, 0, SEEK_END) != 0) { std::fclose(input); return E_FAIL; }
        const long length = std::ftell(input);
        if (length < 0 || std::fseek(input, 0, SEEK_SET) != 0)
        { std::fclose(input); return E_FAIL; }
        char* contents = new char[static_cast<size_t>(length) + 1];
        const size_t read = std::fread(contents, 1, static_cast<size_t>(length), input);
        std::fclose(input);
        if (read != static_cast<size_t>(length)) { delete[] contents; return E_FAIL; }
        contents[length] = 0;
        *data = contents;
        *bytes = static_cast<UINT>(length);
        m_Paths[contents] = path;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Close(LPCVOID data) override
    {
        m_Paths.erase(data);
        delete[] static_cast<const char*>(data);
        return S_OK;
    }

private:
    std::wstring m_Source;
    std::map<const void*, std::wstring> m_Paths;
};
}

C_D3D11Device::C_D3D11Device()
    : m_FeatureLevel(D3D_FEATURE_LEVEL_9_1), m_DriverType(D3D_DRIVER_TYPE_UNKNOWN),
      m_Width(0), m_Height(0)
{
}

HRESULT C_D3D11Device::Initialise()
{
    m_InputLayouts.clear();
    m_SwapChain.Reset();
    m_DepthView.Reset();
    m_DepthShaderView.Reset();
    m_Depth.Reset();
    m_TargetView.Reset();
    m_Target.Reset();
    m_Context.Reset();
    m_Device.Reset();
    m_Width = m_Height = 0;
    m_DriverType = D3D_DRIVER_TYPE_UNKNOWN;
    const D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
    const D3D_DRIVER_TYPE drivers[] = { D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP };
    const bool forceWarp = GetEnvironmentVariableW(L"BLUR_DX11_WARP", nullptr, 0) != 0;
    const bool debug = GetEnvironmentVariableW(L"BLUR_DX11_DEBUG", nullptr, 0) != 0;
    HRESULT result = E_FAIL;
    for (unsigned i = 0; i < sizeof(drivers) / sizeof(drivers[0]); ++i)
    {
        if (forceWarp && drivers[i] != D3D_DRIVER_TYPE_WARP) continue;
        result = D3D11CreateDevice(nullptr, drivers[i], nullptr,
                                   debug ? D3D11_CREATE_DEVICE_DEBUG : 0, levels, 1,
                                   D3D11_SDK_VERSION, &m_Device, &m_FeatureLevel, &m_Context);
        if (SUCCEEDED(result))
        {
            m_DriverType = drivers[i];
            return S_OK;
        }
    }
    return result;
}

HRESULT C_D3D11Device::CreateOffscreenTarget(UINT width, UINT height)
{
    if (!m_Device || width == 0 || height == 0)
        return E_INVALIDARG;

    m_SwapChain.Reset();
    m_DepthView.Reset();
    m_DepthShaderView.Reset();
    m_Depth.Reset();
    m_TargetView.Reset();
    m_Target.Reset();
    m_Width = m_Height = 0;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    HRESULT result = m_Device->CreateTexture2D(&desc, nullptr, &m_Target);
    if (FAILED(result)) return result;
    result = m_Device->CreateRenderTargetView(m_Target.Get(), nullptr, &m_TargetView);
    if (FAILED(result)) return result;
    result = CreateDepthTarget(width, height);
    if (FAILED(result)) return result;
    m_Width = width;
    m_Height = height;
    return S_OK;
}

HRESULT C_D3D11Device::BindSwapChainBuffer(UINT width, UINT height)
{
    HRESULT result = m_SwapChain->GetBuffer(0, IID_PPV_ARGS(&m_Target));
    if (FAILED(result)) return result;
    result = m_Device->CreateRenderTargetView(m_Target.Get(), nullptr, &m_TargetView);
    if (FAILED(result)) { m_Target.Reset(); return result; }
    result = CreateDepthTarget(width, height);
    if (FAILED(result)) return result;
    m_Width = width;
    m_Height = height;
    return S_OK;
}

HRESULT C_D3D11Device::CreateDepthTarget(UINT width, UINT height)
{
    return CreateDepthTexture2D(width, height, 1, m_Depth, m_DepthView, m_DepthShaderView);
}

HRESULT C_D3D11Device::CreateWindowTarget(HWND window, UINT width, UINT height)
{
    if (!m_Device || !IsWindow(window) || width == 0 || height == 0)
        return E_INVALIDARG;
    m_Context->ClearState();
    m_DepthView.Reset();
    m_DepthShaderView.Reset();
    m_Depth.Reset();
    m_TargetView.Reset();
    m_Target.Reset();
    m_SwapChain.Reset();
    m_Width = m_Height = 0;

    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
    Microsoft::WRL::ComPtr<IDXGIFactory> factory;
    HRESULT result = m_Device.As(&dxgiDevice);
    if (FAILED(result)) return result;
    result = dxgiDevice->GetAdapter(&adapter);
    if (FAILED(result)) return result;
    result = adapter->GetParent(IID_PPV_ARGS(&factory));
    if (FAILED(result)) return result;

    DXGI_SWAP_CHAIN_DESC desc = {};
    desc.BufferDesc.Width = width;
    desc.BufferDesc.Height = height;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.OutputWindow = window;
    desc.Windowed = TRUE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    result = factory->CreateSwapChain(m_Device.Get(), &desc, &m_SwapChain);
    if (FAILED(result)) return result;
    factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER);
    return BindSwapChainBuffer(width, height);
}

HRESULT C_D3D11Device::ResizeWindowTarget(UINT width, UINT height)
{
    if (!m_SwapChain || width == 0 || height == 0)
        return E_INVALIDARG;
    m_Context->ClearState();
    m_DepthView.Reset();
    m_DepthShaderView.Reset();
    m_Depth.Reset();
    m_TargetView.Reset();
    m_Target.Reset();
    m_Width = m_Height = 0;
    HRESULT result = m_SwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(result)) return result;
    return BindSwapChainBuffer(width, height);
}

HRESULT C_D3D11Device::Present(UINT syncInterval)
{
    return m_SwapChain ? m_SwapChain->Present(syncInterval, 0) : E_UNEXPECTED;
}

HRESULT C_D3D11Device::CompileShader(const wchar_t* path, const char* profile,
                                     Microsoft::WRL::ComPtr<ID3DBlob>& bytecode)
{
    if (!path || !profile) return E_INVALIDARG;
    const size_t pathLength = std::wcslen(path);
    if (pathLength >= 4 && _wcsicmp(path + pathLength - 4, L".cso") == 0)
    {
        bytecode.Reset();
        return D3DReadFileToBlob(path, &bytecode);
    }
    const D3D_SHADER_MACRO defines[] = { { "BLUR_DX11", "1" }, { nullptr, nullptr } };
    C_ShaderInclude includes(path);
    Microsoft::WRL::ComPtr<ID3DBlob> errors;
    bytecode.Reset();
    const HRESULT result = D3DCompileFromFile(path, defines, &includes, "main", profile,
                                              D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR,
                                              0, &bytecode, &errors);
    if (FAILED(result) && errors)
        std::fprintf(stderr, "D3D11 shader compile: %s\n",
                     static_cast<const char*>(errors->GetBufferPointer()));
    return result;
}

HRESULT C_D3D11Device::CreateTexture2DFromPCData(
    UINT width, UINT height, UINT levels, D3DFORMAT format,
    const void* const* mipData, const UINT* mipSizes,
    Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture,
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& view, bool srgb)
{
    if (!m_Device || !width || !height || !levels || !mipData || !mipSizes)
        return E_INVALIDARG;
    C_FormatLayout layout;
    if (!GetFormatLayout(format, &layout)) return E_NOTIMPL;
    if (layout.m_Format == DXGI_FORMAT_D16_UNORM ||
        layout.m_Format == DXGI_FORMAT_D24_UNORM_S8_UINT)
        return E_INVALIDARG; // Depth data needs a depth-stencil resource/view.
    std::vector<D3D11_SUBRESOURCE_DATA> initial(levels);
    std::vector<std::vector<unsigned char>> converted(levels);
    UINT mipWidth = width, mipHeight = height;
    for (UINT level = 0; level < levels; ++level)
    {
        const UINT columns = (mipWidth + layout.m_BlockWidth - 1) / layout.m_BlockWidth;
        const UINT rows = (mipHeight + layout.m_BlockHeight - 1) / layout.m_BlockHeight;
        const UINT pitch = columns * layout.m_BlockBytes;
        const UINT size = pitch * rows;
        if (!mipData[level] || mipSizes[level] < size) return E_INVALIDARG;
        PreparePCTextureData(format,mipWidth,mipHeight,1,mipData[level],pitch,size,converted[level],initial[level]);
        mipWidth = mipWidth > 1 ? mipWidth / 2 : 1;
        mipHeight = mipHeight > 1 ? mipHeight / 2 : 1;
    }

    texture.Reset();
    view.Reset();
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = levels;
    desc.ArraySize = 1;
    desc.Format = GetPCTextureResourceFormat(format);
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    HRESULT result = m_Device->CreateTexture2D(&desc, initial.data(), &texture);
    if (FAILED(result)) return result;
    result = Display2::D3D11TextureFormats::SetPCFormat(texture.Get(),format);
    if (FAILED(result)) { texture.Reset(); return result; }
    result = CreateTextureView(m_Device.Get(), texture.Get(), srgb, &view);
    if (FAILED(result)) texture.Reset();
    return result;
}

HRESULT C_D3D11Device::CreateTexture3DFromPCData(
    UINT width, UINT height, UINT depth, UINT levels, D3DFORMAT format,
    const void* const* mipData, const UINT* mipSizes,
    Microsoft::WRL::ComPtr<ID3D11Texture3D>& texture,
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& view)
{
    if (!m_Device || !width || !height || !depth || !levels || !mipData || !mipSizes)
        return E_INVALIDARG;
    C_FormatLayout layout;
    if (!GetFormatLayout(format, &layout) || layout.m_BlockWidth != 1 ||
        layout.m_BlockHeight != 1 || layout.m_Format == DXGI_FORMAT_D16_UNORM ||
        layout.m_Format == DXGI_FORMAT_D24_UNORM_S8_UINT)
        return E_NOTIMPL;

    std::vector<D3D11_SUBRESOURCE_DATA> initial(levels);
    std::vector<std::vector<unsigned char>> converted(levels);
    UINT mipWidth = width, mipHeight = height, mipDepth = depth;
    for (UINT level = 0; level < levels; ++level)
    {
        const UINT64 rowPitch = static_cast<UINT64>(mipWidth) * layout.m_BlockBytes;
        const UINT64 slicePitch = rowPitch * mipHeight;
        const UINT64 total = slicePitch * mipDepth;
        if (!mipData[level] || rowPitch > UINT_MAX || slicePitch > UINT_MAX ||
            total > mipSizes[level]) return E_INVALIDARG;
        PreparePCTextureData(format,mipWidth,mipHeight,mipDepth,mipData[level],
            static_cast<UINT>(rowPitch),static_cast<UINT>(slicePitch),converted[level],initial[level]);
        mipWidth = mipWidth > 1 ? mipWidth / 2 : 1;
        mipHeight = mipHeight > 1 ? mipHeight / 2 : 1;
        mipDepth = mipDepth > 1 ? mipDepth / 2 : 1;
    }

    texture.Reset();
    view.Reset();
    D3D11_TEXTURE3D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.Depth = depth;
    desc.MipLevels = levels;
    desc.Format = GetPCTextureResourceFormat(format);
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    HRESULT result = m_Device->CreateTexture3D(&desc, initial.data(), &texture);
    if (FAILED(result)) return result;
    result = Display2::D3D11TextureFormats::SetPCFormat(texture.Get(),format);
    if (FAILED(result)) { texture.Reset(); return result; }
    result = CreateTextureView(m_Device.Get(), texture.Get(), false, &view);
    if (FAILED(result)) texture.Reset();
    return result;
}

HRESULT C_D3D11Device::CreateTextureCubeFromPCData(
    UINT edge, UINT levels, D3DFORMAT format,
    const void* const* faceMipData, const UINT* faceMipSizes,
    Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture,
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& view, bool srgb)
{
    if (!m_Device || !edge || !levels || !faceMipData || !faceMipSizes)
        return E_INVALIDARG;
    C_FormatLayout layout;
    if (!GetFormatLayout(format, &layout) || layout.m_Format == DXGI_FORMAT_D16_UNORM ||
        layout.m_Format == DXGI_FORMAT_D24_UNORM_S8_UINT) return E_NOTIMPL;
    std::vector<D3D11_SUBRESOURCE_DATA> initial(6 * levels);
    std::vector<std::vector<unsigned char>> converted(6 * levels);
    for (UINT face = 0; face < 6; ++face)
    {
        UINT mipEdge = edge;
        for (UINT level = 0; level < levels; ++level)
        {
            const UINT subresource = face * levels + level;
            const UINT64 columns = (static_cast<UINT64>(mipEdge) + layout.m_BlockWidth - 1) /
                                   layout.m_BlockWidth;
            const UINT64 rows = (static_cast<UINT64>(mipEdge) + layout.m_BlockHeight - 1) /
                                layout.m_BlockHeight;
            const UINT64 pitch = columns * layout.m_BlockBytes;
            const UINT64 size = pitch * rows;
            if (!faceMipData[subresource] || pitch > UINT_MAX ||
                size > faceMipSizes[subresource]) return E_INVALIDARG;
            PreparePCTextureData(format,mipEdge,mipEdge,1,faceMipData[subresource],
                static_cast<UINT>(pitch),static_cast<UINT>(size),converted[subresource],initial[subresource]);
            mipEdge = mipEdge > 1 ? mipEdge / 2 : 1;
        }
    }

    texture.Reset();
    view.Reset();
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = edge;
    desc.Height = edge;
    desc.MipLevels = levels;
    desc.ArraySize = 6;
    desc.Format = GetPCTextureResourceFormat(format);
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    HRESULT result = m_Device->CreateTexture2D(&desc, initial.data(), &texture);
    if (FAILED(result)) return result;
    result = Display2::D3D11TextureFormats::SetPCFormat(texture.Get(),format);
    if (FAILED(result)) { texture.Reset(); return result; }
    result = CreateTextureView(m_Device.Get(), texture.Get(), srgb, &view);
    if (FAILED(result)) texture.Reset();
    return result;
}

HRESULT C_D3D11Device::CreateRenderTexture2D(
    UINT width, UINT height, UINT levels, UINT samples, DXGI_FORMAT format,
    Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture,
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& target,
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& view)
{
    if (!m_Device || !width || !height || !levels || !samples ||
        (samples != 1 && levels != 1) || format == DXGI_FORMAT_UNKNOWN)
        return E_INVALIDARG;
    UINT quality = 0;
    HRESULT result = m_Device->CheckMultisampleQualityLevels(format, samples, &quality);
    if (FAILED(result) || quality == 0) return FAILED(result) ? result : E_NOTIMPL;

    texture.Reset();
    target.Reset();
    view.Reset();
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = levels;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = samples;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    result = m_Device->CreateTexture2D(&desc, nullptr, &texture);
    if (FAILED(result)) return result;
    result = m_Device->CreateRenderTargetView(texture.Get(), nullptr, &target);
    if (FAILED(result)) { texture.Reset(); return result; }
    result = m_Device->CreateShaderResourceView(texture.Get(), nullptr, &view);
    if (FAILED(result)) { target.Reset(); texture.Reset(); }
    return result;
}

HRESULT C_D3D11Device::CreateDepthTexture2D(
    UINT width, UINT height, UINT samples,
    Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture,
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& target,
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& view)
{
    if (!m_Device || !width || !height || !samples) return E_INVALIDARG;
    UINT quality = 0;
    HRESULT result = m_Device->CheckMultisampleQualityLevels(
        DXGI_FORMAT_D24_UNORM_S8_UINT, samples, &quality);
    if (FAILED(result) || quality == 0) return FAILED(result) ? result : E_NOTIMPL;

    texture.Reset();
    target.Reset();
    view.Reset();
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    desc.SampleDesc.Count = samples;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    result = m_Device->CreateTexture2D(&desc, nullptr, &texture);
    if (FAILED(result)) return result;

    D3D11_DEPTH_STENCIL_VIEW_DESC depthView = {};
    depthView.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthView.ViewDimension = samples > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS :
                                      D3D11_DSV_DIMENSION_TEXTURE2D;
    result = m_Device->CreateDepthStencilView(texture.Get(), &depthView, &target);
    if (FAILED(result)) { texture.Reset(); return result; }

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderView = {};
    shaderView.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    shaderView.ViewDimension = samples > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS :
                                       D3D11_SRV_DIMENSION_TEXTURE2D;
    if (samples == 1) shaderView.Texture2D.MipLevels = 1;
    result = m_Device->CreateShaderResourceView(texture.Get(), &shaderView, &view);
    if (FAILED(result)) { target.Reset(); texture.Reset(); }
    return result;
}

HRESULT C_D3D11Device::CreateBuffer(UINT bytes, UINT bindFlags, const void* initialData,
                                    bool dynamic, Microsoft::WRL::ComPtr<ID3D11Buffer>& buffer)
{
    if (!m_Device || !bytes || (!dynamic && !initialData)) return E_INVALIDARG;
    buffer.Reset();
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = bytes;
    desc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = bindFlags;
    desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
    D3D11_SUBRESOURCE_DATA initial = {};
    initial.pSysMem = initialData;
    return m_Device->CreateBuffer(&desc, initialData ? &initial : nullptr, &buffer);
}

HRESULT C_D3D11Device::CreateVertexBuffer(UINT bytes, const void* initialData, bool dynamic,
                                          Microsoft::WRL::ComPtr<ID3D11Buffer>& buffer)
{
    return CreateBuffer(bytes, D3D11_BIND_VERTEX_BUFFER, initialData, dynamic, buffer);
}

HRESULT C_D3D11Device::CreateIndexBuffer(UINT bytes, const void* initialData, bool dynamic,
                                         Microsoft::WRL::ComPtr<ID3D11Buffer>& buffer)
{
    return CreateBuffer(bytes, D3D11_BIND_INDEX_BUFFER, initialData, dynamic, buffer);
}

HRESULT C_D3D11Device::UpdateDynamicBuffer(ID3D11Buffer* buffer, const void* data, UINT bytes)
{
    if (!m_Context || !buffer || !data) return E_INVALIDARG;
    D3D11_BUFFER_DESC desc;
    buffer->GetDesc(&desc);
    if (desc.Usage != D3D11_USAGE_DYNAMIC || !bytes || bytes > desc.ByteWidth)
        return E_INVALIDARG;
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    HRESULT result = m_Context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(result)) return result;
    std::memcpy(mapped.pData, data, bytes);
    m_Context->Unmap(buffer, 0);
    return S_OK;
}

HRESULT C_D3D11Device::DrawProbeTriangle(const wchar_t* vertexShaderPath, const wchar_t* pixelShaderPath)
{
    if (!m_Device || !m_Context || !m_TargetView || !vertexShaderPath || !pixelShaderPath)
        return E_INVALIDARG;

    Microsoft::WRL::ComPtr<ID3DBlob> vsBytes, psBytes;
    HRESULT result = CompileShader(vertexShaderPath, "vs_5_0", vsBytes);
    if (FAILED(result)) return result;
    result = CompileShader(pixelShaderPath, "ps_5_0", psBytes);
    if (FAILED(result)) return result;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertices;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indices;
    result = m_Device->CreateVertexShader(vsBytes->GetBufferPointer(), vsBytes->GetBufferSize(), nullptr, &vs);
    if (FAILED(result)) return result;
    result = m_Device->CreatePixelShader(psBytes->GetBufferPointer(), psBytes->GetBufferSize(), nullptr, &ps);
    if (FAILED(result)) return result;

    const D3D11_INPUT_ELEMENT_DESC elements[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    result = m_Device->CreateInputLayout(elements, 2, vsBytes->GetBufferPointer(),
                                         vsBytes->GetBufferSize(), &layout);
    if (FAILED(result)) return result;

    C_ShaderConstants constants;
    result = constants.Initialise(m_Device.Get(), vsBytes->GetBufferPointer(),
                                  vsBytes->GetBufferSize());
    if (FAILED(result)) return result;
    const float clipVector[4] = { 1.f, 1.f, 0.f, 0.f };
    result = constants.SetVariable("g_ClipVector", clipVector, sizeof(clipVector));
    if (FAILED(result)) return result;

    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    if (!MakeDepthStencilDesc(false, false, D3DCMP_ALWAYS, false, D3DCMP_ALWAYS,
                              D3DSTENCILOP_KEEP, D3DSTENCILOP_KEEP, D3DSTENCILOP_KEEP,
                              0xff, 0xff, &depthDesc)) return E_INVALIDARG;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthState;
    result = m_Device->CreateDepthStencilState(&depthDesc, &depthState);
    if (FAILED(result)) return result;

    struct Vertex { float x, y, r, g, b, a; };
    const Vertex data[] = {
        {  0.0f,  0.8f, 1.0f, 0.0f, 0.0f, 1.0f },
        {  0.8f, -0.8f, 0.0f, 1.0f, 0.0f, 1.0f },
        { -0.8f, -0.8f, 0.0f, 0.0f, 1.0f, 1.0f }
    };
    result = CreateVertexBuffer(sizeof(data), data, false, vertices);
    if (FAILED(result)) return result;
    const unsigned short indexData[] = { 0, 1, 2 };
    result = CreateIndexBuffer(sizeof(indexData), indexData, false, indices);
    if (FAILED(result)) return result;

    const float clear[4] = { 0.02f, 0.04f, 0.08f, 1.0f };
    m_Context->ClearRenderTargetView(m_TargetView.Get(), clear);
    m_Context->ClearDepthStencilView(m_DepthView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    ID3D11RenderTargetView* target = m_TargetView.Get();
    m_Context->OMSetRenderTargets(1, &target, m_DepthView.Get());
    m_Context->OMSetDepthStencilState(depthState.Get(), 0);
    D3D11_VIEWPORT viewport = { 0, 0, static_cast<float>(m_Width), static_cast<float>(m_Height), 0, 1 };
    m_Context->RSSetViewports(1, &viewport);
    UINT stride = sizeof(Vertex), offset = 0;
    ID3D11Buffer* vertexBuffer = vertices.Get();
    m_Context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    m_Context->IASetIndexBuffer(indices.Get(), DXGI_FORMAT_R16_UINT, 0);
    m_Context->IASetInputLayout(layout.Get());
    m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_Context->VSSetShader(vs.Get(), nullptr, 0);
    m_Context->PSSetShader(ps.Get(), nullptr, 0);
    result = constants.UploadAndBindVS(m_Context.Get());
    if (FAILED(result)) return result;
    m_Context->DrawIndexed(3, 0, 0);

    Microsoft::WRL::ComPtr<ID3D11InfoQueue> queue;
    if (GetEnvironmentVariableW(L"BLUR_DX11_DEBUG", nullptr, 0) != 0 &&
        SUCCEEDED(m_Device.As(&queue)))
    {
        const UINT64 count = queue->GetNumStoredMessages();
        for (UINT64 i = 0; i < count; ++i)
        {
            SIZE_T size = 0;
            queue->GetMessage(i, nullptr, &size);
            std::string message(size, '\0');
            D3D11_MESSAGE* info = reinterpret_cast<D3D11_MESSAGE*>(&message[0]);
            if (SUCCEEDED(queue->GetMessage(i, info, &size)))
                std::fprintf(stderr, "D3D11: %s\n", info->pDescription);
        }
    }

    // The pipeline retains bindings; release them before local COM objects go away.
    m_Context->ClearState();
    return S_OK;
}

HRESULT C_D3D11Device::DrawTexturedQuad(const wchar_t* vertexShaderPath,
                                         const wchar_t* pixelShaderPath,
                                         const unsigned char rgba[4], bool srgb)
{
    if (!rgba) return E_INVALIDARG;
    const unsigned char bgra[4] = { rgba[2], rgba[1], rgba[0], rgba[3] };
    const void* mipData[] = { bgra };
    const UINT mipSizes[] = { sizeof(bgra) };
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> view;
    const HRESULT result = CreateTexture2DFromPCData(1, 1, 1, D3DFMT_A8R8G8B8,
                                                      mipData, mipSizes, texture,
                                                      view, srgb);
    return FAILED(result) ? result : DrawTexturedQuadView(vertexShaderPath,
                                                           pixelShaderPath, view.Get());
}

HRESULT C_D3D11Device::DrawTexturedQuadView(const wchar_t* vertexShaderPath,
                                             const wchar_t* pixelShaderPath,
                                               ID3D11ShaderResourceView* view,
                                               D3DTEXTUREFILTERTYPE filter)
{
    if (!m_Device || !m_Context || !m_TargetView || !m_DepthView ||
        !vertexShaderPath || !pixelShaderPath || !view)
        return E_INVALIDARG;

    Microsoft::WRL::ComPtr<ID3DBlob> vsBytes, psBytes;
    HRESULT result = CompileShader(vertexShaderPath, "vs_5_0", vsBytes);
    if (FAILED(result)) return result;
    result = CompileShader(pixelShaderPath, "ps_5_0", psBytes);
    if (FAILED(result)) return result;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> layout;
    result = m_Device->CreateVertexShader(vsBytes->GetBufferPointer(), vsBytes->GetBufferSize(),
                                          nullptr, &vs);
    if (FAILED(result)) return result;
    result = m_Device->CreatePixelShader(psBytes->GetBufferPointer(), psBytes->GetBufferSize(),
                                         nullptr, &ps);
    if (FAILED(result)) return result;
    const D3D11_INPUT_ELEMENT_DESC elements[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    result = m_Device->CreateInputLayout(elements, 3, vsBytes->GetBufferPointer(),
                                         vsBytes->GetBufferSize(), &layout);
    if (FAILED(result)) return result;

    C_ShaderConstants constants;
    result = constants.Initialise(m_Device.Get(), vsBytes->GetBufferPointer(),
                                  vsBytes->GetBufferSize());
    if (FAILED(result)) return result;
    const float clipVector[4] = { 1.f, 1.f, 0.f, 0.f };
    result = constants.SetVariable("g_ClipVector", clipVector, sizeof(clipVector));
    if (FAILED(result)) return result;

    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    if (!MakeDepthStencilDesc(false, false, D3DCMP_ALWAYS, false, D3DCMP_ALWAYS,
                              D3DSTENCILOP_KEEP, D3DSTENCILOP_KEEP, D3DSTENCILOP_KEEP,
                              0xff, 0xff, &depthDesc)) return E_INVALIDARG;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthState;
    result = m_Device->CreateDepthStencilState(&depthDesc, &depthState);
    if (FAILED(result)) return result;

    struct Vertex { float x, y, r, g, b, a, u, v; };
    const Vertex vertexData[] = {
        { -1,  1, 1, 1, 1, 1, 0, 0 },
        {  1,  1, 1, 1, 1, 1, 1, 0 },
        {  1, -1, 1, 1, 1, 1, 1, 1 },
        { -1, -1, 1, 1, 1, 1, 0, 1 }
    };
    const unsigned short indexData[] = { 0, 1, 2, 0, 2, 3 };
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertices, indices;
    result = CreateVertexBuffer(sizeof(vertexData), vertexData, false, vertices);
    if (FAILED(result)) return result;
    result = CreateIndexBuffer(sizeof(indexData), indexData, false, indices);
    if (FAILED(result)) return result;

    D3D11_SAMPLER_DESC samplerDesc = {};
    if (!MakeSamplerDesc(filter, filter, filter,
                         D3DTADDRESS_CLAMP, D3DTADDRESS_CLAMP, D3DTADDRESS_CLAMP,
                         1, &samplerDesc)) return E_INVALIDARG;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
    result = m_Device->CreateSamplerState(&samplerDesc, &sampler);
    if (FAILED(result)) return result;

    const float clear[4] = { 0, 0, 0, 1 };
    m_Context->ClearRenderTargetView(m_TargetView.Get(), clear);
    m_Context->ClearDepthStencilView(m_DepthView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
    ID3D11RenderTargetView* target = m_TargetView.Get();
    m_Context->OMSetRenderTargets(1, &target, m_DepthView.Get());
    m_Context->OMSetDepthStencilState(depthState.Get(), 0);
    D3D11_VIEWPORT viewport = { 0, 0, static_cast<float>(m_Width), static_cast<float>(m_Height), 0, 1 };
    m_Context->RSSetViewports(1, &viewport);
    UINT stride = sizeof(Vertex), offset = 0;
    ID3D11Buffer* vertexBuffer = vertices.Get();
    m_Context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    m_Context->IASetIndexBuffer(indices.Get(), DXGI_FORMAT_R16_UINT, 0);
    m_Context->IASetInputLayout(layout.Get());
    m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_Context->VSSetShader(vs.Get(), nullptr, 0);
    m_Context->PSSetShader(ps.Get(), nullptr, 0);
    result = constants.UploadAndBindVS(m_Context.Get());
    if (FAILED(result)) return result;
    ID3D11ShaderResourceView* textureView = view;
    ID3D11SamplerState* samplerState = sampler.Get();
    m_Context->PSSetShaderResources(0, 1, &textureView);
    m_Context->PSSetSamplers(0, 1, &samplerState);
    m_Context->DrawIndexed(6, 0, 0);
    m_Context->ClearState();
    return S_OK;
}

HRESULT C_D3D11Device::DrawGameEffectIndexed(
    C_D3D11Effect& effect, const char* technique, UINT pass,
    const D3DVERTEXELEMENT9* elements, UINT elementCount,
    ID3D11Buffer* vertices, UINT vertexStride, ID3D11Buffer* indices,
    DXGI_FORMAT indexFormat, UINT indexCount, D3D11_PRIMITIVE_TOPOLOGY topology)
{
    if (!m_Device || !m_Context || !technique || !elements || !elementCount ||
        !vertices || !vertexStride || !indices || !indexCount ||
        (indexFormat != DXGI_FORMAT_R16_UINT && indexFormat != DXGI_FORMAT_R32_UINT) ||
        topology == D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED)
        return E_INVALIDARG;
    const void* signature = nullptr;
    SIZE_T signatureBytes = 0;
    // [Blur1PCDX11] CreateGameInputLayout reflects the shader's inputs; an
    // isolated IA signature cannot be passed to D3DReflect.
    HRESULT result = effect.GetPassVertexShaderBytecode(technique, pass,
                                                  &signature, &signatureBytes);
    if (FAILED(result)) return result;
    std::string layoutKey(static_cast<const char*>(signature), signatureBytes);
    layoutKey.append(reinterpret_cast<const char*>(&elementCount), sizeof(elementCount));
    for (UINT i = 0; i < elementCount; ++i)
    {
        const D3DVERTEXELEMENT9& element = elements[i];
        layoutKey.append(reinterpret_cast<const char*>(&element.Stream), sizeof(element.Stream));
        layoutKey.append(reinterpret_cast<const char*>(&element.Offset), sizeof(element.Offset));
        layoutKey.push_back(static_cast<char>(element.Type));
        layoutKey.push_back(static_cast<char>(element.Method));
        layoutKey.push_back(static_cast<char>(element.Usage));
        layoutKey.push_back(static_cast<char>(element.UsageIndex));
    }
    auto found = m_InputLayouts.find(layoutKey);
    if (found == m_InputLayouts.end())
    {
        Microsoft::WRL::ComPtr<ID3D11InputLayout> created;
        result = CreateGameInputLayout(m_Device.Get(), elements, elementCount,
                                       signature, signatureBytes, created);
        if (FAILED(result)) return result;
        found = m_InputLayouts.emplace(std::move(layoutKey), std::move(created)).first;
    }
    const UINT offset = 0;
    m_Context->IASetInputLayout(found->second.Get());
    m_Context->IASetVertexBuffers(0, 1, &vertices, &vertexStride, &offset);
    m_Context->IASetIndexBuffer(indices, indexFormat, 0);
    m_Context->IASetPrimitiveTopology(topology);
    result = effect.Apply(technique, pass, m_Context.Get());
    if (FAILED(result)) return result;
    m_Context->DrawIndexed(indexCount, 0, 0);
    return S_OK;
}

HRESULT C_D3D11Device::DrawColoured3D(const wchar_t* vertexShaderPath,
                                      const wchar_t* pixelShaderPath,
                                      const float worldMatrix[16],
                                      const float viewProjMatrix[16],
                                      const void* vertexData, UINT vertexBytes,
                                      UINT vertexStride, const unsigned short* indexData,
                                      UINT indexCount)
{
    if (!m_Device || !m_Context || !m_TargetView || !m_DepthView ||
        !vertexShaderPath || !pixelShaderPath || !worldMatrix || !viewProjMatrix ||
        !vertexData || !vertexBytes || vertexStride < 28 || !indexData || !indexCount)
        return E_INVALIDARG;

    Microsoft::WRL::ComPtr<ID3DBlob> vsBytes, psBytes;
    HRESULT result = CompileShader(vertexShaderPath, "vs_5_0", vsBytes);
    if (FAILED(result)) return result;
    result = CompileShader(pixelShaderPath, "ps_5_0", psBytes);
    if (FAILED(result)) return result;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> layout;
    result = m_Device->CreateVertexShader(vsBytes->GetBufferPointer(), vsBytes->GetBufferSize(),
                                          nullptr, &vs);
    if (FAILED(result)) return result;
    result = m_Device->CreatePixelShader(psBytes->GetBufferPointer(), psBytes->GetBufferSize(),
                                         nullptr, &ps);
    if (FAILED(result)) return result;
    const D3D11_INPUT_ELEMENT_DESC elements[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    result = m_Device->CreateInputLayout(elements, 2, vsBytes->GetBufferPointer(),
                                         vsBytes->GetBufferSize(), &layout);
    if (FAILED(result)) return result;

    C_ShaderConstants constants;
    result = constants.Initialise(m_Device.Get(), vsBytes->GetBufferPointer(),
                                  vsBytes->GetBufferSize());
    if (FAILED(result)) return result;
    result = constants.SetVariable("g_WorldMatrix", worldMatrix, 16 * sizeof(float));
    if (FAILED(result)) return result;
    result = constants.SetVariable("g_ViewProjMatrix", viewProjMatrix, 16 * sizeof(float));
    if (FAILED(result)) return result;

    Microsoft::WRL::ComPtr<ID3D11Buffer> vertices, indices;
    result = CreateVertexBuffer(vertexBytes, vertexData, false, vertices);
    if (FAILED(result)) return result;
    result = CreateIndexBuffer(indexCount * sizeof(unsigned short), indexData, false, indices);
    if (FAILED(result)) return result;

    D3D11_RASTERIZER_DESC raster = {};
    if (!MakeRasterizerDesc(D3DFILL_SOLID, D3DCULL_NONE, false, &raster))
        return E_INVALIDARG;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterState;
    result = m_Device->CreateRasterizerState(&raster, &rasterState);
    if (FAILED(result)) return result;

    D3D11_BLEND_DESC blend = {};
    if (!MakeBlendDesc(D3DBLEND_ONE, D3DBLEND_ZERO, D3DBLENDOP_ADD,
                       D3DBLEND_ONE, D3DBLEND_ZERO, D3DBLENDOP_ADD,
                       false, D3D11_COLOR_WRITE_ENABLE_ALL, &blend)) return E_INVALIDARG;
    Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;
    result = m_Device->CreateBlendState(&blend, &blendState);
    if (FAILED(result)) return result;

    const float clear[4] = { 0.02f, 0.04f, 0.08f, 1.f };
    m_Context->ClearRenderTargetView(m_TargetView.Get(), clear);
    m_Context->ClearDepthStencilView(m_DepthView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                     1.f, 0);
    ID3D11RenderTargetView* target = m_TargetView.Get();
    m_Context->OMSetRenderTargets(1, &target, m_DepthView.Get());
    m_Context->OMSetBlendState(blendState.Get(), nullptr, 0xffffffffu);
    D3D11_VIEWPORT viewport = { 0, 0, static_cast<float>(m_Width),
                                static_cast<float>(m_Height), 0, 1 };
    m_Context->RSSetViewports(1, &viewport);
    m_Context->RSSetState(rasterState.Get());
    const UINT offset = 0;
    ID3D11Buffer* vertexBuffer = vertices.Get();
    m_Context->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexStride, &offset);
    m_Context->IASetIndexBuffer(indices.Get(), DXGI_FORMAT_R16_UINT, 0);
    m_Context->IASetInputLayout(layout.Get());
    m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_Context->VSSetShader(vs.Get(), nullptr, 0);
    m_Context->PSSetShader(ps.Get(), nullptr, 0);
    result = constants.UploadAndBindVS(m_Context.Get());
    if (FAILED(result)) { m_Context->ClearState(); return result; }
    m_Context->DrawIndexed(indexCount, 0, 0);
    m_Context->ClearState();
    return S_OK;
}

HRESULT C_D3D11Device::ReadPixel(UINT x, UINT y, unsigned char rgba[4])
{
    if (!m_Device || !m_Context || !m_Target || !rgba || x >= m_Width || y >= m_Height)
        return E_INVALIDARG;

    D3D11_TEXTURE2D_DESC desc;
    m_Target->GetDesc(&desc);
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> staging;
    HRESULT result = m_Device->CreateTexture2D(&desc, nullptr, &staging);
    if (FAILED(result)) return result;
    m_Context->CopyResource(staging.Get(), m_Target.Get());
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    result = m_Context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(result)) return result;
    std::memcpy(rgba, static_cast<const unsigned char*>(mapped.pData) + y * mapped.RowPitch + x * 4, 4);
    m_Context->Unmap(staging.Get(), 0);
    return S_OK;
}

} }
