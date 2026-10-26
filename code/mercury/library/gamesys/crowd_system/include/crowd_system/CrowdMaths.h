#ifndef CROWD_MATHS_H
#define CROWD_MATHS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace AnimatedCrowd
{

void	CrossFadeLinear(const F32 t, F32& x, F32& y);
void	CrossFadeCosine(const F32 t, F32& x, F32& y);
F32		EaseInEaseOut(const F32 t);

F32		DistancePointAABBSqr(const Maths::C_AxisBox& aabb, const Maths::HVec3& point);

Maths::HF32			PlaneDistance(const Maths::C_Plane& plane, const Maths::HVec3& point);
Maths::E_PlaneTest	PlaneTestAABB(const Maths::C_Plane& plane, const Maths::AABB& axisBox);

bool	AABBSphereIntersect(const Maths::C_AxisBox& axisBox, const Maths::HVec3& position, const F32 radius);
bool	AABBCylinderIntersect(const Maths::C_AxisBox& axisBox, const Maths::HVec3& position, const F32 radius, const F32 halfHeight);

}; // namespace AnimatedCrowd

#include "CrowdMaths.inl"

#endif // CROWD_MATHS_H
