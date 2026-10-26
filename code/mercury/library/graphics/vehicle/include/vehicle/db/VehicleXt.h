/////////////////////////////////////////////////////////////
// VehicleXt
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_XT_H__
#define __VEHICLE_XT_H__

#include <Xt/XtGame.h>

#include <Vehicle/PartCodes.h>
#include <Vehicle/Material/MaterialTypes.h>
#include <Vehicle/Utils/Curve.h>
#include <Dynamic/Light/Generic/XtInterface.h>

namespace Vehicle
{

////////////////////////////////////////////////////////////////////////
//						Begin: Misc
////////////////////////////////////////////////////////////////////////

struct XtHermiteKeyFrame
{
	float				m_Time;
	float				m_Value;
	float				m_InTangent;
	float				m_OutTangent;
};

struct XtCurve
{
	XtArray<XtHermiteKeyFrame>	m_KeyFrames;
};

////////////////////////////////////////////////////////////////////////
//						End: Misc
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//						Begin: Parts
////////////////////////////////////////////////////////////////////////

struct XtPart
{
	E_PartCode			m_PartCode;
	E_PartSubCode		m_PartSubCode;	
};

struct XtWindow
{
	XtHandle<XtPart>	m_Part;

	XtF32				m_Speed;
	XtF32				m_FinalRotation;
	XtVec3				m_FinalPosition;
};

////////////////////////////////////////////////////////////////////////
//						End: Parts
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//						Begin: Damage
////////////////////////////////////////////////////////////////////////

enum E_ConstraintType
{
	E_ConstraintType_Base = -1,

	E_ConstraintType_FixedJoint,
	E_ConstraintType_SphericalJoint,
	E_ConstraintType_RevoluteJoint,
	E_ConstraintType_RevoluteJointLimit,
	E_ConstraintType_PrismaticJoint,
	E_ConstraintType_PrismaticJointLimit,

	E_ConstraintType_Max,
};

class XtConstraintBase
{	
public:
	XtConstraintBase() { m_Type = E_ConstraintType_Base; }
	virtual void DebugDraw(Maths::HMat43 p_Matrix){}
public:
	E_ConstraintType	m_Type;
	XtHandle<XtPart>	m_Part;
};

class XtFixedJoint : public XtConstraintBase
{	
public:
	XtFixedJoint() { m_Type = E_ConstraintType_FixedJoint; }
	void DebugDraw(Maths::HMat43 p_Matrix);
public:
	XtVec3		m_Position;
};

class XtSphericalJoint : public XtConstraintBase
{	
public:
	XtSphericalJoint() { m_Type = E_ConstraintType_SphericalJoint; }
	void DebugDraw(Maths::HMat43 p_Matrix);
public:
	XtVec3		m_Position;
};

class XtRevoluteJoint : public XtConstraintBase
{	
public:
	XtRevoluteJoint() { m_Type = E_ConstraintType_RevoluteJoint; }
	void DebugDraw(Maths::HMat43 p_Matrix);
public:
	XtVec3		m_Position;
	XtVec3		m_Axis;
};

class XtRevoluteJointLimit : public XtConstraintBase
{	
public:
	XtRevoluteJointLimit() { m_Type = E_ConstraintType_RevoluteJointLimit; }
	void DebugDraw(Maths::HMat43 p_Matrix);
public:
	XtVec3		m_Position;
	XtVec3		m_Axis;
	XtF32		m_LowJointLimit;
	XtF32		m_HighJointLimit;
};

class XtPrismaticJoint : public XtConstraintBase
{	
public:
	XtPrismaticJoint() { m_Type = E_ConstraintType_PrismaticJoint; }
	void DebugDraw(Maths::HMat43 p_Matrix);
public:
	XtVec3		m_Axis;
};

class XtPrismaticJointLimit : public XtConstraintBase
{	
public:
	XtPrismaticJointLimit() { m_Type = E_ConstraintType_PrismaticJointLimit; }
	void DebugDraw(Maths::HMat43 p_Matrix);
public:
	XtVec3		m_Axis;
	XtF32		m_LowJointLimit;
	XtF32		m_HighJointLimit;
};

struct XtConstraints
{
	XtArray<XtConstraintBase*>	m_Constraints;
};

struct XtDamageTransfer
{
	XtHandle<XtPart>		m_Part;
	float					m_Percentage;
};

struct XtDamageBehaviour
{
	XtDamageBehaviour()
	{
		m_FlapDamageLevel	= 0.0f;
		m_DetachDamageLevel = 0.0f;
		m_BreakDamageLevel	= 0.0f;
		m_ImpactScale		= 0.0f;

		m_FlyTowardsCamera	= false;
		m_MakeDynamic		= false;
		m_MakeCollidable	= false;
		m_SpawnDebris		= false;
	}

