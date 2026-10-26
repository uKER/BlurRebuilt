#include "stdafx.h"

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <Game\GameActors\Components\VehicleLogic\VLC_Damage.h>
#include <Game\GameActors\Components\VehicleLogic\HandofGod\VLC_HandOfGod.h>
#include <Game\GameActors\Components\VehicleLogic\HandofGod\VLC_AIController.h>
#include <Game\GameActors\Components\VehicleLogic\HandofGod\VLC_SuperControlController.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_E3Controller.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_Network_Damage.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_MP_Domination.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_MP_Local_Pickups.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_MP_Network_Pickups.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_MP_Pickups_AI.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_MultiplayerChallenges.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_MP_IdleKickController.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_PlayerRubberBanding.h>
#include <Game\GameActors\Components\VehicleLogic\Scoring\VLC_Statistics.h>
#include <Game\GameActors\Components\VehicleLogic\Scoring\VLC_PointsSystem.h>
#include <Game\GameActors\Components\VehicleLogic\Scoring\VLC_NewScoring.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups_AI.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups_Targetting.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Drafting.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Rumble.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_FanDemands.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Sensor.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Abilities.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Network_Abilities.h>

#include <GameLogic\Pickups\PickupMineBase.h>
#include <GameLogic\Pickups\PickupsManager.h>
#include <GameLogic\StickerManager.h>
#include <Ui\game\Multiplayer\MultiplayerDefines.h>
#include <UI/UIHelpers.h>

#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

// All these are need to access the racelogiccore... Must be a better way? 
#include <GameApp.h>
#include <Game/Game.h>
#include <GameLogic/ActorStudioActors/CutSceneMessages.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <Game/GameWorld/GameWorld.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Game/GameActors/Crowds/CrowdGameActor.h>
#include <GameLogic/InGameDialogueManager/InGameDialogueManager.h>

#include <GameLogic\Structures\MpResults.h>

#include "vehicle/input.h"
#include "Automation\GameDataLogging.h"

#include <Ui/Game/Gadgets/ButtonMessages.h>
#include <Ui\UiManager.h>

#include <ai/AI_Amax.h>

#include <Game\Messaging\PickupActorMessages.h>
#include <Game\GameActors\Components\GameComponents.h>

#include <GameLogic\GameData\GameData.h>
#include <algorithm>

#ifndef CFG_GOLD
#include <GameLogic/Replays/ReplayDirector.h>
#endif // #ifndef CFG_GOLD

//Debug::Bool g_debugBankRepairPerk("RLC.Pickups.BankRepair", false);
Debug::Float g_RecentlyHitByPerkTime("Vehicle.Logic.Scoring.RecentlyHitByPerkTime", 3.f, Debug::Persist());
Debug::Float g_RecentlyOvertakenReportTime("Vehicle.Logic.RecentlyOvertakenReportTime", 2.f, Debug::Persist());

Debug::Int g_debugFrontAngle("Vehicle.Components.AIController.FrontAngle", 120, Debug::Persist());
Debug::Int g_debugFrontDistance("Vehicle.Components.AIController.FrontDistance", 250, Debug::Persist());
Debug::Int g_debugRearDistance("Vehicle.Components.AIController.RearDistance", 100, Debug::Persist());

Debug::Bool g_debugShowLogicActorDebugPoints("Vehicle.ShowLogicActorDebugPoints", false);
Debug::Bool g_debugGhostedText("Vehicle.GhostedText", false);

