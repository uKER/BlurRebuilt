#include "stdafx.h"

#include <Game/Game.h>

#include <GameLogic/InGameStartLineInfoManager.h>

#include <Game/Messaging/GameflowMessages.h>

#include <Game/GameWorld/GameWorld.h>

#include <GameApp.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

#include <Game\gameactors\vehiclelogic\VehicleLogicActor.h>
#include <Game/AmaxCam/CameraSystem.h>
#include <framework/application/ApplicationBase.h>
#include <GameLogic/ActorStudioActors/ActorUtils.h>
#include <GameLogic/ActorStudioActors/StartLineInfoActor.h>
#include <Game\GameWorld\ViewType.h>

#include <Ui/UiInterface.h>
#include <Ui/SoundEffects.h>

//#pragma optimize ("",off)


namespace Game
{

C_InGameStartLineInfoManager C_InGameStartLineInfoManager::s_instance;

void C_InGameStartLineInfoManager::Init()
{
	SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );
	
	AddSubscriber( GameMessaging::E_GameMsg_Update );
	AddSubscriber( E_GameflowMessage_RaceStarted );
	AddSubscriber( Game::E_GameLogicMessage_LapIncremented );
	AddSubscriber( E_GameflowMessage_RaceLoaded );

	// Init called at start of each race, ensure all data cleared. 
	mStartLineInfoTypeStore.Clear();
	mSSPreviousLogicPosition.Clear(); 
	mSSPlayerStartInfo.Clear(); 

	mPreviousLogicPosition=-1;
	mRegistrationOnCreationAllowed = true; 

	mStartLinePassed=false;

	RecheckViewports();

	// Only push back if empty. (should be, but issue when exiting to singleplayer)
	if( mSSPreviousLogicPosition.Empty() && 
		mSSPlayerStartInfo.Empty()			)
	{
		// Initialise Player Start info and logic positions
		for(unsigned int i = 0; i < mNumLocalPlayers; i++)
		{
			mSSPreviousLogicPosition.PushBack(-1.0f); 
			mSSPlayerStartInfo.PushBack(::Actors::StartInfoType_Start);
		}
	}

	// Push back vector to hold start line info for primary player (always created)
	mStartLineInfoTypeStore.PushBack(Btl::Vector<StartLineInfoType>());
}

void C_InGameStartLineInfoManager::Clear()
{
	SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );
	
	RemoveSubscriber( GameMessaging::E_GameMsg_Update );
	RemoveSubscriber( E_GameflowMessage_RaceStarted );
	RemoveSubscriber( Game::E_GameLogicMessage_LapIncremented );
	RemoveSubscriber( E_GameflowMessage_RaceLoaded );

	// Init called at start of each race, ensure all data cleared. 
	mStartLineInfoTypeStore.Clear();
	mSSPreviousLogicPosition.Clear(); 
	mSSPlayerStartInfo.Clear(); 

	mPreviousLogicPosition=-1;
	mRegistrationOnCreationAllowed = true; 
}

void C_InGameStartLineInfoManager::RecheckViewports()
{
	// Test num viewports (not game mode since could be debug mode)
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	if(levelState.mNumViewports > 1)
	{
		mIsSplitScreen = true; 

		// Check for debug screen strangeness (i.e. player setting less cars than viewports)
		if(levelState.mNumCars >= levelState.mNumViewports)
		{
			mNumLocalPlayers = levelState.mNumViewports; 
		}
		else
		{
			mNumLocalPlayers = levelState.mNumCars; 
		}
	}
	else
	{
		mIsSplitScreen = false; 
		mNumLocalPlayers = 1; 
	}
}

