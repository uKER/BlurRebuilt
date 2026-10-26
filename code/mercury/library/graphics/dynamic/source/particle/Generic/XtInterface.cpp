#include "stdafx.h"
#include <system/Serialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <system/serialise/C_FileBlock.h>

#include <xt/xtgame.h>

// header
#include "Dynamic/Particle/Generic/XtInterface.h"

#include "Dynamic/Common.h"
#include "Dynamic/Renderer.h"
#include "Dynamic/TextureCoords.h"
#include "Dynamic/Particle/Event.h"
#include "Dynamic/Particle/MathsUtil.h"

#include "Dynamic/Particle/EventInstance.h"

// user particle includes
#include <system/Serialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <system/serialise/C_FileBlock.h>

#include "Dynamic/Particle/Tools_Shared/UserShape3DData.h"
#include "Dynamic/Particle/Tools_Shared/UserShape3DVariants.h"
#include "Dynamic/Particle/Tools_Shared/UserParticleContainer.h"
#include "Dynamic/Particle/UserParticleLibrary.h"

// effect instantiations
#include "Dynamic/Particle/Generic/Shape2DCollision.h"
#include "Dynamic/Particle/Generic/Shape2DSimple.h"
#include "Dynamic/Particle/Generic/Shape2DSimpleSuper.h"
#include "Dynamic/Particle/Generic/Shape2DCollisionSuper.h"
#include "Dynamic/Particle/Generic/Shape3DCollision.h"
#include "Dynamic/Particle/Generic/Shape3DCollisionSuper.h"
#include "Dynamic/Particle/Generic/Shape3DSimple.h"
#include "Dynamic/Particle/Generic/Shape3DSimpleSuper.h"
#include "Dynamic/Particle/Generic/Shape3DDecal.h"
#include "Dynamic/Particle/Generic/SpriteRotating.h"
#include "Dynamic/Particle/Generic/SpriteRotatingCollision.h"
#include "Dynamic/Particle/Generic/SpriteRotatingSuper.h"
#include "Dynamic/Particle/Generic/Shape2DSpline.h"
#include "Dynamic/Particle/Generic/SpriteRotatingSpline.h"

#include "misc_shared/Curve/HermiteCurve.h"

namespace DynamicEffect
{
namespace Particle
{
namespace XtInterface
{

Debug::Logger &g_Log = Debug::GetLogger("dynamic.particles.xtinterface");

// callbacks from sorcery
void WorldEventCallback(XtEventType event, XtRecord* record, WorldEvent* pData, XtCallbackContext*);
void EventCallback(XtEventType event, XtRecord* record, Event* pData, XtCallbackContext*);
void EffectCallback(XtEventType event, XtRecord* record, Effect* pData, XtCallbackContext*);

typedef Ut::UtMap<U32, Effect*>		Effects;
typedef Ut::UtMap<U32, Event*>		Events;
typedef Ut::UtSet<WorldEvent*>		WorldEvents;

Events					g_Events;
Effects					g_Effects;
WorldEvents				g_WorldEvents;
C_ParticleManager*		g_pParticleManager = NULL;

void					CreateEmitter(EmitterDefinition& settings, const Emitter* pEmitter);

void					InvalidateWorldEvent(WorldEvent* pData);
void					InvalidateEvent(Event* pData);
void					InvalidateEffect(Effect* pData);

void					UpdateEffects();
void					CreateEffect(Effect* pDefinition);
void					UpdateEvents();
void					UpdateWorldEvents();

void InvalidateWorldEvent(WorldEvent* pData)
{
	if(pData->bInitialised)
	{
		if(C_EventInstance* pEventInstance = pData->eventHandle.Object())
			pEventInstance->FreeRemove();

		pData->bInitialised = false;
	}
}

void WorldEventCallback(XtEventType event, XtRecord* record, WorldEvent* pData, XtCallbackContext*)
{
	if(event == kXtEtConstruct)
	{
		pData->bInitialised = false;
		g_WorldEvents.Insert(pData);
	}
	else if(event == kXtEtDestruct)
	{
		InvalidateWorldEvent(pData);
		g_WorldEvents.Erase(pData);
	}
}

void InvalidateEvent(Event* pData)
{
	if(pData->bInitialised)
	{
		for(WorldEvents::Iterator it = g_WorldEvents.Begin(); it != g_WorldEvents.End(); ++it)
		{
			WorldEvent* pWorldEvent = it->GetValue();
			if(pWorldEvent->event.Object() == pData)
				InvalidateWorldEvent(pWorldEvent);
		}
		if(pData->pEvent)
		{
			// todo interface with particle system here
			g_pParticleManager->CleanEvent(pData->pEvent);
			delete pData->pEvent;
			pData->pEvent = NULL;
		}

		pData->bInitialised = false;
	}
}


void EventCallback(XtEventType event, XtRecord* record, Event* pData, XtCallbackContext*)
{
	if(event == kXtEtConstruct)
	{
		pData->id = record->id;
		g_Events.Insert(Events::DataType(record->id, pData));
	}
	else if(event == kXtEtDestruct)
	{
		InvalidateEvent(pData);
		g_Events.Erase(pData->id);
	}
}

void InvalidateEffect(Effect* pData)
{
	if(pData->bInitialised)
	{
		// invalidiate any events which spawn particles into this effect
		for(Events::Iterator it = g_Events.Begin(); it != g_Events.End(); ++it)
		{
			Event* pEvent = it->GetValue();
			bool bDestruct = false;
			for(S32 i = 0; i < pEvent->emitters.count; ++i)
			{
				Emitter& emitter = pEvent->emitters[i];
				if(emitter.effect.Object() == pData)
					bDestruct = true;
			}
			if(bDestruct)
				InvalidateEvent(pEvent);
		}

		// invalidate any effects which spawn this effect through their child emitters
		for(Effects::Iterator it = g_Effects.Begin(); it != g_Effects.End(); )
		{
			bool bDestruct = false;
			Effect* pEffect = it->GetValue();
			it++;
			if(pEffect->pEffect) // we might have already deleted the game effect
			{
				for(S32 i = 0; i < pEffect->children.count; ++i)
				{
					Emitter& emitter = pEffect->children[i].child;
					if(emitter.effect.Object() == pData)
						bDestruct = true;
				}
				if(bDestruct)
					InvalidateEffect(pEffect);
			}
		}

		if(pData->pEffect)
		{
			// todo interface with ParticleSystem here
			DynamicEffect::XtInterface::Renderer()->CleanEffect(pData->pEffect);
			pData->pEffect->Destroy();
			pData->pEffect->Reset();
			delete pData->pEffect;
			pData->pEffect = NULL;
		}

		pData->bInitialised = false;
	}
}


void EffectCallback(XtEventType event, XtRecord* record, Effect* pData, XtCallbackContext*)
{
	if(event == kXtEtConstruct)
	{
		pData->id = record->id;
		pData->bInitialised = false;

		g_Effects.Insert(Effects::DataType(record->id, pData));
	}
	else if(event == kXtEtDestruct)
	{
		InvalidateEffect(pData);
		g_Effects.Erase(pData->id);
	}
}

}
}
}

// enumerations