Debug::Bool g_debugUpdateLogicalGhostiness("Vehicle.Ghost.LogicActor.UpdateGhostiness", false);
Debug::Bool g_debugCarLogicalGhost("Vehicle.Ghost.LogicActor.Ghost", false);
Debug::Int g_debugVehicleIndexToLogicalGhost("Vehicle.Ghost.LogicActor.IndexToGhost", 0, Debug::Range(0, 19), Debug::Step(1));
Debug::Float g_LogicalGhostedAlpha("Vehicle.Ghost.LogicActor.GhostedAlpha", 0.6f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_LogicalGhostTransitionTime("Vehicle.Ghost.LogicActor.GhostTime", 5.f, Debug::Range(0.0f, 50.0f), Debug::Step(0.1f));

Debug::Bool g_GhostForceEndOfRaceClearUp("Vehicle.Ghost.ForceEndOfRaceClearUp", false);
Debug::Bool g_GhostForceEndOfRaceSilentRunningDed("Vehicle.Ghost.SilentRunningDed", false);
Debug::Int g_GhostForceEndOfRaceClearUpVehicleIndex("Vehicle.Ghost.EndOfRaceClearUpIndex", 0, Debug::Range(0, 19));

Debug::Float g_debugInvunerableWhenPerkedTimer("Perks.InvunerableWhenPerkedTimer",3.0f);
Debug::Bool  g_DebugEnableUnghostPopping("Vehicle.UnGhostPopping",true, Debug::Persist());
Debug::Float g_DebugUnghostUpForce("Vehicle.UnGhostUpForce",750000.0f, Debug::Persist());
Debug::Float g_DebugUnghostBackForce("Vehicle.UnGhostBackForce",0.0f, Debug::Persist());
Debug::Float g_DebugUnghostForceTimer("Vehicle.UnGhostForceTimer",5.0f, Debug::Persist());

extern Debug::Float g_debugShuntAngleBias;
extern Debug::Float g_debugShuntDistBias;
//extern Debug::Float g_debugShuntPositionBias;
extern Debug::Float g_debugShuntMinRange;
extern Debug::Float g_debugShuntMaxRange;
extern Debug::Float g_ShuntConeAngle;
extern Debug::Float g_ShuntInitialConeAngle;
extern Debug::Float g_debugShuntLOSBias;
extern Debug::Float g_debugProjectileAngle;

Debug::Float g_debugBullBarsAngleBias("Perks.BullBarsAngleBias",0.5f);
Debug::Float g_debugBullBarsDistBias("Perks.BullBarsDistBias",0.5f);

Debug::Float g_debugEliminatedHideTimer("Vehicle.EliminatedHideTimer", 5.0f);
Debug::Float g_debugMineMissedRadius("Perks.Mine.MissedRadius", 6.0f);
Debug::Float g_debugMineMissedDot("Perks.Mine.MissedDot", 0.7f);

Debug::Float g_debugNitrousCoolDownSpeed("Perks.Nitrous.CoolDownSpeed", 0.2f);
Debug::Float g_debugShuntForcePointOffset2("Perks.Shunt.ForcePointOffset2", 10.f);
Debug::Bool g_debugShuntDrawTrail("Perks.Shunt.DrawTrail", false);

Debug::Bool g_OverrideShuntTargetting("Perks.Shunt.OverrideShuntTargetting", false);
Debug::Int g_ShuntTargetVehicleIndex("Perks.Shunt.TargetVehicleIndex", 1, Debug::Range(0, 19));

Debug::Float g_DefaultGhostAlpha("Vehicle.DefaultGhostAlpha", 0.5f);

Debug::Bool g_AbilitiesUseNetworkVersion("Vehicle.Components.Abilities.UseNetworkVersion", false);

extern Debug::Float g_debugCheckCutOffLength;
extern Debug::Float g_debugBargeMinXDistance;
extern Debug::Float g_debugBargeZOffset;
extern Debug::Bool g_debugShowBargeDebug;
extern Debug::Float g_debugBargeWidthModifier;
extern Debug::Float g_debugBargeHeightModifier;
extern Debug::Float g_debugBargeUpperY;
extern Debug::Float g_debugBargeLowerY;
extern Debug::Float g_ShuntInitialConeAngleLowRange;
extern Debug::Float g_ShuntTargettingCutOffRange;

Debug::Logger &g_VehicleLogicActorLogger( Debug::GetLogger("Vehicle.LogicActor") );
extern Debug::Logger &g_ShuntInfoLogger;

extern Debug::Bool g_AddSuperControl;
extern Debug::Bool g_SuperControlDynamicOverride;
extern Debug::Int g_SuperControlVehicleIndex;


//#pragma optimize("", off)

namespace Game
{

namespace Actors
{
const float C_VehicleLogicActor::EFFECTIVELY_INFINITE_GHOST_TIME = 1E+37;

const float NOT_A_GOOD_TARGET_AT_ALL = 2.f;

extern Debug::Bool g_debugEnableFatalDamageCam;

C_VehicleLogicActor::C_VehicleLogicActor()
{
	mStillInRace	= true;
	mLeftRace		= false;
	mRemoveWhenWrecked = false;
	mAlwaysShields = false;

	mDoubleShield = false;
	mSuperShield = false;

	m_pRLC			= NULL;
	
	pPickupComponent = NULL;
	pPickupAIComponent = NULL;
	pAIComponent = NULL;
	pPickupTargettingComponent = NULL;
	pHOGComponent = NULL;
	pStatisticsComponent = NULL;
	pDraftingComponent = NULL;
	pRumbleComponent = NULL;
	pDamageComponent = NULL;
	pFanDemandComponent = NULL;
	
	pSensorComponent = NULL;

	pNewScoringComponent = NULL;
	pAbilitiesComponent = NULL;
	pSuperControlControllerComponent = NULL;

	pMPChallengesComponent = NULL;

	pSSPlayerRubberBandingComponent = NULL;
	pSSNetworkDamageComponent = NULL;
	pSSRumbleComponent = NULL;
	pSSMPPickupAIComponent = NULL;
	pSSPickupTargettingComponent = NULL;
	pSSHandOfGodComponent = NULL;
	pSSStatisticsComponent = NULL;
	pSSDamageComponent = NULL;
	pSSDraftingComponent = NULL;
	pSSPointsSystemComponent = NULL;
	pMPIdleKickControllerComponent = NULL;

	m_SnailTrailPosition = Maths::HVec3(0.0f,0.0f,0.0f);
	m_TrailNo = 0;
	mGraphicalActorRemovedFromRace = false;
	mPickupsAllowed= true;
	mLastPickupIdCollided = -1;

	mForceTargetVehicleEvaluation = false;

	// Stop vehicles getting sorted that are not in the race
	for( int i = 0; i < mShuntEvaluations.Size(); i++ )
	{
		mShuntEvaluations[i].mScore = NOT_A_GOOD_TARGET_AT_ALL;
	}
	mNumShuntsChasingInECMRange = 0;
	mECMWarningActive = false;
}

C_VehicleLogicActor::~C_VehicleLogicActor()
{
}

// -------------------------------------------------------------------------------------------------

JQ_DECLARE_PPU_MODULE(DeferredVehicleLogicUpdateTask, Game::Actors::DeferredVehicleLogicUpdate);

void DeferredVehicleLogicUpdate(Scheduler::Task* pTask)
{
	PERF_EVENT("VehicleLogicUpdateTask");
	C_VehicleLogicActor *p = *(C_VehicleLogicActor**)pTask->ParamData;
	p->DeferredUpdate();
}

void C_VehicleLogicActor::DeferredUpdate()
{
	PERF_EVENT("C_VehicleLogicActor.DeferredUpdate");

	CalculateVehicleProximities();

	// We can force the evaluation if we can't wait until it's our turn
	bool evaluate = mForceTargetVehicleEvaluation;
	if(mRaceLogicLayerPtr->CanEvaluateVehicle(mVehicleIndex))
	{
		evaluate = true;
		mRaceLogicLayerPtr->IncrementEvaluationVehicle();
	}

	if(evaluate)
	{
		mForceTargetVehicleEvaluation = false;
		CalculateVehicleTargetIndex();
	}
}

// [CharlieB] its more expensive to add the task than it is to run it on main thread...
Debug::Bool g_bDeferredVehicleLogicUpdate("Vehicle.DeferredLogicUpdate", false);

void C_VehicleLogicActor::PreUpdate()
{
	PERF_EVENT("C_VehicleLogicActor.PreUpdate");

	C_RaceBookLayer* pRaceBookLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	C_GameModeBase* gameModeBase = pRaceBookLayer->GetGameModePtr();
	if(gameModeBase)
	{
		if(!gameModeBase->IsMultiplayer())
		{
			// This gets set in the RaceLogicLayer as it doesn't use Darren's AI tracking stuff
			if(pRaceBookLayer->GetRaceMode() != E_RaceMode_FanDemand && pRaceBookLayer->GetRaceMode() != E_RaceMode_FriendDemand_FanDemand)
			{
				mPositionInfo.mRacePosition = mPositionInfo.mSpRacePosition;
			}
		}
		else
		{
			mPositionInfo.mRacePosition = Game::C_MpResults::Instance().GetPlayerPosition(mVehicleIndex);
		}
	}

	C_VehicleActor* pVehicleActor = GetGraphicalActor(mVehicleIndex);
	if( pVehicleActor )
	{
		const Maths::HMat43& worldMatrix	= pVehicleActor->GetMatrix();
		Maths::HVec3 old_pos=mPositionInfo.mWorldPosition;
		Maths::HVec3 new_pos=worldMatrix.AW();

		float old_height=mPositionInfo.mWorldPosition.Y();
		float new_height=worldMatrix.AW().Y();

		old_pos.Y(0.f);
		new_pos.Y(0.f);

		mPositionInfo.mDistanceTravelledSinceLastUpdate=old_pos.Distance(new_pos);
		mPositionInfo.mHeightTravelledSinceLastUpdate=old_height-new_height;

		mPositionInfo.mWorldPosition		= worldMatrix.AW();
		mPositionInfo.mDirection			= worldMatrix.AZ();

		if (g_bDeferredVehicleLogicUpdate)
		{
			C_VehicleLogicActor *p = this;
			BIZ_ASSERT(m_hSchedulerGroup != 0); // make sure you set the scheduler group before running the update
			Scheduler::AddTask(Game::Actors::DeferredVehicleLogicUpdateTask, 0, 0, m_hSchedulerGroup, 0, &p, sizeof(p));
			m_TasksFlushed = false;
		}
		else
		{
			DeferredUpdate();
		}

		Game::Actors::RaceLogicCore* pRLC = GetRLC();
		mPositionInfo.mInRangeOfHuman = false;
		if(pRLC->IsVehicleAIControlled(mVehicleIndex))
		{
			for( U32 i = 0; i < pRLC->GetNumVehicles(); i++ )
			{
				if( !pRLC->IsVehicleAIControlled(i) && CheckInVisionRangeOfOtherVehicle(i) )
				{
					mPositionInfo.mInRangeOfHuman = true;
					break;
				}
			}
		}

		//RacingPhysics::Car*	vehPhysics = GetGraphicalActor(mVehicleIndex)->GetVehiclePhysics();
		//bool wheelsOnGround = (vehPhysics->NumWheelsOnGround() == vehPhysics->GetNumWheels());
		//if(wheelsOnGround)
			//CreateSnailTrails(worldMatrix);// snail trails.....
	}

	for( int i = 0; i < mRaceLogicLayerPtr->GetNumVehicles(); i++ )
	{
		if(mOvertakenTimers[i] > 0)
			mOvertakenTimers[i] -= C_GameApp::GetInstance()->GetTimeStep();
	}

	// Reset targetting
	mTargetLockedOn = E_PICKUP_LOCK_ON_NONE;

	// Check overtaking
	// Note that only the overtaking vehicle deals with telling the world about it.
	if( mPositionInfo.mSpRacePosition < mPositionInfo.mPreviousSpRacePosition )
	{
		C_VehicleLogicActor* otherVeh = NULL;
		for( int i = 0; i < mRaceLogicLayerPtr->GetNumVehicles(); i++ )
		{
			if( i != mVehicleIndex ) 
			{
				otherVeh = mRaceLogicLayerPtr->GetLogicVehicleActor(i);

				if( otherVeh->mPositionInfo.mPreviousSpRacePosition >= mPositionInfo.mSpRacePosition && 
					mPositionInfo.mPreviousSpRacePosition > otherVeh->mPositionInfo.mPreviousSpRacePosition )
				{
					if( mOvertakenTimers[i] <= 0 )
					{
						C_VehicleOvertakeMsg overtakeMsg( mVehicleIndex, i );
						overtakeMsg.mOvertakerRacePosition=mPositionInfo.mRacePosition;

						GetActorList()->SendMessageToActor( GetHandle(), &overtakeMsg);
						GetActorList()->SendMessageToActor( otherVeh->GetHandle(), &overtakeMsg);

						// Notify driver actor.
						C_DriverManager::Instance()->OnOvertake(mVehicleIndex, i, mPositionInfo.mRacePosition);

						// Notify crowd
						if (Game::C_CrowdGameManager::Instance())
							Game::C_CrowdGameManager::Instance()->OnOvertake(mVehicleIndex, i, mPositionInfo.mRacePosition);

						if(C_InGameDialogueManager::Instance().IsEnabled())
							C_InGameDialogueManager::Instance().MessageHandler(&overtakeMsg);

						if( C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[mVehicleIndex].mControlType == ::E_VEHICLE_CONTROL_TYPE_PLAYER )
							_UI_MESSAGE_FULL( overtakeMsg );
					}
					
					// So we do not keep reporting overtakes when we keep vying for a position.
					mOvertakenTimers[i] = g_RecentlyOvertakenReportTime;
				}
			}
		}
	}	

	C_PickupsManager::Instance().CheckVehicleTargetted(mVehicleIndex);
}



void C_VehicleLogicActor::CreateSnailTrails(const Maths::HMat43& worldMatrix)
{
	if (m_TrailNo < NO_TRAILS - 1)
	{
		if (m_TrailNo < 1)
		{
			m_snailTrailPositions[m_TrailNo] = worldMatrix.AW();
			m_TrailNo++;
		}
		else
		{
			Maths::HVec3 dir = worldMatrix.AW() - m_snailTrailPositions[m_TrailNo-1];
			if (dir.LengthSquared() > 1.0f)
			{
				dir.Normalise();
				m_snailTrailPositions[m_TrailNo] = m_snailTrailPositions[m_TrailNo-1] + dir;//worldMatrix.AW();
				m_TrailNo++;
			}
		}

	}
	else
	{
		// drop at 1 metre intervals only....
		Maths::HVec3 dir = worldMatrix.AW() - m_snailTrailPositions[m_TrailNo];
		Maths::HVec3 lastPos = m_snailTrailPositions[m_TrailNo];
		if (dir.LengthSquared() > 1.0f)
		{
			memcpy( m_snailTrailPositions+0, m_snailTrailPositions+1, sizeof(m_snailTrailPositions[0])*(NO_TRAILS-1));
			m_snailTrailPositions[m_TrailNo] = worldMatrix.AW();
		}
	}

	if (g_debugShuntDrawTrail)
	{
		for(int i=0; i<NO_TRAILS; i++)
		{
			// debug draw all the positions here
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(m_snailTrailPositions[i], 1.0f, Maths::RGBA(255,255,0));
		}
	}

	m_SnailTrailPosition = m_snailTrailPositions[0];
}


void C_VehicleLogicActor::Update()
{
	if (!m_TasksFlushed)
	{
		PERF_EVENT("LogicActor::Flush");
		Scheduler::Flush(Game::Actors::C_VehicleLogicActor::GetSchedulerGroup());
		m_TasksFlushed = true;
	}

	PERF_EVENT("C_VehicleLogicActor::Update");

	Game::Actors::C_VehicleActor *pActor = GetGraphicalActor(mVehicleIndex);
	float frameTime	= C_GameApp::GetInstance()->GetTimeStep();

	if(pActor && !pActor->IsEnabled())
	{
		return;
	}

	Game::Actors::RaceLogicCore* pRLC = GetRLC();
	//const bool aiControlled = GetRLC()->IsVehicleAIControlled(mVehicleIndex);

	BIZ_ASSERT_MSG(pRLC, "Race Logic Core is NULL and we are about to use it");

	//	Used to remove vehicles which have left the race prematurely (only applicable in network MP modes)
	if(mLeftRace && !mGraphicalActorRemovedFromRace)
	{
		mHideTimer -= frameTime; 
		if(mHideTimer <= 0.0f)
		{
			GetGraphicalActor(mVehicleIndex)->RemoveFromRace();
			mGraphicalActorRemovedFromRace = true; 
		}
	}

	if(pActor && pActor->GetVehicleInput())
	{
		float speedInMetresPerSecond = abs( Magnitude( pActor->GetVehicleInput()->m_vVelocity ) );

		mPositionInfo.mSpeedInMph				= (speedInMetresPerSecond * 3600.f) / 1609.f;
		mPositionInfo.mPreviousRacePosition		= mPositionInfo.mRacePosition;
		mPositionInfo.mPreviousSpRacePosition	= mPositionInfo.mSpRacePosition;

		if (mVehicleIndex == 0)
			SP_BossDemands::Instance().RegisterBossDemandAction(E_BOSSDEMAND_ACTION_SPEED, mPositionInfo.mSpeedInMph);
	}

	if(g_debugShowLogicActorDebugPoints)
	{
		for(int a=0;a<mDebugPoints.Size();a++)
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(mDebugPoints[a].mPos, 0.25f, mDebugPoints[a].mColour);
		}
	}

	if( g_debugGhostedText && mGhosted && ( !mVehicleIndex || mVehicleProximities[0].mDistanceSquared < g_debugCheckCutOffLength ) )
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(GetGraphicalActor(mVehicleIndex)->GetMatrix().AW()+Maths::HVec3(0,0.5f,0),Util::C_String().Printf("GHOSTED"));
	}

	RecordAwardStats(frameTime); 

	if(mGhosted && mGhostTime > 0)
	{
		mGhostTime -= frameTime;

		if(mGhostTime < 0)
			mGhostTime = 0;
	}

	if(mGhostForceTimer > 0.0f)
		mGhostForceTimer -= frameTime;

	if(mGhosted && mGhostTime <= 0)
		Deghost();

	if(mInvunTimer)
	{
		mInvunTimer-=frameTime;

		if(mInvunTimer < 0.f)
		{
			mInvunTimer=0.f;
		}
	}

	if(mPickupInfo.mTimeSinceTouchedLastPickup >= 0.f)
		mPickupInfo.mTimeSinceTouchedLastPickup += frameTime;

	if( g_debugGhostedText && mInvunTimer && ( !mVehicleIndex || mVehicleProximities[0].mDistanceSquared < g_debugCheckCutOffLength ) )
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(GetGraphicalActor(mVehicleIndex)->GetMatrix().AW()+Maths::HVec3(0,0.5f,0),Util::C_String().Printf("INVULNERABLE"));
	}

	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
#ifndef CFG_GOLD

	if (g_debugUpdateLogicalGhostiness)
	{
		if (mVehicleIndex == g_debugVehicleIndexToLogicalGhost)
		{
			if (g_debugCarLogicalGhost)
			{
				//Ghosting
				Ghost(g_LogicalGhostTransitionTime, g_LogicalGhostedAlpha);
			}
			else
			{
				//Deghosting
				Deghost();
			}

			g_debugUpdateLogicalGhostiness = false;
		}
	}

	if(g_SuperControlVehicleIndex == mVehicleIndex && g_SuperControlDynamicOverride)
	{
		if(g_AddSuperControl)
		{
			if(!pSuperControlControllerComponent)
			{
				AddSuperControlControllerComponent();
				BIZ_ASSERT_MSG(pSuperControlControllerComponent, "VehicleLogicActor::Update() > Failed to add SuperControlComponent :-(");
			}
		}
		else if(pSuperControlControllerComponent)
		{
			RemoveSuperControlControllerComponent();
			BIZ_ASSERT_MSG(!pSuperControlControllerComponent, "VehicleLogicActor::Update() > Failed to remove SuperControlComponent :-(");
		}
	}

	if(g_GhostForceEndOfRaceClearUp && g_GhostForceEndOfRaceClearUpVehicleIndex == mVehicleIndex)
	{
		ClearUpSilentRunningSFXForEndOfRace();

		g_GhostForceEndOfRaceClearUp = false;
	}

	Game::E_ReplayMode replayState = levelState.GetReplayMode();
	if(replayState == Game::E_ReplayMode_Play)
	{
		pPickupComponent->Update();
		pSSNetworkDamageComponent->Update();
		if(pFanDemandComponent)
			pFanDemandComponent->Update();
	}
	else	
#endif // #ifndef CFG_GOLD

	{
	switch(mGameMode)
	{
		case Game::E_GameMode_Debug:	//	Deliberate fall-through
		case Game::E_GameMode_SinglePlayer:
			{
				if( pSensorComponent && !GetRLC()->IsVehicleAIControlled(mVehicleIndex) )
				{
					pSensorComponent->UpdateFirstPass();
				}

				pPickupComponent->Update();
				
				if( pPickupAIComponent && pAIComponent && GetRLC()->IsVehicleAIControlled(mVehicleIndex) )
				{
					pPickupAIComponent->Update();
					pAIComponent->Update();
				}

				if( pPickupTargettingComponent && GetRLC()->IsVehiclePlayerControlled( mVehicleIndex ) )
				{
					pPickupTargettingComponent->Update();
				}

				pHOGComponent->Update();
				pStatisticsComponent->Update();

				if(pNewScoringComponent)
					pNewScoringComponent->Update();
				
				if(pDraftingComponent)
					pDraftingComponent->Update();
				
				if( pRumbleComponent && !GetRLC()->IsVehicleAIControlled(mVehicleIndex) )
				{
					pRumbleComponent->Update();
				}

				if( pFanDemandComponent && !GetRLC()->IsVehicleAIControlled(mVehicleIndex) )
				{
					pFanDemandComponent->Update();
				}

				if( pAbilitiesComponent )
				{
					pAbilitiesComponent->Update();
				}

				pDamageComponent->Update();

				if( pSensorComponent && !GetRLC()->IsVehicleAIControlled(mVehicleIndex) )
				{
					pSensorComponent->UpdateSecondPass();
				}
			}
			break;

		// TODO_MP_REFACTOR
		case Game::E_GameMode_Online:
		case Game::E_GameMode_SystemLink:
		case Game::E_GameMode_SplitScreen:
			{
				pPickupComponent->Update();

				if(pSensorComponent)
					pSensorComponent->Update();

				if(pNewScoringComponent)
					pNewScoringComponent->Update();

				if(pSSPlayerRubberBandingComponent)
				{
					pSSPlayerRubberBandingComponent->Update();
				}

				if(levelState.mCars[mVehicleIndex].mControlType == E_VEHICLE_CONTROL_TYPE_NETWORK)
				{
					pSSNetworkDamageComponent->Update();
				}
				else
				{
					if(pSSRumbleComponent && !GetRLC()->IsVehicleAIControlled(mVehicleIndex))
					{
						pSSRumbleComponent->Update();
					}

					pSSMPPickupAIComponent->Update();

					if( pSSPickupTargettingComponent && GetRLC()->IsVehiclePlayerControlled( mVehicleIndex ) )
					{
						pSSPickupTargettingComponent->Update();
					}

					pSSHandOfGodComponent->Update();
					pSSStatisticsComponent->Update();
					pSSDamageComponent->Update();

					if(pSSDraftingComponent)
					{
						pSSDraftingComponent->Update();
					}
				}

				if( pAbilitiesComponent )
				{
					pAbilitiesComponent->Update();
				}
			}
			break;
	}
	}

	Components::C_VehicleComponent_SuperControlController* pSCC = (Components::C_VehicleComponent_SuperControlController*)QueryInterface(Components::E_IF_VehicleComponent_SuperControlController);
	if(pSCC)
	{
		pSCC->Update();
	}

	if(pSensorComponent)
	{
		PERF_EVENT("C_VehicleLogicActor.Update.NearMissSensor");
		AmaxScoringConfig* params = C_GameData::Instance().mScoringConfig[0];

		float frameTime	= C_GameApp::GetInstance()->GetTimeStep();

		//if we have the sensor component then we can score a near-miss
		int numProximalEntities = pSensorComponent->GetMaxProximalEntities();
		for(int i=0;i<numProximalEntities;i++)
		{
			// We are only interested in entities which are no longer in proximity
			// but were for a short period of time. 
			const Kanzen::C_VLC_Sensor::proximalEntity& entity = pSensorComponent->GetProximalEntityAtIndex(i);
			if( entity.IsInUse()) 
			{
				F32 timeSinceLastInProximity = entity.GetTimeSinceLastInProximity();
				F32 timeInProximity = entity.GetMaxTimeInProximity();
				bool timeOutOfProximityCondition = (timeSinceLastInProximity >= params->mRegisterNearMissDelay) && ((timeSinceLastInProximity - frameTime) < params->mRegisterNearMissDelay);
				bool massCondition = entity.IsStaticGeometry() || (entity.GetTargetBody() && entity.GetTargetBody()->GetBody().Mass() > params->mMinMass);
				
				if(timeOutOfProximityCondition && massCondition)
				{
					bool timeInProximityCondition = timeInProximity > params->mMinTimeInProximity &&	timeInProximity < params->mMaxTimeInProximity;
					bool timeBeingTrackedCondition = entity.GetTimeBeingTracked() < params->mMaxTrackingTime;
					bool hasCollided = entity.HasCollidedWithinTime(params->mMaxTrackingTime);

					if(timeInProximityCondition && timeBeingTrackedCondition && !hasCollided)
					{
						if( entity.GetMinimumDistance() <= params->mMinDistanceMetres )
						{
							if( entity.GetClosestProximityData().relativeVelocity.LengthSquared() > MPHTOMS(params->mMinClosingSpeedMPH * params->mMinClosingSpeedMPH))
							{
								if( entity.GetClosestProximityData().myVelocity.LengthSquared() > MPHTOMS(params->mMinSpeedMPH * params->mMinSpeedMPH ))
								{
									int vehicle_index=-1;
									if(entity.IsActor())
									{
										Actor::C_ActorHandle hndl=entity.GetTargetActorHandle();
										const Actor::C_Actor* pActor = this->GetActorPtr( hndl );
										if ( pActor && ( pActor->GetActorTypeId() == Game::E_ActorTypeId_Vehicle ) )
										{
											Game::Actors::C_VehicleActor *pVehicleActor = 0;
											pVehicleActor = (Game::Actors::C_VehicleActor*)Actor::ActorCast< Game::Actors::C_VehicleActor>(pActor);

											vehicle_index=pVehicleActor->GetVehicleIndex();

											Game::Actors::C_VehicleLogicActor *pVehicleLogicActor=mRaceLogicLayerPtr->GetLogicVehicleActor(vehicle_index);
											if(pVehicleLogicActor && (pVehicleLogicActor->IsWreckedOrEliminated() || pVehicleLogicActor->IsGhosted() || pVehicleLogicActor->IsSecretlyGhosted() || !pVehicleLogicActor->GetStillInRace()))
											{
												continue;
											}
										}
									}
									
									if(pNewScoringComponent)
										pNewScoringComponent->NearMiss(vehicle_index);
								}
							}
						}
					}
				}
			}
		}	
	}
}
// -------------------------------------------------------------------------------------------------
void C_VehicleLogicActor::RecordAwardStats(const float frameTime)
{
	// If this vehicle is cloaked, record the time cloaked
	if( IsCloaked( ))
	{
		mStatistics.mTimeStealthed  += frameTime;
	}

	if( mPositionInfo.mSpRacePosition == 1 )
	{
		// If the vehicle is in first place, record the time in first place
		mStatistics.mTimeInFirstPlace += frameTime; 
	}
}

