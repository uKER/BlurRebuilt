#include "stdafx.h"
#include <Game/Game.h>	// [Blur2PC]

#include <GameLogic\Pickups\ShockManager.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include <GameAudio/GameAudioLayer.h>
#include <GameAudio/PickupSubsystem.h>
#include <algorithm>

#include <GameApp.h>
#include <GameLogic\MultiplayerUtilities.h>
#include <GameLogic\Pickups\PickupsManager.h>
#include <GameLogic\RaceLogicCore\Database\RLDatabase.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <GameLogic/Pickups/PickupBase.h>
#include <ai\AI_Amax.h>

Debug::Int		g_debugShockManagerNumShocks("Perks.ShockManager.NumShocks", 3);
Debug::Int		g_debugShockManagerDistanceToMovePerShot("Perks.ShockManager.DistanceToMovePerShot", 5);
Debug::Float	g_debugShockManagerMultiShotBreatherDistancePerShock("Perks.ShockManager.MultiShotBreatherPerShock", 5.f);
Debug::Float	g_debugShockManagerMultiShotBreatherDistanceMin("Perks.ShockManager.MultiShotBreatherDistanceMin", 10.f);
Debug::Float	g_debugShockManagerMultiShotBreatherDistanceMax("Perks.ShockManager.MultiShotBreatherDistanceMax", 35.f);
Debug::Int		g_debugShockManagerCentreNumReq("Perks.ShockManager.CentreNumReq", 4);

Debug::Float	g_debugShockManagerResetCentreProb("Perks.ShockManager.CentreProbReset", 0.2f);
Debug::Float	g_debugShockManagerCentreProbInc("Perks.ShockManager.CentreProbInc", 0.1f);
Debug::Float	g_debugShockManagerWidthModLow("Perks.ShockManager.WidthModLow", 0.15f);
Debug::Float	g_debugShockManagerWidthModHigh("Perks.ShockManager.WidthModHigh", 0.35f);
Debug::Float	g_debugShockManagerDistancePerMph("Perks.ShockManager.DistancePerMph", 0.02);
Debug::Float	g_debugShockManagerTimeToNext("Perks.ShockManager.TimeToNext", 0.3f);
Debug::Float	g_debugShockManagerInitialDelay("Perks.ShockManager.InitialDelay", 1.0f);
Debug::Float	g_debugShockManagerVehicleTooFarBehindDistance("Perks.ShockManager.TooFarBehindDistance", 150.f);
Debug::Float	g_debugShockManagerIndexToCheckPerMph("Perks.ShockManager.IndexToCheckPerMph", 0.15f);

Debug::Float	g_debugOverlapProtectionDistance("Perks.ShockManager.OverlapProtectionDistance", 4.0f);

Debug::Int		g_debugShockManagerInitialDistance("Perks.ShockManager.InitialDistanceMin", 20); // 10, 30
Debug::Float	g_debugShockManagerInitialDistancePerMph("Perks.ShockManager.InitialDistancePerMph", 0.5f);
Debug::Float	g_debugShockManagerMinDistAhead("Perks.ShockManager.MinDistAhead", 50.f);
Debug::Float	g_debugShockManagerMaxDistAhead("Perks.ShockManager.MaxDistAhead", 150.f);
Debug::Float	g_debugShockManagerDistAheadMod("Perks.ShockManager.DistAheadMod", 0.025f);
Debug::Float	g_debugShockManagerDistAheadBreatherMod("Perks.ShockManager.DistAheadBreatherMod", 0.03f);

Debug::Float	g_debugShockManagerSplineWidthDiffForPriority("Perks.ShockManager.SplineWidthDiffForPriority", 10.f);
Debug::Float	g_debugShockManagerSplineDotDiffForPriority("Perks.ShockManager.SplineDotDiffForPriority", 0.002f);

Debug::Float	g_debugShockManagerDistanceLifetimeModifier("Perks.ShockManager.DistanceLifetimeModifier", 0.1f);
Debug::Float	g_debugShockManagerDistanceLifetimeModifierMax("Perks.ShockManager.DistanceLifetimeModifierMax", 10.f);
Debug::Float	g_debugShockManagerLastDropTime("Perks.ShockManager.LastDropTime", 3.f);

Debug::Float	g_debugShockManagerMinSpeedScrub("Perks.FakePhysics.Scaling.MinSpeedScrub", 10.f);

extern Debug::Float g_debugShockSizeDenominator;
extern Debug::Float g_debugShockMinWidth;
extern Debug::Float g_debugShockMaxWidth;
extern Debug::Float g_debugShockRayTraceHeight;
extern Debug::Float g_debugShockTraceGroundBuffer;
extern Debug::Float g_debugShockTraceObstacleBuffer;
extern Debug::Int	g_debugShockNumberCap;

