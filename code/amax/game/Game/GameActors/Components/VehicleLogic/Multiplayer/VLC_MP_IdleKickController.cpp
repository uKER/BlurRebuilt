#include "stdafx.h"

#include <Game/GameActors/Components/GameComponents.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_MP_IdleKickController.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game\Game.h>
#include <Game\GameControls\GameControls.h>
#include <Game/Messaging/GameflowMessages.h>
#include <Game\Messaging\VehicleActorMessages.h>

#include <GameApp.h>

#include <GameLogic\Structures\LevelState.h>
#include <GameLogic\Structures\LevelStateConstants.h>

#include <input\Device.h>
#include <input\GameInput.h>
#include <framework\application\ApplicationBase.h>

#include <ui/Game/Gadgets/ButtonMessages.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>

#include <GameLogic/GameData/MultiplayerData.h>

#include <GameLogic\Structures\MpResults.h>
 
#if !defined(CFG_GOLD) || defined(CFG_HACKED_MASTER)
	#if defined(CFG_HACKED_MASTER)
Debug::Bool g_DisableTimeoutComponent("Multiplayer.IdleTimeout.DisableTimoutComponent_", true );
Debug::Bool g_DisableInactivityGhosting("Multiplayer.IdleTimeout.DisableInactivityGhosting", true ); 
	#else
Debug::Bool g_DisableTimeoutComponent("Multiplayer.IdleTimeout.DisableTimoutComponent", false, Debug::Persist() );
Debug::Bool g_DisableInactivityGhosting("Multiplayer.IdleTimeout.DisableInactivityGhosting", false ); 
	#endif
#else
Debug::Bool g_DisableTimeoutComponent("Multiplayer.IdleTimeout.DisableTimoutComponent", false, Debug::Persist() );
Debug::Bool g_DisableInactivityGhosting("Multiplayer.IdleTimeout.DisableInactivityGhosting", false ); 
#endif
extern Debug::Bool g_PlayerVehiclesUnderAiControl;

Debug::Bool g_UseDebugInactiveTimeLimit("Multiplayer.IdleTimeout.UseDebugInactiveTimeLimit", false);
Debug::Float g_GhostInactiveTimeLimit("Multiplayer.IdleTimeout.GhostInactiveTimeLimit", 30.0f ); 
Debug::Bool g_ReCacheIdleKickControllerParameters("Multiplayer.IdleTimeout.ReCacheParameters", false ); 
Debug::Bool g_ForceIdleGhost("Multiplayer.IdleTimeout.ForceIdleGhost", false); 

extern Debug::Float g_DefaultGhostAlpha;

