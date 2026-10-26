#include "SharedActorsPch.h"

#include <misc_shared/Messaging/GameMessaging.h>
#include <shared_actors/SharedActorMessages.h>
#include <shared_actors/ActorImpl/AnimatedObjectActors.h>
#include <shared_actors/Components/Interfaces/I_AnimatedObjectComponents.h>
#include <shared_actors/Components/Impl/AnimatedObjectComponents.h>
#include <shared_actors/Components/Messages/AnimatedObjectMessages.h>
#include <shared_actors/Components/Messages/MovementMessages.h>
#include <shared_actors/Components/Impl/MovementComponents.h>
#include <actor_studio_msg_game/ActorStudioMessaging.h>

#include <btl/containers/algorithm.h>

namespace Actors
{

C_AnimatedObjectActor::C_AnimatedObjectActor()
{
}

C_AnimatedObjectActor::~C_AnimatedObjectActor()
{
}

bool C_AnimatedObjectActor::InitialiseImpl()
{
	m_WorldMatrix = GetCreationData()->GetCreationMatrix();

	// create animation controller component
	if (InitialiseComponentFromSorcery(&GetCreationData()->m_AnimationPlayer) == NULL)
		return false;
		
	// and slave controller
	if (InitialiseComponentFromSorcery(&GetCreationData()->m_SlaveController) == NULL)
		return false;

	if (GetCreationData()->m_pVisual)
	{
		if (!InitialiseComponentFromSorcery(GetCreationData()->m_pVisual))
		{
			return false;
		}
	}
	
	if (GetCreationData()->m_pMovement)
	{
		if (!InitialiseComponentFromSorcery(GetCreationData()->m_pMovement))
		{
			return false;
		}
	}
	
	if (GetCreationData()->m_pParticleEffects)
	{
		if (!InitialiseComponentFromSorcery(GetCreationData()->m_pParticleEffects))
		{
			return false;
		}
	}
	
	if (GetCreationData()->m_pAnimatedEventTrigger)
	{
		if (!InitialiseComponentFromSorcery(GetCreationData()->m_pAnimatedEventTrigger))
		{
			return false;
		}
	}

	return true;
}

bool C_AnimatedObjectActor::CreateImpl()
{
	bool ok = Actor::C_Actor::CreateImpl();
	if (!ok) return false;
	
	if (m_Children.Size() > 0)
	{
		AddSubscriber(GameMessaging::E_GameMsg_Update);
	}
	return true;
}

bool C_AnimatedObjectActor::OnMessage(const Messaging::C_MessageBase* pMessage)
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
		
		case GameMessaging::E_GameMsg_Update:
			Update(Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue());
			break;

		case ActorMsg::E_MovementMsg_AddChild:
			if (DoesNotHaveMovementComponent())
			{
				if (m_Children.Size() == 0)
				{
					AddSubscriber(GameMessaging::E_GameMsg_Update);
				}
			
				const ActorMsg::C_ActorMovementAddChildMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementAddChildMsg>(pMessage);
				if (Btl::Find(m_Children, msg->GetActorHandle()) == m_Children.End())
				{
					m_Children.PushBack(msg->GetActorHandle());
				}
			}
			break;

		case ActorMsg::E_MovementMsg_RemoveChild:
			if (DoesNotHaveMovementComponent())
			{
				const ActorMsg::C_ActorMovementRemoveChildMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementRemoveChildMsg>(pMessage);
				Btl::Vector<Actor::C_ActorHandle>::Iterator it = Btl::Find(m_Children, msg->GetActorHandle());
				if (it != m_Children.End())
				{
					m_Children.Erase(it);
				}
				
				if (m_Children.Size() == 0)
				{
					RemoveSubscriber(GameMessaging::E_GameMsg_Update);
				}
			}	
			break;	

		case ActorStudioMsgShared::E_ActorStudioMsg_Drag:
		{
			const ActorStudioMsgGame::C_DragMsg* msg = Messaging::MessageCast<ActorStudioMsgGame::C_DragMsg>(pMessage);
			m_WorldMatrix = msg->GetValue();
		}
		break;

		default:
			return C_SharedActorBase::OnMessage(pMessage);
	}

	return true;
}

void C_AnimatedObjectActor::Update(F32 deltaTime)
{
	if (DoesNotHaveMovementComponent())
	{
		ActorMsg::C_ActorMovementUpdateMsg msg(deltaTime, Maths::C_QuatTrans(GetMatrix()));
		for (U32 i = 0, count = m_Children.Size(); i < count; ++i)
		{
			SendMessageToActor(m_Children[i], &msg);
		}
	}
}

bool C_AnimatedObjectActor::DoesNotHaveMovementComponent() const
{
	const Components::C_MovementBase* mover = NULL;
	return !QueryInterface(&mover);
}

// Serialisation Functions
// -----------------------

void C_AnimatedObjectActor::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_Actor::SerialiseForSave(stream);
	Serialise(stream);
}

void C_AnimatedObjectActor::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_Actor::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_AnimatedObjectActor::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_Actor::SerialiseForSync(stream);
	Serialise(stream);
}

void C_AnimatedObjectActor::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_Actor::DeserialiseForSync(stream);
	Deserialise(stream);
}

void C_AnimatedObjectActor::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_WorldMatrix;
	U32 numChildren = m_Children.Size();
	stream << numChildren;
	for (U32 i = 0; i < numChildren; ++i)
	{
		stream << m_Children[i];
	}
}

void C_AnimatedObjectActor::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_WorldMatrix;
	U32 numChildren = 0;
	stream >> numChildren;
	m_Children.Clear();
	
	Actor::C_ActorHandle child;
	for (U32 i = 0; i < numChildren; ++i)
	{
		stream >> child;
		m_Children.PushBack(child);
	}
}

}//namespace Actors
