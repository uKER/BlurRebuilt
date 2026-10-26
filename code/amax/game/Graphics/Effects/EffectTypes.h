/////////////////////////////////////////////////////////////
// VehicleWheel
/////////////////////////////////////////////////////////////

#ifndef __EFFECT_TYPES_H__
#define __EFFECT_TYPES_H__

namespace Game
{

namespace Effects
{


/* 
===============================================================
					enum E_SurfaceParticleEventTrigger

NOTES: 
===============================================================
*/
enum E_SurfaceParticleEventTrigger
{
	E_SurfaceParticleEventTrigger_Speed,
	E_SurfaceParticleEventTrigger_SlideFactor,
	E_SurfaceParticleEventTrigger_WheelSpin,
	E_SurfaceParticleEventTrigger_Contact
};

/* 
===============================================================
					enum E_SurfaceModifier

NOTES: 
===============================================================
*/
enum E_SurfaceModifier
{
	E_SurfaceModifier_Dry,
	E_SurfaceModifier_Wet,
	E_SurfaceModifier_VeryWet,
	E_SurfaceModifier_ShallowPuddle,
	E_SurfaceModifier_DeepPuddle,
	E_SurfaceModifier_Ice,
	E_SurfaceModifier_Snow,

	E_SurfaceModifier_Max
};

/* 
===============================================================
					enum E_ExhuastEffectType

NOTES: 
===============================================================
*/
enum E_ExhuastEffectType
{
	E_ExhuastEffectType_Unknown = -1,

	E_ExhuastEffectType_ExhaustPop,
	E_ExhuastEffectType_ExhaustBang,	
	E_ExhuastEffectType_ExhaustPopLeft,
	E_ExhuastEffectType_ExhaustPopRight,
	E_ExhuastEffectType_ExhaustBangLeft,	
	E_ExhuastEffectType_ExhaustBangRight,
	
	E_ExhuastEffectType_Max,
};

/* 
===============================================================
					enum E_ExhuastEffectPosition

NOTES: 
===============================================================
*/
enum E_ExhuastEffectPosition
{
	E_ExhuastEffectPosition_Left1,
	E_ExhuastEffectPosition_Left2,
	E_ExhuastEffectPosition_Right1,
	E_ExhuastEffectPosition_Right2,

	E_ExhuastEffectPosition_Max,
};

enum E_ParticleEmitterType
{
	E_ParticleEmitterType_RadiatorSmoke,
	E_ParticleEmitterType_EngineSmoke,
	E_ParticleEmitterType_EngineFire,

	E_ParticleEmitterType_Max,
};

/* 
===============================================================
					enum E_ExhuastEffectPosition

NOTES: 
===============================================================
*/
enum E_MiscParticleEventType
{
	E_MiscParticleEventType_ExhaustSmoke,
	
	E_MiscParticleEventType_ExhaustBang,
	E_MiscParticleEventType_ExhaustPop,

	E_MiscParticleEventType_BrakeLightTrail,
	E_MiscParticleEventType_RearLightTrail,

	E_MiscParticleEventType_WheelSparks,

	E_MiscParticleEventType_BurstTyre,

	E_MiscParticleEventType_EngineSmoke,
	E_MiscParticleEventType_EngineFire,

	E_MiscParticleEventType_GlassCrack,
	E_MiscParticleEventType_GlassSmash,

	E_MiscParticleEventType_ScrapeStatic,
	E_MiscParticleEventType_ScrapeDynamic,
	
	E_MiscParticleEventType_LandingSparks,
	E_MiscParticleEventType_LandingDust,
	E_MiscParticleEventType_WaterBigSplash,
	E_MiscParticleEventType_WaterBobbing,
	
	E_MiscParticleEventType_LicensePlate,
	
	E_MiscParticleEventType_Max,
};

} //namespace Effects

} //namespace Game

#endif
