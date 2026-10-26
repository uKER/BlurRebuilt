#include "StdAfx.h"
#include <GameSystems.h>

#include "AI/AI_Amax.h"
#include "AI/AI_Messages.h"
#include <racingphysics/racingphysics.h>
#include <racingphysics/Vehicles/vehicle.h>
#include <Game/Messaging/GameflowMessages.h>
#include <Game/Messaging/GameMessages.h>
#include <actor_system/ActorMessages.h>
#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Game/Messaging/VehicleActorMessages.h>
#include <AI/AI_AmaxXt.h>
#include <AI/AI_AmaxData.h>
#include <messages/EnterDynJunction.h>
#include <algorithm>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>
#include <system/memory/MemoryTracker.h>
#include <system/scheduler/jobqueue.h>
#include <SchedulerConfig.h>
#include <GameLogic\Layers\RaceBookLayer.h>
#include <GameLogic\Layers\RaceLogicLayer.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game/GameActors/Components/GameComponents.h>  
//#include <TrackLogic/ctrackLogic.h>
#include <system/serialise/TextSerialise.h>
#include <GameApp.h>
#include <game/GameWorld/GameWorld.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <perks/PerkLogic/cPerkLogic.h>
#include <Perks/PerkSystem/cPerkSystem.h>
#include <Perks/PerkAndSkillUsage/cPerkSkillUsage.h>
#include <GameLogic/ActorStudioActors/CutSceneMessages.h>
#include <GameLogic/Structures/MpResults.h>
#include <ui/Game/Gadgets/ButtonMessages.h>
#include <Mode/Destruction/cModeDestruction.h>
#include <Mode/FanDemand/cModeFanDemand.h>

#include <Perks/PerkNitro/cPerkNitro.h>
#include <Perks/PerkBarge/cPerkBarge.h>
#include <Perks/PerkShock/cPerkShock.h>
#include <Perks/PerkShunt/cPerkShunt.h>
#include <Perks/PerkBolt/cPerkBolt.h>
#include <Perks/PerkShield/cPerkShield.h>
#include <Perks/PerkMine/cPerkMine.h>
#include <Perks/PerkRepair/cPerkRepair.h>
#include <GameBrain/cGameBrain.h>

#include <GameLogic\FriendsChallenges\FriendChallenges.h>

#include "Game/Game.h"

#if !defined(CFG_GOLD) || defined(CFG_HACKED_MASTER)
#include <System/Misc/Random.h>
#endif

// NOTE : In order to make the AI run on the SPU`s and ease of use, this file is un-optimised as it references game side objects

static Debug::Logger &g_LoggerLoading = Debug::GetLogger("amax.loading");
static Debug::Logger &g_LoggerAI = Debug::GetLogger("amax.AI");
// 
Debug::Bool g_DebugDrawAILogic("AmaxAI.DebugDraw.TrackData.Logic",false);
Debug::Bool g_DebugDrawAILogicStart("AmaxAI.DebugDraw.TrackData.LogicStart",false);
Debug::Bool g_DebugDrawAILogicFinish("AmaxAI.DebugDraw.TrackData.LogicFinish",false);
Debug::Bool g_DebugDrawAILogicInter1("AmaxAI.DebugDraw.TrackData.LogicInter1",false);
Debug::Bool g_DebugDrawAILogicInter2("AmaxAI.DebugDraw.TrackData.LogicInter2",false);


Debug::Bool g_debugAIDebugDrawDepthEnable("AmaxAI.DebugDraw.DepthEnable", false);

Debug::Bool g_DebugDrawAIRacing("AmaxAI.DebugDraw.TrackData.Racing",false);
Debug::Bool g_DebugDrawAIJunctions("AmaxAI.DebugDraw.TrackData.Junctions",false);

Debug::Bool g_DebugDrawAIRacingRacing("AmaxAI.DebugDraw.TrackData.RacingSpline",false);
Debug::Bool g_DebugDrawAIRacingWidth("AmaxAI.DebugDraw.TrackData.RacingWidth",false);
Debug::Bool g_DebugDrawAIExtremeWidth("AmaxAI.DebugDraw.TrackData.ExtremeWidth",false);
Debug::Bool g_DebugDrawAISegment("AmaxAI.DebugDraw.TrackData.Segment",false);

Debug::Bool g_DebugDrawAISplineFollower("AmaxAI.DebugDraw.TrackData.SplineFollower",false);


Debug::Bool g_DebugDrawAIVolume("AmaxAI.DebugDraw.TrackData.Volume",false);

Debug::Float g_AiFrictionHack("AmaxAI.DebugCar.Friction", 1.05f,/*1.85f,*/ Debug::Range(1.0f, 10.0f), Debug::Step(0.01f));
Debug::Float g_AiTorqueHack("AmaxAI.DebugCar.Torque", 0.1f, Debug::Range(1.0f, 50.0f), Debug::Step(0.5f));
Debug::Float g_AiCorneringBoost("AmaxAI.DebugCar.CorneringBoost", 2.25f, Debug::Range(0.0f, 5.0f), Debug::Step(0.01f));
Debug::Float g_CarAirForceHack("AmaxAI.DebugCar.AirForce", 1000.0f, Debug::Range(0.0f, 2000.0f), Debug::Step(0.5f));
Debug::Float g_CarTorqueDampHack("AmaxAI.DebugCar.TorqueDamp", 5.0f, Debug::Range(-2000.0f, 2000.0f), Debug::Step(1.0f));
Debug::Bool d_DebugDisableAIThrottle("AmaxAI.DebugCar.DisableThrottle", false, Debug::Persist());

Debug::Bool g_DebugPlayerUsesSomeSteeringForces("AmaxAI.DebugCar.Player.PlayerUsesSomeSteeringForces", false);
Debug::Float g_DebugPlayerAiTorqueHack("AmaxAI.DebugCar.Player.Torque", 0.01f, Debug::Range(0.0f, 10.0f), Debug::Step(0.5f), Debug::Persist());
Debug::Float g_DebugPlayerAiCorneringBoost("AmaxAI.DebugCar.Player.CorneringBoost", 1.8f, Debug::Range(0.0f, 5.0f), Debug::Step(0.01f), Debug::Persist());
Debug::Float g_DebugPlayerAIBrakeValue("AmaxAI.DebugCar.Player.BrakeBoost", 2.0f, Debug::Range(0.0f, 5.0f), Debug::Step(0.01f), Debug::Persist());
Debug::Float g_DebugPlayerAIBrakeSpeedPercentage("AmaxAI.DebugCar.Player.BrakeTrackSpeedPercentage", 1.0f, Debug::Range(0.1f, 1.0f), Debug::Step(0.01f), Debug::Persist());
Debug::Float g_DebugPlayerAIBrakeSpeedPercentageEasy("AmaxAI.DebugCar.Player.BrakeTrackSpeedPercentageEasy", 0.95f, Debug::Range(0.1f, 1.0f), Debug::Step(0.01f), Debug::Persist());
Debug::Float g_DebugPlayerAIBrakeSpeedPercentageMedium("AmaxAI.DebugCar.Player.BrakeTrackSpeedPercentageMedium", 1.0f, Debug::Range(0.1f, 1.0f), Debug::Step(0.01f), Debug::Persist());
Debug::Float g_DebugPlayerAIBrakeSpeedPercentageHard("AmaxAI.DebugCar.Player.BrakeTrackSpeedPercentageHard", 1.0f, Debug::Range(0.1f, 1.0f), Debug::Step(0.01f), Debug::Persist());
Debug::Bool g_DebugPlayerUsesSomeAIBrake("AmaxAI.DebugCar.Player.PlayerUsesSomeAIBrake", false);

Debug::Bool g_DebugDontUsePickups("AmaxAI.DontUsePickups", false);
Debug::Bool g_DebugManuallySetLaps("AmaxAI.Laps.ManuallySetLapNo", false);
Debug::Int g_DebugManualNoLaps("AmaxAI.DebugCar.Laps.Lap", 1);

#if !defined(CFG_GOLD) || defined(CFG_HACKED_MASTER)
extern Debug::Bool g_AIVehiclesBehaveCrazy;
#endif

#ifdef _MSC_VER
#ifdef CFG_ALLOW_OPTIMIZE_OFF
#ifdef DEBUG_AI
#pragma optimize("", off)
#pragma message("AI_Amax_cpp is enabled")
#endif
#endif
#endif



//#define _RELOAD_SORCERY_DATA

#define _USE_STEERING_ASSIST_CORNERING_BOOST 


namespace AIAmax
{
	struct TaskData
	{
		AIAmax::AmaxAIR *pAI;

		int				m_AIIndex;				
	};




}





namespace AIAmax
{
	const float AmaxAIR::MAX_TRACK_SPEED = 1.f;
	const float AmaxAIR::MIN_TRACK_SPEED = 0.6f;

	using namespace airacingtech;
	using namespace fuzzy_lib_one;
	using namespace Maths;
	using namespace Game;

	AmaxAIR AmaxAIR::m_Instance;
	


	AmaxAIR::AmaxAIR() : m_Group(Scheduler::JQ_GROUP_NONE), m_DataInitilaised(false), m_SystemsInitialised(false), m_enableAI(false), m_safeToSimulate(false), m_RollingStartSpeedEnabled_HACK(false), m_TimeStep(0.0f), m_pIModeDestruction(0), m_pIModeFanDemand(0)
	{
			
	}

	void AmaxAIR::ShutDown()
	{
		if (m_pObjectManager)
		{
			m_pObjectManager->DeInit();
		}

		m_DataInitilaised = false;
		m_SystemsInitialised = false;
		m_pObjectManager = 0;
		m_IDSystemAI = -1;
		m_pISystemAI = 0;
		m_enableAI = true;

		m_pIGameServerTime = 0;
		m_pITrackDataSystem = 0;
		m_pIPerkSystem = 0;
		m_pIModeDestruction = 0;
		m_pIModeFanDemand = 0;


		for(int i=0; i<m_cars.Size(); i++)
		{
			AIAmaxVehicleData *pData = m_cars[i];
			assert(pData);
			delete pData;
		}

		m_cars.Clear();
		m_safeToSimulate = false;
		m_handBrakeOn = true;
		m_startRaceDisablePickupsBeingAdded = false;
		
		Scheduler::DestroyGroup(m_Group);
	}

	void AmaxAIR::Initialise(Messaging::C_MessageSubscriberSystem *pMessageSubscriberSystem)
	{
		m_Group = Scheduler::CreateGroup();
		m_AIPolicy = C_SchedulerConfig::GetInstance()->GetAIPolicy();

		// init the global object and component manager
		m_pObjectManager = &GlobalsComponent::GetObjectManager();
		m_pObjectManager->Init();

		// Register All components needed....
		RegisterComponents();

		InitialiseSystems();

		SetSubscriberSystem( pMessageSubscriberSystem );

		AddSubscriber( Game::E_AIMessage_LoadAITrackResources );
		AddSubscriber( Game::E_AIMessage_LoadAIDriverResources );
		//AddSubscriber( Game::E_AIMessage_Simulate );
		AddSubscriber( Game::E_AIMessage_StartSimulate );
		AddSubscriber( Game::E_AIMessage_StopSimulate );
		AddSubscriber( Game::E_AIMessage_Start );
		AddSubscriber( Game::E_AIMessage_Stop );
		AddSubscriber( Game::E_AIMessage_StartSetSpeed );
		AddSubscriber( Game::E_AIMessage_EndSetSpeed );
		AddSubscriber( Game::E_GameflowMessage_RLCResourcesLoaded );
		AddSubscriber(Game::E_GameflowMessage_RaceLoaded);
		AddSubscriber(Game::E_GameflowMessage_RaceStarted);
		AddSubscriber(Game::E_GameflowMessage_IntroStarted);
		AddSubscriber(Game::E_GameflowMessage_SorceryReloaded);
		AddSubscriber(Game::E_GameflowMessage_StreamerStarted);

		AddSubscriber(Game::E_AIMessage_SetRoute);
		AddSubscriber(ActorMsg::E_DynamicObjMsg_SetAIRoute);
		AddSubscriber(ActorMsg::E_CutSceneMsg_EnableRaceAI);
		AddSubscriber(ActorMsg::E_CutSceneMsg_DisableRaceAI);
		AddSubscriber(ActorMsg::E_CutSceneMsg_SetVehiclesToRaceSpeed);		// TODO - MESSAGE TO START VEHICLES AT RACE SPEED

		AddSubscriber(Game::E_AIMessage_InitialiseIntro); // set the game mode to be the intro

		AddSubscriber(Game::E_AIMessage_CreateIntroSpline); // create an intro spline........

		// intialise the ai on the intro spline

		AddSubscriber(Game::E_GameflowMessage_IntroStarted); // start the intro
		
		// send a speed message to the AI.....involves the vehicle actor handle, spline name, speed
		AddSubscriber(Game::E_AIMessage_SetIntroOutroSpeed);

		AddSubscriber(Game::E_AIMessage_SetIntroOutroSpline);
		AddSubscriber(Game::E_AIMessage_DisableAI);
		AddSubscriber(Game::E_AIMessage_DriveTowardsPoint);

		AddSubscriber(Game::E_GameLogicMessage_Impact);
		
		
		// create a perk.....
		AddSubscriber(Game::E_AIMessage_CreatePerkLink);


		AddSubscriber(Actor::E_ActorMsg_Killed);


		DebugTest();
	}

	void AmaxAIR::DebugTest()
	{
		float pos = airacingtech::DistanceInfrontBehindF(0.9f, 0.1f);	

	}
	
	void AmaxAIR::RegisterComponents()
	{
		// self explanitory.....
		CSystemAI::RegisterComponentType();
		CDriverAI::RegisterComponentType();
		CVehicleState::RegisterComponentType();
		CControlState::RegisterComponentType();
		CPIDControllerAI::RegisterComponentType();	
		CAccelBrakeController::RegisterComponentType();
		CData::RegisterComponentType();
		CParamsAI::RegisterComponentType();	
		CRaceDirector::RegisterComponentType();
		CTrackDataAI::RegisterComponentType();
		CSplineTracker::RegisterComponentType();
		CSplineFollower::RegisterComponentType();
		CSpeedTracker::RegisterComponentType();
		CAvoidance::RegisterComponentType();
		CAvoidanceBasic::RegisterComponentType();
		CVehicleLogic::RegisterComponentType();
		CGameServerTime::RegisterComponentType();
		CDebugAI::RegisterComponentType();
		CRecoveryAI::RegisterComponentType();
		CRubberbandingAI::RegisterComponentType();
		CIntroOutroSplineAI::RegisterComponentType();
		CSplineAI::RegisterComponentType();
		CPerkLogicAI::RegisterComponentType();
		CPerkSystem::RegisterComponentType();
		CPerkSkillUsage::RegisterComponentType();
		CModeDestruction::RegisterComponentType();
		CModeFanDemand::RegisterComponentType();
		CGameBrain::RegisterComponentType();
		CPerkNitro::RegisterComponentType();
		CPerkBolt::RegisterComponentType();
		CPerkShock::RegisterComponentType();
		CPerkShunt::RegisterComponentType();
		CPerkShield::RegisterComponentType();
		CPerkMine::RegisterComponentType();
		CPerkBarge::RegisterComponentType();
		CPerkRepair::RegisterComponentType();
		
	}

	void AmaxAIR::InitialiseSystems()
	{
		InitAISystem();
		InitTrackDataSystem();
		InitServerTime();
		InitRaceDirector();
		InitPerkSystem();


		
	}

	void AmaxAIR::InitAISystem()
	{
		CParameterNode params;
		params.SetData(0);
		params.SetName(CHash("SystemAI"));
		params.SetObjectName(CHash("AmaxSystemAI")); // actual object name
		params.SetType(CHash("OBJECT"));

		m_IDSystemAI = m_pObjectManager->CreateObject(&params);
		m_SystemsInitialised = m_pObjectManager->ObjectExists(m_IDSystemAI);
		m_pISystemAI = static_cast<ISystemAI *>(m_pObjectManager->QueryInterface(m_IDSystemAI, IID_SYSTEM_AI));
		
		// set the group.....
		if (m_pISystemAI)
		{
			m_pISystemAI->SetMultiThreadingGroup(m_Group);
			m_pISystemAI->SetMultiThreadingPolicy(m_AIPolicy);
		}
	}

	void AmaxAIR::InitTrackDataSystem()
	{
		CParameterNode params;
		
		// create the trackdata object.............
		params.SetData(0);
		params.SetName(CHash("TrackData"));
		params.SetObjectName(CHash("TrackDataSystem")); // actual object name
		params.SetType(CHash("OBJECT"));

		if (m_SystemsInitialised)
		{
			CMessage msg;
			msg.mType = MT_INIT_OBJECT;
			msg.mpData = &params;
			m_pObjectManager->PostAMessage(m_IDSystemAI, msg);
		}

		CObjectId trackdataSystem("TrackDataSystem");
		m_pITrackDataSystem = static_cast<ITrackDataAI *>(GlobalsComponent::GetObjectManager().QueryInterface(trackdataSystem, IID_TRACK_DATA_AI));

		
	}


	void AmaxAIR::InitServerTime()
	{
		// AI`s own server time.....
		CParameterNode params;

		params.SetData(0);
		params.SetName(CHash("GameServerTime"));
		params.SetObjectName(CHash("GameServerTimeSystem")); // actual object name
		params.SetType(CHash("OBJECT"));

		if (m_SystemsInitialised)
		{
			CMessage msg;
			msg.mType = MT_INIT_OBJECT;
			msg.mpData = &params;
			m_pObjectManager->PostAMessage(m_IDSystemAI, msg);
		}

		CObjectId gameservertimeID("GameServerTimeSystem");
		m_pIGameServerTime = static_cast<IGameServerTime *>(GlobalsComponent::GetObjectManager().QueryInterface(gameservertimeID, IID_GAMESERVER_TIME));

	}

	void AmaxAIR::InitRaceDirector()
	{
		CParameterNode params;
		
		// race director
		params.SetData(0);
		params.SetName(CHash("RaceDirector"));
		params.SetObjectName(CHash("RaceDirectorSystem")); // actual object name
		params.SetType(CHash("OBJECT"));

		if (m_SystemsInitialised)
		{
			CMessage msg;
			msg.mType = MT_INIT_OBJECT;
			msg.mpData = &params;
			m_pObjectManager->PostAMessage(m_IDSystemAI, msg);
		}
	}

	void AmaxAIR::InitPerkSystem()
	{
		CParameterNode params;

		// perksystem
		params.SetData(0);
		params.SetName(CHash("PerkSystemAI"));
		params.SetObjectName(CHash("PerkSystemAISystem")); // actual object name
		params.SetType(CHash("OBJECT"));

		if (m_SystemsInitialised)
		{
			CMessage msg;
			msg.mType = MT_INIT_OBJECT;
			msg.mpData = &params;
			m_pObjectManager->PostAMessage(m_IDSystemAI, msg);
		}

		CObjectId perkSystemID("PerkSystemAISystem");
		m_pIPerkSystem = static_cast<IPerkSystem *>(GlobalsComponent::GetObjectManager().QueryInterface(perkSystemID, IID_PERK_SYSTEM));

	}


