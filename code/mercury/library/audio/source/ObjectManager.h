#ifndef _OBJECTMANAGER_H_
#define _OBJECTMANAGER_H_

#include "..\include\Audio.h"
//#include "common/linkedlist.h"
#include <core/containers.h>	// [MarkC] for the Ut::Vector type I'm using


#define	DEACTIVATED_INST_QUEUE_SIZE		(1024 * QUEUE_SIZE_MULTIPLIER)	// was 256
#define DOOMED_OBJECT_QUEUE_SIZE		(128 * QUEUE_SIZE_MULTIPLIER)

#define AUDIO_PROFILE_STRING_SIZE		31
#define AUDIO_PROFILE_ENTRIES			256
#define AUDIO_PROFILE_TIMERS			8

#ifndef USE_OBJECT_RELEASE_LIST
#define USE_OBJECT_RELEASE_LIST
#endif


namespace Audio
{
/*
	class TimingData
	{
	public:
		struct TagData
		{
			char pchTag[AUDIO_PROFILE_STRING_SIZE+1];
			S32 uRef;
			F32 fStartTime[AUDIO_PROFILE_TIMERS];
			F32 fEndTime[AUDIO_PROFILE_TIMERS];
			F32 fTotalTime[AUDIO_PROFILE_TIMERS];
		};

	public:
		TimingData();

		void AddTag(const char* pchName);
		void RemoveTag(const char* pchName);

		bool Start(const char* pchName, U32 uTimer = 0);
		bool Stop(const char* pchName, U32 uTimer = 0);

		void ResetAll();

		void ShowAll(U32 uNumTimers);

	private:
		StopWatch m_Time;
		TagData m_aTags[AUDIO_PROFILE_ENTRIES];
	};
*/
////////////////////////////////////////////////////////////////////////////////
//	ObjectManager
////////////////////////////////////////////////////////////////////////////////
    class ObjectManager : public IObjectManager 
    {
    public:
        //  queue messages
        enum Messages
        {
//            eActivate,
//            eDeactivate,
            eConnectMono,
            eConnectStereo,
            eConnectSurround,
            eConnectSurroundLFE,
            eConnectControl,
            eConnectEvent,
			eConnectGeneric,
			eConnectCoordinate,
            eDisconnectMono,
            eDisconnectStereo,
            eDisconnectSurround,
            eDisconnectSurroundLFE,
            eDisconnectControl,
            eDisconnectEvent,
			eDisconnectCoordinate,
			eDisconnectGeneric,
			eSuspend,
			eResume,
			eMute,
			eUnmute,
//			eRunSetupScript,
			eCreateObject,
			eDestroyObject,
			eCreateInstance,
			eDestroyInstance,
			eGetObjectActionIn,
			eGetObjectActionOut,
			eGetObjectParameterIn,
			eGetObjectParameterOut,
			eGetObjectVectorIn,
			eGetObjectControlInput,
			eGetObjectEventInput,
			eGetObjectVectorInput,
			eGetObjectGraphInput,
			eGetObjectMonoAudioInput,
			eGetObjectStereoAudioInput,
			eGetObjectSurroundAudioInput,
			eGetObjectSurroundLFEAudioInput,
			eGetObjectControlOutput,
			eGetObjectEventOutput,
			eGetObjectVectorOutput,
			eGetObjectGraphOutput,
			eGetObjectMonoAudioOutput,
			eGetObjectStereoAudioOutput,
			eGetObjectSurroundAudioOutput,
			eGetObjectSurroundLFEAudioOutput,
			eGetComponentGeneric,

