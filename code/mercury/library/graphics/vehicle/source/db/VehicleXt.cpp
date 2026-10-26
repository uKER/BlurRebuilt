/////////////////////////////////////////////////////////////
// VehicleXt
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include <Vehicle/Db/VehicleXt.h>
#include "Vehicle/PartCodes.h"
#include "Vehicle/Material/Material.h"
#include <Xt/Impl/XtReflection.h>

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

namespace Vehicle
{

} //namespace Vehicle

////////////////////////////////////////////////////////////////////////
//						Begin: Damage
////////////////////////////////////////////////////////////////////////

XT_TYPE(Vehicle::E_ConstraintType,						"VEH_ConstraintType")
	XT_OPTION(Vehicle::E_ConstraintType_FixedJoint,				"FixedJoint")
	XT_OPTION(Vehicle::E_ConstraintType_SphericalJoint,			"SphericalJoint")
	XT_OPTION(Vehicle::E_ConstraintType_RevoluteJoint,			"RevoluteJoint")
	XT_OPTION(Vehicle::E_ConstraintType_RevoluteJointLimit,		"RevoluteJointLimit")
	XT_OPTION(Vehicle::E_ConstraintType_PrismaticJoint,			"PrismaticJoint")
	XT_OPTION(Vehicle::E_ConstraintType_PrismaticJointLimit,	"PrismaticJointLimit")
XT_END_TYPE

XT_TYPE(Vehicle::XtConstraintBase,				"VEH_ConstraintBase")
	XT_FIELD(m_Part,									"part")
XT_END_TYPE

XT_TYPE(Vehicle::XtFixedJoint,					"VEH_FixedJoint")
	XT_BASE(Vehicle::XtConstraintBase)
	XT_FIELD(m_Position,								"position")
XT_END_TYPE

XT_TYPE(Vehicle::XtSphericalJoint,				"VEH_SphericalJoint")
	XT_BASE(Vehicle::XtConstraintBase)
	XT_FIELD(m_Position,								"position")
XT_END_TYPE

XT_TYPE(Vehicle::XtRevoluteJoint,				"VEH_RevoluteJoint")
	XT_BASE(Vehicle::XtConstraintBase)
	XT_FIELD(m_Position,								"position")
	XT_FIELD(m_Axis,									"axis")
XT_END_TYPE

XT_TYPE(Vehicle::XtRevoluteJointLimit,			"VEH_RevoluteJointLimit")
	XT_BASE(Vehicle::XtConstraintBase)
	XT_FIELD(m_Position,								"position")
	XT_FIELD(m_Axis,									"axis")
	XT_FIELD(m_LowJointLimit,							"lowJointLimit")
	XT_FIELD(m_HighJointLimit,							"highJointLimit")
XT_END_TYPE

XT_TYPE(Vehicle::XtPrismaticJoint,				"VEH_PrismaticJoint")
	XT_BASE(Vehicle::XtConstraintBase)
	XT_FIELD(m_Axis,									"axis")
XT_END_TYPE

XT_TYPE(Vehicle::XtPrismaticJointLimit,			"VEH_PrismaticJointLimit")
	XT_BASE(Vehicle::XtConstraintBase)
	XT_FIELD(m_Axis,									"axis")
	XT_FIELD(m_LowJointLimit,							"lowJointLimit")
	XT_FIELD(m_HighJointLimit,							"highJointLimit")
XT_END_TYPE

XT_TYPE(Vehicle::XtConstraints,					"VEH_Constraints")
	XT_FIELD(m_Constraints,								"constraints")
XT_END_TYPE

XT_TYPE(Vehicle::XtDamageTransfer,				"VEH_DamageTransfer")
	XT_FIELD(m_Part,									"part")
	XT_FIELD(m_Percentage,								"percentage")
XT_END_TYPE

