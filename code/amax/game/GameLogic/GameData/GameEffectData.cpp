#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/GameData/GameEffectData.h>
#include <Dynamic/Track/XtInterface.h>
#include <Dynamic/Common.h>

C_GameEffectData C_GameEffectData::s_instance;

//Debug::Bool g_debugCheckForSorceryUpdatesOnDisk("Game.CheckForSorceryDataExportsWhileRunning", false, Debug::Persist());

#define AMAX_GAMEEFFECTDATA_CALLBACK(Type, Member) \
	class Callback##Type { public: AmaxGameData<Type*>* operator()() { return &C_GameEffectData::Instance().Member; } };

//Custom callback function for wheel/surface interaction particle effects
void AmaxWheelSurfaceEffectsHandler(XtEventType event, XtRecord* record, AmaxWheelSurfaceEffects* object, XtCallbackContext*)
{
	switch(event)
	{
		case kXtEtConstruct:
			C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceEffects.Add(object);
		break;

		case kXtEtDestruct:
			C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceEffects.Destroy(object);
		break;
	}

	//Set up mapping from modifier id to the correct element of mAmaxVehicleWheelSurfaceEffects
	for(S32 i = 0; i < Game::Effects::E_SurfaceModifier_Max; ++i)
		C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceEffectsMapping[i] = -1;
	for(S32 i = 0; i < C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceEffects.Number(); ++i)
	{
		AmaxWheelSurfaceEffects* pSurfaceEffects = C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceEffects[i];
		if(pSurfaceEffects->modifier < 0 || pSurfaceEffects->modifier >= Game::Effects::E_SurfaceModifier_Max)
			return;
		
		C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceEffectsMapping[pSurfaceEffects->modifier] = i;
	}

	//Set up mapping from surface id to the correct element of AmaxWheelSurfaceEffect.effects
	for(S32 i = 0; i < Game::E_EffectMaterial_Count; ++i)
		object->effectMapping[i] = -1;	
	for(S32 i = 0; i < object->effects.count; ++i)
	{
		AmaxWheelSurfaceEffect& effect = object->effects[i];

		if(object->effectMapping[effect.surface] != -1)
		{}//LOGWARN( gGameLogicDataLogger, "VehicleWheelSurfaceEffect Modifer: %d Surface: %d duplicate entry\n", object->modifier, effect.surface);
			
		object->effectMapping[effect.surface] = i;
	}
}

void AmaxWheelParticleEventDescriptorHandler(AmaxWheelParticleEventDescriptor* object)
{
	object->node.SetPosition(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&object->position));

	Maths::HVec3 wheelDirection(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&object->direction));
	Maths::HF32 lengthSq = Maths::Length2(wheelDirection);
	if(lengthSq > Maths::Constants<Maths::HF32>::Zero())
	{
		Maths::HF32 length = Maths::Sqrt(lengthSq);
		wheelDirection = wheelDirection * length.Rcp();
		object->node.SetRotation(DynamicEffect::GetQuaternionFromTo<Maths::C_Quaternion>(Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ(), wheelDirection));
	}
	else
		object->node.SetRotation(MathsStorage::Float4A16(0.0f, 0.0f, 0.0f, 1.0f));
}

void AmaxWheelSurfaceParticleEventHandler(XtEventType event, XtRecord* record, AmaxWheelSurfaceParticleEvent* object, XtCallbackContext*)
{
	if(event == kXtEtConstruct)
	{
		object->id = record->id;

		object->intensityCurve.Clear();
		for(S32 i = 0; i < object->intensity.count && i < 3; i++)
		{
			DynamicEffect::XtInterface::HermiteKeyFrame& keyFrame = object->intensity[i];
			F32 timeValue = keyFrame.time;
			if (i == 0)
				timeValue = Maths::Max(timeValue, 0.01f);
			object->intensityCurve.AddKeyFrame(timeValue , keyFrame.value, keyFrame.tangentIn, keyFrame.tangentOut);
		}

		if(object->intensityRate != 0.0f)
		{
			object->intensityRamp = true;
			object->intensityStep = (1.0f/30.0f) / object->intensityRate;
		}

		object->alphaCurve.Clear();
		for(S32 i = 0; i < object->alpha.count && i < 3; i++)
		{
			DynamicEffect::XtInterface::HermiteKeyFrame& keyFrame = object->alpha[i];
			object->alphaCurve.AddKeyFrame(keyFrame.time, keyFrame.value, keyFrame.tangentIn, keyFrame.tangentOut);
		}
		if(object->alphaRate != 0.0f)
		{
			object->alphaRamp = true;
			object->alphaStep = (1.0f/30.0f) / object->alphaRate;
		}

		AmaxWheelParticleEventDescriptorHandler(&object->carFL);
		AmaxWheelParticleEventDescriptorHandler(&object->carFR);
		AmaxWheelParticleEventDescriptorHandler(&object->carRL);
		AmaxWheelParticleEventDescriptorHandler(&object->carRR);
	}
}

