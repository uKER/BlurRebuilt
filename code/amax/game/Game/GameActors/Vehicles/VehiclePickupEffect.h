/////////////////////////////////////////////////////////////
// VehicleActor
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_PICKUP_ACTOR_H__
#define __VEHICLE_PICKUP_ACTOR_H__

namespace SpecialFX
{
	class I_SpecialFXInstance;
}

struct VehiclePickupEffect
{
	void Initialise()
	{
		m_pSFX = 0;
		mTime = 0.f;
	}

	SpecialFX::I_SpecialFXInstance*				m_pSFX;
	float										mTime;
};


#endif //__VEHICLE_PICKUP_ACTOR_H__

