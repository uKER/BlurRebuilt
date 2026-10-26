#include "stdafx.h"

#include <GameLogic\Pickups\Misc\PickupBlackHole.h>

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
#include <Game/Messaging/PickupActorMessages.h>

#include <RaceBook\RaceBookManager.h>

#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups.h>

#include <Ui\UiMessages.h>

#include <actor_system/ActorList.h>

#include <gravity/sdk/BodyHelper.h>
#include <gravity/importer/Importer.h>
#include <gravity/narrowphase/Ray.h>

#include <Game/GameWorld/GameWorld.h>
#include <Game/GameDef/PhysicsConstants.h>
#include <gravity/sdk/BodyHelper.h>
#include <shared_actors/Components/Impl/PhysicsBodyComponent.h>
#include <Game/GameDef/Polyflags.h>

#include <gamelogic\actorstudioactors\PhysicalObjectActor.h>
#include <shared_actors\ActorDamage.h>

#include <GameAudio/GameAudioLayer.h>
#include <Game/GameActors/Components/Messages/AudioMessages.h>



Debug::Bool g_debugShowBlackHoleDebug("Perks.BlackHole.ShowDebug",false,Debug::Persist());

Debug::Float g_debugBlackHoleRadius("Perks.BlackHole.Radius",20.f);
Debug::Float g_debugBlackHoleStrength("Perks.BlackHole.Strength",200.f);
Debug::Float g_debugBlackHoleExplosionStrength("Perks.BlackHole.ExplosionStrength",10.f);
Debug::Float g_debugBlackHoleKillRadius("Perks.BlackHole.KillRadius",0.2f);

Debug::Float g_debugBlackHoleMassScale("Perks.BlackHole.MassScale",15.f);

Debug::Float g_debugBlackHoleLowerMassThreshold("Perks.BlackHole.SingleTapMassThreshold",8.f);
Debug::Float g_debugBlackHoleSingleTapMassThreshold("Perks.BlackHole.SingleTapMassThreshold",200.f);
Debug::Float g_debugBlackHoleDoubleTapMassThreshold("Perks.BlackHole.DoubleTapMassThreshold",8001.f);

Debug::Float g_debugBlackHoleDamageScale("Perks.BlackHole.DamageScale",0.f);

