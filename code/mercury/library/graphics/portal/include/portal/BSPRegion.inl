
//! Default c'tor.
C_BSPRegion::C_BSPRegion() :
	m_iRoomIndex((U32)~0),
	m_iSplitPlaneIndex((U32)~0)
{
	return;
}

// -----------------------------------------------------------------

// D'tor.
C_BSPRegion::~C_BSPRegion()
{
	return;
}

// -----------------------------------------------------------------

// Deserialise region data.
template <typename _T>
void C_BSPRegion::Deserialise(_T& rkStream)
{
	rkStream >> m_iRoomIndex;
	rkStream >> m_iSplitPlaneIndex;
	return;
}

// -----------------------------------------------------------------

// Return the room index.
NOALIAS const U32 C_BSPRegion::GetRoomIndex() const __restrict
{
	return m_iRoomIndex;
}

// -----------------------------------------------------------------

// Return the split plane index.
NOALIAS const U32 C_BSPRegion::GetSplitPlaneIndex() const __restrict
{
	return m_iSplitPlaneIndex;
}

// -----------------------------------------------------------------
