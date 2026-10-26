#ifndef PORTAL_BUILDERPOLYGON_H
#define PORTAL_BUILDERPOLYGON_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Portal/BuilderVertex.h>
#include <Portal/BuilderEdge.h>

namespace Aftk
{
	class C_SceneMeshNode;
};

namespace Portal
{
	// Call these forward.
	class C_BuilderRoom;
	class C_BuilderPortal;
	
	//! A room polygon.
	class C_BuilderPolygon
	{
	public:
		//! C'tor.
		//@param pParentRoom Pointer to the room that owns this polygon.
		//@param iBaseIndex Base index (from Aftk node) for the polygon.
		explicit C_BuilderPolygon(C_BuilderRoom* pParentRoom, U32 iBaseIndex, U32 iAftkPolygonIndex, Aftk::C_SceneMeshNode* pAftkSceneMeshNode);
		//! D'tor.
		~C_BuilderPolygon();

		//! Build the polygon.
		void								Build();
		//! Create a new vertex.
		//@param rkPos New vertex.
		//@return Pointer to the newly created vertex.
		C_BuilderVertex*					CreateVertex(const Maths::HVec3& rkPos);
		//! Dump the polygon contents to the TTY.
		void								Dump();
		//! Return pointer to the aftk scene node.
		//@return Pointer to AFTK scene mesh node that originally contained polygon.
		Aftk::C_SceneMeshNode*				GetAftkSceneMeshNodePtr();
		//! Return polygon index in aftk mesh node.
		//@return Index of polygon in original aftk mesh.
		const U32							GetAftkPolygonIndex() const;
		//! Return the base index of the polygon.
		//@return The base index.
		U32									GetBaseIndex() const;
		//! Get the counterpart polygon to this one, only a valid thing to do if the adjoining flag is set.
		//@return Pointer to adjoining polygon.
		C_BuilderPolygon*					GetCounterpart();
		//! Get a list of edges in the polygon.
		//@return Reference to the list of edges.
		Btl::List<C_BuilderEdge*>&			GetEdgeList();
		//! Return the number of edges.
		//@return Number of edges.
		const U32							GetEdgeCount() const;
		//! Get the parent room.
		//@return Pointer to the parent room.
		C_BuilderRoom*						GetParentRoom();
		//! Return the plane that the polygon lays in.
		//@return World-space plane.
		const Maths::C_Plane&				GetPlane() const;
		//! Return the portal for this polygon, not always valid, only a valid thing to do if the adjoining flag is set.
		//@return Pointer to the portal.
		C_BuilderPortal*					GetPortal();
		//! Return the vertex at specified index.
		//@return Reference to list of vertices.
		Btl::List<C_BuilderVertex*>&		GetVertexList();
		//! Return the vertex at specified index.
		//@return Reference to list of vertices.
		const Btl::List<C_BuilderVertex*>&	GetVertexList() const;
		//! Return the number of vertices in the polygon.
		//@return Number of vertices in the polygon.
		const U32							GetVertexCount() const;
		//! Return true if the polygon has the argument vertex.
		//@param pVertex Pointer to vertex.
		//@return True if polygon contains vertex.
		const bool							HasVertex(const C_BuilderVertex* pVertex) const;
		//! Return true if the polygon is co-incident.
		//@param pPoly Pointer to polygon.
		//@return True if polygons are the same.
		const bool							IsCoincident(C_BuilderPolygon* pPoly) const;
		//! Return true if the polygon is adjoining.
		//@return True if adjoining flag is set on polygon.
		NOALIAS const bool					IsAdjoining() const __restrict;
		//! Set adjoining flag as true.
		void								MakeAdjoining();
		//! Merge two polygons that share an edge into a new polygon in the same room.
		//@param pPoly1 Pointer to polygon.
		//@param pPoly2 Pointer to polygon.
		//@param pEdge Pointer to edge to collapse along.
		//@return Newly created merged polygon.
		C_BuilderPolygon*					Merge(C_BuilderPolygon* pPoly1, C_BuilderPolygon* pPoly2, C_BuilderEdge* pEdge);
		//! Remove adjoining flag.
		void								RemoveAdjoiningFlag();
		//! Set the counterpart polygon pointer.
		//@param pCounterpart Pointer to counterpart portal.
		void								SetCounterpart(C_BuilderPolygon* pCounterpart);
		//! Set the plane on the polygon -- use with care!!
		//@param rkPlane World-space plane.
		void								SetPlane(const Maths::C_Plane& rkPlane);
		//! Set the portal pointer.
		//@param pPortal Portal pointer that the polygon is part of.
		void								SetPortal(C_BuilderPortal* pPortal);
		//! Fix polygon to average plane.
		void								SnapVerticesToAveragePlane();
		
	private:
		C_BuilderRoom*					m_pParentRoom;			//!< Room containing the polygon.
		Btl::List<C_BuilderVertex*>		m_vVertexData;			//!< Vertex data for polygon.
		Btl::List<C_BuilderEdge*>		m_vEdgeData;			//!< Edge data for polygon.
		U32								m_iBaseIndex;			//!< Base index (from Aftk vertex).
		Maths::C_Plane					m_Plane;				//!< Plane that the polygon lays in.
		bool							m_bIsAdjoining;			//!< If true polygon is a portal.
		C_BuilderPolygon*				m_pCounterpart;			//!< Counterpart polygon (only valid if adjoining flag set).
		C_BuilderPortal*				m_pPortal;				//!< Portal created for this polygon (not always valid, only valid if adjoing flag set).
		U32								m_iAftkPolygonIndex;	//!< Aftk polygon index.		 
		Aftk::C_SceneMeshNode*			m_pAftkSceneMeshNode;	//!< Pointer to scene mesh node that this room was created from.
		
		//! Calculate the plane for the polygon.
		void								CalculatePlane();

		// Disallow calls to these:
		explicit C_BuilderPolygon();
	};

};

#endif // PORTAL_BUILDERPOLYGON_H
