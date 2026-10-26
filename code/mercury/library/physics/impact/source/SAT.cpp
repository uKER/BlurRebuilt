#include "stdafx.h"

#include "Impact\ConvexBase.h"
#include "Impact\PersistentData.h"
#include "Impact\SAT.h"

using namespace Maths;

namespace Impact
{

/// construct using a two bodies
SAT::SAT(ConvexBase *pHull1, ConvexBase *pHull2)
{
	m_pHull[0] = pHull1;
	m_pHull[1] = pHull2;
}

/// find the SAT
void SAT::Process(Result &result, HullPersistentData *pPersist)
{
	// test planes of hulls
	m_fPenetration = FLT_MAX;
	int iHull = pPersist->iHull;
	int iPlane = pPersist->iPlane;
	for (int iHullTest=0; iHullTest<2; iHullTest++)
	{
		m_vOppOrigin = m_pHull[iHull^1]->Origin();
		m_iCurHull = iHull;
 		m_fBias = iHull?0:-0.0001f;
		int nPlanes = m_pHull[iHull]->NumPlanes();
		// test all planes starting with the best plane from the last frame
		for (int i=iPlane; i<nPlanes; i++)
		{
			m_iCurPlane = i;
			TestAxis(m_pHull[iHull]->GetPlane(i));
		}
		for (int i=0; i<iPlane; i++)
		{
			m_iCurPlane = i;
			TestAxis(m_pHull[iHull]->GetPlane(i));
		}
		iHull ^= 1;
		iPlane = 0;
	}

	// find the touching plane on the other hull
	float fBest = 0;
	ConvexBase *pHull = m_pHull[m_iHull^1];
	int iOther = 0;
	for (int i=0; i<pHull->NumPlanes(); i++)
	{
		float f = pHull->GetPlane(i).Normal() | m_vSeparatingAxis;
		if (f<fBest)
		{
			iOther = i;
			fBest = f;
		}
	}

	result.fPenetration = m_fPenetration;
	result.vSeparatingAxis = m_vSeparatingAxis;
	result.iPlane[m_iHull] = m_iPlane;
	result.iPlane[m_iHull^1] = iOther;
	result.iReference = m_iHull;
	pPersist->iHull = m_iHull;
	pPersist->iPlane = m_iPlane;
}

/// calculate the penetration depth for a given axis
void SAT::TestAxis(const C_Plane &vPlane)
{
	// test the origin first
	// this relies on the origins always being further apart along any axis than the hulls themselves
	// this is true so long as the origin lies within the hull
	C_Normal vNegNormal = -vPlane.Normal();
	HF32 f = (m_vOppOrigin | vNegNormal) - vPlane.DistanceToOrigin();
	if (f+m_fBias > m_fPenetration)
		return;

	f = m_pHull[m_iCurHull^1]->ProjectExtent(vNegNormal) - vPlane.DistanceToOrigin();
	if (f+m_fBias < m_fPenetration)
	{
		m_fPenetration = f;
		m_vSeparatingAxis = vPlane.Normal();
		m_iHull = m_iCurHull;
		m_iPlane = m_iCurPlane;
	}
}

}; // namespace Impact
