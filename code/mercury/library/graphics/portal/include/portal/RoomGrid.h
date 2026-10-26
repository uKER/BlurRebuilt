#ifndef PORTAL_ROOMGRID_H
#define PORTAL_ROOMGRID_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/Maths/hvec/Geometry.h>
#include <Portal/RoomGridCell.h>


namespace Portal
{
	//! Spatial structure to accelerate containment in room queries.
	class C_RoomGrid
	{
	public:
		//! Default c'tor.
		explicit C_RoomGrid();
		//! D'tor.
		~C_RoomGrid();

		//! Calculate room grid cell from world position.
		//@param rkPosition The world-space position for which the containing cell is desired.
		//@return Cell that contains the world-space position.
		NOALIAS const U32		CalculateCellIndexFromPosition(const Maths::HVec3& rkPosition) const __restrict;
		//! Debug render the grid.
		//@param pDebugRender Pointer to an instance of the debug renderer.
		//@param rkCameraMat View matrix.
		void					DebugRender(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::HMat43& rkCameraMat);
		//! Deserialise the room grid from a stream.
		//@param rkStream A stream from which to serialise the room grid.
		template <typename _T>
		void					Deserialise(_T& rkStream);
		//! Return the cell at a specified index.
		//@param iIndex The index for which the grid cell is desired.
		//@return Pointer to the grid cell for the specified index.
		const C_RoomGridCell*	GetCellAtIndex(U32 iIndex) const;

		//! Invalid cell index.
		static const U32 InvalidCell = 0xd15ea5ed;

	private:
		Maths::C_AxisBox				m_Extents;					//!< Extent of the entire room grid.
		U32								m_iNumberOfCellsX;			//!< Number of grid cells in the X axis.
		U32								m_iNumberOfCellsZ;			//!< Number of grid cells in the Z axis.
		F32								m_fCellSize;				//!< Width and depth of a cell, uniform size (X/Z).
		Btl::Vector<C_RoomGridCell>		m_vCells;					//!< List of cells in the room grid.
	};

	// Include implementations of templated methods.
	#include <Portal/RoomGrid.inl>
};

#endif // PORTAL_ROOMGRID_H
