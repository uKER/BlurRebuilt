
#include "stdafx.h"

#include <misc_shared/Messaging/MessageBase.h>
#include <misc_shared/Messaging/MessageSubscriber.h>
#include <misc_shared/Messaging/MessageSubscriberList.h>
#include <misc_shared/Messaging/MessageSubscriberSystem.h>

#include "system/automation/Automation.h"

#include <shared_actors/Components/Messages/ResourceMessages.h>

#include <framework/application/ApplicationBase.h>
#include <Framework/shared_layers/DebugLayer.h>
#include <camera2.h>


#include "RaceLogicCore.h"
#include "../../../Game/GameWorld/GameWorld.h"

#include "../Data/RLDataStructures.h"
#include "../Controllers/RLControllers.h"
#include "../Database/RLDatabase.h"

//#include "../../ActorStudioActors/ActorStudioActors.h"
#include <GameLogic/ActorStudioActors/ActorStudioActors.h>
#include <GameLogic/ActorStudioActors/PickupPositionActorDecoy.h>
#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game/GameActors/Drivers/DriverManager.h>
#include <Game/GameActors/Drivers/DriverActor.h>	// just for DriverActorCreate
#include <GameLogic/Profile/GameProfileManager.h>

#include "../../../Game/Messaging/VehicleActorMessages.h"
#include "../../../Game/Messaging/PickupActorMessages.h"

#include "../../../Game/Messaging/GameflowMessages.h"
#include "../../../Game/Messaging/GameMessages.h"

#include <GameApp.h>
#include "../../../Game/Game.h"
#include "Game/AmaxCam/CameraSystem.h"
#include "Game/AmaxCam/CameraSwitcher.h"
#include "Game/AmaxCam/VehicleFeed.h"
#include <Game/GameUtil/ResourceBag.h>

#include <Game/GameControls/GameControls.h>
#include <Vehicle/VehicleModel.h>
#include <RacingPhysics/RacingPhysics.h>

#include <RaceBook/RaceBookManager.h>
#include <Racebook/Data/RaceBookData.h>

#include <AI/AI_Messages.h>
#include <AI/AI_AmaxSetup.h>
#include <AI/AI_Amax.h>

#include <GameLogic/ActorStudioActors/ActorUtils.h>
#include <GameLogic\Layers\RaceLogicLayer.h>
#include <GameLogic\Layers\RaceBookLayer.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>
#include <GameLogic/ActorStudioActors/CutSceneMessages.h>
#include <GameLogic\InGameStartLineInfoManager.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>

#include <misc_shared/Messaging/GameMessaging.h>

#include <UI/UiLayer.h>
#include <Ui/UiXtConfig.h>

#include "GameLogic/SinglePlayer/SinglePlayer.h"

namespace Framework { extern Debug::Bool g_DebugKeyCombos; }

#ifdef RLC_DEBUG
#pragma optimize ("", off) //RLC_DEBUG
#endif

extern Debug::Int g_debugUIFocusCar;
extern Debug::Bool g_TextureViewerEnable;

static Debug::Logger &g_RLCoreLog = Debug::GetLogger("Amax.RLC.RLCoreLog");
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////DEBUG VARIABLES//////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Debug::Bool g_Stamp("RLC.Stamp", false);								//Causes controllers to spit out useful information

Debug::Bool g_UseDebugHud("RLC.UseDebugHUD", false, Debug::Persist());

Debug::Bool g_UpdateVehicleCameraFocus("RLC.DebugCamera.UpdateVehicleCameraFocus", false);
Debug::Int g_FocusVehicleIndex("RLC.DebugCamera.FocusVehicleIndex", 0, Debug::Range(0,20));
Debug::Bool g_EnableVehicle("RLC.DebugCamera.EnableVehicle", false);
Debug::Bool g_DisableVehicle("RLC.DebugCamera.DisableVehicle", false);
Debug::Int g_debugVehicleToEnable("RLC.DebugCamera.VehicleToEnable", -1, Debug::Range(0,19));

Debug::Float g_spawnPointMinSafeDistance("Multiplayer.MotorMash.MinimumSafeDistanceForSpawning", 50.0, Debug::Range(0.0f, 1000.0f) );

Debug::Bool g_DrawStartPositions("AmaxAI.DebugDraw.TrackData.StartPositions", false);

Debug::Bool g_ResetVehiclesToSpawn("RLC.ResetVehiclesToSpawn", false);

extern Debug::Bool gRespawnPickups; //From PickupPositionActor.cpp


int g_RLC_QuickDropVehicleIndex = 0;
Debug::Bool g_DisableAllVehicles("RLC.ZZTheBluePillZZ.DisableAllVehicles", false);
Debug::Bool g_EnableAllVehicles("RLC.ZZTheBluePillZZ.EnableAllVehicles", false);

static void EnableAllVehicles(void*){ g_EnableAllVehicles = true; }
static void DisableAllVehicles(void*){ g_DisableAllVehicles = true; }

static Debug::Function EnableAllVehiclesButton("RLC.EnableAllVehicles",EnableAllVehicles,0);
static Debug::Function DisableAllVehiclesButton("RLC.DisableAllVehicles",DisableAllVehicles,0);

Debug::Bool g_QuickFireCarDropModeActive("RLC.ZZTheBluePillZZ.QuickFireCarDropModeActive", false);
static void QuickFireCarDropMode(void*){ g_QuickFireCarDropModeActive = true; g_FocusVehicleIndex = 0; g_DisableAllVehicles = true; }
static Debug::Function QuickFireCarDropModeButton("RLC.DebugCamera.QuickFireMode.QuickFireCarDropMode",QuickFireCarDropMode,0);
static Debug::Function EnableAllVehiclesButton1("RLC.DebugCamera.QuickFireMode.EnableAllVehicles",EnableAllVehicles,0);
static Debug::Function DisableAllVehiclesButton1("RLC.DebugCamera.QuickFireMode.DisableAllVehicles",DisableAllVehicles,0);

namespace GameDebug
{
	extern Debug::Bool g_debugMiniMonitorEnable;
}

extern Debug::Bool g_DebugDisableDebugHud;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////DEBUG VARIABLES//////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace Game
{
namespace Actors
{
RaceLogicCore::RaceLogicCore()
{
	m_UnPauseForFrames = -1;

	m_pGarageLockedCarSFX	= NULL;

	m_fGarageSFXTime		= 0.0f;

	//Hopefully the name says it all
	gRaceModeStore.CachePointerToTheRLC(this);
}

RaceLogicCore::~RaceLogicCore()
{
	Delete();

	//Hopefully the name says it all
	gRaceModeStore.CachePointerToTheRLC((Game::Actors::RaceLogicCore*)NULL);
}


const RaceLogicCoreActorCreate*	RaceLogicCore::GetCreationData() const 
{
	return static_cast<const RaceLogicCoreActorCreate*>(C_Actor::GetCreationData().GetData());
}

Debug::Bool g_debugDefferStartPositionsUnAILoaded("RLC.DelayPositioningVehiclesUntilAIDataLoaded", true);

void RaceLogicCore::ProcessUpdate()
{
	PERF_EVENT("RaceLogicCore::ProcessUpdate");

	if(g_Stamp)
		LOGDEBUG(g_RLCoreLog, "RaceLogicCore::ProcessUpdate() > !! HELLO WORLD !!");


	if(g_FocusVehicleIndex < 0)
		g_FocusVehicleIndex = 0;

	switch(meState)
	{
	case E_RLC_STATE_CREATED:
	case E_RLC_STATE_CONTROLLERS_AND_DATA_INITIALISED:
		{
			break;
		}
	case E_RLC_STATE_CREATE_VEHICLES:
		{
			//Create the vehicles, we ought to only have to do this once
			//SpawnVehicleActors((bool)g_debugDefferStartPositionsUnAILoaded);
			//Let this fall through
			//break;
		}

	
	case E_RLC_STATE_LOAD_VEHICLE_RESOURCES:
		{
			//Essentially just sit and wait for the resources to load...ho hum
			CheckInitialisation();
			break;
		}

	case E_RLC_STATE_VEHICLE_RESOURCES_LOADED:
		{
			SetState(E_RLC_STATE_LOAD_AI_RESOURCES);

			//All vehicles loaded, sending this message will tell the AI system to do what it needs to
			C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RLCResourcesLoaded );
			break;
		}

	case E_RLC_STATE_LOAD_AI_RESOURCES:
		{
			//Sit and wait for the AI system to finish it's loading
			break;
		}

	case E_RLC_STATE_AI_RESOURCES_LOADED:
		{
			//TODO Check whether the AI's loaded or if there's no AI data to load

			SetState(E_RLC_STATE_POST_INITIALISE);
			if (g_debugDefferStartPositionsUnAILoaded && !C_Game::GetInstance()->GetLevelState().mInUI)
				PlaceVehicles();
			break;
		}

	case E_RLC_STATE_POST_INITIALISE:
		{
			//DEBUG DEBUG DEBUG
			int viewportIndex = 0;
			//DEBUG DEBUG DEBUG

			{
				bool foundStartFinish = false;
				bool foundSplit1 = false;
				bool foundSplit2 = false;

				airacingtech::ITrackDataAI *pTrackData = AIAmax::AmaxAIR::Instance().GetLoadedTrackDataSystem();	// [Blur2PC] was GetTrackDataSystem(), NULL before InitRace
				if (pTrackData)
				{
					airacingtech::AITrackData *pAITrackData = pTrackData->GetTrackData();

					if (pAITrackData)
					{
						for (int i=0; i < pAITrackData->m_Routes.Size(); i++)
						{
							const airacingtech::AITrackDataRoutes *pRoute = pAITrackData->m_Routes[i];

							if (pRoute->IsStart())				foundStartFinish = true;
							if (pRoute->IsIntermediate(1))		foundSplit1 = true;
							if (pRoute->IsIntermediate(2))		foundSplit2 = true;
						}
					}
				}
				
				if (!foundStartFinish)
				{
					Automation::SendProblemFoundMsg("LEVEL", 0, "NOSPLITS");
					LOGWARN(g_RLCoreLog, "Split Start/Fin not found");
				}
				if (!foundSplit1)
				{
					Automation::SendProblemFoundMsg("LEVEL", 1, "NOSPLITS");
					LOGWARN(g_RLCoreLog, "Split 1 not found");
				}
				if (!foundSplit2)
				{
					Automation::SendProblemFoundMsg("LEVEL", 2, "NOSPLITS");
					LOGWARN(g_RLCoreLog, "Split 2 not found");
				}
			}

			//Copy the viewport settings from the RaceLogicCore into the Camera System viewports:
			NotifyCameraSystemAboutViewports();
			
			//Check to see if the AI data's all loaded
			GetTrackData()->mAIDataReceived = false;
			if(AIAmax::AmaxAIR::Instance().AllOK())
			{
				GetTrackData()->mAIDataReceived = true;
			}
			else
			{
				Automation::SendProblemFoundMsg("LEVEL", "AIBROKEN");
			}

			//Check that we've got some split lines
			GetTrackData()->mSplitsReceived = false;
			//HACK HACK HACK
			if(GetTrackData()->mSplitData.Size() >= 3)	//TODO Respond to some sort of "AllActorsLoaded" type of message
			//HACK HACK HACK
			{
				GetTrackData()->mSplitsReceived = true;
			}

			SetState(E_RLC_STATE_TELL_THE_WORLD);
			break;
		}

	case E_RLC_STATE_TELL_THE_WORLD:
		{
			//All vehicles loaded, sending this message will tell the AI system to do what it needs to
			C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RLCReadyToRock );

			SetState(E_RLC_STATE_READY);
			break;
		}

	case E_RLC_STATE_READY:
		{
			//	[GavinY] - This lets us "skip frames", unpausing the game and then pausing again very quickly.
			//				It's a bit messy so if this would be best placed elsewhere let me know!
			FrameSkipUpdate();

			//Gather data
			PreUpdate();

			//DEBUG DEBUG DEBUG
			{
				//Drop vehicles, change debug cam etc etc
				CheckDebugKeyCombos();

				ProcessDebugUpdate();
			}
			//DEBUG DEBUG DEBUG


			//Process the data
			Update();

			//Render
			Render();

			//Clean up
			PostUpdate();

			


			break;
		}
	}

	if(g_Stamp)
		g_Stamp = false;
}



