#ifndef _INSTANCE_H_
#define _INSTANCE_H_



namespace Audio
{
#define InstanceNotActivated		0x01
#define InstanceActivationPending	0x02
#define InstanceActivated			0x04
#define InstanceDeactivationPending	0x08
#define InstanceReleasePending		0x10
#define InstanceSuspendPending		0x20
#define InstanceSuspended			0x40
#define InstanceResumePending		0x80
/*
#ifdef PLATFORM_PS3
		struct InstancePs3PpuData
		{
			AudioSpuTaskParams m_spuTaskParams;
			AudioSpuObjectUpdateInfo m_SpuObjectUpdateInfo;
//			Spu::Task m_SpuTask;
		};
#endif*/
    class Object;
////////////////////////////////////////////////////////////////////////////////
//	Instance
////////////////////////////////////////////////////////////////////////////////
    class Instance// : public IInstance
    {

    public:
		static void* operator new(size_t sz, void* pData)
		{
			return pData;
		}

	public:
#ifndef PLATFORM_SPU
		Instance();
		~Instance();
#endif

        inline void SetObject(Object* pObject) { m_pObject = pObject; }

		void SetStateFlag(U32 uStateFlag);
		void ResetStateFlag(U32 uStateFlag);
		inline U32 GetStateFlags() { return m_uStateFlags; }
		inline bool StateFlagIsSet(U32 uFlag) { return m_uStateFlags & uFlag ? true : false; }
        
        inline void SetContext(U8* pContext) { m_pContext = pContext; }
		inline void SetContextSize(U32 u32Size) { m_u32ContextSize = u32Size; }
		inline void SetStreamInfoSize(U32 u32Size) { m_u32StreamInfoSize = u32Size; }
		inline void SetCoreData(U8* pCoreData) { m_pCoreData = pCoreData; m_coreInfo.pCoreData = pCoreData; }
		inline void SetCoreDataSize(U32 u32Size) { m_u32CoreDataSize = u32Size; }
		inline void SetBufferData(U8* pBufferData) { m_pBufferData = pBufferData; m_coreInfo.pStaticBuffers = (F32*)pBufferData; }
		inline void SetBufferDataSize(U32 u32Size) { m_u32BufferDataSize = u32Size; }
        inline void SetNext(Instance* pNext) { m_pNext = pNext; }
        inline void SetPrev(Instance* pPrev) { m_pPrev = pPrev; }
        inline void SetNextActive(Instance* pNext) { m_pNextActive = pNext; }
        inline void SetPrevActive(Instance* pPrev) { m_pPrevActive = pPrev; }
        inline void SetCurrentValues(F32* afCurrentValues) { m_afCurrentValues = afCurrentValues; }
		inline void SetWaveServerOffset(U32 u32Offset) { m_uWaveServerOffset = u32Offset; }
		inline void SetAdpcmInfoOffset(U32 u32Offset) { m_uAdpcmInfoOffset = u32Offset; }
#ifndef PLATFORM_PS3
		inline void Suspend(bool bSuspend) { m_bSuspend = bSuspend; }
#else
		void Suspend(bool bSuspend);
#endif
		inline void PrevSuspend(bool bSuspend) { m_bPrevSuspend = bSuspend; }
#ifndef PLATFORM_PS3
		inline void Mute(bool bMute) { m_bMute = bMute; }
#else
		void Mute(bool bMute);
#endif
		inline void PrevMute(bool bMute) { m_bPrevMute = bMute; }

		inline void SetHandle(InstanceHandle hInstance) { m_hInstance = hInstance; }
		inline InstanceHandle GetHandle() const { return m_hInstance; }

