#ifndef PORTAL_BUILDEREDGE_H
#define PORTAL_BUILDEREDGE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Portal/BuilderVertex.h>
#include <Portal/BuilderEdge.h>

namespace Portal
{
	// Call these forward.
	class C_BuilderRoom;
	
	//! An edge.
	class C_BuilderEdge
	{
	public:
		//! Specified c'tor -- takes 2 vert pointers.
		//@param pVert1 Pointer to first vertex in edge.
		//@param pVert2 Pointer to second vertex in edge.
		explicit C_BuilderEdge(C_BuilderVertex* pVert1, C_BuilderVertex* pVert2);
		//! D'tor.
		~C_BuilderEdge();

		//! Add containing polygon.
		//@param pPoly Pointer to polygon that the edge is part of.
		void							AddPolygon(C_BuilderPolygon* pPoly);
		//! Return true if edge contains specified vertices.
		//@param pVert1 Pointer to vertex.
		//@param pVert2 Pointer to vertex.
		//@return True if edge contains two vertices.
		const bool						ContainsVertices(C_BuilderVertex* pVert1, C_BuilderVertex* pVert2);
		//! Return the polygon pointer at a specified index.
		//@return Reference to polygons that this edge is a part of.
		Btl::List<C_BuilderPolygon*>&	GetPolygonList();
		//! Return the number of polygons this edge is shared by.
		//@return Number of polygons that share this edge.
		const U32						GetPolygonCount() const;
		//! Remove polygon.
		//@param pPoly Pointer to polygon to remove from the polygon list.
		void							RemovePolygon(C_BuilderPolygon* pPoly);
	
	private:
		Btl::Vector<C_BuilderVertex*, Btl::VectorParameters_Static<2> >	m_vVerts;		//!< 2 verts in the edge.
		Btl::List<C_BuilderPolygon*>									m_vPolygons;	//!< Polygons that contain this edge.
		
		// Disallow calls to this:
		explicit C_BuilderEdge();
	};

};

#endif // PORTAL_BUILDEREDGE_H