XT_TYPE(DynamicEffect::E_BlendMode, "Particle_BlendMode")
	XT_OPTION(DynamicEffect::E_BlendMode_Additive, "Additive")
	XT_OPTION(DynamicEffect::E_BlendMode_Alpha, "Alpha")
	XT_OPTION(DynamicEffect::E_BlendMode_AlphaAdditive, "AlphaAdditive")
	XT_OPTION(DynamicEffect::E_BlendMode_AlphaSubtractive, "AlphaSubtractive")
	XT_OPTION(DynamicEffect::E_BlendMode_AlphaAndAdditive, "AlphaAndAdditive")
	XT_OPTION(DynamicEffect::E_BlendMode_Subtractive, "Subtractive")
	XT_OPTION(DynamicEffect::E_BlendMode_Solid, "None")
	XT_OPTION(DynamicEffect::E_BlendMode_SolidAlphaTest, "AlphaTest")
	XT_OPTION(DynamicEffect::E_BlendMode_SolidAlphaMask, "AlphaMask")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::E_ParticleShape, "Particle_Shape")
	XT_OPTION(DynamicEffect::Particle::E_ParticleShape_Beam, "Beam")
	XT_OPTION(DynamicEffect::Particle::E_ParticleShape_BeamCapped, "CappedBeam")
	XT_OPTION(DynamicEffect::Particle::E_ParticleShape_BeamCappedNoLifeScale, "CappedBeamNoLifeScale")
	XT_OPTION(DynamicEffect::Particle::E_ParticleShape_Sprite, "Sprite")
	XT_OPTION(DynamicEffect::Particle::E_ParticleShape_SpriteDirectional, "DirectionalSprite")
	XT_OPTION(DynamicEffect::Particle::E_ParticleShape_SpriteRotating, "RotatingSprite")
	XT_OPTION(DynamicEffect::Particle::E_ParticleShape_SpriteRotatingSoft, "SoftRotatingSprite")
	XT_OPTION(DynamicEffect::Particle::E_ParticleShape_Decal, "Decal")
	XT_OPTION(DynamicEffect::Particle::E_ParticleShape_Quad, "Quad")
	XT_OPTION(DynamicEffect::Particle::E_ParticleShape_QuadAngleFade, "QuadAngleFade")
	XT_OPTION(DynamicEffect::Particle::E_ParticleShape_PointSprite, "PointSprite")
	XT_OPTION(DynamicEffect::Particle::E_ParticleShape_PointSpriteLine, "PointSpriteLine")
	XT_OPTION(DynamicEffect::Particle::E_ParticleShape_Cube, "Cube")
	XT_OPTION(DynamicEffect::Particle::E_ParticleShape_Pyramid, "Pyramid")
	XT_OPTION(DynamicEffect::Particle::E_ParticleShape_Mesh, "Mesh")
	XT_OPTION(DynamicEffect::Particle::E_ParticleShape_None, "None")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::E_ParticleLimitSize, "Particle_E_LimitSize")
	XT_OPTION(DynamicEffect::Particle::E_ParticleLimitSize_NoLimit, "NoLimit")
	XT_OPTION(DynamicEffect::Particle::E_ParticleLimitSize_Clamp, "Clamp")
	XT_OPTION(DynamicEffect::Particle::E_ParticleLimitSize_AlphaFade, "AlphaFade")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::E_ParticleSort, "Particle_Sort")
	XT_OPTION(DynamicEffect::Particle::E_ParticleSort_PerBlock, "PerBlock")
	XT_OPTION(DynamicEffect::Particle::E_ParticleSort_PerParticle, "PerParticle")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::E_PhysicsBehaviour, "Particle_PhysicsBehaviour")
	XT_OPTION(DynamicEffect::Particle::E_PhysicsBehaviour_Default, "Default")
	XT_OPTION(DynamicEffect::Particle::E_PhysicsBehaviour_SplineFollower, "SplineFollower")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::E_Collision, "Particle_Collision")
	XT_OPTION(DynamicEffect::Particle::E_Collision_None, "None")
	XT_OPTION(DynamicEffect::Particle::E_Collision_Planar, "Planar")
	XT_OPTION(DynamicEffect::Particle::E_Collision_Full, "Full")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::E_EmitterDistribution, "Particle_EmitterDistribution")
	XT_OPTION(DynamicEffect::Particle::E_EmitterDistribution_Point, "Point")
	XT_OPTION(DynamicEffect::Particle::E_EmitterDistribution_Line, "Line")
	XT_OPTION(DynamicEffect::Particle::E_EmitterDistribution_Box, "Box")
	XT_OPTION(DynamicEffect::Particle::E_EmitterDistribution_Sphere, "Sphere")
	XT_OPTION(DynamicEffect::Particle::E_EmitterDistribution_Capsule, "Capsule")
	XT_OPTION(DynamicEffect::Particle::E_EmitterDistribution_Mesh, "Mesh")
	XT_OPTION(DynamicEffect::Particle::E_EmitterDistribution_Spline, "Spline")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::E_EmitterSpawn, "Particle_EmitterSpawn")
	XT_OPTION(DynamicEffect::Particle::E_EmitterSpawn_Normal, "Normal")
	XT_OPTION(DynamicEffect::Particle::E_EmitterSpawn_Burst, "Burst")
	XT_OPTION(DynamicEffect::Particle::E_EmitterSpawn_Persistent, "Persistent")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::E_EmitterOrientation, "Particle_EmitterOrientation")
	XT_OPTION(DynamicEffect::Particle::E_EmitterOrientation_Local, "Local")
	XT_OPTION(DynamicEffect::Particle::E_EmitterOrientation_World, "World")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::E_EmitterSpace, "Particle_EmitterSpace")
	XT_OPTION(DynamicEffect::Particle::E_EmitterSpace_World, "World")
	XT_OPTION(DynamicEffect::Particle::E_EmitterSpace_Local, "Local")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::E_EmitterColourMode, "Particle_EmitterColourMode")
	XT_OPTION(DynamicEffect::Particle::E_EmitterColourMode_SingleColour, "SingleColour")
	XT_OPTION(DynamicEffect::Particle::E_EmitterColourMode_LerpColour, "LerpColour")
	XT_OPTION(DynamicEffect::Particle::E_EmitterColourMode_LerpChannels, "LerpChannels")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::E_EmitterUV, "Particle_EmitterUV")
	XT_OPTION(DynamicEffect::Particle::E_EmitterUV_DefaultUV, "DefaultUV")
	XT_OPTION(DynamicEffect::Particle::E_EmitterUV_FlipU, "FlipU")
	XT_OPTION(DynamicEffect::Particle::E_EmitterUV_FlipV, "FlipV")
	XT_OPTION(DynamicEffect::Particle::E_EmitterUV_FlipUV, "FlipUV")
	XT_OPTION(DynamicEffect::Particle::E_EmitterUV_Random, "Random")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::E_EffectChild, "Particle_Child")
	XT_OPTION(DynamicEffect::Particle::E_EffectChild_Trail, "Trail")
	XT_OPTION(DynamicEffect::Particle::E_EffectChild_Death, "Death")
	XT_OPTION(DynamicEffect::Particle::E_EffectChild_Collision, "Collision")
	XT_OPTION(DynamicEffect::Particle::E_EffectChild_CollisionOnce, "CollisionOnce")
XT_END_TYPE

// helper classes


XT_TYPE(DynamicEffect::Particle::XtInterface::SpawnAmount, "Particle_SpawnAmount")
	XT_FIELD(amount, "amount")
	XT_FIELD(distance, "distance")
	XT_FIELD(disable, "disable")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::XtInterface::LimitSize, "Particle_LimitSize")
	XT_FIELD(type, "type")
	XT_FIELD(begin, "begin")
	XT_FIELD(max, "max")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::XtInterface::Cone, "Particle_Cone")
	XT_FIELD(outer, "outer")
	XT_FIELD(inner, "inner")
XT_END_TYPE

// actual classes

