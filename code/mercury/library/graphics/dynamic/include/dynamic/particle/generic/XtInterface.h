#pragma once

#include "Dynamic/Generic/XtInterface.h"
#include "Dynamic/Particle/EventInstanceHandle.h"
#include "Dynamic/Particle/Enums.h"
#include "Dynamic/Enums.h"

namespace DynamicEffect
{
class C_Node;

namespace Particle
{
class C_ParticleManager;
class I_ParticleEffectBase;
class C_Event;

namespace XtInterface
{
	struct Effect;

	struct SpawnAmount
	{
		XtDispersionF32		amount;
		F32					distance;
		bool				disable;
	};

	struct Cone
	{
		F32					outer;
		F32					inner;
	};

	struct EmitterSpawnParams
	{
		E_EmitterSpawn				spawnType;
		E_EmitterSpace				spaceType;
		SpawnAmount					amount;
		Cone						cone;
		
		XtVec3						position;
		XtVec3						direction;

		E_EmitterDistribution		distributionType;
		XtVec3						distributionSize;

		E_EmitterOrientation		orientationType;
	};

	struct EmitterAnimationParams
	{
		bool						repeating;
		XtDispersionF32				time;
		XtDispersionF32				duration;
	};

	struct EmitterParticleParams
	{
		XtDispersionF32				life;
		XtDispersionF32				size;
		XtDispersionF32				speed;
		XtDispersionF32				speedInherited;
		XtDispersionVec3			rotation;
		XtDispersionVec3			spin;
		XtDispersionF32				force;

		E_EmitterColourMode			colourMode;
		XtRgba						colour1;
		XtRgba						colour2;
		E_EmitterUV					uv;
	};

	struct EmitterMiscParams
	{
		XtRangeF32					fadeDistance;
		F32							expiryTime;
		U32							particlesPerBlock;
	};

	struct LimitSize
	{
		E_ParticleLimitSize			type;
		F32							begin;
		F32							max;
	};

	struct Tracking
	{
		Tracking() : collisions(false), deaths(false) {}
		bool	collisions;
		bool	deaths;
	};

	struct Emitter
	{
		U32								id;
		XtHandle<Effect>				effect;
		
		EmitterSpawnParams				spawnParams;
		EmitterAnimationParams			animationParams;
		EmitterParticleParams			particleParams;
		EmitterMiscParams				miscParams;
	};

	struct ChildEmitter
	{
		E_EffectChild					type;
		XtRangeF32						intensitySpeed;
		Emitter							child;
	};

	struct EffectRenderParams
	{
		XtSz8						diffuseTexture;
		XtSz8						bumpTexture;
		E_Atlas						atlas;
		E_AtlasType					atlasType;
		F32							atlasLoopsPerSecond;
		E_ParticleShape				shape;
		XtSz8						shapeName;

		
		E_BlendMode					blendMode;
		E_ParticleSort				sort;
		DynamicEffect::XtInterface::Light	light;
		
		LimitSize					limitSize;
		XtVec3						pivot;
		F32							depthBias;
		F32							cullDistance;
		U32							freezeAmount;
		
		XtArray<DynamicEffect::XtInterface::HermiteKeyFrame>	sizeCurveX;
		XtArray<DynamicEffect::XtInterface::HermiteKeyFrame>	sizeCurveY;
		XtArray<DynamicEffect::XtInterface::HermiteKeyFrame>	sizeCurveZ;
		XtArray<DynamicEffect::XtInterface::HermiteKeyFrame>	animation;
		XtArray<DynamicEffect::XtInterface::ColourTime>			colourOverTime;
		XtArray<DynamicEffect::XtInterface::HermiteKeyFrame>	colourCurveA;
	};

	struct EffectPhysicsParams
	{
		E_PhysicsBehaviour			behaviour;
		E_Collision					collisions;
		F32							gravity;
		F32							airResistance;
		F32							restitution;
		F32							friction;
		bool						dieOnImpact;
	};

	struct Effect
	{
		Effect() : bInitialised(false), id(0), gameIndex(0xFF) {}
		bool						bInitialised;
		I_ParticleEffectBase*		pEffect;

		U32							id;
		U8							gameIndex;
		
		XtSz8						name;
		EffectRenderParams			renderParams;
		EffectPhysicsParams			physicsParams;
		XtArray<ChildEmitter>		children;
		
		Tracking					track;

		bool						uiLightTrail;
	};

	struct Event
	{
		bool						bInitialised;
		C_Event*					pEvent;
		XtU32						id;
		XtSz8						name;
		bool						repeating;
		XtDispersionF32				duration;
		XtArray<Emitter>			emitters;

	};

	struct WorldEvent
	{
		bool							bInitialised;
		C_EventInstanceHandle			eventHandle;

		XtHandle<Event>					event;
		XtVec3							position;
	};
	
	void					Initialise(C_ParticleManager* pParticleManager);
	void					Shutdown();
	
	C_EventInstanceHandle	SpawnEvent(const U32 id, const Maths::C_QuatTrans& transform, const C_Node* pNode = NULL);
	U32						GetNumEvents();
	U32						GetEventId(const U32 index);
	
	void					Update();
	void					Destroy();
};

} // Particle
} // DynamicEffect
