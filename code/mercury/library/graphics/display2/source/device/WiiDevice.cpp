#include "StdAfx.h"
#include "Display2/Device/WiiDevice.h"

namespace Display2
{

// C_DeviceParams

C_DeviceParams::C_DeviceParams()
{
}

C_DeviceParams::~C_DeviceParams()
{
}

// C_Device

C_Device::C_Device()
{
}

C_Device::~C_Device()
{
}

void C_Device::GetParams(C_DeviceParams& params)
{
}

void C_Device::SetParams(const C_DeviceParams& params)
{
}

bool C_Device::Create()
{
	return true;
}

bool C_Device::CreateVolatile()
{
	return true;
}

void C_Device::DestroyVolatile()
{
}

void C_Device::Destroy()
{
}

C_VertexDeclaration* C_Device::FindVertexDeclaration(const S32 vertexElementCount, const C_VertexElement* pVertexElements)
{
	return NULL;
}

C_VertexDeclaration* C_Device::CreateVertexDeclaration(const C_VertexFormat& vertexFormat)
{
	return NULL;
}

bool C_Device::CreateVertexDeclaration(const C_VertexFormat& vertexFormat, C_VertexDeclaration* pVertexDeclaration)
{
	return false;
}

C_VertexDeclaration* C_Device::CreateVertexDeclaration(const C_VertexElement* pVertexElements)
{
	return NULL;
}

bool C_Device::CreateVertexDeclaration(const C_VertexElement* pVertexElements, C_VertexDeclaration* pVertexDeclaration)
{
	return false;
}

void C_Device::DestroyVertexDeclaration(C_VertexDeclaration* pVertexDeclaration)
{
}

void C_Device::DestroyVertexDeclarations()
{
}

bool C_Device::CreateVertexBuffer(U32 size, U32 flags, C_VertexBuffer* pVertexBuffer, void* pAddress)
{
	return false;
}

void C_Device::DestroyVertexBuffer(C_VertexBuffer* pVertexBuffer)
{
}

bool C_Device::CreateIndexBuffer(U32 size, U32 flags, E_IndexFormat format, C_IndexBuffer* pIndexBuffer, void* pAddress)
{
	return false;
}

void C_Device::DestroyIndexBuffer(C_IndexBuffer* pIndexBuffer)
{
}

bool C_Device::CreateVertexShader(U32 size, const U8* pData, U32 flags, C_VertexShader* pVertexShader, void* pAddress)
{
	return false;
}

void C_Device::DestroyVertexShader(C_VertexShader* pVertexShader)
{
}

bool C_Device::CreatePixelShader(U32 size, const U8* pData, U32 flags, C_PixelShader* pPixelShader, void* pAddress)
{
	return false;
}

void C_Device::DestroyPixelShader(C_PixelShader* pPixelShader)
{
}

bool C_Device::CreateColourSurface(U32 width, U32 height, U32 flags, E_SurfaceFormat format, E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface, void* pAddress, U32 addressSize)
{
	return false;
}

bool C_Device::CreateDepthStencilSurface(U32 width, U32 height, U32 flags, E_SurfaceFormat format, E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface, void* pAddress, U32 addressSize)
{
	return false;
}

void C_Device::DestroySurface(C_Surface* pSurface)
{
}

bool C_Device::CreateTexture2D(U32 width, U32 height, U32 levels, U32 flags, E_TextureFormat format, C_Texture* pTexture, void* pAddress, U32 addressSize)
{
	return false;
}

bool C_Device::CreateTexture3D(const U32 width, const U32 height, const U32 depth, const U32 levels, U32 flags, const E_TextureFormat format, C_Texture* pTexture, void* pAddress, U32 addressSize)
{
	return false;
}

bool C_Device::CreateTextureCube(const U32 width, const U32 levels, U32 flags, const E_TextureFormat format, C_Texture* pTexture, void* pAddress, U32 addressSize)
{
	return false;
}

void C_Device::DestroyTexture(C_Texture* pTexture)
{
}

// C_DeviceManagerParams

C_DeviceManagerParams::C_DeviceManagerParams()
{
}

C_DeviceManagerParams::~C_DeviceManagerParams()
{
}

// C_DeviceManager

C_DeviceManager::C_DeviceManager()
{
}

C_DeviceManager::~C_DeviceManager()
{
}

bool C_DeviceManager::Create(const C_DeviceManagerParams& params)
{
	return false;
}

void C_DeviceManager::Destroy()
{
}

void C_DeviceManager::SetupDefaultDeviceParams(C_DeviceParams& params)
{
}

bool C_DeviceManager::GetVideoMode(C_VideoMode* pVideoMode)
{
	return false;
}

} // namespace Display2
