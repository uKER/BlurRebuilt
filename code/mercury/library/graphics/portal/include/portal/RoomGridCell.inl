
// Deserialise a room grid cell from a stream.
template <typename _T>
void C_RoomGridCell::Deserialise(_T& rkStream)
{
	rkStream >> m_iStartIndex;
	rkStream >> m_iEndIndex;
	return;
}