Debug::Int g_debugFramesToUnPauseFor("Game.NumFramesToUnPauseFor", 1, Debug::Range(0, 10), Debug::Step(1));
Debug::Bool g_debugUpdateMotionBlurOnFrameSkip("Game.UpdateMotionBlurOnFrameSkip", true);

void RaceLogicCore::PausedUpdate()
{
	PERF_EVENT("RaceLogicCore::PausedUpdate");

	if(g_FocusVehicleIndex < 0)
		g_FocusVehicleIndex = 0;

	switch(meState)
	{
	case E_RLC_STATE_READY:
		{
#ifndef CFG_GOLD
			S8 viewportIndex = 0;
			S8 vehicleIndex = GetViewportFocusVehicleIndex(viewportIndex);
			
			S8 controllerIndex = -GetViewportControllerIndex(viewportIndex);
			if(controllerIndex < INVALID_CONTROLLER)
				controllerIndex = 0;

			if( Input::GetGameInput(controllerIndex)->GetButtonValue(Framework::E_Button_Debug_ButtonBack) == 1.f)
			{
				if(Input::GetGameInput(controllerIndex)->GetButtonPressedDebounced(Framework::E_Button_Debug_DPadLeft) )
				{
					FocusOnPreviousVehicle();
					AmaxCam::CameraSystem().WorldUpdate(1.0f / 60.0f);
					if (g_debugUpdateMotionBlurOnFrameSkip)
					{						
						if (GetVehicleActor(g_FocusVehicleIndex))
						{
							Maths::HVec3 v = GetVehicleActor(g_FocusVehicleIndex)->GetVehicleModel()->GetModelInput()->m_vVelocity;
							v = v / 30.0f;					
							AmaxCam::CameraSystem().VehicleFeed()->SetMotionBlur(v);
						}
					}
					else
						AmaxCam::CameraSystem().WorldUpdate(1.0f / 60.0f);
				}
				else if(Input::GetGameInput(controllerIndex)->GetButtonPressedDebounced(Framework::E_Button_Debug_DPadRight) )
				{
					FocusOnNextVehicle();
					AmaxCam::CameraSystem().WorldUpdate(1.0f / 60.0f);
					if (g_debugUpdateMotionBlurOnFrameSkip)
					{						
						if (GetVehicleActor(g_FocusVehicleIndex))
						{
							Maths::HVec3 v = GetVehicleActor(g_FocusVehicleIndex)->GetVehicleModel()->GetModelInput()->m_vVelocity;
							v = v / 30.0f;					
							AmaxCam::CameraSystem().VehicleFeed()->SetMotionBlur(v);
						}
					}
					else
						AmaxCam::CameraSystem().WorldUpdate(1.0f / 60.0f);
				}
				else if(Input::GetGameInput(controllerIndex)->GetButtonPressedDebounced(Framework::E_Button_Debug_DPadUp) || 
					Input::GetGameInput(controllerIndex)->GetButtonPressedDebounced(Framework::E_Button_Debug_DPadDown))
				{
						SkipFrames(g_debugFramesToUnPauseFor + 1);
				}
			}
#endif
			break;
		}
	}
}

void RaceLogicCore::FocusOnPreviousVehicle()
{
	//Make sure we actually have some cars to look at
	if(C_GameApp::GetInstance()->GetGame()->GetLevelState().mNumCars > 0)
	{
		S8 viewportIndex = 0;
		S8 vehicleIndex = GetViewportFocusVehicleIndex(viewportIndex);

		//Previous vehicle index
		int prospectiveVehicleIndex = GetViewportFocusVehicleIndex(viewportIndex);
		prospectiveVehicleIndex--;

		//Don't let it loop around
		if(prospectiveVehicleIndex < 0)
			prospectiveVehicleIndex = 0;

		SetViewportFocusVehicleIndex(viewportIndex, prospectiveVehicleIndex);
		g_FocusVehicleIndex = prospectiveVehicleIndex;
	}
}

void RaceLogicCore::FocusOnNextVehicle()
{
	if(C_GameApp::GetInstance()->GetGame()->GetLevelState().mNumCars > 0)
	{
		S8 viewportIndex = 0;
		S8 vehicleIndex = GetViewportFocusVehicleIndex(viewportIndex);

		//Next vehicle index
		int prospectiveVehicleIndex = GetViewportFocusVehicleIndex(viewportIndex);
		prospectiveVehicleIndex++;

		//Don't let it loop around
		if(prospectiveVehicleIndex >= (S8) GetNumVehicles())
			prospectiveVehicleIndex = GetNumVehicles()-1;

		SetViewportFocusVehicleIndex(viewportIndex, prospectiveVehicleIndex);
		g_FocusVehicleIndex = prospectiveVehicleIndex;
	}
}

Debug::Bool g_debugEnableDynamicObjectDrop("ActorDebug.EnableDynamicObjectDropping", false, Debug::Persist());
Debug::Float g_debugSlowDownTimeScale("Game.SlowDownTimeScale", 0.1f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

void RaceLogicCore::CheckDebugKeyCombos()
{
	PERF_EVENT("RaceLogicCore::CheckDebugKeyCombos");

	// Change the assist mode (which is no longer a debug thing)
	// Only allowed if we haven't disabled them.
	//if(!C_GameApp::GetInstance()->GetGame()->GetLevelState().mDisableAssists)
	//{
	//	if( Input::GetGameInput(controllerIndex)->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonBack) )
	//	{
	//		C_VehicleActor* pActor = GetVehicleActor(g_FocusVehicleIndex);
	//		if (pActor)
	//		{
	//			if( pActor->m_corneringAssist && pActor->m_brakeAssist )
	//				pActor->m_corneringAssist = false;
	//			else if ( !pActor->m_corneringAssist && pActor->m_brakeAssist )
	//				pActor->m_brakeAssist = false;
	//			else if ( !pActor->m_corneringAssist && !pActor->m_brakeAssist )
	//				pActor->m_corneringAssist = true;
	//			else if ( pActor->m_corneringAssist && !pActor->m_brakeAssist )
	//				pActor->m_brakeAssist = true;
	//		}

	//	}
	//}



	if (!Framework::g_DebugKeyCombos)
		return;

	C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();


	S8 vehicleIndex = -1;
	S8 controllerIndex = -1;

	//Go through each viewport checking that controllers inputs for debug shizzle
	for(U32 viewportIndex = 0; viewportIndex < levelState.mNumViewports; viewportIndex++)
	{
		vehicleIndex = GetViewportFocusVehicleIndex(viewportIndex);
		
		controllerIndex = GetViewportControllerIndex(viewportIndex);
		if(controllerIndex == INVALID_CONTROLLER)
			continue;

		// Don't turn off the texture viewer when scrolling.
		if (Input::GetGameInput(controllerIndex)->GetButtonPressedDebounced(Framework::E_Button_Debug_DPadDown) && !g_TextureViewerEnable)
		{
			GameDebug::g_debugMiniMonitorEnable = !GameDebug::g_debugMiniMonitorEnable;
			g_DebugDisableDebugHud = !g_DebugDisableDebugHud;
		}

		///////////////////////////////////////////////////////////////////////
		//Swap focus vehicles (via debugview)
		///////////////////////////////////////////////////////////////////////
		if(g_UpdateVehicleCameraFocus)
		{
			//Set to specified vehicle index (if its valid)
			if( VehicleValid(g_FocusVehicleIndex) )
				SetViewportFocusVehicleIndex(viewportIndex, g_FocusVehicleIndex);
			else
				//Oh ho they've set an invalid vehicleIndex, reset debug value to the current vehicle index for the viewport
				g_FocusVehicleIndex = GetViewportFocusVehicleIndex(viewportIndex);

			g_UpdateVehicleCameraFocus = false;
		}
		///////////////////////////////////////////////////////////////////////
		//Swap focus vehicles (via debugview)
		///////////////////////////////////////////////////////////////////////

		if(!g_QuickFireCarDropModeActive)
		{
			///////////////////////////////////////////////////////////////////////
			//Drop vehicle
			///////////////////////////////////////////////////////////////////////
			if (Input::GetGameInput(controllerIndex)->GetButtonPressedDebounced(Framework::E_Button_MenuConfirm) &&
				Input::GetGameInput(controllerIndex)->GetButtonValue(Framework::E_Button_Debug_ButtonRightTrigger) >= 0.5f &&
				AmaxCam::Switcher().Choice() == AmaxCam::E_DebugCam_FlyCamera)
			{
				if (VehicleValid(g_FocusVehicleIndex) )
				{
					Game::Actors::C_VehicleResetAtCameraPositionMsg vehicleResetMsg(mVehicles[vehicleIndex].mActorHandle, vehicleIndex);
					m_pGameWorld->GetActorList()->GetSubscriberSystem().BroadcastMessage(&vehicleResetMsg);
				}
			}
			///////////////////////////////////////////////////////////////////////
			//Drop vehicle
			///////////////////////////////////////////////////////////////////////
//#ifndef CFG_GOLD
//			// Drop a Dynamic object
//			if (g_debugEnableDynamicObjectDrop && Input::GetGameInput(controllerIndex)->GetButtonPressedDebounced(Framework::E_Button_MenuConfirm) &&
//				Input::GetGameInput(controllerIndex)->GetButtonValue(Framework::E_Button_Debug_ButtonLeftTrigger) >= 0.5f &&
//				AmaxCam::Switcher().Choice() == AmaxCam::E_DebugCam_FlyCamera)
//			{
//				::Actors::C_ActorUtils::Instance().DropDynamicObject();
//			}
//#endif

		}
		else
		{
			//Allows the track designers to drop the all vehicles relatively quickly by moving onto the next vehicle index after each vehicle is dropped
			SetViewportFocusVehicleIndex(viewportIndex, g_FocusVehicleIndex);

			///////////////////////////////////////////////////////////////////////
			//Drop vehicle
			///////////////////////////////////////////////////////////////////////
			if (Input::GetGameInput(controllerIndex)->GetButtonPressedDebounced(Framework::E_Button_MenuConfirm) &&
				Input::GetGameInput(controllerIndex)->GetButtonValue(Framework::E_Button_Debug_ButtonRightTrigger) >= 0.5f &&
				AmaxCam::Switcher().Choice() == AmaxCam::E_DebugCam_FlyCamera)
			{
				if( VehicleValid(g_FocusVehicleIndex) )
				{
					//Drop the vehicle
					Game::Actors::C_VehicleResetAtCameraPositionMsg vehicleResetMsg(mVehicles[vehicleIndex].mActorHandle, vehicleIndex);
					m_pGameWorld->GetActorList()->GetSubscriberSystem().BroadcastMessage(&vehicleResetMsg);

					//Now move the vehicle index on one
					int prospectiveVehicleIndex = GetViewportFocusVehicleIndex(viewportIndex);
					prospectiveVehicleIndex++;

					//Don't let it loop around
					if(prospectiveVehicleIndex >= mVehicles.Size())
					{
						//If we've dropped all the vehicles 
						prospectiveVehicleIndex = mVehicles.Size()-1;
						g_QuickFireCarDropModeActive = false;
					}

					SetViewportFocusVehicleIndex(viewportIndex, prospectiveVehicleIndex);
					g_FocusVehicleIndex = prospectiveVehicleIndex;
				}
			}
			///////////////////////////////////////////////////////////////////////
			//Drop vehicle
			///////////////////////////////////////////////////////////////////////
		}

			
		if( Input::GetGameInput(controllerIndex)->GetButtonValue(Framework::E_Button_Debug_ButtonBack) == 1.f)
		{
			if(Input::GetGameInput(controllerIndex)->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonA) )
			{
				///////////////////////////////////////////////////////////////////////
				//Swap between human and AI control
				///////////////////////////////////////////////////////////////////////
				if (VehicleValid(vehicleIndex))
				{
					if(mVehicles[vehicleIndex].m_pVehicleActor->GetVehicleInput()->m_iControlType == RacingPhysics::Vehicle::CONTROL_TYPE_PLAYER)
					{
						Game::Actors::C_VehicleForceAIControlMsg forceAIControlMsg(vehicleIndex);
						m_pGameWorld->GetActorList()->GetSubscriberSystem().BroadcastMessage(&forceAIControlMsg);
					}
					else
					{
						Game::Actors::C_VehicleForceHumanControlMsg forceHumanControlMsg(vehicleIndex);
						m_pGameWorld->GetActorList()->GetSubscriberSystem().BroadcastMessage(&forceHumanControlMsg);
					}
				}
				///////////////////////////////////////////////////////////////////////
				//Swap between human and AI control
				///////////////////////////////////////////////////////////////////////
			}

			if(Input::GetGameInput(controllerIndex)->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonY) )
			{
				if (C_GameApp::GetInstance()->GetTimeScale() == 1.0f)
				{
					C_GameApp::GetInstance()->SetTimeScale((float)g_debugSlowDownTimeScale);
					ActorMsg::C_VehicleSloMoEntryMsg pMsg;
					pMsg.m_Manual = true;
					C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&pMsg);
					if (GetGameWorld())
						GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage(&pMsg);
				}
				else
				{
					ActorMsg::C_VehicleSloMoExitMsg pMsg;
					C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&pMsg);
					if (GetGameWorld())
						GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage(&pMsg);
					C_GameApp::GetInstance()->SetTimeScale(1.0f);
				}
			}

			if(Input::GetGameInput(controllerIndex)->GetButtonPressedDebounced(Framework::E_Button_Debug_DPadLeft) )
				FocusOnPreviousVehicle();			
			else if(Input::GetGameInput(controllerIndex)->GetButtonPressedDebounced(Framework::E_Button_Debug_DPadRight) )
				FocusOnNextVehicle();	
		}
	}

	//Only do this for viewport 0
	U8 viewportIndex = 0;
	if(g_EnableVehicle)
	{
		if (g_debugVehicleToEnable == -1)
		{
			int vehicleIndex = GetViewportFocusVehicleIndex(viewportIndex);

			Game::Actors::C_VehicleControlsEnableMsg enableMsg(mVehicles[vehicleIndex].mActorHandle, vehicleIndex);
			BroadcastMessage(&enableMsg);
		}

		if (g_debugVehicleToEnable >= 0 && g_debugVehicleToEnable < (S32) GetNumVehicles())
		{
			if (GetVehicleActor(g_debugVehicleToEnable))
				GetVehicleActor(g_debugVehicleToEnable)->SendMessageToActor(GetVehicleActor(g_debugVehicleToEnable)->GetHandle(), E_VehicleMessage_Enable);
		}
		g_EnableVehicle = false;
	}

	if(g_DisableVehicle)
	{
		if (g_debugVehicleToEnable == -1)
		{
			int vehicleIndex = GetViewportFocusVehicleIndex(viewportIndex);

			Game::Actors::C_VehicleControlsDisableMsg disableMsg(vehicleIndex);
			BroadcastMessage(&disableMsg);
		}

		if (g_debugVehicleToEnable >= 0 && g_debugVehicleToEnable < (S32) GetNumVehicles())
		{
			if (GetVehicleActor(g_debugVehicleToEnable))
				GetVehicleActor(g_debugVehicleToEnable)->SendMessageToActor(GetVehicleActor(g_debugVehicleToEnable)->GetHandle(), E_VehicleMessage_Disable);
		}
		g_DisableVehicle = false;
	}

}

