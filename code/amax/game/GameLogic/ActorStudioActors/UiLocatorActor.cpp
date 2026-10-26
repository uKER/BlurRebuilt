#include "stdafx.h"

/*
#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>

#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>
*/

#include "UiLocatorActor.h"

#include <actor_studio_msg_game/ActorStudioMessaging.h>




//=========================================================================
//		Sorcery definitions
//=========================================================================



//	Actor creation definition (Sorcery)

XT_TYPE(::Actors::UiLocatorCreationData,		"UiLocatorActor")
	XT_BASE(::Actors::C_PosActorCreate)
	XT_FIELD( m_LocationName, "location_name" )
	XT_FIELD( m_Spline, "spline" )

XT_END_TYPE



	
//=========================================================================
//					Locator Actor Core Routines
//=========================================================================



namespace Actors
{

UiLocatorActor::List	UiLocatorActor::m_Store;

UiLocatorActor* UiLocatorActor::FindByName( const char* name )
{
	if( !name )
		return NULL;

	for(List::Iterator it = m_Store.Begin(); it != m_Store.End(); ++it)
	{
		UiLocatorActor* p = *(it);
		if( !p )
			continue;

		const char* pActorName = p->GetCreationData()->m_LocationName;

		if( !pActorName )
			continue;

		if( stricmp( pActorName, name ) )
			continue;

		return p;
	}

	return NULL;
}


//=========================================================================

UiLocatorActor::UiLocatorActor()
{
}

//=========================================================================

UiLocatorActor::~UiLocatorActor()
{
}

//=========================================================================
bool UiLocatorActor::InitialiseImpl()					// Called first
{
	if (!C_GameActor::InitialiseImpl())
		return(false);

	m_WorldMatrix = GetCreationData()->GetCreationMatrix();

	return(true);
}


//=========================================================================

bool UiLocatorActor::CreateImpl()						// Called second
{
	if (!C_GameActor::CreateImpl())
		return(false);

	m_Store.Insert( this );

//	AddSubscriber( GameMessaging::E_GameMsg_Update );

	return(true);
}

//=========================================================================
void UiLocatorActor::KillImpl()
{
	m_Store.Erase( this );
}

//=========================================================================
//						MESSAGE HANDLERS
//=========================================================================

bool UiLocatorActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	switch(pMessage->Type())
	{
	case ActorStudioMsgShared::E_ActorStudioMsg_Selected:
	case ActorStudioMsgShared::E_ActorStudioMsg_Deselected:
		break;

	case ActorStudioMsgShared::E_ActorStudioMsg_Drag:
		{
			const ActorStudioMsgGame::C_DragMsg* msg = Messaging::MessageCast<ActorStudioMsgGame::C_DragMsg>(pMessage);
			m_WorldMatrix = msg->GetValue();
		}
		break;
//		case GameMessaging::E_GameMsg_Update:
//			break;

	default:
		return C_GameActor::OnMessage(pMessage);
	}

	return(true);
}


}// End namespace Actors



//=========================================================================
//						Might be useful one day
//=========================================================================


//		LOGINFO( Resource::g_Log, "THIS IS A DEBUG MESSAGE\n");




