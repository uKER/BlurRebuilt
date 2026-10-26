#ifndef _APP_INTERFACE_CONTROLLER_H_
#define _APP_INTERFACE_CONTROLLER_H_

#ifdef _MSC_VER
#pragma once
#endif

#define MAX_PREVIOUS_STATE_HISTORY		5

namespace InterfaceLayerLogger {
extern Debug::Logger &g_Logger;
}

namespace Audio
{

	typedef U32					InstanceDefinitionHandle;
	typedef PagedArrayHandle	ReturnDataHandle;


	enum ConnectionType
	{
		eMono = 0,
		eStereo,
		eSurround,
		eSurroundLFE,
		eControl,
		eEvent
	};

	// derive from this to provide your own callback
	class EntityCallbackBase
	{
	public:
							EntityCallbackBase() : m_u32CallCount(0) {}
		virtual				~EntityCallbackBase() {}

		virtual void		Callback() = 0;
		U32					CallCount() { return m_u32CallCount; }

		AudioEntityHandle	m_hEntity;
		U32					m_u32CallCount;
	};

	// the default callback handler
	// sends the callback to the entity's subsystem
	class DefaultEntityCallback : public EntityCallbackBase
	{
	public:
							DefaultEntityCallback(AudioEntityHandle hEnt, U32 uMsgId = 0)
							{
								m_hEntity = hEnt;
								m_u32MessageId = uMsgId;
							}
		
		void				Callback();

		U32					m_u32MessageId;
	};


	class EntityDataPollInfo
	{

	public:
		EntityDataPollInfo() { m_hEntityMember = (U32)-1; m_f32Value = 0.f; }

		AudioEntityHandle		m_hEntity;
		EntityDataMemberHandle	m_hEntityMember;
		F32						m_f32Value;
	};


	class InterfaceController
	{
	public:
		friend class Subsystem;
		friend class DefaultEntityCallback;

		static InterfaceController& Get( )
		{
			static InterfaceController instance;
			return instance;
		}

		// setting up
		bool											Init(U32 u32NumSubsystems, Audio::ISystem* iSystem, Audio::IWaveManager* iWaveManager);
		bool											AddSubsystem(const Subsystem* pSubsystem, const U32 u32SubsystemId);

		void											BroadcastMessage(const U32 u32MessageId, const U32 u32SubsystemFlags = 0xffffffff);
		bool											SendMessageToSubsystem(const U32 u32SubsystemId, const U32 u32MessageId, U32 u32Data = 0, void* pData = 0);
		void											SetNewState(const U32 u32StateId);
		inline U32										GetCurrentState() { return m_u32CurrentState; }
		U32												GetPreviousState(U32 u32OffsetFromPrevious = 0);
		bool											DeactivateSubsystems(const U32 u32SubsystemFlags);
		bool											ActivateSubsystems(const U32 u32SubsystemFlags);
		//AudioEntityTypeHandle							CreateEntityType(const EntityDefinitionHandle hDefinition, U32 u32SubsystemId);
		//AudioEntityHandle								CreateEntity(const AudioEntityTypeHandle& hParentEntityType);
		bool											CreateEntityType(const EntityDefinitionHandle hDefinition, U32 u32SubsystemId, AudioEntityTypeHandle& hType);
		bool											CreateEntity(const AudioEntityTypeHandle& hParentEntityType, AudioEntityHandle& hEntity);
		bool											ReleaseEntityType(const AudioEntityTypeHandle& hEntityType);
		bool											ReleaseEntity(const AudioEntityHandle& hEntity);
		bool											SuspendEntity(const AudioEntityHandle& hEntity);
		bool											ResumeEntity(const AudioEntityHandle& hEntity);
		bool											MuteEntity(const AudioEntityHandle& hEntity);
		bool											UnmuteEntity(const AudioEntityHandle& hEntity);
		bool											SuspendEntity(Audio::EntityInstance* pInstance);
		bool											ResumeEntity(Audio::EntityInstance* pInstance);
		bool											SuspendInstance(const AudioEntityHandle& hEntity, const EntityDataMemberHandle hInstance);
		bool											ResumeInstance(const AudioEntityHandle& hEntity, const EntityDataMemberHandle hInstance);
		bool											IsInstanceSuspended(const AudioEntityHandle& hEntity, const EntityDataMemberHandle hInstance);

