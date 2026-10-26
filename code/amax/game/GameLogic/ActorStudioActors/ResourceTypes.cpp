#include "stdafx.h"



#include <GameLogic/ActorStudioActors/ResourceTypes.h>

#include <Dynamic/Particle/Generic/XtInterface.h>
#include <Dynamic/Particle/Event.h>
#include <Dynamic/Particle/EventInstance.h>

//========================  GameModel definition (Sorcery)  ========================
//							Matches Schema\ResourceTypes.txt


XT_TYPE(Game::ModelAnimation,	"ModelAnimation")
	XT_FIELD(m_Filename,		"Filename")
XT_END_TYPE


XT_TYPE(Game::ReplayRecord,		"ReplayRecord")
	XT_FIELD(m_Filename,		"Filename")
XT_END_TYPE




//========================  Dynamic Object definition  ========================
//						Matches Schema\DynamicObjectDef.txt


XT_TYPE(Game::E_PHYSICS_BEHAVIOUR,					"PhysicsBehaviour")
	XT_OPTION(Game::E_PHYSICS_BEHAVIOUR_NONE,		"None")
	XT_OPTION(Game::E_PHYSICS_BEHAVIOUR_FIXED,		"Fixed")
	XT_OPTION(Game::E_PHYSICS_BEHAVIOUR_DYNAMIC,	"Dynamic")
	XT_OPTION(Game::E_PHYSICS_BEHAVIOUR_KINEMATIC,	"Kinematic")
	XT_OPTION(Game::E_PHYSICS_BEHAVIOUR_POST,		"Post")
	XT_OPTION(Game::E_PHYSICS_BEHAVIOUR_VEHICLE,	"Vehicle")
XT_END_TYPE

XT_TYPE(Game::E_ALIGNMENT_TYPE,						"AlignmentType")
	XT_OPTION(Game::E_ALIGNMENT_TYPE_NONE,			"None")
	XT_OPTION(Game::E_ALIGNMENT_TYPE_VERTICAL,		"Vertical")
	XT_OPTION(Game::E_ALIGNMENT_TYPE_LEFTRIGHT,		"LeftRight")
	XT_OPTION(Game::E_ALIGNMENT_TYPE_FORWARDBACK,	"ForwardBack")
XT_END_TYPE


XT_TYPE(Game::E_ANIM_EVENT_TYPE,						"AnimEventType")
	XT_OPTION(Game::E_ANIM_EVENT_TYPE_ONCREATION,		"OnCreation")
	XT_OPTION(Game::E_ANIM_EVENT_TYPE_ONDELAYEDSTART,	"OnDelayedStart")
	XT_OPTION(Game::E_ANIM_EVENT_TYPE_ONCOLLISION,		"OnCollision")
	XT_OPTION(Game::E_ANIM_EVENT_TYPE_ONEVENT1,			"OnEvent1")
	XT_OPTION(Game::E_ANIM_EVENT_TYPE_ONEVENT2,			"OnEvent2")
	XT_OPTION(Game::E_ANIM_EVENT_TYPE_ONEVENT3,			"OnEvent3")
XT_END_TYPE

XT_TYPE(Game::E_AUDIO_MASS_TYPE,					"AudioMassType")
	XT_OPTION(Game::E_AUDIO_MASS_TYPE_NONE,			"None")
	XT_OPTION(Game::E_AUDIO_MASS_TYPE_LIGHT,		"Light")
	XT_OPTION(Game::E_AUDIO_MASS_TYPE_MEDIUM,		"Medium")
	XT_OPTION(Game::E_AUDIO_MASS_TYPE_HEAVY,		"Heavy")
	XT_OPTION(Game::E_AUDIO_MASS_TYPE_VERY_HEAVY,	"VeryHeavy")
	XT_OPTION(Game::E_AUDIO_MASS_TYPE_IMMOVABLE,	"Immovable")
XT_END_TYPE


