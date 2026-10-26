#include "stdafx.h"

#include "GameAudioLayer.h"

#include <GameApp.h>
#include "AppInterfaceLayer.h"
#include "DialogueSubsystem.h"
#include "XtAudio.h"
#include <GameLogic\Messaging\InGameDialogueMessages.h>
#include <GameApp.h>
#include <Game/Game.h>
#include <Game/GameWorld/GameWorld.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceLogicLayer.h>

#include "GameAudio\SystemSubsystem.H"

//#include <Game/GameWorld/GameWorld.h>

#include "Game/GameActors/Components/Messages/AudioMessages.h"

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off)
#endif	//	AUDIO_OPTIMIZE_OFF

#define LOAD_STREAMS_ON_FLY
//
//
Debug::Bool gbDialogueMessages("Audio.Subsystem.DialogueMsgsEnabled", true, Debug::Persist());
Debug::Bool g_bLoadDialogue("Audio.Subsystem.LoadSubsystem.LoadDialogue", true, Debug::Persist());

Debug::Int g_debugDelayFrames("Audio.Subsystem.Dialogue.DelayFrames", 0);

Debug::Bool g_debugTriggerLine("Audio.Subsystem.Dialogue.TriggerLine", false);

Debug::Float g_debugBankID("Audio.Subsystem.Dialogue.BankID", 0);
Debug::Float g_debugSampleIndex("Audio.Subsystem.Dialogue.SampleIndex", 0);

extern Debug::Logger &g_InGameDialogueLogger;


//-----------------------------------------------------------------------------
// DialogueSubsystem::DialogueSubsystem
//-----------------------------------------------------------------------------
DialogueSubsystem::DialogueSubsystem()
{
		m_playDialogue = false;
		m_VOActive = false;
}