void AmaxWheelSurfaceSplashEventHandler(XtEventType event, XtRecord* record, AmaxWheelSurfaceSplashEvent* object, XtCallbackContext*)
{
	if(event == kXtEtConstruct)
	{
		object->splashCameraFactorCurve.Clear();
		for(S32 i = 0; i < object->splashCameraFactor.count; i++)
		{
			DynamicEffect::XtInterface::HermiteKeyFrame& keyFrame = object->splashCameraFactor[i];
			object->splashCameraFactorCurve.AddKeyFrame(keyFrame.time, keyFrame.value, keyFrame.tangentIn, keyFrame.tangentOut);
		}
		object->splashCameraCurrentCount = 0.f;
	}
}

//Custom callback function for wheel/surface interaction particle effects
void AmaxWheelParticleEventDescriptorHandler(XtEventType event, XtRecord* record, AmaxWheelParticleEventDescriptor* object, XtCallbackContext*)
{
	if(event == kXtEtConstruct)
	{
		object->node.SetPosition(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&object->position));

		Maths::HVec3 wheelDirection(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&object->direction));
		Maths::HF32 lengthSq = Maths::Length2(wheelDirection);
		if(lengthSq > Maths::Constants<Maths::HF32>::Zero())
		{
			Maths::HF32 length = Maths::Sqrt(lengthSq);
			wheelDirection = wheelDirection * length.Rcp();
			object->node.SetRotation(DynamicEffect::GetQuaternionFromTo<Maths::C_Quaternion>(Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ(), wheelDirection));
		}
		else
			object->node.SetRotation(MathsStorage::Float4A16(0.0f, 0.0f, 0.0f, 1.0f));
	}
}


// tracks handler functions

void AmaxWheelSurfaceTracksHandler(XtEventType event, XtRecord* record, AmaxWheelSurfaceTracks* object, XtCallbackContext*)
{
	switch(event)
	{
		case kXtEtConstruct:
			C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceTracks.Add(object);
		break;

		case kXtEtDestruct:
			C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceTracks.Destroy(object);
		break;
	}

	// set up mapping from modifier id to the correct element of mAmaxWheelSurfaceTracksMapping
	for(S32 i = 0; i < Game::Effects::E_SurfaceModifier_Max; ++i)
		C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceTracksMapping[i] = -1;
	for(S32 i = 0; i < C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceTracks.Number(); ++i)
	{
		AmaxWheelSurfaceTracks* pSurfaceTracks = C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceTracks[i];
		if(pSurfaceTracks->modifier < 0 || pSurfaceTracks->modifier >= Game::Effects::E_SurfaceModifier_Max)
			return;
		
		C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceTracksMapping[pSurfaceTracks->modifier] = i;
	}

	// set up mapping from surface id to the correct element of PGRVehicleWheelSurfaceTrack.tracks
	for(S32 i = 0; i < Game::E_EffectMaterial_Count; ++i)
		object->trackMapping[i] = -1;	
	for(S32 i = 0; i < object->tracks.count; ++i)
	{
		AmaxWheelSurfaceTrack& track = object->tracks[i];

		if(object->trackMapping[track.surface] != -1)
		{}	//LOGWARN( gGameLogicDataLogger, "VehicleWheelSurfaceTrack Modifer: %d Surface: %d duplicate entry\n", object->modifier, track.surface);
			
		object->trackMapping[track.surface] = i;
	}
}

