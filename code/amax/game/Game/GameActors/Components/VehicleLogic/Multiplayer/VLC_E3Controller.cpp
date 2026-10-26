#include "stdafx.h"

#include <Game/GameActors/Components/GameComponents.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_E3Controller.h>
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

namespace Components
{

	const F32 E3_CONTROL_TIMEOUT = 5.0f;	//	X seconds of inactivity results in AI taking control of vehicle
	const F32 E3_INITIAL_TIMEOUT = 1.5f;	//	X seconds of initial inactivity from start of race before AI takes control

bool C_VehicleComponent_E3Controller::CreateImpl()
{
	const I_VehicleComponent_E3Controller_CreateInfo* creation_data = static_cast<const I_VehicleComponent_E3Controller_CreateInfo *>(GetCreator().GetData());

	if(creation_data)
	{
		mVehicleIndex=creation_data->mVehicleIndex;
		mRaceLogicLayerPtr=creation_data->mRaceLogicLayerPtr;
	}

	AddSubscriber( GameMessaging::E_GameMsg_Update );
	AddSubscriber( Game::E_GameflowMessage_RaceStarted );
	AddSubscriber( Game::E_GameflowMessage_VehicleFinished );

	mRaceStarted = false;

	mIdleTimer = E3_CONTROL_TIMEOUT;

	return true;
}

C_VehicleComponent_E3Controller::~C_VehicleComponent_E3Controller()
{

}

void C_VehicleComponent_E3Controller::Update()
{
	if(!mRaceStarted)
		return;

	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	U8 controlNum = levelState.mCars[mVehicleIndex].mController;
	//	If this is an invalid controller index, bail out
	if(controlNum == Game::INVALID_CONTROLLER)
		return;

	Input::C_GameInput* pInput = Input::GetGameInputManager()->GetGameInput(controlNum);

	bool gotInput = false;

	//	Worth checking the pad is actually connected (360 pad could go to sleep or something...)
	if(Framework::C_ApplicationBase::GetInstance()->GetPad(controlNum)->IsConnected())
	{
		for(U32 i = 0; i < (Game::E_UiButton_Count); i++)
		{
			if (pInput->GetButtonPressed((Game::E_UiButton)i))
			{
				gotInput = true;
				break;
			}
		}
		if(!gotInput)
		{
			for(U32 i = 0; i < (U32)Game::E_UiAxis_Count; i++)
			{
				if(pInput->GetAxisValue((Game::E_UiAxis)i) != 0.5f)
				{
					gotInput = true;
					break;
				}
			}
		}
	}

	Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
	if(pLogicActor->m_ToggledToAiControl)
	{
		if(gotInput)
		{
			//	Player vehicle is currently under AI control, so if there is any input we need to switch back to player control.
			mIdleTimer = E3_CONTROL_TIMEOUT;

			//	Inform the UI that the player would like to play again, Pretty please
			_UI_MESSAGE( Game::E_GameflowMessage_E3ControllerInput );
		}
	}
	else
	{
		//	Under player control - if there's no input we need to decrement our timeout timer.
		if(!gotInput)
		{
			mIdleTimer -= mDeltaTime;
			if(mIdleTimer <= 0.0f)
			{
				mIdleTimer = 0.0f;

				if(pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Rumble))
					pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Rumble, E_VehicleLogicMsg_DisableRumble);

				pLogicActor->m_ToggledToAiControl = true;
				Game::Actors::C_VehicleForceAIControlMsg forceAiControlMsg(mVehicleIndex);
				pLogicActor->SendMessageToActor(pLogicActor->GetGraphicalActor(mVehicleIndex)->GetHandle(), &forceAiControlMsg);

				//	Inform the UI that the AI... have TAKEN OVER oh no
				_UI_MESSAGE( Game::E_GameflowMessage_E3ControllerInput );
			}
		}
		else
		{
			mIdleTimer = E3_CONTROL_TIMEOUT;
		}
	}
}

bool C_VehicleComponent_E3Controller::OnMessage( const Messaging::C_MessageBase *pMessage )
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
			mIdleTimer = E3_INITIAL_TIMEOUT; 
			return true;
		}
		break;

	case Game::E_GameflowMessage_VehicleFinished:
		{
			Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
			pLogicActor->m_ToggledToAiControl = false;
			mRaceStarted = false;

			return true;
		}
		break;

	case Game::E_GameflowMessage_ToggleToPlayer:
		{
			mIdleTimer = E3_CONTROL_TIMEOUT;

			Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
			pLogicActor->m_ToggledToAiControl = false;

			if(pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Rumble))
				pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Rumble, E_VehicleLogicMsg_EnableRumble);

			Game::Actors::C_VehicleForceHumanControlMsg forceHumanControlMsg(mVehicleIndex);
			pLogicActor->SendMessageToActor(pLogicActor->GetGraphicalActor(mVehicleIndex)->GetHandle(), &forceHumanControlMsg);

			return true;
		}
		break;
	}

	return I_GameActorComponent::OnMessage(pMessage);
}

} // namespace Components