	void AmaxAIR::InitDestructionSystem()
	{
		// ModeDestruction

		CParameterNode params;

		Game::E_RaceMode rm;
		if(Game::C_FriendChallenges::Instance().IsAttemptingFriendDemand())
			rm=E_RaceMode_FriendDemand_Destruction;
		else
			rm=E_RaceMode_Destruction;

		const AmaxRaceModeDestruction* destructionSetupData = ((Game::C_RaceModeDestruction*)gRaceModeStore(rm))->GetSetupData();
		params.SetData((void*)destructionSetupData); // Pass the destruction data which contains the wave information
		params.SetName(CHash("ModeDestructionSystemAI"));
		params.SetObjectName(CHash("ModeDestructionSystemAISystem")); // actual object name
		params.SetType(CHash("OBJECT"));

		if (m_SystemsInitialised)
		{
			CMessage msg;
			msg.mType = MT_INIT_OBJECT;
			msg.mpData = &params;
			m_pObjectManager->PostAMessage(m_IDSystemAI, msg);
		}

		CObjectId modeDestructionSystemID("ModeDestructionSystemAISystem");
		m_pIModeDestruction = static_cast<IModeDestruction *>(GlobalsComponent::GetObjectManager().QueryInterface(modeDestructionSystemID, IID_MODE_DESTRUCTION));

	}

	void AmaxAIR::InitFanDemandSystem()
	{
		CParameterNode params;

		// ModeFanDemand
		params.SetData(0);
		params.SetName(CHash("ModeFanDemandSystemAI"));
		params.SetObjectName(CHash("ModeFanDemandSystemAISystem")); // actual object name
		params.SetType(CHash("OBJECT"));

		if (m_SystemsInitialised)
		{
			CMessage msg;
			msg.mType = MT_INIT_OBJECT;
			msg.mpData = &params;
			m_pObjectManager->PostAMessage(m_IDSystemAI, msg);
		}

		CObjectId modeFanDemandSystemID("ModeFanDemandSystemAISystem");
		m_pIModeFanDemand = static_cast<IModeFanDemand *>(GlobalsComponent::GetObjectManager().QueryInterface(modeFanDemandSystemID, IID_MODE_FANDEMAND));

	}



#if 0
	bool AmaxAIR::LoadAIEventData(const char *fileName)
	{
		MEMORY_TRACKER_PUSH("ai");
		bool status = false;

		if ( IsSystemsInitialised() )
		{
			// we now need to load in the event data from a .csv file.....
			bool loadeventData = LoadEventData(fileName);
		}
		MEMORY_TRACKER_POP("ai");
		return status;
	}
#else
		
	bool AmaxAIR::LoadAIEventData(const char *fileName)
	{
		MEMORY_TRACKER_PUSH("ai");
		bool status = false;

		
		if ( IsSystemsInitialised() )
		{
			// we now need to load in the event data from a .csv file.....
			bool loadeventData = LoadEventData(fileName);
		}
		


		// lets check the state here
		Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

		//lets check the level state
		if (levelState.mRaceMode == E_RaceMode_Destruction || levelState.mRaceMode == E_RaceMode_FriendDemand_Destruction)
		{
			InitDestructionSystem();
		}
		else if (levelState.mRaceMode == E_RaceMode_FanDemand || levelState.mRaceMode == E_RaceMode_FriendDemand_FanDemand)
		{
			InitFanDemandSystem();
		}



		MEMORY_TRACKER_POP("ai");
		return status;
	}
		

#endif

	bool AmaxAIR::LoadAICityData(const char *fileName)
	{
		MEMORY_TRACKER_PUSH("ai");
		bool status = false;

		if ( IsSystemsInitialised() )
		{
			bool loadAITrackDataNew = LoadAITrackData(fileName);

			if (loadAITrackDataNew == true)
			{
				m_DataInitilaised = true;
				m_SystemsInitialised = true;
				status = true;
			}
			else
			{
				m_DataInitilaised = false;
				// [Blur2PC] levels without AI data (the Showroom) used to clear m_SystemsInitialised too,
				// which made every later LoadAICityData a no-op: races had no racing line, no splits and idle AI.
				//m_SystemsInitialised = false;
			}	
		}
		MEMORY_TRACKER_POP("ai");
		return status;
	}


	bool AmaxAIR::MessageHandler(const Messaging::C_MessageBase* pMessage)
	{
		// Handle messages from the game side.....
		bool retValue = false;

		switch( pMessage->Type() )
		{
		
			case Game::E_AIMessage_LoadAITrackResources:
				{
					const AILoadTrackDataMsg* pMsg = Messaging::MessageCast<AILoadTrackDataMsg>(pMessage);
					LoadAICityData(pMsg->m_filename);
					retValue = true;
				}
				break;


			case Game::E_AIMessage_LoadAIDriverResources:
				{
					const AILoadDriveraMsg* pMsg = Messaging::MessageCast<AILoadDriveraMsg>(pMessage);
					LoadAIDriver(pMsg);
					retValue = true;
				}
				break;

			case Game::E_GameflowMessage_RLCResourcesLoaded:
				{
					// due to how the game loads stuff externally, lets now create add this data
					//LoadAIGameplayBalancing();

					// at this point we have valid resources, ie vehicles and AI, so it is safe todo a post initialise
					PostInitialise();	
					retValue = true;
				}
				break;

			case Game::E_GameflowMessage_StreamerStarted:
				{
					InitRace();
					retValue = true;
				}

			//case Game::E_AIMessage_Simulate:
			//	{		
			//		Update();		
			//		retValue = true;
			//	}
			//	break;


			case Game::E_AIMessage_InitialiseIntro:
				{
					// sets the AI to be in intro mode...
					CMessage msg;
					msg.mType = MT_SET_GAMESTATE_INTRO;
					msg.mpData = 0;
					m_pObjectManager->BroadcastMessage(msg);
				}
				break;

			case Game::E_AIMessage_CreateIntroSpline:
				{
					// create a spline object........
					const AICreateSplineMsg* pMsg = Messaging::MessageCast<AICreateSplineMsg>(pMessage);
					std::string splineName(pMsg->m_SplineName.Chars(), pMsg->m_SplineName.Length());
					// convert to lowercase....
					transform(splineName.begin (), splineName.end (), splineName.begin (), (int(*)(int)) tolower);

					// Note : Cleanup the wasted memory...but it is temporary at the moment.
					SplineInitialisationParams splineParams;
					splineParams.m_controlPoints = pMsg->m_samplePoints;
					splineParams.m_resample = pMsg->m_ReSample;
					splineParams.m_sampleSpeed = pMsg->m_sampleSpeeds;

					CParameterNode params;
					params.SetData(&splineParams);
					params.SetName(CHash("SplineAI"));
					params.SetObjectName(CHash(splineName.c_str())); // actual object name
					params.SetType(CHash("OBJECT"));

					if (m_SystemsInitialised)
					{
						CMessage msg;
						msg.mType = MT_INIT_OBJECT;
						msg.mpData = &params;
						m_pObjectManager->PostAMessage(m_IDSystemAI, msg);
					}
				}
				break;

			case Game::E_GameflowMessage_IntroStarted:
				{
#if 0
					// current hack...runs the game still not the intros....
					CMessage msg;
					msg.mType = MT_START_AI_RACE;
					msg.mpData = 0;
					m_pObjectManager->BroadcastMessage(msg);
#else

					CMessage msg;
					msg.mType = MT_START_INTRO;
					msg.mpData = 0;
					m_pObjectManager->BroadcastMessage(msg);
#endif
				}
				break;


			case ActorMsg::E_CutSceneMsg_EnableRaceAI:
				{
					CMessage msg;
					msg.mType = MT_SET_GAMESTATE_GAME;
					msg.mpData = 0;
					int stop = 0;
					m_pObjectManager->BroadcastMessage(msg);

					msg.mType = MT_START_AI_RACE;
					msg.mpData = 0;
					m_pObjectManager->BroadcastMessage(msg);
					
					m_RollingStartSpeedEnabled_HACK = true;
					m_handBrakeOn = false;
					m_startRaceDisablePickupsBeingAdded = true;


					// as we now have no intro anymore sort of a hacked rolling start
					msg.mType = MT_START_AI_ROLLINGSTART_START;
					msg.mpData = 0;
					m_pObjectManager->BroadcastMessage(msg);

					

				}
				break;
			case Game::E_GameflowMessage_RaceStarted:
				{
					// sets the ai to be in game mode....
					CMessage msg;
					msg.mType = MT_SET_GAMESTATE_GAME;
					msg.mpData = 0;
					m_pObjectManager->BroadcastMessage(msg);
			
					msg.mType = MT_START_AI_RACE;
					msg.mpData = 0;
					m_pObjectManager->BroadcastMessage(msg);

					msg.mType = MT_SET_GAME_MODE;
					msg.mpData = 0;
					m_pObjectManager->BroadcastMessage(msg);


					m_RollingStartSpeedEnabled_HACK = false;
					m_handBrakeOn = false;
					m_startRaceDisablePickupsBeingAdded = true;

					msg.mType = MT_START_AI_ROLLINGSTART_END;
					msg.mpData = 0;
					m_pObjectManager->BroadcastMessage(msg);
				}
				break;


			case ActorMsg::E_CutSceneMsg_DisableRaceAI:
				{

					CMessage msg;
					msg.mType = MT_SET_GAMESTATE_OUTRO;
					msg.mpData = 0;
					m_pObjectManager->BroadcastMessage(msg);

					msg.mType = MT_START_OUTRO;
					msg.mpData = 0;
					m_pObjectManager->BroadcastMessage(msg);
					
				}
				break;

			
			
			case Game::E_GameflowMessage_SorceryReloaded:
				{
					ReLoadSorceryAIData();
				}
				break;

			case Game::E_AIMessage_SetRoute:
			case ActorMsg::E_DynamicObjMsg_SetAIRoute:
				{
					const ActorMsg::C_SetAIRouteMsg* pBadMsg = Messaging::MessageCast<ActorMsg::C_SetAIRouteMsg>(pMessage);
					std::string routeName(pBadMsg->m_RouteName.Chars(), pBadMsg->m_RouteName.Length());
					
					// convert to lowercase....
					transform(routeName.begin (), routeName.end (), routeName.begin (), (int(*)(int)) tolower);
					SetRouteMSG aiMsg(pBadMsg->m_Active, CObjectId(routeName.c_str()));

					CMessage msg;
					msg.mType = MT_SET_ROUTE_ACTIVE_INACTIVE;
					msg.mpData = &aiMsg;
					m_pObjectManager->BroadcastMessage(msg);

				}
				break;


			case Game::E_AIMessage_SetIntroOutroSpeed:
				{
					const AISetIntroOutroSpeed* pMsg = Messaging::MessageCast<AISetIntroOutroSpeed>(pMessage);	
					if (pMsg && pMsg->m_pVehicleActor)
					{
						int index = FindIndexFromActor(pMsg->m_pVehicleActor);

						if (index != -1)
						{
							IData *pIData = m_pISystemAI->GetData(index);
							if (pIData)
							{
								IntroOutroSetSpeedParams params;
								params.m_speed = pMsg->m_speed;

								CMessage msg;
								msg.mType = MT_SET_INTRO_OUTRO_SPEED;
								msg.mpData = &params;
								m_pObjectManager->PostAMessage(pIData->GetObjectId(), msg);
							}
						}
					}
				}
				break;

			case Game::E_AIMessage_SetIntroOutroSpline:
				{
					const AISetIntroOutroSpline* pMsg = Messaging::MessageCast<AISetIntroOutroSpline>(pMessage);
					std::string splineName(pMsg->m_SplineName.Chars(), pMsg->m_SplineName.Length());
					// convert to lowercase....
					transform(splineName.begin (), splineName.end (), splineName.begin (), (int(*)(int)) tolower);

					CObjectId splineID(splineName.c_str());

					if (pMsg && pMsg->m_pVehicleActor)
					{
						// find our vehicle actor....
						int index = FindIndexFromActor(pMsg->m_pVehicleActor);

						if (index != -1)
						{
							IData *pIData = m_pISystemAI->GetData(index);
							if (pIData)
							{
								IntroOutroSetSplineParams params;
								params.m_splineName = splineID;

								CMessage msg;
								msg.mType = MT_SET_INTRO_OUTRO_SPLINE;
								msg.mpData = &params;
								m_pObjectManager->PostAMessage(pIData->GetObjectId(), msg);

								
							}
						}
					}
				}
				break;

			case Game::E_AIMessage_DisableAI:
				{
					const AIDisableAI* pMsg = Messaging::MessageCast<AIDisableAI>(pMessage);	
					if (pMsg && pMsg->m_pVehicleActor)
					{
						int index = FindIndexFromActor(pMsg->m_pVehicleActor);

						if (index != -1)
						{
							IData *pIData = m_pISystemAI->GetData(index);
							if (pIData)
							{
								
								CMessage msg;
								msg.mType = MT_SET_INTRO_OUTRO_DISABLE_AI;
								msg.mpData = 0;
								m_pObjectManager->PostAMessage(pIData->GetObjectId(), msg);
							}
						}
					}

				}
				break;

			case Game::E_AIMessage_Stop:
				{
					const AIStopMsg* pMsg = Messaging::MessageCast<AIStopMsg>(pMessage);
					if (pMsg && pMsg->m_pVehicleActor)
					{
						int index = FindIndexFromActor(pMsg->m_pVehicleActor);

						if (index != -1)
						{
							IData *pIData = m_pISystemAI->GetData(index);
							if (pIData)
							{

								CMessage msg;
								msg.mType = MT_STOP_AI;
								msg.mpData = 0;
								m_pObjectManager->PostAMessage(pIData->GetObjectId(), msg);
							}
						}
					}
				}
				break;

			case Game::E_AIMessage_DriveTowardsPoint:
				{
					const AIDriveTowardsPoint* pMsg = Messaging::MessageCast<AIDriveTowardsPoint>(pMessage);	
					if (pMsg && pMsg->m_pVehicleActor)
					{
						int index = FindIndexFromActor(pMsg->m_pVehicleActor);

						if (index != -1)
						{
							IData *pIData = m_pISystemAI->GetData(index);
							if (pIData)
							{
								IntroOutroSetTargetPointParams params;
								params.m_targetPoint = pMsg->m_TargetPoint;

								CMessage msg;
								msg.mType = MT_SET_INTRO_OUTRO_TARGET_POINT;
								msg.mpData = &params;
								m_pObjectManager->PostAMessage(pIData->GetObjectId(), msg);
							}
						}
					}
				}
				break;

			case Game::E_GameLogicMessage_Impact:
				{
					Game::C_ImpactMsg* pMsg = const_cast<Game::C_ImpactMsg*>(Messaging::MessageCast<Game::C_ImpactMsg>(pMessage));
					HandleCollision(pMsg);
				}
				break;

			case Game::E_AIMessage_CreatePerkLink:
				{
					if (!m_startRaceDisablePickupsBeingAdded)
					{
						AIPerkLinkMessage* pMsg = const_cast<AIPerkLinkMessage*>(Messaging::MessageCast<AIPerkLinkMessage>(pMessage));
						HandleCreatePerk(pMsg);
					}
					
				}
				break;


			case ActorMsg::E_CutSceneMsg_SetVehiclesToRaceSpeed:
				{
					ActorMsg::C_CutScene_SetVehiclesToRaceSpeedMsg* pMsg = const_cast<ActorMsg::C_CutScene_SetVehiclesToRaceSpeedMsg*>(Messaging::MessageCast<ActorMsg::C_CutScene_SetVehiclesToRaceSpeedMsg>(pMessage));
					HandleStartVehiclesAtSpeed(pMsg->mPercentageRaceSpeed);		

				}
				break;

			case Actor::E_ActorMsg_Killed:
				{	
					Actor::C_ActorKilledMsg *pMsg =  const_cast<Actor::C_ActorKilledMsg*>(Messaging::MessageCast<Actor::C_ActorKilledMsg>(pMessage));
					{
						if (pMsg)
						{
							Actor::C_ActorHandle handle = pMsg->GetActorHandle();
							if(C_GameApp::GetInstance()->GetRaceBookLayer() && C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer() && C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld())
							{
								const Actor::C_Actor *pActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetActorPtr(handle);
								if (pActor)
								{
									if (pActor->GetActorTypeId() == E_ActorTypeId_PickupPosition)
									{
										::Actors::PickupPositionActor *pPerkActor = (::Actors::PickupPositionActor*)Actor::ActorCast< ::Actors::PickupPositionActor>(pActor);
										if (pPerkActor)	
										{
											// we now need to delete the 
											if (m_pIPerkSystem)
											{
												m_pIPerkSystem->DeletePerkObject(handle);
											}
											
										}
									}
								}
							}
						}
					}
				}
				break;

			default:
				break;
		}

		return retValue;
	}

	void AmaxAIR::AITakeControl(int p_Index)
	{
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		if (!pLayer || p_Index >= m_cars.Size())
			return;

		IData *pIData = m_pISystemAI->GetData(p_Index);
		if (pIData)
		{
			CMessage msg;
			msg.mType = MT_AI_TAKE_CONTROL;
			msg.mpData = NULL;
			m_pObjectManager->PostAMessage(pIData->GetObjectId(), msg);							
		}
	}
	
	void AmaxAIR::AIRelinquishControl(int p_Index)
	{
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		if (!pLayer || p_Index >= m_cars.Size())
			return;

		IData *pIData = m_pISystemAI->GetData(p_Index);
		if (pIData)
		{
			CMessage msg;
			msg.mType = MT_AI_RELINQUISH_CONTROL;
			msg.mpData = NULL;
			m_pObjectManager->PostAMessage(pIData->GetObjectId(), msg);							
		}
	}

	void AmaxAIR::SetIsLocalPlayer(int p_Index, bool p_IsLocalPlayer)
	{
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		if (!pLayer || p_Index >= m_cars.Size())
			return;

		IData *pIData = m_pISystemAI->GetData(p_Index);
		if (pIData)
		{
			CMessage msg;
			msg.mType = MT_SET_LOCAL_PLAYER;
			msg.mpData = &p_IsLocalPlayer;
			m_pObjectManager->PostAMessage(pIData->GetObjectId(), msg);							
		}
	}	

	void AmaxAIR::HandleStartVehiclesAtSpeed(float percentage)
	{
		if (IsDataInitialised() &&  IsSystemsInitialised() && m_safeToSimulate)
		{
			// setting the vehicles at speed.....
			int noVehicles =  m_cars.Size();
			for(int i=0; i<noVehicles; i++)
			{
				Game::Actors::C_VehicleActor *pActor = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor>(m_cars[i]->m_pVehicleActor);
				IData *pIData = m_pISystemAI->GetData(i);	
				float trackspeed = pIData->GetState().m_trackSpeedPreprocessed * (percentage / 100.0f)	;
				pActor->SetLinearVelocityFromSpeed(trackspeed);
			}	
		}	
	}

