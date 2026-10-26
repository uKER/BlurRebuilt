#ifndef PORTAL_ROOMPOLYGON_H
#define PORTAL_ROOMPOLYGON_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Portal/Configuration.h>


// Call this forward.
namespace DbUtil
{
	class I_DebugRender;
	typedef I_DebugRender* I_DebugRenderPtr;
};

namespace Portal
{
	//! A room polygon.
	class C_RoomPolygon
	{
	public:
		//! Default c'tor.
		explicit C_RoomPolygon();
		//! D'tor.
		~C_RoomPolygon();

		//! Debug render the polygon.
		//@param pDebugRender Pointer to an instance of a debug renderer.
		//@param rkColour Colour the polygon should be rendered.
		void				DebugRender(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::RGBA& rkColour) const;
		//! Deserialise a room polygon.
		//@param rkStream Stream from which to deserialise a room polygon.
		template <typename _T>
		void				Deserialise(_T& rkStream);
		//! Return the vertex at specified index.
		//@param iIndex Index for which a vertex is desired.
		//@return The vertex for a specified index.
		const Maths::HVec3& GetVertexAtIndex(U32 iIndex) const;
		//! Return the number of vertices in the polygon.
		//@return The number of vertices in the room.
		const U32			GetVertexCount() const;
		
	private:
		Btl::Vector<Maths::HVec3>		m_vVertexData;	//!< Vertex data for polygon.
	};

	// Include implementations of templated methods.
	#include <Portal/RoomPolygon.inl>
};

#endif // PORTAL_ROOMPOLYGON_H
