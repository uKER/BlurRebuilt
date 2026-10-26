#ifndef PORTAL_ROOMCONTAINMENTSUBSPACE_H
#define PORTAL_ROOMCONTAINMENTSUBSPACE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Portal
{
	//! A list of indices into master plane vector, and room index that the 
	//! convex sub-space helps to define.
	class C_RoomContainmentSubSpace
	{
	public:
		//! Default c'tor.
		__forceinline explicit C_RoomContainmentSubSpace();
		//! C'tor.
		//@param iStart The starting index into the planes vector.
		//@param iEnd The ending index into the planes vector.
		//@param Index of parent room.
		__forceinline explicit C_RoomContainmentSubSpace(U32 iStart, U32 iEnd, U32 iRoomIndex);
		//! D'tor.
		__forceinline ~C_RoomContainmentSubSpace();

		//! Deserialise a room containment sub-space.
		//@param rkStream The stream from which to deserialise the convex sub-space.
		template <typename _T>
		void								Deserialise(_T& rkStream);
		//! Return the end index.
		//@return The ending index into the planes vector for this sub-space.
		__forceinline NOALIAS const U32		GetEndIndex() const __restrict;
		//! Return the room index.
		//@return The index of the room that this sub-space helps to define.
		__forceinline NOALIAS const U32		GetRoomIndex() const __restrict;
		//! Return the start index.
		//@return The starting index into the planes vector for this sub-space.
		__forceinline NOALIAS const U32		GetStartIndex() const __restrict;
		//! Return the number of planes in the sub-space.
		//@return The number of planes that define the sub-space.
		__forceinline NOALIAS const U32		GetSubSpacePlaneCount() const __restrict;
		
	private:
		U32		m_iStart;			//!< Starting index of the planes.
		U32		m_iEnd;				//!< End index of the planes.
		U32		m_iRoomIndex;		//!< Room index this sub-space helps to define.
	};

	// Include the implementation of templated methods.
	#include <Portal/RoomContainmentSubSpace.inl>

};

#endif // PORTAL_ROOMCONTAINMENTSUBSPACE_H