void RaceLogicCore::ProcessDebugUpdate()
{
	PERF_EVENT("RaceLogicCore::ProcessDebugUpdate");

	//
	if(g_ResetVehiclesToSpawn)
	{
		Game::Actors::C_VehicleResetAtSpawnPositionMsg resetAtSpawnMsg(-1);
		BroadcastMessage(&resetAtSpawnMsg);

		g_ResetVehiclesToSpawn = false;
	}

	if(g_DisableAllVehicles)
	{
		for(int vehicleIndex = 0; vehicleIndex < mVehicles.Size(); vehicleIndex++)
		{
			Game::Actors::C_VehicleControlsDisableMsg disableMsg(vehicleIndex);
			BroadcastMessage(&disableMsg);
		}

		g_DisableAllVehicles = false;
	}
	
	if(g_EnableAllVehicles)
	{
		for(int vehicleIndex = 0; vehicleIndex < mVehicles.Size(); vehicleIndex++)
		{
			Game::Actors::C_VehicleControlsEnableMsg enableMsg(mVehicles[vehicleIndex].mActorHandle, vehicleIndex);
			BroadcastMessage(&enableMsg);
		}

		g_EnableAllVehicles = false;
	}

	if(gRespawnPickups)
	{
		gRespawnPickups = false;
		BroadcastMessage(Game::E_PickupMessage_DebugForcePickupRespawn);
	}
}


void RaceLogicCore::FrameSkipUpdate()
{
	if (m_UnPauseForFrames > 0)
	{	
		m_UnPauseForFrames--;
		if (m_UnPauseForFrames == 0)
		{
			Framework::C_ApplicationBase::GetInstance()->Pause( Framework::E_LayerPause_Freeze );					
		}					
	}
}

void RaceLogicCore::PreUpdate()
{
	PERF_EVENT("RaceLogicCore::PreUpdate");

	if(GetTrackData()->mRaceStarted)
		GetTimeData()->mRaceTime += GetFrameTime();

	// Does not appear to still be used

	//Copy the last updates vehicle data
	mLastVehicles = mVehicles;
 	for(int vehicleIndex = 0; vehicleIndex< mVehicles.Size(); vehicleIndex++)
		mVehicles[vehicleIndex].PrepareForFrame();

	//PreUpdate our components
	for(int i = 0; i < E_RLCOMPONENTTYPE_MAX; i++)
	{
		mComponents[i]->PreUpdate();
	}

}

void RaceLogicCore::Update()
{
	PERF_EVENT("RaceLogicCore::Update");

	//Update the components
	for(int i = 0; i < E_RLCOMPONENTTYPE_MAX; i++)
	{
		mComponents[i]->Update();
	}

	for (int iView = 0; iView < mViewportDetails.Size(); iView++)
	{
		C_VehicleFocusPositionMsg msg;
		msg.mVehicleIndex	= mViewportDetails[iView].mVehicleIndex;
		msg.mViewIndex		= iView;
		if (GetVehicleActor(msg.mVehicleIndex))
		{
			msg.mGroundPosition = GetVehicleActor(msg.mVehicleIndex)->GetGroundPosition();
			msg.mGroundNormal = GetVehicleActor(msg.mVehicleIndex)->GetGroundNormal();
			msg.mWorldMatrix = GetVehicleActor(msg.mVehicleIndex)->GetRawMatrix();
		}
		else
		{
			msg.mWorldMatrix.SetIdentity();
			msg.mGroundPosition = Maths::HVec3(0.0f, 0.0f, 0.0);
			msg.mGroundNormal = Maths::C_Normal(0.0f, 1.0f, 0.0f);
		}

		BroadcastMessage(&msg);		
	}

	if(g_DrawStartPositions)
	{
		for( int i = 0; i < GetTrackData()->mStartPositions.Size(); i++ )
		{
			const RLStartPosition& startPos = GetTrackData()->mStartPositions[i];
			Maths::HVec3 pos = startPos.mWorldPosition;
			Maths::HVec3 dir = startPos.mWorldMatrix.AZ();
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(pos, dir, 1, 1, Maths::RGBA(255, 0, 255) );
		}
	}	
}

void RaceLogicCore::PostUpdate()
{
	PERF_EVENT("RaceLogicCore::PostUpdate");

	for(int i = 0; i < E_RLCOMPONENTTYPE_MAX; i++)
	{
		mComponents[i]->PostUpdate();
	}

	SendControls();
}

void RaceLogicCore::Render()
{
	PERF_EVENT("RaceLogicCore::Render");

	for(int i = 0; i < E_RLCOMPONENTTYPE_MAX; i++)
	{
		mComponents[i]->Render();
	}	

}

void RaceLogicCore::SendControls()
{
	PERF_EVENT("RaceLogicCore::SendControls");

	//Make sure we've got some vehicles to update
	if(mVehicles.Size() == 0)
		return;

	bool paused = IsPaused();

	if(!GetTrackData()->mRaceStarted)
		return;

	for(int i = 0; i < mVehicles.Size(); i++)
	{
		int controllerIndex = C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[i].mController;
		if(!IsVehicleLoaded(i) || !mVehicles[i].mLoaded || controllerIndex == INVALID_CONTROLLER || AmaxCam::CameraSystem().PadUsedForDebug(controllerIndex) )
			continue;

		Game::Actors::C_VehicleUpdateControlsMsg controllerInput;

		if(!paused)
		{
			controllerInput.m_fThrottle			= Input::GetGameInput(controllerIndex)->GetButtonValue(Game::E_Button_Throttle);
#ifdef PLATFORM_PS3
			C_GameProfile* pProfile = C_GameProfileManager::Instance().GetProfile(controllerIndex);
			if(pProfile && pProfile->mOptions.mControlIndex == 3)
			{
				controllerInput.m_fThrottle			*= 2.f;
				controllerInput.m_fThrottle=Maths::Clamp(controllerInput.m_fThrottle,0.f,1.0f);
			}
#endif

#ifdef PLATFORM_PC
			// [Blur2PC] retail Blur PC (0x0049c91c) steers from the steering axis alone; its d-pad is bound there too
			float steering=Maths::Clamp(Input::GetGameInput(controllerIndex)->GetAxisSignedValue(Game::E_Axis_Steering), -1.f,1.f);
#else
			float steering=Maths::Clamp(Input::GetGameInput(controllerIndex)->GetAxisSignedValue(Game::E_Axis_Steering) +
				Input::GetGameInput(controllerIndex)->GetAxisSignedValue(Game::E_Axis_DigitalSteering),
				-1.f,1.f);
#endif

			controllerInput.m_fSteering			= steering;

			controllerInput.m_fPitch			= Input::GetGameInput(controllerIndex)->GetAxisSignedValue(Game::E_Axis_Pitch);
			controllerInput.m_fBrake			= Input::GetGameInput(controllerIndex)->GetButtonValue(Game::E_Button_Brake);
#ifdef PLATFORM_PS3
			if(pProfile && pProfile->mOptions.mControlIndex == 3)
			{
				controllerInput.m_fBrake			*= 2.f;
				controllerInput.m_fBrake=Maths::Clamp(controllerInput.m_fBrake,0.f,1.0f);
			}
#endif

#ifdef PLATFORM_PS3
			controllerInput.m_bHandbrake		= Input::GetGameInput(controllerIndex)->GetButtonValue(Game::E_Button_Handbrake) > 0.1f ? true : false;
#else
			controllerInput.m_bHandbrake		= Input::GetGameInput(controllerIndex)->GetButtonValue(Game::E_Button_Handbrake) > 0.5f ? true : false;
#endif
		}

		m_pGameWorld->GetActorList()->SendMessageToActor(mVehicles[i].mActorHandle, &controllerInput);
	}
}



//Use this function to get the player's vehicleIndex...should always be 0 unless we're in split screen
const S8 RaceLogicCore::GetPlayerVehicleIndex(const int controllerIndex) const
{
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	for (U8 i=0; i<levelState.mNumCars; i++)
	{
		if (levelState.mCars[i].mController == controllerIndex)
			return i;
	}

	return -1;

	// Charlie == fail
}

const S8 RaceLogicCore::GetVehicleIndexFromActorHandle(const Actor::C_ActorHandle& actorHandle) const
{
	//Look through all the vehicles for this handle
	S8 vehicleIndex = -1;
	for(int i = 0; i < mVehicles.Size(); i++)
	{
		if(mVehicles[i].mActorHandle == actorHandle)
			return i;
	}
	return vehicleIndex;
}

const S8 RaceLogicCore::GetVehicleIndexFromActorHandle(const Actor::C_ActorHandle* pActorHandle) const
{
	//Look through all the vehicles for this handle
	S8 vehicleIndex = -1;
	for(int i = 0; i < mVehicles.Size(); i++)
	{
		if(&mVehicles[i].mActorHandle == pActorHandle)
			return i;
	}
	return vehicleIndex;
}

