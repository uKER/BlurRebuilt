
// Default c'tor.
__forceinline C_RoomContainmentSubSpace::C_RoomContainmentSubSpace() :
	m_iStart((U32)-1),
	m_iEnd((U32)-1),
	m_iRoomIndex((U32)-1)
{
	return;
}

// -----------------------------------------------------------------

// C'tor.
__forceinline C_RoomContainmentSubSpace::C_RoomContainmentSubSpace(U32 iStart, U32 iEnd, U32 iRoomIndex) :
	m_iStart(iStart),
	m_iEnd(iEnd),
	m_iRoomIndex(iRoomIndex)
{
	return;
}

// -----------------------------------------------------------------

// D'tor.
__forceinline C_RoomContainmentSubSpace::~C_RoomContainmentSubSpace()
{
	return;
}

// -----------------------------------------------------------------

// Return the end index into plane list.
__forceinline NOALIAS const U32 C_RoomContainmentSubSpace::GetEndIndex() const __restrict
{
	return m_iEnd;
}

// -----------------------------------------------------------------

// Return the room index that this sub-space helps to define.
__forceinline NOALIAS const U32 C_RoomContainmentSubSpace::GetRoomIndex() const __restrict
{
	return m_iRoomIndex;
}

// -----------------------------------------------------------------

// Return the start index into plane list.
__forceinline NOALIAS const U32 C_RoomContainmentSubSpace::GetStartIndex() const __restrict
{
	return m_iStart;
}

// -----------------------------------------------------------------

// Return the number of planes in the sub-space.
__forceinline NOALIAS const U32 C_RoomContainmentSubSpace::GetSubSpacePlaneCount() const __restrict
{
	return m_iEnd - m_iStart;
}
