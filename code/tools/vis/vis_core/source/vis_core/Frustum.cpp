#include "stdafx.h"

#include "Vis_Core\Frustum.h"

void Frustum::ExtractPlanes(const HMat44& matrix)
{
	m_planes[LEFT_PLANE].normal.x = (matrix.AX().W() + matrix.AX().X());
	m_planes[LEFT_PLANE].normal.y = (matrix.AY().W() + matrix.AY().X());
	m_planes[LEFT_PLANE].normal.z = (matrix.AZ().W() + matrix.AZ().X());
	m_planes[LEFT_PLANE].dist     = (matrix.AW().W() + matrix.AW().X());

	m_planes[RIGHT_PLANE].normal.x = (matrix.AX().W() - matrix.AX().X());
	m_planes[RIGHT_PLANE].normal.y = (matrix.AY().W() - matrix.AY().X());
	m_planes[RIGHT_PLANE].normal.z = (matrix.AZ().W() - matrix.AZ().X());
	m_planes[RIGHT_PLANE].dist     = (matrix.AW().W() - matrix.AW().X());

	m_planes[TOP_PLANE].normal.x = (matrix.AX().W() - matrix.AX().Y());
	m_planes[TOP_PLANE].normal.y = (matrix.AY().W() - matrix.AY().Y());
	m_planes[TOP_PLANE].normal.z = (matrix.AZ().W() - matrix.AZ().Y());
	m_planes[TOP_PLANE].dist     = (matrix.AW().W() - matrix.AW().Y());

	m_planes[BOTTOM_PLANE].normal.x = (matrix.AX().W() + matrix.AX().Y());
	m_planes[BOTTOM_PLANE].normal.y = (matrix.AY().W() + matrix.AY().Y());
	m_planes[BOTTOM_PLANE].normal.z = (matrix.AZ().W() + matrix.AZ().Y());
	m_planes[BOTTOM_PLANE].dist     = (matrix.AW().W() + matrix.AW().Y());

	m_planes[NEAR_PLANE].normal.x = (matrix.AX().Z());
	m_planes[NEAR_PLANE].normal.y = (matrix.AY().Z());
	m_planes[NEAR_PLANE].normal.z = (matrix.AZ().Z());
	m_planes[NEAR_PLANE].dist     = (matrix.AW().Z());

	m_planes[FAR_PLANE].normal.x = (matrix.AX().W() - matrix.AX().Z());
	m_planes[FAR_PLANE].normal.y = (matrix.AY().W() - matrix.AY().Z());
	m_planes[FAR_PLANE].normal.z = (matrix.AZ().W() - matrix.AZ().Z());
	m_planes[FAR_PLANE].dist     = (matrix.AW().W() - matrix.AW().Z());

	for (U32 i = 0; i < 6; i++)
		m_planes[i].Normalize();
}

// intersect frustum with a bounding box
// this function produces false positives on the frustum's edges (due to expansion), but we can live with that
bool Frustum::IntersectBox(box3f &bbox)
{
	float3 m = bbox.GetCenter();
	float3 d = bbox.GetSize();
	plane3f *p;

	for (int i=0; i<6; i++)
	{
		p = &m_planes[i];
		float NP = (float)(d[0]*fabs(p->normal.x)+d[1]*fabs(p->normal.y)+d[2]*fabs(p->normal.z));
		float MP = m[0]*p->normal.x+m[1]*p->normal.y+m[2]*p->normal.z+p->dist;
        if (MP < -NP)
			return false; // in front of clip plane
    }
    return true;                             // AABB intersects frustum
}
