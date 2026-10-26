#ifndef CROWD_MATHS_INL
#define CROWD_MATHS_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace AnimatedCrowd
{

inline F32 EaseInEaseOut(const F32 t)
{
	//return 0.5f + 0.5f * Maths::SmoothStep<F32>(3.141593f * (1.f - t));
	return Maths::SmoothStep<F32>(t);
}

inline F32 DistancePointAABBSqr(const Maths::C_AxisBox& aabb, const Maths::HVec3& point)
{
	F32 sqrDist = 0.f;

	const Maths::HVec3& vMin(aabb.Min());
	const Maths::HVec3& vMax(aabb.Max());

	sqrDist += (point[0] < vMin[0]) ? (vMin[0] - point[0]) * (vMin[0] - point[0]) : 0.f;
	sqrDist += (point[0] > vMax[0]) ? (point[0] - vMax[0]) * (point[0] - vMax[0]) : 0.f;

	sqrDist += (point[1] < vMin[1]) ? (vMin[1] - point[1]) * (vMin[1] - point[1]) : 0.f;
	sqrDist += (point[1] > vMax[1]) ? (point[1] - vMax[1]) * (point[1] - vMax[1]) : 0.f;

	sqrDist += (point[2] < vMin[2]) ? (vMin[2] - point[2]) * (vMin[2] - point[2]) : 0.f;
	sqrDist += (point[2] > vMax[2]) ? (point[2] - vMax[2]) * (point[2] - vMax[2]) : 0.f;

	return sqrDist;
}

inline Maths::HF32 PlaneDistance(const Maths::C_Plane& plane, const Maths::HVec3& point)
{
	return (plane.m_V * (Maths::HVec3(point) || Maths::HVecW(Maths::HVec4(0.f, 0.f, 0.f, 1.f)))).Sum();
}

inline Maths::E_PlaneTest PlaneTestAABB(const Maths::C_Plane& plane, const Maths::C_AxisBox& axisbox)
{
	Maths::HVec3 vertex[8];
    axisbox.GetVertexArray(vertex);
    
	Maths::E_PlaneTest result = Maths::E_PlaneTestInvalid;
    
    for (U32 i = 0; i < 8; ++i)
    {
		const Maths::HF32 d = PlaneDistance(plane, vertex[i]);
		if (Maths::Abs(d) >= 0.f)
        {
            if (d >= 0.f)
				result = (Maths::E_PlaneTest)( result | Maths::E_PlaneTestFront );
            else
				result = (Maths::E_PlaneTest)( result | Maths::E_PlaneTestBack );
            
            if (result == Maths::E_PlaneTestIntersect)
                return Maths::E_PlaneTestIntersect;
        }
    }
    
    return result;
}

inline bool AABBSphereIntersect(const Maths::C_AxisBox& axisBox, const Maths::HVec3& position, const F32 radius)
{
	if (DistancePointAABBSqr(axisBox, position) <= (radius * radius))
		return true;
	return false;
}

inline bool AABBCylinderIntersect(const Maths::C_AxisBox& axisBox, const Maths::HVec3& position, const F32 radius, const F32 halfHeight)
{
	Maths::C_AxisBox axisBoxXZ(Maths::HVec3(axisBox.Min().X(), 0.f, axisBox.Min().Z()), Maths::HVec3(axisBox.Max().X(), 0.f, axisBox.Max().Z()));

	if (DistancePointAABBSqr(axisBoxXZ, Maths::HVec3(position.X(), 0.f, position.Z())) <= (radius * radius))
	{
		Maths::C_Plane basePlane(Maths::C_Normal(0.f, 1.f, 0.f), Maths::C_Vertex(position.X(), position.Y() - halfHeight, position.Z()));
		Maths::C_Plane topPlane(Maths::C_Normal(0.f, -1.f, 0.f), Maths::C_Vertex(position.X(), position.Y() + halfHeight, position.Z()));
		
		if (PlaneTestAABB(topPlane, axisBox) == Maths::E_PlaneTestBack || PlaneTestAABB(basePlane, axisBox) == Maths::E_PlaneTestBack)
			return false;
		
		return true;
	}
	
	return false;
}

}; // namespace AnimatedCrowd

#endif // CROWD_MATHS_INL
