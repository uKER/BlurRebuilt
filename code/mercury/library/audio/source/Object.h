#ifndef _OBJECT_H_
#define _OBJECT_H_


namespace Audio
{
    class ObjectManager;
    class Definition;
	class AudioComponent;

/**
* An Audio Object. This class is a container for Components.
*/

    class Object
    {
    public:
		friend class Instance;

		static void* operator new(size_t sz);
		static void* operator new(size_t sz, void* pData)
		{
			return pData;
		}
		static void operator delete(void* ptr);

    public:
#ifndef PLATFORM_SPU

        Object(U32 uFrameSkip);
        ~Object();

        Result Init(Definition* pDefinition, IWaveManager* pWaveManager = NULL);
#endif	//	PLATFORM_SPU

		U32 CalculateComponentFootprint(Definition* iDef);
		U32 GetComponentFootprint() { return m_u32ComponentFootprintSize; }
#ifndef AUDIO_USE_COMPONENT_ARRAY
		inline U32 GetComponentIndexArraySize() { return m_apComponents.GetIndexArraySize(); }
#endif
		void AssignBuffers( Instance* pInstance, AudioComponent* pCom, Core::BufferStack& buffers );

#ifndef PLATFORM_SPU
        void CreateMasterInstance();
        Instance* CreateInstance(const char* pchGroupName = 0, const char* pchEntityName = 0);
        void DestroyInstance(Instance* pInstance);
#endif	//	PLATFORM_SPU

        void Update(Instance* pInstance);
        void Generate(Instance* pInstance, bool bOnFrameSkip = true);

#ifndef PLATFORM_SPU
		void ProcessConnections(Instance* pInstance);
#endif

#ifndef CFG_GOLD
		void DumpProcessFlags( Instance* pInstance );
#endif

#ifdef PLATFORM_PS3
	void ProcessActionOuts(Instance* pInstance);
#endif

#ifndef PLATFORM_SPU

        Result Release();

#endif
		//Object* GetObject() { return this; }
		inline Definition* GetDefinition() const { return m_iDefinition; }

#ifndef PLATFORM_SPU
        void ConfigureAudio(Instance* pInstance);

		void StopAllSounds(Instance* pInstance);

        const char* Name() const { return m_pchStringTable; }

		#ifdef AUDIO_STRING_HASH_OBJECT_NAMES
		const StringHash NameHash() const { return m_uNameHash; }
		#endif

        void SetHandle(ObjectHandle handle) { m_handle = handle; }
        ObjectHandle GetHandle() const { return m_handle; }

        ActionIn* GetActionIn(const char* pchName);
        ActionOut* GetActionOut(const char* pchName);
        ParameterIn* GetParameterIn(const char* pchName);
        ParameterOut* GetParameterOut(const char* pchName);
        VectorIn* GetVectorIn(const char* pchName);
        ControlInput* GetControlInput(const char* pchName);
        EventInput* GetEventInput(const char* pchName);
        VectorInput* GetVectorInput(const char* pchName)
        {
            return NULL;
        }
        GraphInput* GetGraphInput(const char* pchName)
        {
            return NULL;
        }
        MonoAudioInput* GetMonoAudioInput(const char* pchName);
        StereoAudioInput* GetStereoAudioInput(const char* pchName);
        SurroundAudioInput* GetSurroundAudioInput(const char* pchName);
        SurroundLFEAudioInput* GetSurroundLFEAudioInput(const char* pchName);
        ControlOutput* GetControlOutput(const char* pchName);
        EventOutput* GetEventOutput(const char* pchName);
        VectorOutput* GetVectorOutput(const char* pchName)
        {
            return NULL;
        }
        GraphOutput* GetGraphOutput(const char* pchName)
        {
            return NULL;
        }
        MonoAudioOutput* GetMonoAudioOutput(const char* pchName);
        StereoAudioOutput* GetStereoAudioOutput(const char* pchName);
        SurroundAudioOutput* GetSurroundAudioOutput(const char* pchName);
		SurroundLFEAudioOutput* GetSurroundLFEAudioOutput(const char* pchName);

		U16 GetComponentIndex(const char* pchName, U32 u32Flags);

		#ifdef AUDIO_STRING_HASH_COMPONENTS
		U16 GetComponentIndexFromHash(const StringHash nameHash, U32 u32Flags);
		#endif

		void SetFrameSkip(U32 frameSkip);
#endif	//	PLATFORM_SPU

