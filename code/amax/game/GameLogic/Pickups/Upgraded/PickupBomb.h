#ifndef PICKUP_BOMB_H
#define PICKUP_BOMB_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Pickups/PickupBase.h>
#include <GameLogic\Pickups\PickupMineBase.h>
#include <GameLogic\RaceLogicCore\Database\RLDatabase.h>

namespace Game
{
	class C_PickupBomb : public C_PickupMine
	{
	public:
		C_PickupBomb(S32 classType, U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX, float timerOverride = -1.f, const bool useMasterMineVersionOfTheMineSFX = false);

		virtual void PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx = 0);
		virtual void SetPostDelayMatrix(Maths::HMat43 matrix, float fVal = 0.f);
		virtual bool Update();

		virtual void Explode();
		virtual void Trigger(int idx);
		virtual void Prime(int idx);

		virtual void DebugDraw();

		void FlagAsLobbedMineRecentlyComeToRest();
		bool GetHasBeenRecentlyLobbed() const;

	private:

		void SetMasterMineFiring(const U8 vehicleIndex, const bool firing);
		void UpdateUnPrimeableTimer(const float elapsedTime);
		void UpdateMasterMineTimer(const float elapsedTime);

	protected:
		RLBombBalancingData*	mpBombData;
		RLBombSpecialFX*		mpBombSFX;

	private:

		bool					mPrimed;
		bool					mMasterMineEquipped;
		float					mMasterMineTimer; //Set if we're just about to convert
		float					mUnPrimeableTimer;
		S8						mVehicleIndexWhoPrimedTheMine;

		// These flags are used to keep track of lobbed mines that have recently come to rest
		F32						mTimeConsideredRecentlyComeToRest;
		F32						mRecentlyComeToRestTimer;

		// helper func
		int GetControllerIndex(const int idx);
	};
}

#endif //PICKUP_BOMB_H