	void AmaxAIR::HandleCreatePerk(AIPerkLinkMessage *pMsg)
	{
		if (pMsg)
		{
			RLPickupPosition tempPerk = pMsg->m_perk;
			
			AIAmax::AIPerk aiPerk;
			aiPerk.m_actorHandle = tempPerk.mActorHandle;
		
			// need to get the position and work out what routes we are in.......
			CObjectId trackdataSystem("TrackDataSystem");
			ITrackDataAI *pITrackData = static_cast<ITrackDataAI *>(GlobalsComponent::GetObjectManager().QueryInterface(trackdataSystem, IID_TRACK_DATA_AI));
						
			CObjectId perkSystemID("PerkSystemAISystem");
			IPerkSystem *pIPerkSystem = static_cast<IPerkSystem *>(GlobalsComponent::GetObjectManager().QueryInterface(perkSystemID, IID_PERK_SYSTEM));

			if (pITrackData && pIPerkSystem)	
			{
				AITrackData *pAIData = pITrackData->GetTrackData();
				if (pAIData)
				{
					for(int i=0; i<pAIData->GetNumRoutes(); i++)
					{
						AITrackDataRoutes *pRoute = pAIData->GetRoute(i);
						if (pRoute && (pRoute->IsChangeableRoute() || pRoute->IsLogic()))
						{
							float paramDist;
							Maths::HVec3 testPoint(tempPerk.mWorldPosition);
							int index = pRoute->FindNearest(1, pRoute->GetNumSegments()-1, testPoint, paramDist, pRoute->GetStepSize());

							if (index != -1)
							{
								// we have found a route....store some data.....
								AIPerkRouteInfo info;
								info.m_Index = index;
								info.m_RouteNo = i;
								
								//info.m_Dist = pITrackData->WorkOutOffsetToRacingSpline(pRoute, testPoint, index);
								
								info.m_Offset = pITrackData->WorkOutOffsetToRacingSpline(pRoute, testPoint, index);
								info.m_Dist = pRoute->GetDistanceUsingOffsetParamAndLine(info.m_Offset, AITrackDataLines::TRACK_DATA_LINE_RACING, index);
								aiPerk.m_routePerkInfo.PushBack(info);
								aiPerk.m_targetBy = -1;

								int TriggerIndex = index - 12;
								TriggerIndex = Maths::Max(TriggerIndex, 0);

								pRoute->SetAsATriggerSegment(TriggerIndex);

								// Maybe add a path of triggers in case the car can skip over...
								pRoute->SetAsATriggerSegment(TriggerIndex+1);
								pRoute->SetAsATriggerSegment(TriggerIndex+2);

							}
						}
					}

					// add the perk to the ai data structures........
					if (aiPerk.m_routePerkInfo.Size() > 0)
					{
						pIPerkSystem->AddPerkObject(aiPerk);
					}
					

				}
			}
		}
	}

	

	void AmaxAIR::HandleCollision(Game::C_ImpactMsg* pMsg)
	{
		if (pMsg->mCollisionInfo.m_ContactPoints.Size() && AllOK() )
		{
			Maths::HVec3 contactPoint = pMsg->mCollisionInfo.m_ContactPoints[0].m_ContactPoint;
			
			Game::Actors::C_VehicleActor *pVehicleActorA = 0;
			Game::Actors::C_VehicleActor *pVehicleActorB = 0;
			
			const Actor::C_Actor *pActorA = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetActorPtr(pMsg->mActorHandleA);
			const Actor::C_Actor *pActorB = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetActorPtr(pMsg->mActorHandleB);

			if(pActorA && pActorA->GetActorTypeId() == E_ActorTypeId_Vehicle)
				pVehicleActorA = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor>(pActorA);
			
			if(pActorB && pActorB->GetActorTypeId() == E_ActorTypeId_Vehicle)
				pVehicleActorB = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor>(pActorB);

		
			// must be done in this order....
			if (pVehicleActorA && pVehicleActorB)
			{
				// vehicle to vehicle collision
				Maths::HVec3 localContactA(contactPoint * pMsg->mMatrixA.InverseRotTrans());
				Maths::HVec3 localContactB(contactPoint * pMsg->mMatrixB.InverseRotTrans());

				int indexA = pVehicleActorA->GetVehicleIndex();
				int indexB = pVehicleActorB->GetVehicleIndex();
		
				if (pVehicleActorA->HasLogicActor())
				{
					// set collision data for vehicleA
					IData *pIDataA = m_pISystemAI->GetData(indexA);
					if (pIDataA)
					{
						CollisionInfo *pCol = pIDataA->GetState().m_Collisions.GetCollision(CollisionInfo::COLLISION_TYPE_VEHICLE_TO_VEHICLE);
						if (pCol)
						{
							pCol->m_collisionType = CollisionInfo::COLLISION_TYPE_VEHICLE_TO_VEHICLE;
							pCol->m_localContactPointA = localContactA;
							pCol->m_localContactPointB = localContactB;
							pCol->m_VehicleIndexA = indexA;
							pCol->m_VehicleIndexB = indexB;
							pCol->m_TimeStamp = m_pIGameServerTime->GetTime(); 

						}
					}
				}

				if (pVehicleActorB->HasLogicActor())
				{
					// set collision data for vehicleB
					IData *pIDataB = m_pISystemAI->GetData(indexB);
					if (pIDataB)
					{
						CollisionInfo *pCol = pIDataB->GetState().m_Collisions.GetCollision(CollisionInfo::COLLISION_TYPE_VEHICLE_TO_VEHICLE);
						if (pCol)
						{
							// just swap the local variables over
							pCol->m_collisionType = CollisionInfo::COLLISION_TYPE_VEHICLE_TO_VEHICLE;
							pCol->m_localContactPointA = localContactB;
							pCol->m_localContactPointB = localContactA;
							pCol->m_VehicleIndexA = indexB;
							pCol->m_VehicleIndexB = indexA;
							pCol->m_TimeStamp = m_pIGameServerTime->GetTime(); 

						}
					}
				}
	
			}
			else if (pVehicleActorA && pVehicleActorA->HasLogicActor() && pActorB)
			{
				// vehicle to dynamic object collision....
				Maths::HVec3 localContactA(contactPoint * pMsg->mMatrixA.InverseRotTrans());
				
				int indexA = pVehicleActorA->GetVehicleIndex();
				IData *pIDataA = m_pISystemAI->GetData(indexA);
				if (pIDataA)
				{
					CollisionInfo *pCol = pIDataA->GetState().m_Collisions.GetCollision(CollisionInfo::COLLISION_TYPE_VEHICLE_TO_OBJECTS);
					if (pCol)
					{
						pCol->m_collisionType = CollisionInfo::COLLISION_TYPE_VEHICLE_TO_OBJECTS;
						pCol->m_localContactPointA = localContactA;
						pCol->m_VehicleIndexA = indexA;
						pCol->m_TimeStamp = m_pIGameServerTime->GetTime(); 
					}
				}
		
			}
			else if (pVehicleActorA && pVehicleActorA->HasLogicActor() && pVehicleActorB == 0)
			{
				// vehicle to environment collision
				Maths::HVec3 localContactA(contactPoint * pMsg->mMatrixA.InverseRotTrans());

				int indexA = pVehicleActorA->GetVehicleIndex();
				IData *pIDataA = m_pISystemAI->GetData(indexA);
				if (pIDataA)
				{
					CollisionInfo *pCol = pIDataA->GetState().m_Collisions.GetCollision(CollisionInfo::COLLISION_TYPE_VEHICLE_TO_ENVIRONMENT);
					if (pCol)
					{
						pCol->m_collisionType = CollisionInfo::COLLISION_TYPE_VEHICLE_TO_ENVIRONMENT;
						pCol->m_localContactPointA = localContactA;
						pCol->m_VehicleIndexA = indexA;
						pCol->m_TimeStamp = m_pIGameServerTime->GetTime(); 
					}
				}
			}
		}
	}




	bool AmaxAIR::LoadEventData(const char *fileName)
	{
		MEMORY_TRACKER_PUSH("ai");

		bool fileFound = false;

		Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
		
		// set the difficulty......
		switch(levelState.m_difficulty)
		{	
		case Game::GAME_DIFFICULTY_VERY_EASY:
		case Game::GAME_DIFFICULTY_EASY:
			{
				g_DebugPlayerAIBrakeSpeedPercentage = g_DebugPlayerAIBrakeSpeedPercentageEasy;
			}
			break;

		case Game::GAME_DIFFICULTY_NORMAL:
			{
				g_DebugPlayerAIBrakeSpeedPercentage = g_DebugPlayerAIBrakeSpeedPercentageMedium;
			}
			break;

		case Game::GAME_DIFFICULTY_HARDCORE:
		case Game::GAME_DIFFICULTY_HARD:
			{
				g_DebugPlayerAIBrakeSpeedPercentage = g_DebugPlayerAIBrakeSpeedPercentageHard;
			}
			break;

		default:
			break;
		}

		IRaceDirector *pIRaceDirector = static_cast<IRaceDirector *>(GlobalsComponent::GetObjectManager().QueryInterface(CObjectId("RaceDirectorSystem"), IID_RACEDIRECTOR));

		std::string filename(fileName);
		Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();

		Sys::I_File *pFile = 0;
		if (fs.FileExists(filename.c_str()))
		{
			pFile = fs.Open(filename.c_str());
		}
		else
		{
			std::string filename2(".\\gamedata\\Events\\Default_Easy.csv");
			pFile = fs.Open(filename2.c_str());
		}
		
		if (pFile && pIRaceDirector)
		{
			Sys::TextDeserialiser ar(*pFile);
			
			float versionNo;
			int numSegments;
			int numOpponents;

			// read version No
			Util::C_String str;
			ar >> str;
			Util::C_String key(",");
			int end = str.Search(key);
			Util::C_String versionNoString = str(0,end);
			versionNo = (float)(atof(versionNoString.Chars()));

			BIZ_ASSERT(versionNo == 1.0f);


			// read no segments
			str.Clear();
			ar >> str;
			end = str.Search(key);
			Util::C_String numSegmentString = str(0,end);
			numSegments = atoi(numSegmentString.Chars());
			
			// read no opponents
			str.Clear();
			ar >> str;
			end = str.Search(key);
			Util::C_String numOpponentsString = str(0,end);	
			numOpponents = atoi(numOpponentsString.Chars());
			BIZ_ASSERT(numOpponents >0 && numOpponents <=20);

			/////////////////////////////////////////////////////////////////////////////
			/// Create the memory structures required.......
			AIEventBalancing *pEventBalancing = pIRaceDirector->GetEventBalancing();
			BIZ_ASSERT(pEventBalancing);
			for (int i=0; i < numOpponents; i++)
			{
				AIEventBalancingData *pOpponentData = new AIEventBalancingData();
				BIZ_ASSERT(pOpponentData);

				pEventBalancing->m_Data.PushBack(pOpponentData);

				pOpponentData->m_SegmentData.Resize(numSegments);
			}
			
			for(int noSegments = 0; noSegments < numSegments; noSegments++)
			{
				// read blank line
				str.Clear();
				ar >> str;
				str.Clear();

				// to
				ar >> str;
				int start = 0;
				for(int i=0; i<numOpponents; i++)
				{
					end = str.Search(key, start);
					Util::C_String toString;
					if (end == -1)
					{
						toString = str(start,str.Length());
					}
					else
					{
						toString = str(start,end);
					}
					

					int to = atoi(toString.Chars());

					AIEventBalancingData *pOpponentData = pEventBalancing->m_Data[i];
					BIZ_ASSERT(pOpponentData);
					AIEventSegmentData *pSegmentData = &pOpponentData->m_SegmentData[noSegments];
					BIZ_ASSERT(pSegmentData);
					
					pSegmentData->m_To = to;

					start = end+1;
				}

				// dist
				str.Clear();
				ar >> str;
				start = 0;
				for(int i=0; i<numOpponents; i++)
				{
					end = str.Search(key, start);
					Util::C_String distString;
					if (end == -1)
					{
						distString = str(start,str.Length());
					}
					else
					{
						distString = str(start,end);
					}
					

					float dist = (float) atof(distString.Chars());

					AIEventBalancingData *pOpponentData = pEventBalancing->m_Data[i];
					BIZ_ASSERT(pOpponentData);
					AIEventSegmentData *pSegmentData = &pOpponentData->m_SegmentData[noSegments];
					BIZ_ASSERT(pSegmentData);

					pSegmentData->m_Distance = dist;

					start = end+1;
				}

				// max track speed
				str.Clear();
				ar >> str;
				start = 0;
				for(int i=0; i<numOpponents; i++)
				{
					end = str.Search(key, start);
					Util::C_String maxTrackSpeedString;
					if (end == -1)
					{
						maxTrackSpeedString = str(start,str.Length());
					}
					else
					{
						maxTrackSpeedString = str(start,end);
					}


					float maxTrackSpeed = (float) atof(maxTrackSpeedString.Chars());

					AIEventBalancingData *pOpponentData = pEventBalancing->m_Data[i];
					BIZ_ASSERT(pOpponentData);
					AIEventSegmentData *pSegmentData = &pOpponentData->m_SegmentData[noSegments];
					BIZ_ASSERT(pSegmentData);
					pSegmentData->m_TrackSpeedPercentageMax = maxTrackSpeed - levelState.m_AIRetryInterval;
					pSegmentData->m_TrackSpeedPercentageMax = Maths::Max(pSegmentData->m_TrackSpeedPercentageMax, levelState.m_AIRetryMinTrackSpeed);

					start = end+1;
				}

				// min track speed
				str.Clear();
				ar >> str;
				start = 0;
				for(int i=0; i<numOpponents; i++)
				{
					end = str.Search(key, start);
					Util::C_String minTrackSpeedString;
					if (end == -1)
					{
						minTrackSpeedString = str(start,str.Length());
					}
					else
					{
						minTrackSpeedString = str(start,end);
					}

					float minTrackSpeed = (float) atof(minTrackSpeedString.Chars());

					AIEventBalancingData *pOpponentData = pEventBalancing->m_Data[i];
					BIZ_ASSERT(pOpponentData);
					AIEventSegmentData *pSegmentData = &pOpponentData->m_SegmentData[noSegments];
					BIZ_ASSERT(pSegmentData);
					pSegmentData->m_TrackSpeedPercentageMin = minTrackSpeed;

					start = end+1;
				}

				// enable
				str.Clear();
				ar >> str;
				start = 0;
				for(int i=0; i<numOpponents; i++)
				{
					end = str.Search(key, start);
					Util::C_String enableString;
					if (end == -1)
					{
						enableString = str(start,str.Length());
					}
					else
					{
						enableString = str(start,end);
					}

					int enable = atoi(enableString.Chars());
					AIEventBalancingData *pOpponentData = pEventBalancing->m_Data[i];
					BIZ_ASSERT(pOpponentData);
					AIEventSegmentData *pSegmentData = &pOpponentData->m_SegmentData[noSegments];
					BIZ_ASSERT(pSegmentData);
					pSegmentData->m_Enable = enable;

					start = end+1;
				}

				// Groups
				str.Clear();
				ar >> str;
				start = 0;
				for(int i=0; i<numOpponents; i++)
				{
					end = str.Search(key, start);
					Util::C_String groupString;
					if (end == -1)
					{
						groupString = str(start,str.Length());
					}
					else
					{
						groupString = str(start,end);
					}


					int groups = atoi(groupString.Chars());
					AIEventBalancingData *pOpponentData = pEventBalancing->m_Data[i];
					BIZ_ASSERT(pOpponentData);
					AIEventSegmentData *pSegmentData = &pOpponentData->m_SegmentData[noSegments];
					BIZ_ASSERT(pSegmentData);
					pSegmentData->m_Group = groups;

					start = end+1;
				}

				// Attributes template index
				str.Clear();
				ar >> str;
				start = 0;
				for(int i=0; i<numOpponents; i++)
				{
					end = str.Search(key, start);
					Util::C_String templateString;
					if (end == -1)
					{
						templateString = str(start,str.Length());
					}
					else
					{
						templateString = str(start,end);
					}


					int t = atoi(templateString.Chars());
					AIEventBalancingData *pOpponentData = pEventBalancing->m_Data[i];
					BIZ_ASSERT(pOpponentData);
					AIEventSegmentData *pSegmentData = &pOpponentData->m_SegmentData[noSegments];
					BIZ_ASSERT(pSegmentData);

#if 0
					// lets get the required handle to the PerkPrefs template
					S32 handle = AIAmaxXt::Instance().FindAISkillzID(t);
					pSegmentData->m_AttributeTemplateIndex = handle;//t;

					// test the handle we found.....
					AISkillzXt *pSkillz = AIAmaxXt::Instance().GetAISkillzData(handle);
					BIZ_ASSERT(pSkillz);
#else
					// lets get the required handle to the PerkPrefs template
					S32 handle = AIAmaxXt::Instance().FindAIPerkSkillzID(t);
					pSegmentData->m_AttributeTemplateIndex = handle;//t;

					// test the handle we found.....
					AIPerkSkillzXt *pSkillz = AIAmaxXt::Instance().GetAIPerkSkillzData(handle);
					BIZ_ASSERT(pSkillz);

#endif

					start = end+1;
				}

				// Perk priority template index
				str.Clear();
				ar >> str;
				start = 0;
				for(int i=0; i<numOpponents; i++)
				{
					end = str.Search(key, start);
					Util::C_String templateString;
					if (end == -1)
					{
						templateString = str(start,str.Length());
					}
					else
					{
						templateString = str(start,end);
					}

					int t = atoi(templateString.Chars());
					AIEventBalancingData *pOpponentData = pEventBalancing->m_Data[i];
					BIZ_ASSERT(pOpponentData);
					AIEventSegmentData *pSegmentData = &pOpponentData->m_SegmentData[noSegments];
					BIZ_ASSERT(pSegmentData);


					// lets get the required handle to the PerkPrefs template
					S32 handle = AIAmaxXt::Instance().FindPerkPrefsID(t);
					pSegmentData->m_PerkPrioritiesTemplateIndex = handle;//t;

					// test the handle we found.....
					AIPerkPrefsXt *pPerkPrefs = AIAmaxXt::Instance().GetAIPerkPrefsData(handle);
					BIZ_ASSERT(pPerkPrefs);

					start = end+1;
				}

			}

			delete pFile;
		}

		MEMORY_TRACKER_POP("ai");
		return fileFound;
	}

	// TODO : move this into it`s own object and component...., use the ai system to load it
	bool AmaxAIR::LoadAITrackData(const char *fileName)
	{
		MEMORY_TRACKER_PUSH("ai");

		bool fileFound = false;

		std::string filename(fileName);

		CObjectId trackdataSystem("TrackDataSystem");
		ITrackDataAI *pITrackData = static_cast<ITrackDataAI *>(GlobalsComponent::GetObjectManager().QueryInterface(trackdataSystem, IID_TRACK_DATA_AI));
		assert(pITrackData);

		AITrackData *pAIData = pITrackData->GetTrackData();
		// [Blur2PC] InitTrackDataSystem caches this interface right after posting MT_INIT_OBJECT, before
		// the object exists, so m_pITrackDataSystem stayed NULL: race logic found no splits and the AI
		// update (which requires it) never ran. The object exists by now; keep the pointer.
		if (!m_pITrackDataSystem)
			m_pITrackDataSystem = pITrackData;

		Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
		if (fs.FileExists(filename.c_str()))
		{
			Sys::I_File *pFile = fs.Open(filename.c_str());
			if (pFile)
			{
				Sys::BinaryDeserialiser<Ut::LittleEndianToNative> ar(*pFile);

				float	versionNo;
				ar >> versionNo;

				
				if (versionNo == 6.0f)
				{
					LOGDEBUG(g_LoggerLoading, "AI data format is revision 6.0....\n");
					fileFound = LoadVersionSix(ar, pAIData, pITrackData);
				}
				else
				{
					// wrong version no......
					LOGDEBUG(g_LoggerLoading, "AI data is not the correct format....\n");
				}

				delete pFile;
			}
		}
		// [Blur2PC] the failure paths above only log at Debug level; report the outcome
		LOGINFO(g_LoggerLoading, "AI track data '%s': exists %d, loaded %d, routes %d\n", fileName, (int)fs.FileExists(filename.c_str()), (int)fileFound, pAIData ? (int)pAIData->m_Routes.Size() : -1);
		MEMORY_TRACKER_POP("ai");
		return fileFound;
	}

	

	

	