XT_TYPE(DynamicEffect::Particle::XtInterface::EmitterSpawnParams, "Particle_Emitter_SpawnParams")
	XT_FIELD(spawnType, "type")
	XT_FIELD(spaceType, "space")
	XT_FIELD(amount, "amount")
	XT_FIELD(cone, "cone")
	XT_FIELD(position, "position")
	XT_FIELD(direction, "direction")
	XT_FIELD(distributionType, "distribution")
	XT_FIELD(distributionSize, "distributionSize")
	XT_FIELD(orientationType, "orientation")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::XtInterface::EmitterAnimationParams, "Particle_Emitter_AnimationParams")
	XT_FIELD(repeating, "repeating")
	XT_FIELD(time, "time")
	XT_FIELD(duration, "duration")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::XtInterface::EmitterParticleParams, "Particle_Emitter_ParticleParams")
	XT_FIELD(life, "life")
	XT_FIELD(size, "size")
	XT_FIELD(speed, "speed")
	XT_FIELD(speedInherited, "speedInherited")
	XT_FIELD(rotation, "rotation")
	XT_FIELD(spin, "spin")
	XT_FIELD(uv, "uv")
	XT_FIELD(colourMode, "colourMode")
	XT_FIELD(colour1, "colour")
	XT_FIELD(colour2, "colour2")
	XT_FIELD(force, "force")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::XtInterface::EmitterMiscParams, "Particle_Emitter_MiscParams")
	XT_FIELD(fadeDistance, "fadeDistance")
	XT_FIELD(expiryTime, "expiryTime")
	XT_FIELD(particlesPerBlock, "particlesPerBlock")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::XtInterface::Emitter, "Particle_Emitter")
	XT_FIELD(effect, "effect")
	
	XT_FIELD(spawnParams, "spawn")
	XT_FIELD(animationParams, "animation")
	XT_FIELD(particleParams, "particle")
	XT_FIELD(miscParams, "misc")
XT_END_TYPE


XT_TYPE(DynamicEffect::Particle::XtInterface::EffectRenderParams, "Particle_Effect_RenderParams")
	XT_FIELD(diffuseTexture, "diffuseTexture")
	XT_FIELD(bumpTexture, "bumpTexture")
	XT_FIELD(atlas, "atlas")
	XT_FIELD(atlasType, "atlasType")
	XT_FIELD(atlasLoopsPerSecond, "LoopsPerSecond")
	XT_FIELD(shape, "shape")
	XT_FIELD(shapeName, "shapeName")
	XT_FIELD(blendMode, "blendMode")
	XT_FIELD(sort, "sort")
	XT_FIELD(light, "light")
	XT_FIELD(limitSize, "limitSize")
	XT_FIELD(pivot, "pivot")
	XT_FIELD(depthBias, "depthBias")
	XT_FIELD(cullDistance, "cullDistance")
	XT_FIELD(freezeAmount, "freezeAmount")
	XT_FIELD(sizeCurveX, "sizeCurveX")
    XT_FIELD(sizeCurveY, "sizeCurveY")
    XT_FIELD(sizeCurveZ, "sizeCurveZ")
    XT_FIELD(animation, "animation") 
	XT_FIELD(colourOverTime, "colourOverTime")
	XT_FIELD(colourCurveA, "colourCurveA")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::XtInterface::EffectPhysicsParams, "Particle_Effect_PhysicsParams")
	XT_FIELD(behaviour, "behaviour")
	XT_FIELD(collisions, "collisions")	
	XT_FIELD(gravity, "gravity")
	XT_FIELD(airResistance, "airResistance")
	XT_FIELD(restitution, "restitution")
	XT_FIELD(friction, "friction")
	XT_FIELD(dieOnImpact, "dieOnImpact")
XT_END_TYPE
	
XT_TYPE(DynamicEffect::Particle::XtInterface::Effect, "Particle_Effect")
	XT_CALLBACK(DynamicEffect::Particle::XtInterface::EffectCallback)
	XT_FIELD(name, "name")
	XT_FIELD(renderParams, "render")
	XT_FIELD(physicsParams, "physics")
	XT_FIELD(children, "children")
	XT_FIELD(uiLightTrail, "uiLightTrail")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::XtInterface::ChildEmitter, "Particle_ChildEmitter")
	XT_FIELD(type, "type")
	XT_FIELD(intensitySpeed, "intensitySpeed")
	XT_FIELD(child, "child")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::XtInterface::Event, "Particle_Event")
	XT_CALLBACK(DynamicEffect::Particle::XtInterface::EventCallback)
	XT_FIELD(name, "name")
	XT_FIELD(duration, "duration")
	XT_FIELD(repeating, "repeating")
	XT_FIELD(emitters, "emitters")
XT_END_TYPE

XT_TYPE(DynamicEffect::Particle::XtInterface::WorldEvent, "Particle_WorldEvent")
	XT_CALLBACK(DynamicEffect::Particle::XtInterface::WorldEventCallback)
	XT_FIELD(event, "event")
	XT_FIELD(position, "position")
XT_END_TYPE

XT_LIBRARY(GraphicsDynamicParticle)
	XT_REGISTER_TYPE(DynamicEffect::E_BlendMode)
	XT_REGISTER_TYPE(DynamicEffect::Particle::E_ParticleShape)
	XT_REGISTER_TYPE(DynamicEffect::Particle::E_ParticleSort)
	XT_REGISTER_TYPE(DynamicEffect::Particle::E_PhysicsBehaviour)
	XT_REGISTER_TYPE(DynamicEffect::Particle::E_Collision)
	XT_REGISTER_TYPE(DynamicEffect::Particle::E_EmitterSpawn)
	XT_REGISTER_TYPE(DynamicEffect::Particle::E_EmitterOrientation)
	XT_REGISTER_TYPE(DynamicEffect::Particle::E_EmitterSpace)
	XT_REGISTER_TYPE(DynamicEffect::Particle::E_EmitterDistribution)
	XT_REGISTER_TYPE(DynamicEffect::Particle::E_EmitterColourMode)
	XT_REGISTER_TYPE(DynamicEffect::Particle::E_EmitterUV)
	XT_REGISTER_TYPE(DynamicEffect::Particle::E_EffectChild)
	XT_REGISTER_TYPE(DynamicEffect::Particle::E_ParticleLimitSize)
	
	XT_REGISTER_TYPE(DynamicEffect::Particle::XtInterface::SpawnAmount)
	XT_REGISTER_TYPE(DynamicEffect::Particle::XtInterface::Cone)
	XT_REGISTER_TYPE(DynamicEffect::Particle::XtInterface::LimitSize)

	XT_REGISTER_TYPE(DynamicEffect::Particle::XtInterface::EffectRenderParams)
	XT_REGISTER_TYPE(DynamicEffect::Particle::XtInterface::EffectPhysicsParams)
	XT_REGISTER_TYPE(DynamicEffect::Particle::XtInterface::Effect)
	
	XT_REGISTER_TYPE(DynamicEffect::Particle::XtInterface::EmitterSpawnParams)
	XT_REGISTER_TYPE(DynamicEffect::Particle::XtInterface::EmitterAnimationParams)
	XT_REGISTER_TYPE(DynamicEffect::Particle::XtInterface::EmitterParticleParams)
	XT_REGISTER_TYPE(DynamicEffect::Particle::XtInterface::EmitterMiscParams)
	XT_REGISTER_TYPE(DynamicEffect::Particle::XtInterface::Emitter)

	XT_REGISTER_TYPE(DynamicEffect::Particle::XtInterface::ChildEmitter)
	XT_REGISTER_TYPE(DynamicEffect::Particle::XtInterface::Event)
	XT_REGISTER_TYPE(DynamicEffect::Particle::XtInterface::WorldEvent)
XT_END_LIBRARY

