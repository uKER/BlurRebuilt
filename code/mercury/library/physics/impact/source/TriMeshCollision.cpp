#include "stdafx.h"

#include "Impact\ContactUID.h"
#include "Impact\Body.h"
#include "Impact\Collision.h"
#include "Impact\CollisionUtil.h"
#include "Impact\GJK.h"
#include "Impact\SAT.h"
#include "Impact\ConvexHull.h"
#include "Impact\ConvexTri.h"
#include "Impact\PersistentData.h"
#include "Impact\I_TriMesh.h"
#include "Impact\TriMeshCollision.h"
#include "System\Memory\MemoryTracker.h"
#include "System/Debug/I_DebugRender.h"

using namespace Maths;

namespace Impact
{

extern Debug::Bool g_bDebugRenderManifold;
Debug::Bool g_bRenderTouchedPolygons("Impact.TriCache.RenderTouchedTris", false);
Debug::Bool g_bDrawTriMeshCache("Impact.TriCache.RenderCache", false);
Debug::Int g_iRenderTouchedPoly("Impact.TriCache.RenderTouchedPoly", 0, Debug::Range(0,10));
Debug::Bool g_bForceTriCacheUpdate("Impact.TriCache.ForceCacheUpdate", false);
Debug::Bool g_bDontAddTrisToFarList("Impact.TriCache.DontAddTrisToFarList", false);

TriMeshCollision::TriMeshCollision()
:	m_fMaxMoveDistance2(0),
	m_vBasePos(ZERO_INIT)
{
}

// return the cached separating plane
C_Plane TriPersistentData::GetSeparatingPlane()
{
	HVec4 v(-normal);
	v.W(fPlane);
	return C_Plane(v);
}

int g_nTrisRejected = 0;
int g_nTrisProcessed = 0;

/// incident polygon against the reference polygon and emit contact points
template<typename T>
int TriMeshCollision::ClipPolyAndGenerateContacts(ContactVector &contacts, T *pRefHull, C_ClipPolygon &incidentPoly, int iRefPlane, int iFlip)
{
	const C_Plane &refPlane = pRefHull->GetPlane(iRefPlane);

	C_ClipPolygon clippedPoly;
	ClipPolyToPlanes(clippedPoly, pRefHull, incidentPoly, iRefPlane);

	if (g_bDebugRenderManifold)
	{
		DrawPoly(clippedPoly);
	}

	EmitContactPoints(contacts, clippedPoly, refPlane, iFlip);

	return contacts.Size();
}

inline const HVec3 VAbs(const HVec3 &v)
{
	return HVec3(fabs(v.X()), fabs(v.Y()), fabs(v.Z()));
}

/// calculate minimum angle between O->vPos and O->plane
float CalcMinRotationAngle(const HVec3 &vP, TriPersistentData *pPersist)
{
	// calculate axis of rotation
	HVec3 vO = pPersist->vBasePos;
	HVec3 vOP = vP - vO;
	C_Normal vRotAxis = C_Normal(pPersist->normal ^ vOP);

	// calculate the intersection of the plane of rotation and the separation plane
	C_Plane rotPlane(vRotAxis, vO);
	C_Vertex linePoint;
	C_Normal lineNormal;
	IntersectPlanes(rotPlane, pPersist->GetSeparatingPlane(), linePoint, lineNormal);

	// calculate the intersection of the rotation sphere with the line on the separation plane
	C_Sphere rotSphere(vO, vOP.Length());
	C_Line line(linePoint, (C_Vector)lineNormal);
	C_QuadraticResult result = rotSphere.Intersect(line);
	assert(result.Hit());
	if (!result.Hit())
	{
		pPersist->fMaxRotationAngle = FLT_MAX; // no rotation possible
		// reduce translation by the object radius
	}
	
	// calculate angle using dot-product
	C_Normal vn = C_Normal(line(result.T0()) - (C_Vertex)pPersist->vBasePos);
	C_Normal vOPn(vOP);
	HF32 fAngle = vn | vOPn;
	vn = C_Normal(line(result.T0()) - (C_Vertex)pPersist->vBasePos);
	fAngle.Min(vn | vOPn);

	return fAngle;
}

/// calculate maximum allowed movement before we have to re-test the separating plane
void TriMeshCollision::CalcMaxMoveDistanceAndRotationAngle(ConvexHullInstance *pHull, TriPersistentData *pPersist, int &iCurTri)
{
	// recalculate base-pose + move distance
	pPersist->vBasePos = pHull->Origin();

	HF32 f = pPersist->GetSeparatingPlane().Distance(pHull->Origin()) + pHull->Radius();
	if (f < HF32(ZERO_INIT))
	{
#ifndef CFG_SPEED
		if (!g_bDontAddTrisToFarList)
#endif
		{
			// add to far list
			m_FarTris.PushBack();
			m_FarTris.Back().iTri = pPersist->iTri;
			m_FarTris.Back().vPlane = pPersist->GetSeparatingPlane();

			// force far-list update on next iteration
			m_fMaxMoveDistance2 = 0;
			m_fMaxRotDistance = 0;

			m_Tris.EraseNonOrdered(iCurTri);
			iCurTri--;
		}
	}
	else
	{
		if (g_bDrawTriMeshCache)
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dPlane(pPersist->GetSeparatingPlane(), pHull->Origin() + pHull->Radius() * pPersist->GetSeparatingPlane().Normal(), 1.f, Maths::RGBA(255,255,255));
		}
	}
}

