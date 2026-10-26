#include <stdafx.h>
#include <Portal/BSPContainer.h>
#include <Portal/BSPSplitPlane.h>
#include <Portal/Ray.h>
#include <system/perf.h>
#include <Btl/Containers/algorithm.h>
#include <Portal/Blocker.h>

using namespace Portal;


// Classify a blocker using the BSP tree.
NOALIAS void C_BSPContainer::Classify(const C_Blocker& rkBlocker, Btl::Vector<U32>& rkResultsVector) const __restrict
{
	PERF_EVENT("portal.classify_bsp_blocker");
	return Classify(rkBlocker, &m_RootRegion, rkResultsVector);
}

// -----------------------------------------------------------------

// Classify a point using the BSP tree.
NOALIAS const U32 C_BSPContainer::Classify(const Maths::HVec3& rkWorldPosition) const __restrict
{
	PERF_EVENT("portal.classify_bsp");
	return Classify(rkWorldPosition, &m_RootRegion);
}

// -----------------------------------------------------------------

// Classify a blocker using the BSP tree.
NOALIAS void C_BSPContainer::Classify(const C_Blocker& rkBlocker, const C_BSPRegion* __restrict pRegion, Btl::Vector<U32>& rkResultsVector) const __restrict
{
	const U32 iSplitPlaneIndex	= pRegion->GetSplitPlaneIndex();
	if(iSplitPlaneIndex == ~0)
	{
		if(Btl::Find(rkResultsVector, pRegion->GetRoomIndex()) == rkResultsVector.End())
			rkResultsVector.PushBack(pRegion->GetRoomIndex());
		return;
	}
	else
	{
		const C_BSPSplitPlane*	pSplitPlane		= &m_pElements[iSplitPlaneIndex];

		bool bVertsInfront	= false;
		bool bVertsBehind	= false;
		Maths::HVec3 verts[8] = 
			{
				// Bottom:
				Maths::HVec3(rkBlocker.GetWorldMatrix().AW() - rkBlocker.GetHalfExtents()),
				
				Maths::HVec3(rkBlocker.GetWorldMatrix().AW().X() - rkBlocker.GetHalfExtents().X(),
						     rkBlocker.GetWorldMatrix().AW().Y() - rkBlocker.GetHalfExtents().Y(),
						     rkBlocker.GetWorldMatrix().AW().Z() + rkBlocker.GetHalfExtents().Z()),
						     
				Maths::HVec3(rkBlocker.GetWorldMatrix().AW().X() + rkBlocker.GetHalfExtents().X(),
							 rkBlocker.GetWorldMatrix().AW().Y() - rkBlocker.GetHalfExtents().Y(),
							 rkBlocker.GetWorldMatrix().AW().Z() + rkBlocker.GetHalfExtents().Z()),
							 
				Maths::HVec3(rkBlocker.GetWorldMatrix().AW().X() + rkBlocker.GetHalfExtents().X(),
							 rkBlocker.GetWorldMatrix().AW().Y() - rkBlocker.GetHalfExtents().Y(),
							 rkBlocker.GetWorldMatrix().AW().Z() - rkBlocker.GetHalfExtents().Z()),

				// Top:
				Maths::HVec3(rkBlocker.GetWorldMatrix().AW().X() - rkBlocker.GetHalfExtents().X(),
							 rkBlocker.GetWorldMatrix().AW().Y() + rkBlocker.GetHalfExtents().Y(),
							 rkBlocker.GetWorldMatrix().AW().Z() + rkBlocker.GetHalfExtents().Z()),
							 
				Maths::HVec3(rkBlocker.GetWorldMatrix().AW().X() - rkBlocker.GetHalfExtents().X(),
							 rkBlocker.GetWorldMatrix().AW().Y() + rkBlocker.GetHalfExtents().Y(),
							 rkBlocker.GetWorldMatrix().AW().Z() - rkBlocker.GetHalfExtents().Z()),
				
				Maths::HVec3(rkBlocker.GetWorldMatrix().AW().X() + rkBlocker.GetHalfExtents().X(),
							 rkBlocker.GetWorldMatrix().AW().Y() + rkBlocker.GetHalfExtents().Y(),
							 rkBlocker.GetWorldMatrix().AW().Z() - rkBlocker.GetHalfExtents().Z()),
				
				Maths::HVec3(rkBlocker.GetWorldMatrix().AW() + rkBlocker.GetHalfExtents())
			};
		
		// Check all the verts against the split plane.
		for(U32 i=0; i<8; ++i)
		{
			const bool bComp	= (pSplitPlane->GetPlane().Distance(verts[i]) > Maths::Const<Maths::VF32>::Zero());
			bVertsInfront		= (bVertsInfront || bComp);
			bVertsBehind		= (bVertsBehind  || !bComp);
		}
		
		// Throw the whole box down the front or back region if any verts of the box
		// lay in that region.
		//  -- STOVE, 30042009
		if(bVertsInfront)
			Classify(rkBlocker, &pSplitPlane->GetFront(), rkResultsVector);
			
		if(bVertsBehind)
			Classify(rkBlocker, &pSplitPlane->GetBack(), rkResultsVector);
	}
}

// -----------------------------------------------------------------

