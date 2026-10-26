#include "stdafx.h"

#include <Game/Game.h>
#include <Game/Messaging/GameMessages.h>
#include <Game/Messaging/GameflowMessages.h>
#include <Game\Messaging\VehicleActorMessages.h>
#include <Game\GameActors\Components\GameComponents.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include "VLC_HandOfGod.h"

#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <GameLogic\RaceLogicCore\Core\RaceLogicCore.h>
#include <Game\GameActors\Vehicles\VehicleActor.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game\AmaxCam\CameraSystem.h>

#include <ai/AI_Amax.h>
#include <GameApp.h>
#include <game/GameDef/Polyflags.h>
#include <Game/GameControls/GameControls.h>
#include <ui\Game\Gadgets\ButtonController.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>

#include "Automation\GameDataLogging.h"

#include "Ui/UiHelpers.h"
#include <Ui/Game/Gadgets/ButtonMessages.h>

#include <GameAudio/GameAudioLayer.h>
#include <GameAudio/SystemSubsystem.h>

Debug::Bool g_debugEnableHandOfGod("Vehicle.Components.HandOfGod.Enabled",	 true, Debug::Persist());

static void ForcedHoGReset(void *)
{
	Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(0);
	const Game::Actors::C_VehicleResetAtVehiclePositionMsg resetMsg(0);
	pLogicActor->GetActorList()->SendMessageToActorComponent(pLogicActor->GetHandle(),Components::E_IF_VehicleComponent_HandOfGod,&resetMsg);		
}
Debug::Function g_debugTriggerHoG("Vehicle.Components.HandOfGod.TriggerHandOfGod", ForcedHoGReset, 0);
Debug::Float g_HoGSloMoCutOutTime("Vehicle.Components.HandOfGod.SloMoCutOutTime", 1.35f);
Debug::Float g_HoGSnapCameraTime("Vehicle.Components.HandOfGod.SnapCameraTime", 0.9f);

Debug::Logger &g_RLHandOfGodLogger( Debug::GetLogger("Amax.VLC.HandOfGod") );
Debug::Logger &g_RLHandOfGodLoggerAI( Debug::GetLogger("Amax.VLC.HandOfGod_AI") );

extern Debug::Float g_DefaultGhostAlpha;

namespace Components
{

	bool C_VehicleComponent_HandOfGod::CreateImpl()
	{
		const I_VehicleComponent_HandOfGod_CreateInfo* creation_data = static_cast<const I_VehicleComponent_HandOfGod_CreateInfo *>(GetCreator().GetData());

		if(creation_data)
		{
			mActorHandle=creation_data->mParentActorHandle;
			mVehicleIndex=creation_data->mVehicleIndex;
			mViewportIndex=creation_data->mViewportIndex;
			mContinueDespiteNoSplineData=creation_data->mContinueDespiteNoSplineData;
			mpHoGParams = creation_data->mpHoGParams;
		}

		mpLevelState = &Game::C_Game::GetInstance()->GetLevelState();
		BIZ_ASSERT(mpLevelState);

		if(mContinueDespiteNoSplineData)
			mResetCriteria = E_HandOfGodResetCriteria_Flipped | E_HandOfGodResetCriteria_StuckOffGround;
		else
			mResetCriteria = mpLevelState->mHoGResetCriteria;

		//AddSubscriber( GameMessaging::E_GameMsg_Update );
		AddSubscriber( Game::E_VehicleMessage_ResetOnTrack );
		AddSubscriber( Game::E_VehicleMessage_ResetAtVehiclePosition );
		AddSubscriber( Game::E_GameflowMessage_RaceStarted );
		AddSubscriber( Game::E_GameflowMessage_RaceFinished );
		AddSubscriber( Game::E_VehicleMessage_ResetEnteredCameraZone );
		AddSubscriber( Game::E_VehicleMessage_ResetExitedCameraZone );
		AddSubscriber( Game::E_VehicleMessage_ResetEntryZone );
		AddSubscriber( Game::E_VehicleMessage_ResetExitZone );
		AddSubscriber( Game::E_VehicleMessage_ResetTwoWayZone );
		AddSubscriber( Game::E_VehicleMessage_ResetAfterWrecked );
		AddSubscriber( Game::E_GameLogicMessage_Impact );
		AddSubscriber( Game::E_VehicleMessage_FatallyDamaged );
		AddSubscriber( E_VehicleLogicMsg_HitByPerk );
		AddSubscriber( ActorMsg::E_DynamicObjMsg_SetVehicleLogic );
		AddSubscriber( Game::E_VehicleMessage_Eliminated );
		AddSubscriber( Game::E_VehicleMessage_EnableDisableAutoHOGPlacement );

		mFlippedTime = mResetZoneTimer = mCollidedRecentlyTimer = mHandOfGodResetWarningTimer = mFacingForwardsTimer =
		mTimeSinceLastReset = mStuckTime = mHitByPerkRecentlyTimer = mCurrentlyResettingTimer = mTumbledTime = mFacingForwardDelay = 0.f;
		
		mResetZoneId = mResetIndivId = mForcedResetZone = mForcedResetZoneBuffered = 0;

		mEnabled = mResetZoneActive = mVehicleImpactThisFrame = mCollidedRecently = mCurrentlyResetting = mFacingForwardDelayGranted =  
		mNeedsHandOfGodResetWarning = mStuckRecently = mDoingHOGResetCam = mHasBeenReset = mResetHoGCam = 
		mForcedResetCameraTrackingBuffered = mBeingResetAfterBeingWrecked = false;

		mResetType = E_HandOfGodResetType_AtSpeedOnSpline;

		mResetDelayTimer=0.f;

		mDiedBySuicide = false;

		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		mpRLC										= pActor->GetRLC();
		BIZ_ASSERT(mpRLC);

		mIsAIControlled								= mpRLC->IsVehicleAIControlled(mVehicleIndex);

		mAutomaticHOGPlacementEnabled = true;
		mManualHOGResetRequired		  = false;

		return true;
	}

	bool C_VehicleComponent_HandOfGod::IsManualHOGResetRequired() const
	{
		return mManualHOGResetRequired;
	}

	void C_VehicleComponent_HandOfGod::SetManualHOGResetRequired(const bool required)
	{
		mManualHOGResetRequired = required;
	}

	void C_VehicleComponent_HandOfGod::Update()
	{
		PERF_EVENT("VLC.HandOfGod.Update");

		if( !mEnabled )
			return;

		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		mIsAIControlled								= mpRLC->IsVehicleAIControlled(mVehicleIndex);
	
		if(mCurrentlyResetting)
		{
			if(mViewportIndex > -1)
			{
				// Stop Slo-Mo
				if(mCurrentlyResettingTimer < g_HoGSloMoCutOutTime && mpRLC->IsVehiclePlayerControlled(mVehicleIndex) && C_GameApp::GetInstance()->GetTimeScale() < 0.95f)
				{
					C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->ExitSloMo(mViewportIndex); 
				}

				// Reset camera
				if(mCurrentlyResettingTimer < g_HoGSnapCameraTime && mpRLC->IsVehiclePlayerControlled(mVehicleIndex) & !mResetHoGCam)
				{
					StopHoGCam();
					mResetHoGCam = true;
				}

			}
			if(!mHasBeenReset && mCurrentlyResettingTimer <= 1.0f)
			{
				if(mContinueDespiteNoSplineData&&mResetType!=E_HandOfGodResetType_AtSpeedAtPoint&&mResetType!=E_HandOfGodResetType_StationaryAtPoint)
				{
					bool atSpeed;
					mResetPosition = pActor->GetWorldMatrix().AW();
					mResetDirection = pActor->GetWorldMatrix().AZ();
					if(mResetType==E_HandOfGodResetType_AtSpeedOnSpline)
					{
						atSpeed = true;
						mResetSpeed = MPHTOMS(mpLevelState->mHoGNoSplineResetSpeedInMPH);
					}
					else
					{
						atSpeed = false;
						mResetSpeed = 0.f;
					}
					ResetVehicleAtVehiclePosition(mVehicleIndex, true, atSpeed, true);
				}
				else
				{
					::Game::Actors::C_VehicleLogicActor* pLogicActor = ((::Game::Actors::C_VehicleLogicActor*)GetActor());

					pLogicActor->mStatistics.mNumHoGTimes[mResetType]++;

					switch(mResetType)
					{
					case E_HandOfGodResetType_AtSpeedOnSpline:
						ResetVehicleOnTrack(false, true);
						break;

					case E_HandOfGodResetType_AtSpeedAtPoint:
						ResetVehicleOnTrack(false, true, true);
						break;

					case E_HandOfGodResetType_StationaryOnSpline:
						ResetVehicleOnTrack(false, false, false);
						break;

					case E_HandOfGodResetType_StationaryAtPoint:
						ResetVehicleOnTrack(false, false, true);
						break;
					}
				}

				mHasBeenReset = true;
				mResetHoGCam = false;

				if(mBeingResetAfterBeingWrecked)
				{
					//// Play a special FX
					//RLPickupSpecialFX sfx;

					//sfx.mLocalSpecialFX=C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetDatabase()->mRLSystems[0]->mPickupSystem->mParameters->mGarageSpecialFX;
					//sfx.mExternalSpecialFX=sfx.mLocalSpecialFX;
					//sfx.mDurationLoop.mStart=sfx.mDurationLoop.mEnd=0;
					//sfx.mDurationIntro.mStart=0.0f;sfx.mDurationIntro.mEnd=5;
					//sfx.mDurationOutro.mStart=sfx.mDurationOutro.mEnd=0;
					//sfx.mFrequency=1.0f;
					//sfx.mSpecialFXLocation = E_PICKUP_SPECIALFX_LOCATION_ACTINGCAR;

					//Game::C_InGameSpecialFXManager::Instance().SetUpVehicleSFX(sfx,pActor->GetWorldMatrix(),mVehicleIndex,-1,Game::E_InGameSpecialFX_Trigger_Repairing, -1.f, Maths::HVec3(1,1,1), Maths::HVec3());

					// Send a repair message to fully repair this vehicle
					// 0 fully repairs the vehicle
					C_VehicleLogicRepairMsg repairVehicleMsg(mVehicleIndex,0); 

					// ONLY send to damage component.. no scoring for this one. 
					pActor->SendMessageToComponent(E_IF_VehicleComponent_Damage, &repairVehicleMsg);

					if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork())
					{
						U8 joinId = mpLevelState->mCars[mVehicleIndex].mNetJoinId;
						Game::C_Game::GetInstance()->GetNetGameVars().m_EventManager->Add_Repaired(joinId);
					}
				}

				Game::Actors::C_VehicleActor* pVehActor	= pActor->GetGraphicalActor(mVehicleIndex);
				if(mpLevelState->mNumViewports == 1 && pVehActor && pVehActor->IsFocusVehicle())
				{
					// fade in sound effects
					Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eFadeInSfx);
				}
			}