// -------------------------------------------------------------------------------------------------
void C_VehicleLogicActor::ModifyEcmWarningCount(const bool giveWarning)
{
	// Update stats
	if(giveWarning)
	{
		mNumShuntsChasingInECMRange++;
	}
	else
	{
		mNumShuntsChasingInECMRange--;
	}

	BIZ_ASSERT_MSG(mNumShuntsChasingInECMRange >= 0, "UpdateEcmWarningCount(const bool giveWarning) < mNumShuntsChasingInECMRange is < 0.... reference counting has gone wrong!");

	// Update display if required. 
	UpdateECMEvadeWarningDisplay(); 
}

// -------------------------------------------------------------------------------------------------
void C_VehicleLogicActor::UpdateECMEvadeWarningDisplay()
{
	// Are we currently displaying an ECM warning?
	if(!mECMWarningActive)
	{
		// If not then we are interested in knowing when the warning should be turned on!
		if( mNumShuntsChasingInECMRange > 0 )
		{
			// TODO: Activate here!
			SetECMEvadeUIStatus(true);
			mECMWarningActive = true;
		}
	}
	else // else if ECM warning active
	{
		// If so then we are interested in knowing when the warning should be turned off!
		if( mNumShuntsChasingInECMRange == 0 )
		{
			// TODO: Deactivate here!
			SetECMEvadeUIStatus(false);
			mECMWarningActive = false;
		}
	}
}


// --------------------------------------------------------------------------------

void C_VehicleLogicActor::SetECMEvadeUIStatus(const bool active)
{
	// Tell the UI to do its thang..
	S32 playerIndex = UI::GetLocalPlayerIndex(mVehicleIndex); // Make sure this is a player, not an AI...
	if(playerIndex >= 0)	
	{
		if(active)
		{
			_UI_MESSAGE1( UI::E_UiGameMsg_DisplayECMWarning, playerIndex );
		}
		else
		{
			_UI_MESSAGE1( UI::E_UiGameMsg_RemoveECMWarning, playerIndex );
		}
	}
}
// -------------------------------------------------------------------------------------------------

C_VehicleActor* C_VehicleLogicActor::GetGraphicalActor(int index) const
{
	if (!m_pRLC)
		return NULL;

	return m_pRLC->GetVehicleActor(index);
}

Game::Actors::RaceLogicCore* C_VehicleLogicActor::GetRLC() const
{
	return m_pRLC;
}

// -------------------------------------------------------------------------------------------------

bool C_VehicleLogicActor::CreateImpl()
{
	const C_VehicleLogicActorCreate * pVehicleLogicCreate = (const C_VehicleLogicActorCreate *)(GetCreationData().GetData());

	//AddSubscriber( Game::E_VehicleMessage_Loaded );
	AddSubscriber( Game::E_GameflowMessage_RaceLoaded );
	AddSubscriber( Game::E_GameflowMessage_RestartLevel );
	AddSubscriber( Game::E_GameflowMessage_StopGameUpdate );
	AddSubscriber( Game::E_VehicleMessage_LeftRace );

	
	return true;
}

void C_VehicleLogicActor::AddSubscribers()
{
	AddSubscriber( GameMessaging::E_GameMsg_Update );
	AddSubscriber( GameMessaging::E_GameMsg_PreUpdate );
	AddSubscriber( Game::E_VehicleMessage_FatallyDamaged );
	AddSubscriber( Game::E_VehicleMessage_ResetAfterWrecked );
	AddSubscriber( Game::E_VehicleMessage_Eliminated );
	AddSubscriber( E_VehicleLogicMsg_HitByPerk );
	AddSubscriber( E_VehicleLogicMsg_ToggleAiControl );
	AddSubscriber( E_VehicleLogicMsg_TogglePlayerControl );
	AddSubscriber( Game::E_GameflowMessage_ClearStoredActorPtrs );
}

void C_VehicleLogicActor::RemoveSubscribers()
{
	RemoveSubscriber( GameMessaging::E_GameMsg_Update );
	RemoveSubscriber( GameMessaging::E_GameMsg_PreUpdate );
	RemoveSubscriber( Game::E_VehicleMessage_FatallyDamaged );
	RemoveSubscriber( Game::E_VehicleMessage_ResetAfterWrecked );
	RemoveSubscriber( Game::E_VehicleMessage_Eliminated );
	RemoveSubscriber( E_VehicleLogicMsg_HitByPerk );
	RemoveSubscriber( E_VehicleLogicMsg_ToggleAiControl );
	RemoveSubscriber( E_VehicleLogicMsg_TogglePlayerControl );
	//RemoveSubscriber( Game::E_GameflowMessage_ClearStoredActorPtrs );
}

// -------------------------------------------------------------------------------------------------

bool C_VehicleLogicActor::InitialiseImpl()

{
	const C_VehicleLogicActorCreate * pVehicleLogicCreate = (const C_VehicleLogicActorCreate *)(GetCreationData().GetData());

	m_pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

	mVehicleIndex = pVehicleLogicCreate->mVehicleIndex;
	mRaceLogicLayerPtr = pVehicleLogicCreate->mRaceLogicLayerPtr;
	mPickupsAllowed = pVehicleLogicCreate->mPickupsAllowed;

	mShieldTime = 0.f;
	mShieldHealth = 0.f;
	mLastShieldId = 0;

	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	Game::LevelStateMultiplayerSettings& multiplayerSettings = levelState.GetMultiplayerSettings();

	AmaxVehicle *av = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)levelState.mCars[mVehicleIndex].mCarID];
	mVehicleStrength = av ? av->m_sUiStrength - 1 : 0;

	// Should never happen, one would hope.
	if(mVehicleStrength < 0)
		mVehicleStrength = 0;

	// We do this here now as someone isn't putting the damage component on a vehicle...
	CalcMaxHealth();
	ResetHealth();

	mGameMode=C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode();

