#include "stdafx.h"
#include "Display/Manager/GCM/GCMManager.h"
#include "Display/Manager/GCM/GCMDebug.h"
#include "Display/Buffer/GCM/GCMUserFifo.h"
#include "Display/Buffer/GCM/GCMDebugCommandBuffer.h"
#include "Display/Memory/GCM/GCMRingBuffer.h"
#include "Display/Device/DeviceMarkers.h"
#include <Display2/Debug/Debug.h>
#include <System/Memory/MemoryTracker.h>

#ifdef USE_GCM_HUD
#include <cell/gcm_pm.h>
#endif

#ifdef USE_GCM_P3X
#include "Display/Utils/GCM/libp3x.h"
#endif

#if CFG_GOLD
//#define STOP_ON_GPU_CRASH_IN_MASTER_BUILD 1 // DO NOT SHIP WITH THIS!
#endif

namespace Display
{
namespace GCM
{

void GCMErrorHandler(const uint32_t val)
{
#if CFG_GOLD
	#if STOP_ON_GPU_CRASH_IN_MASTER_BUILD
	printf("A serious error has thrown in libgcm Master build.\n");
	DebugBreak();
	#endif
#else
	#if DEBUG_COMMAND_BUFFER
	//g_MainDebugCommandBuffer.Dump("DefaultCommandBufferUsage");
	//g_UserDebugCommandBuffer.Dump("ContextCommandBufferUsage");
	#endif

	#if USE_DEVICE_MARKERS
	C_DeviceMarkerManager::GetInstance().Dump();
	#endif

	//LOGINFO(g_Log, "RingBufferUsage : Local %8i Main %8i\n", RingBuffer()->GetUsedSize(), RingBuffer()->GetMainUsedSize());

	#if CFG_GOLD
	const char* cfg = "Master";
	#elif CFG_PROFILE
	const char* cfg = "Profile";
	#elif CFG_DEBUG
	const char* cfg = "Debug";
	#elif CFG_RELEASE
	const char* cfg = "Release";
	#elif CFG_SPEED
	const char* cfg = "Speed";
	#else
	const char* cfg = "Unknown";
	#endif

	LOGERROR(g_Log, "A serious error has thrown in libgcm %s build. Hit continue to get a gpu dump. (tick 'properties|target|load options|enable gcm debug' inside target manager if you don't get a gpu dump)\n", cfg);

	DebugBreak();

#if 0 && TRACK_DISPLAY2_COMMAND_BUFFERS
	Display2::g_CommandBufferWalker.m_pCurrTailAddress = (U8*)gCellGcmCurrentContext->current;

	CellGcmControl* pGcmControl = cellGcmGetControlRegister();
	CORE_PRINT("put %.8x get %.8x ref %.8x\n", pGcmControl->put, pGcmControl->get, pGcmControl->ref);
	
	Display2::C_CommandBufferInstance* pInstance = Display2::g_CommandBufferTracker.FindInstance((U32)pGcmControl->get - 4096);
	if(pInstance)
		Display2::WalkCommandBufferAddress(pInstance->m_pHeadAddress, pInstance->m_pTailAddress, true);
	else
		CORE_PRINT("Failed to find offset %.8x\n", pGcmControl->get);
	
	CORE_PRINT("PrevHeadAddress : %.8x\n", Display2::g_CommandBufferWalker.m_pPrevHeadAddress);
	CORE_PRINT("PrevTailAddress : %.8x\n", Display2::g_CommandBufferWalker.m_pPrevTailAddress);
	CORE_PRINT("PrevHeadOffset : %.8x\n", Display2::AddressToOffset(Display2::g_CommandBufferWalker.m_pPrevHeadAddress));
	CORE_PRINT("PrevTailOffset : %.8x\n", Display2::AddressToOffset(Display2::g_CommandBufferWalker.m_pPrevTailAddress));
	
	CORE_PRINT("CurrHeadAddress : %.8x\n", Display2::g_CommandBufferWalker.m_pCurrHeadAddress);
	CORE_PRINT("CurrTailAddress : %.8x\n", Display2::g_CommandBufferWalker.m_pCurrTailAddress);
	CORE_PRINT("CurrHeadOffset : %.8x\n", Display2::AddressToOffset(Display2::g_CommandBufferWalker.m_pCurrHeadAddress));
	CORE_PRINT("CurrTailOffset : %.8x\n", Display2::AddressToOffset(Display2::g_CommandBufferWalker.m_pCurrTailAddress));

	CORE_PRINT("IOAddress : %.8x\n", (U32)C_GCMManager::GetIOAddress() + C_GCMManager::GetIOSize());

	Display2::g_CommandBufferTracker.Dump();
#endif

#endif
}

U32 C_GCMManager::s_cbSize			= 256 * 1024;

#if 1

U32 C_GCMManager::s_ioSize			= 1024 * 1024 * 1; // We're using our own default command buffer however min size is 1MB >:-(

#else

#ifdef USE_GCM_USER_FIFO
U32 C_GCMManager::s_ioSize			= 8 * 1024 * 1024;
#else
#ifdef USE_GCM_HUD
U32 C_GCMManager::s_ioSize			= 4 * 1024 * 1024;
#else
U32 C_GCMManager::s_ioSize			= 4 * 1024 * 1024;
#endif
#endif

#endif

void* C_GCMManager::s_ioAddress		= NULL;
bool C_GCMManager::s_Initialised	= false;

C_GCMManager::C_GCMManager()
{
}

C_GCMManager::~C_GCMManager()
{
	if(s_ioAddress)
	{
		BizAlignedFree(s_ioAddress);
		s_ioAddress = NULL;
	}
}

U32 C_GCMManager::GetCBSize()
{
	return s_cbSize;
}

U32 C_GCMManager::GetIOSize()
{
	return s_ioSize;
}
	
void* C_GCMManager::GetIOAddress()
{
	return s_ioAddress;
}

bool C_GCMManager::IsInitialised()
{
	return s_Initialised;
}

bool C_GCMManager::Initialise()
{
	if(s_Initialised)
	{
		LOGWARN(g_Log, "Libgcm already initialised\n");
		return false;
	}

	#ifdef USE_GCM_REPLAY
	cell::Gcm::Replay::Network::Init();
	cell::Gcm::Replay::Capture::Init();
	#endif

	if(!s_ioAddress)
	{
		//#define OFFSET_IO_ADDRESS 1
		
		#if OFFSET_IO_ADDRESS
		Ut::Vector<void*> ptrList;
		for(S32 i=0; i<50; i++)
		{
			void* ptr = BizAlignedMalloc(1*1024*1024, 1024*1024);
			if(ptr)
			{
				ptrList.PushBack(ptr);
			}
			else
			{
				LOGWARN(g_Log, "Failed to alloc physical memory %i of size %i\n", i, 1*1024*1024);

				for(S32 i=0; i<ptrList.Size(); i++)
					BizAlignedFree(ptrList[i]);
				ptrList.Clear();

				break;
			}
		}
		#endif

		MEMORY_TRACKER_PUSH("graphics.display.device.libgcm");
		s_ioAddress = BizAlignedMalloc(s_ioSize, 1024*1024);
		MEMORY_TRACKER_POP("graphics.display.device.libgcm");

		#if OFFSET_IO_ADDRESS
		for(S32 i=0; i<ptrList.Size(); i++)
			BizAlignedFree(ptrList[i]);
		ptrList.Clear();
		#endif

		if(s_ioAddress)
		{
			#ifdef USE_GCM_USER_FIFO
			if(cellGcmInit(s_cbSize, s_ioSize, s_ioAddress) != CELL_OK)
			#else
			if(cellGcmInit(s_ioSize-4096, s_ioSize, s_ioAddress) != CELL_OK)
			#endif
			{
				BizAlignedFree(s_ioAddress);
				s_ioAddress = NULL;

				LOGFATAL(g_Log, "Failed to initialise libgcm\n");
				return false;
			}
		}
		else
		{
			LOGFATAL(g_Log, "Failed to create the io memory needed by libgcm\n");
			return false;
		}
	}

	#if defined(CELL_GCM_DEBUG) && defined(USE_GCM_USER_FIFO)
	gCellGcmDebugCallback = NULL;
	#endif

	#ifdef USE_GCM_USER_FIFO
	void* pUserFifoAddress = (void*)((uint32_t)s_ioAddress + s_cbSize);
	const uint32_t userFifoSize = s_ioSize - s_cbSize - 0x2000;
	GCMInitUserFifo(pUserFifoAddress, userFifoSize);
	#endif

	#if defined(CELL_GCM_DEBUG) && defined(USE_GCM_USER_FIFO)
	gCellGcmDebugCallback = GCMDebugFinish;
	#endif 

	#ifdef USE_GCM_HUD
	//cellGcmInitPerfMon();
	#endif

	#if defined(BUILD_CONFIG_MASTER)
	#if STOP_ON_GPU_CRASH_IN_MASTER_BUILD
	cellGcmSetDebugOutputLevel(CELL_GCM_DEBUG_LEVEL2);
	cellGcmSetGraphicsHandler(GCMErrorHandler);
	#else
	cellGcmSetDebugOutputLevel(CELL_GCM_DEBUG_LEVEL0);
	cellGcmSetGraphicsHandler(NULL);
	#endif
	#elif defined(BUILD_CONFIG_RELEASE)
	cellGcmSetDebugOutputLevel(CELL_GCM_DEBUG_LEVEL2);
	cellGcmSetGraphicsHandler(GCMErrorHandler);
	#elif defined(BUILD_CONFIG_DEBUG)
	cellGcmSetDebugOutputLevel(CELL_GCM_DEBUG_LEVEL2);
	cellGcmSetGraphicsHandler(GCMErrorHandler);
	GCMEnableDebug();
	#else
	cellGcmSetDebugOutputLevel(CELL_GCM_DEBUG_LEVEL2);
	cellGcmSetGraphicsHandler(GCMErrorHandler);
	#endif

	#ifdef USE_GCM_REPLAY
	SetCaptureBuffers();
	#endif

	#ifdef USE_GCM_P3X
	MEMORY_TRACKER_PUSH("graphics.display.p3x");
	p3x_prof_init(4000); // Biggest shooter uses is approx 3400...
	MEMORY_TRACKER_POP("graphics.display.p3x");
	#endif

	cellGcmGetConfiguration(&Display2::g_gcmConfig);

	SetMainThread();

	s_Initialised = true;
	
	return true;
}

void C_GCMManager::Shutdown()
{
}

#ifdef USE_GCM_REPLAY
void C_GCMManager::SetCaptureBuffers()
{
	unsigned int tempOffset, capture0Offset, capture1Offset;
	unsigned int captureBufferSize = cell::Gcm::Replay::Capture::kBufferVideo1920x1080;
	{
		unsigned int memorySize = ( ((2*captureBufferSize+8192)+((1<<20)-1)) & ~0xfffff );
		void *addr = memalign(1024*1024, memorySize);
		unsigned offset;
		
		cellGcmMapMainMemory( addr, memorySize, &offset );
		cellGcmAddressToOffset( addr, &capture0Offset );
		
		capture1Offset = capture0Offset + captureBufferSize;
		tempOffset = capture1Offset + captureBufferSize;
	}

	cell::Gcm::Replay::Capture::SetCaptureBuffers(
		tempOffset,
		capture0Offset,
		capture1Offset,
		captureBufferSize,
		new char[cell::Gcm::Replay::Capture::kTempBlockTotalBytes]
	);
}
#endif

bool C_GCMManager::GetVideoMode(C_VideoMode* pVideoMode)
{
	assert(pVideoMode);

	CellVideoOutState gcmVideoState;
	if(cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &gcmVideoState) != CELL_OK)
	{
		LOGERROR(g_Log, "Failed to get resolution - cellVideoOutGetState\n");
		return false;
	}