//-----------------------------------------------------------------------------
// ProcessSystemMessage
//-----------------------------------------------------------------------------
bool DialogueSubsystem::ProcessSystemMessage(const U32 u32MessageId)
{
	if(!gbDialogueMessages)
		return false;

	switch (u32MessageId)
	{
		case Audio::eInit:
		{
		////	// create a Dialogue entity
		////	Util::C_String string = "Dialogue";
		////	Audio::XtEntityDef* pXtEntity = Audio::C_XtAudioDb::Get()->LookupXtEntity(string);
		////	if (pXtEntity)
		////	{
		////		pXtEntity->SetupEntityType();

		////		if (pXtEntity->GetEntityTypeHnd().IsValid())
		////		{
		////			AudioEntityHandle hEntity = Audio::InterfaceController::Get().CreateEntity(pXtEntity->GetEntityTypeHnd());
		////			Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(hEntity, "StartActionIn");

		////			// connect to renderer
		////			Audio::EntityDefinition* pDef = Audio::InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(pXtEntity->GetEntityDefHnd());
		////			if (pDef && pDef->LookupInstanceComponent("OutputToRenderer") != 0xffffffff)
		////				Audio::InterfaceController::Get().SetMaster(hEntity, "OutputToRenderer");

		////			Audio::InterfaceController::Get().TriggerEntityActionIn(hEntity, hActionIn);

		////			// temp. store handle in AudioSystem class
		////			C_GameApp::GetInstance()->GetGameAudioLayer()->SetActorEntityHnd(Game::C_GameAudioLayer::eDialogueSubsystem,hEntity);
		////		}
		////	}
		////	
			SetSubscriberSystem( &Game::C_Game::GetInstance()->GetSubscriberSystem() );
				AddSubscriber(E_AMAXDIALOGUE_ADDWAVEBANK);
				AddSubscriber(E_AMAXDIALOGUE_PLAYAUDIO);

			for(int i = 0;i< mMaxNumDialogueEntites;i++)
			{

				m_DialogueEntityPool[i].m_entityHandle.Zero();
				m_DialogueEntityPool[i].m_entityProxy = NULL;

			}
		}
		break;
		case Audio::eBeginLevel:
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					//Audio::InterfaceController::Get().ResumeEntity(pInstance->GetAudioEntityHandle());
					Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("StartActionIn"));
					Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn);
				}
			}
			m_currentBankID = 0; // reset the currentBankID
			break;

		case Audio::eEndLevel:

				//delete all instance at the end of a level..bug 26549, on a rety, VO plays going inot a level and never plays again. VO was gettting into a state when it never send a stopMessage callback
				if(m_currentBankID >= 0 && m_currentBankID < mMaxNumDialogueEntites)
				{
						if(m_DialogueEntityPool[m_currentBankID].m_entityHandle.IsValid())
						{
							//send a stop msg to dialoguemanager and delete any active instances
							ProcessCustomMessage(eAudioStopped);

							Audio::InterfaceController::Get().ReleaseEntity(m_DialogueEntityPool[m_currentBankID].m_entityHandle);
							m_DialogueEntityPool[m_currentBankID].m_entityHandle.Zero();
							m_DialogueEntityPool[m_currentBankID].m_isActive = false;


						}
				}

			break;

		case Audio::eDumpLevel:
			{
				for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
				{
					Audio::EntityInstance* pInstance = m_aEntityInstance[i];
					if (pInstance)
					{
						Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("RaceFinished"));
						Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn);
					}

					
				}
				
				for(int i = 0;i< mMaxNumDialogueEntites;i++)
				{
					if(m_DialogueEntityPool[i].m_entityHandle.IsValid())
					{
						Audio::InterfaceController::Get().ReleaseEntity(m_DialogueEntityPool[i].m_entityHandle);
						m_DialogueEntityPool[i].m_entityHandle.Zero();
						m_DialogueEntityPool[i].m_isActive = false;


					}

					if(m_DialogueEntityPool[i].m_entityProxy)
					{
						m_DialogueEntityPool[i].m_entityProxy->ReleaseEntityType();
						m_DialogueEntityPool[i].m_entityProxy = NULL;

					}

				}

				
			}
			break;
		
		case Audio::eMute:
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("GamePaused"));
					Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn,0);
					//Audio::InterfaceController::Get().SuspendEntity(pInstance->GetAudioEntityHandle());
				}
			}
			break;

		case Audio::eUnmute:
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					//Audio::InterfaceController::Get().ResumeEntity(pInstance->GetAudioEntityHandle());
					Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("GamePaused"));
					Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn,1);
				}
			}
			break;

		case Audio::eLoadUI:
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
					AudioEntityHandle handle = pInstance->GetAudioEntityHandle();
					for(int i = 0;i<m_listSize;i++)
						if(m_DialogueEntityPool[i].m_entityHandle == handle)
							Audio::InterfaceController::Get().ResumeEntity(handle);
				}
			}

			break;
		default:
			break;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ProcessCustomMessage
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool DialogueSubsystem::ProcessCustomMessage(const U32 u32MessageId, U32 u32Data, void* pData)
{
	if(!gbDialogueMessages)
		return false;

		switch(u32MessageId)
		{
			case eAudioStopped:
				{
					//reset the vo active bool BEFORE we send the message as we may get another play message back immediately
					m_VOActive = false;

					ActorMsg::C_AudioDialogueFinishMsg msg;
					Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&msg);

					if(	C_GameApp::GetInstance()->GetRaceBookLayer() &&
						C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer() &&
						C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld())
						C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage(&msg);		// Send to Actor Studio actors

					//printf("sending audio finished msg\n");
					AudioEntityHandle currentEntity(u32Data,Game::C_GameAudioLayer::eDialogueSubsystem);

					for(int i = 0;i<m_listSize;i++)
						//if(m_DialogueEntityPool[i].m_entityHandle == currentEntity)
							m_DialogueEntityPool[i].m_isActive = false;


					Audio::InterfaceController::Get().UntrackValue(m_VOPosReturnDataHandle);
			

					
					//suspend the entity
					//Audio::InterfaceController::Get().SuspendEntity(currentEntity);
					ProcessSystemMessage(Audio::eSuspend);

				}
				break;
		}

	return true;
}

