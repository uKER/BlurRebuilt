#include <stdafx.h>
#include <Portal/BuilderPolygon.h>
#include <Portal/BuilderRoom.h>
#include <Portal/Builder.h>

using namespace Portal;

// Call this forward.
namespace Tools
{
	extern Debug::Logger& g_log;
}
	

// Default c'tor.
C_BuilderPolygon::C_BuilderPolygon(C_BuilderRoom* pParentRoom, U32 iBaseIndex, U32 iAftkPolygonIndex, Aftk::C_SceneMeshNode* pAftkSceneMeshNode) :
	m_pParentRoom(pParentRoom),
	m_iBaseIndex(iBaseIndex),
	m_bIsAdjoining(false),
	m_pCounterpart(NULL),
	m_pPortal(NULL),
	m_iAftkPolygonIndex(iAftkPolygonIndex),
	m_pAftkSceneMeshNode(pAftkSceneMeshNode)
{
	BIZ_WARNING_MSG(pParentRoom, "Parent room pointer is NULL");
	return;
}

// -----------------------------------------------------------------

// D'tor.
C_BuilderPolygon::~C_BuilderPolygon()
{
	return;
}

// -----------------------------------------------------------------

// Build the supporting polygon data.
void C_BuilderPolygon::Build()
{
	// Work out all the edge data for the polygon.
	C_BuilderVertex* pVert2 = m_vVertexData.Back();
	for(Btl::List<C_BuilderVertex*>::Iterator currentVert = m_vVertexData.Begin(); currentVert!=m_vVertexData.End(); ++currentVert)
	{
		C_BuilderVertex*	pVert1 = (*currentVert);
		C_BuilderEdge*		pEdge = m_pParentRoom->GetBuilder()->CreateEdge(pVert2, pVert1);
		BIZ_WARNING_MSG(pEdge, "Edge was not created.");
		m_vEdgeData.PushBack(pEdge);
		
		// Link up verts, edges and poly refs.
		pVert1->AddEdge(pEdge);
		pVert2->AddEdge(pEdge);
		pEdge->AddPolygon(this);

		pVert2 = pVert1;
	}

	BIZ_WARNING_MSG(m_vEdgeData.Size() == m_vVertexData.Size(), "Sanity check failed, verts should equal edges!");			// Sanity test -- Equal number of edges to verts.
	CalculatePlane();
	return;
}

// -----------------------------------------------------------------

// Calculate the plane in which the polygon lies.
void C_BuilderPolygon::CalculatePlane()
{
	BIZ_WARNING_MSG(m_vVertexData.Size() >= 3, "Can't calculate plane for polygon with less than 3 vertices.");
	Maths::HVec3	verts[3];
	Btl::List<C_BuilderVertex*>::Iterator vertIterator = m_vVertexData.Begin();

	// Shove the first 3 verts from the polygon into the verts array.
	for(U32 iCurrentVert=0; iCurrentVert<3; ++iCurrentVert, ++vertIterator)
		verts[iCurrentVert] = (*vertIterator)->GetPosition();
	
	m_Plane = Maths::C_Plane(verts[0], verts[1], verts[2]);
	return;
}

// -----------------------------------------------------------------

// Create a new vertex in the polygon.
C_BuilderVertex* C_BuilderPolygon::CreateVertex(const Maths::HVec3& rkPos)
{
	C_BuilderVertex* pVertex = NULL;
	pVertex = m_pParentRoom->GetBuilder()->CreateVertex(rkPos);
	BIZ_WARNING_MSG(pVertex, "Couldn't create a vertex.");
	m_vVertexData.PushBack(pVertex);
	pVertex->AddPolygon(this);
	return pVertex;
}

// -----------------------------------------------------------------

// Output contents of polygon to TTY.
void C_BuilderPolygon::Dump()
{
	LOGDEBUG(Tools::g_log, "Room - '%s'\n", m_pParentRoom->GetName().Chars());
	if(m_bIsAdjoining)
	{
		if(m_pCounterpart)
			LOGDEBUG(Tools::g_log, "Adjoining %x in '%s'\n", (U32)m_pCounterpart, m_pCounterpart->m_pParentRoom->GetName().Chars());
		else
			LOGDEBUG(Tools::g_log, "Marked as Adjoining\n");
	}
	else
		LOGDEBUG(Tools::g_log, "Not Adjoining\n");

	for(Btl::List<C_BuilderVertex*>::Iterator currentVertex = m_vVertexData.Begin(); currentVertex != m_vVertexData.End(); ++currentVertex)
		LOGDEBUG(Tools::g_log, "Vertex - %x : X:%.3f, Y:%.3f, Z:%.3f\n", (U32)(*currentVertex), (F32)(*currentVertex)->GetPosition().X(), 
																								(F32)(*currentVertex)->GetPosition().Y(), 
																								(F32)(*currentVertex)->GetPosition().Z());
	
	return;
}

// -----------------------------------------------------------------

// Return the pointer to owning aftk scene mesh node.
Aftk::C_SceneMeshNode* C_BuilderPolygon::GetAftkSceneMeshNodePtr()
{
	return m_pAftkSceneMeshNode;
}