	bool AmaxAIR::LoadVersionSix(Sys::BinaryDeserialiser<Ut::LittleEndianToNative> &ar, AITrackData *pAIData, ITrackDataAI *pITrackData)
	{
		MEMORY_TRACKER_PUSH("ai");

		bool loaded = true;

		int noRoutes;
		ar >> noRoutes;

		for(int i=0; i<noRoutes; i++)
		{
			AITrackDataRoutes *pRoute = new AITrackDataRoutes();
			assert(pRoute);
			pAIData->AddRoute(pRoute);

			int RouteNameLength;
			ar >> RouteNameLength;

			char *RouteName = new char[RouteNameLength];
			ar.Read(RouteName,	sizeof(char)*RouteNameLength);
			std::string RouteNameSTD(RouteName,RouteNameLength );
			transform(RouteNameSTD.begin (), RouteNameSTD.end (), RouteNameSTD.begin (), (int(*)(int)) tolower);
			pRoute->m_Name.CreateHash(RouteNameSTD.c_str());
			delete [] RouteName;
			
			int junctionInputNameLength;
			ar >> junctionInputNameLength;
			char *junctionInputName = new char[junctionInputNameLength];
			ar.Read(junctionInputName,	sizeof(char)*junctionInputNameLength);
			std::string junctionInputNameSTD(junctionInputName,junctionInputNameLength);
			transform(junctionInputNameSTD.begin (), junctionInputNameSTD.end (), junctionInputNameSTD.begin (), (int(*)(int)) tolower);
			delete [] junctionInputName;
		
			pRoute->m_InputRoute.CreateHash(junctionInputNameSTD.c_str());


			
			int junctionOutputNameLength;
			ar >> junctionOutputNameLength;
			char *junctionOutputName = new char[junctionOutputNameLength];
			ar.Read(junctionOutputName,	sizeof(char)*junctionOutputNameLength);
			std::string junctionOutputNameSTD(junctionOutputName,junctionOutputNameLength);
			transform(junctionOutputNameSTD.begin (), junctionOutputNameSTD.end (), junctionOutputNameSTD.begin (), (int(*)(int)) tolower);
			delete [] junctionOutputName;
			
			pRoute->m_OutputRoute.CreateHash(junctionOutputNameSTD.c_str());


			int routeFlags;
			ar >> routeFlags;
			pRoute->m_RouteFlags = routeFlags;

			if (pRoute->m_RouteFlags & AITrackDataRoutes::ROUTE_DYNAMIC_JUNCTION_ZONE)
			{
				// we now need to start reading the junction
				int ruleNo;
				ar >> ruleNo;

				switch(ruleNo)
				{
				case 1:
					{
						int state;
						ar >> state;

						JunctionRule1 *pJunctionRule = new JunctionRule1();
						assert(pJunctionRule);
						pJunctionRule->m_ruleNO = ruleNo;
						
						if (state == 1)
							pJunctionRule->m_state = JunctionRule::JUNCTION_OPEN;
						else
							pJunctionRule->m_state = JunctionRule::JUNCTION_CLOSE;
							
						pRoute->m_pJunctionRule = pJunctionRule;
						
					}
					break;

				case 2:
					{
						JunctionRule2 *pJunctionRule = new JunctionRule2();
						assert(pJunctionRule);
						pJunctionRule->m_ruleNO = ruleNo;

						// route name
						
						int routeRuleNameLength;	
						ar >> routeRuleNameLength;
						char *routeRuleName = new char[routeRuleNameLength];
						ar.Read(routeRuleName,	sizeof(char)*routeRuleNameLength);
						std::string routeRuleNameSTD(routeRuleName,routeRuleNameLength);
						transform(routeRuleNameSTD.begin (), routeRuleNameSTD.end (), routeRuleNameSTD.begin (), (int(*)(int)) tolower);
						delete routeRuleName;

						int state;
						ar >> state;

					
						pJunctionRule->m_routeName.CreateHash(routeRuleNameSTD.c_str());

						if (state == 1)
							pJunctionRule->m_state = JunctionRule::JUNCTION_OPEN;
						else
							pJunctionRule->m_state = JunctionRule::JUNCTION_CLOSE;


						pRoute->m_pJunctionRule = pJunctionRule;
					}
					break;

				case 3:
					{	
						JunctionRule3 *pJunctionRule = new JunctionRule3();
						assert(pJunctionRule);
						pJunctionRule->m_ruleNO = ruleNo;

						
						int routeRuleNameLength;

						ar >> routeRuleNameLength;
						char *routeRuleName = new char[routeRuleNameLength];
						ar.Read(routeRuleName,	sizeof(char)*routeRuleNameLength);

						std::string routeRuleNameSTD(routeRuleName,routeRuleNameLength);
						transform(routeRuleNameSTD.begin (), routeRuleNameSTD.end (), routeRuleNameSTD.begin (), (int(*)(int)) tolower);
						delete routeRuleName;
						
						
						int state;
						ar >> state;

						float seconds;
						ar >> seconds;

						pJunctionRule->m_routeName.CreateHash(routeRuleNameSTD.c_str());
						if (state == 1)
							pJunctionRule->m_state = JunctionRule::JUNCTION_OPEN;
						else
							pJunctionRule->m_state = JunctionRule::JUNCTION_CLOSE;

						pJunctionRule->m_t = seconds;

						pRoute->m_pJunctionRule = pJunctionRule;
					}
					break;

				case 4:
					{
						JunctionRule4 *pJunctionRule = new JunctionRule4();
						assert(pJunctionRule);
						pJunctionRule->m_ruleNO = ruleNo;

						int routeRuleNameLength;

						ar >> routeRuleNameLength;
						char *routeRuleName = new char[routeRuleNameLength];
						ar.Read(routeRuleName,	sizeof(char)*routeRuleNameLength);

						std::string routeRuleNameSTD(routeRuleName,routeRuleNameLength);
						transform(routeRuleNameSTD.begin (), routeRuleNameSTD.end (), routeRuleNameSTD.begin (), (int(*)(int)) tolower);
						delete routeRuleName;
						
						int state[2];
						ar >> state[0];

						float seconds[2];
						ar >> seconds[0];

						ar >> state[1];
						ar >> seconds[1];

						pJunctionRule->m_routeName.CreateHash(routeRuleNameSTD.c_str());
						
						for(int j=0; j<2; j++)
						{
							if (state[j] == 1)
								pJunctionRule->m_state[j] = JunctionRule::JUNCTION_OPEN;
							else
								pJunctionRule->m_state[j] = JunctionRule::JUNCTION_CLOSE;

							pJunctionRule->m_t[j] = seconds[j];
						}
						
						pRoute->m_pJunctionRule = pJunctionRule;

					}
					break;

				case 5:
					{
						int state[2];
						float seconds[2];

						ar >> state[0];
						ar >> seconds[0];
						ar >> state[1];
						ar >> seconds[1];

						JunctionRule5 *pJunctionRule = new JunctionRule5();
						assert(pJunctionRule);
						pJunctionRule->m_ruleNO = ruleNo;

						for(int j=0; j<2; j++)
						{
							if (state[j] == 1)
								pJunctionRule->m_state[j] = JunctionRule::JUNCTION_OPEN;
							else
								pJunctionRule->m_state[j] = JunctionRule::JUNCTION_CLOSE;

							pJunctionRule->m_t[j] = seconds[j];
						}

						pRoute->m_pJunctionRule = pJunctionRule;
					}
					break;

				case 6:
					{
						int state;
						ar >> state;

						float lap;
						ar >> lap;

						JunctionRule6 *pJunctionRule = new JunctionRule6();
						assert(pJunctionRule);
						pJunctionRule->m_ruleNO = ruleNo;

						if (state == 1)
							pJunctionRule->m_state = JunctionRule::JUNCTION_OPEN;
						else
							pJunctionRule->m_state = JunctionRule::JUNCTION_CLOSE;

						pJunctionRule->m_lap = static_cast<int>(lap);

						pRoute->m_pJunctionRule = pJunctionRule;
					}
					break;

				case 7:
					{
						int state;
						ar >> state;

						float lap;
						ar >> lap;

						JunctionRule7 *pJunctionRule = new JunctionRule7();
						assert(pJunctionRule);
						pJunctionRule->m_ruleNO = ruleNo;


						if (state == 1)
							pJunctionRule->m_state = JunctionRule::JUNCTION_OPEN;
						else
							pJunctionRule->m_state = JunctionRule::JUNCTION_CLOSE;

						pJunctionRule->m_lap = static_cast<int>(lap);

						pRoute->m_pJunctionRule = pJunctionRule;
					}
					break;

				case 8:
					{
						int state;
						ar >> state;

						float lap[3];
						ar >> lap[0];
						ar >> lap[1];
						ar >> lap[2];

						JunctionRule8 *pJunctionRule = new JunctionRule8();
						assert(pJunctionRule);
						pJunctionRule->m_ruleNO = ruleNo;

						if (state == 1)
							pJunctionRule->m_state = JunctionRule::JUNCTION_OPEN;
						else
							pJunctionRule->m_state = JunctionRule::JUNCTION_CLOSE;
							
						pJunctionRule->m_lap[0] = static_cast<int>(lap[0]);
						pJunctionRule->m_lap[1] = static_cast<int>(lap[1]);
						pJunctionRule->m_lap[2] = static_cast<int>(lap[2]);


						pRoute->m_pJunctionRule = pJunctionRule;
					}
					break;

				case 9:
					{
						
						int state;
						ar >> state;

						float nCars;
						ar >>nCars;

						float resetSeconds;
						ar >> resetSeconds;

						JunctionRule9 *pJunctionRule = new JunctionRule9();
						assert(pJunctionRule);
						pJunctionRule->m_ruleNO = ruleNo;

						if (state == 1)
							pJunctionRule->m_state = JunctionRule::JUNCTION_OPEN;
						else
							pJunctionRule->m_state = JunctionRule::JUNCTION_CLOSE;

						
						pJunctionRule->m_Ncars = static_cast<int>(nCars);
						pJunctionRule->m_resetTime = resetSeconds;
						pRoute->m_pJunctionRule = pJunctionRule;

					}
					break;

				default:
					{
						JunctionRule *pJunctionRule = new JunctionRule();
						assert(pJunctionRule);
						pJunctionRule->m_ruleNO = ruleNo;
						pRoute->m_pJunctionRule = pJunctionRule;

					}
					break;
				}
			}

			int noSamples;
			ar >> noSamples;

			pRoute->m_Segments.Reserve(noSamples);
			pRoute->m_speedData.Initialise(noSamples);
			pRoute->m_routeLength = pRoute->GetStepSize() * noSamples;

			for (int j=0; j < noSamples; j++)
			{
				pRoute->m_Segments.PushBack();
				AITrackDataSegments& segment = pRoute->m_Segments.Back();

				float xPos, yPos,zPos;
				ar >> xPos;
				ar >> yPos;
				ar >> zPos;

				segment.m_position.x = xPos;
				segment.m_position.y = -zPos;

				float width;
				ar >> width;
				segment.SetWidth(Maths::HF32(width));

				float tangentX, tangentZ;
				ar >> tangentX;
				ar >> tangentZ;

				segment.m_tangent.x = tangentX;
				segment.m_tangent.y = -tangentZ;


				float tangentXYZx, tangentXYZy, tangentXYZz;
				ar >> tangentXYZx;
				ar >> tangentXYZy;
				ar >> tangentXYZz;


				float minY, maxY;
				ar >> minY;
				ar >> maxY;

				segment.SetMinY(Maths::HF32(minY));
				segment.SetMaxY(Maths::HF32(maxY));

				int noLines;
				ar >> noLines;

				segment.m_Lines.Reserve(noLines);

				for (int k=0; k < noLines; k++)
				{
					segment.m_Lines.PushBack();
					AITrackDataLines& line = segment.m_Lines.Back();

					float curvature;
					ar >> curvature;
					line.SetCurvature(Maths::HF32(curvature));

					float distance;
					ar >> distance;
					line.SetDistance(Maths::HF32(distance));

					int	pointFlags;
					ar >> pointFlags;
					//line.m_flags = pointFlags;

					float height;
					ar >> height;
					line.SetHeight(Maths::HF32(height));

					float offset;
					ar >> offset;
					line.SetOffset(Maths::HF32(offset));

					char surfaceFlags;
					ar >> surfaceFlags;
					//pLine->m_surfaceFlags = surfaceFlags;

					float yangle;
					ar >> yangle;
					//pLine->m_yangle = yangle;
				}
			}
		}

		// will do this here......
		pITrackData->CalculateTrackData();

		// we still have stuff to load.......
		int dataCacheSize;
		ar >> dataCacheSize;

		// get the logic route........
		AITrackDataRoutes *pLogicRoute = 0;	
		for(int i=0; i<(int)pAIData->GetNumRoutes(); i++)
		{
			AITrackDataRoutes *pRoute = pAIData->GetRoute(i);
			if (pRoute->IsLogic())
			{
				pLogicRoute = pRoute;
				break;
			}
		}


		if (pLogicRoute)
		{
			pLogicRoute->m_dataCache.Reserve(dataCacheSize);

			for(int i=0; i<dataCacheSize; i++)
			{
				pLogicRoute->m_dataCache.PushBack();
				AITrackDataCache *pDataCache = &pLogicRoute->m_dataCache.Back();

				int dataRecords;
				ar >> dataRecords;

				if (i == 306)
				{
					int stop = 0;

					stop++;
				}

				pDataCache->m_data.Reserve(dataRecords);

				AITrackDataRoutes *pTestRouteLast = 0;
				int lastIndex = -1;

				for(int j=0; j < dataRecords; j++)
				{
					int index;
					ar >> index;

					int routeNameLength;
					ar >> routeNameLength;

					char *routeName = new char[routeNameLength];
					ar.Read(routeName,	sizeof(char)*routeNameLength);

					std::string routeNameNameSTD(routeName, routeNameLength);
					transform(routeNameNameSTD.begin (), routeNameNameSTD.end (), routeNameNameSTD.begin (), (int(*)(int)) tolower);
					delete routeName;

					AITrackDataRoutes *pTestRoute = pAIData->GetRoute(CHash(routeNameNameSTD.c_str()));
					assert(pTestRoute);

					//we have multiple entries per segment.......
					bool addRecord = true;
					if (pTestRoute == pTestRouteLast)
					{
						int diff = index - lastIndex;
						if (diff < 8)
						{
							addRecord = false;
						}
					}
					
					


					if (addRecord)
					{
						lastIndex = index;
						AITrackDataCachedData aiTDCD(pTestRoute, index);
						pDataCache->m_data.PushBack(aiTDCD);
					}
					

					pTestRouteLast = pTestRoute;
				}
			}
		}

		MEMORY_TRACKER_POP("ai");

		return loaded;
	}


	void AmaxAIR::ReLoadSorceryAIData()
	{
		PERF_EVENT("AmaxAIR::ReLoadSorceryAIData");

		if ( AllOK() )
		{
			for(int i=0; i<m_cars.Size(); i++)
			{
				AIAmaxVehicleData *pData = m_cars[i];
				assert(pData);

				AISteeringXt *pAISteering = AIAmaxXt::Instance().GetAISteeringData( pData->m_AISteering );
				AIXt *pAiData = AIAmaxXt::Instance().GetAIPersonalityData(pData->m_AIIndex);
				assert(pAISteering);
				assert(pAiData);

				IData *pIData = m_pISystemAI->GetData(i);
				assert(pIData);
				const airacingtech::DriverData *pDriverData = pIData->GetParams()->GetDriverData();
				assert(pDriverData);

				// Params data.....
				airacingtech::DriverData driverData;
				driverData.m_name = pDriverData->m_name; // set the name...
				CopyDriverData(driverData, pAiData, pDriverData->m_difficulty);

				// PID data....
				PIDControllerInitialiseParams nodePID;
				CopyPIDData(nodePID, pAISteering);

				// Steering data...
				SteeringControllerInitialiseParams nodeSteering;
				CopySteeringData(nodeSteering, pAISteering);

				CMessage msg;
				msg.mType = MT_RELOAD_DRIVER_DATA;
				msg.mpData = &driverData;
				m_pObjectManager->PostAMessage(pIData->GetObjectId(), msg);

				msg.mType = MT_RELOAD_PID_DATA;
				msg.mpData = &nodePID;
				m_pObjectManager->PostAMessage(pIData->GetObjectId(), msg);

				msg.mType = MT_RELOAD_STEERING_DATA;
				msg.mpData = &nodeSteering;
				m_pObjectManager->PostAMessage(pIData->GetObjectId(), msg);

			}
		}
	}

