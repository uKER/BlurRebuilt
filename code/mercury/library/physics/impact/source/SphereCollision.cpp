#include "stdafx.h"

#include "Impact\ContactUID.h"
#include "Impact\Body.h"
#include "Impact\Collision.h"
#include "Impact\CollisionUtil.h"
#include "Impact\GJK.h"
#include "Impact\SAT.h"
#include "Impact\ConvexHull.h"
#include "Impact\PersistentData.h"
#include "Impact\Sphere.h"
#include "Impact\SphereCollision.h"
#include <system/debug/I_DebugRender.h>

using namespace Maths;

namespace Impact
{

Debug::Bool g_bUpdateOrgSpherePos("Impact.UpdateOrgSpherePos", true);
Debug::Bool g_bContinuousCollision("Impact.ContinuousCollision", true);

/// constructor
SphereCollision::SphereCollision()
{
	m_iSeparatingPlane = 0;
//	m_bInitialised = false;
}

#define SPHERE_EPSILON 0.0001f

struct kItem
{
	float k;
	int iPlane;
	bool bHit;
};

/// calculate contact points between a convex hull and a trimesh body
int SphereCollision::Collide(ContactVector &contacts, Body *p[2], Shape *pShape[2])
{
	// get shape/hull
	assert(pShape[0]->m_type == E_ShapeType_Sphere);
	assert(pShape[1]->m_type == E_ShapeType_ConvexHull);
	ConvexHullInstance *pHullInst = (ConvexHullInstance*)pShape[1];
	const ConvexHull *pHull = pHullInst->GetParent();
	SphereInstance *pSphere = (SphereInstance*)pShape[0];
	pHullInst->ApplyTransform();
	pSphere->ApplyTransform();

	Ut::UtStackVector<F32, 128> k; // NOTE: limit to convex hulls with 128 planes

	// find the minimum separation distance / vector

	// check planes, first pass
	int n = pHull->NumPlanes();
	HF32 fFltMax(-FLT_MAX);
	HF32 fDist(fFltMax);
	HF32 fSphereRadius = Load<HF32>(&pSphere->m_fRadius);
	HVec3 vSpherePos = pSphere->m_vWorldPos;
	C_Normal vNormal;
	k.Resize(n);
	int iPlane = m_iSeparatingPlane;
	for (int i=0; i<n; i++)
	{
		const C_Plane &plane = pHullInst->GetPlane(iPlane);
		HF32 f = plane.Distance(vSpherePos);
		if (f-fSphereRadius>HF32(ZERO_INIT))
		{
			m_iSeparatingPlane = iPlane;
			return 0;	// no collision
		}
		Store(f, &k[iPlane]);
		iPlane = (iPlane+1)%n;
	}

	// check planes second pass
	// find the largest distance among all the planes (largest separating plane)
	for (int i=0; i<k.Size(); i++)
	{
		HF32 f = Load<HF32>(&k[i]) - fSphereRadius;
		if (f > fDist && f > -fSphereRadius*2.f)
		{
			C_ClipPolygon poly;
			pHullInst->GetPolygon(i, poly);
			C_Normal vPlaneN = pHullInst->GetPlane(i).Normal();

			// check for each k whether it's inside the polygon
			int n = poly.m_Vertices.Size();
			bool bHit = true;
			int jo = n-1;
			for (int j=0; j<n; j++)
			{
				HVec3 vEdge = poly.m_Vertices[j]-poly.m_Vertices[jo];
				C_Plane plane(C_Normal(vEdge ^ vPlaneN), poly.m_Vertices[j]);
				HF32 _f = plane.Distance(vSpherePos);
				if (_f < HF32(ZERO_INIT))
				{
					bHit = false;
					break;
				}
				jo = j;
			}
			if (bHit == false)
				continue;

			fDist = f;
			vNormal = vPlaneN;

//			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dPlane(pHullInst->GetPlane(i), poly.m_Vertices[0], 1.f, Maths::RGBA(255,255,255,255));

		}
	}

	if (fDist == fFltMax)
	{
		// collide against edges
		// find the smallest distance among all the edges
		fDist = pSphere->m_fRadius;
		float _fDist = pSphere->m_fRadius;
		n = pHull->NumPlanes();
		for (int i=0; i<n; i++)
		{
			if (k[i] < 0.f || k[i]>_fDist)	// if the sphere was entirely in front of this plane we don't need to check the plane's edges
				continue;

			C_ClipPolygon poly;
			pHullInst->GetPolygon(i, poly);

			int n = poly.m_Vertices.Size();
			int jo = n-1;
			for (int j=0; j<n; j++)
			{
				C_Line edge(poly.m_Vertices[j], poly.m_Vertices[jo]);
				HF32 f = edge.Distance(vSpherePos) - fSphereRadius;
				if (f < fDist)
				{
					C_Vertex vDummy;
					HF32 fDummy;
					C_Normal vTmpNormal;
					if (LineClosestPoint(edge, vSpherePos, vDummy, vTmpNormal, fDummy))
					{
						fDist = f;
						vNormal = vTmpNormal;
					}
				}
				jo = j;
			}
		}

		// collide against vertices
		n = pHullInst->NumPoints();
		for (int i=0; i<n; i++)
		{
			HVec3 v = vSpherePos - pHullInst->GetPoint(i);
			HF32 f = v.Length() - fSphereRadius;
			if (f < fDist)
			{
				fDist = f;
				vNormal = v;
			}
		}
	}

	if (fDist < HF32(ZERO_INIT))
	{
		if (contacts.Size() < contacts.Capacity())
		{
			contacts.PushBack();
			contacts.Back().m_vNormal = -vNormal;
			contacts.Back().m_vPos = vSpherePos + vNormal * pSphere->m_fRadius;
			contacts.Back().m_fSeparation = fDist;
			contacts.Back().m_uid = 0;

			DisplayContacts(contacts);
		}

		return 1;
	}

	return 0;
}

/*
/// calculate contact points between a convex hull and a trimesh body
int SphereCollision::CollideContinuous(ContactVector &contacts, Body *p[2], Shape *pShape[2])
{
	assert(pShape[0]->m_type == E_ShapeType_Sphere);
	assert(pShape[1]->m_type == E_ShapeType_ConvexHull);
	ConvexHullInstance *pHullInst = (ConvexHullInstance*)pShape[1];
	const ConvexHull *pHull = pHullInst->GetParent();
	SphereInstance *pSphere = (SphereInstance*)pShape[0];
	pHullInst->ApplyTransform();

	HMat33 testmtx3 = (HMat33)p[1]->m_rotation;
	testmtx3 = testmtx3.Inverse();

	// calculate relative positions
	HVec3 vSphereStart;
	HVec3 vSpherePos;
	if (!m_bInitialised)
	{
		m_bInitialised = true;
		vSpherePos = p[0]->m_vPrevPos + pSphere->m_vPos * p[0]->m_qPrevRot;
		m_vSpherePrev = (vSpherePos - p[1]->m_vPrevPos) * testmtx3;
	}
	vSphereStart = m_vSpherePrev;
	vSpherePos = pSphere->m_vPos;
	HVec3 vSphereEnd = (vSpherePos - p[1]->m_vPos) * testmtx3;
	// FIXME: possible problem here if the convex hull rotates very quickly
	//        the screw motion is approximated by a linear interpolation between the start and end position
	//        this could cause false collisions to be detected

	// FIXME: the direction of the normal depends on the sphere start position.
	//        if the sphere start position isn't updated for a while this can make the collision normal point the wrong way

//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(vSphereStart, pSphere->m_fRadius, Maths::RGBA(255,0,0,255));
//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(vSphereEnd, pSphere->m_fRadius, Maths::RGBA(255,128,128,255));

	if (g_bUpdateOrgSpherePos)
		m_vSpherePrev = vSphereEnd;

	// calculate start to end distance and direction
	HVec3 vDist = vSphereEnd - vSphereStart;
	float fLength = vDist.Length();

	C_Normal vDir( vDist * (1.f/fLength) );

	// trace sphere against convex hull planes
	float frac;
	Ut::UtStackVector<kItem, 128> k; // NOTE: limit to convex hulls with 128 planes
	int n = pHull->NumPlanes();
	for (int i=0; i<n; i++)
	{
		const C_Plane &plane = pHull->GetPlane(i);
		float fStart = plane.Distance(vSphereStart) - pSphere->m_fRadius;
		float fEnd = plane.Distance(vSphereEnd) - pSphere->m_fRadius;

		// case of ray totally outside of this plane - quit
		if (fStart > 0 && fEnd > 0)
			return 0;
		// if the ray doesn't go from the front of the plane to the back, continue
		if (fStart+pSphere->m_fRadius < 0 || fEnd > 0)
			continue;
		// calculate intersection point
		frac=fStart/(fStart-fEnd);
		if (frac<0)
			frac = 0;
		// add this one to the list
		k.PushBack();
		k.Back().k = frac;
		k.Back().iPlane = i;
	}

	float fMinK = 1.0f;
	C_Normal vNormal;
	for (int i=0; i<k.Size(); i++)
	{
		// calculate impact point
		HVec3 v = vSphereStart + vDist * (k[i].k - 0.01f);

		// check for each k whether it's inside the polygon
		C_ClipPolygon poly;
		pHullInst->GetPolygon(k[i].iPlane, poly);
		C_Normal vPlaneN = pHullInst->GetPlane(k[i].iPlane).Normal();

		// check for each k whether it's inside the polygon
		int n = poly.m_Vertices.Size();
		k[i].bHit = true;
		int jo = n-1;
		for (int j=0; j<n; j++)
		{
			HVec3 vEdge = poly.m_Vertices[j]-poly.m_Vertices[jo];
			C_Plane plane(C_Normal(vEdge ^ vPlaneN), poly.m_Vertices[j]);
			float _f = plane.Distance(v);
			if (_f < 0.f)
			{
				k[i].bHit = false;
				break;
			}
			fMinK = k[i].k;
			vNormal = pHull->GetPlane(k[i].iPlane).Normal();
		}
	}

	// collide against edges (sphere vs cylinder)
	// only collide against edges of planes that we passed but didn't hit
	C_Normal vTmpNormal;
	HVec3 vTmpHit;
	n = pHull->NumPlanes();
	for (int i=0; i<n; i++)
	{
		C_ClipPolygon poly;
		pHull->GetPolygon(i, poly);

		int n = poly.m_Vertices.Size();
		int jo = n-1;
		for (int j=0; j<n; j++)
		{
			float fK = fMinK * fLength;
			if (CollideEdgeSphereLine(vSphereStart, vDir, pSphere->m_fRadius, poly.m_Vertices[j], poly.m_Vertices[jo], &fK, vTmpHit, vTmpNormal, -pSphere->m_fRadius))
			{
				fMinK = fK / fLength;
//				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(vTmpHit, vTmpNormal, 2.f, 0.2f, Maths::RGBA(255,255,255,255));
				vNormal = vTmpNormal;
			}
			jo = j;
		}
	}

	// collide against vertices (sphere vs swept sphere) 
	n = pHull->NumPoints();
	float fK, fOrgK = fMinK * fLength;
	for (int i=0; i<n; i++)
	{
		fK = fOrgK;

		if (IntersectRaySphere(vSphereStart, vDir, pHull->GetPoint(i), pSphere->m_fRadius, &fK, vTmpNormal, -pSphere->m_fRadius))
		{
			if (fK < fOrgK)
			{
				fMinK = fK / fLength;
//				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(pHull->GetPoint(i), vTmpNormal, 2.f, 0.2f, Maths::RGBA(255,255,255,255));
				vNormal = vTmpNormal;
			}
		}
	}

	// check for any hits
//	if (fMinK >= 1.0f)
//		return CollideP(contacts, p, pShape); // fallback to simpler penetration-based version
	if (fMinK < 0.f)
		fMinK = 0.f;

	if (contacts.Size() >= contacts.Capacity())
		return 1;

	// emit contact
	if (g_bUpdateOrgSpherePos)
	{
	//	m_vSpherePrev = vSphereStart + (vDist) * (fMinK - 0.01f);
	}
	contacts.PushBack();
	contacts.Back().m_vNormal = -vNormal * ((HMat33)p[1]->m_rotation);
	// contact position in the convex hull's space
	HVec3 vContactPos = p[1]->m_vPrevPos + (p[1]->m_vPrevPos - p[1]->m_vPos) * fMinK;
	contacts.Back().m_vPos = vContactPos;
	contacts.Back().m_fSeparation = -1.0f;//-fLength * (1.f - fMinK);
	contacts.Back().m_uid = 0;

	DisplayContacts(contacts);

	return 1;
}
*/


}; // namespace Impact