void C_InGameStartLineInfoManager::Update()
{
	if (!C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer())
		return;

	if(!mIsSplitScreen)
	{
		if(!mStartLinePassed)
			return;

		// Do Single player Update
		Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

		AmaxEvent* ev = C_GameEventData::Instance().mEvents[(GAMEDATA_ID)levelState.mEvent];
		if(ev && ev->dontShowStartFinishInfo)
			return;

		Actors::C_VehicleLogicActor *pActor=C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(0);

		if ((levelState.mNumViewports > 1) || (pActor == NULL))
			return;

		if(mPreviousLogicPosition == -1)
		{
			return;
		}

		// For each player... get their current state. 

		//test to see if we're half way round the lap
		if(mPreviousLogicPosition <= 0.5f && pActor->GetPositionInfo().mLogicPosition >= 0.5f)
		{
			// Time to change the current state of the start line info
			for(int a=0;a<mStartLineInfoTypeStore[0].Size();a++)
			{
				Actor::C_ActorList* pActorList	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList();
				pActorList->SendMessageToActor(mStartLineInfoTypeStore[0][a].mHandle,ActorMsg::E_GameActorMsg_Deactivate);
			}

			if(levelState.mRaceMode == E_RaceMode_Checkpoint || levelState.mRaceMode == E_RaceMode_FriendDemand_Checkpoint)
			{
				// Checkpoint
				for(int a=0;a<mStartLineInfoTypeStore[0].Size();a++)
				{
					if(mStartLineInfoTypeStore[0][a].mType == ::Actors::StartInfoType_Checkpoint)
					{
						Actor::C_ActorList* pActorList	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList();
						pActorList->SendMessageToActor(mStartLineInfoTypeStore[0][a].mHandle,ActorMsg::E_GameActorMsg_Activate);
					}
				}
			}

			// Finish
			AmaxRoute *route=C_GameData::Instance().mRoutes[(GAMEDATA_ID)levelState.mRoute];
			if(levelState.mLaps == pActor->GetLapInfo().mCurrentLap || ( route && route->onlyAllowsSingleLap == true ) )
			{
				if ((levelState.mRaceMode != Game::E_RaceMode_Destruction) && (levelState.mRaceMode != E_RaceMode_FriendDemand_Destruction))
				{
					for(int a=0;a<mStartLineInfoTypeStore[0].Size();a++)
					{
						if(mStartLineInfoTypeStore[0][a].mType == ::Actors::StartInfoType_Finish)
						{
							Actor::C_ActorList* pActorList	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList();
							pActorList->SendMessageToActor(mStartLineInfoTypeStore[0][a].mHandle,ActorMsg::E_GameActorMsg_Activate);
						}
					}
				}
			} // Final lap
			else if(levelState.mLaps-1 == pActor->GetLapInfo().mCurrentLap && levelState.mRaceMode != E_RaceMode_Checkpoint && levelState.mRaceMode != E_RaceMode_FriendDemand_Checkpoint)
			{
				if ((levelState.mRaceMode != Game::E_RaceMode_Destruction) && (levelState.mRaceMode != E_RaceMode_FriendDemand_Destruction))
				{
					for(int a=0;a<mStartLineInfoTypeStore[0].Size();a++)
					{
						if(mStartLineInfoTypeStore[0][a].mType == ::Actors::StartInfoType_Final)
						{
							Actor::C_ActorList* pActorList	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList();
							pActorList->SendMessageToActor(mStartLineInfoTypeStore[0][a].mHandle,ActorMsg::E_GameActorMsg_Activate);
						}
					}
				}
			}
			else
			{	
				//35798 - Remove checkpoints text from everything - apart from checkpoints
				//if ((levelState.mRaceMode != Game::E_RaceMode_Destruction) && (levelState.mRaceMode != E_RaceMode_FriendDemand_Destruction))
				//{
				//	for(int a=0;a<mStartLineInfoTypeStore[0].Size();a++)
				//	{
				//		if(mStartLineInfoTypeStore[0][a].mType == ::Actors::StartInfoType_Checkpoint)
				//		{
				//			Actor::C_ActorList* pActorList	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList();
				//			pActorList->SendMessageToActor(mStartLineInfoTypeStore[0][a].mHandle,ActorMsg::E_GameActorMsg_Activate);
				//		}
				//	}
				//}

				// Checkpoint
				if(levelState.mRaceMode == E_RaceMode_Checkpoint || levelState.mRaceMode == E_RaceMode_FriendDemand_Checkpoint)
				{
					// Start Finish Checkpoint
					for(int a=0;a<mStartLineInfoTypeStore[0].Size();a++)
					{
						if(mStartLineInfoTypeStore[0][a].mType == ::Actors::StartInfoType_Checkpoint_StartFinish)
						{
							Actor::C_ActorList* pActorList	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList();
							pActorList->SendMessageToActor(mStartLineInfoTypeStore[0][a].mHandle,ActorMsg::E_GameActorMsg_Activate);
						}
					}
				}
			}
		}

		mPreviousLogicPosition=pActor->GetPositionInfo().mLogicPosition;
	}
	else
	{
		// Need to update for multiple vehicles in split screen
		UpdateSplitScreenStartLineInfo(); 
	}
}