XT_TYPE(Game::DynamicObjectDef,			"DynamicObjDef")
	XT_FIELD(m_ModelType,				"Model")
	XT_FIELD(m_DisplayShadow,			"DisplayShadow")
	XT_FIELD(m_ShadowIntensity,			"ShadowIntensity")

	XT_FIELD(m_DrawDistance,			"DrawDistance")
	XT_FIELD(m_DisplayInEnvironment,	"DisplayInEnvironment")
	XT_FIELD(m_DisplayInReflection,		"DisplayInReflection")
	XT_FIELD(m_DisplayInMirror,			"DisplayInMirror")
	XT_FIELD(m_Bouncy,					"Bouncy")

	XT_FIELD(m_PhysType,				"PhysType")
	XT_FIELD(m_Weight,					"Weight")
	XT_FIELD(m_Breakaway,				"BreakawayForce")
	XT_FIELD(m_SleepThreshold,			"SleepThreshold")
	XT_FIELD(m_LinearDamping,			"LinearDamping")
	XT_FIELD(m_AngularDamping,			"AngularDamping")
	XT_FIELD(m_RangeOfMovement,			"RangeOfMovement")
	XT_FIELD(m_LifespanAfterDamage,		"LifespanAfterDamage")
	XT_FIELD(m_RespawnTimer,			"RespawnTimer")
	XT_FIELD(m_DynamicObjectAudioDef,	"DynamicObjAudioEntity")
	XT_FIELD(m_AudioMassType,			"AudioMassLevel")

	XT_FIELD(m_Alignment,				"Alignment")

	XT_FIELD(m_VehicleDamage,			"VehicleDamage")

	XT_FIELD(m_DebrisType,				"DebrisType")

	XT_FIELD(m_SwapElementsOnCollision,	"SwapElementsOnCollision")
	XT_FIELD(m_AnimEvents,				"AnimationEvents")
XT_END_TYPE

XT_TYPE(Game::DynamicObjectDefGroup,	"DynamicObjDefGroup")
	XT_FIELD(ObjDefList,				"ObjectDefs")
XT_END_TYPE

XT_TYPE(Game::PhysicalObjectDefGroup,	"PhysicalObjDefGroup")
	XT_FIELD(ObjDefList,				"ObjectDefs")
XT_END_TYPE

XT_TYPE(Game::AnimationEvent,			"AnimationEvent")
	XT_FIELD(m_EventType,				"EventType")
	XT_FIELD(m_StopAnimation,			"StopAnimation")
	XT_FIELD(m_Animation,				"Animation")
//	XT_FIELD(m_DamagedVersion,			"ChangeToDamagedVersion")
XT_END_TYPE

XT_TYPE(Game::GlobalTime,				"GlobalTime")
	XT_FIELD(m_Time,					"Time")
XT_END_TYPE


//========================  Dynamic Camera definitions  ========================
//						Matches Schema\DynamicCameraDef.txt



XT_TYPE(Game::E_DYCAM_MOVEMENT,								"DyCamMovementType")
	XT_OPTION(Game::E_DYCAM_MOVEMENT_NONE,					"None")
	XT_OPTION(Game::E_DYCAM_MOVEMENT_ATTACHED,				"Attached")
	XT_OPTION(Game::E_DYCAM_MOVEMENT_FLYCAM,				"FlyCam")
	XT_OPTION(Game::E_DYCAM_MOVEMENT_SPLINEFOLLOWER,		"SplineFollower")
	XT_OPTION(Game::E_DYCAM_MOVEMENT_LOCALSPLINEFOLLOWER,	"LocalSplineFollower")
	XT_OPTION(Game::E_DYCAM_MOVEMENT_VEHICLEFEED,			"VehicleFeed")
	XT_OPTION(Game::E_DYCAM_MOVEMENT_LASTCAMERA,			"LastCamera")
XT_END_TYPE

XT_TYPE(Game::E_DYCAM_TARGET,								"DyCamTargetType")
	XT_OPTION(Game::E_DYCAM_TARGET_NONE,					"None")
	XT_OPTION(Game::E_DYCAM_TARGET_SIMPLE,					"Simple")
	XT_OPTION(Game::E_DYCAM_TARGET_SPLINEFOLLOWER,			"SplineFollower")
	XT_OPTION(Game::E_DYCAM_TARGET_LOCALSPLINEFOLLOWER,		"LocalSplineFollower")
XT_END_TYPE

XT_TYPE(Game::E_BLUR_TYPE,									"DyCamBlurType")
	XT_OPTION(Game::E_BLUR_TYPE_MOTIONBLUR,					"MotionBlur")
	XT_OPTION(Game::E_BLUR_TYPE_DEPTHOFFIELD,				"DepthOfField")
XT_END_TYPE


//=============================================================================

XT_TYPE(Game::DyCamPropertyDef,					"DyCamPropertyDef")
	XT_FIELD(m_FOV,								"FOV")

	XT_FIELD(m_BlurType,						"BlurType")

	XT_FIELD(m_DepthOfField,					"DepthOfField")
	XT_FIELD(m_Aperture,						"Aperture")

	XT_FIELD(m_Blur,							"Blur")

	XT_FIELD(m_DOFNearBlurBegin,				"DOFNearBlurBegin")
	XT_FIELD(m_DOFNearBlurEnd,					"DOFNearBlurEnd")
	XT_FIELD(m_DOFFarBlurBegin,					"DOFFarBlurBegin")
	XT_FIELD(m_DOFFarBlurEnd,					"DOFFarBlurEnd")
	XT_FIELD(m_DOFFarBlurScale,					"DOFFarBlurScale")

	XT_FIELD(m_Colour,							"Colour")
	XT_FIELD(m_Grain,							"Grain")
	XT_FIELD(m_Vignette,						"Vignette")
	XT_FIELD(m_Tunnel,							"Tunnel")

	XT_FIELD(m_RedSaturation,					"RedSaturation")
	XT_FIELD(m_GreenSaturation,					"GreenSaturation")
	XT_FIELD(m_BlueSaturation,					"BlueSaturation")
