#ifndef PORTAL_BUILDERROOMCONTAINMENTSUBSPACE_H
#define PORTAL_BUILDERROOMCONTAINMENTSUBSPACE_H

#ifdef _MSC_VER
#pragma once
#endif


namespace Portal
{
	class C_RoomContainmentSpace;

	//! Index pair defining a convex sub-space.
	class C_BuilderRoomContainmentSubSpace
	{
	public:
		//! Default c'tor.
		//@param iStart Start index into the planes vector.
		//@param iEnd End index into the planes vector.
		explicit C_BuilderRoomContainmentSubSpace(U32 iStart, U32 iEnd);
		//! D'tor.
		~C_BuilderRoomContainmentSubSpace();

		//! Return the start index into the parent space's plane list.
		//@return The start index.
		const U32		GetStartIndex() const;
		//! Return the end index into the parent space's plane list.
		//@return The end index.
		const U32		GetEndIndex() const;

	private:
		U32			m_iStartIndex;	//!< First plane in the list that defines this sub-space.
		U32			m_iEndIndex;	//!< Last plane in the list that defines this sub-space.

		// Disallow calls to the following:
		explicit C_BuilderRoomContainmentSubSpace();
	};

};

#endif // PORTAL_BUILDERROOMCONTAINMENTSUBSPACE_H