// -----------------------------------------------------------------

// Return polygon index into aftk mesh node.
const U32 C_BuilderPolygon::GetAftkPolygonIndex() const
{
	return m_iAftkPolygonIndex;
}

// -----------------------------------------------------------------

// Return the base index of the polygon.
U32 C_BuilderPolygon::GetBaseIndex() const
{
	return m_iBaseIndex;
}

// -----------------------------------------------------------------

// Return the counter part polygon to this polygon.
C_BuilderPolygon* C_BuilderPolygon::GetCounterpart()
{
	return m_pCounterpart;
}

// -----------------------------------------------------------------

// Return the edge at a specified index.
Btl::List<C_BuilderEdge*>& C_BuilderPolygon::GetEdgeList() 
{
	return m_vEdgeData;
}

// -----------------------------------------------------------------

// Return the number of edges in the current polygon.
const U32 C_BuilderPolygon::GetEdgeCount() const
{
	return m_vEdgeData.Size();
}

// -----------------------------------------------------------------

// Return pointer to parent room.
C_BuilderRoom* C_BuilderPolygon::GetParentRoom()
{
	return m_pParentRoom;
}

// -----------------------------------------------------------------

// Return the plane that the polygon lays in.
const Maths::C_Plane& C_BuilderPolygon::GetPlane() const
{
	return m_Plane;
}

// -----------------------------------------------------------------

// Return the portal pointer for this polygon.
C_BuilderPortal* C_BuilderPolygon::GetPortal()
{
	return m_pPortal;
}

// -----------------------------------------------------------------

// Return the vertex at the specified index.
Btl::List<C_BuilderVertex*>& C_BuilderPolygon::GetVertexList() 
{
	return m_vVertexData;
}

// -----------------------------------------------------------------

// Return the vertex at the specified index.
const Btl::List<C_BuilderVertex*>& C_BuilderPolygon::GetVertexList() const
{
	return m_vVertexData;
}


// -----------------------------------------------------------------

// Return the number of vertices in the polygon.
const U32 C_BuilderPolygon::GetVertexCount() const
{
	return static_cast<U32>(m_vVertexData.Size());
}

// -----------------------------------------------------------------

// Return true if the polygon has the argument vertex.
const bool C_BuilderPolygon::HasVertex(const C_BuilderVertex* pVertex) const
{
	Btl::List<C_BuilderVertex*>::ConstIterator vertIterator = m_vVertexData.Begin();
	for(; vertIterator != m_vVertexData.End(); ++vertIterator)
	{
		volatile Portal::C_BuilderVertex* pMoo = (*vertIterator);
		if((*vertIterator) == pVertex)
			return true;	// Found it!
	}
	return false;
}

// -----------------------------------------------------------------

// Return true if the argument polygon is Co-incident with the current polygon.
const bool C_BuilderPolygon::IsCoincident(C_BuilderPolygon* pPoly) const
{
	Btl::List<C_BuilderVertex*>::Iterator currentVertex = pPoly->GetVertexList().Begin();
	while(currentVertex != pPoly->GetVertexList().End())
	{
		C_BuilderVertex* pCurrentVertex = *currentVertex;
		if(!HasVertex(pCurrentVertex))
			return false;
			
		++currentVertex;
	}
	
	return true;
}

// -----------------------------------------------------------------

// Returns true if polygon is part of a portal.
NOALIAS const bool C_BuilderPolygon::IsAdjoining() const __restrict
{
	return m_bIsAdjoining;
}

// -----------------------------------------------------------------

// Set the portal polygon flag to true.
void C_BuilderPolygon::MakeAdjoining()
{
	m_bIsAdjoining = true;
	return;
}

// -----------------------------------------------------------------

