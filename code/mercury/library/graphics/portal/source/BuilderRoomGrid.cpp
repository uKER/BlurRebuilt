#include <stdafx.h>
#include <Portal/BuilderRoomGrid.h>
#include <Portal/BuilderRoom.h>
#include <Portal/RoomGrid.h>

using namespace Portal;

// Default c'tor.
C_BuilderRoomGrid::C_BuilderRoomGrid(const Maths::C_AxisBox& rkExtents, U32 iCellsX, U32 iCellsZ, F32 fCellSize) : 
	m_Extents(rkExtents),
	m_iCellsX(iCellsX),
	m_iCellsZ(iCellsZ),
	m_fCellSize(fCellSize)
{
	// Add empty cells to the grid.
	m_vCells.Reserve(iCellsX * iCellsZ);
	for(U32 iCurrentCell=0; iCurrentCell<iCellsX*iCellsZ; ++iCurrentCell)
		m_vCells.PushBack(C_BuilderRoomGridCell());

	return;
}

// -----------------------------------------------------------------

// D'tor.
C_BuilderRoomGrid::~C_BuilderRoomGrid()
{
	return;
}

// -----------------------------------------------------------------

// Add a room to the grid.
void C_BuilderRoomGrid::AddRoom(C_BuilderRoom* pRoom)
{
	// TODO: [Stove] This could be much more fine-grained checking each sub-space.
	BIZ_WARNING_MSG(pRoom, "Room pointer is NULL.");
	if(!pRoom)
		return;
	
	pRoom->ComputeExtents();

	// Loop through all the cells.
	U32 iCellCount = 0;
	Btl::Vector<Portal::C_BuilderRoomGridCell>::Iterator cellIterator = m_vCells.Begin();
	for(; cellIterator != m_vCells.End(); ++cellIterator, ++iCellCount)
	{
		Maths::C_AxisBox cellBox;
		cellBox.Min(Maths::HVec3(m_Extents.Min().X() + static_cast<F32>((iCellCount%m_iCellsX) * m_fCellSize),
								 FLT_MIN,
								 m_Extents.Min().Z() + static_cast<F32>((iCellCount/m_iCellsX) * m_fCellSize)));
		cellBox.Max(Maths::HVec3(m_Extents.Min().X() + static_cast<F32>(((iCellCount%m_iCellsX)+1) * m_fCellSize),
								 FLT_MIN,
								 m_Extents.Min().Z() + static_cast<F32>(((iCellCount/m_iCellsX)+1) * m_fCellSize)));
		
		// Check for early out with our room extents.
		if(pRoom->GetExtents().Intersects(cellBox))
		{
			// TODO: [Stove] Check all the polygons for intersection with course volume intersection positives.
			/*
			bool bFoundAnIntersection = false;

			for(Btl::List<C_BuilderPolygon>::Iterator polygonIterator = pRoom->GetPolygonList().Begin(); polygonIterator != pRoom->GetPolygonList().End(); ++polygonIterator)
			{
				// Don't test against polygons with more than 3 vertices -- should only 
				// be portal polygons which are covered by the pre-collapsed copies in
				// the room geometry.
				if((*polygonIterator).GetVertexCount() > 3)
					continue;

				// Check for intersection of the cellBox with the polygon.
				Btl::List<C_BuilderVertex*>::Iterator vertexIterator = (*polygonIterator).GetVertexList().Begin(); 
				Maths::HVec3 vert0 = (*vertexIterator)->GetPosition();
				Maths::HVec3 vert1 = (*(++vertexIterator))->GetPosition();
				Maths::HVec3 vert2 = (*(++vertexIterator))->GetPosition();
				if(cellBox.Intersects(vert0, vert1, vert2))
				{
					bFoundAnIntersection = true;
					break;
				}
			}*/

			//if(bFoundAnIntersection)
			{
				//LOGINFO(Tools::g_log, "Room %s is in cell %d", pRoom->GetName().Chars(), iCellCount);
				(*cellIterator).AddRoom(pRoom);
			}
		}
	}

	return;
}

// -----------------------------------------------------------------

// Return the cell index for a specified point.
const U32 C_BuilderRoomGrid::ComputeCellForPosition(const Maths::HVec3& rkPoint) const
{
	if(!ContainsVertex(rkPoint))
		return 0xffffffff;

	F32	fDX = m_Extents.Max().X() - m_Extents.Min().X();
	F32 fDZ = m_Extents.Max().Z() - m_Extents.Min().Z();
	U32 iX = static_cast<U32>(floorf(((rkPoint.X() - m_Extents.Min().X())/fDX) * m_iCellsX ));
	U32 iZ = static_cast<U32>(floorf(((rkPoint.Z() - m_Extents.Min().Z())/fDZ) * m_iCellsZ ));
	if(iZ >= m_iCellsZ || iX >= m_iCellsX)
		return C_RoomGrid::InvalidCell;

	return iX + iZ * m_iCellsX;
}

// -----------------------------------------------------------------

// Return true if the grid contains the specified point.
const bool C_BuilderRoomGrid::ContainsVertex(const Maths::HVec3& rkPoint) const
{
	if(m_Extents.Inside(rkPoint))
		return true;
	else 
		return false;
}

// -----------------------------------------------------------------

/// Return the grid cell at a specified index.
const C_BuilderRoomGridCell& C_BuilderRoomGrid::GetCellAtIndex(U32 iIndex) const
{
	BIZ_WARNING_MSG(iIndex < static_cast<U32>(m_vCells.Size()), "Index out of range.");	// Index out of range.
	return m_vCells[iIndex];
}

// -----------------------------------------------------------------

// Return the number of cells in the grid.
const U32 C_BuilderRoomGrid::GetCellCount() const
{
	return static_cast<U32>(m_vCells.Size());
}

// -----------------------------------------------------------------

/// Return the list of cells.
const Btl::Vector<C_BuilderRoomGridCell>&	C_BuilderRoomGrid::GetCellList() const
{
	return m_vCells;
}

// -----------------------------------------------------------------

// Return the size of the cells.
const F32 C_BuilderRoomGrid::GetCellSize() const
{
	return m_fCellSize;
}

// -----------------------------------------------------------------

// Return the number of cells in X axis.
const U32 C_BuilderRoomGrid::GetCellsX() const
{
	return m_iCellsX;
}

// -----------------------------------------------------------------

// Return the number of cells in Z axis.
const U32 C_BuilderRoomGrid::GetCellsZ() const
{
	return m_iCellsZ;
}

// -----------------------------------------------------------------

/// Return AABB that contains grid.
const Maths::C_AxisBox& C_BuilderRoomGrid::GetExtents() const
{
	return m_Extents;
}

