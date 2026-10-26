/////////////////////////////////////////////////////////////
// Setup
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_INPUT_H__
#define __VEHICLE_INPUT_H__

#include <btl/containers/vector.h>

namespace Vehicle
{

/* 
===============================================================
					class C_WheelInput

NOTES: 
===============================================================
*/
class C_WheelInput
{
public:

	C_WheelInput();
	~C_WheelInput();

public:

	Maths::HMat43				m_mMatrix;							//!< Position and orientation of physics vehicle
	Maths::HMat33				m_mYawMatrix;

	Maths::HVec3				m_vGroundPosition;
	Maths::HVec3				m_vGroundNormal;
	Maths::HVec3				m_vAngularVelocity;
	Maths::HVec3				m_vAngularAcceleration;

	U32							m_iSurfaceType;
	U32							m_iSurfaceModifier;

	F32							m_fSlipAngle;

	bool						m_bOnGround;
};

/* 
===============================================================
					class C_ModelInput

NOTES: 
===============================================================
*/
class C_ModelInput
{
public:

	C_ModelInput();
	~C_ModelInput();

	//! Should the vehicle be updated
	bool ShouldUpdate() { return m_bShouldUpdate; }

	//! Should the vehicle be rendered
	bool ShouldRender() { return m_bShouldRender; }

	//! Clears out the structure
	void Clear();

public:

	Maths::HMat43				m_mMatrix;							//!< Position and orientation of physics vehicle

	Maths::HVec3				m_vCentre;
	Maths::HVec3				m_vExtents;

	Maths::HVec3				m_vVelocity;
	Maths::HVec3				m_vAcceleration;
	Maths::HVec3				m_vAngularVelocity;
	Maths::HVec3				m_vAngularAcceleration;

	S8							m_iGear;
	
	F32							m_fRevs;
	F32							m_fRedLine;							//!<Max engine revs
	F32							m_fThrottle;
	F32							m_fBrake;
	F32							m_fSpeed;							//!< Speed in XZ plane, can be negative (usually in odd places)
	F32							m_fMagVelocity;						//!< Magnitude of the velocity
	F32							m_fSteering;

	bool						m_bHandbrake;
	bool						m_bShouldUpdate;
	bool						m_bShouldRender;

	struct LightState
	{
		bool					m_Initial;
		bool					m_Current;
	};
	LightState					m_bHeadLights;
	LightState					m_bHeadLightCones;
	LightState					m_bRearLights;
	LightState					m_bFogLights;

	bool						m_bAllWheelsOnGround;
	Btl::Vector<C_WheelInput>	m_Wheels;

	S32							m_iControlType;						//!< Actually enum RacingPhysics::Vehicle::ControlType but I didn't want to have include the header

	struct PartShakeParams
	{
		float m_MinSpeed;
		float m_SpeedRange;
		float m_Torque;
	};
	PartShakeParams				m_PartShakeParams;
};

} //namespace Vehicle

#endif
