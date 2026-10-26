#include <stdafx.h>
#include <Portal/Blocker.h>
#include <Portal/DeferredBlocker.h>
#include <Portal/Portal.h>
#include <Portal/System.h>

using namespace Portal;

// -----------------------------------------------------------------

// Create a new blocker and return a handle to it -- this is quite a slow operation, 
// use wisely.
NOALIAS BlockerHandle C_DeferredBlocker::CreateBlocker(const Maths::HMat43& rkWorldMatrix, const Maths::HVec3& rkHalfExtents) __restrict
{
	BIZ_WARNING_MSG(m_Blockers.Remaining() > 0, "Trying to create too many blockers.");
	MT::C_MutexLock lock(&m_Mutex);
	C_Blocker* pBlocker = m_Blockers.Allocate();
	new(pBlocker) C_Blocker(rkWorldMatrix, rkHalfExtents);	// Construct the object.
#ifndef CFG_GOLD
	m_vAllocatedBlockers.PushBack(pBlocker);
#endif // #ifndef CFG_GOLD
	
	for(U32 iCurrentSystem=0; iCurrentSystem<static_cast<U32>(C_System::GetActivePortalSystems().Size()); ++iCurrentSystem)
	{
		Btl::Vector<U32> classifyVector;
		classifyVector.Clear();
		C_System::GetActivePortalSystems()[iCurrentSystem]->GetTracer().GetBSPTree().Classify(*pBlocker, classifyVector);
		
		for(U32 iCurrentRoom=0; iCurrentRoom<static_cast<U32>(classifyVector.Size()); ++iCurrentRoom)
		{
			// Check each portal in the room.
			const C_Room* pRoom = &C_System::GetActivePortalSystems()[iCurrentSystem]->GetRoomAtIndex(iCurrentRoom);
			for(U32 iCurrentPortal=0; iCurrentPortal<pRoom->GetPortalCount(); ++iCurrentPortal)
			{
				const C_Portal* pPortal = &C_System::GetActivePortalSystems()[iCurrentSystem]->GetPortalAtIndex(pRoom->GetPortalAtIndex(iCurrentPortal));
				if(Intersection(pBlocker, pPortal))
					pBlocker->AddAffectedPortal(iCurrentSystem, pRoom->GetPortalAtIndex(iCurrentPortal) );	// Add portal to the blocker.
			}
		}
	}
	
	// Force it dirty for the first time round.
	MarkDirty(pBlocker);
	
	return BlockerHandle(pBlocker, this);
}

// -----------------------------------------------------------------

#ifndef PLATFORM_SPU
// Debug render the blocker.
void C_DeferredBlocker::DebugRender(DbUtil::I_DebugRenderPtr pDebugRender)
{
#ifndef CFG_GOLD
	BIZ_WARNING_MSG(pDebugRender, "NULL pointer to debug renderer.");
	for(U32 iCurrentBlocker=0, iCount=m_vAllocatedBlockers.Size(); iCurrentBlocker < iCount; ++iCurrentBlocker)
		m_vAllocatedBlockers[iCurrentBlocker]->DebugRender(pDebugRender);
#endif // #ifndef CFG_GOLD
	return;
}
#endif // #ifndef PLATFORM_SPU

// -----------------------------------------------------------------

// Mark a blocker as dirty.
NOALIAS void C_DeferredBlocker::MarkDirty(C_Blocker* __restrict const pBlocker) __restrict
{
	m_vDirtyBlockers.PushBack(pBlocker);
	return;
}

// -----------------------------------------------------------------

// Remove a blocker from the portal system.
NOALIAS void C_DeferredBlocker::RemoveBlocker(BlockerHandle handle)
{
	BIZ_WARNING_MSG(handle.m_pPointer, "Invalid blocker handle.");
	MT::C_MutexLock lock(&m_Mutex);
	m_Blockers.Free(handle.m_pPointer);
#ifndef CFG_GOLD
	// So glad this isn't in the speed build ;)
	m_vAllocatedBlockers.Erase(Btl::Find(m_vAllocatedBlockers, handle.m_pPointer));
#endif // #ifndef CFG_GOLD
	handle.m_pPointer = NULL;	// Null the pointer.
	return;
}

// -----------------------------------------------------------------

// Immediate blocker updates.
NOALIAS void C_DeferredBlocker::Update()
{
	MT::C_MutexLock lock(&m_Mutex);
	
	// Update the bitfields on portal systems for all dirty blockers:
	for(U32 iCurrentBlocker=0, iCount=m_vDirtyBlockers.Size(); iCurrentBlocker<iCount; ++iCurrentBlocker)
	{
		//DCBT(0, m_vDirtyBlockers[iCurrentBlocker+1]);
		//DCBT(0, m_vDirtyBlockers[iCurrentBlocker+2]);
		const U32 iAffectedPortalCount = m_vDirtyBlockers[iCurrentBlocker]->GetAffectedPortalCount();
		for(U32 iCurrentAffectedPortal=0; iCurrentAffectedPortal<iAffectedPortalCount; ++iCurrentAffectedPortal)
		{
			const U32 iSystemIndex = m_vDirtyBlockers[iCurrentBlocker]->GetAffectedPortalIndexPairAtIndex(iCurrentAffectedPortal).first;
			const U32 iPortalIndex = m_vDirtyBlockers[iCurrentBlocker]->GetAffectedPortalIndexPairAtIndex(iCurrentAffectedPortal).second;
			if(m_vDirtyBlockers[iCurrentBlocker]->IsActive())
				C_System::GetActivePortalSystems()[iSystemIndex]->BlockPortalAtIndex(iPortalIndex);
			else
				C_System::GetActivePortalSystems()[iSystemIndex]->UnblockPortalAtIndex(iPortalIndex);
		}
	}
	
	// Clear the list of dirty blockers for this frame.
	m_vDirtyBlockers.Clear();
	return;
}

