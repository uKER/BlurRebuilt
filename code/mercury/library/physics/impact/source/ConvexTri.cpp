#include "stdafx.h"

#include "Impact\ContactUID.h"
#include "Impact\CollisionUtil.h"
#include "Impact\ConvexTri.h"
#include "System\Memory\MemoryTracker.h"

using namespace Maths;

namespace Impact
{

/// create a flat convex hull from a polygon (for trimesh collision)
void ConvexTri::SetFromPolygon(const C_Triangle &poly)
{
	m_Points.Resize(poly.Size());
	m_Planes.Resize(1);

	for (int i=0; i<poly.Size(); i++)
		m_Points[i] = poly[i];

	// create plane for triangle
	m_Planes[0] = C_Plane(poly[0], poly[1], poly[2]);

	// calculate sideplanes for triangle
	HVec3 vPrev = m_Points.Back();
	for (int i=0; i<m_Points.Size(); i++)
	{
		HVec3 vNew = m_Points[i];
		HVec3 vEdge = vPrev - vNew;
		C_Normal vNormal = C_Normal(m_Planes[0].Normal() ^ vEdge);
		m_SidePlanes[i] = C_Plane(vNormal, vNew);
		vPrev = vNew;
	}

	m_vPos = (poly[0] + poly[1] + poly[2]) * (1.f/3.f);
//	CalculateBoundingRadius();
}

/// get the side-plane index ptr for a given plane index
const Maths::C_Plane &ConvexTri::GetSidePlane(int iSupportingPlane, int iSidePlane) const
{
	return m_SidePlanes[iSidePlane];
}

/// return the number of side-planes for the given plane
int ConvexTri::NumSidePlanes(int iPlane) const
{
	return m_SidePlanes.Size();
}

/// get reference to points array from ConvexBase
Ut::Vector<Maths::HVec3> &ConvexTri::GetPoints()
{
	return m_Points;
}

/// get reference to planes array from ConvexBase
Ut::Vector<Maths::C_Plane> &ConvexTri::GetPlanes()
{
	return m_Planes;
}

/// get reference to sideplanes array from ConvexBase
Ut::UtArrayVector<Maths::C_Plane,3> &ConvexTri::GetSidePlanes()
{
	return m_SidePlanes;
}


}; // namespace Impact
