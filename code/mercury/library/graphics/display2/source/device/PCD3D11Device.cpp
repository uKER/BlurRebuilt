// [Blur1PCDX11] Native Display2 resource ownership. No D3D9 device is created.
#include "StdAfx.h"
#include "Display2/Device/Device.h"
#include "Display/Device/D3D11/D3D11Formats.h"

namespace Display2
{
Util::C_String g_VertexShaderFileExt = "dx11vsh";
Util::C_String g_PixelShaderFileExt = "dx11psh";

C_DeviceParams::C_DeviceParams() {}
C_DeviceParams::~C_DeviceParams() {}

C_Device::C_Device()
{
    m_Context.m_pState = &m_ContextState;
    m_ContextNoCheck.m_pState = &m_ContextState;
}

C_Device::~C_Device() {}
U32 C_Device::IsCreated() const { return m_Created; }

void C_Device::GetParams(C_DeviceParams& params)
{
    params.m_pd3dDevice = m_pd3dDevice;
    params.m_pd3dObject = m_pd3dObject;
}

void C_Device::SetParams(const C_DeviceParams& params)
{
    m_pd3dDevice = params.m_pd3dDevice;
    m_pd3dObject = params.m_pd3dObject;
}

bool C_Device::Create()
{
    if (m_Created || !m_pd3dDevice)
        return false;
    m_Context.m_pd3dDevice = m_pd3dDevice;
    m_ContextNoCheck.m_pd3dDevice = m_pd3dDevice;
    m_ContextNoCheckNoTrack.m_pd3dDevice = m_pd3dDevice;
    m_Created = 1;
    return true;
}

bool C_Device::CreateVolatile()
{
    if (m_Created != 1)
        return false;
    m_Created = 2;
    return true;
}

void C_Device::DestroyVolatile() { m_Created = 1; }

void C_Device::Destroy()
{
    DestroyVertexDeclarations();
    m_Created = 0;
}

bool C_Device::CreateVertexBuffer(U32 size, U32 flags, C_VertexBuffer* buffer, void* memory)
{
    if (!m_pd3dDevice || !buffer || buffer->IsCreated() || !size || memory)
        return false;
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = size;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.Usage = (flags & E_Flags_Dynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = (flags & E_Flags_Dynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
    if (FAILED(m_pd3dDevice->CreateBuffer(&desc, nullptr, &buffer->m_pd3dVertexBuffer)))
        return false;
    buffer->SetFlags(flags);
    return true;
}

void C_Device::DestroyVertexBuffer(C_VertexBuffer* buffer)
{
    if (buffer && buffer->m_pd3dVertexBuffer)
    {
        buffer->m_pd3dVertexBuffer->Release();
        buffer->m_pd3dVertexBuffer = nullptr;
    }
}

bool C_Device::CreateIndexBuffer(U32 size, U32 flags, E_IndexFormat format,
                                 C_IndexBuffer* buffer, void* memory)
{
    if (!m_pd3dDevice || !buffer || buffer->IsCreated() || !size || memory ||
        (format != E_IndexFormat_16 && format != E_IndexFormat_32))
        return false;
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = size;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    desc.Usage = (flags & E_Flags_Dynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = (flags & E_Flags_Dynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
    if (FAILED(m_pd3dDevice->CreateBuffer(&desc, nullptr, &buffer->m_pd3dIndexBuffer)))
        return false;
    buffer->SetFlags(flags);
    buffer->m_Format = format;
    return true;
}

void C_Device::DestroyIndexBuffer(C_IndexBuffer* buffer)
{
    if (buffer && buffer->m_pd3dIndexBuffer)
    {
        buffer->m_pd3dIndexBuffer->Release();
        buffer->m_pd3dIndexBuffer = nullptr;
    }
}

bool C_Device::CreateVertexShader(U32 size, const U8* data, U32 flags,
                                  C_VertexShader* shader, void* memory)
{
    if (!m_pd3dDevice || !shader || shader->IsCreated() || !size || !data || memory)
        return false;
    if (FAILED(m_pd3dDevice->CreateVertexShader(data, size, nullptr,
                                                &shader->m_pd3dVertexShader)))
        return false;
    shader->SetFlags(flags);
    shader->m_Size = size;
    shader->m_Bytecode.assign(data, data + size);
    return true;
}

void C_Device::DestroyVertexShader(C_VertexShader* shader)
{
    if (shader && shader->m_pd3dVertexShader)
    {
        shader->m_pd3dVertexShader->Release();
        shader->m_pd3dVertexShader = nullptr;
    }
}

bool C_Device::CreatePixelShader(U32 size, const U8* data, U32 flags,
                                 C_PixelShader* shader, void* memory)
{
    if (!m_pd3dDevice || !shader || shader->IsCreated() || !size || !data || memory)
        return false;
    if (FAILED(m_pd3dDevice->CreatePixelShader(data, size, nullptr,
                                               &shader->m_pd3dPixelShader)))
        return false;
    shader->SetFlags(flags);
    shader->m_Size = size;
    shader->m_Bytecode.assign(data, data + size);
    return true;
}

void C_Device::DestroyPixelShader(C_PixelShader* shader)
{
    if (shader && shader->m_pd3dPixelShader)
    {
        shader->m_pd3dPixelShader->Release();
        shader->m_pd3dPixelShader = nullptr;
    }
}

void C_Device::Destroy(C_VertexBuffer* buffer) { DestroyVertexBuffer(buffer); }
void C_Device::Destroy(C_IndexBuffer* buffer) { DestroyIndexBuffer(buffer); }
void C_Device::Destroy(C_VertexShader* shader) { DestroyVertexShader(shader); }
void C_Device::Destroy(C_PixelShader* shader) { DestroyPixelShader(shader); }


namespace {
DXGI_FORMAT NativeFormat(E_TextureFormat format)
{
    switch (static_cast<D3DFORMAT>(format))
    {
    // [Blur1PCDX11] PC colour resources support linear and sRGB sampler views.
    case D3DFMT_A8B8G8R8: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
    case D3DFMT_A8R8G8B8: return DXGI_FORMAT_B8G8R8A8_TYPELESS;
    case D3DFMT_X8R8G8B8: return DXGI_FORMAT_B8G8R8X8_TYPELESS;
    case D3DFMT_DXT1: return DXGI_FORMAT_BC1_TYPELESS;
    case D3DFMT_DXT3: return DXGI_FORMAT_BC2_TYPELESS;
    case D3DFMT_DXT5: return DXGI_FORMAT_BC3_TYPELESS;
    case D3DFMT_L8: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
    case D3DFMT_A8: return DXGI_FORMAT_A8_UNORM;
    case D3DFMT_A8L8: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
    case D3DFMT_L16: return DXGI_FORMAT_R16G16B16A16_UNORM;
    case D3DFMT_R16F: return DXGI_FORMAT_R16_FLOAT;
    case D3DFMT_R32F: return DXGI_FORMAT_R32_FLOAT;
    case D3DFMT_G16R16: return DXGI_FORMAT_R16G16_UNORM;
    case D3DFMT_G16R16F: return DXGI_FORMAT_R16G16_FLOAT;
    case D3DFMT_A16B16G16R16F: return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case D3DFMT_A32B32G32R32F: return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case D3DFMT_D16: return DXGI_FORMAT_D16_UNORM;
    case D3DFMT_D24S8: return DXGI_FORMAT_D24_UNORM_S8_UINT;
    default: return DXGI_FORMAT_UNKNOWN;
    }
}
}

bool C_Device::CreateTexture2D(U32 width, U32 height, U32 levels, U32 flags,
                               E_TextureFormat format, C_Texture* texture, void* memory, U32 memorySize)
{
    if (!m_pd3dDevice || !texture || texture->IsCreated()) return false;
    const DXGI_FORMAT nativeFormat = NativeFormat(format);
    if (nativeFormat == DXGI_FORMAT_UNKNOWN) return false;
    D3D11_TEXTURE2D_DESC desc = {};
    const bool blockCompressed = nativeFormat == DXGI_FORMAT_BC1_TYPELESS ||
        nativeFormat == DXGI_FORMAT_BC2_TYPELESS || nativeFormat == DXGI_FORMAT_BC3_TYPELESS;
    // [Blur1PCDX11] D3D11 requires BC resource dimensions in four-pixel blocks.
    // Retail D3D9 assets include standalone 1x1 and 2x2 BC textures.
    desc.Width = blockCompressed ? ((width + 3) & ~3U) : width;
    desc.Height = blockCompressed ? ((height + 3) & ~3U) : height;
    desc.MipLevels = levels ? levels : 1;
    desc.ArraySize = 1;
    // [Blur1PCDX11] Depth sampling requires a typeless resource. The engine's
    // PC deferred pass also allocates a D24S8 texture for later shader reads.
    desc.Format = nativeFormat == DXGI_FORMAT_D24_UNORM_S8_UINT
        ? DXGI_FORMAT_R24G8_TYPELESS : nativeFormat;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT; desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    if (flags & E_Flags_Colour) desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
    if (flags & E_Flags_DepthStencil) desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    ID3D11Texture2D* native = nullptr;
    if (FAILED(m_pd3dDevice->CreateTexture2D(&desc, nullptr, &native))) return false;
    if (FAILED(D3D11TextureFormats::SetPCFormat(native,static_cast<D3DFORMAT>(format)))) { native->Release(); return false; }
    Display::D3D11::RegisterTextureViewOwner(native);
    texture->m_pd3dTexture = native;
    texture->SetFlags(flags);
    texture->SetSize(GetTexture2DSize(width, height, desc.MipLevels, flags, format));
    return true;
}

bool C_Device::CreateTexture3D(U32 width, U32 height, U32 depth, U32 levels, U32 flags,
                               E_TextureFormat format, C_Texture* texture, void* memory, U32 memorySize)
{
    if (!m_pd3dDevice || !texture || texture->IsCreated()) return false;
    D3D11_TEXTURE3D_DESC desc = {};
    desc.Width = width; desc.Height = height; desc.Depth = depth;
    desc.MipLevels = levels ? levels : 1; desc.Format = NativeFormat(format);
    desc.Usage = D3D11_USAGE_DEFAULT; desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    ID3D11Texture3D* native = nullptr;
    if (desc.Format == DXGI_FORMAT_UNKNOWN || FAILED(m_pd3dDevice->CreateTexture3D(&desc, nullptr, &native))) return false;
    if (FAILED(D3D11TextureFormats::SetPCFormat(native,static_cast<D3DFORMAT>(format)))) { native->Release(); return false; }
    Display::D3D11::RegisterTextureViewOwner(native);
    texture->m_pd3dTexture = native; texture->SetFlags(flags);
    texture->SetSize(GetTexture3DSize(width, height, depth, desc.MipLevels, flags, format));
    return true;
}

bool C_Device::CreateTextureCube(U32 width, U32 levels, U32 flags, E_TextureFormat format,
                                 C_Texture* texture, void* memory, U32 memorySize)
{
    if (!m_pd3dDevice || !texture || texture->IsCreated()) return false;
    D3D11_TEXTURE2D_DESC desc = {};
    const DXGI_FORMAT nativeFormat = NativeFormat(format);
    const bool blockCompressed = nativeFormat == DXGI_FORMAT_BC1_TYPELESS ||
        nativeFormat == DXGI_FORMAT_BC2_TYPELESS || nativeFormat == DXGI_FORMAT_BC3_TYPELESS;
    desc.Width = desc.Height = blockCompressed ? ((width + 3) & ~3U) : width;
    desc.MipLevels = levels ? levels : 1;
    desc.ArraySize = 6; desc.Format = NativeFormat(format); desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT; desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    if (flags & E_Flags_Colour) desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
    ID3D11Texture2D* native = nullptr;
    if (desc.Format == DXGI_FORMAT_UNKNOWN || FAILED(m_pd3dDevice->CreateTexture2D(&desc, nullptr, &native))) return false;
    if (FAILED(D3D11TextureFormats::SetPCFormat(native,static_cast<D3DFORMAT>(format)))) { native->Release(); return false; }
    Display::D3D11::RegisterTextureViewOwner(native);
    texture->m_pd3dTexture = native; texture->SetFlags(flags);
    texture->SetSize(GetTexture2DSize(width, width, desc.MipLevels, flags, format) * 6);
    return true;
}

void C_Device::DestroyTexture(C_Texture* texture)
{
    if (texture && texture->m_pd3dTexture)
    {
        // [Blur1PCDX11] Break the native view references with the PC texture
        // owner's destruction, including streaming and volatile recreation.
        Display::D3D11::ReleaseTextureViewOwner(texture->m_pd3dTexture);
        texture->m_pd3dTexture->Release(); texture->m_pd3dTexture = nullptr;
    }
}
void C_Device::Destroy(C_Texture* texture) { DestroyTexture(texture); }

bool C_Device::CreateSurface(U32 width, U32 height, U32 flags, E_SurfaceFormat format,
                             E_Multisample multisample, const C_SurfaceParams* params,
                             C_Surface* surface, void* memory, U32 memorySize)
{
    if (!m_pd3dDevice || !surface || surface->IsCreated()) return false;
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width; desc.Height = height; desc.MipLevels = 1; desc.ArraySize = 1;
    desc.Format = NativeFormat(static_cast<E_TextureFormat>(format));
    // [Blur1PCDX11] E_Multisample holds the D3DMULTISAMPLE_n_SAMPLES value, which is the sample count
    // (2..16); C_D3D11Manager offers every count the adapter supports, including 8 and 16.
    const UINT samples = static_cast<UINT>(multisample);
    desc.SampleDesc.Count = (samples >= 2 && samples <= 16) ? samples : 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = (flags & E_Flags_DepthStencil) ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET;
    if (desc.Format == DXGI_FORMAT_UNKNOWN ||
        FAILED(m_pd3dDevice->CreateTexture2D(&desc, nullptr, &surface->m_pd3dSurface))) return false;
    if (FAILED(D3D11TextureFormats::SetPCFormat(surface->m_pd3dSurface,static_cast<D3DFORMAT>(format))))
    { surface->m_pd3dSurface->Release(); surface->m_pd3dSurface = nullptr; return false; }
    // [Blur1PCDX11] Keep this surface's target views with it (D3D11Formats).
    Display::D3D11::RegisterTextureViewOwner(surface->m_pd3dSurface);
    surface->SetFlags(flags);
    return true;
}
bool C_Device::CreateColourSurface(U32 width, U32 height, U32 flags, E_SurfaceFormat format,
                                   E_Multisample samples, const C_SurfaceParams* params,
                                   C_Surface* surface, void* memory, U32 memorySize)
{ return CreateSurface(width, height, flags | E_Flags_Colour, format, samples, params, surface, memory, memorySize); }
bool C_Device::CreateDepthStencilSurface(U32 width, U32 height, U32 flags, E_SurfaceFormat format,
                                         E_Multisample samples, const C_SurfaceParams* params,
                                         C_Surface* surface, void* memory, U32 memorySize)
{ return CreateSurface(width, height, flags | E_Flags_DepthStencil, format, samples, params, surface, memory, memorySize); }
void C_Device::DestroySurface(C_Surface* surface)
{ if (surface && surface->m_pd3dSurface) { Display::D3D11::ReleaseTextureViewOwner(surface->m_pd3dSurface);
  surface->m_pd3dSurface->Release(); surface->m_pd3dSurface = nullptr; } }
void C_Device::Destroy(C_Surface* surface) { DestroySurface(surface); }
} // namespace Display2
