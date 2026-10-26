#include "stdafx.h"
#include "racingphysics/collisions/collisioncache.h"
#include "system/debuglog.h"

#include "racingphysics/debug/draw.h"
#include "display/render/basicrender.h"

NOALIAS FORCE_INLINE S32 SelectIf(const S32 test, const S32 result_if_test_NE_zero, const S32 result_if_test_EQ_zero )
{
    const U32 mask = ( test != 0 ) ? 0 : -1;
    return result_if_test_NE_zero + ( ( result_if_test_EQ_zero - result_if_test_NE_zero ) & mask );
}

//using namespace Maths;

namespace RacingPhysics
{
//#pragma optimize("", off)

//////////////////////////////////////////////

int g_maxVertices = 0;
int g_maxEdges = 0;
int g_maxTriangles = 0;
int g_findMatches = 1;


CollisionCache::CollisionCache() :	m_state(STATE_EMPTY), m_type(CACHE_TYPE_FULL)
{
	m_boundingBox.SetMinMax(Maths::Constants<Point3R>::NegativeMax(), Maths::Constants<Point3R>::Max());
}

void CollisionCache::SetCacheType(CollisionCache::CacheType type)
{
	RP_ASSERT(m_state == STATE_EMPTY);
	m_type = type;
}

void CollisionCache::SetBoundingBoxMinMax(const Point3R& min, const Point3R& max) 
{
	m_boundingBox.SetMinMax(min, max);
} 

void CollisionCache::ReserveTriangles(int n)
{
	m_triangleVertexIndices.Reserve(n);
	m_triangleEdgeIndices.Reserve(n);
	m_triangleIndices.Reserve(n);

	m_triangleVertices.Reserve(n);
	m_triangleEdges.Reserve(n);
	m_triangles.Reserve(n);
}

void CollisionCache::ReserveVerticalEdges(int n) 
{
	m_verticalSurfaceIndices.Reserve(n); 
	m_verticalSurfaces.Reserve(n);
}

void CollisionCache::Clear()
{
	m_triangleVertexIndices.Clear();
	m_triangleEdgeIndices.Clear();
	m_triangleIndices.Clear();

	m_triangleVertices.Clear();
	m_triangleEdges.Clear();
	m_triangles.Clear();

	m_verticalSurfaceIndices.Clear();
	m_verticalSurfaces.Clear();

	m_boundingBox.SetMinMax(Maths::Constants<Point3R>::NegativeMax(), Maths::Constants<Point3R>::Max());

	m_state = STATE_EMPTY;
}

void CollisionCache::AddTriangleNoSharing(Point3R p0, Point3R p1, Point3R p2, U32 surfaceFlags)
{
	RP_ASSERT(m_state != STATE_READY);
	m_state = STATE_BUILDING;

	Normal3R triangleNormal;

	// triangle
	{
		m_triangles.PushBack();
		m_triangles.Back().vertex0 = p0;
		m_triangles.Back().vertex1 = p1;
		m_triangles.Back().vertex2 = p2;

		triangleNormal = Normalised((p0 - p1) ^ (p2 - p1));
		m_triangles.Back().normal = triangleNormal;
		m_triangles.Back().surfaceFlags = surfaceFlags;
	}
}

void CollisionCache::AddTriangleForImpactTrimeshAdapter(
								 int i0, int i1, int i2,
								 U32 surfaceFlags)
{
	RP_ASSERT(m_state != STATE_READY);
	m_state = STATE_BUILDING;

	Normal3R triangleNormal;

	// triangle
	{
		TriangleIndices indices;

		indices.i0 = i0;
		indices.i1 = i1;
		indices.i2 = i2;

		for (int i = 0; i < m_triangleIndices.Size(); i++)
		{
			if (m_triangleIndices[i].i0 == indices.i0
				&& m_triangleIndices[i].i1 == indices.i1
				&& m_triangleIndices[i].i2 == indices.i2)
				return;
		}

		m_triangleIndices.PushBack(indices);
	}
}


void CollisionCache::AddTriangle(Point3R p0, Point3R p1, Point3R p2,
								 int i0, int i1, int i2,
								 int useEdge0, int useEdge1, int useEdge2,
								 int useVertex0, int useVertex1, int useVertex2,
								 U32 surfaceFlags)
{
	RP_ASSERT(m_state != STATE_READY);
	m_state = STATE_BUILDING;

	Vector3R a = (p0 - p1) ^ (p2 - p1);

	if (Magnitude2(a) == 0.f)
	{
		LOGWARN( g_RacingPhysicsLog, "Tried to add degenerate triangle to cache.");
		return;
	}

	Normal3R triangleNormal = Normalised(a);

	// triangle
	{
		TriangleIndices indices;

		indices.i0 = Min(Min(i0, i1), i2);
		indices.i1 = Max(Min(i0, i1), i2);
		indices.i2 = Max(indices.i1, Max(i0, i1));
		indices.i1 = Min(indices.i1, Max(i0, i1));

		for (int i = 0; i < m_triangleIndices.Size(); i++)
		{
			if (m_triangleIndices[i].i0 == indices.i0
				&& m_triangleIndices[i].i1 == indices.i1
				&& m_triangleIndices[i].i2 == indices.i2)
				return;
		}

		m_triangleIndices.PushBack(indices);
		m_triangles.PushBack();
		m_triangles.Back().vertex0 = p0;
		m_triangles.Back().vertex1 = p1;
		m_triangles.Back().vertex2 = p2;

		m_triangles.Back().normal = triangleNormal;
		m_triangles.Back().surfaceFlags = surfaceFlags;
	}

	// edges
	if (m_type == CACHE_TYPE_FULL)
	{
		int foundEdge0, foundEdge1, foundEdge2;
		foundEdge0 = foundEdge1 = foundEdge2 = 0;

		int foundReverseEdge0, foundReverseEdge1, foundReverseEdge2;
		foundReverseEdge0 = foundReverseEdge1 = foundReverseEdge2 = -1;

		if (g_findMatches)
		{
			int n = m_triangleEdgeIndices.Size();

			if (n > 0)
			{
				Btl::Pair<int, int>* pte = &m_triangleEdgeIndices[0];

				for (int i = 0; i < n; i++, pte++)
				{
					DCBT(0x080, pte);

					int ie0 = pte->first;
					int ie1 = pte->second;

					foundEdge0 |= int(ie0 == i0) & int(ie1 == i1);
					foundEdge1 |= int(ie0 == i1) & int(ie1 == i2);
					foundEdge2 |= int(ie0 == i2) & int(ie1 == i0);

					foundReverseEdge0 = SelectIf((int(ie0 == i1) & int(ie1 == i0)), i, foundReverseEdge0);
					foundReverseEdge1 = SelectIf((int(ie0 == i2) & int(ie1 == i1)), i, foundReverseEdge1);
					foundReverseEdge2 = SelectIf((int(ie0 == i0) & int(ie1 == i2)), i, foundReverseEdge2);
				}
			}
		}

		if (useEdge0)
		{
			if (foundReverseEdge0 > -1)
			{
				m_triangleEdges[foundReverseEdge0].faceNormal1 = triangleNormal;
			}
			else if (foundEdge0 == 0)
			{
				m_triangleEdges.PushBack();
				m_triangleEdges.Back().vertex0 = p0;
				m_triangleEdges.Back().vertex1 = p1;
				m_triangleEdges.Back().direction = NormalLength(p1 - p0);
				m_triangleEdges.Back().faceNormal0 = triangleNormal;
				m_triangleEdges.Back().faceNormal1 = MakeNormal(Vector3R(0.f, 0.f, 0.f));
				m_triangleEdges.Back().surfaceFlags = surfaceFlags;
				m_triangleEdgeIndices.PushBack(Btl::MakePair(i0, i1));
			}
		}

		if (useEdge1)
		{
			if (foundReverseEdge1 > -1)
			{
				m_triangleEdges[foundReverseEdge1].faceNormal1 = triangleNormal;
			}
			else if (foundEdge1 == 0)
			{
				m_triangleEdges.PushBack();
				m_triangleEdges.Back().vertex0 = p1;
				m_triangleEdges.Back().vertex1 = p2;
				m_triangleEdges.Back().direction = NormalLength(p2 - p1);
				m_triangleEdges.Back().faceNormal0 = triangleNormal;
				m_triangleEdges.Back().faceNormal1 = MakeNormal(Vector3R(0.f, 0.f, 0.f));
				m_triangleEdges.Back().surfaceFlags = surfaceFlags;
				m_triangleEdgeIndices.PushBack(Btl::MakePair(i1, i2));
			}
		}

		if (useEdge2)
		{
			if (foundReverseEdge2 > -1)
			{
				m_triangleEdges[foundReverseEdge2].faceNormal1 = triangleNormal;
			}
			else if (foundEdge2 == 0)
			{
				m_triangleEdges.PushBack();
				m_triangleEdges.Back().vertex0 = p2;
				m_triangleEdges.Back().vertex1 = p0;
				m_triangleEdges.Back().direction = NormalLength(p0 - p2);
				m_triangleEdges.Back().faceNormal0 = triangleNormal;
				m_triangleEdges.Back().faceNormal1 = MakeNormal(Vector3R(0.f, 0.f, 0.f));
				m_triangleEdgeIndices.PushBack(Btl::MakePair(i2, i0));
			}
		}
	}

	// vertices
	if (m_type == CACHE_TYPE_FULL)
	{
		int foundVertex0, foundVertex1, foundVertex2;
		foundVertex0 = foundVertex1 = foundVertex2 = 0;

		if (g_findMatches)
		{
			int n = m_triangleVertexIndices.Size();

			if (n > 0)
			{
				int* ptv = & m_triangleVertexIndices[0];

				for (int i = 0; i < n; i++, ptv++)
				{
					DCBT(0x080, ptv);

					int index = *ptv;

					foundVertex0 |= int(index == i0);
					foundVertex1 |= int(index == i1);
					foundVertex2 |= int(index == i2);
				}
			}
		}

		if (useVertex0 && !foundVertex0)
		{
			m_triangleVertices.PushBack(TriangleVertex(p0, surfaceFlags));
			m_triangleVertexIndices.PushBack(i0);
		}

		if (useVertex1 && !foundVertex1)
		{
			m_triangleVertices.PushBack(TriangleVertex(p1, surfaceFlags));
			m_triangleVertexIndices.PushBack(i1);
		}

		if (useVertex2 && !foundVertex2)
		{
			m_triangleVertices.PushBack(TriangleVertex(p2, surfaceFlags));
			m_triangleVertexIndices.PushBack(i2);
		}
	}
}

void CollisionCache::Finish()
{
	RP_ASSERT(m_state != STATE_READY);

	if (m_type == CACHE_TYPE_FULL)
	{
		// remove concave edges
		int iDst = 0;
		int iSrc = 0;

		while (iSrc < m_triangleEdges.Size())
		{
			TriangleEdge& triangleEdge(m_triangleEdges[iSrc]);

			Vector3R faceDirection = triangleEdge.faceNormal0 ^ triangleEdge.direction.Normal();

			if (MakeVector(triangleEdge.faceNormal0) == Maths::Constants<Vector3R>::Zero() 
				| MakeVector(triangleEdge.faceNormal1) == Maths::Constants<Vector3R>::Zero() 
				| (faceDirection | triangleEdge.faceNormal1) > 0.f) // keep edge
			{
				m_triangleEdges[iDst] = m_triangleEdges[iSrc];

				if (MakeVector(triangleEdge.faceNormal0) == Maths::Constants<Vector3R>::Zero()) 
					m_triangleEdges[iDst].faceNormal0 = m_triangleEdges[iDst].faceNormal1;

				if (MakeVector(triangleEdge.faceNormal1) == Maths::Constants<Vector3R>::Zero())
					m_triangleEdges[iDst].faceNormal1 = m_triangleEdges[iDst].faceNormal0;

				iDst++;
			}

			iSrc++;
		}

		m_triangleEdges.Resize(iDst);
	}

	g_maxVertices = Maths::Max(g_maxVertices, m_triangleVertices.Size());
	g_maxEdges = Maths::Max(g_maxEdges, m_triangleEdges.Size());
	g_maxTriangles = Maths::Max(g_maxTriangles, m_triangles.Size());

	m_triangleIndices.Clear();
	m_triangleEdgeIndices.Clear();
	m_triangleVertexIndices.Clear();
	m_verticalSurfaceIndices.Clear();

	m_state = STATE_READY;
}

void CollisionCache::AddVerticalEdge(Point3R p0, Point3R p1, int i0, int i1, U32 surfaceFlags)
{
	RP_ASSERT(m_state != STATE_READY);
	m_state = STATE_BUILDING;

	Btl::Pair<int, int> indices;
	indices.first = Min(i0, i1);
	indices.second = Max(i0, i1);

	for (int i = 0; i < m_verticalSurfaceIndices.Size(); i++)
	{
		if (indices == m_verticalSurfaceIndices[i])
			return;
	}

	m_verticalSurfaceIndices.PushBack(indices);
	m_verticalSurfaces.PushBack();
	m_verticalSurfaces.Back().Initialise(p0, p1, surfaceFlags);
}

///////////////////////////////////////////////////////////////

void CollisionCache::Display() const
{
	RP_ASSERT(m_state == STATE_READY);

	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetDepthEnable(Display::E_Bool_False);
	Display::Device()->SetMaxBeginVerticesUsage(100000);

	Display::BasicRender()->BindColoured3D();

	DrawPrimitive::Instance().Begin(Display::E_PrimitiveType_LineList);
	DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Red);

