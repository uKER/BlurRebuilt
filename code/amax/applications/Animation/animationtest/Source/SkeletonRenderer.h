#ifndef SKELETONRENDERER_H
#define SKELETONRENDERER_H

#include <system/scheduler/jobqueue.h>

class C_Actor;

namespace Animation
{
	class C_Skeleton;
	class C_JointTransform;
}

struct C_SkeletonVertex
{
	F32 x, y, z;
	U32 rgba;
};

void RenderSkeletonVertices(C_SkeletonVertex*& pVertex,
					const Animation::C_Skeleton& skeleton,
					const Maths::Mat43* worldMatrices,
					const Maths::RGBA& colour
);

struct ALIGN16 C_SkeletonRenderParams
{
	C_SkeletonVertex* m_vertices;
	U32 m_vertexBufferSize;
	U16 m_numActors;
	U16 m_firstActorIndex;
	const C_Actor*	m_actors;
};

extern Scheduler::jqModule SkeletonRenderModule;

void SkeletonRenderTask( struct Scheduler::Task* task );

#endif


