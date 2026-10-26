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

#include "CameraPositionActor.h"






//=========================================================================
//		Sorcery definitions
//=========================================================================



//	Actor creation definition (Sorcery)

XT_TYPE(Actors::CameraPositionCreationData,		"CameraPosition")
	XT_BASE(Actors::C_PosActorCreate)
XT_END_TYPE




//=========================================================================
//					CameraPosition Actor Core Routines
//=========================================================================



namespace Actors
{


//=========================================================================

CameraPositionActor::CameraPositionActor()
{
}

//=========================================================================

CameraPositionActor::~CameraPositionActor()
{
}

//=========================================================================

bool CameraPositionActor::InitialiseImpl()					// Called first
{
	if (!C_GameActor::InitialiseImpl())
		return(false);

	m_WorldMatrix = GetCreationData()->GetCreationMatrix();

	return(true);
}


//=========================================================================

bool CameraPositionActor::CreateImpl()						// Called second
{
	if (!C_GameActor::CreateImpl())
		return(false);

//	AddSubscriber( GameMessaging::E_GameMsg_Update );

	return(true);
}


//=========================================================================
//						MESSAGE HANDLERS
//=========================================================================


bool CameraPositionActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
//	switch(pMessage->Type())
//	{
////		case GameMessaging::E_GameMsg_Update:
////			break;
//
//		default:
			return C_GameActor::OnMessage(pMessage);
	//}

	//return(true);
}




}// End namespace Actors



//=========================================================================
//						Might be useful one day
//=========================================================================


//		LOGINFO( Resource::g_Log, "THIS IS A DEBUG MESSAGE\n");




