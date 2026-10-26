#include "stdafx.h"
#include "Display/Device/GCM/GCMDevice.h"
#include "Display/Device/GCM/GCMDeviceLabels.h"
#include "Display/Manager/GCM/GCMDebug.h"
#include "Display/Utils/GCM/GCMMatrix.h"
#include "Display/Buffer/GCM/GCMUserFifo.h"
#include "Display/Device/DeviceMarkers.h"
#include "Display/Texture/Texture2DUtils.h"
#include "Display/Texture/Texture3DUtils.h"
#include "Display/Texture/TextureCubeUtils.h"
#include "Display/Shaders/Constants.h"
#include "Display/Buffer/VertexDeclarations.h"
#include "Display/Memory/Memory.h"
#include "Display/Render/Render.h"
#include "Display/Shader/PixelShaders.h"
#include "Display/Shader/PixelShaderUtils.h"
#include "Display/Shader/ShaderUtils.h"
#include "Display/Shader/VertexShaders.h"
#include "Display/Shader/VertexShaderUtils.h"
#include "Display/Utils/Shader.h"
#include "Display/Timing/Timings.h"
#include "Display/DisplayToDisplay2.h"
#include <Display2/Debug/Debug.h>
#include <System/Memory/MemoryTracker.h>

#ifdef USE_GCM_P3X
#include "Display/Utils/GCM/libp3x.h"
#endif

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

#ifndef CFG_GOLD
//#define DEVICE_INFO
#endif

EXTERN_SHADER(psZCull);
EXTERN_SHADER(vsZCull);

bool g_CrashGPU = false;
Debug::Bool g_debugDisplayCrashGPU							("Graphics.Display.CrashGPU",									false);

Debug::Bool g_debugDisplayDownsampleColAndDep				("Graphics.Display.DownsampleColAndDep",						true);
Debug::Bool g_debugDisplayDownsampleFast					("Graphics.Display.DownsampleFast",								true);
Debug::Bool g_debugDisplayDownsampleTransferSurface			("Graphics.Display.DownsampleTransferSurface",					true);

Debug::Bool g_debugDisplayRestoreZcull						("Graphics.Display.RestoreZcull",								false);

//#define DEBUG_SHOW_CLIP_TRANSFORM
#ifdef DEBUG_SHOW_CLIP_TRANSFORM
Debug::Bool	g_debugDisplayDebugShowClipTransform			("Graphics.Display.Debug.ShowClipTransform",					false);
#endif
Debug::Bool g_debugDisplayDeviceUseVertexShaderCache		("Graphics.Display.Device.Gcm.UseVertexShaderCache",			false);

Debug::Int 	g_debugDisplayTextureMaxAniso					("Graphics.Display.Texture.MaxAniso",							-1, Debug::Range(-1, 7));
Debug::Float g_debugDepthBiasScaleFactor					("Graphics.Display.Device.DepthBiasScale",						-300000.0f);

Debug::Bool	g_debugDisplayUpdateResolution					("Graphics.Display.Resolution.Update",							false);

Debug::Bool	g_debugDisplayWindowsOnly						("Graphics.Display.Resolution.WindowsOnly",						true);

Debug::Int 	g_debugDisplayZcullIndex						("Graphics.Display.Resolution.ZcullIndex",						0, Debug::Range(0, 7));

Debug::Bool	g_debugDisplayBackBufferTiling					("Graphics.Display.Resolution.BackBufferTiling",					true);
Debug::Bool	g_debugDisplayFrontBufferTiling					("Graphics.Display.Resolution.FrontBufferTiling",				true);
Debug::Bool	g_debugDisplayWindowBufferTiling				("Graphics.Display.Resolution.WindowBufferTiling",				true);

Debug::Int 	g_debugDisplayFrontBufferTileBank				("Graphics.Display.Resolution.FrontBufferTileBank",				2, Debug::Range(0, 3));
Debug::Int 	g_debugDisplayBackBufferColourTileBank			("Graphics.Display.Resolution.BackBufferColourTileBank",		0, Debug::Range(0, 3));
Debug::Int 	g_debugDisplayBackBufferDepthTileBank			("Graphics.Display.Resolution.BackBufferDepthTileBank",			1, Debug::Range(0, 3));
Debug::Int 	g_debugDisplayWindowBufferColourTileBank		("Graphics.Display.Resolution.WindowBufferColourTileBank",		0, Debug::Range(0, 3));
Debug::Int 	g_debugDisplayWindowBufferDepthTileBank			("Graphics.Display.Resolution.WindowBufferDepthTileBank",		1, Debug::Range(0, 3));

Debug::Bool	g_debugDisplayColourDownsample					("Graphics.Display.Downsample", true);
Debug::Float g_debugDisplayColourDownsampleScale			("Graphics.Display.DownsampleScale", 1.0f, Debug::Range(-2.0f, 2.0f));

Debug::Bool	g_debugDisplayCaptureFrame						("Graphics.Display.CaptureFrame", false);

Debug::Int g_debugDisplayColourRegisterCount				("Graphics.Display.ColourRegisterCount", 0, Debug::Range(0, 48));
Debug::Int g_debugDisplayDepthRegisterCount					("Graphics.Display.DepthRegisterCount", 0, Debug::Range(0, 48));

Debug::Bool g_debugZcullEnable								("Graphics.Display.ZcullEnable", true);
Debug::Bool g_debugScullEnable								("Graphics.Display.ScullEnable", true);

Debug::Int	g_iOverrideZCullForward							("Graphics.Display.ZCullForwardOverride", 0);
Debug::Int	g_iOverrideZCullBack							("Graphics.Display.ZCullBackwardOverride", 0);

Debug::Bool g_ForceStencilOff								("Graphics.Display.ForceStencilOff", false);

#if defined(CELL_GCM_DEBUG)
Debug::Bool	g_debugDisplayEnableGCMDebug					("Graphics.Display.EnableGCMDebug", true);
#endif

#if TRACK_DISPLAY2_COMMAND_BUFFERS
Debug::Bool	g_debugDisplayCrashGpuOnSyncCpuGpuHang			("Graphics.Display.CrashGpuOnSyncCpuGpuHang", false);
#endif

enum E_CELL_GCM_COMPMODE
{
	E_CELL_GCM_COMPMODE_DISABLED							= CELL_GCM_COMPMODE_DISABLED,
	E_CELL_GCM_COMPMODE_C32_2X1								= CELL_GCM_COMPMODE_C32_2X1,
	E_CELL_GCM_COMPMODE_C32_2X2								= CELL_GCM_COMPMODE_C32_2X2,
	E_CELL_GCM_COMPMODE_Z32_SEPSTENCIL						= CELL_GCM_COMPMODE_Z32_SEPSTENCIL,
	E_CELL_GCM_COMPMODE_Z32_SEPSTENCIL_REG					= CELL_GCM_COMPMODE_Z32_SEPSTENCIL_REG,
	E_CELL_GCM_COMPMODE_Z32_SEPSTENCIL_REGULAR				= CELL_GCM_COMPMODE_Z32_SEPSTENCIL_REGULAR,
	E_CELL_GCM_COMPMODE_Z32_SEPSTENCIL_DIAGONAL				= CELL_GCM_COMPMODE_Z32_SEPSTENCIL_DIAGONAL,
	E_CELL_GCM_COMPMODE_Z32_SEPSTENCIL_ROTATED				= CELL_GCM_COMPMODE_Z32_SEPSTENCIL_ROTATED,
	E_CELL_GCM_COMPMODE_OFF,
};

DEBUG_ENUM(E_CELL_GCM_COMPMODE)
	DE_OPTION(E_CELL_GCM_COMPMODE_OFF,						"OFF")
	DE_OPTION(E_CELL_GCM_COMPMODE_DISABLED,					"DISABLED")
	DE_OPTION(E_CELL_GCM_COMPMODE_C32_2X1,					"C32_2X1")
	DE_OPTION(E_CELL_GCM_COMPMODE_C32_2X2,					"C32_2X2")
	DE_OPTION(E_CELL_GCM_COMPMODE_Z32_SEPSTENCIL,			"Z32")
	DE_OPTION(E_CELL_GCM_COMPMODE_Z32_SEPSTENCIL_REG,		"Z32_REG")
	DE_OPTION(E_CELL_GCM_COMPMODE_Z32_SEPSTENCIL_REGULAR,	"Z32_REGULAR")
	DE_OPTION(E_CELL_GCM_COMPMODE_Z32_SEPSTENCIL_DIAGONAL,	"Z32_DIAGONAL")
	DE_OPTION(E_CELL_GCM_COMPMODE_Z32_SEPSTENCIL_ROTATED,	"Z32_ROTATED")
END_DEBUG_ENUM

Debug::Enum<E_CELL_GCM_COMPMODE> g_debugDisplayFrontBufferColourComp			("Graphics.Display.Resolution.FrontBufferColourComp",		E_CELL_GCM_COMPMODE_OFF);
Debug::Enum<E_CELL_GCM_COMPMODE> g_debugDisplayBackBufferColourComp				("Graphics.Display.Resolution.BackBufferColourComp",			E_CELL_GCM_COMPMODE_OFF);
Debug::Enum<E_CELL_GCM_COMPMODE> g_debugDisplayBackBufferDepthStencilComp		("Graphics.Display.Resolution.BackBufferDepthStencilComp",	E_CELL_GCM_COMPMODE_OFF);
Debug::Enum<E_CELL_GCM_COMPMODE> g_debugDisplayWindowBufferColourComp			("Graphics.Display.Resolution.WindowBufferColourComp",		E_CELL_GCM_COMPMODE_OFF);
Debug::Enum<E_CELL_GCM_COMPMODE> g_debugDisplayWindowBufferDepthStencilComp		("Graphics.Display.Resolution.WindowBufferDepthStencilComp",	E_CELL_GCM_COMPMODE_OFF);

enum E_RestoreWindow
{
	E_RestoreWindow_Default = 0,
	E_RestoreWindow_Quincunx,
	E_RestoreWindow_QuincunxAlt,
	E_RestoreWindow_QuincunxShifted,
	E_RestoreWindow_User,
};

DEBUG_ENUM(E_RestoreWindow)
	DE_OPTION(E_RestoreWindow_User,							"User")
	DE_OPTION(E_RestoreWindow_Default,						"Default")
	DE_OPTION(E_RestoreWindow_Quincunx,						"Quincunx")
	DE_OPTION(E_RestoreWindow_QuincunxAlt,					"QuincunxAlt")
	DE_OPTION(E_RestoreWindow_QuincunxShifted,				"QuincunxShifted")
END_DEBUG_ENUM

Debug::Enum<E_RestoreWindow> g_debugDisplayRestoreWindow	("Graphics.Display.RestoreWindow",	E_RestoreWindow_User);

Debug::Float g_debugDisplayTexelShift						("Graphics.Display.TexelShift",		0.0f, Debug::Range(-1.0f, 1.0f));

Debug::Float g_debugDisplayIdleSafeTime						("Graphics.Display.IdleSafeTime",	2.0f);
Debug::Float g_debugDisplayFrameOutTime						("Graphics.Display.FrameOutTime",	2.0f);

#ifndef CFG_GOLD
Debug::Bool g_debugDisplaySleepMainThread					("Graphics.Display.SleepMainThread",	true);
#endif

#if !CFG_GOLD
Debug::Bool g_debugDisplayPrintFrameTimes					("Graphics.Display.PrintFrameTimes", false);
#endif

#ifdef DONT_INLINE_GCM_DEVICE_INL
#undef INLINE
#define INLINE
#include "Display/Device/GCM/GCMDevice.inl"
#undef INLINE
#define INLINE inline
#endif

#ifndef USE_GCM_USER_FIFO
#define USE_DEFAULT_COMMAND_BUFFER
#endif

#ifdef DEVICE_DEBUG
Debug::Bool g_debugShowBeginVerticesFailing					("Graphics.Display.ShowBeginVerticesFailing",			false);
bool g_BeginVerticesFailingCheck = false;
#endif

#ifndef CFG_GOLD
Debug::Bool g_debugShowIdleTime								("Graphics.Display.ShowIdleTime",						false);
#endif

#if !CFG_GOLD

Debug::Bool	g_Tiled_Override					("Graphics.Render.Resolution.Tiled_Override",					false);

//Debug::Int	g_Tiled_Index_Front0_Color			("Graphics.Render.Resolution.Tiled_Index_Front0_Color",			0,		Debug::Range(0, 14));
//Debug::Int	g_Tiled_Index_Front1_Color			("Graphics.Render.Resolution.Tiled_Index_Front1_Color",			1,		Debug::Range(0, 14));
//Debug::Int	g_Tiled_Index_Back_Color			("Graphics.Render.Resolution.Tiled_Index_Back_Color",			2,		Debug::Range(0, 14));
//Debug::Int	g_Tiled_Index_Back_Depth			("Graphics.Render.Resolution.Tiled_Index_Back_Depth",			3,		Debug::Range(0, 14));
//Debug::Int	g_Tiled_Index_Window_Color			("Graphics.Render.Resolution.Tiled_Index_Window_Color",		4,		Debug::Range(0, 14));
//Debug::Int	g_Tiled_Index_Window_Depth			("Graphics.Render.Resolution.Tiled_Index_Window_Depth",		5,		Debug::Range(0, 14));

//Debug::Int	g_Tiled_Base_Front0_Color			("Graphics.Render.Resolution.Tiled_Base_Front0_Color",			0,		Debug::Range(0, 2047));
//Debug::Int	g_Tiled_Base_Front1_Color			("Graphics.Render.Resolution.Tiled_Base_Front1_Color",			60,		Debug::Range(0, 2047));
//Debug::Int	g_Tiled_Base_Back_Color				("Graphics.Render.Resolution.Tiled_Base_Back_Color",			120,	Debug::Range(0, 2047));
//Debug::Int	g_Tiled_Base_Back_Depth				("Graphics.Render.Resolution.Tiled_Base_Back_Depth",			180,	Debug::Range(0, 2047));
//Debug::Int	g_Tiled_Base_Window_Color			("Graphics.Render.Resolution.Tiled_Base_Window_Color",			240,	Debug::Range(0, 2047));
//Debug::Int	g_Tiled_Base_Window_Depth			("Graphics.Render.Resolution.Tiled_Base_Window_Depth",			360,	Debug::Range(0, 2047));

//Debug::Int	g_Tiled_Bank_Front0_Color			("Graphics.Render.Resolution.Tiled_Bank_Front0_Color",			2,		Debug::Range(0, 3));
//Debug::Int	g_Tiled_Bank_Front1_Color			("Graphics.Render.Resolution.Tiled_Bank_Front1_Color",			2,		Debug::Range(0, 3));
//Debug::Int	g_Tiled_Bank_Back_Color				("Graphics.Render.Resolution.Tiled_Bank_Back_Color",			0,		Debug::Range(0, 3));
//Debug::Int	g_Tiled_Bank_Back_Depth				("Graphics.Render.Resolution.Tiled_Bank_Back_Depth",			1,		Debug::Range(0, 3));
//Debug::Int	g_Tiled_Bank_Window_Color			("Graphics.Render.Resolution.Tiled_Bank_Window_Color",			0,		Debug::Range(0, 3));
//Debug::Int	g_Tiled_Bank_Window_Depth			("Graphics.Render.Resolution.Tiled_Bank_Window_Depth",			1,		Debug::Range(0, 3));

Debug::Bool	g_Zcull_Override					("Graphics.Render.Resolution.Zcull_Override",					false);

Debug::Bool	g_Zcull_Enable_Back_Depth			("Graphics.Render.Resolution.Zcull_Enable_Back_Depth",			true);
Debug::Bool	g_Zcull_Enable_Window_Depth			("Graphics.Render.Resolution.Zcull_Enable_Window_Depth",		true);

//Debug::Int	g_Zcull_Index_Back_Depth			("Graphics.Render.Resolution.Zcull_Index_Back_Depth",			0,		Debug::Range(0, 7));
//Debug::Int	g_Zcull_Index_Window_Depth			("Graphics.Render.Resolution.Zcull_Index_Window_Depth",		1,		Debug::Range(0, 7));

#endif

Debug::Bool g_DisableAA("Graphics.Render.DisableAA", false);
Debug::Bool g_AllowDisableAA("Graphics.Render.AllowDisableAA", true);

Debug::Bool g_UseNewSync("Graphics.Render.UseNewSync", true);

#if 0
void DumpDisplaySurface(Display::C_Surface* pSurface)
{
	if(pSurface)
	{
		LOGINFO(Display::g_Log, "***** SURFACE ***** width %i height %i location %i offset 0x%.8x (%i) address 0x%.8x\n",
			pSurface->GetWidth(),
			pSurface->GetHeight(),
			Display2::FlagsToLocation(pSurface->GetFlags()),
			pSurface->GetOffset(),
			pSurface->GetOffset(),
			pSurface->GetAddress());
	}
	else
	{
		LOGINFO(Display::g_Log, "***** SURFACE ***** not found\n");
	}
}
#endif

namespace Display
{

THREAD_LOCAL_STORAGE int g_iOwnerThreadRefCount = 0;

namespace GCM
{

#define USE_GCM_TEXTURE_POOL

#ifdef USE_GCM_TEXTURE_POOL
Ut::DynamicPool<CellGcmTexture, Ut::ArrayAllocator<U8>, Ut::ObjectLockable> g_gcmTexturePool;
#endif

// C_Sampler

C_Sampler::C_Sampler()
{
	m_pTexture					= NULL;
	m_LodBias					= 0;
	m_LodBiasDefault			= 0;
	m_gcmLodBias				= 0;
	m_gcmTextureFilterDirty		= false;
	m_BorderColour				= 0x00000000;
	m_BorderColourDefault		= 0x00000000;
}

C_Sampler::~C_Sampler()
{
}

// C_Stream

C_Stream::C_Stream() :
	m_pVertexBuffer(NULL),
	m_VertexStride(0),
	m_VertexOffset(0)
{
}

C_Stream::~C_Stream()
{
}

C_DirtySampler::C_DirtySampler()
{
	Invalidate();
}

void C_DirtySampler::Invalidate()
{
	m_AddressU 		= true;
	m_AddressV 		= true;
	m_AddressW 		= true;
	m_FilterMag 	= true;
	m_FilterMin 	= true;
	m_FilterMip 	= true;
	m_BorderColour	= true;
	m_LodBias		= true;
	m_Gamma			= true;
}

C_DirtyDevice::C_DirtyDevice()
{
	Invalidate();
}

void C_DirtyDevice::Invalidate()
{
	for(U32 i=0; i<E_Sampler_Count; i++)
		m_Samplers[i].Invalidate();
}

// C_GCMDevice

bool C_GCMDevice::m_InSystemOSMenu = false;

C_Surface g_SyncSurface;

C_GCMDevice::C_GCMDevice()
{
	m_MaxAniso							= CELL_GCM_TEXTURE_MAX_ANISO_1;
	m_PresentIntervalDefault			= E_PresentInterval_Two;
	m_PresentInterval					= m_PresentIntervalDefault;
	m_VertexShaderProfile				= E_VertexShaderProfile_3_0;
	m_PixelShaderProfile				= E_PixelShaderProfile_3_0;
	m_AlphaTestRef						= 255;
	m_CullModeDefault					= E_CullMode_Cw;
	m_SlopeScaledBias					= 0.f;
	m_pVertexDeclaration				= NULL;
	m_pIndexBuffer						= NULL;
	m_pVertexShader						= NULL;
	m_pPixelShader						= NULL;
	m_FinishRef							= 0;
	m_CurrentBackBuffer					= 0;
	m_CurrentFrontBuffer				= 0;
	m_SwappedFrontBuffer				= 0;
	m_CurrentWindow						= -1;
	m_UniqueBackBuffer					= true;
	m_BeginVerticesUsage				= 0;
	m_MaxBeginVerticesUsage				= 0;
	m_UserBeginVerticesUsage			= 0;
	m_UserMaxBeginVerticesUsage			= 0;
	m_BeginVerticesLocked				= false;
	#ifndef CFG_GOLD
	m_BeginVerticesSize					= 0;
	m_BeginVerticesStart				= NULL;
	#endif
	m_CpuTimeBegin						= 0;
	m_CpuTimeEnd						= 0;
	m_GpuTimeBegin						= 0;
	m_GpuTimeEnd						= 0;
	m_GpuTimeIndex						= 0;

	SetDepthFlip(true);

	m_DisableAA							= false;
	
	m_ZcullEnable						= true;
	m_ScullEnable						= true;

	m_InvalidateVertexCache				= false;
}

C_GCMDevice::~C_GCMDevice()
{
	BIZ_ASSERT(m_Windows.Size() == 0);
}

void C_GCMDevice::InvalidateState()
{
	C_DeviceBase::InvalidateState();

	for(U32 sampler = 0; sampler < E_Sampler_Count; ++sampler)
	{
		m_Sampler[sampler].InvalidateState();
	}

	for(U32 sampler = 0; sampler < E_VertexSampler_Count; ++sampler)
	{
		m_VertexSampler[sampler].InvalidateState();
	}

	for(U32 stream = 0; stream < E_Stream_Count; ++stream)
	{
		m_Stream[stream].InvalidateState();
	}

	m_pVertexShader = NULL;
	m_pPixelShader = NULL;
	m_pVertexDeclaration = NULL;
	m_pIndexBuffer = NULL;

	m_DirtyFlags.Invalidate();
}

void C_GCMDevice::DefaultStates()
{
	// alpha test
	SetAlphaTestEnable(m_AlphaTestEnableDefault);
	SetAlphaTestFunc(m_AlphaTestFuncDefault);
	SetAlphaTestRef(m_AlphaTestRefDefault);
	SetAlphaToMaskEnable(m_AlphaToMaskEnableDefault);
	// blend
	SetBlendEnable(m_BlendEnableDefault);
	SetBlend(m_BlendSrcDefault, m_BlendDstDefault, m_BlendOpDefault, m_BlendSrcAlphaDefault, m_BlendDstAlphaDefault, m_BlendOpAlphaDefault);
	// colour
	SetColourWriteEnable(m_ColourWriteEnableDefault);
	// cull
	SetCullMode(m_CullModeDefault);
	// depth
	SetDepthEnable(m_DepthEnableDefault);
	SetDepthFunc(m_DepthFuncDefault);
	SetDepthWriteEnable(m_DepthWriteEnableDefault);
	// fill
	SetFillMode(m_FillModeDefault);
	// point sprite
	SetPointSpriteEnable(m_PointSpriteEnableDefault);
	// luminance
	SetLuminanceWrite(m_LuminanceWriteDefault);
	// stencil
	SetStencilEnable(m_StencilEnableDefault);
	// pixel shader samplers
	for(U32 sampler=0; sampler<E_Sampler_Count; sampler++)
	{
		C_Sampler& oldSampler = m_Sampler[sampler];
		C_TextureSampler& newSampler = m_TextureSampler[sampler];

		newSampler.m_Address.Set(
			 oldSampler.m_AddressUDefault,
			 oldSampler.m_AddressVDefault,
			 oldSampler.m_AddressWDefault,
			 oldSampler.m_GammaDefault);
		newSampler.m_Control.Set(
			CELL_GCM_FALSE,
			0,
			1<<8,
			CELL_GCM_TEXTURE_MAX_ANISO_1);
		newSampler.m_Filter.Set(
			oldSampler.m_FilterMagDefault,
			oldSampler.m_FilterMinDefault,
			oldSampler.m_FilterMipDefault,
			oldSampler.m_LodBiasDefault);
		newSampler.Set(
			oldSampler.m_BorderColourDefault);

		SetTextureUncached(sampler, &m_NullTexture.m_gcmTexture);
		SetTextureAddressUncached(sampler, m_TextureSampler[sampler].m_Address);
		SetTextureControlUncached(sampler, m_TextureSampler[sampler].m_Control);
		SetTextureFilterUncached(sampler, m_TextureSampler[sampler].m_Filter);
		SetTextureBorderColourUncached(sampler, m_Sampler[sampler].m_BorderColourDefault);
	}
	// vertex shader samplers
	for(U32 sampler = 0; sampler < E_VertexSampler_Count; sampler++)
	{
		SetVertexTextureFilterMag(sampler, m_VertexSampler[sampler].m_FilterMagDefault);
		SetVertexTextureFilterMin(sampler, m_VertexSampler[sampler].m_FilterMinDefault);
		SetVertexTextureFilterMip(sampler, m_VertexSampler[sampler].m_FilterMipDefault);
		SetVertexTextureAddressU(sampler, m_VertexSampler[sampler].m_AddressUDefault);
		SetVertexTextureAddressV(sampler, m_VertexSampler[sampler].m_AddressVDefault);
		SetVertexTextureAddressW(sampler, m_VertexSampler[sampler].m_AddressWDefault);
		SetVertexTextureBorderColour(sampler, m_VertexSampler[sampler].m_BorderColourDefault);
		SetVertexTextureGamma(sampler, m_VertexSampler[sampler].m_GammaDefault);
	}

	SetBlendMode(m_BlendModeDefault);
}

bool C_GCMDevice::CreateDevice()
{
	Display2::Device(&m_Display2);

	Display2::C_DeviceParams display2DeviceParams;

	display2DeviceParams.m_LocalDeviceHeapSize	= m_DeviceParams.m_LocalDeviceMemorySize;
	display2DeviceParams.m_MainDeviceHeapSize	= m_DeviceParams.m_MainDeviceMemorySize;
	display2DeviceParams.m_CmdBufHeapSize		= m_DeviceParams.m_CmdBufMemorySize;
	display2DeviceParams.m_LocalHeapSize		= m_DeviceParams.m_VideoMemorySize;
	display2DeviceParams.m_MainHeapSize			= m_DeviceParams.m_SystemMemorySize;
	
	m_Display2.SetParams(display2DeviceParams);

	if(!m_Display2.Create())
		return false;

	m_CmdBufMemory.SetDisplay2(m_Display2.GetCmdBufHeap());
	m_VideoMemory.SetDisplay2(m_Display2.GetLocalHeap());
	m_SystemMemory.SetDisplay2(m_Display2.GetMainHeap());

	m_DeviceParams.m_LocalDeviceMemorySize		= 0;
	m_DeviceParams.m_MainDeviceMemorySize		= 0;
	m_DeviceParams.m_CmdBufMemorySize			= 0;
	m_DeviceParams.m_VideoMemorySize			= 0;
	m_DeviceParams.m_SystemMemorySize			= 0;

	m_HiDef = m_DeviceParams.m_HiDef;
	m_WideScreen = m_DeviceParams.m_WideScreen;

	CellVideoOutConfiguration config;
	memset(&config, 0, sizeof(CellVideoOutConfiguration));
	
	CellVideoOutState videoState;
	if(cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState) != CELL_OK)
	{
		LOGWARN(g_Log, "Failed to get video out state\n");
		return false;
	}

