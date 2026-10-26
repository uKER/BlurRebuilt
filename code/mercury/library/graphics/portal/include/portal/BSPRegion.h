#ifndef PORTAL_BSPREGION_H
#define PORTAL_BSPREGION_H

#ifdef _MSC_VER
#pragma once
#endif


namespace Portal
{
	//! A region in the tree.
	class C_BSPRegion
	{
	public:
		//!< Default c'tor.
		__forceinline explicit C_BSPRegion();
		//!< D'tor.
		__forceinline ~C_BSPRegion();

		//!< Deserialise a the region data from the stream.
		//@param rkStream Stream from which to deserialise a region object.
		template <typename _T>
		void				Deserialise(_T& rkStream);
		//!< Return the room index.
		//@return The room index for the region.
		__forceinline NOALIAS const U32	GetRoomIndex() const __restrict;
		//!< Return the index of the split plane.
		//@return The index of the spline plane that sub-divides this region, ~0 if none.
		__forceinline NOALIAS const U32	GetSplitPlaneIndex() const __restrict;

	private:
		U32				m_iRoomIndex;								//!< Index of the room.
		U32				m_iSplitPlaneIndex;							//!< Index of split plane.
	};

	// Include implementation of templated methods.
	#include "BSPRegion.inl"
}

#endif // PORTAL_BSPREGION_H