// Classify a point using the BSP tree.
NOALIAS const U32 C_BSPContainer::Classify(const Maths::HVec3& rkWorldPosition, const C_BSPRegion* __restrict pRegion) const __restrict
{
	const U32 iSplitPlaneIndex	= pRegion->GetSplitPlaneIndex();
	if(iSplitPlaneIndex == ~0)
		return pRegion->GetRoomIndex();
	else
	{
		const C_BSPSplitPlane*	pSplitPlane		= &m_pElements[iSplitPlaneIndex];
#ifdef _MSC_VER 
#pragma warning(push)
#pragma warning(disable: 4146)
#endif // _MSC_VER
		const INTPTR			mask			= -static_cast<INTPTR>(pSplitPlane->GetPlane().Distance(rkWorldPosition) > 0.0f);	// [Blur2PC][x64] pointer-wide select
#ifdef _MSC_VER 
#pragma warning(pop)
#endif // _MSC_VER 
		const INTPTR			a				= (const INTPTR)&pSplitPlane->GetFront();
		const INTPTR			b				= (const INTPTR)&pSplitPlane->GetBack();
		const C_BSPRegion*		pRegion			= (const C_BSPRegion*)(((b^a)&mask)^b);
		return Classify(rkWorldPosition, pRegion);
	}
}

// -----------------------------------------------------------------

// Trace a ray in the BSP container.
C_TraceResults C_BSPContainer::Trace(const C_Ray& rkRay) const
{
	// Initialise results and start the trace of the ray.
	C_TraceResults		results;
	results.SetDistance(rkRay.GetLength());
	results.SetFinalRoomIndex(rkRay.GetSolidRegion());
	results.SetHasHit(false);
	results.SetWorldNormal(Maths::C_Normal(rkRay.GetDirection().Normalised()));
	results.SetWorldPosition(rkRay.GetEndPoint());
	Trace(results, m_RootRegion, rkRay, rkRay.GetOrigin(), rkRay.GetEndPoint(), Maths::C_Normal(rkRay.GetDirection().Normalised()));
	return results;
}

// -----------------------------------------------------------------

// Trace a ray in the BSP container.
void C_BSPContainer::Trace(C_TraceResults& rkResults,
						   const C_BSPRegion& rkRegion, 
						   const C_Ray& rkRay,
						   const Maths::C_Vertex& rkStartPosition, 
						   const Maths::C_Vertex& rkEndPosition, 
						   const Maths::C_Normal& rkHitNormal) const
{
#ifdef PLATFORM_SPU
	// TODO: [Stove] Put the SPU intrins back in, selb.
	//snPause();
#endif

	if(rkStartPosition.DistanceSquared(rkEndPosition) <= 0.0001f)
		return;

	const U32 iSplitPlaneIndex	= rkRegion.GetSplitPlaneIndex();
	if(iSplitPlaneIndex == (const U32)-1) // Region is not split.
	{
		if(rkRegion.GetRoomIndex() == rkRay.GetSolidRegion())
		{
			rkResults.SetHasHit(true);
			rkResults.SetWorldPosition(rkStartPosition);
			rkResults.SetWorldNormal(rkHitNormal);
			rkResults.SetDistance(rkRay.GetOrigin().Distance(rkStartPosition));
		}
		else
			rkResults.SetFinalRoomIndex(rkRegion.GetRoomIndex());

		return;
	}
	else
	{
		BIZ_WARNING(iSplitPlaneIndex < m_iNodeCount);	// Out of range.
		const C_BSPSplitPlane*  pSplitPlane		= &m_pElements[iSplitPlaneIndex];
		const Maths::C_Plane	plane			= pSplitPlane->GetPlane();
		Maths::C_Normal			planeNormal		= plane.Normal();
		const Maths::HF32		fStartDistance	= plane.Distance(rkStartPosition);
		const Maths::HF32		fEndDistance	= plane.Distance(rkEndPosition);
#ifdef _MSC_VER 
#pragma warning(push)
#pragma warning(disable: 4146)
#endif // _MSC_VER
		const INTPTR			iStartMask		= -static_cast<INTPTR>(fStartDistance >= 0.0f);	// [Blur2PC][x64] pointer-wide select
		const INTPTR			iEndMask		= -static_cast<INTPTR>(fEndDistance >= 0.0f);
#ifdef _MSC_VER 
#pragma warning(pop)
#endif // _MSC_VER 
		const INTPTR			iBack			= (INTPTR)(&pSplitPlane->GetBack());
		const INTPTR			iFront			= (INTPTR)(&pSplitPlane->GetFront());
		const C_BSPRegion*		pThisSide		= (const C_BSPRegion*)(((iBack ^ iFront) & iStartMask) ^ iBack);
		const C_BSPRegion*		pOtherSide		= (const C_BSPRegion*)(((iBack ^ iFront) & iEndMask) ^ iBack);

		if(pThisSide == pOtherSide)
			Trace(rkResults, *pThisSide, rkRay, rkStartPosition, rkEndPosition, rkHitNormal);
		else
		{
			const Maths::HF32	fT				= (fStartDistance) / (fStartDistance - fEndDistance);
			const Maths::HVec3	midPosition		= rkStartPosition + (rkEndPosition - rkStartPosition) * fT;
			Trace(rkResults, *pThisSide, rkRay, rkStartPosition, midPosition, rkHitNormal);
			if(!rkResults.HasHit())
			{
				if(fStartDistance >= 0.0f)
					planeNormal = planeNormal * -1.0f;

				Trace(rkResults, *pOtherSide, rkRay, midPosition, rkEndPosition, planeNormal);
			}
		}
	}	
	return;
}


