#include "SharedActorsPch.h"

#include <shared_actors/actorimpl/GroupActors.h>

#include <actor_system/ActorDebug.h>
#include <actor_system/ActorList.h>
#include <actor_system/ActorMessages.h>
#include <core/BizAssert.h>
#include <shared_actors/actormessages/GroupActorMessages.h>
#include <shared_actors/components/interfaces/I_Trigger.h>
#include <shared_actors/XtActorCreation.h>
#include <system/serialise/btl.h>


namespace Actors
{

C_GroupActor::C_GroupActor() : 
	C_SharedActorBase(), 
	m_Members(), 
	m_bEnableMessages(true)
{
#ifndef CFG_GOLD
	m_DebugLockCount = 0;
#endif
}

C_GroupActor::~C_GroupActor()
{
}

bool C_GroupActor::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch (pMessage->Type())
	{
	case Actor::E_ActorMsg_Kill:
		ForwardMessage(pMessage);
		return C_SharedActorBase::OnMessage(pMessage);

	case ActorMsg::E_GroupMsg_AddActor:
		{
			AddActor(Messaging::MessageCast<ActorMsg::C_GroupAddActorMsg>(pMessage)->GetActorHandle());
		}
		return true;

	case ActorMsg::E_GroupMsg_RemoveActor:
		{
			RemoveActor(Messaging::MessageCast<ActorMsg::C_GroupRemoveActorMsg>(pMessage)->GetActorHandle());
		}
		return true;

	case ActorMsg::E_GroupMsg_Inherit:
		{
			const ActorMsg::C_GroupInheritMsg* pMsg = Messaging::MessageCast<ActorMsg::C_GroupInheritMsg>(pMessage);
			if ((FindIndex(pMsg->GetActorHandle()) >= 0) && (FindIndex(pMsg->GetValue()) < 0))
			{
#ifndef CFG_GOLD
				BIZ_ASSERT(m_DebugLockCount == 0);
#endif
				m_Members.PushBack(pMsg->GetValue());
			}
		}
		return true;

	case ActorMsg::E_GroupMsg_EnableMessages:
		m_bEnableMessages = true;
		return true;

	case ActorMsg::E_GroupMsg_DisableMessages:
		m_bEnableMessages = false;
		return true;

	case ActorMsg::E_GroupMsg_DeleteActors:
		ForwardMessage(Actor::E_ActorMsg_Kill);	
		return true;

	case ActorMsg::E_GroupMsg_SetForwardingVolume:
		m_ForwardVolume = Messaging::MessageCast<ActorMsg::C_GroupSetForwardingVolumeMsg>(pMessage)->GetActorHandle();
		return true;

	default:
		if(GetActorList()->ActorExists(m_ForwardVolume))
			ForwardMessageToVolume(pMessage, m_ForwardVolume);
		else
			ForwardMessage(pMessage);
		return true;
	}
}

bool C_GroupActor::AddActor(Actor::C_ActorHandle actorHandle)
{
	const S32 index = FindIndex(actorHandle);
	const Actor::C_Actor* pActor = GetActorList()->GetActorPtr(actorHandle);

	if (pActor == NULL)
	{
		LOGWARN(Actor::GetDebugLogLevelDesign(this), DBSTR "failed to add NULL actor to group %s", GetActorName());
	}
	else if (index < 0)
	{
#ifndef CFG_GOLD
		BIZ_ASSERT(m_DebugLockCount == 0);
#endif
		m_Members.PushBack(actorHandle);
	}
#ifndef CFG_GOLD
	else
	{
		LOGWARN(Actor::GetDebugLogLevelDesign(this), DBSTR "handling E_GroupMsg_AddActor : actor %s is already in group %s", pActor->GetActorName(), GetActorName());
	}
#endif

	return true;
}

bool C_GroupActor::RemoveActor(Actor::C_ActorHandle actorHandle)
{
	const S32 index = FindIndex(actorHandle);

	if (index >= 0)
	{
#ifndef CFG_GOLD
		BIZ_ASSERT(m_DebugLockCount == 0);
#endif
		m_Members.Erase(m_Members.Begin() + index);
	}

	return true;
}

bool C_GroupActor::ContainsActor(const Actor::C_ActorHandle& actorHandle) const
{
	return (FindIndex(actorHandle) >= 0);
}

