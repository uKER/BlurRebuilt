#include "stdafx.h"

#include <GameLogic/Pickups/Double/PickupDoubleShock.h>
#include <GameLogic/Pickups/PickupClasses.h>
#include <GameLogic\Layers\RaceBookLayer.h>
#include <GameLogic\Layers\AmaxWorldLayer.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>

#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <GameLogic\Pickups\PickupsManager.h>

#include <ai/AI_Amax.h>
#include <GameApp.h>
#include <Game\Game.h>

Debug::Bool g_debugDoubleShockDraw("Perks.DebugDraw.DoubleShock", true);
Debug::Float g_debugDoubleShockTimeToNext("Perks.DoubleShock.TimeToNext", 3.0f);
Debug::Float g_debugDoubleShockRadius("Perks.DoubleShock.Radius", 10.0f);
Debug::Float g_debugDoubleShockLifetime("Perks.DoubleShock.Lifetime", 30.0f);
Debug::Float g_debugDoubleShockCancelRadius("Perks.DoubleShock.CancelRadius", 30.0f);
Debug::Float g_debugDoubleShockWarningRadius("Perks.DoubleShock.WarningRadius", 50.0f);

namespace Game
{
	C_PickupDoubleShock::C_PickupDoubleShock(U32 id, int firingIdx, int targetIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX)
		: C_PickupBase(E_PickupClass_DoubleShock, id, firingIdx, E_PICKUP_TYPE_SHOCK, subType, noFiringVehicleFX), mShocksDropped(0), mTargetIdx(targetIdx)
	{
		RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_SHOCK);
		//mpShockData							= (RLShockBalancingData*)pPickup->mLevels[0]->mBalancingData;

		//BIZ_ASSERT(mpShockData);

		mTimeToNextShock = g_debugDoubleShockTimeToNext;
		mTotalLifetime = g_debugDoubleShockLifetime;
		mBombardRadius = g_debugDoubleShockRadius;
		mRandGenerator.Seed((U32)Time::GetCurrentTime());
	}

	// --------------------------------------------------------------------

	void C_PickupDoubleShock::PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx)
	{
		C_PickupBase::PostDelayInitialise(matrix, sfx);

		if(VehicleIsLocal(mTargetIdx))
		{
			//matrix = UpdateDomePosition();
			SetPostDelayMatrix(matrix);

			if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork())
			{
				Game::C_Game::GetInstance()->GetNetGameVars().m_EventManager->Add_PickupPostDelay(mUniqueId, matrix);
			}
		}
	}

	// --------------------------------------------------------------------

	void C_PickupDoubleShock::SetPostDelayMatrix( Maths::HMat43 targetMatrix, float fVal )
	{
		C_PickupBase::SetPostDelayMatrix(targetMatrix, fVal);
	}

	// --------------------------------------------------------------------

	bool C_PickupDoubleShock::Update()
	{
		if(!mActivated)
			return false;

		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mFiringIdx);
		float dist							= Maths::Distance(mMatrix.AW(), pActor->GetPositionInfo().mWorldPosition);

		bool expired = mShocksDropped >= 1 && ( dist <= g_debugDoubleShockCancelRadius || mTotalLifetime <= 0.f );

		if(mKillPickup || !VehicleIsLocal(mFiringIdx) || expired)
			return true;

		if(g_debugDoubleShockDraw)
			DebugDraw();

		float frameTime	= C_GameApp::GetInstance()->GetTimeStep();
		mTimeToNextShock -= frameTime;
		mTotalLifetime -= frameTime;

		if(mTimeToNextShock <= 0.f)
		{
			mTimeToNextShock = g_debugDoubleShockTimeToNext;

			float xOffset = mRandGenerator.FRand(-mBombardRadius,mBombardRadius);
			float zOffset = mRandGenerator.FRand(-mBombardRadius,mBombardRadius);

			Maths::HMat43 matrix = mMatrix;
			Maths::HVec3 pos = mMatrix.AW();
			pos.X(pos.X() + xOffset);
			pos.Z(pos.Z() + zOffset);
			matrix.AW(pos);

			mShocksDropped++;

			PickupCreationStruct pickupInfo(E_PICKUP_TYPE_SHOCK, E_PICKUP_SUB_TYPE_NORMAL, mFiringIdx);
			pickupInfo.mTargetIdx			= mFiringIdx;
			pickupInfo.mMatrix				= matrix;
			pickupInfo.mDisableVehicleFX	= true;
			pickupInfo.mOverrideMatrix		= true;

			C_PickupsManager::Instance().CreatePickup(pickupInfo);
		}

		return false;
	}

	// --------------------------------------------------------------------

	void C_PickupDoubleShock::DebugDraw()
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(mMatrix.AW(),Maths::HVec3(0,1,0),mBombardRadius*2,Maths::RGBA(0,0,255));
	}
}
