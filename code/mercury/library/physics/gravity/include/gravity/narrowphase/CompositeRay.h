#ifndef COMPOSITERAY_H
#define COMPOSITERAY_H

#include "gravity/narrowphase/Ray.h"

namespace Collision
{
	bool inline CollideCompositeRay(const C_Shape* const composite, C_Ray& ray, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache);

	bool inline CollideCompositeRay(const C_Shape* const composite, C_TransparentRay& ray, SharedPhysics::C_SafeStack<true>& stack, C_CollisionFilter* pRayFilter, SharedPhysics::C_SmallSPUCache& spuCache,
		SharedPhysics::C_GraphNode* pGraphNode);
};

#include "gravity/narrowphase/CompositeRay.inl"

#endif
