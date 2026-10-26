#include "AudioPch.h"
#include "SystemHeaders.h"
#include "instance.h"

#include <System/Perf.h>
#include <System/Serialise/C_FileSystem.h>
#include <System/memory/MemoryTracker.h>
#include <System/misc/Random.h>
#include <System/time/Time.h>

#ifdef PLATFORM_PS3
#include <sysutil/sysutil_bgmplayback.h>
//#include "System/SPU/Spurs.h"
//#include "System/SPU/TaskSet.h"
//#include "System/SPU/Queue.h"
//#include "System/SPU/Scheduler.h"
//extern uint32_t _binary_audio_spu_ps3_elf_start;
//extern uint32_t _binary_audio_spu_ps3_elf_size;
//extern uint32_t _binary_audio_spu_ps3_elf_end;
//Spu::Scheduler Audio::System::m_SpuScheduler;
//Spu::Task Audio::System::m_SpuTask;
#endif
#ifdef PLATFORM_PC
#include "core/engine_pc.h"	// [Blur2PC] T144
#endif

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

#ifdef XMA_CONTEXT_DEBUG_LIST
Debug::Bool g_DebugDumpXMAs("Audio.System.DumpActiveXMAs", false);
extern void DumpWavNames();
#endif

U32 Audio::Core::uFrameNum = 0;

DEBUG_ENUM(::DebugLevel::Level)
DE_OPTION(DebugLevel::None,	"Default")
DE_OPTION(DebugLevel::Fatal,"Fatal")
DE_OPTION(DebugLevel::Error,"Error")
DE_OPTION(DebugLevel::Warn,	"Warn")
DE_OPTION(DebugLevel::Info,	"Info")
DE_OPTION(DebugLevel::Debug,"Debug")
END_DEBUG_ENUM

////////////////////////////////////////////////////////////////////////////////
//	static variables
////////////////////////////////////////////////////////////////////////////////
Audio::System* Audio::System::m_pSystem = NULL;
Audio::AllocateMemory Audio::System::m_pfnAlloc = NULL;
Audio::FreeMemory Audio::System::m_pfnFree = NULL;
Audio::AllocateMemory Audio::System::m_pfnPhysAlloc = NULL;
Audio::FreeMemory Audio::System::m_pfnPhysFree = NULL;

Audio::DatPathCallback Audio::System::m_pfnGetDatPath = NULL;

Audio::FileOpenFunc Audio::System::m_openFunc = NULL;
Audio::FileReadFunc Audio::System::m_readFunc = NULL;
Audio::FileSeekFunc Audio::System::m_seekFunc = NULL;
Audio::FileWriteFunc Audio::System::m_writeFunc = NULL;
Audio::FileCloseFunc Audio::System::m_closeFunc = NULL;
Audio::FileSizeFunc Audio::System::m_sizeFunc = NULL;
Audio::FilePauseFunc Audio::System::m_pauseFunc = NULL;
Audio::FileResumeFunc Audio::System::m_resumeFunc = NULL;

#ifdef PLATFORM_PS3
Audio::SystemSpuInfo* Audio::System::m_pSystemSpuInfo = NULL;
#endif

#ifndef CFG_GOLD
Debug::Function DumpDefinitionsLoaded("Audio.System.InstanceStats.DumpObjectAndInstanceStats", Audio::System::Get()->DumpDefinitionList, 0);
Debug::Function LogPeakTempFrameBufferUsage("Audio.System.LogPeakInstanceTempBufferUsage", Audio::System::LogPeakTempInstanceBufferUsage, 0);
Debug::Enum<DebugLevel::Level> AudioDebugLevel("Audio.System.AudioDebugLevel", DebugLevel::None, Debug::Persist());
Debug::Bool gbCoreDebugLevelInheritGlobalLevel("Audio.System.InheritCoreDebugLevelFromAudioDebugLevel", true, Debug::Persist());
Debug::Enum<DebugLevel::Level> AudioDebugLevelCore("Audio.System.CoreAudioDebugLevel", DebugLevel::None, Debug::Persist());
Debug::Bool ForceStereo("Audio.System.ForceStereo", false);
Debug::Bool Headphone("Audio.System.Headphone", false);
Debug::Bool g_bClearNamedCounters("Audio.System.Performance.ClearNamedPerfBarTags", false);
Debug::Int g_iAudioThreadProcessor("Audio.System.RendererThread", 4, Debug::Range(0, 5));
Debug::Bool g_DbgMessageQueueResetPeak("Audio.System.Performance.ResetMessageQueuePeakUsage", false);
Debug::Bool gbDebugDumpPeakXmaBufferUsage("Audio.System.Xma.DumpPeakXmaBufferUsage", false);
#endif

Debug::Bool g_bDoBizAllocations("Audio.System.DoBizAllocations", false);

void DefaultPrintAt(F32 f32X, F32 f32Y, U32 u32RGBA, F32 f32Size, char* strText)
{
	AUDIO_SYS_WARN("calling audio engine PrintAt callback when it hasn't been set");
}


void DefaultDrawBox(F32 f32XPos, F32 f32YPos, F32 f32XSize, F32 f32YSize, U32 u32RGBA)
{
	AUDIO_SYS_WARN("calling audio engine DrawBox callback when it hasn't been set");
}


Audio::PrintFunc Audio::System::m_PrintAtCallback = DefaultPrintAt;
Audio::DrawBoxCallback Audio::System::m_pDrawBoxCallback = DefaultDrawBox;


////////////////////////////////////////////////////////////////////////////////
//	new
////////////////////////////////////////////////////////////////////////////////
void* Audio::System::operator new(size_t sz)
{
	return Alloc(sz);
}

////////////////////////////////////////////////////////////////////////////////
//	delete
////////////////////////////////////////////////////////////////////////////////
void Audio::System::operator delete(void* ptr)
{
	Free(ptr);
}