	void AmaxAIR::LoadAIDriver(const AILoadDriveraMsg* pDriver)
	{
		//TODO : Depending upon the game mode, only initialise certain components.....
		AISteeringXt *pAISteering = AIAmaxXt::Instance().GetAISteeringData( pDriver->m_driver.m_SteeringAI );
		AIXt *pAiData = AIAmaxXt::Instance().GetAIPersonalityData(pDriver->m_driver.m_AIIndex);
		assert(pAISteering);
		assert(pAiData);
	
		Util::C_String num;
		num << m_cars.Size();
		Util::C_String uniqueName = Util::C_String("Player_") + num;

		assert(pDriver);
		
		CParameterNode params;
		params.SetData(0);
		params.SetName(CHash("DriverAI"));
		params.SetObjectName(CHash(uniqueName));		// actual object name, ie driver name
		params.SetType(CHash("OBJECT"));
		///////////////////////////////////////

		
		// create the vehicle state...........
		CParameterNode paramsVehicleState;	
		std::string vehName(pDriver->m_driver.m_VehicleName);
		paramsVehicleState.SetData(&vehName);
		paramsVehicleState.SetName(CHash("VehicleState"));	// component type
		paramsVehicleState.SetType(CHash("COMPONENT"));
		///////////////////////////////////////


		// create the control state...........
		CParameterNode paramsControlState;	
		paramsControlState.SetData(0);
		paramsControlState.SetName(CHash("ControlState"));	// component type
		paramsControlState.SetType(CHash("COMPONENT"));
		/////////////////////////////////////

		// create a data
		CParameterNode paramsData;
		paramsData.SetData(0);
		paramsData.SetName(CHash("DataAI"));			// component type
		paramsData.SetType(CHash("COMPONENT"));

		CParameterNode paramsAccelBrakeController;
		paramsAccelBrakeController.SetData(0);
		paramsAccelBrakeController.SetName(CHash("AccelBrakeController"));			// component type
		paramsAccelBrakeController.SetType(CHash("COMPONENT"));


		CParameterNode paramsAIparams;
		airacingtech::DriverData driverData;
		driverData.m_name = CObjectId(pDriver->m_driver.m_CharacterName); // this was the name
		CopyDriverData(driverData, pAiData, pDriver->m_driver.m_difficulty); 
		paramsAIparams.SetData(&driverData);
		paramsAIparams.SetName(CHash("ParamsAI"));
		paramsAIparams.SetType(CHash("COMPONENT"));


		CParameterNode SplineTrackerParams;
		SplineTrackerParams.SetData(0);
		SplineTrackerParams.SetName(CHash("SplineTracker"));			// component type
		SplineTrackerParams.SetType(CHash("COMPONENT"));

		// create the pid controller........
		PIDControllerInitialiseParams nodePID;
		CopyPIDData(nodePID, pAISteering);
		CParameterNode paramsPID;
		paramsPID.SetData(&nodePID);
		paramsPID.SetName(CHash("PIDController"));			// component type
		paramsPID.SetType(CHash("COMPONENT"));


		/// Steering controller setup...
		SteeringControllerInitialiseParams nodeSteering;
		CopySteeringData(nodeSteering, pAISteering);
		CParameterNode paramsSplineFollower;
		paramsSplineFollower.SetData(&nodeSteering);
		paramsSplineFollower.SetName(CHash("SplineFollower"));			// component type
		paramsSplineFollower.SetType(CHash("COMPONENT"));

		CParameterNode paramsSpeedTracker;
		paramsSpeedTracker.SetData(0);
		paramsSpeedTracker.SetName(CHash("SpeedTracker"));			// component type
		paramsSpeedTracker.SetType(CHash("COMPONENT"));


		CParameterNode paramsAvoidance;
		paramsAvoidance.SetData(0);
		paramsAvoidance.SetName(CHash("AvoidanceBasic"));			// component type
		paramsAvoidance.SetType(CHash("COMPONENT"));


		CParameterNode paramsVehicleLogic;
		paramsVehicleLogic.SetData(0);
		paramsVehicleLogic.SetName(CHash("VehicleLogic"));			// component type
		paramsVehicleLogic.SetType(CHash("COMPONENT"));

		CParameterNode paramsRecovery;
		paramsRecovery.SetData(0);
		paramsRecovery.SetName(CHash("RecoveryAI"));			// component type
		paramsRecovery.SetType(CHash("COMPONENT"));

		CParameterNode paramsRubberbanding;
		paramsRubberbanding.SetData(0);
		paramsRubberbanding.SetName(CHash("RubberbandingAI"));			// component type
		paramsRubberbanding.SetType(CHash("COMPONENT"));
		
		CParameterNode paramsDebugAI;
		paramsDebugAI.SetData(0);
		paramsDebugAI.SetName(CHash("DebugAI"));			// component type
		paramsDebugAI.SetType(CHash("COMPONENT"));


		CParameterNode paramsIntroOutroAI;
		paramsIntroOutroAI.SetData(0);
		paramsIntroOutroAI.SetName(CHash("IntroOutroSplineAI"));			// component type
		paramsIntroOutroAI.SetType(CHash("COMPONENT"));

		CParameterNode paramsPerkLogicAI;
		paramsPerkLogicAI.SetData(0);
		paramsPerkLogicAI.SetName(CHash("PerkLogicAI"));			// component type
		paramsPerkLogicAI.SetType(CHash("COMPONENT"));


		CParameterNode paramsPerkSkillUsage;
		paramsPerkSkillUsage.SetData(0);
		paramsPerkSkillUsage.SetName(CHash("PerkSkillUsageAI"));			// component type
		paramsPerkSkillUsage.SetType(CHash("COMPONENT"));


		CParameterNode paramsGameBrain;
		paramsGameBrain.SetData(0);
		paramsGameBrain.SetName(CHash("GameBrain"));			// component type
		paramsGameBrain.SetType(CHash("COMPONENT"));


		// load up the perks
		CParameterNode paramsPerkNitro;
		paramsPerkNitro.SetData(0);
		paramsPerkNitro.SetName(CHash("PerkNitro"));			// component type
		paramsPerkNitro.SetType(CHash("COMPONENT"));

		CParameterNode paramsPerkShunt;
		paramsPerkShunt.SetData(0);
		paramsPerkShunt.SetName(CHash("PerkShunt"));			// component type
		paramsPerkShunt.SetType(CHash("COMPONENT"));

		CParameterNode paramsPerkShock;
		paramsPerkShock.SetData(0);
		paramsPerkShock.SetName(CHash("PerkShock"));			// component type
		paramsPerkShock.SetType(CHash("COMPONENT"));

		CParameterNode paramsPerkShield;
		paramsPerkShield.SetData(0);
		paramsPerkShield.SetName(CHash("PerkShield"));			// component type
		paramsPerkShield.SetType(CHash("COMPONENT"));

		CParameterNode paramsPerkBarge;
		paramsPerkBarge.SetData(0);
		paramsPerkBarge.SetName(CHash("PerkBarge"));			// component type
		paramsPerkBarge.SetType(CHash("COMPONENT"));

		CParameterNode paramsPerkBolt;
		paramsPerkBolt.SetData(0);
		paramsPerkBolt.SetName(CHash("PerkBolt"));			// component type
		paramsPerkBolt.SetType(CHash("COMPONENT"));

		CParameterNode paramsPerkMine;
		paramsPerkMine.SetData(0);
		paramsPerkMine.SetName(CHash("PerkMine"));			// component type
		paramsPerkMine.SetType(CHash("COMPONENT"));

		CParameterNode paramsPerkRepair;
		paramsPerkRepair.SetData(0);
		paramsPerkRepair.SetName(CHash("PerkRepair"));			// component type
		paramsPerkRepair.SetType(CHash("COMPONENT"));


		params.AddChildNode(paramsVehicleState);
		params.AddChildNode(paramsControlState);
		params.AddChildNode(paramsData);
		params.AddChildNode(paramsAccelBrakeController);	
		params.AddChildNode(paramsAIparams);
		params.AddChildNode(paramsPID);
		params.AddChildNode(SplineTrackerParams);
		params.AddChildNode(paramsSplineFollower);
		params.AddChildNode(paramsSpeedTracker);
		params.AddChildNode(paramsAvoidance);
		params.AddChildNode(paramsVehicleLogic);
		params.AddChildNode(paramsDebugAI);
		params.AddChildNode(paramsRecovery);
		params.AddChildNode(paramsRubberbanding);
		params.AddChildNode(paramsIntroOutroAI);
		params.AddChildNode(paramsPerkLogicAI);
		params.AddChildNode(paramsPerkSkillUsage);
		params.AddChildNode(paramsGameBrain);

		params.AddChildNode(paramsPerkNitro);
		params.AddChildNode(paramsPerkShunt);
		params.AddChildNode(paramsPerkShock);
		params.AddChildNode(paramsPerkShield);
		params.AddChildNode(paramsPerkBarge);
		params.AddChildNode(paramsPerkBolt);
		params.AddChildNode(paramsPerkMine);
		params.AddChildNode(paramsPerkRepair);


		// [Blur2PC] the snapshot cleared m_SystemsInitialised when a route's AI data failed to load, so no
		// driver was created for it (LoadAICityData now keeps the flag, or the Showroom's failure would
		// poison the next level). Keep that no-data behaviour here: a driver created without track data
		// never gets MT_POST_INIT_AI_SYSTEM, and the first MT_STOP_AI (C_VehicleActor::Reset) then
		// dereferences CData's NULL vehicle-state interface in CSpeedTracker::HandleMessage.
		if (m_SystemsInitialised && m_DataInitilaised)
		{
			CMessage msg;
			msg.mType = MT_INIT_AI_DRIVER;
			msg.mpData = &params;
			m_pObjectManager->PostAMessage(m_IDSystemAI, msg);

			AIAmaxVehicleData *pData = new AIAmaxVehicleData();
			pData->m_AIIndex = pDriver->m_driver.m_AIIndex;
			pData->m_AISteering = pDriver->m_driver.m_SteeringAI;
			pData->m_pVehicleActor = pDriver->m_driver.m_pVehicleActor;
			m_cars.PushBack(pData);

		}

	}


	void AmaxAIR::CopyPIDData(PIDControllerInitialiseParams &nodePID, AISteeringXt *pAISteering)
	{
		if (pAISteering)
		{
			nodePID.p = pAISteering->m_SteeringParams.m_PID.m_P;
			nodePID.i = pAISteering->m_SteeringParams.m_PID.m_I;
			nodePID.d = pAISteering->m_SteeringParams.m_PID.m_D;
			nodePID.m_numSlots = pAISteering->m_SteeringParams.m_PID.m_Slots;
			nodePID.m_timeStep = pAISteering->m_SteeringParams.m_PID.m_TimeStep;
		}
		else
		{
			
			nodePID.p = 6.0f;
			nodePID.i = 2.8666f;
			nodePID.d = 1.2f;
			nodePID.m_numSlots = 5;//100;
			nodePID.m_timeStep = m_TimeStep;
		}
		
	}

	void AmaxAIR::CopySteeringData(SteeringControllerInitialiseParams &nodeSteering, AISteeringXt *pAISteering)
	{
		if (pAISteering)
		{
			nodeSteering.m_UseVelocity = pAISteering->m_SteeringParams.m_UseVelocity;
			nodeSteering.m_SteeringTimeStep = pAISteering->m_SteeringParams.m_SteeringTimeStep;
			nodeSteering.m_SteeringRate = pAISteering->m_SteeringParams.m_SteeringRate;
			nodeSteering.m_SmoothTimeStep = pAISteering->m_SteeringParams.m_SmoothTimeStep;
			nodeSteering.m_LerpValue = pAISteering->m_SteeringParams.m_LerpValue;
			nodeSteering.m_DelayValue = pAISteering->m_SteeringParams.m_DelayValue;
			nodeSteering.m_Blend = pAISteering->m_SteeringParams.m_Blend;
			nodeSteering.m_AngVelSteering = pAISteering->m_SteeringParams.m_AngVelSteering;
			nodeSteering.m_AlignedSteering = pAISteering->m_SteeringParams.m_AlignedSteering;
			nodeSteering.m_ForwardsDist = pAISteering->m_SteeringParams.m_ForwardsDist;
			nodeSteering.m_UseOffset = pAISteering->m_SteeringParams.m_AIOffset.m_UseOffset;
			nodeSteering.m_OffsetValue = pAISteering->m_SteeringParams.m_AIOffset.m_OffsetValue;
		}
		else
		{
			// should really fill in some debug variables, but hey I want to assert.
			assert(0);
		}
		
	}



	void AmaxAIR::CopyDriverData(airacingtech::DriverData &driver, AIXt *pAiData, float difficulty)
	{
		/*
		driver.m_difficulty = difficulty;
		driver.m_difficulty = Maths::Clamped(driver.m_difficulty, 0.1f, 1.0f);
		
		// Multiply the difficulty through. Currently applied to all paramaters, might be better applying to only a few !!!!
		driver.m_speed.m_brakingCoefficient = pAiData->m_params.m_speed.m_brakingCoefficient;// * difficulty;
		driver.m_speed.m_brakingCoefficient = Maths::Clamped(driver.m_speed.m_brakingCoefficient,0.01f, 3.0f); // Sensible ranges.....
		driver.m_speed.m_frictionCoefficient = pAiData->m_params.m_speed.m_frictionCoefficient;// * difficulty;
		driver.m_speed.m_frictionCoefficient = Maths::Clamped(driver.m_speed.m_frictionCoefficient,0.01f, 3.0f);	
		*/
	
		driver.m_difficulty = 1.0f;
		driver.m_speed.m_brakingCoefficient = 1.0f;
		driver.m_speed.m_frictionCoefficient = 1.0f;

	}

	void AmaxAIR::PreUpdate()
	{
		
		// Darren : This is to get around the rolling start, legacy for when the intro/outro being used, 
		//          this was a quick dirty solution. It will be better to have a rolling start flag. TODO:
		if (!m_RollingStartSpeedEnabled_HACK)
		{
			// update any game mode stuff here
			CMessage msg;
			msg.mType = MT_PRE_UPDATE_GAME_MODE;
			msg.mpData = 0;
			m_pObjectManager->BroadcastMessage(msg);
		}
		

		int noVehicles =  m_cars.Size();
		for(int i=0; i<noVehicles; i++)
		{
			PreUpdateExternal(i);
		}

		m_pISystemAI->PreUpdate();
	}
	
	Debug::Bool g_debugReloadSorceryAIData("AI.ReloadSorceryData", false, Debug::Persist());

	void AmaxAIR::UpdateSystem(bool updateSystem)
	{
		if (IsDataInitialised() &&  IsSystemsInitialised() && m_safeToSimulate)
		{
			if (m_pIGameServerTime) 
			{
				m_pIGameServerTime->SetTimeAccum(m_TimeStep);
			}

			if (updateSystem)
			{
				// run the ai 
				PreUpdate();
				m_pISystemAI->Update();
				PostUpdate();
			}
			else
			{
				// Horrible hack with the way the Ed`s Racing physics work. (ie cannot subdivide the timestep as Gravity can)
				// we just want to forward any forces and steering controls
				PostUpdateControlsOnly();
			}


			
		

#ifndef CFG_GOLD
			// this needs to be knocked out on a master build......
			
			if (!updateSystem)
			{
				DebugDraw();
			}
#ifdef _RELOAD_SORCERY_DATA
		if (g_debugReloadSorceryAIData)
			ReLoadSorceryAIData();
#endif
#endif
		}
	}

	void AmaxAIR::Update(bool updateSystem)
	{
		UpdateSystem(updateSystem);
	}

#define _USE_RUBBER_VELOCITY_
	
	void AmaxAIR::PostUpdateExternal(int i)
	{
		PERF_EVENT("AI.PostUpdateExternal");
		Maths::HVec3 AItorque(0.0f,0.0f,0.0f);
		Maths::HVec3 AIlocalforce(0.0f,0.0f,0.0f);

		Game::Actors::C_VehicleActor *pActor = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor>(m_cars[i]->m_pVehicleActor);
		class RacingPhysics::Car *pCar = pActor->GetVehiclePhysics();
		class RacingPhysics::Vehicle *pVeh =  static_cast<RacingPhysics::Vehicle*>(pCar);
		IData *pIData = m_pISystemAI->GetData(i);
		VehicleState *pVehicleState = pIData->GetVehicleState()->GetState();
		IRubberbandingAI *pRubberBanding = pIData->GetRubberBanding();
		FillInAnyRaceLogic(pActor, i, pIData);

		if (pVeh->GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_AI) 
		{
			//IData *pIData = m_pISystemAI->GetData(i);
			RacingPhysics::CarControls controls;
			airacingtech::ControlState *pState = pIData->GetControlState()->GetState();

			pState->ClampAllValues(); // just to be in the safe side...
		
			float accel = pState->GetAccel();
				
#if !defined(CFG_GOLD) || defined(CFG_HACKED_MASTER)
			static Util::C_Random s_AISteerRandom;

			if (g_AIVehiclesBehaveCrazy)
			{
				float fRandSteer = (float)s_AISteerRandom.FRand(-1.0f, 1.0f);
				if (fRandSteer < 0.0f)
					fRandSteer = -1.0f;
				else
					fRandSteer = 1.0f;

				float fSteer = pState->GetSteering();
				fSteer = Maths::Clamp(fSteer + fRandSteer, -1.0f, 1.0f);
				pState->SetSteering(fSteer);
			}
#endif

			Actor::C_ActorHandle actorHandle = pActor->GetHandle();
			Game::Actors::C_VehicleUpdateAIControlsMsg vehicleControls;
			vehicleControls.m_fSteering		= -pState->GetSteering();
			vehicleControls.m_fPitch		= pState->GetPitch();
			vehicleControls.m_fBrake		= pState->GetBrake();
			//vehicleControls.m_bHandbrake	= pState->GetHandBrake();
			vehicleControls.m_bHandbrake = m_handBrakeOn | pState->GetHandBrake();
			



			if(!d_DebugDisableAIThrottle)
				vehicleControls.m_fThrottle		= accel;
			else
				vehicleControls.m_fThrottle		= 0;

			// Extremely bad hack here
			if (m_RollingStartSpeedEnabled_HACK)
			{
				
				IIntroOutroSplineAI *pIntroOutro = pIData->GetIntroOutroSplineAI();
				if (pIntroOutro)
				{
					float sp = MSTOMPH(pIntroOutro->GetSpeed());
					float fzAccel = FuzzyLeftShoulder(pVehicleState->GetSpeedMPH(), Maths::Max(sp-5.0f, 5.0f), sp);
					vehicleControls.m_fThrottle = fzAccel;
				}
				else
				{
					float fzAccel = FuzzyLeftShoulder(pVehicleState->GetSpeedMPH(), 55.0f, 60.0f);
					vehicleControls.m_fThrottle = fzAccel;
				}	
			}


			// be carfeull here....
			SteeringImpluseForceTorqueAI(pIData, pVeh, &vehicleControls, vehicleControls.m_torque, vehicleControls.m_localForce);
			pActor->SendMessageToActor(actorHandle, &vehicleControls);

		}
		else
		{

			// player vehicle....

			// lets stuff the AI with the player controls.....
			if (pVeh->GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_PLAYER) 
			{
				airacingtech::ControlState *pState = pIData->GetControlState()->GetState();
				RacingPhysics::CarControls& controls = ((RacingPhysics::Car*)pVeh)->Controls();
					
				Actor::C_ActorHandle actorHandle = pActor->GetHandle();
				Game::Actors::C_VehicleUpdateAIControlsMsg vehicleControls;
				vehicleControls.m_torque = Maths::HVec3(0.0f,0.0f,0.0f);
				vehicleControls.m_localForce = Maths::HVec3(0.0f,0.0f,0.0f);

				// store the actual input from the controller..... hoping these are the correct values...
				pState->SetBrakeControllerInput(pActor->m_brakeControllerInput);
				pState->SetAccelControllerInput(pActor->m_accelControllerInput);
				pState->SetHandbrakeControllerInput(0.0f);
				if (pActor->m_handbrakeControllerInput == 1.0f)
					pState->SetHandbrakeControllerInput(1.0f);


				float accel = controls.throttle;
				float brake = controls.brake;
				float handbrake = controls.handbrake;
				bool AIBraking = false;
				bool AICornering = false;

				if (g_DebugPlayerUsesSomeAIBrake && pIData->GetState().m_usingNitro == false && pActor->m_brakeAssist == true) 
				{
					float actual_speed=pVehicleState->GetSpeed();
					float track_speed=pVehicleState->GetTrackSpeed();

					//[PK] Added by me as a potential fix to the AI reversing on the line problem, Darren when you're back we'll sort this properly :)
					if(track_speed)
					{
						if (actual_speed >  (track_speed * g_DebugPlayerAIBrakeSpeedPercentage))
						{
							brake = 1.0f;//pState->GetBrake();
							accel = 0.0f;
							AIBraking = true;
						}
					}
				}
				
				if (pIData->GetState().m_usingNitro == true)
				{
					accel = 1.0f;
					brake = 0.0f;
					handbrake = 0.0f;
				}
				else if (pIData->GetState().m_usingCoolDownNitroExtended)
				{	
					brake *= 0.25f; // reduce the brake amount 
				}
				
				
				pState->SetAccel(accel);
				pState->SetBrake(brake);
				if (handbrake == 1.0f)
				{
					pState->SetHandbrake(true);
				}
				else
				{
					pState->SetHandbrake(false);
				}

				vehicleControls.m_fThrottle		= accel;//pCar->Controls().throttle;
				vehicleControls.m_fSteering		= pCar->Controls().steer;
				vehicleControls.m_fPitch		= pCar->Controls().pitch;
				vehicleControls.m_fBrake		= brake;//pCar->Controls().brake;
				if (pCar->Controls().handbrake > 0.0f)
					vehicleControls.m_bHandbrake = true;
				else
					vehicleControls.m_bHandbrake = false;

				if (g_DebugPlayerUsesSomeAIBrake)
				{
					if (AIBraking)
					{	
						BrakeForceAIandPlayer(pIData, pVeh, &vehicleControls, vehicleControls.m_torque, vehicleControls.m_localForce);
					}
				}

				
				if (g_DebugPlayerUsesSomeSteeringForces)
				{
					AICornering = true;

#ifdef _USE_STEERING_ASSIST_CORNERING_BOOST
					SteeringImpluseForceTorqueAIandPlayer(pIData, pVeh, &vehicleControls, vehicleControls.m_torque, vehicleControls.m_localForce);	
#else
					if (Maths::Abs(vehicleControls.m_fSteering) < 0.05f )
					{
						vehicleControls.m_fSteering = -pState->GetSteering() * 0.75f;
						SteeringImpluseForceTorqueAIandPlayer(pIData, pVeh, &vehicleControls, vehicleControls.m_torque, vehicleControls.m_localForce);
					}
#endif				
					
				}


				if (AICornering || AIBraking)
				{
					pActor->SendMessageToActor(actorHandle, &vehicleControls);
				}


			}
		}
	}