// NOTE: Code duplication is BAD, should handle Single player + SS with one update eventually
void C_InGameStartLineInfoManager::UpdateSplitScreenStartLineInfo()
{
	if(!mStartLineInfoTypeStore.Empty() && !mStartLineInfoTypeStore[0].Empty())
	{
		for(unsigned int i = 0; i < mNumLocalPlayers; i++)
		{
			// Need to update each vehicle in split screen mode
			// Do Split screen update
			Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
			Actors::C_VehicleLogicActor *pActor=C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(i);

			if (pActor == NULL)
			{
				BIZ_ASSERT_MSG(pActor," C_InGameStartLineInfoManager::UpdateSplitScreenStartLineInfo() ERROR - pActor == NULL");
				continue;
			}


			const float epsilon = 0.0001f;
			if(fabsf( 1.0f + mSSPreviousLogicPosition[i]) < epsilon ) //defaulted to -1 at start so dont continue till race started
			{
				continue;
			}

			// Also break out if in rolling start
			if( !(C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr())  ||
				!(C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr()->RaceInProgress()) )
			{
				continue;
			}


			// For each player... get their current state. 
			// test to see if we're half way round the lap
			if(mSSPreviousLogicPosition[i] <= 0.5f && pActor->GetPositionInfo().mLogicPosition >= 0.5f)
			{
				// Dont change start line info actors directly.. just record current 'active' value. Actors Can be activated/deactivated as each viewport rendered
				AmaxRoute *route=C_GameData::Instance().mRoutes[(GAMEDATA_ID)levelState.mRoute];
				if((levelState.mLaps == pActor->GetLapInfo().mCurrentLap) || ( route && route->onlyAllowsSingleLap == true ))
				{   // Finish
					mSSPlayerStartInfo[i] = ::Actors::StartInfoType_Finish;
					SetupInfoActorsForLocalPlayer(i);
				} 
				else if(levelState.mLaps-1 == pActor->GetLapInfo().mCurrentLap)
				{	// Final lap
					mSSPlayerStartInfo[i] = ::Actors::StartInfoType_Final;
					SetupInfoActorsForLocalPlayer(i);
				}
				else
				{	// Checkpoint
					mSSPlayerStartInfo[i] = ::Actors::StartInfoType_Checkpoint;
					SetupInfoActorsForLocalPlayer(i);
				}
			}

			mSSPreviousLogicPosition[i] = pActor->GetPositionInfo().mLogicPosition;
		}
	}
}

void C_InGameStartLineInfoManager::SetupInfoActorsForLocalPlayer(unsigned int i)
{
	// For each player ensure that 'their' score info actors are setup correctly
	if(!mStartLineInfoTypeStore.Empty() && !mStartLineInfoTypeStore[0].Empty())
	{
		BIZ_ASSERT_MSG(i < (unsigned int)mStartLineInfoTypeStore.Size(),"C_InGameStartLineInfoManager::SetupInfoActorsForLocalPlayers() - Error no startLineInfoActors for this palyer - something has gone very wrong...");

		// Disable all ready for setting
		Actor::C_ActorList* pActorList	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList();
		for(int j = 0; j < mStartLineInfoTypeStore[i].Size(); j++)
		{
			pActorList->SendMessageToActor(mStartLineInfoTypeStore[i][j].mHandle,ActorMsg::E_GameActorMsg_Deactivate);
		}
		
		// Get the current start info for this player
		::Actors::StartInfoType& startInfoToShowForPlayer = mSSPlayerStartInfo[i]; 

		// Show it
		for(int j = 0; j < mStartLineInfoTypeStore[i].Size(); j++)
		{
			if(mStartLineInfoTypeStore[i][j].mType == startInfoToShowForPlayer)
			{
				Actor::C_ActorList* pActorList	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList();
				pActorList->SendMessageToActor(mStartLineInfoTypeStore[i][j].mHandle,ActorMsg::E_GameActorMsg_Activate);
			}
		}
		
	}
}

void C_InGameStartLineInfoManager::AddStartLineInfo(::Actors::StartInfoType type, Actor::C_ActorHandle handle)
{
	// (flag used so can allow  push back if from level data, and avoid if creating duplicate for split screen)
	if(mRegistrationOnCreationAllowed)
	{
		// Create start line info type 
		StartLineInfoType sli_type;

		sli_type.mHandle=handle;
		sli_type.mType=type;

		mStartLineInfoTypeStore[0].PushBack(sli_type); // Always push back into primary player 'slot'. Duplication for other split screen players comes later. 
	}
}

