#ifndef CONVEXHULLRAY_H
#define CONVEXHULLRAY_H

#include "gravity/narrowphase/Ray.h"

namespace Collision
{
bool inline CollideConvexHullRay(const C_Shape* const __restrict convex, C_Ray& ray);

bool inline CollideConvexHullFatRay(const C_Shape* const __restrict convex, C_Ray& ray);

bool inline CollideConvexHullRay(const C_Shape* const __restrict convex, C_TransparentRay& ray,
								 SharedPhysics::C_GraphNode* pGraphNode);

bool inline CollideConvexHullFatRay(const C_Shape* const __restrict shape, C_TransparentRay& ray,
									SharedPhysics::C_GraphNode* pGraphNode);
};

#include "gravity/narrowphase/ConvexHullRay.inl"
#endif
