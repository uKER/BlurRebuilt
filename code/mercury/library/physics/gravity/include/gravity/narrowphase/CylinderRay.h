#ifndef CYLINDERRAY_H
#define CYLINDERRAY_H

#include "gravity/narrowphase/Ray.h"

namespace Collision
{
NOALIAS inline bool CollideCylinderRay(const C_Shape* const __restrict cylinder, C_Ray& ray);

NOALIAS inline bool CollideCylinderRay(const C_Shape* const __restrict cylinder, C_TransparentRay& ray);
};

#include "gravity/narrowphase/CylinderRay.inl"

#endif
