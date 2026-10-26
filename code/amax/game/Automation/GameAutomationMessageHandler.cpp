
#include "stdafx.h"

#include <Game/Game.h>
#include <Game/Messaging/GameflowMessages.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <misc_shared/Messaging/MessageBase.h>
#include <ui/Game/UIScreenControl.h>
#include <ui/Game/Gadgets/ButtonMessages.h>
#include <System/Automation/AutomationMessages.h>

#include "GameAutomationMessageHandler.h"

#include "Game/AmaxCam/CameraSwitcher.h"
#include "Game/AmaxCam/CameraSystem.h"
#include <game/AmaxCam/VehicleFeed.h>

#include <GameLogic\Profile\GameProfileManager.h>
#include <RaceBook/RaceBookManager.h>
#include <GameLogic/Layers/RaceBookLayer.h>

#include "GameLogic/Layers/RaceLogicLayer.h"

#include "GameLogic/ActorStudioActors/ActorUtils.h"

#include <GameApp.h>

#include <input\Device.h>
#include <input\Pad360.h>
#include <input\PadVirtual.h>
#include <input\GameInput.h>

#include <Game/GameControls/GameControls.h>
#include <camera2/Camera.h>
#include <camera2/stock/DebugFeed.h>

static Debug::Logger &g_Log = Debug::GetLogger("automation");

namespace Game
{
	extern bool gAllowSavingDebugMenuValues;
}

namespace AmaxPerfLogging
{
	extern int g_uniqueLevelObjects;
}

void GameAutomation_BroadcastMessageClass(const Automation::ProblemString& n, int i, const Automation::ProblemString& area)
{
	Automation::C_SendProblemFoundMsg msg(n, i, area);

	Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&msg);
}

GameAutomationMessageController::GameAutomationMessageController( Messaging::C_MessageSubscriberSystem* pSubscriberSystem )
{
#ifndef CFG_GOLD
	SetSubscriberSystem(pSubscriberSystem);

	AddSubscriber(Game::E_GameflowMessage_StartGameRendering);
	AddSubscriber(Game::E_GameflowMessage_UILoaded);
	AddSubscriber(Game::E_GameflowMessage_RaceStarted);
	AddSubscriber(Game::E_GameflowMessage_RaceFinished);
	AddSubscriber(Game::E_GameLogicMessage_LapIncremented);

	AddSubscriber(Automation::E_AutomationMessage_ProblemFound);

	// Set up callbacks for Automation system to send messages to our system
	Automation::SetBroadcastMessageClass(GameAutomation_BroadcastMessageClass);
#endif
}

GameAutomationMessageController::~GameAutomationMessageController()
{
}

//	//mMessageType.PushBack(GAMEUI_MESSAGE_TIME_LAP);
//	//mMessageType.PushBack(GAMEUI_MESSAGE_TIME_SPLIT);
//	//mMessageType.PushBack(GAMEUI_MESSAGE_TIME_FASTEST_LAP);
//	//mMessageType.PushBack(GAMEUI_MESSAGE_GAME_RACE_STARTED);
//	//mMessageType.PushBack(GAMEUI_MESSAGE_GAME_END_SESSION);
//	//mMessageType.PushBack(GAMEUI_MESSAGE_GAME_TIMEATTACK_SETUP_INFO_RECEIVED);

//#pragma optimize("", off)

bool GameAutomationMessageController::MessageHandler( const Messaging::C_MessageBase *pMessage )
{
	bool handledMessage = true;

	switch (pMessage->Type())
	{
	//case Game::E_GameflowMessage_RaceLoaded:
	case Game::E_GameflowMessage_StartGameRendering:
		Automation::Notify("RaceLoaded");
		break;

	case Game::E_GameflowMessage_UILoaded:
		Automation::Notify("UiLoaded");
		break;

	case Game::E_GameflowMessage_RaceStarted:
		Automation::Notify("RaceStarted");
		break;

	case Game::E_GameflowMessage_RaceFinished:
		Automation::Notify("RaceFinished");
		break;

	case Game::E_GameLogicMessage_LapIncremented:
		{
			const Game::C_LapIncrementedMsg* pMsg = Messaging::MessageCast<Game::C_LapIncrementedMsg>(pMessage);

			if (pMsg->mVehicleIndex == 0)
			{
				unsigned int currentLap = pMsg->mLap-1;

				Util::C_String32 notifyLap;
				notifyLap.Printf("LapInc:0:%d", currentLap);
				Automation::Notify(notifyLap.Chars());
			}
			break;
		}

	case Automation::E_AutomationMessage_ProblemFound:
		{
			const Automation::C_SendProblemFoundMsg* msg = (Automation::C_SendProblemFoundMsg*)pMessage;

			Util::C_String text = Util::C_String::Format("%s:%s:%d", msg->name.Chars(), msg->area.Chars(), msg->index);

			Automation::Problem(text.Chars());
		
			int mrf = 10;
		}
		break;

	default:
		handledMessage = false;
		break;
	}

	return handledMessage;
}

