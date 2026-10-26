#ifndef RENDER_DYNAMICEFFECT_PARTICLE_BUILDPOLICY_JOBQUEUEPARAMS_H
#define RENDER_DYNAMICEFFECT_PARTICLE_BUILDPOLICY_JOBQUEUEPARAMS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DynamicEffect
{
class RenderNodePtrs;
struct VertexBuildConstants;

namespace Particle
{
struct VertexBuildSettings;

namespace BuildPolicy
{

struct VertexBuildData
{
	const RenderNodePtrs*		pBlocks;
	const VertexBuildSettings*	pSettings;
	void*						pBuffer;
};

struct VertexBuildParams
{
	VertexBuildData*			pVertexBuildDataArray;
	S32							vertexBuildDataCount;
	const VertexBuildConstants*	pConstants;
};

} // BuildPolicy
} // Particle
} // DynamicEffect

#endif // RENDER_DYNAMICEFFECT_PARTICLE_BUILDPOLICY_JOBQUEUEPARAMS_H
