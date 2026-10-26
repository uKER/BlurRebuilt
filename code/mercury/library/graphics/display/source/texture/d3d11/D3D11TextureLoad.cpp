// [Blur1PCDX11] Upload PC mip and cube payloads into native D3D11 textures.
#include "StdAfx.h"
#include "Display/Texture/D3D11/D3D11TextureLoad.h"
#include "Display/Texture/TextureDesc.h"
#include "Display/Device/D3D11/D3D11Formats.h"
#include "Display2/Device/D3D11TextureFormats.h"
#include "Display/Device/Device.h"
#include <system/serialise/C_MemoryStream.h>
#include <vector>
#include <algorithm>

namespace Display
{
namespace
{
U32 MipBytes(U32 width, U32 height, const D3D11::C_FormatLayout& layout)
{
    const U32 blocksX = std::max<U32>(1, (width + layout.m_BlockWidth - 1) / layout.m_BlockWidth);
    const U32 blocksY = std::max<U32>(1, (height + layout.m_BlockHeight - 1) / layout.m_BlockHeight);
    return blocksX * blocksY * layout.m_BlockBytes;
}

bool TextureLayout(const TextureDesc& desc, D3D11::C_FormatLayout& layout)
{
    if (desc.Width <= 0 || desc.Height <= 0 || desc.Levels <= 0 ||
        (desc.Type != 3 && desc.Type != 5) ||
        !D3D11::GetFormatLayout(static_cast<D3DFORMAT>(desc.Format), &layout))
    {
        LOGERROR(g_Log, "Unsupported DX11 PC texture: type %u format %u dimensions %dx%d levels %d\n",
                 desc.Type, desc.Format, desc.Width, desc.Height, desc.Levels);
        return false;
    }
    return true;
}

U32 TotalBytes(const TextureDesc& desc, const D3D11::C_FormatLayout& layout)
{
    U32 total = 0;
    const U32 faces = desc.Type == 5 ? 6 : 1;
    for (U32 face = 0; face < faces; ++face)
        for (S32 mip = 0; mip < desc.Levels; ++mip)
            total += MipBytes(std::max(1, desc.Width >> mip),
                              std::max(1, desc.Height >> mip), layout);
    return total;
}
}

S32 CreateTexture2D(C_Texture2D* texture, Sys::I_BinaryStream* stream)
{
    if (!texture || !stream) return 0;
    TextureDesc source = {};
    Sys::BinaryDeserialiser<Ut::NoEndianSwap> reader(*stream);
    reader >> source;
    D3D11::C_FormatLayout layout = {};
    if (!TextureLayout(source, layout)) return 0;
    D3D11_TEXTURE2D_DESC desc = {};
    // [Blur1PCDX11] Standalone BC textures can be smaller than one 4x4 block
    // in PC data; DX11 requires block-aligned resource dimensions.
    desc.Width = layout.m_BlockWidth > 1 ?
        (source.Width + layout.m_BlockWidth - 1) / layout.m_BlockWidth * layout.m_BlockWidth : source.Width;
    desc.Height = layout.m_BlockHeight > 1 ?
        (source.Height + layout.m_BlockHeight - 1) / layout.m_BlockHeight * layout.m_BlockHeight : source.Height;
    desc.MipLevels = source.Levels;
    desc.ArraySize = source.Type == 5 ? 6 : 1;
    desc.Format = D3D11::GetPCTextureResourceFormat(static_cast<D3DFORMAT>(source.Format));
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = source.Type == 5 ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;
    ID3D11Texture2D* native = nullptr;
    ID3D11Device* device = Device()->GetD3DDevice();
    if (!device || FAILED(device->CreateTexture2D(&desc, nullptr, &native)))
        return 0;
    if (FAILED(Display2::D3D11TextureFormats::SetPCFormat(native,static_cast<D3DFORMAT>(source.Format))))
    { native->Release(); return 0; }
    // [Blur1PCDX11] Serialized assets share Display2's texture owner, but are
    // allocated here. Retire old views before replacing a streamed resource.
    if (texture->m_pd3dTexture)
    {
        D3D11::ReleaseTextureViewOwner(texture->m_pd3dTexture);
        texture->m_pd3dTexture->Release();
    }
    D3D11::RegisterTextureViewOwner(native);
    texture->m_pd3dTexture = native;
    const U32 total = TotalBytes(source, layout);
    texture->SetSize(total);
    return static_cast<S32>(total);
}

void LoadTextureData2D(C_Texture2D* texture, Sys::I_BinaryStream* stream, int size)
{
    if (!texture || !texture->m_pd3dTexture || !stream || size <= 0) return;
    ID3D11Texture2D* native = static_cast<ID3D11Texture2D*>(texture->m_pd3dTexture);
    D3D11_TEXTURE2D_DESC desc = {};
    native->GetDesc(&desc);
    D3D11::C_FormatLayout layout = {};
    if (!D3D11::GetFormatLayout(static_cast<D3DFORMAT>(texture->GetFormat()), &layout))
        return;
    ID3D11DeviceContext* context = nullptr;
    Device()->GetD3DDevice()->GetImmediateContext(&context);
    if (!context) return;
    Sys::BinaryDeserialiser<Ut::NoEndianSwap> reader(*stream);
    U32 consumed = 0;
    for (U32 face = 0; face < desc.ArraySize; ++face)
        for (U32 mip = 0; mip < desc.MipLevels; ++mip)
        {
            // [Blur1PCDX11] Streaming supplies only the newly loaded top mips;
            // the existing mip tail has already been copied from the old texture.
            if (consumed == static_cast<U32>(size)) break;
            const U32 width = std::max<U32>(1, desc.Width >> mip);
            const U32 height = std::max<U32>(1, desc.Height >> mip);
            const U32 bytes = MipBytes(width, height, layout);
            if (consumed + bytes > static_cast<U32>(size))
            {
                LOGERROR(g_Log, "Truncated PC texture mip payload\n");
                context->Release();
                return;
            }
            std::vector<U8> data(bytes);
            reader.Read(data.data(), bytes);
            const U32 rowPitch = std::max<U32>(1, (width + layout.m_BlockWidth - 1) / layout.m_BlockWidth)
                * layout.m_BlockBytes;
            std::vector<U8> converted;
            D3D11_SUBRESOURCE_DATA upload = {};
            D3D11::PreparePCTextureData(static_cast<D3DFORMAT>(texture->GetFormat()),width,height,1,
                data.data(),rowPitch,bytes,converted,upload);
            context->UpdateSubresource(native,
                D3D11CalcSubresource(mip, face, desc.MipLevels), nullptr,
                upload.pSysMem,upload.SysMemPitch,upload.SysMemSlicePitch);
            Display2::D3D11ResourceWritten(native, context); // [Blur1PCDX11]
            consumed += bytes;
        }
    context->Release();
}

void LoadTexture2D(C_Texture2D* texture, Sys::I_BinaryStream* stream)
{
    const S32 size = CreateTexture2D(texture, stream);
    if (size > 0) LoadTextureData2D(texture, stream, size);
}

void PCLoadTexture(C_Texture2D* texture, Sys::I_BinaryStream* stream)
{
    LoadTexture2D(texture, stream);
}

void AllocateTextureMemory(C_Texture2D*) {}

void CopyTextureData2D(C_Texture2D* destination, const C_Texture2D* source)
{
    if (!destination || !source || !destination->m_pd3dTexture || !source->m_pd3dTexture)
        return;
    ID3D11DeviceContext* context = nullptr;
    Device()->GetD3DDevice()->GetImmediateContext(&context);
    if (!context) return;
    ID3D11Texture2D* dst = static_cast<ID3D11Texture2D*>(destination->m_pd3dTexture);
    ID3D11Texture2D* src = static_cast<ID3D11Texture2D*>(source->m_pd3dTexture);
    D3D11_TEXTURE2D_DESC d = {}, s = {};
    dst->GetDesc(&d);
    src->GetDesc(&s);
    if (d.Width == s.Width && d.Height == s.Height && d.MipLevels == s.MipLevels &&
        d.ArraySize == s.ArraySize && d.Format == s.Format)
        context->CopyResource(dst, src);
    else if (d.Format == s.Format)
    {
        // [Blur1PCDX11] Match PCCopySurfaces: preserve the common mip tail
        // when streaming grows or shrinks a texture, rather than cropping mip 0
        // into mip 0 of a differently sized resource.
        const U32 sourceFirst = s.MipLevels > d.MipLevels ? s.MipLevels - d.MipLevels : 0;
        const U32 destinationFirst = d.MipLevels > s.MipLevels ? d.MipLevels - s.MipLevels : 0;
        for (U32 face = 0; face < std::min(d.ArraySize, s.ArraySize); ++face)
            for (U32 mip = 0; mip < std::min(d.MipLevels, s.MipLevels); ++mip)
            {
                context->CopySubresourceRegion(dst,
                    D3D11CalcSubresource(mip + destinationFirst, face, d.MipLevels),
                    0, 0, 0, src,
                    D3D11CalcSubresource(mip + sourceFirst, face, s.MipLevels), nullptr);
            }
    }
    Display2::D3D11ResourceWritten(dst, context); // [Blur1PCDX11]
    context->Release();
}

void CloneTexture2D(C_Texture2D* destination, const C_Texture2D* source,
                    int width, int height, int levels, int size)
{
    if (!destination || !source || !source->m_pd3dTexture ||
        width <= 0 || height <= 0 || levels <= 0) return;
    D3D11_TEXTURE2D_DESC desc = {};
    static_cast<ID3D11Texture2D*>(source->m_pd3dTexture)->GetDesc(&desc);
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = levels;
    ID3D11Texture2D* native = nullptr;
    if (FAILED(Device()->GetD3DDevice()->CreateTexture2D(&desc, nullptr, &native))) return;
    if (FAILED(Display2::D3D11TextureFormats::SetPCFormat(native,
        static_cast<D3DFORMAT>(source->GetFormat())))) { native->Release(); return; }
    // [Blur1PCDX11] A streaming clone gets its own view lifetime and descriptor.
    if (destination->m_pd3dTexture)
    {
        D3D11::ReleaseTextureViewOwner(destination->m_pd3dTexture);
        destination->m_pd3dTexture->Release();
    }
    D3D11::RegisterTextureViewOwner(native);
    destination->m_pd3dTexture = native;
    destination->SetSize(size);
    destination->SetFlags(source->GetFlags());
}
}