//#pragma optimize("", on)

//void GameAutomationMessageController::SendOnMessage(GameUIMessage *message)
//{
//#ifndef CFG_GOLD
//#ifdef _MSTEST_
//	if (message->mMessageType == GAMEUI_MESSAGE_TIME_LAP)
//	{
//		const float lapTime = ((GameUITimeMessage *)message->mData)->mTime;
//		const float lapNum = ((GameUITimeMessage *)message->mData)->mLap;
//		const float raceTime = ((GameUITimeMessage *)message->mData)->mRaceTime;
//		const int vehicleId = ((GameUITimeMessage *)message->mData)->mVehicleId;
//
//		char buffer[128];
//		sprintf_s(buffer, "LapCompleted %d %d %d %d %d %d", vehicleId, lapNum, gRaceState.m_info.m_wallCollisions, gRaceState.m_info.m_riderFallenOff, (int)lapTime, (int)raceTime);
//
//		gRaceState.m_info.Reset();
//		
//		MSTest::DebugChannel::Instance().PrintGlobalEvent(buffer);
//	}
//	else if (message->mMessageType == GAMEUI_MESSAGE_TIME_SPLIT)
//	{
//		const float lapTime = ((GameUITimeMessage *)message->mData)->mTime;
//		const int splitNum = ((GameUITimeMessage *)message->mData)->mSplit;
//		const float raceTime = ((GameUITimeMessage *)message->mData)->mRaceTime;
//		const int vehicleId = ((GameUITimeMessage *)message->mData)->mVehicleId;
//
//		char buffer[128];
//		sprintf_s(buffer, "SplitPassed %d %d %d %d", vehicleId, splitNum, (int)lapTime, (int)raceTime);
//		
//		MSTest::DebugChannel::Instance().PrintGlobalEvent(buffer);
//	}
//#endif
//#endif
//}

//void Automation_SendMessageToDebugChannel(const char* message)
//{
//#ifdef _MSTEST_
//	MSTest::DebugChannel::Instance().PrintGlobalEvent(message);
//#endif
//}

