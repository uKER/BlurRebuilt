#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "CpuCounters.h"

#include <System/Serialise/I_FileSystem.h>

//  random number seeds
#define AUDIO_BIG_PRIME_A 99923
#define AUDIO_BIG_PRIME_B 72649

extern bool g_bDumpStats;

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

#define PS3_AUDIO_USE_SPU


namespace Audio
{
	struct InstancePs3PpuData;

	inline ObjectComponentHandle ExtractObjectComponentHandle(InstanceComponentHandle hInstComp)
	{
//		return (ObjectComponentHandle)(hInstComp >> 16);
		return hInstComp.GetObjectComponentHandle();
	}

	inline InstanceHandle ExtractInstanceHandle(InstanceComponentHandle hInstComp)
	{
//		return (InstanceHandle)(hInstComp & 0xffff);
		return hInstComp.GetInstanceHandle();
	}

	inline ObjectHandle ExtractObjectHandle(U32 u32ObjectComponentResource)
	{
		ObjectHandle hRetValue((U16)(u32ObjectComponentResource >> 16));
		return hRetValue;
	}

	inline U16 ExtractComponentIndex(U32 u32ObjectComponentResource)
	{
		return (U16)(u32ObjectComponentResource & 0xffff);
	}

////////////////////////////////////////////////////////////////////////////////
//	System
////////////////////////////////////////////////////////////////////////////////
    class System : public ISystem
    {
    public:
		struct Speakers	//	TODO: move to System
		{
			F32 fCentre;
			F32 fFrontLeft;
			F32 fFrontRight;
			F32 fSurroundLeft;
			F32 fSurroundRight;
			F32 fBackLeft;
			F32 fBackRight;
		};

    public:
		static void* operator new(size_t sz);
		static void operator delete(void* ptr);

    public:
        System(ISystem::InitParams* pInitParams);
        ~System()
        {
            Shutdown();
        }

        bool Init();
        void Update();
        void PostUpdate();
        bool Shutdown();

		bool IsPaused() const
		{
			return m_bPaused;
		}

		void Pause(bool pause)
		{
			m_bPaused = pause;
		}

        void SetMaster(InstanceComponentHandle hComponent);
        void SetMonoMaster(InstanceComponentHandle hComponent);
        void SetStereoMaster(InstanceComponentHandle hComponent);
        void SetSurroundMaster(InstanceComponentHandle hComponent);
        void SetSurroundLFEMaster(InstanceComponentHandle hComponent);

        void ReceiveSetMaster(InstanceComponentHandle hComponent);
        void ReceiveSetMonoMaster(InstanceComponentHandle hComponent);
        void ReceiveSetStereoMaster(InstanceComponentHandle hComponent);
        void ReceiveSetSurroundMaster(InstanceComponentHandle hComponent);
        void ReceiveSetSurroundLFEMaster(InstanceComponentHandle hComponent);

		U32	GetFramesDropped();

		inline IDefinitionManager* GetIDefinitionManager() { return &m_DefinitionManager; }
		inline DefinitionManager* GetDefinitionManager() { return &m_DefinitionManager; }
		inline IObjectManager* GetIObjectManager() { return &m_ObjectManager; }
		inline ObjectManager* GetObjectManager() { return &m_ObjectManager; }

//		inline Core::FileStreamManager* GetStreamManager() { return &m_FileStreamManager; }
		inline Core::WaveServer* GetWaveServer() { return &m_WaveServer; }

		Core::SendQueue* GetQueue() { return &m_Queue; }
		Core::ReturnQueue* GetReturnQueue() { return &m_ReturnQueue; }

        inline const U32 GetFrameNum() { return m_SystemState.m_uFrameNum; }

        inline F32 GetRandom()
        {
            m_uSeed = (m_uSeed + AUDIO_BIG_PRIME_A) * AUDIO_BIG_PRIME_B;
            return (F32)m_uSeed / 4294967296.f;
        }

		inline Sys::I_FileSystem& GetFileSystem() { return *Sys::GetDefaultFileSystem(); }

		// diagnostic functions
		inline void SetPrintAtCallback(PrintFunc pPrintAtCallback) { m_PrintAtCallback = pPrintAtCallback; }
		inline void SetDrawBoxCallback(DrawBoxCallback pCallback) { m_pDrawBoxCallback = pCallback; }

