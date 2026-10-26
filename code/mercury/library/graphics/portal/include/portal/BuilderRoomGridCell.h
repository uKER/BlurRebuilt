#ifndef PORTAL_BUILDERROOMGRIDCELL_H
#define PORTAL_BUILDERROOMGRIDCELL_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/List.h>


namespace Portal
{
	// Call these forward.
	class C_BuilderRoom;
	
	//! An edge.
	class C_BuilderRoomGridCell
	{
	public:
		//! Default c'tor.
		explicit C_BuilderRoomGridCell();
		//! D'tor.
		~C_BuilderRoomGridCell();

		//! Add room.
		//@param pRoom Room to add to the grid cell.
		void									AddRoom(C_BuilderRoom* pRoom);
		//! Return the number of rooms in this grid cell.
		//@return The number of rooms in the grid cell.
		const U32								GetRoomCount() const;
		//! Return the cell list.
		//@return Reference to the list of rooms in the cell.
		const Btl::List<C_BuilderRoom*>&		GetRoomList() const;

	private:
		Btl::List<C_BuilderRoom*>		m_vRooms;		//!< Rooms that are in this cell.
	};

};

#endif // PORTAL_BUILDERROOMGRIDCELL_H