			eGetObjectActionInFromHash,
			eGetObjectActionOutFromHash,
			eGetObjectParameterInFromHash,
			eGetObjectParameterOutFromHash,
			eGetObjectVectorInFromHash,
			eGetObjectControlInputFromHash,
			eGetObjectEventInputFromHash,
			eGetObjectVectorInputFromHash,
			eGetObjectGraphInputFromHash,
			eGetObjectMonoAudioInputFromHash,
			eGetObjectStereoAudioInputFromHash,
			eGetObjectSurroundAudioInputFromHash,
			eGetObjectSurroundLFEAudioInputFromHash,
			eGetObjectControlOutputFromHash,
			eGetObjectEventOutputFromHash,
			eGetObjectVectorOutputFromHash,
			eGetObjectGraphOutputFromHash,
			eGetObjectMonoAudioOutputFromHash,
			eGetObjectStereoAudioOutputFromHash,
			eGetObjectSurroundAudioOutputFromHash,
			eGetObjectSurroundLFEAudioOutputFromHash,
			eGetComponentGenericFromHash,

			eSetValue,
			eValue,
			eExecute,
			eRegisterActionOutCallback,
			eSetDimensions,
			eReleaseObjectComponentHandle,

            NumMessages
        };

        //  packets
        struct Packet
        {
            U32 uMessage;
        };

		//struct ScriptPacket
		//{
		//	Packet packet;
		//	SetupScriptHandle hScript;
		//};

		struct InstancePacket
		{
			Packet packet;
			InstanceHandle hInstance;
		};

        struct ConnectionPacket
        {
			Packet packet;
			InstanceComponentHandle hOut;
			InstanceComponentHandle hIn;
		};

        struct DisconnectionPacket
        {
			Packet packet;
			InstanceComponentHandle hOut;
        };

		struct GenericDisconnectionPacket
		{
			Packet packet;
			InstanceComponentHandle hOut;
			InstanceComponentHandle hIn;
		};

		struct ObjectCreationPacket
		{
			Packet packet;
			ObjectHandle hObject;
			IWaveManager* pWaveManager;
			U32 uFilenameLen;
		};

		struct InstanceCreationPacket
		{
			Packet packet;
			ObjectHandle hObject;
			InstanceHandle hInstance;
			#if !defined PLATFORM_SPU && !defined PLATFORM_PS3 && !defined CFG_GOLD
			char pchEntityName[16];
			char pchGroupName[16];
			#endif
		};

		struct ObjectDestructionPacket
		{
			Packet packet;
			ObjectHandle hObject;
			bool bReserveHandle;
		};

		struct InstanceDestructionPacket
		{
			Packet packet;
			InstanceHandle hInstance;
			bool bReserveHandle;
		};

		struct ReleaseObjectComponentHandlePacket
		{
			Packet packet;
			ObjectComponentHandle hObjComp;
			bool bReserveHandle;
		};

		struct GetComponentPacket
		{
			Packet packet;
			ObjectHandle hObject;
			ObjectComponentHandle hComponent;
		};

		struct GetComponentFromHashPacket
		{
			Packet packet;
			ObjectHandle hObject;
			ObjectComponentHandle hComponent;
			StringHash nameHash;
		};

		struct SetValuePacket
		{
			Packet packet;
			InstanceComponentHandle hParamIn;
			F32 fValue;
		};

		struct GetValuePacket
		{
			Packet packet;
			InstanceComponentHandle hParamOut;
			U32 hDataTracker;
			AudioRequestDataCallback pCallback;
		};

		struct ExecutePacket
		{
			Packet packet;
			InstanceComponentHandle hActionIn;
		};

		//struct CountPacket
		//{
		//	Packet packet;
		//	InstanceComponentHandle hActionOut;
		//	AudioRequestDataCallback pCallback;
		//	bool bReset;
		//};

		struct SetDimensionsPacket
		{
			Packet packet;
			InstanceComponentHandle hVectorIn;
			F32 afDimensions[3];
		};

		struct RegisterActionOutCallbackPacket
		{
			Packet packet;
			InstanceComponentHandle hActionOut;
			AudioActionOutCallback pCallback;
			U32 u32AppData;
		};

    public:
        ObjectManager();
        ~ObjectManager();