/// collide one triangle with a convex hull
int TriMeshCollision::CollideTriMeshHull(ContactVector &contacts, ConvexHullInstance *pHull2, int &iCurTri)
{
	if (contacts.Size() == contacts.Capacity())
	{
		//OutputDebugString("HACK\n");
		return 0;	// TEMPORARY HACK
	}

	ConvexTri *pHull1 = &m_Tris[iCurTri].tri;
	TriPersistentData *pPersist = &m_Tris[iCurTri];

	if (g_bRenderTouchedPolygons)
	{
		if (!g_iRenderTouchedPoly || g_iRenderTouchedPoly == g_nTrisRejected)
		{
			C_ClipPolygon incidentPoly;
			incidentPoly.m_Vertices.Resize(3);
			incidentPoly.m_Vertices[0] = pHull1->GetPoint(0);
			incidentPoly.m_Vertices[1] = pHull1->GetPoint(1);
			incidentPoly.m_Vertices[2] = pHull1->GetPoint(2);
			DrawPoly(incidentPoly);
		}
	}

	// do early-out test
	if (pPersist->bCull)
	{
		HF32 f = pHull2->ProjectExtent(-pPersist->normal) + pPersist->fPlane;
		if (f < HF32(ZERO_INIT)) // f < 0
		{
			// potentially add to far list
			CalcMaxMoveDistanceAndRotationAngle(pHull2, pPersist, iCurTri);
			return 0;
		}
	}

	ConvexBase *pHull[2] = { pHull1, pHull2 };

	// use GJK to determine whether the bodies are intersecting
	GJK gjk(pHull[0], pHull[1]);
	bool bCollision = gjk.Process( pPersist);
	if (!bCollision)
	{
		pPersist->fPlane = pHull1->ProjectExtent(pPersist->normal);
		pPersist->bCull = true;

		CalcMaxMoveDistanceAndRotationAngle(pHull2, pPersist, iCurTri);

		return 0;
	}
	pPersist->bCull = false;

	// use SAT to determine the penetration depth and contact points
	SAT sat(pHull[0], pHull[1]);
	SAT::Result satResult;
	sat.Process(satResult, pPersist);

	// get refplane and side-plane info
	int iRefPlane = satResult.iPlane[satResult.iReference];

	// clip the incident poly against the reference poly
	int rtn;
	if (satResult.iReference == 0)
	{
		// clip convex hull against triangle
		C_ClipPolygon incidentPoly, tmp;
		pHull2->GetPolygon(satResult.iPlane[satResult.iReference^1], incidentPoly);
		rtn = ClipPolyAndGenerateContacts(contacts, pHull1, incidentPoly, iRefPlane, 0);
	}
	else
	{
		int x = (iCurTri&0x7);
		int x2 = (x<<4) | (x<<12);

		// clip triangle against convex hull
		C_ClipPolygon incidentPoly;
		incidentPoly.m_Vertices.Resize(3);
		incidentPoly.m_Vertices[0] = pHull1->GetPoint(0);
		incidentPoly.m_Vertices[0].uid.key = 0x00 | x2;
		incidentPoly.m_Vertices[1] = pHull1->GetPoint(1);
		incidentPoly.m_Vertices[1].uid.key = 0x11 | x2;
		incidentPoly.m_Vertices[2] = pHull1->GetPoint(2);
		incidentPoly.m_Vertices[2].uid.key = 0x22 | x2;
		// TODO: set edge keys
		rtn = ClipPolyAndGenerateContacts(contacts, pHull2, incidentPoly, iRefPlane, 1);
	}

	DisplayContacts(contacts);

	return rtn;
}

/// update polygon data from the trimesh body
void TriMeshCollision::UpdatePolygons(I_TriMesh *pTriMesh, const C_AxisBox &bbox)
{
	// get indices of the polygons inside this bounding box
	Ut::Vector<TriangleId> indices;
	indices.Reserve(16);
	pTriMesh->Query(bbox, indices, m_QueryBox);

//	static Debug::Logger &log = Debug::GetLogger("physics");
//	LOGINFO( log, "TriMeshCollision::UpdatePolygons %d\n", indices.Size());

	// copy the polygon data from the trimesh
	int n = indices.Size();
	if (n == 0)
	{
		m_Tris.Clear();
		return;
	}
	m_Tris.Resize(n);
	for (int i=0; i<n; i++)
	{
		pTriMesh->SetConvexTri(indices[i], &m_Tris[i].tri);
		m_Tris[i].iTri = indices[i];
		m_Tris[i].p1[0] = -1;
		m_Tris[i].iHull = 0;
		m_Tris[i].iPlane = 0;
		m_Tris[i].bCull = false;
		m_Tris[i].fMaxMoveDistance = 0;
	}
}