XT_TYPE(Vehicle::XtDamageBehaviour,				"VEH_DamageBehaviour")
	XT_FIELD(m_FlapDamageLevel,							"flapDamageLevel")
	XT_FIELD(m_DetachDamageLevel,						"detachDamageLevel")
	XT_FIELD(m_BreakDamageLevel,						"breakDamageLevel")
	XT_FIELD(m_ImpactScale,								"impactScale")
	XT_FIELD(m_FlyTowardsCamera,						"flyTowardsCamera")
	XT_FIELD(m_MakeDynamic,								"makeDynamic")
	XT_FIELD(m_MakeCollidable,							"makeCollidable")
	XT_FIELD(m_SpawnDebris,								"spawnDebris")
XT_END_TYPE

XT_TYPE(Vehicle::XtDamagedPart,					"VEH_DamagedPart")
	XT_FIELD(m_Part,									"part")
	XT_FIELD(m_Mass,									"mass")
	XT_FIELD(m_Behaviour,								"behaviour")
	XT_FIELD(m_DamageTransfer,							"damageTransfer")
XT_END_TYPE

XT_TYPE(Vehicle::XtDamageCurveSetup,			"VEH_DamageCurveSetup")
	XT_FIELD(m_ZOffset,									"zOffset")
	XT_FIELD(m_Width,									"width")
	XT_FIELD(m_Length,									"length")
	XT_FIELD(m_Height,									"height")
	XT_FIELD(m_HeightRange,								"heightRange")	
XT_END_TYPE

XT_TYPE(Vehicle::XtDamageSetup,					"VEH_DamageSetup")
	XT_FIELD(m_Parts,									"parts")
XT_END_TYPE

XT_TYPE(Vehicle::XtDeformationSetup,					"VEH_DeformationSetup")
	XT_FIELD(m_ZOffset,									"zOffset")
	XT_FIELD(m_FrontCrumpleZone,						"frontCrumpleZone")
	XT_FIELD(m_RearCrumpleZone,							"rearCrumpleZone")
	XT_FIELD(m_SideCrumpleZone,							"sideCrumpleZone")
	XT_FIELD(m_FrontNoDamageZone,						"frontNoDamageZone")
	XT_FIELD(m_RearNoDamageZone,						"rearNoDamageZone")
	XT_FIELD(m_SideNoDamageZone,						"sideNoDamageZone")
	XT_FIELD(m_FrontYMovement,							"frontYMovement")
	XT_FIELD(m_RearYMovement,							"rearYMovement")
	XT_FIELD(m_SideYMovement,							"sideYMovement")
XT_END_TYPE

XT_TYPE(Vehicle::XtWheelDamageSetup,				"VEH_WheelDamageSetup")
	XT_FIELD(m_FrontTyreLossRadiusScale,				"frontTyreLossRadiusScale")
	XT_FIELD(m_RearTyreLossRadiusScale,					"rearTyreLossRadiusScale")
	XT_FIELD(m_FrontWheelLossRadiusScale,				"frontWheelLossRadiusScale")
	XT_FIELD(m_RearWheelLossRadiusScale,				"rearWheelLossRadiusScale")
	XT_FIELD(m_DontBurstTyres,							"dontBurstTyres")
	XT_FIELD(m_DontLoseWheels,							"dontLoseWheels")
XT_END_TYPE

////////////////////////////////////////////////////////////////////////
//						End: Damage
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//						Begin: Vehicle
////////////////////////////////////////////////////////////////////////

XT_TYPE(Vehicle::XtVehicle,						"VEH_Vehicle")
	XT_FIELD(m_DamagedParts,							"damagedParts")
	XT_FIELD(m_DamageCurveSetup,						"damageCurveSetup")
	XT_FIELD(m_DeformationSetup,						"deformationSetup")
	XT_FIELD(m_WheelDamageSetup,						"wheelDamageSetup")
	XT_FIELD(m_Constraints,								"constraints")
	XT_FIELD(m_PaintJobs,								"paintJobs")
	XT_FIELD(m_MaterialOverrides,						"materialOverrides")
	XT_FIELD(m_Headlight,								"headlight")
	XT_FIELD(m_LightFlares,								"lightFlares")
	XT_FIELD(m_FrontLightRenderOffset,					"frontLightRenderOffset")
	XT_FIELD(m_RearLightRenderOffset,					"rearLightRenderOffset")
	XT_FIELD(m_DynamicLights,							"dynamicLights")
	XT_FIELD(m_FunctionalWindow,						"functionalWindow")	
