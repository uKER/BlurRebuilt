#ifndef GAME_EFFECT_DATA_H
#define GAME_EFFECT_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/GameData/GameDataDefs.h>
#include <Graphics/Effects/EffectTypes.h>

#include <Game\GameDef\Materials.h>

#include <Dynamic/Particle/Generic/XtInterface.h>
#include <Dynamic/Particle/Event.h>
#include <Dynamic/Particle/EventInstance.h>

#include <Dynamic/Track/XtInterface.h>

#include "SplashCamera/SplashCamera.h"

struct AmaxExhaustEmitter
{
	Game::Effects::E_ExhuastEffectPosition exhaust;
	XtVec3 position;
	XtVec3 direction;
};

enum AmaxParticleEmitterType
{
	EngineSmoke,
	EngineFire,
};

struct AmaxVehicleParticleEmitter
{
	XtHandle<DynamicEffect::Particle::XtInterface::Event>	particleEventIdle;
	XtHandle<DynamicEffect::Particle::XtInterface::Event>	particleEventSpeed;
	XtVec3													position;
	XtVec3													direction;
};

struct AmaxVehicleParticleEmitterDesc
{
	XtArray<AmaxVehicleParticleEmitter>	emitters;
};

struct AmaxVehicleParticleEffects
{
	AmaxVehicleParticleEmitterDesc radiatorSmoke;	
	AmaxVehicleParticleEmitterDesc engineSmokeLight;	
	AmaxVehicleParticleEmitterDesc engineSmokeHeavy;	
	AmaxVehicleParticleEmitterDesc engineFire;
};

struct AmaxImpactSurfaceEffect : AmaxGameDataObj
{
	float					m_MinImpact;
	float					m_MaxImpact;
	float					m_MinIntensity;
	float					m_MaxIntensity;

	XtArray< Game::E_EffectMaterial >			m_SurfaceTypes;

	XtHandle<DynamicEffect::Particle::XtInterface::Event>	m_ParticleEvent;
};

// wheel/surface particle interaction effects

struct AmaxWheelParticleEventDescriptor
{
	XtVec3										position;
	XtVec3										direction;
	F32											intensity;
	F32											alpha;
	DynamicEffect::C_Node						node;
};

struct AmaxWheelSurfaceParticleEvent
{
	U32											id;

	XtHandle<DynamicEffect::Particle::XtInterface::Event>	particleEvent;
	Game::Effects::E_SurfaceParticleEventTrigger			trigger;
	XtArray<DynamicEffect::XtInterface::HermiteKeyFrame>	intensity;
	Curve::C_HermiteCurveFixed3								intensityCurve;
	F32														intensityRate;
	F32														intensityStep;

	XtArray<DynamicEffect::XtInterface::HermiteKeyFrame>	alpha;
	Curve::C_HermiteCurveFixed3								alphaCurve;
	F32														alphaRate;
	F32														alphaStep;

	bool													intensityRamp;
	bool													alphaRamp;

	AmaxWheelParticleEventDescriptor	carFL;
	AmaxWheelParticleEventDescriptor	carFR;
	AmaxWheelParticleEventDescriptor	carRL;
	AmaxWheelParticleEventDescriptor	carRR;
};

struct AmaxWheelSurfaceSplashEvent
{
	Game::Effects::E_SurfaceParticleEventTrigger			trigger;
	XtHandle<SplashCamera::C_SplashType>					splashCameraType;
	XtArray<DynamicEffect::XtInterface::HermiteKeyFrame>	splashCameraFactor;
	Curve::C_HermiteCurveFixed3								splashCameraFactorCurve;
	F32														splashCameraCountPerSecond;
	F32														splashCameraCurrentCount;
	F32														splashCameraSpread;
};


struct AmaxWheelSurfaceEffect
{
	Game::E_EffectMaterial		surface;
	
	XtArray< XtHandle<AmaxWheelSurfaceParticleEvent> >	events;
	XtArray< XtHandle<AmaxWheelSurfaceParticleEvent> >	rimEvents;
	XtArray< XtHandle<AmaxWheelSurfaceSplashEvent> >	splashEvents;
	bool												rimLightEnable;
	XtRgb												rimLightColour;
	F32													rimLightRadiusRange;
	F32													rimLightFrequency;
};

struct AmaxWheelSurfaceEffects
{
	Game::Effects::E_SurfaceModifier		modifier;
	XtArray<AmaxWheelSurfaceEffect>			effects;	
	S32										effectMapping[Game::E_EffectMaterial_Count];
};

// wheel/surface tyre track effects

struct AmaxWheelSurfaceTrackEvent
{
	U32												id;

	XtHandle<DynamicEffect::Track::XtInterface::Material>	material;

	XtRgba											colour;
	Game::Effects::E_SurfaceParticleEventTrigger	trigger;
	XtArray<DynamicEffect::XtInterface::HermiteKeyFrame>	alpha;
	Curve::C_HermiteCurveFixed3						alphaCurve;
	F32												alphaRate;
	F32												alphaStep;

	bool											alphaRamp;
};

struct AmaxWheelSurfaceTrack
{
	Game::E_EffectMaterial							surface;
	XtHandle<AmaxWheelSurfaceTrackEvent>			event;
};

struct AmaxWheelSurfaceTracks
{
	Game::Effects::E_SurfaceModifier	modifier;
	XtArray<AmaxWheelSurfaceTrack>		tracks;
	S32									trackMapping[Game::E_EffectMaterial_Count];
};

// misc particle effects

struct AmaxMiscParticleEvent
{
	Game::Effects::E_MiscParticleEventType					type;
	XtHandle<DynamicEffect::Particle::XtInterface::Event>	particleEvent;
};

struct AmaxMiscEffects : AmaxGameDataObj
{
	float								lightTrailMinSpeed;
	float								lightTrailMaxSpeed;
	float								lightTrailRangeRcp;
	XtArray<AmaxMiscParticleEvent>		effects;
	S32									effectMapping[Game::Effects::E_MiscParticleEventType_Max];
	XtArray<DynamicEffect::XtInterface::HermiteKeyFrame>		lightTrailIntensity;
	XtRgba								lightTrailRearColour;
	XtRgba								lightTrailBreakColour;
	float								lightTrailCameraFactor;
};

struct AmaxVehicleDirtSettings
{
	XtRgb baseColour;
	float baseStartTime;
	float baseDuration;
	float baseSpeedThreshold;
	XtRgb detailColour;
	float detailStartTime;
	float detailDuration;
	float detailSpeedThreshold;
};

class C_GameEffectData
{
public:
	C_GameEffectData();
	~C_GameEffectData() {}
	//void Init(XtDb* db);
	void Init();
	void Update();

	static C_GameEffectData& Instance() {return s_instance;}

	AmaxGameData<AmaxImpactSurfaceEffect*>		mAmaxImpactSurfaceEffects;

	AmaxGameData<AmaxWheelSurfaceEffects*>		mAmaxVehicleWheelSurfaceEffects;
	S32											mAmaxVehicleWheelSurfaceEffectsMapping[Game::Effects::E_SurfaceModifier_Max];
	AmaxGameData<AmaxWheelSurfaceTracks*>		mAmaxVehicleWheelSurfaceTracks;
	S32											mAmaxVehicleWheelSurfaceTracksMapping[Game::Effects::E_SurfaceModifier_Max];

	AmaxGameData<AmaxMiscEffects*>			mAmaxMiscEffects;

	XtPack*	mXtAmaxPack;

private:
	static C_GameEffectData s_instance;
};

#endif // GAME_EFFECT_DATA_H
