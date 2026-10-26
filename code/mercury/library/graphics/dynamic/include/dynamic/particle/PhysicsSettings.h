#ifndef RENDER_DYNAMICEFFECT_PARTICLE_PHYSICSSETTINGS_H
#define RENDER_DYNAMICEFFECT_PARTICLE_PHYSICSSETTINGS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Particle/Enums.h"

namespace DynamicEffect
{
namespace Particle
{

struct PhysicsSettings					// shared settings between all types of particle systems
{
	PhysicsSettings() : eBehaviour(E_PhysicsBehaviour_Default), eCollision(E_Collision_None), fRestitution(0.f), fFriction(0.f), fAirResistance(.0f), vertexSize(0), bDieOnImpact(false) {}
	~PhysicsSettings() {}
	
	E_PhysicsBehaviour		eBehaviour;
	E_Collision				eCollision;
	
	F32			fRestitution;				// collision response
	F32			fFriction;					// collision response
	F32			fAirResistance;

	U32			vertexSize;
	bool		bDieOnImpact;

	MathsStorage::Float4A16		vAcceleration;
};

// [Blur2PC] The velocity factor air resistance applies over one particle update of timeStep
// seconds. The source multiplies by 1 - timeStep * airResistance once per update, which depends
// on how time is split into updates: the game advances in whole 1/60 s steps (GameWorld.cpp
// m_GameStep) and particles update once per rendered frame with the steps it ran, so a frame of
// two steps decays less than two frames of one. Effect data is tuned on its platform's frame,
// g_ParticleSourceFrameRate: 60 for PC data (one step per frame); the console devices present at
// interval two (D3D9Device.cpp, GCMDevice.cpp), 30 Hz. One such frame multiplies by
// f = 1 - airResistance / rate; any timeStep gets f^(timeStep * rate), the same decay per second at
// every frame rate. At whole frames this is the platform's own result, a negative f included;
// between frames a negative f (airResistance above the rate) gives 0.
extern F32 g_ParticleSourceFrameRate;
inline F32 AirResistanceFactor(const F32 timeStep, const F32 airResistance)
{
	const F32 frames = timeStep * g_ParticleSourceFrameRate;
	const F32 frameFactor = 1.0f - airResistance / g_ParticleSourceFrameRate;
	const F32 wholeFrames = floorf(frames + 0.5f);
	if (fabsf(frames - wholeFrames) < 1.0e-3f)
		return powf(frameFactor, wholeFrames);
	return frameFactor > 0.0f ? powf(frameFactor, frames) : 0.0f;
}

}
}

#endif // RENDER_DYNAMICEFFECT_PARTICLE_PHYSICSSETTINGS_H
