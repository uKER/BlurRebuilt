#include "stdafx.h"

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>

#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>

#include <GameLogic/ActorStudioActors/CutSceneMessages.h>

#ifndef CFG_GOLD
#include <GameLogic/Replays/ReplayDirector.h>
#endif // #ifndef CFG_GOLD

#include "ReplayManagerActor.h"







//=========================================================================
//		Sorcery definitions
//=========================================================================



//	Actor creation definition (Sorcery)

XT_TYPE(::Actors::ReplayManagerCreationData,		"ReplayManager")
	XT_BASE(::Actors::C_GameActorCreate)
XT_END_TYPE




//=========================================================================
//					ReplayManager Actor Core Routines
//=========================================================================



namespace Actors
{


//=========================================================================

ReplayManagerActor::ReplayManagerActor()
{
}

//=========================================================================

ReplayManagerActor::~ReplayManagerActor()
{
}

//=========================================================================

bool ReplayManagerActor::InitialiseImpl()					// Called first
{
	if (!C_GameActor::InitialiseImpl())
		return(false);

	return(true);
}


//=========================================================================

bool ReplayManagerActor::CreateImpl()						// Called second
{
	if (!C_GameActor::CreateImpl())
		return(false);

//	AddSubscriber( GameMessaging::E_GameMsg_Update );		// Needed for timed events

	return(true);
}


//=========================================================================
//						MESSAGE HANDLERS
//=========================================================================


bool ReplayManagerActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{

	switch(pMessage->Type())
	{
#ifndef CFG_GOLD
		case ActorMsg::E_CutSceneMsg_Replay_SetSaveFilename:
		{
			//	TO DO - Tony, do you want to decide how you want to send a filename here and implement it? [GY]
			Game::C_ReplayDirector::Instance().SetSaveFilename("");
			break;
		}

		case ActorMsg::E_CutSceneMsg_Replay_StartRecordingCs:
		{
			Game::C_ReplayDirector::Instance().StartRecordingCsReplay();
			break;
		}


		case ActorMsg::E_CutSceneMsg_Replay_Load:
		{
			const ActorMsg::C_Replay_LoadMsg* msg = Messaging::MessageCast<ActorMsg::C_Replay_LoadMsg>(pMessage);
			Game::C_ReplayDirector::Instance().LoadCsReplayFile(msg->mReplayFile);
			break;
		}

		case ActorMsg::E_CutSceneMsg_Replay_Play:
		{
			const ActorMsg::C_Replay_PlayMsg* msg = Messaging::MessageCast<ActorMsg::C_Replay_PlayMsg>(pMessage);
			Game::C_ReplayDirector::Instance().PlayCsReplay(msg->mStartTime);
			// Start timer for stop message
			break;
		}

		case ActorMsg::E_CutSceneMsg_Replay_Stop:
		{
			Game::C_ReplayDirector::Instance().StopCsReplay();
			break;
		}

		case ActorMsg::E_CutSceneMsg_Replay_Pause:
		{
			Game::C_ReplayDirector::Instance().PauseCsReplay();
			break;
		}

		case ActorMsg::E_CutSceneMsg_Replay_Resume:
		{
			Game::C_ReplayDirector::Instance().ResumeCsReplay();
			break;
		}
#endif // CFG_GOLD

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










