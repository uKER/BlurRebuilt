#include "stdafx.h"
#include "Display/Device/GL/PCGLDevice.h"

namespace NewDisplay
{
namespace GL
{

 C_PCGLDevice::C_PCGLDevice()
{
	m_VSync	= false;
	m_hDC	= NULL;
	m_hGLRC	= NULL;
}

C_PCGLDevice::~C_PCGLDevice()
{
	wglDeleteContext(m_hGLRC);
}

bool C_PCGLDevice::CreateDevice(C_DeviceParams& deviceParams)
{
	m_hDC = deviceParams.m_hDC;
	m_VSync = deviceParams.m_VSync;

	if (!CreateGL())
		return false;
	if (!CreateCG())
		return false;

	if (!GLInitialiseMultitexture())
		return false;
	if (!GLInitialiseTextureCompression())
		return false;
	if (!GLInitialiseVertexBuffer())
		return false;
	if (!GLInitialiseBlendMinMax())
		return false;

	if (!C_GLDevice::CreateDevice(deviceParams))
		return false;
	
	InvalidateState();
	DefaultStates();

	Clear(E_Clear_Colour, Maths::RGBA(128, 128, 128, 255));
	Present();

	return true;
}

bool C_PCGLDevice::CreateDeviceVolatile(C_DeviceParams& deviceParams)
{
	return C_GLDevice::CreateDeviceVolatile(deviceParams);
}

void C_PCGLDevice::DestroyDeviceVolatile()
{
	C_GLDevice::DestroyDeviceVolatile();
}

void C_PCGLDevice::DestroyDevice()
{
	C_GLDevice::DestroyDevice();
}

bool C_PCGLDevice::CreateGL()
{
	PIXELFORMATDESCRIPTOR descriptor = 
	{ 
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0,
		0,
		0,
		0,
		0,
		0,
		8,
		0,
		0,
		0,
		0,
		0,
		0,
		24,
		8,
		0,
		PFD_MAIN_PLANE,
		0,
		0,
		0,
		0
	}; 

	S8 format = ChoosePixelFormat(m_hDC, &descriptor);

	if (!format)
	{
		LOGERROR(g_Log, "Choosing the device pixel format\n");
		return false;
	}

	if (!SetPixelFormat(m_hDC, format, &descriptor))
	{
		LOGERROR(g_Log, "Setting the device pixel format\n");
		return false;
	}

	if (!DescribePixelFormat(m_hDC, format, sizeof(PIXELFORMATDESCRIPTOR), &descriptor))
	{
		LOGERROR(g_Log, "Describing the device pixel format\n");
		return false;
	}

	m_hGLRC = wglCreateContext(m_hDC);

	if (!m_hGLRC)
	{
		LOGERROR(g_Log, "Creating the device opengl render context\n");
		return false;
	}

	wglMakeCurrent(m_hDC, m_hGLRC);

	return true;
}

bool C_PCGLDevice::CreateCG()
{
	assert(!m_cgContext);
	m_cgContext = cgCreateContext();
	
	if (!m_cgContext)
	{
		LOGFATAL(g_Log, "Failed to create the cg context\n");
		return false;
	}

	//m_cgLatestProfileVS = cgGLGetLatestProfile(CG_GL_VERTEX); // [Lloyd] Why is this underfined on the PC?
	//m_cgLatestProfilePS = cgGLGetLatestProfile(CG_GL_FRAGMENT); // [Lloyd] Why is this underfined on the PC?

	m_cgLatestProfileVS = CG_PROFILE_VP30;
	m_cgLatestProfilePS = CG_PROFILE_FP30;

	m_LatestProfileVS = E_VertexShaderProfile_3_0;
	m_LatestProfilePS = E_PixelShaderProfile_3_0;

	LOGINFO(g_Log, "Hardware supported vertex shader %s and pixel shader %s\n", cgGetProfileString(m_cgLatestProfileVS), cgGetProfileString(m_cgLatestProfilePS));

	return true;
}

void C_PCGLDevice::Present()
{
	SwapBuffers(m_hDC);
}

bool C_PCGLDevice::AllocMemory(const E_ResourceType type, const U32 size, U8** ppAddress, const S32 systemID)
{
	return true;
}

void C_PCGLDevice::FreeMemory(const E_ResourceType type, const U32 size, U8* pAddress, const S32 systemID)
{
}

}; // namespace GL
}; // namespace NewDisplay