		bool											IsEntitySuspended(const AudioEntityHandle& hEntity);



		EntityDefinitionHandle							GetDefinitionHandle(const AudioEntityHandle& hEntity); 

		EntityTypeDataMemberHandle						LookupObject(const EntityDefinitionHandle hDefinition, const char* strName);
		EntityTypeDataMemberHandle						LookupObjectComponent(const EntityDefinitionHandle hDefinition, const char* strName);
		EntityDataMemberHandle							LookupInstance(const EntityDefinitionHandle hDefinition, const char* strName);
		EntityDataMemberHandle							LookupInstance(const EntityDefinitionHandle hDefinition, const StringHash hash );
		EntityDataMemberHandle							LookupInstanceComponent(const EntityDefinitionHandle hDefinition, const char* strName);
		EntityDataMemberHandle							LookupInstanceComponent(const EntityDefinitionHandle hDefinition, const StringHash hash);
		EntityDataMemberHandle							LookupInstanceComponent(const AudioEntityHandle& hEntity, const char* strName);

		S32												GetNumParameterisedFunctions(const EntityDefinitionHandle hEntity, const char* strName);
		LocalFunctionHandle								LookupParameterisedFunctionByIndex(const EntityDefinitionHandle hEntity, const char* strName, const S32 sIndex);
		LocalFunctionHandle								LookupParameterisedFunctionById(const EntityDefinitionHandle hEntity, const char* strName, const U32 uParameterId);

		PagedArrayHandle								SetCallback(EntityCallbackBase* pData, const EntityDataMemberHandle hMember);

		U32												GetCallbackTriggeredCount(PagedArrayHandle hCallback, bool bReset = false);

		ReturnDataHandle								TrackValue(const AudioEntityHandle& hEntity, EntityDataMemberHandle hEntityMember);
		bool											UntrackValue(ReturnDataHandle& hValue);
		bool											GetValue(F32& f32Dest, ReturnDataHandle& hValue);

		bool											TriggerEntityActionIn(const AudioEntityHandle& hEntity, EntityDataMemberHandle hEntityMember,const bool resumeOnActionIn = true);
		bool											SetEntityVectorIn(const AudioEntityHandle& hEntity, EntityDataMemberHandle hEntityMember, F32* afVec);

		bool											SetEntityValue(const AudioEntityHandle& hEntity, EntityDataMemberHandle hEntityMember, F32 f32Value);
//		F32												GetEntityValue(const AudioEntityHandle& hEntity, EntityDataMemberHandle hMember, bool bReportErrors = false);

		bool											IsDataMemberValid(const AudioEntityHandle& hEntity, EntityDataMemberHandle hMember);

		bool											ReloadSubsystems(const U32 u32SubsystemFlags);
		bool											ConnectEntities(const AudioEntityHandle& hUpstreamEntity, const AudioEntityHandle& hDownstreamEntity);
		bool											RunFunctionInEntityType(const AudioEntityTypeHandle& hEntityType, const char* strFunctionName);
		bool											RunFunctionInEntityInstance(const AudioEntityHandle& hEntityInstance, const char* strFunctionName);
		bool											RunFunctionInParameterisedEntityInstance(const AudioEntityHandle& hEntityInstance, const AudioEntityHandle& hParameterEntityInstance, const char* strFunctionName);
		bool											RunFunctionInParameterisedEntityInstance(const AudioEntityHandle& hEntityInstance, const AudioEntityHandle& hParameterEntityInstance, const LocalFunctionHandle hFunc);
        bool											SetMaster(const AudioEntityHandle& hEntityInstance, const char* strOutput);
        bool											SetMonoMaster(const AudioEntityHandle& hEntityInstance, const char* strOutput);
        bool											SetStereoMaster(const AudioEntityHandle& hEntityInstance, const char* strOutput);
        bool											SetSurroundMaster(const AudioEntityHandle& hEntityInstance, const char* strOutput);
        bool											SetSurroundLFEMaster(const AudioEntityHandle& hEntityInstance, const char* strOutput);
        bool											SetCustomDataPtr(const AudioEntityHandle& hEntityInstance, void* pData);
        const void*										GetCustomDataPtr(const AudioEntityHandle& hEntityInstance);
        bool											SetCustomDataFreed(const AudioEntityHandle& hEntityInstance);

