#include "stdafx.h"
#include <GameAudio/GameAudioLayer.h>	// [Blur2PC]

#include <GameLogic/Pickups/Reverse/PickupNitrousReverse.h>
#include <GameLogic/Pickups/PickupClasses.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>
#include <GameLogic/StickerManager.h>
#include <ai/AI_Amax.h>

Debug::Float g_debugNitrousReverseRotateSpeed("Perks.NitrousReverse.RotationSpeed", 0.035f);
Debug::Float g_debugNitrousReverseRotateSteerLevel("Perks.NitrousReverse.RotationSteerLevel", 0.3f);
Debug::Float g_debugNitrousReverseRiftDuration("Perks.NitrousReverse.RiftDuration", 0.8f);
Debug::Float g_debugNitrousReverseRiftDot("Perks.NitrousReverse.RiftDot", 0.8f);
Debug::Float g_debugNitrousReverseMinSpeedMultiplier("Perks.NitrousReverse.MinSpeedMultiplier", 1.1f);
Debug::Bool	 g_debugNitrousReverseDraw("Perks.DebugDraw.NitrousReverse", false);

namespace Game
{
	C_PickupNitrousReverse::C_PickupNitrousReverse(U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX)
		: C_PickupBase(E_PickupClass_NitrousReverse, id, firingIdx, E_PICKUP_TYPE_NITROUS, subType, noFiringVehicleFX), mTimer(0.f)
	{
		RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_NITROUS);
		mpNitrousData						= (RLNitrousReverseBalancingData*)pPickup->mReversed->mBalancingData;