XT_END_TYPE


XT_TYPE(Game::DyCamMovementDef,					"DyCamMovementDef")
	XT_FIELD(m_MovementType,					"MovementType")

	XT_FIELD(m_Shake,							"Shake")
	XT_FIELD(m_Lag,								"Lag")
	XT_FIELD(m_TurnTilt,						"TurnTilt")
	XT_FIELD(m_DynPitch,						"DynPitch")
	XT_FIELD(m_DoNotLoop,						"DoNotLoop")
XT_END_TYPE

XT_TYPE(Game::DyCamTargetDef,					"DyCamTargetDef")
	XT_FIELD(m_TargetType,						"TargetType")

	XT_FIELD(m_Shake,							"Shake")
	XT_FIELD(m_ProximityShake,					"ProximityShake")
	XT_FIELD(m_Lag,								"Lag")
	XT_FIELD(m_DoNotLoop,						"DoNotLoop")
	XT_FIELD(m_CheckLineOfSight,				"CheckLineOfSight")
XT_END_TYPE

XT_TYPE(Game::DyCamShakeDef,					"DyCamShakeDef")
	XT_FIELD(m_X,								"X")
	XT_FIELD(m_Y,								"Y")
	XT_FIELD(m_Z,								"Z")
	XT_FIELD(m_Freq,							"Freq")
XT_END_TYPE

//=============================================================================

XT_TYPE(Game::RightStickCameraParams,			"RightStickCameraParams")
	XT_FIELD(m_PivotDistance,					"PivotDistance")
	XT_FIELD(m_DollyDistance,					"DollyDistance")
	XT_FIELD(m_MaxHorizontalAngle,				"MaxHorizontalAngle")
	XT_FIELD(m_MaxVerticalAngle,				"MaxVerticalAngle")
	XT_FIELD(m_BlendValue,						"BlendValue")
XT_END_TYPE

//=============================================================================


XT_TYPE(Game::CutSceneLightDef,					"CutSceneLightDef")
	XT_FIELD(m_MasterHeading,					"MasterHeading")

	XT_FIELD(m_Colour0,							"Colour0")
	XT_FIELD(m_Wrap0,							"Wrap0")
	XT_FIELD(m_Heading0,						"Heading0")
	XT_FIELD(m_Elevation0,						"Elevation0")

	XT_FIELD(m_Colour1,							"Colour1")
	XT_FIELD(m_Wrap1,							"Wrap1")
	XT_FIELD(m_Heading1,						"Heading1")
	XT_FIELD(m_Elevation1,						"Elevation1")

	XT_FIELD(m_Colour2,							"Colour2")
	XT_FIELD(m_Wrap2,							"Wrap2")
	XT_FIELD(m_Heading2,						"Heading2")
	XT_FIELD(m_Elevation2,						"Elevation2")

	XT_FIELD(m_Colour3,							"Colour3")
	XT_FIELD(m_Wrap3,							"Wrap3")
	XT_FIELD(m_Heading3,						"Heading3")
	XT_FIELD(m_Elevation3,						"Elevation3")
XT_END_TYPE




//========================  Physical Object Definitions  ========================
//						Matches Schema\Actors\??.txt

XT_TYPE(Game::C_ParticleEventSpawn, "ParticleEventSpawn")
	XT_FIELD(m_Position, "position")
	XT_FIELD(m_Direction, "direction")
	XT_FIELD(m_Intensity, "intensity")
	XT_FIELD(m_LeaveAtSpawn, "leaveAtSpawn")
	XT_FIELD(m_SpawnAtHitPosition, "spawnAtHitPosition")
	XT_FIELD(m_DontUseCarVelocity, "dontUseCarVelocity")	
	XT_FIELD(m_ParticleEvent, "particleEvent")
XT_END_TYPE 

XT_TYPE(Game::C_PhysicalObjectElementDamageLevelDef, "PhysicalObjectElementDamageLevel")
	XT_BASE(BasePhysicalObject::C_ElementDamageLevelDef)
XT_END_TYPE

 
XT_TYPE(Game::C_PhysicalObjectElementDef, "PhysicalObjectElement")
	XT_BASE(BasePhysicalObject::C_ElementDef)
