#ifndef RENDER_DYNAMICEFFECT_PARTICLE_PARTICLESORT_H
#define RENDER_DYNAMICEFFECT_PARTICLE_PARTICLESORT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core\containers\radix.h>

namespace DynamicEffect
{
namespace Particle
{

class Particle3D;

// sorting structures
struct ParticleSortInfo
{
	ParticleSortInfo() {}
	ParticleSortInfo(Particle3D const *particle, U32 sort) : sortKey(sort), pParticle(particle) {}

	U32 sortKey;
	Particle3D const *pParticle;
};

inline U32 RadixKey(const ParticleSortInfo& info)
{
	return info.sortKey;
}

typedef Ut::UtVector<Ut::StackVector<ParticleSortInfo, 1024> > ParticleArray;
extern ParticleArray g_ParticleSortList;

} // Particle
} // DynamicEffect

#endif // RENDER_DYNAMICEFFECT_PARTICLE_PARTICLESORT_H