namespace Game
{
	bool C_PickupFireBlackHole::Update(bool p_IsMultiplayer)
	{
		Game::C_LevelState& levelState		= Game::C_Game::GetInstance()->GetLevelState();
		RLPickupType* pPickup				= (RLPickupType*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetPickupType(E_PICKUP_TYPE_BLACK_HOLE);
		float frameTime						= C_GameApp::GetInstance()->GetTimeStep();
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pVLActor	= pLayer->GetLogicVehicleActor(mFiringVehicle);
		Actors::C_VehicleActor* pVehActor	= pVLActor->GetGraphicalActor(mFiringVehicle);

		if (!pPickup || pPickup->mLevels.count == 0)
			return true;

		RLBlackHoleBalancingData* pBlackHoleData = (RLBlackHoleBalancingData *)pPickup->mLevels[0]->mBalancingData;

		if(mDelay)
		{
			mDelay -= frameTime;
			WarnPlayers();

			if(mDelay < 0.f)
			{
				mDelay=0.f;
				mMatrix = pLayer->mVehiclePositions[mFiringVehicle].mMatrix;

				float life=10.f;

				pLayer->mLiveMines.PushBack((C_PickupFireMineBase*)this);
				
				C_VehicleLogicMsg_DroppedPerkMsg msg(E_PICKUP_TYPE_BLACK_HOLE, mFiringVehicle, mLevel, mMatrix, life, mUniqueId);
				C_InGameSpecialFXManager::Instance().MessageHandler(&msg);
				mIsMultiplayer = p_IsMultiplayer;

				//Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eDefaultSubsystem, 0, 0, &msg);

				//CreateCollisionBody(false);

				SendFirePerkMessage();
			}

			return false;
		}

		//UpdateCollisionBody();

		Maths::HVec3 pos = mMatrix.AW();

		mTimer -= frameTime;

		if(mTimer < 0.f)
		{
			C_VehicleLogicMsg_DestroyPerkMsg msg(E_PICKUP_TYPE_BLACK_HOLE, 0, mMatrix, 2.0f, mUniqueId, mFiringVehicle);
			Game::C_InGameSpecialFXManager::Instance().MessageHandler(&msg);

			Explode();

			C_VehicleLogicMsg_MissedPerkMsg missedMsg(E_PICKUP_TYPE_BLACK_HOLE, mFiringVehicle, -1, mMatrix, mUniqueId);
			Game::C_InGameSpecialFXManager::Instance().MessageHandler(&missedMsg);

			return true;
		}	

		for(U32 a = 0; a<levelState.mNumCars; a++)
		{
			if(a == mFiringVehicle)
				continue;

			Maths::HVec3 vehicle_pos=pLayer->mVehiclePositions[a].mPosition;

			if(pos.Distance(vehicle_pos) < g_debugBlackHoleRadius)
			{
				Maths::HVec3 blackhole_vector = mMatrix.AW() - pLayer->mVehiclePositions[a].mPosition;
				
				float length=blackhole_vector.Length();
				
				blackhole_vector = Maths::Normalise(blackhole_vector);

				float force = g_debugBlackHoleStrength * (length/g_debugBlackHoleRadius);

				blackhole_vector *= force;
				blackhole_vector=blackhole_vector * pLayer->mVehiclePositions[a].mMatrix.Rotation().Inverse();

				Gravity::C_Body* pBody = pVLActor->GetGraphicalActor(a)->GetCollisionBody();
				Maths::HVec3 centreOfMassPosition = pBody->GetBody().m_CenterOfMassOffset;

				pBody->ApplyImpulseLocalSpace( blackhole_vector, centreOfMassPosition);
			}
		}

		Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		S32						bodyCount;
		Gravity::C_Scene		*pscene = pRLC->GetGameWorld()->GetPhysicsScene();
		S32						body;
		Actor::C_ActorHandle	handle;
		const Actor::C_Actor	*pActor;
		Maths::HVec3			actorpos;

		Collision::C_Sphere sphere(pos, (float)g_debugBlackHoleRadius);			// Get list of physics bodies within our sphere
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

					if(mass < g_debugBlackHoleLowerMassThreshold)
						continue;

					if(mass < g_debugBlackHoleSingleTapMassThreshold || (mLevel == 1 && mass < g_debugBlackHoleDoubleTapMassThreshold))
					{
						actorpos = pActor->GetPosition();

						if (pActor->GetActorTypeId() == Game::E_ActorTypeId_PhysicalObject)
						{
							::Actors::C_PhysicalObject *pPhysicalObject=((::Actors::C_PhysicalObject *)pActor);
							
							if(pPhysicalObject->GetLifetimeState() != ActorSys::E_LifetimeState_Killed)
							{
								//pPhysicalObject->Enable(true);

								//ActorDamage::C_DamageData dmgData;
								//dmgData.m_eDamageType = ActorDamage::E_Damage_Electric;
								//dmgData.m_DamageAmount = g_debugBlackHoleDamageScale;
								//pPhysicalObject->DamageAllElements(&dmgData);
							}
						}

						Maths::HVec3 blackhole_vector = mMatrix.AW() - actorpos;

						float length=blackhole_vector.Length();

						blackhole_vector = Maths::Normalise(blackhole_vector);

						float force = g_debugBlackHoleStrength * (length/g_debugBlackHoleRadius);
						force += mass*g_debugBlackHoleMassScale;

						blackhole_vector *= force;

						pBody->Activate();
						pBody->ApplyForce(blackhole_vector, pBody->GetBody().m_Position);
					}

					//blackhole_vector=blackhole_vector * pActor->GetMatrix().Rotation().Inverse();
					//pBody->ApplyImpulseLocalSpace(blackhole_vector, pBody->GetBody().m_CenterOfMassOffset);
				}
			}
		}

		
		if(g_debugShowBlackHoleDebug)
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(mMatrix.AW(),1.f,Maths::RGBA(255,0,0));
			
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(mMatrix.AW(),Maths::HVec3(0,1,0),g_debugBlackHoleRadius,Maths::RGBA(255,0,0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(mMatrix.AW(),Maths::HVec3(0,1,0),g_debugBlackHoleRadius * g_debugBlackHoleKillRadius,Maths::RGBA(255,255,255));
			
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(mMatrix.AW()+Maths::HVec3(0,0.5f,0),Util::C_String().Printf("%0.1f",mTimer));
		}

		return false;
	}
	
	void C_PickupFireBlackHole::Trigger(int idx, int mine_idx)
	{

	}

	void C_PickupFireBlackHole::WarnPlayers()
	{
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mFiringVehicle);

		for(int a = 0; a < pLayer->GetNumVehicles(); a++)
		{
			if(mFiringVehicle == a)
				continue;

			Actors::C_VehicleLogicActor* pOtherActor = pLayer->GetLogicVehicleActor(a);

			float inRange = pActor->InRangeOfPickup(GetType(), 0, a);
			if(inRange <= 1.0f)
				pOtherActor->mTargetInDanger[mFiringVehicle] = true;
			else
				pOtherActor->mTargetInDanger[mFiringVehicle] = false;
		}
	}

	void C_PickupFireBlackHole::Explode()
	{
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

		pLayer->RemoveMine((C_PickupFireMineBase*)this);

		Maths::HVec3 pos = mMatrix.AW();

		Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		S32						bodyCount;
		Gravity::C_Scene		*pscene = pRLC->GetGameWorld()->GetPhysicsScene();
		S32						body;
		Actor::C_ActorHandle	handle;
		const Actor::C_Actor	*pActor;
		Maths::HVec3			actorpos;

		Collision::C_Sphere sphere(pos, (float)g_debugBlackHoleRadius * g_debugBlackHoleKillRadius);			// Get list of physics bodies within our sphere
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
					actorpos = pActor->GetPosition();

					Maths::HVec3 blackhole_vector = actorpos - mMatrix.AW();
					blackhole_vector = Maths::Normalise(blackhole_vector);

					float force = g_debugBlackHoleStrength * g_debugBlackHoleExplosionStrength;

					blackhole_vector *= force;
					blackhole_vector=blackhole_vector * pActor->GetMatrix().Rotation().Inverse();
				
					//pBody->ApplyImpulseLocalSpace(blackhole_vector, pBody->GetBody().m_CenterOfMassOffset);

					pRLC->GetGameWorld()->GetActorList()->SendMessageToActor(handle,Actor::E_ActorMsg_Kill);
				}
			}
		}

		C_VehicleLogicMsg_DestroyPerkMsg msg(E_PICKUP_TYPE_BLACK_HOLE, 0, mMatrix, 2.0f, mUniqueId, mFiringVehicle);

		Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, ActorMsg::E_AudioMsg_Pickups_Destroy, 0, &msg);
	}
}
