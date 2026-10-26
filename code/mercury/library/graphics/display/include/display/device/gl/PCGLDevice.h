#ifndef DISPLAY_PCGLDEVICE_H
#define DISPLAY_PCGLDEVICE_H

#pragma once

#include "Display/Device/GL/GLDevice.h"

namespace NewDisplay
{
namespace GL
{

class C_PCGLDevice : public C_GLDevice
{
public:

	C_PCGLDevice();
	~C_PCGLDevice();

	/**************************************************************************************************
	- Public cross-platform functions.
	- Safe to use by game as these functions are guaranteed to be cross-platform.
	- Basically anything here must be the interface.
	**************************************************************************************************/
	
	void Present();

	void SetPixelShaderConstantB(const U32 startRegister, const BOOL* pData, const U32 registerCount);
	void SetPixelShaderConstantI(const U32 startRegister, const S32* pData, const U32 registerCount);
	void SetPixelShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount);
	void SetVertexShaderConstantB(const U32 startRegister, const BOOL* pData, const U32 registerCount);
	void SetVertexShaderConstantI(const U32 startRegister, const S32* pData, const U32 registerCount);
	void SetVertexShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount);

	/**************************************************************************************************
	- Public platform specific functions.
	- Not safe to use by game as these functions are not guaranteed to be cross-platform.
	- Wrap functions calls with define DEVICE_PCGL.
	- Basically anything here dont put in the interface.
	**************************************************************************************************/

	// Memory management
	bool AllocMemory(const E_ResourceType type, const U32 size, U8** ppAddress, const S32 systemID = -1);
	void FreeMemory(const E_ResourceType type, const U32 size, U8* pAddress, const S32 systemID = -1);
	
protected:

	/**************************************************************************************************
	- Protected cross-platform functions.
	- Basically anything here must be the interface.
	**************************************************************************************************/

	// Device management
	bool CreateDevice(C_DeviceParams& deviceParams);
	bool CreateDeviceVolatile(C_DeviceParams& deviceParams);
	void DestroyDeviceVolatile();
	void DestroyDevice();

	/**************************************************************************************************
	- Protected platform specific functions.
	- Basically anything here dont put in the interface.
	**************************************************************************************************/

	// TODO: Add functions here.

private:

	/**************************************************************************************************
	- Private cross-platform functions.
	- Basically anything here must be the interface.
	**************************************************************************************************/

	// TODO: Add functions here.

	/**************************************************************************************************
	- Private platform specific functions.
	- Basically anything here dont put in the interface.
	**************************************************************************************************/

	// GL management
	bool CreateGL();

	// CG management
	bool CreateCG();
	
protected:

	bool	m_VSync;
	HDC		m_hDC;
	HGLRC	m_hGLRC;
};

#include "PCGLDevice.inl"

}; // namespace GL
}; // namespace NewDisplay

#endif // DISPLAY_PCGLDEVICE_H