void AmaxWheelSurfaceTrackEventHandler(XtEventType event, XtRecord* record, AmaxWheelSurfaceTrackEvent* object, XtCallbackContext*)
{
	if(event == kXtEtConstruct)
	{
		object->id = record->id;

		object->alphaCurve.Clear();
		for(S32 j = 0; j < object->alpha.count && j < 3; ++j)
		{
			DynamicEffect::XtInterface::HermiteKeyFrame& keyFrame = object->alpha[j];
			object->alphaCurve.AddKeyFrame(keyFrame.time, keyFrame.value, keyFrame.tangentIn, keyFrame.tangentOut);
		}
		
		if(object->alphaRate != 0.0f)
		{
			object->alphaRamp = true;
			object->alphaStep = (1.0f/30.0f) / object->alphaRate;
		}
	}
}

// misc effects handler

void AmaxMiscEffectsHandler(XtEventType event, XtRecord* record, AmaxMiscEffects* object, XtCallbackContext*)
{
	switch(event)
	{
		case kXtEtConstruct:
			C_GameEffectData::Instance().mAmaxMiscEffects.Add(object);
		break;

		case kXtEtDestruct:
			C_GameEffectData::Instance().mAmaxMiscEffects.Destroy(object);
		break;
	}

	object->lightTrailRangeRcp = 1.0f / (object->lightTrailMaxSpeed - object->lightTrailMinSpeed);

	// set all the mappings to NULL
	for(S32 i = 0; i < Game::Effects::E_MiscParticleEventType_Max; ++i)
		object->effectMapping[i] = -1;

	// update the mappings
	for(S32 i = 0; i < object->effects.count; ++i)
	{
		AmaxMiscParticleEvent& effect = object->effects[i];

		if(object->effectMapping[effect.type] != -1)
		{}//	LOGWARN( gGameLogicDataLogger, "VehicleMiscEffect Effect: %d duplicate entry\n", effect.type);
		
		object->effectMapping[effect.type] = i;
	}
}

AMAX_GAMEEFFECTDATA_CALLBACK(AmaxImpactSurfaceEffect, mAmaxImpactSurfaceEffects)
XT_TYPE(AmaxImpactSurfaceEffect,		"AmaxImpactSurfaceEffect")
	XT_CALLBACK((GameDataHandler<AmaxImpactSurfaceEffect,CallbackAmaxImpactSurfaceEffect>))
	XT_FIELD(m_MinImpact,				"minImpact")
	XT_FIELD(m_MaxImpact,				"maxImpact")
	XT_FIELD(m_MinIntensity,			"minIntensity")
	XT_FIELD(m_MaxIntensity,			"maxIntensity")
	XT_FIELD(m_ParticleEvent,			"particleEvent")
	XT_FIELD(m_SurfaceTypes,			"surfaceTypes")
XT_END_TYPE

