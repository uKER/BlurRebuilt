
// Deserialise a room grid from a stream.
template <typename _T>
void C_RoomGrid::Deserialise(_T& rkStream)
{
	rkStream >> m_Extents;
	rkStream >> m_fCellSize;
	rkStream >> m_iNumberOfCellsX;
	rkStream >> m_iNumberOfCellsZ;
	U32 iCellCount = 0;
	rkStream >> iCellCount;
	BIZ_WARNING_MSG(iCellCount, "Grid with no cells, invalid data.");		// Grid with no cells!?

	m_vCells.Reserve(iCellCount);
	for(U32 iCurrentCell=0; iCurrentCell<iCellCount; ++iCurrentCell)
	{
		C_RoomGridCell currentSubSpace;
		rkStream >> currentSubSpace;
		m_vCells.PushBack(currentSubSpace);
	}
	return;
}

