#include "stdafx.h"
#include <GameLogic/Pickups/PickupClasses.h>	// [Blur2PC]

#include <GameLogic/Pickups/Normal/PickupBarge.h>

#include <GameApp.h>
#include <Game/game.h>

#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <GameLogic/Layers/RaceBookLayer.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups.h>
#include <GameLogic\Pickups\PickupsManager.h>
#include <GameLogic\StickerManager.h>
#include <GameLogic/Pickups/PickupMineBase.h>
#include <GameLogic/Pickups/Normal/PickupShunt.h>

Debug::Bool g_debugBargeDraw("Perks.DebugDraw.Barge", false);
Debug::Bool g_debugBargeHold("Perks.DebugDraw.BargeHold", false);
Debug::Float g_debugBargeIdiotLeniency("Perks.Barge.IdiotLeniency", 3.0f);
Debug::Float g_debugBargeOuterRadius("Perks.Barge.OuterRadius", 2.0f);
Debug::Float g_debugBargeRadiusAddition("Perks.Barge.RadiusAddition", 0.5f);
Debug::Float g_debugBargeRadiusExtentsFudgeAverage("Perks.Barge.RadiusExtentsFudgeAverage", 1.75f);
Debug::Float g_debugBargeMajorRadius("Perks.Barge.MajorRadiusAddition", 0.0f);
Debug::Float g_splashDamageBargeMultiplier("Multiplayer.Upgrades.SplashDamage.BargeMultiplier", 1.22f);
Debug::Int g_dbMaxDamageObjectsBarge("Perks.Barge.MaxObjectsToDamage", 3, Debug::Range(1, 100));

namespace Game
{
	C_PickupBarge::C_PickupBarge(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX) 
		: C_PickupBase(E_PickupClass_Barge, id, firingIdx, E_PICKUP_TYPE_BARGE, subType, 
		 true //dont pass true value of noFiringVehicleFX through as we dont want the base class firing the charging effect. We conditionally choose which one to do in this constructor
	)
	{
		RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BARGE);
		mpBargeData							= (RLBargeBalancingData*)pPickup->mNormal->mBalancingData;

		BIZ_ASSERT(mpBargeData);

		LevelStateCar& levelCar = C_Game::GetInstance()->GetLevelState().mCars[firingIdx];
		BargeEffectInfo& bargeInfo = levelCar.mPickupInfo.BargeInfo();

		mNoFiringVehicleFX = noFiringVehicleFX;
		mMajorRadius = mpBargeData->mRadius + g_debugBargeMajorRadius;
		mMinorRadius = mpBargeData->mRadius;
		mFixedPosition = bargeInfo.mFixedPosition;

