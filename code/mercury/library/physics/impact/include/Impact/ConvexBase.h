#ifndef __PHYSICS_I_CONVEXHULL_H__
#define __PHYSICS_I_CONVEXHULL_H__

#include "Impact\Shape.h"

namespace Impact
{

/// base class for a representation of a collision hull
/// used as input for GJK and SAT. ConvexHull and ConvexTri derive from this
class ConvexBase : public Shape
{
public:
	/// calculate the point on the convex hull that is farthest along the support axis
	Maths::HVec3			ProjectSupportAxis(const Maths::C_Normal &vNormal, int *pI) const;

	/// calculate the extent of the convex hull in the given direction
	Maths::HF32				ProjectExtent(const Maths::C_Normal &vNormal) const;

	/// return the number of planes
	int						NumPlanes() const { return m_Planes.Size(); }

	/// return the n'th plane
	const Maths::C_Plane	&GetPlane(int iPlane) const { return m_Planes[iPlane]; }

	/// return the number of points
	inline int				NumPoints() const { return m_Points.Size(); }

	/// retrieve the given point
	inline const Maths::HVec3 &GetPoint(int i) const { return m_Points[i]; }

	/// retrieve the convex hull origin in world-space
	const Maths::HVec3		&Origin() const { return m_vPos; }

	/// retrieve the bounding sphere radios
	Maths::HF32				Radius() const { return Maths::Load<Maths::HF32>(&m_fRadius); }

protected:
	/// calculate bounding radius
	void					CalculateBoundingRadius();

	/// perform ProjectSupportAxis using SIMD
	Maths::HVec3			ProjectSupportAxisSIMD8(const Maths::C_Normal &vNormal, int *pI) const;

	Ut::Vector<Maths::HVec3>	m_Points;
	Ut::Vector<Maths::C_Plane>	m_Planes;
	float						m_fRadius;
};

};

#endif