        inline Object* GetObject() { return m_pObject; }
        inline U8* GetStateData(U32 uOffset = 0) { return &m_pContext[uOffset]; }
		inline void SetNewStateData(U8* pData) { m_pNewStateData = pData; }
        inline U8* GetNewStateData(U32 uOffset = 0)
		{
			//	borrow scratch buffer mem...
//			U8* pData = (U8*)m_coreInfo.pDynamicBuffers;
//			return &pData[uOffset];
			Assert (m_pNewStateData != NULL);
			return &m_pNewStateData[uOffset];
		}
		inline U32 GetStateSize() { return m_u32ContextSize; }
        inline U8* GetCoreData(U32 uOffset = 0) { return &m_pCoreData[uOffset]; }
		inline U32 GetCoreDataSize() { return m_u32CoreDataSize; }
		inline U32 GetStreamInfoSize() { return m_u32StreamInfoSize; }
		inline U8* GetBufferData() { return m_pBufferData; }
		inline U32 GetBufferDataSize() { return m_u32BufferDataSize; }
        inline Instance* Prev() { return m_pPrev; }
        inline Instance* Next() { return m_pNext; }
        inline Instance* PrevActive() { return m_pPrevActive; }
        inline Instance* NextActive() { return m_pNextActive; }
        inline F32* GetCurrentValues() { return m_afCurrentValues; }
		inline bool IsSuspended() { return m_bSuspend; }
		inline bool WasSuspended() { return m_bPrevSuspend; }
		inline bool IsMuted() { return m_bMute; }
		inline bool WasMuted() { return m_bPrevMute; }
		inline bool IsLinkedIntoActive() { return m_bLinkedIntoActiveList; }
		inline void LinkedIntoActive(bool b) { m_bLinkedIntoActiveList = b; }
		inline U32 GetWaveServerOffset() { return m_uWaveServerOffset; }
		Core::WaveStreamData* GetWaveServerStreamData(U32 u32StreamIndex);
		inline U8* GetWaveServerData() { return &(m_pCoreData[m_uWaveServerOffset]); }
		Core::AdpcmWaveInfo* GetAdpcmWaveInfo(U32 u32WaveIndex);
		inline U8* GetAdpcmWaveInfoDataChunk() { return &(m_pCoreData[m_uAdpcmInfoOffset]); }

		U32*	GetNumAdpcmWavesNeedingUpdatePointer()	{ if (! GetNumberOfAdpcmWavesCreated()) return 0; return (U32*)(GetAdpcmWaveInfoDataChunk()); }
		U32		GetNumAdpcmWavesNeedingUpdate()			{ if (! GetNumberOfAdpcmWavesCreated()) return 0; return *(GetNumAdpcmWavesNeedingUpdatePointer()); }
		void	IncrementNumAdpcmWavesNeedingUpdate()	{ if (! GetNumberOfAdpcmWavesCreated()) return; U32* p = GetNumAdpcmWavesNeedingUpdatePointer(); *(p)++; }
		void	ResetNumAdpcmWavesNeedingUpdate()		{ if (! GetNumberOfAdpcmWavesCreated()) return; U32* p = GetNumAdpcmWavesNeedingUpdatePointer(); *(p) = 0; }

		U32*	GetNumStreamsNeedingUpdatePointer()		{ return (U32*)(GetWaveServerData()); }
		U32		GetNumStreamsNeedingUpdate()			{ if (! GetNumberOfStreamsCreated()) return 0; return *(GetNumStreamsNeedingUpdatePointer()); }
		void	IncrementNumStreamsNeedingUpdate()		{ if (! GetNumberOfStreamsCreated()) return; U32* p = GetNumStreamsNeedingUpdatePointer(); (*(p))++; }
		void	ResetNumStreamsNeedingUpdate()			{ U32* p = GetNumStreamsNeedingUpdatePointer(); (*(p)) = 0; }

		//inline IndexedAllocator& CoreAllocator() { return m_allocCore; }

		Core::DecoderDataInfo GetDecoderDataInfo() { Core::DecoderDataInfo info; info.pWaveServerData = GetWaveServerData(); info.pAdpcmBufferData = GetAdpcmWaveInfoDataChunk(); return info; }

