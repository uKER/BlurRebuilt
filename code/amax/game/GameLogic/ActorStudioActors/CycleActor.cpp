#include "stdafx.h"

#include <Game/Game.h>

#include <GameApp.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>

#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>

#include <GameLogic\ActorStudioActors\CutSceneMessages.h>
//#include <GameLogic\Messaging\InGameDialogueMessages.h>
#include <Game/Messaging\GameFlowMessages.h>
//#include <Game/GameActors/Components/Messages/AudioMessages.h>
#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>

//#include <GameLogic/Layers/RaceBookLayer.h>
//#include <GameLogic/Layers/AmaxWorldLayer.h>
//#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

//#include <Game\GameActors\Vehicles\VehicleActor.h>

//#include <GameApp.h>
//#include <Game/Game.h>

//#include <GameLogic\RaceIntroOutro\RaceIntroOutro.h>

//#include <GameLogic/ActorStudioActors/DynamicLightActor.h>

//#include <GameLogic/ActorStudioActors/ActorUtils.h>


#include "CycleActor.h"




//=========================================================================
//		Sorcery definitions
//=========================================================================



//	Actor creation definition (Sorcery)

XT_TYPE(::Actors::CycleElementCreationData,		"CycleElement")
	XT_FIELD(m_ElementActor,					"ElementActor")
	XT_FIELD(m_InitialState,					"InitialState")
XT_END_TYPE


XT_TYPE(::Actors::CycleActorCreationData,		"CycleActor")
	XT_BASE(::Actors::C_GameActorCreate)
	XT_FIELD(m_TimeStep,						"TimeStep")
	XT_FIELD(m_Elements,						"Elements")
	XT_FIELD(m_ActivateMessage,					"ActivateMessage")
	XT_FIELD(m_DeactivateMessage,				"DeactivateMessage")
XT_END_TYPE


//=========================================================================
//					Component Creation Setup
//=========================================================================


void ::Actors::CycleActorCreationData::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_GameActorCreate::DoSorceryCallback(pCallbackFn);
}




//=========================================================================
//					Cycle Actor Core Routines
//=========================================================================



namespace Actors
{


//=========================================================================

CycleActor::CycleActor()
{
}

//=========================================================================

CycleActor::~CycleActor()
{
}

//=========================================================================

bool CycleActor::InitialiseImpl()					// Called first
{
	if (!C_GameActor::InitialiseImpl())
		return(false);

	const CycleActorCreationData *pCreate = GetCreationData();

	return(true);
}


//=========================================================================

bool CycleActor::CreateImpl()						// Called second
{
	if (!C_GameActor::CreateImpl())
		return(false);


	m_NextChangeTime = 0;
	m_CurrectStateOffset = 0;

//	AddSubscriber(GameMessaging::E_GameMsg_PostUpdate);			// Now subscribe after race load
	AddSubscriber(Game::E_GameflowMessage_RaceLoaded);
	
	return(true);
}

//=========================================================================

void CycleActor::ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData )
{
	switch(result)
	{
		case Resource::E_Notify_DataReady:
			break;
		
		case Resource::E_Notify_DataError:
			break;
		
		case Resource::E_Notify_NoData:
			break;
	}
}

//=========================================================================
//						MESSAGE HANDLERS
//=========================================================================


bool CycleActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	S32		a;
	bool	oldstate,newstate;

	switch(pMessage->Type())
	{
		case Game::E_GameflowMessage_RaceLoaded:		// Send initial setup messages
		{
			const CycleActorCreationData *cd = GetCreationData();
			m_NumElements = cd->m_Elements.count;
			for(a=0;a<m_NumElements;a++)
			{
				if (cd->m_Elements[a].m_ElementActor)
				{
					if (cd->m_Elements[a].m_InitialState)
						SendMessageToActor(cd->m_Elements[a].m_ElementActor->GetActorHandle(), cd->m_ActivateMessage->GetMessage(this));
					else
						SendMessageToActor(cd->m_Elements[a].m_ElementActor->GetActorHandle(), cd->m_DeactivateMessage->GetMessage(this));
				}
			}

			m_NextChangeTime = C_GameApp::GetInstance()->GetTimer().GetElapsedTime() + GetCreationData()->m_TimeStep->m_Time;
			m_CurrectStateOffset = 0;
			AddSubscriber(GameMessaging::E_GameMsg_PostUpdate);
			break;
		}

		case GameMessaging::E_GameMsg_PostUpdate:
		{
			if (C_GameApp::GetInstance()->GetTimer().GetElapsedTime() > m_NextChangeTime)			// If we've just gone past the next state change
			{
				const CycleActorCreationData *cd = GetCreationData();
				m_NumElements = cd->m_Elements.count;

				m_NextChangeTime = C_GameApp::GetInstance()->GetTimer().GetElapsedTime() + GetCreationData()->m_TimeStep->m_Time;							// Store the next one
				m_CurrectStateOffset--;										// Shift all the states
				if (m_CurrectStateOffset < 0)
					m_CurrectStateOffset =  m_NumElements-1;

				for(a=0;a<m_NumElements;a++)
				{
					if (cd->m_Elements[a].m_ElementActor)
					{
						oldstate = cd->m_Elements[(m_CurrectStateOffset + a + 1) % m_NumElements].m_InitialState;
						newstate = cd->m_Elements[(m_CurrectStateOffset + a) % m_NumElements].m_InitialState;
						if (oldstate && !newstate)
							SendMessageToActor(cd->m_Elements[a].m_ElementActor->GetActorHandle(), cd->m_DeactivateMessage->GetMessage(this));
						else if (!oldstate && newstate)
							SendMessageToActor(cd->m_Elements[a].m_ElementActor->GetActorHandle(), cd->m_ActivateMessage->GetMessage(this));
					}
				}
			}
			break;
		}
	}

	return(true);
}



} // namespace Actors