void C_GroupActor::AddValidActors(const C_LevelActorCreationList& levelActorCreationList)
{
	const Actor::C_ActorList* pActorList = GetActorList();

	for(S32 actorNo = 0; actorNo < levelActorCreationList.m_LevelActors.count; actorNo++)
	{
		const Actors::C_GameActorCreate* pActorCreate = levelActorCreationList.m_LevelActors[actorNo];

		if((pActorCreate != NULL) && !pActorCreate->m_IsCommented)
		{
			const Actor::C_ActorHandle handle = pActorCreate->GetActorHandle();

			if (pActorList->ActorExists(handle))
			{
				AddActor(handle);
			}
		}
	}
}

void C_GroupActor::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_Members;
	stream << m_bEnableMessages;
	stream << m_ForwardVolume;
}

void C_GroupActor::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_Members;
	stream >> m_bEnableMessages;
	stream >> m_ForwardVolume;
}

void C_GroupActor::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	Actor::C_Actor::SerialiseForSave(stream);
	Serialise(stream);
}

void C_GroupActor::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	Actor::C_Actor::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_GroupActor::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	Actor::C_Actor::SerialiseForSync(stream);
	Serialise(stream);
}

void C_GroupActor::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	Actor::C_Actor::DeserialiseForSync(stream);
	Deserialise(stream);
}

bool C_GroupActor::CreateImpl()
{
	if (!Actor::C_Actor::CreateImpl())
		return false;

	const C_GroupActorCreate* pCreationData = GetCreationData();

	AddSubscriber(ActorMsg::E_GroupMsg_Inherit);

	const U32 numMembers = pCreationData->GetNumMembers();
	m_Members.Reserve(m_Members.Size() + numMembers);

	for (U32 i = 0; i < numMembers; ++i)
	{
		AddActor(pCreationData->GetMember(i));
	}

	return true;
}

S32 C_GroupActor::FindIndex(const Actor::C_ActorHandle& handle) const
{
	for (S32 i = 0; i < m_Members.Size(); ++i)
		if (m_Members[i] == handle)
			return i;

	return -1;
}

void C_GroupActor::ForwardMessage(const Messaging::C_MessageBase* pMessage)
{
	if (m_bEnableMessages)
	{
#ifndef CFG_GOLD
		++m_DebugLockCount;
#endif
		Actor::C_ActorList* pActorList = GetActorList();

		for (C_Members::ConstIterator it = m_Members.Begin(); it != m_Members.End(); ++it)
		{
			const Actor::C_Actor *pActor = pActorList->GetActorPtr(*it);
			if(pActor && pActor->GetLifetimeState() == ActorSys::E_LifetimeState_Created)
				pActorList->SendMessageToActor(*it, pMessage);
		}
#ifndef CFG_GOLD
		--m_DebugLockCount;
#endif
	}
}

void C_GroupActor::ForwardMessageToVolume(const Messaging::C_MessageBase* pMessage, Actor::C_ActorHandle volumeActor)
{

	if (m_bEnableMessages)
	{
#ifndef CFG_GOLD
		++m_DebugLockCount;
#endif
		Actor::C_ActorList* pActorList = GetActorList();
		const Actor::C_Actor *pVolumeActor = pActorList->GetActorPtr(volumeActor);
		const Components::I_VolumeTrigger * pVolumeIF = pVolumeActor ? (const Components::I_VolumeTrigger *)pVolumeActor->QueryInterface(Components::E_TriggerIF_Volume) : NULL;

		if(pVolumeIF == NULL)	// No volume - return
			return;

		for (C_Members::ConstIterator it = m_Members.Begin(); it != m_Members.End(); ++it)
		{
			const Actor::C_Actor *pActor = pActorList->GetActorPtr(*it);

			if(pActor && pActor->GetLifetimeState() == ActorSys::E_LifetimeState_Created)
			{
				if(pVolumeIF->IsInside(pActor->GetPosition()))		// Check if actor is in volume
					pActorList->SendMessageToActor(*it, pMessage);
			}
		}
#ifndef CFG_GOLD
		--m_DebugLockCount;
#endif
	}
}

void C_GroupActor::ForwardMessage(Messaging::TMsgType msgType)
{
	if (m_bEnableMessages)
	{
#ifndef CFG_GOLD
		++m_DebugLockCount;
#endif
		Actor::C_ActorList* pActorList = GetActorList();

		for (C_Members::ConstIterator it = m_Members.Begin(); it != m_Members.End(); ++it)
		{
			pActorList->SendMessageToActor(*it, msgType);
		}
#ifndef CFG_GOLD
		--m_DebugLockCount;
#endif
	}
}

} // namespace Actors