	m_WideScreen = videoState.displayMode.aspect == CELL_VIDEO_OUT_ASPECT_16_9;
	m_AspectRatio = m_WideScreen ? E_AspectRatio_16_9 : E_AspectRatio_4_3;
	
	m_RefreshRate = videoState.displayMode.refreshRates & CELL_VIDEO_OUT_REFRESH_RATE_50HZ ? E_RefreshRate_50HZ : E_RefreshRate_60HZ;
	
	config.resolutionId = videoState.displayMode.resolutionId;
	config.aspect = videoState.displayMode.aspect;

	MEMORY_TRACKER_PUSH("graphics.display.device.rendertargets");

	// Create the surfaces.
	{
		if(!CreateSurfaces())
		{
			MEMORY_TRACKER_POP("graphics.display.device.rendertargets");
			LOGFATAL(g_Log, "Failed to create the surfaces\n");
			return false;
		}
	}

	// Create the windows.
	{
		if(!CreateWindows())
		{
			MEMORY_TRACKER_POP("graphics.display.device.rendertargets");
			LOGFATAL(g_Log, "Failed to create the windows\n");
			return false;
		}
	}

	MEMORY_TRACKER_POP("graphics.display.device.rendertargets");
	
	// Set the video out configuration.

	config.format = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8;
	config.pitch = m_FrontBuffers[0]->GetSurface()->GetPitch();

	if(cellVideoOutConfigure(CELL_VIDEO_OUT_PRIMARY, &config, NULL, 0) != CELL_OK)
	{
		LOGERROR(g_Log, "cellVideoOutConfigure() failed\n");
		return false;
	}
	
	// Initialise the blocker
	if (!AllocMemory(16, g_BlockerMemorySize, &m_pBlockerMemory, E_AllocHeap_Video, E_AllocType_Start, Display2::E_MemoryType_Misc))
		return false;
	m_BlockerPool.Create((U32*)m_pBlockerMemory, g_BlockerMemorySize);

	// Initialise the fences
	GCMFenceInitialise(m_Fence[E_Fence_Command], g_CommandFenceLabel);
	GCMFenceInitialise(m_Fence[E_Fence_Texture], g_TextureFenceLabel);
	GCMFenceInitialise(m_Fence[E_Fence_Raster], g_RasterFenceLabel);
	
	if (!AllocMemory(16, 16, (U8**)&m_GpuFenceAddress, E_AllocHeap_Main, E_AllocType_Start, Display2::E_MemoryType_Misc))
		return false;
	if (cellGcmAddressToOffset(m_GpuFenceAddress, &m_GpuFenceOffset) != CELL_OK)
		return false;
	m_GpuFenceLocation	= CELL_GCM_LOCATION_MAIN;
	*m_GpuFenceAddress	= m_Fence[E_Fence_Texture].m_Value;

	// Create the dynamic vertex buffer used for begin/end vertices.
	
	MEMORY_TRACKER_PUSH("graphics.display.device.dynamicvertexbuffer");
	if(!m_DynamicVertexBuffer.Create(m_DeviceParams.m_BeginVerticesSize))
	{
		MEMORY_TRACKER_POP("graphics.display.device.dynamicvertexbuffer");
		return false;
	}
	MEMORY_TRACKER_POP("graphics.display.device.dynamicvertexbuffer");

	m_MaxBeginVerticesUsage = m_UserMaxBeginVerticesUsage = (m_DeviceParams.m_BeginVerticesSize-m_DeviceParams.m_FreeBeginVerticesSize);
	
	// Set the default gcm states.

	if(!CreateTexture2D(1, 1, 1, 0, E_Format_A8R8G8B8, &m_NullTexture))
	{
		LOGFATAL(g_Log, "Failed to create the null texture\n");
	}
	else
	{
		U32 *pData = (U32*)m_NullTexture.GetAddress();
		*pData = 0xffffffff;
	}

	#ifdef GCM_DEBUG
	//CheckForErrors();
	#endif

	if(!CreateDownsampleShaders())
	{
		LOGFATAL(g_Log, "Failed to create the downsample shaders\n");
		return false;
	}
	if(!CreatePixelShaderFromFunc("Shaders/psZCull", 0, &m_psZCull, REF_SHADER(psZCull)))
		return false;
	if(!CreateVertexShaderFromFunc("Shaders/vsZCull", 0, &m_vsZCull, REF_SHADER(vsZCull)))
		return false;

	CreateFlipEvent();

	InitialiseThreadOwnership();

	if(!m_Display2.CreateColourSurface(1, 1, Display2::E_Flags_System, Display2::E_SurfaceFormat_L8, Display2::E_Multisample_None, NULL, &g_SyncSurface))
		return false;

	*g_SyncSurface.GetAddress() = 0;
	