			if(!mIsAIControlled)
			{
				if(mCurrentlyResettingTimer > 0.0f)
					mCurrentlyResettingTimer -= C_GameApp::GetInstance()->GetTimeStep();
				else
				{
#ifndef CFG_GOLD
					// For automation, don't put back to Human control again
					if (!mpLevelState->mCars[pActor->mVehicleIndex].mAutomatedHuman)
#endif
					{
						Game::Actors::C_VehicleForceHumanControlMsg controlMsg(pActor->mVehicleIndex);
						
						/*[ChadW] - BroadcastMessage_CleanUp*///pActor->GetActorList()->GetSubscriberSystem().BroadcastMessage(&controlMsg);
						pActor->GetGraphicalActor(mVehicleIndex)->SendMessageToActor(pActor->GetGraphicalActor(mVehicleIndex)->GetHandle(), &controlMsg);
					}

					pActor->mTemporaryAIControl = false;

					mCurrentlyResetting = false;
				}
			}
			else
				mCurrentlyResetting = false;
		}
		else
		{
			if(g_debugEnableHandOfGod)
			{
				if( mResetZoneActive )
					CheckResetZone();
				else if( pActor->GetCurrentHealthLevel() > E_HEALTH_LEVEL_WRECKED)
				{
					if(mResetDelayTimer == 0.f)
					{
						CheckResetWarning();
						CheckReset();
					}
				}
			}
		}

		Input::C_GameInput* input = Input::GetGameInput(0);
		bool resetButton =
			 input->GetButtonPressedDebounced(Game::E_Button_ResetVehicle) &&
			!input->GetButtonPressedDebounced(Framework::E_Button_CameraMenu);


		if(mResetDelayTimer)
		{
			mResetDelayTimer-=C_GameApp::GetInstance()->GetTimeStep();

			if(mResetDelayTimer <= 0.f)
			{
				mResetDelayTimer=0.f;

				InitiateReset();
			}
		}
	}

	// ---------------------------------------------------------------------------------------------------

	void C_VehicleComponent_HandOfGod::InitiateReset(bool ResetFromWrecked)
	{
		// If we are not authorised to manually put vehicles back on track, Flag this vehicle as requiring a manual reset
		if(!mAutomaticHOGPlacementEnabled)
		{
			mManualHOGResetRequired = true;
			return;
		}

		PERF_EVENT("VLC.HandOfGod.InitiateReset");

		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

		if(!mIsAIControlled)
		{
			Game::Actors::C_VehicleActor* pVehActor		= pActor->GetGraphicalActor(mVehicleIndex);

			// Make fade to black.
			Util::C_String32 string;
			string.Printf( "HoG_fade_%d", UI::GetLocalPlayerIndex( pActor->mVehicleIndex ) );
			UI::C_UiTimelineMessage msg( string.Chars(), true, 0.0f );
			UI::gButtons.BroadcastMessageNow( 0xffffffff, &msg );

			mCurrentlyResettingTimer = mpHoGParams->mAIControlTime;
			pActor->mTemporaryAIControl = true;
			mCurrentlyResetting = true;

			Game::Actors::C_VehicleForceAIControlMsg controlMsg(pActor->mVehicleIndex);
			/*[ChadW] - BroadcastMessage_CleanUp*///pActor->GetActorList()->GetSubscriberSystem().BroadcastMessage(&controlMsg);
			pVehActor->SendMessageToActor(pActor->GetHandle(), &controlMsg);

			if(mpLevelState->mNumViewports == 1 && pVehActor->IsFocusVehicle())
			{
				// fade out sound effects
				Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eFadeOutSfx);
			}
		}
		else
		{
			mCurrentlyResettingTimer = 0.0f;
			mCurrentlyResetting = true;
		}

		mBeingResetAfterBeingWrecked = ResetFromWrecked;
	
		mNeedsHandOfGodResetWarning = mFacingForwardDelayGranted = mHasBeenReset = false;
		mTimeSinceLastReset = 0.f;

		mHitByPerkRecentlyTimer = mRidiculousSlappedTime = mSlappedTime = mCollidedRecentlyTimer = mFacingForwardsTimer = 0.f;
		mStuckTime = mFlippedTime = mTumbledTime = mOffSplineTime = 0.f;
	}

	// ---------------------------------------------------------------------------------------------------

	void C_VehicleComponent_HandOfGod::CheckResetZone()
	{
		PERF_EVENT("VLC.HandOfGod.CheckResetZone");

		if( mResetZoneTimer <= 0 )
		{
			InitiateReset();
			mResetType = E_HandOfGodResetType_AtSpeedAtPoint;
			mResetZoneActive = false;
		}
		else
		{
			Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
			if(!mIsAIControlled)
				LOGDEBUG(g_RLHandOfGodLogger, "Potential reset in %.2f. Reason: In HoG reset region", mResetZoneTimer);
			else
				LOGDEBUG(g_RLHandOfGodLoggerAI, "%i: Potential reset in %.2f. Reason: In HoG reset region", mVehicleIndex, mResetZoneTimer);

			mResetZoneTimer -= C_GameApp::GetInstance()->GetTimeStep();
		}
	}

	// ---------------------------------------------------------------------------------------------------

	void C_VehicleComponent_HandOfGod::CheckResetWarning()
	{
		PERF_EVENT("VLC.HandOfGod.CheckResetWarning");

		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		Game::Actors::C_VehicleActor* pVehActor		= pActor->GetGraphicalActor(mVehicleIndex);

		if(!mpHoGParams)
			return;

		Maths::HVec3 splineDir, vehicleDir;

		bool isTumbling = false, moveAttempt = false, facingForward = true, facingForwardReset = false;
		bool lowSpeed = pActor->mPositionInfo.mSpeedInMph < mpHoGParams->mMinimumSpeedInMPH;
		bool wrecked = (pActor->GetCurrentHealthLevel() == E_HEALTH_LEVEL_WRECKED);

		float frameTime = C_GameApp::GetInstance()->GetTimeStep();

		if(mHitByPerkRecentlyTimer > 0.0f)
			mHitByPerkRecentlyTimer -= frameTime;

		if(mSlappedTime >= 0.0f)
			mSlappedTime -= frameTime;

		if(mRidiculousSlappedTime >= 0.0f)
			mRidiculousSlappedTime -= frameTime;


		// -------------------------------
		// Check if we are facing forwards
		// -------------------------------
		float forwardDot = -1;
		if(mResetCriteria & E_HandOfGodResetCriteria_NotFacingForwards)
		{
			PERF_EVENT("VLC.HandOfGod.NotFacingForward");

			bool drifting = false;

			const C_VehicleComponent_NewScoring* pScoring = (Components::C_VehicleComponent_NewScoring*)pActor->QueryInterface(E_IF_VehicleComponent_NewScoring);
			if(pScoring)
			{
				float driftDistance = pScoring->GetDriftDistance();
				drifting = (driftDistance > mpHoGParams->mDriftDistance);
			}

			airacingtech::ISplineTracker *vehicleDataSplineTracker = AIAmax::AmaxAIR::Instance().GetAISplineTracker(mVehicleIndex);
			if(vehicleDataSplineTracker)
			{
				const airacingtech::RouteInfo* pRoute = vehicleDataSplineTracker->GetCurrentRoute();
				if( pRoute && pRoute->m_pRoute )
				{
					pRoute->m_pRoute->GetDirection(pRoute->m_index, splineDir);
					vehicleDir = pActor->mPositionInfo.mDirection;

					Maths::HVec3 diffVec = vehicleDir - splineDir;
					forwardDot = Maths::DotProduct(diffVec,vehicleDir);

					facingForward = ( forwardDot < mpHoGParams->mForwardDot );

					if(!drifting)
						facingForwardReset = ( forwardDot < mpHoGParams->mForwardResetDot );
					else
						facingForwardReset = ( forwardDot < mpHoGParams->mDriftDot );
				}
				else 
				{
					const airacingtech::RouteInfo* pLogicRoute = vehicleDataSplineTracker->GetLogicRoute();
					if(pLogicRoute && pLogicRoute->m_pRoute && pLogicRoute->m_in )
					{
						pLogicRoute->m_pRoute->GetDirection(pLogicRoute->m_index, splineDir);
						vehicleDir = pActor->mPositionInfo.mDirection;

						Maths::HVec3 diffVec = vehicleDir - splineDir;
						forwardDot = Maths::DotProduct(diffVec,vehicleDir);

						facingForward = ( forwardDot < mpHoGParams->mForwardDot );

						if(!drifting)
							facingForwardReset = ( forwardDot < mpHoGParams->mForwardResetDot );
						else
							facingForwardReset = ( forwardDot < mpHoGParams->mDriftDot );
					}
				}

				// Not facing forward or not moving forward after collision
				bool movingBackwards = pActor->mPositionInfo.mForwardSpeed <= mpHoGParams->mMinForwardSpeed && ( mCollidedRecently || mFacingForwardsTimer > 0.f );
				if(!facingForward || movingBackwards)
				{
					mFacingForwardsTimer += frameTime;

					if(!mIsAIControlled)
						LOGDEBUG(g_RLHandOfGodLogger, "Potential reset in %.2f. Reason: Facing or moving backwards (%i, %i)", 
						mpHoGParams->mBackwardsTimer - mFacingForwardsTimer, !facingForward, movingBackwards);
					else
						LOGDEBUG(g_RLHandOfGodLoggerAI, "%i: Potential reset in %.2f. Reason: Facing or moving backwards (%i, %i)", 
						mVehicleIndex, mpHoGParams->mBackwardsTimer - mFacingForwardsTimer, !facingForward, movingBackwards);
				}
				else
					mFacingForwardsTimer = 0.0f;
			}
		}


		// ------------------------------------
		// Work out if we have tumbled recently
		// ------------------------------------
		Maths::HVec3 angVel = pVehActor->GetCollisionBody()->AngularVelocity();

		if(mResetCriteria & E_HandOfGodResetCriteria_Tumbled)
		{
			PERF_EVENT("VLC.HandOfGod.Tumbled");

			bool tumbleCheck = false;

			// Post-collision tumble
			if( !facingForward && mCollidedRecentlyTimer > 0.f )
			{
				bool tumblingX = abs(angVel.X()) > mpHoGParams->mMaximumCollisionAngularVelocity.mPitch ? true : false;
				bool tumblingY = abs(angVel.Y()) > mpHoGParams->mMaximumCollisionAngularVelocity.mYaw ? true : false;
				bool tumblingZ = abs(angVel.Z()) > mpHoGParams->mMaximumCollisionAngularVelocity.mRoll ? true : false;

				if( tumblingX || tumblingY || tumblingZ )
				{
					if(!mIsAIControlled)
						LOGDEBUG(g_RLHandOfGodLogger, "Tumbling post-collision: X(%i), Y(%i), Z(%i) - Timer(%.2f)", tumblingX, tumblingY, tumblingZ, mCollidedRecentlyTimer);

					tumbleCheck = true;
				}
			}
			else
			{
				// Post-perk tumble
				if( !facingForward && mHitByPerkRecentlyTimer > 0.f )
				{
					bool tumblingX = abs(angVel.X()) > mpHoGParams->mMaximumPerkAngularVelocity.mPitch ? true : false;
					bool tumblingY = abs(angVel.Y()) > mpHoGParams->mMaximumPerkAngularVelocity.mYaw ? true : false;
					bool tumblingZ = abs(angVel.Z()) > mpHoGParams->mMaximumPerkAngularVelocity.mRoll ? true : false;

					if( tumblingX || tumblingY || tumblingZ )
					{
						if(!mIsAIControlled)
							LOGDEBUG(g_RLHandOfGodLogger, "Tumbling post-perk: X(%i), Y(%i), Z(%i) - Timer(%.2f", tumblingX, tumblingY, tumblingZ, mHitByPerkRecentlyTimer);

						tumbleCheck = true;
					}
				}
				else
				{
					bool tumblingX = abs(angVel.X()) > mpHoGParams->mMaximumAngularVelocity.mPitch ? true : false;
					bool tumblingY = abs(angVel.Y()) > mpHoGParams->mMaximumAngularVelocity.mYaw ? true : false;
					bool tumblingZ = abs(angVel.Z()) > mpHoGParams->mMaximumAngularVelocity.mRoll ? true : false;

					// General tumble
					if( tumblingX || tumblingY || tumblingZ )
					{
						if(!mIsAIControlled)
							LOGDEBUG(g_RLHandOfGodLogger, "Tumbling general: X(%i), Y(%i), Z(%i)", tumblingX, tumblingY, tumblingZ);

						tumbleCheck = true;
					}
				}

			}

			if(tumbleCheck)
			{
				isTumbling = true;
				mTumbledTime += frameTime;

				if(!mIsAIControlled)
					LOGDEBUG(g_RLHandOfGodLogger, "Potential reset in %.2f. Reason: Currently tumbling.", mpHoGParams->mTumbleTimer - mTumbledTime);
				else
					LOGDEBUG(g_RLHandOfGodLoggerAI, "%i: Potential reset in %.2f. Reason: Currently tumbling.", mVehicleIndex, mpHoGParams->mTumbleTimer - mTumbledTime);
			}
			else
			{
				mTumbledTime = 0;
			}
		}

		// -------------------------------------
		// Work out if we had an impact recently
		// -------------------------------------
		if(mResetCriteria & E_HandOfGodResetCriteria_Crashed)
		{
			PERF_EVENT("VLC.HandOfGod.Crashed");

			if( mVehicleImpactThisFrame )
			{
				mCollidedRecently = true;
				mCollidedRecentlyTimer = mpHoGParams->mCollisionTimer;
			}
			else if( !isTumbling && mCollidedRecently )
			{
				mCollidedRecentlyTimer -= frameTime;
				if(mCollidedRecentlyTimer <= 0.f)
				{
					if(!mIsAIControlled)
						LOGDEBUG(g_RLHandOfGodLogger, "No longer tumbling.", mpHoGParams->mTumbleTimer - mTumbledTime);

					mCollidedRecently = false;
				}
			}
		}
		
		// -------------------------------------
		// Work out if I have been stuck a while
		// -------------------------------------
		bool stuck = false;
		if(mResetCriteria & E_HandOfGodResetCriteria_StuckOffGround)
		{
			PERF_EVENT("VLC.HandOfGod.Stuck");

			bool careAboutCollisions	= ((mResetCriteria & E_HandOfGodResetCriteria_Stuck) != 0);
			bool wheelsOnGround			= (pVehActor->GetVehiclePhysics()->NumWheelsOnGround() == pVehActor->GetVehiclePhysics()->GetNumWheels());
			bool isStuckRecently		= careAboutCollisions && (mCollidedRecently || mStuckTime > 0.f);
			bool isUsingReverseNitrous	= pActor->mPickupInfo.mUsingNitrousReverse;
			bool stuck					= lowSpeed && !isUsingReverseNitrous && (isStuckRecently || !wheelsOnGround || mIsAIControlled);

			if(stuck)
			{
				mStuckTime += frameTime;

				if(!mIsAIControlled)
					LOGDEBUG(g_RLHandOfGodLogger, "Potential reset in %.2f. Reason: Stuck.", mpHoGParams->mStuckTimer - mStuckTime);
				else
					LOGDEBUG(g_RLHandOfGodLoggerAI, "%i: Potential reset in %.2f. Reason: Stuck.", mVehicleIndex, mpHoGParams->mStuckTimer - mStuckTime);
			}
			else
				mStuckTime = 0.f;
		}


		// ----------------------------
		// Check if I have flipped over
		// ----------------------------
		bool flipped = false;
		if(mResetCriteria & E_HandOfGodResetCriteria_Flipped)
		{
			PERF_EVENT("VLC.HandOfGod.Flipped");

			float dotUp = Maths::DotProduct( pActor->GetWorldMatrix().AY(), Maths::HVec3(0.f, 1.f, 0.f) );

			if( dotUp <= mpHoGParams->mMinDotUp )
			{
				if( mFlippedTime >= 0 )
				{
					mFlippedTime -= frameTime;

					if(!mIsAIControlled)
						LOGDEBUG(g_RLHandOfGodLogger, "Potential reset in %.2f. Reason: Flipped Over.", mFlippedTime);
					else
						LOGDEBUG(g_RLHandOfGodLoggerAI, "%i: Potential reset in %.2f. Reason: Flipped Over.", mVehicleIndex, mFlippedTime);
				}
			}
			else
				mFlippedTime = mpHoGParams->mFlippedTimer;

			if( mFlippedTime <= 0 )
				flipped = true;
		}

		// -------------------------------
		// Set the Hand of God if required
		// -------------------------------
		{
			PERF_EVENT("VLC.HandOfGod.SetIfRequired");

			if(mHandOfGodResetWarningTimer > 0.0f)
				mHandOfGodResetWarningTimer -= frameTime;

			if(mTimeSinceLastReset < mpHoGParams->mWaitTime)
				mTimeSinceLastReset += frameTime;

			if(mTumbledTime > mpHoGParams->mTumbleTimer || mStuckTime > mpHoGParams->mStuckTimer || mFacingForwardsTimer > mpHoGParams->mBackwardsTimer || flipped)
			{
				if(!mNeedsHandOfGodResetWarning && mTimeSinceLastReset >= mpHoGParams->mWaitTime)
				{
					mNeedsHandOfGodResetWarning = true;
					mRidiculousSlappedTime = mpHoGParams->mRidiculousSlappedTimer;
					mSlappedTime			= mpHoGParams->mSlappedTimer;
					mHandOfGodResetWarningTimer = mpHoGParams->mResetTimer;

					// AI gets more punished than humans
					if(mIsAIControlled)
					{
						mHandOfGodResetWarningTimer += mpHoGParams->mAIResetTimeAddition;
						mRidiculousSlappedTime += mpHoGParams->mAIResetTimeAddition;
						mSlappedTime			+= mpHoGParams->mAIResetTimeAddition;
					}

					if(!mIsAIControlled)
						LOGDEBUG(g_RLHandOfGodLogger, "Need reset: Tumbled %i, Stuck %i, Backwards %i, Flipped %i, Angle: %f, Extra Time:%i\n", mTumbledTime > mpHoGParams->mTumbleTimer, 
						mStuckTime > mpHoGParams->mStuckTimer, mFacingForwardsTimer > mpHoGParams->mBackwardsTimer, flipped, forwardDot, facingForward);
					else
						LOGDEBUG(g_RLHandOfGodLoggerAI, "%i: Need reset: Tumbled %i, Stuck %i, Backwards %i, Flipped %i, Angle: %f, Extra Time:%i\n", mVehicleIndex,
						mTumbledTime > mpHoGParams->mTumbleTimer, mStuckTime > mpHoGParams->mStuckTimer, mFacingForwardsTimer > mpHoGParams->mBackwardsTimer, 
						flipped, forwardDot, facingForward);
				}
			}
		}
		
		if( mNeedsHandOfGodResetWarning )
		{
			PERF_EVENT("VLC.HandOfGod.NeedsWarning");

			bool knockedAround = abs(angVel.X()) >= mpHoGParams->mFinishedSpinX || abs(angVel.Y()) >= mpHoGParams->mFinishedSpinY || 
				abs(angVel.Z()) >= mpHoGParams->mFinishedSpinZ;

			bool comeToHalt = mHandOfGodResetWarningTimer <= 0.f && !knockedAround;

			// If we are still reasonably facing forwards, we get a bit of extra time to get up to speed.
			if(comeToHalt && !mFacingForwardDelayGranted && facingForward)
			{
				mFacingForwardDelay			= mpHoGParams->mForwardRecoveryTimeAddition;
				mFacingForwardDelayGranted	= true;
			}
			else if(mFacingForwardDelay > 0.f)
				mFacingForwardDelay -= frameTime;

			if( (comeToHalt && ( facingForwardReset || mFacingForwardDelay <= 0.f)) || mStuckTime > mpHoGParams->mStuckTimer || (mSlappedTime < 0.f && isTumbling) || mRidiculousSlappedTime < 0.f )
			{
				if(!mIsAIControlled)
				{
					if(mRidiculousSlappedTime < 0.f)
						LOGDEBUG(g_RLHandOfGodLogger, "Attempt reset: Ridiculously slapped.");
					else if(mSlappedTime < 0.f)
						LOGDEBUG(g_RLHandOfGodLogger, "Attempt reset: Slapped.");
					else if(mStuckTime > mpHoGParams->mStuckTimer)
						LOGDEBUG(g_RLHandOfGodLogger, "Attempt reset: Stuck.");
					else
						LOGDEBUG(g_RLHandOfGodLogger, "Attempt reset: Come to a halt.");
				}
				else
				{
					if(mRidiculousSlappedTime < 0.f)
						LOGDEBUG(g_RLHandOfGodLoggerAI, "%i: Attempt reset: Ridiculously slapped.", mVehicleIndex);
					else if(mSlappedTime < 0.f)
						LOGDEBUG(g_RLHandOfGodLoggerAI, "%i: Attempt reset: Slapped.", mVehicleIndex);
					else if(mStuckTime > mpHoGParams->mStuckTimer)
						LOGDEBUG(g_RLHandOfGodLoggerAI, "%i: Attempt reset: Stuck.", mVehicleIndex);
					else
						LOGDEBUG(g_RLHandOfGodLoggerAI, "%i: Attempt reset: Come to a halt.", mVehicleIndex);
				}

				bool notFacingForward = !facingForwardReset && mResetCriteria & E_HandOfGodResetCriteria_NotFacingForwards;
				if( notFacingForward || flipped || knockedAround || mStuckTime > 0.f )
				{
					InitiateReset();
					if(	IsAIDataAvailable() )
					{
						mResetType = E_HandOfGodResetType_AtSpeedOnSpline;
					}
					else
					{
						GetBestStartPosition(mResetPosition, mResetDirection, mResetSpeed);

						mResetType = E_HandOfGodResetType_AtSpeedAtPoint;
					}

					if(!mIsAIControlled)
						LOGDEBUG(g_RLHandOfGodLogger, "Reset Success - Forward: %i, Flipped: %i, Speed: %i, Knocked Around: %i, Stuck: %i, Angle: %f", 
						!facingForwardReset, flipped, lowSpeed, knockedAround, mStuckTime > 0.f, forwardDot);
					else
						LOGDEBUG(g_RLHandOfGodLoggerAI, "%i: Reset Success - Forward: %i, Flipped: %i, Speed: %i, Knocked Around: %i, Stuck: %i, Angle: %f", 
						mVehicleIndex, !facingForwardReset, flipped, lowSpeed, knockedAround, mStuckTime > 0.f, forwardDot);
				}
				else
				{			
					if(!mIsAIControlled)
					{
						C_VehicleLogicMsg_HoGRecoveryMsg msg(mVehicleIndex);
						pActor->GetActorList()->SendMessageToActorComponent(pActor->GetHandle(), E_IF_VehicleComponent_NewScoring, &msg);

						LOGDEBUG(g_RLHandOfGodLogger, "Reset Failed - Forward: %i, Flipped: %i, Speed: %i, Knocked Around: %i, Stuck: %i, Angle: %f", 
							facingForward, flipped, lowSpeed, knockedAround, stuck, forwardDot);
					}
					else
						LOGDEBUG(g_RLHandOfGodLoggerAI, "%i: Reset Failed - Forward: %i, Flipped: %i, Speed: %i, Knocked Around: %i, Stuck: %i, Angle: %f", 
						mVehicleIndex, facingForward, flipped, lowSpeed, knockedAround, stuck, forwardDot);

					mNeedsHandOfGodResetWarning = mFacingForwardDelayGranted = false;
					mTimeSinceLastReset = 0.f;

					mHitByPerkRecentlyTimer = mRidiculousSlappedTime = mSlappedTime = mCollidedRecentlyTimer = mFacingForwardsTimer = 0.f;
					mStuckTime = mFlippedTime = mTumbledTime = mOffSplineTime = 0.f;
				}
			}
			else if(!mIsAIControlled)
			{
				LOGDEBUG(g_RLHandOfGodLogger, "Not Yet Finished: Tumble(%i,%i,%i), Stuck(%i), Slap(Timer:%i,isTumbling:%i), RidiculousSlap(%.2f)", abs(angVel.X()) >= mpHoGParams->mFinishedSpinX, 
					abs(angVel.Y()) >= mpHoGParams->mFinishedSpinY, abs(angVel.Z()) >= mpHoGParams->mFinishedSpinZ, mStuckTime > mpHoGParams->mStuckTimer, 
					mSlappedTime <= 0.f, isTumbling, mRidiculousSlappedTime);
			}
		}

		mVehicleImpactThisFrame = false;
	}

	// ---------------------------------------------------------------------------------------------------

	void C_VehicleComponent_HandOfGod::CheckReset()
	{
		PERF_EVENT("VLC.HandOfGod.CheckReset");

		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

		if( !mpHoGParams )
			return;

		bool offSpline = false;

		if(mResetCriteria & E_HandOfGodResetCriteria_OffSpline)
		{
			// ----------------------------
			// Check if I am off the spline
			// ----------------------------
			airacingtech::ISplineTracker *vehicleDataSplineTracker = AIAmax::AmaxAIR::Instance().GetAISplineTracker(mVehicleIndex);

			if(vehicleDataSplineTracker)
			{
				const airacingtech::RouteInfo* pLogicRoute = vehicleDataSplineTracker->GetLogicRoute();
				if( !pLogicRoute || !pLogicRoute->m_pRoute || !pLogicRoute->m_in || mDoingHOGResetCam )
				{
					if(!mIsAIControlled)
						LOGDEBUG(g_RLHandOfGodLogger, "Potential Reset in %.2f. Reason: Off Spline.", mOffSplineTime);
					else
						LOGDEBUG(g_RLHandOfGodLoggerAI, "%i: Potential Reset in %.2f. Reason: Off Spline.", mVehicleIndex, mOffSplineTime);

					mOffSplineTime -= C_GameApp::GetInstance()->GetTimeStep();
				}
				else
					mOffSplineTime = mpHoGParams->mOffSplineTimer;

				if(mOffSplineTime < mpHoGParams->mOffSplineTimer && mForcedResetZone > 0 && !mDoingHOGResetCam)
				{
					pActor->GetGraphicalActor(mVehicleIndex)->SetGoneOffTrack(true);
					pActor->mStatistics.mNumTimesOffEdge++;
					StartHoGCam();
				}

				if( mOffSplineTime <= 0 )
					offSpline = true;
			}
		}

		bool resetOutOfWorld = false;
		if (pActor->GetWorldMatrix().AY().Y() < -400.0f)
		{
			// potentially out of the world...reset.
			resetOutOfWorld = true;
		}

		// -----------------
		// Reset as required
		// -----------------
		if( offSpline || resetOutOfWorld)
		{
			if(!mIsAIControlled)
				LOGDEBUG(g_RLHandOfGodLogger, "Reset Success - Off Spline.");
			else
				LOGDEBUG(g_RLHandOfGodLoggerAI, "%i: Reset Success - Off Spline.", mVehicleIndex);


			InitiateReset();
			mResetType = E_HandOfGodResetType_AtSpeedOnSpline;
		}
	}

	// ---------------------------------------------------------------------------------------------------

	const float C_VehicleComponent_HandOfGod::GetResetDelayTime()
	{
		MultiplayerConfig* pMultiplayerConfig = C_MultiplayerData::Instance().m_pConfig;
		BIZ_ASSERT_MSG(pMultiplayerConfig, "C_VehicleComponent_HandOfGod::GetResetDelayTime() > Couldn't get a pointer to the sorcery multiplayer config");

		float resetDelayTime = 5.f;

		return resetDelayTime;
	}

	// ---------------------------------------------------------------------------------------------------

	void C_VehicleComponent_HandOfGod::StartHoGCam()
	{
		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		Game::Actors::C_VehicleActor* pVehActor		= pActor->GetGraphicalActor(mVehicleIndex);

		if( mViewportIndex != -1)
		{
			Game::Actors::C_VehicleControlsDisableMsg disableControlsMsg(pActor->GetVehicleIndex());
			pVehActor->SendMessageToActor(pVehActor->GetHandle(), &disableControlsMsg);

			// This needs to send across the viewport at some point.
			AmaxCam::CameraSystem().StartHOGResetCamera(mForcedResetCameraTracking, mViewportIndex);
			mDoingHOGResetCam = true;

			Game::C_InGameSpecialFXManager::Instance().ForceChaseCam(mViewportIndex);
			pActor->SetAwayFromBody(true);
		}
	}

	void C_VehicleComponent_HandOfGod::StopHoGCam()
	{
		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		Game::Actors::C_VehicleActor* pVehActor		= pActor->GetGraphicalActor(mVehicleIndex);

		if( mViewportIndex != -1)
		{
			Game::Actors::C_VehicleControlsEnableMsg enableControlsMsg(pVehActor->GetHandle(), pActor->GetVehicleIndex());
			pVehActor->SendMessageToActor(pVehActor->GetHandle(), &enableControlsMsg);

			AmaxCam::CameraSystem().StopHOGResetCamera(mViewportIndex);
			AmaxCam::CameraSystem().ResetVehicleFeed(mViewportIndex);
			mDoingHOGResetCam = false;

			Game::C_InGameSpecialFXManager::Instance().CeaseForceChaseCam(mViewportIndex);
			pActor->SetAwayFromBody(false);
		}
	}

	// ---------------------------------------------------------------------------------------------------

	Debug::Float g_debugHoGNormalResetThreshold("Vehicle.Components.HandOfGod.NormalDotProductThreshold", 0.4f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

	void C_VehicleComponent_HandOfGod::ResetVehicleOnTrack(bool isManual, bool atSpeed, bool useOverride)
	{
		PERF_EVENT("VLC.HandOfGod.ResetVehicleOnTrack");

		if(mResetDelayTimer)
			return;

		using namespace Maths;
		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		Game::Actors::C_VehicleActor* pVehActor		= pActor->GetGraphicalActor(mVehicleIndex);

		const float ghostTime						= GetGhostTime();

		Maths::HVec3 direction, newPosition, lookAt;
		float speed = 0.0f;

		if(useOverride || mResetZoneActive)
		{
			direction	= mResetDirection;
			newPosition = mResetPosition;
			speed		= mResetSpeed;
		}
		else
		{
			// Get spline information from Darren.
			if ( !AIAmax::AmaxAIR::Instance().GetValidVehiclePosDir(mVehicleIndex, newPosition, direction, speed) )
			{
				LOGWARN(g_RLHandOfGodLogger, "Failed to find valid reset point - cannot reset vehicle %i with HoG", mVehicleIndex);
				return;
			}

		}

		Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
		if(levelState.mCars[mVehicleIndex].mUpgrades.GetMultiplayerAbilities()->GetAbility(UI::E_MultiplayerVehicleAbilities_Jump_The_Gun)->GetActive())
		{
			bool jumpedTheGun = false;

			MiscModSettings& modSettings = C_MultiplayerData::Instance().m_pConfig->m_MiscModSettings;
			if(mBeingResetAfterBeingWrecked)
			{
				speed = modSettings.m_JumpTheGunRespawnSpeedWreck;
				jumpedTheGun = true;
			}
			else if(modSettings.m_JumpTheGunRespawnSpeed > 0.f)
			{
				speed = modSettings.m_JumpTheGunRespawnSpeed;
				jumpedTheGun = true;
			}
			if(jumpedTheGun)
			{
				levelState.mCars[mVehicleIndex].mUpgrades.GetMultiplayerAbilities()->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Jump_The_Gun, true);
			}
		}

		bool success = ResetVehicleOnTrackStatic( mpRLC, pActor, newPosition, direction, atSpeed?speed:0.0f, ghostTime, isManual );
		

		if(!success)
			LOGWARN(g_RLHandOfGodLogger, "Unable to reset vehicle on the spline. If no spline exists for this map yet, please turn off Hand of God in DebugView -> Vehicle.Components.HandOfGod.Enable");

		CancelPerkEffects();

		if(success && mDiedBySuicide)
		{
			mDiedBySuicide = false;
		}
	}

	bool C_VehicleComponent_HandOfGod::ResetVehicleOnTrackStatic( Game::Actors::RaceLogicCore* pRLC, Game::Actors::C_VehicleLogicActor* pActor, Maths::HVec3 newPosition, Maths::HVec3 direction, F32 speed, F32 ghostTime, bool isManual )
	{
		int vehicleIndex = pActor->GetVehicleIndex();
		Game::Actors::C_VehicleActor* pVehActor = pActor->GetGraphicalActor(vehicleIndex);


		BIZ_ASSERT(newPosition.Finite());
		BIZ_ASSERT(direction.Finite());

		Maths::C_Normal up(Maths::C_Normal::AxisPositiveY());
		Maths::AxisY aUp(up);

		direction.Normalise();

		Maths::C_Normal down(0.f, -1.f, 0.f);
		Maths::HVec3 heightIncrement(0.f, 2.0f, 0.f);

		U8 data[1024 * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> stack(data, 1024);
		Collision::C_Ray ray;
		Collision::C_CollisionFilter rayCollisionFilter( Game::PolyFlags::kCameraSolid, Game::PolyFlags::kCameraSolid);

		float maxHeight = 20;
		float totalHeightInc = 0;
		bool success = false;

		// In case the spline is below the track, iterate upwards looking for it.
		while( totalHeightInc < maxHeight && !success)
		{
			ray.InitialiseRay(newPosition, newPosition + down * 1000.f, &rayCollisionFilter);
			pRLC->GetPhysicsScene()->DoRayTrace(ray, stack);
			Collision::C_RayTracingResult& result = ray.m_result;

			if( result.Hit() )
			{	
				Maths::HMat33 orientation;
				direction = result.TriangleNormal() ^ direction;
				Maths::C_Normal dir(direction.X(), direction.Y(), direction.Z());
				Maths::AxisX aDir(dir);

				float fDot = Maths::DotProduct(up, result.TriangleNormal());
				if (fDot <= (float)g_debugHoGNormalResetThreshold)
					orientation = Maths::HMat33( Maths::AxisY( up ), aDir );
				else
					orientation = Maths::HMat33( Maths::AxisY( result.TriangleNormal() ), aDir );

				Maths::HMat43 worldMatrixPoint( orientation, result.HitPoint() );
				BIZ_ASSERT(result.HitPoint().Finite());

				Maths::HVec3 velocity(0,0,0);
				
				velocity = worldMatrixPoint.AZ() * speed;
				Game::Actors::C_VehicleResetAtWorldPositionMsg resetMsg(pVehActor->GetHandle(), vehicleIndex, worldMatrixPoint, velocity);
				pVehActor->SendMessageToActor(pVehActor->GetHandle(), &resetMsg);

				pActor->Ghost(ghostTime, g_DefaultGhostAlpha);
				success = true;

				//C_VehicleLogicShieldTimeMsg shieldMsg(mVehicleIndex, mpHoGParams->mShieldTime, false);
				//pActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Fan_Power, &shieldMsg);

				LOGDEBUG(g_RLHandOfGodLogger, "Player Hand of God - engaging: index(%d), pos(%.2f, %.2f, %.2f), dir(%.2f, %.2f, %.2f)",
					vehicleIndex, 
					(float)newPosition.X(), (float)newPosition.Y(), (float)newPosition.Z(), 
					(float)direction.X(), (float)direction.Y(), (float)direction.Z());


				C_VehicleLogicMsg_HoGResetMsg msg(vehicleIndex);

				pActor->GetActorList()->SendMessageToActorComponent(pActor->GetHandle(), E_IF_VehicleComponent_NewScoring, &msg);

				// damage wants this too please
				pActor->GetActorList()->SendMessageToActorComponent(pActor->GetHandle(), E_IF_VehicleComponent_Damage, &msg);
				//And Abilities too
				pActor->GetActorList()->SendMessageToActorComponent(pActor->GetHandle(), E_IF_VehicleComponent_Abilities, &msg);
#ifndef CFG_GOLD
				AmaxDataLogging::LogReset(pActor->mPositionInfo.mWorldPosition, newPosition, true, isManual);
#endif

				//AIAmax::AmaxAIR::Instance().ResetAfterHOG(mVehicleIndex);
				


			}
			else
			{
				newPosition += heightIncrement;
				totalHeightInc += heightIncrement.Y();
			}
		}

		return success;
	}
																				
	//Work out which ghost reset parameters we should be using
	const float C_VehicleComponent_HandOfGod::GetGhostTime() const
	{
		//Get the actor
		Game::Actors::C_VehicleLogicActor* pActor = ((Game::Actors::C_VehicleLogicActor*)GetActor());

		BIZ_ASSERT_MSG(pActor, "C_VehicleComponent_HandOfGod::GetGhostTime() > pActor is an invalid pointer...sit back and watch the fireworks");

		RLGhostTimeAfterSpawnParameters ghostParams = mpHoGParams->mGhostParametersStandard;
		

		//Work out whether we're currently wrecked
		if(mBeingResetAfterBeingWrecked)
		{
			return ghostParams.mGhostTimeAfterWrecked;
		}
		else
		{
			return ghostParams.mGhostTimeAfterHoG;
		}
	}


	void C_VehicleComponent_HandOfGod::ResetVehicleAtVehiclePosition(U8 resetIndex, bool isManual /*= false*/, bool atSpeed /*= false*/, bool useOverride /*= false*/)
	{
		using namespace Maths;
		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		Game::Actors::C_VehicleActor* pVehActor		= pActor->GetGraphicalActor(mVehicleIndex);

		const float	ghostTime						= GetGhostTime();

		Maths::HVec3 direction, newPosition, lookAt;
		float speed = 0.0f;

		if(useOverride)
		{
			direction	= mResetDirection;
			newPosition = mResetPosition;
			speed		= mResetSpeed;
		}
		else
		{
			// Get spline information from Darren.
			if ( !AIAmax::AmaxAIR::Instance().GetValidVehiclePosDir(resetIndex, newPosition, direction, speed) )
			{
				LOGWARN(g_RLHandOfGodLogger, "Failed to find valid reset point - cannot reset vehicle %i with HoG", mVehicleIndex);
				return;
			}
		}

		// reset the ai logic state.....assumes the stored record was at the same as the hog reset.
		//AIAmax::AmaxAIR::Instance().ResetResetLogicState(mVehicleIndex);

		BIZ_ASSERT(newPosition.Finite());
		BIZ_ASSERT(direction.Finite());

		Maths::C_Normal up(C_Normal::AxisPositiveY());
		Maths::AxisY aUp(up);

		Maths::C_Normal dir(direction.X(), direction.Y(), direction.Z());

		Maths::AxisZ aDir(dir);

		Maths::HVec3 velocity(0,0,0);
		if(atSpeed)
			velocity = aDir * speed;

		Maths::C_Normal down(0.f, -1.f, 0.f);
		Maths::HVec3 heightIncrement(0.f, 2.0f, 0.f);

		U8 data[1024 * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> stack(data, 1024);
		Collision::C_Ray ray;
		Collision::C_CollisionFilter rayCollisionFilter( Game::PolyFlags::kCameraSolid, Game::PolyFlags::kCameraSolid);

		float maxHeight = 20;
		float totalHeightInc = 0;
		bool success = false;

		// In case the spline is below the track, iterate upwards looking for it.
		while( totalHeightInc < maxHeight && !success)
		{
			ray.InitialiseRay(newPosition, newPosition + down * 1000.f, &rayCollisionFilter);
			mpRLC->GetPhysicsScene()->DoRayTrace(ray, stack);
			Collision::C_RayTracingResult& result = ray.m_result;

			Maths::HMat33 orientation;
			float fDot = Maths::DotProduct(up, result.TriangleNormal());
			if (fDot <= (float)g_debugHoGNormalResetThreshold)
				orientation = Maths::HMat33( Maths::AxisY( up ), aDir );
			else
				orientation = Maths::HMat33( Maths::AxisY( result.TriangleNormal() ), aDir );

			newPosition += heightIncrement;
			totalHeightInc += heightIncrement.Y();

			if( result.Hit() )
			{	
				Maths::HMat43 worldMatrixPoint( orientation, result.HitPoint() );
				BIZ_ASSERT(result.HitPoint().Finite());

				Game::Actors::C_VehicleResetAtWorldPositionMsg resetMsg(pVehActor->GetHandle(), mVehicleIndex, worldMatrixPoint, velocity);
				pVehActor->SendMessageToActor(pVehActor->GetHandle(), &resetMsg);

				pActor->Ghost(ghostTime, g_DefaultGhostAlpha);
				success = true;

				//C_VehicleLogicShieldTimeMsg shieldMsg(mVehicleIndex, mpHoGParams->mShieldTime, false);
				//pActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Fan_Power, &shieldMsg);

				LOGDEBUG(g_RLHandOfGodLogger, "Player Hand of God - engaging: index(%d), pos(%.2f, %.2f, %.2f), dir(%.2f, %.2f, %.2f)",
					mVehicleIndex, 
					(float)newPosition.X(), (float)newPosition.Y(), (float)newPosition.Z(), 
					(float)direction.X(), (float)direction.Y(), (float)direction.Z());
#ifndef CFG_GOLD
				AmaxDataLogging::LogReset(pActor->mPositionInfo.mWorldPosition, newPosition, true, isManual);
#endif
/* Is this legacy? Seems to fade to black twice when using this function if this is uncommented? [mtaylor]
				// Make fade to black.
				Util::C_String32 string;
				string.Printf( "HoG_fade_%d", UI::GetLocalPlayerIndex( pActor->mVehicleIndex ) );
				UI::C_UiTimelineMessage msg( string.Chars(), true, 0.0f );
				UI::gButtons.BroadcastMessageNow( 0xffffffff, &msg );
*/		
				//	Reset the camera to behind the vehicle
//				AmaxCam::CameraSystem().VehicleFeed()->Reset();



				//AIAmax::AmaxAIR::Instance().ResetAfterHOG(mVehicleIndex);
			}
		}

		CancelPerkEffects();

		if(success)
		{
			if(mDiedBySuicide)
			{
				mDiedBySuicide = false;
				SendSuicideMessageToUI();
			}
		}

		if(!success)
			LOGWARN(g_RLHandOfGodLogger, "Unable to reset vehicle on the spline. If no spline exists for this map yet, please turn off Hand of God in DebugView -> Vehicle.Components.HandOfGod.Enable");
	}

	void C_VehicleComponent_HandOfGod::CancelPerkEffects()
	{
		Game::Actors::PickupClearHitUpdatersMsg removeHitUpdatersMsg(mVehicleIndex);
		Game::Actors::C_VehicleLogicActor* pActor = ((Game::Actors::C_VehicleLogicActor*)GetActor());
		pActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &removeHitUpdatersMsg);

		Game::Actors::VehiclePerformanceModifiers mods;
		mods.InitialiseWithOverwrite();

		Game::Actors::C_VehicleUpdatePerformanceModifiersMsg perfModMsg(pActor->GetHandle(), mVehicleIndex, mods);
		pActor->GetGraphicalActor(mVehicleIndex)->SendMessageToActor(pActor->GetGraphicalActor(mVehicleIndex)->GetHandle(), &perfModMsg);

		if(mpLevelState->mCars[mVehicleIndex].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER)
		{
			S32 local_index = UI::GetLocalPlayerIndex( mVehicleIndex );
			Util::C_String32 buffer;

			buffer.Printf( "Flash_fade_%d", local_index );
			UI::C_UiTimelineMessage msg( buffer.Chars(), true, 1.99f );
			UI::gButtons.BroadcastMessageNow( 0xffffffff, &msg );

			buffer.Printf( "FlashGraphic_fade_%d", local_index );
			UI::C_UiTimelineMessage msg1( buffer.Chars(), true, 2.99f );
			UI::gButtons.BroadcastMessageNow( 0xffffffff, &msg1 );
		}

		pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Stop();
	}

	void C_VehicleComponent_HandOfGod::GetBestStartPosition(Maths::HVec3& position, Maths::HVec3& direction, float& speed) const
	{
		//We're probably in an arena type of track so we'll respawn to a random start position
		const RLStartPosition& randomStartPos = mpRLC->GetBestStartPosition(mVehicleIndex);

		position = randomStartPos.mWorldPosition;
		direction = randomStartPos.mWorldMatrix.AZ();
		speed = mpLevelState->mHoGNoSplineResetSpeedInMPH;

	}

	const bool C_VehicleComponent_HandOfGod::IsAIDataAvailable() const
	{
		return AIAmax::AmaxAIR::Instance().AllOK();
	}


	// Message Handlers
	bool C_VehicleComponent_HandOfGod::OnMessage(const Messaging::C_MessageBase *pMessage) 
	{
		PERF_EVENT("VLC.HandOfGod.MessageHandler");
		switch(pMessage->Type())
		{
		//case GameMessaging::E_GameMsg_Update:
		//	{
		//		if( mEnabled )
		//			Update();
		//		return true;
		//	}
		//	break;

		case E_VehicleLogicMsg_HitByPerk:
			{
				const C_VehicleLogicMsg_HitByPerkMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_HitByPerkMsg>(pMessage);

				if( mVehicleIndex == pMsg->mVehicleIndex )
					mHitByPerkRecentlyTimer = mpHoGParams->mHitByPerkTimer;

				return true;
			}
			break;

		case Game::E_VehicleMessage_ResetOnTrack:
			{
				const Game::Actors::C_VehicleResetOnTrack* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleResetOnTrack>(pMessage);

				if( g_debugEnableHandOfGod || pMsg->mOverride )
				{
					if(pMsg->mOverride)
					{
						mResetDirection = pMsg->mDirection;
						mResetPosition = pMsg->mPosition;
						mResetSpeed = pMsg->mVelocity;
					}
					ResetVehicleOnTrack(false, pMsg->mOverride, pMsg->mOverride );
				}
				return true;
			}
			break;

		case Game::E_VehicleMessage_Eliminated:
			{
				const Game::Actors::C_VehicleEliminationMessage* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleEliminationMessage>(pMessage);

				if(pMsg->mVehicleIndex == mVehicleIndex)
					mEnabled = false;

				return true;
			}
			break;

		case Game::E_VehicleMessage_ResetAfterWrecked:
			{
				const Game::Actors::C_VehicleResetAfterWreckedMsg* pMsg	= Messaging::MessageCast<Game::Actors::C_VehicleResetAfterWreckedMsg>(pMessage);
				if(pMsg->mVehicleIndex == mVehicleIndex)
				{
					HandleResetAfterWrecked();
				}
				return true;
			}
			break;

		case Game::E_GameflowMessage_RaceStarted:
			{
				mEnabled = true;

				if(mForcedResetZoneBuffered > 0)
				{
					mForcedResetZone += mForcedResetZoneBuffered;
					mForcedResetCameraTracking = mForcedResetCameraTrackingBuffered;
				}

				return true;
			}
			break;

		case Game::E_GameflowMessage_RaceFinished:
			{
				//mEnabled = false;
				return true;
			}
			break;

		case Game::E_VehicleMessage_ResetAtVehiclePosition:
			{
				const Game::Actors::C_VehicleResetAtVehiclePositionMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleResetAtVehiclePositionMsg>(pMessage);
				ResetVehicleAtVehiclePosition(pMsg->mResetVehicleIndex, false, true);

				return true;
			}
			break;

		case Game::E_VehicleMessage_ResetForceZone:
			{
				return HandleResetForceZone(pMessage);

			}
			break;

		case Game::E_VehicleMessage_ResetForceCameraZone:
			{
				const Game::Actors::C_VehicleResetForceCameraZone* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleResetForceCameraZone>(pMessage);

				if(mEnabled)
				{
					mForcedResetCameraTracking = pMsg->mCameraFollow;
					StartHoGCam();

					// In motormash we have no spline data so can never technically be 'off track'. 
					// However.. if we are starting the HOG camera we can be pretty sure we arent racing happily along terra firma. 
					Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
					if( levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Destruction )
					{
						Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
						if(pActor)
						{
							pActor->GetGraphicalActor(mVehicleIndex)->SetGoneOffTrack(true);
						}
					}
				}

				return true;
			}
			break;

		case Game::E_VehicleMessage_ResetEnteredCameraZone:
			{
				const Game::Actors::C_VehicleResetEnteredCameraZone* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleResetEnteredCameraZone>(pMessage);

				if(mEnabled)
				{
					mForcedResetZone++;
					mForcedResetCameraTracking = pMsg->mCameraFollow;
				}
				else
				{
					mForcedResetZoneBuffered++;
					mForcedResetCameraTracking = pMsg->mCameraFollow;
				}

				return true;
			}
			break;

		case Game::E_VehicleMessage_ResetExitedCameraZone:
			{	
				if(mEnabled)
					mForcedResetZone--;
				else
					mForcedResetZoneBuffered--;

				return true;
			}
			break;

		case Game::E_VehicleMessage_ResetTwoWayZone:
			{
				const Game::Actors::C_VehicleResetTwoWayZone* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleResetTwoWayZone>(pMessage);

				if(mEnabled)
				{
					if( mResetZoneActive )
					{
						if(mResetZoneId == pMsg->mResetZoneId && mResetIndivId != pMsg->mResetIndividualId)
						{
							mResetZoneTimer = 0.f;
							mResetIndivId = mResetZoneId = 0;
							mResetZoneActive = false;
						}
					}
					else
					{
						mResetZoneActive	= true;
						mResetZoneTimer		= pMsg->mTimer;
						mResetPosition		= pMsg->mResetPosition;
						mResetDirection		= pMsg->mResetDirection;
						mResetZoneId		= pMsg->mResetZoneId;
						mResetIndivId		= pMsg->mResetIndividualId;
						mResetSpeed			= 30;
					}
				}


				return true;
			}
			break;

		case Game::E_VehicleMessage_ResetEntryZone:
			{
				const Game::Actors::C_VehicleResetEntryZone* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleResetEntryZone>(pMessage);

				if(!mResetZoneActive && mEnabled)
				{
					mResetZoneActive	= true;
					mResetZoneTimer		= pMsg->mTimer;
					mResetPosition		= pMsg->mResetPosition;
					mResetDirection		= pMsg->mResetDirection;
					mResetZoneId		= pMsg->mResetZoneId;
					mResetSpeed			= 30;
				}

				return true;
			}
			break;

		case Game::E_VehicleMessage_ResetExitZone:
			{
				const Game::Actors::C_VehicleResetExitZone* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleResetExitZone>(pMessage);

				if( mResetZoneId == pMsg->mResetZoneId )
				{
					mResetZoneTimer = 0;
					mResetZoneActive = false;
				}

				return true;
			}
			break;

		case Game::E_GameLogicMessage_Impact:
			{
				const Game::C_ImpactMsg* pMsg = Messaging::MessageCast<Game::C_ImpactMsg>(pMessage);

				if(!mCollidedRecently)
				{
					const Actor::C_Actor* actorA = GetActorList()->GetActorPtr(pMsg->mActorHandleA);
					const Actor::C_Actor* actorB = GetActorList()->GetActorPtr(pMsg->mActorHandleB);

					bool vehicleCollision = (actorA && actorA->GetActorTypeId() == Game::E_ActorTypeId_Vehicle && actorB && actorB->GetActorTypeId() == Game::E_ActorTypeId_Vehicle);

					// Impulse must at least be noteworthy or vehicle-vehicle.
					if(vehicleCollision || pMsg->mCollisionInfo.m_GameplayImpulse >= mpHoGParams->mNoteworthyCollisionImpulse) 
						mVehicleImpactThisFrame = true;
				}

				return true;
			}

		case ActorMsg::E_DynamicObjMsg_SetVehicleLogic:
			{
				const ActorMsg::C_SetVehicleLogicMsg* pMsg = Messaging::MessageCast<ActorMsg::C_SetVehicleLogicMsg>(pMessage);
				mEnabled = pMsg->m_HandOfGod;			
				return true;
			}

		case Game::E_VehicleMessage_EnableDisableAutoHOGPlacement:
			{
				const Game::Actors::C_EnableDisableAutoHOGPlacementMsg* pMsg = Messaging::MessageCast<Game::Actors::C_EnableDisableAutoHOGPlacementMsg>(pMessage);
				mAutomaticHOGPlacementEnabled = pMsg->m_Enabled;			
				return true;
			}

		}	

		return I_GameActorComponent::OnMessage(pMessage);
	}

	bool C_VehicleComponent_HandOfGod::HandleResetForceZone( const Messaging::C_MessageBase * pMessage )
	{
		const Game::Actors::C_VehicleResetForceZone* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleResetForceZone>(pMessage);

		if(mEnabled)
		{
			int resetPosCount=pMsg->mResetPositions->count;
			if (resetPosCount==0)
			{
				mResetType = E_HandOfGodResetType_AtSpeedOnSpline;
				InitiateReset();
			}
			else
			{
				U32 randomNumber = rand();
				U32 value = randomNumber % resetPosCount;

				::Actors::C_GameActorCreate* actorCreate=(*pMsg->mResetPositions)[value];
				
				int i = 0;
				while(!actorCreate && i < resetPosCount)
				{
					LOGERROR(g_RLHandOfGodLogger, "There is a NULL random reset position in the list! Resetting to the first valid point we find.");

					actorCreate = (*pMsg->mResetPositions)[i];
					i++;
				}

				if(actorCreate)
				{
					mResetPosition=actorCreate->GetCreationMatrix().AW();
					mResetDirection=actorCreate->GetCreationMatrix().AZ();
					mResetSpeed = 0.f;
					mResetType = E_HandOfGodResetType_StationaryAtPoint;
					if (pMsg->mDelay>0.01f)
					{
						mResetDelayTimer=pMsg->mDelay;
					}
					else
					{
						InitiateReset();
					}
				}
				else
				{
					LOGERROR(g_RLHandOfGodLogger, "There are no valid reset positions! We cannot reset this vehicle.");
				}
			}
			
			//Was there foul play in this death?
			CheckForMurderOrSuicide();
		}

		return true;
	}

	//The vehicle is about to be dead. Was it a murder or a suicide?
	void C_VehicleComponent_HandOfGod::CheckForMurderOrSuicide()
	{
		//This vehicle's been forced to reset because its got into this zone thingy, which counts as a 
		//death. However, the vehicle could be already dead so we wouldn't want to double count that
		::Game::Actors::C_VehicleLogicActor *pLogicActor = ((::Game::Actors::C_VehicleLogicActor *) GetActor());
		if(pLogicActor->GetCurrentHealthLevel() <= E_HEALTH_LEVEL_WRECKED)
		{
			//They're already dead so can't have been murdered, however this might cause a double reset, once for the 
			//force reset and another when the wrecked timer kicks in so we'll quash that sucker here
			Game::Actors::C_CancelWreckedTimerMsg cancelWreckedTimerMsg(mVehicleIndex);
			GetActor()->SendMessageToComponent(Components::E_IF_VehicleComponent_Damage, &cancelWreckedTimerMsg);
			mResetDelayTimer = 0.f;
			return;
		}

		// EWW:This is nasty..
		// BUG: 36484 : if the lastplayer that hit us didnt hit us RECENTLY .. then its still a suicide!
		bool anotherPlayerCausedOurDeath = false;
		if(pLogicActor->mScoringInfo.mMostRecentVehicleIdxHitBy != INVALID_VEHICLE_IDX)
		{
			const C_VehicleComponent_NewScoring* pScoring = (Components::C_VehicleComponent_NewScoring*)pLogicActor->QueryInterface(E_IF_VehicleComponent_NewScoring);
			if(pScoring)
			{
				// [JBunner]: Bug 37373. If physically damaged by somebody recently OR somebody has CAUSED us to 'freefall over a cliff', its NOT a suicide. 
			    anotherPlayerCausedOurDeath  = pScoring->WasRecentlyHitByVehicle(pLogicActor->mScoringInfo.mMostRecentVehicleIdxHitBy) || 
											   pScoring->CurrentlyFreeFallingOverCliff(); // We only ever freefall over cliff if somebody has CAUSED us to go over. i.e. NEVER a suicide. 
			}
		}

		if(pLogicActor->mScoringInfo.mOutOfBoundsTimer && pLogicActor->mScoringInfo.mMostRecentVehicleIdxHitBy != INVALID_VEHICLE_IDX &&
		   anotherPlayerCausedOurDeath)
		{
			//Murder
			C_VehicleLogicMsg_VehicleMurdered msg(pLogicActor->mVehicleIndex, pLogicActor->mScoringInfo.mMostRecentVehicleIdxHitBy);
			pLogicActor->SendMessageToComponent(E_IF_VehicleComponent_Statistics, &msg);
			pLogicActor->SendMessageToComponent(E_IF_VehicleComponent_NewScoring, &msg);
		}
		else
		{
			//Suicide
			C_VehicleLogicMsg_VehicleSuicided msg(pLogicActor->mVehicleIndex);			
			pLogicActor->SendMessageToComponent(E_IF_VehicleComponent_Statistics, &msg);

			mDiedBySuicide = true;
		}

	}

	void C_VehicleComponent_HandOfGod::HandleResetAfterWrecked()
	{
		//Check to see whether we have AI data available...
		if(IsAIDataAvailable())
		{
			//...if we do we'll just respawn the car as with any other HoG incident (hopefully)
			mResetType = E_HandOfGodResetType_AtSpeedOnSpline;
		}
		else
		{
			//...if we don't we're probably in an arena type of track so we'll respawn to a random start position
			GetBestStartPosition(mResetPosition, mResetDirection, mResetSpeed);

			mResetType = E_HandOfGodResetType_AtSpeedAtPoint;
		}
		
		InitiateReset(true);

		// Send a new UI message if in motormash.. as a the UI should now show the new rival! (dont do before spawned as user wont see!)
		// New rival selected.. fire off a UI message incase we need to refresh the display
		Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
		if( levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Destruction && 
			!levelState.mTeams)
		{
			S32 playerIndex = UI::GetLocalPlayerIndex(mVehicleIndex); 
			if(playerIndex >= 0) // Make sure this is a player, not an AI...
			{
				_UI_MESSAGE1(UI::E_UiGameMsg_NewRivalSelected, playerIndex);
			}
		}
	}

	void C_VehicleComponent_HandOfGod::SendSuicideMessageToUI()
	{
		::Game::Actors::C_VehicleLogicActor *pLogicActor = ((::Game::Actors::C_VehicleLogicActor *) GetActor());
		// Display Suicide Message here
		UI::C_WideString128 str;
		str.Clear();
		str.Format( UI::C_TextStringID("MP_MOTORMASH_SUICIDE_POINT").String(), C_MultiplayerData::Instance().m_pConfig->m_MotormashScoringValues.m_SuicideScore );

		// PR - this is no longer supported
		//UI::C_UiGenericMessage uiMsg(str, UI::C_TextStringID("MP_SUICIDE").String(), pLogicActor->mVehicleIndex);
		//_UI_MESSAGE_FULL( uiMsg );
	}
} // namespace Components