const Actor::C_ActorHandle RaceLogicCore::GetActorHandleFromVehicleIndex(const U8 vehicleIndex) const
{
	Actor::C_ActorHandle actorHandle;
	if(VehicleValid(vehicleIndex))
	{
		actorHandle = mVehicles[vehicleIndex].mActorHandle;
	}
	return actorHandle;
}

C_VehicleActor *RaceLogicCore::GetVehicleActor(const U8 vehicleIndex)const
{
	if( VehicleValid(vehicleIndex) )
		return mVehicles[vehicleIndex].m_pVehicleActor;
	
	return NULL;
}

const Vehicle::C_ModelInput* RaceLogicCore::GetVehicleModelInput(const U8 vehicleIndex) const
{
	const Vehicle::C_ModelInput *pInput = 0;
	if( VehicleValid(vehicleIndex) )	//Make sure this is a valid vehicle
		if(mVehicles[vehicleIndex].m_pVehicleActor)	//Make sure we've got a valid pointer to the actor
			if(mVehicles[vehicleIndex].m_pVehicleActor->GetVehicleInput())
				pInput = mVehicles[vehicleIndex].m_pVehicleActor->GetVehicleInput();

	return pInput;
}

const U32 RaceLogicCore::GetVehicleRevs(const U8 vehicleIndex) const
{
	U32 revs = 0;

	if( VehicleValid(vehicleIndex) )
	{
		const Vehicle::C_ModelInput *pInput = GetVehicleModelInput(vehicleIndex);
		if(pInput)
			revs = (U32)pInput->m_fRevs;
	}

	return revs;
}

const U32 RaceLogicCore::GetVehicleRedLine(const U8 vehicleIndex) const
{
	U32 redLine = 0;

	if( VehicleValid(vehicleIndex) )
	{
		const Vehicle::C_ModelInput *pInput = GetVehicleModelInput(vehicleIndex);
		if(pInput)
			redLine = (U32)pInput->m_fRedLine;
	}

	return redLine;
}

const S8 RaceLogicCore::GetVehicleGear(const U8 vehicleIndex) const
{
	S8 gear = 0;

	if( VehicleValid(vehicleIndex) )
	{
		const Vehicle::C_ModelInput *pInput = GetVehicleModelInput(vehicleIndex);
		if(pInput)
			gear = (S8)pInput->m_iGear;
	}

	return gear;
}

const PickupPositionArray& RaceLogicCore::GetPickupPositions() const 
{
	return GetTrackData()->mPickupPositions;
}

const S8 RaceLogicCore::GetViewportFocusVehicleIndex(const U8 viewportIndex) const
{
	S8 vehicleIndex = INVALID_VALUE;

	//Check this is a valid veiwport index
	if( ViewportValid(viewportIndex) )
		return mViewportDetails[viewportIndex].mVehicleIndex;

	return vehicleIndex;
}

void RaceLogicCore::SetViewportFocusVehicleIndex(const U8 viewportIndex, const U8 vehicleIndex)
{
	if( ViewportValid(viewportIndex) )
	{
		if( VehicleValid(vehicleIndex) )
		{
			//Now we can set the focus
			mViewportDetails[viewportIndex].mVehicleIndex = vehicleIndex;

			//Focus on correct vehicle
			AmaxCam::CameraSystem().SetFocusHandle(viewportIndex, mVehicles[vehicleIndex].mActorHandle);

			AmaxCam::CameraSystem().ResetVehicleFeed(viewportIndex);
		}
	}
	else
		LOGERROR( g_RLCoreLog, "RaceLogicCore::SetViewportFocusVehicleIndex() > *INVALID VIEWPORT INDEX*, try asking CharlieA");
}

const S8 RaceLogicCore::GetViewportControllerIndex(const U8 viewportIndex) const
{
	S8 controllerIndex = INVALID_CONTROLLER;

	if( ViewportValid(viewportIndex) )
	{
		//Now we know that this is a valid viewport we can check to see if we're controlling a vehicle
		if( VehicleValid(mViewportDetails[viewportIndex].mVehicleIndex) )
			controllerIndex = mViewportDetails[viewportIndex].mControllerIndex;
	}

	return controllerIndex;
}

const S8 RaceLogicCore::GetViewportIndex(const U8 vehicleIndex) const
{
	for ( S32 i = 0; i < mViewportDetails.Size(); i++ )
	{
		if ( GetViewportFocusVehicleIndex( i ) == vehicleIndex )
			return i;
	}

	return -1;
}

const float RaceLogicCore::GetFrameTime() const
{
	float time = C_GameApp::GetInstance()->GetTimeStep();

	return time;
}

const float RaceLogicCore::GetRaceTime() const
{
	return GetTimeData()->mRaceTime;
}

const U32 RaceLogicCore::GetNumVehicles() const
{
	return mVehicles.Size();
}

const RLPickupType* RaceLogicCore::GetPickupType(const enum E_PICKUP_TYPE ePickupType) const
{
	for(int i = 0; i < GetRLSystem()->mPickupSystem->mTypes.count; i++)
	{
		if(GetRLSystem()->mPickupSystem->mTypes[i]->meType == ePickupType)
		{
			return GetRLSystem()->mPickupSystem->mTypes[i];
		}
	}

	return (const RLPickupType*)0;
}

XtHandle<RLPickupType> RaceLogicCore::GetPickupHandle(const enum E_PICKUP_TYPE ePickupType)
{
	for(int i = 0; i < GetRLSystem()->mPickupSystem->mTypes.count; i++)
	{
		if(GetRLSystem()->mPickupSystem->mTypes[i]->meType == ePickupType)
		{
			return GetRLSystem()->mPickupSystem->mTypes[i];
		}
	}

	XtHandle<RLPickupType> null_handle;
	return null_handle;
}


const bool RaceLogicCore::IsVehicleLoaded(const U8 vehicleIndex) const
{
	if(VehicleValid(vehicleIndex))
		return mVehicles[vehicleIndex].mLoaded;

	return false;
}

const bool RaceLogicCore::IsVehicleAIControlled(const U8 vehicleIndex) const
{
	const C_LevelState& ls = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	const Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehicleIndex);
	bool logicAi = false;
	if(pLogicActor && pLogicActor->GetToggledToAiControl())
		logicAi = true;

	// For automation purposes, we pretend some cars are really human
#ifndef CFG_GOLD
	if (ls.mCars[vehicleIndex].mAutomatedHuman)
		return false;
#endif

	return (ls.mCars[vehicleIndex].mControlType == E_VEHICLE_CONTROL_TYPE_AI || logicAi);
}


const bool RaceLogicCore::IsVehiclePlayerControlled( const U8 vehicleIndex ) const
{
	const C_LevelState& ls = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	return(ls.mCars[vehicleIndex].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER);
}

const bool RaceLogicCore::ArePickupsEnabled() const
{ 
	return GetTrackData()->mPickupsEnabled;
}


const bool RaceLogicCore::IsAIDataAnalysed() const
{
	if( GetTrackData() )
		return GetTrackData()->mAIDataAnalysed;

	return false;
}

const bool RaceLogicCore::IsSplitDataAnalysed() const 
{
	if( GetTrackData() )
		return GetTrackData()->mSplitsAnalysed;

	return false;
}

const RLSystem* RaceLogicCore::GetRLSystem() const
{
	if(GetDatabase()->mRLSystems[0])
		return GetDatabase()->mRLSystems[0];
	else
	{
		//THIS IS PRETTY SERIOUS SO I'M GOING TO SPAM THE OUTPUT
		LOGERROR(g_RLCoreLog, "RaceLogicCore::GetRLSystem() > ** RLDatabase data missing...this isn't going to work very well :-( **");

		return (RLSystem*)0;
	}
}

const bool RaceLogicCore::IsPaused() const
{
	return GetTimeData()->mPaused;
}

void RaceLogicCore::SetPaused(const bool paused)
{
	GetTimeData()->mPaused = paused;
}

void RaceLogicCore::Reset()
{
	//Data & Controllers
	Delete();

	SetState(E_RLC_STATE_CREATED);
}

RLTrackData* RaceLogicCore::GetTrackData() const
{
	return m_pTrackData;
}

RLTimeData* RaceLogicCore::GetTimeData() const
{
	return m_pTimeData;
}

::RLDatabase* RaceLogicCore::GetDatabase() const
{
	return &::RLDatabase::Instance();
}

C_GameWorld* RaceLogicCore::GetGameWorld() const
{
	return m_pGameWorld;
}

//Called before createImpl()
bool RaceLogicCore::InitialiseImpl()
{	
	SetState(E_RLC_STATE_CREATED);

	return true;
}

//Called after InitialiseImpl()
bool RaceLogicCore::CreateImpl()
{
	AddSubscriber( GameMessaging::E_GameMsg_Update );
	AddSubscriber( GameMessaging::E_GameMsg_PausedUpdate );
	
	AddSubscriber( Actor::E_ActorMsg_Created );

	AddSubscriber( Game::E_GameflowMessage_LoadResources );
	AddSubscriber( Game::E_GameflowMessage_RaceLoaded );
	AddSubscriber( Game::E_GameflowMessage_RaceStarted );
	AddSubscriber( Game::E_GameflowMessage_RaceFinished );
	AddSubscriber( Game::E_VehicleMessage_Loaded );
	AddSubscriber( Game::E_GameflowMessage_QuitLevel );
	AddSubscriber( Game::E_GameflowMessage_QuitAndWinRace );

	AddSubscriber( Game::E_AIMessage_AllLoaded );
	AddSubscriber( Game::E_AIMessage_LoadFailed );

	AddSubscriber( Game::E_PickupMessage_PickupPickedUp );

	AddSubscriber( Game::E_GameMsg_PostCreateInit );

	m_pGameWorld = GetCreationData()->m_pGameWorld;

	Initialise();
	return true;
}

void RaceLogicCore::Initialise()
{
	InitialiseDataStructures();
	InitialiseControllers();

	InitialiseViewportDetails();

	//Make sure our collision callback is blanked out correctly

	SetPaused(false);
	SetState(E_RLC_STATE_CONTROLLERS_AND_DATA_INITIALISED);	
}

void RaceLogicCore::InitialiseDataStructures()
{
	m_pTrackData = new RLTrackData();
	m_pTrackData->Initialise();

	m_pTimeData = new RLTimeData();
	m_pTimeData->Initialise();
}

void RaceLogicCore::InitialiseControllers()
{
	for(int iComponent = 0; iComponent < E_RLCOMPONENTTYPE_MAX; iComponent++)
	{
		switch(iComponent)
		{
		case E_RLCOMPONENTTYPE_DATABASE_CONTROLLER:
			{
				mComponents.PushBack(new RLDatabaseController());
				break;
			}
		}

		//Set a pointer to the component
		mComponents[iComponent]->SetRLC(this);
	}
}

void RaceLogicCore::InitialiseViewportDetails()
{
	// Copy the information from the level state into the mViewportDetails array
	const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	mViewportDetails.Resize(levelState.mNumViewports);

	for (unsigned int iv=0; iv < levelState.mNumViewports; iv++)
	{
		S32 controller = levelState.mCars[iv].mController;

		mViewportDetails[iv].mVehicleIndex = iv;
		mViewportDetails[iv].mControllerIndex = controller;
	}


#ifndef CFG_GOLD
	// Automation: When the player car is set to 'ai' for automation, make sure viewport is still set up right for them
	for (unsigned int iv=0; iv < levelState.mNumViewports; iv++)
	{
		if ((mViewportDetails[iv].mControllerIndex == INVALID_CONTROLLER) && (levelState.mCars[iv].mAutomatedHuman == true))
		{
			mViewportDetails[iv].mVehicleIndex = iv;
			mViewportDetails[iv].mControllerIndex = iv;
		}
	}
#endif
}

