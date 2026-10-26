/////////////////////////////////////////////////////////////
// Lights
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/Effects/Lights.h"
#include "Vehicle/Component.h"

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

namespace Vehicle
{

/* 
===============================================================
					class C_Headlight

NOTES: 
===============================================================
*/

/*
===============
NAME: C_Headlight

NOTE: Constructor
===============
*/
C_Headlight::C_Headlight()
{
	m_Position	= Maths::HVec3(0.0f, 0.0f, 0.0f);
	m_Colour	= Maths::RGBA(0.0f, 0.0f, 0.0f, 0.0f);

	m_WorldMatrix.SetIdentity();

	m_ZOffset			= 0.0f;
	m_Width				= 0.0f;
	m_Length			= 0.0f;
	m_Height			= 0.0f;
	m_Intensity			= 0.0f;
	m_Dip				= 0.0f;
	m_IntensityModifier	= 1.0f;
	m_IntensityFade		= 1.0f;

	m_Enabled		= true;
}

} //namespace Vehicle