/// update the far list, maxmovedistance etc.
void TriMeshCollision::UpdateFarList(I_TriMesh *pTriMesh, ConvexHullInstance *pHull2)
{
	HF32 fMaxMove = FLT_MAX;

	HVec3 vO = pHull2->Origin();

	// for each object in the far list, calculate maxmovedistance
	for (int i=0; i<m_FarTris.Size(); i++)
	{
		HF32 f = m_FarTris[i].vPlane.Distance(pHull2->Origin()) + pHull2->Radius();
		if (f > HF32(ZERO_INIT)) // f > 0
		{
			// add to active list
			m_Tris.PushBack();
			const TriangleId &iID = m_FarTris[i].iTri;
			pTriMesh->SetConvexTri(iID, &m_Tris.Back().tri);
			m_Tris.Back().iTri = iID;
			m_Tris.Back().normal = C_Normal(0,1.f,0);

			// remove from far list
			m_FarTris[i] = m_FarTris.Back();
			m_FarTris.PopBack();
			i--;
		}
		else
		fMaxMove = fMaxMove.Min(-f);
	}

	m_fMaxMoveDistance2 = fMaxMove * fMaxMove;
	m_vBasePos = vO;

#ifndef CFG_SPEED
	if (g_bForceTriCacheUpdate)
		m_fMaxMoveDistance2 = 0;
#endif
}

/// calculate contact points between a convex hull and a trimesh body
int TriMeshCollision::Collide(ContactVector &contacts, Body *pBody[2], Shape *pShape[2])
{
	assert(pShape[0]->m_type == E_ShapeType_TriMesh);
	assert(pShape[1]->m_type == E_ShapeType_ConvexHull);
	I_TriMesh *pTriMesh = (I_TriMesh*)pShape[0];
	ConvexHullInstance *pHull2 = (ConvexHullInstance*)pShape[1];

	pHull2->ApplyTransform();

	// if we went outside the query bbox, update the polygon list
	C_AxisBox &aabb = pHull2->GetAABB(pBody[1]);
	if (!m_QueryBox.Inside(aabb.Min()) || !m_QueryBox.Inside(aabb.Max()))
	{
		UpdatePolygons(pTriMesh, aabb);
		// clear the far-list
		m_FarTris.Clear();
	}

	if (g_bDrawTriMeshCache)
	{
		// draw near triangles
		for (int i=0; i<m_Tris.Size(); i++)
		{
			C_Triangle tri = pTriMesh->GetPolygon(m_Tris[i].iTri);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(tri[0], tri[1], tri[2], Maths::RGBA(0,255,0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(tri[0], tri[2], tri[1], Maths::RGBA(0,255,0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri[0],tri[1],Maths::RGBA(255,255,255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri[1],tri[2],Maths::RGBA(255,255,255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri[2],tri[0],Maths::RGBA(255,255,255));
		}

		// draw far triangles
		for (int i=0; i<m_FarTris.Size(); i++)
		{
			C_Triangle tri = pTriMesh->GetPolygon(m_FarTris[i].iTri);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(tri[0], tri[1], tri[2], Maths::RGBA(255,0,0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(tri[0], tri[2], tri[1], Maths::RGBA(255,0,0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri[0],tri[1],Maths::RGBA(255,255,255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri[1],tri[2],Maths::RGBA(255,255,255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri[2],tri[0],Maths::RGBA(255,255,255));
		}

		// draw bounding box
		Maths::HMat43 mtxIdentity;
		mtxIdentity.SetIdentity();
		Maths::C_AxisBox clampedBox(m_QueryBox.Min().Max(HVec3(-10000000,0,-10000000)), m_QueryBox.Max().Min(HVec3(10000000,100,100000000)));
		
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(clampedBox, mtxIdentity, Maths::RGBA(255,255,255));

		// draw far-list bounding sphere
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(m_vBasePos, sqrtf(m_fMaxMoveDistance2), Maths::RGBA(255,255,255));
	}

	// process the far list if we exceeded _m_fMaxMoveDistance_
	HVec3 vMove = m_vBasePos - pHull2->Origin();
	float fMove = vMove.Length2();

	if (fMove > m_fMaxMoveDistance2)
		UpdateFarList(pTriMesh, pHull2);

	// process the active list
	int nContacts = 0;
	for (int i=0; i<m_Tris.Size(); i++)
	{
		nContacts += CollideTriMeshHull(contacts, pHull2, i);
	}

	return nContacts;
}

}; // namespace Impact