bool C_InGameStartLineInfoManager::MessageHandler(const  Messaging::C_MessageBase* pMessage)
{
	bool handledMessage = true;

	switch( pMessage->Type() )
	{
		case GameMessaging::E_GameMsg_Update:
			Update();
			break;

		case Game::E_GameLogicMessage_LapIncremented:
			{
				const Game::C_LapIncrementedMsg* pMsg = Messaging::MessageCast<Game::C_LapIncrementedMsg>(pMessage);

				if (pMsg->mVehicleIndex == 0)
				{
					if (pMsg->mLap == 1)
						UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_PassedStartLineActionIn );
					else
					{
						Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

						if (levelState.mRaceMode != E_RaceMode_Destruction && levelState.mRaceMode != E_RaceMode_Checkpoint
							&& levelState.mRaceMode != E_RaceMode_FriendDemand_Destruction && levelState.mRaceMode != E_RaceMode_FriendDemand_Checkpoint )
						{
							if (pMsg->mLap == levelState.mLaps)
								UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_LastLapActionIn );
							else
								UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_LapIncrementedActionIn );
						}
					}
				}

				if(pMsg->mLap > 0)
				{
					mStartLinePassed=true;
				}
			}
			break;

		case E_GameflowMessage_RaceLoaded:
			{
				if(mIsSplitScreen)
				{
					// Time to duplicate the start line info for each player racing (splitscreen)
					CloneStartLineInfoActors(mNumLocalPlayers);

					// Bind clones to each player's viewport (by default actors show on all VPs) 
					BindActorEffectsToViewports(mNumLocalPlayers);
				}
			}
			break;

		case E_GameflowMessage_RaceStarted:
			{
				Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

				if(levelState.mNumViewports == 1)
				{
					AmaxEvent* ev = C_GameEventData::Instance().mEvents[(GAMEDATA_ID)levelState.mEvent];
					if(ev && ev->dontShowStartFinishInfo)
						return true;

					mStartLinePassed=false;
					
					for(int a=0;a<mStartLineInfoTypeStore[0].Size();a++)
					{
						if(mStartLineInfoTypeStore[0][a].mType == ::Actors::StartInfoType_Start)
						{
							Actor::C_ActorList* pActorList	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList();
							pActorList->SendMessageToActor(mStartLineInfoTypeStore[0][a].mHandle,ActorMsg::E_GameActorMsg_Activate);
						}

						if(levelState.mRaceMode == E_RaceMode_Checkpoint || levelState.mRaceMode == E_RaceMode_FriendDemand_Checkpoint)
						{
							// Checkpoint
							if(mStartLineInfoTypeStore[0][a].mType == ::Actors::StartInfoType_Checkpoint)
							{
								Actor::C_ActorList* pActorList	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList();
								pActorList->SendMessageToActor(mStartLineInfoTypeStore[0][a].mHandle,ActorMsg::E_GameActorMsg_Activate);
							}
						}

						AmaxRoute *route=C_GameData::Instance().mRoutes[(GAMEDATA_ID)levelState.mRoute];
						if ( route && route->onlyAllowsSingleLap == true )
						{
							if(mStartLineInfoTypeStore[0][a].mType == ::Actors::StartInfoType_Finish)
							{
								Actor::C_ActorList* pActorList	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList();
								pActorList->SendMessageToActor(mStartLineInfoTypeStore[0][a].mHandle,ActorMsg::E_GameActorMsg_Activate);
							}
						}

					}

					if (Actors::C_VehicleLogicActor *pActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(0))
					{
						mPreviousLogicPosition = pActor->GetPositionInfo().mLogicPosition;
					}
				}
				else
				{
					SetupSplitScreenRaceStart(); 
				}
			}
			break;

		default :
			break;
	}

	return handledMessage;
}

void C_InGameStartLineInfoManager::BindActorEffectsToViewports(const unsigned int numLocalPlayers)
{
	if(!mStartLineInfoTypeStore.Empty() && !mStartLineInfoTypeStore[0].Empty())
	{
		for(unsigned int i = 0; i < numLocalPlayers; i++)
		{
			if(!mStartLineInfoTypeStore[i].Empty())
			{
				// Ensure each info actor is correctly bound to the appropriate viewport
				for(int j = 0; j <  mStartLineInfoTypeStore[i].Size(); j++)
				{
					// Ensure  player's viewport indices for effect are set 
 					::Actors::StartLineInfoActor *pActor = (::Actors::StartLineInfoActor*)(C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetActorPtr(mStartLineInfoTypeStore[i][j].mHandle));

					U32 valueAndMask[2];
					valueAndMask[0] = 1 << (Game::E_ViewType_Main1 + (int)i);
					valueAndMask[1] = Game::E_ViewTypeFlag_Main;
					pActor->m_pSFX->SetParameter(SpecialFX::E_ParamID_SetMeshViewFlags, (void*)valueAndMask);
				}
			}
		}

	}
}

