#include "stdafx.h"

#include <GameLogic\Pickups\Misc\PickupBombard.h>

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

Debug::Bool g_debugShowBombardDebug("Perks.Bombard.ShowDebug",false,Debug::Persist());

Debug::Float g_debugBombardRadius("Perks.Bombard.Radius",20.f,Debug::Persist());
Debug::Float g_debugBombardMassThreshold("Perks.Bombard.MassThreshold",8.f,Debug::Persist());
Debug::Float g_debugBombardStrength("Perks.Bombard.Strength",20.f,Debug::Persist());
Debug::Float g_debugBombardMassScale("Perks.Bombard.MassScale",4.f,Debug::Persist());

Debug::Float g_debugBombardHeight("Perks.Bombard.Height",3.f,Debug::Persist());

Debug::Float g_debugBombardUpForce("Perks.Bombard.UpForce",20.f,Debug::Persist());

Debug::Float g_debugBombardDamping("Perks.Bombard.Damping",0.5f,Debug::Persist());

Debug::Float g_debugBombardShootStrength("Perks.Bombard.ShootStrength",5000.f,Debug::Persist());
Debug::Float g_debugBombardShootMassScale("Perks.Bombard.ShootMassScale",4.f,Debug::Persist());

Debug::Float g_debugBombardShootRadius("Perks.Bombard.ShootRadius",4.f,Debug::Persist());

Debug::Float g_debugBombardShootDistance("Perks.Bombard.Shootistance",20.f,Debug::Persist());

Debug::Float g_debugBombardSingleTapMassThreshold("Perks.Bombard.SingleTapMassThreshold",200.f);
Debug::Float g_debugBombardDoubleTapMassThreshold("Perks.Bombard.DoubleTapMassThreshold",8001.f);


Debug::Float g_debugBombardPointingAngle("Perks.Bombard.PointingAngle",0.075f);

namespace Game
{
	bool C_PickupFireBombard::Update(bool p_IsMultiplayer)
	{
		Game::C_LevelState& levelState		= Game::C_Game::GetInstance()->GetLevelState();
		RLPickupType* pPickup				= (RLPickupType*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetPickupType(E_PICKUP_TYPE_BOMBARD);
		float frameTime						= C_GameApp::GetInstance()->GetTimeStep();
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pVLActor	= pLayer->GetLogicVehicleActor(mFiringVehicle);
		Actors::C_VehicleActor* pVehActor	= pVLActor->GetGraphicalActor(mFiringVehicle);

		if (!pPickup || pPickup->mLevels.count == 0 )
			return true;

		RLBombardBalancingData* pBombardData = (RLBombardBalancingData *)pPickup->mLevels[0]->mBalancingData;

		if(mDelay)
		{
			mDelay -= frameTime;

			WarnPlayers();

			mMatrix = pLayer->mVehiclePositions[mFiringVehicle].mMatrix;
			mHeight = pLayer->mVehiclePositions[mFiringVehicle].mPosition.Y() + g_debugBombardHeight;

			Maths::HVec3 heading=mMatrix.AZ();
			heading.Normalise();
			
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(mMatrix.AW() + (heading*g_debugBombardShootDistance),3.0f,Maths::RGBA(255,0,0));

			if(mDelay < 0.f)
			{
				mMatrix = pLayer->mVehiclePositions[mFiringVehicle].mMatrix;
				mHeight = pLayer->mVehiclePositions[mFiringVehicle].mPosition.Y() + g_debugBombardHeight;

				mAngleTimer = 0.f;

				Maths::HVec3 heading=mMatrix.AZ();
				heading.Normalise();
				mMatrix.AW( mMatrix.AW() + (heading*g_debugBombardShootDistance));

				mDelay	= 0.f;
				SendFirePerkMessage();
			}

			return false;
		}

		mTimer -= frameTime;

		if(mTimer < 0.f)
		{
			C_VehicleLogicMsg_DestroyPerkMsg msg(E_PICKUP_TYPE_BOMBARD, 0, mMatrix, 2.0f, mUniqueId, mFiringVehicle);
			Game::C_InGameSpecialFXManager::Instance().MessageHandler(&msg);

			Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
			S32						bodyCount;
			Gravity::C_Scene		*pscene = pRLC->GetGameWorld()->GetPhysicsScene();
			S32						body;
			Actor::C_ActorHandle	handle;
			const Actor::C_Actor	*pActor;
			Maths::HVec3			actorpos;

			Collision::C_Sphere sphere(mMatrix.AW(), (float)g_debugBombardRadius);			// Get list of physics bodies within our sphere
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

						if(mass > g_debugBombardMassThreshold)
						{
							pBody->SetAngularDampingCoefficient(0.99f);
							pBody->SetLinearDampingCoefficient(0.99f);
						}
					}
				}
			}

