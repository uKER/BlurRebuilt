#include "stdafx.h"
#include "Display/Device/GL/PS3GLDevice.h"

namespace NewDisplay
{
namespace GL
{

C_PS3GLDevice::C_PS3GLDevice() :
	m_pRenderTexture2D(NULL)
{
	m_VertexShaderProfile	= E_VertexShaderProfile_3_0;
	m_PixelShaderProfile	= E_PixelShaderProfile_3_0;
	m_VSync					= false;
	m_glDevice				= NULL;
	m_glContext				= NULL;
}

C_PS3GLDevice::~C_PS3GLDevice()
{
	assert(!m_glDevice && "The ps3gl device has not been destroyed");
	assert(!m_glContext && "The ps3gl context has not been destroyed");
}

bool C_PS3GLDevice::CreateDevice(C_DeviceParams& deviceParams)
{
	m_VSync = deviceParams.m_VSync;

	if (!CreatePSGL(deviceParams))
		return false;
	if (!CreateCG())
		return false;
	if (!C_GLDevice::CreateDevice(deviceParams))
		return false;
	
	InvalidateState();
	DefaultStates();

	Clear(E_Clear_Colour, Maths::RGBA(128, 128, 128, 255));
	Present();

	return true;
}

bool C_PS3GLDevice::CreateDeviceVolatile(C_DeviceParams& deviceParams)
{
	return C_GLDevice::CreateDeviceVolatile(deviceParams);
}

void C_PS3GLDevice::DestroyDeviceVolatile()
{
	C_GLDevice::DestroyDeviceVolatile();
}

void C_PS3GLDevice::DestroyDevice()
{
	C_GLDevice::DestroyDevice();

	DestroyPSGL();
}

bool C_PS3GLDevice::CreatePSGL(C_DeviceParams& deviceParams)
{
	PSGLinitOptions initOptions;
    initOptions.maxSPUs					= deviceParams.m_MaxSPUs;
	initOptions.initializeSPUs			= deviceParams.m_MaxSPUs > 0 ? GL_TRUE : GL_FALSE;
    initOptions.persistentMemorySize	= 0;
	initOptions.transientMemorySize		= 0;
	initOptions.errorConsole			= TRUE;
	initOptions.fifoSize				= 1024 << 10; // [LloydW] Default is 256KB, but drawing a single quad asserts with the current fifo pointer going over the end of the buffer...nice!
	initOptions.hostMemorySize			= deviceParams.m_HostMemorySize;
	initOptions.enable					= PSGL_INIT_MAX_SPUS
										| PSGL_INIT_INITIALIZE_SPUS
										| PSGL_INIT_PERSISTENT_MEMORY_SIZE
										| PSGL_INIT_TRANSIENT_MEMORY_SIZE
										| PSGL_INIT_ERROR_CONSOLE
										| PSGL_INIT_FIFO_SIZE
										| PSGL_INIT_HOST_MEMORY_SIZE;
    
    psglInit(&initOptions);
	
	PSGLdeviceParameters deviceParameters;
	deviceParameters.colorFormat		= GL_ARGB_SCE;
	deviceParameters.depthFormat		= GL_DEPTH_COMPONENT24;
	deviceParameters.multisamplingMode	= GL_MULTISAMPLING_4X_SQUARE_CENTERED_SCE;
	deviceParameters.TVStandard			= PSGL_TV_STANDARD_HD720P;
	deviceParameters.connector			= PSGL_DEVICE_CONNECTOR_HDMI;
	deviceParameters.bufferingMode		= PSGL_BUFFERING_MODE_DOUBLE;
	deviceParameters.width				= deviceParams.m_Width;
	deviceParameters.height				= deviceParams.m_Height;
	deviceParameters.enable				= PSGL_DEVICE_PARAMETERS_COLOR_FORMAT
										| PSGL_DEVICE_PARAMETERS_DEPTH_FORMAT
										| PSGL_DEVICE_PARAMETERS_MULTISAMPLING_MODE
										| PSGL_DEVICE_PARAMETERS_TV_STANDARD
										| PSGL_DEVICE_PARAMETERS_CONNECTOR
										| PSGL_DEVICE_PARAMETERS_BUFFERING_MODE
										| PSGL_DEVICE_PARAMETERS_WIDTH_HEIGHT;

	if (!(m_glDevice = psglCreateDeviceExtended(&deviceParameters)))
	{
		LOGFATAL(g_Log, "Failed to create the ps3gl device\n");
		return false;
	}
	
	if (!(m_glContext = psglCreateContext()))
	{
		LOGFATAL(g_Log, "Failed to create the ps3gl context\n");
		return false;
	}

	psglMakeCurrent(m_glContext, m_glDevice);

	return true;
}

void C_PS3GLDevice::DestroyPSGL()
{
	if (m_glDevice)
	{
		psglDestroyDevice(m_glDevice);
		m_glDevice = NULL;
	}

	if (m_glContext)
	{
		psglDestroyContext(m_glContext);
		m_glContext = NULL;
	}
}

bool C_PS3GLDevice::CreateCG()
{
	assert(!m_cgContext);
	m_cgContext = cgCreateContext();
	
	if (!m_cgContext)
	{
		LOGFATAL(g_Log, "Failed to create the cg context\n");
		return false;
	}

	m_cgLatestProfileVS = cgGLGetLatestProfile(CG_GL_VERTEX); // [Lloyd] Why is this underfined on the ps3?
	m_cgLatestProfilePS = cgGLGetLatestProfile(CG_GL_FRAGMENT); // [Lloyd] Why is this underfined on the ps3?

	m_LatestProfileVS = E_VertexShaderProfile_3_0;
	m_LatestProfilePS = E_PixelShaderProfile_3_0;

	LOGINFO(g_Log, "Hardware supported vertex shader %s and pixel shader %s\n", cgGetProfileString(m_cgLatestProfileVS), cgGetProfileString(m_cgLatestProfilePS));

	return true;
}

void C_PS3GLDevice::Present()
{
	psglSwap();
}

void C_PS3GLDevice::BeginRenderTexture2D(C_RenderTexture2D* pRenderTexture2D)
{
	assert(pRenderTexture2D);
	assert(!m_pRenderTexture2D);
	m_pRenderTexture2D = pRenderTexture2D;
	SetColourSurface(&m_pRenderTexture2D->m_Surface);
}

void C_PS3GLDevice::EndRenderTexture2D()
{
	assert(m_pRenderTexture2D);
	m_pRenderTexture2D = NULL;
}

bool C_PS3GLDevice::AllocMemory(const E_ResourceType type, const U32 size, U8** ppAddress, const S32 systemID)
{
	return true;
}

void C_PS3GLDevice::FreeMemory(const E_ResourceType type, const U32 size, U8* pAddress, const S32 systemID)
{
}

}; // namespace GL
}; // namespace NewDisplay