namespace DynamicEffect
{
namespace Particle
{
namespace XtInterface
{

void CreateEmitter(DynamicEffect::Particle::EmitterDefinition& settings, const Emitter* pEmitter)
{
	// super effects are always in world space due to nested transform complications
	settings.eSpace = pEmitter->spawnParams.spaceType;

	settings.eCollision = settings.pEffect->GetCollisionType();

	settings.eSpawn = pEmitter->spawnParams.spawnType;
	settings.bRepeating = pEmitter->animationParams.repeating;
	settings.eOrientation = pEmitter->spawnParams.orientationType;
	settings.eDistribution = pEmitter->spawnParams.distributionType;
	settings.eColourMode = pEmitter->particleParams.colourMode; 
	
	settings.fConeOuter = pEmitter->spawnParams.cone.outer * -Maths::Pi/360.0f;
	settings.fConeInner = pEmitter->spawnParams.cone.inner * -Maths::Pi/360.0f;

	// if duration is zero set it to infinity
	if(settings.bRepeating && settings.eSpawn == DynamicEffect::Particle::E_EmitterSpawn_Normal && pEmitter->animationParams.duration.mean == 0.0f)
	{
		settings.fDuration = 0x7F800000;
		settings.fDurationVariation = 0.0f;
	}
	else
	{
		settings.fDuration = pEmitter->animationParams.duration.mean;
		settings.fDurationVariation = pEmitter->animationParams.duration.d;
	}

	settings.fRate = pEmitter->spawnParams.amount.amount.mean;
	settings.fRateVariation = pEmitter->spawnParams.amount.amount.d;
		
	if(settings.fRateVariation > settings.fRate)
	{
		LOGWARN(g_Log, "DynamicEffect::Particle::XtInterface: Emitter spawn variation (%f) is higher than rate (%f), variation clamped to rate (%X)", settings.fRateVariation, settings.fRate, pEmitter->id);
		settings.fRateVariation = settings.fRate;
	}

	settings.fRateDistance = pEmitter->spawnParams.amount.distance;

	if(pEmitter->spawnParams.amount.disable)
	{
		settings.fRate = 0.0f;
		settings.fRateVariation = 0.0f;
		settings.fRateDistance = 0.0f;
	}

	settings.fTime = pEmitter->animationParams.time.mean;
	settings.fTimeVariation = pEmitter->animationParams.time.d;
	settings.vPosition = MathsStorage::Float4A16(pEmitter->spawnParams.position.vx, pEmitter->spawnParams.position.vy, pEmitter->spawnParams.position.vz, 1.0f);

	// get a rotation quaternion from straight up to required direction
	Maths::HVec3 vUp = Maths::Constants<Maths::HVec3>::UnitAxisPositiveY();
	Maths::HVec3 vDefault = Maths::Load<Maths::HVec3>(&g_pParticleManager->GetDefaultSpawnDirection());
	Maths::HVec3 vDirection(pEmitter->spawnParams.direction.vx, pEmitter->spawnParams.direction.vy, pEmitter->spawnParams.direction.vz);
	
	// if no direction has been set, set the default direction
	if(Maths::Length2(vDirection) == Maths::Constants<Maths::HF32>::Zero())
		vDirection = vDefault;

	vDirection.Normalise();
	settings.vRotation = DynamicEffect::GetQuaternionFromTo<MathsStorage::Float4A16>(vUp, vDirection);
	
	// half size because interal representation is from centre to boundary
	settings.fSize = pEmitter->particleParams.size.mean * 0.5f;
	settings.fSizeRandom = pEmitter->particleParams.size.d * 0.5f;

	if(pEmitter->particleParams.life.mean == 0.0f)
		settings.fLife = 1.0f;
	else
		settings.fLife = pEmitter->particleParams.life.mean;

	settings.fLifeRandom = pEmitter->particleParams.life.d;

	if(settings.fLifeRandom > settings.fLife)
	{
		LOGWARN(g_Log, "DynamicEffect::Particle::XtInterface: Emitter life variation (%f) is higher than life (%f), variation set to zero (%X)", settings.fLifeRandom, settings.fLife, pEmitter->id);
		settings.fLifeRandom = 0.0f;
	}
	
	settings.fSpeed = pEmitter->particleParams.speed.mean;
	settings.fSpeedRandom = pEmitter->particleParams.speed.d;
	settings.fSpeedInherited = pEmitter->particleParams.speedInherited.mean;
	settings.fSpeedInheritedRandom = pEmitter->particleParams.speedInherited.d;
	settings.fForce = pEmitter->particleParams.force.mean;
	settings.fForceRandom = pEmitter->particleParams.force.d;

	settings.vSize = MathsStorage::Float4A16(pEmitter->spawnParams.distributionSize.vx, pEmitter->spawnParams.distributionSize.vy, pEmitter->spawnParams.distributionSize.vz, 1.0f);

	// calculate the cull sphere size based on distribution
	/*settings.fEmitRadiusSq = pEmitter->miscParams.emitRadius * pEmitter->miscParams.emitRadius;
	if(pEmitter->miscParams.cullDistance > 0.0f)
		settings.fCullDistance = pEmitter->miscParams.cullDistance;
	else
		settings.fCullDistance = g_pParticleManager->GetCullDistance();*/
	
	settings.fExpiryTime = pEmitter->miscParams.expiryTime;
	settings.iParticlesPerBlock = pEmitter->miscParams.particlesPerBlock != 0 ? pEmitter->miscParams.particlesPerBlock : 0xFFFFFFFF;
	

	settings.cullSphere = Maths::C_Sphere(Maths::Constants<Maths::HF32>::Zero(), Maths::Constants<Maths::HF32>::Zero(), Maths::Constants<Maths::HF32>::Zero(), Maths::Constants<Maths::HF32>::Quarter());
	if(settings.eDistribution == E_EmitterDistribution_Box)
	{
		Maths::HVec3 vSize(Maths::Load<Maths::HVec3>(&settings.vSize));
		settings.cullSphere = Maths::C_Sphere(settings.cullSphere.Centre(), settings.cullSphere.Radius() + Maths::Length(vSize) * Maths::Constants<Maths::HF32>::Half());
	}
	else if(settings.eDistribution == E_EmitterDistribution_Sphere)
	{
		Maths::HVec3 vSize(Maths::Load<Maths::HVec3>(&settings.vSize));
		Maths::HF32 fBig = vSize.X();
		fBig = fBig.Max(vSize.Y());
		fBig = fBig.Max(vSize.Z());
		settings.cullSphere = Maths::C_Sphere(settings.cullSphere.Centre(), settings.cullSphere.Radius() + fBig * Maths::Constants<Maths::HF32>::Half());
	}

	if(pEmitter->miscParams.fadeDistance.to != 0.0f)
	{
		DynamicEffect::GetLodFade(settings.lodFade, pEmitter->miscParams.fadeDistance.to, pEmitter->miscParams.fadeDistance.from);
		settings.cullDistance = pEmitter->miscParams.fadeDistance.to;
	}
	else
		settings.cullDistance = 0.0f;

	if(pEmitter->particleParams.colour1.a == 0.0f)
		settings.vColour1 = MathsStorage::Float4A16(1.0f, 1.0f, 1.0f, 1.0f);
	else
		settings.vColour1 = MathsStorage::Float4A16(pEmitter->particleParams.colour1.r, pEmitter->particleParams.colour1.g, pEmitter->particleParams.colour1.b, pEmitter->particleParams.colour1.a);

	if(pEmitter->particleParams.colour2.a == 0.0f)
		settings.vColour2 = MathsStorage::Float4A16(1.0f, 1.0f, 1.0f, 1.0f);
	else
		settings.vColour2 = MathsStorage::Float4A16(pEmitter->particleParams.colour2.r, pEmitter->particleParams.colour2.g, pEmitter->particleParams.colour2.b, pEmitter->particleParams.colour2.a);
	settings.eUV = pEmitter->particleParams.uv;

	// get orientation and random orientation
	settings.vOrientation.x = Maths::DegToRad(pEmitter->particleParams.rotation.mean.vx);
	settings.vOrientation.y = Maths::DegToRad(pEmitter->particleParams.rotation.mean.vy);
	settings.vOrientation.z = Maths::DegToRad(pEmitter->particleParams.rotation.mean.vz);
	settings.vOrientation.w = 0.0f;

	settings.vOrientationRandom.x = Maths::DegToRad(pEmitter->particleParams.rotation.d.vx);
	settings.vOrientationRandom.y = Maths::DegToRad(pEmitter->particleParams.rotation.d.vy);
	settings.vOrientationRandom.z = Maths::DegToRad(pEmitter->particleParams.rotation.d.vz);
	settings.vOrientationRandom.w = 0.0f;

	settings.vAngularVelocity.x = -Maths::DegToRad(pEmitter->particleParams.spin.mean.vx);
	settings.vAngularVelocity.y = -Maths::DegToRad(pEmitter->particleParams.spin.mean.vy);
	settings.vAngularVelocity.z = -Maths::DegToRad(pEmitter->particleParams.spin.mean.vz);
	settings.vAngularVelocity.w = 0.0f;

	settings.vAngularVelocityRandom.x = Maths::DegToRad(pEmitter->particleParams.spin.d.vx);
	settings.vAngularVelocityRandom.y = Maths::DegToRad(pEmitter->particleParams.spin.d.vy);
	settings.vAngularVelocityRandom.z = Maths::DegToRad(pEmitter->particleParams.spin.d.vz);
	settings.vAngularVelocityRandom.w = 0.0f;
}

void UpdateEvents()
{
	for(Events::Iterator it = g_Events.Begin(); it != g_Events.End(); ++it)
	{
		Event* pData = it->GetValue();
		if(!pData->bInitialised)
		{
			DynamicEffect::Particle::EventDefinition definition;
			definition.name = Util::C_String(pData->name);
			definition.bRepeating = pData->repeating;
			definition.fDuration = pData->duration.mean;
			definition.fDurationVariation = pData->duration.d;
			definition.fCullDistance = g_pParticleManager->GetCullDistance();
			definition.bCollisionPlane = false;
			
			F32 fMaxDuration = 0.0f;
			for(S32 i = 0; i < pData->emitters.count; ++i)
			{
				Emitter& emitter = pData->emitters[i];
				F32 fDuration = emitter.animationParams.time.mean + emitter.animationParams.time.d;
				fDuration += emitter.animationParams.duration.mean + emitter.animationParams.duration.d;

				fMaxDuration = Maths::Max(fMaxDuration, emitter.animationParams.duration.mean + emitter.animationParams.duration.d);
			}

			definition.fDuration = Maths::Max(definition.fDuration, fMaxDuration);
			definition.fLocalParticleLife = 0.0f;

			// check whether any of the effects used by this event requires a collision plane
			for(S32 i = 0; i < pData->emitters.count; ++i)
			{
				Emitter& emitter = pData->emitters[i];
				Effect* effect = emitter.effect.Object();
				if(effect)
				{
					assert(effect->bInitialised);

					// store the maximum life time of local space particles for 
					if(emitter.spawnParams.spaceType == DynamicEffect::Particle::E_EmitterSpace_Local)
					{
						F32 fLocalParticleLife = emitter.particleParams.life.mean != 0.0f ? emitter.particleParams.life.mean : 1.0f;
						fLocalParticleLife += emitter.particleParams.life.d;
						definition.fLocalParticleLife = Maths::Max(definition.fLocalParticleLife, fLocalParticleLife);
					}

					DynamicEffect::Particle::I_ParticleEffectBase* pEffect = (I_ParticleEffectBase*)g_pParticleManager->FindEffect(effect->id);
					
					assert(pEffect);
					if(pEffect->GetCollisionType() == DynamicEffect::Particle::E_Collision_Planar || pEffect->GetCollisionType() == DynamicEffect::Particle::E_Collision_Full)
						definition.bCollisionPlane = true;
				}
			}

			pData->pEvent = new DynamicEffect::Particle::C_Event(g_pParticleManager, definition);

			for(S32 i = 0; i < pData->emitters.count; ++i)
			{
				Emitter& emitter = pData->emitters[i];
				emitter.id = pData->id;
				if(emitter.effect.Object())
				{
					DynamicEffect::Particle::EmitterDefinition settings;
					settings.pEffect = (I_ParticleEffectBase*)g_pParticleManager->FindEffect(emitter.effect.Object()->id);
					assert(settings.pEffect);
					CreateEmitter(settings, &emitter);
					settings.id = pData->id;
					pData->pEvent->AddDefinition(settings);
				}

			}
			pData->bInitialised = true;
		}
	}
}

void UpdateWorldEvents()
{
	for(WorldEvents::Iterator it = g_WorldEvents.Begin(); it != g_WorldEvents.End(); ++it)
	{
		WorldEvent* pData = it->GetValue();
		if(!pData->bInitialised)
		{
			if(Event* event = pData->event.Object())
			{
				DynamicEffect::Particle::C_Event* pEvent = event->pEvent;
				DynamicEffect::C_Node local(MathsStorage::Float4A16(pData->position.vx, pData->position.vy, pData->position.vz, 1.0f), MathsStorage::Float4A16(0.0f, 0.0f, 0.0f, 1.0f));
				pData->eventHandle = pEvent->Create(local, NULL);

				pData->eventHandle.Object()->SetLighting(MathsStorage::Float4A16(1.0f, 1.0f, 1.0f, 1.0f));
				pData->eventHandle.Object()->SetCollisionPlane(MathsStorage::Float4A16(0.0f, 0.0f, 0.0f, 1.0f), MathsStorage::Float4A16(0.0f, 1.0f, 0.0f, 0.0f));
			}
			pData->bInitialised = true;
		}
	}
}

DynamicEffect::Particle::C_EventInstanceHandle	SpawnEvent(const U32 id, const Maths::C_QuatTrans& transform, const C_Node* pNode)
{
	Events::Iterator it = g_Events.Find(id);
	DynamicEffect::Particle::C_EventInstanceHandle particleHandle;
	
	if(it != g_Events.End())
	{
		Event* pSorceryEvent = it->GetValue();
		DynamicEffect::Particle::C_Event* pEvent = pSorceryEvent->pEvent;
		DynamicEffect::C_Node local(transform);

		particleHandle = pEvent->Create(local, pNode);
	}

	return particleHandle;
}

U32 GetNumEvents()
{
	return g_Events.Size();
}

U32 GetEventId(U32 index)
{
	U32 count = 0;
	for(Events::Iterator it = g_Events.Begin(); it != g_Events.End(); ++it)
	{
		if(count == index)
			return it->GetKey();
		else
			++count;
	}

	return 0xFFFFFFFF;
}

// recursive creation function instantiates all effects
void CreateEffect(Effect* pDefinition)
{
	RESOURCE_MEMORY_ID("ParticleEffects");

	Util::C_String effectName(pDefinition->name);
			
	DynamicEffect::Particle::UpdateSettings updateSettings;
	{
		updateSettings.iFreezeAmount = pDefinition->renderParams.freezeAmount;
		
		updateSettings.bTrackCollisions = pDefinition->track.collisions;
		updateSettings.bTrackDeaths = pDefinition->track.deaths;
		updateSettings.iGameIndex = pDefinition->gameIndex;
	}

	DynamicEffect::RenderStates renderStates;
	{
		renderStates.fDepthBias = pDefinition->renderParams.depthBias;
	}

	DynamicEffect::Particle::RenderSettings renderSettings;
	{
		renderSettings.lightTrail = pDefinition->uiLightTrail;

		renderSettings.bLightReceive = pDefinition->renderParams.light.receive;
		renderSettings.bLightEmit = pDefinition->renderParams.light.emit;
		if(pDefinition->renderParams.light.radius == 0.0f)
			renderSettings.fLightRadius = 1.0f;
		else
			renderSettings.fLightRadius = pDefinition->renderParams.light.radius;
		renderSettings.fLightIntensity = pDefinition->renderParams.light.intensity;
		renderSettings.iLightPriority = pDefinition->renderParams.light.priority;

		if(pDefinition->renderParams.shape == E_ParticleShape_SpriteRotatingSoft)
			renderSettings.soft = true;
		else
			renderSettings.soft = false;

		renderSettings.blendMode = pDefinition->renderParams.blendMode;
		if(renderSettings.blendMode == DynamicEffect::E_BlendMode_Solid || renderSettings.blendMode == DynamicEffect::E_BlendMode_SolidAlphaTest)
			renderSettings.blendModeTransparent = false;
		else
			renderSettings.blendModeTransparent = true;

		if(renderSettings.blendMode == DynamicEffect::E_BlendMode_Additive || renderSettings.blendMode == DynamicEffect::E_BlendMode_AdditiveA8 || 
			renderSettings.blendMode == DynamicEffect::E_BlendMode_AlphaAdditive)
			renderSettings.additive = true;
		else
			renderSettings.additive = false;
	
		RESOURCE_MEMORY_ID("DynamicEffect");

		if(strlen(pDefinition->renderParams.diffuseTexture) > 0)
		{
			Util::C_String filename(pDefinition->renderParams.diffuseTexture);
			renderSettings.pDiffuse = DynamicEffect::XtInterface::Renderer()->LoadTexture(filename);

			// texture couldn't be loaded, so print additional information
			if(!renderSettings.pDiffuse)
				LOGWARN(g_Log, "DynamicEffect: Could not find texture '%s' for effect '%s' (%X)",(const char*)pDefinition->renderParams.diffuseTexture, (const char*)pDefinition->name, pDefinition->id);
			else
			{
#if !PLATFORM_PS3
				if(renderSettings.pDiffuse->GetFormat() == Display2::E_TextureFormat_A8)
				{
					if(renderSettings.blendMode == DynamicEffect::E_BlendMode_Alpha)
						renderSettings.blendMode = DynamicEffect::E_BlendMode_AlphaA8;
					else if(renderSettings.blendMode == DynamicEffect::E_BlendMode_Additive)
						renderSettings.blendMode = DynamicEffect::E_BlendMode_AdditiveA8;
					else if(renderSettings.blendMode == DynamicEffect::E_BlendMode_Subtractive)
						renderSettings.blendMode = DynamicEffect::E_BlendMode_SubtractiveA8;
				}
#endif
			}
		}
		else
			renderSettings.pDiffuse = NULL;

		//renderSettings.eParticleType = pDefinition->type;
		renderSettings.vertexBuildSettings.eParticleShape	= pDefinition->renderParams.shape;
		renderSettings.vertexBuildSettings.eParticleSort	= pDefinition->renderParams.sort;

		renderSettings.vertexBuildSettings.eAtlasType		= pDefinition->renderParams.atlasType;
		renderSettings.vertexBuildSettings.iAtlasOffset		= DynamicEffect::C_TextureCoords::Instance().GetAtlasOffset(pDefinition->renderParams.atlas);
		renderSettings.vertexBuildSettings.iAtlasSize		= DynamicEffect::C_TextureCoords::Instance().GetAtlasSize(pDefinition->renderParams.atlas);
		renderSettings.vertexBuildSettings.fAtlasAnimPeriod = pDefinition->renderParams.atlasLoopsPerSecond != 0.0f ? 1.0f / pDefinition->renderParams.atlasLoopsPerSecond : 0.0f;
				
		if (renderSettings.vertexBuildSettings.eParticleShape == E_ParticleShape_Mesh)
		{
			renderSettings.vertexBuildSettings.iParticleLibraryIndex = 
				DynamicEffect::Particle::C_UserParticleLibrary::Instance().GetShape3DVariantsIndex(
					(const char*)pDefinition->renderParams.shapeName);

			// -1 returned if particle mesh not found, so make it a cube instead
			if (renderSettings.vertexBuildSettings.iParticleLibraryIndex == -1)
			{
				LOGWARN(g_Log, "DynamicEffect: Could not find user particle mesh '%s', defaulting to cube.\n",(const char*)pDefinition->renderParams.shapeName);
				renderSettings.vertexBuildSettings.eParticleShape = E_ParticleShape_Cube;
				renderSettings.vertexBuildSettings.pUserShape3DVariants = NULL;
			}
			else
			{
				LOGDEBUG(g_Log, "DynamicEffect: Found user particle mesh '%s'.\n",(const char*)pDefinition->renderParams.shapeName);
				renderSettings.vertexBuildSettings.pUserShape3DVariants = &DynamicEffect::Particle::C_UserParticleLibrary::Instance().GetShape3DVariants(renderSettings.vertexBuildSettings.iParticleLibraryIndex);
		}
		}
		else
		{
			renderSettings.vertexBuildSettings.iParticleLibraryIndex = -1;
		}
		
		renderSettings.vertexBuildSettings.eLimitSize = pDefinition->renderParams.limitSize.type;

		renderSettings.vertexBuildSettings.limitSize.x = pDefinition->renderParams.limitSize.begin;
		renderSettings.vertexBuildSettings.limitSize.y = pDefinition->renderParams.limitSize.max;
		renderSettings.vertexBuildSettings.limitSize.z = pDefinition->renderParams.limitSize.max - pDefinition->renderParams.limitSize.begin;
		if((pDefinition->renderParams.limitSize.max - pDefinition->renderParams.limitSize.begin) > 0.0f)
			renderSettings.vertexBuildSettings.limitSize.w = 1.0f / (pDefinition->renderParams.limitSize.max - pDefinition->renderParams.limitSize.begin);
		else
			renderSettings.vertexBuildSettings.limitSize.w = 0.0f;

		if(renderSettings.vertexBuildSettings.eParticleShape == E_ParticleShape_SpriteRotating)
		{
			renderSettings.vertexBuildSettings.pivot.x = pDefinition->renderParams.pivot.vx * 0.5f;
			renderSettings.vertexBuildSettings.pivot.y = pDefinition->renderParams.pivot.vy * 0.5f;
			renderSettings.vertexBuildSettings.pivot.z = pDefinition->renderParams.pivot.vz * 0.5f;
		}
		else
		{
			renderSettings.vertexBuildSettings.pivot.x = pDefinition->renderParams.pivot.vx;
			renderSettings.vertexBuildSettings.pivot.y = pDefinition->renderParams.pivot.vy;
			renderSettings.vertexBuildSettings.pivot.z = pDefinition->renderParams.pivot.vz;
		}

		if(strlen(pDefinition->renderParams.bumpTexture) > 0)
		{
			Util::C_String bumpFilename(pDefinition->renderParams.bumpTexture);

			renderSettings.pNormalMap = DynamicEffect::XtInterface::Renderer()->LoadTexture(bumpFilename);
			
			if(renderSettings.pNormalMap)
			{
				if(renderSettings.vertexBuildSettings.eParticleShape == E_ParticleShape_SpriteRotating)
				{
					if(renderSettings.vertexBuildSettings.eLimitSize != E_ParticleLimitSize_NoLimit)
						renderSettings.vertexBuildSettings.eParticleShape = E_ParticleShape_SpriteRotatingNormalMapLimitSize;
					else
						renderSettings.vertexBuildSettings.eParticleShape = E_ParticleShape_SpriteRotatingNormalMap;
				}
			}
		}

		// choose between different sprite rotating implementations
		if(renderSettings.vertexBuildSettings.eLimitSize != E_ParticleLimitSize_NoLimit)
		{
			switch(renderSettings.vertexBuildSettings.eParticleShape)
			{
				case E_ParticleShape_SpriteRotatingSoft:
					renderSettings.vertexBuildSettings.eParticleShape = E_ParticleShape_SpriteRotatingSoftLimitSize;
					break;
				case E_ParticleShape_SpriteRotating:
					renderSettings.vertexBuildSettings.eParticleShape = E_ParticleShape_SpriteRotatingLimitSize;
					break;
				case E_ParticleShape_Sprite:
					renderSettings.vertexBuildSettings.eParticleShape = E_ParticleShape_SpriteLimitSize;
					break;
				case E_ParticleShape_QuadAngleFade:
					renderSettings.vertexBuildSettings.eParticleShape = E_ParticleShape_QuadAngleFadeLimitSize;
					break;
				default:
					break;
			}
		}

		renderSettings.cullDistance = pDefinition->renderParams.cullDistance;
		if(renderSettings.cullDistance == 0.0f)
			renderSettings.cullDistance = g_pParticleManager->GetCullDistance();
		
		DynamicEffect::GetLodFade(renderSettings.lodFade, renderSettings.cullDistance);
				
		// setup colour curve
		{
			// resize table if necessary
			S32 nSteps = 8;
			renderSettings.vertexBuildSettings.colourTable.table.Resize(nSteps);

			// if both curves are empty just create a lookup table with 2 entries
			if(pDefinition->renderParams.colourOverTime.count == 0 && pDefinition->renderParams.colourCurveA == 0)
			{
				CreateCurve(pDefinition->renderParams.colourCurveA, renderSettings.vertexBuildSettings.colourTable);
			}
			else
			{
				Curve::C_HermiteCurve red, green, blue;
				bool bSuccess = true;
				for(S32 i = 0; i < pDefinition->renderParams.colourOverTime.count; ++i)
				{
					bSuccess = bSuccess && red.AddKeyFrame(pDefinition->renderParams.colourOverTime[i].time, pDefinition->renderParams.colourOverTime[i].colour.r, 0.0f, 0.0f);
					bSuccess = bSuccess && green.AddKeyFrame(pDefinition->renderParams.colourOverTime[i].time, pDefinition->renderParams.colourOverTime[i].colour.g, 0.0f, 0.0f);
					bSuccess = bSuccess && blue.AddKeyFrame(pDefinition->renderParams.colourOverTime[i].time, pDefinition->renderParams.colourOverTime[i].colour.b, 0.0f, 0.0f);
				}

				if(!bSuccess)
					LOGWARN(g_Log, "DynamicEffect: Invalid ColourOverTime on effect '%s' (%X)", (const char*)pDefinition->name, pDefinition->id);
								
				DynamicEffect::XtInterface::CreateCurve(red, renderSettings.vertexBuildSettings.colourTable, nSteps, 0);
				DynamicEffect::XtInterface::CreateCurve(green, renderSettings.vertexBuildSettings.colourTable, nSteps, 1);
				DynamicEffect::XtInterface::CreateCurve(blue, renderSettings.vertexBuildSettings.colourTable, nSteps, 2);
				
				if(!CreateCurve(pDefinition->renderParams.colourCurveA, renderSettings.vertexBuildSettings.colourTable, nSteps, 3))
					LOGWARN(g_Log, "DynamicEffect: Invalid AlphaCurve on effect '%s' (%X)", (const char*)pDefinition->name, pDefinition->id);

			}
		}

		// create size curve
		{
			S32 nSteps = 8;
			renderSettings.vertexBuildSettings.sizeTable.table.Resize(nSteps);

			// if size curve Y and Z are empty then duplicate X to all components
			if(pDefinition->renderParams.sizeCurveY.count == 0 && pDefinition->renderParams.sizeCurveZ.count == 0)
			{
				if(pDefinition->renderParams.sizeCurveX.count == 0 && pDefinition->renderParams.animation.count == 0)
					CreateCurve(pDefinition->renderParams.sizeCurveX, renderSettings.vertexBuildSettings.sizeTable);
				else
				{
					if(!CreateCurve(pDefinition->renderParams.sizeCurveX, renderSettings.vertexBuildSettings.sizeTable, nSteps))
						LOGWARN(g_Log, "DynamicEffect: Invalid SizeCurveX on effect '%s' (%X)", (const char*)pDefinition->name, pDefinition->id);
					if(!CreateCurve(pDefinition->renderParams.animation, renderSettings.vertexBuildSettings.sizeTable, nSteps, 3))
						LOGWARN(g_Log, "DynamicEffect: Invalid AnimationCurve on effect '%s' (%X)", (const char*)pDefinition->name, pDefinition->id);
				}
			}
			else
			{
				if(!CreateCurve(pDefinition->renderParams.sizeCurveX, renderSettings.vertexBuildSettings.sizeTable, nSteps, 0))
					LOGWARN(g_Log, "DynamicEffect: Invalid SizeCurveX on effect '%s' (%X)", (const char*)pDefinition->name, pDefinition->id);
				
				if(!CreateCurve(pDefinition->renderParams.sizeCurveY, renderSettings.vertexBuildSettings.sizeTable, nSteps, 1))
					LOGWARN(g_Log, "DynamicEffect: Invalid SizeCurveY on effect '%s' (%X)", (const char*)pDefinition->name, pDefinition->id);

				if(!CreateCurve(pDefinition->renderParams.sizeCurveZ, renderSettings.vertexBuildSettings.sizeTable, nSteps, 2))
					LOGWARN(g_Log, "DynamicEffect: Invalid SizeCurveZ on effect '%s' (%X)", (const char*)pDefinition->name, pDefinition->id);
				
				if(!CreateCurve(pDefinition->renderParams.animation, renderSettings.vertexBuildSettings.sizeTable, nSteps, 3))
					LOGWARN(g_Log, "DynamicEffect: Invalid AnimationCurve on effect '%s' (%X)", (const char*)pDefinition->name, pDefinition->id);
			}
		}
	}

	DynamicEffect::Particle::PhysicsSettings physicsSettings;
	{
		physicsSettings.eBehaviour = pDefinition->physicsParams.behaviour;
		physicsSettings.eCollision = pDefinition->physicsParams.collisions;
		physicsSettings.fRestitution = 1.0f + pDefinition->physicsParams.restitution;
		physicsSettings.fFriction = pDefinition->physicsParams.friction;
		physicsSettings.bDieOnImpact = pDefinition->physicsParams.dieOnImpact;

		physicsSettings.vAcceleration = MathsStorage::Float4A16(0.0f, pDefinition->physicsParams.gravity, 0.0f, 0.0f);
		physicsSettings.fAirResistance = pDefinition->physicsParams.airResistance;
	}

	pDefinition->bInitialised = true;
	
	if(pDefinition->children.count > 0 && pDefinition->renderParams.shape != DynamicEffect::Particle::E_ParticleShape_Decal)
	{
		// super effects
		if(	pDefinition->renderParams.shape == DynamicEffect::Particle::E_ParticleShape_Cube ||
			pDefinition->renderParams.shape == DynamicEffect::Particle::E_ParticleShape_Pyramid ||
			pDefinition->renderParams.shape == DynamicEffect::Particle::E_ParticleShape_Quad ||
			pDefinition->renderParams.shape == DynamicEffect::Particle::E_ParticleShape_QuadAngleFade ||
			pDefinition->renderParams.shape == DynamicEffect::Particle::E_ParticleShape_Mesh)
		{
			// 3d shapes
			if(pDefinition->physicsParams.collisions == DynamicEffect::Particle::E_Collision_Planar || pDefinition->physicsParams.collisions == DynamicEffect::Particle::E_Collision_Full)
				pDefinition->pEffect = new DynamicEffect::Particle::Shape3DCollisionSuper(DynamicEffect::XtInterface::Renderer(), g_pParticleManager, pDefinition->id);
			else
				pDefinition->pEffect = new DynamicEffect::Particle::Shape3DSimpleSuper(DynamicEffect::XtInterface::Renderer(), g_pParticleManager, pDefinition->id);
		}
		else if(pDefinition->renderParams.shape == DynamicEffect::Particle::E_ParticleShape_SpriteRotating ||
			pDefinition->renderParams.shape == DynamicEffect::Particle::E_ParticleShape_SpriteRotatingSoft)
		{
			pDefinition->pEffect = new DynamicEffect::Particle::SpriteRotatingSuper(DynamicEffect::XtInterface::Renderer(), g_pParticleManager, pDefinition->id);
		}
		else
		{
			// 2d shapes
			if(pDefinition->physicsParams.collisions == DynamicEffect::Particle::E_Collision_Planar || pDefinition->physicsParams.collisions == DynamicEffect::Particle::E_Collision_Full)
				pDefinition->pEffect = new DynamicEffect::Particle::Shape2DCollisionSuper(DynamicEffect::XtInterface::Renderer(), g_pParticleManager, pDefinition->id);
			else
				pDefinition->pEffect = new DynamicEffect::Particle::Shape2DSimpleSuper(DynamicEffect::XtInterface::Renderer(), g_pParticleManager, pDefinition->id);
		}
		pDefinition->pEffect->Create(updateSettings, renderStates, renderSettings, physicsSettings);

		for(S32 j = 0; j < pDefinition->children.count; ++j)
		{
			ChildEmitter& childEmitter = pDefinition->children[j];
			Emitter& emitter = childEmitter.child;
			if(emitter.effect.Object())
			{
				Effect& effect = emitter.effect;
				DynamicEffect::Particle::I_ParticleEffectBase* pChild = (I_ParticleEffectBase*)g_pParticleManager->FindEffect(effect.id);
				if(!pChild)
				{
					CreateEffect(&effect);
					pChild = (I_ParticleEffectBase*)g_pParticleManager->FindEffect(effect.id);
				}
				assert(pChild);
				DynamicEffect::Particle::EmitterDefinition definition;
				definition.pEffect = pChild;
				CreateEmitter(definition, &emitter);
				definition.id = pDefinition->id;

				// if emitter type is collision then make sure its a burst emitter with zero duration
				/*if(childEmitter.type == E_EffectChild_Collision || childEmitter.type == E_EffectChild_CollisionOnce)
				{
					definition.bRepeating = false;
					definition.eSpawn = E_EmitterSpawn_Burst;
					definition.fDuration = 0.0f;
				}*/
			
				MathsStorage::Float4A16 settings(childEmitter.intensitySpeed.from, childEmitter.intensitySpeed.to - childEmitter.intensitySpeed.from, 0.0f, 0.0f);
				if(settings.y <= 0.0f)
					settings.y = 1.0f;
				
				pDefinition->pEffect->AddChild(childEmitter.type, definition, settings);
			}
		}
	}
	else
	{
		// normal effects

		if( pDefinition->renderParams.shape == DynamicEffect::Particle::E_ParticleShape_Decal)
		{
			if(pDefinition->children.count > 0)
				LOGWARN(g_Log, "DynamicEffect::Particle::XtInterface: Decal particle has children, these will not work, effect '%s' (%X)", (const char*)pDefinition->name, pDefinition->id);

			pDefinition->pEffect = new DynamicEffect::Particle::Shape3DDecal(DynamicEffect::XtInterface::Renderer(), g_pParticleManager, pDefinition->id);
		}
		else if(pDefinition->renderParams.shape == DynamicEffect::Particle::E_ParticleShape_Cube ||
				pDefinition->renderParams.shape == DynamicEffect::Particle::E_ParticleShape_Pyramid ||
				pDefinition->renderParams.shape == DynamicEffect::Particle::E_ParticleShape_Quad ||
				pDefinition->renderParams.shape == DynamicEffect::Particle::E_ParticleShape_QuadAngleFade ||
				pDefinition->renderParams.shape == DynamicEffect::Particle::E_ParticleShape_Mesh)
		{
			// 3d shapes
			if(pDefinition->physicsParams.collisions != DynamicEffect::Particle::E_Collision_None)
				pDefinition->pEffect = new DynamicEffect::Particle::Shape3DCollision(DynamicEffect::XtInterface::Renderer(), g_pParticleManager, pDefinition->id);
			else
				pDefinition->pEffect = new DynamicEffect::Particle::Shape3DSimple(DynamicEffect::XtInterface::Renderer(), g_pParticleManager, pDefinition->id);
		}
		else if(pDefinition->renderParams.shape == DynamicEffect::Particle::E_ParticleShape_SpriteRotating ||
				pDefinition->renderParams.shape == DynamicEffect::Particle::E_ParticleShape_SpriteRotatingSoft)
		{
			// rotating
			if(pDefinition->physicsParams.collisions != DynamicEffect::Particle::E_Collision_None)
			{
				pDefinition->pEffect = new DynamicEffect::Particle::SpriteRotatingCollision(DynamicEffect::XtInterface::Renderer(), g_pParticleManager, pDefinition->id);
			}
			else if(pDefinition->physicsParams.behaviour == DynamicEffect::Particle::E_PhysicsBehaviour_Default)
			{
				pDefinition->pEffect = new DynamicEffect::Particle::SpriteRotating(DynamicEffect::XtInterface::Renderer(), g_pParticleManager, pDefinition->id);
			}
			else
			{
				pDefinition->pEffect = new DynamicEffect::Particle::SpriteRotatingSpline(DynamicEffect::XtInterface::Renderer(), g_pParticleManager, pDefinition->id);
			}
		}
		else
		{
			// 2d shapes, default physics and spline follower types
			if(pDefinition->physicsParams.behaviour == DynamicEffect::Particle::E_PhysicsBehaviour_Default)
			{
				if(pDefinition->physicsParams.collisions != DynamicEffect::Particle::E_Collision_None)
					pDefinition->pEffect = new DynamicEffect::Particle::Shape2DCollision(DynamicEffect::XtInterface::Renderer(), g_pParticleManager, pDefinition->id);
				else
					pDefinition->pEffect = new DynamicEffect::Particle::Shape2DSimple(DynamicEffect::XtInterface::Renderer(), g_pParticleManager, pDefinition->id);
			}
			else
				pDefinition->pEffect = new DynamicEffect::Particle::Shape2DSpline(DynamicEffect::XtInterface::Renderer(), g_pParticleManager, pDefinition->id);
		}

		pDefinition->pEffect->Create(updateSettings, renderStates, renderSettings, physicsSettings);
	}
}

void UpdateEffects()
{
	// go through adding all effects into the system
	for(Effects::Iterator it = g_Effects.Begin(); it != g_Effects.End(); ++it)
	{
		Effect* pDefinition = it->GetValue();
		if(!pDefinition->bInitialised)
		{
			CreateEffect(pDefinition);
		}
	}

	for(Effects::Iterator it = g_Effects.Begin(); it != g_Effects.End(); ++it)
	{
		Effect* pDefinition = it->GetValue();
		assert(pDefinition->bInitialised);
	}
}

void Initialise(DynamicEffect::Particle::C_ParticleManager* pParticleManager)
{
	g_pParticleManager = pParticleManager;

	Update();
}

void Shutdown()
{
	for(Effects::Iterator it = g_Effects.Begin(); it != g_Effects.End(); ++it)
		InvalidateEffect(it->GetValue());
	for(Events::Iterator it = g_Events.Begin(); it != g_Events.End(); ++it)
		InvalidateEvent(it->GetValue());
	for(WorldEvents::Iterator it = g_WorldEvents.Begin(); it != g_WorldEvents.End(); ++it)
		InvalidateWorldEvent(it->GetValue());
	g_pParticleManager = NULL;
}

void Update()
{
	if(DynamicEffect::XtInterface::Renderer())
	{
		UpdateEffects();
		UpdateEvents();
		UpdateWorldEvents();
	}
}

void Destroy()
{
	while(g_Effects.Begin() != g_Effects.End())
	{
		Effect* pEffect = g_Effects.Begin()->GetValue();
		InvalidateEffect(pEffect);
		g_Effects.Erase(pEffect->id);
	}
	while(g_Events.Begin() != g_Events.End())
	{
		Event* pEvent = g_Events.Begin()->GetValue();
		InvalidateEvent(pEvent);
		g_Events.Erase(pEvent->id);
	}
	while(g_WorldEvents.Begin() != g_WorldEvents.End())
	{
		WorldEvent* pWorldEvent = g_WorldEvents.Begin()->GetValue();
		InvalidateWorldEvent(pWorldEvent);
		g_WorldEvents.Erase(pWorldEvent);
	}

	g_Effects.Destroy();
	g_Events.Destroy();
	g_WorldEvents.Destroy();
}

} // XtInterface
} // Particle
} // DynamicEffect





	
























