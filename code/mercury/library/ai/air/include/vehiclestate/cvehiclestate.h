#ifndef _VEHICLE_STATE_C_H
#define _VEHICLE_STATE_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <vehiclestate/ivehiclestate.h>


namespace airacingtech
{

	BEGIN_COMPONENT_DEC(CVehicleState, IVehicleState)
	
	public:
		inline airacingtech::VehicleState *GetState() {return &m_vehicleState;}

	private:
		airacingtech::VehicleState	m_vehicleState;

	END_COMPONENT_DEC


}



#endif //_VEHICLE_STATE_C_H