XT_TYPE(Game::E_EffectMaterial,				"EffectMaterial")
	XT_OPTION(Game::E_EffectMaterial_Default,				"Default")
	XT_OPTION(Game::E_EffectMaterial_Tarmac,				"Tarmac")
	XT_OPTION(Game::E_EffectMaterial_Concrete,				"Concrete")
	XT_OPTION(Game::E_EffectMaterial_Rock,					"Rock")
	XT_OPTION(Game::E_EffectMaterial_Dirt,					"Dirt")
	XT_OPTION(Game::E_EffectMaterial_Mud,					"Mud")
	XT_OPTION(Game::E_EffectMaterial_Grass,					"Grass")
	XT_OPTION(Game::E_EffectMaterial_Gravel,				"Gravel")
	XT_OPTION(Game::E_EffectMaterial_Sand,					"Sand")
	XT_OPTION(Game::E_EffectMaterial_WetSand,				"WetSand")
	XT_OPTION(Game::E_EffectMaterial_Water,					"Water")
	XT_OPTION(Game::E_EffectMaterial_Brick,					"Brick")
	XT_OPTION(Game::E_EffectMaterial_Vegetation,			"Vegetation")
	XT_OPTION(Game::E_EffectMaterial_Wood,					"Wood")
	XT_OPTION(Game::E_EffectMaterial_Metal,					"Metal")
	XT_OPTION(Game::E_EffectMaterial_Plastic,				"Plastic")
	XT_OPTION(Game::E_EffectMaterial_Glass,					"Glass")
	XT_OPTION(Game::E_EffectMaterial_Rubber,				"Rubber")
	XT_OPTION(Game::E_EffectMaterial_Upholstery,			"Upholstery")
	XT_OPTION(Game::E_EffectMaterial_Cardboard,				"Cardboard")
	XT_OPTION(Game::E_EffectMaterial_Flesh,					"Flesh")
	XT_OPTION(Game::E_EffectMaterial_TrainTracks,			"TrainTracks")
	XT_OPTION(Game::E_EffectMaterial_WaterDeep,				"WaterDeep")
	XT_OPTION(Game::E_EffectMaterial_ConcreteFenceChain,	"ConcreteFenceChain")
	XT_OPTION(Game::E_EffectMaterial_ConcreteFencePlastic,	"ConcreteFencePlastic")
	XT_OPTION(Game::E_EffectMaterial_MetalHollow,			"MetalHollow")
	XT_OPTION(Game::E_EffectMaterial_MetalArmco,			"MetalArmco")
	XT_OPTION(Game::E_EffectMaterial_MetalSheet,			"MetalSheet")
	XT_OPTION(Game::E_EffectMaterial_MetalFenceChain,		"MetalFenceChain")
	XT_OPTION(Game::E_EffectMaterial_MetalFenceSolid,		"MetalFenceSolid")
	XT_OPTION(Game::E_EffectMaterial_MetalFenceSlatted,		"MetalFenceSlatted")
	XT_OPTION(Game::E_EffectMaterial_MetalSolidPost,		"MetalSolidPost")
	XT_OPTION(Game::E_EffectMaterial_DirtLightRough,		"DirtLightRough")
	XT_OPTION(Game::E_EffectMaterial_DirtHeavyRough,		"DirtHeavyRough")
	XT_OPTION(Game::E_EffectMaterial_WackyMagic,			"WackyMagic")
	XT_OPTION(Game::E_EffectMaterial_PavingStone,			"PavingStone")
	XT_OPTION(Game::E_EffectMaterial_Vehicle,				"Vehicle")
XT_END_TYPE

XT_TYPE(Game::Effects::E_SurfaceModifier,				"SurfaceModifier")
	XT_OPTION(Game::Effects::E_SurfaceModifier_Dry,				"Dry")
	XT_OPTION(Game::Effects::E_SurfaceModifier_Wet,				"Wet")
	XT_OPTION(Game::Effects::E_SurfaceModifier_VeryWet,			"VeryWet")
	XT_OPTION(Game::Effects::E_SurfaceModifier_ShallowPuddle,	"ShallowPuddle")
	XT_OPTION(Game::Effects::E_SurfaceModifier_DeepPuddle,		"DeepPuddle")
	XT_OPTION(Game::Effects::E_SurfaceModifier_Ice,				"Ice")
	XT_OPTION(Game::Effects::E_SurfaceModifier_Snow,			"Snow")
XT_END_TYPE

// wheel surface particle effects

XT_TYPE(AmaxWheelParticleEventDescriptor, "WheelParticleEventDescriptor")
	XT_CALLBACK(AmaxWheelParticleEventDescriptorHandler)
	XT_FIELD(position,			"position")
	XT_FIELD(direction,			"direction")
	XT_FIELD(intensity,			"intensity")
	XT_FIELD(alpha,				"alpha")
XT_END_TYPE

XT_TYPE(Game::Effects::E_SurfaceParticleEventTrigger, "WheelSurfaceParticleEventTrigger")
	XT_OPTION(Game::Effects::E_SurfaceParticleEventTrigger_Speed, "Speed")
	XT_OPTION(Game::Effects::E_SurfaceParticleEventTrigger_SlideFactor, "SlideFactor")
	XT_OPTION(Game::Effects::E_SurfaceParticleEventTrigger_WheelSpin, "WheelSpin")
	XT_OPTION(Game::Effects::E_SurfaceParticleEventTrigger_Contact, "Contact")
XT_END_TYPE

XT_TYPE(AmaxWheelSurfaceEffects, "WheelSurfaceEffects")
	XT_CALLBACK(AmaxWheelSurfaceEffectsHandler)
	XT_FIELD(modifier, "modifier")
	XT_FIELD(effects,"effects")