// --------------------------------------------------------------------------------

namespace Game
{
	C_ShockManager::C_ShockManager() 
		: mCurrentDistanceAhead(0.f), mCentreProb(0.f), mShocksSinceLastCentre(0), mNextShockIdx(0), mTimeToNextShock(0.f), mTimeSinceLastPrimaryDrop(0.f)
	{
		mRandGenerator.Seed((U32)Time::GetCurrentTime());
		mOffsetLeft = (mRandGenerator.Rand(1) == 0);
	}

	// --------------------------------------------------------------------------------

	void C_ShockManager::Init(C_RaceLogicLayer* pLayer)
	{
		mpRaceLogicLayer = pLayer;
	}

	// --------------------------------------------------------------------------------

	C_ShockManager::~C_ShockManager()
	{
		ClearShocks();
	}

	// --------------------------------------------------------------------------------

	void C_ShockManager::Update()
	{
		if (!AIAmax::AmaxAIR::Instance().GetTrackDataSystem())
			return;
		
		PERF_EVENT("ShockManager.Update");
		float frameTime = C_GameApp::GetInstance()->GetTimeStep();
		if(mTimeToNextShock > 0.f)
		{
			mTimeToNextShock -= frameTime;

			if(mTimeToNextShock < 0)
				mTimeToNextShock = 0.f;
		}
		else if(mNextShockIdx < mShocksInSystem.Size())
		{
			ManagedShock* pShock = mShocksInSystem[mNextShockIdx];

			Maths::HMat43 matrix;
			matrix.AW(pShock->mPoint.mPosition);

			float distMod = -CheckVehicleDistance(pShock, pShock->mTargetIdx) * g_debugShockManagerDistanceLifetimeModifier;
			distMod = Maths::Clamp(distMod, 0.f, (F32)g_debugShockManagerDistanceLifetimeModifierMax);

			NetVehicleIndexStack vehicleList;
			GetLocalPlayerList(vehicleList);

			int idx = 0;
			if(vehicleList.Size())
				idx = vehicleList[0];

			PickupCreationStruct pickupInfo(E_PICKUP_TYPE_SHOCK, E_PICKUP_SUB_TYPE_NORMAL, pShock->mFirerIdx);
			pickupInfo.mTargetIdx			= pShock->mTargetIdx;
			pickupInfo.mMatrix				= matrix;
			pickupInfo.mDisableVehicleFX	= true;
			pickupInfo.mOverrideMatrix		= true;
			pickupInfo.mVal					= pShock->mPoint.mWidth;
			pickupInfo.mVal2				= distMod;
			pickupInfo.mPickupId			= C_PickupsManager::Instance().AllocateUniqueId(idx);
			pickupInfo.mHostIsController	= true;
			pickupInfo.mVal4 = pShock->mPoint.mLogicIdx;

			pShock->mUniqueId = C_PickupsManager::Instance().CreatePickup(pickupInfo);	

			mNextShockIdx++;
			mTimeToNextShock = g_debugShockManagerTimeToNext;
		}

		mTimeSinceLastPrimaryDrop += frameTime;
	}

	// --------------------------------------------------------------------------------

	void C_ShockManager::PlayFX(int firerIdx, int targetIdx, U32 id)
	{
		if(targetIdx != -1 && firerIdx != Game::SYSTEM_FIRING_INDEX)
		{
			Actors::C_VehicleLogicActor* pActor = mpRaceLogicLayer->GetLogicVehicleActor(firerIdx);

			RLPickupType* pPickup = (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_SHOCK);
			if(pPickup)
				C_InGameSpecialFXManager::Instance().CreateSpecialFX(pPickup->mNormal->mSpecialFX->mChargingSpecialFX, pActor->GetWorldMatrix(), firerIdx, -1, 
				id, E_InGameSpecialFX_Trigger_Charging);

			C_VehicleLogicMsg_FiredPerkMsg fired_msg(E_PICKUP_TYPE_SHOCK, 0, firerIdx, id, pActor->GetWorldMatrix(), 1, false, E_PICKUP_SUB_TYPE_NORMAL);
			Audio::InterfaceController::Get().SendMessageToSubsystem(C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Fire, 0, &fired_msg);
		}
	}

	// --------------------------------------------------------------------------------