	return true;
}

bool C_GCMDevice::CreateDeviceVolatile()
{
	if(!m_Display2.CreateVolatile())
		return false;

	return true;
}

void C_GCMDevice::DestroyDeviceVolatile()
{
	m_Display2.DestroyVolatile();
}

void C_GCMDevice::DestroyDevice()
{
	m_Display2.Destroy(&g_SyncSurface);

	DestroyFlipEvent();

	DestroyDownsampleShaders();

	DestroyPixelShader(&m_psZCull);
	DestroyVertexShader(&m_vsZCull);

	DestroyTexture2D(&m_NullTexture);

	m_DynamicVertexBuffer.Destroy();

	FreeMemory(g_BlockerMemorySize, m_pBlockerMemory, E_AllocHeap_Video, Display2::E_MemoryType_Misc);
	FreeMemory(16, (U8*)m_GpuFenceAddress, E_AllocHeap_Main, Display2::E_MemoryType_Misc);

	DestroySurfaces();

	m_VideoMemory.Destroy();
	m_SystemMemory.Destroy();

	m_Display2.Destroy();

	Display2::Device(NULL);
}


void C_GCMDevice::ResizeSurfaces()
{
	DestroySurfaces();
	CreateSurfaces();
}

bool C_GCMDevice::CreateSurfaces()
{
	m_UniqueBackBuffer = m_DeviceParams.m_UniqueBackBuffer;

	U32 renderBufferFlags = Display2::E_Flags_Device;

	// allocate shared tiled area for all the front buffers rather than waste indices on each one...
	if(!m_Display2.CreateSharedTiledArea(m_FrontBufferTiledArea, m_DeviceParams.m_FrontBufferWidth, m_DeviceParams.m_FrontBufferHeight, renderBufferFlags | Display2::E_Flags_Video, Display2::E_SurfaceFormat_A8R8G8B8, Display2::E_Multisample_None, 2, "_SharedFrontBuffers"))
		return false;

	// Create the first front buffer.
	{
		C_FrontBuffer* pFrontBuffer = new C_FrontBuffer();
		pFrontBuffer->SetWidth(m_DeviceParams.m_FrontBufferWidth);
		pFrontBuffer->SetHeight(m_DeviceParams.m_FrontBufferHeight);
		pFrontBuffer->SetFormat(m_DeviceParams.m_FrontBufferFormat);
		m_FrontBuffers.PushBack(pFrontBuffer);
			
		Display2::C_RenderBuffer* pRenderBuffer = &pFrontBuffer->m_RenderBuffer;
		pRenderBuffer->SetFlags(renderBufferFlags);
		pRenderBuffer->SetName("_Front0");

		pRenderBuffer->OverlapColourBuffer(0, m_FrontBufferTiledArea.GetSize(), m_FrontBufferTiledArea.GetAddress(0));
		pRenderBuffer->m_ColourTileInfo[0].enable	= true;
		pRenderBuffer->m_ColourTileInfo[0].indirect = true;

		Display2::C_RenderBufferSetup renderBufferSetup;
		if(!m_Display2.CreateRenderBuffer(pRenderBuffer, renderBufferSetup))
			return false;

		if(cellGcmSetDisplayBuffer(
			0,
			pFrontBuffer->GetSurface()->GetOffset(),
			pFrontBuffer->GetSurface()->GetPitch(),
			pFrontBuffer->GetWidth(),
			pFrontBuffer->GetHeight()) != CELL_OK)
		{
			LOGFATAL(g_Log, "C_GCMDevice::CreateSurfaces - cellGcmSetDisplayBuffer\n");
			return false;
		}
	}

	// Create the second front buffer.
	{
		C_FrontBuffer* pFrontBuffer = new C_FrontBuffer();
		pFrontBuffer->SetWidth(m_DeviceParams.m_FrontBufferWidth);
		pFrontBuffer->SetHeight(m_DeviceParams.m_FrontBufferHeight);
		pFrontBuffer->SetFormat(m_DeviceParams.m_FrontBufferFormat);
		m_FrontBuffers.PushBack(pFrontBuffer);
			
		Display2::C_RenderBuffer* pRenderBuffer = &pFrontBuffer->m_RenderBuffer;
		pRenderBuffer->SetFlags(renderBufferFlags);
		pRenderBuffer->SetName("_Front1");

		pRenderBuffer->OverlapColourBuffer(0, m_FrontBufferTiledArea.GetSize(), m_FrontBufferTiledArea.GetAddress(1));
		pRenderBuffer->m_ColourTileInfo[0].enable	= true;
		pRenderBuffer->m_ColourTileInfo[0].indirect = true;

		Display2::C_RenderBufferSetup renderBufferSetup;
		if(!m_Display2.CreateRenderBuffer(pRenderBuffer, renderBufferSetup))
			return false;

		if(cellGcmSetDisplayBuffer(
			1,
			pFrontBuffer->GetSurface()->GetOffset(),
			pFrontBuffer->GetSurface()->GetPitch(),
			pFrontBuffer->GetWidth(),
			pFrontBuffer->GetHeight()) != CELL_OK)
		{
			LOGFATAL(g_Log, "C_GCMDevice::CreateSurfaces - cellGcmSetDisplayBuffer\n");
			return false;
		}
	}

	// Create the back buffer.
	{
		C_BackBuffer* pBackBuffer = new C_BackBuffer();
		m_BackBuffers.PushBack(pBackBuffer);

		Display2::C_RenderBuffer* pRenderBuffer = &pBackBuffer->m_RenderBuffer;
		pRenderBuffer->SetFlags(renderBufferFlags);
		pRenderBuffer->SetName("_Back");

		if(m_UniqueBackBuffer)
		{
			pRenderBuffer->SetWidth(m_DeviceParams.m_BackBufferWidth);
			pRenderBuffer->SetHeight(m_DeviceParams.m_BackBufferHeight);
			pRenderBuffer->SetMultisample(GetMultisample2(E_Multisample_None));
			pRenderBuffer->SetColourFormat(0, GetTextureFormat2(m_DeviceParams.m_BackBufferColourFormat));
			pRenderBuffer->SetDepthStencilFormat(GetTextureFormat2(m_DeviceParams.m_BackBufferDepthStencilFormat));
			pRenderBuffer->SetColourAddress(0, NULL); // Unique back buffer
		}
		else
		{
			pRenderBuffer->SetWidth(m_DeviceParams.m_FrontBufferWidth);
			pRenderBuffer->SetHeight(m_DeviceParams.m_FrontBufferHeight);
			pRenderBuffer->SetMultisample(GetMultisample2(E_Multisample_None));
			pRenderBuffer->SetColourFormat(0, GetTextureFormat2(m_DeviceParams.m_FrontBufferFormat));
			pRenderBuffer->SetDepthStencilFormat(GetTextureFormat2(m_DeviceParams.m_BackBufferDepthStencilFormat));
			pRenderBuffer->SetColourAddress(0, m_FrontBuffers[0]->GetSurface()->GetAddress()); // Ping pong back buffer with front buffers
		}

		#if CFG_GOLD
		pRenderBuffer->m_ColourTileInfo[0].enable		= m_UniqueBackBuffer ? true : false;
		pRenderBuffer->m_DepthStencilTileInfo.enable	= true;
		#else
		pRenderBuffer->m_ColourTileInfo[0].enable		= m_UniqueBackBuffer ? true : false;
		pRenderBuffer->m_DepthStencilTileInfo.enable	= true;
		/*if(g_Tiled_Override)
		{
			pRenderBuffer->m_ColourTileInfo[0].enable		= m_UniqueBackBuffer ? g_Tiled_Enable_Back_Color : false;
			pRenderBuffer->m_ColourTileInfo[0].index		= g_Tiled_Index_Back_Color;
			pRenderBuffer->m_ColourTileInfo[0].base			= g_Tiled_Base_Back_Color;
			pRenderBuffer->m_ColourTileInfo[0].bank			= g_Tiled_Bank_Back_Color;

			pRenderBuffer->m_DepthStencilTileInfo.enable	= g_Tiled_Enable_Back_Depth;
			pRenderBuffer->m_DepthStencilTileInfo.index		= g_Tiled_Index_Back_Depth;
			pRenderBuffer->m_DepthStencilTileInfo.base		= g_Tiled_Base_Back_Depth;
			pRenderBuffer->m_DepthStencilTileInfo.bank		= g_Tiled_Bank_Back_Depth;
		}*/
		#endif

		#if CFG_GOLD
		pRenderBuffer->m_DepthStencilZcullInfo.enable	= true;
		#else
		pRenderBuffer->m_DepthStencilZcullInfo.enable	= g_Zcull_Enable_Back_Depth;
		/*if(g_Zcull_Override)
		{
			pRenderBuffer->m_DepthStencilZcullInfo.enable	= g_Zcull_Enable_Back_Depth;
			pRenderBuffer->m_DepthStencilZcullInfo.index	= g_Zcull_Index_Back_Depth;
		}*/
		#endif

		Display2::C_RenderBufferSetup renderBufferSetup;
		if(!m_Display2.CreateRenderBuffer(pRenderBuffer, renderBufferSetup))
			return false;
	}

	return true;
}

void C_GCMDevice::DestroySurfaces()
{
	for(S32 i=0; i<m_BackBuffers.Size(); i++)
	{
		m_Display2.DestroyRenderBuffer(&m_BackBuffers[i]->m_RenderBuffer);

		SAFE_DELETE_PTR(m_BackBuffers[i]);
	}
	m_BackBuffers.Clear();

	m_Display2.DestroySharedTiledArea(m_FrontBufferTiledArea);
	
	for(S32 i=0; i<m_FrontBuffers.Size(); i++)
	{
		m_Display2.DestroyRenderBuffer(&m_FrontBuffers[i]->m_RenderBuffer);

		SAFE_DELETE_PTR(m_FrontBuffers[i]);
	}
	m_FrontBuffers.Clear();
}

//#define TRACK_COMMAND_BUFFER_BUFFER_USAGE
#ifdef TRACK_COMMAND_BUFFER_BUFFER_USAGE
Debug::Bool g_debugDisplayTrackCommandBufferUsage("Graphics.Display.TrackCommandBufferUsage", false);
S32 g_CommandBufferUsage = 0;
#endif

//#define TRACK_DYNAMIC_VERTEX_BUFFER_USAGE
#ifdef TRACK_DYNAMIC_VERTEX_BUFFER_USAGE
Debug::Bool g_debugDisplayTrackDynamicVertexBufferUsage("Graphics.Display.TrackDynamicVertexBufferUsage", false);
S32 g_DynamicVertexBufferUsage = 0;
#endif

bool C_GCMDevice::BeginScene()
{
	BIZ_ASSERT(HaveThreadOwnership());

	for(S32 i=0; i<m_Windows.Size(); i++)
	{
		C_Window* pWindow = m_Windows[i];

		if(AADisabled())
		{
			pWindow->OverlapBuffers(
				m_LocalSharedRenderBuffer.GetColourSurface(0),
				m_LocalSharedRenderBuffer.GetDepthStencilSurface(),
				m_LocalSharedRenderBuffer.GetColourTexture(0),
				m_LocalSharedRenderBuffer.GetDepthStencilTexture());
		}
		else
		{
			pWindow->OverlapBuffers();
		}
	}

	#if TRACK_DISPLAY2_COMMAND_BUFFERS
	Display2::g_CommandBufferWalker.m_pPrevHeadAddress = Display2::g_CommandBufferWalker.m_pCurrHeadAddress;
	Display2::g_CommandBufferWalker.m_pCurrHeadAddress = (U8*)gCellGcmCurrentContext->current;
	#endif

	#ifdef USE_GCM_P3X
	if(g_debugDisplayCaptureFrame)
	{
		p3x_begin(gCellGcmCurrentContext->current);
		g_debugDisplayCaptureFrame = false; 
	}
	#endif

	{
		#ifdef TRACK_COMMAND_BUFFER_BUFFER_USAGE
		uint8_t index = 0;
		CellGcmControl* pgcmControl = NULL;
		GCMGetCurrentCommandBuffer(index, &pgcmControl);
		g_CommandBufferUsage = (S32)pgcmControl->get;
		#endif

		#ifdef TRACK_DYNAMIC_VERTEX_BUFFER_USAGE
		g_DynamicVertexBufferUsage = (S32)m_DynamicVertexBuffer.GetOffset();
		#endif
	}

	BIZ_ASSERT(!m_InScene);
	m_InScene = true;

	SetBackBuffer();
	
	const C_BackBuffer& backBuffer = GetBackBuffer();

	C_Viewport viewport;
	viewport.m_W = backBuffer.GetWidth();
	viewport.m_H = backBuffer.GetHeight();
	SetViewport(viewport);

	m_pView = NULL;
	
	Reset();

	SetZcullEnable(IsZcullEnabled(), IsScullEnabled()); // Make sure zcull is on at the start of the frame

	return true;
}

void C_GCMDevice::EndScene()
{
	BIZ_ASSERT(m_InScene);
	m_InScene = false;

	#ifdef TRACK_DYNAMIC_VERTEX_BUFFER_USAGE
	g_DynamicVertexBufferUsage = ((S32)m_DynamicVertexBuffer.GetOffset())-g_DynamicVertexBufferUsage;
	if(g_debugDisplayTrackDynamicVertexBufferUsage)
	{
		LOGINFO(g_Log, "Dynamic vertex buffer usage for frame '%i'\n", g_DynamicVertexBufferUsage);
	}
	#endif

	#ifdef TRACK_COMMAND_BUFFER_BUFFER_USAGE
	uint8_t index = 0;
	CellGcmControl* pgcmControl = NULL;
	GCMGetCurrentCommandBuffer(index, &pgcmControl);
	g_CommandBufferUsage = ((S32)pgcmControl->get)-g_CommandBufferUsage;
	if(g_debugDisplayTrackCommandBufferUsage)
	{
		LOGINFO(g_Log, "Command buffer usage for frame '%i'\n", g_CommandBufferUsage);
	}
	#endif

	#ifdef USE_GCM_P3X
	if(p3x_is_capturing())
		p3x_end(gCellGcmCurrentContext->current);
	#endif

	#if TRACK_DISPLAY2_COMMAND_BUFFERS
	Display2::g_CommandBufferWalker.m_pPrevTailAddress = Display2::g_CommandBufferWalker.m_pCurrTailAddress;
	Display2::g_CommandBufferWalker.m_pCurrTailAddress = (U8*)gCellGcmCurrentContext->current;
	#endif
}

void C_GCMDevice::CopyBackToFrontBuffer()
{
	if(!m_UniqueBackBuffer)
		return;

	BIZ_ASSERT(!m_InScene);
	m_InScene = true;

	SetFrontBuffer();

	const C_BackBuffer& backBuffer = GetBackBuffer();
	const C_FrontBuffer& frontBuffer = GetFrontBuffer();

	C_Viewport viewport;
	viewport.m_W = frontBuffer.GetWidth();
	viewport.m_H = frontBuffer.GetHeight();
	SetViewport(viewport);

	CellGcmTexture gcmTexture;
	gcmTexture.mipmap = 1;
	gcmTexture.dimension = CELL_GCM_TEXTURE_DIMENSION_2;
	gcmTexture.cubemap = CELL_GCM_FALSE;
	gcmTexture.format = CELL_GCM_TEXTURE_A8R8G8B8 | CELL_GCM_TEXTURE_NR | CELL_GCM_TEXTURE_LN;
	gcmTexture.remap =
		CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
		CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
		CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
		CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
		CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
		CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
		CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
		CELL_GCM_TEXTURE_REMAP_FROM_A;
    
	gcmTexture.width = backBuffer.GetWidth();
	gcmTexture.height = backBuffer.GetHeight();

	// depending on multisample we need to tell the texture to be bigger
	switch(backBuffer.GetColourSurface()->GetMultisample())
	{
	case E_Multisample_2:
		gcmTexture.width *= 2;
		break;
	case E_Multisample_4:
		gcmTexture.width *= 2;
		gcmTexture.height *= 2;
		break;
	default:
		break;
	}

	gcmTexture.depth  = 1;
	gcmTexture.pitch = backBuffer.GetColourSurface()->GetPitch();
	gcmTexture.location = CELL_GCM_LOCATION_LOCAL;
	gcmTexture.offset = backBuffer.GetColourSurface()->GetOffset();

	cellGcmSetInvalidateTextureCache(CELL_GCM_INVALIDATE_TEXTURE);
	cellGcmSetTexture(0, &gcmTexture);
	
	const C_TextureControl textureControl(
		CELL_GCM_TRUE,
		0<<8,
		0<<8,
		CELL_GCM_TEXTURE_MAX_ANISO_16);
	SetTextureControlCached(0, textureControl);
	
	const C_TextureAddress textureAddress(
		CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
		CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
		CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
		CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL,
		CELL_GCM_TEXTURE_ZFUNC_LESS,
		0);
	SetTextureAddressCached(0, textureAddress);
	
	const C_TextureFilter textureFilter(
		0,
		CELL_GCM_TEXTURE_LINEAR,
		CELL_GCM_TEXTURE_LINEAR,
		CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX);
	SetTextureFilterCached(0, textureFilter);

	SetBlendMode(E_BlendMode_Solid);

	Quad::DrawFullScreen(NULL);	

	BIZ_ASSERT(m_InScene);
	m_InScene = false;
}

void C_GCMDevice::Reset()
{
	cellGcmSetShadeMode(CELL_GCM_SMOOTH);
	//cellGcmSetDitherEnable(CELL_GCM_TRUE);
	//cellGcmSetLineSmoothEnable(CELL_GCM_TRUE);
	//cellGcmSetPolySmoothEnable(CELL_GCM_TRUE);
	cellGcmSetRestartIndexEnable(CELL_GCM_TRUE);
	cellGcmSetRestartIndex(0xFFFF);

	for (int i = 0 ; i < 15 ; i++)
	{
		cellGcmSetAnisoSpread( i, CELL_GCM_FALSE, CELL_GCM_FALSE, CELL_GCM_FALSE, CELL_GCM_TEXTURE_ANISO_SPREAD_2_00_TEXEL, CELL_GCM_TEXTURE_ANISO_SPREAD_2_00_TEXEL, CELL_GCM_TEXTURE_ANISO_SPREAD_2_00_TEXEL );
	}
		
	InvalidateState();
	DefaultStates();

	InvalidateVertexShaderCache();

	SetVertexShader(NULL);
	SetPixelShader(NULL);
	SetVertexDeclaration(NULL);
	SetIndexBuffer(NULL);
	for(U32 sampler = 0; sampler < E_Sampler_Count; sampler++)
		SetTexture(sampler, NULL);
	for(U32 stream = 0; stream < E_Stream_Count; stream++)
		SetVertexBuffer(stream, NULL, 0, 0);

	SetView(m_pView);

#ifdef GCM_DEBUG
	CheckForErrors();
#endif
}

void C_GCMDevice::Clear(const U32 flags)
{
	Clear(flags, GetColourClear(), GetDepthClear(), GetStencilClear());
}

void C_GCMDevice::Clear(const U32 flags, const Maths::RGBA colour)
{
	Clear(flags, colour, GetDepthClear(), GetStencilClear());
}

void C_GCMDevice::Clear(const U32 flags, const Maths::RGBA colour, const F32 depth, const U8 stencil)
{
	U32 gcmMask = 0;

	if(m_RenderTarget.m_pDepthStencilSurface)
	{
		const U32 gcmDepthStencil = (U32(depth * 0xffffff) << 8) | stencil;
		cellGcmSetClearDepthStencil(gcmDepthStencil);
	
		if(flags & E_Clear_Depth)
		{
			gcmMask |= CELL_GCM_CLEAR_Z;
		}
		if(flags & E_Clear_Stencil)
		{
			gcmMask |= CELL_GCM_CLEAR_S;
		}
	}
	
	if(m_RenderTarget.m_pColourSurface)
	{
		switch(m_RenderTarget.m_pColourSurface->GetFormat())
		{
			case E_Format_A16B16G16R16F:
			case E_Format_R32F:
				Quad::DrawFullScreenColour(colour);
				break;
			default:
				const U32 gcmColour = ((colour.A() & 0xff) << 24) | ((colour.R() & 0xff) << 16) | ((colour.G() & 0xff) << 8) | (colour.B() & 0xff);
				cellGcmSetClearColor(gcmColour);
				
				if(flags & E_Clear_Colour)
				{
					gcmMask |= CELL_GCM_CLEAR_R;
					gcmMask |= CELL_GCM_CLEAR_G;
					gcmMask |= CELL_GCM_CLEAR_B;
					gcmMask |= CELL_GCM_CLEAR_A;
				}
				break;
		}
	}

	if(gcmMask != 0)
	{
		cellGcmSetClearSurface(gcmMask);
	}

#ifdef GCM_DEBUG
	CheckForErrors();
#endif
}

enum
{
	CELL_GCM_ZCULL_STATS		= (2),
	CELL_GCM_ZCULL_STATS1		= (3),
	CELL_GCM_ZCULL_STATS2		= (4),
	CELL_GCM_ZCULL_STATS3		= (5)
};

void ZCullReport(void)
{
	GCMSetReport(CELL_GCM_ZCULL_STATS,  g_ReportZcullStats0);
	GCMSetReport(CELL_GCM_ZCULL_STATS1, g_ReportZcullStats1);
}

void ZCullParametersSet(void)
{
	int32_t	maxSlope = cellGcmGetReport(CELL_GCM_ZCULL_STATS,  g_ReportZcullStats0);
	int32_t	sumSlope = cellGcmGetReport(CELL_GCM_ZCULL_STATS1, g_ReportZcullStats1);

	//LOGDEBUG( g_Log, "maxslope = %d\n", maxSlope);
	//LOGDEBUG( g_Log, "sumSlope = %d\n", maxSlope);

	int32_t	numTiles, avgSlope, moveForward, pushBack;

	numTiles = maxSlope & 0xffff;
	maxSlope = (maxSlope & 0xFFFF0000) >> 16;
	avgSlope = numTiles ? sumSlope / numTiles : 0;

	moveForward = (avgSlope + maxSlope) / 2;
	pushBack = moveForward / 2;

	if (moveForward < 1)
		moveForward = 1;

	if (pushBack < 1)
		pushBack = 1;

	if (g_iOverrideZCullForward>0)
		moveForward = g_iOverrideZCullForward;
	if (g_iOverrideZCullBack>0)
		pushBack = g_iOverrideZCullBack;

	cellGcmSetZcullLimit(moveForward, pushBack);
}

void C_GCMDevice::WaitForFlip()
{
	static bool firstTime = true;

	if(!firstTime)
	{
		while(cellGcmGetFlipStatus() != 0 && !m_InSystemOSMenu)
		{
			sys_timer_usleep(300);
		}
	}

	cellGcmResetFlipStatus();

	firstTime = false;
}

void C_GCMDevice::BlockUntilIdle()
{
	static U32 labelValue = 1;

	AcquireThreadOwnership();

	GCMGpuSetLabelOnRasterEnd(g_BlockUntilIdleWaitLabel, labelValue);
	GCMGpuWaitOnLabel(g_BlockUntilIdleWaitLabel, labelValue);

	++labelValue;
	
	GCMGpuSetLabelOnRasterEnd(g_BlockUntilIdleWaitLabel, labelValue);
	cellGcmFlush();
	GCMCpuWaitOnLabel(g_BlockUntilIdleWaitLabel, labelValue);
	
	++labelValue;

	ReleaseThreadOwnership();
}

U32* C_GCMDevice::InsertBlocker(const U32 count)
{
	U32* pBlocker = GetBlocker(count);
	SetBlocker(pBlocker);
	return pBlocker;
}

U32* C_GCMDevice::GetBlocker(const U32 count)
{
	return m_BlockerPool.Allocate(count);
}

void C_GCMDevice::SetBlocker(const U32* handle)
{
	U32 offset = 0;
	cellGcmAddressToOffset(handle, &offset);
	cellGcmSetCallCommand(offset);
}

U32 C_GCMDevice::InsertFence()
{
	return InsertFence(E_Fence_Texture);
}

U32 C_GCMDevice::GetCurrentFence()
{
	return GetCurrentFence(E_Fence_Texture);
}

bool C_GCMDevice::IsFencePassed(const U32 value)
{
	return IsFencePassed(E_Fence_Texture, value);
}

void C_GCMDevice::BlockOnFence(const U32 value)
{
	BlockOnFence(E_Fence_Texture, value);
}

void C_GCMDevice::WriteFence()
{
	const U32 waitValue	= m_Fence[E_Fence_Texture].m_Value;
	const U32 waitLabel	= m_Fence[E_Fence_Texture].m_LabelId;
	GCMGpuWaitOnLabel(waitLabel, waitValue);

	cellGcmInlineTransfer(m_GpuFenceOffset, &m_Fence[E_Fence_Texture].m_Value, 1, m_GpuFenceLocation);
	cellGcmSetTransferLocation(CELL_GCM_LOCATION_LOCAL);
}

volatile U32* C_GCMDevice::GetFenceAddress()
{
	return GetFenceAddress(E_Fence_Texture);
}

volatile U32* C_GCMDevice::GetWriteFenceAddress()
{
	return m_GpuFenceAddress;
}

U32 C_GCMDevice::InsertFence(const E_Fence eFence)
{
	const U32 index = GCMFenceLabel(m_Fence[eFence]);
	const U32 value = GCMFenceNextValue(m_Fence[eFence]);
	if (E_Fence_Command == eFence)
		GCMGpuSetLabelOnCommandEnd(index, value);
	else if (E_Fence_Texture == eFence)
		GCMGpuSetLabelOnTextureEnd(index, value);
	else if (E_Fence_Raster == eFence)
		GCMGpuSetLabelOnRasterEnd(index, value);
	return value;
}

U32 C_GCMDevice::GetCurrentFence(const E_Fence eFence)
{
	return GCMFenceValue(m_Fence[eFence]);
}

bool C_GCMDevice::IsFencePassed(const E_Fence eFence, const U32 value)
{
	return GCMFenceIsPassed(m_Fence[eFence], value);
}

void C_GCMDevice::BlockOnFence(const E_Fence eFence, const U32 value)
{
	while (!IsFencePassed(eFence, value))
	{
		sys_timer_usleep(100);
	}
}

volatile U32* C_GCMDevice::GetFenceAddress(const E_Fence eFence)
{
	return GCMFenceAddress(m_Fence[eFence]);
}

void C_GCMDevice::SyncCpuGpu()
{
	if(g_UseNewSync)
	{
		static U8 cpuCounter = 0;

		SetColourDepthStencilSurface(&g_SyncSurface, NULL);
		SetViewport(C_Viewport());

		SetBlendMode(E_BlendMode_Solid);
		SetColourWriteEnable(E_ColourWriteEnable_All);
		SetCullMode(E_CullMode_None);

		Quad::DrawVertexColour(Maths::RGBA(cpuCounter, cpuCounter, cpuCounter, cpuCounter));
		
		SetCullMode(E_CullMode_Default);
		SetColourWriteEnable(E_ColourWriteEnable_Default);

		cellGcmFlush();

		U8 gpuCounter = *g_SyncSurface.GetAddress();
		U32 cpuCounter1 = gpuCounter > cpuCounter ? (cpuCounter + 256) : cpuCounter;
		
		while((cpuCounter1 - gpuCounter) > 1)
		{
			sys_timer_usleep(30);

			gpuCounter = *g_SyncSurface.GetAddress();
			cpuCounter1 = gpuCounter > cpuCounter ? (cpuCounter + 256) : cpuCounter;
		}

		cpuCounter = cpuCounter == 255 ? 0 : (cpuCounter + 1);
	}
	else
	{
		//CPU_TIMING(0, Maths::RGBA(255, 255, 255, 255), "SyncCpuGpu");

		static volatile int32_t *piBufferSwapLabel;
		static int32_t iBufferSwapLabelValue=0;
		static bool bBufferSwapInitialized=false;

		if (!bBufferSwapInitialized)
		{
			piBufferSwapLabel = (volatile int32_t*)cellGcmGetLabelAddress(g_CpuGpuSyncWaitLabel);
			
			*piBufferSwapLabel = iBufferSwapLabelValue;
			
			bBufferSwapInitialized = true;
		}

		cellGcmSetWriteBackEndLabel(g_CpuGpuSyncWaitLabel, iBufferSwapLabelValue);
		cellGcmFlush();

		#if !CFG_GOLD
		Time::C_Time oldTime = Time::GetPerformanceCounter();
		#endif

		while (abs(iBufferSwapLabelValue-*piBufferSwapLabel) > 1)
		{
			sys_timer_usleep(30);

			#if !CFG_GOLD && 0
			Time::C_Time newTime = Time::GetPerformanceCounter();
			Time::C_Time curTime = (newTime - oldTime) / 1000;
			if(curTime > 33 * 30 * 3)
			{
				LOGERROR(g_Log, "Possible race condition hit! Please send TTY to LloydW...\n");

				#if USE_DEVICE_MARKERS
				C_DeviceMarkerManager::GetInstance().Dump();
				#endif

				DebugBreak();
			}
			#endif

			#if TRACK_DISPLAY2_COMMAND_BUFFERS
			if(g_debugDisplayCrashGpuOnSyncCpuGpuHang)
			{
				CellGcmControl* pGcmControl = cellGcmGetControlRegister();
				CORE_PRINT("CellGcmControl : put %.8x get %.8x ref %.8x\n", pGcmControl->put, pGcmControl->get, pGcmControl->ref);

				CellGcmContextData* pGcmContextData = gCellGcmCurrentContext;
				CORE_PRINT("CellGcmContextData : begin %.8x end %.8x current %.8x\n", pGcmContextData->begin, pGcmContextData->end, pGcmContextData->current);

				Display2::g_CommandBufferWalker.m_pCurrTailAddress = (U8*)gCellGcmCurrentContext->current;
				CORE_PRINT("CurrHeadAddress : %.8x\n", Display2::g_CommandBufferWalker.m_pCurrHeadAddress);
				CORE_PRINT("CurrTailAddress : %.8x\n", Display2::g_CommandBufferWalker.m_pCurrTailAddress);
				Display2::g_CommandBufferTracker.Dump();

				Display2::C_CommandBufferInstance* pInstance = Display2::g_CommandBufferTracker.FindInstance((U32)pGcmControl->get - 4096);
				if(pInstance)
					Display2::WalkCommandBufferAddress(pInstance->m_pHeadAddress, pInstance->m_pTailAddress, true);
				else
					CORE_PRINT("Failed to find offset %.8x\n", pGcmControl->get);
				
				cellGcmSetAlphaFunc(-1, -1);
			}
			#endif
		}

		iBufferSwapLabelValue++;
	}
}

U32 g_ReadJumpToSelf		= 0;
U32 g_WriteJumpToSelf		= 0;
U32 g_JumpToSelfAddress[3]	= {0,0,0};
S32 g_VSyncCounter			= 0;

bool C_GCMDevice::Present(bool flip)
{
	CopyBackToFrontBuffer(); // Copy back to front buffer if unique back buffer here instead of EndScene...

	#ifdef CELL_GCM_DEBUG
	void* pCellGcmDebugCallback = Display2::DisableDebugFinish();
	#endif 

	ZCullReport();

	EndTimingFrame();

#ifdef USE_DEFAULT_COMMAND_BUFFER
	if(flip)
	{
		while(g_JumpToSelfAddress[g_WriteJumpToSelf] != 0)
		{
			sys_timer_usleep(30);
		}

		g_JumpToSelfAddress[g_WriteJumpToSelf++] = GCMCreateJumpToSelf();
		if(g_WriteJumpToSelf == 3)
			g_WriteJumpToSelf = 0;
		
		if(cell::Gcm::cellGcmSetFlip(m_CurrentFrontBuffer) != CELL_OK)
		{
			#ifdef CELL_GCM_DEBUG
			Display2::EnableDebugFinish(pCellGcmDebugCallback);
			#endif

			LOGFATAL(g_Log, "Failed to flip byuffer %i\n", m_CurrentFrontBuffer);
			return false;
		}

		cellGcmSetWaitFlip();
	}
	
	cellGcmFlush();

	// This is needed otherwise resources can be freed by the streamer before
	// the GPU has finished using them leading to corruption.
	SyncCpuGpu();

	if(flip)
	{
		g_VSyncCounter = m_PresentInterval;
	}
#else
	WaitForFlip();

	#ifdef USE_UNSAFE_GCM
	if(cell::Gcm::cellGcmSetFlip(m_CurrentFrontBuffer) != CELL_OK)
	#else
	if(cellGcmSetFlip(m_CurrentFrontBuffer) != CELL_OK)
	#endif
	{
		return false;
	}
	
	#ifdef USE_GCM_HUD
	cellGcmHUDFlush();
	#else
	cellGcmFlush();
	#endif

	#ifdef USE_GCM_REPLAY
	cell::Gcm::Replay::Capture::Heartbeat(gCellGcmCurrentContext);
	#endif

	#ifdef USE_UNSAFE_GCM
	cell::Gcm::cellGcmSetWaitFlip();
	#else
	cellGcmSetWaitFlip();
	#endif
#endif

	SwapFrontBuffers(flip);
	
	#ifdef USE_GCM_USER_FIFO
	//GCMSwapUserFifo();
	#endif

	StartTimingFrame();
	
	ZCullParametersSet();

	#ifdef CELL_GCM_DEBUG
	Display2::EnableDebugFinish(pCellGcmDebugCallback);
	#endif

	#if defined(CELL_GCM_DEBUG)
	if(g_debugDisplayEnableGCMDebug)
		GCMEnableDebug();
	else
		GCMDisableDebug();
	#endif
	
	return true;
}

void* C_GCMDevice::BeginVertices(const E_PrimitiveType primitiveType, const U32 vertexCount, const U32 vertexStride, C_VertexDeclaration* pVertexDeclaration, const bool checkUsage)
{
	BIZ_ASSERT(!m_BeginVerticesLocked);

	BIZ_ASSERT( HaveThreadOwnership() );
	
	U32 size = vertexCount * vertexStride;

	if(checkUsage)
	{
		if((m_UserBeginVerticesUsage+size) > m_UserMaxBeginVerticesUsage)
		{
			#ifdef DEVICE_DEBUG
			g_BeginVerticesFailingCheck = true;
			#endif

			return NULL;
		}

		if((m_BeginVerticesUsage+size) > m_MaxBeginVerticesUsage)
		{
			#ifdef DEVICE_DEBUG
			g_BeginVerticesFailingCheck = true;
			#endif

			return NULL;
		}
	}
	
	void* pVertices = m_DynamicVertexBuffer.Begin(primitiveType, vertexCount, vertexStride, pVertexDeclaration);
	
	if(pVertices)
	{
		m_BeginVerticesLocked = true;
		m_BeginVerticesUsage += size;
		m_UserBeginVerticesUsage += size;

		#ifndef CFG_GOLD
		m_BeginVerticesSize = size;
		m_BeginVerticesStart = (U8*)pVertices;
		#endif

		return pVertices;
	}
	else
	{
		#ifdef DEVICE_DEBUG
		g_BeginVerticesFailingCheck = true;
		#endif

		LOGERROR(g_Log, "Vertices is null! Failed to begin vertices of count '%i' stride '%i' size '%i' primitive '%i'\n", vertexCount, vertexStride, vertexCount * vertexStride, primitiveType);

		return NULL;
	}
}

void C_GCMDevice::EndVertices(void* pVertex)
{
	BIZ_ASSERT(m_BeginVerticesLocked);

	const U32 offset = m_DynamicVertexBuffer.GetOffset(); // Store offset before its advanced by ending dynamic vertex buffer.

	m_DynamicVertexBuffer.End();
	
	SetVertexBuffer(0, m_DynamicVertexBuffer.GetVertexBuffer(), offset, m_DynamicVertexBuffer.GetVertexStride());
	SetVertexDeclaration(m_DynamicVertexBuffer.GetVertexDeclaration());
	
	DrawPrimitive(m_DynamicVertexBuffer.GetPrimitiveType(), 0, m_DynamicVertexBuffer.GetVertexCount());

	m_BeginVerticesLocked = false;

	#ifndef CFG_GOLD
	if(pVertex)
	{
		U32 usedEnd = (U32)pVertex;
		U32 actualEnd = (U32)m_BeginVerticesStart+m_BeginVerticesSize;
		if(usedEnd > actualEnd)
		{
			LOGFATAL(g_Log, "C_GCMDevice::EndVertices - usedEnd > actualEnd by %i\n", usedEnd-actualEnd);
		}
		else
		if(usedEnd < actualEnd)
		{
			LOGWARN(g_Log, "C_GCMDevice::EndVertices - usedEnd < actualEnd by %i\n", actualEnd-usedEnd);
		}
	}
	m_BeginVerticesSize = 0;
	m_BeginVerticesStart = NULL;
	#endif
}

void C_GCMDevice::SetSurface()
{
	if(!m_RenderTarget.m_pDepthStencilSurface)
	{
		SetDepthEnable(E_Bool_False);
		SetDepthWriteEnable(E_Bool_False);
		SetStencilEnable(E_Bool_False);
	}
	
	CellGcmSurface gcmSurface;
	
	gcmSurface.type					= CELL_GCM_SURFACE_PITCH;
	gcmSurface.antialias			= m_RenderTarget.m_pColourSurface ? GCMMultisample((E_Multisample)m_RenderTarget.m_pColourSurface->GetMultisample()) : m_RenderTarget.m_pDepthStencilSurface ? GCMMultisample((E_Multisample)m_RenderTarget.m_pDepthStencilSurface->GetMultisample()) : CELL_GCM_SURFACE_CENTER_1;
	gcmSurface.x 					= 0;
	gcmSurface.y 					= 0;
	gcmSurface.width 				= m_RenderTarget.m_pColourSurface ? m_RenderTarget.m_pColourSurface->GetWidth() : m_RenderTarget.m_pDepthStencilSurface ? m_RenderTarget.m_pDepthStencilSurface->GetWidth() : 1;
	gcmSurface.height 				= m_RenderTarget.m_pColourSurface ? m_RenderTarget.m_pColourSurface->GetHeight() : m_RenderTarget.m_pDepthStencilSurface ? m_RenderTarget.m_pDepthStencilSurface->GetHeight() : 1;
	
	U32 colourLocation = CELL_GCM_LOCATION_LOCAL;
	if(m_RenderTarget.m_pColourSurface && m_RenderTarget.m_pColourSurface->GetFlags() & E_Usage_System)
		colourLocation = CELL_GCM_LOCATION_MAIN;
	
	// colour buffers
	gcmSurface.colorFormat 			= m_RenderTarget.m_pColourSurface ? GCMSurfaceFormat(m_RenderTarget.m_pColourSurface->GetFormat()) : CELL_GCM_SURFACE_A8R8G8B8;
	gcmSurface.colorTarget			= m_RenderTarget.m_pColourSurface ? CELL_GCM_SURFACE_TARGET_0 : CELL_GCM_SURFACE_TARGET_NONE;
	gcmSurface.colorLocation[0]		= colourLocation;
	gcmSurface.colorLocation[1]		= colourLocation;
	gcmSurface.colorLocation[2]		= colourLocation;
	gcmSurface.colorLocation[3]		= colourLocation;
	gcmSurface.colorOffset[0] 		= m_RenderTarget.m_pColourSurface ? m_RenderTarget.m_pColourSurface->GetOffset() : 0;
	gcmSurface.colorOffset[1] 		= 0;
	gcmSurface.colorOffset[2] 		= 0;
	gcmSurface.colorOffset[3] 		= 0;
	gcmSurface.colorPitch[0] 		= m_RenderTarget.m_pColourSurface ? m_RenderTarget.m_pColourSurface->GetPitch() : 64;
	gcmSurface.colorPitch[1]		= 64;
	gcmSurface.colorPitch[2]		= 64;
	gcmSurface.colorPitch[3]		= 64;

	U32 depthLocation = CELL_GCM_LOCATION_LOCAL;
	if(m_RenderTarget.m_pDepthStencilSurface && m_RenderTarget.m_pDepthStencilSurface->GetFlags() & E_Usage_System)
		depthLocation = CELL_GCM_LOCATION_MAIN;
	
	// depth buffer
	gcmSurface.depthFormat 			= m_RenderTarget.m_pDepthStencilSurface ? GCMSurfaceFormat(m_RenderTarget.m_pDepthStencilSurface->GetFormat()) : CELL_GCM_SURFACE_Z24S8;
	gcmSurface.depthLocation		= depthLocation;
	gcmSurface.depthOffset			= m_RenderTarget.m_pDepthStencilSurface ? m_RenderTarget.m_pDepthStencilSurface->GetOffset() : 0;
	gcmSurface.depthPitch 			= m_RenderTarget.m_pDepthStencilSurface ? m_RenderTarget.m_pDepthStencilSurface->GetPitch() : 64;

	BIZ_ASSERT(	gcmSurface.type == CELL_GCM_SURFACE_PITCH ||
				gcmSurface.type == CELL_GCM_SURFACE_SWIZZLE);

	BIZ_ASSERT(	gcmSurface.antialias == CELL_GCM_SURFACE_CENTER_1 ||
				gcmSurface.antialias == CELL_GCM_SURFACE_DIAGONAL_CENTERED_2 ||
				gcmSurface.antialias == CELL_GCM_SURFACE_SQUARE_CENTERED_4 ||
				gcmSurface.antialias == CELL_GCM_SURFACE_SQUARE_ROTATED_4);

	BIZ_ASSERT(	gcmSurface.colorFormat == CELL_GCM_SURFACE_X1R5G5B5_Z1R5G5B5 ||
				gcmSurface.colorFormat == CELL_GCM_SURFACE_X1R5G5B5_O1R5G5B5 ||
				gcmSurface.colorFormat == CELL_GCM_SURFACE_R5G6B5 ||
				gcmSurface.colorFormat == CELL_GCM_SURFACE_X8R8G8B8_Z8R8G8B8 ||
				gcmSurface.colorFormat == CELL_GCM_SURFACE_X8R8G8B8_O8R8G8B8 ||
				gcmSurface.colorFormat == CELL_GCM_SURFACE_A8R8G8B8 ||
				gcmSurface.colorFormat == CELL_GCM_SURFACE_B8 ||
				gcmSurface.colorFormat == CELL_GCM_SURFACE_G8B8 ||
				gcmSurface.colorFormat == CELL_GCM_SURFACE_F_W16Z16Y16X16 ||
				gcmSurface.colorFormat == CELL_GCM_SURFACE_F_W32Z32Y32X32 ||
				gcmSurface.colorFormat == CELL_GCM_SURFACE_F_X32 ||
				gcmSurface.colorFormat == CELL_GCM_SURFACE_X8B8G8R8_Z8B8G8R8 ||
				gcmSurface.colorFormat == CELL_GCM_SURFACE_X8B8G8R8_O8B8G8R8 ||
				gcmSurface.colorFormat == CELL_GCM_SURFACE_A8B8G8R8);

	for(U32 i=0; i<CELL_GCM_MRT_MAXCOUNT; i++)
	{
		BIZ_ASSERT(	gcmSurface.colorLocation[i] == CELL_GCM_LOCATION_LOCAL ||
					gcmSurface.colorLocation[i] == CELL_GCM_LOCATION_MAIN);

		BIZ_ASSERT(	gcmSurface.colorOffset[i] <= Display2::g_gcmConfig.localSize);

		BIZ_ASSERT(	gcmSurface.colorOffset[i] % 64 == 0);

		BIZ_ASSERT(	gcmSurface.colorPitch[i] % 64 == 0);

		BIZ_ASSERT(	gcmSurface.colorOffset[i] != 0 ? gcmSurface.colorPitch[i] >= 64 : 1);
	}

	BIZ_ASSERT(	gcmSurface.colorTarget == CELL_GCM_SURFACE_TARGET_NONE ||
				gcmSurface.colorTarget == CELL_GCM_SURFACE_TARGET_0 ||
				gcmSurface.colorTarget == CELL_GCM_SURFACE_TARGET_1 ||
				gcmSurface.colorTarget == CELL_GCM_SURFACE_TARGET_MRT1 ||
				gcmSurface.colorTarget == CELL_GCM_SURFACE_TARGET_MRT2 ||
				gcmSurface.colorTarget == CELL_GCM_SURFACE_TARGET_MRT3);

	BIZ_ASSERT(	gcmSurface.depthFormat == CELL_GCM_SURFACE_Z16 ||
				gcmSurface.depthFormat == CELL_GCM_SURFACE_Z24S8);

	BIZ_ASSERT(	gcmSurface.depthLocation == CELL_GCM_LOCATION_LOCAL ||
				gcmSurface.depthLocation == CELL_GCM_LOCATION_MAIN);

	BIZ_ASSERT(	gcmSurface.depthOffset <= Display2::g_gcmConfig.localSize);

	BIZ_ASSERT(	gcmSurface.depthOffset % 64 == 0);

	BIZ_ASSERT(	gcmSurface.depthPitch % 64 == 0);

	BIZ_ASSERT(	gcmSurface.depthOffset != 0 ? gcmSurface.depthPitch >= 64 : 1);

	BIZ_ASSERT(	gcmSurface.width >= 1 &&
				gcmSurface.width <= 4096);

	BIZ_ASSERT(	gcmSurface.height >= 1 &&
				gcmSurface.height <= 4096);

	BIZ_ASSERT(	gcmSurface.x <= 4095);

	BIZ_ASSERT(	gcmSurface.y <= 4095);

	BIZ_ASSERT(	gcmSurface.width == 1 ? gcmSurface.height == 1 : true);

	cellGcmSetSurface(&gcmSurface);

    const uint32_t multisampleEnable = (m_RenderTarget.m_pColourSurface && m_RenderTarget.m_pColourSurface->GetMultisample() != E_Multisample_None) ? CELL_GCM_TRUE : CELL_GCM_FALSE;
	cellGcmSetAntiAliasingControl(multisampleEnable, CELL_GCM_FALSE, CELL_GCM_FALSE, 0xffff);

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif

	#ifdef USE_GCM_REPLAY
	if (m_RenderTarget.m_pColourSurface && gcmSurface.colorFormat == CELL_GCM_SURFACE_A8R8G8B8) // this is the only format that can be captured
	{
		cell::Gcm::Replay::Capture::NotifyCaptureSurface(gCellGcmCurrentContext, &gcmSurface);
	}
	#endif
}

void C_GCMDevice::SetColourSurfaces(const C_Surface* pSurface0, const C_Surface* pSurface1, const C_Surface* pSurface2, const C_Surface* pSurface3)
{
	SetDepthEnable(E_Bool_False);
	SetDepthWriteEnable(E_Bool_False);
	SetStencilEnable(E_Bool_False);

	const C_Surface* pSurface = NULL;
	if(pSurface0)
	{
		pSurface = pSurface0;
	}
	else
	if(pSurface1)
	{
		pSurface = pSurface1;
	}
	else
	{
		LOGERROR(g_Log, "Either render target 0 or 1 must be set\n");
		return;
	}

	BIZ_ASSERT(pSurface);
	BIZ_ASSERT(pSurface->IsCreated());

	const U32 width					= pSurface->GetWidth();
	const U32 height				= pSurface->GetHeight();
	const E_Format format			= (E_Format)pSurface->GetFormat();
	const E_Multisample multisample	= (E_Multisample)pSurface->GetMultisample();
	
	U8 target = 0;
	if(pSurface0 && pSurface1 && pSurface2 && pSurface3)
	{
		target = CELL_GCM_SURFACE_TARGET_MRT3;
	}
	else
	if(pSurface0 && pSurface1 && pSurface2)
	{
		target = CELL_GCM_SURFACE_TARGET_MRT2;
	}
	else
	if(pSurface0 && pSurface1)
	{
		target = CELL_GCM_SURFACE_TARGET_MRT1;
	}
	else
	if(!pSurface0 && pSurface1)
	{
		target = CELL_GCM_SURFACE_TARGET_1;
	}
	else
	if(pSurface0 && !pSurface1)
	{
		target = CELL_GCM_SURFACE_TARGET_0;
	}
	else
	{
		target = CELL_GCM_SURFACE_TARGET_NONE;
	}

	CellGcmSurface gcmSurface;
	
	gcmSurface.type						= CELL_GCM_SURFACE_PITCH;
	gcmSurface.antialias				= GCMMultisample(multisample);
	gcmSurface.x 						= 0;
	gcmSurface.y 						= 0;
	gcmSurface.width 					= width;
	gcmSurface.height 					= height;
	gcmSurface.colorFormat 				= GCMSurfaceFormat(format);
	gcmSurface.colorTarget				= target;
	
	if(pSurface0)
	{
		BIZ_ASSERT(width == pSurface0->GetWidth());
		BIZ_ASSERT(height == pSurface0->GetHeight());
		BIZ_ASSERT(multisample == pSurface0->GetMultisample());

		gcmSurface.colorLocation[0]		= pSurface0->GetFlags() & E_Usage_System ? CELL_GCM_LOCATION_MAIN : CELL_GCM_LOCATION_LOCAL;
		gcmSurface.colorOffset[0] 		= pSurface0->GetOffset();
		gcmSurface.colorPitch[0] 		= pSurface0->GetPitch();
	}
	else
	{
		gcmSurface.colorLocation[0]		= CELL_GCM_LOCATION_LOCAL;
		gcmSurface.colorOffset[0] 		= 0;
		gcmSurface.colorPitch[0] 		= 64;
	}

	if(pSurface1)
	{
		BIZ_ASSERT(width == pSurface1->GetWidth());
		BIZ_ASSERT(height == pSurface1->GetHeight());
		BIZ_ASSERT(multisample == pSurface1->GetMultisample());

		gcmSurface.colorLocation[1]		= pSurface1->GetFlags() & E_Usage_System ? CELL_GCM_LOCATION_MAIN : CELL_GCM_LOCATION_LOCAL;
		gcmSurface.colorOffset[1] 		= pSurface1->GetOffset();
		gcmSurface.colorPitch[1] 		= pSurface1->GetPitch();
	}
	else
	{
		gcmSurface.colorLocation[1]		= CELL_GCM_LOCATION_LOCAL;
		gcmSurface.colorOffset[1] 		= 0;
		gcmSurface.colorPitch[1] 		= 64;
	}

	if(pSurface2)
	{
		BIZ_ASSERT(width == pSurface2->GetWidth());
		BIZ_ASSERT(height == pSurface2->GetHeight());
		BIZ_ASSERT(multisample == pSurface2->GetMultisample());

		gcmSurface.colorLocation[2]		= pSurface2->GetFlags() & E_Usage_System ? CELL_GCM_LOCATION_MAIN : CELL_GCM_LOCATION_LOCAL;
		gcmSurface.colorOffset[2] 		= pSurface2->GetOffset();
		gcmSurface.colorPitch[2] 		= pSurface2->GetPitch();
	}
	else
	{
		gcmSurface.colorLocation[2]		= CELL_GCM_LOCATION_LOCAL;
		gcmSurface.colorOffset[2] 		= 0;
		gcmSurface.colorPitch[2] 		= 64;
	}

	if(pSurface3)
	{
		BIZ_ASSERT(width == pSurface3->GetWidth());
		BIZ_ASSERT(height == pSurface3->GetHeight());
		BIZ_ASSERT(multisample == pSurface3->GetMultisample());

		gcmSurface.colorLocation[3]		= pSurface3->GetFlags() & E_Usage_System ? CELL_GCM_LOCATION_MAIN : CELL_GCM_LOCATION_LOCAL;
		gcmSurface.colorOffset[3] 		= pSurface3->GetOffset();
		gcmSurface.colorPitch[3] 		= pSurface3->GetPitch();
	}
	else
	{
		gcmSurface.colorLocation[3]		= CELL_GCM_LOCATION_LOCAL;
		gcmSurface.colorOffset[3] 		= 0;
		gcmSurface.colorPitch[3] 		= 64;
	}

	gcmSurface.depthFormat 				= CELL_GCM_SURFACE_Z24S8;
	gcmSurface.depthLocation			= CELL_GCM_LOCATION_LOCAL;
	gcmSurface.depthOffset				= 0;
	gcmSurface.depthPitch 				= 64;

	cellGcmSetSurface(&gcmSurface);

	const uint32_t multisampleEnable = pSurface->GetMultisample() != E_Multisample_None ? CELL_GCM_TRUE : CELL_GCM_FALSE;
	cellGcmSetAntiAliasingControl(multisampleEnable, CELL_GCM_FALSE, CELL_GCM_FALSE, 0xffff);

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

void C_GCMDevice::SetBackBuffer()
{
	SetColourSurface();
	SetDepthStencilSurface();
	SetSurface();
}

void C_GCMDevice::SetFrontBuffer()
{
	const C_FrontBuffer& frontBuffer = GetFrontBuffer();

	SetColourSurface(frontBuffer.GetSurface());
	SetDepthStencilSurface(NULL);
	SetSurface();
}

bool C_GCMDevice::CreateSurface(const U32 width, const U32 height, const U32 flags, const E_Format format, const E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface, U8* pAddress, const U32 addressSize)
{
	return m_Display2.CreateSurface(width, height, flags, Display2::GetSurfaceFormatFromTexture((Display2::E_TextureFormat)format), (Display2::E_Multisample)multisample, pParams, pSurface, pAddress, addressSize);
}

void C_GCMDevice::DestroySurface(C_Surface* pSurface)
{
	m_Display2.DestroySurface(pSurface);
}

bool C_GCMDevice::CreateTexture2D(const U32 width, const U32 height, const U32 levels, const U32 flags, const E_Format format, C_Texture2D* pTexture, U8* pAddress, const U32 addressSize)
{
	return m_Display2.CreateTexture2D(width, height, levels, flags, (Display2::E_TextureFormat)format, pTexture, pAddress, addressSize);
}

void C_GCMDevice::DestroyTexture2D(C_Texture2D* pTexture)
{
	m_Display2.DestroyTexture(pTexture);
}

bool C_GCMDevice::CreateTexture3D(const U32 width, const U32 height, const U32 depth, const U32 levels, const U32 flags, const E_Format format, C_Texture3D* pTexture, U8* pAddress, const U32 addressSize)
{
	return m_Display2.CreateTexture3D(width, height, depth, levels, flags, (Display2::E_TextureFormat)format, pTexture, pAddress, addressSize);
}

void C_GCMDevice::DestroyTexture3D(C_Texture3D* pTexture)
{
	m_Display2.DestroyTexture(pTexture);
}

bool C_GCMDevice::CreateTextureCube(const U32 width, const U32 levels, const U32 flags, const E_Format format, C_TextureCube* pTexture, U8* pAddress, const U32 addressSize)
{
	return m_Display2.CreateTextureCube(width, levels, flags, (Display2::E_TextureFormat)format, pTexture, pAddress, addressSize);
}

void C_GCMDevice::DestroyTextureCube(C_TextureCube* pTexture)
{
	m_Display2.DestroyTexture(pTexture);
}

bool C_GCMDevice::CreateRenderTexture2D(const U32 width, const U32 height, const U32 levels, U32 flags, const E_Format format, const E_Multisample multisample, const C_SurfaceParams* pParams, C_RenderTexture2D* pRenderTexture, U8* pAddress, const U32 addressSize)
{
#if 1
	const Display2::E_TextureFormat textureFormat = (Display2::E_TextureFormat)format;
	const Display2::E_SurfaceFormat surfaceFormat = Display2::GetSurfaceFormatFromTexture(textureFormat);
	return m_Display2.CreateRenderTexture2D(width, height, levels, flags, textureFormat, surfaceFormat, (Display2::E_Multisample)multisample, pParams, pRenderTexture, pAddress, addressSize);
#else
	BIZ_ASSERT(width > 0);
	BIZ_ASSERT(height > 0);
	BIZ_ASSERT(levels > 0);
	BIZ_ASSERT(pRenderTexture);
	BIZ_ASSERT(!pRenderTexture->IsCreated());

	if(!(flags & Display2::E_Flags_Colour) || !(flags & Display2::E_Flags_DepthStencil))
		flags |= Display2::E_Flags_Colour;
	
	pRenderTexture->m_Flags = flags;

	const U32 buffers = flags & E_Usage_ReadWrite ? 2 : 1;
	for(U32 buffer=0; buffer<buffers; buffer++)
	{
		const U32 textureWidth = Display2::GetTextureWidthFromMultisample(width, (Display2::E_Multisample)multisample);
		const U32 textureHeight = Display2::GetTextureHeightFromMultisample(height, (Display2::E_Multisample)multisample);
		const Display2::E_TextureFormat textureFormat = GetTextureFormatFromFormat(format);
		
		if(buffer == 1 && pAddress)
		{
			const U32 textureSize = Display2::GetTexture2DSize(textureWidth, textureHeight, levels, flags, textureFormat);
			BIZ_ASSERT(textureSize == pRenderTexture->m_Texture[0].GetSize());
			pAddress += textureSize;
		}
		
		C_Texture& texture = pRenderTexture->m_Texture[buffer];
		if(CreateTexture2D(textureWidth, textureHeight, levels, flags, format, &texture, pAddress, addressSize))
		{
			U32 textureLevelWidth = textureWidth;
			U32 textureLevelHeight = textureHeight;
			
			U32 surfaceLevelWidth = width;
			U32 surfaceLevelHeight = height;
			const U32 surfaceFlags = flags | Display2::E_Flags_Indirect;
			const Display2::E_SurfaceFormat surfaceFormat = GetSurfaceFormatFromFormat(format);
			const U32 surfaceSize = Display2::GetSurfaceSize(width, height, flags, surfaceFormat, (Display2::E_Multisample)multisample);
			U8* pSurfaceLevelAddress = texture.GetAddress();

			BIZ_ASSERT(pRenderTexture->m_Surfaces[buffer].Size() == 0);
			pRenderTexture->m_Surfaces[buffer].Resize(levels);

			for(U32 level=0; level<levels; level++)
			{
				C_Surface& surface = pRenderTexture->m_Surfaces[buffer][level];

				const U32 textureLevelSize = Display2::GetTexture2DSize(textureLevelWidth, textureLevelHeight, flags, textureFormat);
				const U32 surfaceLevelSize = Display2::GetSurfaceSize(surfaceLevelWidth, surfaceLevelHeight, flags, surfaceFormat, (Display2::E_Multisample)multisample);
				BIZ_ASSERT(textureLevelSize == surfaceLevelSize);

				if(!CreateColourSurface(surfaceLevelWidth, surfaceLevelHeight, surfaceFlags, format, multisample, NULL, &surface, pSurfaceLevelAddress, surfaceLevelSize))
					return false;

				pSurfaceLevelAddress += surfaceLevelSize;

				if(textureLevelWidth > 1)
					textureLevelWidth /= 2;
				if(textureLevelHeight > 1)
					textureLevelHeight /= 2;

				if(surfaceLevelWidth > 1)
					surfaceLevelWidth /= 2;
				if(surfaceLevelHeight > 1)
					surfaceLevelHeight /= 2;
			}

			BIZ_ASSERT(pSurfaceLevelAddress == texture.GetAddress()+texture.GetSize());
		}
		else
		{
			return false;
		}
	}

	pRenderTexture->m_TextureIndex = buffers == 2 ? 1 : 0;
	pRenderTexture->m_SurfaceIndex = 0;

	#if TRACK_DISPLAY2_RESOURCES
	if(!(flags & Display2::E_Flags_DoNotTrack))
		m_Display2.InsertRenderTexture2D(pRenderTexture);
	#endif
#endif
	
	return true;
}

void C_GCMDevice::DestroyRenderTexture2D(C_RenderTexture2D* pRenderTexture)
{
#if 1
	m_Display2.DestroyRenderTexture2D(pRenderTexture);
#else
	BIZ_ASSERT(pRenderTexture);

	const U32 buffers = pRenderTexture->m_Flags & E_Usage_ReadWrite ? 2 : 1;
	for(U32 buffer=0; buffer<buffers; buffer++)
	{
		for(S32 level=0; level<pRenderTexture->m_Surfaces[buffer].Size(); level++)
		{
			C_Surface& surface = pRenderTexture->m_Surfaces[buffer][level];

			DestroySurface(&surface);
		}

		pRenderTexture->m_Surfaces[buffer].Clear();

		DestroyTexture2D(&pRenderTexture->m_Texture[buffer]);
	}

	pRenderTexture->m_TextureIndex = 0;
	pRenderTexture->m_SurfaceIndex = 0;

	#if TRACK_DISPLAY2_RESOURCES
	if(!(pRenderTexture->m_Flags & Display2::E_Flags_DoNotTrack))
		m_Display2.RemoveRenderTexture2D(pRenderTexture);
	#endif

	pRenderTexture->m_Flags = 0;
#endif
}

bool C_GCMDevice::CreateRenderTextureCube(const U32 width, const U32 levels, U32 flags, const E_Format format, const E_Multisample multisample, const C_SurfaceParams* pParams, C_RenderTextureCube* pRenderTexture, U8* pAddress, const U32 addressSize)
{
	BIZ_ASSERT(width > 0);
	BIZ_ASSERT(levels > 0);
	BIZ_ASSERT(multisample != E_Multisample_2);
	BIZ_ASSERT(pRenderTexture);
	BIZ_ASSERT(!pRenderTexture->IsCreated());
	
	if(!(flags & Display2::E_Flags_Colour) || !(flags & Display2::E_Flags_DepthStencil))
		flags |= Display2::E_Flags_Colour;
	
	pRenderTexture->m_Flags = flags;

	const U32 textureWidth = Display2::GetTextureWidthFromMultisample(width, (Display2::E_Multisample)multisample);
	const Display2::E_TextureFormat textureFormat = GetTextureFormatFromFormat(format);
	const U32 textureSize = Display2::GetTexture2DSize(textureWidth, textureWidth, flags, textureFormat);
	
	C_Texture& texture = pRenderTexture->m_Texture;
	if(!CreateTextureCube(textureWidth, levels, flags, format, &texture, pAddress, addressSize))
		return false;

	const U32 faceSize = ALIGN_SIZE(Display2::GetTexture2DSize(textureWidth, textureWidth, levels, flags, textureFormat), 128);

	for(U32 face=0; face<6; face++)
	{
		U32 textureLevelWidth = textureWidth;

		U32 surfaceLevelWidth = width;
		const U32 surfaceFlags = flags | Display2::E_Flags_Indirect;
		const Display2::E_SurfaceFormat surfaceFormat = GetSurfaceFormatFromFormat(format);
		const U32 surfaceSize = Display2::GetSurfaceSize(width, width, flags, surfaceFormat, (Display2::E_Multisample)multisample);
		U8* pSurfaceLevelAddress = texture.GetAddress()+(face*faceSize);

		BIZ_ASSERT(textureSize == surfaceSize);
		
		BIZ_ASSERT(pRenderTexture->m_Surfaces[face].Size() == 0);
		pRenderTexture->m_Surfaces[face].Resize(levels);

		for(U32 level=0; level<levels; level++)
		{
			C_Surface& surface = pRenderTexture->m_Surfaces[face][level];

			const U32 textureLevelSize = Display2::GetTexture2DSize(textureLevelWidth, textureLevelWidth, flags, textureFormat);
			const U32 surfaceLevelSize = Display2::GetSurfaceSize(surfaceLevelWidth, surfaceLevelWidth, flags, surfaceFormat, (Display2::E_Multisample)multisample);
			BIZ_ASSERT(textureLevelSize == surfaceLevelSize);

			if(!CreateColourSurface(surfaceLevelWidth, surfaceLevelWidth, surfaceFlags, format, multisample, NULL, &surface, pSurfaceLevelAddress, surfaceLevelSize))
				return false;

			pSurfaceLevelAddress += surfaceLevelSize;

			if(textureLevelWidth > 1)
				textureLevelWidth /= 2;

			if(surfaceLevelWidth > 1)
				surfaceLevelWidth /= 2;
		}

		BIZ_ASSERT(pSurfaceLevelAddress == texture.GetAddress()+(face*faceSize)+textureSize);
	}

	#if TRACK_DISPLAY2_RESOURCES
	if(!(flags & Display2::E_Flags_DoNotTrack))
		m_Display2.InsertRenderTextureCube(pRenderTexture);
	#endif

	return true;
}

void C_GCMDevice::DestroyRenderTextureCube(C_RenderTextureCube* pRenderTexture)
{
	BIZ_ASSERT(pRenderTexture);
	
	for(U32 face=0; face<6; face++)
	{
		for(U32 level=0; level<pRenderTexture->m_Surfaces[face].Size(); level++)
		{
			C_Surface& surface = pRenderTexture->m_Surfaces[face][level];

			DestroySurface(&surface);
		}

		pRenderTexture->m_Surfaces[face].Clear();
	}

	DestroyTextureCube(&pRenderTexture->m_Texture);

	#if TRACK_DISPLAY2_RESOURCES
	if(!(pRenderTexture->m_Flags & Display2::E_Flags_DoNotTrack))
		m_Display2.RemoveRenderTextureCube(pRenderTexture);
	#endif
	
	pRenderTexture->m_Flags = 0;
}


void C_GCMDevice::DestroyRenderBuffer(C_RenderBuffer* pRenderBuffer)
{
	m_Display2.DestroyRenderBuffer(pRenderBuffer);
}

C_VertexDeclaration* C_GCMDevice::GetVertexDeclaration(const S32 vertexElementCount, const C_VertexElement* pVertexElements)
{
	return m_Display2.FindVertexDeclaration(vertexElementCount, pVertexElements);
}

C_VertexDeclaration* C_GCMDevice::InitVertexDeclaration(const S32 vertexElementCount, const C_VertexElement* pVertexElements)
{
	return m_Display2.CreateVertexDeclaration(pVertexElements);
}

bool C_GCMDevice::CreateVertexBuffer(const U32 size, const U32 flags, C_VertexBuffer* pVertexBuffer, bool allowFail)
{
	return m_Display2.CreateVertexBuffer(size, flags, pVertexBuffer);
}

void C_GCMDevice::DestroyVertexBuffer(C_VertexBuffer* pVertexBuffer)
{
	m_Display2.DestroyVertexBuffer(pVertexBuffer);
}

bool C_GCMDevice::CreateIndexBuffer(const U32 size, const U32 flags, const E_Format format, C_IndexBuffer* pIndexBuffer, bool allowFail)
{
	return m_Display2.CreateIndexBuffer(size, flags, (Display2::E_IndexFormat)format, pIndexBuffer);
}

void C_GCMDevice::DestroyIndexBuffer(C_IndexBuffer* pIndexBuffer)
{
	m_Display2.DestroyIndexBuffer(pIndexBuffer);
}

bool C_GCMDevice::CreateVertexShader(const U32 size, const S32* pData, const U32 flags, C_VertexShader* pVertexShader)
{
	return m_Display2.CreateVertexShader(size, (const U8*)pData, flags, pVertexShader);
}

void C_GCMDevice::DestroyVertexShader(C_VertexShader* pVertexShader)
{
	m_Display2.DestroyVertexShader(pVertexShader);
}

void C_GCMDevice::SetVertexShader(const C_VertexShader* pVertexShader)
{
	BIZ_ASSERT(HaveThreadOwnership());

#if 1
	m_Display2.GetContextNoCheckNoTrack().SetVertexShader(pVertexShader);
#else
	if(pVertexShader != m_pVertexShader)
	{
		if(pVertexShader)
		{
			// Set the default constants.
			uint32_t count = cellCgbGetVertexConstantCount(&pVertexShader->m_cgbProgram);
			uint32_t i = 0;
			const float* value = 0;
			uint16_t reg = 0;
			while(i < count)
			{
				cellCgbGetVertexConstantValues(&pVertexShader->m_cgbProgram, i, &reg, &value);
				cellGcmSetVertexProgramParameterBlock(reg, 1, value);
				i++;
			}
			
			if (!g_debugDisplayDeviceUseVertexShaderCache)
			{
				cellGcmSetVertexProgramLoad(&pVertexShader->m_cgbProgramConfig, pVertexShader->m_uCodeData);
			}
			else
			{
				U32 slotNo = m_VertexShaderCache.Allocate(pVertexShader);
				cellGcmCgSetInstructionSlotUCode(slotNo, 0, pVertexShader->m_cgbProgramConfig.instructionCount, pVertexShader->m_uCodeData);
				cellGcmSetVertexProgramRegisterCount(pVertexShader->m_cgbProgramConfig.registerCount);
				cellGcmSetVertexProgramLoadSlot(slotNo, pVertexShader->m_cgbProgramConfig.instructionCount, pVertexShader->m_uCodeData);
				cellGcmSetVertexProgramStartSlot(slotNo);
			}
		}

		m_pVertexShader = pVertexShader;
	}

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
#endif
}

bool C_GCMDevice::CreatePixelShader(const U32 size, const S32* pData, const U32 flags, C_PixelShader* pPixelShader)
{
	return m_Display2.CreatePixelShader(size, (const U8*)pData, flags, pPixelShader);
}

void C_GCMDevice::DestroyPixelShader(C_PixelShader* pPixelShader)
{
	m_Display2.DestroyPixelShader(pPixelShader);
}

void C_GCMDevice::SetupViewIndex(const S32 viewIndex, const C_ViewSetup& viewSetup)
{
	BIZ_ASSERT(viewIndex >= 0 && viewIndex < m_Views.Size());
	m_Views[viewIndex]->Setup(viewSetup, m_RenderTarget.GetWidth(), m_RenderTarget.GetHeight());
}

void C_GCMDevice::SetupViewIndex(const S32 viewIndex, const C_ViewSetup& viewSetup, const U32 width, const U32 height)
{
	BIZ_ASSERT(viewIndex >= 0 && viewIndex < m_Views.Size());
	m_Views[viewIndex]->Setup(viewSetup, width, height);
}

void C_GCMDevice::SetViewIndex(const S32 viewIndex, const bool setStates)
{
	if(viewIndex >= 0 && viewIndex < m_Views.Size())
	{
		SetView(m_Views[viewIndex], setStates);
	}
	else
	{
		SetView(NULL);
	}
}

void C_GCMDevice::SetupView(C_View* pView, const C_ViewSetup& viewSetup)
{
	BIZ_ASSERT(pView);
	pView->Setup(viewSetup, m_RenderTarget.GetWidth(), m_RenderTarget.GetHeight());
}

void C_GCMDevice::SetupView(C_View* pView, const C_ViewSetup& viewSetup, const U32 width, const U32 height)
{
	BIZ_ASSERT(pView);
	pView->Setup(viewSetup, width, height);
}

void C_GCMDevice::SetView(const C_View* pView, const bool setStates)
{
	if(pView)
	{
		m_pView = pView;

		SetViewport(m_pView->GetViewport());

		SetDepthFunc(E_CmpFunc_Default);

		SetViewShaderConstants2D();
		SetViewShaderConstants3D();

		#ifdef DEBUG_SHOW_CLIP_TRANSFORM
		if(g_debugDisplayDebugShowClipTransform)
		{
			const C_ViewSettings2D& viewSettings2D = pView->GetViewSettings2D();
			LOGINFO(g_Log, "Clip transform %f %f %f %f\n",
				viewSettings2D.GetClipTransform().x,
				viewSettings2D.GetClipTransform().y,
				viewSettings2D.GetClipTransform().z,
				viewSettings2D.GetClipTransform().w);
			g_debugDisplayDebugShowClipTransform = false;
		}
		#endif
	}
	else
	{
		if(setStates)
		{
			SetViewShaderConstants2D();
		}
	}
}

void C_GCMDevice::SetViewShaderConstants()
{
	SetViewShaderConstants2D();
	SetViewShaderConstants3D();
}

void C_GCMDevice::SetViewShaderConstants2D()
{
	if(m_pView)
	{
		const C_ViewSettings2D& viewSettings2D = m_pView->GetViewSettings2D();
		cellGcmSetVertexProgramConstants(CV_DISPLAY_CLIP_VECTOR,		1 * 4, (F32*)&viewSettings2D.GetClipTransform());
	}
	else
	{
		const C_Float4 clipTransform(0.001953125f, -0.00260416674f, -1.f, 1.f);
		cellGcmSetVertexProgramConstants(CV_DISPLAY_CLIP_VECTOR,		1 * 4, (F32*)&clipTransform);
	}
}

void C_GCMDevice::SetViewShaderConstants3D()
{
	if(m_pView)
	{
		const C_ViewSettings3D& viewSettings3D = m_pView->GetViewSettings3D();
		cellGcmSetVertexProgramConstants(CV_DISPLAY_PROJ_MATRIX,		4 * 4, (F32*)&viewSettings3D.m_ProjMatrix);
		cellGcmSetVertexProgramConstants(CV_DISPLAY_VIEW_MATRIX,		4 * 4, (F32*)&viewSettings3D.m_ViewMatrix);
		cellGcmSetVertexProgramConstants(CV_DISPLAY_VIEWPROJ_MATRIX,	4 * 4, (F32*)&viewSettings3D.m_ViewProjMatrix);
		cellGcmSetVertexProgramConstants(CV_DISPLAY_VIEW_POSITION,		1 * 4, (F32*)&viewSettings3D.m_ViewPosition);
	}
}

const U32 C_GCMDevice::GetMemoryAlignment(const E_ResourceType type, const U32 flags)
{
	switch(type)
	{
	case E_ResourceType_Surface:
		return GCM_ALIGNMENT_SURFACE;
	case E_ResourceType_Texture2D:
		return GCM_ALIGNMENT_TEXTURE2D;
	case E_ResourceType_Texture3D:
		return GCM_ALIGNMENT_TEXTURE3D;
	case E_ResourceType_TextureCube:
		return GCM_ALIGNMENT_TEXTURECUBE;
	case E_ResourceType_VertexBuffer:
		return GCM_ALIGNMENT_VERTEXBUFFER;
	case E_ResourceType_IndexBuffer:
		return GCM_ALIGNMENT_INDEXBUFFER;
	case E_ResourceType_VertexShader:
		return flags & E_Usage_System ? 128 : GCM_ALIGNMENT_VERTEXSHADER;
	case E_ResourceType_PixelShader:
		return flags & E_Usage_System ? 128 : GCM_ALIGNMENT_PIXELSHADER;
	default:
		BIZ_ASSERT_FAIL("Unsupported resource type");
		return 0;
	}
}

bool C_GCMDevice::AllocResourceMemory(const E_ResourceType type, U32& flags, const U32 size, U8** ppAddress, const S32 systemID, E_AllocType allocType)
{
#if 1
	return m_Display2.AllocResourceMemory(ToResourceType2(type), flags, size, ppAddress);
#else
	#ifndef CFG_SPEED
	if(MemoryGetSystemID() == 1)
	{
		LOGWARN(g_Log, "Allocating unknown graphics memory of size %i type %i\n", size, type);
	}
	#endif
	
	E_MemoryType memoryType;

	switch(type)
	{
	case E_ResourceType_Surface:
		allocType			= allocType != E_AllocType_Default ? allocType : E_AllocType_Start;
		memoryType			= E_MemoryType_Surface;
		break;
	case E_ResourceType_Texture2D:
		allocType			= allocType != E_AllocType_Default ? allocType : E_AllocType_Start;
		memoryType			= E_MemoryType_Texture2D;
		break;
	case E_ResourceType_Texture3D:
		allocType			= allocType != E_AllocType_Default ? allocType : E_AllocType_Start;
		memoryType			= E_MemoryType_Texture3D;
		break;
	case E_ResourceType_TextureCube:
		allocType			= allocType != E_AllocType_Default ? allocType : E_AllocType_Start;
		memoryType			= E_MemoryType_TextureCube;
		break;
	case E_ResourceType_VertexBuffer:
		allocType			= allocType != E_AllocType_Default ? allocType : E_AllocType_End;
		memoryType			= E_MemoryType_VertexBuffer;
		break;
	case E_ResourceType_IndexBuffer:
		allocType			= allocType != E_AllocType_Default ? allocType : E_AllocType_End;
		memoryType			= E_MemoryType_IndexBuffer;
		break;
	case E_ResourceType_VertexShader:
		allocType			= allocType != E_AllocType_Default ? allocType : E_AllocType_Start;
		memoryType			= E_MemoryType_VertexShader;
		break;
	case E_ResourceType_PixelShader:
		allocType			= allocType != E_AllocType_Default ? allocType : E_AllocType_Start;
		memoryType			= E_MemoryType_PixelShader;
		break;
	default:
		BIZ_ASSERT_FAIL("Unsupported resource type");
		return false;
	}

	U32 memoryAlign = GetMemoryAlignment(type, flags);

	if(flags & E_Usage_System)
	{
		// We want to alloc out of system memory.

		if(AllocMemory(memoryAlign, size, ppAddress, E_AllocHeap_System, allocType))
		{
			flags |= E_Usage_System;
			flags &= ~E_Usage_Video;
		}
		else
		{
			// We failed to alloc out of system memory so lets try video memory.

			memoryAlign = GetMemoryAlignment(type, E_Usage_Video);

			if(AllocMemory(memoryAlign, size, ppAddress, E_AllocHeap_Video, allocType))
			{
				flags &= ~E_Usage_System;
				flags |= E_Usage_Video;
			}
			else
			{
				return false; // We failed to alloc any graphics memory...this is bad!
			}
		}
	}
	else
	{
		// We want to alloc out of video memory.

		if(AllocMemory(memoryAlign, size, ppAddress, E_AllocHeap_Video, allocType))
		{
			flags &= ~E_Usage_System;
			flags |= E_Usage_Video;
		}
		else
		{
			// We failed to alloc out of video memory so lets try system memory.

			memoryAlign = GetMemoryAlignment(type, E_Usage_System);

			if(AllocMemory(memoryAlign, size, ppAddress, E_AllocHeap_System, allocType))
			{
				flags |= E_Usage_System;
				flags &= ~E_Usage_Video;
			}
			else
			{
				return false; // We failed to alloc any graphics memory...this is bad!
			}
		}
	}

	#ifdef USE_DISPLAY_MEMORY_TRACKER
	MemoryIncSystem(systemID, size, memoryType);
	#endif
	
	return true;
#endif
}

void C_GCMDevice::FreeResourceMemory(const E_ResourceType type, const U32 flags, const U32 size, U8* pAddress, const S32 systemID)
{
#if 1
	Display2::E_MemoryType memoryType;
	switch(type)
	{
	case E_ResourceType_Surface:		memoryType = Display2::E_MemoryType_Surface; 		break;
	case E_ResourceType_Texture2D:		memoryType = Display2::E_MemoryType_Texture; 		break;
	case E_ResourceType_Texture3D:		memoryType = Display2::E_MemoryType_Texture; 		break;
	case E_ResourceType_TextureCube:	memoryType = Display2::E_MemoryType_Texture; 		break;
	case E_ResourceType_VertexBuffer:	memoryType = Display2::E_MemoryType_VertexBuffer;	break;
	case E_ResourceType_IndexBuffer:	memoryType = Display2::E_MemoryType_IndexBuffer;	break;
	case E_ResourceType_VertexShader:	memoryType = Display2::E_MemoryType_VertexShader;	break;
	case E_ResourceType_PixelShader:	memoryType = Display2::E_MemoryType_PixelShader;	break;
	default:							memoryType = Display2::E_MemoryType_Unknown;		break;
	}
	m_Display2.FreeMemory(size, pAddress, memoryType);
#else
	E_MemoryType memoryType;

	switch(type)
	{
	case E_ResourceType_Surface:
		memoryType			= E_MemoryType_Surface;
		break;
	case E_ResourceType_Texture2D:
		memoryType			= E_MemoryType_Texture2D;
		break;
	case E_ResourceType_Texture3D:
		memoryType			= E_MemoryType_Texture3D;
		break;
	case E_ResourceType_TextureCube:
		memoryType			= E_MemoryType_TextureCube;
		break;
	case E_ResourceType_VertexBuffer:
		memoryType			= E_MemoryType_VertexBuffer;
		break;
	case E_ResourceType_IndexBuffer:
		memoryType			= E_MemoryType_IndexBuffer;
		break;
	case E_ResourceType_VertexShader:
		memoryType			= E_MemoryType_VertexShader;
		break;
	case E_ResourceType_PixelShader:
		memoryType			= E_MemoryType_PixelShader;
		break;
	default:
		BIZ_ASSERT_FAIL("Unsupported resource type");
		return;
	}

	if(flags & E_Usage_System)
	{
		FreeMemory(size, pAddress, E_AllocHeap_System);
	}
	else
	{
		FreeMemory(size, pAddress, E_AllocHeap_Video);
	}

	#ifdef USE_DISPLAY_MEMORY_TRACKER
	MemoryDecSystem(systemID, size, memoryType);
	#endif
#endif
}

bool C_GCMDevice::AllocMemory(const U32 align, const U32 size, U8** ppAddress, const E_AllocHeap allocHeap, const E_AllocType allocType, const Display2::E_MemoryType memoryType)
{
#if 1
	U32 flags = 0;
	switch(allocHeap)
	{
	case E_AllocHeap_Video:
		flags |= Display2::E_Flags_Video;
		break;
	case E_AllocHeap_System:
	case E_AllocHeap_Main:
		flags |= Display2::E_Flags_System;
		break;
	default:
		BIZ_ASSERT_FAIL("Unsupported alloc heap");
		return false;
	}
	return m_Display2.AllocMemory(size, align, flags, ppAddress, memoryType);
#else
	C_MemoryBuffer* pHeap = NULL;

	switch(allocHeap)
	{
	case E_AllocHeap_Video:
		pHeap = &m_VideoMemory;
		break;
	case E_AllocHeap_System:
		pHeap = &m_SystemMemory;
		break;
	case E_AllocHeap_Main:
		pHeap = &m_SystemMemory;
		break;
	default:
		BIZ_ASSERT_FAIL("Unsupported alloc heap");
		return false;
	}

	bool success = false;

	switch(allocType)
	{
	case E_AllocType_Default:
	case E_AllocType_Start:
		success = pHeap->AllocateFromStart(align, size, ppAddress);
		break;
	case E_AllocType_End:
		success = pHeap->AllocateFromEnd(align, size, ppAddress);
		break;
	default:
		BIZ_ASSERT_FAIL("Unsupported alloc type");
		break;
	}

	return success;
#endif
}

void C_GCMDevice::FreeMemory(const U32 size, U8* pAddress, const E_AllocHeap allocHeap, const Display2::E_MemoryType memoryType)
{
#if 1
	m_Display2.FreeMemory(size, pAddress, memoryType);
#else
	if(!pAddress)
		return;

	BIZ_ASSERT(size > 0);

	C_MemoryBuffer* pHeap = NULL;

	switch(allocHeap)
	{
	case E_AllocHeap_Video:
		pHeap = &m_VideoMemory;
		break;
	case E_AllocHeap_System:
		pHeap = &m_SystemMemory;
		break;
	case E_AllocHeap_Main:
		pHeap = &m_SystemMemory;
		break;
	default:
		BIZ_ASSERT_FAIL("Unsupported alloc heap");
		return;
	}

	pHeap->Free(size, pAddress);
#endif
}

CellGcmTexture* C_GCMDevice::AllocTextureHeader()
{
	#ifdef USE_GCM_TEXTURE_POOL
	return g_gcmTexturePool.Allocate();
	#else
	return new CellGcmTexture;
	#endif
}

void C_GCMDevice::FreeTextureHeader(CellGcmTexture*& pgcmTexture)
{
	if(pgcmTexture)
	{
		#ifdef USE_GCM_TEXTURE_POOL
		g_gcmTexturePool.Free(pgcmTexture);
		#else
		delete pgcmTexture;
		#endif
		pgcmTexture = NULL;
	}
}

void C_GCMDevice::ResolveSurface(const C_Surface* pSurface, const C_RenderTexture2D* pTarget)
{
	BIZ_ASSERT(pSurface);
	BIZ_ASSERT(pTarget);

	const C_Surface *pColourSurface = m_RenderTarget.GetColourSurface();

	SetColourSurface(&pTarget->GetSurface());
	SetSurface();

	SetDepthEnable(E_Bool_False);

	CellGcmTexture gcmTexture;

	const U32 gcmFormat = GCMTextureFormat((E_Format)pSurface->GetFormat());

	gcmTexture.mipmap		= 1;
	gcmTexture.dimension	= CELL_GCM_TEXTURE_DIMENSION_2;
    gcmTexture.cubemap		= CELL_GCM_FALSE;
	gcmTexture.width		= pSurface->GetWidth();
	gcmTexture.height		= pSurface->GetHeight();
	gcmTexture.depth		= 1;
	gcmTexture.pitch		= pSurface->GetPitch();
	gcmTexture.location		= pSurface->GetFlags() & E_Usage_System ? CELL_GCM_LOCATION_MAIN : CELL_GCM_LOCATION_LOCAL;
	gcmTexture.offset		= pSurface->GetOffset();

	gcmTexture.format =
		gcmFormat |
		CELL_GCM_TEXTURE_LN;
    
	gcmTexture.remap =
		CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
		CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
		CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
		CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
		CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
		CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
		CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
		CELL_GCM_TEXTURE_REMAP_FROM_A;
	
	switch(pSurface->GetMultisample())
	{
	case Display::E_Multisample_2:
		gcmTexture.width *= 2;
		break;
	case Display::E_Multisample_4:
		gcmTexture.width *= 2;
		gcmTexture.height *= 2;
		break;
	}

	cellGcmSetInvalidateTextureCache(CELL_GCM_INVALIDATE_TEXTURE);
	
	cellGcmSetTexture(0, &gcmTexture);
	
	const C_TextureControl textureControl(
		CELL_GCM_TRUE,
		0<<8,
		0<<8,
		CELL_GCM_TEXTURE_MAX_ANISO_1);
	SetTextureControlCached(0, textureControl);
	
	const C_TextureAddress textureAddress(
		CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
		CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
		CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
		CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL,
		CELL_GCM_TEXTURE_ZFUNC_LESS,
		0);
	SetTextureAddressCached(0, textureAddress);
	
	const C_TextureFilter textureFilter(
		0,
         CELL_GCM_TEXTURE_NEAREST,
         CELL_GCM_TEXTURE_NEAREST,
         CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX);
	SetTextureFilterCached(0, textureFilter);
	
	SetBlendMode(E_BlendMode_Solid);

	Quad::DrawFullScreen(NULL);

	SetDepthEnable(E_Bool_True);

	SetColourSurface(pColourSurface);
	SetSurface();
}

void C_GCMDevice::ResolveDepthStencilSurface(const C_Surface* pSurface, const C_RenderTexture2D* pTarget)
{
	BIZ_ASSERT(pSurface);
	BIZ_ASSERT(pTarget);

	const C_Surface *pColourSurface = m_RenderTarget.GetColourSurface();

	SetColourSurface(&pTarget->GetSurface());
	SetSurface();

	SetDepthEnable(E_Bool_False);

	CellGcmTexture gcmTexture;

	const U32 gcmFormat = CELL_GCM_TEXTURE_A8R8G8B8;

	gcmTexture.mipmap		= 1;
	gcmTexture.dimension	= CELL_GCM_TEXTURE_DIMENSION_2;
    gcmTexture.cubemap		= CELL_GCM_FALSE;
	gcmTexture.width		= pSurface->GetWidth();
	gcmTexture.height		= pSurface->GetHeight();
	gcmTexture.depth		= 1;
	gcmTexture.pitch		= pSurface->GetPitch();
    gcmTexture.location		= pSurface->GetFlags() & E_Usage_System ? CELL_GCM_LOCATION_MAIN : CELL_GCM_LOCATION_LOCAL;
	gcmTexture.offset		= pSurface->GetOffset();

	gcmTexture.format =
		gcmFormat |
		CELL_GCM_TEXTURE_LN;
    
	gcmTexture.remap =
		CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
		CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
		CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
		CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
		CELL_GCM_TEXTURE_REMAP_FROM_G << 6 |
		CELL_GCM_TEXTURE_REMAP_FROM_R << 4 |
		CELL_GCM_TEXTURE_REMAP_FROM_A << 2 |
		CELL_GCM_TEXTURE_REMAP_FROM_B;
	
	switch(pSurface->GetMultisample())
	{
	case Display::E_Multisample_2:
		gcmTexture.width *= 2;
		break;
	case Display::E_Multisample_4:
		gcmTexture.width *= 2;
		gcmTexture.height *= 2;
		break;
	}

	cellGcmSetInvalidateTextureCache(CELL_GCM_INVALIDATE_TEXTURE);
	
	cellGcmSetTexture(0, &gcmTexture);
	
	const C_TextureControl textureControl(
		CELL_GCM_TRUE,
		0<<8,
		0<<8,
		CELL_GCM_TEXTURE_MAX_ANISO_1);
	SetTextureControlCached(0, textureControl);
	
	const C_TextureAddress textureAddress(
		CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
		CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
		CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
		CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL,
		CELL_GCM_TEXTURE_ZFUNC_LESS,
		0);
	SetTextureAddressCached(0, textureAddress);
	
	const C_TextureFilter textureFilter(
		0,
		CELL_GCM_TEXTURE_NEAREST,
		CELL_GCM_TEXTURE_NEAREST,
		CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX);
	SetTextureFilterCached(0, textureFilter);
	
	SetBlendMode(E_BlendMode_Solid);

	Quad::DrawFullScreen(NULL);

	SetDepthEnable(E_Bool_True);
	
	SetColourSurface(pColourSurface);
	SetSurface();
}

void C_GCMDevice::SetVertexDeclaration()
{
	static const U32 POSITION_INDEX		= 0;
	static const U32 NORMAL_INDEX		= 2;
	static const U32 COLOUR_INDEX		= 3;
	static const U32 TEXCOORD_INDEX		= 8;
	static const U32 TANGENT_INDEX		= 14;

	if(!m_SetVertexDeclaration)
		return;

	m_SetVertexDeclaration = false;

	for (int i=0; i<16; i++)
		cellGcmSetVertexDataArrayFormat(i, 0, 0, 0, CELL_GCM_VERTEX_F);

	const S32 vertexElementCount = m_pVertexDeclaration->m_ElementCount;
	for(S32 vertexElementIndex=0; vertexElementIndex<vertexElementCount; vertexElementIndex++)
	{
		const C_VertexElement& vertexElement = m_pVertexDeclaration->m_ElementArray[vertexElementIndex];
		
		switch(vertexElement.m_Usage)
		{
		case(E_VertexElementUsage_Position):
			{
				BIZ_ASSERT(vertexElement.m_Stream < E_Stream_Count);
				const C_Stream& vertexStream = m_Stream[vertexElement.m_Stream];
				BIZ_ASSERT(vertexElement.m_UsageIndex == 0);
				if(vertexStream.m_pVertexBuffer)
				{
					switch(vertexElement.m_Type)
					{
						case E_VertexElementType_F32_1:
						case E_VertexElementType_F32_2:
						case E_VertexElementType_F32_3:
						case E_VertexElementType_F32_4:
						{
							cellGcmSetVertexDataArray(
								POSITION_INDEX,
								0,
								vertexStream.m_VertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_F,
								Display2::FlagsToLocation(vertexStream.m_pVertexBuffer->GetFlags()),
								vertexStream.m_pVertexBuffer->GetOffset() + vertexStream.m_VertexOffset + vertexElement.m_Offset);
						}
						break;
						case E_VertexElementType_S16_2:
							{
								cellGcmSetVertexDataArray(
									POSITION_INDEX,
									0,
									vertexStream.m_VertexStride,
									vertexElement.GetCount(),
									CELL_GCM_VERTEX_S16_UN,
									Display2::FlagsToLocation(vertexStream.m_pVertexBuffer->GetFlags()),
									vertexStream.m_pVertexBuffer->GetOffset() + vertexStream.m_VertexOffset + vertexElement.m_Offset);
							}
							break;
						default: BIZ_ASSERT(0);
					}
				}
			}
			break;
		case E_VertexElementUsage_Normal:
			{
				BIZ_ASSERT(vertexElement.m_Stream < E_Stream_Count);
				const C_Stream& vertexStream = m_Stream[vertexElement.m_Stream];
				BIZ_ASSERT(vertexElement.m_UsageIndex == 0);
				if(vertexStream.m_pVertexBuffer)
				{
					switch(vertexElement.m_Type)
					{
					case E_VertexElementType_F32_3:
					case E_VertexElementType_F32_4:
						{
							cellGcmSetVertexDataArray(
								NORMAL_INDEX,
								0,
								vertexStream.m_VertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_F,
								Display2::FlagsToLocation(vertexStream.m_pVertexBuffer->GetFlags()),
								vertexStream.m_pVertexBuffer->GetOffset() + vertexStream.m_VertexOffset + vertexElement.m_Offset);
						}
						break;
					case E_VertexElementType_U10_3N:
						{
							BIZ_ASSERT(0);
						}
					case E_VertexElementType_SHHD_3:
					case E_VertexElementType_SHHD_3N:
						{
							cellGcmSetVertexDataArray(
								NORMAL_INDEX,
								0,
								vertexStream.m_VertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_CMP,
								Display2::FlagsToLocation(vertexStream.m_pVertexBuffer->GetFlags()),
								vertexStream.m_pVertexBuffer->GetOffset() + vertexStream.m_VertexOffset + vertexElement.m_Offset);
						}
						break;
					case E_VertexElementType_U8_4N:
						{
							cellGcmSetVertexDataArray(
								NORMAL_INDEX,
								0,
								vertexStream.m_VertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_UB,
								Display2::FlagsToLocation(vertexStream.m_pVertexBuffer->GetFlags()),
								vertexStream.m_pVertexBuffer->GetOffset() + vertexStream.m_VertexOffset + vertexElement.m_Offset);
						}
						break;
					default: BIZ_ASSERT(0);
					}
				}
			}
			break;
		case E_VertexElementUsage_Colour:
			{
				BIZ_ASSERT(vertexElement.m_Stream < E_Stream_Count);
				const C_Stream& vertexStream = m_Stream[vertexElement.m_Stream];
				BIZ_ASSERT(vertexElement.m_UsageIndex < 2);
				if(vertexStream.m_pVertexBuffer)
				{
					switch(vertexElement.m_Type)
					{
					case E_VertexElementType_F32_1:
					case E_VertexElementType_F32_2:
					case E_VertexElementType_F32_3:
					case E_VertexElementType_F32_4:
						{
							cellGcmSetVertexDataArray(
								COLOUR_INDEX + vertexElement.m_UsageIndex,
								0,
								vertexStream.m_VertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_F,
								Display2::FlagsToLocation(vertexStream.m_pVertexBuffer->GetFlags()),
								vertexStream.m_pVertexBuffer->GetOffset() + vertexStream.m_VertexOffset + vertexElement.m_Offset);
						}
						break;
					case E_VertexElementType_U8_4N:
					case E_VertexElementType_Colour:
						{
							cellGcmSetVertexDataArray(
								COLOUR_INDEX + vertexElement.m_UsageIndex,
								0,
								vertexStream.m_VertexStride,
								4,
								CELL_GCM_VERTEX_UB,
								Display2::FlagsToLocation(vertexStream.m_pVertexBuffer->GetFlags()),
								vertexStream.m_pVertexBuffer->GetOffset() + vertexStream.m_VertexOffset + vertexElement.m_Offset);
						}
						break;
					case E_VertexElementType_U8_4:
						{
							cellGcmSetVertexDataArray(
								COLOUR_INDEX + vertexElement.m_UsageIndex,
								0,
								vertexStream.m_VertexStride,
								4,
								CELL_GCM_VERTEX_UB256,
								Display2::FlagsToLocation(vertexStream.m_pVertexBuffer->GetFlags()),
								vertexStream.m_pVertexBuffer->GetOffset() + vertexStream.m_VertexOffset + vertexElement.m_Offset);
						}
						break;
					default: BIZ_ASSERT(0);
					}
				}
			}
			break;
		case E_VertexElementUsage_TexCoord:
			{
				BIZ_ASSERT(vertexElement.m_Stream < E_Stream_Count);
				const C_Stream& vertexStream = m_Stream[vertexElement.m_Stream];
				if(vertexStream.m_pVertexBuffer)
				{
					switch(vertexElement.m_Type)
					{
					case E_VertexElementType_F32_1:
					case E_VertexElementType_F32_2:
					case E_VertexElementType_F32_3:
					case E_VertexElementType_F32_4:
						{
							cellGcmSetVertexDataArray(
								TEXCOORD_INDEX + vertexElement.m_UsageIndex,
								0,
								vertexStream.m_VertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_F,
								Display2::FlagsToLocation(vertexStream.m_pVertexBuffer->GetFlags()),
								vertexStream.m_pVertexBuffer->GetOffset() + vertexStream.m_VertexOffset + vertexElement.m_Offset);
						}
						break;
					case E_VertexElementType_F16_2:
					case E_VertexElementType_F16_4:
						{
							cellGcmSetVertexDataArray(
								TEXCOORD_INDEX + vertexElement.m_UsageIndex,
								0,
								vertexStream.m_VertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_SF,
								Display2::FlagsToLocation(vertexStream.m_pVertexBuffer->GetFlags()),
								vertexStream.m_pVertexBuffer->GetOffset() + vertexStream.m_VertexOffset + vertexElement.m_Offset);
						}
						break;
					case E_VertexElementType_S16_2N:
						{
							cellGcmSetVertexDataArray(
								TEXCOORD_INDEX + vertexElement.m_UsageIndex,
								0,
								vertexStream.m_VertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_S1,
								Display2::FlagsToLocation(vertexStream.m_pVertexBuffer->GetFlags()),
								vertexStream.m_pVertexBuffer->GetOffset() + vertexStream.m_VertexOffset + vertexElement.m_Offset);
						}
						break;
					case E_VertexElementType_S16_2:
						{
							cellGcmSetVertexDataArray(
								TEXCOORD_INDEX + vertexElement.m_UsageIndex,
								0,
								vertexStream.m_VertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_S32K,
								Display2::FlagsToLocation(vertexStream.m_pVertexBuffer->GetFlags()),
								vertexStream.m_pVertexBuffer->GetOffset() + vertexStream.m_VertexOffset + vertexElement.m_Offset);
						}
						break;
					case E_VertexElementType_SHHD_3:
					case E_VertexElementType_SHHD_3N:
						{
							cellGcmSetVertexDataArray(
								TEXCOORD_INDEX + vertexElement.m_UsageIndex,
								0,
								vertexStream.m_VertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_CMP,
								Display2::FlagsToLocation(vertexStream.m_pVertexBuffer->GetFlags()),
								vertexStream.m_pVertexBuffer->GetOffset() + vertexStream.m_VertexOffset + vertexElement.m_Offset);
						}
						break;
					case E_VertexElementType_U8_4N:
						{
							cellGcmSetVertexDataArray(
								TEXCOORD_INDEX + vertexElement.m_UsageIndex,
								0,
								vertexStream.m_VertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_UB,
								Display2::FlagsToLocation(vertexStream.m_pVertexBuffer->GetFlags()),
								vertexStream.m_pVertexBuffer->GetOffset() + vertexStream.m_VertexOffset + vertexElement.m_Offset);
						}
						break;
					default: BIZ_ASSERT(0);
					}
				}
			}
			break;
		case E_VertexElementUsage_BlendIndices:
			{
				BIZ_ASSERT(vertexElement.m_Stream < E_Stream_Count);
				const C_Stream& vertexStream = m_Stream[vertexElement.m_Stream];
				if(vertexStream.m_pVertexBuffer)
				{
					cellGcmSetVertexDataArray(
						TEXCOORD_INDEX + 3,
						0,
						vertexStream.m_VertexStride,
						4,
						CELL_GCM_VERTEX_S32K,
						Display2::FlagsToLocation(vertexStream.m_pVertexBuffer->GetFlags()),
						vertexStream.m_pVertexBuffer->GetOffset() + vertexStream.m_VertexOffset + vertexElement.m_Offset);
				}
			}
			break;
		case E_VertexElementUsage_BlendWeight:
			{
				BIZ_ASSERT(vertexElement.m_Stream < E_Stream_Count);
				const C_Stream& vertexStream = m_Stream[vertexElement.m_Stream];
				if(vertexStream.m_pVertexBuffer)
				{
					cellGcmSetVertexDataArray(
						TEXCOORD_INDEX + 4,
						0,
						vertexStream.m_VertexStride,
						4,
						CELL_GCM_VERTEX_UB,
						Display2::FlagsToLocation(vertexStream.m_pVertexBuffer->GetFlags()),
						vertexStream.m_pVertexBuffer->GetOffset() + vertexStream.m_VertexOffset + vertexElement.m_Offset);
				}
			}
			break;
		case E_VertexElementUsage_Tangent:
			{
				BIZ_ASSERT(vertexElement.m_Stream < E_Stream_Count);
				const C_Stream& vertexStream = m_Stream[vertexElement.m_Stream];
				BIZ_ASSERT(vertexElement.m_UsageIndex == 0);
				if(vertexStream.m_pVertexBuffer)
				{
					switch(vertexElement.m_Type)
					{
					case E_VertexElementType_F32_3:
						{
							cellGcmSetVertexDataArray(
								TANGENT_INDEX,
								0,
								vertexStream.m_VertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_F,
								Display2::FlagsToLocation(vertexStream.m_pVertexBuffer->GetFlags()),
								vertexStream.m_pVertexBuffer->GetOffset() + vertexStream.m_VertexOffset + vertexElement.m_Offset);
						}
						break;
					case E_VertexElementType_U10_3N:
						{
							BIZ_ASSERT(0);
						}
					case E_VertexElementType_SHHD_3:
					case E_VertexElementType_SHHD_3N:
						{
							cellGcmSetVertexDataArray(
								TANGENT_INDEX,
								0,
								vertexStream.m_VertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_CMP,
								Display2::FlagsToLocation(vertexStream.m_pVertexBuffer->GetFlags()),
								vertexStream.m_pVertexBuffer->GetOffset() + vertexStream.m_VertexOffset + vertexElement.m_Offset);
						}
						break;
					case E_VertexElementType_U8_4N:
						{
							cellGcmSetVertexDataArray(
								TANGENT_INDEX,
								0,
								vertexStream.m_VertexStride,
								vertexElement.GetCount(),
								CELL_GCM_VERTEX_UB,
								Display2::FlagsToLocation(vertexStream.m_pVertexBuffer->GetFlags()),
								vertexStream.m_pVertexBuffer->GetOffset() + vertexStream.m_VertexOffset + vertexElement.m_Offset);
						}
						break;
					default: BIZ_ASSERT(0);
					}
				}
			}
			break;
		default:
			BIZ_ASSERT_FAIL("Unsupported vertex element usage");
			break;
		}
	}

	if(!m_InvalidateVertexCache)
		m_InvalidateVertexCache = true;
}

void C_GCMDevice::ClearWindows()
{
	DestroyWindows();

	for(S32 i=0; i<m_Windows.Size(); i++)
	{
		SAFE_DELETE_PTR(m_Windows[i]);
	}

	m_Windows.Clear();
}

const S32 C_GCMDevice::AddWindow(const Maths::C_Rect& windowCoords)
{
	C_Window* pWindow = new C_Window(windowCoords);
	m_Windows.PushBack(pWindow);
	return m_Windows.Size()-1;
}

void C_GCMDevice::ResizeWindows()
{
	DestroyWindows();
	DestroySurfaces();
	
	const S32 windowCount = m_Windows.Size();
	for(S32 windowIndex=0; windowIndex<windowCount; windowIndex++)
	{
		Display::C_Window* pWindow = m_Windows[windowIndex];

		pWindow->Resize(
			m_DeviceParams.m_WindowBufferWidth,
			m_DeviceParams.m_WindowBufferHeight,
			m_DeviceParams.m_WindowBufferMultisample,
			m_DeviceParams.m_WindowBufferColourFormat,
			m_DeviceParams.m_WindowBufferDepthStencilFormat);
	}
	
	CreateSurfaces();
	CreateWindows();
}

bool C_GCMDevice::CreateWindows()
{
	U32 renderBufferFlags = Display2::E_Flags_Device;

	Display::C_Window* pWindow = m_Windows[0];
	U32 width = pWindow->GetWidth();
	U32 height = pWindow->GetHeight();

	if(!m_Display2.CreateSharedTiledArea(m_WindowsColourTiledArea, width, height, renderBufferFlags | Display2::E_Flags_Video, Display2::E_SurfaceFormat_A8R8G8B8, GetMultisample2(m_DeviceParams.m_WindowBufferMultisample), m_Windows.Size(), "_SharedWindowColourBuffers"))
		return false;
	if(!m_Display2.CreateSharedTiledArea(m_WindowsDepthTiledArea, width, height, renderBufferFlags | Display2::E_Flags_Video | Display2::E_Flags_DepthStencil, Display2::E_SurfaceFormat_D24S8, GetMultisample2(m_DeviceParams.m_WindowBufferMultisample), m_Windows.Size(), "_SharedWindowDepthBuffers"))
		return false;

	for(S32 windowIndex=0; windowIndex<m_Windows.Size(); windowIndex++)
	{
		C_Window* pWindow = m_Windows[windowIndex];
				
		Display2::C_RenderBuffer* pRenderBuffer = &pWindow->m_RenderBuffer;
		pRenderBuffer->SetFlags(renderBufferFlags);

		Util::C_String renderBufferName = "";
		renderBufferName.Printf("_Window%i", windowIndex);
		pRenderBuffer->SetName(renderBufferName.Chars());

		pRenderBuffer->m_ColourTileInfo[0].enable		= true;
		pRenderBuffer->m_ColourTileInfo[0].indirect		= true;
		pRenderBuffer->OverlapColourBuffer(0, m_WindowsColourTiledArea.GetSize(), m_WindowsColourTiledArea.GetAddress(windowIndex));
		
		pRenderBuffer->m_DepthStencilTileInfo.enable	= true;
		pRenderBuffer->m_DepthStencilTileInfo.indirect	= true;
		pRenderBuffer->OverlapDepthStencilBuffer(m_WindowsDepthTiledArea.GetSize(), m_WindowsDepthTiledArea.GetAddress(windowIndex));
		
		pRenderBuffer->m_DepthStencilZcullInfo.enable	= true;
	
		Display2::C_RenderBufferSetup renderBufferSetup;
		if(!m_Display2.CreateRenderBuffer(pRenderBuffer, renderBufferSetup))
			return false;
	}

	if(m_Windows.Size() > 0)
	{
		C_Window* pWindow = m_Windows[0];

		CreateMainSharedRenderBuffer(
			pWindow->GetWidth(),
			pWindow->GetHeight(),
			E_Multisample_None,
			E_Format_A8R8G8B8,
			E_Format_Unknown);

		CreateLocalSharedRenderBuffer(
			pWindow->GetWidth(),
			pWindow->GetHeight(),
			E_Multisample_None,
			E_Format_A8R8G8B8,
			E_Format_D24S8);
	}

	return true;
}

void C_GCMDevice::DestroyWindows()
{
	for(S32 windowIndex=0; windowIndex<m_Windows.Size(); windowIndex++)
	{
		C_Window* pWindow = m_Windows[windowIndex];

		m_Display2.DestroyRenderBuffer(&pWindow->m_RenderBuffer);
	}
	m_Display2.DestroySharedTiledArea(m_WindowsColourTiledArea);
	m_Display2.DestroySharedTiledArea(m_WindowsDepthTiledArea);

	DestroyMainSharedRenderBuffer();

	DestroyLocalSharedRenderBuffer();
}

S32 C_GCMDevice::GetWindowIndex()
{
	return m_CurrentWindow;
}

void C_GCMDevice::SetWindow(const S32 windowIndex, const bool aa)
{
	if(windowIndex != -1)
	{
		C_Window* pWindow = GetWindow(windowIndex);

		SetColourDepthStencilSurface(pWindow->GetColourSurface(), pWindow->GetDepthStencilSurface());
	}

	m_CurrentWindow = windowIndex;
}

void C_GCMDevice::BeginWindow()
{
	SetZcullEnable(true, true); // Make sure zcull is on as we're about to render to the aa buffers
}

void C_GCMDevice::EndWindow()
{
}

void C_GCMDevice::RestoreWindow(const S32 windowIndex, const U32 colourWrite)
{
	if(AADisabled())
	{
		SetZcullEnable(true, true); // Make sure zcull is on as we're reusing the information from the aa buffers

		return;
	}
	
	BIZ_ASSERT(windowIndex != -1);

	C_Window* pWindow = GetWindow(windowIndex);

	SetBlendMode(E_BlendMode_Solid);

	if(g_debugDisplayDownsampleTransferSurface)
	{
		GPU_TIMING(4, Maths::RGBA(255, 255, 255, 255), "GPU.RestoreWindow.TransferSurface");

		Display::C_Texture* pColourTexture = pWindow->GetColourTexture();
		Display::C_Texture* pDepthStencilTexture = pWindow->GetDepthStencilTexture();

		cellGcmSetTransferScaleMode(CELL_GCM_TRANSFER_LOCAL_TO_LOCAL, CELL_GCM_TRANSFER_SURFACE);

		// downsample colour
		{
			const U32 dstWidth = m_Viewport.m_W;
			const U32 dstHeight = m_Viewport.m_H;
			const U32 dstPitch = m_RenderTarget.m_pColourSurface->GetPitch();
			const U32 dstOffset = m_RenderTarget.m_pColourSurface->GetOffset();

			const U32 srcWidth = pColourTexture->GetWidth();
			const U32 srcHeight = pColourTexture->GetHeight();
			const U32 srcPitch = pColourTexture->GetPitch();
			const U32 srcOffset = pColourTexture->GetOffset();

			const F32 xScaleInToOut = (F32)srcWidth / (F32)dstWidth;
			const F32 yScaleInToOut = (F32)srcHeight / (F32)dstHeight;

			CellGcmTransferScale colourTransferScale;
			colourTransferScale.conversion = 0;
			colourTransferScale.format = CELL_GCM_TRANSFER_SCALE_FORMAT_A8R8G8B8;
			colourTransferScale.operation = 0;
			colourTransferScale.clipX = m_Viewport.m_X;
			colourTransferScale.clipY = m_Viewport.m_Y;
			colourTransferScale.outX = m_Viewport.m_X;
			colourTransferScale.outY = m_Viewport.m_Y;
			colourTransferScale.outW = m_Viewport.m_W;
			colourTransferScale.outH = m_Viewport.m_H;
			colourTransferScale.inX = cellGcmGetFixedUint16(xScaleInToOut == 1.0f ? 0.0f : 0.5f);
			colourTransferScale.inY = cellGcmGetFixedUint16(yScaleInToOut == 1.0f ? 0.0f : 0.5f);
			colourTransferScale.ratioX = cellGcmGetFixedSint32(xScaleInToOut);
			colourTransferScale.ratioY = cellGcmGetFixedSint32(yScaleInToOut);
			colourTransferScale.pitch = srcPitch;
			colourTransferScale.origin = CELL_GCM_TRANSFER_ORIGIN_CENTER;
			colourTransferScale.interp = CELL_GCM_TRANSFER_INTERPOLATOR_FOH;

			CellGcmTransferSurface colourTransferSurface;
			colourTransferSurface.format = CELL_GCM_TRANSFER_SURFACE_FORMAT_A8R8G8B8;
			colourTransferSurface.pitch = dstPitch;

			// Transfer 1024x1024 tiles
			U32 srcMaxBlockWidth = 1024;
			U32 srcMaxBlockHeight = 1024;
			U32 dstMaxBlockWidth = (U32)(srcMaxBlockWidth / xScaleInToOut);
			U32 dstMaxBlockHeight = (U32)(srcMaxBlockHeight / yScaleInToOut);	
			if(xScaleInToOut < 1.0f)
			{
				srcMaxBlockWidth = (U32)(1024 * xScaleInToOut);
				dstMaxBlockWidth = 1024;
			}
			if(yScaleInToOut < 1.0f)
			{
				srcMaxBlockHeight = (U32)(1024 * yScaleInToOut);
				dstMaxBlockHeight = 1024;
			}
			for (U32 dstBlockY=0; dstBlockY<dstHeight; dstBlockY += dstMaxBlockHeight)
			{
				U32 dstBlockHeight = (dstHeight-dstBlockY < dstMaxBlockHeight) ? dstHeight-dstBlockY : dstMaxBlockHeight;

				for (U32 dstBlockX=0; dstBlockX<dstWidth; dstBlockX += dstMaxBlockWidth)
				{
					U32 dstBlockWidth  = (dstWidth-dstBlockX < dstMaxBlockWidth) ? dstWidth-dstBlockX : dstMaxBlockWidth;

					U32 srcBlockX = (U32)(dstBlockX * xScaleInToOut);
					U32 srcBlockY = (U32)(dstBlockY * yScaleInToOut);
					U32 srcBlockWidth = (U32)(dstBlockWidth * xScaleInToOut);
					U32 srcBlockHeight = (U32)(dstBlockHeight * yScaleInToOut);

					colourTransferScale.clipW = colourTransferScale.outW = dstBlockWidth;
					colourTransferScale.clipH = colourTransferScale.outH = dstBlockHeight;
					colourTransferScale.inW = srcBlockWidth;
					colourTransferScale.inH = srcBlockHeight;

					U32 dstBlockOffset = (dstBlockX * 4) + (dstBlockY * colourTransferSurface.pitch);
					U32 srcBlockOffset = (srcBlockX * 4) + (srcBlockY * colourTransferScale.pitch);
					colourTransferSurface.offset = (dstOffset + dstBlockOffset + 0x3F) & ~0x3F;
					colourTransferScale.offset = srcOffset + srcBlockOffset;

					BIZ_ASSERT(colourTransferSurface.format == CELL_GCM_TRANSFER_SURFACE_FORMAT_A8R8G8B8);
					BIZ_ASSERT(colourTransferSurface.pitch % 64 == 0);
					BIZ_ASSERT(colourTransferSurface.offset % 64 == 0);
					BIZ_ASSERT(colourTransferSurface.offset >= m_RenderTarget.m_pColourSurface->GetOffset() && colourTransferSurface.offset <= m_RenderTarget.m_pColourSurface->GetOffset()+m_RenderTarget.m_pColourSurface->GetSize());

					cellGcmSetTransferScaleSurface(&colourTransferScale, &colourTransferSurface);
				}
			}
		}

		// downsample depth stencil
		{
			const U32 dstWidth = m_Viewport.m_W;
			const U32 dstHeight = m_Viewport.m_H;
			const U32 dstPitch = m_RenderTarget.m_pDepthStencilSurface->GetPitch();
			const U32 dstOffset = m_RenderTarget.m_pDepthStencilSurface->GetOffset();

			const U32 srcWidth = pDepthStencilTexture->GetWidth();
			const U32 srcHeight = pDepthStencilTexture->GetHeight();
			const U32 srcPitch = pDepthStencilTexture->GetPitch();
			const U32 srcOffset = pDepthStencilTexture->GetOffset();

			const F32 xScaleInToOut = (F32)srcWidth / (F32)dstWidth;
			const F32 yScaleInToOut = (F32)srcHeight / (F32)dstHeight;

			CellGcmTransferScale depthTransferScale;
			depthTransferScale.conversion = 0;
			depthTransferScale.format = CELL_GCM_TRANSFER_SCALE_FORMAT_A8R8G8B8;
			depthTransferScale.operation = 0;
			depthTransferScale.clipX = 0;
			depthTransferScale.clipY = 0;
			depthTransferScale.outX = 0;
			depthTransferScale.outY = 0;
			depthTransferScale.outW = dstWidth;
			depthTransferScale.outH = dstHeight;
			depthTransferScale.inX = cellGcmGetFixedUint16(0.0f);
			depthTransferScale.inY = cellGcmGetFixedUint16(0.0f);
			depthTransferScale.ratioX = cellGcmGetFixedSint32(xScaleInToOut);
			depthTransferScale.ratioY = cellGcmGetFixedSint32(yScaleInToOut);
			depthTransferScale.pitch = srcPitch;
			depthTransferScale.origin = CELL_GCM_TRANSFER_ORIGIN_CORNER;
			depthTransferScale.interp = CELL_GCM_TRANSFER_INTERPOLATOR_ZOH;

			CellGcmTransferSurface depthTransferSurface;
			depthTransferSurface.format = CELL_GCM_TRANSFER_SURFACE_FORMAT_A8R8G8B8;
			depthTransferSurface.pitch = dstPitch;

			// Transfer 1024x1024 tiles
			U32 srcMaxBlockWidth = 1024;
			U32 srcMaxBlockHeight = 1024;
			U32 dstMaxBlockWidth = (U32)(srcMaxBlockWidth / xScaleInToOut);
			U32 dstMaxBlockHeight = (U32)(srcMaxBlockHeight / yScaleInToOut);
			if(xScaleInToOut < 1.0f)
			{
				srcMaxBlockWidth = (U32)(1024 * xScaleInToOut);
				dstMaxBlockWidth = 1024;
			}
			if(yScaleInToOut < 1.0f)
			{
				srcMaxBlockHeight = (U32)(1024 * yScaleInToOut);
				dstMaxBlockHeight = 1024;
			}
			for (U32 dstBlockY=0; dstBlockY<dstHeight; dstBlockY += dstMaxBlockHeight)
			{
				U32 dstBlockHeight = (dstHeight-dstBlockY < dstMaxBlockHeight) ? dstHeight-dstBlockY : dstMaxBlockHeight;

				for (U32 dstBlockX=0; dstBlockX<dstWidth; dstBlockX += dstMaxBlockWidth)
				{
					U32 dstBlockWidth  = (dstWidth-dstBlockX < dstMaxBlockWidth) ? dstWidth-dstBlockX : dstMaxBlockWidth;

					U32 srcBlockX = (U32)(dstBlockX * xScaleInToOut);
					U32 srcBlockY = (U32)(dstBlockY * yScaleInToOut);
					U32 srcBlockWidth = (U32)(dstBlockWidth * xScaleInToOut);
					U32 srcBlockHeight = (U32)(dstBlockHeight * yScaleInToOut);

					depthTransferScale.clipW = depthTransferScale.outW = dstBlockWidth;
					depthTransferScale.clipH = depthTransferScale.outH = dstBlockHeight;
					depthTransferScale.inW = srcBlockWidth;
					depthTransferScale.inH = srcBlockHeight;

					U32 dstBlockOffset = (dstBlockX * 4) + (dstBlockY * depthTransferSurface.pitch);
					U32 srcBlockOffset = (srcBlockX * 4) + (srcBlockY * depthTransferScale.pitch);
					depthTransferSurface.offset = (dstOffset + dstBlockOffset + 0x3F) & ~0x3F;
					depthTransferScale.offset = srcOffset + srcBlockOffset;

					BIZ_ASSERT(depthTransferSurface.format == CELL_GCM_TRANSFER_SURFACE_FORMAT_A8R8G8B8);
					BIZ_ASSERT(depthTransferSurface.pitch % 64 == 0);
					BIZ_ASSERT(depthTransferSurface.offset % 64 == 0);
					BIZ_ASSERT(depthTransferSurface.offset >= m_RenderTarget.m_pDepthStencilSurface->GetOffset() && depthTransferSurface.offset <= m_RenderTarget.m_pDepthStencilSurface->GetOffset()+m_RenderTarget.m_pDepthStencilSurface->GetSize());

					cellGcmSetTransferScaleSurface(&depthTransferScale, &depthTransferSurface);
				}
			}
		}
	}
	else if(g_debugDisplayDownsampleColAndDep && pWindow->GetMultisample() == E_Multisample_2)
	{
		GPU_TIMING(4, Maths::RGBA(255, 255, 255, 255), "GPU.RestoreWindow.ColAndDep");

		InvalidateState();
		DefaultStates();
		
		const U32 nWidth = pWindow->GetColourTexture()->GetWidth() / 2;
		const U32 nHeight = pWindow->GetColourTexture()->GetHeight();

		const F32 fImageWidth = nWidth;
		const F32 fImageHeight = nHeight;
		const F32 fPixelWidth = 1.0f / (F32) nWidth;
		const F32 fPixelHeight = 1.0f / (F32) nHeight;

		SetPixelShaderParameterF(PixelShader::m_PixelShaderColAndDep_PixelSize, Display::C_PSConstant(fPixelWidth, fPixelHeight, fImageWidth, fImageHeight));
		SetPixelShader(&PixelShader::m_PixelShaderColAndDep);

		SetColourWriteEnable(E_ColourWriteEnable_All);

		SetStencilEnable(E_Bool_False);

		SetDepthEnable(E_Bool_True);
		SetDepthWriteEnable(E_Bool_True);
		SetDepthFunc(E_CmpFunc_Always);

		SetTexture(0, pWindow->GetColourTexture());
		SetTextureFilterMagMinMip(0, E_TextureFilter_Point, E_TextureFilter_Point, E_TextureFilter_None);
		SetTextureAddressUVW(0, E_TextureAddress_Clamp, E_TextureAddress_Clamp, E_TextureAddress_Clamp);
		SetTextureGamma(0, E_Bool_False);
		
		SetTexture(1, pWindow->GetDepthStencilTexture());
		SetTextureFilterMagMinMip(1, E_TextureFilter_Point, E_TextureFilter_Point, E_TextureFilter_None);
		SetTextureAddressUVW(1, E_TextureAddress_Clamp, E_TextureAddress_Clamp, E_TextureAddress_Clamp);
		SetTextureGamma(1, E_Bool_False);

		//cellGcmSetZMinMaxControl(gCellGcmCurrentContext, CELL_GCM_FALSE, CELL_GCM_FALSE, CELL_GCM_FALSE);
		
		Quad::Draw();

		SetDepthEnable(E_Bool_True);
		SetDepthWriteEnable(E_Bool_True);
		SetDepthFunc(E_CmpFunc_Default);
		
		SetColourWriteEnable(E_ColourWriteEnable_Default);
	}
	else
	{
		{
			GPU_TIMING(4, Maths::RGBA(255, 255, 255, 255), "GPU.RestoreWindow.Colour");

			if(g_debugDisplayRestoreWindow == E_RestoreWindow_User)
			{
				if (pWindow->GetMultisample() == E_Multisample_4)
				{
					const U32 nWidth = pWindow->GetColourTexture()->GetWidth() / 4;
					const U32 nHeight = pWindow->GetColourTexture()->GetHeight();
					const F32 fImageWidth = nWidth;
					const F32 fImageHeight = nHeight;
					const F32 fPixelWidth = 1.0f / (F32) nWidth;
					const F32 fPixelHeight = 1.0f / (F32) nHeight;

					SetPixelShaderParameterF(PixelShader::m_PixelShaderTexture4xAA_PixelSize, Display::C_PSConstant(fPixelWidth, fPixelHeight, fImageWidth, fImageHeight));
					SetPixelShader(&PixelShader::m_PixelShaderTexture4xAA);
				}
				else if (pWindow->GetMultisample() == E_Multisample_2)
				{
					const U32 nWidth = pWindow->GetColourTexture()->GetWidth() / 2;
					const U32 nHeight = pWindow->GetColourTexture()->GetHeight();

					const F32 fImageWidth = nWidth;
					const F32 fImageHeight = nHeight;
					const F32 fPixelWidth = 1.0f / (F32) nWidth;
					const F32 fPixelHeight = 1.0f / (F32) nHeight;

					if(!g_debugDisplayDownsampleFast)
					{
						SetPixelShaderParameterF(PixelShader::m_PixelShaderTexture2xAA_PixelSize, Display::C_PSConstant(fPixelWidth, fPixelHeight, fImageWidth, fImageHeight));
						SetPixelShader(&PixelShader::m_PixelShaderTexture2xAA);
					}
					else
					{
						SetPixelShaderParameterF(PixelShader::m_PixelShaderTextureFast_PixelSize, Display::C_PSConstant(fPixelWidth, fPixelHeight, fImageWidth, fImageHeight));
						SetPixelShader(&PixelShader::m_PixelShaderTextureFast);
					}
				}
				else
				{
					SetPixelShader(&PixelShader::m_PixelShaderTexture);
				}

				SetColourWriteEnable(colourWrite);

				SetDepthEnable(E_Bool_False);
				SetDepthWriteEnable(E_Bool_False);

				#if 0
				SetTextureCached(0, pWindow->GetColourTexture());
				
				const C_TextureControl textureControl(
					1,
					0,
					0,
					CELL_GCM_TEXTURE_MAX_ANISO_1);
				SetTextureControlCached(0, textureControl);

				const C_TextureAddress textureAddress(
					CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
					CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
					CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
					CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL,
					CELL_GCM_TEXTURE_ZFUNC_ALWAYS,
					0);
				SetTextureAddressCached(0, textureAddress);

				const C_TextureFilter textureFilter(
					0,
					CELL_GCM_TEXTURE_NEAREST,
					CELL_GCM_TEXTURE_NEAREST,
					CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX);
				SetTextureFilterCached(0, textureFilter);
				#else
				if(!g_debugDisplayDownsampleFast)
				{
					SetTexture(0, pWindow->GetColourTexture());
					SetTextureFilterMagMinMip(0, E_TextureFilter_Point, E_TextureFilter_Point, E_TextureFilter_None);
					SetTextureAddressUVW(0, E_TextureAddress_Clamp, E_TextureAddress_Clamp, E_TextureAddress_Clamp);
					SetTextureGamma(0, E_Bool_False);
				}
				else
				{
					SetTexture(0, pWindow->GetColourTexture());
					SetTextureFilterMagMinMip(0, E_TextureFilter_Linear, E_TextureFilter_Linear, E_TextureFilter_None);
					SetTextureAddressUVW(0, E_TextureAddress_Clamp, E_TextureAddress_Clamp, E_TextureAddress_Clamp);
					SetTextureGamma(0, E_Bool_False);
				}
				#endif

				Quad::Draw();

				SetDepthEnable(E_Bool_True);
				SetDepthWriteEnable(E_Bool_True);

				SetColourWriteEnable(E_ColourWriteEnable_Default);
			}
			else
			{
				SetColourWriteEnable(colourWrite);

				SetDepthEnable(E_Bool_False);
				SetDepthWriteEnable(E_Bool_False);

				const E_Downsample downsample = (E_Downsample)(S32)g_debugDisplayRestoreWindow;
				SetupDownsample(downsample, pWindow->GetColourTexture());

				F32 du = 0.0f;
				F32 dv = 0.0f;

				U32 width = pWindow->GetColourTexture()->GetWidth() / 2;
				
				switch(downsample)
				{
				case E_Downsample_Quincunx:
					du = 0.25f/(F32)width;
					du *= g_debugDisplayTexelShift;
					break;
				case E_Downsample_QuincunxAlt:
					du = -0.25f/(F32)width;
					dv = -0.5f/(F32)width;
					dv *= g_debugDisplayTexelShift;
					break;
				}
				
				SetCullMode(E_CullMode_None);

				Quad::DrawFullscreenQuad(du, dv);
				
				SetCullMode(E_CullMode_Default);

				SetDepthEnable(E_Bool_True);
				SetDepthWriteEnable(E_Bool_True);

				SetColourWriteEnable(E_ColourWriteEnable_Default);
			}
		}
		
		{
			GPU_TIMING(4, Maths::RGBA(255, 255, 255, 255), "GPU.RestoreWindow.Depth");

			if (pWindow->GetMultisample() == E_Multisample_4)
			{
				const U32 nWidth = pWindow->GetColourTexture()->GetWidth() / 4;
				const U32 nHeight = pWindow->GetColourTexture()->GetHeight();
				const F32 fImageWidth = nWidth;
				const F32 fImageHeight = nHeight;
				const F32 fPixelWidth = 1.0f / (F32) nWidth;
				const F32 fPixelHeight = 1.0f / (F32) nHeight;

				SetPixelShaderParameterF(PixelShader::m_PixelShaderDepth_PixelSize, Display::C_PSConstant(fPixelWidth, fPixelHeight, fImageWidth, fImageHeight));
				SetPixelShader(&PixelShader::m_PixelShaderDepth);
			}
			else if (pWindow->GetMultisample() == E_Multisample_2)
			{
				const U32 nWidth = pWindow->GetColourTexture()->GetWidth() / 2;
				const U32 nHeight = pWindow->GetColourTexture()->GetHeight();

				const F32 fImageWidth = nWidth;
				const F32 fImageHeight = nHeight;
				const F32 fPixelWidth = 1.0f / (F32) nWidth;
				const F32 fPixelHeight = 1.0f / (F32) nHeight;

				SetPixelShaderParameterF(PixelShader::m_PixelShaderDepth_PixelSize, Display::C_PSConstant(fPixelWidth, fPixelHeight, fImageWidth, fImageHeight));
				SetPixelShader(&PixelShader::m_PixelShaderDepth);
			}
			else
			{
				SetPixelShader(&PixelShader::m_PixelShaderDepth);
			}

			SetColourWriteEnable(0);

			SetDepthEnable(E_Bool_True);
			SetDepthWriteEnable(E_Bool_True);
			SetDepthFunc(E_CmpFunc_Always);

			#if 0
			SetTextureCached(1, pWindow->GetDepthStencilTexture());
				
			const C_TextureControl textureControl(
				1,
				0,
				0,
				CELL_GCM_TEXTURE_MAX_ANISO_1);
			SetTextureControlCached(1, textureControl);

			const C_TextureAddress textureAddress(
				CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
				CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
				CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
				CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL,
				CELL_GCM_TEXTURE_ZFUNC_ALWAYS,
				0);
			SetTextureAddressCached(1, textureAddress);

			const C_TextureFilter textureFilter(
				0,
				CELL_GCM_TEXTURE_NEAREST,
				CELL_GCM_TEXTURE_NEAREST,
				CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX);
			SetTextureFilterCached(1, textureFilter);
			#else
			SetTexture(1, pWindow->GetDepthStencilTexture());
			SetTextureFilterMagMinMip(1, E_TextureFilter_Point, E_TextureFilter_Point, E_TextureFilter_None);
			SetTextureAddressUVW(1, E_TextureAddress_Clamp, E_TextureAddress_Clamp, E_TextureAddress_Clamp);
			SetTextureGamma(1, E_Bool_False);
			#endif

			Quad::Draw();

			SetDepthEnable(E_Bool_True);
			SetDepthWriteEnable(E_Bool_True);
			SetDepthFunc(E_CmpFunc_Default);
			
			SetColourWriteEnable(E_ColourWriteEnable_Default);
		}
	}

	if(g_debugDisplayRestoreZcull)
	{
		GPU_TIMING(4, Maths::RGBA(255, 255, 255, 255), "GPU.RestoreWindow.Zcull");
		ReloadZCull(false);
	}
	else
	{
		SetZcullEnable(false, false); // Make sure zcull is off as we haven't restored zcull
	}
}

void C_GCMDevice::ReloadZCull(const bool bSCull)
{
	cellGcmSetClearZcullSurface(CELL_GCM_TRUE, CELL_GCM_TRUE);

	SetVertexShader(&m_vsZCull);
	SetPixelShader(&m_psZCull);

	SetBlendMode(E_BlendMode_Solid);
	SetColourWriteEnable(E_ColourWriteEnable_None);
	SetCullMode(E_CullMode_None);
	SetDepthEnable(E_Bool_True);
	if (GetDepthFlip())
		SetDepthFunc(E_CmpFunc_Greater);
	else
		SetDepthFunc(E_CmpFunc_Less);
	SetDepthWriteEnable(E_Bool_True);

	C_StencilState stencilState;
	if(bSCull)
	{
		stencilState.m_StencilEnable	= E_Bool_True;
		stencilState.m_StencilFunc		= E_CmpFunc_Always;
		stencilState.m_StencilRef		= 0x00;
		stencilState.m_StencilMask		= 0x00;
		stencilState.m_StencilWriteMask	= 0xFF;
		stencilState.m_StencilFail		= E_StencilOp_Keep;
		stencilState.m_StencilDepthFail	= E_StencilOp_Keep;
		stencilState.m_StencilDepthPass	= E_StencilOp_Invert;
		SetStencilState(stencilState);
	}
	
	cellGcmSetZMinMaxControl(CELL_GCM_TRUE, CELL_GCM_TRUE, CELL_GCM_FALSE);
	cellGcmSetDepthBoundsTestEnable(CELL_GCM_FALSE);

	Vertex_XY* pVertices = (Vertex_XY*)BeginVertices(E_PrimitiveType_TriangleStrip, 4, sizeof(Vertex_XY), Vertex_XY::m_pDeclaration);
	if(pVertices)
	{
		pVertices->x = -1.0f; pVertices->y = -1.0f; pVertices++;
		pVertices->x = -1.0f; pVertices->y = +1.0f; pVertices++;
		pVertices->x = +1.0f; pVertices->y = -1.0f; pVertices++;
		pVertices->x = +1.0f; pVertices->y = +1.0f; pVertices++;
		EndVertices();
	}

	SetColourWriteEnable(E_ColourWriteEnable_Default);
	SetCullMode(E_CullMode_Default);

	if(bSCull)
	{
		stencilState.m_StencilEnable	= E_Bool_False;
		SetStencilState(stencilState);
	}

	cellGcmSetZMinMaxControl(CELL_GCM_TRUE, CELL_GCM_FALSE, CELL_GCM_FALSE);
	
	SetZcullEnable(true, true); // Make sure zcull is on as we've reloaded the information from the aa buffers
}

void C_GCMDevice::ReloadSCull()
{
	SetVertexShader(&m_vsZCull);
	SetPixelShader(&m_psZCull);

	SetBlendMode(E_BlendMode_Solid);
	SetColourWriteEnable(E_ColourWriteEnable_None);
	SetCullMode(E_CullMode_None);
	SetDepthEnable(E_Bool_False);
	SetDepthWriteEnable(E_Bool_False);

	C_StencilState stencilState;
	stencilState.m_StencilEnable	= E_Bool_True;
	stencilState.m_StencilFunc		= E_CmpFunc_Less;
	stencilState.m_StencilRef		= 0xFF;
	stencilState.m_StencilMask		= 0xFF;
	stencilState.m_StencilWriteMask	= 0xFF;
	stencilState.m_StencilFail		= E_StencilOp_Keep;
	stencilState.m_StencilDepthFail	= E_StencilOp_Keep;
	stencilState.m_StencilDepthPass	= E_StencilOp_Invert;
	SetStencilState(stencilState);
	
	cellGcmSetZMinMaxControl(CELL_GCM_TRUE, CELL_GCM_TRUE, CELL_GCM_FALSE);
	cellGcmSetDepthBoundsTestEnable(CELL_GCM_FALSE);

	Vertex_XY* pVertices = (Vertex_XY*)BeginVertices(E_PrimitiveType_TriangleStrip, 4, sizeof(Vertex_XY), Vertex_XY::m_pDeclaration);
	if(pVertices)
	{
		pVertices->x = -1.0f; pVertices->y = -1.0f; pVertices++;
		pVertices->x = -1.0f; pVertices->y = +1.0f; pVertices++;
		pVertices->x = +1.0f; pVertices->y = -1.0f; pVertices++;
		pVertices->x = +1.0f; pVertices->y = +1.0f; pVertices++;
		EndVertices();
	}

	SetColourWriteEnable(E_ColourWriteEnable_Default);
	SetCullMode(E_CullMode_Default);
	SetDepthEnable(E_Bool_True);
	SetDepthWriteEnable(E_Bool_True);

	stencilState.m_StencilEnable	= E_Bool_False;
	SetStencilState(stencilState);

	cellGcmSetZMinMaxControl(CELL_GCM_TRUE, CELL_GCM_FALSE, CELL_GCM_FALSE);
}

U32 C_GCMDevice::CalculateMemorySize()
{
	#ifdef DISABLE_TILING
	const U32 flags = 0;
	#else
	const U32 flags = E_Format_Tiled;
	#endif

	U32 memorySize = 0;

	// Front buffer
	{
		U32 alignedWidth = GCM_ALIGN_SIZE(m_DeviceParams.m_FrontBufferWidth, 64);
		U32 alignedHeight = GCM_ALIGN_SIZE(m_DeviceParams.m_FrontBufferHeight, 64);
	
		// First front buffer
		{
			U32 pitch = Display2::GetSurfacePitchFromWidth(
				alignedWidth,
				flags,
				GetSurfaceFormatFromFormat(m_DeviceParams.m_FrontBufferFormat),
				(Display2::E_Multisample)E_Multisample_None);
			U32 size = Display2::GetSurfaceSizeFromHeightAndPitch(
				alignedHeight,
				pitch,
				flags,
				GetSurfaceFormatFromFormat(m_DeviceParams.m_FrontBufferFormat),
				(Display2::E_Multisample)E_Multisample_None);
			memorySize += size;
		}

		// Second front buffer
		{
			U32 pitch = Display2::GetSurfacePitchFromWidth(
				alignedWidth,
				flags,
				GetSurfaceFormatFromFormat(m_DeviceParams.m_FrontBufferFormat),
				(Display2::E_Multisample)E_Multisample_None);
			U32 size = Display2::GetSurfaceSizeFromHeightAndPitch(
				alignedHeight,
				pitch,
				flags,
				GetSurfaceFormatFromFormat(m_DeviceParams.m_FrontBufferFormat),
				(Display2::E_Multisample)E_Multisample_None);
			memorySize += size;
		}
	}

	// Back buffer
	{
		U32 alignedWidth = GCM_ALIGN_SIZE(m_DeviceParams.m_BackBufferWidth, 64);
		U32 alignedHeight = GCM_ALIGN_SIZE(m_DeviceParams.m_BackBufferHeight, 64);

		// Colour stencil back buffer
		if(m_DeviceParams.m_UniqueBackBuffer)
		{
			U32 pitch = Display2::GetSurfacePitchFromWidth(
				alignedWidth,
				flags,
				GetSurfaceFormatFromFormat(m_DeviceParams.m_BackBufferColourFormat),
				(Display2::E_Multisample)m_DeviceParams.m_BackBufferMultisample);
			U32 size = Display2::GetSurfaceSizeFromHeightAndPitch(
				alignedHeight,
				pitch,
				flags,
				GetSurfaceFormatFromFormat(m_DeviceParams.m_BackBufferColourFormat),
				(Display2::E_Multisample)m_DeviceParams.m_BackBufferMultisample);
			memorySize += size;
		}

		// Depth stencil stencil back buffer
		{
			U32 pitch = Display2::GetSurfacePitchFromWidth(
				alignedWidth,
				flags,
				GetSurfaceFormatFromFormat(m_DeviceParams.m_BackBufferDepthStencilFormat),
				(Display2::E_Multisample)m_DeviceParams.m_BackBufferMultisample);
			U32 size = Display2::GetSurfaceSizeFromHeightAndPitch(
				alignedHeight,
				pitch,
				flags,
				GetSurfaceFormatFromFormat(m_DeviceParams.m_BackBufferDepthStencilFormat),
				(Display2::E_Multisample)m_DeviceParams.m_BackBufferMultisample);
			memorySize += size;
		}
	}

	// Window buffer
	{
		/*const S32 windowCount = m_Windows.Size();
		if(windowCount > 0)
		{
			for(S32 windowIndex=0; windowIndex<windowCount; windowIndex++)
			{
				C_Window* pWindow = m_Windows[windowIndex];
			}
		}
		else*/
		{
			U32 alignedWidth = GCM_ALIGN_SIZE(m_DeviceParams.m_WindowBufferWidth, 64);
			U32 alignedHeight = GCM_ALIGN_SIZE(m_DeviceParams.m_WindowBufferHeight, 64);

			// Colour stencil window buffer
			{
				U32 pitch = Display2::GetSurfacePitchFromWidth(
					alignedWidth,
					flags,
					GetSurfaceFormatFromFormat(m_DeviceParams.m_WindowBufferColourFormat),
					(Display2::E_Multisample)m_DeviceParams.m_WindowBufferMultisample);
				U32 size = Display2::GetSurfaceSizeFromHeightAndPitch(
					alignedHeight,
					pitch,
					flags,
					GetSurfaceFormatFromFormat(m_DeviceParams.m_WindowBufferColourFormat),
					(Display2::E_Multisample)m_DeviceParams.m_WindowBufferMultisample);
				memorySize += size;
			}

			// Depth stencil stencil window buffer
			{
				U32 pitch = Display2::GetSurfacePitchFromWidth(
					alignedWidth,
					flags,
					GetSurfaceFormatFromFormat(m_DeviceParams.m_WindowBufferDepthStencilFormat),
					(Display2::E_Multisample)m_DeviceParams.m_WindowBufferMultisample);
				U32 size = Display2::GetSurfaceSizeFromHeightAndPitch(
					alignedHeight,
					pitch,
					flags,
					GetSurfaceFormatFromFormat(m_DeviceParams.m_WindowBufferDepthStencilFormat),
					(Display2::E_Multisample)m_DeviceParams.m_WindowBufferMultisample);
				memorySize += size;
			}
		}
	}
	
	return memorySize;
}

bool C_GCMDevice::InSystemOSMenu()
{
	return m_InSystemOSMenu;
}

void C_GCMDevice::EnterSystemOSMenu()
{
	//BIZ_ASSERT(!m_InSystemOSMenu);
	m_InSystemOSMenu = true;
}

void C_GCMDevice::LeaveSystemOSMenu()
{
	//BIZ_ASSERT(m_InSystemOSMenu);
	m_InSystemOSMenu = false;
}

void C_GCMDevice::StartTimingFrame()
{
	m_GpuTimeIndex = m_GpuTimeIndex == 0 ? 1 : 0;
	
	m_GpuTimeBegin = cellGcmGetTimeStamp(g_ReportGpuTime+(m_GpuTimeIndex*2));
	
	m_GpuTimeEnd = cellGcmGetTimeStamp(g_ReportGpuTime+(m_GpuTimeIndex*2)+1);

	if(m_CpuTimeEnd > m_CpuTimeBegin)
		m_CpuTime = ((F32)(m_CpuTimeEnd - m_CpuTimeBegin) / (F32)SYS_USEC_PER_SEC) * 1000.f;

	if(m_GpuTimeEnd > m_GpuTimeBegin)
		m_GpuTime = ((F32)(m_GpuTimeEnd - m_GpuTimeBegin)) / 1000000.0f;

	m_CpuTimeBegin = Time::GetPerformanceCounter();

	GCMSetTimeStamp(g_ReportGpuTime+(m_GpuTimeIndex*2));

	#if !CFG_GOLD
	if(g_debugDisplayPrintFrameTimes)
	{
		LOGINFO(g_Log, "Gpu %.2f Cpu %.2f\n", m_GpuTime, m_CpuTime);
	}
	#endif
}

void C_GCMDevice::EndTimingFrame()
{
	m_CpuTimeEnd = Time::GetPerformanceCounter();

	GCMSetTimeStamp(g_ReportGpuTime+(m_GpuTimeIndex*2)+1);
}

void C_GCMDevice::SwapFrontBuffers(bool flip)
{
#ifndef CFG_GOLD
	m_ptrToPtrToFrontbufferMemory = &(m_FrontBuffers[m_CurrentFrontBuffer]->GetSurface()->m_pAddress);
#endif

	if(flip)
	{
		m_SwappedFrontBuffer = m_CurrentFrontBuffer;
		
		// Swap to new front buffer
		if(++m_CurrentFrontBuffer >= m_FrontBuffers.Size())
		{
			m_CurrentFrontBuffer = 0;
		}

		// Overlap back buffer over new front buffer
		if(!m_UniqueBackBuffer)
		{
			m_BackBuffers[0]->GetColourSurface()->m_Offset = m_FrontBuffers[m_CurrentFrontBuffer]->GetSurface()->GetOffset();
			m_BackBuffers[0]->GetColourTexture()->m_gcmTexture.offset = m_FrontBuffers[m_CurrentFrontBuffer]->GetSurface()->GetOffset();
		}
	}

	#ifdef DEVICE_DEBUG
	if(g_BeginVerticesFailingCheck)
	{
		if(g_debugShowBeginVerticesFailing)
		{
			static U32 beginVerticesFailing = 1;
			LOGINFO(g_Log, "*** No of begin vertices failed %i with size %i out of max size %i\n",
				beginVerticesFailing,
				m_DynamicVertexBuffer.GetOffset(),
				m_DynamicVertexBuffer.GetSize());
			if(++beginVerticesFailing >= UINT_MAX)
				beginVerticesFailing = 1;
		}
		g_BeginVerticesFailingCheck = false;
	}
	#endif

	// Swap to the new dynamic vertex buffer
	m_DynamicVertexBuffer.Swap();

	// Reset begin vertices usage
	m_BeginVerticesUsage = m_UserBeginVerticesUsage = 0;
}

void GCMFlipCallback(const U32 m00)
{
	g_VSyncCounter--;
	if(g_VSyncCounter <= 0)
	{
		if(g_JumpToSelfAddress[g_ReadJumpToSelf] != 0)
		{
			U32* j2sCommand = (U32*)g_JumpToSelfAddress[g_ReadJumpToSelf];
			
			Display2::C_ContextData j2sContextData;
			cellGcmSetupContextData(&j2sContextData, j2sCommand, 4, NULL);
			
			Display2::C_ContextNoCheckNoTrackUnsafeInline j2sContext(&j2sContextData);
			j2sContext.SetNopCommand(1);

			g_JumpToSelfAddress[g_ReadJumpToSelf] = 0;
			g_ReadJumpToSelf++;
			if(g_ReadJumpToSelf == 3)
				g_ReadJumpToSelf = 0;
		}
	}
}

const void* C_GCMDevice::GetScreenshotInfo(U32* width, U32* height, U32* pitch, U32* colour, const void** bpLocation, const void** ptrToPtrToMemory) const
{
	const Display::C_FrontBuffer& fb = GetFrontBuffer();

	*width		= fb.GetSurface()->GetWidth();
	*height		= fb.GetSurface()->GetHeight();
	*pitch		= fb.GetSurface()->GetPitch();

	switch (fb.GetSurface()->GetFormat())
	{
	case Display2::E_SurfaceFormat_A8R8G8B8:
		*colour		= 0;
		break;

	default:
		BIZ_ASSERT_FAIL("Unknown surface type (%d)", fb.GetSurface()->GetFormat());
	}


	*bpLocation			= *((void**)GCMFlipCallback);
#ifndef CFG_GOLD
	*ptrToPtrToMemory	= m_ptrToPtrToFrontbufferMemory;
#endif
}

void C_GCMDevice::CreateFlipEvent()
{
	cellGcmSetVBlankHandler(GCMFlipCallback);
	
	cellGcmSetFlipMode(CELL_GCM_DISPLAY_HSYNC);
}

void C_GCMDevice::DestroyFlipEvent()
{
	cellGcmSetFlipHandler(NULL);
}

void C_GCMDevice::SetDepthFlip(const bool depthCmpFlip, const bool depthViewportFlip)
{
	m_DepthCmpFlip = depthCmpFlip;
	m_DepthViewportFlip = depthCmpFlip && depthViewportFlip;
	m_DepthFuncDefault = depthCmpFlip ? E_CmpFunc_GreaterEqual : E_CmpFunc_LessEqual;

	m_DepthClear = m_DepthCmpFlip ? 1.f : 0.f;
}

void C_GCMDevice::SetBlendMode(const E_BlendMode blendMode)
{
	switch(blendMode)
	{
		case E_BlendMode_Solid:
		{
			SetAlphaTestEnable(E_Bool_False);
			
			SetBlendEnable(E_Bool_False);
			
			if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
				SetColourWriteEnable(E_ColourWriteEnable_All);
		}
		break;

		case E_BlendMode_SolidAlphaTest:
		{
			SetAlphaTestEnable(E_Bool_True);
			SetAlphaTestFunc(E_CmpFunc_Greater);
			SetAlphaTestRef(0x80);

			SetBlendEnable(E_Bool_False);
            
			if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
				SetColourWriteEnable(E_ColourWriteEnable_Red|E_ColourWriteEnable_Green|E_ColourWriteEnable_Blue);
		}
		break;

		case E_BlendMode_Alpha:
		{
			SetAlphaTestEnable(E_Bool_True);
			SetAlphaTestFunc(E_CmpFunc_Greater);
			SetAlphaTestRef(0x00);

			SetBlendEnable(E_Bool_True);
			SetBlend(
				E_Blend_SrcAlpha,
				E_Blend_InvSrcAlpha,
				E_BlendOp_Add,
				E_Blend_SrcAlpha,
				E_Blend_InvSrcAlpha,
				E_BlendOp_Add);

			if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
				SetColourWriteEnable(E_ColourWriteEnable_Red|E_ColourWriteEnable_Green|E_ColourWriteEnable_Blue);
		}
		break;

		case E_BlendMode_AlphaPreMultiply:
		{
			SetAlphaTestEnable(E_Bool_False);
			SetAlphaTestFunc(E_CmpFunc_Greater);
			SetAlphaTestRef(0x00);
			
			SetBlendEnable(E_Bool_True);
			SetBlend(
				E_Blend_One,
				E_Blend_InvSrcAlpha,
				E_BlendOp_Add,
				E_Blend_One,
				E_Blend_InvSrcAlpha,
				E_BlendOp_Add);
			
			if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
				SetColourWriteEnable(E_ColourWriteEnable_Red|E_ColourWriteEnable_Green|E_ColourWriteEnable_Blue);
		}
		break;

		case E_BlendMode_Additive:
		{
			SetAlphaTestEnable(E_Bool_True);
			SetAlphaTestFunc(E_CmpFunc_Greater);
			SetAlphaTestRef(0x00);

			SetBlendEnable(E_Bool_True);
			SetBlend(
				E_Blend_One,
				E_Blend_One,
				E_BlendOp_Add,
				E_Blend_One,
				E_Blend_One,
				E_BlendOp_Add);
			
			if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
				SetColourWriteEnable(E_ColourWriteEnable_Red|E_ColourWriteEnable_Green|E_ColourWriteEnable_Blue);
		}
		break;

		case E_BlendMode_AlphaAdditive:
		{
    		SetAlphaTestEnable(E_Bool_True);
			SetAlphaTestFunc(E_CmpFunc_Greater);
			SetAlphaTestRef(0x00);

			SetBlendEnable(E_Bool_True);
			SetBlend(
				E_Blend_SrcAlpha,
				E_Blend_One,
				E_BlendOp_Add,
				E_Blend_SrcAlpha,
				E_Blend_One,
				E_BlendOp_Add);

			if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
				SetColourWriteEnable(E_ColourWriteEnable_Red|E_ColourWriteEnable_Green|E_ColourWriteEnable_Blue);
		}
		break;

		case E_BlendMode_AlphaSubtractive:
		{
			SetAlphaTestEnable(E_Bool_True);
			SetAlphaTestFunc(E_CmpFunc_Greater);
			SetAlphaTestRef(0x00);

			SetBlendEnable(E_Bool_True);
			SetBlend(
				E_Blend_SrcAlpha,
				E_Blend_One,
				E_BlendOp_RevSubtract,
				E_Blend_SrcAlpha,
				E_Blend_One,
				E_BlendOp_RevSubtract);

			if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
				SetColourWriteEnable(E_ColourWriteEnable_Red|E_ColourWriteEnable_Green|E_ColourWriteEnable_Blue);
		}
		break;

		case E_BlendMode_Subtractive:
		{
			SetAlphaTestEnable(E_Bool_False);
			
			SetBlendEnable(E_Bool_True);
			SetBlend(
				E_Blend_One,
				E_Blend_One,
				E_BlendOp_RevSubtract,
				E_Blend_One,
				E_Blend_One,
				E_BlendOp_RevSubtract);

			if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
				SetColourWriteEnable(E_ColourWriteEnable_Red|E_ColourWriteEnable_Green|E_ColourWriteEnable_Blue);
		}
		break;

		case E_BlendMode_Multiply:
		{
			SetAlphaTestEnable(E_Bool_False);

			SetBlendEnable(E_Bool_True);
			SetBlend(
				E_Blend_Zero,
				E_Blend_SrcColour,
				E_BlendOp_Add,
				E_Blend_Zero,
				E_Blend_SrcColour,
				E_BlendOp_Add);

			if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
				SetColourWriteEnable(E_ColourWriteEnable_Red|E_ColourWriteEnable_Green|E_ColourWriteEnable_Blue);
		}
		break;

		case E_BlendMode_AdditiveSmooth:
		{
			SetAlphaTestEnable(E_Bool_True);
			SetAlphaTestFunc(E_CmpFunc_Greater);
			SetAlphaTestRef(0x00);

			SetBlendEnable(E_Bool_True);
			SetBlend(
				E_Blend_One,
				E_Blend_InvSrcColour,
				E_BlendOp_Add,
				E_Blend_One,
				E_Blend_InvSrcColour,
				E_BlendOp_Add);

			if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
				SetColourWriteEnable(E_ColourWriteEnable_Red|E_ColourWriteEnable_Green|E_ColourWriteEnable_Blue);
		}
		break;

		case E_BlendMode_Unknown:
			break;

		default:
		{
			SetAlphaTestEnable(E_Bool_False);
			
			SetBlendEnable(E_Bool_False);

			if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
				SetColourWriteEnable(E_ColourWriteEnable_All);
		}
		break;
    }
	
	m_BlendMode = blendMode;

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

void C_GCMDevice::SetGpuGroupMarker(const U32 group)
{
	//cellGcmSetWriteBackEndLabel(gCellGcmCurrentContext, 32, group);

	#ifdef GCM_DEBUG
	//CheckForErrors();
	#endif
}

void C_GCMDevice::SetGpuPassAndTypeMarker(const U32 pass, const U32 type)
{
	//cellGcmSetBackStencilFunc(gCellGcmCurrentContext, CELL_GCM_ALWAYS, pass, type);

	#ifdef GCM_DEBUG
	//CheckForErrors();
	#endif
}

void C_GCMDevice::DumpMemoryHeaps()
{
#ifndef CFG_GOLD
	m_Display2.DumpMemoryHeaps();
#endif
}

bool C_GCMDevice::IsMainThread() const
{
	return GCM::IsMainThread();
}

void C_GCMDevice::CheckCanRender()
{
	// Render call not from main thread
	BIZ_ASSERT(HaveThreadOwnership());

	// No colour or depth buffer set
	BIZ_ASSERT(m_RenderTarget.GetColourSurface() || m_RenderTarget.GetDepthStencilSurface());
	
	// Colour buffer set but not created
	BIZ_ASSERT(m_RenderTarget.GetColourSurface() ? m_RenderTarget.GetColourSurface()->IsCreated() : 1);

	// Depth buffer set but not created
	BIZ_ASSERT(m_RenderTarget.GetDepthStencilSurface() ? m_RenderTarget.GetDepthStencilSurface()->IsCreated() : 1);

	// No depth buffer set but depth enabled
	BIZ_ASSERT(!m_RenderTarget.GetDepthStencilSurface() || !m_RenderTarget.GetDepthStencilSurface()->IsCreated() ? m_DepthEnable == E_Bool_False : 1);

	// No depth buffer set but depth write enabled
	BIZ_ASSERT(!m_RenderTarget.GetDepthStencilSurface() || !m_RenderTarget.GetDepthStencilSurface()->IsCreated() ? m_DepthWriteEnable == E_Bool_False : 1);

	// No depth buffer set but stenciling enabled
	BIZ_ASSERT(!m_RenderTarget.GetDepthStencilSurface() || !m_RenderTarget.GetDepthStencilSurface()->IsCreated() ? m_StencilEnable == E_Bool_False : 1);
}

void C_GCMDevice::DrawPrimitive(const E_PrimitiveType primitiveType, const U32 startVertex, const U32 vertexCount)
{
	#if !CFG_SPEED
	CheckCanRender();
	#endif

	SetVertexDeclaration();

	SetInvalidateVertexCache();
	
	cellGcmSetDrawArrays(GCMPrimitiveType(primitiveType), startVertex, vertexCount);

	#ifdef DEVICE_DEBUG
	if(g_CrashGPU)
	{
		cellGcmSetAlphaFunc(-1, -1);
		g_CrashGPU = false;
	}
	#endif

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

void C_GCMDevice::DrawIndexedPrimitive(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 startIndex, const U32 indexCount)
{
	#if !CFG_SPEED
	CheckCanRender();
	#endif

	SetVertexDeclaration();

	BIZ_ASSERT(m_pIndexBuffer);

	SetInvalidateVertexCache();

	const U32 gcmOffset = m_pIndexBuffer->GetOffset() + SizeOf((E_Format)m_pIndexBuffer->GetFormat()) * startIndex;
	cellGcmSetDrawIndexArray(GCMPrimitiveType(primitiveType), indexCount, GCMFormat((E_Format)m_pIndexBuffer->GetFormat()), Display2::FlagsToLocation(m_pIndexBuffer->GetFlags()), gcmOffset);

	#ifdef DEVICE_DEBUG
	if(g_CrashGPU)
	{
		cellGcmSetAlphaFunc(-1, -1);
		g_CrashGPU = false;
	}
	#endif

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

void C_GCMDevice::DrawIndexedPrimitive(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount)
{
	#if !CFG_SPEED
	CheckCanRender();
	#endif
	
	SetVertexDeclaration();

	BIZ_ASSERT(m_pIndexBuffer);

	SetInvalidateVertexCache();

	const U32 gcmOffset = m_pIndexBuffer->GetOffset() + SizeOf((E_Format)m_pIndexBuffer->GetFormat()) * startIndex;
	cellGcmSetDrawIndexArray(GCMPrimitiveType(primitiveType), indexCount, GCMFormat((E_Format)m_pIndexBuffer->GetFormat()), Display2::FlagsToLocation(m_pIndexBuffer->GetFlags()), gcmOffset);

	#ifdef DEVICE_DEBUG
	if(g_CrashGPU)
	{
		cellGcmSetAlphaFunc(-1, -1);
		g_CrashGPU = false;
	}
	#endif

	#ifdef GCM_DEBUG
	CheckForErrors();
	#endif
}

//#define CLEAR_GCM_CONTEXT_HACK

#ifdef CLEAR_GCM_CONTEXT_HACK
struct CellGcmContextData *g_pTempCellGcmCurrentContext = NULL;

void HackyClearGCMContext()
{
	g_pTempCellGcmCurrentContext = gCellGcmCurrentContext;
	gCellGcmCurrentContext = NULL;
}

void HackySetGCMContext()
{
	// ignore this the first time set is called
	if(g_pTempCellGcmCurrentContext == NULL)
	{
	}
	else
	{
		gCellGcmCurrentContext = g_pTempCellGcmCurrentContext;
	}
}
#endif

void C_GCMDevice::InitialiseThreadOwnership()
{
	// different case for init as the main thread already has ownership of device
	g_iOwnerThreadRefCount = 1;
	m_ThreadOwnershipMutex.Wait();
}

bool C_GCMDevice::HaveThreadOwnership()
{
	return (g_iOwnerThreadRefCount > 0);
}

bool C_GCMDevice::AcquireThreadOwnership( U32 timeMs )
{
	if (m_ThreadOwnershipMutex.Wait(timeMs))
	{
		if (g_iOwnerThreadRefCount++ == 0)
		{
//#ifdef PLATFORM_XENON
//			m_pd3dDevice->AcquireThreadOwnership();
//#endif
			#ifdef CLEAR_GCM_CONTEXT_HACK
			HackySetGCMContext();
			#endif
		}

		return true;
	}

	return false;
}

void C_GCMDevice::ReleaseThreadOwnership()
{
	// Check this thread has ownership before we allow it to release
	BIZ_ASSERT(g_iOwnerThreadRefCount > 0);

	if (--g_iOwnerThreadRefCount == 0)
	{
//#ifdef PLATFORM_XENON
//		m_pd3dDevice->ReleaseThreadOwnership();
//#endif
		#ifdef CLEAR_GCM_CONTEXT_HACK
		HackyClearGCMContext();
		#endif
	}

	m_ThreadOwnershipMutex.Release();
}

bool C_GCMDevice::AADisabled() const
{
	return g_AllowDisableAA ? (m_DisableAA || g_DisableAA ? true : false) : false;
}

void C_GCMDevice::EnableAA()
{
	m_DisableAA = false;
}

void C_GCMDevice::DisableAA()
{
	m_DisableAA = true;
}

C_Surface* C_GCMDevice::GetSharedColourSurface(const U32 width, const U32 height, const Display2::E_Multisample multisample, const Display2::E_SurfaceFormat colourFormat, bool main)
{
	if(main)
		return GetMainSharedColourSurface(width, height, multisample, colourFormat);
	else
		return GetLocalSharedColourSurface(width, height, multisample, colourFormat);
}

C_Surface* C_GCMDevice::GetSharedColourSurface(const U32 width, const U32 height, const E_Multisample multisample, const E_Format colourFormat, bool main)
{
	if(main)
		return GetMainSharedColourSurface(width, height, multisample, colourFormat);
	else
		return GetLocalSharedColourSurface(width, height, multisample, colourFormat);
}

C_Surface* C_GCMDevice::GetSharedDepthStencilSurface(const U32 width, const U32 height, const Display2::E_Multisample multisample, const Display2::E_SurfaceFormat depthStencilFormat, bool main)
{
	if(main)
		return GetMainSharedDepthStencilSurface(width, height, multisample, depthStencilFormat);
	else
		return GetLocalSharedDepthStencilSurface(width, height, multisample, depthStencilFormat);
}

C_Surface* C_GCMDevice::GetSharedDepthStencilSurface(const U32 width, const U32 height, const E_Multisample multisample, const E_Format depthStencilFormat, bool main)
{
	if(main)
		return GetMainSharedDepthStencilSurface(width, height, multisample, depthStencilFormat);
	else
		return GetLocalSharedDepthStencilSurface(width, height, multisample, depthStencilFormat);
}

bool C_GCMDevice::CreateMainSharedRenderBuffer(const U32 width, const U32 height, const E_Multisample multisample, const E_Format colourFormat, const E_Format depthStencilFormat)
{
	if(	width == m_MainSharedRenderBuffer.GetWidth() &&
		height == m_MainSharedRenderBuffer.GetHeight() &&
		multisample == GetMultisample(m_MainSharedRenderBuffer.GetMultisample()) && 
		colourFormat == GetFormatFromSurfaceFormat(m_MainSharedRenderBuffer.GetColourSurfaceFormat(0)) &&
		depthStencilFormat == GetFormatFromSurfaceFormat(m_MainSharedRenderBuffer.GetDepthStencilSurfaceFormat()))
		return true;

	DestroyMainSharedRenderBuffer();
	
	m_MainSharedRenderBuffer.SetName("_MainShared");
	m_MainSharedRenderBuffer.SetFlags(Display2::E_Flags_Device | Display2::E_Flags_System);
	m_MainSharedRenderBuffer.SetWidth(width);
	m_MainSharedRenderBuffer.SetHeight(height);
	m_MainSharedRenderBuffer.SetMultisample(GetMultisample2(multisample));
	m_MainSharedRenderBuffer.SetColourFormat(0, GetTextureFormat2(colourFormat));
	m_MainSharedRenderBuffer.SetDepthStencilFormat(GetTextureFormat2(depthStencilFormat));

	m_MainSharedRenderBuffer.m_ColourTileInfo[0].enable = true;
	m_MainSharedRenderBuffer.m_DepthStencilTileInfo.enable = true;
	m_MainSharedRenderBuffer.m_DepthStencilZcullInfo.enable = true;
	m_MainSharedRenderBuffer.m_DepthStencilZcullInfo.flags |= Display2::E_ZcullFlags_UserMemory;
	m_MainSharedRenderBuffer.m_DepthStencilZcullInfo.cullStart = 0;

	Display2::C_RenderBufferSetup mainSharedRenderBufferSetup;
	if(!m_Display2.CreateRenderBuffer(&m_MainSharedRenderBuffer, mainSharedRenderBufferSetup))
	{
		LOGFATAL(g_Log, "Failed to create main shared render buffer\n");
		return false;
	}

	//LOGINFO(Display::g_Log, "Allocating main shared colour\n");
	//DumpDisplaySurface(m_MainSharedRenderBuffer.GetColourSurface(0));
	//LOGINFO(Display::g_Log, "Allocating main shared depth stencil\n");
	//DumpDisplaySurface(m_MainSharedRenderBuffer.GetDepthStencilSurface());

	return true;
}

void C_GCMDevice::DestroyMainSharedRenderBuffer()
{
	DestroyRenderBuffer(&m_MainSharedRenderBuffer);
}

C_RenderBuffer* C_GCMDevice::GetMainSharedRenderBuffer()
{
	return &m_MainSharedRenderBuffer;
}

C_Surface* C_GCMDevice::GetMainSharedColourSurface()
{
	return m_MainSharedRenderBuffer.GetColourSurface(0);
}

C_Texture* C_GCMDevice::GetMainSharedColourTexture()
{
	return m_MainSharedRenderBuffer.GetColourTexture(0);
}

C_Surface* C_GCMDevice::GetMainSharedColourSurface(const U32 width, const U32 height, const Display2::E_Multisample multisample, const Display2::E_SurfaceFormat colourFormat)
{
	if(	m_MainSharedRenderBuffer.GetWidth() == width &&
		m_MainSharedRenderBuffer.GetHeight() == height && 
		m_MainSharedRenderBuffer.GetMultisample() == multisample &&
		m_MainSharedRenderBuffer.GetColourSurfaceFormat(0) == colourFormat)
	{
		return m_MainSharedRenderBuffer.GetColourSurface(0);
	}
	else
	{
		return NULL;
	}
}

C_Surface* C_GCMDevice::GetMainSharedColourSurface(const U32 width, const U32 height, const E_Multisample multisample, const E_Format colourFormat)
{
	return GetMainSharedColourSurface(width, height, GetMultisample2(multisample), GetSurfaceFormat2(colourFormat));
}

C_Surface* C_GCMDevice::GetMainSharedDepthStencilSurface()
{
	return m_MainSharedRenderBuffer.GetDepthStencilSurface();
}

C_Texture* C_GCMDevice::GetMainSharedDepthStencilTexture()
{
	return m_MainSharedRenderBuffer.GetDepthStencilTexture();
}

C_Surface* C_GCMDevice::GetMainSharedDepthStencilSurface(const U32 width, const U32 height, const Display2::E_Multisample multisample, const Display2::E_SurfaceFormat depthStencilFormat)
{
	if(	m_MainSharedRenderBuffer.GetWidth() == width &&
		m_MainSharedRenderBuffer.GetHeight() == height && 
		m_MainSharedRenderBuffer.GetMultisample() == multisample &&
		m_MainSharedRenderBuffer.GetDepthStencilSurfaceFormat() == depthStencilFormat)
	{
		return m_MainSharedRenderBuffer.GetDepthStencilSurface();
	}
	else
	{
		return NULL;
	}
}

C_Surface* C_GCMDevice::GetMainSharedDepthStencilSurface(const U32 width, const U32 height, const E_Multisample multisample, const E_Format depthStencilFormat)
{
	return GetMainSharedDepthStencilSurface(width, height, GetMultisample2(multisample), GetSurfaceFormat2(depthStencilFormat));
}

bool C_GCMDevice::CreateLocalSharedRenderBuffer(const U32 width, const U32 height, const E_Multisample multisample, const E_Format colourFormat, const E_Format depthStencilFormat)
{
	if(	width == m_LocalSharedRenderBuffer.GetWidth() &&
		height == m_LocalSharedRenderBuffer.GetHeight() &&
		multisample == GetMultisample(m_LocalSharedRenderBuffer.GetMultisample()) && 
		colourFormat == GetFormatFromSurfaceFormat(m_LocalSharedRenderBuffer.GetColourSurfaceFormat(0)) &&
		depthStencilFormat == GetFormatFromSurfaceFormat(m_LocalSharedRenderBuffer.GetDepthStencilSurfaceFormat()))
		return true;

	DestroyLocalSharedRenderBuffer();

	m_LocalSharedRenderBuffer.SetName("_LocalShared");
	m_LocalSharedRenderBuffer.SetFlags(Display2::E_Flags_Device | Display2::E_Flags_Video);
	m_LocalSharedRenderBuffer.SetWidth(width);
	m_LocalSharedRenderBuffer.SetHeight(height);
	m_LocalSharedRenderBuffer.SetMultisample(GetMultisample2(multisample));
	m_LocalSharedRenderBuffer.SetColourFormat(0, GetTextureFormat2(colourFormat));
	m_LocalSharedRenderBuffer.SetDepthStencilFormat(GetTextureFormat2(depthStencilFormat));

	m_LocalSharedRenderBuffer.m_ColourTileInfo[0].enable = true;

	// Overlap the back depth stencil buffer with the local shared depth stencil buffer if buffers match

	BIZ_ASSERT(m_BackBuffers.Size() == 1);

	for(S32 iBackBuffer=0; iBackBuffer<m_BackBuffers.Size(); iBackBuffer++)
	{
		C_BackBuffer* pBackBuffer = m_BackBuffers[iBackBuffer];
		Display2::C_RenderBuffer* pBackRenderBuffer = &pBackBuffer->m_RenderBuffer;
		C_Surface* pBackDepthStencilSurface = pBackRenderBuffer->GetDepthStencilSurface();
		
		if(pBackDepthStencilSurface->GetWidth() == width && pBackDepthStencilSurface->GetHeight() == height)
		{
			m_LocalSharedRenderBuffer.OverlapDepthStencilBuffer(pBackDepthStencilSurface->GetSize(), pBackDepthStencilSurface->GetAddress());

			m_LocalSharedRenderBuffer.m_DepthStencilTileInfo.enable = false;
			m_LocalSharedRenderBuffer.m_DepthStencilZcullInfo.enable = false;
		}
		else
		{
			m_LocalSharedRenderBuffer.m_DepthStencilTileInfo.enable = true;
			m_LocalSharedRenderBuffer.m_DepthStencilZcullInfo.enable = true;

			m_LocalSharedRenderBuffer.m_DepthStencilZcullInfo.flags |= Display2::E_ZcullFlags_UserMemory;
			m_LocalSharedRenderBuffer.m_DepthStencilZcullInfo.cullStart = 0;
		}
	}

	Display2::C_RenderBufferSetup localSharedRenderBufferSetup;
	if(!m_Display2.CreateRenderBuffer(&m_LocalSharedRenderBuffer, localSharedRenderBufferSetup))
	{
		LOGFATAL(g_Log, "Failed to create local shared render buffer\n");
		return false;
	}

	//LOGINFO(Display::g_Log, "Allocating local shared colour\n");
	//DumpDisplaySurface(m_LocalSharedRenderBuffer.GetColourSurface(0));
	//LOGINFO(Display::g_Log, "Allocating local shared depth stencil\n");
	//DumpDisplaySurface(m_LocalSharedRenderBuffer.GetDepthStencilSurface());

	return true;
}

void C_GCMDevice::DestroyLocalSharedRenderBuffer()
{
	DestroyRenderBuffer(&m_LocalSharedRenderBuffer);
}

C_RenderBuffer* C_GCMDevice::GetLocalSharedRenderBuffer()
{
	return &m_LocalSharedRenderBuffer;
}

C_Surface* C_GCMDevice::GetLocalSharedColourSurface()
{
	return m_LocalSharedRenderBuffer.GetColourSurface(0);
}

C_Texture* C_GCMDevice::GetLocalSharedColourTexture()
{
	return m_LocalSharedRenderBuffer.GetColourTexture(0);
}

C_Surface* C_GCMDevice::GetLocalSharedColourSurface(const U32 width, const U32 height, const Display2::E_Multisample multisample, const Display2::E_SurfaceFormat colourFormat)
{
	if(	m_LocalSharedRenderBuffer.GetWidth() == width &&
		m_LocalSharedRenderBuffer.GetHeight() == height && 
		m_LocalSharedRenderBuffer.GetMultisample() == multisample &&
		m_LocalSharedRenderBuffer.GetColourSurfaceFormat(0) == colourFormat)
	{
		return m_LocalSharedRenderBuffer.GetColourSurface(0);
	}
	else
	{
		return NULL;
	}
}

C_Surface* C_GCMDevice::GetLocalSharedColourSurface(const U32 width, const U32 height, const E_Multisample multisample, const E_Format colourFormat)
{
	return GetLocalSharedColourSurface(width, height, GetMultisample2(multisample), GetSurfaceFormat2(colourFormat));
}

C_Surface* C_GCMDevice::GetLocalSharedDepthStencilSurface()
{
	return m_LocalSharedRenderBuffer.GetDepthStencilSurface();
}

C_Texture* C_GCMDevice::GetLocalSharedDepthStencilTexture()
{
	return m_LocalSharedRenderBuffer.GetDepthStencilTexture();
}

C_Surface* C_GCMDevice::GetLocalSharedDepthStencilSurface(const U32 width, const U32 height, const Display2::E_Multisample multisample, const Display2::E_SurfaceFormat depthStencilFormat)
{
	if(	m_LocalSharedRenderBuffer.GetWidth() == width &&
		m_LocalSharedRenderBuffer.GetHeight() == height && 
		m_LocalSharedRenderBuffer.GetMultisample() == multisample &&
		m_LocalSharedRenderBuffer.GetDepthStencilSurfaceFormat() == depthStencilFormat)
	{
		return m_LocalSharedRenderBuffer.GetDepthStencilSurface();
	}
	else
	{
		return NULL;
	}
}

C_Surface* C_GCMDevice::GetLocalSharedDepthStencilSurface(const U32 width, const U32 height, const E_Multisample multisample, const E_Format depthStencilFormat)
{
	return GetLocalSharedDepthStencilSurface(width, height, GetMultisample2(multisample), GetSurfaceFormat2(depthStencilFormat));
}

bool C_GCMDevice::IsZcullUsed() const
{
	return g_debugZcullEnable;
}

bool C_GCMDevice::IsScullUsed() const
{
	return g_debugScullEnable;
}

bool C_GCMDevice::IsZcullEnabled() const
{
	return !IsZcullUsed() ? false : m_ZcullEnable;
}

bool C_GCMDevice::IsScullEnabled() const
{
	return !IsScullUsed() ? false : m_ScullEnable;
}

void C_GCMDevice::SetZcullEnable(bool zcullEnable, bool scullEnable)
{
	if(!IsZcullUsed())
		zcullEnable = false;
	if(!IsScullUsed())
		scullEnable = false;

	cellGcmSetZcullEnable(zcullEnable ? CELL_GCM_TRUE : CELL_GCM_FALSE, scullEnable ? CELL_GCM_TRUE : CELL_GCM_FALSE);

	m_ZcullEnable = zcullEnable;
	m_ScullEnable = scullEnable;
}

void C_GCMDevice::SetInvalidateVertexCache()
{
	if(!m_InvalidateVertexCache)
		return;

	cellGcmSetInvalidateVertexCache();

	m_InvalidateVertexCache = false;
}

}; // namespace GCM
}; // namespace Display