	void AmaxAIR::PostUpdate()
	{
		m_pISystemAI->PostUpdate();

		// any game related tasks here, ie rubber banding and usage of nitro, etc, etc.
		//m_pIGameLogic->Update();
		AIGameLogic();

		int noVehicles =  m_cars.Size();
		for(int i=0; i<noVehicles; i++)
		{
			PostUpdateExternal(i);
		}
	}


	void AmaxAIR::PostUpdateControlsOnly()
	{
		int noVehicles =  m_cars.Size();
		for(int i=0; i<noVehicles; i++)
		{
			PostUpdateExternal(i);
		}
	}

	void AmaxAIR::AddInAirForcesAndTorques(airacingtech::IData *pIData, class RacingPhysics::Vehicle *pVeh, Game::Actors::C_VehicleUpdateAIControlsMsg *pInput)
	{
		const float t = m_TimeStep;

		VehicleState *pVehicleState = pIData->GetVehicleState()->GetState();

		if (!pVehicleState->GetWheel(0)->GetOnGround() &&
			!pVehicleState->GetWheel(1)->GetOnGround() &&
			!pVehicleState->GetWheel(2)->GetOnGround() &&
			!pVehicleState->GetWheel(3)->GetOnGround() 
			)
		{
			Maths::HVec3 force(0.0f,-1.0f, 0.0f);

			force *= pVehicleState->GetMass() * g_CarAirForceHack;
			pVeh->GetGravityBody()->AddForce(force * t);

			//Maths::HVec3 angVel = pVeh->GetAngularVelocity();
			Maths::HVec3 angVel = pVeh->AngularAcceleration();

			float ydir = angVel | pVeh->GetAxis().AxisY();
			Maths::HVec3 torque = pVeh->GetAxis().AxisY();
			torque = pVeh->GetAxis().AxisY() * Maths::Abs(ydir) * pVeh->GetGravityBody()->GetBody().Mass() * g_CarTorqueDampHack;
			pVeh->GetGravityBody()->AddTorque(torque * t);

			float zDir = angVel | pVeh->GetAxis().AxisZ();
			torque = pVeh->GetAxis().AxisZ() * Maths::Abs(zDir) * pVeh->GetGravityBody()->GetBody().Mass() * g_CarTorqueDampHack;
			pVeh->GetGravityBody()->AddTorque(torque * t);


			float xDir = angVel | pVeh->GetAxis().AxisX();
			torque = pVeh->GetAxis().AxisX() * Maths::Abs(xDir) * pVeh->GetGravityBody()->GetBody().Mass() * g_CarTorqueDampHack;	
			pVeh->GetGravityBody()->AddTorque(torque * t);
		
		}
		

		/*if (Maths::Abs(pInput->m_fSteering) > 0.0f)
		{
			const Mat33 inverse(pVeh->GetGravityBody()->GetBody().m_OrientationMatrix.InverseRotation());

			//Maths::HVec3 torque = pVeh->GetAxis().AxisY();
			//torque *= (3000000.0f * -pInput->m_fSteering);
			//Maths::HVec3 f = torque * inverse; 
			//pVeh->GetGravityBody()->AddTorque(f * t);


			Maths::HVec3 torque = pVeh->GetAxis().AxisZ();
			torque *= (3000000.0f * pInput->m_fSteering);
			//Maths::HVec3 f = torque * inverse; 
			pVeh->GetGravityBody()->AddTorque(torque * t);

			// -ve angular velocity Z, we go to the right

			// +ve angular velocity Z we go to the left...

			torque = -pVeh->GetAxis().AxisZ();
			torque *= (3000000.0f * pInput->m_fSteering);
			pVeh->GetGravityBody()->AddTorque(torque * t);
			//float zDir = pVeh->GetAngularVelocity() | pVeh->GetAxis().AxisZ();
			//torque *= (19000000.0f * pVeh->GetAngularVelocity().Z());
			//pVeh->GetGravityBody()->AddTorque(torque * t);

		}
		*/
	}

	void AmaxAIR::SteeringImpluseForceTorqueAI(airacingtech::IData *pIData, class RacingPhysics::Vehicle *pVeh, Game::Actors::C_VehicleUpdateAIControlsMsg *pInput,
												Maths::HVec3 &AItorque, Maths::HVec3 &AIlocalforce)			
	{
		// Tuned for the AI
		const float t= m_TimeStep * 0.25f;//0.5f;

		VehicleState *pVehicleState = pIData->GetVehicleState()->GetState();
		IRubberbandingAI *pRubberBanding = pIData->GetRubberBanding();

		// Darren....
		float mass = Maths::Min(pVehicleState->GetMass(), 1700.0f);


		int wheelsOnFloor = 0;
		for(int i=0; i<4; i++)
		{
			if (pVehicleState->GetWheel(i)->GetOnGround())
				wheelsOnFloor++;
		}

		// in air braking
		if (!pVehicleState->GetWheel(0)->GetOnGround() &&
			!pVehicleState->GetWheel(1)->GetOnGround() &&
			!pVehicleState->GetWheel(2)->GetOnGround() &&
			!pVehicleState->GetWheel(3)->GetOnGround() 
			)
		{

			if (pInput->m_fBrake > 0.0f)
			{
				// air braking is probably needed
				Maths::HVec3 offset =  pVeh->GetGravityBody()->GetBody().m_CenterOfMassOffset;
				Maths::HVec3 force(0.0f, 0.0f, -1.0f);
				// 0.3 very good, lets see what 0.4 does.....
				float forwardsSpeed = pVehicleState->GetVelocity() | pVeh->GetAxis().AxisZ();
				forwardsSpeed = Maths::Max(forwardsSpeed, 0.0f);
				force *= 0.6f * mass * forwardsSpeed * pInput->m_fBrake; // 0.4
				force *= t;
				AIlocalforce += force;
			}
			else 
			{
				// air braking is probably needed
				Maths::HVec3 offset =  pVeh->GetGravityBody()->GetBody().m_CenterOfMassOffset;
				Maths::HVec3 force(0.0f, 0.0f, -1.0f);
				// 0.3 very good, lets see what 0.4 does.....
				float forwardsSpeed = pVehicleState->GetVelocity() | pVeh->GetAxis().AxisZ();
				forwardsSpeed = Maths::Max(forwardsSpeed, 0.0f);
				force *= 0.6f * mass * forwardsSpeed * 1.0f; // 0.4
				force *= t;
				AIlocalforce += force;
			}
		}

		


	
		//if (pVehicleState->GetWheel(0)->GetOnGround() &&
		//	pVehicleState->GetWheel(1)->GetOnGround() &&
		//	pVehicleState->GetWheel(2)->GetOnGround() &&
		//	pVehicleState->GetWheel(3)->GetOnGround() 
		//	)

		if (wheelsOnFloor >= 4) // 2
		{

			Maths::HVec3 force;

#if 1
			// apply steering Torque AI
			if (Maths::Abs(pInput->m_fSteering) > 0.01f)
			{
				//TODO : Should make sure we are not tumbling, check angular velocity, maybe in the air also
				if (pVehicleState->GetSpeed() > 5.0f)// 2.0f
				{
					// Ed`s angular velocity is OBVIOUSLY opposite to Gravity.
					Maths::HVec3 torque = pVeh->GetAxis().AxisY();
					// 2.0f, 100 without the speed part...
					float minSpeed = Maths::Max(pVehicleState->GetSpeed(), 20.0f); // 40.0f;
					minSpeed = Maths::Max(minSpeed, 40.0f); // 100.0f
					torque *= g_AiTorqueHack * -pInput->m_fSteering * mass * minSpeed; // 300.0f
					torque *= t;
					AItorque += torque;
				}
			}

			Maths::HVec3 offset =  pVeh->GetGravityBody()->GetBody().m_CenterOfMassOffset;
			const Maths::HMat33 inverse(pVeh->GetGravityBody()->GetBody().m_OrientationMatrix.InverseRotation());
			Maths::HVec3 angVel = pVeh->GetGravityBody()->GetBody().m_AngularVelocity;
			float speed =  pVeh->GetGravityBody()->GetBody().m_LinearVelocity | pVeh->GetAxis().AxisZ();
			float speed1 = Maths::Clamp(speed, 0.0f, 30.0f);
			float cpAccel = (angVel | pVeh->GetAxis().AxisY()) * speed1 * g_AiCorneringBoost;
			force = -pVeh->GetAxis().AxisX() * cpAccel * mass ;
			
			Maths::HVec3 f = (force * inverse) * t;
			AIlocalforce += f;
#endif
		
#ifdef _USE_RUBBER_VELOCITY_
		
			// TODO : Move this to a much better place.....maybe a little holiday in the Alps.
			IRubberbandingAI *pRubberBanding = pIData->GetRubberBanding();
			if (pRubberBanding )
			{
				float targetSpeed = pRubberBanding->GetDirectTargetSpeed();
				if (targetSpeed > 0.0f)
				{
					// we are not using the velocity..........but magic forces instead....
					float speed = pVehicleState->GetSpeed();
					// simple execution here......
					if (speed < targetSpeed)//(targetSpeed - 1.0f))
					{
						// add in a force in the +ve z direction
						Maths::HVec3 forceDir(0.0f, 0.0f, 1.0f);
						float speedIncrease = 25.0f;//75.0f;//75.0f;
						force += (forceDir * (speedIncrease * mass)); //100.0f
						force *= t;	
						AIlocalforce += force;
					}
					else if (speed > targetSpeed)//(targetSpeed + 1.0f))
					{
						// add a force in the -ve z direction
						Maths::HVec3 forceDir(0.0f, 0.0f, -1.0f);
						float speedIncrease = 45.0f; // 175.0f
						force += (forceDir * (speedIncrease * mass)); // 150.0f
						force *= t;	
						AIlocalforce += force;
					}
				}
			}
			
#endif

		}

	}


	void AmaxAIR::BrakeForceAIandPlayer(airacingtech::IData *pIData, class RacingPhysics::Vehicle *pVeh, Game::Actors::C_VehicleUpdateAIControlsMsg *pInput,
											Maths::HVec3 &AItorque, Maths::HVec3 &AIlocalforce)			
	{
		// Tuned for the AI
		const float t= m_TimeStep * 0.25f;//0.5f;

		VehicleState *pVehicleState = pIData->GetVehicleState()->GetState();

		if (pInput->m_fBrake > 0.0f)
		{
			// air braking is probably needed
			Maths::HVec3 offset =  pVeh->GetGravityBody()->GetBody().m_CenterOfMassOffset;
			Maths::HVec3 force(0.0f, 0.0f, -1.0f);
			// 0.3 very good, lets see what 0.4 does.....
			float forwardsSpeed = pVehicleState->GetVelocity() | pVeh->GetAxis().AxisZ();
			forwardsSpeed = Maths::Max(forwardsSpeed, 0.0f);
			force *= g_DebugPlayerAIBrakeValue * pVehicleState->GetMass() * forwardsSpeed * pInput->m_fBrake; // 0.4

			force *= t;
			AIlocalforce += force;
		}
		
	}
	


	void AmaxAIR::SteeringImpluseForceTorqueAIandPlayer(airacingtech::IData *pIData, class RacingPhysics::Vehicle *pVeh, Game::Actors::C_VehicleUpdateAIControlsMsg *pInput,
		Maths::HVec3 &AItorque, Maths::HVec3 &AIlocalforce)			
	{

		// Tuned for the AI
		const float t= m_TimeStep * 0.25f;//0.5f;

		VehicleState *pVehicleState = pIData->GetVehicleState()->GetState();
		

		// in air braking
		if (!pVehicleState->GetWheel(0)->GetOnGround() &&
			!pVehicleState->GetWheel(1)->GetOnGround() &&
			!pVehicleState->GetWheel(2)->GetOnGround() &&
			!pVehicleState->GetWheel(3)->GetOnGround() 
			)
		{

			if (pInput->m_fBrake > 0.0f)
			{
				// air braking is probably needed
				Maths::HVec3 offset =  pVeh->GetGravityBody()->GetBody().m_CenterOfMassOffset;
				Maths::HVec3 force(0.0f, 0.0f, -1.0f);
				// 0.3 very good, lets see what 0.4 does.....
				float forwardsSpeed = pVehicleState->GetVelocity() | pVeh->GetAxis().AxisZ();
				forwardsSpeed = Maths::Max(forwardsSpeed, 0.0f);
				force *= 0.6f * pVehicleState->GetMass() * forwardsSpeed * pInput->m_fBrake; // 0.4



				force *= t;
				AIlocalforce += force;
			}
		}


		if (pVehicleState->GetWheel(0)->GetOnGround() &&
			pVehicleState->GetWheel(1)->GetOnGround() &&
			pVehicleState->GetWheel(2)->GetOnGround() &&
			pVehicleState->GetWheel(3)->GetOnGround() 
			)
		{

			Maths::HVec3 force;


			// apply steering Torque AI
			if (Maths::Abs(pInput->m_fSteering) > 0.01f)
			{
				//TODO : Should make sure we are not tumbling, check angular velocity, maybe in the air also
				if (pVehicleState->GetSpeed() > 5.0f)// 2.0f
				{
					// Ed`s angular velocity is OBVIOUSLY opposite to Gravity.
					Maths::HVec3 torque = pVeh->GetAxis().AxisY();
					// 2.0f, 100 without the speed part...
					float minSpeed = Maths::Max(pVehicleState->GetSpeed(), 20.0f); // 40.0f;
					minSpeed = Maths::Max(minSpeed, 40.0f); // 100.0f
					torque *= g_DebugPlayerAiTorqueHack * -pInput->m_fSteering * pVehicleState->GetMass() * minSpeed; // 300.0f
					torque *= t;
					AItorque += torque;
				}
			}

			Maths::HVec3 offset =  pVeh->GetGravityBody()->GetBody().m_CenterOfMassOffset;
			const Maths::HMat33 inverse(pVeh->GetGravityBody()->GetBody().m_OrientationMatrix.InverseRotation());
			Maths::HVec3 angVel = pVeh->GetGravityBody()->GetBody().m_AngularVelocity;
			float speed =  pVeh->GetGravityBody()->GetBody().m_LinearVelocity | pVeh->GetAxis().AxisZ();
			float speed1 = Maths::Clamp(speed, 0.0f, 30.0f);
			float cpAccel = (angVel | pVeh->GetAxis().AxisY()) * speed1 * g_DebugPlayerAiCorneringBoost;
			force = -pVeh->GetAxis().AxisX() * cpAccel * pVehicleState->GetMass() ;

			Maths::HVec3 ff = (force * inverse);
			Maths::HVec3 f = ff * t;
			AIlocalforce += f;

		}

	}

	void AmaxAIR::FillInAnyRaceLogic(Game::Actors::C_VehicleActor *pActor, int iteration, IData *pIData)
	{
		// Why oh why do I need the level state, surely, everything should be in the racelogiccore ???
		Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
		
		pIData->GetVehicleLogic()->GetLogicState()->SetRaceLaps(levelState.mLaps);

		BIZ_ASSERT(C_GameApp::GetInstance()->GetRaceBookLayer());
		if (!C_GameApp::GetInstance()->GetRaceBookLayer())
			return;

		BIZ_ASSERT(C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer());
		if (!C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer())
			return;

		// Darren : This is how we get the data from my system, when needed.
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(iteration);
		VehiclePositionInfo& positionInfo = pLogicActor->GetPositionInfo();
		VehicleLapsInfo& lapInfo = pLogicActor->GetLapInfo();
		VehiclePickupInfo& pickupInfo = pLogicActor->GetPickupInfo();


		BIZ_ASSERT(pLogicActor);
		if (!pLogicActor)
			return;

		//float speedInMetresPerSecond					= pIData->GetVehicleState()->GetState()->GetSpeed();

		positionInfo.mSpRacePosition			= pIData->GetVehicleLogic()->GetLogicState()->GetRank();
		
		positionInfo.mPrevLogicPosition			= positionInfo.mLogicPosition;
		positionInfo.mLogicPosition				= pIData->GetVehicleLogic()->GetLogicState()->GetLogicPosition();
		positionInfo.mForwardSpeed				= pIData->GetVehicleState()->GetState()->GetSpeedForwardSpeed();
		
		positionInfo.mLogicPositionCombined		= pIData->GetVehicleLogic()->GetLogicState()->GetLogicPositionCombined();

		positionInfo.mGoingWrongWay				= pIData->GetVehicleLogic()->GetLogicState()->m_wrongway;
		positionInfo.mDistanceFromStart			= pIData->GetVehicleLogic()->GetLogicState()->m_DistanceFromStart;

		lapInfo.mCurrentLap						= pIData->GetVehicleLogic()->GetLogicState()->GetLap();
		lapInfo.mCurrentLapTime					= pIData->GetVehicleLogic()->GetLogicState()->GetCurrentLapTime()->m_realLapTime;

		//pickupInfo.mCanFireNitro				= pIData->GetState().m_fireNitro;

		// Darren's stuff is slightly delayed, so this stops us thinking we are 2/1
		if(positionInfo.mSpRacePosition > pLayer->GetNumLiveVehicles())
			positionInfo.mSpRacePosition = pLayer->GetNumLiveVehicles();
		
		if( lapInfo.mBestLapTime != pIData->GetVehicleLogic()->GetLogicState()->GetFastestLapTime()->m_realLapTime )
			lapInfo.mBestLapTime = pIData->GetVehicleLogic()->GetLogicState()->GetFastestLapTime()->m_realLapTime;

		/*
		if(pickupInfo.mCurrentlyUsingType == E_PICKUP_TYPE_NITROUS)
		{
			pIData->GetState().m_usingNitro = true;
		}
		else
		{
			pIData->GetState().m_usingNitro = false;
		}
		*/

		pIData->GetState().m_DoNotUseInPositionCalculation = pActor->IsEliminated();
	}