		void Init(U32 uTempFrameBufferSize, ISystem::eDistributedUpdateSetting eDistributedUpdate, Scheduler::PolicyHandle hPolicy, Scheduler::GroupHandle hGroup, Scheduler::PolicyHandle hPolicyB);
		ObjectHandle Create(const char* pchFilename, IWaveManager* pWaveManager = 0);
		ObjectHandle CreateImmediate(const char* pchFilename, IWaveManager* pWaveManager = 0);
		bool CreateUsingHandle(const char* pchFilename, IWaveManager* pWaveManager, ObjectHandle hObject);
		Result Destroy(ObjectHandle hObject, bool bReserveHandle = false);
		void DestroyImmediate(ObjectHandle hObject, bool bReserveHandle = false);
        Result InternalDestroy(ObjectHandle hObject, bool bReserveHandle);

		void Update();

		bool Suspend(InstanceHandle hInstance, bool bSuspend);
        bool IsSuspended(InstanceHandle hInstance);
		bool Mute(InstanceHandle hInstance, bool bMute);
        bool IsMuted(InstanceHandle hInstance);

		//	Queue methods
		bool ConnectGeneric(InstanceComponentHandle hOut, InstanceComponentHandle hIn);
		bool DisconnectGeneric(InstanceComponentHandle hOut, InstanceComponentHandle hIn);
		bool RegisterActionOutCallback(InstanceComponentHandle hActionOut, AudioActionOutCallback pCallback, U32 u32AppData);

        void ReceiveData(void* pData, U32 uSize);

        void ReceiveSuspend(InstanceHandle hInstance);
        void ReceiveResume(InstanceHandle hInstance);
		void ReceiveMute(InstanceHandle hInstance);
        void ReceiveUnmute(InstanceHandle hInstance);
		void ReceiveConnectControl(InstanceComponentHandle hControlOut, InstanceComponentHandle hControlIn);
        void ReceiveConnectEvent(InstanceComponentHandle hEventOut, InstanceComponentHandle hEventIn);
		void ReceiveConnectGeneric(InstanceComponentHandle hOut, InstanceComponentHandle hIn);
		void ReceiveDisconnectGeneric(InstanceComponentHandle hOut, InstanceComponentHandle hIn);
		void ReceiveCreate(ObjectHandle hObject, char* pchFilename, IWaveManager* pWaveManager);
		void ReceiveDestroy(ObjectHandle hObject, bool bReserveHandle);
		void ReceiveCreateInstance(ObjectHandle hObject, InstanceHandle hInstance, const char* pchGroupName = 0, const char* pchEntityName = 0);
		void ReceiveDestroyInstance(InstanceHandle hInstance, bool bReserveHandle);
		void ReceiveGetComponent(U32 u32Message, char* pchName, ObjectHandle hObject, ObjectComponentHandle hComponent);
		void ReceiveGetComponentFromHash(U32 u32Message, StringHash nameHash, ObjectHandle hObject, ObjectComponentHandle hComponent);
		void ReceiveReleaseObjectComponentHandle(ObjectComponentHandle hObjComp, bool bReserveHandle);
		void ReceiveSetValue(InstanceComponentHandle hParamIn, F32 fValue);
		void ReceiveValue(InstanceComponentHandle hParamOut, U32 hDataTracker, AudioRequestDataCallback pCallback);
		void ReceiveExecute(InstanceComponentHandle hActionIn);
		void ReceiveSetDimensions(InstanceComponentHandle hVectorIn, F32* afDimensions);
		void ReceiveRegisterActionOutCallback(InstanceComponentHandle hActionOut, AudioActionOutCallback pCallback, U32 u32AppData);

		// object access functions

        InstanceHandle CreateObjectInstance(ObjectHandle hObject, const char* pchGroupName=0, const char* pchEntityName=0);
		bool CreateObjectInstanceUsingHandle(ObjectHandle hObject, InstanceHandle hInstance, const char* pchGroupName=0, const char* pchEntityName=0);
        void DestroyObjectInstance(InstanceHandle hInstance, bool bReserveHandle = false) ;

		bool IsInstanceValid(InstanceHandle hInstance);
		bool IsObjectValid(ObjectHandle hObject);
		bool IsObjectComponentValid(ObjectComponentHandle hComponent);
		bool IsInstanceComponentValid(InstanceComponentHandle hComponent);

        void ConfigureAudio(Instance* pInstance) ;

