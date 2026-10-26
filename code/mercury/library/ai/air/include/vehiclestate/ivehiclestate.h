#ifndef _VEHICLE_STATE_I_H
#define _VEHICLE_STATE_I_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ComponentSystem/componentlib.h>
#include <vehiclestate/vehiclestate.h>


namespace airacingtech
{

	BEGIN_INTERFACE_DEC(IVehicleState)
	
	public:
	
		virtual airacingtech::VehicleState *GetState()= 0;

	END_INTERFACE_DEC
}

#endif //_VEHICLE_STATE_I_H