XT_END_TYPE

XT_TYPE(AmaxWheelSurfaceEffect, "WheelSurfaceEffect")
	XT_FIELD(surface, "surface")
	XT_FIELD(events, "events")
	XT_FIELD(rimEvents, "rimEvents")
	XT_FIELD(splashEvents, "splashEvents")
	XT_FIELD(rimLightEnable, "rimLightEnable")
	XT_FIELD(rimLightColour, "rimLightColour")
	XT_FIELD(rimLightRadiusRange, "rimLightRadiusRange")
	XT_FIELD(rimLightFrequency, "rimLightFrequency")
XT_END_TYPE

XT_TYPE(AmaxWheelSurfaceParticleEvent, "WheelSurfaceParticleEvent")
	XT_CALLBACK(AmaxWheelSurfaceParticleEventHandler)
	XT_FIELD(particleEvent, "particleEvent")	
	XT_FIELD(trigger, "trigger")
	XT_FIELD(intensity, "intensity")
	XT_FIELD(intensityRate, "intensityRate")
	XT_FIELD(alpha, "alpha")
	XT_FIELD(alphaRate, "alphaRate")
	XT_FIELD(carFL, "carFL")
	XT_FIELD(carFR, "carFR")
	XT_FIELD(carRL, "carRL")
	XT_FIELD(carRR, "carRR")
XT_END_TYPE

XT_TYPE(AmaxWheelSurfaceSplashEvent, "WheelSurfaceSplashEvent")
	XT_CALLBACK(AmaxWheelSurfaceSplashEventHandler)
	XT_FIELD(trigger, "trigger")
	XT_FIELD(splashCameraType,"splashCameraType")
	XT_FIELD(splashCameraFactor, "splashCameraFactor")
	XT_FIELD(splashCameraCountPerSecond, "splashCameraCountPerSecond")
	XT_FIELD(splashCameraSpread, "splashCameraSpread")
XT_END_TYPE

// wheel surface tracks

XT_TYPE(DynamicEffect::Track::XtInterface::E_BlendMode, "WheelSurfaceTrackMaterialBlendMode")
	XT_OPTION(DynamicEffect::Track::XtInterface::E_BlendMode_Multiply, "Multiply")
	XT_OPTION(DynamicEffect::Track::XtInterface::E_BlendMode_Alpha, "Alpha")
XT_END_TYPE

XT_TYPE(DynamicEffect::Track::XtInterface::Material, "WheelSurfaceTrackMaterial")
	XT_CALLBACK(DynamicEffect::Track::XtInterface::MaterialCallback)
	XT_FIELD(texture, "texture")
	XT_FIELD(normal, "normal")
	XT_FIELD(blendMode, "blendMode")
XT_END_TYPE

XT_TYPE(AmaxWheelSurfaceTracks, "WheelSurfaceTracks")
	XT_CALLBACK(AmaxWheelSurfaceTracksHandler)
	XT_FIELD(modifier, "modifier")
	XT_FIELD(tracks,"tracks")
XT_END_TYPE

XT_TYPE(AmaxWheelSurfaceTrack, "WheelSurfaceTrack")
	XT_FIELD(surface, "surface")
	XT_FIELD(event, "event")
XT_END_TYPE

XT_TYPE(AmaxWheelSurfaceTrackEvent, "WheelSurfaceTrackEvent")
	XT_CALLBACK(AmaxWheelSurfaceTrackEventHandler)
	XT_FIELD(colour, "colour")
	XT_FIELD(material, "material")
	XT_FIELD(trigger, "trigger")
	XT_FIELD(alpha, "alpha")
	XT_FIELD(alphaRate, "alphaRate")
XT_END_TYPE

// misc effects

XT_TYPE(AmaxMiscParticleEvent, "MiscParticleEvent")
	XT_FIELD(type, "type")
	XT_FIELD(particleEvent, "particleEvent")
XT_END_TYPE

