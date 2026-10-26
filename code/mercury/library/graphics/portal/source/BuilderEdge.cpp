#include <stdafx.h>
#include <btl/containers/algorithm.h>
#include <Portal/BuilderEdge.h>

using namespace Portal;


// Main c'tor -- takes two vertex pointers.
C_BuilderEdge::C_BuilderEdge(C_BuilderVertex* pVert1, C_BuilderVertex* pVert2)
{
	BIZ_WARNING_MSG(pVert1 && pVert2, "Null vertex pointers are not recommended!");		// NULL vertex pointers not recommended =).
	m_vVerts.PushBack(pVert1);
	m_vVerts.PushBack(pVert2);
	return;
}

// -----------------------------------------------------------------

// D'tor.
C_BuilderEdge::~C_BuilderEdge()
{
	return;
}

// -----------------------------------------------------------------

// Add a polygon that contains this edge.
void C_BuilderEdge::AddPolygon(Portal::C_BuilderPolygon *pPoly)
{
	BIZ_WARNING_MSG(pPoly, "Polygon pointer is NULL.");
	m_vPolygons.PushBack(pPoly);
	return;
}

// -----------------------------------------------------------------

// Return true if edge contains the specified vertices.
const bool C_BuilderEdge::ContainsVertices(C_BuilderVertex* pVert1, C_BuilderVertex* pVert2)
{
	if((m_vVerts[0] == pVert1 && m_vVerts[1] == pVert2) ||
	   (m_vVerts[0] == pVert2 && m_vVerts[1] == pVert1))
			return true;
	
	return false;
}

// -----------------------------------------------------------------

// Return a pointer to the polygon that shares this edge at the specified index.
Btl::List<C_BuilderPolygon*>& C_BuilderEdge::GetPolygonList()
{
	return m_vPolygons;
}

// -----------------------------------------------------------------

// Return the number of polygons that share this edge.
const U32 C_BuilderEdge::GetPolygonCount() const
{
	return static_cast<U32>(m_vPolygons.Size());
}

// -----------------------------------------------------------------

// Remove polygon reference from the edge.
void C_BuilderEdge::RemovePolygon(Portal::C_BuilderPolygon *pPoly)
{
	BIZ_WARNING_MSG(pPoly, "Polygon pointer is NULL.");
	Btl::List<C_BuilderPolygon*>::Iterator result = Btl::Find(m_vPolygons, pPoly);
	if(result != m_vPolygons.End())
		m_vPolygons.Erase(result);

	return;
}

// -----------------------------------------------------------------


