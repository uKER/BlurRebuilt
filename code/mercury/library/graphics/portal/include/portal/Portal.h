#ifndef PORTAL_PORTAL_H
#define PORTAL_PORTAL_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Portal
{
	// Call these forward.
	class C_Room;
	class C_Portal;
	class C_System;

	//! A single portal.
	class C_Portal
	{
	public:
		//! Default c'tor.
		explicit C_Portal();
		//! D'tor.
		~C_Portal();

		//! Deserialise a portal from a stream.
		//@param rkStream The stream from which to serialise the portal object.
		template <typename _T>
		void					Deserialise(_T& rkStream);
		//! Debug render.
		//@param pDebugRender Pointer to an instance of the debug renderer.
		//@param rkSystem Owning portal system object.
		void					DebugRender(DbUtil::I_DebugRenderPtr pDebugRender, const C_System& rkSystem, const bool bBlocked, const U32 iPortalIndex);
		//! Get the portal which is connected to this one.
		//@return Index of the portal which is connected to this one.
		const U32				GetCounterpartPortal() const;
		//! Return the room that this portal is a part of.
		//@return Index of the room that contains this portal.
		const U32				GetParentRoomIndex() const;
		//! Return the support plane.
		//@return The plane for this portal.
		const Maths::C_Plane&	GetPlane() const;
		//! Compute the screen-space area of the portal's polygon.
		//@param rkViewProj The view-projection matrix.
		//@return The screen-space area of the portal's polygon.
		const Maths::HF32		GetScreenSpaceArea(const Maths::HMat44& rkViewProj) const;
		//! Return a vertex at a specified index.
		//@param iIndex Index for which the vertex is desired.
		//@return Vertex of the portal at a specified index.
		const Maths::C_Vertex&	GetVertexAtIndex(U32 iIndex) const;
		//! Return the number of vertices in the portal polygon.
		//@return Number of vertices in the portal.
		const U32				GetVertexCount() const;
		//! Return a pointer to the vertices in the polygon.
		//@return Pointer to the vertex data.
		const Maths::C_Vertex*	GetVertexPtr() const;
		
		//! Value of an invalid portal.
		static const U32		InvalidPortal = 0xffffffff;

	private:
		Maths::C_Plane				m_Plane;				//!< Plane of the portal (normal points into the containing room).
		U32							m_iPortalIndex;			//!< Counterpart portal in another room.
		U32							m_iRoomIndex;			//!< Room that contains the portal.
		Btl::Vector<Maths::C_Vertex>
									m_vVertexPos;			//!< Vertex positions for the portal.
	};

	// Include implementations of templated methods in C_Portal.
	#include <Portal/Portal.inl>
};

#endif // PORTAL_PORTAL_H
