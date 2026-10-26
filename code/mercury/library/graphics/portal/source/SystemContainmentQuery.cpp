#include <stdafx.h>
#include <Portal/BSPContainer.h>
#include <Portal/System.h>
#include <Portal/SystemContainmentQuery.h>

#ifdef PLATFORM_SPU
#include "BSPContainer.cpp"
#include "BSPRegion.cpp"
#include "BSPSplitPlane.cpp"
#include "../../../common/core/source/maths/hvec/geometry.cpp"
#endif // #ifdef PLATFORM_SPU

using namespace Portal;


// C'tor.
C_SystemContainmentQuery::C_SystemContainmentQuery(const C_BSPContainer* pTreeEA) :
	m_pTree(pTreeEA)
{
#ifdef PLATFORM_SPU
	
	// Dma up the BSP container.
	const uint32_t tagGroup = 0x0;
	const uint64_t ea = (uint64_t)m_pTree;
	BIZ_WARNING(ea);
	BIZ_WARNING(ea == (ea & ~0xf));
	C_BSPContainer* pBSPContainer					= (Portal::C_BSPContainer*)Spu::Alloc((sizeof(Portal::C_BSPContainer)+0xf) & ~0xf);
	cellDmaGet((void*)pBSPContainer, ea, (sizeof(Portal::C_BSPContainer)+0xf) & ~0xf, tagGroup, 0, 0);
	cellDmaWaitTagStatusAll(1<<tagGroup);	// Wait for Dma of BSPContainer object.
	const uint32_t iNodeCount						= pBSPContainer->GetNodeCount();
	const uint32_t iNodeSize						= ((sizeof(Portal::C_BSPSplitPlane)*iNodeCount)+0xf) & ~0xf;
	C_BSPSplitPlane* pNodes							= (Portal::C_BSPSplitPlane*)Spu::Alloc(iNodeSize);
	BIZ_WARNING(((U32)pNodes & 0xf) == 0);					// Sanity check: Unaligned ls address.
	BIZ_WARNING(pNodes);									// Sanity check: NULL ptr.
	BIZ_WARNING(iNodeSize % 16 == 0);						// Sanity check: dmaSize not a 16byte multiple.
	cellDmaLargeGet((void*)pNodes, (uint64_t)pBSPContainer->GetNodes(), iNodeSize, tagGroup, 0, 0);
	cellDmaWaitTagStatusAll(1 << tagGroup);
	pBSPContainer->SetNodePtr(pNodes);
	m_pTree = pBSPContainer;
	cellDmaWaitTagStatusAll(1 << tagGroup);
	
#endif	// #ifdef PLATFORM_SPU
	return;
}

// -----------------------------------------------------------------

// D'tor.
C_SystemContainmentQuery::~C_SystemContainmentQuery()
{
	// Placeholder method.
	return;
}

// -----------------------------------------------------------------

// Classify the point with respect to the portal system object, returns a room index.
NOALIAS const U32 C_SystemContainmentQuery::Classify(const Maths::HVec3& rkPoint, const U32 iLastSeen) const __restrict
{
	return m_pTree->Classify(rkPoint);
}
