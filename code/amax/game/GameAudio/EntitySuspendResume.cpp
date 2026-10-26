//#include "nightwing_audio_pch.h"
#include "stdafx.h"

#include "GameAudioLayer.h"

#include "EntitySuspendResume.h"

//#include "GameActors/Components/Messages/AudioMessages.h"



#include <GameApp.h>
#include <Game/Game.h>
#include <Game/GameWorld/GameWorld.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceLogicLayer.h>
//#include "../GameComponents.h"
//#include "../AudioComponents.h"

#include "actor_system/Actor.h"

#include "Game/GameActors/Components/Messages/AudioMessages.h"

//extern Debug::Bool g_bAudioLogActionTriggers;

Debug::Bool g_bAudioSuspendEntities("Audio.Performance.SuspendEntities", true);
Debug::Bool g_bAudioLogSuspendingAndResuming("Audio.ActorAudioComponent.LogSuspendingAndResuming", false);

namespace EntityResumerLogger {
Debug::Logger &g_Log = Debug::GetLogger("audio.entitysuspender");
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize("", off)
#endif

//namespace Audio
//{

//-----------------------------------------------------------------------------
// EntitySuspendResumer()
//-----------------------------------------------------------------------------
bool C_EntitySuspenderResumer::Suspend(Audio::EntityDataMemberHandle hCom)
{
	bool bOk = true;
	
	if (!g_bAudioSuspendEntities)
		return true;

	bOk = Audio::InterfaceController::Get().SuspendEntity(GetEntity());

	if (bOk)
		SetState(E_EntityAudioState_Finished);
	else
		LOGWARN(EntityResumerLogger::g_Log, "Could not suspend entity");

	return bOk;
}

bool C_EntitySuspenderResumer::Resume(Audio::EntityDataMemberHandle hCom)
{
	bool bOk = Audio::InterfaceController::Get().ResumeEntity(GetEntity());
	
	if (bOk)
		SetState(E_EntityAudioState_Playing);
	else
		LOGWARN(EntityResumerLogger::g_Log, "Could not resume entity");
	
	return bOk;
}

void C_EntitySuspenderResumer::SetCallbacks(const Actor::C_ActorHandle& hNotifyActor)
{
	Audio::EntityDataMemberHandle hActionOut = Audio::InterfaceController::Get().LookupInstanceComponent(GetEntity(), "AudioStopped");

	if (hActionOut != I_AudioEntityDataSource::uInvalidDataMemberHandle)
	{
		C_AudioEntityCallback* pCallback = new C_AudioEntityCallback(GetEntity(), hNotifyActor);
		m_hAudioStoppedCallback = Audio::InterfaceController::Get().SetCallback(pCallback, hActionOut);
	}
}

void C_EntitySuspenderResumer::Init(const Actor::C_ActorHandle& hActor, Audio::EntityDefinitionHandle hDef, AudioEntityHandle hEntity)
{
	m_hEntityDef = hDef;
	m_hEntity =hEntity;

	SetCallbacks(hActor);
}

//-----------------------------------------------------------------------------
// C_InstanceSuspendResumer()
//-----------------------------------------------------------------------------
void C_InstanceSuspenderResumer::Init(const Actor::C_ActorHandle& hActor, Audio::EntityDefinitionHandle hDef, AudioEntityHandle hEntity)
{
	m_hEntityDef = hDef;
	m_hEntity =hEntity;

	U32 uNum = GetNumInstances();
	for (U32 uId = 0; uId < uNum; uId++)
	{
		// look up action outs
		m_ahActionOutHandle.PushBack(Audio::InterfaceController::Get().LookupInstanceComponent(GetEntityDef(), GetActionOutString(uId)));

		// look up instances
		m_ahInstanceHandle.PushBack(Audio::InterfaceController::Get().LookupInstance(GetEntityDef(), GetInstanceString( uId )));
	}

	const U32 numDataMembers = GetDataSource()->GetNumDataMembers();
	for (U32 uId = 0; uId < numDataMembers; uId++)
	{
		Audio::EntityDataMemberHandle hComponent = GetDataSource()->GetDataMemberHandle(uId);

		// add to map so we can lookup the id from the handle. we need this because some actor messages only send a handle
		m_ComIdLookupMap.Insert(hComponent, uId);

		#ifndef CFG_GOLD
		Audio::EntityInstance* pInstance = Audio::InterfaceController::Get().GetEntityInstanceFromHandle(GetEntity());
		if (pInstance)
		{
			Audio::InstanceComponentHandle hInstCom =  pInstance->GetConnector(hComponent);

		//	if (g_bAudioLogActionTriggers)
			//	LOGINFO(EntityResumerLogger::g_Log, "ComponentId: %d hMember 0x%x = '%s', hInst 0x%x, hCom 0x%x", uId, hComponent, GetDataSource()->GetDataMemberName( uId ), hInstCom.GetInstanceHandle().Value(), hInstCom.GetObjectComponentHandle().Value());
		}
		#endif
	}

	SetCallbacks(hActor);
}

C_InstanceSuspenderResumer::C_InstanceSuspenderResumer(const I_AudioEntityDataSource* pDataSource)
	: m_pDataSource(pDataSource)
{
}

//-----------------------------------------------------------------------------
// C_InstanceSuspendResumer()
//-----------------------------------------------------------------------------
Audio::PagedArrayHandle C_InstanceSuspenderResumer::SubscribeToCallback(const Audio::EntityDataMemberHandle& hActionOut, U32 uInstanceId, const Actor::C_ActorHandle& hActor)
{
	Audio::PagedArrayHandle hRet;

	if (uInstanceId == uInvalidInstanceId)
		return hRet;

	if (hActionOut == I_AudioEntityDataSource::uInvalidDataMemberHandle)
		return hRet;

	C_AudioInstanceCallback* pCallback = new C_AudioInstanceCallback(GetEntity(), hActor, uInstanceId);
	hRet = Audio::InterfaceController::Get().SetCallback(pCallback, hActionOut);

	return hRet;
}

void C_InstanceSuspenderResumer::SetCallbacks(const Actor::C_ActorHandle& hActor)
{
	for (U32 i=0; i<GetNumInstances(); i++)
	{
		//Audio::EntityDataMemberHandle hActionOut = GetEntityHandler()->GetActionOutComponentId(i);
		//if (pInst->actionOutId != uActionOutNone)
		{
			//Audio::EntityDataMemberHandle hActionOut = GetActionOutHandle(i);
			Audio::PagedArrayHandle hCallback = SubscribeToCallback(GetActionOutHandle(i), i, hActor);
	
			m_ahActionOutCallback.PushBack(hCallback);

			if (g_bAudioLogSuspendingAndResuming)
			{
				if (!hCallback.IsValid())
					LOGINFO(EntityResumerLogger::g_Log, "Could not set actionout callback for instance %d '%s'", i, GetInstanceString(i));
				else
					LOGINFO(EntityResumerLogger::g_Log, "Set callback for instance %d '%s'ok", i, GetInstanceString(i));
			}
		}
	}
}

bool C_InstanceSuspenderResumer::IsCallbackSet() const
{
	// TODO
	return true;
}

bool C_InstanceSuspenderResumer::IsInstancePlaying(U32 uInstanceId) const
{
	Btl::Map<U32, bool>::ConstIterator it = m_InstancesPlaying.Find(uInstanceId);
	if (it == m_InstancesPlaying.End())
		return false;
	else
		return true;
}

void C_InstanceSuspenderResumer::SetInstancePlaying(U32 uInstanceId, bool bPlaying)
{
	Btl::Map<U32, bool>::Iterator it = m_InstancesPlaying.Find(uInstanceId);

	if (bPlaying)
	{
		if (it == m_InstancesPlaying.End())
			m_InstancesPlaying.Insert(uInstanceId, true);
	}
	else
	{
		if (it == m_InstancesPlaying.End())
			LOGWARN(EntityResumerLogger::g_Log, "Setting instance %d to state Stopped but was not playing", uInstanceId);
		else
			m_InstancesPlaying.Erase(it);
	}
}

bool C_InstanceSuspenderResumer::Suspend(const Audio::EntityDataMemberHandle hCom)
{
	if (hCom == uSuspendAll)
	{
		SuspendAll();
		return true;
	}

	return false;
}

void C_InstanceSuspenderResumer::SuspendAll()
{
	if (!g_bAudioSuspendEntities)
		return;
	
	if (g_bAudioLogSuspendingAndResuming)
		LOGINFO(EntityResumerLogger::g_Log, "SuspendInstance() : Suspending all instances");

	for (U32 i=0; i<GetNumInstances(); i++)
	{
		Audio::InterfaceController::Get().SuspendInstance(GetEntity(), GetInstanceHandle(i));
	}
}

void C_InstanceSuspenderResumer::SuspendInstance(U32 uInstanceId)
{
	if (!g_bAudioSuspendEntities)
		return;
		
	if (uInstanceId != uInvalidInstanceId)
	{
		if (IsInstancePlaying(uInstanceId))
		{
			if (g_bAudioLogSuspendingAndResuming && IsInstancePlaying(uInstanceId))
				LOGINFO(EntityResumerLogger::g_Log, "SuspendInstance() : Suspending instance '%s' %d", GetInstanceString(uInstanceId), uInstanceId);
			
			if (Audio::InterfaceController::Get().SuspendInstance(GetEntity(), GetInstanceHandle(uInstanceId)))
			{
				SetInstancePlaying(uInstanceId, false);
			}
		}
	}
}

void C_InstanceSuspenderResumer::ResumeInstance(U32 uInstanceId)
{
	 if (uInstanceId != uInvalidInstanceId)
	 {
		if (!IsInstancePlaying(uInstanceId))
		{
			if (g_bAudioLogSuspendingAndResuming)
				LOGINFO(EntityResumerLogger::g_Log, "ResumeInstance() : Resuming instance '%s' %d", GetInstanceString(uInstanceId), uInstanceId);

			if (Audio::InterfaceController::Get().ResumeInstance(GetEntity(), GetInstanceHandle(uInstanceId)))
			{
				SetInstancePlaying(uInstanceId, true);
			}
		}
	 }
}

U32 C_InstanceSuspenderResumer::LookupComponentId(const Audio::EntityDataMemberHandle hCom) const
{
	if (hCom == I_AudioEntityDataSource::uInvalidDataMemberHandle)
		return I_AudioEntityDataSource::uInvalidDataMemberId;

	Btl::Map<Audio::EntityDataMemberHandle, U32>::ConstIterator it = m_ComIdLookupMap.Find(hCom);
	if (it == m_ComIdLookupMap.End())
		return I_AudioEntityDataSource::uInvalidDataMemberId;

	return it->GetValue();
}

E_EntityAudioState C_InstanceSuspenderResumer::GetState() const
{
	if (!InstancePlayingCount())
		return E_EntityAudioState_Finished;
	else
		return E_EntityAudioState_Playing;	
}


void C_AudioEntityCallback::Callback()
{
	// inform the actor which started the sound it has finished
	if (m_hActor)
	{
		if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList())
		//if(C_GameWorld::GetInstance() && C_GameWorld::GetInstance()->GetActorList())	
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->SendMessageToActor(m_hActor, ActorMsg::E_AudioMsg_EntityAudioStopped);
	}
}

void C_AudioInstanceCallback::Callback()
{
	if (m_hActor)
	{
		ActorMsg::C_InstanceAudioStoppedMsg msg;
		msg.m_sInstanceId = m_uInstanceId;
		
		//if(C_GameWorld::GetInstance() && C_GameWorld::GetInstance()->GetActorList())	
			if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList())				
				C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->SendMessageToActorComponent(m_hActor, Components::E_IF_AudioEntity, &msg);
	}
}

//}	// namespace Components
#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize("", on)
#endif

