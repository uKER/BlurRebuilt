#ifndef RENDER_DYNAMICEFFECT_PARTICLE_BUILDPOLICY_JOBQUEUEDYNAMICVERTEXBUFFERBATCHER_H
#define RENDER_DYNAMICEFFECT_PARTICLE_BUILDPOLICY_JOBQUEUEDYNAMICVERTEXBUFFERBATCHER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Particle/Build/JobQueueParams.h"
#include "Dynamic/Particle/Enums.h"

namespace DynamicEffect
{
class C_View;

namespace Particle
{
namespace BuildPolicy
{

struct JobQueueDynamicVertexBufferBatch
{
	ALIGN16 Btl::Vector<VertexBuildData, Btl::VectorParameters_Static<32> >		vertexBuildData;
};

typedef Btl::Vector<JobQueueDynamicVertexBufferBatch*>	JobQueueDynamicVertexBufferBatches;

class C_JobQueueDynamicVertexBufferBatcher
{
public:
	C_JobQueueDynamicVertexBufferBatcher() {}
	~C_JobQueueDynamicVertexBufferBatcher() {}

	void AddNode(const C_View& view, const RenderNodePtrs* pBlocks, const VertexBuildSettings*	pSettings, void* pBuffer);
	void EndBuild(const C_View& view);
	void EndFrame();

private:

	JobQueueDynamicVertexBufferBatches							m_Batches[E_VertexBuild_Count];
	static Btl::DynamicPool<JobQueueDynamicVertexBufferBatch>	m_JobQueueDynamicVertexBufferBatch;
};

} // BuildPolicy
} // Particles
} // DynamicEffect

#endif