bool DialogueSubsystem::MessageHandler(const Messaging::C_MessageBase *pMessage )
{

	switch(pMessage->Type())
	{

	case E_AMAXDIALOGUE_ADDWAVEBANK:
		{

			if(!g_bLoadDialogue)
				break;

			m_VOActive = false;

			//flush wavebanks if still loaded
				ProcessSystemMessage(Audio::eDumpLevel);

			const Game::InGameDialogueAddWavebank* msg = Messaging::MessageCast<const Game::InGameDialogueAddWavebank>(pMessage);

			m_listSize = msg->mSplitGroupList.Size();


			for( Btl::Set<U32>::ConstIterator bankit = msg->mSplitGroupList.Begin(); bankit != msg->mSplitGroupList.End(); bankit++ )
			{
				U32 val = *bankit;
				//U32 val = bankIt->GetKey();

				////load all dialogue banks here
				LoadEntity(val);

				//if( bankIt == list.End() )				
				//	break;


				//send them all to sleep
				ProcessSystemMessage(Audio::eSuspend);

			}
		}
		break;

		case E_AMAXDIALOGUE_PLAYAUDIO:
		{
			if(!g_bLoadDialogue)
				break;
			
			// ignore any messages if we haven't finished the last dialogue line
			if (m_VOActive)
			{
				LOGWARN(g_InGameDialogueLogger, DBSTR "Ignoring PlayAudio message as VO is still active.previous sample played was: sample %d bank %d", m_sampleIndex, m_bankID);
				break;
			}

			const Game::InGameDialogueEvent* msg = Messaging::MessageCast<Game::InGameDialogueEvent>(pMessage);

			m_playDialogue = true;

			//need to insert a delay to the message here due to streaming issues...
			m_delayFrames = g_debugDelayFrames;

			m_bankID = msg->mSplitGroup;
			m_sampleIndex = msg->mSplitIndex;

			LOGDEBUG(g_InGameDialogueLogger, DBSTR "Audio received PlayAudio message. sample %d bank %d", m_sampleIndex, m_bankID);

			//U32 bankID = msg->mSplitGroup;
			//U32 sampleIndex = msg->mSplitIndex;
			////U32 splitGroup =	



			//if(bankID < mMaxNumDialogueEntites &&  m_DialogueEntityPool[bankID].m_entityHandle.IsValid())
			//{
			//				//resume the entity
			//	Audio::InterfaceController::Get().ResumeEntity(m_DialogueEntityPool[bankID].m_entityHandle);
			//	m_DialogueEntityPool[bankID].m_isActive = true;

			//	Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_DialogueEntityPool[bankID].m_entityHandle, "VOTriggerActionIn");
			//	Audio::InterfaceController::Get().TriggerEntityActionIn(m_DialogueEntityPool[bankID].m_entityHandle, hActionIn);

			//	hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_DialogueEntityPool[bankID].m_entityHandle, "SampleIndex");
			//	Audio::InterfaceController::Get().SetEntityValue(m_DialogueEntityPool[bankID].m_entityHandle, hActionIn,(F32)sampleIndex);



			//}

		}
		break;



	}

	return false;


}

