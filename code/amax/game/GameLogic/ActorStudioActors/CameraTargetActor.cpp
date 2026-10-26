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

#include "CameraTargetActor.h"






//=========================================================================
//		Sorcery definitions
//=========================================================================



//	Actor creation definition (Sorcery)

XT_TYPE(Actors::CameraTargetCreationData,		"CameraTarget")
	XT_BASE(Actors::C_PosActorCreate)
XT_END_TYPE




//=========================================================================
//					CameraTarget Actor Core Routines
//=========================================================================



namespace Actors
{


//=========================================================================

CameraTargetActor::CameraTargetActor()
{
}

//=========================================================================

CameraTargetActor::~CameraTargetActor()
{
}

//=========================================================================

bool CameraTargetActor::InitialiseImpl()					// Called first
{
	if (!C_GameActor::InitialiseImpl())
		return(false);

	m_WorldMatrix = GetCreationData()->GetCreationMatrix();

	return(true);
}


//=========================================================================

bool CameraTargetActor::CreateImpl()						// Called second
{
	if (!C_GameActor::CreateImpl())
		return(false);

//	AddSubscriber( GameMessaging::E_GameMsg_Update );

	return(true);
}


//=========================================================================
//						MESSAGE HANDLERS
//=========================================================================


bool CameraTargetActor::OnMessage(const Messaging::C_MessageBase *pMessage)
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