		static PrintFunc GetPrintAtCallback() { return m_PrintAtCallback; }

		//	component access functions
		inline void SetValue(InstanceComponentHandle hParamIn, F32 fValue) { m_ObjectManager.SetValue(hParamIn, fValue); }
		inline void Value(InstanceComponentHandle hParamOut, U32 hDataTracker, AudioRequestDataCallback pCallback) { m_ObjectManager.Value(hParamOut, hDataTracker, pCallback); }
		inline void Execute(InstanceComponentHandle hActionIn) { m_ObjectManager.Execute(hActionIn); }
//		inline void Count(InstanceComponentHandle hActionOut, bool bReset = false, AudioRequestDataCallback pCallback = 0) { m_ObjectManager.Count(hActionOut, bReset, pCallback); }
		inline void SetDimensions(InstanceComponentHandle hVectorIn, F32* afDimensions) { m_ObjectManager.SetDimensions(hVectorIn, afDimensions); }

		const char* GetName(ObjectComponentHandle hActionIn) { /* TODO: reimplement*/ return "implement this"; }

//		static void TriggerDecoderCallback(U32 u32CallbackId, U32 u32StateOffset);

		// Engine muting functions
		void MuteOutput() { m_pRenderer->MuteOutput(); }
		void UnmuteOutput() { m_pRenderer->UnmuteOutput(); }
		F32 GetRendererLevel(U32 uCh) { return m_pRenderer->GetLevel(uCh); }

		//	memory functions
		void SetAllocator(AllocateMemory pfnAlloc, FreeMemory pfnFree)
		{
			System::m_pfnAlloc = pfnAlloc;
			System::m_pfnFree = pfnFree;
		}
		void SetPhysicalAllocator(AllocateMemory pfnAlloc, FreeMemory pfnFree)
		{
			System::m_pfnPhysAlloc = pfnAlloc;
			System::m_pfnPhysFree = pfnFree;
		}

#ifdef TRACK_AUDIO_ENGINE_MEMORY
		void PushCurrentMemoryAllocationId(U32 u32Id);
		void PopCurrentMemoryAllocationId();
		U32 QueryCurrentMemoryAllocationId();
#else
		inline void PushCurrentMemoryAllocationId(U32 u32Id) { }
		inline void PopCurrentMemoryAllocationId() { }
		inline U32 QueryCurrentMemoryAllocationId() { return 0; }
#endif

		static void* Alloc(size_t uSize, U32 uAlignment = 1, AllocType eType = eVirtual);
		static void Free(void* pAddress, AllocType eType = eVirtual);

		//	file access functions
		void SetFileOpenCallback(FileOpenFunc openFunc)		{ m_openFunc = openFunc; }
		void SetFileReadCallback(FileReadFunc readFunc)		{ m_readFunc = readFunc; }
		void SetFileSeekCallback(FileSeekFunc seekFunc)		{ m_seekFunc = seekFunc; }
		void SetFileWriteCallback(FileWriteFunc writeFunc)	{ m_writeFunc = writeFunc; }
		void SetFileCloseCallback(FileCloseFunc closeFunc)	{ m_closeFunc = closeFunc; }
		void SetFileSizeCallback(FileSizeFunc sizeFunc)		{ m_sizeFunc = sizeFunc; }
		void SetFilePauseCallback(FilePauseFunc pauseFunc)		{ m_pauseFunc = pauseFunc; }
		void SetFileResumeCallback(FileResumeFunc resumeFunc)		{ m_resumeFunc = resumeFunc; }

		// signal audio thread to do some work
		void SignalEvent() { m_pRenderer->SignalEvent(); }

		static FileOpenFunc GetFileOpenCallback()		{ return m_openFunc; }
		static FileReadFunc GetFileReadCallback()		{ return m_readFunc; }
		static FileSeekFunc GetFileSeekCallback()		{ return m_seekFunc; }
		static FileWriteFunc GetFileWriteCallback()	{ return m_writeFunc; }
		static FileCloseFunc GetFileCloseCallback()	{ return m_closeFunc; }
		static FileSizeFunc GetFileSizeCallback()	{ return m_sizeFunc; }
		static FilePauseFunc GetFilePauseCallback()	{ return m_pauseFunc; }
		static FileResumeFunc GetFileResumeCallback()	{ return m_resumeFunc; }