XT_END_TYPE

////////////////////////////////////////////////////////////////////////
//						End: Vehicle
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//						Begin: Materials
////////////////////////////////////////////////////////////////////////

XT_TYPE(Vehicle::E_MaterialType,				"VEH_MaterialType")
	XT_OPTION(Vehicle::E_MaterialType_Base,				"E_MaterialType_Base")
	XT_OPTION(Vehicle::E_MaterialType_Paint,			"E_MaterialType_Paint")
	XT_OPTION(Vehicle::E_MaterialType_Glass,			"E_MaterialType_Glass")
	XT_OPTION(Vehicle::E_MaterialType_Alloy,			"E_MaterialType_Alloy")
	XT_OPTION(Vehicle::E_MaterialType_Light,			"E_MaterialType_Light")
XT_END_TYPE

XT_TYPE(Vehicle::XtMaterialBase,				"VEH_MaterialBase")
XT_END_TYPE

XT_TYPE(Vehicle::XtMaterialPaint,				"VEH_MaterialPaint")
	XT_BASE(Vehicle::XtMaterialBase)
	XT_FIELD(m_Colour,									"colour")
XT_END_TYPE

XT_TYPE(Vehicle::XtMaterialGlass,				"VEH_MaterialGlass")
	XT_BASE(Vehicle::XtMaterialBase)
	XT_FIELD(m_Tint,									"tint")
	XT_FIELD(m_MinAlpha,								"minAlpha")
	XT_FIELD(m_MaxAlpha,								"maxAlpha")
	XT_FIELD(m_MinReflectivity,							"minReflectivity")
	XT_FIELD(m_MaxReflectivity,							"maxReflectivity")	
XT_END_TYPE

XT_TYPE(Vehicle::XtMaterialTyre,				"VEH_MaterialTyre")
	XT_BASE(Vehicle::XtMaterialBase)
	XT_FIELD(m_TreadDepth,								"treadDepth")
XT_END_TYPE

XT_TYPE(Vehicle::XtMaterialAlloy,				"VEH_MaterialAlloy")
	XT_BASE(Vehicle::XtMaterialBase)
	XT_FIELD(m_Colour,									"colour")
	XT_FIELD(m_SpecularPower,							"specPow")
	XT_FIELD(m_SpecularIntensity,						"specInt")
XT_END_TYPE

XT_TYPE(Vehicle::XtMaterialLight,				"VEH_MaterialLight")
	XT_BASE(Vehicle::XtMaterialBase)
	XT_FIELD(m_Tint,									"tint")
	XT_FIELD(m_Intensity,								"intensity")
XT_END_TYPE

XT_TYPE(Vehicle::XtMaterialInstance,			"VEH_MaterialInstance")
	XT_FIELD(m_Part,									"part")
	XT_FIELD(m_Material,								"material")
XT_END_TYPE

XT_TYPE(Vehicle::XtMaterialDefaults,			"VEH_MaterialDefaults")
	XT_FIELD(m_Defaults,								"defaults")
	XT_FIELD(m_DefaultPaintJob,							"defaultPaintJob")
XT_END_TYPE

////////////////////////////////////////////////////////////////////////
//						End: Materials
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//						Begin: Paint
////////////////////////////////////////////////////////////////////////

XT_TYPE(Vehicle::XtHermiteKeyFrame,					"VEH_HermiteKeyFrame")
	XT_FIELD(m_Time,							"time")
	XT_FIELD(m_Value,							"value")
	XT_FIELD(m_InTangent,						"inTangent")
	XT_FIELD(m_OutTangent,						"outTangent")
XT_END_TYPE