	for (int i = 0; i < m_triangles.Size(); i++)
	{
		Point3R pt0 = m_triangles[i].vertex0;
		Point3R pt1 = m_triangles[i].vertex1;
		Point3R pt2 = m_triangles[i].vertex2;

		DrawPrimitive::Instance().SetVertexDataXYZ(pt0.X(), pt0.Y(), pt0.Z());
		DrawPrimitive::Instance().SetVertexDataXYZ(pt1.X(), pt1.Y(), pt1.Z());

		DrawPrimitive::Instance().SetVertexDataXYZ(pt1.X(), pt1.Y(), pt1.Z());
		DrawPrimitive::Instance().SetVertexDataXYZ(pt2.X(), pt2.Y(), pt2.Z());

		DrawPrimitive::Instance().SetVertexDataXYZ(pt2.X(), pt2.Y(), pt2.Z());
		DrawPrimitive::Instance().SetVertexDataXYZ(pt0.X(), pt0.Y(), pt0.Z());
	}

	DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Magenta);

	for (int i = 0; i < m_triangleEdges.Size(); i++)
	{
		Point3R pe0 = m_triangleEdges[i].vertex0;
		Point3R pe1 = m_triangleEdges[i].vertex1;

		DrawPrimitive::Instance().SetVertexDataXYZ(pe0.X(), pe0.Y(), pe0.Z());
		DrawPrimitive::Instance().SetVertexDataXYZ(pe1.X(), pe1.Y(), pe1.Z());
	}

	DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Green);

	for (int i = 0; i < m_triangleVertices.Size(); i++)
	{
		Point3R p = m_triangleVertices[i].position;

		DrawPrimitive::Instance().SetVertexDataXYZ(p.X() - 0.05f, p.Y(), p.Z());
		DrawPrimitive::Instance().SetVertexDataXYZ(p.X() + 0.05f, p.Y(), p.Z());

		DrawPrimitive::Instance().SetVertexDataXYZ(p.X(), p.Y() - 0.05f, p.Z());
		DrawPrimitive::Instance().SetVertexDataXYZ(p.X(), p.Y() + 0.05f, p.Z());

		DrawPrimitive::Instance().SetVertexDataXYZ(p.X(), p.Y(), p.Z() - 0.05f);
		DrawPrimitive::Instance().SetVertexDataXYZ(p.X(), p.Y(), p.Z() + 0.05f);
	}

	DrawPrimitive::Instance().SetVertexDataColor(ColourARGB::Yellow);

	for (int i = 0; i < m_verticalSurfaces.Size(); i++)
	{
		Point3R p0 = m_verticalSurfaces[i].vertex0;
		Point3R p1 = m_verticalSurfaces[i].vertex1;
		Point3R c = MakePoint((MakeVector(p0) + MakeVector(p1)) * 0.5f);
		Normal3R n = m_verticalSurfaces[i].normal;

		DrawPrimitive::Instance().SetVertexDataXYZ(p0.X(), m_boundingBox.GetCentre().Y(), p0.Z());
		DrawPrimitive::Instance().SetVertexDataXYZ(p1.X(), m_boundingBox.GetCentre().Y(), p1.Z());

		DrawPrimitive::Instance().SetVertexDataXYZ(p0.X(), m_boundingBox.GetCentre().Y() - 0.5f, p0.Z());
		DrawPrimitive::Instance().SetVertexDataXYZ(p0.X(), m_boundingBox.GetCentre().Y() + 0.5f, p0.Z());

		DrawPrimitive::Instance().SetVertexDataXYZ(p1.X(), m_boundingBox.GetCentre().Y() - 0.5f, p1.Z());
		DrawPrimitive::Instance().SetVertexDataXYZ(p1.X(), m_boundingBox.GetCentre().Y() + 0.5f, p1.Z());

		DrawPrimitive::Instance().SetVertexDataXYZ(c.X(), m_boundingBox.GetCentre().Y(), c.Z());
		DrawPrimitive::Instance().SetVertexDataXYZ(c.X() + n.X(), m_boundingBox.GetCentre().Y(), c.Z() + n.Z());
	}

	DrawPrimitive::Instance().End();
}