	void C_ShockManager::RegisterShocks(int firerIdx, int targetIdx, U32 id)
	{
		if (!AIAmax::AmaxAIR::Instance().GetTrackDataSystem())
			return;

		PERF_EVENT("ShockManager.RegisterShocks");
		if(firerIdx != -1 && targetIdx != -1 && mShocksInSystem.Size() <= g_debugShockNumberCap)
		{
			Actors::C_VehicleLogicActor* pActor = mpRaceLogicLayer->GetLogicVehicleActor(targetIdx);
			float speed							= pActor->GetPositionInfo().mForwardSpeed;

			mInitialTargetPos = pActor->GetWorldMatrix().AW();

			Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
			Game::C_LevelStateMultiplayerVehicleAbilities* pMPAbilities = levelState.mCars[firerIdx].mUpgrades.GetMultiplayerAbilities();
			Game::C_MultiplayerVehicleAbility* pAbility = pMPAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Boss_AdditionalShockDomes);

			int numShocksToFire	= g_debugShockManagerNumShocks;
			if(pAbility->GetActive())
			{
				// This might need to move to VLC_Pickups if wanted for MP because only the host gets this.

				// This doesn't work in SP yet it seems
				//pMPAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Boss_PlusOneShock, true, false, firerIdx, targetIdx);
				numShocksToFire += pAbility->GetValue();
			}

			int numBackToCheck		= (int)Maths::Floor(g_debugShockManagerIndexToCheckPerMph * speed);
			int idxToCheck			= mShocksInSystem.Size() - numBackToCheck;
			idxToCheck				= Maths::Clamp(idxToCheck, -numShocksToFire, mShocksInSystem.Size() - 1);

			float dist				= 0.f;
			if(idxToCheck >= 0 && idxToCheck < mShocksInSystem.Size())
				dist = CheckVehicleDistance(mShocksInSystem[idxToCheck], targetIdx);

			//bool sameTarget			= mShocksInSystem.Size() > 0 && mShocksInSystem[mShocksInSystem.Size()-1]->mTargetIdx == targetIdx;
  
			if( dist < 0 || (mShocksInSystem.Size() > 0 && mTimeSinceLastPrimaryDrop < g_debugShockManagerLastDropTime) )
			{
				float distAhead = GetTargetDistanceAhead(targetIdx);
				distAhead = (g_debugShockManagerMaxDistAhead - distAhead) * g_debugShockManagerDistAheadBreatherMod;
				distAhead = Maths::Clamp(distAhead, g_debugShockManagerMinDistAhead * g_debugShockManagerDistAheadMod, g_debugShockManagerMaxDistAhead * g_debugShockManagerDistAheadMod);

				float breatherDist = mNumSequentialShocks * g_debugShockManagerMultiShotBreatherDistancePerShock;
				breatherDist += distAhead;
				breatherDist = Maths::Clamp(breatherDist, (F32)g_debugShockManagerMultiShotBreatherDistanceMin, (F32)g_debugShockManagerMultiShotBreatherDistanceMax);

				mCurrentDistanceAhead += breatherDist;
				mNumSequentialShocks++;
			}
			else
			{
				int distPerMph = (int)Maths::Floor(g_debugShockManagerInitialDistancePerMph * speed);
				mCurrentDistanceAhead = (g_debugShockManagerInitialDistance + distPerMph);
				mNumSequentialShocks = 0;
				mTimeSinceLastPrimaryDrop = 0.f;

				GetRoutes(targetIdx);
			}
			
			ManagedShock* prevShock = NULL;
			for( int i = 0; i < numShocksToFire; i++ )
			{
				ManagedShock* pShock = new ManagedShock();
				pShock->mFirerIdx			= firerIdx;
				pShock->mTargetIdx			= targetIdx;
				
				SetIndexFromOffset(pShock);

				bool foundRoute = false;
				for( int i = 0; i < mpCurrentRoutes.Size() - 1; i++ )
				{
					if(GenerateShockPosition(pShock, mpCurrentRoutes[i]))
					{
						foundRoute = true;
						break;
					}
				}

				// Try and find a new route
				if(!foundRoute)
				{
					airacingtech::ITrackDataAI* pTrackData			= AIAmax::AmaxAIR::Instance().GetTrackDataSystem();
					airacingtech::AITrackDataRoutes* pLogicRoute	= pTrackData->GetTrackData()->m_TrackLogicData.m_pLogicRoute;
					airacingtech::ISplineTracker* pSplineTracker	= AIAmax::AmaxAIR::Instance().GetAISplineTracker(targetIdx);
					int idx											= pSplineTracker->GetLogicRoute()->m_index;

					if(pLogicRoute)
					{
						float oldDistance = mCurrentDistanceAhead;
						Maths::HVec3 oldPos = mInitialTargetPos;

						Maths::HF32 offset(0.5f);
						int index2 = pLogicRoute->GetIndexOffset(idx, mCurrentDistanceAhead);
						if(index2 == 0)
							index2 = 1;

						if(index2 < pLogicRoute->GetNumSegments())
						{
							pLogicRoute->GetPositionFromOffset(offset, index2, mInitialTargetPos);	

							mCurrentDistanceAhead = 0.f;

							GetRoutes(targetIdx, index2);
							for( int i = 0; i < mpCurrentRoutes.Size() - 1; i++ )
							{
								if(GenerateShockPosition(pShock, mpCurrentRoutes[i]))
								{
									foundRoute = true;
									break;
								}
							}

							if(!foundRoute)
							{
								mCurrentDistanceAhead = oldDistance;
								mInitialTargetPos = oldPos;
							}
						}
					}
				}

				// Use the logic
				if(!foundRoute && mpCurrentRoutes.Size())
				{
					GenerateShockPosition(pShock, mpCurrentRoutes[mpCurrentRoutes.Size() - 1]);
				}

				// This is a safety measure to stop them overlaying
				bool canInsert = true;
				RLPickupType* pPickup		= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_SHOCK);
				RLShockBalancingData* pData	= (RLShockBalancingData*)pPickup->mNormal->mBalancingData;
				int radius					= pData->mOuterRadius;

				for( int i = 0; i < mShocksInSystem.Size(); i++ )
				{
					float dist = Maths::Distance(mShocksInSystem[i]->mPoint.mPosition, pShock->mPoint.mPosition);
					if(dist < (mShocksInSystem[i]->mPoint.mWidth * radius) + (pShock->mPoint.mWidth * radius) + g_debugOverlapProtectionDistance)
					{
						canInsert = false;
						break;
					}
				}

				// Give each 
				
				if(canInsert)
				{
				
					if(prevShock)
					{
						prevShock->mNext = pShock;
					}
					pShock->mPrev = prevShock;
					pShock->mNext = NULL;

					mShocksInSystem.PushBack(pShock);
					
					prevShock = pShock;
				}
				else
				{
					SAFE_DELETE_PTR(pShock);
				}
			}

