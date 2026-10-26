
// Default c'tor.
C_RoomContainmentSpace::C_RoomContainmentSpace()
{
	return;
}

// -----------------------------------------------------------------

// D'tor.
C_RoomContainmentSpace::~C_RoomContainmentSpace()
{
	// Placeholder method.
	return;
}

// -----------------------------------------------------------------

// Deserialise a room's containment space.
template <typename _T>
void C_RoomContainmentSpace::Deserialise(_T& rkStream)
{
	rkStream >> m_iStartSubSpaceIndex;
	rkStream >> m_iEndSubSpaceIndex;
	return;
}

// -----------------------------------------------------------------

// Return the end index into the sub-spaces vector.
NOALIAS const U32 C_RoomContainmentSpace::GetEndIndex() const __restrict
{
	return m_iEndSubSpaceIndex;
}

// -----------------------------------------------------------------

// Return the start index into the sub-spaces vector.
NOALIAS const U32 C_RoomContainmentSpace::GetStartIndex() const
{
	return m_iStartSubSpaceIndex;
}

// -----------------------------------------------------------------

// Return the number of sub-spaces within this space.
NOALIAS const U32 C_RoomContainmentSpace::GetSubSpaceCount() const __restrict
{
	return m_iEndSubSpaceIndex - m_iStartSubSpaceIndex;
}
