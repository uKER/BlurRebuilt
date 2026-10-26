#include "stdafx.h"

#include "Impact\ContactUID.h"
#include "Impact\CollisionUtil.h"
#include "Impact\ConvexHull.h"
#include "Impact\Body.h"
#include "System/Debug/I_DebugRender.h"
#include "System/Misc/Random.h"

using namespace Maths;

namespace Impact
{

/// constructor
ConvexHull::ConvexHull()
{
	m_type = E_ShapeType_ConvexHull;
}

/// destructor
ConvexHull::~ConvexHull()
{
}

void ConvexHull::AddPoint(const HVec3 &_v)
{
	HVec3 v = _v + m_vOriginOffset;

	m_Points.PushBack(v);
	m_AABB.UpdateExtents(v);
}

/// get the local-space AABB
const C_AxisBox &ConvexHull::GetAABB() const
{
	return m_AABB;
}

/// set center of gravity
void ConvexHull::SetCenterOfMass(const Maths::HVec3 &v)
{
	m_vOriginOffset = -v;
}

/// get center of mass
const Maths::HVec3 &ConvexHull::GetCenterOfMass() const
{
	return m_vOriginOffset;
}

/// generate the plane list from the point list
void ConvexHull::CalculatePlanes()
{
	for (int i1=0; i1<m_Points.Size(); i1++)
	{
		for (int i2=0; i2<m_Points.Size(); i2++)
		{
			for (int i3=0; i3<m_Points.Size(); i3++)
			{
				if (i1 == i2 || i1 == i3 || i2 == i3)
					continue;

				C_Plane plane(m_Points[i1], m_Points[i2], m_Points[i3]);

				// does the plane already exist?
				bool bAddPlane = true;
				for (int i=0; i<m_Planes.Size(); i++)
				{
					HVec3 v = (HVec3)m_Planes[i].Normal() - (HVec3)plane.Normal();
					if (v.Length() < 0.0000001f && 
						fabs(m_Planes[i].DistanceToOrigin() - plane.DistanceToOrigin()) < 0.0001)
					{
						bAddPlane = false;
						break;
					}
				}
				if (!bAddPlane)
					continue;

				// do all points lie on one side of the plane?
				int iFront = 0, iBack = 0;
				for (int i=0; i<m_Points.Size(); i++)
				{
					if (plane.Distance(m_Points[i]) > 0.0001f)
					{
						iFront++;
					}
					else if (plane.Distance(m_Points[i]) < -0.0001f)
					{
						iBack++;
					}
				}
				if (iBack == 0 && iFront>0)
				{
					plane = -plane;
				}
				else if (iFront == 0 && iBack>0)
				{
				}
				else
					continue;

				// add the plane
				m_Planes.PushBack(plane);
			}
		}
	}
}

/// generate the polygon list from the plane list
void ConvexHull::CalculatePolygons()
{
	for (int iPlane=0; iPlane<m_Planes.Size(); iPlane++)
	{
		const C_Plane &plane = m_Planes[iPlane];
		HVec3 vCentre = -plane.Normal() * plane.DistanceToOrigin();
		HVec3 p,q;
		PlaneSpace(plane.Normal(), p, q);
		p = p * 10.f; // TODO: calculate the necessary scaling factor from the AABB
		q = q * 10.f;

		// create polygon in the plane
		C_ClipPolygon polygon;
		C_ClipPolygon tmp;
		ClipVertex vtx;

		vtx = vCentre + p + q;		polygon.AddVertex(vtx);
		vtx = vCentre + p - q;		polygon.AddVertex(vtx);
		vtx = vCentre - p - q;		polygon.AddVertex(vtx);
		vtx = vCentre - p + q;		polygon.AddVertex(vtx);
		
		// clip the polygon against all the planes
		for (int i=0; i<m_Planes.Size(); i++)
		{
			if (i == iPlane)
				continue;
			ClipPlane clipPlane( m_Planes[i] );
			clipPlane.edge = i;
			SplitPolygon(polygon, m_Planes[i], tmp);
			polygon = tmp;
		}

		// replace the vertices of the polygon with indices into the point list
		Polygon idxpoly;
		idxpoly.Resize(polygon.m_Vertices.Size());
		for (int i=0; i<polygon.m_Vertices.Size(); i++)
		{
			int iBest = 0;
			float fDist = FLT_MAX;
			for (int j=0; j<m_Points.Size(); j++)
			{
				HVec3 v = m_Points[j] - polygon.m_Vertices[i];
				float f = v.Length2();
				if (f < fDist)
				{
					iBest = j;
					fDist = f;
				}
			}

			idxpoly[i] = iBest;
		}

		// remove duplicate vertices
		for (int i=0; i<idxpoly.Size(); i++)
		{
			if (idxpoly[i] == idxpoly[(i+1)%idxpoly.Size()])
			{
				idxpoly.EraseByIndex(i);
				i--;
			}
		}

		// if the plane has no vertices for it's polygon, delete it
		if (idxpoly.Size() == 0)
		{
			m_Planes.EraseByIndex(iPlane);
			iPlane--;
			continue;
		}

		// store the polygon
		m_Polygons.PushBack(idxpoly);
	}
}

/// find the given edge on another plane. returns the index of the plane or -1 if not found
int ConvexHull::FindMatchingEdge(int iVtx1, int iVtx2, int iOrgPlane)
{
	for (int iPlane=0; iPlane<m_Planes.Size(); iPlane++)
	{
		if (iOrgPlane == iPlane)
			continue;

		Polygon &poly = m_Polygons[iPlane];

		// check each edge
		for (int iEdge1=0; iEdge1<poly.Size(); iEdge1++)
		{
			int a1 = poly[iEdge1];
			int a2 = poly[(iEdge1+1)%poly.Size()];

			// does it match?
			if ((a1 == iVtx1 && a2 == iVtx2) ||
				(a1 == iVtx2 && a2 == iVtx1))
				return iPlane;
		}
	}

	return -1;
}

/// for each plane, calculate a list of planes that bound it
void ConvexHull::CalculateSidePlanes()
{
	m_SidePlanes.Resize(m_Planes.Size());
	for (int iPlane=0; iPlane<m_Planes.Size(); iPlane++)
	{
		Polygon &poly = m_Polygons[iPlane];

		m_SidePlanes[iPlane].Resize(poly.Size());

		// for each edge, find another polygon that shares that edge
		for (int iEdge1=0; iEdge1<poly.Size(); iEdge1++)
		{
			int a1 = poly[iEdge1];
			int a2 = poly[(iEdge1+1)%poly.Size()];

			// find a matching edge
			int iMatchPlane = FindMatchingEdge(a1, a2, iPlane);
			assert(iMatchPlane != -1);

			m_SidePlanes[iPlane][iEdge1] = iMatchPlane;
		}
	}
}

/// get side plane idx
int	ConvexHullInstance::GetSidePlaneIdx(int iSupportingPlane, int iSidePlane) const
{
	return m_pParent->m_SidePlanes[iSupportingPlane][iSidePlane];
}

/// get the side-plane index ptr for a given plane index
const C_Plane &ConvexHullInstance::GetSidePlane(int iSupportingPlane, int iSidePlane) const
{
	return GetPlane(m_pParent->m_SidePlanes[iSupportingPlane][iSidePlane]);
}

/// return the number of side-planes for the given plane
int ConvexHullInstance::NumSidePlanes(int iPlane) const
{
	return m_pParent->m_SidePlanes[iPlane].Size();
}

/// returns the parent convex hull
const ConvexHull *ConvexHullInstance::GetParent() const
{
	return m_pParent;
}

/// retrieve the given polygon
void ConvexHull::GetPolygon(int i, C_ClipPolygon &poly) const
{
	int n = m_Polygons[i].Size();
	poly.m_Vertices.Resize(n);
	for (int j=0; j<n; j++)
	{
		ClipVertex &vtx = poly.m_Vertices[j];
		vtx = m_Points[m_Polygons[i][j]];
		vtx.uid.key = (i<<4) + j;
	}
}

/// create an instance of this Shape, to attach to a body
Shape *ConvexHull::Instance()
{
	return new ConvexHullInstance(this);
}

/// construct given a parent _ConvexHull_
ConvexHullInstance::ConvexHullInstance(ConvexHull *pParent)
	: m_pParent( pParent )
{
	m_type =  E_ShapeType_ConvexHull;
}

/// calculate the point on the convex hull that is farthest along the support axis
void ConvexHullInstance::ApplyTransform()
{
	if (m_bDirty)
	{
		HMat43 mtx = (HMat33)m_qRotation || m_vPos;

		m_Points.Resize(m_pParent->m_Points.Size());
		m_Planes.Resize(m_pParent->m_Planes.Size());

		// copy and transform points
		for (int i=0; i<m_Points.Size(); i++)
			m_Points[i] = m_pParent->m_Points[i] * mtx;

		// copy and transform planes
		for (int i=0; i<m_Planes.Size(); i++)
			m_Planes[i] = m_pParent->m_Planes[i] * mtx;

		m_vPos = mtx.Translation();
		m_fRadius = m_pParent->m_fRadius;
		m_bDirty = false;
	}
}

/// get the current world-space AABB for a given position and rotation
C_AxisBox ConvexHullInstance::GetAABB(const Maths::HVec3 &vPos, const Maths::C_Quaternion &qRot)
{
	return TransformBBox(m_pParent->m_AABB, vPos, qRot);
}

/// get the current world-space AABB
C_AxisBox ConvexHullInstance::GetAABB(Body *pBody)
{
	C_AxisBox bbox = GetAABB(pBody->m_vPos, pBody->m_rotation);
	return bbox;
}

/// retrieve the given polygon
void ConvexHullInstance::GetPolygon(int i, C_ClipPolygon &poly) const
{
	int n = m_pParent->m_Polygons[i].Size();
	poly.m_Vertices.Resize(n);
	for (int j=0; j<n; j++)
	{
		ClipVertex &vtx = poly.m_Vertices[j];
		vtx = m_Points[m_pParent->m_Polygons[i][j]];
		vtx.uid.key = (i<<4) + j;
	}
}

/// returns 'true' if this objects wants to be deleted by it's body when the body itself is deleted
bool ConvexHullInstance::WantDelete()
{
	return true;
}

/// generate hull data from the given list of points
void ConvexHull::Build()
{
	CalculatePlanes();

	CalculatePolygons();

	CalculateSidePlanes();

	CalculateBoundingRadius();
}


}; // namespace Impact