XT_TYPE(Vehicle::XtCurve,					"VEH_Curve")
	XT_FIELD(m_KeyFrames,						"keyFrames")
XT_END_TYPE

XT_TYPE(Vehicle::XtPaintJob,				"VEH_PaintJob")
	XT_FIELD(m_Name,							"name")
	XT_FIELD(m_UndercoatColour,					"undercoatColour")
	XT_FIELD(m_ScuffColour,						"scuffColour")
	XT_FIELD(m_Colours,							"colours")
	XT_FIELD(m_ColourCurve,						"colourCurve")
	XT_FIELD(m_ReflectivityCurve,				"reflectivityCurve")
XT_END_TYPE

////////////////////////////////////////////////////////////////////////
//						End: Paint
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//						Begin: Effects
////////////////////////////////////////////////////////////////////////
XT_TYPE(Vehicle::XtHeadlight,				"VEH_Headlight")
	XT_FIELD(m_ZOffset,							"zOffset")
	XT_FIELD(m_Width,							"width")
	XT_FIELD(m_Length,							"length")
	XT_FIELD(m_Height,							"height")
	XT_FIELD(m_Intensity,						"intensity")
	XT_FIELD(m_Dip,								"dip")
	XT_FIELD(m_Colour,							"colour")
XT_END_TYPE

XT_TYPE(Vehicle::XtLightFlares,					"VEH_LightFlares")
	XT_FIELD(m_HeadLight,						"headLight")
	XT_FIELD(m_ShowRoomHeadLight,				"showRoomHeadLight")
	XT_FIELD(m_RearLight,						"rearLight")
	XT_FIELD(m_FogLight,						"fogLight")
	XT_FIELD(m_BrakeLight,						"brakeLight")
	XT_FIELD(m_CentreBrakeLight,				"centreBrakeLight")
	XT_FIELD(m_ReverseLight,					"reverseLight")
	XT_FIELD(m_PlateLight,						"plateLight")
	XT_FIELD(m_DashBoardLight,					"dashBoardLight")
XT_END_TYPE

XT_TYPE(Vehicle::XtDynamicLight,		"VEH_DynamicLight")
	XT_FIELD(m_Enable,						"enable")
	XT_FIELD(m_Colour,						"colour")
	XT_FIELD(m_Radius,						"radius")
	XT_FIELD(m_Offset,						"offset")
XT_END_TYPE

XT_TYPE(Vehicle::XtDynamicLights,			"VEH_DynamicLights")
	XT_FIELD(m_DashBoardDynamicLight,			"dashBoardLight")
	XT_FIELD(m_HeadDynamicLight,				"headLight")
	XT_FIELD(m_RearDynamicLight,				"rearLight")
	XT_FIELD(m_FogDynamicLight,					"fogLight")
	XT_FIELD(m_BrakeDynamicLight,				"brakeLight")
	XT_FIELD(m_ReverseDynamicLight,				"reverseLight")
XT_END_TYPE

////////////////////////////////////////////////////////////////////////
//						End: Effects
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//						Begin: Parts
////////////////////////////////////////////////////////////////////////

XT_TYPE(Vehicle::XtPart,						"VEH_Part")
	XT_FIELD(m_PartCode,								"partCode")
	XT_FIELD(m_PartSubCode,								"subCode")
XT_END_TYPE

XT_TYPE(Vehicle::XtWindow,				"VEH_Window")
	XT_FIELD(m_Part,									"part")
	XT_FIELD(m_Speed,									"speed")
	XT_FIELD(m_FinalRotation,							"rotation")
	XT_FIELD(m_FinalPosition,							"position")
XT_END_TYPE