		ObjectComponentHandle GetObjectActionIn(ObjectHandle hObject, const char* pchName);				
		ObjectComponentHandle GetObjectActionOut(ObjectHandle hObject, const char* pchName);				
		ObjectComponentHandle GetObjectParameterIn(ObjectHandle hObject, const char* pchName);			
		ObjectComponentHandle GetObjectParameterOut(ObjectHandle hObject, const char* pchName);			
		ObjectComponentHandle GetObjectVectorIn(ObjectHandle hObject, const char* pchName);				
		ObjectComponentHandle GetObjectControlInput(ObjectHandle hObject, const char* pchName);			
		ObjectComponentHandle GetObjectEventInput(ObjectHandle hObject, const char* pchName);				
		ObjectComponentHandle GetObjectVectorInput(ObjectHandle hObject, const char* pchName);			
		ObjectComponentHandle GetObjectGraphInput(ObjectHandle hObject, const char* pchName);				
		ObjectComponentHandle GetObjectMonoAudioInput(ObjectHandle hObject, const char* pchName);			
		ObjectComponentHandle GetObjectStereoAudioInput(ObjectHandle hObject, const char* pchName);		
		ObjectComponentHandle GetObjectSurroundAudioInput(ObjectHandle hObject, const char* pchName);		
		ObjectComponentHandle GetObjectSurroundLFEAudioInput(ObjectHandle hObject, const char* pchName);	
		ObjectComponentHandle GetObjectControlOutput(ObjectHandle hObject, const char* pchName);			
		ObjectComponentHandle GetObjectEventOutput(ObjectHandle hObject, const char* pchName);			
		ObjectComponentHandle GetObjectVectorOutput(ObjectHandle hObject, const char* pchName);			
		ObjectComponentHandle GetObjectGraphOutput(ObjectHandle hObject, const char* pchName);			
		ObjectComponentHandle GetObjectMonoAudioOutput(ObjectHandle hObject, const char* pchName);		
		ObjectComponentHandle GetObjectStereoAudioOutput(ObjectHandle hObject, const char* pchName);		
		ObjectComponentHandle GetObjectSurroundAudioOutput(ObjectHandle hObject, const char* pchName);	
		ObjectComponentHandle GetObjectSurroundLFEAudioOutput(ObjectHandle hObject, const char* pchName);	
		ObjectComponentHandle GetObjectComponent(ObjectHandle hObject, const char* pchName);

		ObjectComponentHandle GetObjectActionInFromHash(ObjectHandle hObject, StringHash nameHash);				
		ObjectComponentHandle GetObjectActionOutFromHash(ObjectHandle hObject, StringHash nameHash);				
		ObjectComponentHandle GetObjectParameterInFromHash(ObjectHandle hObject, StringHash nameHash);			
		ObjectComponentHandle GetObjectParameterOutFromHash(ObjectHandle hObject, StringHash nameHash);			
		ObjectComponentHandle GetObjectVectorInFromHash(ObjectHandle hObject, StringHash nameHash);				
		ObjectComponentHandle GetObjectControlInputFromHash(ObjectHandle hObject, StringHash nameHash);			
		ObjectComponentHandle GetObjectEventInputFromHash(ObjectHandle hObject, StringHash nameHash);				
		ObjectComponentHandle GetObjectVectorInputFromHash(ObjectHandle hObject, StringHash nameHash);			
		ObjectComponentHandle GetObjectGraphInputFromHash(ObjectHandle hObject, StringHash nameHash);				
		ObjectComponentHandle GetObjectMonoAudioInputFromHash(ObjectHandle hObject, StringHash nameHash);			
		ObjectComponentHandle GetObjectStereoAudioInputFromHash(ObjectHandle hObject, StringHash nameHash);		
		ObjectComponentHandle GetObjectSurroundAudioInputFromHash(ObjectHandle hObject, StringHash nameHash);		
		ObjectComponentHandle GetObjectSurroundLFEAudioInputFromHash(ObjectHandle hObject, StringHash nameHash);	
		ObjectComponentHandle GetObjectControlOutputFromHash(ObjectHandle hObject, StringHash nameHash);			
		ObjectComponentHandle GetObjectEventOutputFromHash(ObjectHandle hObject, StringHash nameHash);			
		ObjectComponentHandle GetObjectVectorOutputFromHash(ObjectHandle hObject, StringHash nameHash);			
		ObjectComponentHandle GetObjectGraphOutputFromHash(ObjectHandle hObject, StringHash nameHash);			
		ObjectComponentHandle GetObjectMonoAudioOutputFromHash(ObjectHandle hObject, StringHash nameHash);		
		ObjectComponentHandle GetObjectStereoAudioOutputFromHash(ObjectHandle hObject, StringHash nameHash);		
		ObjectComponentHandle GetObjectSurroundAudioOutputFromHash(ObjectHandle hObject, StringHash nameHash);	
		ObjectComponentHandle GetObjectSurroundLFEAudioOutputFromHash(ObjectHandle hObject, StringHash nameHash);	
		ObjectComponentHandle GetObjectComponentFromHash(ObjectHandle hObject, StringHash nameHash);