void C_InGameStartLineInfoManager::CloneStartLineInfoActors(const unsigned int numLocalPlayers)
{
	// Ensure when creating duplicates we dont register them on creation! creates infinite loop!
	mRegistrationOnCreationAllowed = false; 

	// Time to duplicate the start line info actors for each player racing 
	if(!mStartLineInfoTypeStore.Empty())
	{
		// If start line info actors have been created in the level
		if(!mStartLineInfoTypeStore[0].Empty())
		{
			// Duplicate them for each additional player (i.e. other than player 0)
			for(unsigned int j = 1; j < mNumLocalPlayers; j++)
			{
				// Create a new vector to hold  new start line info actor data for this player
				Btl::Vector<StartLineInfoType> newStartLineActors; 

				for(int i = 0; i < mStartLineInfoTypeStore[0].Size(); i++)
				{
					// For each actor in the primary player's vector
					// Create a duplicate of the actor in the primary player's vector for each other player
					::Actors::StartLineInfoActor *pActor = (::Actors::StartLineInfoActor*)(C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetActorPtr(mStartLineInfoTypeStore[0][i].mHandle));
					// for each active player Create duplicate using same creation data

					// Create new creation data
					Actor::C_ActorCreationDataFactoryHelper< ::Actors::StartLineInfoCreationData > pStartLineInfoActorCreate;
					const ::Actors::StartLineInfoCreationData* pExistingCreationData = pActor->GetCreationData();
					
					// Setup to match existing
					// StartLineInfoActor create params
					pStartLineInfoActorCreate->mType				   = pExistingCreationData->mType;

					// Effect Generator create params
					pStartLineInfoActorCreate->m_Alignment			   = pExistingCreationData->m_Alignment;
					pStartLineInfoActorCreate->m_StartDeactivated	   = pExistingCreationData->m_StartDeactivated;
					pStartLineInfoActorCreate->m_SpecialEffect		   = pExistingCreationData->m_SpecialEffect;
					pStartLineInfoActorCreate->m_T1LoopStart		   = pExistingCreationData->m_T1LoopStart;
					pStartLineInfoActorCreate->m_T2LoopEnd			   = pExistingCreationData->m_T2LoopEnd;
					pStartLineInfoActorCreate->m_StartTime			   = pExistingCreationData->m_StartTime;
					pStartLineInfoActorCreate->m_LoopType			   = pExistingCreationData->m_LoopType;

					// C_PosActorCreate  params
					pStartLineInfoActorCreate->m_WorldMatrix		   = pExistingCreationData->m_WorldMatrix;
					
					// C_GameActorCreate params
					pStartLineInfoActorCreate->m_Name				   = pExistingCreationData->m_Name;

					// C_ActorCreationData params
					// DONT copy sorcery ID ( advice from Oscar Cooper )

					// Store
					StartLineInfoType		sli_type;
					sli_type.mHandle  =		pActor->GetActorList()->Spawn(pStartLineInfoActorCreate.Get());
					sli_type.mType	  =		pExistingCreationData->mType;

					newStartLineActors.PushBack(sli_type); // push back into additional player 'slot'.

				}

				// Done.. push back the new vector of cloned actors
				mStartLineInfoTypeStore.PushBack(newStartLineActors); 
			}
		}
	}

	// Finished allow registration again
	mRegistrationOnCreationAllowed = true; 
}

void C_InGameStartLineInfoManager::SetupSplitScreenRaceStart()
{
	if(!mStartLineInfoTypeStore.Empty() && !mStartLineInfoTypeStore[0].Empty())
	{
		for(unsigned int i = 0; i < mNumLocalPlayers; i++)
		{
			SetupInfoActorsForLocalPlayer(i);

			if (Actors::C_VehicleLogicActor *pActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(i))
			{
				mSSPreviousLogicPosition[i] = pActor->GetPositionInfo().mLogicPosition;
			}
		}
	}
}

}; // namespace Game