// Merge two polygons into the current one -- new polygon does not build edges/vertices.
C_BuilderPolygon* C_BuilderPolygon::Merge(C_BuilderPolygon* pPoly1, C_BuilderPolygon* pPoly2, C_BuilderEdge* pEdge)
{
	BIZ_WARNING_MSG(pPoly1, "Sanity check failed, invalid pointer.");			// Sanity test -- invalid pointer.
	BIZ_WARNING_MSG(pPoly2, "Sanity check failed, invalid pointer.");			// Sanity test -- invalid pointer.
	BIZ_WARNING_MSG(pEdge, "Sanity check failed, invalid pointer.");			// Sanity test -- invalid pointer.
	pPoly1->Dump();
	pPoly2->Dump();

	C_BuilderPolygon*						pNewPoly = m_pParentRoom->CreatePolygon(pPoly1->m_iBaseIndex, pPoly1->m_iAftkPolygonIndex, pPoly1->m_pAftkSceneMeshNode);
	C_BuilderVertex*						pPreviousVertex = pPoly1->GetVertexList().Back();
	C_BuilderVertex*						pCurrentVertex = NULL;
	Btl::List<C_BuilderVertex*>::Iterator	vertIterator = pPoly1->GetVertexList().Begin();
	for(; vertIterator != pPoly1->GetVertexList().End(); ++vertIterator)
	{
		pCurrentVertex = (*vertIterator);
		if(pEdge->ContainsVertices(pCurrentVertex, pPreviousVertex))
			break;

		pPreviousVertex = pCurrentVertex;
	}

	// Output all the vertices in polygon 1 starting from the edge we're collapsing (don't write last edge vert!).
	U32 iPoly1VertexCount = pPoly1->GetVertexCount() - 1;
	while(iPoly1VertexCount--)
	{
		pNewPoly->CreateVertex((*vertIterator)->GetPosition());
		
		// Move around the polygon, wrap if needed.
		++vertIterator;
		if(vertIterator == pPoly1->GetVertexList().End())
			vertIterator = pPoly1->GetVertexList().Begin();
	}

	// Find the edge we're collapsing in polygon 2.
	pPreviousVertex = pPoly2->GetVertexList().Back();
	vertIterator = pPoly2->GetVertexList().Begin();
	for(; vertIterator != pPoly2->GetVertexList().End(); ++vertIterator)
	{
		pCurrentVertex = (*vertIterator);
		if(pEdge->ContainsVertices(pCurrentVertex, pPreviousVertex))
			break;

		pPreviousVertex = pCurrentVertex;
	}

	// Output the verts of polygon 2, need to skip the edge first.
	U32 iPoly2VertexCount = pPoly2->GetVertexCount() - 1;
	while(iPoly2VertexCount--)
	{
		pNewPoly->CreateVertex((*vertIterator)->GetPosition());
		
		// Move and wrap (if needed).
		++vertIterator;
		if(vertIterator == pPoly2->GetVertexList().End())
			vertIterator = pPoly2->GetVertexList().Begin();
	}

	pNewPoly->Dump();
	
	return pNewPoly;
}

// -----------------------------------------------------------------

// Set the adjoining flag to false.
void C_BuilderPolygon::RemoveAdjoiningFlag()
{
	m_bIsAdjoining = false;
	return;
}

// -----------------------------------------------------------------

// Set the room polygon.
void C_BuilderPolygon::SetCounterpart(C_BuilderPolygon* pCounterpart)
{
	m_pCounterpart = pCounterpart;
	return;
}

// -----------------------------------------------------------------

// Set the plane.
void C_BuilderPolygon::SetPlane(const Maths::C_Plane& rkPlane)
{
	m_Plane = rkPlane;
	return;
}

// -----------------------------------------------------------------

// Set the portal for the current polygon.
void C_BuilderPolygon::SetPortal(C_BuilderPortal* pPortal)
{
	m_pPortal = pPortal;
	return;
}

// -----------------------------------------------------------------

// Fix vertices of polygon to average plane -- has side effect of computing plane.
void C_BuilderPolygon::SnapVerticesToAveragePlane()
{
	BIZ_WARNING_MSG(m_vVertexData.Size() >= 3, "Cannot snap verticies to plane if polygon contains less than 3 vertices.");	// Invalid polygon.
	if(m_vVertexData.Size() < 3)
		return;

	Maths::C_Plane	averagePlane;
	Maths::HVec3	normal(0.0f, 0.0f, 0.0f);
	Maths::HVec3	centroid(0.0f, 0.0f, 0.0f);
	Maths::HVec3	prevVertex = m_vVertexData.Back()->GetPosition();

	// Calculate and preserve average plane -- Newell's method.
	Btl::List<C_BuilderVertex*>::Iterator vertIterator = m_vVertexData.Begin();
	for(; vertIterator != m_vVertexData.End(); ++vertIterator)
	{
		Maths::HVec3 currentVertex = (*vertIterator)->GetPosition();
		Maths::HF32 fX = (currentVertex.Y() - prevVertex.Y()) * (currentVertex.Z() + prevVertex.Z());
		Maths::HF32 fY = (currentVertex.Z() - prevVertex.Z()) * (currentVertex.X() + prevVertex.X());
		Maths::HF32 fZ = (currentVertex.X() - prevVertex.X()) * (currentVertex.Y() + prevVertex.Y());
		normal		+= Maths::HVec3(fX, fY, fZ);
		centroid	+= currentVertex;
		prevVertex	= currentVertex;
	}

	centroid *= 1.0f/static_cast<F32>(m_vVertexData.Size());
	averagePlane = Maths::C_Plane(Maths::C_Normal(normal), Maths::C_Vertex(centroid));
	m_Plane = averagePlane;

	// Do another pass and project vertices onto the average plane.
	vertIterator = m_vVertexData.Begin();
	for(; vertIterator != m_vVertexData.End(); ++vertIterator)
	{
		Maths::HVec3 currentVertex = (*vertIterator)->GetPosition();
		Maths::HVec3 newVertexPosition = averagePlane.OnPlane(currentVertex);
		(*vertIterator)->SetPosition(newVertexPosition);
	}
	
	/*
	Q = P - ((P-O).N)N
	Q = resultant point
	P = point to project
	O = any point on the plane
	N = plane normal
	*/

	return;
}