		RegisterHit(firingIdx);
	}

	// ------------------------------------------------------

	bool C_PickupNitrousReverse::Update()
	{
		PERF_EVENT("PickupNitrousReverse.Update");

		if(!mActivated)
			return false;

		if(mTimer <= g_debugNitrousReverseRiftDuration)
		{
			C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mFiringIdx);

			if(g_debugNitrousReverseDraw)
				DebugDraw();

			for(int a = 0; a < pLayer->GetNumVehicles(); a++)
			{
				if(a == mFiringIdx)
					continue;

				if(pActor->mVehicleProximities[a].mDistanceSquared <= mpNitrousData->mRiftRangeSq)
				{
					Actors::C_VehicleLogicActor* pOtherActor = pLayer->GetLogicVehicleActor(a);

					Maths::HVec3 vehicleDir = pActor->GetPositionInfo().mDirection;
					Maths::HVec3 vec		= Maths::Normalise(pOtherActor->GetPositionInfo().mWorldPosition - pActor->GetPositionInfo().mWorldPosition);
					float dot				= Maths::DotProduct(vec,vehicleDir);

					if(dot < g_debugNitrousReverseRiftDot)
					{
						if(!CheckCanHit(a, E_PICKUP_HIT_TYPE_MINOR))
							continue;

						RegisterHit(a, E_PICKUP_HIT_TYPE_MINOR);
					}
				}
			}

			mTimer += C_GameApp::GetInstance()->GetTimeStep();
			return false;	
		}

		return true;
	}

	// --------------------------------------------------------------------------------

	void C_PickupNitrousReverse::PostDelayInitialise( Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx)
	{
		C_PickupBase::PostDelayInitialise(matrix, sfx);
		C_PickupBase::SetPostDelayMatrix(matrix);
	}

	// --------------------------------------------------------------------------------

	E_TARGETTING_DIRECTION C_PickupNitrousReverse::GetHitDirection(Actors::C_VehicleLogicActor* pActor)
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

	// ------------------------------------------------------

	void C_PickupNitrousReverse::DebugDraw()
	{
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mFiringIdx);

		Maths::HVec3 vehPos = pActor->GetWorldMatrix().AW();

		float riftRadius = Maths::Sqrt(mpNitrousData->mRiftRangeSq);

		float angle1 = Maths::ArcCos((F32)g_debugNitrousReverseRiftDot);
		float angle2 = Maths::ArcCos((F32)-g_debugNitrousReverseRiftDot);

		Maths::HVec3 newPos1 = Maths::HVec3(Maths::Cos(angle1), 0, Maths::Sin(angle1));
		Maths::HVec3 newPos2 = Maths::HVec3(Maths::Cos(angle2), 0, Maths::Sin(angle2));

		newPos1 = (newPos1 * riftRadius)* pActor->GetWorldMatrix();
		newPos2 = (newPos2 * riftRadius)* pActor->GetWorldMatrix();

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(vehPos,Maths::HVec3(0,1,0),riftRadius,Maths::RGBA(0,0,255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(vehPos, newPos1, Maths::RGBA(0,0,255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(vehPos, newPos2, Maths::RGBA(0,0,255));
	}

	// ------------------------------------------------------

	C_PickupEffectsNitrousReverse::C_PickupEffectsNitrousReverse(E_PICKUP_TYPE type, int vehIdx, float time)
		: C_PickupLocalEffectsBase(type, vehIdx, time, E_PICKUP_SUB_TYPE_REVERSED), mStraightenDelay(0.f), mPreviousSpeed(0.f), mDisableForwardBoost(false)
	{
		RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_NITROUS);
		mpNitrousData						= (RLNitrousReverseBalancingData*)pPickup->mReversed->mBalancingData;

		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(vehIdx);
		pActor->GetGraphicalActor(vehIdx)->GetFakeSpeed()->Start(Game::Actors::E_FAKE_SPEED_BRAKE, 25.f);

		mPreviousSpeed						= pActor->GetPositionInfo().mSpeedInMph;
		mStraightenDelay					= 5.f;
	}

	// ------------------------------------------------------

	void C_PickupEffectsNitrousReverse::KillEffect()
	{
		mDisableForwardBoost = true;
		C_PickupLocalEffectsBase::KillEffect();
	}

	bool C_PickupEffectsNitrousReverse::Update()
	{
		float frameTime						= C_GameApp::GetInstance()->GetTimeStep();
		mTimer += frameTime;

		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor	= pLayer->GetLogicVehicleActor(mVehicleIndex);

		if(mTimer > mpNitrousData->mDuration)
		{
			if(pActor && !mDisableForwardBoost)
			{
				float newSpeed = mPreviousSpeed * g_debugNitrousReverseMinSpeedMultiplier;
				pActor->GetGraphicalActor(mVehicleIndex)->GetFakeSpeed()->Start(Game::Actors::E_FAKE_SPEED_ACCELERATE, newSpeed);

				RLPickupType* pPickup = (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_NITROUS);

				if(pPickup && pPickup->mReversed && pPickup->mReversed->mSpecialFX)
				{
					RLNitrousReverseSpecialFX* pSFX		= (RLNitrousReverseSpecialFX*)pPickup->mReversed->mSpecialFX;
					C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
					Actors::C_VehicleLogicActor* pActor	= pLayer->GetLogicVehicleActor(mVehicleIndex);

					Game::C_InGameSpecialFXManager::Instance().CreateSpecialFX(pSFX->mForwardSpecialFX, pActor->GetWorldMatrix(), mVehicleIndex, -1, -1, Game::E_InGameSpecialFX_Trigger_Firing);
				}

				//send end msg to audio
				C_VehicleLogicMsg_PerkEnded pMsg(mVehicleIndex,mUniqueId,E_PICKUP_TYPE_NITROUS,Game::E_BodyUserDataType_Actor,E_PICKUP_SUB_TYPE_REVERSED);//	= Messaging::MessageCast<C_VehicleLogicMsg_PerkEnded>(pMessage);
				Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Ended, 0, &pMsg);
				Game::C_StickerManager::Instance().MessageHandler(&pMsg); //we also need to know about this for a sticker
			}

			return true;
		}
		else
		{
			Actors::C_VehicleActor* pVehActor		= pActor->GetGraphicalActor(mVehicleIndex);
			RacingPhysics::CarControls& controls	= ((RacingPhysics::Car*)pVehActor->GetVehiclePhysics())->Controls();

			// 2 wheels must be on the ground to rotate
			if(abs(controls.steer) >= g_debugNitrousReverseRotateSteerLevel && pVehActor->GetVehiclePhysics()->NumWheelsOnGround() >= 2)
			{
				Gravity::C_Body* pBody					= pVehActor->GetCollisionBody();
				SharedPhysics::C_RigidBody& pRigidBody	= pBody->GetBody();

				Maths::C_MatrixRotateRPH rph;
				rph.m_Rotate.Z((F32)g_debugNitrousReverseRotateSpeed * controls.steer);

				pRigidBody.m_OrientationMatrix	= Maths::HMat33(rph) * pRigidBody.m_OrientationMatrix;
				pRigidBody.m_Orientation		= Maths::C_Quaternion(pRigidBody.m_OrientationMatrix);
			}
		}

		return false;
	}
}
