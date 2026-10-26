#ifndef PICKUP_BLINDFIRE_H
#define PICKUP_BLINDFIRE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <GameLogic/Pickups/PickupProjectileBase.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>


namespace Game
{
	class C_PickupBlindfireProjectile : public C_PickupProjectile
	{
	public:
		C_PickupBlindfireProjectile(S32 classType, U32 id, int firingIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX, int group_id);

		virtual void				PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx = 0);
		void						SetPostDelayMatrix(Maths::HMat43 targetMatrix, float fVal = 0.f);
		virtual bool				Update();

		bool						IsDefenderCentric() { return false; }
		virtual bool				NinjaKillIfAnyHit() { return true; }

		virtual void				Explode(Maths::HVec3 pos, Maths::C_Normal normal);
		virtual void				Trigger(int idx);

		virtual void				NinjaKill(const Game::PickupAutopsyReport& autoReport);

		virtual E_TARGETTING_DIRECTION GetHitDirection(Actors::C_VehicleLogicActor* pActor);

		virtual void				DebugDraw();
		void						PhotoMove(F32 frac) OVERRIDE;

	protected:
		RLBlindfireBalancingData*	mpBlindfireData;
		RLBlindfireSpecialFX*		mpBlindfireSFX;

		float						mAliveTime;

	private:
		Maths::HVec3				m_PrevPos;
	};
}

#endif //PICKUP_SHUNT_H