#ifndef CFG_GOLD

	Game::E_ReplayMode replayState = C_Game::GetInstance()->GetLevelState().GetReplayMode();
	if(replayState == Game::E_ReplayMode_Play)
	{
		//	Only network vehicles (i.e. vehicles not based locally) need these components
		{
			Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_MP_Network_Damage_CreateInfo> pCreator;
			pCreator->mParentActorHandle=this->GetHandle();
			pCreator->mVehicleIndex=mVehicleIndex;

			pSSNetworkDamageComponent=(Components::C_VehicleComponent_MP_Network_Damage *)InitialiseComponent( pCreator.Get() );
			if( pSSNetworkDamageComponent == NULL )
				return false;
		}

		{

			Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_MP_Network_Pickups_CreateInfo> pCreator;
			pCreator->mVehicleIndex=mVehicleIndex;
			pCreator->mRaceLogicLayerPtr=mRaceLogicLayerPtr;

			pPickupComponent=(Components::C_VehicleComponent_MP_Network_Pickups *)InitialiseComponent( pCreator.Get() );
			if( pPickupComponent == NULL )
				return false;
		}

		if(mVehicleIndex == 0)//	Don't judge me.
		{
			{
				Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_Fan_Demand_CreateInfo> pCreator;
				pCreator->mVehicleIndex=mVehicleIndex;
				pCreator->mRaceLogicLayerPtr=mRaceLogicLayerPtr;

				pFanDemandComponent=(Components::C_VehicleComponent_Fan_Demand *)InitialiseComponent( pCreator.Get() );
				if( pFanDemandComponent == NULL )
					return false;
			}
		}

	}
	else

#endif // #ifndef CFG_GOLD

	{
	switch(mGameMode)
	{
	case Game::E_GameMode_Debug:	//	Deliberate fall-through
	case Game::E_GameMode_SinglePlayer:
		{
			{
				Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_Pickup_CreateInfo> pCreator;
				pCreator->mVehicleIndex=mVehicleIndex;
				pCreator->mRaceLogicLayerPtr=mRaceLogicLayerPtr;

				pPickupComponent=(Components::C_VehicleComponent_Pickups *)InitialiseComponent( pCreator.Get() );
				if( pPickupComponent == NULL )
					return false;
			}

			if( GetRLC()->IsVehicleAIControlled(mVehicleIndex) )
			{
				Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_Pickup_AI_CreateInfo> pCreator;
				pCreator->mParentActorHandle=this->GetHandle();
				pCreator->mVehicleIndex=mVehicleIndex;
				pCreator->mpRaceLogicLayer=mRaceLogicLayerPtr;

				pPickupAIComponent=(Components::C_VehicleComponent_Pickups_AI *)InitialiseComponent( pCreator.Get() );
					if( pPickupAIComponent == NULL )
						return false;
			}

			/*if( GetRLC()->IsVehicleAIControlled(mVehicleIndex) )
			{
				Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_AIController_CreateInfo> pCreator;
				pCreator->mParentActorHandle=this->GetHandle();
				pCreator->mVehicleIndex=mVehicleIndex;

				pAIComponent=(Components::C_VehicleComponent_AIController *)InitialiseComponent( pCreator.Get() );
					if( pAIComponent == NULL )
						return false;
			}
			*/

			if( GetRLC()->IsVehiclePlayerControlled( mVehicleIndex ) )
			{
				Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_Pickup_Targetting_CreateInfo> pCreator;
				pCreator->mParentActorHandle=this->GetHandle();
				pCreator->mVehicleIndex=mVehicleIndex;
				pCreator->mpRaceLogicLayer=mRaceLogicLayerPtr;

				pPickupTargettingComponent=(Components::C_VehicleComponent_Pickups_Targetting *)InitialiseComponent( pCreator.Get() );
					if( pPickupTargettingComponent == NULL )
						return false;
			}

			{
				Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_HandOfGod_CreateInfo> pCreator;
				pCreator->mParentActorHandle=this->GetHandle();
				pCreator->mVehicleIndex=mVehicleIndex;
				pCreator->mViewportIndex=GetRLC()->GetViewportIndex(mVehicleIndex); //Dependency injection for the win!!
				pCreator->mContinueDespiteNoSplineData = false;
				pCreator->mpHoGParams = RLDatabase::Instance().mRLSystems[0]->mHandOfGodParameters.Object();
	
				pHOGComponent=(Components::C_VehicleComponent_HandOfGod *)InitialiseComponent( pCreator.Get() );
					if( pHOGComponent == NULL )
						return false;
			}

			{
				Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_Statistics_CreateInfo> pCreator;
				pCreator->mVehicleIndex=mVehicleIndex;
				pCreator->mpRaceLogicLayer=mRaceLogicLayerPtr;
				pCreator->mpStatistics=&GetStatistics();

				pStatisticsComponent=(Components::C_VehicleComponent_Statistics *)InitialiseComponent( pCreator.Get() );
					if( pStatisticsComponent == NULL )
						return false;
			}

			{
				Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_NewScoring_CreateInfo> pCreator;
				pCreator->mVehicleIndex=mVehicleIndex;
				pCreator->mpRaceLogicLayer=mRaceLogicLayerPtr;
				pCreator->mpScoringInfo = &GetScoring();

				pNewScoringComponent=(Components::C_VehicleComponent_NewScoring *)InitialiseComponent( pCreator.Get() );
				if( pNewScoringComponent == NULL )
					return false;
			}

			if( !GetRLC()->IsVehicleAIControlled(mVehicleIndex) )
			{
				{
					Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_Rumble_CreateInfo> pCreator;
					pCreator->mVehicleIndex=mVehicleIndex;
					pCreator->mRaceLogicLayerPtr=mRaceLogicLayerPtr;

					pRumbleComponent=(Components::C_VehicleComponent_Rumble *)InitialiseComponent( pCreator.Get() );
					if( pRumbleComponent == NULL )
						return false;
				}

				{
					Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_Fan_Demand_CreateInfo> pCreator;
					pCreator->mVehicleIndex=mVehicleIndex;
					pCreator->mRaceLogicLayerPtr=mRaceLogicLayerPtr;

					pFanDemandComponent=(Components::C_VehicleComponent_Fan_Demand *)InitialiseComponent( pCreator.Get() );
					if( pFanDemandComponent == NULL )
						return false;
				}

				{
					Component::C_ComponentCreatorFactoryHelper<Kanzen::I_VLC_Sensor_CreateInfo> pCreator;
					pCreator->m_VehicleIndex=mVehicleIndex;
					pCreator->m_ParentActorHandle=this->GetHandle();

					pSensorComponent=(Kanzen::C_VLC_Sensor *)InitialiseComponent( pCreator.Get() );
					if( pSensorComponent == NULL )
						return false;
				}


				if( !AddAbilitiesComponent(levelState) )
					return false;
			}

			{
				Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_Damage_CreateInfo> pCreator;
				pCreator->mParentActorHandle=this->GetHandle();
				pCreator->mVehicleIndex=mVehicleIndex;

				pDamageComponent=(Components::C_VehicleComponent_Damage *)InitialiseComponent( pCreator.Get() );
					if( pDamageComponent == NULL )
						return false;
			}
		}
		break;

	case Game::E_GameMode_Online:
		{
			if(levelState.mCars[mVehicleIndex].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER) 
			{
				Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
				if(pLayer->GetGameModePtr()->IsRanked())
				{
					Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_MP_Challenges_CreateInfo> pCreator;
					pCreator->mParentActorHandle=this->GetHandle();
					pCreator->mVehicleIndex=mVehicleIndex;
					pCreator->mRaceLogicLayerPtr=mRaceLogicLayerPtr;
					pCreator->mRaceBookLayerPtr=C_GameApp::GetInstance()->GetRaceBookLayer();
					pCreator->m_pRLC=GetRLC();
					pCreator->m_pLogicActor=this;

					pMPChallengesComponent=(Components::C_VehicleComponent_MP_Challenges *)InitialiseComponent( pCreator.Get() );
					if( pMPChallengesComponent == NULL )
						return false;
				}

				// Always need the idle kick component in online MP games user created or not!
				Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_MP_IdleKickController_CreateInfo> pCreator;
				pCreator->mVehicleIndex=mVehicleIndex;
				pCreator->mRaceLogicLayerPtr=mRaceLogicLayerPtr;

				pMPIdleKickControllerComponent=(Components::C_VehicleComponent_MP_IdleKickController *)InitialiseComponent( pCreator.Get() );
				if( pMPIdleKickControllerComponent == NULL )
				{
					return false;
				}
			}
		} //deliberate fall-through, we do not want the challenges component in split screen games [mtaylor]
	case Game::E_GameMode_SystemLink:
	case Game::E_GameMode_SplitScreen:	//	[GavinY] - Split screen shouldn't really be using the network MP components but it works so I'll leave this for now...
		{
			//	All vehicles in Multiplayer need these components:

			if(levelState.mCars[mVehicleIndex].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER
				|| ((levelState.mCars[mVehicleIndex].mControlType == E_VEHICLE_CONTROL_TYPE_NETWORK) && levelState.mCars[mVehicleIndex].mOwnerNetJoinId == 0)) // add the component to network vehicles but only those that are really players
			{
				// only split screen needs player rubber banding and then only if the if the player's selected the option on the UI
				if( multiplayerSettings.mHandicap )
				{
					Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_PlayerRubberBanding_CreateInfo> pCreator;
					pCreator->mVehicleIndex=mVehicleIndex;
					pCreator->mParentActorHandle=this->GetHandle();
					pCreator->mLevelState=&levelState;
					pCreator->mRaceLogicLayer=mRaceLogicLayerPtr;
					pCreator->mRLC=GetRLC();

					pSSPlayerRubberBandingComponent=(Components::C_VehicleComponent_PlayerRubberBanding *)InitialiseComponent( pCreator.Get() );
					if( pSSPlayerRubberBandingComponent == NULL )
						return false;
				}
			}

			if(levelState.mCars[mVehicleIndex].mControlType == E_VEHICLE_CONTROL_TYPE_NETWORK)
			{
				//	Only network vehicles (i.e. vehicles not based locally) need these components
				{
					Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_MP_Network_Damage_CreateInfo> pCreator;
					pCreator->mParentActorHandle=this->GetHandle();
					pCreator->mVehicleIndex=mVehicleIndex;

					pSSNetworkDamageComponent=(Components::C_VehicleComponent_MP_Network_Damage *)InitialiseComponent( pCreator.Get() );
					if( pSSNetworkDamageComponent == NULL )
						return false;
				}

				{

					Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_MP_Network_Pickups_CreateInfo> pCreator;
					pCreator->mVehicleIndex=mVehicleIndex;
					pCreator->mRaceLogicLayerPtr=mRaceLogicLayerPtr;

					pPickupComponent=(Components::C_VehicleComponent_MP_Network_Pickups *)InitialiseComponent( pCreator.Get() );
					if( pPickupComponent == NULL )
						return false;
				}

				if(!AddNetworkAbilitiesComponent(levelState))
					return false;
			}
			else
			{
				if(!GetRLC()->IsVehicleAIControlled(mVehicleIndex))
				{
					//	Only player-controlled vehicles need these components
					{
						Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_Rumble_CreateInfo> pCreator;
						pCreator->mVehicleIndex=mVehicleIndex;
						pCreator->mRaceLogicLayerPtr=mRaceLogicLayerPtr;

						pSSRumbleComponent=(Components::C_VehicleComponent_Rumble *)InitialiseComponent( pCreator.Get() );
						if( pSSRumbleComponent == NULL )
							return false;
					}

					{
						Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_Pickup_Targetting_CreateInfo> pCreator;
						pCreator->mParentActorHandle=this->GetHandle();
						pCreator->mVehicleIndex=mVehicleIndex;
						pCreator->mpRaceLogicLayer=mRaceLogicLayerPtr;

						pSSPickupTargettingComponent=(Components::C_VehicleComponent_Pickups_Targetting *)InitialiseComponent( pCreator.Get() );
						if( pSSPickupTargettingComponent == NULL )
							return false;
					}

					{
						Component::C_ComponentCreatorFactoryHelper<Kanzen::I_VLC_Sensor_CreateInfo> pCreator;
						pCreator->m_VehicleIndex=mVehicleIndex;
						pCreator->m_ParentActorHandle=this->GetHandle();

						pSensorComponent=(Kanzen::C_VLC_Sensor *)InitialiseComponent( pCreator.Get() );
							if( pSensorComponent == NULL )
								return false;
					}

					//Only in MotorGash and only for humans
					if( levelState.mRaceMode == E_RaceMode_Multiplayer_Destruction)
					{
						AddSuperControlControllerComponent(false);
					}
				}

				//	All local vehicles (players or AI) need these components in Multiplayer
				{
					Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_MP_Local_Pickups_CreateInfo> pCreator;
					pCreator->mVehicleIndex=mVehicleIndex;
					pCreator->mRaceLogicLayerPtr=mRaceLogicLayerPtr;

					pPickupComponent=(Components::C_VehicleComponent_MP_Local_Pickups *)InitialiseComponent( pCreator.Get() );
					if( pPickupComponent == NULL )
						return false;
				}

				{
					Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_MP_Pickup_AI_CreateInfo> pCreator;
					pCreator->mParentActorHandle=this->GetHandle();
					pCreator->mVehicleIndex=mVehicleIndex;
					pCreator->mpRaceLogicLayer=mRaceLogicLayerPtr;

					pSSMPPickupAIComponent=(Components::C_VehicleComponent_MP_Pickups_AI *)InitialiseComponent( pCreator.Get() );
					if( pSSMPPickupAIComponent == NULL )
						return false;
				}

				{
					Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_HandOfGod_CreateInfo> pCreator;
					pCreator->mParentActorHandle=this->GetHandle();
					pCreator->mVehicleIndex=mVehicleIndex;
					pCreator->mViewportIndex=GetRLC()->GetViewportIndex(mVehicleIndex); //Dependency injection for the win!!
					pCreator->mContinueDespiteNoSplineData = !( AIAmax::AmaxAIR::Instance().IsDataInitialised() );
					if(levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Destruction)
					{
						pCreator->mpHoGParams = RLDatabase::Instance().mRLSystems[0]->mHandOfGodMotormashParameters.Object();
					}
					else
					{
						pCreator->mpHoGParams = RLDatabase::Instance().mRLSystems[0]->mHandOfGodParameters.Object();
					}
				
					pSSHandOfGodComponent=(Components::C_VehicleComponent_HandOfGod *)InitialiseComponent( pCreator.Get() );
					if( pSSHandOfGodComponent == NULL )
						return false;
				}

				{
					Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_Statistics_CreateInfo> pCreator;
					pCreator->mVehicleIndex=mVehicleIndex;
					pCreator->mpRaceLogicLayer=mRaceLogicLayerPtr;
					pCreator->mpStatistics = &GetStatistics();

					pSSStatisticsComponent=(Components::C_VehicleComponent_Statistics *)InitialiseComponent( pCreator.Get() );
					if( pSSStatisticsComponent == NULL )
						return false;
				}

				{
					Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_Damage_CreateInfo> pCreator;
					pCreator->mParentActorHandle=this->GetHandle();
					pCreator->mVehicleIndex=mVehicleIndex;

					pSSDamageComponent=(Components::C_VehicleComponent_Damage *)InitialiseComponent( pCreator.Get() );
					if( pSSDamageComponent == NULL )
						return false;
				}

				{
					//	Even though only Player vehicles accrue score, AI vehicles also need a scoring component so they award
					//	scoring events for wrecking a vehicle, etc
					Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_NewScoring_CreateInfo> pCreator;
					pCreator->mVehicleIndex=mVehicleIndex;
					pCreator->mpRaceLogicLayer=mRaceLogicLayerPtr;
					pCreator->mpScoringInfo = &GetScoring();

					pNewScoringComponent=(Components::C_VehicleComponent_NewScoring *)InitialiseComponent( pCreator.Get() );
						if( pNewScoringComponent == NULL )
							return false;
				}

				{
					Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_PointsSystem_CreateInfo> pCreator;
					pCreator->mParentActorHandle=this->GetHandle();
					pCreator->mVehicleIndex=mVehicleIndex;
					pCreator->mpRaceLogicLayer=mRaceLogicLayerPtr;

					pSSPointsSystemComponent=(Components::C_VehicleComponent_PointsSystem *)InitialiseComponent( pCreator.Get() );
					if( pSSPointsSystemComponent == NULL )
						return false;
				}

				if( !AddAbilitiesComponent(levelState) )
					return false;
			}
		}
		break;
	}
	}

	mTargetLockedOn = E_PICKUP_LOCK_ON_NONE;

	mFatallyDamaged = mGhosted = mSecretGhosting = mCloaked = mTemporaryAIControl = m_ToggledToAiControl = mGhostPoppingActive = mAwayFromBody = false;

	//clear out the pickup stuff
	mPickupInfo.mPickupStore.Clear();
	mPickupInfo.mCurrentlySelectedType = mPickupInfo.mCurrentlyUsingType = E_PICKUP_TYPE_NONE;
	mPickupInfo.mCharging = mPickupInfo.mUsingNitrousReverse = false;
	mPickupInfo.mPickupStoreIndex = -1;
	mPickupInfo.mCurrentlyUsingLevel=-1;
	mPickupInfo.mTimeSinceTouchedLastPickup = -1.f;

	mLapsInfo.mCurrentLap = 0;
	mLapsInfo.mCurrentLapTime = mLapsInfo.mBestLapTime = -1.f;

	mPositionInfo.mLogicPosition = mPositionInfo.mSpeedInMph = mPositionInfo.mForwardSpeed = 0.f;
	mPositionInfo.mRacePosition = mPositionInfo.mSpRacePosition = mPositionInfo.mCurrentRaceRuleArea = mPositionInfo.mLastRaceRuleArea = 
	mPositionInfo.mPreviousRacePosition = mPositionInfo.mPreviousSpRacePosition = -1;

	mPositionInfo.mGoingWrongWay = mPositionInfo.mInRangeOfHuman = false;

	for( int i = 0; i < mVehicleProximities.Size(); i++ )
	{
		mVehicleProximities[i].mRelativeHeading = 0;
		mVehicleProximities[i].mDistanceSquared = 0;
	}

	for( int i = 0; i < mOvertakenTimers.Size(); i++ )
		mOvertakenTimers[i] = 0.f;

	mInvunTimer=0.f;
	mDraftingAmount=0.f;

	// [JBUNNER] BUG: 30163 - The memset zero call originally used to reset scoring info, overrides any values set in the constructor....
	mScoringInfo.Reset();

	mIsInLastGaspMode = false;

	// Setup ECM warning data.. used by ECM mod AND used all the time in split screen
	mNumShuntsChasingInECMRange = 0;		
	mECMWarningActive			= false;
	
	return true;
}

float C_VehicleLogicActor::GetTimeTillLastGaspForceFire() const
{
	// Get damage component and return the time remaining till last gasp force fire
	const Components::C_VehicleComponent_Damage* pDamage = (Components::C_VehicleComponent_Damage*)QueryInterface(Components::E_IF_VehicleComponent_Damage);
	if(pDamage)
	{
		return pDamage->GetTimeTillLastGaspForceFire();
	}
	else
	{
		return 0.0f;
	}
}

void C_VehicleLogicActor::FireLastGasp()
{
	Components::C_VehicleComponent_Damage* pDamage = (Components::C_VehicleComponent_Damage*)QueryInterface(Components::E_IF_VehicleComponent_Damage);
	if(pDamage)
	{
		pDamage->FireLastGasp();
	}
}

float C_VehicleLogicActor::GetWreckedSequenceDuration() const
{
	bool playerAlsoHasLastGasp = false;

	Game::C_LevelState& levelState	= Game::C_Game::GetInstance()->GetLevelState();	
	Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = levelState.mCars[mVehicleIndex].mUpgrades.GetMultiplayerAbilities();
	Game::C_MultiplayerVehicleAbility* pLastGaspMod = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Last_Gasp);
	Game::C_MultiplayerVehicleAbility* pJumpTheGunMod = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Jump_The_Gun);

	// Default to standard wrecked sequence duration...
	MiscModSettings& modSettings = C_MultiplayerData::Instance().m_pConfig->m_MiscModSettings;
	LastGaspModSettings& lastGaspModSettings = C_MultiplayerData::Instance().m_pConfig->m_LastGaspModSettings;
	float wreckedSequenceDuration = m_pRLC->GetRLSystem()->mDamageSystem->mParameters->mResetTimerAfterWrecked;
	if( pJumpTheGunMod && pJumpTheGunMod->GetActive())
	{	
		if(pLastGaspMod && pLastGaspMod->GetActive())
		{
			// If we have the last gasp mod AND the JTG mod.. 
			wreckedSequenceDuration = (wreckedSequenceDuration * lastGaspModSettings.m_LastGaspJumpTheGunCombinedWreckedSequenceReductionMultiplier);
		}
		else
		{
			// Just the JTG mod == accellerated wrecked sequence!
			wreckedSequenceDuration = (wreckedSequenceDuration * modSettings.m_JumpTheGunWreckedSequenceReductionMultiplier);
		}
	}
	else
	{
		// If just the last gasp mod.. we will delay the wrecked sequence.. and show the accellerated wrecked sequence
		if(pLastGaspMod && pLastGaspMod->GetActive())
		{
			wreckedSequenceDuration = (wreckedSequenceDuration * lastGaspModSettings.m_LastGaspWreckedSequenceReductionMultiplier); 
		}
	}

	return wreckedSequenceDuration;
}

const bool C_VehicleLogicActor::AddAbilitiesComponent(Game::C_LevelState& levelState)
{
	if(g_AbilitiesUseNetworkVersion)
		return AddNetworkAbilitiesComponent(levelState);

	C_VehicleActor* pGraphicalActor = GetGraphicalActor(mVehicleIndex);
	BIZ_ASSERT_MSG(pGraphicalActor, "C_VehicleLogicActor::AddAbilitiesComponent() > Couldn't get a pointer to the graphical actor, this is going to go badly :(");

	Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_Abilities_CreateInfo> pCreator;
	pCreator->mVehicleIndex = mVehicleIndex;
	pCreator->mParentActorHandle = this->GetHandle();
	pCreator->m_pLevelState = &levelState;
	pCreator->m_pScoringInfo = &mScoringInfo;
	pCreator->m_pRLC = m_pRLC;
	pCreator->m_pConfig = C_MultiplayerData::Instance().m_pConfig;
	pCreator->m_pSilentRunningAbility = C_Game::GetInstance()->GetMultiplayerAbilities().GetAbility(UI::E_MultiplayerVehicleAbilities_Silent_Running);
	pCreator->m_pXtAmaxVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)pGraphicalActor->GetXtIndex()];

	pCreator->m_pVehicleGraphicalActor = pGraphicalActor;
	pCreator->m_pVehicleLogicActor = this;

	pAbilitiesComponent = (Components::C_VehicleComponent_Abilities*)InitialiseComponent( pCreator.Get() );
	if( pAbilitiesComponent == NULL )
		return false;
	else
		return true;
}

const bool C_VehicleLogicActor::AddNetworkAbilitiesComponent(Game::C_LevelState& levelState)
{
	C_VehicleActor* pGraphicalActor = GetGraphicalActor(mVehicleIndex);
	BIZ_ASSERT_MSG(pGraphicalActor, "C_VehicleLogicActor::AddAbilitiesComponent() > Couldn't get a pointer to the graphical actor, this is going to go badly :(");

	Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_Network_Abilities_CreateInfo> pCreator;
	pCreator->mVehicleIndex = mVehicleIndex;
	pCreator->mParentActorHandle = this->GetHandle();
	pCreator->m_pLevelState = &levelState;
	pCreator->m_pScoringInfo = &mScoringInfo;
	pCreator->m_pRLC = m_pRLC;
	pCreator->m_pConfig = C_MultiplayerData::Instance().m_pConfig;
	pCreator->m_pSilentRunningAbility = C_Game::GetInstance()->GetMultiplayerAbilities().GetAbility(UI::E_MultiplayerVehicleAbilities_Silent_Running);
	pCreator->m_pXtAmaxVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)pGraphicalActor->GetXtIndex()];

	pCreator->m_pVehicleGraphicalActor = pGraphicalActor;
	pCreator->m_pVehicleLogicActor = this;

	pAbilitiesComponent = (Components::C_VehicleComponent_Network_Abilities*)InitialiseComponent( pCreator.Get() );
	if( pAbilitiesComponent == NULL )
		return false;
	else
		return true;
}

const bool C_VehicleLogicActor::AddSuperControlControllerComponent(const bool addDynamically)
{
	Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_SuperControlController_CreateInfo> pCreator;

	pCreator->mVehicleIndex = mVehicleIndex;
	pCreator->mParentActorHandle = this->GetHandle();
	pCreator->m_pLevelState = &Game::C_Game::GetInstance()->GetLevelState();
	pCreator->m_pRaceLogicLayer = mRaceLogicLayerPtr;
	pCreator->m_pLogicActor = this;
	pCreator->m_pGraphicalActor = GetGraphicalActor(mVehicleIndex);

	if(addDynamically)
	{
		pSuperControlControllerComponent = (Components::C_VehicleComponent_SuperControlController*)SpawnComponent( pCreator.Get() );
	}
	else
	{
		pSuperControlControllerComponent = (Components::C_VehicleComponent_SuperControlController*)InitialiseComponent( pCreator.Get() );
	}

	if( pSuperControlControllerComponent == NULL )
		return false;
	else
		return true;
}

const bool C_VehicleLogicActor::RemoveSuperControlControllerComponent()
{
	RemoveComponent(Components::E_IF_VehicleComponent_SuperControlController);

	pSuperControlControllerComponent = NULL;
	return true;
}

void C_VehicleLogicActor::SetSilentRunningSFXEnabled(const bool silentRunningSFXEnabled)
{
	if(pAbilitiesComponent)
	{
		pAbilitiesComponent->SetSilentRunningSFXEnabled(silentRunningSFXEnabled);
	}
}

void C_VehicleLogicActor::ClearUpSilentRunningSFXForEndOfRace()
{
	if(pAbilitiesComponent)
	{
		pAbilitiesComponent->EmergencyKillSilentRunningSFX();
	}
}

void C_VehicleLogicActor::KillImpl()
{
	m_pRLC = NULL;
}

// -------------------------------------------------------------------------------------------------

void C_VehicleLogicActor::CalcMaxHealth()
{
	const RLDamageSystem* pRLDamageSystem	= const_cast<C_VehicleLogicActor*>(this)->GetRLC()->GetRLSystem()->mDamageSystem;
	Game::C_LevelState& levelState			= Game::C_Game::GetInstance()->GetLevelState();
	GAMEDATA_ID carId						= (GAMEDATA_ID)levelState.mCars[mVehicleIndex].mCarID;
	AmaxVehicle* pAmaxVeh					= C_GameVehicleData::Instance().mVehicles[carId];

	mMaxHealth = 0;
	if(pAmaxVeh)
	{
		for( int i = 0; i < pRLDamageSystem->mParameters->mVehicleHealthGroups.count; i++ )
		{
			if( pAmaxVeh->vehicleHealthGroup == pRLDamageSystem->mParameters->mVehicleHealthGroups[i].mHealthGroup )
			{
				if (levelState.mCars[mVehicleIndex].mHealthOverride > 0)
					mMaxHealth = levelState.mCars[mVehicleIndex].mHealthOverride;
				else
				{
					mMaxHealth = (int)pRLDamageSystem->mParameters->mVehicleHealthGroups[i].mBaseHealth;
					mMaxHealth += pRLDamageSystem->mParameters->mHealthIncrement * levelState.mCars[mVehicleIndex].mUpgrades.NumAdditionalHealthSlots();
				}
				break;
			}
		}
	}
}

// -------------------------------------------------------------------------------------------------

// [Blur2PC] retail Blur PC (0x004390b0 -> 0x020c08f3) has no ignoreSlots argument: a full carousel always
// refuses the pickup
bool C_VehicleLogicActor::CanHavePickUp(E_PICKUP_TYPE type) const
{
	if(mPickupInfo.mPickupStore.Size() >= Game::C_Game::GetInstance()->GetLevelState().mCars[mVehicleIndex].mUpgrades.NumPickupSlots())
		return false;

	if(!mPickupsAllowed)
		return false;

	return true;
}

// -------------------------------------------------------------------------------------------------

Debug::Float g_debugGhostTransitionTime("Vehicle.GhostTransitionTime", 0.25f);

void C_VehicleLogicActor::Ghost(const float time, const float targetAlpha)
{
	mGhostTime = time;
	mGhosted = true;

	if(mCloaked)
	{
		DisEngageCloak();
	}
	SetSilentRunningSFXEnabled(false); //Disable silent running

	C_VehicleActor* pGraphicalActor = GetGraphicalActor(mVehicleIndex);

	//Nb/If you're secretly ghosting don't call this function, use SecretlyGhost() instead
	pGraphicalActor->SetAlphaTransitionCurve((const Curve::C_HermiteCurve *)NULL);
	pGraphicalActor->SetAlpha(E_VAC_TYPE_ALL, targetAlpha, E_VAC_TARGET_TYPE_DESIRED);

	pGraphicalActor->Ghost(g_debugGhostTransitionTime, targetAlpha);
	pGraphicalActor->SetCollisions(false);

	S8 viewport = m_pRLC->GetViewportIndex(mVehicleIndex);
	C_VehicleLogicMsg_GhostedMsg ghostedMsg(mVehicleIndex, mGhostTime, viewport);
	
	if(viewport != -1)
		m_pRLC->GetActorList()->BroadcastMessage(&ghostedMsg);
	else
		SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &ghostedMsg);
}

const bool C_VehicleLogicActor::Deghost()
{
	C_VehicleActor* pGraphicalActor = GetGraphicalActor(mVehicleIndex);
	BIZ_ASSERT_MSG(pGraphicalActor, "C_VehicleLogicActor::Deghost() > Couldn't get a pointer to the graphical vehicle actor");

	//Set ourselves up to be ghosted so that we will attempt to de-ghost
	//every frame until we succeed.
	mGhostTime = 0.0f;
	mGhosted = true;

	const bool safeToDeGhost = CanISafelyTurnMyCollisionsBackOn();
	if(safeToDeGhost)
	{
		mGhosted = false;
		if(mCloaked)
		{
			DisEngageCloak();
		}
		SetSilentRunningSFXEnabled(); //Allow silent running again

		//pGraphicalActor->SetForceParticlesVisible(false); //Let the car make its own decisions on particles
		float targetAlpha = 1.f;
		pGraphicalActor->SetAlpha(E_VAC_TYPE_ALL, targetAlpha, E_VAC_TARGET_TYPE_DESIRED);
		pGraphicalActor->SetAlphaTransitionCurve((const Curve::C_HermiteCurve *)NULL);
		pGraphicalActor->Deghost((float)g_debugGhostTransitionTime, targetAlpha);
		pGraphicalActor->SetCollisions(true);

		S8 viewport = m_pRLC->GetViewportIndex(mVehicleIndex);
		if(viewport != -1)
		{
			C_VehicleLogicMsg_DeghostedMsg deghostedMsg(mVehicleIndex, viewport);
			m_pRLC->GetActorList()->BroadcastMessage(&deghostedMsg);
		}

		return true;
	}

	return false;
}

//Checks whether or not we can unghost, secretly or otherwise based upon other cars being near me
const bool C_VehicleLogicActor::CanISafelyTurnMyCollisionsBackOn() const
{
	C_VehicleActor* pGraphicalActor = GetGraphicalActor(mVehicleIndex);
	BIZ_ASSERT_MSG(pGraphicalActor, "C_VehicleLogicActor::CanISafelyTurnMyCollisionsBackOn() > Couldn't get a pointer to the graphical vehicle actor");

	Btl::Vector<int> vehiclesUnderneath;
	pGraphicalActor->CheckForPotentialCollisions(vehiclesUnderneath);
	for( int i = 0; i < vehiclesUnderneath.Size(); i++ )
	{
		//One of the many vehicles underneath us isn't ghosted (hopefully there won't me more than one!) so we can't safely unghost
		if(!mRaceLogicLayerPtr->GetLogicVehicleActor(vehiclesUnderneath[i])->IsGhosted())
		{
			return false;
		}
	}

	return true;
}

void C_VehicleLogicActor::SecretlyGhost()
{
	if(!mSecretGhosting)
	{
		//Secret ghosting is where we just turn off the collisions, secretly...
		C_VehicleActor* pGraphicalActor = GetGraphicalActor(mVehicleIndex);
		BIZ_ASSERT_MSG(pGraphicalActor, "C_VehicleLogicActor::SecretlyGhost() > Couldn't get a pointer to the graphical actor");
		pGraphicalActor->SetCollisions(false);

		mSecretGhosting = true;
	}	
}

void C_VehicleLogicActor::SecretlyDeGhost()
{
	if(mSecretGhosting)
	{
		//We can't necessarily deghost (we might be within) another vehicle
		if(Deghost())
		{
			mSecretGhosting = false;
		}
	}
}

//Used for the silent running mod
void C_VehicleLogicActor::EngageCloak(const Curve::C_HermiteCurve *pAlphaTransitionCurve)
{
	//Ghost overrides cloaking
	if( !IsGhosted() )
	{
		float targetAlpha = 0.f;
		float transitionTime = C_MultiplayerData::Instance().m_pConfig->m_SilentRunningModSettings.m_CloakFadeDownTime;

		if( VehicleIsLocal(mVehicleIndex) )
			targetAlpha = C_MultiplayerData::Instance().m_pConfig->m_SilentRunningModSettings.m_LocalGhostOpacity;
		else
			targetAlpha = C_MultiplayerData::Instance().m_pConfig->m_SilentRunningModSettings.m_RemoteGhostOpacity;

		
		C_VehicleActor* pGraphicalActor = GetGraphicalActor(mVehicleIndex);
		pGraphicalActor->SetAlphaTransitionCurve(pAlphaTransitionCurve);
		pGraphicalActor->Ghost(transitionTime, targetAlpha);
		
		mCloaked = true;

		if(pAbilitiesComponent)
		{
			pAbilitiesComponent->StartSilentRunningSFX();
		}
	}
}

//Used for the silent running mod
void C_VehicleLogicActor::DisEngageCloak(const Curve::C_HermiteCurve *pAlphaTransitionCurve)
{
	//Ghost overrides cloaking
	if( !IsGhosted() )
	{
		float transitionTime = C_MultiplayerData::Instance().m_pConfig->m_SilentRunningModSettings.m_CloakFadeDownTime;

		C_VehicleActor* pGraphicalActor = GetGraphicalActor(mVehicleIndex);
		pGraphicalActor->SetAlphaTransitionCurve(pAlphaTransitionCurve);
		pGraphicalActor->Deghost(transitionTime);
	}
	mCloaked = false;

	//Stop the SFX
	if(pAbilitiesComponent)
	{
		pAbilitiesComponent->FinishSilentRunningSFX();
	}
}

//Used for the silent running mod
const bool C_VehicleLogicActor::IsCloaked() const
{
	return mCloaked;
}

void C_VehicleLogicActor::ResetHealth()
{
	const RLDamageSystem* pRLDamageSystem = GetRLC()->GetRLSystem()->mDamageSystem;

	mCurrentHealthLevel = E_HEALTH_LEVEL_HIGH;
	mCurrentHealth		= GetMaxHealth() - 1;

	for( int i = 0; i < pRLDamageSystem->mHealthZones.count; i++ )
	{
		if( pRLDamageSystem->mHealthZones[i]->mHealthZone == E_HEALTH_LEVEL_HIGH )
		{
			mCurrentHealthZone = pRLDamageSystem->mHealthZones[i];
			break;
		}
	}
}

void C_VehicleLogicActor::PreRemoveFromRace(float hideTime )
{
	//	This should protect from a vehicle being wrecked and then immediately eliminated or something and calling mRaceLogicLayerPtr->DisableVehicle() twice.
	if(!mStillInRace)
		return;

	if(!GetRLC()->IsVehiclePlayerControlled(mVehicleIndex) || C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() != E_GameMode_SinglePlayer)
	{
		Ghost(g_debugEliminatedHideTimer);
		mHideTimer = hideTime;
	}

	//Bug fix 15905
	mPickupInfo.mCharging = false;

	GetGraphicalActor(mVehicleIndex)->HaltAndRemoveFromRaceLogic();

	//	Decrement the number of active vehicles in the race (for the HUD).

	mRaceLogicLayerPtr->DisableVehicle();

	mStillInRace = false;
	mLeftRace = true;
}

void C_VehicleLogicActor::RemoveFromRace()
{
	//	Disable the graphical/physical actor.
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	levelState.mCars[mVehicleIndex].mState = Game::E_VEHICLE_STATE_LEFT_RACE;
	levelState.mCars[mVehicleIndex].mUpgrades.GetMultiplayerAbilities()->Reset();

	GetGraphicalActor(mVehicleIndex)->RemoveFromRace();
}

void C_VehicleLogicActor::ResetHitBy( U8 hitByVehicleIndex )
{
	mScoringInfo.mMostRecentVehicleIdxHitBy = hitByVehicleIndex;

	AmaxScoringConfig* pScoringConfig = C_GameData::Instance().mScoringConfig[0];
	if(!pScoringConfig)
	{
		BIZ_ASSERT_MSG(pScoringConfig, "Couldn't get scoring config data");
		return;
	}

	mScoringInfo.mWreckedTimer		= pScoringConfig->wrecked_timer;
	mScoringInfo.mFlippedOutTimer	= pScoringConfig->flipped_out_timer;
	mScoringInfo.mCrashedOutTimer	= pScoringConfig->crashed_out_timer;
	mScoringInfo.mDrownedTimer		= pScoringConfig->wrecked_timer;	//	Just use this one, they're all the same right now anyway
	mScoringInfo.mOutOfBoundsTimer	= pScoringConfig->out_of_bounds_timer;
}

void C_VehicleLogicActor::OverrideCurrentPickup( E_PICKUP_TYPE type )
{
	mPickupInfo.mPickupStore.Clear();
	if(type == E_PICKUP_TYPE_NONE)
		mPickupInfo.mPickupStoreIndex = -1;
	else
	{
		PickupStore pus;
		pus.mType = type;
		mPickupInfo.mPickupStore.PushBack(pus);
		mPickupInfo.mPickupStoreIndex = 0;
	}
}

// -------------------------------------------------------------------------------------------------

void C_VehicleLogicActor::CalculateVehicleProximities()
{
	PERF_EVENT("C_VehicleLogicActor.CalculateVehicleProximities");

	Maths::HVec3	relativePos, relativeDir;

	VehicleProximity* prox = NULL;
	C_VehicleLogicActor* pOtherVeh;

	for( int otherVehIdx = 0; otherVehIdx < mRaceLogicLayerPtr->GetNumVehicles(); otherVehIdx++ )
	{
		if( mVehicleIndex == otherVehIdx )
			continue;

		prox		= &mVehicleProximities[otherVehIdx];
		pOtherVeh	= mRaceLogicLayerPtr->GetLogicVehicleActor(otherVehIdx);

		relativePos = mPositionInfo.mWorldPosition - pOtherVeh->mPositionInfo.mWorldPosition;
		prox->mDistanceSquared = relativePos.LengthSquared();

		if (prox->mDistanceSquared > 40000.0f) //200metres
			continue;

		relativeDir = relativePos;
		relativeDir.Normalise();

		prox->mRelativeHeading = Maths::DotProduct(mPositionInfo.mDirection, -relativeDir);
	}
}

void C_VehicleLogicActor::CalculateVehicleTargetIndex()
{
	PERF_EVENT("C_VehicleLogicActor.CalculateVehicleTargetIndex");

	mPickupInfo.mCurrentTargetIdx = -1;

	if(mPickupInfo.mPickupStoreIndex >= 0 && mPickupInfo.mPickupStoreIndex < mPickupInfo.mPickupStore.Size())
	{
		PickupStore& store = mPickupInfo.mPickupStore[mPickupInfo.mPickupStoreIndex];
		
		switch(store.mType)
		{
		case E_PICKUP_TYPE_SHUNT:
			{
				int bestIdx = -1;

				bool onSpline = false;
				bool atDistance = false;

				for( int i = 0; i < mRaceLogicLayerPtr->GetNumVehicles(); i++ )
				{
					
					float score = EvaluateShuntTarget(i, mVehicleProximities[i].mRelativeHeading, onSpline, atDistance);

					mShuntEvaluations[i].mVehIdx		= i;
					mShuntEvaluations[i].mScore			= score;
					mShuntEvaluations[i].mOnSpline		= onSpline;
					mShuntEvaluations[i].mAtDistance	= atDistance;
				}

				std::sort(mShuntEvaluations.Begin(), mShuntEvaluations.End(), ShuntEvaluationSort());

				// Look through the targets in priority order to see if I think I can hit them.
				for( int i = 0; i < mRaceLogicLayerPtr->GetNumVehicles(); i++ )
				{
					int vehIdx = mShuntEvaluations[i].mVehIdx;
					if(vehIdx == mVehicleIndex || mShuntEvaluations[i].mScore >= NOT_A_GOOD_TARGET_AT_ALL)
						continue;

					Maths::HVec3 targetPos = mRaceLogicLayerPtr->mVehiclePositions[vehIdx].mPosition;

					bool canTarget = mShuntEvaluations[i].mOnSpline || mShuntEvaluations[i].mAtDistance || LineOfSight(mPositionInfo.mWorldPosition, targetPos);
					if(canTarget)
					{
						bestIdx = vehIdx;
						break;
					}
				}

				// Still no target - everyone is out of sight - so use the best available
				if(bestIdx == -1 &&  mShuntEvaluations[0].mScore < NOT_A_GOOD_TARGET_AT_ALL)
				{
					bestIdx = mShuntEvaluations[0].mVehIdx;
				}

#ifndef CFG_GOLD
				if(mVehicleIndex == 0 && bestIdx != mPickupInfo.mCurrentTargetIdx)
				{
					LOGDEBUG(g_ShuntInfoLogger, "Shunt target has evaluated to %i", bestIdx);
				}
#endif

				mPickupInfo.mCurrentTargetIdx = bestIdx;
				break;
			}

		case E_PICKUP_TYPE_SHOCK:
			{
				mPickupInfo.mCurrentTargetIdx = EvaluateShockTarget();
				break;
			}
		}
	}
}

// -------------------------------------------------------------------------------------------------

E_PICKUP_TYPE C_VehicleLogicActor::GetCurrentlySelectedPickup()
{
	E_PICKUP_TYPE type = E_PICKUP_TYPE_NONE;

	int idx = mPickupInfo.mPickupStoreIndex;
	if(idx >= 0 && idx < mPickupInfo.mPickupStore.Size())
		type = mPickupInfo.mPickupStore[idx].mType;

	return type;
}

// -------------------------------------------------------------------------------------------------

bool C_VehicleLogicActor::CheckInVisionRangeOfOtherVehicle(int vehIdx)
{
	float rawDist					= mVehicleProximities[vehIdx].mDistanceSquared;

	float halfAngleInRadians		= Maths::DegToRad( g_debugFrontAngle / 2.f);
	float cosHalfAngleInRadians		= Maths::Cos(Maths::HF32(halfAngleInRadians));

	float distanceToCompare			= 0;

	if( mVehicleProximities[vehIdx].mRelativeHeading >= cosHalfAngleInRadians )
		distanceToCompare = (float)(g_debugFrontDistance * g_debugFrontDistance);
	else
		distanceToCompare = (float)(g_debugRearDistance * g_debugRearDistance);

	return ( rawDist < distanceToCompare );
}

// -------------------------------------------------------------------------------------------------

void C_VehicleLogicActor::DrawDebugBarge(Maths::HVec3 corner1, Maths::HVec3 corner2, Maths::HVec3 corner3, Maths::HVec3 corner4)
{
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(corner1, corner2, Maths::RGBA(255,255,0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(corner2, corner3, Maths::RGBA(255,255,0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(corner3, corner4, Maths::RGBA(255,255,0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(corner4, corner1, Maths::RGBA(255,255,0));
}

float C_VehicleLogicActor::InsideRectangle(Maths::HVec3 point, Maths::HVec3 corner1, Maths::HVec3 corner2, Maths::HVec3 corner3)
{
	Maths::HVec3 v = point - corner1;
	Maths::HVec3 v1 = corner2 - corner1;
	Maths::HVec3 v2 = corner3 - corner1;

	float dot1 = Maths::DotProduct(v, v1);
	float dot2 = Maths::DotProduct(v, v2);
	float dot3 = Maths::DotProduct(v1, v1);
	float dot4 = Maths::DotProduct(v2, v2);

	float check1 = (float)(abs((dot1/dot3) - 0.5));
	float check2 = (float)(abs((dot2/dot4) - 0.5));

	return ( check1 + check2 );
}

// -------------------------------------------------------------------------------------------------

const int C_VehicleLogicActor::EvaluateShockTarget()
{
	int numVehicles = mRaceLogicLayerPtr->GetNumVehicles();
	int bestPos		= numVehicles + 1;
	int bestIdx		= -1;
	bool isNetwork  = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork();

	for( int i = 0; i < numVehicles; i++ )
	{
		C_VehicleLogicActor* pOtherActor = mRaceLogicLayerPtr->GetLogicVehicleActor(i);
		if( pOtherActor->mPositionInfo.mRacePosition < bestPos )
		{
			bool inRace = false;
			if(isNetwork)
				inRace = (C_MpResults::Instance().m_LiveVehicleData[i].m_State == E_VEHICLE_STATE_RACING);
			else
				inRace = pOtherActor->GetStillInRace();

			if(inRace)
			{
				bestPos = pOtherActor->mPositionInfo.mRacePosition;
				bestIdx = i;
			}
		}
	}

	return bestIdx;
}

// -------------------------------------------------------------------------------------------------

const float C_VehicleLogicActor::EvaluateShuntTarget(const int potentialTargetVehicleIndex, const float checkAngle, bool& onSameSpline, bool& atDistance)
{
	static const float BEST_TARGET = 0.f;
	static const float MAX_TARGET_OFFSET_ANGLE = 0.707f; //Cos(45 deg)
	static const float MAX_TARGET_DISTANCE = 500.f;

	onSameSpline	= false;
	atDistance		= false;

	const bool aiDataAvailable = AIAmax::AmaxAIR::Instance().AllOK();

	float probVal = NOT_A_GOOD_TARGET_AT_ALL;

	C_VehicleLogicActor* pPotentialTargetLogicActor = mRaceLogicLayerPtr->GetLogicVehicleActor(potentialTargetVehicleIndex);
	//See if this vehicle's a valid target
	if(mVehicleIndex == potentialTargetVehicleIndex || VehiclesAreTeammates(mVehicleIndex, potentialTargetVehicleIndex) || pPotentialTargetLogicActor->IsWreckedOrEliminated() || !pPotentialTargetLogicActor->mStillInRace)
	{
		return NOT_A_GOOD_TARGET_AT_ALL;
	}

	Maths::HVec3 targetPosition = pPotentialTargetLogicActor->GetWorldMatrix().AW();

	if(aiDataAvailable && !g_OverrideShuntTargetting)
	{
		// forwards only at present........
		float AIDist = AIAmax::AmaxAIR::Instance().DistanceInfrontBehindLogicCalculationParamatised(mPositionInfo.mLogicPosition, pPotentialTargetLogicActor->mPositionInfo.mLogicPosition);
		if (AIDist < 0.0f)
		{
			// we are behind, but pick a target
			AIDist += AIAmax::AmaxAIR::Instance().TrackLength();
		}
		// Not sure about this, ideally we only want people infront in the race, but after x laps things become screwed.
		float diffPos = (float)(mPositionInfo.mRacePosition - pPotentialTargetLogicActor->mPositionInfo.mRacePosition);
		if (diffPos < 0.0f)
			return NOT_A_GOOD_TARGET_AT_ALL;	

		airacingtech::ISplineTracker* pSplineTracker			= AIAmax::AmaxAIR::Instance().GetAISplineTracker(mVehicleIndex);
		airacingtech::ISplineTracker* pTargetSplineTracker		= AIAmax::AmaxAIR::Instance().GetAISplineTracker(potentialTargetVehicleIndex);

		if(pSplineTracker && pTargetSplineTracker && pSplineTracker->GetLogicRoute()->m_in && pTargetSplineTracker->GetLogicRoute()->m_in)
		{
			airacingtech::ITrackDataAI *pTrackData				= AIAmax::AmaxAIR::Instance().GetTrackDataSystem();
			airacingtech::AITrackDataRoutes* pLogicRoute		= pTrackData->GetTrackData()->m_TrackLogicData.m_pLogicRoute;
			int idx												= pSplineTracker->GetLogicRoute()->m_index;
			int targetIdx										= pTargetSplineTracker->GetLogicRoute()->m_index;

			if(idx != -1 && targetIdx != -1)
			{
				const Btl::Vector<airacingtech::RouteInfo>& routeInfoList = pSplineTracker->GetRouteInfo();

				for( int routeIdx = 0; routeIdx < routeInfoList.Size(); routeIdx++ )
				{
					const airacingtech::RouteInfo& routeInfo = routeInfoList[routeIdx];
					
					if(routeInfo.m_in && routeInfo.m_pRoute && routeInfo.m_pRoute->IsChangeableRoute() && !routeInfo.m_pRoute->IsLogic())
					{
						if(pTargetSplineTracker->IsInRoute(routeInfo.m_RouteIndex))
						{
							onSameSpline = true;
							break;
						}
					}
				}
			}	
		}

		if(AIDist > g_ShuntTargettingCutOffRange)
		{
			probVal = 0.5f + Maths::Min(2.0f, (AIDist / g_debugShuntMaxRange)) * g_debugShuntDistBias;
			atDistance = true;
		}
		else
		{
			float angle		= Maths::RadToDeg(acos(Maths::Abs(checkAngle)));

			if(angle > g_ShuntInitialConeAngleLowRange)
			{
				return NOT_A_GOOD_TARGET_AT_ALL;	
			}
			else
			{
				float distBias	= Maths::Min(2.0f, (AIDist / g_ShuntTargettingCutOffRange)) * g_debugShuntDistBias;
				float angleBias = Maths::Min(1.0f,(angle / g_ShuntInitialConeAngleLowRange)) * g_debugShuntAngleBias;

				probVal = angleBias + distBias;
				probVal = Maths::Min(0.5f, probVal);
			}
		}

		if(!onSameSpline)
			probVal += 0.5f;

		probVal = Maths::Min(probVal / 10, 1.f); // Make it < 1 if it has reached here.
			
		return probVal;
	}
	else
	{
		if(g_OverrideShuntTargetting)
		{
			if(potentialTargetVehicleIndex == g_ShuntTargetVehicleIndex)
			{
				return BEST_TARGET;
			}
			else
			{
				return NOT_A_GOOD_TARGET_AT_ALL;
			}
		}
		else
		{
			//No AI data available (mp bowls?)

			int viewportIndex = m_pRLC->GetViewportIndex(mVehicleIndex);
			Framework::C_ApplicationBase* pAppBase = Framework::C_ApplicationBase::GetInstance();

			if(!pAppBase || viewportIndex < 0 || viewportIndex >= pAppBase->GetSplitscreenViewCount())
			{
				return NOT_A_GOOD_TARGET_AT_ALL;
			}

			const Framework::C_View& view = pAppBase->GetSplitscreenView(viewportIndex);
			Maths::HVec3 cameraPosition = view.DebugFeed().Mat43().AW();

			//Work out if we can see the target
			bool canSee = LineOfSight(cameraPosition, targetPosition);
			if(!canSee)
			{
				return NOT_A_GOOD_TARGET_AT_ALL;
			}

			//For the time being just target the vehicle straight ahead
			Maths::HVec3 targetDirection = targetPosition - mPositionInfo.mWorldPosition;
			const float targetDistance = Maths::Magnitude(targetDirection);
			targetDirection = (Maths::HVec3)Maths::Normalise(targetDirection);

			//If the target is ahead of use (+ve dot)
			const float dotForward = Maths::DotProduct(mPositionInfo.mDirection, targetDirection);

			float angleScore = 1.f;
			float distanceScore = 1.f;

			if(dotForward > MAX_TARGET_OFFSET_ANGLE)
			{
				//A value of 0.f means its the best target and 1.f means a poor target, where 
				//as a dotForward of 1.f would be straight ahead
				angleScore = (1.f - dotForward);
			}
			else
			{
				return NOT_A_GOOD_TARGET_AT_ALL;
			}

			if(targetDistance <= MAX_TARGET_DISTANCE)
			{
				distanceScore = ((targetDistance / MAX_TARGET_DISTANCE)); //1.f badish
				distanceScore = Maths::Sqrt(distanceScore);
			}
			else
			{
				return NOT_A_GOOD_TARGET_AT_ALL;
			}

			return (angleScore * distanceScore);
		}
	}
}

// -------------------------------------------------------------------------------------------------

bool C_VehicleLogicActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	switch(pMessage->Type())
	{
		case Game::E_GameflowMessage_ClearStoredActorPtrs:
			{
				m_pRLC = NULL;
				return true;
			}

		case GameMessaging::E_GameMsg_Update:
			{
				Update();
				return true;
			}
			

		case GameMessaging::E_GameMsg_PreUpdate:
			{
				PreUpdate();
				return true;
			}
	
		case Game::E_GameflowMessage_RaceLoaded:
			{
				AddSubscribers();

				m_pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

				return true;
			}
		case Game::E_GameflowMessage_StopGameUpdate:
		case Game::E_GameflowMessage_RestartLevel:
			{
				RemoveSubscribers();

				return true;
			}		

		case Game::E_VehicleMessage_FatallyDamaged:
			{
				const Game::Actors::C_VehicleFatallyDamagedMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleFatallyDamagedMsg>(pMessage);

				if( pMsg->m_VehicleIndex == mVehicleIndex)
				{
					Game::Actors::C_VehicleActor* GraphicalActor = GetGraphicalActor(mVehicleIndex);

					// Dont apply a wrecked impulse if the last gasp mod has already applied its special last gasp detonation impulse.
					bool lastGaspWreckedImpulseApplied = false;
					const Components::C_VehicleComponent_Damage* pDamage = (Components::C_VehicleComponent_Damage*)QueryInterface(Components::E_IF_VehicleComponent_Damage);
					if(pDamage)
					{
						lastGaspWreckedImpulseApplied = pDamage->GetHasLastGaspAppliedWreckedImpulse();
					}
					
					// Apply wrecked impulse.. unless last gasp mod has already taking responsibility for doing that
					if(!lastGaspWreckedImpulseApplied )
					{
						GraphicalActor->ApplyWreckedImpulse();
					}

					GraphicalActor->SetLostControl(true);
					GraphicalActor->SetFatallyDamaged(true);
					GraphicalActor->PlayWreckedEffect();
					
				}

				return true;
			}
			break;

		case Game::E_VehicleMessage_ResetAfterWrecked:
			{
				const Game::Actors::C_VehicleResetAfterWreckedMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleResetAfterWreckedMsg>(pMessage);

				if( pMsg->mVehicleIndex == mVehicleIndex)
				{
					Game::Actors::C_VehicleActor* GraphicalActor = GetGraphicalActor(mVehicleIndex);
					GraphicalActor->SetLostControl(false);
					GraphicalActor->SetFatallyDamaged(false);

					// End of wrecked Sequence
				}
				return true;
			}
			break;

		case Game::E_VehicleMessage_Eliminated:
			{
				const Game::Actors::C_VehicleEliminationMessage* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleEliminationMessage>(pMessage);
				if(mVehicleIndex == pMsg->mVehicleIndex)
				{
					PreRemoveFromRace(g_debugEliminatedHideTimer);
				}
				return true;
			} 
			break;

		case Game::E_VehicleMessage_LeftRace:
			{
				const Game::Actors::C_VehicleLeftRaceMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleLeftRaceMsg>(pMessage);
				if(mVehicleIndex == pMsg->m_VehicleIndex)
				{
					PreRemoveFromRace(pMsg->m_HideTime);
				}
				return true;
			}
			break;

		case E_VehicleLogicMsg_HitByPerk:
			{
				const C_VehicleLogicMsg_HitByPerkMsg* msg = Messaging::MessageCast< C_VehicleLogicMsg_HitByPerkMsg >(pMessage);

				if( msg->mVehicleIndex == mVehicleIndex)
				{
					//make player invunerable
					//mInvunTimer=g_debugInvunerableWhenPerkedTimer;
															
					if(!GetRLC()->IsVehicleAIControlled(mVehicleIndex) )
					{
						mRaceLogicLayerPtr->mTimer_PerkPlayerHitAIDisable += Game::C_Game::GetInstance()->GetLevelState().mAIController->mSharedPlayerHitDisableTime;
					}
#ifndef CFG_GOLD
					AmaxDataLogging::LogHitByPickup(msg->mFiringVehicleIndex, msg->mVehicleIndex, msg->mUniqueID, mRaceLogicLayerPtr->ConvertPickupTypeToString(msg->mPickupType));
#endif
				}
				
				return true;
			}

		case Game::E_GameflowMessage_VehicleFinished:
			{
				Game::Actors::C_VehicleActor* GraphicalActor = GetGraphicalActor(mVehicleIndex);
				GraphicalActor->SetHealthEffect(NULL);

				ClearUpSilentRunningSFXForEndOfRace();
				RemoveSuperControlControllerComponent();
				
				if( mVehicleIndex == C_StickerManager::Instance().GetPrimaryProfileVehicleIndex() )
				{
					const Messaging::C_MessageBase message(Game::E_GameflowMessage_VehicleFinished, sizeof(Messaging::C_MessageBase));
					C_StickerManager::Instance().MessageHandler( &message );
				}
				return true;
			}


			// Flag this message as handled, even though we do nothing with it
		case Actor::E_ActorMsg_ComponentCreated:
			return true;

	}

	return false;
}

bool C_VehicleLogicActor::LineOfSight(Maths::HVec3 position, Maths::HVec3 targetPosition)
{
	PERF_EVENT("VehicleLogicActor.LineOfSight");

	Collision::C_TransparentRay ray;
	U8 data[1024 * sizeof(void*)];
	SharedPhysics::C_SafeStack<true> stack(data, 1024);

	S32 vehGroupId = GetGraphicalActor(mVehicleIndex)->GetPhysicalGroupID();

	Collision::C_CollisionFilter raySolidFilter(0,0);

	Collision::C_CollisionFilter rayTransFilter;
	rayTransFilter.SetBodyMask(PolyFlags::kbWorld);

	ray.InitialiseRay(position, targetPosition, &raySolidFilter, &rayTransFilter);
	m_pRLC->GetPhysicsScene()->DoTransparentRayTraceMesh(ray, stack, true);

	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(position, targetPosition, Maths::RGBA(255,255,0));

	for(U32 i = 0; i < ray.m_numLayers; i++)
	{
		Maths::C_Normal upNormal(0,1,0);
		float dot = Maths::DotProduct(ray.m_SolidResult.TriangleNormal(), upNormal);
		if(dot > -g_debugProjectileAngle && dot < g_debugProjectileAngle)
		{
			return false;
		}
	}

	return true;
}



void C_VehicleLogicActor::AddDebugPoint(Maths::HVec3 pos, Maths::RGBA col)
{
	VehicleLogicActorDebugPoint debug_point;
	debug_point.mPos=pos;
	debug_point.mColour=col;

	mDebugPoints.PushBack(debug_point);
}

Scheduler::GroupHandle C_VehicleLogicActor::m_hSchedulerGroup = 0;
bool C_VehicleLogicActor::m_TasksFlushed = false;

void C_VehicleLogicActor::SetSchedulerGroup(Scheduler::GroupHandle handle)
{
	m_hSchedulerGroup = handle;
}

//Public accessor for newly Norbinated statistics info
VehicleRaceStatistics& C_VehicleLogicActor::GetStatistics()
{
	return mStatistics;
}

//Public accessor for newly Norbinated scoring info
VehicleScoringInfo& C_VehicleLogicActor::GetScoring()
{
	return mScoringInfo;
}

//Public accessor for newly Norbinated pickup info
VehiclePickupInfo& C_VehicleLogicActor::GetPickupInfo()
{
	return mPickupInfo;
}

//Public accessor for newly Norbinated pickup info
VehiclePositionInfo& C_VehicleLogicActor::GetPositionInfo()
{
	return mPositionInfo;
}

//Public accessor for newly Norbinated lap info
VehicleLapsInfo& C_VehicleLogicActor::GetLapInfo()
{
	return mLapsInfo;
}

//Public const accessor for newly Norbinated statistics info
const VehicleRaceStatistics& C_VehicleLogicActor::GetStatistics() const
{
	return mStatistics;
}
//Public const accessor for newly Norbinated scoring info
const VehicleScoringInfo& C_VehicleLogicActor::GetScoring() const
{
	return mScoringInfo;
}

//Public accessor for newly Norbinated pickup info
const VehiclePickupInfo& C_VehicleLogicActor::GetPickupInfo() const 
{
	return mPickupInfo;
}

//Public accessor for newly Norbinated pickup info
const VehiclePositionInfo& C_VehicleLogicActor::GetPositionInfo() const
{
	return mPositionInfo;
}

//Public accessor for newly Norbinated lap info
const VehicleLapsInfo& C_VehicleLogicActor::GetLapInfo() const
{
	return mLapsInfo;
}

}; //namespace Actors
}; //namespace Game