XT_TYPE(AmaxMiscEffects, "MiscEffects")
	XT_CALLBACK(AmaxMiscEffectsHandler)
	XT_FIELD(lightTrailMinSpeed, "lightTrailMinSpeed")
	XT_FIELD(lightTrailMaxSpeed, "lightTrailMaxSpeed")
	XT_FIELD(effects, "effects")	
	XT_FIELD(lightTrailIntensity, "lightTrailIntensity")
	XT_FIELD(lightTrailRearColour, "lightTrailRearColour")
	XT_FIELD(lightTrailBreakColour, "lightTrailBreakColour")
	XT_FIELD(lightTrailCameraFactor, "lightTrailCameraFactor")
XT_END_TYPE

XT_TYPE(Game::Effects::E_ExhuastEffectPosition, "ExhuastEmitPosition")
	XT_OPTION(Game::Effects::E_ExhuastEffectPosition_Left1, "Left1")
	XT_OPTION(Game::Effects::E_ExhuastEffectPosition_Left2, "Left2")
	XT_OPTION(Game::Effects::E_ExhuastEffectPosition_Right1, "Right1")
	XT_OPTION(Game::Effects::E_ExhuastEffectPosition_Right2, "Right2")
XT_END_TYPE

XT_TYPE(AmaxExhaustEmitter, "AmaxExhaustEmitter")
	XT_FIELD(exhaust, "exhaust")
	XT_FIELD(position, "position")
	XT_FIELD(direction, "direction")
XT_END_TYPE

XT_TYPE(Game::Effects::E_MiscParticleEventType,					"MiscParticleEventType")
	XT_OPTION(Game::Effects::E_MiscParticleEventType_ExhaustSmoke,		"ExhaustSmoke")
	XT_OPTION(Game::Effects::E_MiscParticleEventType_ExhaustBang,		"ExhaustBang")
	XT_OPTION(Game::Effects::E_MiscParticleEventType_ExhaustPop,		"ExhaustPop")
	XT_OPTION(Game::Effects::E_MiscParticleEventType_BrakeLightTrail,	"BrakeLightTrail")	
	XT_OPTION(Game::Effects::E_MiscParticleEventType_RearLightTrail,	"RearLightTrail")	
	XT_OPTION(Game::Effects::E_MiscParticleEventType_WheelSparks,		"WheelSparks")		
	XT_OPTION(Game::Effects::E_MiscParticleEventType_BurstTyre,			"BurstTyre")
	XT_OPTION(Game::Effects::E_MiscParticleEventType_EngineSmoke,		"EngineSmoke")	
	XT_OPTION(Game::Effects::E_MiscParticleEventType_EngineFire,		"EngineFire")	
	XT_OPTION(Game::Effects::E_MiscParticleEventType_GlassCrack,		"GlassCrack")
	XT_OPTION(Game::Effects::E_MiscParticleEventType_GlassSmash,		"GlassSmash")
	XT_OPTION(Game::Effects::E_MiscParticleEventType_ScrapeStatic,		"ScrapeStatic")
	XT_OPTION(Game::Effects::E_MiscParticleEventType_ScrapeDynamic,		"ScrapeDynamic")
	XT_OPTION(Game::Effects::E_MiscParticleEventType_LandingSparks,		"LandingSparks")
	XT_OPTION(Game::Effects::E_MiscParticleEventType_LandingDust,		"LandingDust")
	XT_OPTION(Game::Effects::E_MiscParticleEventType_WaterBigSplash,	"WaterBigSplash")
	XT_OPTION(Game::Effects::E_MiscParticleEventType_WaterBobbing,		"WaterBobbing")
	XT_OPTION(Game::Effects::E_MiscParticleEventType_LicensePlate,		"LicensePlate")	
XT_END_TYPE

XT_TYPE(Game::Effects::E_ParticleEmitterType, "AmaxVehicleParticleEmitterType")
	XT_OPTION(Game::Effects::E_ParticleEmitterType_RadiatorSmoke, "RadiatorSmoke")
	XT_OPTION(Game::Effects::E_ParticleEmitterType_EngineSmoke, "EngineSmoke")
	XT_OPTION(Game::Effects::E_ParticleEmitterType_EngineFire, "EngineFire")
XT_END_TYPE

XT_TYPE(AmaxVehicleParticleEmitter, "AmaxVehicleParticleEmitter")
	XT_FIELD(particleEventIdle, "particleEventIdle")
	XT_FIELD(particleEventSpeed, "particleEventSpeed")
	XT_FIELD(position, "position")
	XT_FIELD(direction, "direction")
