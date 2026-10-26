#ifndef RENDER_DYNAMICEFFECT_PARTICLE_RENDERPOLICY_QUADANGLEFADELIMITSIZE_H
#define RENDER_DYNAMICEFFECT_PARTICLE_RENDERPOLICY_QUADANGLEFADELIMITSIZE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ShapePolicyInterface.h"
#include "Dynamic/TextureCoords.h"
#include "Dynamic/Particle/Shape/Quad.h"

namespace DynamicEffect
{
namespace Particle
{

namespace ShapePolicy
{

class QuadAngleFadeLimitSize : public Quad
{
public:
	using Quad::ParticleVertex;
	using Quad::ParticleVertexPtr;
	static void*	Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer);
	static void*	Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const ParticleArray& sortedList, volatile void* __restrict pBuffer) { return (void*)pBuffer; }
};


}
}
}

#endif