			if(mTimeToNextShock == 0.f)
				mTimeToNextShock = g_debugShockManagerInitialDelay;
		}
	}

	// --------------------------------------------------------------------------------

	void C_ShockManager::ClearShocks()
	{
		for( int i = 0; i < mShocksInSystem.Size(); i++ )
			SAFE_DELETE_PTR(mShocksInSystem[i]);

		mShocksInSystem.Clear();

		mCurrentDistanceAhead = mCentreProb = mTimeToNextShock = 0.f;
		mShocksSinceLastCentre = mNextShockIdx = 0;
	}

	// --------------------------------------------------------------------------------

	void C_ShockManager::RemoveShock(U32 id)
	{
		PERF_EVENT("ShockManager.RemoveShock");

		for( Btl::Vector<ManagedShock*>::Iterator it = mShocksInSystem.Begin(); it != mShocksInSystem.End(); it++ )
		{
			ManagedShock* pShock = *it;
			if(pShock->mUniqueId == id)
			{
				C_PickupBase* pPickup = C_PickupsManager::Instance().GetPickup(id);
				if(pPickup)
				{
					// Only check if we have not yet missed a target
					if(!pShock->mMissedATarget)
					{
						// I either missed everybody or hit myself
						if(!pPickup->GetDidHitTarget(-1) || pPickup->GetDidHitTarget(pShock->mFirerIdx))
						{
							pShock->mMissedATarget = true;

							// Mark everyone in our shock group as missed.
							ManagedShock* pCurrentShock = pShock;
							while( pCurrentShock->mPrev )
							{
								pCurrentShock = pCurrentShock->mPrev;
							}

							while( pCurrentShock )
							{
								pCurrentShock->mMissedATarget = true;
								pCurrentShock = pCurrentShock->mNext;
							}
						}
						else if(!pShock->mPrev && !pShock->mNext)
						{
							// I hit someone with every dome! Happy days!
							if(VehicleIsLocal(pShock->mFirerIdx))
							{
								C_VehicleLogicMsg_HitWithAllDomes msg(pShock->mFirerIdx);
								C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&msg);
							}
							else
							{
								Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
								C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_EventManager->Add_HitAllShockEvent(levelState.mCars[pShock->mFirerIdx].mNetJoinId);
							}
						}
					}

					// If the target has has already collided with 
					// one of the shocks in our group we don't need
					// to continue checking.
					if(!pShock->mHitTarget)
					{

						// Are we being destroyed because we 
						// hit our target?
						if( pPickup->GetDidHitTarget(pShock->mTargetIdx))
						{
							// If we did hit our target we need to invalidate
							// the other managed shock entities in our group
							// so that we do not award a scoring event for them.

							ManagedShock* pCurrentShock = pShock;
							// Iterate through all of the remaining shocks in 
							// our group and mark them as target hit.
							// First we need to iterate to the beginning of our list
							while( pCurrentShock->mPrev )
							{
								pCurrentShock = pCurrentShock->mPrev;
							}

							// now iterate forwards marking them as target hit.
							while( pCurrentShock )
							{
								pCurrentShock->mHitTarget = true;
								pCurrentShock = pCurrentShock->mNext;
							}
						}
						else 
						{
							// If mHitTarget is still not set then we
							// have not yet hit any of the shocks in this group.
							// Here we check to see if we are the last shock
							// in our group and if so award the scoring event 
							// for avoiding all the shocks.
							if(!pShock->mPrev && !pShock->mNext)
							{
								// so we are the last shock of this group 
								// and the target didn't hit any of the 
								// shocks in this group so lets inform of
								// a scoring event
								if(pShock->mTargetIdx != -1)
								{
									// If it was us (host) that evaded the shock, than notch up progress directly
									if(Game::VehicleIsLocal(pShock->mTargetIdx))
									{
										Game::Actors::C_VehicleLogicActor* pLogicActor = mpRaceLogicLayer->GetLogicVehicleActor(pShock->mTargetIdx);
										BIZ_ASSERT_MSG(pLogicActor, "Shock Evader Failed to get pLogicactor");

										C_VehicleLogicMsg_EvadedPowerUp msg(pShock->mTargetIdx, pShock->mFirerIdx, E_PICKUP_TYPE_SHOCK, 0);
										pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_NewScoring, &msg);

										C_VehicleLogicMsg_AvoidedAllDomes avoidMsg(pShock->mFirerIdx, pShock->mTargetIdx);
										C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&avoidMsg);

									}
									else if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork())
									{
										Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

										// If it is a network game, then need to send a net message to tell the vehicle in question they made some progress!
										C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_EventManager->Add_EvadedShockEvent(levelState.mCars[pShock->mTargetIdx].mNetJoinId, levelState.mCars[pShock->mFirerIdx].mNetJoinId);
									}
								}
							}
						}
					}
				}

				// Remove ourselves from our group based linked list
				if(pShock->mPrev)
				{
					pShock->mPrev->mNext = pShock->mNext;
				}
				if(pShock->mNext)
				{
					pShock->mNext->mPrev = pShock->mPrev;
				}

				// It is now safe to delete the shock and clean up.
				SAFE_DELETE_PTR(pShock);
				mShocksInSystem.Erase(it); // Can't swap/pop - order matters.
				mNextShockIdx--;

				break;
			}
		}

		if(!mShocksInSystem.Size())
		{
			mCentreProb = mCurrentDistanceAhead = 0.f;
			mShocksSinceLastCentre = 0;
			mpCurrentRoutes.Destroy();
		}
	}

	// --------------------------------------------------------------------------------

	float C_ShockManager::CheckVehicleDistance(ManagedShock* pShock, int vehIdx)
	{
		if (!AIAmax::AmaxAIR::Instance().GetTrackDataSystem())
			return 0.0f;

		airacingtech::ITrackDataAI *pTrackData			= AIAmax::AmaxAIR::Instance().GetTrackDataSystem();
		airacingtech::AITrackDataRoutes* pLogicRoute	= pTrackData->GetTrackData()->m_TrackLogicData.m_pLogicRoute;
		airacingtech::ISplineTracker* pSplineTracker	= AIAmax::AmaxAIR::Instance().GetAISplineTracker(vehIdx);
		int idx											= pSplineTracker->GetLogicRoute()->m_index;

		if(pLogicRoute && pSplineTracker->GetLogicRoute()->m_in && idx != -1 && pTrackData)
		{
			int numSegments = pLogicRoute->GetNumSegments();
			return (airacingtech::DistanceInfrontBehind(idx, pShock->mPoint.mLogicIdx, numSegments));		
		}

		return 0.0f;
	}

	// --------------------------------------------------------------------------------

	void C_ShockManager::TraceToGround(Maths::HVec3& pos)
	{
		PERF_EVENT("ShockManager.TraceToGround");

		Maths::HMat43 mat;
		mat.SetIdentity();
		mat.AW(pos);

		Game::Actors::RaceLogicCore* pCore = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

		U8 data[1024 * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> stack(data, 1024);
		Collision::C_Ray ray;
		Collision::C_CollisionFilter rayCollisionFilter;
		rayCollisionFilter.SetCollisionMask(PolyFlags::kWheelSolid);
		rayCollisionFilter.SetBodyMask(PolyFlags::kbWorld);

		Maths::HVec3 up(0,1,0);

		Maths::HVec3 GodLightningPos = mat.AW() + up * g_debugShockRayTraceHeight;
		Maths::HVec3 ThroughGroundPos = mat.AW() - up * g_debugShockRayTraceHeight;
		ray.InitialiseRay(GodLightningPos, ThroughGroundPos, &rayCollisionFilter);

		pCore->GetPhysicsScene()->DoRayTrace(ray, stack);
		Collision::C_RayTracingResult& result = ray.m_result;

		if(result.Hit())
		{
			pos = result.HitPoint();
			pos.Y(pos.Y() + g_debugShockTraceGroundBuffer);
		}
	}

	// --------------------------------------------------------------------------------

	void C_ShockManager::TraceToObstacles(Maths::HVec3& pos)
	{
		PERF_EVENT("ShockManager.TraceToObstacles");

		Maths::HMat43 mat;
		mat.SetIdentity();
		mat.AW(pos);

		Game::Actors::RaceLogicCore* pCore = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

		U8 data[1024 * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> stack(data, 1024);
		Collision::C_Ray ray;
		Collision::C_CollisionFilter rayCollisionFilter;
		rayCollisionFilter.SetCollisionMask(PolyFlags::kWheelSolid);
		rayCollisionFilter.SetBodyMask(PolyFlags::kbWorld);

		Maths::HVec3 up(0,1,0);

		Maths::HVec3 godLightningPos = mat.AW() + up * g_debugShockRayTraceHeight;
		ray.InitialiseRay(mat.AW(), godLightningPos, &rayCollisionFilter);

		pCore->GetPhysicsScene()->DoRayTrace(ray, stack);
		Collision::C_RayTracingResult& result = ray.m_result;

		if(result.Hit())
		{
			pos = result.HitPoint();
			pos.Y(pos.Y() + g_debugShockTraceObstacleBuffer);
		}
	}

	// --------------------------------------------------------------------

	void C_ShockManager::SetIndexFromOffset(ManagedShock* pShock)
	{
		if(mpCurrentRoutes.Size() > 0)
		{
			airacingtech::AITrackDataRoutes* pLogicRoute = mpCurrentRoutes[mpCurrentRoutes.Size()-1].mpRoute;
			int idx = mpCurrentRoutes[mpCurrentRoutes.Size()-1].mIndex;

			if(pLogicRoute)
			{
				int index2 = pLogicRoute->GetIndexOffset(idx, mCurrentDistanceAhead);
				if(index2 == 0)
					index2 = 1;

				pShock->mPoint.mLogicIdx = index2;
			}
		}
	}

	// --------------------------------------------------------------------

	void C_ShockManager::GetRoutes(int targetIdx, int idxOverride)
	{
		PERF_EVENT("ShockManager.GetRoutes");

		airacingtech::ITrackDataAI *pTrackData			= AIAmax::AmaxAIR::Instance().GetTrackDataSystem();
		if (!pTrackData)
			return;

		airacingtech::AITrackDataRoutes* pLogicRoute	= pTrackData->GetTrackData()->m_TrackLogicData.m_pLogicRoute;
		airacingtech::ISplineTracker* pSplineTracker	= AIAmax::AmaxAIR::Instance().GetAISplineTracker(targetIdx);
		int idx											= pSplineTracker->GetLogicRoute()->m_index;
		int logicIdx									= -1;

		if(idxOverride != -1)
			idx = idxOverride;

		mpCurrentRoutes.Clear();

		if(pLogicRoute && pSplineTracker->GetLogicRoute()->m_in && idx != -1 && pTrackData)
		{
			const airacingtech::AITrackDataCache& dc = pLogicRoute->m_dataCache[idx];

			for(int entries = 0; entries < dc.m_data.Size(); entries++ )
			{
				const airacingtech::AITrackDataCachedData& cachedData = dc.m_data[entries];

				if(cachedData.m_pRoute && (cachedData.m_pRoute->IsChangeableRoute() || cachedData.m_pRoute->IsLogic()))
				{
					int cacheIdx = cachedData.m_index;
					airacingtech::AITrackDataRoutes* pCachedRoute = cachedData.m_pRoute;

					if(pCachedRoute)
					{
						for( int idxOffset = -2; idxOffset <= 8; idxOffset++ )
						{
							int checkIdx = pCachedRoute->GetIndexOffset(cacheIdx, idxOffset);
							if(checkIdx == 0)
								checkIdx = 1;

							if(pCachedRoute->InSegmentBounds(mInitialTargetPos, checkIdx))
							{
								int index2 = pCachedRoute->GetIndexOffset(cacheIdx, mCurrentDistanceAhead);
								if(index2 == 0)
									index2 = 1;

								if(index2 != 0 && index2 != pCachedRoute->GetNumSegments() - 1)
								{
									ShockCachedRoute route;
									route.mIndex	= cachedData.m_index;
									route.mpRoute	= cachedData.m_pRoute;
									route.mRandVal	= mRandGenerator.FRand(0,10);

									mpCurrentRoutes.PushBack(route);
								}
								
								break;
							}
						}
					}
				}
			}

			ShockCachedRoute route;
			route.mIndex	= idx;
			route.mpRoute	= pLogicRoute;
			route.mRandVal	= 11;
			mpCurrentRoutes.PushBack(route);
		}

		Actors::C_VehicleLogicActor* pTargetActor = mpRaceLogicLayer->GetLogicVehicleActor(targetIdx);
		ShockSort sort(pTargetActor);
		std::sort(mpCurrentRoutes.Begin(), mpCurrentRoutes.End(), sort);
	}

	// --------------------------------------------------------------------------------

	bool C_ShockManager::GenerateShockPosition(ManagedShock* pShock, ShockCachedRoute& route)
	{
		PERF_EVENT("ShockManager.GenerateShockPosition");

		if (!AIAmax::AmaxAIR::Instance().GetTrackDataSystem())
			return false;

		int cacheIdx	= route.mIndex;

		int index2 = route.mpRoute->GetIndexOffset(cacheIdx, mCurrentDistanceAhead);
		if(index2 == 0)
			index2 = 1;

		if((index2 > 1 && index2 < route.mpRoute->GetNumSegments() - 1) || route.mpRoute->IsLogic())
		{
			int racingLine = airacingtech::AITrackDataLines::TRACK_DATA_EXTREME_RIGHT;
			if(mOffsetLeft)
				racingLine = airacingtech::AITrackDataLines::TRACK_DATA_EXTREME_LEFT;

			Maths::HVec3 pos;
			route.mpRoute->GetPositionFromLine(racingLine, index2, pos);

			float width = route.mpRoute->GetWidth(index2);
			Maths::HVec3 dir;
			route.mpRoute->GetDirection(index2, dir);

			Maths::HVec3 perpVec(-dir.Z(), dir.Y(), dir.X());

			//float randVal = mRandGenerator.FRand(0.f, 1.f);
			//if(randVal <= mCentreProb && mShocksSinceLastCentre >= g_debugShockManagerCentreNumReq)
			//{
			//	mCentreProb = g_debugShockManagerResetCentreProb;
			//	mShocksSinceLastCentre = 0;
			//}
			//else
			//{
				mShocksSinceLastCentre++;
				mCentreProb += g_debugShockManagerCentreProbInc;

				float widthMod = mRandGenerator.FRand(g_debugShockManagerWidthModLow, g_debugShockManagerWidthModHigh);									

				if(mOffsetLeft)
					pos -= perpVec * width * widthMod;
				else
					pos += perpVec * width * widthMod;
			//}

			pos.Y(mInitialTargetPos.Y() + 3.f);

			TraceToGround(pos);			

			Maths::HVec3 newPos = pos;
			for( float i = -1; i <= 1; i++ )
			{
				for( float j = -1; j <= 1; j++ )
				{
					newPos.X(newPos.X() + i);
					newPos.Y(pos.Y());
					newPos.Z(newPos.Z() + j);
					TraceToGround(newPos);		

					if(newPos.Y() < pos.Y())
						pos = newPos;
				}
			}

			TraceToObstacles(pos);

			Game::C_LevelState& levelState	= Game::C_Game::GetInstance()->GetLevelState();	
			AmaxRoute* pRoute = C_GameData::Instance().mRoutes[(GAMEDATA_ID)levelState.mRoute];

			float extremeWidthWeighting = 1.f;
			if(pRoute && pRoute->shockWidthWeighting > 0.f)
				extremeWidthWeighting = pRoute->shockWidthWeighting;

			pShock->mPoint.mPosition = pos;
			pShock->mPoint.mWidth = Maths::Clamp((width * extremeWidthWeighting)/ (F32)g_debugShockSizeDenominator, (F32)g_debugShockMinWidth, (F32)g_debugShockMaxWidth);

			Actors::C_VehicleLogicActor* pActor = mpRaceLogicLayer->GetLogicVehicleActor(pShock->mTargetIdx);
			int distPerMph						= (int)Maths::Floor(g_debugShockManagerDistancePerMph * pActor->GetPositionInfo().mForwardSpeed);

			float distAhead = GetTargetDistanceAhead(pShock->mTargetIdx);
			distAhead = (g_debugShockManagerMinDistAhead + (g_debugShockManagerMaxDistAhead - distAhead)) * g_debugShockManagerDistAheadMod;
			distAhead = Maths::Clamp(distAhead, 0.f, g_debugShockManagerMaxDistAhead * g_debugShockManagerDistAheadMod);

			mCurrentDistanceAhead += g_debugShockManagerDistanceToMovePerShot + distPerMph + distAhead;
			mOffsetLeft = !mOffsetLeft;

			return true;
		}

		return false;
	}

	// --------------------------------------------------------------------------------

	void C_ShockManager::RegisterNetworkShockHit(U32 vehHitIndex, U32 pickupId, E_PICKUP_HIT_TYPE hitType)
	{
		// Sanity check
		if(mShocksInSystem.Empty())
		{
			return;
		}

		// Make sure we find our local instance of this shock (if it still exists....)
		for( Btl::Vector<ManagedShock*>::Iterator it = mShocksInSystem.Begin(); it != mShocksInSystem.End(); it++ )
		{
			ManagedShock* pShock = *it;

			// Is this the shock we are interested in?
			if(pShock->mUniqueId == pickupId)
			{
				// [J BUNNER] - Bug 38362 - MP shock 'hit all domes' etc not always registering - 
				// As we know at this point we have definitely hit *someone* (may or may not be target)...
				C_PickupBase* pPickup = C_PickupsManager::Instance().GetPickup(pickupId);
				if(pPickup)
				{
					pPickup->RecordHit(vehHitIndex,hitType); 
				}

				// has it hit its target? 
				if(pShock->mTargetIdx == vehHitIndex) 
				{
					// Set the flag so that we correctly dont assign evasion score for this!
					pShock->mHitTarget = true;

					// Iterate through all of the remaining shocks in 
					// our group and mark them as target hit.
					// First we need to iterate to the beginning of our list
					while( pShock->mPrev )
					{
						pShock = pShock->mPrev;
					}

					// now iterate forwards marking them as target hit.
					while( pShock )
					{
						pShock->mHitTarget = true;
						pShock = pShock->mNext;
					}
				}
			}
		}
	}

	// --------------------------------------------------------------------------------

	float C_ShockManager::GetTargetDistanceAhead(int idx)
	{
		PERF_EVENT("ShockManager.GetTargetDistanceAhead");

		Actors::C_VehicleLogicActor* pActor				= mpRaceLogicLayer->GetLogicVehicleActor(idx);
		airacingtech::ISplineTracker* pSplineTracker	= AIAmax::AmaxAIR::Instance().GetAISplineTracker(idx);

		float distAhead = 0.f;
		if(pActor->GetPositionInfo().mRacePosition == 1 && pSplineTracker)
		{
			const airacingtech::RouteInfo* pLogicRoute = pSplineTracker->GetLogicRoute();
			if(pLogicRoute && pLogicRoute->m_pRoute)
			{
				for( int i = 0; i < mpRaceLogicLayer->GetNumVehicles(); i++ )
				{
					Actors::C_VehicleLogicActor* pOtherActor = mpRaceLogicLayer->GetLogicVehicleActor(i);
					if(pOtherActor->GetPositionInfo().mRacePosition == pActor->GetPositionInfo().mRacePosition + 1)
					{
						int numSegs = pLogicRoute->m_pRoute->GetNumSegments();

						float distRoundTrack		= (pActor->GetPositionInfo().mLogicPositionCombined * numSegs);
						float otherDistRoundTrack	= (pOtherActor->GetPositionInfo().mLogicPositionCombined * numSegs);

						distAhead = distRoundTrack - otherDistRoundTrack;
					}
				}
			}
		}

		return distAhead;
	}

	bool ShockSort::operator ()(ShockCachedRoute& lhs, ShockCachedRoute& rhs)
	{
		float lWidth = lhs.mpRoute->GetWidth(lhs.mIndex);
		float rWidth = rhs.mpRoute->GetWidth(rhs.mIndex);

		if(lhs.mpRoute && lhs.mpRoute->IsLogic())
			return false;

		if(rhs.mpRoute && rhs.mpRoute->IsLogic())
			return true;

		float widthDiff = Maths::Abs(lWidth - rWidth);
		if(widthDiff > g_debugShockManagerSplineWidthDiffForPriority)
		{
			if(lWidth > rWidth)
				return true;
			else
				return false;
		}

		if(lhs.mpRoute && rhs.mpRoute && pTargetActor)
		{
			Maths::HVec3 lhsDir, rhsDir;
			lhs.mpRoute->GetDirection(lhs.mIndex, lhsDir);
			rhs.mpRoute->GetDirection(rhs.mIndex, rhsDir);
			
			Maths::HVec3 vehDir = pTargetActor->GetPositionInfo().mDirection;

			Maths::HVec3 lhsDiffVec = vehDir - lhsDir;
			Maths::HVec3 rhsDiffVec = vehDir - rhsDir;

			float lhsDot = Maths::DotProduct(lhsDiffVec, vehDir);
			float rhsDot = Maths::DotProduct(rhsDiffVec, vehDir);

			float dotDiff = Maths::Abs(lhsDot - rhsDot);

			if(dotDiff > g_debugShockManagerSplineDotDiffForPriority)
			{
				if(lhsDot > rhsDot)
					return false;
				else
					return true;
			}
		}

		if(lhs.mRandVal < rhs.mRandVal)
			return true;

		return false;
	}
}

