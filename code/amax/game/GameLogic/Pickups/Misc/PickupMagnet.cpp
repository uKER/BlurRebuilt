#include "stdafx.h"

#include <GameLogic/Pickups/Misc/PickupMagnet.h>

#include <GameLogic\Structures\LevelState.h>
#include <GameApp.h>
#include <Game/game.h>

#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

#include <GameLogic/Layers/RaceLogicLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/InGameDialogueManager/InGameDialogueManager.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>

#include <GameLogic/ActorStudioActors/ActorUtils.h>

#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups.h>
#include <GameLogic/Pickups/Normal/PickupFlash.h>

#include <gravity\narrowphase\Shapes.h>
#include <Game\GameDef\PhysicsConstants.h>

#include <Ui\UiMessages.h>
#include <Vehicle/VehicleModel.h>

Debug::Bool g_debugShowMagnetDebug("Perks.Magnet.ShowMagnetDebug",false,Debug::Persist());
Debug::Float g_debugMagnetFiringHeight("Perks.Magnet.FiringHeight", 0.5);
Debug::Float g_debugMagnetLiveTimeScale("Perks.Magnet.LiveTimeScale", 2.5);

Debug::Float g_debugMagnetRadius("Perks.Magnet.Radius",20.f,Debug::Persist());
Debug::Float g_debugMagnetShootStrength("Perks.Magnet.ShootStrength",5000.f,Debug::Persist());
Debug::Float g_debugMagnetShootMassScale("Perks.Magnet.ShootMassScale",4.f,Debug::Persist());

Debug::Float g_debugMagnetSingleTapMassThreshold("Perks.Magnet.SingleTapMassThreshold",200.f);
Debug::Float g_debugMagnetDoubleTapMassThreshold("Perks.Magnet.DoubleTapMassThreshold",8001.f);

Debug::Float g_debugMagnetMassThreshold("Perks.Magnet.MassThreshold",8.f,Debug::Persist());

Debug::Float g_debugMagnetDistanceOffset("Perks.Magnet.DistanceOffsetSpeedMultiplier",0.75f,Debug::Persist());

namespace Game
{
	C_PickupFireMagnet::C_PickupFireMagnet(U32 id, int level, int firer)
		: C_PickupFireBase(id, level, firer), mRadius(0), mTargettedVehicle(-1)
	{
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mFiringVehicle);

		float shortestDist = 0;
		int shortestIdx = -1;

		for(int a = 0; a < pLayer->GetNumVehicles(); a++)
		{
			//Maths::HVec3 dist=mRaceLogicLayerPtr->mVehiclePositions[mVehicleIndex].mPosition;
			if(mFiringVehicle == a)
				continue;

			float inRange = pActor->InRangeOfPickup(GetType(), 0, a);

			if(inRange > 1.0f)
				continue;

			if(inRange < shortestDist || shortestIdx == -1)
			{
				shortestDist = pActor->mVehicleProximities[a].mDistanceSquared;
				shortestIdx = a;
			}
		}

