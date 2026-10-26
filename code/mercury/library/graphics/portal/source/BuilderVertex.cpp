#include <stdafx.h>
#include <Portal/BuilderVertex.h>

using namespace Portal;


// Default c'tor.
C_BuilderVertex::C_BuilderVertex(const Maths::HVec3& rkPos) :
	m_Position(rkPos)
{
	return;
}

// -----------------------------------------------------------------

// D'tor.
C_BuilderVertex::~C_BuilderVertex()
{
	Reset();
	return;
}

// -----------------------------------------------------------------

// Add a new containing edge to the vertex.
void C_BuilderVertex::AddEdge(C_BuilderEdge* pEdge)
{
	m_vEdges.PushBack(pEdge);
	return;
}

// -----------------------------------------------------------------

// Add a new containing polygon to the vertex.
void C_BuilderVertex::AddPolygon(C_BuilderPolygon* pPolygon)
{
	m_vPolygons.PushBack(pPolygon);
	return;
}

// -----------------------------------------------------------------

// Return the number of edges the polygon is a member of.
NOALIAS const U32 C_BuilderVertex::GetContainingEdgeCount() const __restrict
{
	return static_cast<U32>(m_vEdges.Size());
}

// -----------------------------------------------------------------

// Return the number of polygons this vertex is a member of.
NOALIAS const U32 C_BuilderVertex::GetContainingPolygonCount() const __restrict
{
	return static_cast<U32>(m_vPolygons.Size());
}

// -----------------------------------------------------------------

// Return reference to list of edges that contain this vertex.
Btl::List<C_BuilderEdge*>& C_BuilderVertex::GetEdgeList()
{
	return m_vEdges;
}

// -----------------------------------------------------------------

// Return reference to list of polygons that contain this vertex.
Btl::List<C_BuilderPolygon*>& C_BuilderVertex::GetPolygonList()
{
	return m_vPolygons;
}

// -----------------------------------------------------------------

// Return the position of the vertex.
const Maths::HVec3&	C_BuilderVertex::GetPosition() const
{
	return m_Position;
}

// -----------------------------------------------------------------

// Reset the vertex.
void C_BuilderVertex::Reset()
{
	m_vEdges.Clear();
	m_vPolygons.Clear();
	return;
}

// -----------------------------------------------------------------

// Set the position of the vertex.
void C_BuilderVertex::SetPosition(const Maths::HVec3& rkPosition)
{
	m_Position = rkPosition;
	return;
}
