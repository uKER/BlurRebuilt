#ifndef SPHERERAY_H
#define SPHERERAY_H

#include "gravity/narrowphase/Ray.h"

namespace Collision
{
NOALIAS inline bool CollideSphereRay(const C_Shape* const sphere, C_Ray& ray);

NOALIAS inline bool CollideSphereRay(const C_Shape* const sphere, C_TransparentRay& ray, SharedPhysics::C_GraphNode* pGraphNode);
};

#endif
