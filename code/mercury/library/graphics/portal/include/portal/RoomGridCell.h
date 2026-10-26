#ifndef PORTAL_ROOMGRIDCELL_H
#define PORTAL_ROOMGRIDCELL_H

#ifdef _MSC_VER
#pragma once
#endif


namespace Portal
{
	//! A single cell in the room grid.
	class C_RoomGridCell
	{
	public:
		//! Default c'tor.
		explicit C_RoomGridCell();
		//! D'tor.
		~C_RoomGridCell();

		//! Deserialise the room grid cell from a stream.
		//@param rkStream Stream from which to deserialise a grid cell.
		template <typename _T>
		void					Deserialise(_T& rkStream);
		/// Return the end index into the sub-space vector.
		//@return The end index into sub-space vector for the grid cell.
		NOALIAS const U32		GetEndIndex() const __restrict;
		/// Return the start index into the sub-space vector.
		//@return The start index into sub-space vector for the grid cell.
		NOALIAS const U32		GetStartIndex() const __restrict;
		/// Get sub-space count.
		//@return The number of sub-spaces contained in the grid cell.
		NOALIAS const U32		GetSubSpaceCount() const __restrict;

	private:
		U32		m_iStartIndex;			//!< Start index into cell/sub-space index vector.
		U32		m_iEndIndex;			//!< End index into cell/sub-space index vector.
	};

	// Include implementation of templated methods.
	#include <Portal/RoomGridCell.inl>
};

#endif // PORTAL_ROOMGRID_H
