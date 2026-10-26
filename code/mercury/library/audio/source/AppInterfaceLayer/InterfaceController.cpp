#include "AudioPch.h"

#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#endif

#include <System/DebugLog.h>

Audio::ISystem * Audio::InterfaceController::m_iSystem = 0;

namespace InterfaceLayerLogger {
Debug::Logger &g_Logger = Debug::GetLogger("audio.InterfaceLayerLogger");
}


Debug::Bool g_bReloadAllAudio("Audio.System.ReloadAllAudio", false);


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetParameterCallback
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool SetParameterCallback(U32 hDataTracker, F32 f32Value)
{
	Audio::InterfaceController::Get().m_DataTrackerLock.Enter();

	Audio::EntityDataPollInfo* pInfo = Audio::InterfaceController::Get().m_DataTracker.GetByHandle(hDataTracker);

	Audio::InterfaceController::Get().m_DataTrackerLock.Exit();

	if (! pInfo)
		return false;

	Audio::EntityInstance* pInstance = Audio::InterfaceController::Get().GetEntityInstanceFromHandle(pInfo->m_hEntity);

	if (! pInstance)
		return false;

	Audio::EntityDefinition* pDef = Audio::InterfaceController::Get().GetDefinition(pInstance);

	if (! pDef)
		return false;

	pInfo->m_f32Value = f32Value;
//	pInstance->SetReturnValue(f32Value, s32ParamOutIndex);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	constructor
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::InterfaceController::InterfaceController()
{
	m_u32PreviousStateHistoryPosition = 0;
	m_iWaveManager = 0;
	m_u32NumSubsystems = 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	destructor
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::InterfaceController::~InterfaceController()
{
	if (m_apSubsystem)
		delete [] m_apSubsystem;

	//for (U32 i = 0; i < m_apInstanceInterfaceDefinition.GetHighestNodeIndex(); i++)
	//{
	//	if (m_apInstanceInterfaceDefinition.GetByIndex(i))
	//	{
	//		delete m_apInstanceInterfaceDefinition.GetByIndex(i);
	//		m_apInstanceInterfaceDefinition.FreeByIndex(i);
	//	}
	//}

	m_apSubsystem = 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Init
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::Init(U32 u32NumSubsystems, Audio::ISystem* iSystem, Audio::IWaveManager* iWaveManager)
{
	if (m_iSystem)
	{
		Error("Trying to initialise audio interface controller a 2nd time!");
		return false;
	}

	m_u32NumSubsystems = u32NumSubsystems;
	m_apSubsystem = new Subsystem*[u32NumSubsystems];

	for (U32 i = 0; i < u32NumSubsystems; i++)
		m_apSubsystem[i] = 0;

	m_iWaveManager = iWaveManager;
	m_iSystem = iSystem;

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Release
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
void Audio::InterfaceController::Release()
{
	if (m_apSubsystem)
	{
		delete [] m_apSubsystem;
		m_apSubsystem = 0;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ReloadAllAudio
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CFG_GOLD
void Audio::InterfaceController::ReloadAllAudio()
{
	// loop thru all entity instances in all subsystems, running all their parameterised functions in reverse

	AUDIO_CORE_INFO("** looping thru all entity instances in all subsystems, running all their parameterised functions in reverse");

	for (U32 u32Subsystem = 0; u32Subsystem < m_u32NumSubsystems; u32Subsystem++)
	{
		for (U32 u32Entity = 0; u32Entity < m_apSubsystem[u32Subsystem]->GetHighestEntityInstanceIndex(); u32Entity++)
		{
			EntityInstance* pInstance = m_apSubsystem[u32Subsystem]->GetEntityInstanceByIndex(u32Entity);

			if (pInstance)
			{
				EntityDefinitionHandle hDef = pInstance->GetDefinitionHandle();
				EntityDefinition* pDef = InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(hDef);

				AUDIO_CORE_DEBUG("**\t Subsystem %d, instance %d (%s/%s)", u32Subsystem, u32Entity, pDef->Name().Chars(), pDef->GroupName().Chars());

				FunctionExecutionHistoryEntry historyEntry;

				for (S32 i = pInstance->GetNumFunctionExecutionHistoryEntries() - 1; i >= 0; i--)
				{
					if (pInstance->GetFunctionExecutionEntry(i, historyEntry))
					{
						if (historyEntry.eFunctionType == eParameterisedEntityInstanceFunction)
						{
							EntityInstance* pHistEntryInstance = InterfaceController::Get().GetEntityInstanceFromHandle(historyEntry.hEntity);
							EntityInstance* pHistEntryParamInstance = InterfaceController::Get().GetEntityInstanceFromHandle(historyEntry.hParameterEntity);

							if (pHistEntryInstance && pHistEntryParamInstance)
							{
								EntityDefinitionHandle hHistEntryDef = pHistEntryInstance->GetDefinitionHandle();
								EntityDefinitionHandle hHistEntryParamDef = pHistEntryParamInstance->GetDefinitionHandle();
								EntityDefinition* pHistEntryDef = InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(hHistEntryDef);

								AUDIO_CORE_DEBUG("**\t\t running function %d in reverse, other entity is %s/%s", i, pHistEntryDef->Name().Chars(), pHistEntryDef->GroupName().Chars());

								pHistEntryDef->RunReverseEntityInstanceParameterisedFunction(historyEntry.hFunction, pHistEntryInstance, pHistEntryParamInstance, true);
							}
						}
					}
				}
			}
		}
	}

	// loop thru all entity instances in all subsystems, running all their non-parameterised instance functions in reverse
	AUDIO_CORE_INFO("** looping thru all entity instances in all subsystems, running all their non-parameterised instance functions in reverse");

	for (U32 u32Subsystem = 0; u32Subsystem < m_u32NumSubsystems; u32Subsystem++)
	{
		for (U32 u32Entity = 0; u32Entity < m_apSubsystem[u32Subsystem]->GetHighestEntityInstanceIndex(); u32Entity++)
		{
			EntityInstance* pInstance = m_apSubsystem[u32Subsystem]->GetEntityInstanceByIndex(u32Entity);

			if (pInstance)
			{
				EntityDefinitionHandle hDef = pInstance->GetDefinitionHandle();
				EntityDefinition* pDef = InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(hDef);

				AUDIO_CORE_DEBUG("**\t Subsystem %d, instance %d (%s/%s)", u32Subsystem, u32Entity, pDef->Name().Chars(), pDef->GroupName().Chars());

				FunctionExecutionHistoryEntry historyEntry;

				for (S32 i = pInstance->GetNumFunctionExecutionHistoryEntries() - 1; i >= 0; i--)
				{
					if (pInstance->GetFunctionExecutionEntry(i, historyEntry))
					{
						if (historyEntry.eFunctionType == eEntityInstanceFunction)
						{
							AUDIO_CORE_DEBUG("**\t\t running function %d in reverse", i);
							pDef->RunReverseEntityInstanceFunction(historyEntry.hFunction, pInstance, true);
						}
					}
				}
			}
		}
	}

	// loop thru all entity types in all subsystems, running all their type functions in reverse
	AUDIO_CORE_INFO("** looping thru all entity types in all subsystems, running all their type functions in reverse");

	for (U32 u32Subsystem = 0; u32Subsystem < m_u32NumSubsystems; u32Subsystem++)
	{
		for (U32 u32Entity = 0; u32Entity < m_apSubsystem[u32Subsystem]->GetHighestEntityTypeIndex(); u32Entity++)
		{
			EntityType* pType = m_apSubsystem[u32Subsystem]->GetEntityTypeByIndex(u32Entity);

			if (pType)
			{
				EntityDefinitionHandle hDef = pType->GetDefinitionHandle();
				EntityDefinition* pDef = InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(hDef);

				AUDIO_CORE_DEBUG("**\t Subsystem %d, type %d (%s/%s)", u32Subsystem, u32Entity, pDef->Name().Chars(), pDef->GroupName().Chars());

				FunctionExecutionHistoryEntry historyEntry;

				for (S32 i = pType->GetNumFunctionExecutionHistoryEntries() - 1; i >= 0; i--)
				{
					if (pType->GetFunctionExecutionEntry(i, historyEntry))
					{
						if (historyEntry.eFunctionType == eEntityTypeFunction)
						{
							AUDIO_CORE_DEBUG("**\t\t running function %d in reverse", i);
							pDef->RunReverseEntityTypeFunction(historyEntry.hFunction, pType, true);
						}
					}
				}
			}
		}
	}

	// loop thru all entity types in all subsystems, running all their type functions normally (ie. forwards)
	AUDIO_CORE_INFO("** looping thru all entity types in all subsystems, running all their type functions normally (ie. forwards)");

	for (U32 u32Subsystem = 0; u32Subsystem < m_u32NumSubsystems; u32Subsystem++)
	{
		for (U32 u32Entity = 0; u32Entity < m_apSubsystem[u32Subsystem]->GetHighestEntityTypeIndex(); u32Entity++)
		{
			EntityType* pType = m_apSubsystem[u32Subsystem]->GetEntityTypeByIndex(u32Entity);

			if (pType)
			{
				EntityDefinitionHandle hDef = pType->GetDefinitionHandle();
				EntityDefinition* pDef = InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(hDef);

				AUDIO_CORE_DEBUG("**\t Subsystem %d, type %d (%s/%s)", u32Subsystem, u32Entity, pDef->Name().Chars(), pDef->GroupName().Chars());

				FunctionExecutionHistoryEntry historyEntry;

				for (U32 i = 0; i < pType->GetNumFunctionExecutionHistoryEntries(); i++)
				{
					if (pType->GetFunctionExecutionEntry(i, historyEntry))
					{
						if (historyEntry.eFunctionType == eEntityTypeFunction)
						{
							AUDIO_CORE_DEBUG("**\t\t running function %d", i);
							pDef->RunEntityTypeFunction(historyEntry.hFunction, pType, true);
						}
					}
				}
			}
		}
	}

	// loop thru all entity instances in all subsystems, running all their non-parameterised instance functions normally (ie. forwards)
	AUDIO_CORE_INFO("** looping thru all entity instances in all subsystems, running all their non-parameterised instance functions normally (ie. forwards)");

	for (U32 u32Subsystem = 0; u32Subsystem < m_u32NumSubsystems; u32Subsystem++)
	{
		for (U32 u32Entity = 0; u32Entity < m_apSubsystem[u32Subsystem]->GetHighestEntityInstanceIndex(); u32Entity++)
		{
			EntityInstance* pInstance = m_apSubsystem[u32Subsystem]->GetEntityInstanceByIndex(u32Entity);

			if (pInstance)
			{
				EntityDefinitionHandle hDef = pInstance->GetDefinitionHandle();
				EntityDefinition* pDef = InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(hDef);

				AUDIO_CORE_DEBUG("**\t Subsystem %d, instance %d (%s/%s)", u32Subsystem, u32Entity, pDef->Name().Chars(), pDef->GroupName().Chars());

				FunctionExecutionHistoryEntry historyEntry;

				for (U32 i = 0; i < pInstance->GetNumFunctionExecutionHistoryEntries(); i++)
				{
					if (pInstance->GetFunctionExecutionEntry(i, historyEntry))
					{
						if (historyEntry.eFunctionType == eEntityInstanceFunction)
						{
							AUDIO_CORE_DEBUG("**\t\t running function %d", i);
							pDef->RunEntityInstanceFunction(historyEntry.hFunction, pInstance, true);
						}
					}
				}
				// connect to renderer if necessary
				if (pDef->LookupInstanceComponent("OutputToRenderer") != 0xffffffff)
					SetMaster(pInstance->GetAudioEntityHandle(), "OutputToRenderer");

				if (pDef->LookupInstanceComponent("StartActionIn") != 0xffffffff)
				{
					Audio::EntityDataMemberHandle hActionIn = LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("StartActionIn"));
					TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn, false);
				}
			}
		}
	}

	// loop thru all entity instances in all subsystems, running all their parameterised functions normally (ie. forwards)
	AUDIO_CORE_INFO("** looping thru all entity instances in all subsystems, running all their parameterised functions normally (ie. forwards)");

	for (U32 u32Subsystem = 0; u32Subsystem < m_u32NumSubsystems; u32Subsystem++)
	{
		for (U32 u32Entity = 0; u32Entity < m_apSubsystem[u32Subsystem]->GetHighestEntityInstanceIndex(); u32Entity++)
		{
			EntityInstance* pInstance = m_apSubsystem[u32Subsystem]->GetEntityInstanceByIndex(u32Entity);

			if (pInstance)
			{
				EntityDefinitionHandle hDef = pInstance->GetDefinitionHandle();
				EntityDefinition* pDef = InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(hDef);

				AUDIO_CORE_DEBUG("**\t Subsystem %d, instance %d (%s/%s)", u32Subsystem, u32Entity, pDef->Name().Chars(), pDef->GroupName().Chars());

				FunctionExecutionHistoryEntry historyEntry;

				for (U32 i = 0; i < pInstance->GetNumFunctionExecutionHistoryEntries(); i++)
				{
					if (pInstance->GetFunctionExecutionEntry(i, historyEntry))
					{
						if (historyEntry.eFunctionType == eParameterisedEntityInstanceFunction)
						{
							EntityInstance* pHistEntryInstance = InterfaceController::Get().GetEntityInstanceFromHandle(historyEntry.hEntity);
							EntityInstance* pHistEntryParamInstance = InterfaceController::Get().GetEntityInstanceFromHandle(historyEntry.hParameterEntity);

							if (pHistEntryInstance && pHistEntryParamInstance)
							{
								EntityDefinitionHandle hHistEntryDef = pHistEntryInstance->GetDefinitionHandle();
								EntityDefinition* pHistEntryDef = InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(hHistEntryDef);

								AUDIO_CORE_DEBUG("**\t\t running function %d, other entity is %s/%s", i, pHistEntryDef->Name().Chars(), pHistEntryDef->GroupName().Chars());

								pHistEntryDef->RunEntityInstanceParameterisedFunction(historyEntry.hFunction, pHistEntryInstance, pHistEntryParamInstance, true);
							}
						}
					}
				}
			}
		}
	}
}
#endif	//	#ifndef CFG_GOLD


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AddSubsystem
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::AddSubsystem(const Subsystem* pSubsystem, const U32 u32SubsystemId)
{
	if (u32SubsystemId >= m_u32NumSubsystems || m_apSubsystem[u32SubsystemId])
		return false;

	m_apSubsystem[u32SubsystemId] = (Subsystem*)pSubsystem;

	m_apSubsystem[u32SubsystemId]->SetId(u32SubsystemId);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CreateEntityInSubsystem
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
//bool Audio::InterfaceController::CreateEntityInSubsystem(EntityHandle& hEntity, const U32 u32SubsystemId)
//{
//	if (u32SubsystemId >= m_u32NumSubsystems || (! m_apSubsystem[u32SubsystemId]))
//	{
//		Error("Invalid Subsystem specified in CreateEntityInSubsystem()");
//		hEntity.m_u32PagedArrayHandle = Entity::InvalidHandle();
//
//		return false;
//	}
//
//	bool bSuccess = m_apSubsystem[u32SubsystemId]->CreateEntity(hEntity, u32SubsystemId);
//
//	if (! bSuccess)
//	{
//		Error("Couldn't create Entity in CreateEntityInSubsystem()");
//		hEntity.m_u32PagedArrayHandle = Entity::InvalidHandle();
//	}
//
//	return bSuccess;
//}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SendSystemwideMessage
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
void Audio::InterfaceController::BroadcastMessage(U32 u32MessageId, U32 u32SubsystemFlags)
{
	for (U32 i = 0; i < m_u32NumSubsystems; i++)
	{
		if (m_apSubsystem[i]->CheckIdFlags(u32SubsystemFlags))
			m_apSubsystem[i]->ProcessSystemMessage(u32MessageId);
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SendMessageToSubsystem
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::SendMessageToSubsystem(const U32 u32SubsystemId, const U32 u32MessageId, U32 u32Data, void* pData)
{
	if (!m_apSubsystem || ! m_apSubsystem[u32SubsystemId])
		return false;

	return m_apSubsystem[u32SubsystemId]->ProcessCustomMessage(u32MessageId, u32Data, pData);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetNewState
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
void Audio::InterfaceController::SetNewState(const U32 u32StateId)
{
	U32 u32OriginalState = m_u32CurrentState;
	m_au32PreviousStateHistory[m_u32PreviousStateHistoryPosition] = m_u32CurrentState;
	m_u32CurrentState = u32StateId;
	m_u32PreviousStateHistoryPosition = (m_u32PreviousStateHistoryPosition + 1) % MAX_PREVIOUS_STATE_HISTORY;

	for (U32 i = 0; i < m_u32NumSubsystems; i++)
		m_apSubsystem[i]->SendStateTransition(u32OriginalState, m_u32CurrentState);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GetPreviousState
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
U32 Audio::InterfaceController::GetPreviousState(U32 u32OffsetFromPrevious)
{
	Assert(u32OffsetFromPrevious <= MAX_PREVIOUS_STATE_HISTORY);

	if (m_u32CurrentState - u32OffsetFromPrevious < 0)
		u32OffsetFromPrevious += MAX_PREVIOUS_STATE_HISTORY;

	return m_au32PreviousStateHistory[u32OffsetFromPrevious];
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DeactivateSubsystems
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::DeactivateSubsystems(U32 u32SubsystemFlags)
{
	// TODO: this!
	return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ActivateSubsystems
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::ActivateSubsystems(U32 u32SubsystemFlags)
{
	// TODO: this!
	return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CreateEntityType
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::CreateEntityType(const EntityDefinitionHandle hDefinition, U32 u32SubsystemId, AudioEntityTypeHandle& hType)
{
	m_Lock.Enter();

	if (hType.IsValid())
	{
		Error("Trying to overwrite an in-use Entity Type handle!");
		m_Lock.Exit();
		return false;
	}

	if (u32SubsystemId >= m_u32NumSubsystems || (! m_apSubsystem[u32SubsystemId]))
	{
		Error("Invalid subsystem ID passed into CreateEntityType");
		m_Lock.Exit();
		return false;
	}

	EntityTypeHandle hEntityType = m_apSubsystem[u32SubsystemId]->CreateEntityType(hDefinition);

	AudioEntityTypeHandle hRet(hEntityType, u32SubsystemId);

	RunFunctionInEntityType(hRet, "SetupObject");

	hType = hRet;

	m_Lock.Exit();

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CreateEntity
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::CreateEntity(const AudioEntityTypeHandle& hParentEntityType, AudioEntityHandle& hEntity)
{
	m_Lock.Enter();

	if (hEntity.IsValid())
	{
		Error("Trying to overwrite an in-use Entity handle!");
		m_Lock.Exit();
		return false;
	}

	U32 u32SubsystemId = hParentEntityType.SubsystemId();

	if (u32SubsystemId >= m_u32NumSubsystems || (! m_apSubsystem[u32SubsystemId]))
	{
		Error("Invalid subsystem ID passed into CreateEntity");
		m_Lock.Exit();
		return false;
	}

	EntityInstanceHandle hInstance = m_apSubsystem[u32SubsystemId]->CreateEntityInstance(hParentEntityType.ArrayHandle());

	AudioEntityHandle hRet(hInstance, u32SubsystemId);

	EntityInstance* pInstance = GetEntityInstanceFromHandle(hRet);

	if (pInstance)
		pInstance->SetAudioEntityHandle(hRet);

	hEntity = hRet;

	m_Lock.Exit();

	return true;
} 


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ReleaseEntityType
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::ReleaseEntityType(const AudioEntityTypeHandle& hEntityType)
{
	if (! hEntityType.IsValid())
		return false;

	U32 u32SubsystemId = hEntityType.SubsystemId();

	if (u32SubsystemId >= m_u32NumSubsystems || (! m_apSubsystem[u32SubsystemId]))
		return false;

	return m_apSubsystem[u32SubsystemId]->ReleaseEntityType(hEntityType.ArrayHandle());
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ReleaseEntity
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::ReleaseEntity(const AudioEntityHandle& hEntity)
{
	if (! hEntity.IsValid())
	{
		Warning("ReleaseEntity() - Entity invalid, already released?");
		//DebugBreak();
		return false;

	}

	U32 u32SubsystemId = hEntity.SubsystemId();

	if (u32SubsystemId >= m_u32NumSubsystems || (! m_apSubsystem[u32SubsystemId]))
	{
		Error("Subsystem boned");
		DebugBreak();
		return false;
	}

	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntity);

	if (! pInstance)
	{
		Error("No instance to release");
		DebugBreak();
		
		return false;

	}

	U32 u32NumCallbacks = pInstance->GetCallbackRefCount();
	if (u32NumCallbacks)
	{
		m_CallbackLock.Enter();

		U32 u32HighestEntry = m_CallbackDataList.GetHighestNodeIndex();

		m_CallbackLock.Exit();

		for (U32 i = 0; i < u32HighestEntry && pInstance->GetCallbackRefCount(); i++)
		{
			m_CallbackLock.Enter();

			EntityCallbackBase* pCallbackData = m_CallbackDataList.GetByIndex(i);

			m_CallbackLock.Exit();

			if (pCallbackData)
			{
				AudioEntityHandle hComp = hEntity;
				if (pCallbackData->m_hEntity == hComp)
				{
					delete pCallbackData;

					m_CallbackLock.Enter();

					m_CallbackDataList.FreeByIndex(i);

					m_CallbackLock.Exit();

					pInstance->DecrementCallbackRefCount();
				}
			}
		}
	}

	U32 u32NumDataTrackers = pInstance->GetDataTrackerRefCount();
	if (u32NumDataTrackers)
	{
		m_DataTrackerLock.Enter();

		U32 u32HighestEntry = m_DataTracker.GetHighestNodeIndex();

		m_DataTrackerLock.Exit();

		for (U32 i = 0; i < u32HighestEntry && pInstance->GetDataTrackerRefCount(); i++)
		{
			m_DataTrackerLock.Enter();

			EntityDataPollInfo* pDataTracker = m_DataTracker.GetByIndex(i);

			m_DataTrackerLock.Exit();

			if (pDataTracker)
			{
				AudioEntityHandle hComp = hEntity;
				if (pDataTracker->m_hEntity == hComp)
				{
					delete pDataTracker;

					m_DataTrackerLock.Enter();

					m_DataTracker.FreeByIndex(i);

					m_DataTrackerLock.Exit();

					pInstance->DecrementDataTrackerRefCount();
				}
			}
		}
	}

	return m_apSubsystem[u32SubsystemId]->ReleaseEntityInstance(hEntity.ArrayHandle());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SuspendEntity
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::SuspendEntity(Audio::EntityInstance* pInstance)
{
	return pInstance->SetSuspendState(true);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ResumeEntity
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::ResumeEntity(Audio::EntityInstance* pInstance)
{
	return pInstance->SetSuspendState(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GetDefinitionHandle
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityDefinitionHandle Audio::InterfaceController::GetDefinitionHandle(const AudioEntityHandle& hEntity)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntity);

	EntityDefinitionHandle hDef;

	if (! pInstance)
		return hDef;

	return pInstance->GetDefinitionHandle();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SuspendInstance
//
//	suspend an audioobject instance within an entity
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::SuspendInstance(const AudioEntityHandle& hEntity, const EntityDataMemberHandle hInstance)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntity);

	if (! pInstance)
		return false;

	InstanceHandle hInst = pInstance->GetInstance(hInstance);

	if (!hInst.IsValid())
		return false;

	System::Get()->GetObjectManager()->Suspend(hInst, true);

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ResumeInstance
//
//	resume an audioobject instance within an entity
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::ResumeInstance(const AudioEntityHandle& hEntity, const EntityDataMemberHandle hInstance)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntity);

	if (! pInstance)
		return false;

	InstanceHandle hInst = pInstance->GetInstance(hInstance);

	if (!hInst.IsValid())
		return false;

	System::Get()->GetObjectManager()->Suspend(hInst, false);

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	IsSuspended
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::IsInstanceSuspended(const AudioEntityHandle& hEntity, const EntityDataMemberHandle hInstance)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntity);

	if (! pInstance)
		return false;

	InstanceHandle hInst = pInstance->GetInstance(hInstance);

	if (!hInst.IsValid())
		return false;

	return System::Get()->GetObjectManager()->IsSuspended(hInst);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	IsSuspended
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::IsEntitySuspended(const AudioEntityHandle& hEntity)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntity);

	if (! pInstance)
		return false;

	bool bSuspended = pInstance->IsSuspended();

	return bSuspended;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SuspendEntity
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::SuspendEntity(const AudioEntityHandle& hEntity)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntity);

	if (! pInstance)
		return false;

	return pInstance->SetSuspendState(true);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ResumeEntity
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::ResumeEntity(const AudioEntityHandle& hEntity)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntity);

	if (! pInstance)
		return false;

	return pInstance->SetSuspendState(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	MuteEntity
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::MuteEntity(const AudioEntityHandle& hEntity)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntity);

	if (! pInstance)
		return false;

	return pInstance->SetMuteState(true);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	UnMuteEntity
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::UnmuteEntity(const AudioEntityHandle& hEntity)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntity);

	if (! pInstance)
		return false;

	return pInstance->SetMuteState(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LookupObject
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityTypeDataMemberHandle Audio::InterfaceController::LookupObject(const EntityDefinitionHandle hDefinition, const char* strName)
{
	EntityDefinition* pDef = m_EntityManager.GetDefinitionFromHandle(hDefinition);

	if (! pDef)
		return 0xffffffff;

	return pDef->LookupObject(strName);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LookupObjectComponent
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityTypeDataMemberHandle Audio::InterfaceController::LookupObjectComponent(const EntityDefinitionHandle hDefinition, const char* strName)
{
	EntityDefinition* pDef = m_EntityManager.GetDefinitionFromHandle(hDefinition);

	if (! pDef)
		return 0xffffffff;

	return pDef->LookupObjectComponent(strName);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LookupInstance
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityDataMemberHandle Audio::InterfaceController::LookupInstance(const EntityDefinitionHandle hDefinition, const char* strName)
{
	EntityDefinition* pDef = m_EntityManager.GetDefinitionFromHandle(hDefinition);

	if (! pDef)
		return 0xffffffff;

	return pDef->LookupInstance(strName);
}

Audio::EntityDataMemberHandle Audio::InterfaceController::LookupInstance(const EntityDefinitionHandle hDefinition, const StringHash hash )
{
	EntityDefinition* pDef = m_EntityManager.GetDefinitionFromHandle(hDefinition);

	if (! pDef)
		return 0xffffffff;

	return pDef->LookupInstance(hash);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LookupInstanceComponent - by string
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityDataMemberHandle Audio::InterfaceController::LookupInstanceComponent(const EntityDefinitionHandle hDefinition, const char* strName)
{
	EntityDefinition* pDef = m_EntityManager.GetDefinitionFromHandle(hDefinition);

	if (! pDef)
		return 0xffffffff;

	return pDef->LookupInstanceComponent(strName);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LookupInstanceComponent - by string hash
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityDataMemberHandle Audio::InterfaceController::LookupInstanceComponent(const EntityDefinitionHandle hDefinition, const StringHash hash)
{
	EntityDefinition* pDef = m_EntityManager.GetDefinitionFromHandle(hDefinition);

	if (! pDef)
		return 0xffffffff;// why not use a constant?????? What if you forget an f ???

	return pDef->LookupInstanceComponent(hash);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LookupInstanceComponent - instance version
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityDataMemberHandle Audio::InterfaceController::LookupInstanceComponent(const AudioEntityHandle& hEntity, const char* strName)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntity);

	if (! pInstance)
		return false;

	EntityDefinition* pDef = m_EntityManager.GetDefinitionFromHandle(pInstance->GetDefinitionHandle());

	if (! pDef)
		return 0xffffffff;

	return pDef->LookupInstanceComponent(strName);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GetNumParameterisedFunctions
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
S32	Audio::InterfaceController::GetNumParameterisedFunctions(const EntityDefinitionHandle hDefinition, const char* strName)
{
	EntityDefinition* pDef = m_EntityManager.GetDefinitionFromHandle(hDefinition);

	if (! pDef)
		return 0xffffffff;

	return pDef->GetNumParameterisedFunctions(strName);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LookupParameterisedFunctionByIndex
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::LocalFunctionHandle Audio::InterfaceController::LookupParameterisedFunctionByIndex(const EntityDefinitionHandle hDefinition, const char* strName, const S32 sIndex)
{
	EntityDefinition* pDef = m_EntityManager.GetDefinitionFromHandle(hDefinition);

	if (! pDef)
		return 0xffffffff;

	return pDef->GetParameterisedFunctionByIndex(strName, sIndex);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LookupParameterisedFunctionById
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::LocalFunctionHandle Audio::InterfaceController::LookupParameterisedFunctionById(const EntityDefinitionHandle hDefinition, const char* strName, const U32 uParameterId)
{
	EntityDefinition* pDef = m_EntityManager.GetDefinitionFromHandle(hDefinition);

	if (! pDef)
		return 0xffffffff;

	return pDef->GetParameterisedFunctionById(strName, uParameterId);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetCallback
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::PagedArrayHandle Audio::InterfaceController::SetCallback(EntityCallbackBase* pData, const EntityDataMemberHandle hMember)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(pData->m_hEntity);
	Audio::PagedArrayHandle hRet;

	if (! pInstance)
	{
		delete pData;
		return hRet;
	}

	m_CallbackLock.Enter();

	hRet = m_CallbackDataList.AddEntry(pData);

	m_CallbackLock.Exit();

	U32 u32CallbackIndex = hRet.GetValue();

	InstanceComponentHandle hActionOut =  pInstance->GetConnector(hMember);
	System::Get()->GetObjectManager()->RegisterActionOutCallback(hActionOut, StaticProcessCallback, u32CallbackIndex);

	pInstance->IncrementCallbackRefCount();

	return hRet;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	TrackValue
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::ReturnDataHandle Audio::InterfaceController::TrackValue(const AudioEntityHandle& hEntity, EntityDataMemberHandle hEntityMember)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntity);
	Audio::PagedArrayHandle hRet;

	if (! pInstance)
		return hRet;

	EntityDataPollInfo* pData = new EntityDataPollInfo();

	pData->m_hEntity = hEntity;
	pData->m_hEntityMember = hEntityMember;

	pInstance->IncrementDataTrackerRefCount();

	m_DataTrackerLock.Enter();

	hRet = m_DataTracker.AddEntry(pData);

	m_DataTrackerLock.Exit();

	return hRet;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	UntrackValue
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::UntrackValue(ReturnDataHandle& hValue)
{
	m_DataTrackerLock.Enter();

	EntityDataPollInfo* pData = m_DataTracker.GetByHandle(hValue);

	m_DataTrackerLock.Exit();

	if (! pData)
		return false;

	EntityInstance* pInstance = GetEntityInstanceFromHandle(pData->m_hEntity);

	if (! pInstance)
		return false;

	delete pData;

	m_DataTrackerLock.Enter();

	m_DataTracker.Free(hValue);

	m_DataTrackerLock.Exit();

	pInstance->DecrementDataTrackerRefCount();

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GetValue
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::GetValue(F32& f32Dest, ReturnDataHandle& hValue)
{
	m_DataTrackerLock.Enter();

	EntityDataPollInfo* pData = m_DataTracker.GetByHandle(hValue);

	m_DataTrackerLock.Exit();

	if (! pData)
		return false;

	f32Dest = pData->m_f32Value;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DefaultEntityCallback::Callback
//	Forwards the callback message to the subsystem
//	
/////////////////////////////////////////////////////////////////////////////////////////////////////
void Audio::DefaultEntityCallback::Callback()
{
	U32 u32SubsystemId = m_hEntity.SubsystemId();

	if (u32SubsystemId >= Audio::InterfaceController::Get().m_u32NumSubsystems || Audio::InterfaceController::Get().m_apSubsystem[u32SubsystemId] == 0)
	{
		Audio::InterfaceController::Get().Error("Entity handle doesn't belong to a valid subsystem");
		return;
	}

	Audio::InterfaceController::Get().m_apSubsystem[u32SubsystemId]->ProcessCustomMessage(m_u32MessageId, m_hEntity.ArrayHandle().GetValue());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GetCallbackTriggeredCount
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
U32 Audio::InterfaceController::GetCallbackTriggeredCount(PagedArrayHandle hCallback, bool bReset)
{
	m_CallbackLock.Enter();

	EntityCallbackBase* pCallbackData = m_CallbackDataList.GetByHandle(hCallback);

	m_CallbackLock.Exit();

	if (! pCallbackData)
		return 0;

	U32 u32Retvalue = pCallbackData->m_u32CallCount;

	if (bReset)
		pCallbackData->m_u32CallCount = 0;

	return u32Retvalue;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	TriggerEntityActionIn
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::TriggerEntityActionIn(const AudioEntityHandle& hEntity, const EntityDataMemberHandle hEntityMember, const bool resumeOnActionIn)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntity);

	if (! pInstance)
		return false;

	if(resumeOnActionIn)
	{
		if (pInstance->IsSuspended())
			ResumeEntity(hEntity);
	}

	InstanceComponentHandle hComp = pInstance->GetConnector(hEntityMember);

	if (!hComp.IsValid())
		return false;

	System::Get()->Execute(hComp);

	return true;
}

#ifndef CFG_GOLD
#ifndef CFG_SPEED
#define CHECK_VALUES_BUG_26589
#endif
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetEntityVectorIn
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::SetEntityVectorIn(const AudioEntityHandle& hEntity, const EntityDataMemberHandle hEntityMember, F32* afVec)
{
	#ifdef CHECK_VALUES_BUG_26589
	if (!::Maths::Finite(afVec[0]))
		return false;
		
	if (!::Maths::Finite(afVec[1]))
		return false;
		
	if (!::Maths::Finite(afVec[2]))
		return false;
	#endif

	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntity);

	if (! pInstance)
		return false;

	InstanceComponentHandle hComp = pInstance->GetConnector(hEntityMember);

	if (!hComp.IsValid())
		return false;

	System::Get()->SetDimensions(hComp, afVec);

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetEntityValue
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::SetEntityValue(const AudioEntityHandle& hEntity, const EntityDataMemberHandle hEntityMember, F32 f32Value)
{
	#ifdef CHECK_VALUES_BUG_26589
	if (!::Maths::Finite(f32Value))
		return false;
	#endif

	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntity);

	if (! pInstance)
		return false;

	InstanceComponentHandle hComp = pInstance->GetConnector(hEntityMember);

	if (!hComp.IsValid())
		return false;

	System::Get()->SetValue(hComp, f32Value);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GetEntityValue
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
//F32 Audio::InterfaceController::GetEntityValue(const AudioEntityHandle& hEntity, const EntityDataMemberHandle hMember, bool bReportErrors)
//{
//	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntity);
//
//	if (! pInstance)
//	{
//		if (bReportErrors)
//			Warning("Couldn't get entity value");
//
//		return 0.f;
//	}
//
//	EntityDefinition* pDef = GetDefinition(pInstance);
//
//	if (! pDef)
//	{
//		if (bReportErrors)
//			Warning("Couldn't get entity value");
//
//		return 0.f;
//	}
//	
//	if ((S32)hMember == -1)
//	{
//		if (bReportErrors)
//			Warning("Couldn't get entity value");
//
//		return 0.f;
//	}
//
//	S32 s32ParamOutIndex = pDef->GetParameterOutIndex(hMember);
//
//	if (s32ParamOutIndex == -1)
//	{
//		if (bReportErrors)
//			Warning("Couldn't get entity value");
//
//		return 0.f;
//	}
//
//	return pInstance->GetOutputValue(s32ParamOutIndex);
//}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	IsDataMemberValid
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::IsDataMemberValid(const AudioEntityHandle& hEntity, const EntityDataMemberHandle hMember)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntity);

	if (! pInstance)
		return false;

	InstanceComponentHandle hComp = pInstance->GetConnector(hMember);

	if (!hComp.IsValid())
		return false;

	return System::Get()->GetObjectManager()->IsInstanceComponentValid(hComp);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ReloadSubsystems
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::ReloadSubsystems(U32 u32SubsystemFlags)
{
	// TODO: this!
	return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ConnectEntities
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::ConnectEntities(const AudioEntityHandle& hUpstreamEntity, const AudioEntityHandle& hDownstreamEntity)
{
	return RunFunctionInParameterisedEntityInstance(hUpstreamEntity, hDownstreamEntity, "DoConnections");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	RunFunctionInEntityType
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::RunFunctionInEntityType(const AudioEntityTypeHandle& hEntityType, const char* strFunctionName)
{
	EntityType* pType = GetEntityTypeFromHandle(hEntityType);

	if (! pType)
		return false;

	EntityDefinition* pDef = m_EntityManager.GetDefinitionFromHandle(pType->GetDefinitionHandle());

	if (! pDef)
		return false;

	CommonFunctionHandle hFunction = pDef->GetCommonFunction(strFunctionName);

	bool bSuccess = pDef->RunEntityTypeFunction(hFunction, pType);

	if (bSuccess)
	{
		FunctionExecutionHistoryEntry entry;

		entry.hEntity = hEntityType;
		entry.hFunction = hFunction;
		entry.eFunctionType = eEntityTypeFunction;

		pType->PushFunctionExecutionEntry(entry);
	}

	return bSuccess;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	RunFunctionInEntityInstance
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::RunFunctionInEntityInstance(const AudioEntityHandle& hEntityInstance, const char* strFunctionName)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntityInstance);

	if (! pInstance)
		return false;

	EntityDefinition* pDef = m_EntityManager.GetDefinitionFromHandle(pInstance->GetDefinitionHandle());

	if (! pDef)
		return false;

	LocalFunctionHandle hFunction = pDef->GetLocalFunction(strFunctionName);

	bool bSuccess = pDef->RunEntityInstanceFunction(hFunction, pInstance);

	if (bSuccess)
	{
		FunctionExecutionHistoryEntry entry;

		entry.hEntity = hEntityInstance;
		entry.hFunction = hFunction;
		entry.eFunctionType = eEntityInstanceFunction;

		pInstance->PushFunctionExecutionEntry(entry);
	}

	return bSuccess;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	RunFunctionInParameterisedEntityInstance - generic internal version. can pass either str or handle
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::RunFunctionInParameterisedEntityInstance(const AudioEntityHandle& hEntityInstance, const AudioEntityHandle& hParameterEntityInstance, const char* strFunctionName, const LocalFunctionHandle hFuncIn)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntityInstance);
	EntityInstance* pParameterInstance = GetEntityInstanceFromHandle(hParameterEntityInstance);
	if (! (pInstance && pParameterInstance))
		return false;

	EntityDefinition* pDef = m_EntityManager.GetDefinitionFromHandle(pInstance->GetDefinitionHandle());
	EntityDefinition* pParameterDef = m_EntityManager.GetDefinitionFromHandle(pParameterInstance->GetDefinitionHandle());

	if (! pDef || !pParameterDef)
		return false;

	LocalFunctionHandle hFunction;
	if (strFunctionName)
		hFunction = pDef->GetParameterisedFunctionById(strFunctionName, pParameterDef->GetId());
	else
		 hFunction = hFuncIn;

	bool bSuccess = pDef->RunEntityInstanceParameterisedFunction(hFunction, pInstance, pParameterInstance);

	if (bSuccess)
	{
		FunctionExecutionHistoryEntry entry;

		entry.hEntity = hEntityInstance;
		entry.hParameterEntity = hParameterEntityInstance;
		entry.hFunction = hFunction;
		entry.eFunctionType = eParameterisedEntityInstanceFunction;

		pInstance->PushFunctionExecutionEntry(entry);

		pParameterInstance->PushFunctionExecutionEntry(entry);
	}

	return bSuccess;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	RunFunctionInParameterisedEntityInstance - string version
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::RunFunctionInParameterisedEntityInstance(const AudioEntityHandle& hEntityInstance, const AudioEntityHandle& hParameterEntityInstance, const char* strFunctionName)
{
	return RunFunctionInParameterisedEntityInstance(hEntityInstance, hParameterEntityInstance, strFunctionName, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	RunFunctionInParameterisedEntityInstance - function handle version
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::RunFunctionInParameterisedEntityInstance(const AudioEntityHandle& hEntityInstance, const AudioEntityHandle& hParameterEntityInstance, const LocalFunctionHandle hFunction)
{
	return RunFunctionInParameterisedEntityInstance(hEntityInstance, hParameterEntityInstance, 0, hFunction);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetMaster
//	generic version. can pass mono, stereo, surround and should work
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::SetMaster(const AudioEntityHandle& hEntityInstance, const char* strOutput)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntityInstance);

	if (! pInstance)
	{
		Warning("SetMaster failed");
		return false;
	}

	EntityDefinition* pDef = GetDefinition(pInstance);

	if (! pDef)
	{
		Warning("SetMaster failed");
		return false;
	}

	U32 u32OutputIndex = pDef->LookupInstanceComponent(strOutput);

	if (u32OutputIndex == 0xffffffff)
	{
		Warning("SetMaster failed");
		return false;
	}

	InstanceComponentHandle hOutput = pInstance->GetConnector(u32OutputIndex);

	System::Get()->SetMaster(hOutput);

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetMonoMaster
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::SetMonoMaster(const AudioEntityHandle& hEntityInstance, const char* strOutput)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntityInstance);

	if (! pInstance)
	{
		Warning("SetMonoMaster failed");
		return false;
	}

	EntityDefinition* pDef = GetDefinition(pInstance);

	if (! pDef)
	{
		Warning("SetMonoMaster failed");
		return false;
	}

	U32 u32OutputIndex = pDef->LookupInstanceComponent(strOutput);

	if (u32OutputIndex == 0xffffffff)
	{
		Warning("SetMonoMaster failed");
		return false;
	}

	InstanceComponentHandle hOutput = pInstance->GetConnector(u32OutputIndex);

	System::Get()->SetMonoMaster(hOutput);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetStereoMaster
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::SetStereoMaster(const AudioEntityHandle& hEntityInstance, const char* strOutput)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntityInstance);

	if (! pInstance)
	{
		Warning("SetStereoMaster failed");
		return false;
	}

	EntityDefinition* pDef = GetDefinition(pInstance);

	if (! pDef)
	{
		Warning("SetStereoMaster failed");
		return false;
	}

	U32 u32OutputIndex = pDef->LookupInstanceComponent(strOutput);

	if (u32OutputIndex == 0xffffffff)
	{
		Warning("SetStereoMaster failed");
		return false;
	}

	InstanceComponentHandle hOutput = pInstance->GetConnector(u32OutputIndex);

	System::Get()->SetStereoMaster(hOutput);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetSurroundMaster
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::SetSurroundMaster(const AudioEntityHandle& hEntityInstance, const char* strOutput)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntityInstance);

	if (! pInstance)
	{
		Warning("SetSurroundMaster failed");
		return false;
	}

	EntityDefinition* pDef = GetDefinition(pInstance);

	if (! pDef)
	{
		Warning("SetSurroundMaster failed");
		return false;
	}

	U32 u32OutputIndex = pDef->LookupInstanceComponent(strOutput);

	if (u32OutputIndex == 0xffffffff)
	{
		Warning("SetSurroundMaster failed");
		return false;
	}

	InstanceComponentHandle hOutput = pInstance->GetConnector(u32OutputIndex);

	System::Get()->SetSurroundMaster(hOutput);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetSurroundLFEMaster
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::SetSurroundLFEMaster(const AudioEntityHandle& hEntityInstance, const char* strOutput)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntityInstance);

	if (! pInstance)
	{
		Warning("SetSurroundLFEMaster failed");
		return false;
	}

	EntityDefinition* pDef = GetDefinition(pInstance);

	if (! pDef)
	{
		Warning("SetSurroundLFEMaster failed");
		return false;
	}

	U32 u32OutputIndex = pDef->LookupInstanceComponent(strOutput);

	if (u32OutputIndex == 0xffffffff)
	{
		Warning("SetSurroundLFEMaster failed");
		return false;
	}

	InstanceComponentHandle hOutput = pInstance->GetConnector(u32OutputIndex);

	System::Get()->SetSurroundLFEMaster(hOutput);

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetCustomDataPtr
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::SetCustomDataPtr(const AudioEntityHandle& hEntityInstance, void* pData)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntityInstance);

	if (! pInstance)
	{
		Warning("SetCustomDataPtr failed");
		return false;
	}

	pInstance->SetCustomDataPtr(pData);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GetCustomDataPtr
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
const void* Audio::InterfaceController::GetCustomDataPtr(const AudioEntityHandle& hEntityInstance)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntityInstance);

	if (! pInstance)
	{
		Warning("GetCustomDataPtr failed");
		return 0;
	}

	return pInstance->GetCustomDataPtr();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetCustomDataFreed
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Audio::InterfaceController::SetCustomDataFreed(const AudioEntityHandle& hEntityInstance)
{
	EntityInstance* pInstance = GetEntityInstanceFromHandle(hEntityInstance);

	if (! pInstance)
	{
		Warning("SetCustomDataFreed failed");
		return false;
	}

	pInstance->SetCustomDataFreed();
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GetEntityTypeFromHandle
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityType* Audio::InterfaceController::GetEntityTypeFromHandle(const AudioEntityTypeHandle hType)
{
	U32 u32SubsystemId = hType.SubsystemId();

	if (u32SubsystemId >= m_u32NumSubsystems || (! m_apSubsystem[u32SubsystemId]))
		return 0;

	return m_apSubsystem[u32SubsystemId]->GetEntityType(hType.ArrayHandle());
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GetEntityInstanceFromHandle
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::EntityInstance* Audio::InterfaceController::GetEntityInstanceFromHandle(const AudioEntityHandle hInstance)
{
	U32 u32SubsystemId = hInstance.SubsystemId();

	Assert (u32SubsystemId < m_u32NumSubsystems && (m_apSubsystem[u32SubsystemId]));

	return m_apSubsystem[u32SubsystemId]->GetEntityInstance(hInstance.ArrayHandle());
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	StaticProcessCallback (static)
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
void Audio::InterfaceController::StaticProcessCallback(U32 u32Data)
{
	InterfaceController& controller = Get();

	controller.ProcessCallback(u32Data);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ProcessCallback
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
void Audio::InterfaceController::ProcessCallback(U32 u32Data)
{
	m_CallbackLock.Enter();

	EntityCallbackBase* pCallbackData = m_CallbackDataList.GetByHandle(u32Data);

	m_CallbackLock.Exit();

	if (pCallbackData)
	{
		pCallbackData->m_u32CallCount++;

		// Note: callbacks can used to release entities- which causes any callback objects relating to the entity to be destroyed-
		// which means this callback pointer might not be valid any more after we return from the callback
		pCallbackData->Callback();
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Update
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
void Audio::InterfaceController::Update()
{
	for (U32 i = 0; i < m_u32NumSubsystems; i++)
	{
		if (m_apSubsystem[i])
			m_apSubsystem[i]->SystemUpdate();
		else
			Error("Missing subsystem in Audio Interface Controller!");
	}

#ifndef CFG_GOLD
	if (g_bReloadAllAudio)
	{
		g_bReloadAllAudio = false;
		ReloadAllAudio();
	}
#endif

	m_DataTrackerLock.Enter();

	U32 u32HighestEntry = m_DataTracker.GetHighestNodeIndex();

	m_DataTrackerLock.Exit();

	for (U32 i = 0; i < u32HighestEntry; i++)
	{
		m_DataTrackerLock.Enter();

		EntityDataPollInfo* pInfo = m_DataTracker.GetByIndex(i);

		m_DataTrackerLock.Exit();

		if (pInfo)
		{
			EntityInstance* pInstance = GetEntityInstanceFromHandle(pInfo->m_hEntity);

			if (pInstance)
			{
				InstanceComponentHandle hComp = pInstance->GetConnector(pInfo->m_hEntityMember);

				if (hComp.IsValid())
				{
					m_DataTrackerLock.Enter();

					U32 u32Tracker = m_DataTracker.GetHandleFromIndex(i).GetValue();

					m_DataTrackerLock.Exit();

					System::Get()->Value(hComp, u32Tracker, SetParameterCallback);
				}
			}
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Assert
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
void Audio::InterfaceController::InternalAssert(bool bExp, const char* strMessage)
{
	if (! bExp)
	{
		if (strMessage)
			LOGERROR(InterfaceLayerLogger::g_Logger, strMessage);
		else
			LOGERROR(InterfaceLayerLogger::g_Logger, "Audio interface layer exception triggered");
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Error
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
void Audio::InterfaceController::Error(const char* strMessage)
{
	LOGERROR(InterfaceLayerLogger::g_Logger, strMessage);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Warning
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
void Audio::InterfaceController::Warning(const char* strMessage)
{
	LOGWARN(InterfaceLayerLogger::g_Logger, strMessage);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Info
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
void Audio::InterfaceController::Info(const char* strMessage)
{
	LOGINFO(InterfaceLayerLogger::g_Logger, strMessage);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	FATAL
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
void Audio::InterfaceController::Fatal(const char* strMessage)
{
	LOGFATAL(InterfaceLayerLogger::g_Logger, strMessage);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GetParentSystem
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::Subsystem* Audio::InterfaceController::GetParentSubsystem(const AudioEntityHandle& hEntity)
{
	U32 u32SubsystemId = hEntity.SubsystemId();

	if (u32SubsystemId >= m_u32NumSubsystems || m_apSubsystem[u32SubsystemId] == 0)
	{
		Error("Entity handle doesn't belong to a valid subsystem");
		return 0;
	}

	return m_apSubsystem[u32SubsystemId];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetSystemSpeakerDistance
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
void	Audio::InterfaceController::SetSystemSpeakerDistance( U32 eOption, F32 fDistance )
{
	System::Get()->SetSpeakerDistance( (Audio::ISystem::SpeakerOption) eOption, fDistance);
}

#ifdef PLATFORM_PC
/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetSystemOutputSampleRate
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
// [Blur2PC] T144: retail PC 0x00535ab0
void	Audio::InterfaceController::SetSystemOutputSampleRate( U32 uSampleRate )
{
	System::Get()->SetOutputSampleRate( uSampleRate );
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetSystemSpeakerAngles
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
void	Audio::InterfaceController::SetSystemSpeakerAngles( U32 eMode, F32 fFrontLeft, F32 fFrontRight, F32 fSurroundLeft, F32 fSurroundRight, F32 fBackLeft, F32 fBackRight )
{
	System::Get()->SetSpeakerAngles( (Audio::ISystem::SpeakerConfig) eMode,  fFrontLeft,  fFrontRight,  fSurroundLeft ,  fSurroundRight, fBackLeft,fBackRight);
	
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetSystemSpeakerOptions
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
void	Audio::InterfaceController::SetSystemSpeakerOption( U32 eOption )
{

	System::Get()->SetSpeakerOption((Audio::ISystem::SpeakerOption) eOption);	
}