		void SetObjectFrameSkip(ObjectHandle hObject, U32 frameSkip) ;

		bool ReuseObjectComponent(ObjectHandle hObject, ObjectComponentHandle hObjComp, const char* pchName);
		bool ReuseObjectComponent(ObjectHandle hObject, ObjectComponentHandle hObjComp, StringHash nameHash);

		void SetValue(InstanceComponentHandle hParamIn, F32 fValue);
		void Value(InstanceComponentHandle hParamOut, U32 hDataTracker, AudioRequestDataCallback pCallback);
		void Execute(InstanceComponentHandle hActionIn);
//		void Count(InstanceComponentHandle hActionOut, bool bReset = false, AudioRequestDataCallback pCallback = 0);
		void SetDimensions(InstanceComponentHandle hVectorIn, F32* afDimensions);

		/// add the given instance into the active list
		void LinkActive(Instance *pInstance);
		/// remove the given instance from the active list
		void UnlinkActive(Instance *pInstance);

		ObjectComponentHandle GetObjectComponentHandle(ObjectHandle hObject, ObjectManager::Messages eMessage, const char* pchName);
		ObjectComponentHandle GetObjectComponentHandleFromHash(ObjectHandle hObject, ObjectManager::Messages eMessage, StringHash nameHash);

		InstanceComponentHandle GetInstanceComponentHandle(ObjectComponentHandle hObjComp, InstanceHandle hInstance);
		U32 GetResourceFromObjectComponentHandle(ObjectComponentHandle hObjComp)
		{
			if (m_ObjectComponentHandles.IsResourceRetrievable(hObjComp))
				return m_ObjectComponentHandles.GetResource(hObjComp); 
			
			return 0xffffffff;
		}
		U32 GetDoomedResourceFromObjectComponentHandle(ObjectComponentHandle hObjComp)
		{
			return m_ObjectComponentHandles.GetDoomedResource(hObjComp);
		}
		bool DereferenceInstanceComponentHandle(InstanceComponentHandle hComponent, U32 u32Flags, Instance** ppInst, Component** ppComp, bool bInstanceIsDoomed = false);
		inline bool DereferenceDoomedInstanceComponentHandle(InstanceComponentHandle hComponent, U32 u32Flags, Instance** ppInst, Component** ppComp) { return DereferenceInstanceComponentHandle(hComponent, u32Flags, ppInst, ppComp, true); }

		bool ReleaseObjectComponentHandle(ObjectComponentHandle hObjComp, bool bReserveHandle = false);

		U32 TempFrameBufferSizeInBytes() { return m_uTempFrameBufferSize; }
		U32 TempFrameBufferSizeInSamples() { return m_uTempFrameBufferSize / sizeof(F32); }

		inline Object* GetObject(ObjectHandle hObj)
		{
			if (m_ObjectHandles.IsHandleValid(hObj))
				return (Object*)m_ObjectHandles.GetResource(hObj);

			return NULL;
		}

		inline Object* GetDoomedObject(ObjectHandle hObj)
		{
			if (m_ObjectHandles.IsHandleValid(hObj))
				return (Object*)m_ObjectHandles.GetDoomedResource(hObj);
				
			return NULL;
		}
		