		if(shortestIdx != -1)
			mTargettedVehicle = shortestIdx;
		else
		{
			mFireDir = pActor->GetGraphicalActor(mFiringVehicle)->GetMatrix().AZ();
			mFireDir.Normalise();
		}
	}
	bool C_PickupFireMagnet::Update(bool p_IsMultiplayer)
	{
		Game::C_LevelState& levelState		= Game::C_Game::GetInstance()->GetLevelState();
		Actors::RaceLogicCore* pRLC			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		float frameTime						= C_GameApp::GetInstance()->GetTimeStep();
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mFiringVehicle);
		RLPickupType* pPickup				= (RLPickupType*)pRLC->GetPickupType(E_PICKUP_TYPE_MAGNET);

		if (!pPickup || pPickup->mLevels.count == 0 || !pActor || !pLayer || !pRLC)
			return true;

		RLMagnetBalancingData* pMagnetData=(RLMagnetBalancingData *)pPickup->mLevels[0]->mBalancingData;

		if (!pMagnetData->mForce.count || !pMagnetData->mUpwardForce.count)
			return true;

		if(mDelay)
		{
			mDelay -= frameTime;
			WarnPlayers();

			if(mDelay < 0.f)
			{
				mDelay	=0.f;
				mMatrix = pActor->GetGraphicalActor(mFiringVehicle)->GetMatrix();
				Maths::HVec3 up(0,1,0);
				Maths::HVec3 dir = mFireDir;

				if(mTargettedVehicle != -1)
				{
					Actors::C_VehicleLogicActor* pTargetVeh = pLayer->GetLogicVehicleActor(mTargettedVehicle);
					dir = ( pTargetVeh->mPositionInfo.mWorldPosition + up * g_debugMagnetFiringHeight ) - ( mMatrix.AW() + up * g_debugMagnetFiringHeight );
					dir.Normalise();
					
					mMatrix.AZ( dir );
				}

				CheckObstacles(dir);
				::Actors::C_ActorUtils::Instance().RegisterForceEffectWithActors(&mMatrix, ::Actors::E_FORCE_EFFECT_SHUNT_FIRE);
				SendFirePerkMessage();
			}

			UI::C_UiGameMsg_TargettingEvent uimsg(mFiringVehicle,Components::AMAX_TARGETTING_PHASE_CHARGING,mDelay,mTargettedVehicle,Components::AMAX_TARGETTING_OUTCOME_NONE);
			_UI_MESSAGE_FULL( uimsg );

			return false;
		}

		if(mLiveTime > 0)
		{
			mLiveTime -= frameTime * g_debugMagnetLiveTimeScale;
			return false;
		}

		//mRadius += ((frameTime * pMagnetData->mSpeed) * pMagnetData->mRadius);	//arbitary numbers until I get an effect to attach to

		//if(mRadius > mTotalRadius)
		//	return true;

		Maths::HVec3 dist = mMatrix.AW();

		for(int i = 0; i < pLayer->mLiveMines.Size(); i++)
		{
			C_PickupFireMineBase* pickup = pLayer->mLiveMines[i];

			if(pickup->mMatrix.AW().DistanceSquared(pActor->mPositionInfo.mWorldPosition) > pMagnetData->mRadius * pMagnetData->mRadius)
				break;

			Maths::HVec3 normalPos = Maths::Normalise( pActor->mPositionInfo.mWorldPosition - pLayer->mVehiclePositions[mFiringVehicle].mMatrix.AW() );

			float dotProd = Maths::DotProduct(pLayer->mVehiclePositions[mFiringVehicle].mMatrix.AZ(), normalPos);

			if( dotProd < 1 - pMagnetData->mAngleRange )
				break;

			pickup->mTimer = 0;
		}

		if(mTargettedVehicle == -1)
		{
			C_VehicleLogicMsg_MissedPerkMsg missedMsg(E_PICKUP_TYPE_MAGNET, mFiringVehicle, mTargettedVehicle, mMissMatrix, mUniqueId);
			Game::C_InGameSpecialFXManager::Instance().MessageHandler(&missedMsg);

			::Actors::C_ActorUtils::Instance().RegisterForceEffectWithActors(&mMissMatrix, ::Actors::E_FORCE_EFFECT_SHUNT_EXPLODE);

			return true;
		}

		//if(shortestIdx == -1)
		//{
		//	C_VehicleLogicMsg_MissedPerkMsg missedMsg(E_PICKUP_TYPE_MAGNET, mFiringVehicle, mTargettedVehicle, mMissMatrix, mUniqueId);
		//	Game::C_InGameSpecialFXManager::Instance().MessageHandler(&missedMsg);
		//}

		//if(shortestIdx == -1 && mTargettedVehicle != -1)
		//{
		//	//evaded
		//	UI::C_UiGameMsg_TargettingEvent uimsg(mFiringVehicle,Components::AMAX_TARGETTING_PHASE_FINISHED,1.0f,mTargettedVehicle,Components::AMAX_TARGETTING_OUTCOME_EVADED);
		//	_UI_MESSAGE_FULL( uimsg );

		//	C_VehicleLogicMsg_MissedPerkMsg missedMsg(E_PICKUP_TYPE_MAGNET, mFiringVehicle, mTargettedVehicle, mMissMatrix, mUniqueId);

		//	if( Game::C_InGameDialogueManager::Instance().IsEnabled() )
		//		Game::C_InGameDialogueManager::Instance().MessageHandler(&missedMsg);

		//	return true;
		//}
		//else if(shortestIdx != -1)
		//{
		//	if(mTargettedVehicle == shortestIdx)
		//	{
		//		AMAX_PICKUPS_CANT_HIT_REASON reason=AMAX_PICKUPS_CANT_HIT_REASON_NONE;
		//		if(p_IsMultiplayer)
		//		{
		//			if(!CanHitMp(mTargettedVehicle))
		//			{
		//				return false;
		//			}
		//		}
		//		else
		//		{
		//			if(!CanHit(mTargettedVehicle,&reason))
		//			{
		//				if(reason == AMAX_PICKUPS_CANT_HIT_REASON_SHIELDED)
		//				{
		//					UI::C_UiGameMsg_TargettingEvent uimsg(mFiringVehicle,Components::AMAX_TARGETTING_PHASE_FINISHED,1.0f,mTargettedVehicle,Components::AMAX_TARGETTING_OUTCOME_SHIELDED);
		//					_UI_MESSAGE_FULL( uimsg );
		//				}

		//				return true;
		//			}
		//		}
		//	}
		//	else
		//	{
		//		//evaded but hit someone else
		//		mTargettedVehicle=shortestIdx;
		//		UI::C_UiGameMsg_TargettingEvent uimsg(mFiringVehicle,Components::AMAX_TARGETTING_PHASE_FINISHED,1.0f,mTargettedVehicle,Components::AMAX_TARGETTING_OUTCOME_HIT_SOMEONE_ELSE);
		//		_UI_MESSAGE_FULL( uimsg );
		//	}
		//}
		//else
		//{
		//	//destoy updater
		//	return true;
		//}

		float power_multiplier=pMagnetData->mSingleTapMultiplier;
		if(mLevel == 1)
			power_multiplier=pMagnetData->mDoubleTapMultiplier;

		Maths::HVec3 hv= mMatrix.AZ();
		hv=Maths::Normalise(hv);

		hv   *= mRadius;
		dist += hv;

		if(p_IsMultiplayer)
		{
			if(!CanHitMp(mTargettedVehicle))
				return true;
		}
		else
		{
			if(!CanHit(mTargettedVehicle))
				return true;
		}

		mAlreadyHit[mTargettedVehicle] = true;

		UI::C_UiGameMsg_TargettingEvent uimsg(mFiringVehicle,Components::AMAX_TARGETTING_PHASE_FINISHED,1.0f,mTargettedVehicle,Components::AMAX_TARGETTING_OUTCOME_HIT);
		_UI_MESSAGE_FULL( uimsg );






		C_VehicleLogicMsg_HitByPerkMsg hit_by_msg(GetType(), mLevel, mTargettedVehicle, mFiringVehicle, mUniqueId, mDroppedByIndex, -1);
		pActor->GetActorList()->BroadcastMessage(&hit_by_msg);
		C_InGameSpecialFXManager::Instance().MessageHandler(&hit_by_msg);

		if( Game::C_InGameDialogueManager::Instance().IsEnabled() )
			Game::C_InGameDialogueManager::Instance().MessageHandler(&hit_by_msg);

		return true;
	}

	void C_PickupFireMagnet::WarnPlayers()
	{
		C_RaceLogicLayer* pLayer					= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor			= pLayer->GetLogicVehicleActor(mFiringVehicle);

		for(int a = 0; a < pLayer->GetNumVehicles(); a++)
		{
			Actors::C_VehicleLogicActor* pOtherActor = pLayer->GetLogicVehicleActor(a);
			pOtherActor->mTargetInDanger[mFiringVehicle] = (mTargettedVehicle == a);
		}
	}

	void C_PickupFireMagnet::CheckObstacles(Maths::HVec3 dir)
	{
		Game::Actors::RaceLogicCore* pCore	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mFiringVehicle);
		RLPickupType* pPickup				= (RLPickupType*)pCore->GetPickupType(E_PICKUP_TYPE_MAGNET);
		RLMagnetBalancingData* pMagnetData	= (RLMagnetBalancingData *)pPickup->mLevels[0]->mBalancingData;

		// ----------- RAY TRACE -----------
		U8 data[1024 * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> stack(data, 1024);
		Collision::C_Ray ray;
		//Collision::C_CollisionFilter rayCollisionFilter(Game::PolyFlags::kCameraSolid | Game::PolyFlags::kVehicleBlocker, Game::PolyFlags::kCameraSolid);
		Collision::C_CollisionFilter rayCollisionFilter(0xffffffff, Game::PolyFlags::kCameraSolid, pActor->GetGraphicalActor(mFiringVehicle)->GetPhysicalGroupID(), 0, 0);
		rayCollisionFilter.m_CollisionFlags &= ~(Game::PolyFlags::kDynamicObjectSolid);

		Maths::HVec3 up(0,1,0);
		Maths::HVec3 pos = mMatrix.AW() + up * g_debugMagnetFiringHeight;
		ray.InitialiseRay(pos, pos + dir * pMagnetData->mRadius, &rayCollisionFilter);
		pCore->GetPhysicsScene()->DoRayTrace(ray, stack);
		Collision::C_RayTracingResult& result = ray.m_result;

		if(result.Hit())
		{
			Maths::HVec3 toz = result.Normal();
			toz.Normalise();
			Maths::HVec3 tox = Maths::HVec3(0.f,1.f,0.f) ^ toz;
			tox.Normalise();
			Maths::HVec3 toy = toz ^ tox;
			toy.Normalise();

			Maths::HMat33 look;
			look.AX(tox);
			look.AY(toy);
			look.AZ(toz);
			mMissMatrix = Maths::HMat43(look, result.HitPoint());
			
			mLiveTime = result.Distance() / mTotalRadius;
		}
		// ---------------------------------
	}

	// ---------------------------------------------------------------------------------------------------------------------------------

	bool C_PickupHitMagnet::Update(int vehIdx, bool p_IsMultiplayer)
	{
		Game::C_LevelState& levelState		= Game::C_Game::GetInstance()->GetLevelState();
		Actors::RaceLogicCore* pRLC			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		float frameTime						= C_GameApp::GetInstance()->GetTimeStep();
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pVLActor = pLayer->GetLogicVehicleActor(vehIdx);
		RLPickupType* pPickup				= (RLPickupType*)pRLC->GetPickupType(E_PICKUP_TYPE_MAGNET);

		if (!pPickup || pPickup->mLevels.count == 0|| !pLayer || !pVLActor)
			return false;

		RLMagnetBalancingData* pMagnetData=(RLMagnetBalancingData *)pPickup->mLevels[0]->mBalancingData;

		mTimer += frameTime;

		if(mTimer > pMagnetData->mTimeMagneted)
		{
			return true;
		}

		Maths::HVec3 vehicle_pos=pLayer->mVehiclePositions[vehIdx].mPosition;
		Maths::HVec3 vehicle_dir=pLayer->mVehiclePositions[vehIdx].mMatrix.AZ();
		vehicle_dir=Maths::Normalise(vehicle_dir);

		float speedInMetresPerSecond = abs( Magnitude( pVLActor->GetGraphicalActor(vehIdx)->GetVehicleInput()->m_vVelocity ) );
		float mph= (speedInMetresPerSecond * 3600.f) / 1609.f;

		vehicle_pos += vehicle_dir * (g_debugMagnetDistanceOffset * mph);

		S32						bodyCount;
		Gravity::C_Scene		*pscene = pRLC->GetGameWorld()->GetPhysicsScene();
		S32						body;
		Actor::C_ActorHandle	handle;
		const Actor::C_Actor	*pActor;
		Maths::HVec3			actorpos;

		Collision::C_Sphere sphere(vehicle_pos, (float)g_debugMagnetRadius);			// Get list of physics bodies within our sphere
		SharedPhysics::MemoryAllocator<Gravity::C_Body*> allocator;
		U8 stackData[1024 * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> stack(stackData, 1024);
		bodyCount = pscene->GetIntersectingBodies(sphere, &Game::PhysicsConstants::kPerkCollisionFilter, allocator, stack);
		SharedPhysics::C_MemoryIterator<Gravity::C_Body*> iter(&allocator);
		for(body=0;body<bodyCount;body++)										// Loop through them all
		{
			Gravity::C_Body** pDoublePtr = iter.Get();
			Gravity::C_Body* pBody = *pDoublePtr;
			Game::C_BodyUserDataBase* pUserData = (Game::C_BodyUserDataBase*)pBody->GetUserData<void>();
			if (pUserData && pUserData->m_eType == Game::E_BodyUserDataType_Actor)
			{
				handle = ((Game::C_BodyUserDataActor*)pUserData)->m_hActorHandle;
				pActor = pRLC->GetGameWorld()->GetActorList()->GetActorPtr(handle);
				if (pActor)
				{
					float mass = pBody->GetBody().m_Mass;

					if(mass < g_debugMagnetMassThreshold)
						continue;

					if(mass < g_debugMagnetSingleTapMassThreshold || (mLevel == 1 && mass < g_debugMagnetDoubleTapMassThreshold))
					{
						if (pActor->GetActorTypeId() == Game::E_ActorTypeId_PhysicalObject)
						{
							::Actors::C_PhysicalObject *pPhysicalObject=((::Actors::C_PhysicalObject *)pActor);
							//pPhysicalObject->Enable(true);
						}

						actorpos = pActor->GetPosition();

						pBody->Activate();

						Maths::HVec3 magnet_vector = vehicle_pos - actorpos;
						magnet_vector = Maths::Normalise(magnet_vector);

						float force = g_debugMagnetShootStrength ;
						force += mass*g_debugMagnetShootMassScale;

						magnet_vector *= force;
						magnet_vector=magnet_vector * pActor->GetMatrix().Rotation().Inverse();

						Maths::HVec3 centreOfMassPosition = pBody->GetBody().m_CenterOfMassOffset;
						pBody->ApplyImpulseLocalSpace( magnet_vector, centreOfMassPosition);

					}
				}
			}
		}

		return false;
	}
}
