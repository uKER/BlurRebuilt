#include "CrowdSystemPch.h"

#include <crowd_system/CrowdSystemActor.h>
#include <crowd_system/CrowdSystem.h>
#include <crowd_system/CrowdMessaging.h>
#include <crowd_system/CrowdResources.h>
#include <crowd_system/CrowdWorld.h>

#include <crowd_system/XtCrowd.h>

#include <actor_system/ActorList.h>
#include <actor_system/ActorMessages.h>
#include <misc_shared/messaging/GameMessaging.h>

#include <game_resource/ModelResource.h>

namespace AnimatedCrowd
{

#ifndef CFG_GOLD
extern Debug::Bool	g_DebugRenderEvents;
#endif // CFG_GOLD

C_CrowdSystemActor::C_CrowdSystemActor()
	: m_pCrowdSystem(NULL),
	m_State(E_CROWD_ACTOR_STATE_CREATED)
{ }

C_CrowdSystemActor::~C_CrowdSystemActor()
{
	Destroy();
}

#ifndef CFG_GOLD
void C_CrowdSystemActor::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if (m_pCrowdSystem)
	{
		if (g_DebugRenderEvents)
			m_pCrowdSystem->DebugRenderEvents(debugRenderPtr);

		if (m_pCrowdSystem->GetWorld())
			m_pCrowdSystem->GetWorld()->DebugRender(debugRenderPtr);
	}
}
#endif // CFG_GOLD

bool C_CrowdSystemActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	switch (pMessage->Type())
	{
	case E_CrowdMsg_LoadResources:
		{
			BIZ_ASSERT(GetState() >= E_CROWD_ACTOR_STATE_INITIALISED);

			SetState(E_CROWD_ACTOR_STATE_FLUSH);
			return true;
		}
		break;
	case GameMessaging::E_GameMsg_Update:
		{
			F32 deltaTime = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue();

			Process(deltaTime);
			return true;
		}
		break;
	case Actor::E_ActorMsg_Created:
		{
			if(GetState() < E_CROWD_ACTOR_STATE_CROWD_RESOURCES_LOADED)
				return false;

			return true;
		}
		break;
	default:
		return Actors::C_SharedActorBase::OnMessage(pMessage);
		break;
	}

	return true;
}

void C_CrowdSystemActor::OnXtCrowdPopulationCreated(XtRecord* pRecord, const C_XtCrowdPopulation* pPopulation)
{
	SetState(E_CROWD_ACTOR_STATE_FLUSH);
}

void C_CrowdSystemActor::OnXtCrowdPopulationDestroyed(XtRecord* pRecord, const C_XtCrowdPopulation* pPopulation)
{
	SetState(E_CROWD_ACTOR_STATE_FLUSH);
}

void C_CrowdSystemActor::OnXtCrowdCharacterCreated(XtRecord* pRecord, const C_XtCrowdCharacter* pCharacter)
{
	SetState(E_CROWD_ACTOR_STATE_FLUSH);
}

void C_CrowdSystemActor::OnXtCrowdCharacterDestroyed(XtRecord* pRecord, const C_XtCrowdCharacter* pCharacter)
{
	SetState(E_CROWD_ACTOR_STATE_FLUSH);
}

void C_CrowdSystemActor::OnXtCrowdCycleGroupCreated(XtRecord* pRecord, const C_XtCrowdCycleGroup* pCycleGroup)
{
	SetState(E_CROWD_ACTOR_STATE_FLUSH);
}

void C_CrowdSystemActor::OnXtCrowdCycleGroupDestroyed(XtRecord* pRecord, const C_XtCrowdCycleGroup* pCycleGroup)
{
	SetState(E_CROWD_ACTOR_STATE_FLUSH);
}

void C_CrowdSystemActor::OnXtCrowdCycleGroupItemCreated(XtRecord* pRecord, const C_XtCrowdCycleGroupItem* pCycleGroupItem)
{
	SetState(E_CROWD_ACTOR_STATE_FLUSH);
}

void C_CrowdSystemActor::OnXtCrowdCycleGroupItemDestroyed(XtRecord* pRecord, const C_XtCrowdCycleGroupItem* pCycleGroupItem)
{
	SetState(E_CROWD_ACTOR_STATE_FLUSH);
}

void C_CrowdSystemActor::OnXtCrowdEffectSystemCreated(XtRecord* pRecord, const C_XtCrowdEffectSystem* pEffectSystem)
{
	SetState(E_CROWD_ACTOR_STATE_FLUSH);
}

void C_CrowdSystemActor::OnXtCrowdEffectSystemDestroyed(XtRecord* pRecord, const C_XtCrowdEffectSystem* pEffectSystem)
{
	SetState(E_CROWD_ACTOR_STATE_FLUSH);
}

void C_CrowdSystemActor::OnXtCrowdAnimationEventEffectCreated(XtRecord* pRecord, const C_XtCrowdAnimationEventEffect* pAnimationEventEffect)
{
	SetState(E_CROWD_ACTOR_STATE_FLUSH);
}

void C_CrowdSystemActor::OnXtCrowdAnimationEventEffectDestroyed(XtRecord* pRecord, const C_XtCrowdAnimationEventEffect* pAnimationEventEffect)
{
	SetState(E_CROWD_ACTOR_STATE_FLUSH);
}

