#ifndef PORTAL_BUILDERROOMGRID_H
#define PORTAL_BUILDERROOMGRID_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Portal/BuilderRoomGridCell.h>


namespace Portal
{
	//! An edge.
	class C_BuilderRoomGrid
	{
	public:
		//! Default c'tor.
		//@param rkExtents Axis-aligned box minimally containing the grid.
		//@param iCellsX Number of cells in the X axis.
		//@param iCellsZ Number of cells in the Z axis.
		//@param fCellSize Size of a cell in world units.
		explicit C_BuilderRoomGrid(const Maths::C_AxisBox& rkExtents, U32 iCellsX, U32 iCellsZ, F32 fCellSize);
		//! D'tor.
		~C_BuilderRoomGrid();

		//! Add room.
		//@param pRoom Room to insert into the grid.
		void										AddRoom(C_BuilderRoom* pRoom);
		//! Return cell that contains a world-space point.
		//@param rkPoint World-space point.
		//@return Index of cell that contains the point.
		const U32									ComputeCellForPosition(const Maths::HVec3& rkPoint) const;
		//! Return true if the grid contains a specified vertex.
		//@param rkPoint World-space point.
		//@return True if grid contains point, else false.
		const bool									ContainsVertex(const Maths::HVec3& rkPoint) const;
		//! Return a cell at a specified index.
		//@param iIndex Index of the desired cell.
		//@return Reference to the cell at the specified index.
		const C_BuilderRoomGridCell&				GetCellAtIndex(U32 iIndex) const;
		//! Return the number of cells in the grid.
		//@return Number of cells in the grid.
		const U32									GetCellCount() const;
		//! Return the cell list.
		//@return Reference to the list of cells in the grid.
		const Btl::Vector<C_BuilderRoomGridCell>&	GetCellList() const;
		//! Return the size of a cell.
		//@return The size of the cells in world-space units.
		const F32									GetCellSize() const;
		//! Return the number of cells in X axis.
		//@return Number of cells in the X axis.
		const U32									GetCellsX() const;
		//! Return the number of cells in Z axis.
		//@return Number of cells in the Z axis.
		const U32									GetCellsZ() const;
		//! Return the extents.
		//@return Axis-aligned box that minimally contains the grid.
		const Maths::C_AxisBox&						GetExtents() const;		
		
	private:
		Maths::C_AxisBox						m_Extents;		//!< Extents of the grid.
		U32										m_iCellsX;		//!< Number of cells in x.
		U32										m_iCellsZ;		//!< Number of cells in z.
		F32										m_fCellSize;	//!< Size of cells.
		Btl::Vector<C_BuilderRoomGridCell>		m_vCells;		//!< Polygons that contain this edge.

		// Disallow calls to this:
		explicit C_BuilderRoomGrid();
	};

};

#endif // PORTAL_BUILDERROOMGRID_H
