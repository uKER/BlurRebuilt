/////////////////////////////////////////////////////////////
// Setup
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/Setup.h"
#include "Vehicle/Db/VehicleXt.h"

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

namespace Vehicle
{

/* 
===============================================================
					class C_VehicleModelSetup

NOTES: 
===============================================================
*/

/*
===============
NAME: C_VehicleModelSetup

NOTE: Constructor
===============
*/
C_VehicleModelSetup::C_VehicleModelSetup()
{
	m_FilePath				= "";
	m_FileName				= "";

	m_UniqueID				= UINT_MAX;
	m_Index					= UINT_MAX;

	m_bCreateRenderLayer	= true;
	m_bCreateGameLayer		= true;
	m_bCreateDamageLayer	= true;

	m_pXtVehicle			= NULL;

	m_InitialState.m_NightLights	= false;
	m_InitialState.m_ScratchLevel	= 0.0f;
	m_InitialState.m_DamageLevel	= 0.0f;

	m_pLightSamplePointScene		= NULL;
}

/*
===============
NAME: ~C_VehicleModelSetup

NOTE: Denstructor
===============
*/
C_VehicleModelSetup::~C_VehicleModelSetup()
{

}

/*
===============
NAME: Create

NOTE: Create
===============
*/
void C_VehicleModelSetup::Create()
{

}

} //namespace Vehicle

