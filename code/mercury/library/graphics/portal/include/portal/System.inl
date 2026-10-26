
#ifndef PLATFORM_SPU

// Default c'tor.
__forceinline C_System::C_System() :
	m_Tracer(this),
	m_pLastCameraRoom(NULL)
{
	BIZ_WARNING_MSG(m_vPortalSystems.Size() != PORTAL_CONFIG_MAXPORTALSYSTEMS, "Global portal systems list full!");
	m_vPortalSystems.PushBack(this);
	return;
}

// -----------------------------------------------------------------

// D'tor.
__forceinline C_System::~C_System()
{
	Btl::Vector<C_System*, Btl::VectorParameters_Static<PORTAL_CONFIG_MAXPORTALSYSTEMS> >::Iterator thisSystem = Btl::Find(m_vPortalSystems, this);
	BIZ_WARNING_MSG(thisSystem != m_vPortalSystems.End(), "Portal system not in global systems list.");
	m_vPortalSystems.Erase(thisSystem);
	return;
}

// -----------------------------------------------------------------

// Block a portal with specified index.
__forceinline NOALIAS void C_System::BlockPortalAtIndex(const U32 iIndex)
{
	m_vPortalsBlocked.SetBitValue(iIndex, true);
	return;
}

// -----------------------------------------------------------------

// Calculate the room index from the argument room.
__forceinline NOALIAS const U32 C_System::CalculateRoomIndex(const C_Room* pRoom) const __restrict
{
	const char* iRoomPtr	= reinterpret_cast<const char*>(pRoom);
	const char* iListPtr	= reinterpret_cast<const char*>(&m_vRooms[0]);
	const U32 iIndex		= (iRoomPtr-iListPtr)/sizeof(C_Room);
#ifdef _MSC_VER 
#pragma warning(push)
#pragma warning(disable: 4146)	// Stop moaning VS, if your optimiser didn't suck I'd inline the asm for neg ;-)
#endif // _MSC_VER
	const U32 iMask			= -static_cast<U32>(iIndex>static_cast<U32>(m_vRooms.Size()));
#ifdef _MSC_VER 
#pragma warning(pop)
#endif // _MSC_VER 

#if PORTAL_DEBUG_CHECK_ROOM_INDEX_HACK
	U32 iResult = C_Room::InvalidRoom;
	for(U32 iCurrentRoom = 0; iCurrentRoom < static_cast<U32>(m_vRooms.Size()); ++iCurrentRoom)
		if(&m_vRooms[iCurrentRoom] == pRoom)
			iResult = iCurrentRoom;

	// Calculated to different room indices for room pointer.
	BIZ_WARNING_MSG(iResult == (((iIndex^C_Room::InvalidRoom)&iMask)^iIndex), "Epic dodgy hackery fail!");
#endif
	return (((iIndex^C_Room::InvalidRoom)&iMask)^iIndex);
}

// -----------------------------------------------------------------

// Find the room that contains the camera.
__forceinline const C_Room* C_System::FindRoomContainingCamera(const Maths::HMat43& rkCameraMatrix) const
{
	return &m_vRooms[Classify(rkCameraMatrix.AW())];
	//return &m_vRooms[m_Tracer.GetBSPTree().Classify(rkCameraMatrix.AW())];
}

// -----------------------------------------------------------------

// Return true if we should show the current room.
__forceinline const bool C_System::GetHighlightCurrentRoom()
{
	return m_bDebugShowCurrentRoom;
}

// -----------------------------------------------------------------

// Return a pointer to the room that last contained the camera.
__forceinline const C_Room* C_System::GetLastCameraRoom() const
{
	return m_pLastCameraRoom;
}

// -----------------------------------------------------------------

// Return true if we should render debug room names.
__forceinline const bool C_System::GetRenderDebugRoomNames()
{
	return m_bDebugShowRoomNames;
}

// -----------------------------------------------------------------

// Return true if we should render solid polygons for the portal geometry.
__forceinline const bool C_System::GetRenderSolidPortals()
{
	return m_bDebugSolidPortals.Value();
}

// -----------------------------------------------------------------

// Return true if we should render solid polygons for the room geometry.
__forceinline const bool C_System::GetRenderSolidRooms()
{
	return m_bDebugSolidRooms;
}

// -----------------------------------------------------------------

// Return the plane at a specified index.
__forceinline const Maths::C_Plane& C_System::GetPlaneAtIndex(const U32 iIndex) const
{
	BIZ_WARNING_MSG(iIndex < static_cast<U32>(m_vPlanes.Size()), "Index out of range.");
	return m_vPlanes[iIndex]; 
}

// -----------------------------------------------------------------

// Return the portal at a specified index.
__forceinline const C_Portal& C_System::GetPortalAtIndex(const U32 iIndex) const
{
	return m_vPortals[iIndex];
}	

// -----------------------------------------------------------------

// Return the room at a specified index.
__forceinline const C_Room&	C_System::GetRoomAtIndex(const U32 iIndex) const
{
	return m_vRooms[iIndex];
}

// -----------------------------------------------------------------

// Return the number of rooms in the portal system.
__forceinline NOALIAS const U32 C_System::GetRoomCount() const __restrict
{
	return static_cast<U32>(m_vRooms.Size());
}

// -----------------------------------------------------------------

// Return the space at the specified index.
__forceinline const C_RoomContainmentSpace& C_System::GetSpaceAtIndex(const U32 iIndex) const
{
	BIZ_WARNING_MSG(iIndex < static_cast<U32>(m_vSpaces.Size()), "Index out of range.");
	return m_vSpaces[iIndex];
}

// -----------------------------------------------------------------

// Return the sub-space at a specified index.
__forceinline const C_RoomContainmentSubSpace& C_System::GetSubSpaceAtIndex(const U32 iIndex) const
{
	BIZ_WARNING_MSG(iIndex < static_cast<U32>(m_vSubSpaces.Size()), "Index out of range.");
	return m_vSubSpaces[iIndex];
}

// -----------------------------------------------------------------

// Return the deferred tracer object.
__forceinline C_DeferredTracer& C_System::GetTracer()
{
	return m_Tracer;
}

// -----------------------------------------------------------------

// Return the deferred tracer object.
__forceinline  const C_DeferredTracer& C_System::GetTracer() const
{
	return m_Tracer;
}

// -----------------------------------------------------------------

// Block a portal with specified index.
__forceinline NOALIAS void C_System::UnblockPortalAtIndex(const U32 iIndex)
{
	m_vPortalsBlocked.SetBitValue(iIndex, false);
	return;
}
	
#endif
