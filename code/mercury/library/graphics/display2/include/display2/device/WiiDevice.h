#ifndef DISPLAY2_WII_DEVICE_H
#define DISPLAY2_WII_DEVICE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display2/Device/BaseDevice.h"

namespace Display2
{

//! C_DeviceParams

class C_DeviceParams
{
public:

	C_DeviceParams();
	~C_DeviceParams();
};

//! C_Device

class C_Device : public C_BaseDevice
{
public:

	C_Device();
	~C_Device();

	void GetParams(C_DeviceParams& params);
	void SetParams(const C_DeviceParams& params);

	U32 IsCreated() const;
	bool Create();
	bool CreateVolatile();
	void DestroyVolatile();
	void Destroy();

	C_Context& GetContext();
	const C_Context& GetContext() const;
	
	C_ContextNoCheck& GetContextNoCheck();
	const C_ContextNoCheck& GetContextNoCheck() const;
	
	C_ContextNoCheckNoTrack& GetContextNoCheckNoTrack();
	const C_ContextNoCheckNoTrack& GetContextNoCheckNoTrack() const;

	S32 GetVertexDeclarationCount() const;
	C_VertexDeclaration* GetVertexDeclaration(const S32 index);
	C_VertexDeclaration* FindVertexDeclaration(const S32 vertexElementCount, const C_VertexElement* pVertexElements);
	
	C_VertexDeclaration* CreateVertexDeclaration(const C_VertexFormat& vertexFormat);
	bool CreateVertexDeclaration(const C_VertexFormat& vertexFormat, C_VertexDeclaration* pVertexDeclaration);
	C_VertexDeclaration* CreateVertexDeclaration(const C_VertexElement* pVertexElements);
	bool CreateVertexDeclaration(const C_VertexElement* pVertexElements, C_VertexDeclaration* pVertexDeclaration);
	
	void DestroyVertexDeclaration(C_VertexDeclaration* pVertexDeclaration);
	void Destroy(C_VertexDeclaration* pVertexDeclaration);
	void DestroyVertexDeclarations();

	bool CreateVertexBuffer(U32 size, U32 flags, C_VertexBuffer* pVertexBuffer, void* pAddress = NULL);
	void DestroyVertexBuffer(C_VertexBuffer* pVertexBuffer);
	void Destroy(C_VertexBuffer* pVertexBuffer);

	bool CreateIndexBuffer(U32 size, U32 flags, E_IndexFormat format, C_IndexBuffer* pIndexBuffer, void* pAddress = NULL);
	void DestroyIndexBuffer(C_IndexBuffer* pIndexBuffer);
	void Destroy(C_IndexBuffer* pIndexBuffer);

	bool CreateVertexShader(U32 size, const U8* pData, U32 flags, C_VertexShader* pVertexShader, void* pAddress = NULL);
	void DestroyVertexShader(C_VertexShader* pVertexShader);
	void Destroy(C_VertexShader* pVertexShader);

	bool CreatePixelShader(U32 size, const U8* pData, U32 flags, C_PixelShader* pPixelShader, void* pAddress = NULL);
	void DestroyPixelShader(C_PixelShader* pPixelShader);
	void Destroy(C_PixelShader* pPixelShader);

	bool CreateColourSurface(U32 width, U32 height, U32 flags, E_SurfaceFormat format, E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface, void* pAddress = NULL, U32 addressSize = 0);
	bool CreateDepthStencilSurface(U32 width, U32 height, U32 flags, E_SurfaceFormat format, E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface, void* pAddress = NULL, U32 addressSize = 0);
	void DestroySurface(C_Surface* pSurface);
	void Destroy(C_Surface* pSurface);

	bool CreateTexture2D(U32 width, U32 height, U32 levels, U32 flags, E_TextureFormat format, C_Texture* pTexture, void* pAddress = NULL, U32 addressSize = 0);
	bool CreateTexture3D(const U32 width, const U32 height, const U32 depth, const U32 levels, U32 flags, const E_TextureFormat format, C_Texture* pTexture, void* pAddress = NULL, U32 addressSize = 0);
	bool CreateTextureCube(const U32 width, const U32 levels, U32 flags, const E_TextureFormat format, C_Texture* pTexture, void* pAddress = NULL, U32 addressSize = 0);
	void DestroyTexture(C_Texture* pTexture);
	void Destroy(C_Texture* pTexture);

private:

	C_Context					m_Context;
	C_ContextNoCheck			m_ContextNoCheck;
	C_ContextNoCheckNoTrack		m_ContextNoCheckNoTrack;
	C_ContextState				m_ContextState;
};

//! C_DeviceManagerParams

class C_DeviceManagerParams
{
public:
	
	C_DeviceManagerParams();
	~C_DeviceManagerParams();
};

//! C_DeviceManager

class C_DeviceManager
{
public:

	C_DeviceManager();
	~C_DeviceManager();

	bool IsCreated();
	bool Create(const C_DeviceManagerParams& params);
	void Destroy();

	void SetupDefaultDeviceParams(C_DeviceParams& params);

	bool GetVideoMode(C_VideoMode* pVideoMode);
};

} // namespace Display2

#include "Display2/Device/WiiDevice.inl"

#endif // DISPLAY2_WII_DEVICE_H
