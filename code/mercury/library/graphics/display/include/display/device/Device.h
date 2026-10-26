#ifndef DISPLAY_DEVICE_DEVICE_H
#define DISPLAY_DEVICE_DEVICE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "core\containers\string.h"
#include "core\containers\sortedbinarytree.h"

#include "core/threads/misc.h"

#include "Display/Device/DeviceDefines.h"

#if defined(DEVICE_PCD3D11) // [Blur1PCDX11] Native game device.
#include "Display/Device/D3D11/PCD3D11GameDevice.h"
#elif defined(DEVICE_PS3GCM)
#include "Display/Device/GCM/GCMDevice.h"
#elif defined(DEVICE_PCGL)
#include "Display/Device/GL/PCGLDevice.h"
#elif defined(DEVICE_PS3GL)
#include "Display/Device/GL/PS3GLDevice.h"
#endif

#include "Display/Buffer/VertexBuffer.h"
#include "Display/Buffer/IndexBuffer.h"
#include "Display/Surface/SurfaceUtils.h"
#include "Display/Texture/Texture2DUtils.h"
#include "Display/Texture/Texture3DUtils.h"
#include "Display/Texture/TextureCubeUtils.h"
#include "Display/Shader/VertexShaderUtils.h"
#include "Display/Shader/PixelShaderUtils.h"

namespace Display
{

class Context;

#if defined(DEVICE_PCD3D11)
class C_Device : public D3D11::C_PCD3D11GameDevice
#elif defined(DEVICE_PS3GCM)
class C_Device : public GCM::C_GCMDevice
#elif defined(DEVICE_PCGL)
class C_Device : public GL::C_PCGLDevice
#elif defined(DEVICE_PS3GL)
class C_Device : public GL::C_PS3GLDevice
#endif
{
public:

	C_Device();
	~C_Device();

	// Device management
	bool Create(C_DeviceParams& deviceParams);
	bool CreateVolatile(C_DeviceParams& deviceParams);
	void DestroyVolatile();
	void Destroy();

	// Surface management
	void Destroy(C_Surface* pSurface);

	// Texture 2d management
	void Destroy(C_Texture2D* pTexture);
	
	// Render texture 2d management
	void Destroy(C_RenderTexture2D* pRenderTexture);

	// Render texture cube management
	void Destroy(C_RenderTextureCube* pRenderTexture);

	#if PLATFORM_PS3
	void Destroy(C_RenderBuffer* pRenderBuffer);
	#endif

	// Vertex declaration management
	C_VertexDeclaration* CreateVertexDeclaration(const C_VertexFormat& vertexFormat);
	C_VertexDeclaration* CreateVertexDeclaration(const C_VertexElement* pVertexElements);
	C_VertexDeclaration* CreateVertexDeclaration(const S32 vertexElementCount, const C_VertexElement* pVertexElements);
	
	// Vertex buffer management
	void Destroy(C_VertexBuffer* pVertexBuffer);
	
	// Index buffer management
	void Destroy(C_IndexBuffer* pIndexBuffer);
	
	// Vertex shader management
	void Destroy(C_VertexShader* pVertexShader);
	
	// Pixel shader management
	void Destroy(C_PixelShader* pPixelShader);
	
	// State setting
	void SetTextureAddress(const U32 sampler, const E_TextureAddress address);
	void SetTextureFilter(const U32 sampler, const E_TextureFilter filter);

	// Resource querying
	C_Texture2D* GetTexture2D(const Util::C_String& fileName);
	C_Texture3D* GetTexture3D(const Util::C_String& fileName);
	C_TextureCube* GetTextureCube(const Util::C_String& fileName);

private:

	// Vertex declaration management
	void DestroyVertexDeclarations();

	// Resource management
	void DestroyVertexShaders();
	void DestroyPixelShaders();
	void AssertManagedResourcesAreDestroyed();
};

extern void* g_pDevice;
C_Device* Device();
void Device(C_Device* pDevice);
const bool DeviceExists();

}; // namespace Display

#include "Display/Device/Device.inl"

#endif // DISPLAY_DEVICE_DEVICE_H
