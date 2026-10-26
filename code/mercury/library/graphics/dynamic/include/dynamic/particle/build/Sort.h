#ifndef RENDER_DYNAMICEFFECT_PARTICLE_PARTICLESORT_H
#define RENDER_DYNAMICEFFECT_PARTICLE_PARTICLESORT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DynamicEffect
{

struct VertexBuildConstants;
class RenderNodePtrs;

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

class ParticleArray : public Ut::UtVector<Ut::StackVector<ParticleSortInfo, 1024> > {};
extern ParticleArray g_ParticleSortList;

void SortParticles(const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, ParticleArray& sortList);

} // Particle
} // DynamicEffect

#endif // RENDER_DYNAMICEFFECT_PARTICLE_PARTICLESORT_H