		Core::WaveStreamData* GetNextStreamInfoSlot();
		inline U32* GetNumStreamsCreatedPointer() { return &m_u32NumStreamsCreated; }
		inline U32 GetNumberOfStreamsCreated() { return m_u32NumStreamsCreated; }

		inline U32 GetNumberOfAdpcmWavesCreated() { return m_u32NumAdpcmWavesCreated; }
		inline void IncrementNumberOfAdpcmWavesCreated() { m_u32NumAdpcmWavesCreated++; }

#if defined (PLATFORM_PS3) || defined (PLATFORM_SPU)
		inline U32* GetAdpcmBitmask() { return (U32*)((U8*)m_au32AdpcmBitmaskPadded + m_u32BitmaskOffset); }
#endif

/*#ifdef PLATFORM_PS3
		inline InstancePs3PpuData* GetPpuData() { return m_pPpuData; }
#endif*/
		inline InstanceUpdateParams* GetDistributedTaskData() { return &m_DistributedTaskData; }

		const char* GroupName() const;
		const char* EntityName() const;
		void SetGroupName(const char* pchName);
		void SetEntityName(const char* pchName);

		void DumpStats( bool bDumpStats ) { m_bDumpStats = bDumpStats; }

		//inline Core::Array<Core::IGenerator*> Waves() { return m_apWaves; }
		//inline Core::Array<Core::IGenerator*> Mixers() { return m_apMixers; }
		//inline Core::Array<Core::IGenerator*> Outputs() { return m_apOutputs; }

		//void Reset();
		//void SetActivated();
		//void SetDeactivated();

#if defined (AUDIO_ENABLE_PROFILING)
		void SetEventLogging(bool bLogging) { m_bLogging = bLogging; }
		bool EventLogging() { return m_bLogging; }
#endif

	public:	//	for now...

		// temp, jkc- moved back to start of class for spu dma alignment happiness
		InstanceUpdateParams m_DistributedTaskData; 

		Core::ProcessInfo m_coreInfo;

		#if !defined (PLATFORM_PS3) && !defined (PLATFORM_SPU) && !defined CFG_GOLD
		U32 m_uEntityNameHash;
		U32 m_uGroupNameHash;
		#endif

    private:
        Instance* m_pPrev;
        Instance* m_pNext;

		Instance* m_pPrevActive;
		Instance* m_pNextActive;

        Object* m_pObject;
		InstanceHandle m_hInstance;
		bool m_bSuspend;
		bool m_bPrevSuspend;
		bool m_bMute;
		bool m_bPrevMute;

		bool m_bDumpStats;

		bool m_bLinkedIntoActiveList;

		U8* m_pContext;
        U8* m_pCoreData;
        U8* m_pBufferData;
		U8* m_pNewStateData;

		U32 m_uStateFlags;
		U32 m_u32ContextSize;
		U32 m_u32CoreDataSize;
		U32 m_u32BufferDataSize;
		U32 m_u32StreamInfoSize;

		//IndexedAllocator m_allocCore;
		U32 m_uWaveServerOffset;
		U32 m_uAdpcmInfoOffset;

		U32 m_u32NumStreamsCreated;
		U32 m_u32NumAdpcmWavesCreated;

#if defined (PLATFORM_PS3) || defined (PLATFORM_SPU)
//		U32 m_u32StreamInfoOffset;
//		U32* m_au32AdpcmBitmask;
		U32 m_au32AdpcmBitmaskPadded[ADPCM_BUFFER_ARRAY_SIZE + 4];
		U32 m_u32BitmaskOffset;
#endif
		F32* m_afCurrentValues;

		//#if !defined PLATFORM_SPU && !defined CFG_GOLD
		#if !defined (PLATFORM_PS3) && !defined (PLATFORM_SPU) && !defined CFG_GOLD
		char m_pchEntityName[16];
		char m_pchGroupName[16];
		#endif

#if defined (AUDIO_ENABLE_PROFILING)
		bool m_bLogging;
#endif
	};
}

#endif // _INSTANCE_H_