	void AmaxAIR::FillInVehicleStructure(const class RacingPhysics::Vehicle *pVeh, VehicleState *pVehState)
	{
		// Collision object no longer exists in RacingPhysics.
		//Maths::HVec3 objSize( pVeh->CollisionObject().Width(), pVeh->CollisionObject().Height(), pVeh->CollisionObject().Length());
		Maths::HVec3 objSize(Maths::Constants<Maths::HF32>::Two(), Maths::Constants<Maths::HF32>::One(), Maths::Constants<Maths::HF32>::Four());

		pVehState->SetVehicleSize(objSize);

		Maths::HVec3 pos(pVeh->GetPosition());		
		pVehState->SetPosition(pos);

		Maths::HVec3 xAxis(pVeh->GetAxis().AxisX());
		pVehState->SetXaxis(xAxis);

		Maths::HVec3 yAxis(pVeh->GetAxis().AxisY());
		pVehState->SetYaxis(yAxis);

		Maths::HVec3 zAxis(pVeh->GetAxis().AxisZ()); // Check the sign on this
		pVehState->SetZaxis(zAxis); 

		Maths::HVec3 velocity(pVeh->GetVelocity());
		pVehState->SetVelocity(velocity);

		Maths::HVec3 AngVelocity(pVeh->GetAngularVelocity());
		pVehState->SetAngularVelocity(AngVelocity);

		pVehState->SetGear(pVeh->GetGear() + 1);

		//pVehState->SetPlayer(false);
		//if (pVeh->GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_PLAYER || pVeh->GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_NETWORK)
		//{
		//	// human player controlled.....
		//	pVehState->SetPlayer(true);
		//}
		pVehState->SetPlayer(pVeh->GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_PLAYER || pVeh->GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_NETWORK);

		pVehState->SetActive(true);
		pVehState->SetDriverPresent(true);

		/* // todo ghosts, replays, etc, etc...
		if (gRaceState.mVehicles[pVeh->VehicleIndex()].mVehicleControlType == RaceStateVehicle::CONTROL_TYPE_GHOST || pVeh->ShouldUpdate() == false || !gRaceState.mCollisionsEnabled)
		{
			// not active.....
			pVehState->SetActive(false);
		}
		*/



		// TODO [Darren] fill in the rest of the params	
		{
			const RacingPhysics::Car *pCar = RacingPhysics::Car::GetCarPointer(pVeh);
			assert(pCar);

			pVehState->SetGear(pCar->Transmission().Gear());
			
			airacingtech::AmaxVehicleCarData &cdata = pVehState->GetAmaxCarData();
			cdata.m_AirFriction = pCar->Dynamics().friction.airFriction;
			cdata.m_ConstantDownForce = pCar->Dynamics().friction.constantDownforce;
			cdata.m_DownForceRatioZ = 0.5f;//pCar->Dynamics().friction.downforceRatioZ;
			cdata.m_DownForce = pCar->Dynamics().friction.downforce;
			cdata.m_BackwardGripScale =  (pCar->Wheel(0).BackwardsGripScaleDry() + pCar->Wheel(1).BackwardsGripScaleDry()) * 0.5f;//pCar->Dynamics().tyres.frontTyres.backwardsGripScale;
			cdata.m_FrontGripScale =  (pCar->Wheel(0).ForwardsGripScaleDry() +  pCar->Wheel(1).ForwardsGripScaleDry()) * 0.5f;//pCar->Dynamics().tyres.frontTyres.grip.yScale;
			cdata.m_GripScale = 1.0f;//pCar->Dynamics().tyres.gripScale;
			cdata.m_FrontTransverseGripScale = (pCar->Wheel(0).TransverseGripScaleDry() + pCar->Wheel(1).TransverseGripScaleDry()) * 0.5f;//pCar->Dynamics().tyres.frontTyres.transverseGripScale;
			cdata.m_loadDependence = 0.f;//pCar->Dynamics().tyres.loadDependence;
			cdata.m_RearGripScale = (pCar->Wheel(2).ForwardsGripScaleDry() +  pCar->Wheel(3).ForwardsGripScaleDry()) * 0.5f;//pCar->Dynamics().tyres.rearTyres.grip.yScale;
			cdata.m_RearTransverseGripScale = (pCar->Wheel(2).TransverseGripScaleDry() + pCar->Wheel(3).TransverseGripScaleDry()) * 0.5f;//pCar->Dynamics().tyres.rearTyres.transverseGripScale;
			cdata.m_TransverseGripScale = 1.f;//pCar->Dynamics().tyres.transverseGripScale;
			cdata.m_tyreSlip = pCar->Wheel(0).LongitudinalTyreSlip();//pCar->Dynamics().tyres.rearTyres.transverseTyreSlipScale;
			cdata.m_DownForceFriction = pCar->Dynamics().friction.downforce;

			if (!pVehState->GetHaveSentConstantsOver())
			{
				// vehicle dynamics, some are constants that do not need to be sent over every frame
				int numGears = pCar->Dynamics().gears.topGear + 2;
				float gratio = 0.0f;

				pVehState->SetGearRatios(0, pCar->Dynamics().gears.gearRatios[0]);
				pVehState->SetGearRatios(1, 0.f);

				for(int i=2; i<numGears; i++ )
				{
					gratio = pCar->Dynamics().gears.gearRatios[i - 2];
					pVehState->SetGearRatios(i, gratio);
				}

				float diffRatio = pCar->Dynamics().gears.finalRatio;
				float peakTorque = pCar->Dynamics().engine.engineTorque_Rpm.yScale;
				float maxRevs = pCar->Dynamics().engine.engineTorque_Rpm.x1;
				float revLimit = pCar->Dynamics().engine.limiterRPM;
				float tyreDiametre = pCar->Dynamics().tyres.rearTyres.radius * 2.f;
				float topSpeed = 0.0f;//pCar->TopSpeed();
				//float wheelmass = pCar->Dynamics().general.wheelMass * 4.0f;
				float mass = /*wheelmass +*/ pCar->Dynamics().mass.mass + 75.0f;
				pVehState->SetDiffRatio(diffRatio);
				pVehState->SetMass(mass);
				pVehState->SetPeakTorque(peakTorque);
				pVehState->SetRevLimit(revLimit);
				pVehState->SetMaxRevs(maxRevs);
				pVehState->SetTyreDiameter(tyreDiametre);
				pVehState->SetTopSpeed(topSpeed);
				

				// top speed
				float wheelc = pCar->Wheel(2).Radius() * 2.0f * 3.14f;
				int topGear = numGears - 1;
				float driveshaftRPM = revLimit / gratio;
				float wheelRPM = driveshaftRPM / diffRatio;
				float wheelRPKH = 1000.0f / wheelc;
				float kpm = wheelRPM / wheelRPKH;
				float kph = (kpm * 60.0f) * 0.97f;//* 0.8f; // reduce top speed by 2% // 3%
				float mps = KPHTOMS(kph);
				float mph = MSTOMPH(mps);
				pVehState->SetTopSpeed(mps);

				const float hp = 745.699872f; // 1hp = 745.699872 watts......
				float bhp = (peakTorque  * maxRevs) / 9549; 
				float ptwratio = (bhp * hp)  / mass;
				pVehState->SetPowerWeightRatio(ptwratio);

				

				pVehState->SetWeatherPatchType(VehicleState::NONE);


				pVehState->SetHaveSentConstantsOver(true); // Set flag so we do not re-calc and send above info
			
				// Darren Hack
				pVehState->SetSurfaceFriction(g_AiFrictionHack);
				pVehState->CalculateCarFrictions(80.0f);

				float bgrip = pVehState->GetBrakingFriction();
				float lgrip = pVehState->GetLateralFriction();

				int stip = 0;

			}
			else
			{

				// Darren Hack...
				U32 surfaceT = 0;
				float wheelFrictionScale = 1.0f;

				if (pCar->Wheel(2).OnGround())
				{	
					surfaceT = pCar->Wheel(2).SurfaceType();
					wheelFrictionScale = RacingPhysics::Surfaces::GetGripScale(surfaceT, 0, pVehState->GetSpeed());
				}

				float friction = wheelFrictionScale; 


				Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
				if (levelState.m_overrideAISurface)
				{
					friction = Maths::Clamped(levelState.m_overrideAISurfaceValue, 0.8f, 1.5f);
				}

				pVehState->SetSurfaceFriction(friction);
				pVehState->CalculateCarFrictions(pVehState->GetSpeed());

				//float bgrip = pVehState->GetBrakingFriction();
				//float lgrip = pVehState->GetLateralFriction();

				
			}


			
			

			//TODO [Darren] : Send this over to the vehicle state.
			//float sp = 90.0f;//Maths::Max(Maths::Min(pVehState->GetSpeed(),90.0f), 30.0f); // 
			//float friction = 1.0f;
			//float sp = Maths::Max(90.0f, pVehState->GetSpeed());
			//sp = Maths::Min(pVehState->GetSpeed(), pVehState->GetTrackSpeed());
			//sp = Maths::Min(MPHTOMS(250.0f), sp); //170
			
			

			//pVehState->SetSurfaceFriction(friction);
			//pVehState->CalculateCarFrictions(sp);
			pVehState->SetRevs(pCar->Engine().RPM());
			pVehState->SetVehicleType(0); // default..
			pVehState->SetNoWheels(4);



			//Maths::HVec3 vForce[4];
			//float vForceLength[4];
			//float forceScaleFactor = pCar->Dynamics().mass.mass * 2.0f * (1.0f + pCar->CalculateDownforceFactor(50.f));

			// TODO [Darren] make sure the bikes are handled or, wheels with more than one
			for(int i=0; i<4; i++)
			{
				//vForce[i] = pCar->Wheel(i).GroundForce() / forceScaleFactor;
				//vForceLength[i] = vForce[i].Length();

				Wheels *pwheel = pVehState->GetWheel(i);
				//float angMot = pCar->Wheel(i).AngularMomentum();
				float angRot = pCar->Wheel(i).GetAngularVelocity1D();
				pwheel->SetAngularRotation(angRot);
				pwheel->SetRadius(pCar->Wheel(i).Radius());
				pwheel->SetOnGround(pCar->Wheel(i).OnGround());
				pwheel->SetSlipAngle(pCar->Wheel(i).SlipAngle());
				pwheel->SetSlideFactorAmax(0.f);//pCar->Wheel(i).SlideFactor());
				//float headingAngle = pCar->Wheel(i).Heading();
				//pwheel->SetHeading(headingAngle);
				pwheel->SetDriven( pCar->Wheel(i).Driven());
				pwheel->SetWheelSpin(pCar->Wheel(i).WheelSpin());

				// work out the heading vector......
				if (i==0 || i==1)
				{
					// front....
					//float cs = cos(headingAngle);
					//float sn = sin(headingAngle);
					//Maths::HVec3 headingVec =  (zAxis * cs) + (xAxis * sn);

					HVec3 headingVec(-pCar->Wheel(i).GetRotor().GetRotationMatrix().AxisZ());
					pwheel->SetHeadingVec(headingVec);
					pwheel->SetHeading(1.f - (headingVec | zAxis));
				}
				else
				{
					// rear.....
					pwheel->SetHeadingVec(zAxis);
					pwheel->SetHeading(0.f);
				}

				// TODO [Darren] fill in the rest of the wheel params

				pwheel->SetLongSlip(0.0f);
				pwheel->SetLateralSlip(0.0f);
			}	

		}


	}

	void AmaxAIR::PreUpdateExternal(int i)
	{
		PERF_EVENT("AI.PreUpdateExternal");
		if(m_cars.Size() > i)
		{
			Game::Actors::C_VehicleActor *pActor = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor>(m_cars[i]->m_pVehicleActor);
			class RacingPhysics::Car *pCar = pActor->GetVehiclePhysics();
			class RacingPhysics::Vehicle *pVeh =  static_cast<RacingPhysics::Vehicle*>(pCar);
			IData *pIData = m_pISystemAI->GetData(i);
			VehicleState *pVehicleState = pIData->GetVehicleState()->GetState();

			FillInVehicleStructure(pVeh, pVehicleState);
			FillInAnyRaceLogic(pActor, i, pIData);
		}

#if 0
		float playerDist = AIAmax::AmaxAIR::Instance().ApproxDistanceThroughRace(0);
		float otherDist = AIAmax::AmaxAIR::Instance().ApproxDistanceThroughRace(i);
		float dist = otherDist - playerDist;
		if (Maths::Abs(dist) > 1500.0f)
		{
			LOGDEBUG(g_LoggerAI, "Something might be wrong here\n");
			IData *pIData = m_pISystemAI->GetData(i);
			VehicleState *pVehicleState = pIData->GetVehicleState()->GetState();

			int debugStop = 0;
			int stop = 0;
		}
#endif

	}


	void AmaxAIR::ResetAfterHOG(int index)
	{
		if ( AllOK())
		{
			PreUpdateExternal(index);
			//ResetResetLogicState(index);
			m_pISystemAI->PreUpdate(index);
		}
		
	}

	void AmaxAIR::PostInitialise() // TODO : 
	{
		// tell the ai system to cache pointers if neceassry...	
		if (m_pITrackDataSystem && m_DataInitilaised == true && m_SystemsInitialised == true && m_pISystemAI)
		{
			CMessage msg;
			msg.mType = MT_POST_INIT_AI_SYSTEM;
			msg.mpData = 0;
			m_pObjectManager->BroadcastMessage(msg);

			// run through the physics once so AI has valid data....
			int noVehicles =  m_cars.Size();
			for(int i=0; i<noVehicles; i++)
			{
				const Game::Actors::C_VehicleActor* pActor = m_cars[i]->m_pVehicleActor;
				if (pActor)
				{
					if (pActor->GetVehiclePhysics()->GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_AI)
						AITakeControl(i);
					else
						AIRelinquishControl(i);

					SetIsLocalPlayer(i, pActor->GetVehiclePhysics()->GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_PLAYER);
				}

				PreUpdateExternal(i);
			}	

			GetSubscriberSystem().BroadcastMessage( E_AIMessage_AllLoaded );
		}
		else
		{
			GetSubscriberSystem().BroadcastMessage( E_AIMessage_LoadFailed );
		}
	}

	void AmaxAIR::InitRace()
	{
		if (m_pITrackDataSystem && m_DataInitilaised == true && m_SystemsInitialised == true && m_pISystemAI)
		{
			CMessage msg;
			msg.mType = MT_INIT_RACE;
			msg.mpData = 0;
			m_pObjectManager->BroadcastMessage(msg);

			// we have loaded all data, cached data, post initialisation done, therefore we are ready to rock`n`roll (well it is safe to start simulation)
			m_safeToSimulate = true;
		}
	}

	void AmaxAIR::AIGameLogic()
	{
		int noVehicles =  m_cars.Size();

		for(int i=0; i<noVehicles; i++)
		{
			Game::Actors::C_VehicleActor *pActor = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor>(m_cars[i]->m_pVehicleActor);
			class RacingPhysics::Car *pCar = pActor->GetVehiclePhysics();
			class RacingPhysics::Vehicle *pVeh =  static_cast<RacingPhysics::Vehicle*>(pCar);
			//IData *pIData = m_pISystemAI->GetData(i);
			//VehicleState *pVehicleState = pIData->GetVehicleState()->GetState();

			if (pVeh->GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_AI) 
			{
				// Recovery
				AIGameLogicRecovery(i);
				
				IData *pIData = m_pISystemAI->GetData(i);
				if (pIData->GetSplineFollower())
				{
					if (pIData->GetSplineFollower()->IsAtEndIntroOutroSpline())
					{
						// broadcast message to the 
						AIAtEndOfIntroOutroSpline msg;
						msg.m_pVehicleActor = pActor;
						pActor->BroadcastMessage(&msg);
					}
				}

			}

			BroadCastTrackLogicMessages(i);
		}


		UpdatePerks();

		FireFireMessages();

	}


	void AmaxAIR::FireFireMessages()
	{
		if (m_pIPerkSystem)
		{
			m_pIPerkSystem->FireMessages();
		}
	}

	void AmaxAIR::UpdatePerks()
	{
		//static int PerkStage = 0;
		BIZ_ASSERT(m_pIPerkSystem);

		Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
		m_pIPerkSystem->SetAIPerkUsage(levelState.m_AIDontPickupPickups);

		if (g_DebugDontUsePickups)
		{
			m_pIPerkSystem->SetAIPerkUsage(g_DebugDontUsePickups);
		}

		/*	
		const int incs = 5; // 	
		int startPerk = PerkStage * incs;
		int endPerk = startPerk + incs;
		PerkStage++;
		if ( endPerk >= m_pIPerkSystem->GetNumPerks() )
		{
			endPerk = m_pIPerkSystem->GetNumPerks();
			PerkStage = 0;
		}
		*/
		

		//for(int i=startPerk; i<endPerk; i++)
		for(int i=0; i<m_pIPerkSystem->GetNumPerks(); i++)
		{
			AIAmax::AIPerk &perk = m_pIPerkSystem->GetPerk(i);
			const Actor::C_Actor *pActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetActorPtr(perk.m_actorHandle);
			if (pActor)
			{
				::Actors::PickupPositionActor *pPerkActor = (::Actors::PickupPositionActor*)Actor::ActorCast< ::Actors::PickupPositionActor>(pActor);
				if (pPerkActor)
				{
					perk.m_perkEnabled = pPerkActor->IsEnabled();
					perk.m_perkActive = pPerkActor->IsActive();
				}
			}
		}
	}

	

	void AmaxAIR::BroadCastTrackLogicMessages(int i)
	{
		Game::Actors::C_VehicleActor *pActor = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor>(m_cars[i]->m_pVehicleActor);
		class RacingPhysics::Car *pCar = pActor->GetVehiclePhysics();
		class RacingPhysics::Vehicle *pVeh =  static_cast<RacingPhysics::Vehicle*>(pCar);
		
		IData *pIData = m_pISystemAI->GetData(i);	
		BIZ_ASSERT(pIData);
		
		IVehicleLogic *pIVehicleLogic = pIData->GetVehicleLogic();
		BIZ_ASSERT(pIVehicleLogic);
		
		LogicState *pLogicState = pIVehicleLogic->GetLogicState();
		BIZ_ASSERT(pLogicState);

		if (pLogicState->m_LapIncMessage)
		{
			Game::C_LapIncrementedMsg incLap(pIData->GetState().m_index,
											pLogicState->GetLap(),
											pLogicState->GetLastLapTime()->m_Laptime);

			// What the f@@k is this sh@t
			C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&incLap);
			C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&incLap);

