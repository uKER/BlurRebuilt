#ifndef _PERKSKILLUSGAE_AI_I_H
#define _PERKSKILLUSGAE_AI_I_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ComponentSystem/componentlib.h>

namespace Game
{
	class C_PickupBase;

	namespace Actors
	{
		class C_VehicleActor;
	}
}

struct AIPerkPrefsXt;
struct AIPerkSkillzXt;


namespace airacingtech
{

	class IData;


	BEGIN_INTERFACE_DEC(IPerkSkillUsage)

	public:

		virtual void PreUpdate() = 0;
		virtual void Update() = 0;
		virtual void PostUpdate() = 0;
		virtual void FireMessages() = 0;

		virtual int					GetNumProjectiles() = 0;
		virtual Game::C_PickupBase*	GetProjectile(int i) = 0;

		virtual float				GetTargetDistanceAhead(int idx) = 0;

		virtual void				PredictVehiclePos(Game::Actors::C_VehicleActor* pVehActor, Maths::HVec3& predictPos, float lookAheadTime) = 0;
		virtual void				PredictProjectilePos(Game::C_PickupBase* pPickup, Maths::HVec3& predictPos, float lookAheadTime) = 0;
		virtual float				PredictWillHitTarget(Maths::HVec3 shotPos, Maths::HVec3 shotVel, Maths::HVec3 targetPos, float missDot) = 0;


		virtual float GetPerkPreferenceValue(int perkType) = 0;
		virtual int	GetPickupStashSize() = 0;

		virtual AIPerkPrefsXt	*GetPerkPrefs() = 0;
		virtual AIPerkSkillzXt		*GetSkillz() = 0;

		virtual float GetMineTimeStamp() = 0;
		virtual int GetGroup(IData *pIData) = 0;

		virtual int	DebugActivePickupCount() = 0;

	END_INTERFACE_DEC


}


#endif //_PERKSKILLUSGAE_AI_I_H