XT_END_TYPE

XT_TYPE(AmaxVehicleParticleEmitterDesc, "AmaxVehicleParticleEmitterDesc")
	XT_FIELD(emitters, "emitters")
XT_END_TYPE

XT_TYPE(AmaxVehicleParticleEffects, "AmaxVehicleParticleEffects")
	XT_FIELD(radiatorSmoke, "radiatorSmoke")
	XT_FIELD(engineSmokeLight, "engineSmokeLight")
	XT_FIELD(engineSmokeHeavy, "engineSmokeHeavy")
	XT_FIELD(engineFire, "engineFire")
XT_END_TYPE

XT_TYPE(AmaxVehicleDirtSettings, "VehicleDirtSettings")
	XT_FIELD(baseColour, "baseColour")
	XT_FIELD(baseStartTime, "baseStartTime")
	XT_FIELD(baseDuration, "baseDuration")
	XT_FIELD(baseSpeedThreshold, "baseSpeedThreshold")
	XT_FIELD(detailColour, "detailColour")
	XT_FIELD(detailStartTime, "detailStartTime")
	XT_FIELD(detailDuration, "detailDuration")
	XT_FIELD(detailSpeedThreshold, "detailSpeedThreshold")
XT_END_TYPE

XT_LIBRARY(GAMEEFFECTDATA)

	//splash camera
	XT_REGISTER_TYPE(SplashCamera::C_HermiteKeyFrame)
	XT_REGISTER_TYPE(SplashCamera::C_HermiteCurve)
	XT_REGISTER_TYPE(SplashCamera::C_SplashType)
	XT_REGISTER_TYPE(AmaxWheelSurfaceSplashEvent)

	XT_REGISTER_TYPE(AmaxImpactSurfaceEffect)
	XT_REGISTER_TYPE(Game::Effects::E_SurfaceModifier)
	XT_REGISTER_TYPE(Game::E_EffectMaterial)
	XT_REGISTER_TYPE(AmaxWheelParticleEventDescriptor)
	XT_REGISTER_TYPE(Game::Effects::E_SurfaceParticleEventTrigger)
	// particles
	XT_REGISTER_TYPE(AmaxWheelSurfaceEffects)
	XT_REGISTER_TYPE(AmaxWheelSurfaceEffect)
	XT_REGISTER_TYPE(AmaxWheelSurfaceParticleEvent)
	// tracks
	XT_REGISTER_TYPE(AmaxWheelSurfaceTracks)
	XT_REGISTER_TYPE(AmaxWheelSurfaceTrack)
	XT_REGISTER_TYPE(AmaxWheelSurfaceTrackEvent)
	XT_REGISTER_TYPE(DynamicEffect::Track::XtInterface::E_BlendMode)
	XT_REGISTER_TYPE(DynamicEffect::Track::XtInterface::Material)
	// misc
	XT_REGISTER_TYPE(AmaxMiscParticleEvent)
	XT_REGISTER_TYPE(AmaxMiscEffects)
	XT_REGISTER_TYPE(Game::Effects::E_MiscParticleEventType)
	XT_REGISTER_TYPE(Game::Effects::E_ExhuastEffectPosition)
	XT_REGISTER_TYPE(AmaxExhaustEmitter)
	// vehicle
	XT_REGISTER_TYPE(AmaxVehicleDirtSettings)
	XT_REGISTER_TYPE(Game::Effects::E_ParticleEmitterType)
	XT_REGISTER_TYPE(AmaxVehicleParticleEmitter)
	XT_REGISTER_TYPE(AmaxVehicleParticleEmitterDesc)
	XT_REGISTER_TYPE(AmaxVehicleParticleEffects)	

XT_END_LIBRARY

C_GameEffectData::C_GameEffectData()
{
	for(S32 i = 0; i < Game::Effects::E_SurfaceModifier_Max; ++i)
	{
		mAmaxVehicleWheelSurfaceEffectsMapping[i] = -1;
		mAmaxVehicleWheelSurfaceTracksMapping[i] = -1;
	}
}

void C_GameEffectData::Init()
{

}
