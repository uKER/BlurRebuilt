#ifndef RENDER_DYNAMICEFFECT_PARTICLE_SUPER_H
#define RENDER_DYNAMICEFFECT_PARTICLE_SUPER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Particle/Emitter.h"
#include "Dynamic/Particle/Block.h"
#include <btl/Containers/vector.h>	// [Blur2PC] Btl::Vector
#include "Dynamic/BlockManager.h"	// [Blur2PC]

namespace DynamicEffect
{
namespace Particle
{

typedef ParticleTrailBlock														TrailBlock;
typedef Btl::Vector<ParticleTrailBlock*, Btl::VectorParameters_Static<4> >		TrailPtrs;

struct TrailEffect
{
	EmitterDefinition				definition;
	AtomicVector<TrailBlock*>		blocks;
};

struct CollisionEffect
{
	EmitterDefinition			definition;
	MathsStorage::Float4A16		settings;
};

struct GenericEffect
{
	EmitterDefinition	definition;
};

struct SuperEffects
{
	Btl::Vector<TrailEffect, Btl::VectorParameters_Static<4> >	trailEffects;
	Btl::Vector<GenericEffect>									deathEffects;
	Btl::Vector<CollisionEffect>								collisionEffects;
	Btl::Vector<GenericEffect>									collisionOnceEffects;
	C_EmitterList*												pEmitterList;
	VectorBlockManager<ParticleTrailBlock>*						pTrailBlockManager;
};


} // Particle
} // DynamicEffect

#endif // RENDER_DYNAMICEFFECT_PARTICLE_SUPER_H