XT_TYPE(Vehicle::E_PartCode,					"VEH_PartCode")
	XT_OPTION(Vehicle::E_PartCode_None,					"None")
	XT_OPTION(Vehicle::E_PartCode_Wheel,				"Wheel")
	XT_OPTION(Vehicle::E_PartCode_Tyre,					"Tyre")
	XT_OPTION(Vehicle::E_PartCode_AlloyRim,				"AlloyRim")	
	XT_OPTION(Vehicle::E_PartCode_Alloy,				"Alloy")
	XT_OPTION(Vehicle::E_PartCode_AlloyBlur,			"AlloyBlur")
	XT_OPTION(Vehicle::E_PartCode_BrakeDisc,			"BrakeDisc")
	XT_OPTION(Vehicle::E_PartCode_BrakeCaliper,			"BrakeCaliper")
	XT_OPTION(Vehicle::E_PartCode_WheelLod,				"WheelLod")
	XT_OPTION(Vehicle::E_PartCode_Bonnet,				"Bonnet")
	XT_OPTION(Vehicle::E_PartCode_Boot,					"Boot")
	XT_OPTION(Vehicle::E_PartCode_Door,					"Door")
	XT_OPTION(Vehicle::E_PartCode_Wing,					"Wing")
	XT_OPTION(Vehicle::E_PartCode_Bumper,				"Bumper")
	XT_OPTION(Vehicle::E_PartCode_Mirror,				"Mirror")
	XT_OPTION(Vehicle::E_PartCode_Exhaust,				"Exhaust")
	XT_OPTION(Vehicle::E_PartCode_LicensePlate,			"LicensePlate")
	XT_OPTION(Vehicle::E_PartCode_SteeringWheel,		"SteeringWheel")
	XT_OPTION(Vehicle::E_PartCode_HandBrake,			"HandBrake")
	XT_OPTION(Vehicle::E_PartCode_GearStick,			"GearStick")
	XT_OPTION(Vehicle::E_PartCode_PaddleShift,			"PaddleShift")
	XT_OPTION(Vehicle::E_PartCode_SignalStalk,			"SignalStalk")
	XT_OPTION(Vehicle::E_PartCode_Window,				"Window")
	XT_OPTION(Vehicle::E_PartCode_WindowInterior,		"WindowInterior")
	XT_OPTION(Vehicle::E_PartCode_Wiper,				"Wiper")
	XT_OPTION(Vehicle::E_PartCode_LightCluster,			"LightCluster")
	XT_OPTION(Vehicle::E_PartCode_HeadLight,			"HeadLight")
	XT_OPTION(Vehicle::E_PartCode_RearLight,			"RearLight")
	XT_OPTION(Vehicle::E_PartCode_FogLight,				"FogLight")
	XT_OPTION(Vehicle::E_PartCode_BrakeLight,			"BrakeLight")
	XT_OPTION(Vehicle::E_PartCode_ReverseLight,			"ReverseLight")
	XT_OPTION(Vehicle::E_PartCode_RunningLight,			"RunningLight")
	XT_OPTION(Vehicle::E_PartCode_SignalLight,			"SignalLight")
	XT_OPTION(Vehicle::E_PartCode_RearAndBrakeLight,	"RearAndBrakeLight")
	XT_OPTION(Vehicle::E_PartCode_HeadlightCover,		"HeadLightCover")
	XT_OPTION(Vehicle::E_PartCode_FogLightCover,		"FogLightCover")
	XT_OPTION(Vehicle::E_PartCode_RearLightCover,		"RearLightCover")
XT_END_TYPE

