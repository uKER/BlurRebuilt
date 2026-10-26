// [Blur1PCDX11] Exact PC luminance storage conversion. Expanding at upload
// preserves RGB replication and hardware sRGB decoding before filtering.
#ifndef DISPLAY2_DEVICE_D3D11_TEXTURE_FORMATS_H
#define DISPLAY2_DEVICE_D3D11_TEXTURE_FORMATS_H

#include <d3d9.h>
#include <d3d11.h>
#include <cstring>

namespace Display2 { namespace D3D11TextureFormats {

inline bool LuminanceLayout(D3DFORMAT format, DXGI_FORMAT* storage,
                            unsigned* pcBytes, unsigned* nativeBytes)
{
    switch (format)
    {
    case D3DFMT_L8: *storage = DXGI_FORMAT_R8G8B8A8_TYPELESS; *pcBytes = 1; *nativeBytes = 4; return true;
    case D3DFMT_A8L8: *storage = DXGI_FORMAT_R8G8B8A8_TYPELESS; *pcBytes = 2; *nativeBytes = 4; return true;
    case D3DFMT_L16: *storage = DXGI_FORMAT_R16G16B16A16_UNORM; *pcBytes = 2; *nativeBytes = 8; return true;
    default: return false;
    }
}

// Resource-local identity survives aliases without a pointer-keyed side table.
inline const GUID& PCFormatKey()
{
    static const GUID key = {0x3e987278,0x6b61,0x4d78,{0x9e,0x62,0xc6,0x83,0x57,0xa6,0xc5,0x71}};
    return key;
}
inline HRESULT SetPCFormat(ID3D11Resource* resource, D3DFORMAT format)
{
    return resource ? resource->SetPrivateData(PCFormatKey(),sizeof(format),&format) : E_INVALIDARG;
}
inline D3DFORMAT GetPCFormat(ID3D11Resource* resource)
{
    D3DFORMAT format = D3DFMT_UNKNOWN;
    UINT bytes = sizeof(format);
    if (!resource || FAILED(resource->GetPrivateData(PCFormatKey(),&bytes,&format)) || bytes != sizeof(format))
        return D3DFMT_UNKNOWN;
    return format;
}

inline void ConvertLuminance(D3DFORMAT format, unsigned width, unsigned height, unsigned depth,
                             const void* input, unsigned inputRow, unsigned inputSlice,
                             void* output, unsigned outputRow, unsigned outputSlice, bool toNative)
{
    DXGI_FORMAT storage;
    unsigned pcBytes, nativeBytes;
    if (!LuminanceLayout(format,&storage,&pcBytes,&nativeBytes)) return;
    for (unsigned z=0; z<depth; ++z)
        for (unsigned y=0; y<height; ++y)
        {
            const unsigned char* src = static_cast<const unsigned char*>(input)+z*inputSlice+y*inputRow;
            unsigned char* dst = static_cast<unsigned char*>(output)+z*outputSlice+y*outputRow;
            for (unsigned x=0; x<width; ++x)
            {
                if (toNative)
                {
                    if (format == D3DFMT_L16)
                    {
                        std::memcpy(dst,src,2); std::memcpy(dst+2,src,2); std::memcpy(dst+4,src,2);
                        dst[6] = dst[7] = 255;
                    }
                    else
                    {
                        dst[0] = dst[1] = dst[2] = src[0];
                        dst[3] = format == D3DFMT_A8L8 ? src[1] : 255;
                    }
                }
                else
                {
                    dst[0] = src[0];
                    if (pcBytes == 2) dst[1] = format == D3DFMT_L16 ? src[1] : src[3];
                }
                src += toNative ? pcBytes : nativeBytes;
                dst += toNative ? nativeBytes : pcBytes;
            }
        }
}

} }
#endif