		if(subType == E_PICKUP_SUB_TYPE_MOD)
		{
			MultiplayerConfig* pMultiplayerConfig = C_MultiplayerData::Instance().m_pConfig;
			BIZ_ASSERT_MSG(pMultiplayerConfig, "Could Not Get Multiplayer Config for Last Gasp Mod");
			LastGaspModSettings& modSettings = pMultiplayerConfig->m_LastGaspModSettings;

			mMajorRadius *= modSettings.m_bargeRadiusModifier;
			mMinorRadius *= modSettings.m_bargeRadiusModifier; 
			mSizeMultiplier = modSettings.m_bargeRadiusModifier;
		}
		else
		{
			// The Splash Damage mod increases the radius of barge -- hack to increase the size of the effect
			if( HasAbility(UI::E_MultiplayerVehicleAbilities_Splash_Damage, firingIdx) )
			{
				mMajorRadius *= g_splashDamageBargeMultiplier;
				mMinorRadius *= g_splashDamageBargeMultiplier; 
				C_Game::GetInstance()->GetLevelState().mCars[firingIdx].mUpgrades.GetMultiplayerAbilities()->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Splash_Damage, true);
				mSizeMultiplier = g_splashDamageBargeMultiplier;
			}
			else
			{
				mSizeMultiplier = 1.0f;
			}
		}

		mSizeMultiplier *= (mpBargeData->mRadius / 10.f);
		mScale = Maths::HVec3(mSizeMultiplier, mSizeMultiplier, mSizeMultiplier);

		mTimer = mpBargeData->mDuration;

		mMajorRadius += g_debugBargeRadiusAddition;
		mMinorRadius += g_debugBargeRadiusAddition;
	}

	// --------------------------------------------------------------------------------

	void C_PickupBarge::FireChargingSpecialFX()
	{
		RLPickupLevel* pLevel = GetRLSubTypeLevel();
		if(pLevel)
		{
			if(!mNoFiringVehicleFX && pLevel->mSpecialFX)
			{
				if(mPickupSubType == E_PICKUP_SUB_TYPE_MOD)
				{
					C_InGameSpecialFXManager::Instance().CreateSpecialFX(((RLBargeSpecialFX*)pLevel->mSpecialFX)->mLastGaspChargingSpecialFX, mMatrix, mFiringIdx, -1, mUniqueId, E_InGameSpecialFX_Trigger_Charging);
				}
				else
				{
					if( HasAbility(UI::E_MultiplayerVehicleAbilities_Splash_Damage, mFiringIdx) )
					{
						C_InGameSpecialFXManager::Instance().CreateSpecialFX(((RLBargeSpecialFX*)pLevel->mSpecialFX)->mSplashDamageChargingSpecialFX, mMatrix, mFiringIdx, -1, mUniqueId, E_InGameSpecialFX_Trigger_Charging);
					}
					else
					{
						C_InGameSpecialFXManager::Instance().CreateSpecialFX(pLevel->mSpecialFX->mChargingSpecialFX, mMatrix, mFiringIdx, -1, mUniqueId, E_InGameSpecialFX_Trigger_Charging);
					}
				}
			}
		}
	}

	// --------------------------------------------------------------------------------
	
	bool C_PickupBarge::Update()
	{
		PERF_EVENT("PickupBarge.Update");

		if(g_debugBargeDraw)
			DebugDraw();

		if(!mActivated || g_debugBargeHold)
			return false;

		if(mKillPickup || mTimer <= 0.f)
			return true;

		mTimer -= C_GameApp::GetInstance()->GetTimeStep();

		C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pFiringActor = pLayer->GetLogicVehicleActor(mFiringIdx);
		for( int i = 0; i < pLayer->GetNumVehicles(); i++ )
		{
			if( mFiringIdx != i && CheckCanHit(i) && !mVehiclesShielded[i])
			{
				Actors::C_VehicleLogicActor* pTargetActor = pLayer->GetLogicVehicleActor(i);

				// Have to check the vehicles hit, because we don't want to trigger if EITHER have been hit
				Maths::HVec3 extents = pTargetActor->GetGraphicalActor(i)->GetExtents();
				float extendsFudge = (extents.X() + extents.Z()) / 2;
				extendsFudge -= g_debugBargeRadiusExtentsFudgeAverage;
				if(IsInsideEllipse(pTargetActor->GetPositionInfo().mWorldPosition, extendsFudge) && !mVehiclesHitMajor[i] && !mVehiclesHitMinor[i])
				{
					Maths::HMat43 mat = pTargetActor->GetGraphicalActor(i)->GetMatrix();
					mHitInfo.mDirection = pTargetActor->GetPositionInfo().mWorldPosition - pFiringActor->GetPositionInfo().mWorldPosition;
					mHitInfo.mDirection.Normalise();
					mHitInfo.mDirection = mHitInfo.mDirection * mat.Rotation().Inverse();
					mHitInfo.mPosition = pFiringActor->GetPositionInfo().mWorldPosition;

					if(IsInsideEllipse(pTargetActor->GetPositionInfo().mWorldPosition, -g_debugBargeOuterRadius + extendsFudge))
					{
						RegisterHit(i);	
					}
					else
					{
						RegisterHit(i, E_PICKUP_HIT_TYPE_MINOR);	
					}
				}
			}
		}

		CheckDynamicObjects();
		
		if(!mFixedPosition)
			mMatrix = pFiringActor->GetWorldMatrix();

		if(VehicleIsLocal(mFiringIdx))
		{
			ActivePickupList& pickupList = C_PickupsManager::Instance().GetActivePickupList();
			for( ActivePickupList::Iterator it = pickupList.Begin(); it != pickupList.End(); it++ )
			{
				C_PickupBase* pPickup = it->GetValue();
				if(pPickup)
				{
					float dist = Maths::Distance(pPickup->GetMatrix().AW(), mMatrix.AW());
					if(IsInsideEllipse(pPickup->GetMatrix().AW(), g_debugBargeIdiotLeniency))
					{
						switch(pPickup->GetType())
						{
						case E_PICKUP_TYPE_SHUNT:
							{
								C_PickupShuntProjectile* pShunt = (C_PickupShuntProjectile*)pPickup;
								pShunt->CollideWithPickup(mFiringIdx);
								ReportProjectileDestroyedEvent(pPickup); 
								break;
							}

						case E_PICKUP_TYPE_BLINDFIRE:
							{
								pPickup->KillPickup(mFiringIdx, E_PICKUP_CAUSE_OF_DEATH_HIT_OTHER_PICKUP);
								ReportProjectileDestroyedEvent(pPickup); 
								break;
							}
						case E_PICKUP_TYPE_BOMB:
							{
								pPickup->KillPickup(mFiringIdx, E_PICKUP_CAUSE_OF_DEATH_HIT_OTHER_PICKUP);
								ReportProjectileDestroyedEvent(pPickup);
							}

						/*case E_PICKUP_TYPE_BOMB:
							{
								if(pPickup->GetSubType() == E_PICKUP_SUB_TYPE_NORMAL)
								{
									C_PickupMine* pMine = (C_PickupMine*)pPickup;
									Maths::HVec3 dir	= Maths::Normalise(pPickup->GetMatrix().AW() - mMatrix.AW());
									float carSpeed		= pFiringActor->GetPositionInfo().mSpeedInMph;
									float force			= (mpBargeData->mMinePushForceMultiplier * carSpeed) + mpBargeData->mMinMinePushForce;

									dir.Y(0.f);
									pMine->ApplyForce(dir, force);
								}
								else if( pPickup->GetSubType() == E_PICKUP_SUB_TYPE_REVERSED )
								{
									pPickup->KillPickup(mFiringIdx);
								}

								break;
							}*/
						}
					}
				}
			}
		}

		return false;
	}

	// --------------------------------------------------------------------------------

	void C_PickupBarge::CheckDynamicObjects()
	{
		Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		Gravity::C_Scene		*pscene = pRLC->GetGameWorld()->GetPhysicsScene();
		Actor::C_ActorHandle	handle;
		const Actor::C_Actor	*pActor;

		Collision::C_Sphere sphere(mMatrix.AW(), mpBargeData->mRadius);	
		SharedPhysics::MemoryAllocator<Gravity::C_Body*> allocator;
		U8 stackData[1024 * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> stack(stackData, 1024);
		S32 bodyCount = pscene->GetIntersectingBodies(sphere, &Game::PhysicsConstants::kPerkCollisionFilter, allocator, stack);
		SharedPhysics::C_MemoryIterator<Gravity::C_Body*> iter(&allocator);
		U32 count = 0;

		for(S32 body = 0; body < bodyCount; body++)
		{
			Gravity::C_Body** pDoublePtr = iter.Get();
			Gravity::C_Body* pBody = *pDoublePtr;
			Game::C_BodyUserDataBase* pUserData = (Game::C_BodyUserDataBase*)pBody->GetUserData<void>();
			if (pUserData && pUserData->m_eType == Game::E_BodyUserDataType_Actor)
			{
				handle = ((Game::C_BodyUserDataActor*)pUserData)->m_hActorHandle;
				pActor = pRLC->GetGameWorld()->GetActorList()->GetActorPtr(handle);

				if (pActor->GetActorTypeId() == E_ActorTypeId_DynamicObject || pActor->GetActorTypeId() == E_ActorTypeId_PhysicalObject)
				{
					Btl::Set<Actor::C_ActorHandle>::InsertionReturnType insResult = mDynamicsHit.Insert(handle);
					if(!insResult.inserted)
					{
						RLPickupForceEffects* forces = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetDatabase()->mRLSystems[0]->mPickupForceEffects;
						Maths::HVec3 dir = Maths::Normalise(pActor->GetMatrix().AW() - mMatrix.AW());

						ActorMsg::C_DynamicObjectApplyImpulseMsg msg;
						msg.m_Force = dir * forces->m_Barge.m_MaxForce;
						msg.m_ElectricalEffect = forces->m_Barge.m_ElectricalEffect;
						msg.m_ElectricalEffectColour = forces->m_Barge.m_ElectricalEffectColour;
						msg.m_DamageScale = forces->m_Barge.m_DamageScale;

						pActor->GetActorList()->SendMessageToActor(pActor->GetHandle(), &msg);

						if (forces->m_Barge.m_ElectricalEffect)
							pActor->GetActorList()->DelayMessageToActorBySeconds(1.0f + ::Actors::C_ActorUtils::Instance().GetRandomNum(), pActor->GetHandle(), 
							ActorMsg::E_DynamicObjMsg_DynamicObjectDisableElectricalEffect);	// End electrical effect message

						if((++count) >= g_dbMaxDamageObjectsBarge)
							return;
					}
				}
			}
		}
	}

	// --------------------------------------------------------------------------------

	E_TARGETTING_DIRECTION C_PickupBarge::GetHitDirection(Actors::C_VehicleLogicActor* pActor)
	{
		E_TARGETTING_DIRECTION hitDir = E_TARGETTING_DIRECTION_NONE;

		if(pActor)
		{
			Maths::HVec3 dir = Maths::Normalise(mMatrix.AW() - pActor->GetWorldMatrix().AW());
			float dotProdX = Maths::DotProduct(dir, pActor->GetWorldMatrix().AX());
			if( dotProdX < 0 )
			{
				hitDir = E_TARGETTING_DIRECTION_LEFT;
			}
			else
			{
				hitDir = E_TARGETTING_DIRECTION_RIGHT;
			}
		}

		return hitDir;
	}

	// --------------------------------------------------------------------------------

	bool C_PickupBarge::IsInsideEllipse(Maths::HVec3 targetPos, float distAddition)
	{
		float majorRadius = mMajorRadius + distAddition;
		float minorRadius = mMinorRadius + distAddition;

		float majorDistSq = majorRadius * majorRadius;
		float minorDistSq = minorRadius * minorRadius;

		float eccentricity = Maths::Sqrt((majorDistSq - minorDistSq)/majorDistSq);

		Maths::HVec3 foci1 = mMatrix.AW() + mMatrix.AZ() * eccentricity * majorRadius;
		Maths::HVec3 foci2 = mMatrix.AW() - mMatrix.AZ() * eccentricity * majorRadius;

		float dist1 = Maths::Distance(foci1, targetPos);
		float dist2 = Maths::Distance(foci2, targetPos);
		float dist  = dist1 + dist2;

		if(dist <= 2 * majorRadius)
			return true;

		return false;
	}

	// --------------------------------------------------------------------------------

	void C_PickupBarge::ReportProjectileDestroyedEvent(C_PickupBase* pPickup)
	{
		// If mine was destroyed, we should let interested parties know about this cool event so they can get some credit. 
		if(Game::VehicleIsLocal(mFiringIdx))
		{
			Game::C_RaceLogicLayer* pRaceLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			BIZ_ASSERT_MSG(pRaceLogicLayer,"C_PickupBarge::ReportProjectileDestroyedEvent(C_PickupBase* pPickup) < pRaceLogicLayer is NULL..");

			C_VehicleLogicMsg_DestroyedPickupWithPickup projectileDestroyedMsg( E_PICKUP_TYPE_BARGE, GetSubType(), mFiringIdx, mUniqueId,
																				pPickup->GetType(),pPickup->GetSubType(), pPickup->GetFiringIdx(), pPickup->GetUid());

			Actors::C_VehicleLogicActor* pActor	= pRaceLogicLayer->GetLogicVehicleActor(mFiringIdx);
			if(pActor->QueryInterface(Components::E_IF_VehicleComponent_Challenges))
			{
				//Tell the multiplayer challenges component 
				pActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Challenges, &projectileDestroyedMsg);
			}
			Game::C_StickerManager::Instance().MessageHandler(&projectileDestroyedMsg);
		}
	}

	// --------------------------------------------------------------------------------

	void C_PickupBarge::DebugDraw()
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(mMatrix.AW(),Maths::HVec3(0,1,0),mMajorRadius,Maths::RGBA(0,0,255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(mMatrix.AW(),Maths::HVec3(0,1,0),mMajorRadius - g_debugBargeOuterRadius,Maths::RGBA(0,0,255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(mMatrix.AW(),Maths::HVec3(0,1,0),mMajorRadius + g_debugBargeIdiotLeniency,Maths::RGBA(0,0,255));

		Maths::HVec3 point1 = mMatrix.AW() + mMatrix.AX() * mMinorRadius;
		Maths::HVec3 point2 = mMatrix.AW() - mMatrix.AX() * mMinorRadius;
		Maths::HVec3 point3 = mMatrix.AW() + mMatrix.AZ() * mMajorRadius;
		Maths::HVec3 point4 = mMatrix.AW() - mMatrix.AZ() * mMajorRadius;

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(point1,Maths::HVec3(0,1,0),0.25f,Maths::RGBA(0,0,255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(point2,Maths::HVec3(0,1,0),0.25f,Maths::RGBA(0,0,255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(point3 ,Maths::HVec3(0,1,0),0.25f,Maths::RGBA(0,0,255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(point4,Maths::HVec3(0,1,0),0.25f,Maths::RGBA(0,0,255));
	}

	
	// --------------------------------------------------------------------------------

	void C_PickupBarge::PostDelayInitialise( Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx )
	{
		matrix.AX( Maths::HVec3( matrix.AX() * mSizeMultiplier ) );
		matrix.AY( Maths::HVec3( matrix.AY() * mSizeMultiplier ) );
		matrix.AZ( Maths::HVec3( matrix.AZ() * mSizeMultiplier ) );
		C_PickupBase::SetPostDelayMatrix(matrix);

		if( HasAbility(UI::E_MultiplayerVehicleAbilities_Iron_Fist, mFiringIdx) && AbilityActiveOnPickupType(UI::E_MultiplayerVehicleAbilities_Iron_Fist) )
		{
			RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BARGE);
			RLBargeSpecialFX* pBargeFX			= (RLBargeSpecialFX*)pPickup->mNormal->mSpecialFX;

			if(pBargeFX)
			{
				C_PickupBase::PostDelayInitialise(matrix, &pBargeFX->mIronFistFiredSpecialFX);
			}
		}
		else if( HasAbility(UI::E_MultiplayerVehicleAbilities_Splash_Damage, mFiringIdx) )
		{
			RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BARGE);
			RLBargeSpecialFX* pBargeFX			= (RLBargeSpecialFX*)pPickup->mNormal->mSpecialFX;

			if(pBargeFX)
			{
				C_PickupBase::PostDelayInitialise(matrix, &pBargeFX->mSplashDamageFiredSpecialFX);
			}
		}
		else if(mPickupSubType == E_PICKUP_SUB_TYPE_MOD)
		{
			RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BARGE);
			RLBargeSpecialFX* pBargeFX			= (RLBargeSpecialFX*)pPickup->mNormal->mSpecialFX;

			if(pBargeFX)
			{
				C_PickupBase::PostDelayInitialise(matrix, &pBargeFX->mLastGaspFiredSpecialFX);
			}
		}
		else if(mFixedPosition)
		{
			RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BARGE);
			RLBargeSpecialFX* pBargeFX			= (RLBargeSpecialFX*)pPickup->mNormal->mSpecialFX;

			if(pBargeFX)
			{
				C_PickupBase::PostDelayInitialise(matrix, &pBargeFX->mFiredFixedPositionSpecialFX);
			}	
		}
		else
		{
			C_PickupBase::PostDelayInitialise(matrix, sfx);
		}
	}
}
