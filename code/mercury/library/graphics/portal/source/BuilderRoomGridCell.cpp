#include <stdafx.h>
#include <Portal/BuilderRoomGrid.h>

using namespace Portal;


// Default c'tor.
C_BuilderRoomGridCell::C_BuilderRoomGridCell()
{
	return;
}

// -----------------------------------------------------------------

// D'tor.
C_BuilderRoomGridCell::~C_BuilderRoomGridCell()
{
	return;
}

// -----------------------------------------------------------------

// Add a room to the cell's room list.
void C_BuilderRoomGridCell::AddRoom(C_BuilderRoom* pRoom)
{
	m_vRooms.PushBack(pRoom);
	return;
}

// -----------------------------------------------------------------

// Return the number of rooms in the grid cell.
const U32 C_BuilderRoomGridCell::GetRoomCount() const
{
	return static_cast<U32>(m_vRooms.Size());
}

// -----------------------------------------------------------------

// Return a list of rooms in the current grid cell.
const Btl::List<C_BuilderRoom*>& C_BuilderRoomGridCell::GetRoomList() const
{
	return m_vRooms;
}
