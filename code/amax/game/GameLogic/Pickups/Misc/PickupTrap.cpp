#include "stdafx.h"

#include <GameLogic\Pickups\Misc\PickupTrap.h>

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
#include <Game/GameDef/Polyflags.h>

#include <RaceBook\RaceBookManager.h>

#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups.h>

#include <Ui\UiMessages.h>

#include "System/misc/Random.h"

Debug::Float g_debugLaserBoxWidth("Perks.Trap.Width",20.f,Debug::Persist());

Debug::Int g_debugLaserBoxRange("Perks.Trap.Range",80,Debug::Persist());
Debug::Int g_debugLaserBoxStep("Perks.Trap.Step",4,Debug::Persist());

Debug::Float g_debugTrapDisablePerkShieldTimer("Perks.Trap.DisablePerkShieldTimer",4.f);

Debug::Int g_debugLaserBoxBlastForce("Perks.Trap.BlastForce",200,Debug::Persist());

Util::C_Random gPickupTrapRandomNumber;

namespace Game
{
	const bool C_PickupFireTrap::TrapTriggered(Maths::HVec3 currentPosition, Maths::HVec3 lastPosition, Maths::HVec3 lineStart, Maths::HVec3 lineEnd)
	{
		Maths::HVec3 normal = Maths::HVec3(lineStart.Z() - lineEnd.Z(), 0.f, -lineStart.X() + lineEnd.X());

		float a1 = normal | (lineStart - currentPosition);
		float a2 = normal | (lineStart - lastPosition);

		if(a1 * a2 >= 0.f || a1 == 0.f)
			return false;

		Maths::HVec3 direction = (lineEnd - currentPosition);

		//Reject by line normal
		normal = Maths::HVec3((currentPosition.Z() - lastPosition.Z()), 0.f, -currentPosition.X() + lastPosition.X()); 

		float a21 = normal | (lineStart - currentPosition);
		float a22 = normal | direction;

		if(a21 * a22 >= 0.f || a21 == 0.f)
			return false;

		return true;
	}