void RaceLogicCore::CheckInitialisation()
{
	if(GetState() == E_RLC_STATE_READY)
		return;

	//Put all stuff that needs to be checked here

	//Check to see if we've begun to create the vehicles
	if(mVehicles.Size() < int(C_GameApp::GetInstance()->GetGame()->GetLevelState().mNumCars) && GetTrackData()->mWarnAboutMissingVehicles)
	{
		//
		LOGERROR(g_RLCoreLog, "RaceLogicCore::CheckInitialisation() > We've not been able to create all the vehicles we were trying to...this is bad...but possibly not fatal");

		GetTrackData()->mWarnAboutMissingVehicles = false;
		return;
	}

	//Check that we've got some vehicles
	for(int vehicleIndex = 0; vehicleIndex < mVehicles.Size(); vehicleIndex++)
	{
		if(!mVehicles[vehicleIndex].mLoaded)
			//This vehicle's not loaded so we're not initialised yet
			return;
	}

	//If we're here then we should be fully initialised so...
	SetState(E_RLC_STATE_VEHICLE_RESOURCES_LOADED);
}

void RaceLogicCore::SetState(enum E_RLC_STATE eNewState)
{
	//TODO Do some sort of checking here?
	meState = eNewState;
}

const enum E_RLC_STATE RaceLogicCore::GetState() const
{
	return meState;
}

void RaceLogicCore::Delete()
{
	DeleteDataStructures();
	DeleteControllers();
}

void RaceLogicCore::DeleteDataStructures()
{
	SAFE_DELETE_PTR(m_pTrackData);
	SAFE_DELETE_PTR(m_pTimeData);	
	
	mVehicles.Clear();
}

void RaceLogicCore::DeleteControllers()
{
	for(int i = 0; i < mComponents.Size(); i++)
	{
		SAFE_DELETE_PTR(mComponents[i]);
	}
}

void RaceLogicCore::SetVehicleLoadingHandle(Actor::C_ActorHandle p_Handle)
{
	m_LoadingVehicleHandle = p_Handle;
}

void RaceLogicCore::HandleVehicleLoadedUI(bool p_Status)
{
	RLVehicleData* pRLVehicle = NULL;

	if (mVehicles.Size() == 0)
	{
		mVehicles.PushBack();
		pRLVehicle = &mVehicles.Back();
		pRLVehicle->Initialise();
		Game::C_Game::GetInstance()->GetLevelState().mNumCars = 1;
	}
	else
	{
		pRLVehicle = &mVehicles[0];
		if (pRLVehicle->m_pVehicleActor)
		{
			pRLVehicle->m_pVehicleActor->SetEvictDelay(5);
			pRLVehicle->m_pVehicleActor->SendMessageToActor(pRLVehicle->m_pVehicleActor->GetHandle(), Actor::E_ActorMsg_Kill);			
		}

		#if PLATFORM_PS3
		Vehicle::C_VehicleVertexBufferManager::GetInstance().DestroyVertexBuffers();
		#endif
	}

	pRLVehicle->mActorHandle	= m_LoadingVehicleHandle;
	pRLVehicle->mCreated		= true;
	pRLVehicle->mLoaded			= true;
	pRLVehicle->mVehicleIndex	= 0;
	pRLVehicle->mValid			= true;
	pRLVehicle->m_pVehicleActor	= (Game::Actors::C_VehicleActor*)m_pGameWorld->GetActorList()->GetActorPtr(pRLVehicle->mActorHandle);
	
	if (pRLVehicle->m_pVehicleActor)
	{
		pRLVehicle->m_pVehicleActor->SendMessageToActor(pRLVehicle->mActorHandle, Game::E_VehicleMessage_Enable);
		RLStartPosition startPos;
		startPos.Initialise();
		startPos = GetStartPosition(0);

		U8 data[1024 * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> stack(data, 1024);
		Collision::C_Ray ray;
		Collision::C_CollisionFilter rayCollisionFilter( Game::PolyFlags::kCameraSolid, Game::PolyFlags::kCameraSolid);
		ray.InitialiseRay(startPos.mWorldMatrix.AW(), Maths::Constants<Maths::HVec3>::UnitAxisNegativeY(), &rayCollisionFilter);
		GetPhysicsScene()->DoRayTrace(ray, stack);
		Collision::C_RayTracingResult& result = ray.m_result;
		if( result.Hit() )
		{	
			startPos.mWorldMatrix.AW(result.HitPoint());
			startPos.mWorldPosition = result.HitPoint();
			Game::Actors::C_VehicleResetAtWorldPositionMsg resetMsg(pRLVehicle->mActorHandle, 0, startPos.mWorldMatrix, Maths::HVec3(0.0f, 0.0f, 0.0f));
			pRLVehicle->m_pVehicleActor->SendMessageToActor(pRLVehicle->mActorHandle, &resetMsg);
		}
	}
	else
		LOGERROR(g_RLCoreLog, "HandleVehicleLoadedUI : C_VehicleActor == NULL");

	m_LoadingVehicleHandle		= Actor::C_ActorHandle();
}

void RaceLogicCore::StartAmaxVehicleLoad( AmaxVehicle* pAmaxVehicle, bool bMultiplayer, U32 uTeamColour )
{
	if ( !pAmaxVehicle )
		return;

	if ( !UI::C_UiXtConfig::Instance() )
		return;

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();

	//Set the RLC state so we can wait for the vehicles to load
	SetState(E_RLC_STATE_LOAD_VEHICLE_RESOURCES);

	#if PLATFORM_PS3
	Vehicle::C_VehicleVertexBufferManager::GetInstance().DestroyVertexBuffers();
	#endif

	Maths::HMat43 spawnMatrix;

	Actor::C_ActorCreationDataFactoryHelper<Actors::C_VehicleActorCreate> pVehicleActorCreate;

	UI::C_UiXtConfig* pConfig = UI::C_UiXtConfig::Instance();
	spawnMatrix = Maths::HMat43(Maths::HMat33(Maths::C_Quaternion(Maths::HVec3(0.0f, 1.0f, 0.0f), Maths::DegToRad((float)pConfig->m_CarOrientation))),
				 			    Maths::HVec3((float)pConfig->m_CarPosition.vx, (float)pConfig->m_CarPosition.vy, (float)pConfig->m_CarPosition.vz));

	pVehicleActorCreate->m_Dormant = true;

	pVehicleActorCreate->m_WorldMatrix			= Store(spawnMatrix);
	pVehicleActorCreate->m_pGameWorld			= GetGameWorld();
	pVehicleActorCreate->m_pRLC					= this;

	pVehicleActorCreate->m_ModelFilename		= Util::C_String::Format("models/vehicles/%s.model", static_cast<const char*>(pAmaxVehicle->tag));
	if (pAmaxVehicle->wheelModel)
		pVehicleActorCreate->m_WheelFilename	= Util::C_String::Format("models/vehicles/wheels/%s.model|wheel|10", static_cast<const char*>(pAmaxVehicle->wheelModel->fileName));
	else
		pVehicleActorCreate->m_WheelFilename	= "models/vehicles/wheels/default.model|wheel|10";

#ifndef CFG_GOLD
	//Check file exists
	Sys::C_FilePath filePath = Sys::GetDefaultFileSystem()->GetWorkingDirectory() + "/" + pVehicleActorCreate->m_ModelFilename;
	if (!Sys::GetDefaultFileSystem()->FileExists(filePath))
		pVehicleActorCreate->m_ModelFilename	= "models/vehicles/shelby_mustanggt500_stock.model";
#endif
	pVehicleActorCreate->m_ModelFilename		+= "|car|10";

	pVehicleActorCreate->m_DynamicsFilename		= static_cast<const char*>(pAmaxVehicle->dynamics->name);
	pVehicleActorCreate->m_ControlType			= E_VEHICLE_CONTROL_TYPE_SHOWROOM;

	pVehicleActorCreate->m_XtIndex				= pAmaxVehicle->ID;
	pVehicleActorCreate->m_HasLogicActor		= false;

	// set team colour
	if ( uTeamColour )
	{
		pVehicleActorCreate->m_XtResprayJobId = uTeamColour;
	}
	else
	{
		// set respray colour
		if ( pGameProfile )
			pVehicleActorCreate->m_XtResprayJobId = pGameProfile->ResprayGet( pAmaxVehicle->ID, bMultiplayer );
		else
			pVehicleActorCreate->m_XtResprayJobId = 0;
	}

	GetGameWorld()->GetActorList()->CreateActor(pVehicleActorCreate.Get());	
}

void RaceLogicCore::ShowLockedCarSFX()
{
	m_pGarageLockedCarSFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(	GetDatabase()->mRLSystems[0]->mPickupSystem->mParameters->mGarageLockedCarSFX, 
																					GetGameWorld()->GetContextProvider(), 
																					C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr());
	if (!m_pGarageLockedCarSFX)
		return;
	
	UI::C_UiXtConfig* pConfig = UI::C_UiXtConfig::Instance();
	m_GarageSFXMatrix = Maths::HMat43(	Maths::HMat33(Maths::C_Quaternion(Maths::HVec3(0.0f, 1.0f, 0.0f), Maths::DegToRad((float)pConfig->m_CarOrientation))),
										Maths::HVec3((float)pConfig->m_CarPosition.vx, (float)pConfig->m_CarPosition.vy, (float)pConfig->m_CarPosition.vz));

	m_pGarageLockedCarSFX->Start();
	m_pGarageLockedCarSFX->UpdateWorldMatrix(m_GarageSFXMatrix);
	m_pGarageLockedCarSFX->Update( 0 );

	m_fGarageSFXTime = 0.0f;

	if (mVehicles.Size() > 0)
	{
		RLVehicleData* pRLVehicle = &mVehicles[0];
		if (pRLVehicle->m_pVehicleActor)
			pRLVehicle->m_pVehicleActor->SetVisibility(false, -1, true, false);
	}

	RLPickupSpecialFX sfx;
		
	sfx.mLocalSpecialFX=C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetDatabase()->mRLSystems[0]->mPickupSystem->mParameters->mGarageSpecialFX;
	
	sfx.mDurationLoop.mStart	= 0.0f;
	sfx.mDurationLoop.mEnd		= 0.0f;	
	sfx.mDurationOutro.mStart	= 0.0f;
	sfx.mDurationOutro.mEnd		= 0.0f;
	sfx.mDurationIntro.mStart	= 0.0f;
	sfx.mDurationIntro.mEnd		= 2.0f;
	sfx.mFrequency				= 1.0f;

	Game::C_InGameSpecialFXManager::Instance().SetUpWorldSFX(sfx,m_GarageSFXMatrix,0,-1,-1,Maths::HVec3(1,1,1));
}

void RaceLogicCore::HideLockedCarSFX()
{
	if (m_pGarageLockedCarSFX)
	{
		m_pGarageLockedCarSFX->Stop();
		SAFE_DELETE_PTR(m_pGarageLockedCarSFX);
	}
}

bool RaceLogicCore::ShouldLoadVehicle(AmaxVehicle* p_pVehicle)
{
	if (p_pVehicle && mVehicles.Size() > 0)
	{
		RLVehicleData* pRLVehicle = &mVehicles[0];
		if (pRLVehicle->m_pVehicleActor)
		{
			if (pRLVehicle->m_pVehicleActor->GetXtIndex() == p_pVehicle->ID && pRLVehicle->m_pVehicleActor->GetVisibility())
				return false;
		}
	}

	return true;
}

void RaceLogicCore::StartVehicleLoad( RaceBook::C_RaceBookVehicle* pRaceBookVehicle )
{
	AmaxVehicle* pAmaxVehicle = NULL;
	bool bMultiplayer = false;

	if ( !pRaceBookVehicle )
	{
		Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();

		if ( !pGameProfile )
			return;

		if ( C_GameApp::GetInstance()->GetRaceBookLayer()->SetMpUICar() || !pGameProfile->mRaceBookProfile )
		{
			pAmaxVehicle = C_GameVehicleData::Instance().mVehicles[ (GAMEDATA_ID)pGameProfile->mMultiplayerProfile.GetLastCarUsed() ];
			bMultiplayer = true;
		}
		else
		{
			XtId vehicleId = SP_GameGetLastVehicle( SP_GetCurrentSaveGame() );
			pAmaxVehicle = C_GameVehicleData::Instance().mVehicles[ (GAMEDATA_ID)vehicleId ];

			if (!pAmaxVehicle)
				pAmaxVehicle = C_GameVehicleData::Instance().mVehicles[ "Audi_TT_Stock" ];
		}
	}
	else
	{
		pAmaxVehicle = C_GameVehicleData::Instance().mVehicles[ (GAMEDATA_ID)pRaceBookVehicle->mBaseModelID ];
	}

	StartAmaxVehicleLoad( pAmaxVehicle, bMultiplayer );
}

void RaceLogicCore::LoadNewVehicle(S32 p_Slot)
{
	if (p_Slot < 0 || p_Slot > mVehicles.Size())
		return;

	RLVehicleData* pRLVehicle = NULL;

	if (p_Slot == mVehicles.Size())
	{
		mVehicles.PushBack();
		pRLVehicle = &mVehicles.Back();
		pRLVehicle->Initialise();
	}
	else
	{
		pRLVehicle = &mVehicles[p_Slot];
		if (pRLVehicle->m_pVehicleActor)
		{
			C_DriverManager::Instance()->DestroyDriver(p_Slot);
			
			pRLVehicle->m_pVehicleActor->SendMessageToActor(pRLVehicle->m_pVehicleActor->GetHandle(), ActorMsg::E_ResourceMsg_UnloadAll);
			pRLVehicle->Initialise();
		}
	}

	const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	Maths::HMat43 startPositionWorldMatrix;
	RLVehicleData newVehicle;
	RLStartPosition startPos;

	Actor::C_ActorHandle driverActorHandle;

	if (!levelState.mInUI)
	{
		C_DriverActorCreate driverCreate;

		// Use the base character record to decide which model and skeleton to use.
		RaceBookCharacter* pRaceBookCharacter = C_RaceBookData::Instance().mCharacters[(GAMEDATA_ID)levelState.mCars[p_Slot].mCharacterID];
		if (pRaceBookCharacter != NULL)
			driverCreate.characterDef = pRaceBookCharacter->base_character;
		else
			driverCreate.characterDef.Clear();
			
		driverCreate.vehicleIndex = p_Slot;
		const AmaxVehicle* pAmaxVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)levelState.mCars[p_Slot].mCarID];
		if (pAmaxVehicle != NULL)
			driverCreate.vehicleDef = XtHandle<AmaxVehicle>(XtGetRecord(C_GameApp::GetInstance()->GetGlobalXtDatabase(), pAmaxVehicle->ID));
		else
			driverCreate.vehicleDef.Clear();
				
		C_DriverManager::Instance()->CreateDriver(p_Slot, driverCreate);	
	}

	Actor::C_ActorCreationDataFactoryHelper<Actors::C_VehicleActorCreate> pVehicleActorCreate;

	if(levelState.mReplayMode == E_ReplayMode_Play)
	{
		startPositionWorldMatrix = levelState.mCars[p_Slot].m_InitialWorldMatrix;
	}
	else 
	{
		startPos.Initialise();
		UI::C_UiXtConfig* pConfig = UI::C_UiXtConfig::Instance();
		if (levelState.mInUI && pConfig)
		{		
			startPos.mWorldMatrix = Maths::HMat43(Maths::HMat33(Maths::C_Quaternion(Maths::HVec3(0.0f, 1.0f, 0.0f), Maths::DegToRad((float)pConfig->m_CarOrientation))),
												  Maths::HVec3((float)pConfig->m_CarPosition.vx, (float)pConfig->m_CarPosition.vy, (float)pConfig->m_CarPosition.vz));
		}
		else
		{
			startPos.mWorldMatrix.AW(Maths::HVec3(0.0f, 0.5f + (float)p_Slot * 2.0f, 0.0f));
			startPos.mWorldPosition = startPos.mWorldMatrix.AW();
		}
		startPos.mWorldPosition = startPos.mWorldMatrix.AW();
		startPositionWorldMatrix = startPos.mWorldMatrix;
		pVehicleActorCreate->m_Dormant = true;
	}

	//if (levelState.mCars[p_Slot].mStartDisabled)
	if (p_Slot >= (S32) C_GameApp::GetInstance()->GetGame()->GetLevelState().mNumCars)
		pVehicleActorCreate->m_Dormant = true;

	pVehicleActorCreate->m_WorldMatrix = Store(startPositionWorldMatrix);
	pVehicleActorCreate->m_pGameWorld = GetGameWorld();
	pVehicleActorCreate->m_pRLC = this;

	pVehicleActorCreate->m_ModelFilename = C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[p_Slot].mFilename;
	pVehicleActorCreate->m_WheelFilename = C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[p_Slot].mWheelFilename;
	pVehicleActorCreate->m_DynamicsFilename = C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[p_Slot].mDynamicsFilename;
	pVehicleActorCreate->m_ControlType = C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[p_Slot].mControlType;

	pVehicleActorCreate->m_VehicleIndex = p_Slot;
	pVehicleActorCreate->m_XtIndex = C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[p_Slot].mCarID;
	if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer())
		pVehicleActorCreate->m_HasLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(p_Slot) == NULL ? false : true;
	else
		pVehicleActorCreate->m_HasLogicActor = false;
	pVehicleActorCreate->m_XtResprayJobId = C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[p_Slot].mResprayID;
	pVehicleActorCreate->m_XtPaintJobId = C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[p_Slot].mResprayID; //is one of these redundant?

	//pVehicleActorCreate->m_ResprayColour = C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[p_Slot].mResprayColour;
	
	mVehicles[p_Slot].mVehicleIndex = p_Slot;
	mVehicles[p_Slot].mCreated = true;
	mVehicles[p_Slot].mLoaded = false;

	Actor::C_ActorHandle vehicleActorHandle = GetGameWorld()->GetActorList()->CreateActor(pVehicleActorCreate.Get());
	mVehicles[p_Slot].mActorHandle = vehicleActorHandle;

	//Now get the actor itself stuff with actor handle
	const Actor::C_Actor* pActor = m_pGameWorld->GetActorList()->GetActorPtr(mVehicles[p_Slot].mActorHandle);

	//Store the pointer to the actor
	mVehicles[p_Slot].m_pVehicleActor = (Game::Actors::C_VehicleActor *)pActor;	//PK - apologies, this will change in time