        inline U32 GetNumComponents() { return m_apComponents.Size(); }
        inline Component* GetComponent(U32 uCom) { return m_apComponents[uCom]; }
#ifndef PLATFORM_SPU
        inline const char* GetComponentType(U32 uCom) { return m_apComponents[uCom]->Type(); }
        inline const char* GetComponentName(U32 uCom) { return m_apComponents[uCom]->Name(); }
#endif

#ifdef PLATFORM_SPU
		const char* Name() { return m_iDefinition->StringTable(); }
		inline U32 GetComponentTypeId(U32 uCom) { return m_apComponents.GetType(uCom); }
#endif

#ifdef ENABLE_DEBUG_EXTENSIONS
		inline IDBSource* GetDBSource(U32 uCom) { return GetComponent(uCom)->DBSource(); }
		inline IDBMixer* GetDBMixer(U32 uCom) { return GetComponent(uCom)->DBMixer(); }
		inline IDBOut* GetDBOut(U32 uCom) { return GetComponent(uCom)->DBOut(); }
#endif

#ifdef PLATFORM_SPU
		inline void SetDefinition(Definition* pDef) { m_iDefinition = pDef; }
		inline void SetComponents(U8* pComp) { m_apComponents.SetDataPointer(pComp); m_apComponents.GetNumComponentsFromIndex(); }
#endif	//	PLATFORM_SPU

#ifndef AUDIO_USE_COMPONENT_ARRAY
		void* GetComponents() { return m_apComponents.GetBasePointer(); }
#else
		void* GetComponents() { return m_pComponents; }
#endif

//#ifdef PLATFORM_PS3
//		inline void* GetSpuComponents() { return m_pComponentsSpu; }
//#endif
		inline U8* GetInitData(U32 uOffset = 0)
		{
			return &m_pInitData[uOffset];
		}
        
        inline const char* GetString(U32 uString) const { return &m_pchStringTable[uString]; }

#ifndef PLATFORM_SPU
        inline IObjectManager* GetObjectManager() { return (IObjectManager*)m_pManager; }
        inline IWaveManager* GetWaveManager() { return m_pWaveManager; }

        inline void SetObjectManager(ObjectManager* pManager) { m_pManager = pManager; }

		void IncrementInstanceCounter();
		void DecrementInstanceCounter();

		// debug
		void IncrementSuspendedInstanceCounter();
		void DecrementSuspendedInstanceCounter();

				// debug
		void IncrementMutedInstanceCounter();
		void DecrementMutedInstanceCounter();
#endif	//	PLATFORM_SPU
		U32 GetActiveInstanceCount() { return m_u32ActiveInstanceCount; }

		U32 GetTotalInstanceCount() { return m_u32TotalInstanceCount; }

		// debug
		U32 GetSuspendedInstanceCount() { return m_u32SuspendedInstanceCount; }

		inline U32 FrameSkip() { return m_uFrameSkip; }
		inline F32 FrameSkipF() { return m_fFrameSkip; }

#ifndef PLATFORM_SPU
		void DumpStats(Instance* pInstance)
		{
			pInstance->DumpStats( true );
		}
#endif	//	PLATFORM_SPU

#ifdef PLATFORM_SPU
		U32 GetComponentFlags(U32 u32Com);
#else
		inline U32 GetComponentFlags(U32 u32Com) { return GetComponent(u32Com)->Flags(); }
#endif

		U32 GetDynamicBufferSize() { return m_u32DynamicBufferSizeInSamples; }

#ifdef PLATFORM_PS3
		U32 GetDmaBufferSize() { return m_u32LargestRequiredDmaBuffer; }
		void SetDmaBufferSize(U32 u32Size) { if (u32Size > m_u32LargestRequiredDmaBuffer) m_u32LargestRequiredDmaBuffer = u32Size; }
#else
		U32 GetDmaBufferSize() { return 0; }
#endif