		inline bool IsObjectReservedForUse(ObjectHandle hObj)
		{
			if (m_ObjectHandles.IsHandleValid(hObj))
				return m_ObjectHandles.IsReserved(hObj);

			return false;
		}

		inline Instance* GetInstance(InstanceHandle hInst)
		{
			if (m_InstanceHandles.IsResourceRetrievable(hInst))
				return (Instance*)m_InstanceHandles.GetResource(hInst);
			
			return NULL;
		}

		inline Instance* GetDoomedInstance(InstanceHandle hInst)
		{
			if (m_InstanceHandles.IsHandleValid(hInst))
				return (Instance*)m_InstanceHandles.GetDoomedResource(hInst);

			return NULL;
		}
 
		inline Ut::Vector<Object*>& GetObjects() { return m_Objects; }

		static F32* GetDynamicBuffers()
		{
			if (Scheduler::GetWorkerID() < 7)
				return &m_pfMixerFrames[ Scheduler::GetWorkerID() * m_uTempFrameBufferSize / sizeof(F32) ];
			else
				return m_pfMixerFrames;
		}

		// debug
		U32 GetNumMixersActive() { return m_u32NumMixersActive; }
		U32 GetNumInstancesActive() { return m_u32NumInstancesActive; }
		U32 GetNumInstancesMuted() { return m_u32NumInstancesMuted; }
		U32 GetNumTotalInstances() { return m_u32NumTotalInstances; }
		U32 GetPeakTempFrameBufferSize() { return m_uPeakTempFrameBufferSize; }
		void SetPeakTempFrameBufferSize(U32 uSize) { m_uPeakTempFrameBufferSize = uSize; }
		void SetPeakTempFrameBufferObjectName(const char* pchName);
		char* GetPeakTempFrameBufferObjectName();
		static void LogPeakTempInstanceBufferUsage();
		U32 GetMaxInstanceStateDataSize() { return m_uPeakInstanceStateDataSize; }
		void SetMaxInstanceStateDataSize(U32 uSize) { m_uPeakInstanceStateDataSize = uSize; }
		void SetPeakInstanceStateDataObjectName(const char* pchName);
		char* GetPeakInstanceStateDataObjectName();

		Util::C_String GetPath();
		
		Core::Lock* GetLock() { return &m_Lock; }

	private:
		Object* InternalCreate(U32 uFrameSkip);
		
        void Activate(Instance* pInstance);
        void Deactivate(Instance* pInstance);

		void LogActiveInstanceStates();
		void LogAllInstanceStates();

        Instance* m_pFirst;
        Instance* m_pLast;
		Instance* m_pFirstActive;
		Instance* m_pLastActive;

		HandleCollection<ObjectHandle, 0> m_ObjectHandles;
		Ut::Vector<Object*> m_Objects;

		HandleCollection<InstanceHandle, 0> m_InstanceHandles;
		HandleCollection<ObjectComponentHandle, -1> m_ObjectComponentHandles;

		Scheduler::GroupHandle m_hAudioSchedulerGroup;
		Scheduler::PolicyHandle m_hAudioPolicy;

		// for debug
		U32 m_u32NumMixersActive;
		U32 m_u32NumInstancesActive;
		U32 m_u32NumInstancesMuted;
		U32 m_u32NumTotalInstances;
		static U32 m_uPeakTempFrameBufferSize;
		static U32 m_uPeakInstanceStateDataSize;
		#ifndef CFG_GOLD
		static char m_pchPeakFrameBufferObjectName[32];
		static char m_pchPeakInstanceStateDataObjectName[32];
		#endif

		static U32 m_uTempFrameBufferSize;

		Core::Lock m_Lock;

		bool m_bDistributeInstanceUpdateTasks;

		StaticBufferAllocator m_aBatchListMemory;

	//	temp
	public:
		static F32* m_pfMixerFrames;

	private:
		Core::Lock m_lockObject;
    };
}

#endif // _OBJECTMANAGER_H_