XT_END_TYPE

XT_TYPE(Game::C_PhysicalObjectDef,			"PhysicalObject")
	XT_BASE(BasePhysicalObject::C_Def)
	XT_FIELD(m_AudioEntity,						"AudioEntity")
	XT_FIELD(m_BreakParticleEvents,				"BreakParticleEvents")	
	XT_FIELD(m_LifeTimeAfterDamage,				"LifeTimeAfterDamage")	
	XT_FIELD(m_DisplayInEnvironment,			"DisplayInEnvironment")
	XT_FIELD(m_DisplayInReflection,				"DisplayInReflection")
	XT_FIELD(m_DisplayInMirror,					"DisplayInMirror")
	XT_FIELD(m_Bouncy,							"Bouncy")
	XT_FIELD(m_UseStaticActivate,				"UseStaticActivate")
	XT_FIELD(m_AudioMassType,					"AudioMassLevel")
XT_END_TYPE 

//========================  Dynamic Object Misc Info  ========================
//=============================================================================



XT_TYPE(Game::E_TRIGGER_TYPE,						"TriggerTypes")
	XT_OPTION(Game::E_TRIGGER_TYPE_NORMAL,			"Normal")
	XT_OPTION(Game::E_TRIGGER_TYPE_GAMELOGIC,		"GameLogic")
	XT_OPTION(Game::E_TRIGGER_TYPE_RACERULES,		"RaceRules")
	XT_OPTION(Game::E_TRIGGER_TYPE_HANDOFGOD_ENTRY,	"HandOfGodEntry")
	XT_OPTION(Game::E_TRIGGER_TYPE_HANDOFGOD_EXIT,	"HandOfGodExit")
	XT_OPTION(Game::E_TRIGGER_TYPE_HANDOFGOD_TWOWAY,"HandOfGodTwoWay")
	XT_OPTION(Game::E_TRIGGER_TYPE_HANDOFGOD_CAMERA,"HandOfGodCamera")
	XT_OPTION(Game::E_TRIGGER_TYPE_ICONLEFT,		"IconLeft")
	XT_OPTION(Game::E_TRIGGER_TYPE_ICONRIGHT,		"IconRight")
	XT_OPTION(Game::E_TRIGGER_TYPE_ICONFORK,		"IconFork")
	XT_OPTION(Game::E_TRIGGER_TYPE_TUNNEL_ENTER,	"TunnelEnter")
	XT_OPTION(Game::E_TRIGGER_TYPE_TUNNEL_EXIT,		"TunnelExit")
XT_END_TYPE



//========================  Library (Sorcery)  ========================


XT_LIBRARY(AmaxResourceTypes)									// Include ref to all of above
	XT_REGISTER_TYPE(Game::ModelAnimation)
	XT_REGISTER_TYPE(Game::ReplayRecord)

	XT_REGISTER_TYPE(Game::E_PHYSICS_BEHAVIOUR)
	XT_REGISTER_TYPE(Game::E_ALIGNMENT_TYPE)
	XT_REGISTER_TYPE(Game::E_ANIM_EVENT_TYPE)
	XT_REGISTER_TYPE(Game::E_TRIGGER_TYPE)
	XT_REGISTER_TYPE(Game::DynamicObjectDef)
	XT_REGISTER_TYPE(Game::DynamicObjectDefGroup)
	XT_REGISTER_TYPE(Game::PhysicalObjectDefGroup)

	XT_REGISTER_TYPE(Game::AnimationEvent)

	XT_REGISTER_TYPE(Game::GlobalTime)

	XT_REGISTER_TYPE(Game::E_DYCAM_MOVEMENT)
	XT_REGISTER_TYPE(Game::E_DYCAM_TARGET)
	XT_REGISTER_TYPE(Game::E_BLUR_TYPE)

	XT_REGISTER_TYPE(Game::DyCamPropertyDef)
	XT_REGISTER_TYPE(Game::DyCamMovementDef)
	XT_REGISTER_TYPE(Game::DyCamTargetDef)
	XT_REGISTER_TYPE(Game::DyCamShakeDef)

	XT_REGISTER_TYPE(Game::RightStickCameraParams)

	XT_REGISTER_TYPE(Game::CutSceneLightDef)

	XT_REGISTER_TYPE(Game::C_ParticleEventSpawn)
	XT_REGISTER_TYPE(Game::C_PhysicalObjectElementDamageLevelDef)
	XT_REGISTER_TYPE(Game::C_PhysicalObjectElementDef)
	XT_REGISTER_TYPE(Game::C_PhysicalObjectDef)
	XT_REGISTER_TYPE(Game::E_AUDIO_MASS_TYPE)
XT_END_LIBRARY