		static DatPathCallback GetDatPathCallback() { return m_pfnGetDatPath; }

        //  surround setup
		inline MixerComponent::PanningMode GetPanningMode() { return m_SystemState.m_ePanningMode; }
		inline U32 GetNumSpeakers() { return m_SystemState.m_uNumSpeakers; }
		inline F32* GetSpeakerAngles() { return m_SystemState.m_afAngles; }
        inline F32 GetSpeakerDistance() { return m_SystemState.m_fSpeakerDistance; }
        inline bool HasCentre() { return m_SystemState.m_bHasCentre; }
		inline bool	AnglesChanged(){ return m_SystemState.m_SpeakerAngleHasChanged; }

		void SetSpeakerDistance( SpeakerOption eOption, F32 fDistance );
		void SetSpeakerAngles( SpeakerConfig eMode, F32 fFrontLeft, F32 fFrontRight, F32 fSurroundLeft = 0.f, F32 fSurroundRight = 0.f, F32 fBackLeft = 0.f, F32 fBackRight = 0.f );
		void SetSpeakerOption( SpeakerOption eOption );
#ifdef PLATFORM_PC
		//! [Blur2PC] T144: retail PC 0x0053aad0: restarts the output engine at uSampleRate (the audio quality option)
		void SetOutputSampleRate( U32 uSampleRate );
#endif

        //  render callback
        static void RenderCallback(void* pContext);
        static void PostRenderCallback(void* pContext);

		//	other callbacks
		inline void SetPreUpdateCallback(SystemCallback pCallback) { m_pPreCallback = pCallback; }
		inline void SetPostUpdateCallback(SystemCallback pCallback) { m_pPostCallback = pCallback; }

		inline void SetDebugger(IDebug* pDebugger) { m_pDebugger = pDebugger; }
		inline Audio::IDebug* GetDebugger() { return m_pDebugger; }

#ifdef PLATFORM_PS3
		bool SubmitSpuTask(void* pData, void* pReturnAddress, AudioSpuTaskId eId, U32 u32Size);
		void PopulateSpuInfoStruct() { m_pSystemSpuInfo->state = m_SystemState; GetInfo(&(m_pSystemSpuInfo->debugInfo), true); }
		SystemSpuInfo* GetSystemSpuInfoAddress() { return m_pSystemSpuInfo; }

#ifdef SPU_MEMCPY
		void RunFrameFinishMarker(FrameUpdateFinishInfo *pInfo);

		// ringbuffer of frame update infos
		int m_iCurrentFrameUpdateFinishInfo;
		FrameUpdateFinishInfo *m_pFrameUpdateFinishInfo[MAX_FRAME_UPDATE_INFO];
		InstancePs3PpuData *m_pFrameUpdateFinishTask[MAX_FRAME_UPDATE_INFO];
#endif	//	#ifdef SPU_MEMCPY
#endif

        //  create/delete static object
		static System* Create(Audio::ISystem::InitParams* pInitParams);
        static void Release();

        //  return static object
        static System* Get() { return m_pSystem; }

		//	reverb
#if defined(PLATFORM_PC) 
		void SetReverb(EAXREVERBPROPERTIES* pSettings, U32 uIndex);
#endif	//	PLATFORM_PC

#ifdef PLATFORM_PS3
#ifdef PS3_OUTPUT_TO_MULTISTREAM
#ifdef AUDIO_USE_OLD_REVERB
		void SetReverb(CellMSFXReverbParams* pSettings, U32 uIndex);
#else
		void SetReverb(CellMSFXI3DL2Params* pSettings, U32 uIndex);
#endif	//	AUDIO_USE_OLD_REVERB
#endif	//	PS3_OUTPUT_TO_MULTISTREAM
#endif	//	PLATFORM_PS3


		void DebugRender();

		static void PrintAt(F32 f32X, F32 f32Y, U32 u32RGBA, F32 f32Size, char* strText, ...);
		static void DrawBox(F32 f32XPos, F32 f32YPos, F32 f32XSize, F32 f32YSize, U32 u32RGBA);

		const void GetInfo(Info* pInfo, bool bAverage) /*const*/;
		static void DumpDefinitionList(void*) { Get()->GetDefinitionManager()->DumpStats(); }
		static void LogPeakTempInstanceBufferUsage(void*) { Get()->GetObjectManager()->LogPeakTempInstanceBufferUsage(); }