////////////////////////////////////////////////////////////////////////////////
//	CreateSystem
////////////////////////////////////////////////////////////////////////////////
Audio::ISystem* Audio::CreateSystem(Audio::ISystem::InitParams* pInitParams)
{
    return (ISystem*)System::Create(pInitParams);
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseSystem
////////////////////////////////////////////////////////////////////////////////
void Audio::ReleaseSystem()
{
    System::Release();
}

////////////////////////////////////////////////////////////////////////////////
//	GetSystem
////////////////////////////////////////////////////////////////////////////////
Audio::ISystem* Audio::GetSystem()
{
	return System::Get();
}

////////////////////////////////////////////////////////////////////////////////
//	Create
////////////////////////////////////////////////////////////////////////////////


Audio::System* Audio::System::Create(Audio::ISystem::InitParams* pInitParams)
{
	m_pfnAlloc = pInitParams->pfnAllocate;
	m_pfnFree = pInitParams->pfnFree;
	m_pfnPhysAlloc = pInitParams->pfnPhyAllocate;
	m_pfnPhysFree = pInitParams->pfnPhyFree;
	m_pfnGetDatPath = pInitParams->pfnGetDatPath;

    if (System::m_pSystem == NULL)
        m_pSystem = new Audio::System(pInitParams);

#ifdef PLATFORM_PS3
	if (System::m_pSystemSpuInfo == NULL)
		System::m_pSystemSpuInfo = new SystemSpuInfo;
#endif

    return m_pSystem;
}


////////////////////////////////////////////////////////////////////////////////
//	Release
////////////////////////////////////////////////////////////////////////////////
void Audio::System::Release()
{
    if (m_pSystem)
    {
        m_pSystem->Shutdown();
        delete m_pSystem;
        m_pSystem = NULL;

#ifdef PLATFORM_PS3
	if (m_pSystemSpuInfo)
		delete m_pSystemSpuInfo;
#endif

    }
}

////////////////////////////////////////////////////////////////////////////////
//	GetFramesDropped
////////////////////////////////////////////////////////////////////////////////

U32 Audio::System::GetFramesDropped()
{
	return m_pRenderer->GetFramesDropped();
}


////////////////////////////////////////////////////////////////////////////////
//	SetReverb
////////////////////////////////////////////////////////////////////////////////
#ifdef PLATFORM_PC
void Audio::System::SetReverb(EAXREVERBPROPERTIES* pSettings, U32 uIndex)
{
	m_pRenderer->SetReverb(pSettings, uIndex);
}
#endif	//	PLATFORM_PC

#ifdef PLATFORM_PS3
#ifdef PS3_OUTPUT_TO_MULTISTREAM
#ifdef AUDIO_USE_OLD_REVERB
void Audio::System::SetReverb(CellMSFXReverbParams* pSettings, U32 uIndex)
#else
void Audio::System::SetReverb(CellMSFXI3DL2Params* pSettings, U32 uIndex)
#endif	//	AUDIO_USE_OLD_REVERB
{
	m_pRenderer->SetReverb(pSettings, uIndex);
}
#endif	//	PS3_OUTPUT_TO_MULTISTREAM
#endif	//	PLATFORM_PS3


////////////////////////////////////////////////////////////////////////////////
//	SpeakerConfig
////////////////////////////////////////////////////////////////////////////////
//void Audio::System::SetSpeakerConfig( SpeakerConfig eMode )
//{
//	switch( eMode )
//	{
//	case eStereo:
//		{
//			m_SystemState.m_uNumSpeakers = 3;
//			m_SystemState.m_bHasCentre = false;
//			memcpy( m_SystemState.m_afAngles, m_af3Angles, 4 * sizeof(F32) );
//			m_SystemState.m_ePanningMode = MixerComponent::eStereo;
//		}
//		break;
//
//	case eFivePointOne:
//		{
//			m_SystemState.m_uNumSpeakers = 5;
//			m_SystemState.m_bHasCentre = true;
//			memcpy( m_SystemState.m_afAngles, m_af5Angles, 6 * sizeof(F32) );
//
//#ifdef CFG_GOLD
//			m_SystemState.m_ePanningMode = MixerComponent::eFivePointZero;
//#else
//			if( AudioUseAmbisonicPanner )
//				m_SystemState.m_ePanningMode = MixerComponent::eFivePointZeroAmbisonic;
//			else
//				m_SystemState.m_ePanningMode = MixerComponent::eFivePointZero;
//#endif
//		}
//		break;
//
//	case eSevenPointOne:
//		{
//			m_SystemState.m_uNumSpeakers = 7;
//			m_SystemState.m_bHasCentre = true;
//			memcpy( m_SystemState.m_afAngles, m_af7Angles, 8 * sizeof(F32) );
//			m_SystemState.m_ePanningMode = MixerComponent::eSevenPointZero;
//		}
//		break;
//	}
//
//	m_SystemState.m_fSpeakerDistance = m_fSpeakerDistances[ eMode ];
//}

#ifdef PLATFORM_PC
////////////////////////////////////////////////////////////////////////////////
//	SetOutputSampleRate
////////////////////////////////////////////////////////////////////////////////
// [Blur2PC] T144: retail PC 0x0053aad0
void Audio::System::SetOutputSampleRate( U32 uSampleRate )
{
	Audio::Core::Engine::Instance().SetOutputSampleRate( uSampleRate );
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	SetSpeakerOption
////////////////////////////////////////////////////////////////////////////////
void Audio::System::SetSpeakerOption( SpeakerOption eOption )
{
	switch( eOption )
	{
	case eHeadphone:
		{
			m_SystemState.m_uNumSpeakers = 3;
			m_SystemState.m_bHasCentre = false;
			m_SystemState.m_afAngles[ 0 ] = 0.f;
			m_SystemState.m_afAngles[ 1 ] = 90.0f / 180.f;
			m_SystemState.m_afAngles[ 2 ] = 270.0f / 180.f;
			m_SystemState.m_afAngles[ 3 ] = 360.f / 180.f;
			m_SystemState.m_ePanningMode = MixerComponent::eStereo;
		}
		break;

	case eStereoHiFi:
		{
			m_SystemState.m_uNumSpeakers = 3;
			m_SystemState.m_bHasCentre = false;
			memcpy( m_SystemState.m_afAngles, m_af3Angles, 4 * sizeof(F32) );
			m_SystemState.m_ePanningMode = MixerComponent::eStereo;
		}
		break;

	case eSurround:
		{
#ifdef PS3_USES_SEVEN_POINT_ONE
			m_SystemState.m_uNumSpeakers = 7;
			m_SystemState.m_bHasCentre = true;
			memcpy( m_SystemState.m_afAngles, m_af7Angles, 8 * sizeof(F32) );
			m_SystemState.m_ePanningMode = MixerComponent::eSevenPointZero;
#else
			m_SystemState.m_uNumSpeakers = 5;
			m_SystemState.m_bHasCentre = true;
			memcpy( m_SystemState.m_afAngles, m_af5Angles, 6 * sizeof(F32) );
			m_SystemState.m_ePanningMode = MixerComponent::eFivePointZero;
#endif // PS3_USES_SEVEN_POINT_ONE
		}
		break;
	}

	m_SystemState.m_fSpeakerDistance = m_fSpeakerDistances[ eOption ];
}

////////////////////////////////////////////////////////////////////////////////
//	SetSpeakerDistance
////////////////////////////////////////////////////////////////////////////////
void Audio::System::SetSpeakerDistance( SpeakerOption eOption, F32 fDistance )
{
	m_fSpeakerDistances[ eOption ] = fDistance;
}

////////////////////////////////////////////////////////////////////////////////
//	SetSpeakerAngles
////////////////////////////////////////////////////////////////////////////////
void Audio::System::SetSpeakerAngles( SpeakerConfig eMode, F32 fFrontLeft, F32 fFrontRight, F32 fSurroundLeft, F32 fSurroundRight, F32 fBackLeft, F32 fBackRight )
{
	switch( eMode )
	{
	case eStereo:
		{
			m_af3Angles[ 0 ] = 0.f;
			m_af3Angles[ 1 ] = fFrontRight / 180.f;
			m_af3Angles[ 2 ] = fFrontLeft / 180.f;
			m_af3Angles[ 3 ] = 360.f / 180.f;
		}
		break;

	case eFivePointOne:
		{
			m_af5Angles[ 0 ] = 0.f;
			m_af5Angles[ 1 ] = fFrontRight / 180.f;
			m_af5Angles[ 2 ] = fSurroundRight / 180.f;
			m_af5Angles[ 3 ] = fSurroundLeft / 180.f;
			m_af5Angles[ 4 ] = fFrontLeft / 180.f;
			m_af5Angles[ 5 ] = 360.f / 180.f;
		}
		break;

	case eSevenPointOne:
		{
			m_af7Angles[ 0 ] = 0.f;
			m_af7Angles[ 1 ] = fFrontRight / 180.f;
			m_af7Angles[ 2 ] = fSurroundRight / 180.f;
			m_af7Angles[ 3 ] = fBackRight / 180.f;
			m_af7Angles[ 4 ] = fBackLeft / 180.f;
			m_af7Angles[ 5 ] = fSurroundLeft / 180.f;
			m_af7Angles[ 6 ] = fFrontLeft / 180.f;
			m_af7Angles[ 7 ] = 360.f / 180.f;
		}
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	CreateWaveManager
////////////////////////////////////////////////////////////////////////////////
Audio::IWaveManager* Audio::CreateWaveManager()
{
    return (IWaveManager*)new Audio::WaveBankManager;
}

////////////////////////////////////////////////////////////////////////////////
//	ReleaseSystem
////////////////////////////////////////////////////////////////////////////////
void Audio::ReleaseWaveManager(IWaveManager* pWaveManager)
{
    delete pWaveManager;
}

U32 GetAlignedOffset(U32 u32Alignment, U32 u32Size)
{
	U32 u32RetValue = u32Alignment - (u32Size % u32Alignment);
	return (u32RetValue == u32Alignment) ? 0 : u32RetValue;
}

////////////////////////////////////////////////////////////////////////////////
//	Alloc
////////////////////////////////////////////////////////////////////////////////
void* Audio::System::Alloc(size_t uSize, U32 uAlignment, AllocType eType)
{
	void* pMem = NULL;

	#ifdef PLATFORM_PS3
	if (uAlignment > 128) uAlignment = 128;
	#endif

	if (eType == eVirtual)
	{
		if (m_pfnAlloc)
			return m_pfnAlloc(uSize, uAlignment);

#ifndef CFG_GOLD
		if (g_bDoBizAllocations)
			return BizAlignedMalloc(uSize, uAlignment);
#endif

		// add 4 bytes that will act as a header, and store the location at which the actual allocation begins,
		// once the header and memory alignment have been taken into account
		// ..we'll need this information when we come to free the memory, as the address passed in to free won't
		// be the actual address we need to free memory from

		// [Blur2PC][x64] The header holds a pointer: sizeof(void*) bytes (4 on x86).
		U32 u32ActualSize = uSize + sizeof(void*) + uAlignment;

		// also account for alignment
		u32ActualSize = ((u32ActualSize / uAlignment) * uAlignment);

		if (uAlignment >= 2)
			u32ActualSize += uAlignment;

		pMem = (void*)(new U8[u32ActualSize]);

		// work out location of 4-byte header..
		// find next aligned memory location after allocated address plus 4 byte header
		U8* pReturnAddress = (U8*)pMem + sizeof(void*);
		pReturnAddress += GetAlignedOffset(uAlignment, (U32)(INTPTR)pReturnAddress);	// low address bits decide the alignment
		U8* pHeaderLoc = pReturnAddress - sizeof(void*);

		// populate this location with the address the memory is allocated from
		*((void**)pHeaderLoc) = pMem;

		return (void*)pReturnAddress;
	}

	// else physical alloc

	if (m_pfnPhysAlloc)
		pMem = m_pfnPhysAlloc(uSize, uAlignment);
	else
	{
#if defined(PLATFORM_PS3)
		pMem = BizAlignedMalloc((size_t)uSize, (size_t)uAlignment);
#elif defined(PLATFORM_PC)
		pMem = malloc(uSize);
#else
		Assert(false);
#endif
	}

	Assert(pMem != NULL);

	return pMem;
}

////////////////////////////////////////////////////////////////////////////////
//	Free
////////////////////////////////////////////////////////////////////////////////
void Audio::System::Free(void* pAddress, AllocType eType)
{
	// the address passed in won't be the address we want to free from, but will be preceded by a 4-byte header
	// containing the address we need to use

	if (eType == eVirtual)
	{
		if (m_pfnFree)
		{
			m_pfnFree(pAddress);
			return;
		}

#ifndef CFG_GOLD
		if (g_bDoBizAllocations)
		{
			BizAlignedFree(pAddress);
			return;
		}
#endif

		void* pFreeAddress = *((void**)((U8*)pAddress - sizeof(void*)));	// [Blur2PC][x64] see Alloc
		delete [] (U8*)pFreeAddress;

		return;
	}

	// else physical alloc

	if (m_pfnPhysFree)
		m_pfnPhysFree(pAddress);
	else
	{
#if defined(PLATFORM_PS3)
		BizAlignedFree(pAddress);
#elif defined(PLATFORM_PC)
		free(pAddress);
#else
		Assert(false);
#endif
	}
}
	
////////////////////////////////////////////////////////////////////////////////
//	Callback (static)
////////////////////////////////////////////////////////////////////////////////
void Audio::System::RenderCallback(void* pContext)
{
    Audio::System* pSystem = (Audio::System*)pContext;

    pSystem->Update();
}

////////////////////////////////////////////////////////////////////////////////
//	Post Callback (static)
////////////////////////////////////////////////////////////////////////////////
void Audio::System::PostRenderCallback(void* pContext)
{
    Audio::System* pSystem = (Audio::System*)pContext;

    pSystem->PostUpdate();
}

////////////////////////////////////////////////////////////////////////////////
//	Post Callback (static)
////////////////////////////////////////////////////////////////////////////////
void SetDebugger(Audio::IDebug* pDebugger)
{
}

////////////////////////////////////////////////////////////////////////////////
//	System
////////////////////////////////////////////////////////////////////////////////
Audio::System::System(Audio::ISystem::InitParams* pInitParams) : m_bPaused(false)
{
    m_pMaster = NULL;
    m_pRenderer = NULL;

	m_uNumReverbs = pInitParams->uNumReverbs;

    m_SystemState.m_uFrameNum = 0;

	Util::C_Random rand;
	rand.Seed((U32)Time::GetCurrentTime());
    m_uSeed = (U32)rand.Rand(0xffffffff);

	SetSpeakerDistance( eHeadphone, 1.f );
	SetSpeakerDistance( eStereoHiFi, 2.f );
	SetSpeakerDistance( eSurround, 2.f );

	SetSpeakerAngles( eStereo, 330.f, 30.f );
	SetSpeakerAngles( eFivePointOne, 330.f, 30.f, 250.f, 110.f );
	SetSpeakerAngles( eSevenPointOne, 330.f, 30.f, 270.f, 90.f, 210.f, 150.f );

	SetSpeakerOption( eSurround ); 

	m_SystemState.m_SpeakerAngleHasChanged = false;

	for (U32 i = 0; i < 32; i++)
		m_u32AllocId[i] = 0;

	m_u32AllocIdStackPointer = 0;

	m_pPreCallback = 0;
	m_pPostCallback = 0;

	m_pDebugger = NULL;
	m_prevDebugLevel = DebugLevel::None;

	m_bHighQuality = true;

	m_bStaggerInstanceCreation = false;

	#ifdef PLATFORM_PC
	m_hWnd = (HWND)pInitParams->hWnd;
	#endif

// jkc- commented out to compile. not sure what it was doing..
//#ifdef PLATFORM_PS3
//	// JAN: WARNING: not sure where to free this up! possible memory leak when exiting app
//	m_iCurrentFrameUpdateFinishInfo = 0;
//	for (int i=0; i<MAX_FRAME_UPDATE_INFO; i++)
//	{
//		m_pFrameUpdateFinishInfo[i] = new FrameUpdateFinishInfo;
//		m_pFrameUpdateFinishTask[i] = new InstancePs3PpuData;
//	}
//	m_pFrameFinishedEvent = (CellSpursEventFlag*)BizAlignedMalloc(sizeof(CellSpursEventFlag), 128);
//#endif

//	m_StateMemoryAllocator.Init(pInitParams->uInstanceStateDataBufferSize, 6);	// size in bytes, max number of buffers required

	m_ObjectManager.Init(pInitParams->uTempFrameBufferNumBytes, pInitParams->eDistributedUpdate, pInitParams->hPolicy, pInitParams->hGroup, pInitParams->hPolicyB/*, pInitParams->uInstanceStateDataBufferSize*/);

	MEMORY_TRACKER_PUSH("Audio.SendQueue");
#ifdef PLATFORM_PC
	// [Blur2PC] T144: retail PC's System constructor (0x0054f9c0) passes the size as given; with retail's
	// 128-sample frame QUEUE_SIZE_MULTIPLIER would be 0
	m_Queue.Init(pInitParams->uMessageQueueNumBytes);
#else
	m_Queue.Init(pInitParams->uMessageQueueNumBytes * QUEUE_SIZE_MULTIPLIER);
#endif
	MEMORY_TRACKER_POP("Audio.SendQueue");

	MEMORY_TRACKER_PUSH("Audio.ReceiveQueue");
	m_ReturnQueue.Init(pInitParams->uReturnMessageQueueNumBytes);
	MEMORY_TRACKER_POP("Audio.ReceiveQueue");


#ifdef AUDIO_NEW_DECODERS
	Core::ISampleRateConverter::Init( 4 * Core::uFrameLength );
#endif // AUDIO_NEW_DECODERS
}


////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
bool Audio::System::Init()
{
#if defined (AUDIO_ENABLE_PROFILING)
	Profiler::Instance().InitializeComms();
#endif

	//Frame::Init();

#ifdef PLATFORM_PS3
//#ifdef PS3_AUDIO_USE_SPU

	m_pOverlayTable = 0;

	// libovis overlay stuff. load the .elf file in to generate the overlay table, then release it.

	U8* pElf = 0;
/*
	// mainly for debugging- check if we can load .elf from disk first.
#ifndef CFG_GOLD
	File elfFile;
	if (elfFile.Open("/app_home/image/audio_SPU_ps3.elf", false))
	{
		U32 u32ElfSize = elfFile.Size();
		pElf = (U8*)(Alloc(u32ElfSize));
		elfFile.Read(pElf, u32ElfSize, 0);

		elfFile.Close();
	}
	else
#endif
	{
		U32 u32ElfSize = (U32)(void *)&_binary_audio_spu_ps3_elf_end - (U32)(void *)&_binary_audio_spu_ps3_elf_start;
		
		pElf = (U8*)(Alloc(u32ElfSize));
		memcpy(pElf,(void *)&_binary_audio_spu_ps3_elf_start,u32ElfSize);
	}

	U32 u32TableSize = cellOvisGetOverlayTableSize((const char*)pElf);

	Assert(u32TableSize);

	m_pOverlayTable = (U8*)(Alloc(u32TableSize, 128));

	U32 u32Ret = cellOvisInitializeOverlayTable(m_pOverlayTable, (const char*)pElf);

	Free(pElf);

	if (u32Ret)
	{
		AUDIO_SYS_ERROR("Audio overlay table woes.");
		return false;
	}
*/
/*
	U32 u32TableSize = cellOvisGetOverlayTableSize(_binary_audio_spu_ps3_elf_start);

	Assert(u32TableSize);

	m_pOverlayTable = (U8*)(Alloc(u32TableSize, 128));

	cellOvisInitializeOverlayTable(m_pOverlayTable, _binary_audio_spu_ps3_elf_start);
*/
	// set up spurs task

// commented out, jkc
/*	m_SpuScheduler.Initialise("audio_SPU_ps3.elf", (void*)&_binary_audio_spu_ps3_elf_start, 2048, 1);
	bSpuTaskSubmitted = false;

	int rtn = cellSpursEventFlagInitialize(m_SpuScheduler.GetTaskSet()->GetSpursTaskSetPtr(), m_pFrameFinishedEvent, CELL_SPURS_EVENT_FLAG_CLEAR_AUTO, CELL_SPURS_EVENT_FLAG_SPU2PPU);
	if (rtn)
		DebugBreak();
	rtn = cellSpursEventFlagAttachLv2EventQueue(m_pFrameFinishedEvent);
	if (rtn)
		DebugBreak();
	rtn = cellSpursEventFlagClear(m_pFrameFinishedEvent, 0xffff);
	if (rtn)
		DebugBreak();*/

// TODO: still need the audio SPU setup pass

//	SubmitSpuTask(m_pOverlayTable, 0, eSetup, 0);
//#endif
#endif

    if (m_pRenderer == NULL)
    {
		Core::Renderer* pRenderer = Audio::CreateRenderer();
		bool bUseStereoReverb = true;
        pRenderer->Init(this, RenderCallback, PostRenderCallback, m_uNumReverbs, bUseStereoReverb);
        pRenderer->Start();
        m_pRenderer = pRenderer;
        
    }


	m_bTitleHasControl = 1; //set to control by default

	// moved to Audio::System::System()
	//m_StateMemoryAllocator.Init(20 * 1024, 1);	// size in bytes, max number of buffers required

    //m_pRenderer->SetInput(m_pMaster);
	

    return true;
}

////////////////////////////////////////////////////////////////////////////////
//	SubmitSpuTask
////////////////////////////////////////////////////////////////////////////////
#ifdef PLATFORM_PS3
bool Audio::System::SubmitSpuTask(void* pData, void* pReturnAddress, AudioSpuTaskId eId, U32 u32Size)
{
// commented out.. jkc

/*#ifdef PS3_AUDIO_USE_SPU

	if (bSpuTaskSubmitted)
		return false;

	Assert (U32(pData) % 16 == 0 && U32(pReturnAddress) % 16 == 0);

	bSpuTaskSubmitted = true;

	spuTaskParams.eTaskId = eId;
	spuTaskParams.pSrcAddress = pData;
	spuTaskParams.pRetAddress = pReturnAddress;
	spuTaskParams.u32Size = u32Size;

	m_SpuTask.Initialise((void*)&spuTaskParams);

	m_SpuScheduler.Schedule(&m_SpuTask);

	// temp- block until task completed
	m_SpuTask.Join();

	bSpuTaskSubmitted = false;
#endif*/

	return true;
}

#endif	//	PLATFORM_PS3

////////////////////////////////////////////////////////////////////////////////
//	Update
////////////////////////////////////////////////////////////////////////////////
void Audio::System::Update()
{
	AUDIO_PERF_EVENT("Audio.System.Update");

	if (m_bPaused)
		return;

#if defined (AUDIO_ENABLE_PROFILING)	
	if (!Profiler::Instance().CommsInitialized())		
		Profiler::Instance().InitializeComms();
#endif

#ifndef CFG_GOLD
	if( Headphone )
		SetSpeakerOption( eHeadphone );
	else
	{
		if( ForceStereo )
			SetSpeakerOption( eStereoHiFi );
		else
			SetSpeakerOption( eSurround );
	}
#endif

	MEMORY_TRACKER_PUSH("Audio.SystemUpdate");
	CPU_COUNTER_BEGIN(eCpu_EntireSystem);

	if (m_pPreCallback)
		m_pPreCallback(0);

	GetLock()->Acquire();

#if defined (AUDIO_ENABLE_PROFILING)
	//Profiler::Instance().Lock().Acquire();
#endif

	//  service message queue
	NAMED_CPU_COUNTER_BEGIN("SYS: Msg Q");
	CPU_COUNTER_BEGIN(eCpu_MessageQueue);

	m_Queue.Update();

	#ifndef CFG_GOLD
	if (g_DbgMessageQueueResetPeak)
	{
		m_Queue.ResetPeakQueuedBytes();
		g_DbgMessageQueueResetPeak = false;
	}
	#endif

	CPU_COUNTER_END(eCpu_MessageQueue);
	NAMED_CPU_COUNTER_END("SYS: Msg Q");


	// update streams
	NAMED_CPU_COUNTER_BEGIN("SYS: Streams");
	CPU_COUNTER_BEGIN(eCpu_FileStreamer);

//#ifdef PLATFORM_PS3
//	m_ObjectManager.UpdatePS3Streams();
//#endif

//	m_FileStreamManager.Update();
	GetWaveServer()->Update();

	NAMED_CPU_COUNTER_END("SYS: Streams");
	CPU_COUNTER_END(eCpu_FileStreamer);


	//  update instances/components
	CPU_COUNTER_BEGIN(eCpu_ObjectManagerUpdateAndRender);

	m_ObjectManager.Update();

	CPU_COUNTER_END(eCpu_ObjectManagerUpdateAndRender);


	GetLock()->Release();

	if (m_pPostCallback)
		m_pPostCallback(0);


    m_SystemState.m_uFrameNum++;

	m_SystemState.m_SpeakerAngleHasChanged = false;

	CPU_COUNTER_END(eCpu_EntireSystem);

#ifdef NAMED_COUNTERS_ENABLED
	if (g_bClearNamedCounters)
	{
		System::Get()->GetNamedCpuCounters().Clear();
		g_bClearNamedCounters = false;
	}
	System::Get()->GetNamedCpuCounters().EndAllEvents();
#endif

	
#if defined (AUDIO_ENABLE_PROFILING)
		Profiler::Instance().Reply();
#endif

	//set debug level if changed
#ifndef CFG_GOLD
	if(((AudioDebugLevel != DebugLevel::None) && (AudioDebugLevel != m_prevDebugLevel)) || ((AudioDebugLevelCore != DebugLevel::None) && (AudioDebugLevelCore != m_prevDebugLevelCore)))
	{
		Audio::SetDebugLevels(gbCoreDebugLevelInheritGlobalLevel ? AudioDebugLevel : AudioDebugLevelCore, AudioDebugLevel);	
		m_prevDebugLevel = AudioDebugLevel;
		m_prevDebugLevelCore = AudioDebugLevelCore;
	}
#endif		

#ifndef CFG_GOLD
	if (gbDebugDumpPeakXmaBufferUsage)
	{
		gbDebugDumpPeakXmaBufferUsage = false;
	}
#endif

#ifdef XMA_CONTEXT_DEBUG_LIST
	if( g_DebugDumpXMAs )
	{
		DumpWavNames();
		g_DebugDumpXMAs = false;
	}
#endif		

	MEMORY_TRACKER_POP("Audio.SystemUpdate");
}

////////////////////////////////////////////////////////////////////////////////
//	PostUpdate
////////////////////////////////////////////////////////////////////////////////
void Audio::System::PostUpdate()
{
	if (m_pPostCallback)
		m_pPostCallback(0);
}

////////////////////////////////////////////////////////////////////////////////
//	Release
////////////////////////////////////////////////////////////////////////////////
bool Audio::System::Shutdown()
{
    if (m_pRenderer)
    {
        delete m_pRenderer;
        m_pRenderer = NULL;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
//	SetMaster
////////////////////////////////////////////////////////////////////////////////
void Audio::System::SetMaster(InstanceComponentHandle hComponent)
{
    AUDIO_SYS_DEBUG("System::SetMaster()");

	GetQueue()->SendItem(Core::SendQueue::eSetMaster, &hComponent, sizeof(InstanceComponentHandle));
}

////////////////////////////////////////////////////////////////////////////////
//	SetMonoMaster
////////////////////////////////////////////////////////////////////////////////
void Audio::System::SetMonoMaster(InstanceComponentHandle hComponent)
{
    AUDIO_SYS_DEBUG("System::SetMonoMaster()");

	GetQueue()->SendItem(Core::SendQueue::eSetMonoMaster, &hComponent, sizeof(InstanceComponentHandle));
}

////////////////////////////////////////////////////////////////////////////////
//	SetStereoMaster
////////////////////////////////////////////////////////////////////////////////
void Audio::System::SetStereoMaster(InstanceComponentHandle hComponent)
{
    AUDIO_SYS_DEBUG("System::SetStereoMaster()");

	GetQueue()->SendItem(Core::SendQueue::eSetStereoMaster, &hComponent, sizeof(InstanceComponentHandle));
}

////////////////////////////////////////////////////////////////////////////////
//	SetSurroundMaster
////////////////////////////////////////////////////////////////////////////////
void Audio::System::SetSurroundMaster(InstanceComponentHandle hComponent)
{
    AUDIO_SYS_DEBUG("System::SetSurroundMaster()");

	GetQueue()->SendItem(Core::SendQueue::eSetSurroundMaster, &hComponent, sizeof(InstanceComponentHandle));
}

////////////////////////////////////////////////////////////////////////////////
//	SetSurroundLFEMaster
////////////////////////////////////////////////////////////////////////////////
void Audio::System::SetSurroundLFEMaster(InstanceComponentHandle hComponent)
{
    AUDIO_SYS_DEBUG("System::SetSurroundLFEMaster()");

	GetQueue()->SendItem(Core::SendQueue::eSetSurroundLFEMaster, &hComponent, sizeof(InstanceComponentHandle));
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveSetMaster
////////////////////////////////////////////////////////////////////////////////
void Audio::System::ReceiveSetMaster(InstanceComponentHandle hComponent)
{
	Instance* pInst = 0;
	Component* pComp = 0;

	if (! m_ObjectManager.DereferenceInstanceComponentHandle(hComponent, Component::flagOutputConnector, &pInst, &pComp))
	{
		AUDIO_SYS_ERROR("Set Master failed");
		return;
	}

	if (pComp && pInst)
	{
		if (pComp->Flags() & Component::flagMono)
		{
			((MonoAudioOutput*)pComp)->OutputTo(pInst, m_pRenderer->MonoInput());
			AUDIO_SYS_DEBUG("Set Master '%s' '%s' [Mono]", pInst->GetObject()->Name(), pComp->Name());
		}
		else if (pComp->Flags() & Component::flagStereo)
		{
			((StereoAudioOutput*)pComp)->OutputTo(pInst, m_pRenderer->StereoInput());
			AUDIO_SYS_DEBUG("Set Master '%s' '%s' [Stereo]", pInst->GetObject()->Name(), pComp->Name());
		}
		else if (pComp->Flags() & Component::flagSurround)
		{
			((SurroundAudioOutput*)pComp)->OutputTo(pInst, m_pRenderer->SurroundInput());
			AUDIO_SYS_DEBUG("Set Master '%s' '%s' [Surround]", pInst->GetObject()->Name(), pComp->Name());
		}
		else if (pComp->Flags() & Component::flagSurroundLFE)
		{
			((SurroundLFEAudioOutput*)pComp)->OutputTo(pInst, m_pRenderer->SurroundLFEInput());
			AUDIO_SYS_DEBUG("Set Master '%s' '%s' [SurroundLFE]", pInst->GetObject()->Name(), pComp->Name());
		}
		else
		{
			AUDIO_SYS_ERROR("Set Master failed");
			return;
		}

	}
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveSetMonoMaster
////////////////////////////////////////////////////////////////////////////////
void Audio::System::ReceiveSetMonoMaster(InstanceComponentHandle hComponent)
{
	Instance* pInst = 0;
	Component* pComp = 0;

	if (! m_ObjectManager.DereferenceInstanceComponentHandle(hComponent, Component::flagMono | Component::flagOutputConnector, &pInst, &pComp))
	{
		AUDIO_SYS_ERROR("Set Mono Master failed");
	}

	if (pComp && pInst)
	{
		((MonoAudioOutput*)pComp)->OutputTo(pInst, m_pRenderer->MonoInput());
	    AUDIO_SYS_DEBUG("Set Mono Master '%s' '%s'", pInst->GetObject()->Name(), pComp->Name());
	}
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveSetStereoMaster
////////////////////////////////////////////////////////////////////////////////
void Audio::System::ReceiveSetStereoMaster(InstanceComponentHandle hComponent)
{
	Instance* pInst = 0;
	Component* pComp = 0;

	if (! m_ObjectManager.DereferenceInstanceComponentHandle(hComponent, Component::flagStereo | Component::flagOutputConnector, &pInst, &pComp))
	{
		AUDIO_SYS_ERROR("Set Stereo Master failed");
	}

	if (pComp && pInst)
	{
		((StereoAudioOutput*)pComp)->OutputTo(pInst, m_pRenderer->StereoInput());
	    AUDIO_SYS_DEBUG("Set Stereo Master '%s' '%s'", pInst->GetObject()->Name(), pComp->Name());
	}
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveSetSurroundMaster
////////////////////////////////////////////////////////////////////////////////
void Audio::System::ReceiveSetSurroundMaster(InstanceComponentHandle hComponent)
{
	Instance* pInst = 0;
	Component* pComp = 0;

	if (! m_ObjectManager.DereferenceInstanceComponentHandle(hComponent, Component::flagSurround | Component::flagOutputConnector, &pInst, &pComp))
	{
		AUDIO_SYS_ERROR("Set Surround Master failed");
	}

	if (pComp && pInst)
	{	
		((SurroundAudioOutput*)pComp)->OutputTo(pInst, m_pRenderer->SurroundInput());
	    AUDIO_SYS_DEBUG("Set Surround Master '%s' '%s'", pInst->GetObject()->Name(), pComp->Name());
	}
}

////////////////////////////////////////////////////////////////////////////////
//	ReceiveSetSurroundLFEMaster
////////////////////////////////////////////////////////////////////////////////
void Audio::System::ReceiveSetSurroundLFEMaster(InstanceComponentHandle hComponent)
{
	Instance* pInst = 0;
	Component* pComp = 0;

	if (! m_ObjectManager.DereferenceInstanceComponentHandle(hComponent, Component::flagSurroundLFE | Component::flagOutputConnector, &pInst, &pComp))
	{
		AUDIO_SYS_ERROR("Set Surround LFE Master failed");
	}

	if (pComp && pInst)
	{
		((SurroundLFEAudioOutput*)pComp)->OutputTo(pInst, m_pRenderer->SurroundLFEInput());
	    AUDIO_SYS_DEBUG("Set Surround LFE Master '%s' '%s'", pInst->GetObject()->Name(), pComp->Name());
	}
}

////////////////////////////////////////////////////////////////////////////////
//	PushCurrentMemoryAllocationId
////////////////////////////////////////////////////////////////////////////////
#ifdef TRACK_AUDIO_ENGINE_MEMORY
void Audio::System::PushCurrentMemoryAllocationId(U32 u32Id)
{
	Assert (m_u32AllocIdStackPointer < 32);
	m_u32AllocId[m_u32AllocIdStackPointer] = u32Id;
	m_u32AllocIdStackPointer++;
}

////////////////////////////////////////////////////////////////////////////////
//	PopCurrentMemoryAllocationId
////////////////////////////////////////////////////////////////////////////////
void Audio::System::PopCurrentMemoryAllocationId()
{
	Assert (m_u32AllocIdStackPointer > 0);
	m_u32AllocIdStackPointer--;
}

////////////////////////////////////////////////////////////////////////////////
//	QueryCurrentMemoryAllocationId
////////////////////////////////////////////////////////////////////////////////
U32 Audio::System::QueryCurrentMemoryAllocationId()
{
	if (m_u32AllocIdStackPointer > 0)
		return m_u32AllocId[m_u32AllocIdStackPointer - 1];
	else
		return 0;
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	CPUUsage
////////////////////////////////////////////////////////////////////////////////
F32 Audio::System::CPUUsage(U32 uIndex) const
{
#ifdef CPU_COUNTERS_ENABLED
	F32 fAv = m_aCPUCounters[uIndex].Value();
	return 100.f * fAv * (F32)Core::uSampleRate / (F32)Core::uFrameLength;
#else
	return 0.f;
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	DoWork
//		needs to be called regularly by the main thread
////////////////////////////////////////////////////////////////////////////////
void Audio::System::DoWork()
{
	m_ReturnQueue.Update();

#if defined(PLATFORM_PS3)
	CellSysutilBgmPlaybackStatus2 status;
	cellSysutilGetBgmPlaybackStatus2( &status );
	m_bTitleHasControl = (status.playerState == CELL_SYSUTIL_BGMPLAYBACK_STATUS_STOP) ? true : false;
#endif

#ifndef CFG_GOLD
#endif
}

////////////////////////////////////////////////////////////////////////////////
//	GetInfo
////////////////////////////////////////////////////////////////////////////////
const void Audio::System::GetInfo(Info* pInfo, bool bAverage) /*const*/
{
#ifndef CFG_GOLD
	GetInfoCommon((void*)pInfo, bAverage);

	pInfo->fTotalCpuTime = CPUUsage(eCpu_EntireSystem);
	pInfo->fMsgQueueCpuTime = CPUUsage(eCpu_MessageQueue);
	pInfo->fObjUpdateAndGenerateCpuTime = CPUUsage(eCpu_ObjectManagerUpdateAndRender);
	pInfo->fStreamerCpuTime = CPUUsage(eCpu_FileStreamer);

	#ifdef NAMED_COUNTERS_ENABLED
	NamedCpuCounters& nc = GetNamedCpuCounters();
	pInfo->uNumNamedCpuEvents = 0;

	for (S32 c=0; c<MAX_CPU_NAMED_EVENTS; c++)
	{
		if (c < nc.GetNumCounters())
		{
			nc.GetEventName(c).CopyTo(pInfo->namedCpuEvents[c].pchName, 16);
			pInfo->namedCpuEvents[c].pchName[15] = 0;

			pInfo->namedCpuEvents[c].fTime = nc.GetEventCpuTime(c);
			pInfo->uNumNamedCpuEvents++;
		}
		else
		{
			pInfo->namedCpuEvents[c].pchName[0] = 0;
			pInfo->namedCpuEvents[c].fTime = 0.f;
		}
	}
	#endif

	//pInfo->fPreUpdateTime =   m_afTimers[ePreUpdate].GetAverage() / fScale;
	//pInfo->fObjUpdateTime = m_afTimers[eObjUpdate].GetAverage() / fScale;
	//pInfo->fObjGenerateTime = m_afTimers[eObjGenerate].GetAverage() / fScale;
	//pInfo->fPostUpdateTime = m_afTimers[ePostUpdate].GetAverage() / fScale;

	/*F32 fScale = (F32)Audio::Core::uFrameLength / (F32)Audio::Core::uSampleRate;

	if (bAverage)
	{
		pInfo->fPreUpdateTime = m_afTimers[ePreUpdate].GetAverage() / fScale;
		pInfo->fMsgQueueTime = m_afTimers[eMsgQueue].GetAverage() / fScale;
		pInfo->fObjUpdateTime = m_afTimers[eObjUpdate].GetAverage() / fScale;
		pInfo->fObjGenerateTime = m_afTimers[eObjGenerate].GetAverage() / fScale;
		pInfo->fPostUpdateTime = m_afTimers[ePostUpdate].GetAverage() / fScale;
		pInfo->fStreamerTime = m_afTimers[eStreamer].GetAverage() / fScale;
	}
	else
	{
		pInfo->fPreUpdateTime = m_afTimers[ePreUpdate].GetValue() / fScale;
		pInfo->fMsgQueueTime = m_afTimers[eMsgQueue].GetValue() / fScale;
		pInfo->fObjUpdateTime = m_afTimers[eObjUpdate].GetValue() / fScale;
		pInfo->fObjGenerateTime = m_afTimers[eObjGenerate].GetValue() / fScale;
		pInfo->fPostUpdateTime = m_afTimers[ePostUpdate].GetValue() / fScale;
		pInfo->fStreamerTime = m_afTimers[eStreamer].GetValue() / fScale;
	}*/
#endif
}


void Audio::System::DebugRender()
{
	m_WaveServer.GetFileStreamManager().Render();

#ifdef AUDIO_STREAMER_DEBUG_INFO
	Audio::Core::FileStreamPlaybackBuffer::TheBuffer()->DebugInfo();
#endif
	GetDefinitionManager()->RenderStats();
}


void Audio::System::PrintAt(F32 f32X, F32 f32Y, U32 u32RGBA, F32 f32Size, char* strText, ...)
{
	char buffer[512];
#ifdef COMPILER_VC
	va_list vl;

	va_start(vl, strText);
	_vsnprintf(buffer, sizeof(buffer), strText, vl);
	va_end(vl);
#else
	va_list vl;

	va_start(vl, strText);
	vsprintf(buffer, strText, vl);
	va_end(vl);
#endif

	buffer[511]=0;

	if (m_PrintAtCallback)
		m_PrintAtCallback(f32X, f32Y, u32RGBA, f32Size, buffer);
}


void Audio::System::DrawBox(F32 f32XPos, F32 f32YPos, F32 f32XSize, F32 f32YSize, U32 u32RGBA)
{
	if (m_pDrawBoxCallback)
		m_pDrawBoxCallback(f32XPos, f32YPos, f32XSize, f32YSize, u32RGBA);
}


#ifdef PLATFORM_PS3
Debug::Bool g_WaitOnNewEvent("Audio.System.NewJoin", true);

// RUN a memcpy job on the SPU
#ifdef SPU_MEMCPY
void Audio::System::RunFrameFinishMarker(FrameUpdateFinishInfo *pInfo)
{
	FrameUpdateFinishInfo &info = *m_pFrameUpdateFinishInfo[m_iCurrentFrameUpdateFinishInfo];
	InstancePs3PpuData &data = *m_pFrameUpdateFinishTask[m_iCurrentFrameUpdateFinishInfo];
	m_iCurrentFrameUpdateFinishInfo++;
	if (m_iCurrentFrameUpdateFinishInfo >= MAX_FRAME_UPDATE_INFO)
		m_iCurrentFrameUpdateFinishInfo = 0;

	info = *pInfo;
	data.m_spuTaskParams.eTaskId = eFrameUpdateFinishedMarker;
	data.m_spuTaskParams.pSrcAddress = &info;
	data.m_spuTaskParams.pRetAddress = 0;
	data.m_spuTaskParams.u32Size = sizeof(info);
//jkc
	info.pEvent = m_pFrameFinishedEvent;
	data.m_SpuTask.Initialise((void*)&data.m_spuTaskParams);

	System::GetScheduler().Schedule(&data.m_SpuTask);

	if (g_WaitOnNewEvent)
	{
		U16 bitmask = 1;
		int result = cellSpursEventFlagWait(m_pFrameFinishedEvent, &bitmask, CELL_SPURS_EVENT_FLAG_AND);
		if (result != 0)
			DebugBreak();
	}
}
#endif	//	#ifdef SPU_MEMCPY
#endif

void Audio::System::SetStaggeredInstanceCreation(bool bEnable)
{
	m_bStaggerInstanceCreation = bEnable;
}


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