		void											InternalAssert(bool bExp, const char* strMessage = 0);
		void											Error(const char* strMessage);
		void											Warning(const char* strMessage);
		void											Info(const char* strMessage);
		void											Fatal(const char* strMessage);
		static Audio::ISystem*							GetAudioSystem() { return m_iSystem; }
		inline Audio::IWaveManager*						GetWaveManager() { return m_iWaveManager; }

		// temporary.. we probably don't want to expose the entity manager to everything
		EntityManager&									GetEntityManager() { return m_EntityManager; }

		void											Release();

#ifndef CFG_GOLD
		void											ReloadAllAudio();
#endif

		Subsystem*										GetSubsystem( const U32 u32SubsystemId )
		{
			if( m_apSubsystem && u32SubsystemId < m_u32NumSubsystems )
				return m_apSubsystem[u32SubsystemId];
			else
				return NULL;
		}

		void 											SetSystemSpeakerDistance( U32 eOption, F32 fDistance );
		void 											SetSystemSpeakerAngles( U32 eMode, F32 fFrontLeft, F32 fFrontRight, F32 fSurroundLeft = 0.f, F32 fSurroundRight = 0.f, F32 fBackLeft = 0.f, F32 fBackRight = 0.f );
		void 											SetSystemSpeakerOption( U32 eOption );
#ifdef PLATFORM_PC
		//! [Blur2PC] T144: retail PC 0x00535ab0
		void 											SetSystemOutputSampleRate( U32 uSampleRate );
#endif

	


	//protected:	// why wasn't this public?
	public:
		void											Update();

	//private:
		InterfaceController();
		~InterfaceController();
		Subsystem*										GetParentSubsystem(const AudioEntityHandle& hEntity);
		EntityType*										PrivateCreateEntityType(EntityDefinitionHandle hDefinition) { return m_EntityManager.SpawnEntityType(hDefinition); }
		EntityInstance*									PrivateCreateEntityInstance(EntityType* pType) { return m_EntityManager.SpawnEntityInstance(pType); }
		bool											RunFunctionInParameterisedEntityInstance(EntityInstance* pEntityInstance, EntityInstance* pParameterEntityInstance, const LocalFunctionHandle hFunc);
		EntityType*										GetEntityTypeFromHandle(AudioEntityTypeHandle hType);
		EntityInstance*									GetEntityInstanceFromHandle(AudioEntityHandle hInstance);
		EntityDefinition*								GetDefinition(EntityInstance* pInstance) { return m_EntityManager.GetDefinitionFromHandle(pInstance->GetDefinitionHandle()); }
		EntityDefinition*								GetDefinition(EntityType* pType) { return m_EntityManager.GetDefinitionFromHandle(pType->GetDefinitionHandle()); }

		bool											RunFunctionInParameterisedEntityInstance(const AudioEntityHandle& hEntityInstance, const AudioEntityHandle& hParameterEntityInstance, const char* strFunctionName, const LocalFunctionHandle hFunction);

		static void										StaticProcessCallback(U32 u32Data);
		void											ProcessCallback(U32 u32Data);

		Subsystem**										m_apSubsystem;
		U32												m_u32NumSubsystems;
		Audio::IWaveManager*							m_iWaveManager;
		static Audio::ISystem*							m_iSystem;
		PagedArray<EntityCallbackBase*>					m_CallbackDataList;
		PagedArray<EntityDataPollInfo*>					m_DataTracker;
		U32												m_u32CurrentState;
		U32												m_au32PreviousStateHistory[MAX_PREVIOUS_STATE_HISTORY];
		U32												m_u32PreviousStateHistoryPosition;
		EntityManager									m_EntityManager;
		MT::Lock										m_Lock;
		MT::Lock										m_CallbackLock;
		MT::Lock										m_DataTrackerLock;
	};
}

#endif	//	#ifndef _APP_INTERFACE_CONTROLLER_H_
