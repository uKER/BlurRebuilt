#include "stdafx.h"

#include "GameAudioLayer.h"
//#include "StringHash.h"

#include "AppInterfaceLayer.h"
#include "ListenerSubsystem.h"
#include <GameApp.h>

#include "GameAudio\SystemSubsystem.H"

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/GameData/GameData.h>

//#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
//#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
//#include <shared_actors/Components/Messages/ResourceMessages.h>
//#include "Game/GameActors/Components/Creation/AudioComponentsCreate.h"

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off)
#endif	//	AUDIO_OPTIMIZE_OFF

Debug::Bool gbCameraMessages("Audio.Subsystem.CameraMsgsEnabled", true, Debug::Persist());
Debug::Bool g_bLoadCamera("Audio.Subsystem.LoadSubsystem.LoadCamera", true, Debug::Persist());

//-----------------------------------------------------------------------------
// ListenerSubsystem::ListenerSubsystem
//-----------------------------------------------------------------------------
ListenerSubsystem::ListenerSubsystem()
{
}

//-----------------------------------------------------------------------------
// ProcessSystemMessage
//-----------------------------------------------------------------------------
bool ListenerSubsystem::ProcessSystemMessage(const U32 u32MessageId)
{
	if(!gbCameraMessages)
		return false;

	switch (u32MessageId)
	{
		case Audio::eInit:
			{
			if(!g_bLoadCamera)
				break;

			BIZ_ASSERT(!hEntity.IsValid());

			//Util::C_String string = "Listener";

			//pXtEntity = Audio::C_XtAudioDb::Get()->LookupXtEntityProxy(string);
			//if (pXtEntity)
			//{
			//	Game::CacheEntityResources(*pXtEntity, "ListenerSubsystem");
			//}
		}
		break;

		case Audio::eBeginLevel:
			{
			
				//m_CameraActor.Create(C_GameApp::GetInstance()->GetGameAudioLayer()->GetSubscriberSystem());
				
				Actor::C_ActorCreationDataFactoryHelper<Game::Actors::C_ListenerAudioActorCreate> pAudioActorCreate;
				m_listenerHandle = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->Spawn(pAudioActorCreate.Get());
				
				//set the actor handle
				C_GameApp::GetInstance()->GetGameAudioLayer()->SetAudioActorHnd(Game::C_GameAudioLayer::eListenerSubsystem,m_listenerHandle);
				
				const Actor::C_Actor* pActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetActorPtr(m_listenerHandle);

				Game::Actors::C_ListenerAudioActor *pListenerAudioActor = (Game::Actors::C_ListenerAudioActor*)Actor::ActorCast< Game::Actors::C_ListenerAudioActor>(pActor);
				
				pListenerAudioActor->CreateTrigger();
				//ProcessSystemMessage(-1);

			}
			break;

		case Audio::eEndLevel:
			{
				//const Actor::C_Actor* pActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetActorPtr(m_listenerHandle);

				//Game::Actors::C_ListenerAudioActor *pListenerAudioActor = (Game::Actors::C_ListenerAudioActor*)Actor::ActorCast< Game::Actors::C_ListenerAudioActor>(pActor);
				//
				//pListenerAudioActor->CreateTrigger();
				//Actor::C_ActorHandle emptyHandle;
				//emptyHandle.

				//C_GameApp::GetInstance()->GetGameAudioLayer()->SetAudioActorHnd(Game::C_GameAudioLayer::eListenerSubsystem,emptyHandle);
				if(hEntity.IsValid())
				{
						Audio::InterfaceController::Get().ReleaseEntity(hEntity);
						hEntity.Zero();

					/*if(pXtEntity)
					{
						pXtEntity->ReleaseEntityType();
						pXtEntity = NULL;

					}*/

				}



			}
			break;
		case Audio::eMute:
			for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("GamePaused"));
					Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn,0);
				}
			}
			break;

		case Audio::eUnmute:
			for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("GamePaused"));
					Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn,1);
				}
			}
			break;

		case Audio::eSuspend:
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::InterfaceController::Get().SuspendEntity(pInstance->GetAudioEntityHandle());
				}
			}

			break;

		case Audio::eResume:
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::InterfaceController::Get().ResumeEntity(pInstance->GetAudioEntityHandle());
				}
			}

			break;

		case Audio::eRaceStarted:
			for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("RaceStarted"));
					Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn);
				}
			}
			break;


	
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ProcessCustomMessage
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ListenerSubsystem::ProcessCustomMessage(const U32 u32MessageId, U32 u32Data , void* pData)
{
	return false;
}

void ListenerSubsystem::Update()
{
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on)
#endif	//	AUDIO_OPTIMIZE_OFF


