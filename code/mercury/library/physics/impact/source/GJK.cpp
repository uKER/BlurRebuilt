#include "stdafx.h"

#include "Impact\PersistentData.h"
#include "Impact\ContactUID.h"
#include "Impact\CollisionUtil.h"
#include "Impact\ConvexBase.h"
#include "Impact\GJK.h"

#include "System/Misc/Random.h"
#include "System/Debug/I_DebugRender.h"

#pragma optimize("", on)

using namespace Maths;

namespace Impact
{

Debug::Int	g_iDebugDrawGJKIterations("Impact.GJK.DrawIterations", 0, Debug::Range(0,15));
Debug::Bool	g_bDebugDrawGJKMinkowski("Impact.GJK.DrawMinkowski", false);

const int kMaxGJKIterations = 12;

/// construct using a minkowski sum of two convex objects
GJK::GJK(ConvexBase *pHull1, ConvexBase *pHull2)
{
	m_pHull1 = pHull1;
	m_pHull2 = pHull2;

	if (g_bDebugDrawGJKMinkowski)
	{
		DebugDrawMinkowski();

		// draw the origin
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix(HMat43());
	}
}

/// calculate the maximum point on the minkowski sum of the two hulls, along a support axis
void GJK::ProjectSupportAxis(const Maths::C_Normal &vNormal, SimplexItem &SI)
{
	Maths::HVec3 vP1, vP2;
	vP1 = m_pHull1->ProjectSupportAxis(vNormal, &SI.iP1);
	vP2 = m_pHull2->ProjectSupportAxis(-vNormal, &SI.iP2);
	SI = (vP1 - vP2);
}

/// draw the minkowski sum into the debug renderer by projecting a bunch points in random directions
void GJK::DebugDrawMinkowski()
{
	Util::C_Random rnd;
	for (int i=0; i<100; i++)
	{
		Maths::HVec3 vDir = rnd.SphereRand();
		SimplexItem vS;
		ProjectSupportAxis((C_Normal)vDir, vS);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(vS, .2f, Maths::RGBA(255,0,0));
	}
}

/// calculate the result
bool GJK::Process(HullPersistentData *pPersist)
{
using namespace Maths;
	SimplexItem vS;
	int iIterations = 0;
	float fBestDistance = FLT_MAX;

	// if the last frame was in collision, use it's simplex as the starting point for this frame
	if (pPersist->p1[0] != (U8)-1)
	{
		// load vectors into simplex
		m_Simplex.Resize(4);
		m_Simplex[0] = m_pHull1->GetPoint(pPersist->p1[0]) - m_pHull2->GetPoint(pPersist->p2[0]);
		m_Simplex[0].iP1 = pPersist->p1[0]; m_Simplex[0].iP2 = pPersist->p2[0];
		m_Simplex[1] = m_pHull1->GetPoint(pPersist->p1[1]) - m_pHull2->GetPoint(pPersist->p2[1]);
		m_Simplex[1].iP1 = pPersist->p1[1]; m_Simplex[1].iP2 = pPersist->p2[1];
		m_Simplex[2] = m_pHull1->GetPoint(pPersist->p1[2]) - m_pHull2->GetPoint(pPersist->p2[2]);
		m_Simplex[2].iP1 = pPersist->p1[2]; m_Simplex[2].iP2 = pPersist->p2[2];
		m_Simplex[3] = m_pHull1->GetPoint(pPersist->p1[3]) - m_pHull2->GetPoint(pPersist->p2[3]);
		m_Simplex[3].iP1 = pPersist->p1[3]; m_Simplex[3].iP2 = pPersist->p2[3];
		goto warmStart;
	}

	// pick a random direction
	m_vDir = pPersist->normal;

	// add first point to simplex
	ProjectSupportAxis( m_vDir, vS);
	if ((m_vDir | vS) < HF32( ZERO_INIT ))
		return false;	// no intersection - stop immediately
	m_Simplex.PushBack( vS );

	// search from the starting point in the direction of the origin
	m_vDir = -vS;

	do
	{
		ProjectSupportAxis( m_vDir, vS);
		int n = m_Simplex.Size();
		if (iIterations<g_iDebugDrawGJKIterations)
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(m_Simplex[n-1], vS-m_Simplex[n-1], 1.f, 0.2f, Maths::RGBA(255,255,255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(m_Simplex[n-1], m_vDir, 1.f, 0.2f, Maths::RGBA(0,0,255));
		}

		if ((m_vDir | vS) < 0.f)
		{
			pPersist->normal = m_vDir;
			pPersist->p1[0] = -1;
			return false; // no intersection
		}
		m_Simplex.PushBack( vS );

warmStart:
		// determine new search direction
		switch (m_Simplex.Size())
		{
		case 2:	DetermineSearchDirection2(m_Simplex[1], m_Simplex[0]); break;
		case 3:	DetermineSearchDirection3(m_Simplex[2], m_Simplex[1], m_Simplex[0]); break;
		case 4:
			if (DetermineSearchDirection4(m_Simplex[3], m_Simplex[2], m_Simplex[1], m_Simplex[0]))
			{
				pPersist->normal = m_Simplex[0];
				pPersist->p1[0] = m_Simplex[0].iP1; pPersist->p2[0] = m_Simplex[0].iP2;
				pPersist->p1[1] = m_Simplex[1].iP1; pPersist->p2[1] = m_Simplex[1].iP2;
				pPersist->p1[2] = m_Simplex[2].iP1; pPersist->p2[2] = m_Simplex[2].iP2;
				pPersist->p1[3] = m_Simplex[3].iP1; pPersist->p2[3] = m_Simplex[3].iP2;
				return true;
			}
			break;
		}
	}
	while (++iIterations<kMaxGJKIterations);

	return false;
}

/// determine the new search direction given a simplex with 2 points (edge)
void GJK::DetermineSearchDirection2(const Maths::HVec3 &vA, const Maths::HVec3 &vB)
{
	Maths::HVec3 vAO = -vA;
	// get edge
	Maths::HVec3 vAB = vB - vA;
	if ((vAO | vAB) > 0.f)
		m_vDir = vAB ^ vAO ^ vAB;
	else
		m_vDir = vAO;
}

/// determine the new search direction given a simplex with 3 points (triangle)
void GJK::DetermineSearchDirection3(const Maths::HVec3 &vA, const Maths::HVec3 &vB, const Maths::HVec3 &vC)
{
using namespace Maths;

	HVec3 vAO = -vA;
	// get triangle edges
	HVec3 vAB = vB - vA;
	HVec3 vBC = vC - vB;
	HVec3 vAC = vC - vA;
	HVec3 vABC = vAB ^ vAC; // triangle normal
	if (((vABC ^ vAC) | vAO) > 0.f)		// outside AC edge?
	{
		if ((vAC | vAO) > 0.f)
			m_vDir = vAC ^ vAO ^ vAC;	// closest feature is AC edge
		else
			goto partb;
	}
	else
	{
		// outside AB edge?
		if (((vAB ^ vABC) | vAO) > 0.f)
			goto partb;
		else
		{
			// inside the triangle, test which side
			if ((vABC | vAO) > 0.f)
				m_vDir = vABC;
			else
				m_vDir = -vABC;
		}
	}

	return;
partb:
	if ((vAB | vAO) > 0.f)
		m_vDir = vAB ^ vAO ^ vAB;	// closest feature is AB edge
	else
		m_vDir = vAO;				// closest features is A
	return;
}

static const float fEpsilon = 0.00001f;

/// determine the new search direction given a simplex with 4 points (tetrahedon)
/// returns true if the origin is inside the tetrahedron
bool GJK::DetermineSearchDirection4(const Maths::HVec3 &_vA, const Maths::HVec3 &_vB, const Maths::HVec3 &_vC, const Maths::HVec3 &_vD)
{
using namespace Maths;

	HVec3 vA(_vA), vB(_vB), vC(_vC), vD(_vD);

	// calculate the planes for each triangle (ABC, ABD, ACD)
	HVec3 vAO = -vA;
	HVec3 vB0 = -vB;
	HVec3 vAB = vB - vA;
	HVec3 vAC = vC - vA;
	HVec3 vAD = vD - vA;

	HVec3 vBC = vC - vB;
	HVec3 vCD = vD - vC;

	HVec3 vABC = vAB ^ vAC;
	HVec3 vABD = vAB ^ vAD;
	HVec3 vACD = vAC ^ vAD;
	HVec3 vBCD = vBC ^ vCD;

	float f,f1,f2;

	f1 = (vABC | vAO);
	f2 = (vABC | vAD);
	f = f1 * f2;
	if (f < -fEpsilon)
	{
		// remove D
		m_Simplex[0] = m_Simplex[1];
		m_Simplex[1] = m_Simplex[2];
		m_Simplex[2] = m_Simplex[3];
		m_Simplex.PopBack();

		DetermineSearchDirection3(vA, vB, vC);
		return false;
	}
	f1 = (vABD | vAO);
	f2 = (vABD | vAC);
	f = f1 * f2;
	if (f < -fEpsilon)
	{
		// remove C
		m_Simplex[1] = m_Simplex[2];
		m_Simplex[2] = m_Simplex[3];
		m_Simplex.PopBack();

		DetermineSearchDirection3(vA, vB, vD);
		return false;
	}
	f1 = (vACD | vAO);
	f2 = (vACD | vAB);
	f = f1 * f2;
	if (f < -fEpsilon)
	{
		// remove B
		m_Simplex[2] = m_Simplex[3];
		m_Simplex.PopBack();

		DetermineSearchDirection3(vA, vC, vD);
		return false;
	}
	f1 = (vBCD | vB0);
	f2 = (vBCD | vAB);
	f = f1 * f2;
	if (f > fEpsilon)
	{
		// remove A
		m_Simplex.PopBack();

		DetermineSearchDirection3(vB, vC, vD);
		return false;
	}

	// origin is inside the tetrahedron
	return true;
}

};