	bool C_PickupFireTrap::Update(bool p_IsMultiplayer)
	{
		Game::C_LevelState& levelState		= Game::C_Game::GetInstance()->GetLevelState();
		RLPickupType* pPickup				= (RLPickupType*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetPickupType(E_PICKUP_TYPE_TRAP);
		float frameTime						= C_GameApp::GetInstance()->GetTimeStep();
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor	= pLayer->GetLogicVehicleActor(mFiringVehicle);
		Actors::C_VehicleActor* pVehActor	= pActor->GetGraphicalActor(mFiringVehicle);
		Actors::RaceLogicCore* pRLC			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

		if (!pPickup || pPickup->mLevels.count == 0)
			return true;

		RLTrapBalancingData* pTrapData = (RLTrapBalancingData *)pPickup->mLevels[0]->mBalancingData;
	
		if(mDelay)
		{
			mDelay -= frameTime;
			WarnPlayers();

			if(mDelay < 0.f)
			{
				mDelay	=0.f;
				SendFirePerkMessage();
			}

			return false;
		}

		if(!mCreateGrid)
		{
			mCreateGrid=true;

			mGridActive=false;

			U8 data[1024 * sizeof(void*)];
			SharedPhysics::C_SafeStack<true> stack(data, 1024);
			Collision::C_Ray ray;
			Collision::C_CollisionFilter rayCollisionFilter(0xffffffff, Game::PolyFlags::kCameraSolid, pActor->GetGraphicalActor(mFiringVehicle)->GetPhysicalGroupID(), 0, 0);
			rayCollisionFilter.m_CollisionFlags &= ~(Game::PolyFlags::kDynamicObjectSolid);

			Maths::C_Normal up(0,1,0);

			pActor->ClearDebugPoints();

			for(int a=0;a<g_debugLaserBoxRange;a+=g_debugLaserBoxStep)
			{
				Maths::HVec3 dir=pActor->GetGraphicalActor(mFiringVehicle)->GetMatrix().AZ();
				dir.Normalise();

				Maths::HVec3 raisedWorldPos = pActor->GetGraphicalActor(mFiringVehicle)->GetMatrix().AW() + (dir*a) + Maths::HVec3(0,1,0);

				pActor->AddDebugPoint(raisedWorldPos,Maths::RGBA(255,255,0));

				Maths::HVec3 side_dir = pActor->GetGraphicalActor(mFiringVehicle)->GetMatrix().AX();
				side_dir.Normalise();

				ray.InitialiseRay(raisedWorldPos, raisedWorldPos + (side_dir * (g_debugLaserBoxWidth)), &rayCollisionFilter);
				pRLC->GetPhysicsScene()->DoRayTrace(ray, stack);
				Collision::C_RayTracingResult resultleft = ray.m_result;

				side_dir = -pActor->GetGraphicalActor(mFiringVehicle)->GetMatrix().AX();
				side_dir.Normalise();

				ray.InitialiseRay(raisedWorldPos, raisedWorldPos + (side_dir * (g_debugLaserBoxWidth)), &rayCollisionFilter);
				pRLC->GetPhysicsScene()->DoRayTrace(ray, stack);
				Collision::C_RayTracingResult resultright = ray.m_result;

				float ray_size=resultleft.m_distance+resultright.m_distance;

				if(resultleft.Hit())
					pActor->AddDebugPoint(resultleft.m_contact,Maths::RGBA(255,255,255));

				if(resultright.Hit())
					pActor->AddDebugPoint(resultright.m_contact,Maths::RGBA(255,255,255));

				if(ray_size <= g_debugLaserBoxWidth)	// && (resultleft.Hit() && resultright.Hit()))
				{
					mGridActive=true;
					
					mGridLeftPos=resultleft.HitPoint();
					mGridRightPos=resultright.HitPoint();

					for(int a=0;a<NUM_FRICKING_LASER_BEAMS * (mLevel+1);a++)
					{
						mLeftHeights[a].mDirection=gPickupTrapRandomNumber.Rand(1) ? true : false;
						mLeftHeights[a].mPos=gPickupTrapRandomNumber.FRand(-1.f,4.f);

						mRightHeights[a].mDirection=gPickupTrapRandomNumber.Rand(1) ? true : false;
						mRightHeights[a].mPos=gPickupTrapRandomNumber.FRand(-1.f,4.f);
					}

					break;
				}
			}

			if(!mGridActive)
				return true;
		}

		if(mGridActive)
		{
			for(U32 a=0;a<levelState.mNumCars;a++)
			{
				if(a == mFiringVehicle)
					continue;

				Maths::HVec3 last_vehicle_pos = pLayer->mVehiclePositions[a].mLastPosition;
				Maths::HVec3 vehicle_pos = pLayer->mVehiclePositions[a].mPosition;

				if(TrapTriggered(vehicle_pos,last_vehicle_pos,mGridLeftPos,mGridRightPos))
				{
					if(mLevel == 0)
					{
						//disable perks + shield
						C_VehicleLogicMsg_EnableDisablePickupSystem pu_msg(a, g_debugTrapDisablePerkShieldTimer, g_debugTrapDisablePerkShieldTimer);
						pLayer->GetLogicVehicleActor(a)->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &pu_msg);


						Maths::HVec3 c4_pos = mGridLeftPos;
						c4_pos.Y(pLayer->mVehiclePositions[a].mPosition.Y());
						Maths::HVec3 bomb_vector = pLayer->mVehiclePositions[a].mPosition - c4_pos;
						float length=bomb_vector.Length();

						length=length/g_debugLaserBoxWidth;
						length=Maths::Clamp(length,0.f,1.f);
						length=1.0f-length;

						bomb_vector = Maths::Normalise(bomb_vector);

						Actors::C_VehicleLogicActor* pTargetVeh = pLayer->GetLogicVehicleActor(a);

						bomb_vector *= (g_debugLaserBoxBlastForce * length);
						bomb_vector=bomb_vector * pLayer->mVehiclePositions[a].mMatrix.Rotation().Inverse();

						Gravity::C_Body* pBody = pActor->GetGraphicalActor(a)->GetCollisionBody();
						Maths::HVec3 centreOfMassPosition = pBody->GetBody().m_CenterOfMassOffset;

						pBody->ApplyImpulseLocalSpace( bomb_vector, centreOfMassPosition);


						c4_pos = mGridRightPos;
						c4_pos.Y(pLayer->mVehiclePositions[a].mPosition.Y());
						bomb_vector = pLayer->mVehiclePositions[a].mPosition - c4_pos;

						length=bomb_vector.Length();

						length=length/g_debugLaserBoxWidth;
						length=Maths::Clamp(length,0.f,1.f);
						length=1.0f-length;

						bomb_vector = Maths::Normalise(bomb_vector);

						bomb_vector *= (g_debugLaserBoxBlastForce * length);
						bomb_vector=bomb_vector * pLayer->mVehiclePositions[a].mMatrix.Rotation().Inverse();

						pBody = pActor->GetGraphicalActor(a)->GetCollisionBody();
						centreOfMassPosition = pBody->GetBody().m_CenterOfMassOffset;

						pBody->ApplyImpulseLocalSpace( bomb_vector, centreOfMassPosition);

					}
					else
					{
						//remove perks off the player
						Game::Actors::PickupRemovePowerMsg removePickupMsg(a, 3);	//need to remove hard coding
						pLayer->GetLogicVehicleActor(a)->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &removePickupMsg);
					}
					return true;
				}
			}

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(mGridLeftPos,1.f,Maths::RGBA(255,255*mLevel,0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(mGridRightPos,1.f,Maths::RGBA(255,255*mLevel,0));

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(mGridLeftPos,mGridRightPos,Maths::RGBA(255,255*mLevel,0));

			for(int a=0;a<NUM_FRICKING_LASER_BEAMS * (mLevel+1);a++)
			{
				if(mLeftHeights[a].mDirection)
				{
					mLeftHeights[a].mPos+=frameTime * (2.f * (mLevel+1));
					if(mLeftHeights[a].mPos > 4.0f)
					{
						mLeftHeights[a].mPos=4.0f;
						mLeftHeights[a].mDirection=false;
					}
				}
				if(mLeftHeights[a].mDirection == false)
				{
					mLeftHeights[a].mPos-=frameTime * (2.f * (mLevel+1));
					if(mLeftHeights[a].mPos < -1.0f)
					{
						mLeftHeights[a].mPos=-1.0f;
						mLeftHeights[a].mDirection=true;
					}
				}

				if(mRightHeights[a].mDirection)
				{
					mRightHeights[a].mPos+=frameTime * (2.f * (mLevel+1));
					if(mRightHeights[a].mPos > 4.0f)
					{
						mRightHeights[a].mPos=4.0f;
						mRightHeights[a].mDirection=false;
					}
				}
				if(mRightHeights[a].mDirection == false)
				{
					mRightHeights[a].mPos-=frameTime * (2.f * (mLevel+1));
					if(mRightHeights[a].mPos < -1.0f)
					{
						mRightHeights[a].mPos=-1.0f;
						mRightHeights[a].mDirection=true;
					}
				}

				//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(mGridLeftPos+Maths::HVec3(0.f,mLeftHeights[a].mPos,0.f),mGridRightPos+Maths::HVec3(0.f,mRightHeights[a].mPos,0.f),Maths::RGBA(255,255*mLevel,0));
			}

		}

		mTimer -= frameTime;

		if(mTimer < 0.f)
		{
			return true;
		}	

		return false;
	}

	void C_PickupFireTrap::WarnPlayers()
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
	


	bool C_PickupHitTrap::Update(int vehIdx, bool p_IsMultiplayer)
	{
		C_RaceLogicLayer* pLayer					= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pFiringActor	= pLayer->GetLogicVehicleActor(mFiringVehicle);
		Actors::C_VehicleLogicActor* pHitActor		= pLayer->GetLogicVehicleActor(vehIdx);


		return true;
	}
}