		U32 GetInstanceFootprint() { return m_u32InstanceFootprint; }
		U32 GetSpuInstanceFootprint() { return m_u32SpuInstanceFootprint; }
		U32 GetTotalInstanceFootprint() { return m_u32TotalInstanceFootprint; }

#if defined PLATFORM_SPU
//		void* InitComponent(void* pCom, void* pDefData);
		void UpdateComponent(void* pCom, Instance* pInst);
		void GenerateComponent(void* pCom, Instance* pInst);
		void SetProcessFlagOnComponent(void* pCom, Instance* pInst);
		void ClearComponent(void* pCom, Instance* pInst);
		//F32 GetComponentValue(void* pCom, Instance* pInst, U32 u32InputOut = 0);
		F32 GetComponentValue(S32 s32Com, Instance* pInst, U32 u32InputOut = 0);
		void SetComponentValue(S32 s32Com, Instance* pInst, F32 fValue);
		void GetComponentCoords(S32 s32Com, Instance* pInst, Maths::Vector* pVector);
		void ExecuteComponent(S32 s32Com, Instance* pInst);
		void DoComponentMessage(S32 s32Com, Instance* pInst, U32 uMessage);
#else
//		inline void* InitComponent(Component* pCom, void* pDefData) { return pCom->Init(pDefData); }
		inline void UpdateComponent(Component* pCom, Instance* pInst) { pCom->Update(pInst); }
		inline void GenerateComponent(Component* pCom, Instance* pInst) { pCom->Generate(pInst); }
		//inline void ClearComponent(Component* pCom, Instance* pInst) { pCom->Clear(pInst); }
		//inline F32 GetComponentValue(Component* pCom, Instance* pInst, U32 u32InputOut = 0) { return pCom->Value(pInst, u32InputOut); }
		inline F32 GetComponentValue(S32 s32Com, Instance* pInst, U32 u32InputOut = 0) { return GetComponent(s32Com)->Value(pInst, u32InputOut); }
		inline void SetComponentValue(S32 s32Com, Instance* pInst, F32 fValue) { GetComponent(s32Com)->SetValueDirect(pInst, fValue); }
		inline void GetComponentCoords(S32 s32Com, Instance* pInst, Maths::Vector* pVector) { GetComponent(s32Com)->Coords(pInst, pVector); }
		inline void ExecuteComponent(S32 s32Com, Instance* pInst) { GetComponent(s32Com)->Execute(pInst); }
		inline void DoComponentMessage(S32 s32Com, Instance* pInst, U32 uMessage) {  GetComponent(s32Com)->DoMessage(pInst, uMessage); }

		int NumInstances() { return m_aInstances.Size(); }

#endif

		void IncrementNumberOfStreams() { m_u32NumStreams++; }
		U32 GetNumberOfStreams() { return m_u32NumStreams; }

		void IncrementNumberOfAdpcmWaves(U32 u32Amount = 1) { m_u32NumAdpcmWaves += u32Amount; }
		U32 GetNumberOfAdpcmWaves() { return m_u32NumAdpcmWaves; }

		bool IsDoomed() { return bObjectIsDoomed; }
		void SetDoomed() { bObjectIsDoomed = true; }

		inline Ut::Vector<Instance*>& GetInstances() { return m_aInstances; }

	public:
		#ifdef AUDIO_STRING_HASH_OBJECT_NAMES
		U32 m_uNameHash;
		#endif
    private:
        //char* m_pchName;
        const char* m_pchStringTable;
		
		ObjectHandle m_handle;

		U32 m_uFrameSkip;
		F32 m_fFrameSkip;

		U8* m_pInitData;

		U32 m_u32CoreSize;

        ObjectManager* m_pManager;
        IWaveManager* m_pWaveManager;

		volatile U32 m_u32ActiveInstanceCount;
		U32 m_u32TotalInstanceCount;

		U32 m_u32ComponentFootprintSize;

#ifdef PLATFORM_PS3
		U32 m_u32LargestRequiredDmaBuffer;
#endif

		bool bObjectIsDoomed;				// is this object on the doomed list ready to be destroyed?

		// debug
		U32 m_u32InstanceFootprint;			// how many bytes does 1 instance of this object need in memory?
		U32 m_u32SpuInstanceFootprint;		// how many bytes does 1 instance of this object need on the SPU? includes definition size, component footprint etc
		U32 m_u32TotalInstanceFootprint;	// how many bytes do all instances need?
		U32 m_u32SuspendedInstanceCount;
		U32 m_u32MutedInstanceCount;

		U8* m_pComponents;

		U32 m_u32DynamicBufferSizeInSamples;

#ifdef AUDIO_USE_COMPONENT_ARRAY
        Core::Array<Component*> m_apComponents;
#else
        ComponentContainer m_apComponents;
#endif

        Core::Array<Component*> m_apUpdaters;
        Core::Array<Component*> m_apSounds;
        Core::Array<Component*> m_apTransmitters;
        Core::Array<Component*> m_apReceivers;
		Core::Array<Component*>	m_apScripts;
		Core::Array<Component*>	m_apAudioConnections;
		Core::Array<Component*>	m_apConnections;
		Core::Array<Component*>	m_apActionOuts;

		Definition* m_iDefinition;

		U32 m_u32NumStreams;
		U32 m_u32NumAdpcmWaves;

		Instance* m_pMasterInstance;
		U32 m_uMasterInstanceSize;

#ifndef PLATFORM_SPU
		Core::Lock m_lockInstance;
#endif
		Ut::Vector<Instance*> m_aInstances;
    };
}

#endif // _Object_H_