void DialogueSubsystem::LoadEntity(int entityIndex)
{	


				//only attempt to create if not valid
				if(!m_DialogueEntityPool[entityIndex].m_entityHandle.IsValid()) // check validity
				{

					//map string from message enum
					//Util::C_String32 string = entityString;

					
					char  stringName[512];
					sprintf_s( stringName, 512, "RaceDialogue_%d", entityIndex );

					Audio::XtEntityProxy* pXtEntity = Audio::C_XtAudioDb::Get()->LookupXtEntityProxy(stringName);
					if (pXtEntity)
					{
						Game::CacheEntityResources(*pXtEntity, "DialogueSubsystem", true, true);

						if (pXtEntity->GetEntityTypeHnd().IsValid())
						{
#ifndef LOAD_STREAMS_ON_FLY
							AudioEntityHandle hEntity;
							Audio::InterfaceController::Get().CreateEntity(pXtEntity->GetEntityTypeHnd(), hEntity);
							//Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(hEntity, "StartActionIn");

							// connect to system entity
							Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eConnectToSystem, 0, &hEntity);

							m_DialogueEntityPool[entityIndex].m_entityHandle = hEntity;


							Audio::EntityDataMemberHandle hActionOut = Audio::InterfaceController::Get().LookupInstanceComponent(m_DialogueEntityPool[entityIndex].m_entityHandle, "VOFinished");		

							if(hActionOut != -1)
								Audio::InterfaceController::Get().SetCallback(new Audio::DefaultEntityCallback(m_DialogueEntityPool[entityIndex].m_entityHandle,eAudioStopped), hActionOut);

							Audio::InterfaceController::Get().SuspendEntity(hEntity);

							//Audio::InterfaceController::Get().SetCallback(new Audio::DefaultEntityCallback(pInstance->GetAudioEntityHandle(), C_GameAudioSystem::eSuspendCallback), hActionOut);

						//	Audio::InterfaceController::Get().TriggerEntityActionIn(hEntity, hActionIn);

							// temp. store handle in AudioSystem class
							//C_GameApp::GetInstance()->GetGameAudioLayer()->SetActorEntityHnd(Game::C_GameAudioLayer::eUISubsystem,hEntity);

#endif

							//store the proxy entity
							m_DialogueEntityPool[entityIndex].m_entityProxy = pXtEntity;
						}
					}
				}
				else
				{
					//BIZ_ASSERT_FAIL("Duplicate Sorcery dialogue ID - record for %s using ID number %d", entityString.Chars(), entityIndex);
				}
}