namespace GameAutomation
{

bool Callback(Automation::CommandContext& commandContext)
{
#ifndef CFG_GOLD
	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	if (commandContext.IsCommand("StartEvent"))
	{
		LOGINFO(g_Log, "Automation: StartEvent");

		Game::gAllowSavingDebugMenuValues = false;

		levelState.mDebugEvent = true;

		if (levelState.mAIController == NULL)
		{
			levelState.mAIController = RLDatabase::Instance().mRLSystems[0]->mAIPickupController->mControllers[0].mController.Object();
		}

		if (levelState.mNumCars > 0)
		{
			levelState.mCars[0].mAutomatedHuman = true;
		}

		levelState.Finished();

		const S32 mask = UI::C_ScreenControlManager::Instance().MessageMask();

		_UI_MESSAGE2_MASK( UI::E_UiMsg_AutomationLoadGame, 0, 1, mask );
	}
	else if (commandContext.IsCommand("EndEvent"))
	{
		LOGINFO(g_Log, "Automation: EndEvent");

		Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(Game::E_GameflowMessage_QuitRace);

		UI::gUIScreenControl.SetNextScreen("Loading\\LoadingUI.lua");
	}
	else if (!strcmp(commandContext.Command(), "Reset"))
	{
		levelState.Clear();
	}
	else if (commandContext.IsCommand("EventId"))
	{
		ArteryU32 eventID, vehicleID;
		if (commandContext.MessageReader().Read(eventID) &&
			commandContext.MessageReader().Read(vehicleID))
		{
			C_GameApp::GetInstance()->GetRaceBookLayer()->SetGameMode(Game::E_GameMode_SinglePlayer);

			// Set Event
			levelState.mPlayerIsAI = true;
			Game::C_GameProfileManager::Instance().GetPrimaryProfile()->mCurrentEventId = eventID;
			
			// Set Player Car
			RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->mCurrentCarID = vehicleID;
			C_GameApp::GetInstance()->GetRaceBookLayer()->SetupRace(NULL); // pState);
		}
		else
			return false;
	}
	else if (commandContext.IsCommand("RouteId"))
	{
		ArteryU32 routeID;
		if (commandContext.MessageReader().Read(routeID))
		{
			AmaxRoute* route = C_GameData::Instance().mRoutes[(GAMEDATA_ID)routeID];

			levelState.mCity = route->city->ID;
			levelState.mRoute = routeID;
		}
		else
			return false;
	}
	else if (commandContext.IsCommand("NumVehicles"))
	{
		ArteryI32 numVehicles;
		if (commandContext.MessageReader().Read(numVehicles))
		{
			levelState.mNumCars = numVehicles;
		}
		else
			return false;
	}
	else if (commandContext.IsCommand("Laps"))
	{
		ArteryI32 numLaps;
		if (commandContext.MessageReader().Read(numLaps))
		{
			levelState.mLaps = numLaps;
		}
		else
			return false;
	}
	else if (commandContext.IsCommand("Viewports"))
	{
		ArteryI32 numViewports;
		if (commandContext.MessageReader().Read(numViewports))
		{
			levelState.mNumViewports = numViewports;
		}
		else
			return false;
	}
	else if (commandContext.IsCommand("Vehicle"))
	{
		ArteryI32 vehicleIndex;
		if (commandContext.MessageReader().Read(vehicleIndex))
		{
			ArteryU32 id, controlType, startPos;
			if (commandContext.MessageReader().Read(id) &&
				commandContext.MessageReader().Read(controlType) &&
				commandContext.MessageReader().Read(startPos))
			{
				Game::LevelStateCar& vehicle = levelState.mCars[vehicleIndex];
				
				vehicle.mCarID = id;
				vehicle.mCharacterID = C_RaceBookData::Instance().mCharacters[0]->ID;
				vehicle.mControlType = (E_VEHICLE_CONTROL_TYPE)controlType;
				vehicle.mStartPosition = startPos;

				vehicle.mUpgrades.DebugValues();
			}
			else
				return false;
		}
		else
			return false;
	}
	else if (commandContext.IsCommand("CarCam"))
	{
		ArteryI32 cameraIndex;
		if (commandContext.MessageReader().Read(cameraIndex))
		{
			AmaxCam::Switcher().SetType(AmaxCam::E_DebugCam_VehicleFeed);
			AmaxCam::CameraSystem().VehicleFeed()->ChangeCamera(cameraIndex);
		}
	}
	else if (commandContext.IsCommand("TopCam"))
	{
			const Game::RaceLogicCarInfo& ci = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->mVehiclePositions[0];

			AmaxCam::Switcher().SetType(AmaxCam::E_DebugCam_FlyCamera);

			Cam2::C_Camera cam;
			Maths::C_Quaternion xrot(Maths::HVec3(1.0f, 0.0f, 0.0f), -90.f/57.3f);
			Maths::C_Quaternion yrot(Maths::HVec3(0.0f, 1.0f, 0.0f), 90.f/57.3f);

			AmaxCam::Switcher().SetType(AmaxCam::E_DebugCam_FlyCamera);
			cam.SetPosition(ci.mPosition + Maths::HVec3(0,4,0));
			cam.SetOrient(yrot * xrot);

			AmaxCam::CameraSystem().DebugFeed(0).CopyFrom(cam);
	}
#if defined(PLATFORM_PC)
	else if (commandContext.IsCommand("CreateVirtual360Pad"))
	{
		ArteryI32 noOfControllers;
		//Get the button pressed
		if (commandContext.MessageReader().Read(noOfControllers))
		{
			for(S16 controller = 0; controller < noOfControllers; controller++)
			{
				Game::SetupAmaxUserControls( Input::GetGameInput(controller), 0, 0, Input::C_VirtualPadHandler::GetVirtualPad(controller) );
			}
		}
	}
	else if (commandContext.IsCommand("360ResetControls"))
	{
		Input::C_VirtualPadHandler::GetVirtualPadState()->Reset();
	}
	//Received a pressed "normal" button
	else if (commandContext.IsCommand("360Button"))
	{
		ArteryI32 button;
		F32 testVal = 0.5;
		//Get the button pressed
		if (commandContext.MessageReader().Read(button))
		{
			button-=1;
			Input::C_VirtualPadHandler::GetVirtualPadState()->SetButtonValue(button, 1);
		}
	}
	//Received a pressed pressure button
	else if (commandContext.IsCommand("360ButtonP"))
	{
		ArteryI32 button;
		//Get the button pressed
		if (commandContext.MessageReader().Read(button))
		{
			button -= 1;
			//And the value of the pressure applied
			ArteryU32 value;
			if (commandContext.MessageReader().Read(value))
			{
				// TODO: U32 used with float value
				Input::C_VirtualPadHandler::GetVirtualPadState()->SetButtonValue(button, value);
			}
			else
			{
				Input::C_VirtualPadHandler::GetVirtualPadState()->SetButtonValue(button, 0);
			}
		}
	}
	//Received an axis
	else if (commandContext.IsCommand("360Axis"))
	{
		ArteryI32 axis;
		//Get the button pressed
		if (commandContext.MessageReader().Read(axis))
		{
			axis-=1;
			//And the value of the pressure applied
			ArteryI32 value;
			if (commandContext.MessageReader().Read(value))
			{
				Input::C_VirtualPadHandler::GetVirtualPadState()->SetAxisValue(axis, value);
			}
			else
			{
				Input::C_VirtualPadHandler::GetVirtualPadState()->SetAxisValue(axis, 0);
			}
		}
	}
	else if (commandContext.IsCommand("Disconnect360Controllers"))
	{
		Input::C_VirtualPadHandler::DisconnectVirtualPads();

		Input::C_Device* pad = new Input::C_Pad360(Input::GetDeviceManager(),0);
		Input::GetDeviceManager()->CreateDevice(0, Input::E_Device_Pad360, &pad);
		Game::SetupAmaxUserControls( Input::GetGameInput(0), 0, 0, pad );
	}

#endif
#if defined(PLATFORM_PS3)
	//PS3 pad support stuff

	//Create a given number of virtual PS3 pads
	else if (commandContext.IsCommand("CreateVirtualPS3Pad"))
	{
		ArteryI32 noOfControllers;
		//Get the button pressed
		if (commandContext.MessageReader().Read(noOfControllers))
		{
			for(S16 controller = 0; controller < noOfControllers; controller++)
			{
				Framework::SetupApplicationUserControls(Input::GetGameInput(controller), 0, 0, Input::C_VirtualPadHandler::GetVirtualPad(controller));
				Game::SetupAmaxUserControls( Input::GetGameInput(controller), 0, 0, Input::C_VirtualPadHandler::GetVirtualPad(controller) );
			}
		}
	}
	else if (commandContext.IsCommand("PS3ResetControls"))
	{
		//ArteryI32 number;
		////Get the pad number
		//if (commandContext.MessageReader().Read(number))
		//{
		//	
		//}
		Input::C_VirtualPadHandler::GetVirtualPadState()->Reset();
	}
	//Received a pressed "normal" button
	else if (commandContext.IsCommand("PS3Button"))
	{
		ArteryI32 button;
		F32 testVal = 0.5;
		//Get the button pressed
		if (commandContext.MessageReader().Read(button))
		{
			Input::C_VirtualPadHandler::GetVirtualPadState()->SetButtonValue(button, 1);
		}
	}
	//Received a pressed pressure button
	else if (commandContext.IsCommand("PS3ButtonP"))
	{
		ArteryI32 button;
		//Get the button pressed
		if (commandContext.MessageReader().Read(button))
		{
			//And the value of the pressure applied
			ArteryF32 value;
			if (commandContext.MessageReader().Read(value))
			{
				Input::C_VirtualPadHandler::GetVirtualPadState()->SetButtonValue(button, value);
			}
		}
	}
	//Received an axis
	else if (commandContext.IsCommand("PS3Axis"))
	{
		ArteryI32 axis;
		//Get the button pressed
		if (commandContext.MessageReader().Read(axis))
		{
			//And the value of the pressure applied
			ArteryF32 value;
			if (commandContext.MessageReader().Read(value))
			{
				Input::C_VirtualPadHandler::GetVirtualPadState()->SetAxisValue(axis, value);
			}
		}
	}
	else if (commandContext.IsCommand("DisconnectPS3Controllers"))
	{
		Input::C_VirtualPadHandler::DisconnectVirtualPads();
	}
#endif
	else if (commandContext.IsCommand("IsXboxBladeDisplayed"))
	{
		//Not handled yet
	}
	else if (commandContext.IsCommand("LevelObjectInfo"))
	{
		Util::C_String text = Util::C_String::Format("%d!%d!%d",
			AmaxPerfLogging::g_uniqueLevelObjects,
			::Actors::C_ActorUtils::Instance().NumDynamicActors(),
			::Actors::C_ActorUtils::Instance().NumPhysicalActors());

		commandContext.SingleReply(text.Chars());
		return true;
	}


	else
	{
		return false;
	}
#endif

	commandContext.SingleReply("-OK-");

	return true;
}

}
