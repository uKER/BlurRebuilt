#ifndef PORTAL_BUILDERVERTEX_H
#define PORTAL_BUILDERVERTEX_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/List.h>


namespace Portal
{
	// Call these forward.
	class C_BuilderPolygon;
	class C_BuilderEdge;
	
	//! A room polygon.
	class C_BuilderVertex
	{
	public:
		//! Default c'tor.
		//@param rkPos Position of the vertex.
		explicit C_BuilderVertex(const Maths::HVec3& rkPos);
		//! D'tor.
		~C_BuilderVertex();

		//! Add a new containing edge to the vertex.
		//@param pEdge Pointer to edge that contains this vertex.
		void							AddEdge(C_BuilderEdge* pEdge);
		//! Add a new containing polygon to the vertex.
		//@param Pointer to the polygon that contains this vertex.
		void							AddPolygon(C_BuilderPolygon* pPolygon);
		//! Return the number of edges the polygon is a member of.
		//@return Number of edges that this vertex is part of.
		NOALIAS const U32				GetContainingEdgeCount() const __restrict;
		//! Return the number of polygons this vertex is a member of.
		//@return Number of polygons that this vertex is part of.
		NOALIAS const U32				GetContainingPolygonCount() const __restrict;
		//! Return reference to list object containing edges.
		//@return Reference to the edge list.
		Btl::List<C_BuilderEdge*>&		GetEdgeList();	
		//! Return reference to list object containing polygons.
		//@return Reference to the polygon list.
		Btl::List<C_BuilderPolygon*>&	GetPolygonList();
		//! Return the position of the vertex.
		//@return The world-space position of the vertex.
		const Maths::HVec3&				GetPosition() const;
		//! Reset the vertex.
		void							Reset();
		//! Set the position of the vertex.
		//@param rkPosition The new position of the vertex.
		void							SetPosition(const Maths::HVec3& rkPosition);
		
	private:
		Maths::HVec3					m_Position;			//!< Position of the vertex.
		Btl::List<C_BuilderEdge*>		m_vEdges;			//!< List of edges the vertex is a member of.
		Btl::List<C_BuilderPolygon*>	m_vPolygons;		//!< List of polygons the vertex is a member of.
		
		// Disallow calls to these:
		explicit C_BuilderVertex();
	};

};

#endif // PORTAL_BUILDERVERTEX_H