	CellVideoOutResolution gcmResolution;
	if(cellVideoOutGetResolution(gcmVideoState.displayMode.resolutionId, &gcmResolution) != CELL_OK)
	{
		LOGERROR(g_Log, "Failed to get resolution - cellVideoOutGetResolution\n");
		return false;
	}
	
	pVideoMode->m_Width		= gcmResolution.width;
	pVideoMode->m_Height	= gcmResolution.height;

	switch(gcmVideoState.displayMode.resolutionId)
	{
	case CELL_VIDEO_OUT_RESOLUTION_480:
		pVideoMode->m_HiDef			= false;
		break;
	case CELL_VIDEO_OUT_RESOLUTION_576:
		pVideoMode->m_HiDef			= false;
		break;
	case CELL_VIDEO_OUT_RESOLUTION_720:
		pVideoMode->m_HiDef			= true;
		break;
	case CELL_VIDEO_OUT_RESOLUTION_1080:
		pVideoMode->m_HiDef			= true;;
		break;
	case CELL_VIDEO_OUT_RESOLUTION_960x1080:
		pVideoMode->m_HiDef			= true;
		break;
	case CELL_VIDEO_OUT_RESOLUTION_1280x1080:
		pVideoMode->m_HiDef			= true;
		break;
	case CELL_VIDEO_OUT_RESOLUTION_1440x1080:
		pVideoMode->m_HiDef			= true;
		break;
	case CELL_VIDEO_OUT_RESOLUTION_1600x1080:
		pVideoMode->m_HiDef			= true;
		break;
	default:
		LOGERROR(g_Log, "Failed to get resolution - Unsupported resolution\n");
		return false;
	}

	switch(gcmVideoState.displayMode.aspect)
	{
	case CELL_VIDEO_OUT_ASPECT_4_3:
		pVideoMode->m_WideScreen = false;
		break;
	default:
		pVideoMode->m_WideScreen = true;
		break;
	}
	
	return true;
}

}; // namespace GCM
}; // namespace Display