		Core::Lock* GetLock() { return &m_Lock; }

#ifdef PLATFORM_PC
		HWND GetHWND()
		{
			return m_hWnd;
		}
#endif

		void DoWork();

		void SetHighQuality(bool bHighQuality) { m_bHighQuality = bHighQuality; }
		char* GetQualityPathString()
		{
			if (m_bHighQuality)
				return "High/";
			else
				return "Low/";
		}


		bool TitleHasControl() { return (m_bTitleHasControl == TRUE) ? true : false; }

#ifdef PLATFORM_PS3
//		static Spu::Scheduler& GetScheduler() { return m_SpuScheduler; }
//		void *GetSpuScheduler() { return &m_SpuScheduler; }
#endif

//#ifdef CPU_COUNTERS_ENABLED
		F32 CPUUsage(U32 uIndex) const;
//#endif
#ifdef NAMED_COUNTERS_ENABLED
		NamedCpuCounters& GetNamedCpuCounters() { return m_NamedCounters; }
#endif

		void SetStaggeredInstanceCreation(bool bEnable);
		bool StaggerInstanceCreation() { return m_bStaggerInstanceCreation; }

    private:
        static System*		m_pSystem;
	
#ifdef PLATFORM_PS3
		static SystemSpuInfo*		m_pSystemSpuInfo;
#endif
        DefinitionManager m_DefinitionManager;
//        InstanceManager m_InstanceManager;
        ObjectManager m_ObjectManager;

		Core::WaveServer m_WaveServer;

//		StaticBufferAllocator m_StateMemoryAllocator;

//		ControlScriptManager m_ControlScriptManager;

        Audio::Core::SendQueue m_Queue;
        Audio::Core::ReturnQueue m_ReturnQueue;
		Core::Lock m_Lock;

        Audio::Core::IGenerator* m_pMaster;
        Audio::Core::Renderer* m_pRenderer;

		//U32 m_uFrameSkip;
        U32 m_uSeed;

		F32 m_af3Angles[ 4 ];
		F32 m_af5Angles[ 6 ];
		F32 m_af7Angles[ 8 ];
		F32 m_fSpeakerDistances[ eNumSpeakerOptions ];

		SystemState m_SystemState;

		// for memory tracking
		U32 m_u32AllocId[32];
		U32 m_u32AllocIdStackPointer;

		U32 m_uNumReverbs;

		//	callbacks
		SystemCallback m_pPreCallback;
		SystemCallback m_pPostCallback;

		static AllocateMemory m_pfnAlloc;
		static FreeMemory m_pfnFree;
		static AllocateMemory m_pfnPhysAlloc;
		static FreeMemory m_pfnPhysFree;

		static DatPathCallback m_pfnGetDatPath;

		static PrintFunc m_PrintAtCallback;
		static DrawBoxCallback m_pDrawBoxCallback;

		static FileOpenFunc m_openFunc;
		static FileReadFunc m_readFunc;
		static FileSeekFunc m_seekFunc;
		static FileWriteFunc m_writeFunc;
		static FileCloseFunc m_closeFunc;
		static FileSizeFunc m_sizeFunc;
		static FilePauseFunc m_pauseFunc;
		static FileResumeFunc m_resumeFunc;

		IDebug* m_pDebugger;
		DebugLevel::Level	m_prevDebugLevel;
		DebugLevel::Level	m_prevDebugLevelCore;

#ifdef PLATFORM_PC
		HWND	m_hWnd;
#endif
		bool	m_bPaused;

//#ifdef PLATFORM_XENON
		BOOL m_bTitleHasControl;
//#endif

#ifdef PLATFORM_PS3
//		static Spu::Scheduler m_SpuScheduler;
//		static Spu::Task m_SpuTask;
		AudioSpuTaskParams spuTaskParams;
//		bool bSpuTaskSubmitted;
		void* m_pOverlayTable;

//		CellSpursEventFlag *m_pFrameFinishedEvent;
#endif

		bool m_bHighQuality;

		bool m_bStaggerInstanceCreation;

#ifdef NAMED_COUNTERS_ENABLED
		NamedCpuCounters m_NamedCounters;
#endif
#ifdef CPU_COUNTERS_ENABLED
		Counter m_aCPUCounters[eNumCpuCounters];
#endif
    };
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

#endif // _SYSTEM_H_