void DialogueSubsystem::Update()
{
	if(!g_bLoadDialogue)
			return;

	if(g_debugTriggerLine)
	{

		m_playDialogue = true;
		m_sampleIndex = (U32)g_debugSampleIndex;
		m_bankID = (U32)g_debugBankID;
		g_debugTriggerLine = false;
		m_delayFrames = 0;

	}

	if(m_playDialogue)
	{
		//const Game::InGameDialogueEvent* msg = Messaging::MessageCast<Game::InGameDialogueEvent>(pMessage);

		//U32 bankID = msg->mSplitGroup;
		//U32 sampleIndex = msg->mSplitIndex;
		//U32 splitGroup =	

		//wait till framecounter decrements, then play audio
		if(m_delayFrames == 0)
		{

			BIZ_ASSERT(m_bankID >= 0 && m_bankID < mMaxNumDialogueEntites);

			if(m_bankID >= 0 && m_bankID < mMaxNumDialogueEntites/* &&  m_DialogueEntityPool[m_bankID].m_entityHandle.IsValid()*/)
			{

#ifdef LOAD_STREAMS_ON_FLY
				//create the entity
				if(!m_DialogueEntityPool[m_bankID].m_entityHandle.IsValid())
				{

					//release anything else that's there already, sanity check for baNK
					if(m_currentBankID >= 0 && m_currentBankID < mMaxNumDialogueEntites)
					{
						if(m_DialogueEntityPool[m_currentBankID].m_entityHandle.IsValid())
						{
							Audio::InterfaceController::Get().ReleaseEntity(m_DialogueEntityPool[m_currentBankID].m_entityHandle);
							m_DialogueEntityPool[m_currentBankID].m_entityHandle.Zero();
							m_DialogueEntityPool[m_currentBankID].m_isActive = false;


						}
					}


					AudioEntityHandle hEntity;

					if(m_DialogueEntityPool[m_bankID].m_entityProxy == NULL)
					{
						BIZ_ASSERT_FAIL("no entity proxy, bank ID was %d", m_bankID);
						return;
					}

					//if for any reason the proxy is invalid
					if(!m_DialogueEntityPool[m_bankID].m_entityProxy->IsValid())
					{
						BIZ_ASSERT_FAIL("entity proxy is invalid, bank ID was %d", m_bankID);
						return;
					}

					Audio::InterfaceController::Get().CreateEntity(m_DialogueEntityPool[m_bankID].m_entityProxy->GetEntityTypeHnd(), hEntity);

					// connect to system entity
					Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eConnectToSystem, 0, &hEntity);

					m_DialogueEntityPool[m_bankID].m_entityHandle = hEntity;
					
					Audio::EntityDataMemberHandle hActionOut = Audio::InterfaceController::Get().LookupInstanceComponent(m_DialogueEntityPool[m_bankID].m_entityHandle, "VOFinished");		

					if(hActionOut != -1)
					{
						Audio::InterfaceController::Get().SetCallback(new Audio::DefaultEntityCallback(m_DialogueEntityPool[m_bankID].m_entityHandle,eAudioStopped), hActionOut);
					}
					else
						LOGWARN(g_InGameDialogueLogger, DBSTR "ActionOut not found for audio dialogue line %d, bank %d. This means sample will not signal it has finished.", m_sampleIndex, m_bankID);
					//set up param out
					

					 char stringBuffer[50];

					 sprintf (stringBuffer, "RaceDialogue_%dSoundActive", m_bankID);


					m_VOPosParamOutHandle= Audio::InterfaceController::Get().LookupInstanceComponent(m_DialogueEntityPool[m_bankID].m_entityHandle, stringBuffer);
					
					


					m_currentBankID = m_bankID;
				}
#endif
				//Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(hEntity, "StartActionIn");

				BIZ_ASSERT(m_DialogueEntityPool[m_bankID].m_entityHandle.IsValid());

				 if(m_DialogueEntityPool[m_bankID].m_entityHandle.IsValid())
				 {

					//Audio::InterfaceController::Get().SuspendEntity(hEntity);
					
					//send the dialogue message
					//resume the entity
					LOGDEBUG(g_InGameDialogueLogger, DBSTR "Audio triggering ActionIn for line %d bank %d.", m_sampleIndex, m_bankID);

					Audio::InterfaceController::Get().ResumeEntity(m_DialogueEntityPool[m_bankID].m_entityHandle);
					m_DialogueEntityPool[m_bankID].m_isActive = true;

					Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_DialogueEntityPool[m_bankID].m_entityHandle, "VOTriggerActionIn");
					BIZ_ASSERT(hActionIn != 0xffffffff);
					Audio::InterfaceController::Get().TriggerEntityActionIn(m_DialogueEntityPool[m_bankID].m_entityHandle, hActionIn);

					hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_DialogueEntityPool[m_bankID].m_entityHandle, "SampleIndex");
					BIZ_ASSERT(hActionIn != 0xffffffff);
					Audio::InterfaceController::Get().SetEntityValue(m_DialogueEntityPool[m_bankID].m_entityHandle, hActionIn,(F32)m_sampleIndex);

					m_VOPosReturnDataHandle = Audio::InterfaceController::Get().TrackValue(m_DialogueEntityPool[m_bankID].m_entityHandle, m_VOPosParamOutHandle);

					m_VOActive = true;
					m_delayFrames--;
					
				 }

				
			}

			m_playDialogue = false;

	

		}


	}

	//send wave posotion
	if(m_VOActive)
	{
		Audio::InterfaceController::Get().GetValue(m_VOPosition, m_VOPosReturnDataHandle);	

	}
	else
		m_VOPosition = 0;



}


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on)
#endif	//	AUDIO_OPTIMIZE_OFF