XT_TYPE(Vehicle::E_PartSubCode,					"VEH_PartSubCode")
	XT_OPTION(Vehicle::E_PartSubCode_None,				"None")
	XT_OPTION(Vehicle::E_PartSubCode_Left,				"Left")
	XT_OPTION(Vehicle::E_PartSubCode_Right,				"Right")
	XT_OPTION(Vehicle::E_PartSubCode_Centre,			"Centre")
	XT_OPTION(Vehicle::E_PartSubCode_Front,				"Front")
	XT_OPTION(Vehicle::E_PartSubCode_FrontLeft,			"FrontLeft")
	XT_OPTION(Vehicle::E_PartSubCode_FrontRight,		"FrontRight")
	XT_OPTION(Vehicle::E_PartSubCode_Back,				"Back")
	XT_OPTION(Vehicle::E_PartSubCode_BackLeft,			"BackLeft")
	XT_OPTION(Vehicle::E_PartSubCode_BackRight,			"BackRight")
	XT_OPTION(Vehicle::E_PartSubCode_Top,				"Top")
	XT_OPTION(Vehicle::E_PartSubCode_TopLeft,			"TopLeft")
	XT_OPTION(Vehicle::E_PartSubCode_TopRight,			"TopRight")
	XT_OPTION(Vehicle::E_PartSubCode_Bottom,			"Bottom")
	XT_OPTION(Vehicle::E_PartSubCode_BottomLeft,		"BottomLeft")
	XT_OPTION(Vehicle::E_PartSubCode_BottomRight,		"BottomRight")
XT_END_TYPE

////////////////////////////////////////////////////////////////////////
//						End: Parts
////////////////////////////////////////////////////////////////////////

XT_LIBRARY(Vehicle)
	XT_REGISTER_TYPE(Vehicle::E_ConstraintType)
	XT_REGISTER_TYPE(Vehicle::XtVehicle)
	XT_REGISTER_TYPE(Vehicle::XtConstraintBase)
	XT_REGISTER_TYPE(Vehicle::XtFixedJoint)
	XT_REGISTER_TYPE(Vehicle::XtSphericalJoint)
	XT_REGISTER_TYPE(Vehicle::XtRevoluteJoint)
	XT_REGISTER_TYPE(Vehicle::XtRevoluteJointLimit)
	XT_REGISTER_TYPE(Vehicle::XtPrismaticJoint)
	XT_REGISTER_TYPE(Vehicle::XtPrismaticJointLimit)
	XT_REGISTER_TYPE(Vehicle::XtDamageTransfer)
	XT_REGISTER_TYPE(Vehicle::XtDamageBehaviour)
	XT_REGISTER_TYPE(Vehicle::XtDamagedPart)
	XT_REGISTER_TYPE(Vehicle::XtWindow)
	XT_REGISTER_TYPE(Vehicle::XtPart)
	XT_REGISTER_TYPE(Vehicle::XtConstraints)	
	XT_REGISTER_TYPE(Vehicle::XtDamageCurveSetup)
	XT_REGISTER_TYPE(Vehicle::XtDamageSetup)
	XT_REGISTER_TYPE(Vehicle::XtDeformationSetup)	
	XT_REGISTER_TYPE(Vehicle::XtWheelDamageSetup)
	XT_REGISTER_TYPE(Vehicle::E_PartCode)
	XT_REGISTER_TYPE(Vehicle::E_PartSubCode)
	XT_REGISTER_TYPE(Vehicle::E_MaterialType)
	XT_REGISTER_TYPE(Vehicle::XtMaterialBase)
	XT_REGISTER_TYPE(Vehicle::XtMaterialPaint)
	XT_REGISTER_TYPE(Vehicle::XtMaterialGlass)
	XT_REGISTER_TYPE(Vehicle::XtMaterialTyre)
	XT_REGISTER_TYPE(Vehicle::XtMaterialAlloy)
	XT_REGISTER_TYPE(Vehicle::XtMaterialLight)
	XT_REGISTER_TYPE(Vehicle::XtMaterialInstance)	
	XT_REGISTER_TYPE(Vehicle::XtMaterialDefaults)
	XT_REGISTER_TYPE(Vehicle::XtHermiteKeyFrame)
	XT_REGISTER_TYPE(Vehicle::XtCurve)
	XT_REGISTER_TYPE(Vehicle::XtPaintJob)	
	XT_REGISTER_TYPE(Vehicle::XtHeadlight)	
	XT_REGISTER_TYPE(Vehicle::XtLightFlares)	
	XT_REGISTER_TYPE(Vehicle::XtDynamicLight)
	XT_REGISTER_TYPE(Vehicle::XtDynamicLights)
XT_END_LIBRARY