			return true;
		}

		for(U32 a = 0; a<levelState.mNumCars; a++)
		{
			if(a == mFiringVehicle)
				continue;
		
			Maths::HVec3 pos = mMatrix.AW();

			Maths::HVec3 vehicle_pos=pLayer->mVehiclePositions[a].mPosition;

			if(pos.Distance(vehicle_pos) < g_debugBombardShootRadius)
			{
				Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
				S32						bodyCount;
				Gravity::C_Scene		*pscene = pRLC->GetGameWorld()->GetPhysicsScene();
				S32						body;
				Actor::C_ActorHandle	handle;
				const Actor::C_Actor	*pActor;
				Maths::HVec3			actorpos;

				Collision::C_Sphere sphere(pos, (float)g_debugBombardRadius);			// Get list of physics bodies within our sphere
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

							if(mass < g_debugBombardMassThreshold)
								continue;

							if(mass < g_debugBombardSingleTapMassThreshold || (mLevel == 1 && mass < g_debugBombardDoubleTapMassThreshold))
							{
								if (pActor->GetActorTypeId() == Game::E_ActorTypeId_PhysicalObject)
								{
									::Actors::C_PhysicalObject *pPhysicalObject=((::Actors::C_PhysicalObject *)pActor);
									//pPhysicalObject->Enable(true);
								}

								actorpos = pActor->GetPosition();

								pBody->Activate();
								pBody->SetAngularDampingCoefficient(0.99f);
								pBody->SetLinearDampingCoefficient(0.99f);

								Maths::HVec3 bombard_vector = vehicle_pos - actorpos;
								bombard_vector = Maths::Normalise(bombard_vector);

								float force = g_debugBombardShootStrength ;
								force += mass*g_debugBombardShootMassScale;

								bombard_vector *= force;
								bombard_vector=bombard_vector * pActor->GetMatrix().Rotation().Inverse();

								Maths::HVec3 centreOfMassPosition = pBody->GetBody().m_CenterOfMassOffset;
								pBody->ApplyImpulseLocalSpace( bombard_vector, centreOfMassPosition);

							}
						}
					}
				}

				return true;
			}
		}	

		Maths::HVec3 pos = mMatrix.AW();

		if(g_debugShowBombardDebug)
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(mMatrix.AW(),Maths::HVec3(0,1,0),g_debugBombardRadius,Maths::RGBA(255,0,0));

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(mMatrix.AW(),Maths::HVec3(0,1,0),g_debugBombardShootRadius,Maths::RGBA(255,255,0));
			
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(mMatrix.AW()+Maths::HVec3(0,0.5f,0),Util::C_String().Printf("%0.1f",mTimer));
		}

		Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		S32						bodyCount;
		Gravity::C_Scene		*pscene = pRLC->GetGameWorld()->GetPhysicsScene();
		S32						body;
		Actor::C_ActorHandle	handle;
		const Actor::C_Actor	*pActor;
		Maths::HVec3			actorpos;

		Collision::C_Sphere sphere(pos, (float)g_debugBombardRadius);			// Get list of physics bodies within our sphere
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

					if(mass < g_debugBombardMassThreshold)
						continue;

					if(mass < g_debugBombardSingleTapMassThreshold || (mLevel == 1 && mass < g_debugBombardDoubleTapMassThreshold))
					{
						actorpos = pActor->GetPosition();

						if(g_debugShowBombardDebug)
						{
							DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar((actorpos * Maths::HVec3(1,0,1)) + Maths::HVec3(0,mHeight,0),1.f,Maths::RGBA(255,0,0));
						}

						if (pActor->GetActorTypeId() == Game::E_ActorTypeId_PhysicalObject)
						{
							::Actors::C_PhysicalObject *pPhysicalObject=((::Actors::C_PhysicalObject *)pActor);
							//pPhysicalObject->Enable(true);
						}

						pBody->Activate();
						pBody->SetAngularDampingCoefficient((float)g_debugBombardDamping);
						pBody->SetLinearDampingCoefficient((float)g_debugBombardDamping);

						//mExternalConstants.m_Gravity=Maths::HVec3(0,0,0);
						//pBody->SetExternalConstants(&mExternalConstants);

						float force=mHeight - actorpos.Y();
						if(force < 0.1f)
						{
							actorpos.Y(mHeight);
							
							Maths::HMat43 elem=pBody->ElementMatrix();

							elem.AW(actorpos);

							pBody->ElementMatrix(elem);






							//float mass = pBody->GetBody().m_Mass;

							//pBody->Activate();
							//pBody->SetAngularDampingCoefficient(0.99f);
							//pBody->SetLinearDampingCoefficient(0.99f);

							//Maths::HVec3 bombard_vector = mMatrix.AW() - actorpos;
							//bombard_vector = Maths::Normalise(bombard_vector);

							//float force = g_debugBombardShootStrength;
							//force += mass*g_debugBombardShootMassScale;

							//bombard_vector *= force;
							//bombard_vector=bombard_vector * pActor->GetMatrix().Rotation().Inverse();

							//Maths::HVec3 centreOfMassPosition = pBody->GetBody().m_CenterOfMassOffset;
							//pBody->ApplyImpulseLocalSpace( bombard_vector, centreOfMassPosition);










							//actorpos.Y(mHeight);
							//
							//Maths::HMat43 elem=pBody->ElementMatrix();

							////hold the pos
							//elem.AW(actorpos);

							////angle towards the player ... lamp-posts are now a threat

							//Maths::HVec3 up=Maths::HVec3(0,1,0);
							//Maths::HVec3 angle=mMatrix.AW() - actorpos;
							//angle.Normalise();

							//Maths::HVec3 cross_prod=up^angle;

							//mAngleTimer -= (frameTime * g_debugBombardPointingAngle);
							//if(mAngleTimer < -1.85f)
							//{
							//	mAngleTimer=-1.85f;
							//}

							//Maths::C_Quaternion q(cross_prod, mAngleTimer);

							//Maths::HMat33 rot_mat(q);

							////mAngleTimer += (frameTime * g_debugBombardPointingAngle);
							////if(mAngleTimer > Maths::PiBy2)
							////{
							////	if(mAngleTimer > Maths::Pi2)
							////	{
							////		mAngleTimer-=Maths::Pi2;
							////	}
							////}

							////Maths::HMat33 rot_mat(Maths::HMat33(Maths::RotX(mAngleTimer)) * Maths::HMat33(Maths::RotY(-mAngleTimer)) * Maths::HMat33(Maths::RotZ(mAngleTimer)));
							//
							//elem.Rotation(rot_mat);
			
							//pBody->ElementMatrix(elem);
						}
						else
						{
							if(actorpos.Y() > mHeight)
							{
								force*=g_debugBombardStrength;
								force -= mass*g_debugBombardMassScale;

								pBody->ApplyForce(Maths::HVec3(0,force,0), pBody->GetBody().m_Position);

								//Maths::HVec3 centreOfMassPosition = pBody->GetBody().m_CenterOfMassOffset;
								//pBody->ApplyImpulseLocalSpace( Maths::HVec3(0,force,0), centreOfMassPosition);
							}
							else
							{
								force*=g_debugBombardUpForce;
								force += mass*g_debugBombardMassScale;

								pBody->ApplyForce(Maths::HVec3(0,force,0), pBody->GetBody().m_Position);
							}
						}
					}
				}
			}
		}
		
		return false;
	}

	void C_PickupFireBombard::WarnPlayers()
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
	


	bool C_PickupHitBombard::Update(int vehIdx, bool p_IsMultiplayer)
	{
		C_RaceLogicLayer* pLayer					= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pFiringActor	= pLayer->GetLogicVehicleActor(mFiringVehicle);
		Actors::C_VehicleLogicActor* pHitActor		= pLayer->GetLogicVehicleActor(vehIdx);


		return true;
	}
}
