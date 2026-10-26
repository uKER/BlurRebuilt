#include "stdafx.h"
#include "Display/Device/GCM/GCMDeviceBuffer.h"

namespace NewDisplay
{
namespace GCM
{

static bool s_Flipped = true;

static void FlipCallback(U32 head)
{
	s_Flipped = true;
}

C_GCMBuffer::C_GCMBuffer() :
	m_Width(0),
	m_Height(0),
	m_Format(E_Format_Unknown),
	m_ColourPitch(0),
	m_DepthPitch(0),
	m_ColourOffset(0),
	m_DepthOffset(0),
	//m_pColourResource(NULL),
	//m_pDepthResource(NULL),
	//m_pRescColourResource(NULL),
	//m_pRescVertexResource(NULL),
	//m_pRescShaderResource(NULL),
	m_Created(false)
{
}

C_GCMBuffer::~C_GCMBuffer()
{
	/*if (m_pColourResource)
		ResourceManager()->DestroyResource(m_pColourResource);

	if (m_pDepthResource)
		ResourceManager()->DestroyResource(m_pDepthResource);

	if (m_pRescColourResource)
		ResourceManager()->DestroyResource(m_pRescColourResource);

	if (m_pRescVertexResource)
		ResourceManager()->DestroyResource(m_pRescVertexResource);

	if (m_pRescShaderResource)
		ResourceManager()->DestroyResource(m_pRescShaderResource);*/
}

bool C_GCMBuffer::Create(const C_DeviceParams& deviceParams)
{
	BIZ_ASSERT(!m_Created);
	m_Created = true;

	m_Width = deviceParams.m_Width;
	m_Height = deviceParams.m_Height;
	m_Format = deviceParams.m_Format;

	if (!CreateColourSurface())
		return false;
	if (!CreateDepthSurface())
		return false;
	if (!CreateResc())
		return false;

	cellRescSetFlipHandler(FlipCallback);

	m_Surface.colorFormat 			= GCMSurfaceFormat(m_Format);
	m_Surface.colorTarget			= CELL_GCM_SURFACE_TARGET_0;
	
	m_Surface.colorLocation[0]		= CELL_GCM_LOCATION_LOCAL;
	m_Surface.colorLocation[1]		= CELL_GCM_LOCATION_LOCAL;
	m_Surface.colorLocation[2]		= CELL_GCM_LOCATION_LOCAL;
	m_Surface.colorLocation[3]		= CELL_GCM_LOCATION_LOCAL;

	m_Surface.colorOffset[0] 		= m_ColourOffset;
	m_Surface.colorOffset[1] 		= 0;
	m_Surface.colorOffset[2]		= 0;
	m_Surface.colorOffset[3]		= 0;
	
	m_Surface.colorPitch[0] 		= m_ColourPitch;
	m_Surface.colorPitch[1]			= 64;
	m_Surface.colorPitch[2]			= 64;
	m_Surface.colorPitch[3]			= 64; 

	m_Surface.depthFormat 			= CELL_GCM_SURFACE_Z24S8;
	m_Surface.depthLocation			= CELL_GCM_LOCATION_LOCAL;
	m_Surface.depthOffset			= m_DepthOffset;
	m_Surface.depthPitch			= m_DepthPitch;

	m_Surface.type					= CELL_GCM_SURFACE_PITCH;
	m_Surface.antialias				= CELL_GCM_SURFACE_CENTER_1;

	m_Surface.width 				= m_Width;
	m_Surface.height 				= m_Height;
	m_Surface.x 					= 0;
	m_Surface.y 					= 0;

	return true;
}

bool C_GCMBuffer::CreateColourSurface()
{
	/*BIZ_ASSERT(!m_pColourResource);

	m_pColourResource = ResourceManager()->AddResource(E_ResourceType_Surface, g_LocalBuffer);

	U32 colourBits;

	switch (m_Format)
	{
	case E_TextureFormat_A8R8G8B8:
		colourBits = 4;
		break;

	default:
		BIZ_ASSERT_FAIL("Unsupported format");
		break;
	}

	m_ColourPitch = m_Width * colourBits;

	U32 colourSize = m_Height * m_ColourPitch;

	if (!ResourceManager()->CreateResource(m_pColourResource, colourSize, 16))
	{
		LOGFATAL(g_Log, "Failed to create the colour resource\n");

		return false;
	}

	if (cellGcmAddressToOffset(m_pColourResource->m_pBufferAddress, &m_ColourOffset) != CELL_OK)
	{
		LOGFATAL(g_Log, "Failed to set the colour offset\n");

		return false;
	}*/

	return true;
}

bool C_GCMBuffer::CreateDepthSurface()
{
	/*BIZ_ASSERT(!m_pDepthResource);

	m_pDepthResource = ResourceManager()->AddResource(E_ResourceType_Surface, g_LocalBuffer);

	U32 depthBits;

	switch (m_Format)
	{
	case E_TextureFormat_D24S8:
		depthBits = 4;
		break;

	default:
		BIZ_ASSERT_FAIL("Unsupported format");
		break;
	}

	m_DepthPitch = m_Width * depthBits;

	U32 depthSize = m_Height * m_DepthPitch;

	if (!ResourceManager()->CreateResource(m_pDepthResource, depthSize, 65536))
	{
		LOGFATAL(g_Log, "Failed to create the depth resource\n");

		return false;
	}

	if (cellGcmAddressToOffset(m_pDepthResource->m_pBufferAddress, &m_DepthOffset) != CELL_OK)
	{
		LOGFATAL(g_Log, "Failed to set the depth offset\n");

		return false;
	}*/

	return true;
}

bool C_GCMBuffer::CreateResc()
{
	/*CellRescInitConfig config;

	memset(&config, 0, sizeof(CellRescInitConfig));

	config.resourcePolicy	= CELL_RESC_MINIMUM_GPU_LOAD | CELL_RESC_CONSTANT_VRAM;
	config.supportModes		= CELL_RESC_720x480 | CELL_RESC_720x576 | CELL_RESC_1280x720;
	config.dstFormat		= CELL_RESC_SURFACE_A8R8G8B8;
	config.ratioMode		= CELL_RESC_FULLSCREEN;
	config.palTemporalMode	= CELL_RESC_PAL_50;
	config.tileIndex		= 0x7;

	if (cellRescInit(&config) != CELL_OK)
	{
		LOGFATAL(g_Log, "Failed to initialise resc\n");

		return false;
	}

	CellRescBufferMode bufferMode = CELL_RESC_720x480;
	
	if (m_Width == 720 && m_Height == 576)
	{
		bufferMode = CELL_RESC_720x576;
	}
	else if (m_Width == 1280 && m_Height == 720)
	{
		bufferMode = CELL_RESC_1280x720;
	}
	else
	{
		LOGWARN(g_Log, "Unsupported display mode %ix%i using 720x480 instead\n", m_Width, m_Height);
	}

	if (cellRescSetDisplayMode(CELL_RESC_720x576) != CELL_OK)
	{
		LOGFATAL(g_Log, "Failed to set the resc display mode\n");

		return false;
	}

	S32 rescColourSize, rescVertexSize, rescShaderSize;

	if (cellRescGetBufferSize(&rescColourSize, &rescVertexSize, &rescShaderSize) != CELL_OK)
	{
		LOGFATAL(g_Log, "Failed to get the resc buffer size\n");

		return false;
	}

	BIZ_ASSERT(!m_pRescColourResource);

	m_pRescColourResource = ResourceManager()->AddResource(E_ResourceType_Surface, g_LocalBuffer);

	if (!ResourceManager()->CreateResource(m_pRescColourResource, rescColourSize, 65536))
	{
		LOGFATAL(g_Log, "Failed to set the resc colour resource\n");

		return false;
	}

	BIZ_ASSERT(!m_pRescVertexResource);

	m_pRescVertexResource = ResourceManager()->AddResource(E_ResourceType_VertexBuffer, g_LocalBuffer);

	if (!ResourceManager()->CreateResource(m_pRescVertexResource, rescVertexSize, 128))
	{
		LOGFATAL(g_Log, "Failed to set the resc vertex resource\n");

		return false;
	}

	BIZ_ASSERT(!m_pRescShaderResource);

	m_pRescShaderResource = ResourceManager()->AddResource(E_ResourceType_PixelShader, g_LocalBuffer);

	if (!ResourceManager()->CreateResource(m_pRescShaderResource, rescShaderSize, 128))
	{
		LOGFATAL(g_Log, "Failed to set the resc shader resource\n");

		return false;
	}

	if (cellRescSetBufferAddress(m_pRescColourResource->m_pBufferAddress, m_pRescVertexResource->m_pBufferAddress, m_pRescShaderResource->m_pBufferAddress) != CELL_OK)
	{
		LOGFATAL(g_Log, "Failed to set the resc buffer address\n");

		return false;
	}*/

	return true;
}

void C_GCMBuffer::Set()
{
	//cellGcmSetSurface(&m_Surface);
}

void C_GCMBuffer::Wait()
{
	while (!s_Flipped)
	{
		sys_timer_usleep(300);
	}

	s_Flipped = false;
}

void C_GCMBuffer::Flip()
{
	if (cellRescSetConvertAndFlip(0) == CELL_OK)
	{
		//cellGcmFlush();
		cellRescSetWaitFlip();
	}
}

}; // namespace GCM
}; // namespace NewDisplay
