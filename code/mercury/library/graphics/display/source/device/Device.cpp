#include "stdafx.h"
#include "Display/Device/Device.h"
#include <system/memory.h>

namespace Display
{

const Maths::RGBA BLACK		= Maths::RGBA(000, 000, 000, 255);
const Maths::RGBA WHITE		= Maths::RGBA(255, 255, 255, 255);
const Maths::RGBA GREY		= Maths::RGBA(128, 128, 128, 255);
const Maths::RGBA RED		= Maths::RGBA(255, 000, 000, 255);
const Maths::RGBA GREEN		= Maths::RGBA(000, 255, 000, 255);
const Maths::RGBA BLUE		= Maths::RGBA(000, 000, 255, 255);
const Maths::RGBA YELLOW	= Maths::RGBA(255, 255, 128, 255);
const Maths::RGBA PINK		= Maths::RGBA(255, 128, 255, 255);

C_Device::C_Device()
{
	BIZ_ASSERT(!g_pDevice);
	g_pDevice = this;
}

C_Device::~C_Device()
{
	BIZ_ASSERT(g_pDevice == this);
	g_pDevice = NULL;
}

bool C_Device::Create(C_DeviceParams& deviceParams)
{
	RESOURCE_MEMORY_ID("Device");

	BIZ_ASSERT(!m_Created);
	
	m_DeviceParams = deviceParams;
	
	if(!CreateDevice())
	{
		return false;
	}

	deviceParams = m_DeviceParams;

	m_Created = true;

	#ifdef PLATFORM_PC
	if(BeginScene())
	{
		Clear(E_Clear_Colour, BLACK);
		EndScene();
	}
	Present();
	#endif

	return true;
}

bool C_Device::CreateVolatile(C_DeviceParams& deviceParams)
{
	RESOURCE_MEMORY_ID("Device");

	BIZ_ASSERT(m_Created);

	m_DeviceParams = deviceParams;

	if(!CreateDeviceVolatile())
	{
		return false;
	}

	deviceParams = m_DeviceParams;

	return true;
}

void C_Device::DestroyVolatile()
{
	DestroyDeviceVolatile();
}

void C_Device::Destroy()
{
	DestroyVertexDeclarations();
	DestroyVertexShaders();
	DestroyPixelShaders();
	
	AssertManagedResourcesAreDestroyed();

	DestroyDevice();

	m_Created = false;
}

void C_Device::DestroyVertexShaders()
{
}

void C_Device::DestroyPixelShaders()
{
}

void C_Device::AssertManagedResourcesAreDestroyed()
{
}

C_VertexDeclaration* C_Device::CreateVertexDeclaration(const C_VertexElement* pVertexElements)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.vertexdecls");
	
	BIZ_ASSERT(pVertexElements);

	U32 vertexElementCount = 0;
	const C_VertexElement* pVertexElement = pVertexElements;
	#if defined(DEVICE_PCD3D11) // [Blur1PCDX11] PC vertex declarations end with stream 0xff.
	while(pVertexElement->GetStream() != 0xFF)
	#else
	while(pVertexElement->GetStream() != E_Stream_Count)
	#endif
	{
		vertexElementCount++;

		if(vertexElementCount > 100) // sanity check
		{
			LOGFATAL(g_Log, "Too many vertex elements created. Possible infinite loop here. Make sure array is null terminated.");
			return NULL;
		}

		pVertexElement++;
	}
	
	BIZ_ASSERT(vertexElementCount > 0);
	C_VertexDeclaration* pDeclaration = InitVertexDeclaration(vertexElementCount, pVertexElements);

	MEMORY_TRACKER_POP("graphics.display.device.vertexdecls");

	return pDeclaration;
}

C_VertexDeclaration* C_Device::CreateVertexDeclaration(const C_VertexFormat& vertexFormat)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.vertexdecls");

	BIZ_ASSERT(vertexFormat.m_ElementCount > 0);
	C_VertexDeclaration* pDeclaration = InitVertexDeclaration(vertexFormat.m_ElementCount, &vertexFormat.m_Elements[0]);
	
	MEMORY_TRACKER_POP("graphics.display.device.vertexdecls");

	return pDeclaration;
}

C_VertexDeclaration* C_Device::CreateVertexDeclaration(const S32 vertexElementCount, const C_VertexElement* pVertexElements)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.vertexdecls");
	
	BIZ_ASSERT(vertexElementCount > 0);
	C_VertexDeclaration* pDeclaration = InitVertexDeclaration(vertexElementCount, pVertexElements);
	
	MEMORY_TRACKER_POP("graphics.display.device.vertexdecls");

	return pDeclaration;
}

void C_Device::DestroyVertexDeclarations()
{
	m_Display2.DestroyVertexDeclarations();
}

C_Texture2D* C_Device::GetTexture2D(const Util::C_String& fileName)
{
	return NULL;
}

C_Texture3D* C_Device::GetTexture3D(const Util::C_String& fileName)
{
	return NULL;
}

C_TextureCube* C_Device::GetTextureCube(const Util::C_String& fileName)
{
	return NULL;
}

void* g_pDevice = NULL;

void Device(C_Device* pDevice)
{
	g_pDevice = (C_Device*)pDevice;
}

}; // namespace Display

// I AM A TEST! 3
