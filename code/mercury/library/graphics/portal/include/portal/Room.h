#ifndef PORTAL_ROOM_H
#define PORTAL_ROOM_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Portal/RoomPolygon.h>
#include <Portal/RoomContainmentSpace.h>
#include <misc_shared/DataStructure/StringTable.h>
#include <System/debug/Variables.h>


namespace Portal
{
	// Call these forward.
	class C_Portal;
	class C_System;
	class C_RoomPolygon;

	//! A single room.
	class C_Room
	{
	public:
		//! Specified c'tor.
		//@param pParentSystem Pointer to the portal system object that owns the room.
		//@param iStringIndex Index into the string table for the room name.
		explicit C_Room(C_System* pParentSystem, U32 iStringIndex = 0);
		//! D'tor.
		~C_Room();

		//! Return true if specified point is in room.
		//@param rkPoint Point to test for containment within the room.
		NOALIAS const bool			Contains(const Maths::HVec3& rkPoint) const __restrict;
		//! Debug render.
		//@param pDebugRender Pointer to an instance of a debug renderer.
		//@param rkCameraMat View matrix.
		void						DebugRender(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::HMat43& rkCameraMat);
		//! Deserialise a room from a stream.
		//@param rkStream Stream from which to deserialise a room object.
		template <typename _T>
		void						Deserialise(_T& rkStream);
		//! [Slow] Get the debug centre of the room, as the name suggests should be used for debugging only.
		//@return Centre of the room.
		NOALIAS const Maths::HVec3	GetDebugCentre() const __restrict;
		//! Return the portal index at a specified array index.
		//@param iIndex The index for which the portal index is desired.
		//@return Portal index.
		NOALIAS const U32			GetPortalAtIndex(U32 iIndex) const __restrict;
		//! Return the number of portals in the room.
		//@return The number of portals in the room.
		NOALIAS const U32			GetPortalCount() const __restrict;
		//! Return the room polygon at specified index.
		//@param iIndex The index for which the room polygon is desired.
		//@return Room polygon at a specified index.
		const C_RoomPolygon&		GetRoomPolygonAtIndex(U32 iIndex) const;
		//! Return number of polygons in this room.
		//@return The number of polygons in the room.
		NOALIAS const U32			GetRoomPolygonCount() const __restrict;
		//! Get the index into the Portal string table.
		//@return The string Id for the room name.
		NOALIAS Util::C_StringId	GetStringIndex() const __restrict;
		//! Read in debug polygons from a stream.
		//@param rkStream The stream from which to deserialise the debug geometry for a room object.
		template <typename _T>
		void						ReadDebugPolygons(_T& rkStream);
		
		//! Index for an invalid room.
		static const U32			InvalidRoom = 0xffffffff;

	private:
		C_System*					m_pParentSystem;		//!< Parent system.
		Btl::Vector<U32>			m_vPortals;				//!< List of portal indices that are in this room contains.
		Util::C_StringId			m_iNameIndex;			//!< Index into the string table for room name.
		U32							m_iSpaceIndex;			//!< Index into the space definition table in C_System.
		Btl::Vector<C_RoomPolygon>	m_vPolys;				//!< List of polygons in the room -- portal polygons are stored toward the end of the list.

		static Debug::Int			m_iPolygonIndex;		//!< Index of polygon to draw.

		// Disallow calls to the following:
		explicit C_Room();
		const C_Room& operator = (const C_Room& rkCopy);
	};

	// Include implementations of templated methods in C_Room.
	#include <Portal/Room.inl>
};

#endif // PORTAL_ROOM_H