//	if (!levelState.mInUI && !levelState.mCars[p_Slot].mCutSceneOnly)
	if (p_Slot < (S32) C_GameApp::GetInstance()->GetGame()->GetLevelState().mNumCars)
	{
		if(mVehicles[p_Slot].m_pVehicleActor)
		{
			Game::Actors::C_VehicleActor *pActorForAI = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor>(mVehicles[p_Slot].m_pVehicleActor);
			CreateAIDriver(	pActorForAI,
							C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[p_Slot].mAICharacterName, 
							C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[p_Slot].mDynamicsFilename,
							C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[p_Slot].mAIDriver,
							C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[p_Slot].mAISteering);
		}
	}
}

void RaceLogicCore::SpawnVehicleActors(bool bDelayPositioning)
{
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	if (levelState.mInUI)
		return;

	//See how many vehicles we're going to have
	int noVehicles = levelState.mNumCars+levelState.mNumAdditionalCars;
	for(int vehicleIndex = 0; vehicleIndex < noVehicles; vehicleIndex++)
	{
		LoadNewVehicle(vehicleIndex);
	}	

	//Set the RLC state so we can wait for the vehicles to load
	SetState(E_RLC_STATE_LOAD_VEHICLE_RESOURCES);
}

void RaceLogicCore::PlaceVehicles()
{
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	//See how many vehicles we're going to have
	int noVehicles = levelState.mNumCars+levelState.mNumAdditionalCars;
	for(int vehicleIndex = 0; vehicleIndex < noVehicles; vehicleIndex++)
	{
		if (!mVehicles[vehicleIndex].m_pVehicleActor)
			continue;

		RLStartPosition startPos;
		startPos.Initialise();
		startPos = GetStartPosition(levelState.mCars[vehicleIndex].mStartPosition);
		
		UI::C_UiXtConfig* pConfig = UI::C_UiXtConfig::Instance();
		if (levelState.mInUI && pConfig)
		{		
			startPos.mWorldMatrix = Maths::HMat43(Maths::HMat33(Maths::C_Quaternion(Maths::HVec3(0.0f, 1.0f, 0.0f), Maths::DegToRad((float)pConfig->m_CarOrientation))),
												  Maths::HVec3((float)pConfig->m_CarPosition.vx, (float)pConfig->m_CarPosition.vy, (float)pConfig->m_CarPosition.vz));
		}

		//Check to see if we can use this start position
		if(startPos.mStartNumber < 0)
		{
			//No dice
			LOGDEBUG(g_RLCoreLog, "RaceLogicCore::SpawnVehicles() > Couldn't find start position for vehicle therefore spawning at 0,0");
			startPos.mStartNumber = vehicleIndex;
		}
		mVehicles[vehicleIndex].m_pVehicleActor->SetSpawnMatrices(startPos.mWorldMatrix);
		Game::Actors::C_VehicleResetAtWorldPositionMsg resetMsg(mVehicles[vehicleIndex].mActorHandle, vehicleIndex, startPos.mWorldMatrix, Maths::HVec3(0.0f, 0.0f, 0.0f));
		mVehicles[vehicleIndex].m_pVehicleActor->SendMessageToActor(mVehicles[vehicleIndex].mActorHandle, &resetMsg);
		if (levelState.mCars[vehicleIndex].mStartDisabled)
		{
			mVehicles[vehicleIndex].m_pVehicleActor->SendMessageToActor(mVehicles[vehicleIndex].mActorHandle, Game::E_VehicleMessage_Disable);
			mVehicles[vehicleIndex].m_pVehicleActor->DeferredUpdate();
		}
		else
		{
			if (vehicleIndex < (S32) C_GameApp::GetInstance()->GetGame()->GetLevelState().mNumCars)
			{
				mVehicles[vehicleIndex].m_pVehicleActor->SendMessageToActor(mVehicles[vehicleIndex].mActorHandle, Game::E_VehicleMessage_Enable);
			}
		}
	}
}

//Copy the viewport settings from the RaceLogicCore into the Camera System viewports
void RaceLogicCore::NotifyCameraSystemAboutViewports()
{
	// Set switcher to gamefeed so it doesn't go out of sync:
	AmaxCam::Switcher().SetType(AmaxCam::E_DebugCam_GameFeed);

	bool debugMenu = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() == Game::E_GameMode_Debug;

	AmaxCam::C_CameraSystem& cameraSystem = AmaxCam::CameraSystem();
	for (S32 i = 0; i < mViewportDetails.Size(); i++)
	{
		const RLViewportDetails& viewport = mViewportDetails[i];
		
		S32 vehicleIndex = viewport.mVehicleIndex;
		// Set the debug viewport to show a game viewport:
		if (vehicleIndex < 0 || vehicleIndex >= mVehicles.Size())
		{
			// Just duplicate viewport 0:
			cameraSystem.DebugFeedShow(i, AmaxCam::E_DebugCam_GameFeed, 0);
			continue;
		}
		else
		{
			cameraSystem.DebugFeedShow( i, AmaxCam::E_DebugCam_GameFeed, i);
			// Set the actor handle of the vehicle we are following:
			cameraSystem.SetFocusHandle(i, mVehicles[i].mActorHandle);
		}
		
		//// Show the vehicle feed in this viewport if we are launching from the debug menu:
		if (debugMenu)
			cameraSystem.GameFeedShow(i, AmaxCam::E_GameCam_VehicleFeed);

		AmaxCam::C_VehicleFeed* vehicleFeed = cameraSystem.VehicleFeed(i);
		if (vehicleFeed)
			vehicleFeed->SetPadIndex(viewport.mControllerIndex);

		cameraSystem.ResetVehicleFeed(i);
	}

	//is this the right place to put this? solves an issue whereby split screen code is being called when
	//entering the single player front end and crashing the game (see bug #s 20067 and 19671) - [mtaylor]
	C_InGameStartLineInfoManager::Instance().RecheckViewports();
}