			// Send to UI too
			if( pActor->GetControlType() == ::E_VEHICLE_CONTROL_TYPE_PLAYER )
				_UI_MESSAGE_FULL( incLap );
		}
		else if (pLogicState->m_SplitOneMessage)
		{
			Game::C_SplitCrossedMsg splitOne(pIData->GetState().m_index,1, pLogicState->GetLap());
			// What the f@@k is this sh@t
			C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&splitOne);
			C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&splitOne);

		}
		else if (pLogicState->m_SplitTwoMessage)
		{
			Game::C_SplitCrossedMsg splitTwo(pIData->GetState().m_index,2, pLogicState->GetLap());
			// What the f@@k is this sh@t
			C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&splitTwo);
			C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&splitTwo);

		}


		// TODO : Side swipe message......
		/*
		AIAmax::AISideSwipeMessage aiSideSwipeMessage;
		aiSideSwipeMessage.m_aiDoing = pData->m_indexAIDoingSideSwipe;
		aiSideSwipeMessage.m_aiAgainst = pData->m_indexAIDoingSideSwipeAgainst;
		aiSideSwipeMessage.m_approxDistFromPlayer = pData->m_approxDistFromPlayerAlongTrack;
		C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&aiSideSwipeMessage);	
		*/
		
	}


	void AmaxAIR::AIGameLogicRecovery(int i)
	{
		Game::Actors::C_VehicleActor *pActor = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor>(m_cars[i]->m_pVehicleActor);
		class RacingPhysics::Car *pCar = pActor->GetVehiclePhysics();
		class RacingPhysics::Vehicle *pVeh =  static_cast<RacingPhysics::Vehicle*>(pCar);
		IData *pIData = m_pISystemAI->GetData(i);	
		
		IRecoveryAI *pRecovery = pIData->GetRecovery();
		if (pRecovery && m_pIGameServerTime && m_pIGameServerTime->GetTime() >= pRecovery->GetLastPollTime())
		{
			pRecovery->SetLastPollTime(m_pIGameServerTime->GetTime() + 1.0f);
			// rule has fired, we need to reset the ai vehicle in a valid location
			if (pRecovery->GetRuleFired() > 0)
			{
				airacingtech::ResetInfo *pResetInfo = pRecovery->GetResetInfo();
				if (pResetInfo && pResetInfo->m_pResetRoute && pResetInfo->m_ResetRouteIndex != -1)
				{
					Actor::C_ActorHandle actorHandle = pActor->GetHandle();

					// sh@t maths library....
					Maths::C_Normal up(C_Normal::AxisPositiveY());
					Maths::AxisY yAxis(up);
						
					Maths::HVec3 dir = pResetInfo->m_SplineDirection;
					dir.Normalised();
					
					Maths::C_Normal dirN(dir.X(), dir.Y(), dir.Z());
					Maths::AxisZ zAxis(dirN);

					Maths::HMat33 rot( yAxis, zAxis );
					Maths::HMat43 worldMat( rot, pResetInfo->m_position );
					Maths::HVec3 velocity = zAxis * (pResetInfo->m_speed * 0.8f);

					Game::Actors::C_VehicleResetOnTrack resetMSG(actorHandle);
					pActor->SendMessageToActor(actorHandle, &resetMSG);

					// Todo : Potential bugs here on crossover areas. Reset the internals of the ai tracking 

					// reset the state and the rule
					pRecovery->SetState(1);
				}

				
			}
		}
	}

	// debug functions from here on in
#define DEBUG_DRAW_OPT

#ifdef _MSC_VER
#ifdef DEBUG_AI
#ifdef DEBUG_DRAW_OPT
#pragma optimize("", on)
#endif
#endif
#endif

	Debug::Bool g_debugUseDeferredRenderer("AmaxAI.DebugDraw.UseDeferredRenderer", true, Debug::Persist());

	DbUtil::I_DebugRender* AmaxAIR::DebugRenderer()
	{
		if (g_debugUseDeferredRenderer)
			return DbUtil::DefaultDebugRenderSys()->Deferred();

		return DbUtil::DefaultDebugRenderSys()->Primitive();
	}

	void AmaxAIR::DebugDraw()
	{
		if ( AllOK() )
		{
			DebugDrawTrackData();
			DebugDrawSegment();
			DebugDrawSplineFollower();
#ifndef CFG_GOLD
			if(m_pIModeDestruction)
			{
				m_pIModeDestruction->DebugRender();
			}
#endif 

		}

	}

	void AmaxAIR::DebugDrawTrackData()
	{
		if (m_pITrackDataSystem)
		{
			DebugRenderer()->SetDepthEnable((bool)g_debugAIDebugDrawDepthEnable);

			airacingtech::AITrackData *pAIData = m_pITrackDataSystem->GetTrackData();
			if (pAIData)
			{
				int noRoutes = pAIData->m_Routes.Size();
				for(int i=0; i<noRoutes; i++)
				{
					airacingtech::AITrackDataRoutes *pRoute = pAIData->m_Routes[i];
					if (pRoute)
					{
						if (pRoute->IsLogic() && g_DebugDrawAILogic)
						{
							
							DebugDrawVolume(pRoute, Maths::RGBA(0x00, 0xff, 0x00, 0x00));
							

							DebugDrawWidth(pRoute, AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, AITrackDataLines::TRACK_DATA_EXTREME_LEFT,Maths::RGBA(0x00, 0xff, 0x00, 0xff));
						}


						if (pRoute->IsStart() && g_DebugDrawAILogicStart)
						{

							DebugDrawVolume(pRoute, Maths::RGBA(0x00, 0xff, 0x00, 0x00));


							DebugDrawWidth(pRoute, AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, AITrackDataLines::TRACK_DATA_EXTREME_LEFT,Maths::RGBA(0x00, 0xff, 0x00, 0xff));
						}


						if (pRoute->IsFinish() && g_DebugDrawAILogicFinish)
						{

							DebugDrawVolume(pRoute, Maths::RGBA(0x00, 0xff, 0x00, 0x00));


							DebugDrawWidth(pRoute, AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, AITrackDataLines::TRACK_DATA_EXTREME_LEFT,Maths::RGBA(0x00, 0xff, 0x00, 0xff));
						}

						if (pRoute->IsIntermediate(1) && g_DebugDrawAILogicInter1)
						{

							DebugDrawVolume(pRoute, Maths::RGBA(0x00, 0xff, 0x00, 0x00));


							DebugDrawWidth(pRoute, AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, AITrackDataLines::TRACK_DATA_EXTREME_LEFT,Maths::RGBA(0x00, 0xff, 0x00, 0xff));
						}

						if (pRoute->IsIntermediate(2) && g_DebugDrawAILogicInter2)
						{

							DebugDrawVolume(pRoute, Maths::RGBA(0x00, 0xff, 0x00, 0x00));


							DebugDrawWidth(pRoute, AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, AITrackDataLines::TRACK_DATA_EXTREME_LEFT,Maths::RGBA(0x00, 0xff, 0x00, 0xff));
						}

						if (pRoute->IsJunction() && g_DebugDrawAIJunctions)
						{
							if (pRoute->IsActive())
							{
								DebugDrawVolume(pRoute, Maths::RGBA(0x00, 0xff, 0x00, 0x00));

								DebugDrawWidth(pRoute, AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, AITrackDataLines::TRACK_DATA_EXTREME_LEFT,Maths::RGBA(0x00, 0xff, 0x00, 0xff));
							}
							else
							{
								DebugDrawVolume(pRoute, Maths::RGBA(0xff, 0x00, 0x00, 0x00));

								DebugDrawWidth(pRoute, AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, AITrackDataLines::TRACK_DATA_EXTREME_LEFT,Maths::RGBA(0xff, 0x00, 0x00, 0xff));
							}
							
						}


						// a route we can possibly drive on 
						if (pRoute->IsChangeableRoute() && g_DebugDrawAIRacing)
						{
							
							if (pRoute->IsActive())
							{
								DebugDrawVolume(pRoute, Maths::RGBA(0x00, 0xff, 0x00, 0x00));
							}
							else
							{
								DebugDrawVolume(pRoute, Maths::RGBA(0xff, 0x00, 0x00, 0x00));
							}

							if (g_DebugDrawAIRacingRacing)
							{
								DebugDrawSpline(pRoute, AITrackDataLines::TRACK_DATA_LINE_RACING, Maths::RGBA(0x00, 0x00, 0xff, 0xff));
							}		

							if (g_DebugDrawAIRacingWidth)
							{
								DebugDrawWidth(pRoute, AITrackDataLines::TRACK_DATA_LINE_RIGHT_5, AITrackDataLines::TRACK_DATA_LINE_RACING, Maths::RGBA(0x00, 0xff, 0x00, 0xff));
								DebugDrawWidth(pRoute, AITrackDataLines::TRACK_DATA_LINE_LEFT_5, AITrackDataLines::TRACK_DATA_LINE_RACING,Maths::RGBA(0xff, 0xff, 0x00, 0xff));
							}

							if (g_DebugDrawAIExtremeWidth)
							{
								DebugDrawWidth(pRoute, AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, AITrackDataLines::TRACK_DATA_EXTREME_LEFT,Maths::RGBA(0x00, 0xff, 0x00, 0xff));
							}

							
						}
					}
				}
			}

			DebugRenderer()->SetDepthEnable(true);

		}
	}


	void AmaxAIR::DebugDrawWidth(const airacingtech::AITrackDataRoutes *pRoute, int line1, int line2,  Maths::RGBA colour)
	{
		if (pRoute)
		{
			DebugRenderer()->SetDepthEnable((bool)g_debugAIDebugDrawDepthEnable);

			for (int j=0 ;j<pRoute->m_Segments.Size(); j++)
			{
				Maths::HVec3 lastPosition;
				pRoute->GetPositionFromLine(line1, j, lastPosition);

				Maths::HVec3 currentPosition;
				pRoute->GetPositionFromLine(line2, j, currentPosition);

				DebugRenderer()->Draw3dLine( lastPosition, currentPosition, colour );
			}		

			DebugRenderer()->SetDepthEnable(true);
		}
	}

	void AmaxAIR::DebugDrawSpline(const airacingtech::AITrackDataRoutes *pRoute, int line,  Maths::RGBA colour)
	{
		if (pRoute)
		{
			DebugRenderer()->SetDepthEnable((bool)g_debugAIDebugDrawDepthEnable);

			for (int j=1 ;j<pRoute->m_Segments.Size(); j++)
			{
				Maths::HVec3 lastPosition;
				pRoute->GetPositionFromLine(line, j-1, lastPosition);

				Maths::HVec3 currentPosition;
				pRoute->GetPositionFromLine(line, j, currentPosition);

				
				DebugRenderer()->Draw3dLine( lastPosition, currentPosition, colour );
				
			}		

			DebugRenderer()->SetDepthEnable(true);
		}
	}

	void AmaxAIR::DebugDrawSegment(const airacingtech::RouteInfo *pRouteInfo,  Maths::RGBA colour)
	{
		if (pRouteInfo && pRouteInfo->m_pRoute && pRouteInfo->m_in)
		{
			int index = pRouteInfo->m_index;

			Maths::HVec3 BottomLeft, TopLeft, BottomRight, TopRight;

			DebugRenderer()->SetDepthEnable((bool)g_debugAIDebugDrawDepthEnable);

			if (index >= 1)
			{
				pRouteInfo->m_pRoute->GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, index , TopLeft);
				pRouteInfo->m_pRoute->GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, index-1 , BottomLeft);
				pRouteInfo->m_pRoute->GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, index , TopRight);
				pRouteInfo->m_pRoute->GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, index-1 , BottomRight);
			
				
				DebugRenderer()->Draw3dLine( TopLeft, BottomLeft, colour );
				DebugRenderer()->Draw3dLine( BottomLeft, BottomRight, colour );
				DebugRenderer()->Draw3dLine( BottomRight, TopRight, colour );
				DebugRenderer()->Draw3dLine( TopRight, TopLeft, colour );
			}

			DebugRenderer()->SetDepthEnable(true);

		}
	}


	void AmaxAIR::DebugDrawSegment()
	{
		if (g_DebugDrawAISegment)
		{
			DebugRenderer()->SetDepthEnable((bool)g_debugAIDebugDrawDepthEnable);

			int noVehicles =  m_cars.Size();
			for(int i=0; i<noVehicles; i++)
			{
				airacingtech::IData *pIData = m_pISystemAI->GetData(i);
				if (pIData)
				{
					airacingtech::ISplineTracker *pSplineTracker =  pIData->GetSplineTracker();
					if (pSplineTracker)
					{
						const Btl::Vector<RouteInfo> &routeInfo = pSplineTracker->GetRouteInfo();
						for(int i=0; i<routeInfo.Size(); i++)
						{
							const airacingtech::RouteInfo *pRouteInfo = &routeInfo[i];
							if (pRouteInfo->m_in && pRouteInfo->m_pRoute && g_DebugDrawAISegment)
							{
								DebugDrawSegment(pRouteInfo, Maths::RGBA(0x00, 0x00, 0xff, 0xff));
							}
						}
					}

				}
			}

			DebugRenderer()->SetDepthEnable(true);
		}
		
	}

	void AmaxAIR::DebugDrawSplineFollower()
	{
		if (g_DebugDrawAISplineFollower)
		{
			DebugRenderer()->SetDepthEnable((bool)g_debugAIDebugDrawDepthEnable);

			int noVehicles =  m_cars.Size();
			for(int i=0; i<noVehicles; i++)
			{
				airacingtech::IData *pIData = m_pISystemAI->GetData(i);
				if (pIData)
				{
					airacingtech::ISplineFollower *pSplineFollower =  pIData->GetSplineFollower();
					if (pSplineFollower)
					{
						const Btl::Vector<Maths::HVec3, Btl::VectorParameters_Static< 100 > > &samples = pSplineFollower->GetDebugSpline();

						int sz = samples.Size();
						for(int i=1; i<sz; i++)
						{
							int previous = i-1;
							Maths::HVec3 p1 = samples[previous];
							Maths::HVec3 p2 = samples[i];

							DebugRenderer()->Draw3dLine( p1, p2, Maths::RGBA(0xff, 0x00, 0x00, 0xff) );
							// Debug purposes...
							//float dist = p1.Distance(p2);
						}

					

						int nIndex = pSplineFollower->GetNearestIndex();
						int fIndex = pSplineFollower->GetFurthestIndex();
						if (nIndex > 0)//!= -1)
						{
							Maths::HVec3 p1 = samples[nIndex];
							Maths::HVec3 p2 = samples[fIndex];

							DebugRenderer()->Draw3dLine( p1, p2, Maths::RGBA(0x00, 0x00, 0xff, 0xff) );
						}

					}
				}
			}

			DebugRenderer()->SetDepthEnable(true);

		}
	}

	void AmaxAIR::DebugDrawVolume(airacingtech::AITrackDataRoutes *pRoute, Maths::RGBA colour)
	{	


		if (g_DebugDrawAIVolume)
		{
			assert(pRoute);

			DebugRenderer()->SetDepthEnable(true);

			for (int i=1 ;i<pRoute->m_Segments.Size(); i+=2)
			{
				Maths::HVec3 bottomleft1, bottomright1;
				pRoute->GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, i-1, bottomleft1);
				pRoute->GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, i-1 , bottomright1);
				Maths::HF32 minY1(Maths::UNDEFINED), maxY1(Maths::UNDEFINED);
				pRoute->GetHeightBounds(minY1, maxY1, i-1);
				bottomleft1.Y(minY1);
				bottomright1.Y(minY1);
				Maths::HVec3 topleft1 = bottomleft1;
				Maths::HVec3 topright1 = bottomright1;
				topleft1.Y(maxY1);
				topright1.Y(maxY1);

				Maths::HVec3 bottomleft, bottomright;
				pRoute->GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_LEFT, i, bottomleft);
				pRoute->GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, i , bottomright);
				Maths::HF32 minY(Maths::UNDEFINED), maxY(Maths::UNDEFINED);
				pRoute->GetHeightBounds(minY, maxY, i);
				bottomleft.Y(minY);
				bottomright.Y(minY);
				Maths::HVec3 topleft = bottomleft;
				Maths::HVec3 topright = bottomright;
				topleft.Y(maxY);
				topright.Y(maxY);
				

#if 0			
				// POLYS......
				// bottom
				DebugRenderer()->Draw3dTriangle(bottomleft1, bottomleft, bottomright, colour);
				DebugRenderer()->Draw3dTriangle(bottomright, bottomright1, bottomleft1, colour);

				// top 
				DebugRenderer()->Draw3dTriangle(topleft1, topleft, topright, colour);
				DebugRenderer()->Draw3dTriangle(topright, topright1, topleft1, colour);

				// left side
				DebugRenderer()->Draw3dTriangle(bottomleft1, topleft1, topleft, colour);
				DebugRenderer()->Draw3dTriangle(topleft, bottomleft, bottomleft1, colour);

				// right side
				DebugRenderer()->Draw3dTriangle(bottomright1, topright1, topright, colour);
				DebugRenderer()->Draw3dTriangle(topright, bottomright, bottomright1, colour);


				// being lazy, but lets draw  them in a different winding order also
				
				// bottom
				DebugRenderer()->Draw3dTriangle(bottomright, bottomleft, bottomleft1, colour);
				DebugRenderer()->Draw3dTriangle(bottomleft1, bottomright1, bottomright, colour);

				// top 
				DebugRenderer()->Draw3dTriangle(topright, topleft, topleft1, colour);
				DebugRenderer()->Draw3dTriangle(topleft1, topright1, topright, colour);

				// left side
				DebugRenderer()->Draw3dTriangle(topleft, topleft1, bottomleft1, colour);
				DebugRenderer()->Draw3dTriangle(bottomleft1, bottomleft, topleft, colour);

				// right side
				DebugRenderer()->Draw3dTriangle(topright, topright1, bottomright1, colour);
				DebugRenderer()->Draw3dTriangle(bottomright1, bottomright, topright, colour);
			
#else

				DebugRenderer()->Draw3dLine( bottomright, bottomleft, colour );
				//DebugRenderer()->Draw3dLine( bottomleft, bottomleft1, colour );
				//DebugRenderer()->Draw3dLine( bottomleft1, bottomright1, colour );
				//DebugRenderer()->Draw3dLine( bottomright1, bottomright, colour );

				// top
				DebugRenderer()->Draw3dLine( topright, topleft, colour );
				//DebugRenderer()->Draw3dLine( topleft, topleft1, colour );
				//DebugRenderer()->Draw3dLine( topleft1, topright1, colour );
				//DebugRenderer()->Draw3dLine( topright1, topright, colour );

				
				// sides
				DebugRenderer()->Draw3dLine( bottomleft, topleft, colour );
				//DebugRenderer()->Draw3dLine( bottomleft1, topleft1, colour );
				//DebugRenderer()->Draw3dLine( bottomright1, topright1, colour );
				DebugRenderer()->Draw3dLine( bottomright, topright, colour );

#endif
			
			}
			DebugRenderer()->SetDepthEnable((bool)g_debugAIDebugDrawDepthEnable);
		}
	}

	
}



#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif



