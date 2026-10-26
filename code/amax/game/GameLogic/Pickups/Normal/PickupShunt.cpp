#include "stdafx.h"

#include <GameLogic/Pickups/Normal/PickupShunt.h>

#include <GameApp.h>
#include <Game/game.h>

#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/RaceLogicLayer.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>
#include <GameLogic/Pickups/PickupClasses.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups.h>
#include <Core/Maths/HVec/New/Mat33.h>

#include <ui/UiMessages.h>

#include "GameAudio/GameAudioLayer.h"

#include "GameAudio/PickupSubsystem.h"


Debug::Bool g_debugShuntDrawPositionInfo("Perks.Shunt.DrawPositionInfo", false);
Debug::Bool g_debugShuntDrawTarget("Perks.Shunt.DrawTarget", false);
Debug::Float g_ShuntLoseHomingDist("Perks.Shunt.LoseHomingDist", 10.0f);
Debug::Float g_ShuntMaxSpeed("Perks.Shunt.MaxSpeed", 110.0f);	// Any higher and the missile will crash.
Debug::Float g_ShuntMaxHomingSpeed("Perks.Shunt.MaxHomingSpeed", 50.0f);	
Debug::Float g_ShuntKillTime("Perks.Shunt.KillTime", 40.0f);	// Seconds....
Debug::Float g_ShuntConeAngle("Perks.Shunt.ConeAngle", 60.0f);	// Homing cone angle.
Debug::Float g_ShuntInitialConeAngle("Perks.Shunt.InitialConeAngle", 50.0f);	// Homing cone angle.
Debug::Float g_ShuntInitialConeAngleLowRange("Perks.Shunt.InitialConeAngleLowRange", 30.0f);	// Homing cone angle.
Debug::Float g_ShuntTargettingCutOffRange("Perks.Shunt.TargettingCutOffRange", 75.0f);
Debug::Float g_ShuntWarningDot("Perks.Shunt.WarningDot", 0.5f);
Debug::Float g_ShuntWarningRange("Perks.Shunt.WarningRange", 100.f);
Debug::Float g_ShuntMaxTurnRate("Perks.Shunt.MaxTurnRate", 180.0f);	// Turn rate.....
Debug::Float g_ShuntMaxFallRate("Perks.Shunt.MaxFallRate", 8.0f);	// Clamp to ground.....
//Debug::Float g_ShuntMinTurnRate("Perks.Shunt.MinTurnRate", 90.0f);	// Turn rate.....
Debug::Float g_ShuntHeightFromGround("Perks.Shunt.HeightFromGround", 0.3f);//
Debug::Float g_ShuntHomingDecayDist("Perks.Shunt.HomingDecayDist", 40.0f);
Debug::Float g_debugShuntAngleBias("Perks.Shunt.AngleBias",0.15f);		// 0.95f
Debug::Float g_debugShuntDistBias("Perks.Shunt.DistBias",0.1f);		// 0.9f
Debug::Float g_debugShuntLOSBias("Perks.Shunt.LOSBias",0.2f); // 0.8f
//Debug::Float g_debugShuntPositionBias("Perks.Shunt.PositionBias",0.9f); // 0.8f
Debug::Float g_debugShuntMinRange("Perks.Shunt.MinRange",150.0f);
Debug::Float g_debugShuntMaxRange("Perks.Shunt.MaxRange", 10000.0f); // change this to 10000.0f
Debug::Float g_debugShuntHomingDist("Perks.Shunt.HomingDist", 50.0f); 
Debug::Float g_debugShuntHomingTurnRate("Perks.Shunt.HomingTurnRate", 65.0f); // 30.0f
Debug::Float g_debugShuntDirectTurnRate("Perks.Shunt.DirectTurnRate", 20.0f); // 30.0f
Debug::Float g_debugShuntPredictedSeconds("Perks.Shunt.PredictedSeconds", 2.0f); //
Debug::Float g_debugShuntStartPosMult("Perks.Shunt.StartPosMult", 0.f);

Debug::Logger &g_ShuntInfoLogger(Debug::GetLogger("Amax.Perks.Shunt.TargetLogger"));

namespace Game
{
	C_PickupShuntProjectile::C_PickupShuntProjectile(U32 id, int firingIdx, int targetIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX) 
		: C_PickupProjectile(E_PickupClass_ShuntProjectile, id, firingIdx, targetIdx, E_PICKUP_TYPE_SHUNT, subType, noFiringVehicleFX)
	{
		m_CurrentECMHomingTurnScalar			= 1.0f;
		RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_SHUNT);
		mpShuntData							= (RLShuntBalancingData *)pPickup->mNormal->mBalancingData;
		mpShuntSFX							= (RLShuntSpecialFX*)pPickup->mNormal->mSpecialFX;

		BIZ_ASSERT(mpShuntData);
		BIZ_ASSERT(mpShuntSFX);

		mDropFXTimer		= mpShuntData->mFXDropTime;
		mRadius				= mpShuntData->mRadius;
		mHeightFromGround	= g_ShuntHeightFromGround;
		m_targetIndex = -1;
		mTargetIdx = targetIdx;
		m_distanceInfront = -1;
		m_CurrentECMHomingTurnScalar = 1.0f;
		m_ECMActivationDistSqrd			= mpShuntData->mECMActivationDist * mpShuntData->mECMActivationDist; 
		m_ECMDeactivationDistSqrd		= mpShuntData->mECMDeactivationDist * mpShuntData->mECMDeactivationDist;
		m_ECMWarningDistSqrd			= mpShuntData->mECMWarningDist * mpShuntData->mECMWarningDist;
		m_ECMEffectsActive				= false;
		m_ECMWarningGiven				= false;
		m_ECMWarningEnabled				= mpShuntData->mECMWarningEnabled;
		mFallRate						= mpShuntData->mFallRate;

		// Cache wether we are in splitscreen, if so ecm warnings always neeed to be given
		m_IsSplitscreen					= false;
		Game::C_GameModeBase *pGameModeBase = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();
		if(pGameModeBase && pGameModeBase->GetID() == Game::E_GameMode_SplitScreen)
		{
			m_IsSplitscreen	 = true;
		}

		m_HasHitPlayer = false;

		m_HasBeenLockedOn = false;

#ifndef CFG_GOLD
		LOGDEBUG(g_ShuntInfoLogger, "%i fired a shunt at %i with id %u", mFiringIdx, mTargetIdx, mUniqueId);
#endif // CFG_GOLD
	}

	// --------------------------------------------------------------------------------
	void C_PickupShuntProjectile::CalculateECMTurnScalar(const float currentDistToTargetSqrd)
	{
		// Default to 1.0
		m_CurrentECMHomingTurnScalar = 1.0f;
	
		if(mpShuntData && m_targetIndex != -1)
		{
			// If ! ECM effects active
			if(!m_ECMEffectsActive)
			{
				// If  shunt is < ECMActivationDist from target
				if( currentDistToTargetSqrd < m_ECMActivationDistSqrd )
				{
					// Use reduced rate turn scalar
					m_CurrentECMHomingTurnScalar = mpShuntData->mShuntTurnRateScalar;

					m_ECMEffectsActive = true;
				}
			}
			else // else if ECM effects active
			{
				// If  shunt is < ECMDeactivationDist from target
				if( currentDistToTargetSqrd < m_ECMDeactivationDistSqrd )
				{
					// Use reduced rate turn scalar
					m_CurrentECMHomingTurnScalar = mpShuntData->mShuntTurnRateScalar;
				}
				else
				{
					// else If  shunt is > ECMDectivationDist from target
					// Use default turn rate scalar and flag not active
					m_ECMEffectsActive = false;
				}
			}
		}

		/// In splitscreen never use the turn scalar
		if(m_IsSplitscreen)
		{
			m_CurrentECMHomingTurnScalar = 1.0f;
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupShuntProjectile::PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx)
	{
		if( HasAbility(UI::E_MultiplayerVehicleAbilities_Iron_Fist, mFiringIdx) && AbilityActiveOnPickupType(UI::E_MultiplayerVehicleAbilities_Iron_Fist) )
		{
			RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_SHUNT);
			RLShuntSpecialFX* pShuntFX			= (RLShuntSpecialFX*)pPickup->mNormal->mSpecialFX;

			if(pShuntFX)
			{
				C_PickupBase::PostDelayInitialise(matrix, &pShuntFX->mIronFistFiredSpecialFX);
			}
		}
		else
		{
			C_PickupBase::PostDelayInitialise(matrix, sfx);
		}

		if(VehicleIsLocal(mFiringIdx))
		{
			SetPostDelayMatrix(matrix);

			if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork())
			{
				Game::C_Game::GetInstance()->GetNetGameVars().m_EventManager->Add_PickupPostDelay(mUniqueId, matrix);
			}
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupShuntProjectile::SetPostDelayMatrix( Maths::HMat43 targetMatrix, float fVal )
	{
		C_PickupBase::SetPostDelayMatrix(targetMatrix, fVal);

		Maths::HVec3 up(0.f, 1.f, 0.f);
		mMatrix.AW(mMatrix.AW() + (up * mRadius));

		CreateCollisionBody();
		C_InGameSpecialFXManager::Instance().CreateSpecialFX(mpShuntSFX->mProjectileSpecialFX, mMatrix, mFiringIdx, -1, mUniqueId, E_InGameSpecialFX_Trigger_Projectile);

		// Set up Darren's stuff
		Maths::HVec3 firingPos = mpRaceLogicLayer->GetLogicVehicleActor(mFiringIdx)->GetWorldMatrix().AW();
		firingPos.Y(firingPos.Y());
		m_ProjectileState = SHUNT_STATE_STAGE_1;
		m_forwards = true;

		m_shuntCar.Init();	

		Maths::HVec3 forwardsVehicle = mpRaceLogicLayer->GetLogicVehicleActor(mFiringIdx)->GetWorldMatrix().AZ();//mStartingMatrix.AX();
		float forwardsSpeed = mpRaceLogicLayer->GetLogicVehicleActor(mFiringIdx)->GetPositionInfo().mForwardSpeed + 15.0f; // add in an extra 5mps
		Maths::HVec3 initialVelocity = forwardsVehicle * Maths::Max(forwardsSpeed, 10.0f);
		m_shuntCar.m_minSpeed = 60.0f; // min speed
		m_shuntCar.SetVelocity(initialVelocity);
		m_shuntCar.SetPosition(firingPos + (forwardsVehicle * g_debugShuntStartPosMult) );
	 
		// get and store the track data for the shunt...
		airacingtech::ITrackDataAI *pTrackDataSystem = AIAmax::AmaxAIR::Instance().GetTrackDataSystem();
		airacingtech::ISplineTracker *pFiringSplineTracker = AIAmax::AmaxAIR::Instance().GetAISplineTracker(mFiringIdx);
		if (pTrackDataSystem && pFiringSplineTracker)
		{
			const airacingtech::RouteInfo *pLogicRoute = pFiringSplineTracker->GetLogicRoute();
			if (pLogicRoute)
			{
				m_shuntCar.m_CurrentLogicIndex = pLogicRoute->m_index;
			}
		}

		m_stage1Timer = 0.0f;
		m_killTime = g_ShuntKillTime;

		mSpeed = m_shuntCar.GetVelocity().Length();
		mMovementDir = Maths::Normalise(m_shuntCar.GetVelocity());
	}

	// --------------------------------------------------------------------------------

	E_PICKUP_LOCK_ON C_PickupShuntProjectile::CheckVehicleTargetted(int vehIdx)
	{
		C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

		// Check for warning distance.
		Maths::HVec3 dir	= Maths::Normalise(pLayer->mVehiclePositions[vehIdx].mPosition - mMatrix.AW());
		float dot			= Maths::DotProduct(mMatrix.AZ(), dir);
		float distance		= Maths::Distance(pLayer->mVehiclePositions[vehIdx].mPosition , mMatrix.AW());

		if(vehIdx == mTargetIdx && mActivated)
		{
			if(distance <= g_ShuntWarningRange && dot >= g_ShuntWarningDot)
			{
				if(!m_HasBeenLockedOn)
				{
					m_HasBeenLockedOn = true;
				}

				if(distance <= g_ShuntLoseHomingDist)
					return E_PICKUP_LOCK_ON_RAPID;
				else
					return E_PICKUP_LOCK_ON_NORMAL;
			}
		}

		return E_PICKUP_LOCK_ON_NONE;
	}

	// --------------------------------------------------------------------------------

	bool C_PickupShuntProjectile::Update()
	{
		PERF_EVENT("PickupShunt.Update");

		if(!mActivated)
			return false;

		float frameTime = C_GameApp::GetInstance()->GetTimeStep();

		//DebugCheckTarget();

		if(mKillPickup)
		{
			Game::C_InGameSpecialFXManager::Instance().DestroyPerk(mUniqueId);

			// If we had a target - but it didnt hit them
			if((!m_HasHitPlayer) && mTargetIdx != -1)
			{
				//If we got close enough to warn the player that a shunt was about hit them
				if(m_HasBeenLockedOn)
				{
					// Send it a scoring message for evading 
					SendShuntEvadedScoringMessageToVehicle(mTargetIdx);
				}
			}
				
			SetAbilityActive(UI::E_MultiplayerVehicleAbilities_ECM, mTargetIdx, false);

			// Make sure all players know we are know longer in ECM range if we previously were
			CleanUpECMWarningsOnDeath(); 

			KillPickupAudio();

			return true;
		}

		UpdateAbstractProjectile();

		mDropFXTimer -= frameTime;

		if(mDropFXTimer <= 0.0f)
		{
			mDropFXTimer = mpShuntData->mFXDropTime;
			C_InGameSpecialFXManager::Instance().CreateSpecialFX(mpShuntSFX->mProjectileDroppedSpecialFX, mMatrix, mFiringIdx, mTargetIdx, -1, E_InGameSpecialFX_Trigger_Firing);
		}

		m_killTime -= frameTime;
		if (m_killTime < 0.0f)
			KillPickup(-1, E_PICKUP_CAUSE_OF_DEATH_OLD_AGE);

		C_VehicleLogicMsg_PerkUpdateMatrixMsg updateMatrixMsg(mUniqueId, mMatrix, mFiringIdx,m_shuntCar.GetVelocity());

		Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_UpdateMatrix, 0, &updateMatrixMsg);

		// Check wether ECM kicks in (either in MP due to mod, or always in splitscreen)
		if( (m_IsSplitscreen	||														
			SetAbilityActive(UI::E_MultiplayerVehicleAbilities_ECM, m_targetIndex, true) )&&
			m_targetIndex != -1) // This func also checks if target has the mod equipped..
		{
			// calculate distance once
			C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			float currentDistToTargetSqrd = Maths::DistanceSquared(pLayer->mVehiclePositions[m_targetIndex].mPosition , mMatrix.AW());

			// Now do all ECM checks
			CheckDisplayECMEvadeWarning(currentDistToTargetSqrd); 

			// Turn scalar only kick in with mod, NOT in splitscreen.
			CalculateECMTurnScalar(currentDistToTargetSqrd);
		}


		return false;
	}

	// --------------------------------------------------------------------------------

	bool C_PickupShuntProjectile::Rebound(Maths::C_Normal normal, Maths::HVec3 pos)
	{
		if(m_ProjectileState == SHUNT_STATE_STAGE_4 || m_ProjectileState == SHUNT_STATE_STAGE_3)
		{
			m_shuntCar.m_velocityWorld = m_shuntCar.m_velocityWorld - ( normal * 2 * Maths::DotProduct(m_shuntCar.m_velocityWorld, (Maths::HVec3)normal) );
			mMovementDir = Maths::Normalise(m_shuntCar.m_velocityWorld);

			m_shuntCar.m_positionWorld = pos + mMovementDir;
			mMatrix.AW(m_shuntCar.m_positionWorld);
			return true;
		}

		return false;
	}

	// --------------------------------------------------------------------------------

	void C_PickupShuntProjectile::CheckDisplayECMEvadeWarning(const float currentDistToTargetSqrd)
	{
		if(	m_ECMWarningEnabled)
		{
			if(!m_ECMWarningGiven)
			{
				// Check if we are within ECM warning range, and if so register ourselves with the player
				if(currentDistToTargetSqrd <= m_ECMWarningDistSqrd)
				{
					// Make sure we tell the logic actor we are now in ECM warning range.
					Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(m_targetIndex);
					if(pLogicActor)
					{
						pLogicActor->ModifyEcmWarningCount(true);
						m_ECMWarningGiven = true;
					}
				}
			}
			else
			{
				// Check if we have fallen to a distance where a warning should no longer be given, 
				// deregister ourselves with the player
				if(currentDistToTargetSqrd >= m_ECMDeactivationDistSqrd)
				{
					// Make sure we tell the logic actor we are now in ECM warning range.
					Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(m_targetIndex);
					if(pLogicActor)
					{
						pLogicActor->ModifyEcmWarningCount(false);
						m_ECMWarningGiven = false;
					}
				}
			}
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupShuntProjectile::Trigger(int idx)
	{
		if( idx != mFiringIdx && CheckCanHit(idx) )
		{
			E_PICKUP_HIT_TYPE hitType = E_PICKUP_HIT_TYPE_MAJOR;
			if(mHealth < mpShuntData->mHealth)
				hitType = E_PICKUP_HIT_TYPE_MINOR;

			C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			if (pLayer)
			{
				Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(idx);
				if (pActor)
				{					
					Maths::HMat43 mat = pActor->GetGraphicalActor(idx)->GetMatrix();
					mHitInfo.mPosition = mHitPoint;
					mHitInfo.mDirection = mHitPoint - mPrevPos;
					mHitInfo.mDirection.Normalise();
					mHitInfo.mDirection = mHitInfo.mDirection * mat.Rotation().Inverse();
				}
			}

			RegisterHit(idx, hitType);
			if(!CheckLegalHit(idx))
			{
				NinjaKill(mAutopsyReport);
				CleanUpECMWarningsOnDeath();
			}

			// Store any data specific to this type of death
			mAutopsyReport.mTargettedVehicleId =  m_targetIndex;
			mAutopsyReport.mVehicleIdHit	   =  idx;

			// Do the killing
			KillPickup(-1, E_PICKUP_CAUSE_OF_DEATH_HIT_VEHICLE);

			if(idx == mTargetIdx)
			{
				SetHitPlayer();
			}
		}
	}


	// --------------------------------------------------------------------------------
	// When we die.. we need to make sure no player still thinks we are tracking them
	void C_PickupShuntProjectile::CleanUpECMWarningsOnDeath()
	{
		if(	m_ECMWarningEnabled	&& m_ECMWarningGiven)
		{
			// Make sure we tell the logic actor we are no longer in ECM warning range... as we are dead!
			Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(m_targetIndex);
			if(pLogicActor)
			{
				pLogicActor->ModifyEcmWarningCount(false);
				m_ECMWarningGiven = false;
			}
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupShuntProjectile::DebugDraw()
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(mMatrix.AW(), mRadius, Maths::RGBA(255,255,0));
	}

	// --------------------------------------------------------------------------------

	void C_PickupShuntProjectile::Explode(Maths::HVec3 pos, Maths::C_Normal normal)
	{
		Maths::AxisY axisY(normal);
		Maths::HMat33 mat33(axisY);
		Maths::HMat43 mat(mat33, pos);
		C_InGameSpecialFXManager::Instance().CreateSpecialFX(mpShuntSFX->mHitEnvironmentSpecialFX, mat, mSpeed * mMatrix.AZ(), mGroundPoint, mGroundNormal, mFiringIdx, mTargetIdx, -1, E_InGameSpecialFX_Trigger_Firing);
	}

	// --------------------------------------------------------------------------------

	void C_PickupShuntProjectile::CollideWithPickup(int vehIdx)
	{
		C_InGameSpecialFXManager::Instance().CreateSpecialFX(mpShuntSFX->mExplosionSpecialFX, mMatrix, mSpeed * mMatrix.AZ(), mGroundPoint, mGroundNormal, mFiringIdx, mTargetIdx, -1, E_InGameSpecialFX_Trigger_Projectile);
		C_PickupProjectile::CollideWithPickup(vehIdx);
	}

	// --------------------------------------------------------------------------------

	void C_PickupShuntProjectile::NinjaKill(const Game::PickupAutopsyReport& autoReport) 
	{ 
		// Ensure no ability icons still on (could this happen??)
		SetAbilityActive(UI::E_MultiplayerVehicleAbilities_ECM, mTargetIdx, false);

		switch(autoReport.mCauseOfDeath)
		{
		case E_PICKUP_CAUSE_OF_DEATH_HIT_STATIC_OBJECT:
		case E_PICKUP_CAUSE_OF_DEATH_HIT_DYNAMIC_OBJECT:
		case E_PICKUP_CAUSE_OF_DEATH_HIT_ENVIRONMENT:
			{
				C_InGameSpecialFXManager::Instance().CreateSpecialFX(mpShuntSFX->mHitEnvironmentSpecialFX, mMatrix, mSpeed * mMatrix.AZ(), mGroundPoint, mGroundNormal, mFiringIdx, mTargetIdx, -1, E_InGameSpecialFX_Trigger_Missed);
				break;
			}

		case E_PICKUP_CAUSE_OF_DEATH_HIT_OTHER_PICKUP:
			{
				C_InGameSpecialFXManager::Instance().CreateSpecialFX(mpShuntSFX->mExplosionSpecialFX, mMatrix, mSpeed * mMatrix.AZ(), mGroundPoint, mGroundNormal, mFiringIdx, mTargetIdx, -1, E_InGameSpecialFX_Trigger_Missed);
				break;
			}
		}

		C_PickupProjectile::NinjaKill(autoReport); 
	};

	// --------------------------------------------------------------------------------

	void C_PickupShuntProjectile::UpdateAbstractProjectile()
	{
		m_PrevPos = mMatrix.AW();
		//TODO: BACKWARDS fire

		float frameTime = C_GameApp::GetInstance()->GetTimeStep();

		if(!mpRaceLogicLayer || !mpRLC  || !m_forwards)
			return;

	
		m_shuntCar.SetMaxSpeed((float)g_ShuntMaxSpeed);
		m_coneAngle = g_ShuntConeAngle;	// Homing cone angle.
		m_collisionCheck = true;
		m_shuntCar.SetMaxTurnRate((float) g_ShuntMaxTurnRate);
		Actors::C_VehicleLogicActor* pActor	= mpRaceLogicLayer->GetLogicVehicleActor(mFiringIdx);

		Maths::HVec3 heading = m_shuntCar.GetVelocity();

		m_stage1Timer += frameTime;
		
		// update route info.......
		TrackAILogicSpline();
		UpdateTargetRoute();
		UpdateCurrentRoute();


		switch(m_ProjectileState)
		{
		
			case SHUNT_STATE_STAGE_1:
				{
					// no steering... go straight
					m_ProjectileState = SHUNT_STATE_STAGE_2;
					m_shuntCar.SetTargetSpeed(80.0f);
					m_shuntCar.SetMaxTurnRate(0.f);

					m_shuntCar.m_targetPos = m_shuntCar.GetPosition() + m_shuntCar.GetVelocity();

					if (m_LogicRoute.m_pRoute && m_LogicRoute.m_Index != -1)
					{
						m_LogicRoute.m_StartIndex = m_LogicRoute.m_Index;
					}
					m_ClosestApproach = false;
					
					m_Sequence = 0;

				}
				break;

			case SHUNT_STATE_STAGE_2:
				{
					// no steering... go straight
					m_shuntCar.SetTargetSpeed(100.0f);
					m_shuntCar.SetMaxTurnRate(0.f);
					m_shuntCar.m_targetPos = m_shuntCar.GetPosition() + m_shuntCar.GetVelocity();

					if (m_stage1Timer > 0.075f) // 1.0f // 0.5f
					{
						if(mTargetIdx != -1)
							m_ProjectileState = SHUNT_STATE_STAGE_3;
						else
							m_ProjectileState = SHUNT_STATE_STAGE_5;
					}

					m_shuntCar.m_useGroundHeight = true;
				}
				break;

			case SHUNT_STATE_STAGE_3:
				{
					ShuntState3();
				
					if (m_distanceInfront > 0.0f && m_distanceInfront < g_debugShuntHomingDist)
					{
						m_ClosestApproach = true;
					}
				}
				break;

			case SHUNT_STATE_STAGE_4:
				{
					Maths::HVec3 targetPos;
					if (m_targetIndex != -1)
					{
						FindATarget(); // Track Distance here

						targetPos = mpRaceLogicLayer->GetLogicVehicleActor(m_targetIndex)->GetWorldMatrix().AW();
							
						Maths::HVec3 targetVel = mpRaceLogicLayer->GetLogicVehicleActor(m_targetIndex)->GetGraphicalActor(m_targetIndex)->GetCollisionBody()->LinearVelocity();
						
						float speedOfTarget = MPHTOMS(mpRaceLogicLayer->GetLogicVehicleActor(m_targetIndex)->GetPositionInfo().mSpeedInMph);

						Maths::HVec3 dir = targetPos -  m_shuntCar.GetPosition();
						float lenghtSq = dir.LengthSquared();

						bool canSee = LineOfSight();
						if (!canSee)
						{
							m_ProjectileState = SHUNT_STATE_STAGE_3;
							targetPos = m_shuntCar.GetPosition()+ m_shuntCar.GetVelocity();
						}

						Maths::HVec3 offset = targetPos - m_shuntCar.GetPosition();
						Maths::HVec3 vel = m_shuntCar.GetVelocity();
						vel.Normalise();
						float forwardsDist  = offset | vel; // in metres
						offset.Normalise();
						//float angle = Maths::Abs(Maths::RadToDeg(acosf((offset | vel))));

						float normalisedDist = (lenghtSq / (g_debugShuntHomingDist * g_debugShuntHomingDist));
						float newSpeed = Maths::Clamped(normalisedDist, 0.3f , 1.0f) * g_ShuntMaxHomingSpeed;
						newSpeed += speedOfTarget;
						m_shuntCar.SetTargetSpeed(newSpeed); 
						m_shuntCar.SetMaxTurnRate(g_debugShuntHomingTurnRate * m_CurrentECMHomingTurnScalar); // 40.0 sweeping turns it will miss

						if (m_distanceInfront > 0.0f && m_distanceInfront < g_debugShuntHomingDist)
						{
							m_ClosestApproach = true;
						}

						if (m_ClosestApproach == true && m_distanceInfront < -10.0f)
						{
							// we have been close to the target but now in front, so stop homing and go straight.
							m_ProjectileState = SHUNT_STATE_STAGE_5;
							targetPos = m_shuntCar.GetPosition()+ m_shuntCar.GetVelocity();
							m_shuntCar.SetTargetSpeed(150.0f); 
						}

						float loseHomingDist = g_ShuntLoseHomingDist*g_ShuntLoseHomingDist;
						float predictedTime = g_debugShuntPredictedSeconds;
	
			
						// predicted position.....
						if (lenghtSq < loseHomingDist)
						{
							targetPos = mpRaceLogicLayer->GetLogicVehicleActor(m_targetIndex)->GetWorldMatrix().AW();
							Maths::HVec3 velAfterSecs = targetVel * predictedTime;
							targetPos += velAfterSecs;
							m_ProjectileState = SHUNT_STATE_STAGE_6;
							m_shuntCar.m_targetPos = targetPos;
							//m_shuntCar.SetTargetSpeed(150.0f); 
						}

					}
					else
					{
						// If we have no target ID, go straight....
						m_ProjectileState = SHUNT_STATE_STAGE_5;//SHUNT_STATE_STAGE_3;
						targetPos = m_shuntCar.GetPosition()+ m_shuntCar.GetVelocity();
						m_shuntCar.SetTargetSpeed(150.0f); 
					}
	
					m_shuntCar.m_targetPos = targetPos;

				}
				break;

			case SHUNT_STATE_STAGE_5:
				{
					Maths::HVec3 targetPos;
					m_shuntCar.SetMaxTurnRate(0.f);
					targetPos = m_shuntCar.GetPosition() + m_shuntCar.GetVelocity();
					m_shuntCar.m_targetPos = targetPos;
					m_shuntCar.SetTargetSpeed(150.0f); 
				}
				break;

			case SHUNT_STATE_STAGE_6:
				{
					// shunt target pos is already set
					
					Maths::HVec3 targetPos = mpRaceLogicLayer->GetLogicVehicleActor(m_targetIndex)->GetWorldMatrix().AW();
					Maths::HVec3 targetVel = mpRaceLogicLayer->GetLogicVehicleActor(m_targetIndex)->GetGraphicalActor(m_targetIndex)->GetCollisionBody()->LinearVelocity();
					float speedOfTarget = MPHTOMS(mpRaceLogicLayer->GetLogicVehicleActor(m_targetIndex)->GetPositionInfo().mSpeedInMph);

					Maths::HVec3 offset = targetPos - m_shuntCar.GetPosition();
					Maths::HVec3 vel = m_shuntCar.GetVelocity();
					vel.Normalise();
					float forwardsDist  = offset | vel; // in metres
					offset.Normalise();
					//float angle = Maths::Abs(Maths::RadToDeg(acosf((offset | vel))));
					Maths::HVec3 dir = targetPos -  m_shuntCar.GetPosition();
					float lenghtSq = dir.LengthSquared();

					float normalisedDist = (lenghtSq / (g_debugShuntHomingDist * g_debugShuntHomingDist));
					float newSpeed = Maths::Clamped(normalisedDist, 0.3f , 1.0f) * g_ShuntMaxHomingSpeed;
					newSpeed += speedOfTarget;
					m_shuntCar.SetTargetSpeed(newSpeed); 
					m_shuntCar.SetMaxTurnRate(g_debugShuntHomingTurnRate * m_CurrentECMHomingTurnScalar); // 40.0 sweeping turns it will miss

					if (m_distanceInfront > 0.0f && m_distanceInfront < g_debugShuntHomingDist)
					{
						m_ClosestApproach = true;
					}

					if (m_ClosestApproach == true && m_distanceInfront < -10.0f)
					{
						// we have been close to the target but now in front of the target car, speed up the 
						m_ProjectileState = SHUNT_STATE_STAGE_5;
						m_shuntCar.SetTargetSpeed(150.0f); 
					}

				}
				break;

			case SHUNT_STATE_STAGE_7:
				{

				}
				break;

			case SHUNT_STATE_STAGE_8:
				{

				}
				break;

			case SHUNT_STATE_STAGE_9:
				{

				}
				break;

			default:
				break;

		}


		if (g_debugShuntDrawTarget)
		{
			// debug draw all the positions here
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(m_shuntCar.m_targetPos, 1.5f, Maths::RGBA(255,255,0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(m_shuntCar.m_positionWorld, m_shuntCar.m_targetPos, Maths::RGBA(255,255,0));
			
			if (m_targetIndex != -1)
			{
				Maths::HVec3 tPos = mpRaceLogicLayer->GetLogicVehicleActor(m_targetIndex)->GetWorldMatrix().AW();
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tPos, m_shuntCar.m_targetPos, Maths::RGBA(255,255,0));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(tPos, 1.5f, Maths::RGBA(255,255,0));
			}
		}

		m_shuntCar.Update(frameTime);
		//Maths::HVec3 newPos = m_shuntCar.GetPosition();
		//mMatrix.AW(newPos);

		m_shuntCar.SetPosition(mMatrix.AW());

		mSpeed = m_shuntCar.GetVelocity().Length();
		mMovementDir = Maths::Normalise(m_shuntCar.GetVelocity());

		if (m_collisionCheck)
		{
			if(!CheckCollisions(mPrevPos, 0))
				CheckCollisions(mPrevPos, 1);
		}

		// fill in the orientation
		//Maths::HVec3 dir(mMatrix.AW() - mPrevPos);
		mMatrix.AZ(mMovementDir);
		Maths::HVec3 mySideDirection(mMovementDir.Z(), mMovementDir.Y(), -mMovementDir.X());
		mMatrix.AX(mySideDirection);
		mMatrix.AY(mySideDirection ^ mMovementDir);

		// should really then orthonormalise the matrix to be on the safe side
		Maths::Mat33 newRotation = Maths::OrthoNormaliseZX( mMatrix.Rotation() );
		Maths::HMat33 temp(newRotation);
		mMatrix.AX(temp.AX()); mMatrix.AY(temp.AY()); mMatrix.AZ(temp.AZ());
		
		if (g_debugShuntDrawPositionInfo)
		{
			DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dArrow(mMatrix.AW(), temp.AX(),1.0f, 0.1f, Maths::RGBA(255, 0, 0));	
			DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dArrow(mMatrix.AW(), temp.AY(),1.0f, 0.1f, Maths::RGBA(0, 255, 0));	
			DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dArrow(mMatrix.AW(), temp.AZ(),1.0f, 0.1f, Maths::RGBA(0, 0, 255));	
		}
	}

	void C_PickupShuntProjectile::ShuntState3()
	{
		// can I see target ?????? // infront by AI distance
		m_shuntCar.SetTargetSpeed(150.0f); // max speed....
							
		FindATarget(); // TRack Distance

		if (m_ClosestApproach == true && m_distanceInfront < -10.0f)
		{
			// we have been close to the target but now in front of the target car, speed up the 
			m_ProjectileState = SHUNT_STATE_STAGE_5;
			m_shuntCar.SetTargetSpeed(150.0f); 
		}

		airacingtech::ITrackDataAI *pTrackDataSystem = AIAmax::AmaxAIR::Instance().GetTrackDataSystem();
		if(pTrackDataSystem)
		{
			if (m_targetIndex != -1)
			{
				// Home in on the target....

				// we need to see if we can do a line of sight test to the target
				bool canSee = LineOfSight();
				if (canSee && m_distanceInfront > 0.0f && m_distanceInfront < g_debugShuntHomingDist)
				{
					m_ProjectileState = SHUNT_STATE_STAGE_4;
				}
			}
			else
			{
				// go straight if we don`t have a target
				m_ProjectileState = SHUNT_STATE_STAGE_5;

			}
			
			if (m_CurrentRoute.m_pRoute && m_CurrentRoute.m_Index != -1)
			{
				// this should be safe
				Maths::HVec3 targetPos;
				bool status = false;
				const int infrontDist = 16;//10.0f; // 14 metres, increase
				int currentIndex = m_CurrentRoute.m_pRoute->GetIndexOffsetByDistance(m_CurrentRoute.m_Index, (float)infrontDist, airacingtech::AITrackDataLines::TRACK_DATA_LINE_RACING, status);	
				targetPos = m_shuntCar.GetPosition();
				float offset = pTrackDataSystem->WorkOutOffsetToRacingSpline(m_CurrentRoute.m_pRoute, targetPos, currentIndex);
				m_CurrentRoute.m_pRoute->GetPositionOffsetLine(offset, currentIndex , targetPos, airacingtech::AITrackDataLines::TRACK_DATA_LINE_RACING);	
				//m_CurrentRoute.m_pRoute->GetPositionFromLine(airacingtech::AITrackDataLines::TRACK_DATA_LINE_RACING, currentIndex , targetPos);
				m_shuntCar.m_targetPos = targetPos;
			}
			else
			{
				Maths::HVec3 targetPos;
				targetPos = m_shuntCar.GetPosition() + m_shuntCar.GetVelocity();
				m_shuntCar.m_targetPos = targetPos;
			}
		}
		else
		{
			if (m_targetIndex != -1)
			{
				// Home in on the target....

				// we need to see if we can do a line of sight test to the target
				bool canSee = LineOfSight();
				
				if (canSee)
				{
					m_ProjectileState = SHUNT_STATE_STAGE_4;
				}

				m_shuntCar.m_targetPos = mpRaceLogicLayer->GetLogicVehicleActor(m_targetIndex)->GetWorldMatrix().AW();
			}
			else
			{
				m_ProjectileState = SHUNT_STATE_STAGE_5;
			}
		}
	}

	bool C_PickupShuntProjectile::LineOfSight()
	{
		bool canSee = false;
		
		if (m_targetIndex != -1 && mTargetIdx != -1 && mpRLC)
		{
			
			Maths::HVec3 targetPos = mpRaceLogicLayer->GetLogicVehicleActor(mTargetIdx)->GetWorldMatrix().AW();
			Maths::HVec3 startPos = m_shuntCar.GetPosition();

			Collision::C_Ray ray;
			U8 data[1024 * sizeof(void*)];
			SharedPhysics::C_SafeStack<true> stack(data, 1024);
		
			S32 vehGroupId = mpRaceLogicLayer->GetLogicVehicleActor(mFiringIdx)->GetGraphicalActor(mFiringIdx)->GetPhysicalGroupID();

			Collision::C_CollisionFilter raySolidFilter;
			raySolidFilter.SetBodyMask(PolyFlags::kbWorld);//PolyFlags::kbPerk | PolyFlags::kbVehicle | PolyFlags::kbWorld;
			raySolidFilter.SetGroupId(vehGroupId);
			raySolidFilter.SetGroupFlags( 0 );

			float dist = Maths::Distance(startPos, targetPos);
			Maths::HVec3 direction = Maths::Normalise(targetPos - startPos);
			Maths::C_Normal normal(direction);

			ray.InitialiseRay(startPos, normal, dist, &raySolidFilter, 0.1f);
			ray.SetMaxCosAngle(-0.5f);
			//NB - only interested in if we can get stopped by static mesh - this will mean that its a lot quicker again!
			mpRLC->GetPhysicsScene()->DoRayTraceMesh(ray, stack);
			
			//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(oldPos, oldPos + (normal * dist), Maths::RGBA(255,255,0));
			canSee = !ray.m_result.Hit();
		}

		return canSee;
	}

	void C_PickupShuntProjectile::FindATarget()
	{
		if (m_targetIndex == -1 && mTargetIdx != -1)
		{
			m_targetIndex = mTargetIdx;
		}

		if (m_targetIndex != -1 && mTargetIdx != -1)
		{
			airacingtech::ITrackDataAI *pTrackDataSystem = AIAmax::AmaxAIR::Instance().GetTrackDataSystem();

			airacingtech::ISplineTracker *pFiringSplineTracker = AIAmax::AmaxAIR::Instance().GetAISplineTracker(mFiringIdx);
			
			C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			if (pFiringSplineTracker && pTrackDataSystem && pLayer)
			{
				Actors::C_VehicleLogicActor* pTargetActor = pLayer->GetLogicVehicleActor(mTargetIdx);

				// get the ai spline tracker.......
				airacingtech::ISplineTracker *pTargetSplineTracker = AIAmax::AmaxAIR::Instance().GetAISplineTracker(mTargetIdx);
				if (pTargetSplineTracker)
				{	
					if (pTargetSplineTracker->GetLogicRoute())
					{
						int targetLogicIndex = pTargetSplineTracker->GetLogicRoute()->m_index;

						float targetLogicPos = pTrackDataSystem->CalculateLogicPositionFromIndex(targetLogicIndex);

						// infront or behind
						m_distanceInfront = AIAmax::AmaxAIR::Instance().DistanceInfrontBehindLogicCalculationParamatised(m_LogicRoute.m_IndexPos, targetLogicPos);

					}
				}
			}
		}

#if 0
		if (m_targetIndex == -1 && mTargetIdx != -1)
		{
			airacingtech::ITrackDataAI *pTrackDataSystem = AIAmax::AmaxAIR::Instance().GetTrackDataSystem();

			airacingtech::ISplineTracker *pFiringSplineTracker = AIAmax::AmaxAIR::Instance().GetAISplineTracker(mFiringIdx);
			if (pFiringSplineTracker && pTrackDataSystem)
			{
				float minDist = FLT_MAX;

				C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
				for( int i = 0; i < pLayer->GetNumVehicles(); i++ )
				{
					if( mFiringIdx != i && mTargetIdx == i)
					{
						Actors::C_VehicleLogicActor* pTargetActor = pLayer->GetLogicVehicleActor(i);

						// get the ai spline tracker.......
						airacingtech::ISplineTracker *pTargetSplineTracker = AIAmax::AmaxAIR::Instance().GetAISplineTracker(i);
						if (pTargetSplineTracker)
						{	
							if (pTargetSplineTracker->GetLogicRoute())
							{
								int targetLogicIndex = pTargetSplineTracker->GetLogicRoute()->m_index;

								float targetLogicPos = pTrackDataSystem->CalculateLogicPositionFromIndex(targetLogicIndex);

								// infront or behind
								float distInfrontBehind = AIAmax::AmaxAIR::Instance().DistanceInfrontBehindLogicCalculationParamatised(m_LogicRoute.m_IndexPos, targetLogicPos);

								// work out the cone angle..........
								
								//if (distInfrontBehind > 20.0f) // reduce this
								{
									Maths::HVec3 tPos = pTargetActor->GetWorldMatrix().AW();

									// check target angle
									Maths::HVec3 offset = tPos - m_shuntCar.GetPosition();
									Maths::HVec3 vel = m_shuntCar.GetVelocity();
									vel.Normalise();

									float forwardsDist  = offset | vel; // in metres
									offset.Normalise();
									float angle = Maths::Abs(Maths::RadToDeg(acosf((offset | vel))));

									int debugStop = 0;
									if ((angle < m_coneAngle && forwardsDist > 8.0f && forwardsDist < 50.0f) ||
										(angle < m_coneAngle*0.25f && forwardsDist > 8.0f && forwardsDist < 70.0f))
									{
										if (distInfrontBehind < minDist)
										{
											minDist = distInfrontBehind;
											m_targetIndex = i;
										}
									}
								}
							}
						}
					}
				}
			}
		}
#endif
	}

	//void C_PickupShuntProjectile::AlignProjectileToGroundFirstPart(Maths::HVec3 old, Maths::HVec3 pos)
	//{
	//	float frameTime = C_GameApp::GetInstance()->GetTimeStep();
	//	Collision::C_Ray ray;
	//	U8 data[1024 * sizeof(void*)];
	//	SharedPhysics::C_SafeStack<true> stack(data, 1024);
	//	const Maths::HVec3 down(0.f, -1.f, 0.f);
	//	const Maths::HVec3 up(0.f, 1.f, 0.f);

	//	// only interested in the dynamic surface.....
	//	Collision::C_CollisionFilter rayFilter;
	//	rayFilter.m_CollisionMask = PolyFlags::kWheelSolid;
	//	rayFilter.m_BodyMask = PolyFlags::kbWorld;

	//	ray.InitialiseRay(old, pos , &rayFilter);
	//	mpRLC->GetPhysicsScene()->DoRayTrace(ray, stack);
	//	Collision::C_RayTracingResult& result = ray.m_result;

	//	if( result.Hit() )
	//	{
	//		Maths::HVec3 newPos = result.HitPoint();

	//		Maths::HVec3 shuntPos = mMatrix.AW();
	//		shuntPos.Y(newPos.Y() + mRadius);
	//		mMatrix.AW(shuntPos);	
	//	}	
	//	
	//}

	//float C_PickupShuntProjectile::AlignProjectileToGround(Maths::HVec3 pos)
	//{
	//	float yheight = FLT_MIN;

	//	float frameTime = C_GameApp::GetInstance()->GetTimeStep();

	//	Collision::C_Ray ray;

	//	U8 data[1024 * sizeof(void*)];
	//	SharedPhysics::C_SafeStack<true> stack(data, 1024);
	//	const Maths::HVec3 down(0.f, -1.f, 0.f);
	//	const Maths::HVec3 up(0.f, 1.f, 0.f);

	//	// only interested in the dynamic surface.....
	//	Collision::C_CollisionFilter rayFilter;
	//	rayFilter.m_CollisionMask = PolyFlags::kWheelSolid;
	//	rayFilter.m_BodyMask = PolyFlags::kbWorld;

	//	Maths::HVec3 end;
	//	Maths::HVec3 start(pos + (up * (0.5f + 0.5 + mRadius))); // should always hit
	//	ray.InitialiseRay(start, Maths::C_Normal(down, Maths::C_Normal::UNIT_LENGTH_VECTOR), Maths::HF32((150.0f)), &rayFilter, Maths::HF32(0.1f));
	//	mpRLC->GetPhysicsScene()->DoRayTrace(ray, stack);
	//	Collision::C_RayTracingResult& result = ray.m_result;

	//	if( result.Hit() )
	//	{
	//		Maths::HVec3 newPos = result.HitPoint();
	//		yheight = newPos.Y();
	//	}	
	//	else
	//	{
	//		// We might have ended up under the world with out move, so lets try and save ourselves.
	//		Maths::HVec3 end;
	//		Maths::HVec3 start; 

	//		bool secondaryHit = false;
	//		for(int i=1;i<30; i++)
	//		{
	//			start = pos + (up * i * 1);
	//			end = pos + (down * i * 0.1f);
	//			//start = pos + (up * 1.5f);
	//			//end = pos + (down * 1.0f);

	//			ray.InitialiseRay(start, end, &rayFilter);
	//			mpRLC->GetPhysicsScene()->DoRayTrace(ray, stack);
	//			Collision::C_RayTracingResult& secondRayRes = ray.m_result;

	//			if( secondRayRes.Hit() )
	//			{
	//				Maths::HVec3 newPos = result.HitPoint();
	//				yheight = newPos.Y();
	//				secondaryHit = true;
	//				//DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dStar(newPos, 0.2f, Maths::RGBA(0, 0, 255));
	//				break;
	//			}
	//		}
	//	}


	//	return yheight;
	//}



	void C_PickupShuntProjectile::UpdateRoute(ShuntRouteInfo *pShuntRoute)
	{
			
	}

	void C_PickupShuntProjectile::FindNearest(ShuntRouteInfo *pShuntRoute, Maths::HVec3 pos)
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		float paramDist = zero;

		if (pShuntRoute->m_Index == -1)
		{
			// TODO : use the firing vehicles Logic route index
			pShuntRoute->m_Index = pShuntRoute->m_pRoute->FindNearestQuick(1, pShuntRoute->m_pRoute->GetNumSegments()-1, pos,paramDist, 3, pShuntRoute->m_pRoute->GetStepSize());
		}
		else
		{
			int startIndex = pShuntRoute->m_pRoute->GetIndexOffset(pShuntRoute->m_Index, -5);//-5); // probably get away with 5
			int endIndex = pShuntRoute->m_pRoute->GetIndexOffset(pShuntRoute->m_Index, 12);//5);

			if (pShuntRoute->m_pRoute->IsLoopedTrack() & (startIndex > endIndex))
			{
				int st = startIndex;
				int ed = pShuntRoute->m_pRoute->GetNumSegments() - 1;
				pShuntRoute->m_Index =  pShuntRoute->m_pRoute->FindNearest( st, ed, pos, paramDist, pShuntRoute->m_pRoute->GetStepSize());
				if (pShuntRoute->m_Index == -1)
				{
					st = 1;
					ed = endIndex;
					pShuntRoute->m_Index =  m_LogicRoute.m_pRoute->FindNearest( st, ed, pos, paramDist, pShuntRoute->m_pRoute->GetStepSize());
				}
			}
			else
			{
				startIndex = Maths::Max(startIndex-1, 1);
				pShuntRoute->m_Index =  pShuntRoute->m_pRoute->FindNearest( startIndex, endIndex, pos, paramDist, pShuntRoute->m_pRoute->GetStepSize());
			}					
		}
		

	}

	void C_PickupShuntProjectile::TrackAILogicSpline()
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		Maths::HF32 paramDist = zero;

		//Grab the required AI Data
		airacingtech::ITrackDataAI *pTrackData = AIAmax::AmaxAIR::Instance().GetTrackDataSystem();
		if (pTrackData)
		{
			m_LogicRoute.m_pRoute = pTrackData->GetTrackData()->m_TrackLogicData.m_pLogicRoute;
				
			if (m_LogicRoute.m_pRoute)
			{
				Maths::HVec3 pos = m_shuntCar.GetPosition();
			
				FindNearest(&m_LogicRoute, pos);
				
				if (m_LogicRoute.m_Index != -1)
				{
					m_LogicRoute.m_IndexPos = pTrackData->CalculateLogicPositionFromIndex(m_LogicRoute.m_Index);

					// debug draw the segment here.........
					DebugDrawSegment(m_LogicRoute.m_pRoute, m_LogicRoute.m_Index, Maths::RGBA(0x00, 0x00, 0xff), pos.Y());
				}
			}
		}
	}

	void C_PickupShuntProjectile::UpdateCurrentRoute()
	{
		Maths::HVec3 pos = m_shuntCar.GetPosition();

		airacingtech::ITrackDataAI *pTrackData = AIAmax::AmaxAIR::Instance().GetTrackDataSystem();

		if (m_LogicRoute.m_pRoute && m_LogicRoute.m_Index != -1 && pTrackData)
		{
			// work out or choose our current route.....
			// Darren....
			
			if (!m_CurrentRoute.m_pRoute)
			{
				const airacingtech::AITrackDataCache& dc = m_LogicRoute.m_pRoute->m_dataCache[m_LogicRoute.m_Index];
				const int noEntries = dc.m_data.Size();	

				for(int entries = 0; entries <noEntries; entries++ )
				{
					const airacingtech::AITrackDataCachedData& cachedData = dc.m_data[entries];

					// check that we are in it
					if (cachedData.m_pRoute && cachedData.m_pRoute->IsChangeableRoute())
					{
						// we have found one valid route....
						// More check here please
						m_CurrentRoute.m_pRoute = cachedData.m_pRoute;
						m_CurrentRoute.m_Index = cachedData.m_index;

						FindNearest(&m_CurrentRoute, pos);

						// cached check
						if (m_CurrentRoute.m_Index != -1)
						{
							int diff = (m_CurrentRoute.m_pRoute->GetNumSegments()-1) - m_CurrentRoute.m_Index;
							if (diff > 4)
							{
								break;
							}
							else
							{
								m_CurrentRoute.Reset();
							}
						}
					}
				}
			}
			else
			{
				FindNearest(&m_CurrentRoute, pos);
				int diff = (m_CurrentRoute.m_pRoute->GetNumSegments()-1) - m_CurrentRoute.m_Index;
				if (diff < 4 || m_CurrentRoute.m_Index == -1)
				{
					m_CurrentRoute.Reset();
				}
			}

			if (m_CurrentRoute.m_Index != -1)
			{
				// debug draw the segment here.........
				DebugDrawSegment(m_CurrentRoute.m_pRoute, m_CurrentRoute.m_Index, Maths::RGBA(0xff, 0x00, 0x00), pos.Y());
			}
			else
			{
				if (g_debugShuntDrawPositionInfo)
				{
					Maths::HVec3 persistPos(pos);
					persistPos.Y(pos.Y() + 0.5f);

					DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dStar(persistPos, 0.2f, Maths::RGBA(0, 0, 255));	
				}
				
			}

		}
	}

	void C_PickupShuntProjectile::UpdateTargetRoute()
	{
		Maths::HVec3 pos = m_shuntCar.GetPosition();
		bool found = false;

		airacingtech::ITrackDataAI *pTrackData = AIAmax::AmaxAIR::Instance().GetTrackDataSystem();

		if (mTargetIdx != -1 && pTrackData) // get the spline tracker for this
		{
			airacingtech::ISplineTracker *pTargetSplineTracker =  AIAmax::AmaxAIR::Instance().GetAISplineTracker(mTargetIdx);
			if (pTargetSplineTracker)
			{
				const Btl::Vector<airacingtech::RouteInfo> &routeInfo = pTargetSplineTracker->GetRouteInfo();

				for(int k=0; k<routeInfo.Size(); k++)
				{
					const airacingtech::RouteInfo *pRouteInfo = &routeInfo[k];
					if (pRouteInfo->m_in)
					{
						// now loop around the routes we are in and update.....
						if (m_LogicRoute.m_Index != -1)
						{
							const airacingtech::AITrackDataCache& dc = m_LogicRoute.m_pRoute->m_dataCache[m_LogicRoute.m_Index];
							const int noEntries = dc.m_data.Size();	

							for(int entries = 0; entries <noEntries; entries++ )
							{
								const airacingtech::AITrackDataCachedData& cachedData = dc.m_data[entries];

								// check that we are in it
								if (cachedData.m_pRoute && cachedData.m_pRoute->IsChangeableRoute() && cachedData.m_pRoute == pRouteInfo->m_pRoute)
								{
									// now check we are in it.....
									m_CurrentRoute.m_pRoute = cachedData.m_pRoute;
									m_CurrentRoute.m_Index = cachedData.m_index;

									FindNearest(&m_CurrentRoute, pos);
									
									if (m_CurrentRoute.m_Index != -1)
									{
										int diff = (m_CurrentRoute.m_pRoute->GetNumSegments()-1) - m_CurrentRoute.m_Index;
										if (diff > 4)
										{
											found = true;
											break;
										}
										else
										{
											m_CurrentRoute.Reset();
										}
									}
									else
									{
										m_CurrentRoute.Reset();
									}
								}
							}
						}
					}

					if (found)
					{
						break;
					}

				}
			}
		}
	}


	void C_PickupShuntProjectile::DebugDrawSegment(airacingtech::AITrackDataRoutes *pRoute,  int index, Maths::RGBA colour, float y)
	{
		if (pRoute && index != -1 && g_debugShuntDrawPositionInfo)
		{
			Maths::HVec3 BottomLeft, TopLeft, BottomRight, TopRight;

			if (index >= 1)
			{

				DbUtil::DefaultDebugRenderSys()->Primitive()->SetDepthEnable(false);
				pRoute->GetPositionFromLine(airacingtech::AITrackDataLines::TRACK_DATA_EXTREME_LEFT, index , TopLeft);
				pRoute->GetPositionFromLine(airacingtech::AITrackDataLines::TRACK_DATA_EXTREME_LEFT, index-1 , BottomLeft);
				pRoute->GetPositionFromLine(airacingtech::AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, index , TopRight);
				pRoute->GetPositionFromLine(airacingtech::AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, index-1 , BottomRight);

				TopLeft.Y(y);	BottomLeft.Y(y);	TopRight.Y(y);	BottomRight.Y(y);

				DbUtil::DefaultDebugRenderSys()->Primitive()->Draw3dLine( TopLeft, BottomLeft, colour );
				DbUtil::DefaultDebugRenderSys()->Primitive()->Draw3dLine( TopLeft, BottomLeft, colour );
				DbUtil::DefaultDebugRenderSys()->Primitive()->Draw3dLine( BottomLeft, BottomRight, colour );
				DbUtil::DefaultDebugRenderSys()->Primitive()->Draw3dLine( BottomRight, TopRight, colour );
				DbUtil::DefaultDebugRenderSys()->Primitive()->Draw3dLine( TopRight, TopLeft, colour );
				DbUtil::DefaultDebugRenderSys()->Primitive()->SetDepthEnable(true);


			}
		}
	}



	void C_PickupShuntProjectile::SendShuntEvadedScoringMessageToVehicle(S32 vehicleIndex)
	{
		if(vehicleIndex != -1)
		{
			Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehicleIndex);
			BIZ_ASSERT_MSG(pLogicActor, "Shunt Evader Failed to get pLogicactor");

			C_VehicleLogicMsg_EvadedPowerUp msg(vehicleIndex, mFiringIdx, E_PICKUP_TYPE_SHUNT, mUniqueId);
			pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_NewScoring, &msg);
		}
	}

	void C_PickupShuntProjectile::PhotoMove(F32 frac)
	{
		using namespace Maths;
		
		HVec3 movement = mMatrix.AW() - m_PrevPos;

		HMat43 tweakMatrix(mMatrix);
		tweakMatrix.AW(tweakMatrix.AW() + movement * frac);

		C_InGameSpecialFXManager::Instance().UpdatePerkMatrix(mUniqueId, tweakMatrix, true);
	}
}




/*	-----------------------
	PROTYTYPE OF TUNNEL HIT
	-----------------------

	Debug::Float g_DebugShuntEffectsFirstSpinTime("Perks.Shunt.Effects.FirstSpinTime", 0.35f);
	Debug::Float g_DebugShuntEffectsSecondSpinTime("Perks.Shunt.Effects.SecondSpinTime", 2.0f);
	Debug::Float g_DebugShuntEffectsFlipTime("Perks.Shunt.Effects.FlipTime", 2.0f);

	Debug::Float g_debugRotTest("Perks.Shunt.Effects.RotTest", 0.2f);
	Debug::Float g_debugRotTestCooldown("Perks.Shunt.Effects.RotTestCooldown", 0.95f);

	C_PickupEffectsShunt::C_PickupEffectsShunt(E_PICKUP_TYPE type, int vehIdx)
		: C_PickupLocalEffectsBase(type, vehIdx, 0.f), mSequence(0)
	{
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mVehicleIndex);
		Actors::C_VehicleActor*	pVehActor	= pActor->GetGraphicalActor(mVehicleIndex);

		if(pVehActor->GetFakePhysics()->IsActive())
			mSequence = 4;

		mCooldown = g_debugRotTestCooldown;
	}

	bool C_PickupEffectsShunt::Update()
	{
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mVehicleIndex);
		Actors::C_VehicleActor*	pVehActor	= pActor->GetGraphicalActor(mVehicleIndex);
		float frameTime						= C_GameApp::GetInstance()->GetTimeStep();

		mTimer -= frameTime;

		switch(mSequence)
		{
		case 0:
			{
				//pVehActor->GetFakePhysics()->Start(Actors::E_FAKE_PHYSICS_SPIN_90_LEFT);
				pVehActor->GetFakePhysics()->Start(Actors::E_FAKE_PHYSICS_ROLL_LEFT);

				mSequence = 1;
				mTimer = g_DebugShuntEffectsFirstSpinTime;
				break;
			}

		case 1:
			{

				SharedPhysics::C_RigidBody& rb = pVehActor->GetCollisionBody()->GetBody();
				Maths::HMat33 rotMat = Maths::HMat33(Maths::RotY(-g_debugRotTest * mCooldown));

				mCooldown *= g_debugRotTestCooldown;

				rb.m_OrientationMatrix	= rb.m_OrientationMatrix * rotMat;
				rb.m_Orientation		= Maths::C_Quaternion(rb.m_OrientationMatrix);

				if(mTimer <= 0.f)
				{
					//pVehActor->GetFakePhysics()->Stop();
					//pVehActor->GetSecondaryFakePhysics()->Start(Actors::E_FAKE_PHYSICS_SPIN_90_RIGHT);

					mSequence = 3;
					mTimer = g_DebugShuntEffectsFlipTime;
				}

				break;
			}

		case 2:
			{
				if(mTimer <= 0.f)
				{
					pVehActor->GetFakePhysics()->Stop();
					pVehActor->GetFakePhysics()->Start(Actors::E_FAKE_PHYSICS_SPIN_90_RIGHT);
					mSequence = 3;
					mTimer = g_DebugShuntEffectsSecondSpinTime;
				}
				break;
			}

		case 3:
			{
				SharedPhysics::C_RigidBody& rb = pVehActor->GetCollisionBody()->GetBody();
				Maths::HMat33 rotMat = Maths::HMat33(Maths::RotY(g_debugRotTest * mCooldown));

				mCooldown *= g_debugRotTestCooldown;

				rb.m_OrientationMatrix	= rb.m_OrientationMatrix * rotMat;
				rb.m_Orientation		= Maths::C_Quaternion(rb.m_OrientationMatrix);

				if(mTimer <= 0.f)
				{
					pVehActor->GetFakePhysics()->Stop();
					mSequence = 4;
				}
				break;
			}

		default:
			{
				return true;
			}
		}

		return false;
	}
}

static void Vehicle0HitByShuntFunc(void *)
{
	Game::C_RaceLogicLayer* pLayer						=	C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	Game::Actors::C_VehicleLogicActor* pActor			= pLayer->GetLogicVehicleActor(0);
	Components::C_VehicleComponent_Pickups* pPickups	= (Components::C_VehicleComponent_Pickups*)pActor->QueryInterface(Components::E_IF_VehicleComponent_Pickups);

	Game::PickupHitInfo hitInfo;
	pPickups->RegisterHitByPickup(E_PICKUP_TYPE_SHUNT, E_PICKUP_SUB_TYPE_NORMAL, E_PICKUP_HIT_TYPE_MAJOR, -1, -1, E_TARGETTING_DIRECTION_NONE, false, hitInfo, 0);
}
Debug::Function	g_debugHitVehicle0WithShunt("Perks.Shunt.Effects.HitVehicle0", Vehicle0HitByShuntFunc, 0);

*/