void RaceLogicCore::CreateAIDriver(Game::Actors::C_VehicleActor *pActor, Util::C_String &AIname, Util::C_String &Vehiclename,  S32 index, S32 steeringIndex)
{
	AIAmax::AILoadDriveraMsg	aiDriverMessage;
	aiDriverMessage.m_driver.m_pVehicleActor = pActor;
	aiDriverMessage.m_driver.m_CharacterName = AIname;
	aiDriverMessage.m_driver.m_VehicleName = Vehiclename;
	aiDriverMessage.m_driver.m_AIIndex = index;
	aiDriverMessage.m_driver.m_SteeringAI = steeringIndex;

	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&aiDriverMessage);

}

const RLStartPosition RaceLogicCore::GetStartPosition(const int startNumber, const enum E_STARTPOSITION_TYPE eStartPositionType)
{
	//Look through our start positions for the correct type, we'll do this using brute force as its not going to happen everyframe

	for(int startPositionArrayIndex = 0; startPositionArrayIndex < GetTrackData()->mStartPositions.Size(); startPositionArrayIndex++)
	{
		const RLStartPosition& pos = GetTrackData()->mStartPositions[startPositionArrayIndex];

		if (pos.mStartNumber == startNumber && pos.meType == eStartPositionType)
			return pos;
	}

	if (GetTrackData()->mStartPositions.Size() > 0)
		Automation::SendProblemFoundMsg("LEVEL", startNumber, "NOSTARTPOS");

	LOGERROR(g_RLCoreLog, "RaceLogicCore::GetStartPosition() > Can't find start positions for start number %d of type %d", startNumber, eStartPositionType);

	RLStartPosition startPos;
	startPos.Initialise();
	return startPos;
}

const RLStartPosition& RaceLogicCore::GetRandomStartPosition(const enum E_STARTPOSITION_TYPE eStartPositionType) const
{
	const int numberOfStartPositions = GetNumberOfStartPositionsOfType(eStartPositionType);
	BIZ_ASSERT_MSG(numberOfStartPositions, "RaceLogicCore::GetRandomStartPosition() > Couldn't find a single start position of type %d, starting at (0,0,0)", eStartPositionType);
	
	//Get a "random" index from rand
	int randomStartPositionArrayIndex = GenerateRandomSpawnPointIndex(numberOfStartPositions);

	return GetTrackData()->mStartPositions[randomStartPositionArrayIndex];
}

const RLStartPosition& RaceLogicCore::GetBestStartPosition(const int vehicleIndex, const enum E_STARTPOSITION_TYPE eStartPositionType) const
{
	const int numberOfStartPositions = GetNumberOfStartPositionsOfType(eStartPositionType);
	BIZ_ASSERT_MSG(numberOfStartPositions, "RaceLogicCore::GetBestStartPosition() > Couldn't find a single start position of type %d, starting at (0,0,0)", eStartPositionType);

	if(C_Game::GetInstance()->GetLevelState().mNumCars == 1)
	{
		// There's just one guy, but we still want to spawn him at a random point (all are presumed available) to spice up his lonely miserable existance
		return GetRandomStartPosition(eStartPositionType);
	}
	else
	{
		return GetTrackData()->mStartPositions[DetermineBestSpawnPointIndex(numberOfStartPositions, vehicleIndex)];
	}
}

const U32 RaceLogicCore::GenerateRandomSpawnPointIndex(const int numberOfStartPositions) const
{
	const U32 raceTime = GetTimeData()->mRaceTime;
	srand( raceTime * raceTime * raceTime );

	const U32 randomNumber = rand();
	const U32 randomStartPosition = randomNumber % numberOfStartPositions;

	BIZ_ASSERT_MSG(randomNumber > 0, "RaceLogicCore::GenerateRandomSpawnPointIndex() > Invalid random number, n00bism reigns");
	BIZ_ASSERT_MSG((randomStartPosition >= 0 && randomStartPosition < numberOfStartPositions), "RaceLogicCore::GenerateRandomSpawnPointIndex() > Invalid random start position");
	return randomStartPosition;
}

const int RaceLogicCore::DetermineBestSpawnPointIndex(const int numberOfStartPositions, const int spawningVehicleIndex) const
{
	// Brute force, there surely must be a better way but I can't see it right now

	int safestSpawnIndexFoundSoFar = 0;
	float distanceSqrdForSafestSoFar = -1.f;
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	// If we just iterate through the spawns with no randomness, we will give an unfair bias to the smaller indexes (particularly in races with few cars)
	// So let's find a random place to start our loop
	int randomLoopStartIndex = GenerateRandomSpawnPointIndex(numberOfStartPositions);

	//we loop through all spawn positions and find the first safe one
	for( int i=0; i < numberOfStartPositions; i++ )
	{
		float distanceSqrdClosestVehicleToSpawn = Maths::Constants<F32>::Max(); //a big number that someone will definitely be closer than :)
		int spawnIndex = (i+randomLoopStartIndex) % numberOfStartPositions;
		RLStartPosition thisSpawnPosition = GetTrackData()->mStartPositions[spawnIndex];

		//find the closest vehicle to this spawn position
		for( U8 vehicleIndex = 0; vehicleIndex<levelState.mNumCars; vehicleIndex++ )
		{
			if(vehicleIndex == spawningVehicleIndex)
			{
				continue; //it's ok if it's ourselves in range of the spawn point
			}

			Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehicleIndex);
			if(pLogicActor)
			{
				float distanceSqrdThisVehicleToSpawn = pLogicActor->GetWorldMatrix().AW().DistanceSquared(thisSpawnPosition.mWorldMatrix.AW());
				if( distanceSqrdThisVehicleToSpawn < distanceSqrdClosestVehicleToSpawn )
				{
					distanceSqrdClosestVehicleToSpawn = distanceSqrdThisVehicleToSpawn;
				}
			}
		}

		if( distanceSqrdClosestVehicleToSpawn > (g_spawnPointMinSafeDistance * g_spawnPointMinSafeDistance) )
		{
			LOGINFO(g_RLCoreLog, "RaceLogicCore::DetermineBestSpawnPoint() has found spawn point %d for vehicle %d (closest vehicle was %.2f metres away)",
				spawnIndex, spawningVehicleIndex, Maths::Sqrt(distanceSqrdClosestVehicleToSpawn));
			return spawnIndex; //we've found a safe one
		}
		else if(distanceSqrdForSafestSoFar < distanceSqrdClosestVehicleToSpawn )
		{
			//it's not safe, but it's safer than the others
			distanceSqrdForSafestSoFar = distanceSqrdClosestVehicleToSpawn;
			safestSpawnIndexFoundSoFar = spawnIndex;
		}
	}

	//if we've got through to here, every spawn point has another car within the defined "vicinity", we'll have to go with the safest
	LOGINFO(g_RLCoreLog, "RaceLogicCore::DetermineBestSpawnPoint() discounted all spawn points for vehicle %d, best spawn point was %d with the nearest vehicle being %.2f metres from it",
				spawningVehicleIndex, safestSpawnIndexFoundSoFar, Maths::Sqrt(distanceSqrdForSafestSoFar));
	return safestSpawnIndexFoundSoFar;
}

const int RaceLogicCore::GetNumberOfStartPositionsOfType(const enum E_STARTPOSITION_TYPE eStartPositionType) const
{
	int numberOfStartPositions = 0;
	for(int startPositionArrayIndex = 0; startPositionArrayIndex < GetTrackData()->mStartPositions.Size(); startPositionArrayIndex++)
	{
		const RLStartPosition& pos = GetTrackData()->mStartPositions[startPositionArrayIndex];
		if(pos.meType == eStartPositionType)
			numberOfStartPositions++;
	}

	return numberOfStartPositions;
}

void RaceLogicCore::SwitchUIFocusCar()
{
	if (Game::C_Game::GetInstance()->GetLevelState().mInUI)
	{
		S32 oldSlot = g_debugUIFocusCar;
		if (oldSlot >= 0 && oldSlot < mVehicles.Size())
		{
			if (mVehicles[oldSlot].m_pVehicleActor)
			{
				//FIXME: Unload, don't completely remove?
				C_DriverManager::Instance()->DestroyDriver(oldSlot); 

				mVehicles[oldSlot].m_pVehicleActor->SendMessageToActor(mVehicles[oldSlot].m_pVehicleActor->GetHandle(), E_VehicleMessage_Disable);
				mVehicles[oldSlot].m_pVehicleActor->SendMessageToActor(mVehicles[oldSlot].m_pVehicleActor->GetHandle(), ActorMsg::E_ResourceMsg_UnloadAll);
				mVehicles[oldSlot].Initialise();
			}
		}
		if (g_debugUIFocusCar == 0)
			g_debugUIFocusCar = 1;
		else
			g_debugUIFocusCar = 0;

		if (g_debugUIFocusCar >= mVehicles.Size())
			return;

		if (mVehicles[g_debugUIFocusCar].m_pVehicleActor)
			mVehicles[g_debugUIFocusCar].m_pVehicleActor->SendMessageToActor(mVehicles[g_debugUIFocusCar].m_pVehicleActor->GetHandle(), E_VehicleMessage_Enable);

		g_FocusVehicleIndex = g_debugUIFocusCar;
		SetViewportFocusVehicleIndex(0, g_FocusVehicleIndex);
	}
}


void RaceLogicCore::SkipFrames( S32 p_NumFrames )
{
	m_UnPauseForFrames = p_NumFrames;
	Framework::C_ApplicationBase::GetInstance()->Resume( Framework::E_LayerPause_Freeze );	
}

Debug::Float	g_debugGarageEffectRotSpeed("Amax.Garage.SFXRotateSpeed", 0.05f);

