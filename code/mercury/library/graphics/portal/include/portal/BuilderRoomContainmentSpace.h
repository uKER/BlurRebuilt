#ifndef PORTAL_BUILDERROOMCONTAINMENTSPACE_H
#define PORTAL_BUILDERROOMCONTAINMENTSPACE_H

#ifdef _MSC_VER
#pragma once
#endif


#include <Portal/BuilderRoomContainmentSubSpace.h>
#include <Btl/Containers/List.h>

namespace Portal
{
	// Call these forward.
	class C_BuilderRoom;

	//! Defining a room space (concave or convex).
	class C_BuilderRoomContainmentSpace
	{
	public:
		//! C'tor.
		//@param pParent Pointer to the room that parents this space.
		explicit C_BuilderRoomContainmentSpace(C_BuilderRoom* pParent);
		//! D'tor.
		~C_BuilderRoomContainmentSpace();

		//! Return true if this room contains specified point.
		//@param rkPoint World-space point.
		//@return True if room contains the specified point, else false.
		const bool											Contains(const Maths::HVec3& rkPoint) const;
		//! Create a new plane.
		//@param rkPlane Reference to newly created plane.
		void												CreatePlane(const Maths::C_Plane& rkPlane);
		//! Create sub-space.
		//@param iStart Index into planes vector.
		//@param iEnd Index into the planes vector.
		void												CreateSubSpace(U32 iStart, U32 iEnd);
		//! Return the parent room.
		//@return Pointer to the room that this space defines.
		const C_BuilderRoom*								GetParentRoom() const;
		//! Return the number of planes that define the space.
		//@return Number of planes that define the space.
		const U32											GetPlaneCount() const;
		//! Return the plane list.
		//@return Reference to the list of planes.
		const Btl::List<Maths::C_Plane>&					GetPlaneList() const;
		//! Return the number of subspaces.
		//@return Number of convex sub-spaces that are in this space.
		const U32											GetSubSpaceCount() const;
		//! Return the sub-space list.
		//@return Reference to the sub-space list.
		const Btl::List<C_BuilderRoomContainmentSubSpace>&	GetSubSpaceList() const;
		//! Return true if this room intersects an axis box.
		//@param rkBox Box to test for intersection with the room.
		//@return True if room intersects an axis-aligned box.
		const bool											Intersects(const Maths::C_AxisBox& rkBox) const;

	private:
		C_BuilderRoom*									m_pParentRoom;	//!< Pointer to the room that this space defines.
		Btl::List<Maths::C_Plane>						m_vPlanes;		//!< List of planes that define the space.
		Btl::List<C_BuilderRoomContainmentSubSpace>		m_vSubSpaces;	//!< List of index pairs that define convex sub-spaces.

		// Disallow calls to the following:
		explicit C_BuilderRoomContainmentSpace();
	};

};

#endif // PORTAL_BUILDERROOMCONTAINMENTSPACE_H