void CollisionCache::DisplayTriangles(float collisionMeshBias) const
{
	/*RP_ASSERT(m_state == STATE_READY);

	if (m_triangles.Size() > 0)
	{
		DrawPrimitive::Instance().Begin(D3DPT_TRIANGLELIST, m_triangles.Size() * 3);

		if(DrawPrimitive::Instance().BeginValid())
		{
			for (int i = 0; i < m_triangles.Size(); i++)
			{
				Point3R pt0 = m_triangles[i].vertex0;
				Point3R pt1 = m_triangles[i].vertex1;
				Point3R pt2 = m_triangles[i].vertex2;

				DrawPrimitive::Instance().SetVertexDataXYZ(D3DVSDE_VERTEX,	pt0.X(), pt0.Y() + collisionMeshBias,	pt0.Z());
				DrawPrimitive::Instance().SetVertexDataXYZ(D3DVSDE_VERTEX,	pt1.X(), pt1.Y() + collisionMeshBias,	pt1.Z());
				DrawPrimitive::Instance().SetVertexDataXYZ(D3DVSDE_VERTEX,	pt2.X(), pt2.Y() + collisionMeshBias,	pt2.Z());
			}

			DrawPrimitive::Instance().End();
		}
	}*/
}

int CollisionCache::GetMemoryUsage() const
{	
	int usage = 0;

	usage += sizeof(m_state);
	usage += sizeof(m_type);

	usage += sizeof(m_boundingBox);

	usage += sizeof(int) * m_triangleVertexIndices.Capacity();
	usage += sizeof(Btl::Pair<int, int>) * m_triangleEdgeIndices.Capacity();
	usage += sizeof(TriangleIndices) * m_triangleIndices.Capacity();

	usage += sizeof(TriangleVertex) * m_triangleVertices.Capacity();
	usage += sizeof(TriangleEdge) * m_triangleEdges.Capacity();
	usage += sizeof(Triangle) * m_triangles.Capacity();

	usage += sizeof(Btl::Pair<int, int>) * m_verticalSurfaceIndices.Capacity();
	usage += sizeof(VerticalSurface) * m_verticalSurfaces.Capacity();

	return usage;
}

}