bool RaceLogicCore::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	PERF_EVENT("RaceLogicCore::OnMessage");

	switch(pMessage->Type())
	{
	case Game::E_GameflowMessage_LoadResources:
		{
			assert(GetState() >= E_RLC_STATE_CONTROLLERS_AND_DATA_INITIALISED);

			SetState(E_RLC_STATE_CREATE_VEHICLES);	
			return true;
		}
	case Game::E_GameflowMessage_StartRaceLoad:
		{
			//Check we're already to go
			assert(GetState() >= E_RLC_STATE_READY);

			//Should do something here
			return true;
		}
	case Game::E_GameflowMessage_RaceStarted:
		{
			GetTrackData()->mPickupsEnabled = true;
			GetTrackData()->mRaceStarted = true;

			m_pGameWorld->GetActorList()->GetSubscriberSystem().BroadcastMessage( Game::E_VehicleMessage_RaceStarted );
			//m_pGameWorld->GetPhysicsScene()->RegisterCollisionCallback((Gravity::C_CollisionCallback *)GetDamageController());

			return true;
		}

	case Game::E_GameflowMessage_RaceFinished:
		{
			GetTrackData()->mRaceFinished = true;
			GetTrackData()->mPickupsEnabled = false;
			return true;
		}

	case GameMessaging::E_GameMsg_Update:
		{			
			if (m_pGarageLockedCarSFX)
			{
				const GameMessaging::C_PreUpdateMsg* pMsg = Messaging::MessageCast<GameMessaging::C_PreUpdateMsg>(pMessage);

				Maths::HVec3 pos = m_GarageSFXMatrix.AW();
				m_GarageSFXMatrix.AW(Maths::HVec3());
				m_GarageSFXMatrix = Maths::HMat43(Maths::RotY(g_debugGarageEffectRotSpeed)) * m_GarageSFXMatrix;
				m_GarageSFXMatrix.AW(pos);

				float fDuration = m_pGarageLockedCarSFX->GetDuration();
				m_fGarageSFXTime += pMsg->GetValue();
				if (m_fGarageSFXTime >= fDuration)
					m_fGarageSFXTime = 0.0f;
				m_pGarageLockedCarSFX->UpdateWorldMatrix(m_GarageSFXMatrix);
				m_pGarageLockedCarSFX->Update(m_fGarageSFXTime);				
			}
			ProcessUpdate();
			return true;
		}
	case GameMessaging::E_GameMsg_PausedUpdate:
		{
			PausedUpdate();
			return true;
		}
	case Actor::E_ActorMsg_Created:
		{
			if(GetState() < E_RLC_STATE_CONTROLLERS_AND_DATA_INITIALISED)
				return false;

			Actor::C_ActorHandle actorHandle = Messaging::MessageCast<Actor::C_ActorCreatedMsg>(pMessage)->GetActorHandle();
			// do stuff with actor handle
			const Actor::C_Actor* pActor = m_pGameWorld->GetActorList()->GetActorPtr(actorHandle);
			
			switch(pActor->GetActorTypeId())
			{
			case Game::E_ActorTypeId_StartPosition:
				{
					RLStartPosition newStartPosition;
					newStartPosition.Initialise();

					const ::Actors::StartPositionActor *pStartPositionActor = Actor::ActorCast< ::Actors::StartPositionActor > (pActor);

					newStartPosition.mStartNumber = pStartPositionActor->GetStartNumber();
					newStartPosition.mWorldPosition = pStartPositionActor->GetPosition();
					newStartPosition.mWorldMatrix = pStartPositionActor->GetMatrix();

					//Start ray-trace
					Collision::C_CollisionFilter filter(Game::PolyFlags::kWheelSolid, Game::PolyFlags::kWheelSolid, 0, 0, 0);	
					Collision::C_Ray ray;
					ray.InitialiseRay(newStartPosition.mWorldPosition + Maths::Constants<Maths::HVec3>::UnitAxisPositiveY(), newStartPosition.mWorldPosition, &filter);
					GetPhysicsScene()->DoRayTrace(ray, GetPhysicsScene()->m_Stack);
					if (ray.GetResult().Hit())
					{
						newStartPosition.mWorldPosition = ray.GetResult().HitPoint();
						newStartPosition.mWorldMatrix.AW(ray.GetResult().HitPoint());
					}

					GetTrackData()->mStartPositions.PushBack(newStartPosition);
					break;
				}
			case Game::E_ActorTypeId_Vehicle:
				{
					/*
					const S8 vehicleIndex = GetVehicleIndexFromActorHandle(actorHandle);

					if( VehicleValid(vehicleIndex) )
					{
						// do stuff with actor handle
						const Actor::C_Actor* pActor = m_pGameWorld->GetActorList()->GetActorPtr(actorHandle);


						//Store the pointer to the actor
						mVehicles[vehicleIndex].m_pVehicleActor = Actor::ActorCast< Game::Actors::C_VehicleActor> (pActor);
					}
					*/
					break;
				}
			case Game::E_ActorTypeId_SplitLine:
				{
					if(GetState() < E_RLC_STATE_CONTROLLERS_AND_DATA_INITIALISED)
						return false;

					const ::Actors::SplitLineActor *pSplitLineActor = Actor::ActorCast< ::Actors::SplitLineActor > (pActor);
					int splitNumber = pSplitLineActor->GetSplitNumber();
					bool found = false;

					//We have a bit of weirdness here...the split line is actually made of two points, logical enough I hear you think, well no.
					//Both of the points are separate actors which, even though they store a weak reference to each other, won't be created at the 
					//same instant...ie now. What we have to do is to partially create the split line, push it onto the splitLine array then when the
					//second half of the line comes through update it.

					//So...go through the split array looking for this split number
					SplitData split;
					split.Initialise();
					for(int i = 0; i < GetTrackData()->mSplitData.Size(); i++)
					{
						if(splitNumber == GetTrackData()->mSplitData[i].mSplitNumber)
						{
							//We've found our splitNumber already so we want to update this one
							GetTrackData()->mSplitData[i].mEnd = pSplitLineActor->GetPosition();
							found = true;
							break;
						}
					}
					if(found)
						break;

					//If we got here then we haven't found our split line already
					split.mSplitNumber = splitNumber;
					split.mStart = pSplitLineActor->GetPosition();
					
					GetTrackData()->mSplitData.PushBack(split);
					break;
				}
			case Game::E_ActorTypeId_PickupPosition:
				{
					RLPickupPosition newPickupPosition;
					newPickupPosition.Initialise();

					const ::Actors::PickupPositionActor *pPickupPositionActor = Actor::ActorCast< ::Actors::PickupPositionActor > (pActor);

					newPickupPosition.mId = GetTrackData()->mPickupPositions.Size();
					newPickupPosition.mActorHandle = actorHandle;

					if(pPickupPositionActor->GetCreationData()->m_pPickupType)
						newPickupPosition.meType = pPickupPositionActor->GetCreationData()->m_pPickupType->meType;
					else
						newPickupPosition.meType = E_PICKUP_TYPE_NONE;

					newPickupPosition.mWorldPosition = pPickupPositionActor->GetPosition();

					GetTrackData()->mPickupPositions.PushBack(newPickupPosition);

					//Now we've worked out what the pickup positions Id should be, tell the pickup position itself
					Game::Actors::SetPickupPositionIdMsg pMsg(newPickupPosition.mId);
					pPickupPositionActor->SendMessageToActor(actorHandle, &pMsg);



					// inform the AI about this perk.......
					AIAmax::AIPerkLinkMessage aiPerkMessage;
					aiPerkMessage.m_perk = newPickupPosition;
					C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &aiPerkMessage);

					break;
				}

			case Game::E_ActorTypeId_PickupPositionDecoy:
				{
					RLPickupPosition newPickupPosition;
					newPickupPosition.Initialise();

					const ::Actors::PickupPositionActorDecoy *pPickupPositionActorDecoy = Actor::ActorCast< ::Actors::PickupPositionActorDecoy > (pActor);

					newPickupPosition.mId = GetTrackData()->mPickupPositions.Size();
					newPickupPosition.mActorHandle = actorHandle;

					if(pPickupPositionActorDecoy->GetCreationData()->m_pPickupType)
						newPickupPosition.meType = pPickupPositionActorDecoy->GetCreationData()->m_pPickupType->meType;
					else
						newPickupPosition.meType = E_PICKUP_TYPE_NONE;

					newPickupPosition.mWorldPosition = pPickupPositionActorDecoy->GetPosition();

					GetTrackData()->mPickupPositions.PushBack(newPickupPosition);

					//Now we've worked out what the pickup positions Id should be, tell the pickup position itself
					Game::Actors::SetPickupPositionIdMsg pMsg(newPickupPosition.mId);
					pPickupPositionActorDecoy->SendMessageToActor(actorHandle, &pMsg);

					// NOTE: WE DO NOT CURRENTLY INFORM THE AI ABOUT THESE PICKUPS (SEE NORMAL PICKUP POSITION CASE)
					// If we want the AI to know about these, best inform them here :)

					break;
				}
			}
			
			return true;
		}
		
	case Game::E_VehicleMessage_Loaded:
		{
			const Game::Actors::C_VehicleLoadedMsg* pLoadedMsg = Messaging::MessageCast< Game::Actors::C_VehicleLoadedMsg >(pMessage);

			if (Game::C_Game::GetInstance()->GetLevelState().mInUI)
			{
				HandleVehicleLoadedUI(pLoadedMsg->m_LoadStatus);
				break;
			}

			//See which vehicle's been loaded 
			S8 vehicleIndex = pLoadedMsg->m_VehicleIndex;
			bool loadedStatus = pLoadedMsg->m_LoadStatus;
			if( VehicleValid(vehicleIndex)  )
			{
				if(!loadedStatus)
					LOGERROR(g_RLCoreLog, "RaceLogicCore::OnMessage() > Vehicle has failed to load");
				else
				{
					if (Game::C_Game::GetInstance()->GetLevelState().mInUI)
						SwitchUIFocusCar();
					mVehicles[vehicleIndex].mLoaded = true;
#if 0 //Spawn() vs. Create()					
					const Actor::C_Actor* pActor = m_pGameWorld->GetActorList()->GetActorPtr(mVehicles[vehicleIndex].mActorHandle);
					mVehicles[vehicleIndex].m_pVehicleActor = (Game::Actors::C_VehicleActor *)pActor;

					Game::Actors::C_VehicleActor *pActorForAI = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor>(mVehicles[vehicleIndex].m_pVehicleActor);
					CreateAIDriver(	pActorForAI,
									C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[vehicleIndex].mAICharacterName, 
									C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[vehicleIndex].mDynamicsFilename,
									C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[vehicleIndex].mAIDriver,
									C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[vehicleIndex].mAISteering);
#endif
				}
			}
			else
				LOGERROR(g_RLCoreLog, "RaceLogicCore::OnMessage() > Has an invalid vehicle been spawned?");

			return true;
			break;
		}

	case Game::E_AIMessage_LoadFailed:
	case Game::E_AIMessage_AllLoaded:
		{
			//We don't (currently) actually care what's in these messages just that the AI's finished doing what the AI does
			SetState(E_RLC_STATE_AI_RESOURCES_LOADED);
			return true;
		}
	//case Game::E_PickupMessage_PickupPickedUp:
	//	{
	//		//Get the vehicleIndex
	//		const Game::Actors::PickupPickedUpMsg* pPickupMsg = Messaging::MessageCast< Game::Actors::PickupPickedUpMsg >(pMessage);

	//		U8 vehicleIndex = pPickupMsg->mVehicleIndex;
	//		enum E_PICKUP_TYPE ePickupType = pPickupMsg->mePickupType;

	//		if(IsVehicleAIControlled(vehicleIndex))
	//			GetPickupController()->GiftPickup(vehicleIndex, ePickupType);		//Allows us to cheat and give them a chance of picking up a level 2 in one swoop
	//		else
	//			GetPickupController()->AwardPickup(vehicleIndex, ePickupType);
	//		return true;
	//	}
	
	case Game::E_GameflowMessage_QuitLevel:
		{
			GetTrackData()->mRaceFinished = true;
			GetTrackData()->mPickupsEnabled = false;
			return true;
		}
	case Game::E_GameflowMessage_QuitAndWinRace:
		{
			GetTrackData()->mRaceFinished = true;
			GetTrackData()->mPickupsEnabled = false;
			return true;
		}

	case Game::E_GameMsg_PostCreateInit:
		{
			// Check we've got some start positions loaded
			if (GetTrackData()->mStartPositions.Size() <= 0 && !C_Game::GetInstance()->GetLevelState().mInUI)
			{
				// If we've not got any start positions we're not going to be able to create any vehicles but this shouldn't really be fatal just warn the player
				LOGWARN(g_RLCoreLog, "RaceLogicCore::CheckInitialisation() > ** NO START POSITIONS ** We'll be spawning vehicles at 0,0");
			}

#ifndef CFG_GOLD
			// Used for debug functionality to move camera to start position when no vehicles are loaded
			if (mVehicles.Size() == 0)
			{
				AmaxCam::Switcher().SetType(AmaxCam::E_DebugCam_FlyCamera);

				if (GetTrackData()->mStartPositions.Size() > 0)
				{
					int index = -1;

					for (int spIndex=0; spIndex < GetTrackData()->mStartPositions.Size(); spIndex++)
					{
						// Have we found the first start pos
						if (GetTrackData()->mStartPositions[spIndex].mStartNumber == 0)
						{
							index = spIndex;
							break;
						}
					}

					// If we didn't find a start pos zero, we pick the first one in the list
					if (index == -1)
						index = 0;

					const RLStartPosition& startPos = GetTrackData()->mStartPositions[index];

					Maths::HVec3 pos = startPos.mWorldPosition + Maths::HVec3(0.f, 1.f, 0.f);

					// Try to set the camera position to the start position - failing that, set it to somewhere near the centre of the world
					Cam2::C_DebugFeed& debugFeed = Framework::C_ApplicationBase::GetInstance()->GetDebugFeed(0);

					Cam2::C_Camera tmpCamera;
					tmpCamera.SetPosition(pos);
					tmpCamera.LookAt(pos + startPos.mWorldMatrix.AZ());

					debugFeed.CopyFrom(tmpCamera);

					Framework::C_ApplicationBase::GetInstance()->SetupViews();
				}
				else
				{
					// TODO: We try and find somewhere near the centre of the world mesh and put the camera there
					//const RenderingData::Target::Scene& scene = m_pLevelResource->GetRenderLevel()->GetScene();
					//RenderingData::Point3 centre = scene.GetRootNode().GetBounds().GetCentre();
				}
			}
#endif
		}
		break;
	}
	return false;
}

} //namespace Actors

} //namespace Game

#ifdef RLC_DEBUG
#pragma optimize ("", on) //RLC_DEBUG
#endif