void C_CrowdSystemActor::OnXtCrowdTextureListCreated(XtRecord* pRecord, const C_XtCrowdTextureList* pTextureList)
{
	if (m_pCrowdSystem)
		m_pCrowdSystem->AddTextureList(pTextureList);
}

void C_CrowdSystemActor::OnXtCrowdTextureListDestroyed(XtRecord* pRecord, const C_XtCrowdTextureList* pTextureList)
{
	if (m_pCrowdSystem)
		m_pCrowdSystem->RemoveTextureList(pTextureList);
}

void C_CrowdSystemActor::OnCrowdSystemRouteChanged()
{
	SetState(E_CROWD_ACTOR_STATE_FLUSH);
}

bool C_CrowdSystemActor::InitialiseImpl()
{
	SetState(E_CROWD_ACTOR_STATE_CREATED);

	const C_CrowdSystemCreate* pSystemCreate = (const C_CrowdSystemCreate*)(GetCreationData().GetData());

	m_pCrowdSystem = pSystemCreate->m_pCrowdSystem;
	
	if (m_pCrowdSystem)
		m_pCrowdSystem->AddListener(this);
	
	return true;
}

bool C_CrowdSystemActor::CreateImpl()
{
	AddSubscriber( GameMessaging::E_GameMsg_Update );

	AddSubscriber( Actor::E_ActorMsg_Created );

	// Crowd messages
	AddSubscriber( E_CrowdMsg_LoadResources );

	Initialise();
	return true;
}

void C_CrowdSystemActor::Initialise()
{
	SetState(E_CROWD_ACTOR_STATE_INITIALISED);
}

void C_CrowdSystemActor::Destroy()
{
	if (m_pCrowdSystem)
		m_pCrowdSystem->RemoveListener(this);
}

void C_CrowdSystemActor::SetState(const CrowdSystemActorState state)
{
	m_State = state;
}

const CrowdSystemActorState C_CrowdSystemActor::GetState() const
{
	return m_State;
}

void C_CrowdSystemActor::Process(const F32 deltaTime)
{
	switch (GetState())
	{
	case E_CROWD_ACTOR_STATE_FLUSH:
		{
			if (m_pCrowdSystem->Flush())
				SetState(E_CROWD_ACTOR_STATE_LOAD_POPULATION);
		}
		break;

	case E_CROWD_ACTOR_STATE_INITIALISED:
		{
			if (m_pCrowdSystem && m_pCrowdSystem->GetResources())
			{
				if (m_pCrowdSystem->GetResources()->IsLoaded())
					SetState(E_CROWD_ACTOR_STATE_CROWD_RESOURCES_LOADED);
			}
		}
		break;

	case E_CROWD_ACTOR_STATE_LOAD_POPULATION:
		{
			const C_XtCrowdPopulation* pPopulation = m_pCrowdSystem->GetRoutePopulation();

			if (pPopulation)
				BuildPopulation(pPopulation);
		}
		break;

	case E_CROWD_ACTOR_STATE_LOAD_CROWD_RESOURCES:
		{
			LoadCrowdResources();
		}
		break;

	case E_CROWD_ACTOR_STATE_CROWD_RESOURCES_LOADED:
		{
			SetState(E_CROWD_ACTOR_STATE_POST_INITIALISE);
		}
		break;

	case E_CROWD_ACTOR_STATE_POST_INITIALISE:
		{
			SetState(E_CROWD_ACTOR_STATE_READY);
		}
		break;

	case E_CROWD_ACTOR_STATE_READY:
		{
			if (m_pCrowdSystem)
			{
				m_pCrowdSystem->UpdateEffects(deltaTime);
				m_pCrowdSystem->PreUpdateWorld(deltaTime);
			}
		}
		break;
	default:
		break;
	}
}

void C_CrowdSystemActor::LoadCrowdResources()
{
	if (GetState() == E_CROWD_ACTOR_STATE_READY)
		return;

	if (m_pCrowdSystem && m_pCrowdSystem->GetResources())
	{
		if (!m_pCrowdSystem->GetResources()->CreatePoseFormat())
			return; // Can't proceed without a valid pose format

		m_pCrowdSystem->GetResources()->SetLoaded(m_pCrowdSystem->GetWorld()->GetPaletteMap());

		BIZ_ASSERT(m_pCrowdSystem->GetResources()->GetPopulation().GetSkeleton() && m_pCrowdSystem->GetResources()->GetPopulation().GetSkeleton()->IsValid());

		const C_XtCrowdPopulation* pPopulation = m_pCrowdSystem->GetRoutePopulation();

		if (pPopulation)
			pPopulation->PostLoadResources(m_pCrowdSystem->GetResources()->GetPopulation());

		m_pCrowdSystem->GetResources()->CreateAnimationEvents();
	}

	SetState(E_CROWD_ACTOR_STATE_CROWD_RESOURCES_LOADED);
}

bool C_CrowdSystemActor::BuildPopulation(const C_XtCrowdPopulation* pPopulation)
{
	if (!pPopulation) return false;

	//if (!m_pCrowdSystem->Flush())
	//	return false;

	if (!pPopulation->Build(m_pCrowdSystem->GetResources()->GetPopulation()))
		return false;

	SetState(E_CROWD_ACTOR_STATE_LOAD_CROWD_RESOURCES);

	return true;
}

}; // namespace AnimatedCrowd
