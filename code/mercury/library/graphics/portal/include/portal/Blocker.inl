
// Specified c'tor.
__forceinline NOALIAS C_Blocker::C_Blocker(const Maths::HMat43& rkWorldMatrix, const Maths::HVec3& rkHalfExtents) :
	m_WorldMatrix(rkWorldMatrix),
	m_HalfExtents(rkHalfExtents),
	m_bActive(false)
{
	return;
}

// -----------------------------------------------------------------

// Add an affected portal to the blocker's affected portal list.
__forceinline NOALIAS void C_Blocker::AddAffectedPortal(U32 iSystemIndex, U32 iPortalIndex)
{
	BIZ_WARNING_MSG(m_vAffectedPortals.Size() < m_vAffectedPortals.Capacity(), "Blocker affects too many portals.");
	m_vAffectedPortals.PushBack(Btl::MakePair((U16)iSystemIndex, (U16)iPortalIndex));
	return;
}

// -----------------------------------------------------------------

// Return affected portal index pair at a specified index.
__forceinline NOALIAS const Btl::Pair<U16, U16>& C_Blocker::GetAffectedPortalIndexPairAtIndex(const U32 iIndex) const
{
	return m_vAffectedPortals[iIndex];
}

// -----------------------------------------------------------------

// Return the number of portals affected by this blocker.
__forceinline NOALIAS const U32 C_Blocker::GetAffectedPortalCount() const
{
	return m_vAffectedPortals.Size();
}

// -----------------------------------------------------------------

// Return the extents of the blocker.
__forceinline NOALIAS const Maths::HVec3& C_Blocker::GetHalfExtents() const __restrict
{
	return m_HalfExtents;
}

// -----------------------------------------------------------------

// Return the world position of the blocker.
__forceinline NOALIAS const Maths::HMat43& C_Blocker::GetWorldMatrix() const __restrict
{
	return m_WorldMatrix;
}

// -----------------------------------------------------------------

// Return true if the blocker is active.
__forceinline NOALIAS const bool C_Blocker::IsActive() const __restrict
{
	return m_bActive;
}

// -----------------------------------------------------------------

// Turn the blocker off.
__forceinline NOALIAS void C_Blocker::TurnOff()
{
	m_bActive		 = false;
	return;
}

// -----------------------------------------------------------------

// Turn the blocker on.
__forceinline NOALIAS void C_Blocker::TurnOn()
{
	m_bActive		 = true;
	return;
}

// -----------------------------------------------------------------

// C'tor -- takes pointer to the blocker.
__forceinline NOALIAS BlockerHandle::BlockerHandle(C_Blocker* pPointer, C_DeferredBlocker* pDeferredBlocker) :
	m_pPointer(pPointer),
	m_pDeferredBlocker(pDeferredBlocker)
{
	return;
}

