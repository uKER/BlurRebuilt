#include <stdafx.h>
#include <Portal/BuilderRoom.h>
#include <Portal/Builder.h>
#include <Portal/BuilderPolygon.h>

using namespace Portal;


// Default c'tor.
C_BuilderRoom::C_BuilderRoom(const char *pName, C_Builder* pBuilder) : 
	m_pBuilder(pBuilder),
	m_Name(pName)
{
	return;
}

// -----------------------------------------------------------------

// D'tor.
C_BuilderRoom::~C_BuilderRoom()
{
	// Placeholder method.
	return;
}


// -----------------------------------------------------------------

// Add a portal to the current room.
void C_BuilderRoom::AddPortal(C_BuilderPortal* pPortal)
{
	m_vPortals.PushBack(pPortal);
	return;
}

// -----------------------------------------------------------------

// Compute the extents of the room.
void C_BuilderRoom::ComputeExtents()
{
	m_Extents.SetZeroExtents();
	
	Btl::List<C_BuilderPolygon>::Iterator polygonIterator = m_vPolys.Begin();
	for(; polygonIterator != m_vPolys.End(); ++polygonIterator)
	{
		Btl::List<C_BuilderVertex*>::Iterator vertexIterator = (*polygonIterator).GetVertexList().Begin();
		for(; vertexIterator != (*polygonIterator).GetVertexList().End(); ++vertexIterator)
			m_Extents.UpdateExtents((*vertexIterator)->GetPosition());
	}
	return;
}

// -----------------------------------------------------------------

// Create a new room polygon and return a pointer to it.
C_BuilderPolygon* C_BuilderRoom::CreatePolygon(U32 iBaseIndex, U32 iAftkPolygonIndex, Aftk::C_SceneMeshNode* pAftkSceneMeshNode)
{
	m_vPolys.PushBack(C_BuilderPolygon(this, iBaseIndex, iAftkPolygonIndex, pAftkSceneMeshNode));
	return &m_vPolys.Back();
}

// -----------------------------------------------------------------

// Return pointer to the builder object.
C_Builder* C_BuilderRoom::GetBuilder()
{
	return m_pBuilder;
}

// -----------------------------------------------------------------

// Return the extents of the room.
const Maths::C_AxisBox& C_BuilderRoom::GetExtents() const
{
	return m_Extents;
}

// -----------------------------------------------------------------

// Return the number of polygons in the room.
U32 C_BuilderRoom::GetPolygonCount() const
{
	return m_vPolys.Size();
}

// -----------------------------------------------------------------

// Return the polygon at the specified index.
Btl::List<C_BuilderPolygon>& C_BuilderRoom::GetPolygonList()
{
	return m_vPolys;
}

// -----------------------------------------------------------------

// Return the polygon at the specified index.
const Btl::List<C_BuilderPolygon>& C_BuilderRoom::GetPolygonList() const
{
	return m_vPolys;
}

// -----------------------------------------------------------------

// Return the number of portals in the room.
U32 C_BuilderRoom::GetPortalCount() const
{
	return m_vPortals.Size();
}

// -----------------------------------------------------------------

// Return the list of portals in the room.
Btl::List<C_BuilderPortal*>& C_BuilderRoom::GetPortalList()
{
	return m_vPortals;
}

// -----------------------------------------------------------------

// Return the list of portals in the room.
const Btl::List<C_BuilderPortal*>& C_BuilderRoom::GetPortalList() const
{
	return m_vPortals;
}

// -----------------------------------------------------------------

// Return the name of the room.
const Btl::String& C_BuilderRoom::GetName() const
{
	return m_Name;
}

// -----------------------------------------------------------------

// Return the connected region Id in the room BSP tree for this room.
const S32 C_BuilderRoom::GetRegionId() const
{
	return m_iRegionId;
}

// -----------------------------------------------------------------

// Return the index for the space defintion for this room.
C_BuilderRoomContainmentSpace* C_BuilderRoom::GetSpace()
{
	return m_pSpace;
}

// -----------------------------------------------------------------

// Return the index for the space defintion for this room.
const C_BuilderRoomContainmentSpace* C_BuilderRoom::GetSpace() const
{
	return m_pSpace;
}

// -----------------------------------------------------------------

// Return the number of sub-spaces used by the room.
const U32 C_BuilderRoom::GetSubSpaceCount() const
{
	return m_pSpace->GetSubSpaceCount();
}

// -----------------------------------------------------------------

// Return true if ray intersects room:
//		pOutDistance - The distance along the ray that the intersection occurs.
//		pOutNormal	 - The normal at the point of intersection.
const bool C_BuilderRoom::Intersects(const Maths::HVec3& rkOrigin, const Maths::HVec3& rkDir, F32 fMaxT, F32* pOutDistance, Maths::HVec3* pOutNormal) const
{
	BIZ_ASSERT_FAIL("Don't use this method!");
	return false;
}

// -----------------------------------------------------------------

// Remove polygon from the room.
Btl::List<C_BuilderPolygon>::Iterator C_BuilderRoom::RemovePolygon(C_BuilderPolygon* pPoly)
{
	BIZ_WARNING_MSG(pPoly, "Polygon pointer is NULL.");
	if(!pPoly)
		return m_vPolys.End();

	// Remove polygon from all verts.
	for(Btl::List<C_BuilderVertex*>::Iterator currentVert = pPoly->GetVertexList().Begin(); currentVert != pPoly->GetVertexList().End(); ++currentVert)
	{
		for(Btl::List<C_BuilderPolygon*>::Iterator currentParentPoly = (*currentVert)->GetPolygonList().Begin(); currentParentPoly != (*currentVert)->GetPolygonList().End(); /*nothing*/)
		{
			if(*currentParentPoly == pPoly)
				currentParentPoly = (*currentVert)->GetPolygonList().Erase(currentParentPoly);
			else
				++currentParentPoly;
		}
	}
	
	for(Btl::List<C_BuilderEdge*>::Iterator currentEdge = pPoly->GetEdgeList().Begin(); currentEdge != pPoly->GetEdgeList().End(); ++currentEdge)
	{
		for(Btl::List<C_BuilderPolygon*>::Iterator currentParentPoly = (*currentEdge)->GetPolygonList().Begin(); currentParentPoly != (*currentEdge)->GetPolygonList().End(); /*nothing*/)
		{
			if(*currentParentPoly == pPoly)
				currentParentPoly = (*currentEdge)->GetPolygonList().Erase(currentParentPoly);
			else
				++currentParentPoly;
		}
	}
	
	// Remove polygon from the room's polygon list.
	Btl::List<C_BuilderPolygon>::Iterator currentPoly = m_vPolys.Begin();
	for(; currentPoly!=m_vPolys.End(); ++currentPoly)
		if(&*currentPoly == pPoly)
			break;
			
	if(currentPoly != m_vPolys.End())
		currentPoly = m_vPolys.Erase(currentPoly);
		
	return currentPoly;
}

// -----------------------------------------------------------------

// Set the Region Id for room (used for connectivity to room BSP).
void C_BuilderRoom::SetRegionId(S32 iRegionId)
{
	m_iRegionId = iRegionId;
	return;
}

// -----------------------------------------------------------------

// Set space def on the room.
void C_BuilderRoom::SetSpace(C_BuilderRoomContainmentSpace* pSpace)
{
	m_pSpace = pSpace;
	return;
}
