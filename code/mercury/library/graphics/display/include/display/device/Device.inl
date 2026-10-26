#ifndef DISPLAY_DEVICE_DEVICE_INL
#define DISPLAY_DEVICE_DEVICE_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE void C_Device::Destroy(C_Surface* pSurface)
{
	DestroySurface(pSurface);
}

INLINE void C_Device::Destroy(C_Texture2D* pTexture)
{
	DestroyTexture2D(pTexture);
}

INLINE void C_Device::Destroy(C_RenderTexture2D* pRenderTexture)
{
	DestroyRenderTexture2D(pRenderTexture);
}

INLINE void C_Device::Destroy(C_RenderTextureCube* pRenderTexture)
{
	DestroyRenderTextureCube(pRenderTexture);
}

#if PLATFORM_PS3
INLINE void C_Device::Destroy(C_RenderBuffer* pRenderBuffer)
{
	DestroyRenderBuffer(pRenderBuffer);
}
#endif

INLINE void C_Device::Destroy(C_VertexBuffer* pVertexBuffer)
{
	DestroyVertexBuffer(pVertexBuffer);
}

INLINE void C_Device::Destroy(C_IndexBuffer* pIndexBuffer)
{
	DestroyIndexBuffer(pIndexBuffer);
}

INLINE void C_Device::Destroy(C_VertexShader* pVertexShader)
{
	DestroyVertexShader(pVertexShader);
}

INLINE void C_Device::Destroy(C_PixelShader* pPixelShader)
{
	DestroyPixelShader(pPixelShader);
}

INLINE void C_Device::SetTextureAddress(const U32 sampler, const E_TextureAddress address)
{
	SetTextureAddressUVW(sampler, address, address, address);
}

INLINE void C_Device::SetTextureFilter(const U32 sampler, const E_TextureFilter filter)
{
	SetTextureFilterMagMinMip(sampler, filter, filter, filter);
}

FORCE_INLINE C_Device* Device()
{
	assert(g_pDevice);
	return (C_Device*)g_pDevice;
}

FORCE_INLINE const bool DeviceExists()
{
	return g_pDevice != NULL;
}

}; // namespace Display

#endif // DISPLAY_DEVICE_DEVICE_INL
