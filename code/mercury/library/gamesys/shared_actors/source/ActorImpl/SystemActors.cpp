#include "SharedActorsPch.h"

#include <actor_system/ActorList.h>

#include <shared_actors/ActorImpl/SystemActors.h>
#include <shared_actors/SharedActorMessages.h>
#include <shared_actors/Components/Creation/MovementComponentsCreate.h>

#ifndef CFG_GOLD
#include <actor_studio_msg_game/ActorStudioMessaging.h>
#endif

Debug::Bool g_DebugRenderNodeActor("ActorDebug.Actors.Node.DebugRender", true);

namespace Actors
{
// Null Actor
bool C_NullActor::InitialiseImpl()
{
	m_WorldMatrix = Load(GetCreationData()->m_WorldMatrix);
	return true;
}

// Proxy Actor
bool C_ProxyActor::InitialiseImpl()
{
	if(GetCreationData()->m_InitialActor)
		m_hActor = GetCreationData()->m_InitialActor->GetActorHandle();

	return true;
}

bool C_ProxyActor::OnMessage(const Messaging::C_MessageBase *pMessage )
{
	switch(pMessage->Type())
	{
		// Set Proxy Message Handler
	case ActorMsg::E_GameActorMsg_SetProxyActor:
		m_hActor = Messaging::MessageCast<ActorMsg::C_SetProxyActorMsg>(pMessage)->GetActorHandle();
		break;

	default:
		if(GetActorList()->ActorExists(m_hActor))
			return GetActorList()->SendMessageToActor(m_hActor,pMessage) == Actor::E_ActorMessageSendResult_OK;
		else
			return C_SharedActorBase::OnMessage(pMessage);
	}

	return true;
}

void C_ProxyActor::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_hActor;
}

void C_ProxyActor::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_hActor;
}

void C_ProxyActor::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	Actor::C_Actor::SerialiseForSave(stream);
	Serialise(stream);
}

void C_ProxyActor::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	Actor::C_Actor::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_ProxyActor::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	Actor::C_Actor::SerialiseForSync(stream);
	Serialise(stream);
}

void C_ProxyActor::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	Actor::C_Actor::DeserialiseForSync(stream);
	Deserialise(stream);
}

const Maths::HMat43	& C_ProxyActor::GetMatrix(void) const
{
	const C_Actor *pActor = GetActorList()->GetActorPtr(m_hActor);

	return (pActor) ? pActor->GetMatrix() : C_Actor::GetMatrix();
}

const Maths::C_Vertex C_ProxyActor::GetPosition(void) const
{
	const C_Actor *pActor = GetActorList()->GetActorPtr(m_hActor);

	return (pActor) ? pActor->GetPosition() : C_Actor::GetPosition();
}

// Node Actor


bool C_NodeActor::InitialiseImpl()
{
	const C_NodeActorCreate* pNodeCreate = GetCreationData();

	m_WorldMatrix = pNodeCreate->GetCreationMatrix();
	m_selected = false;

	if(pNodeCreate->m_pSorceryMovementComponentCreate != NULL)
	{
		if (InitialiseComponentFromSorcery(pNodeCreate->m_pSorceryMovementComponentCreate) == NULL)
			return false;
	}

	return true;
}

bool C_NodeActor::OnMessage(const Messaging::C_MessageBase *pMessage )
{
	switch(pMessage->Type())
	{
		case ActorMsg::E_GameActorMsg_SetTransform:
		{
			const ActorMsg::C_ActorSetTransformMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorSetTransformMsg>(pMessage);
			m_WorldMatrix = msg->Transform().Matrix();
		}
		break;

		case ActorMsg::E_GameActorMsg_AnimateTransform:
		{
			const ActorMsg::C_ActorAnimateTransformMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorAnimateTransformMsg>(pMessage);
			m_WorldMatrix = msg->Transform().Matrix();
		}
		break;
#ifndef CFG_GOLD
		case ActorStudioMsgShared::E_ActorStudioMsg_Drag:
		{
			const ActorStudioMsgGame::C_DragMsg* msg = Messaging::MessageCast<ActorStudioMsgGame::C_DragMsg>(pMessage);
			m_WorldMatrix = msg->GetValue();
		}
		break;

		case ActorStudioMsgShared::E_ActorStudioMsg_EndDrag:
		{
			bool dummy = true;
		}
		break;

		case ActorStudioMsgShared::E_ActorStudioMsg_Selected:
		{
			m_selected = true;
		}
		break;

		case ActorStudioMsgShared::E_ActorStudioMsg_Deselected:
		{
			m_selected = false;
		}
		break;
#endif
		// Allow node to be a proxy for messages?
		case ActorMsg::E_GameActorMsg_SetProxyActor:
		{
		}
		break;
		default:
			return C_SharedActorBase::OnMessage(pMessage);
	}

	return true;
}

void C_NodeActor::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_WorldMatrix;
}

void C_NodeActor::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_WorldMatrix;

}

void C_NodeActor::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_Actor::SerialiseForSave(stream);
	Serialise(stream);
}

void C_NodeActor::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_Actor::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_NodeActor::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_Actor::SerialiseForSync(stream);
	Serialise(stream);
}

void C_NodeActor::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_Actor::DeserialiseForSync(stream);
	Deserialise(stream);
}

#ifndef CFG_GOLD
void C_NodeActor::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if(g_DebugRenderNodeActor)
	{
		debugRenderPtr->Draw3dMatrix(m_WorldMatrix, 0.5f);

		if(m_selected)
			debugRenderPtr->Draw3dText(Maths::HVec3(m_WorldMatrix.Translation()), "Selected");
	}
}
#endif

}