	float					m_FlapDamageLevel;
	float					m_DetachDamageLevel;
	float					m_BreakDamageLevel;
	float					m_ImpactScale;

	bool					m_FlyTowardsCamera;
	bool					m_MakeDynamic;
	bool					m_MakeCollidable;
	bool					m_SpawnDebris;
};

class XtDamagedPart
{
public:
	XtHandle<XtPart>			m_Part;
	float						m_Mass;
	XtDamageBehaviour			m_Behaviour;
	XtArray<XtDamageTransfer>	m_DamageTransfer;
};

struct XtDamageCurveSetup
{
	float m_ZOffset;
	XtCurve	m_Width;	
	XtCurve	m_Length;
	XtCurve	m_Height;
	float m_HeightRange;
};

struct XtDamageSetup
{
	XtArray<XtDamagedPart>	m_Parts;
};

struct XtDeformationSetup
{
	float m_ZOffset;
	float m_FrontCrumpleZone;
	float m_RearCrumpleZone;
	float m_SideCrumpleZone;
	float m_FrontNoDamageZone;
	float m_RearNoDamageZone;
	float m_SideNoDamageZone;
	float m_FrontYMovement;
	float m_RearYMovement;
	float m_SideYMovement;
};

struct XtWheelDamageSetup
{
	float m_FrontTyreLossRadiusScale;
	float m_RearTyreLossRadiusScale;
	float m_FrontWheelLossRadiusScale;
	float m_RearWheelLossRadiusScale;
	bool m_DontBurstTyres;
	bool m_DontLoseWheels;
};

////////////////////////////////////////////////////////////////////////
//						End: Damage
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//						Begin: Paint
////////////////////////////////////////////////////////////////////////

struct XtPaintJob
{
	XtSz8				m_Name;
	XtRgb				m_UndercoatColour;
	XtRgb				m_ScuffColour;
	XtArray<XtRgb>		m_Colours;
	XtHandle<XtCurve>	m_ColourCurve;
	XtHandle<XtCurve>	m_ReflectivityCurve;
};

////////////////////////////////////////////////////////////////////////
//						End: Paint
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//						Begin: Materials
////////////////////////////////////////////////////////////////////////

class XtMaterialBase
{
public:
	XtMaterialBase() { m_Type = E_MaterialType_Base; }
public:
	E_MaterialType	m_Type;
};

class XtMaterialPaint : public XtMaterialBase
{
public:
	XtMaterialPaint() { m_Type = E_MaterialType_Paint; }
public:
	XtRgb				m_Colour;
};

class XtMaterialGlass : public XtMaterialBase
{
public:
	XtMaterialGlass() { m_Type = E_MaterialType_Glass; }
public:
	XtRgb				m_Tint;
	float				m_MinAlpha;
	float				m_MaxAlpha;
	float				m_MinReflectivity;
	float				m_MaxReflectivity;
};

class XtMaterialTyre : public XtMaterialBase
{
public:
	XtMaterialTyre() { m_Type = E_MaterialType_Tyre; }
public:
	float				m_TreadDepth;
};

class XtMaterialAlloy : public XtMaterialBase
{
public:
	XtMaterialAlloy() { m_Type = E_MaterialType_Alloy; }
public:
	XtRgb				m_Colour;
	float				m_SpecularPower;
	float				m_SpecularIntensity;
};

class XtMaterialLight : public XtMaterialBase
{
public:
	XtMaterialLight() { m_Type = E_MaterialType_Light; }
public:
	XtRgb				m_Tint;
	float				m_Intensity;
};

struct XtMaterialInstance
{
	XtPart						m_Part;
	XtHandle<XtMaterialBase>	m_Material;
};

struct XtMaterialDefaults
{
	XtArray< XtHandle< XtMaterialInstance> >	m_Defaults;
	XtHandle<XtPaintJob>						m_DefaultPaintJob;
};

////////////////////////////////////////////////////////////////////////
//						End: Materials
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//						Begin: Effects
////////////////////////////////////////////////////////////////////////
struct XtHeadlight
{
	float				m_ZOffset;
	float				m_Width;
	float				m_Length;
	float				m_Height;
	float				m_Intensity;
	float				m_Dip;
	XtRgb				m_Colour;
};

struct XtLightFlares
{
	XtHandle<DynamicEffect::Light::XtInterface::Light> m_HeadLight;
	XtHandle<DynamicEffect::Light::XtInterface::Light> m_ShowRoomHeadLight;
	XtHandle<DynamicEffect::Light::XtInterface::Light> m_RearLight;
	XtHandle<DynamicEffect::Light::XtInterface::Light> m_FogLight;
	XtHandle<DynamicEffect::Light::XtInterface::Light> m_BrakeLight;
	XtHandle<DynamicEffect::Light::XtInterface::Light> m_CentreBrakeLight;
	XtHandle<DynamicEffect::Light::XtInterface::Light> m_ReverseLight;
	XtHandle<DynamicEffect::Light::XtInterface::Light> m_PlateLight;
	XtHandle<DynamicEffect::Light::XtInterface::Light> m_DashBoardLight;
};

struct XtDynamicLight
{
	bool				m_Enable;
	XtRgb				m_Colour;
	float				m_Radius;
	XtVec3				m_Offset;
};

struct XtDynamicLights
{
	XtDynamicLight m_DashBoardDynamicLight;
	XtDynamicLight m_HeadDynamicLight;
	XtDynamicLight m_RearDynamicLight;
	XtDynamicLight m_FogDynamicLight;
	XtDynamicLight m_BrakeDynamicLight;
	XtDynamicLight m_ReverseDynamicLight;
};

////////////////////////////////////////////////////////////////////////
//						End: Effects
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//						Begin: Vehicle
////////////////////////////////////////////////////////////////////////

struct XtVehicle
{
	XtHandle<XtDamageSetup>			m_DamagedParts;
	XtHandle<XtDamageCurveSetup>	m_DamageCurveSetup;
	XtHandle<XtDeformationSetup>	m_DeformationSetup;
	XtWheelDamageSetup				m_WheelDamageSetup;
	XtHandle<XtConstraints>			m_Constraints;
	XtArray<XtHandle<XtPaintJob> >	m_PaintJobs;
	XtHandle<XtMaterialDefaults>	m_MaterialOverrides;
	XtHandle<XtHeadlight>			m_Headlight;
	XtHandle<XtLightFlares>			m_LightFlares;
	XtF32							m_FrontLightRenderOffset;
	XtF32							m_RearLightRenderOffset;
	XtDynamicLights					m_DynamicLights;
	XtWindow						m_FunctionalWindow;
};

////////////////////////////////////////////////////////////////////////
//						End: Vehicle
////////////////////////////////////////////////////////////////////////

} //namespace Vehicle

#endif
