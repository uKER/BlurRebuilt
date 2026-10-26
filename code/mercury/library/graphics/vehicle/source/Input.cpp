/////////////////////////////////////////////////////////////
// Setup
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/Input.h"

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

namespace Vehicle
{

/* 
===============================================================
					class C_WheelInput

NOTES: 
===============================================================
*/

/*
===============
NAME: C_WheelInput

NOTE: Constructor
===============
*/
C_WheelInput::C_WheelInput()
{
	m_mMatrix.SetIdentity();
	m_mMatrix.Translation(Maths::HVec3(0.0f, 0.0f, 0.0f));

	m_vGroundPosition			= Maths::HVec3(0.0f, 0.0f, 0.0f);
	m_vGroundNormal				= Maths::HVec3(0.0f, 0.0f, 0.0f);	
	m_vAngularVelocity			= Maths::HVec3(0.0f, 0.0f, 0.0f);
	m_vAngularAcceleration		= Maths::HVec3(0.0f, 0.0f, 0.0f);

	m_iSurfaceType				= UINT_MAX;
	m_iSurfaceModifier			= UINT_MAX;

	m_bOnGround					= false;

	m_fSlipAngle				= 0.f;
}

/*
===============
NAME: ~C_WheelInput

NOTE: Destructor
===============
*/
C_WheelInput::~C_WheelInput()
{

}

/* 
===============================================================
					class C_ModelInput

NOTES: 
===============================================================
*/

/*
===============
NAME: C_ModelInput

NOTE: Constructor
===============
*/
C_ModelInput::C_ModelInput()
{
	m_mMatrix.SetIdentity();
	m_mMatrix.Translation(Maths::HVec3(0.0f, 0.0f, 0.0f));

	m_vVelocity					= Maths::HVec3(0.0f, 0.0f, 0.0f);
	m_vAcceleration				= Maths::HVec3(0.0f, 0.0f, 0.0f);
	m_vAngularVelocity			= Maths::HVec3(0.0f, 0.0f, 0.0f);
	m_vAngularAcceleration		= Maths::HVec3(0.0f, 0.0f, 0.0f);

	m_iGear						= 0;
	
	m_fRevs						= 0.0f;
	m_fThrottle					= 0.0f;
	m_fBrake					= 0.0f;
	m_fSpeed					= 0.0f;
	m_fSteering					= 0.0f;

	m_bHandbrake				= false;

	m_bShouldUpdate				= true;
	m_bShouldRender				= true;
	
	m_bHeadLights.m_Current		= false;
	m_bHeadLights.m_Initial		= false;
	m_bHeadLightCones.m_Current	= false;
	m_bHeadLightCones.m_Initial	= false;
	m_bRearLights.m_Current		= false;
	m_bRearLights.m_Initial		= false;
	m_bFogLights.m_Current		= false;
	m_bFogLights.m_Initial		= false;
}

/*
===============
NAME: ~C_ModelInput

NOTE: Destructor
===============
*/
C_ModelInput::~C_ModelInput()
{

}

/*
===============
NAME: Clear

NOTE: Clears out the structure
===============
*/
void C_ModelInput::Clear()
{
	m_mMatrix.SetIdentity();
	m_mMatrix.Translation(Maths::HVec3(0.0f, 0.0f, 0.0f));

	m_vVelocity					= Maths::HVec3(0.0f, 0.0f, 0.0f);
	m_vAcceleration				= Maths::HVec3(0.0f, 0.0f, 0.0f);
	m_vAngularVelocity			= Maths::HVec3(0.0f, 0.0f, 0.0f);
	m_vAngularAcceleration		= Maths::HVec3(0.0f, 0.0f, 0.0f);

	m_iGear						= 0;
	
	m_fRevs						= 0.0f;
	m_fThrottle					= 0.0f;
	m_fBrake					= 0.0f;
	m_fSpeed					= 0.0f;
	m_fSteering					= 0.0f;

	m_bHandbrake				= false;

	m_bHeadLights.m_Current		= false;
	m_bHeadLights.m_Initial		= false;
	m_bHeadLightCones.m_Current	= false;
	m_bHeadLightCones.m_Initial	= false;
	m_bRearLights.m_Current		= false;
	m_bRearLights.m_Initial		= false;
	m_bFogLights.m_Current		= false;
	m_bFogLights.m_Initial		= false;
	
	for (S32 iWheel = 0; iWheel < m_Wheels.Size(); iWheel++)
	{
		m_Wheels[iWheel].m_mMatrix.SetIdentity();
		m_Wheels[iWheel].m_mMatrix.Translation(Maths::HVec3(0.0f, 0.0f, 0.0f));

		m_Wheels[iWheel].m_vGroundPosition				= Maths::HVec3(0.0f, 0.0f, 0.0f);
		m_Wheels[iWheel].m_vAngularVelocity				= Maths::HVec3(0.0f, 0.0f, 0.0f);
		m_Wheels[iWheel].m_vAngularAcceleration			= Maths::HVec3(0.0f, 0.0f, 0.0f);

		m_Wheels[iWheel].m_iSurfaceType			= 0;
		m_Wheels[iWheel].m_iSurfaceModifier		= 0;

		m_Wheels[iWheel].m_fSlipAngle			= 0.0f;

		m_Wheels[iWheel].m_bOnGround				= false;
	}
}

} //namespace Vehicle