namespace Components
{
	const float MP_LISTEN_DELAY_TIME	= 1.0f;		 // small delay to avoid detecting input on first frame component is listening 
	const float MP_MINIMUM_VALID_EVENT_TIME = 30.0f; // Only kick if player has actually had decent chance to provide input (what if 2 player race and P2 quits after 5 secs etc)
	const float MP_VERY_LONG_GHOST_TIME = 99999.0f;

bool C_VehicleComponent_MP_IdleKickController::CreateImpl()
{
	const I_VehicleComponent_MP_IdleKickController_CreateInfo* creation_data = static_cast<const I_VehicleComponent_MP_IdleKickController_CreateInfo *>(GetCreator().GetData());

	if(creation_data)
	{
		mVehicleIndex=creation_data->mVehicleIndex;
		mRaceLogicLayerPtr=creation_data->mRaceLogicLayerPtr;
	}

	mRaceLogicCorePtr = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
	BIZ_ASSERT_MSG(mRaceLogicCorePtr, "C_VehicleComponent_MP_IdleKickController::CreateImpl() < mRaceLogicCorePtr invalid");
	mNetGameVars = &C_GameApp::GetInstance()->GetGame()->GetNetGameVars();

	AddSubscriber( GameMessaging::E_GameMsg_Update );
	AddSubscriber( Game::E_GameflowMessage_RaceStarted );
	AddSubscriber( Game::E_GameflowMessage_VehicleFinished );	// Need this since once vehicle finished AI takes over.. So wont be any input! (not exacty fair to kick the player that won the race!)

	mRaceStarted = false;
	mPlayerImmuneToKick = false;
	mListenDelayTimer = MP_LISTEN_DELAY_TIME;
	mTotalTimeListening = 0.0f;

	mPlayerCurrentlyGhosted			= false;
	mTimeSinceLastControllerInput	= 0.0f;
	
	CacheParameters();

	return true;
}

C_VehicleComponent_MP_IdleKickController::~C_VehicleComponent_MP_IdleKickController()
{

}

void C_VehicleComponent_MP_IdleKickController::Update()
{

	// Dont continue if debug flag says disabled
#if !defined(CFG_GOLD) || defined(CFG_HACKED_MASTER)
	if(g_DisableTimeoutComponent || g_PlayerVehiclesUnderAiControl)
	{
		mPlayerImmuneToKick = true;
		return;
	}
#endif //CFG_GOLD

#ifndef CFG_GOLD
	UpdateDebugFunctionality();
#endif //CFG_GOLD


	// Dont continue if PC version (dont bother idle kicking on pc)
	// For now dont bother ghosting either
#ifdef PLATFORM_PC
	mPlayerImmuneToKick = true;
#endif //PLATFORM_PC

	if(!mRaceStarted)
	{
		return;
	}

	// Small delay before recognise 'new' input
	if(	mListenDelayTimer > 0.0f )
	{
		mListenDelayTimer -= mDeltaTime; 
		if(mListenDelayTimer < 0.0f)
		{
			mListenDelayTimer = 0.0f;
		}
		return;
	}

	// Increment time listening
	mTotalTimeListening += mDeltaTime; 

	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
	BIZ_ASSERT_MSG(pLogicActor, "C_VehicleComponent_MP_IdleKickController::GetIsPlayerImmuneToIdleKick() < pLogicActor Invalid. We have far more serious things to worry about than kicking the player...");
	if(!pLogicActor)
	{
		return; 
	}

	//	If this is an invalid controller index, bail out
	U8 controlNum = levelState.mCars[mVehicleIndex].mController;
	if(controlNum == Game::INVALID_CONTROLLER)
	{
		return;
	}

	//	Worth checking the pad is actually connected (360 pad could go to sleep or something...)
	bool gotInput = CheckForControllerInput(controlNum) && !g_ForceIdleGhost;

	// if got input , the player is safe!
	if(gotInput)
	{
		// We are done, player can't be kicked. 
		mPlayerImmuneToKick = true;

		// reset time since last input
		mTimeSinceLastControllerInput = 0.0f;

		// Make sure the player is in control and not ghosted
		HandlePlayerNotGhosted(pLogicActor); 
	}
	else
	{
		// If have not got input, increment time since last input and check if time to ghost player
		mTimeSinceLastControllerInput +=  mDeltaTime;
		if( (mTimeSinceLastControllerInput >= mGhostInactiveTimeLimit) || g_ForceIdleGhost)
		{
			HandlePlayerGhosted(pLogicActor); 
		}
	}
}

void C_VehicleComponent_MP_IdleKickController::HandlePlayerNotGhosted(Game::Actors::C_VehicleLogicActor* pLogicActor)
{

#if !defined(CFG_GOLD) || defined(CFG_HACKED_MASTER)
	if(g_DisableInactivityGhosting)
	{
		return;
	}
#endif // CFG_GOLD

	// If player WAS ghosted at this point, unghost them, give them control back, and let the other network players know!
	if(mPlayerCurrentlyGhosted)
	{
		// Unghost the car (in  a time)
		pLogicActor->Ghost(0.01f, g_DefaultGhostAlpha);

		// Check that deghosting was successful! (may have been inside another car and avoided our command!)
		U8 vehIndex = pLogicActor->GetVehicleIndex();
		
		// Set flag so we dont try to DeGhost again
		mPlayerCurrentlyGhosted = false;

		// Give control back
		Game::Actors::C_VehicleActor* pVehicleActor = mRaceLogicCorePtr->GetVehicleActor(vehIndex);
		pVehicleActor->GetVehiclePhysics()->Controls().Clear();
		Game::Actors::C_VehicleControlsEnableMsg enableControlsMsg(pVehicleActor->GetHandle(), vehIndex);
		pVehicleActor->SendMessageToActor(pVehicleActor->GetHandle(), &enableControlsMsg);

		mNetGameVars->m_EventManager->Add_Idle(Game::GetJoinIdFromLocalIndex(vehIndex), false);
		Game::C_MpResults::Instance().m_LiveVehicleData[vehIndex].m_Idle=false;
	}
}

void C_VehicleComponent_MP_IdleKickController::HandlePlayerGhosted(Game::Actors::C_VehicleLogicActor* pLogicActor)
{

#if !defined(CFG_GOLD) || defined(CFG_HACKED_MASTER)
	if(g_DisableInactivityGhosting)
	{
		return;
	}
#endif // CFG_GOLD

	if(!mPlayerCurrentlyGhosted)
	{
		// ghost the car
		pLogicActor->Ghost(MP_VERY_LONG_GHOST_TIME, g_DefaultGhostAlpha);

		// put on handbrake and disable controls
		U8 vehIndex = pLogicActor->GetVehicleIndex();
		Game::Actors::C_VehicleActor* pVehicleActor = mRaceLogicCorePtr->GetVehicleActor(vehIndex);
		pVehicleActor->GetVehiclePhysics()->Controls().Clear();
		pVehicleActor->GetVehiclePhysics()->Controls().Hold();
		Game::Actors::C_VehicleControlsDisableMsg disableControlsMsg(vehIndex);
		pVehicleActor->SendMessageToActor(pVehicleActor->GetHandle(), &disableControlsMsg);

		// NOTE: do we need to disable HOG ?

		// Set flag so we dont do this again. 
		mPlayerCurrentlyGhosted = true;

		mNetGameVars->m_EventManager->Add_Idle(Game::GetJoinIdFromLocalIndex(vehIndex), true);
		Game::C_MpResults::Instance().m_LiveVehicleData[vehIndex].m_Idle=true;
	}
}

void C_VehicleComponent_MP_IdleKickController::KickPlayerFromGame()
{
	// Set flag that can be picked up
	mNetGameVars->m_InactivePlayer = true;

	// Done
	mRaceStarted = false; 
}

bool C_VehicleComponent_MP_IdleKickController::CheckForControllerInput(const U8 controlNum) const
{
	Input::C_GameInput* pInput = Input::GetGameInputManager()->GetGameInput(controlNum);

	bool gotInput = false;
	if(Framework::C_ApplicationBase::GetInstance()->GetPad(controlNum)->IsConnected())
	{
		for(U32 i = 0; i < (Game::E_UiButton_Count); i++)
		{
			if (pInput->GetButtonPressedDebounced((Game::E_UiButton)i))
			{
				gotInput = true;
				break;
			}
		}
	}

	// If reach here no input
	return gotInput; 
}

void C_VehicleComponent_MP_IdleKickController::CacheParameters()
{
	if(g_UseDebugInactiveTimeLimit)
	{
		mGhostInactiveTimeLimit = g_GhostInactiveTimeLimit;
	}
	else
	{
		MultiplayerConfig* pConfig = C_MultiplayerData::Instance().m_pConfig;
		if(Game::C_Game::GetInstance()->GetLevelState().mRaceMode == Game::E_RaceMode_Multiplayer_Destruction)
		{
			//Motormash
			mGhostInactiveTimeLimit = pConfig->m_IdleTimeoutData.m_MotorMashIdleTimeout;
		}
		else
		{
			//Not Motormash
			mGhostInactiveTimeLimit = pConfig->m_IdleTimeoutData.m_RacingIdleTimeout;
		}
	}
}

#ifndef CFG_GOLD
void C_VehicleComponent_MP_IdleKickController::UpdateDebugFunctionality()
{
	if(g_ReCacheIdleKickControllerParameters)
	{
		CacheParameters();

		g_ReCacheIdleKickControllerParameters = false;
	}
}
#endif //CFG_GOLD


bool C_VehicleComponent_MP_IdleKickController::OnMessage( const Messaging::C_MessageBase *pMessage )
{
	switch(pMessage->Type())
	{
	case GameMessaging::E_GameMsg_Update:
		{
			mDeltaTime = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue();
			Update();
			return true;
		}
		break;

	case Game::E_GameflowMessage_RaceStarted:
		{
			mRaceStarted=true;
			mListenDelayTimer = MP_LISTEN_DELAY_TIME;
			return true;
		}
		break;

	case Game::E_GameflowMessage_VehicleFinished:
		{
			// Received whenever ANY finish type happens (e.g. wrecked / eliminated / finish etc)
			mRaceStarted = false;

			// Has the player provided input? And did the player have a large enough time period in
			// which to make input?
			if(!mPlayerImmuneToKick && mTotalTimeListening >= MP_MINIMUM_VALID_EVENT_TIME)
			{
				KickPlayerFromGame(); 
			}
			return true;
		}
		break;
	}

	return I_GameActorComponent::OnMessage(pMessage);
}

C_VehicleComponent_MP_IdleKickController::C_VehicleComponent_MP_IdleKickController():
	mNetGameVars(0)
{
	
}

void C_VehicleComponent_MP_IdleKickController::SetHandler( BaseMessageHandler* pHandler )
{
	mMessageHandler = pHandler;
}

} // namespace Components
