#include <stdafx.h>
#include <Portal/Builder.h>
#include <Portal/System.h>
#include <Portal/BuilderRoom.h>
#include <Portal/BuilderPortal.h>
#include <Aftk/Aftk.h>
#include <AftkUt/AftkUt.h>
#include <Aftkut/AftkUtNodeFlagParser.h>
#include <converter_shared/helpers/AftkHelpers.h>
#include <Btl/Containers/algorithm.h>
#include <windows.h>

//#pragma optimize("", off)

using namespace Portal;

bool C_Builder::m_bAlwaysVisibleIndexCreated = false;
U32  C_Builder::m_iAlwaysVisibleIndex = -1;

// Call this forward.
namespace Tools
{
	extern Debug::Logger& g_log;
}


/// Default c'tor.
C_Builder::C_Builder() :
	m_pRoomGrid(NULL)
{
	return;
}

// -----------------------------------------------------------------

// D'tor.
C_Builder::~C_Builder()
{
	// Kill off the room grid.
	if(m_pRoomGrid)
		delete m_pRoomGrid;

	return;
}

// -----------------------------------------------------------------

// Calculate the room BSP tree.
void C_Builder::BuildRoomBSP()
{
	RegionBSP::C_BuildTree buildTree;
	buildTree.Initialise(0);

	// Set-up region Id's for existing rooms.
	Btl::List<C_BuilderRoom>::Iterator currentRoom = m_vRooms.Begin();
	for(U32 iRoomIndex=0; currentRoom != m_vRooms.End(); ++currentRoom, ++iRoomIndex)
		currentRoom->SetRegionId(iRoomIndex);

	currentRoom = m_vRooms.Begin();
	++currentRoom;	// Skip the world room.
	for(; currentRoom != m_vRooms.End(); ++currentRoom)
	{
		C_BuilderRoom* pCurrentRoom = &*currentRoom;
		RegionBSP::C_Polyhedron roomPolyhedron;

		Btl::List<C_BuilderPolygon>::Iterator currentPolygon = currentRoom->GetPolygonList().Begin();
		for(; currentPolygon != currentRoom->GetPolygonList().End(); ++currentPolygon)
		{
			bool bSkipCurrent = false;
			
			// Check if it's a duplicate of each other polygon.
			Btl::List<C_BuilderPolygon>::Iterator currentCheckPolygon = currentRoom->GetPolygonList().Begin();
			for(; currentCheckPolygon != currentRoom->GetPolygonList().End(); ++currentCheckPolygon)
			{
				if(currentPolygon == currentCheckPolygon)
					continue;

				if(currentPolygon->IsCoincident(&*currentCheckPolygon) && currentCheckPolygon->IsCoincident(&*currentPolygon))
				{
					bSkipCurrent = true;
					LOGDEBUG(Tools::g_log, "Room %s contains co-incident polygon -- removing.\n", currentRoom->GetName().Chars()); 
				}
			}
				
			if(currentPolygon->IsAdjoining() || bSkipCurrent)
				continue;		// Skip portal polygons.

			RegionBSP::C_Polyhedron::C_PolyhedronPolygon& polyhedronPolygon = roomPolyhedron.AddPolygon();
			
			Btl::List<Portal::C_BuilderVertex*>::ReverseIterator currentVertex = currentPolygon->GetVertexList().ReverseBegin();
			for(; currentVertex!=currentPolygon->GetVertexList().ReverseEnd(); ++currentVertex)
			{
				S32 iVertexId = roomPolyhedron.AddVertex((*currentVertex)->GetPosition());
				roomPolyhedron.AddVertexToPolygon(polyhedronPolygon, iVertexId);
			}
		}

		if(!roomPolyhedron.IsClosed())	// Found an unclosed room.
			LOGINFO(Tools::g_log, "Room %s does not form a closed volume, skipped!\n", currentRoom->GetName().Chars());
		
		buildTree.AddPolyhedron(roomPolyhedron, currentRoom->GetRegionId());	// Room is closed.
	}

	// Build from the build-tree we just constructed.
	m_RoomBSPTree.Build(buildTree);

	// Add an index for an object only in the world room.
	Btl::BitVector roomBits;
	roomBits.Resize(m_vRooms.Size());
	roomBits.ClearBits();
	roomBits.SetBitValue(0, true);	// Set world room bit.
	m_vPrimitiveContainment.PushBack(roomBits);
	m_vPrimitiveContainmentMap.Insert(roomBits, 0);
	return;
}

// -----------------------------------------------------------------

// Calculate the rooms that a specified polygon intersects and return an index into a list of BitVectors.
const U32 C_Builder::CalculateContainmentBitVector(const Aftk::C_SceneMeshNode& rkNode, const Aftk::C_Polygon& rkPoly)
{
	// Build a polygon from aftk poly.
	SimplePoly::C_Polygon	testPolygon;
	testPolygon.Reserve(rkPoly.GetNumPolygonVertices());	// Stop an avalanche of allocations.
	for(U32 iCurrentVertex=0; iCurrentVertex<rkPoly.GetNumPolygonVertices(); ++iCurrentVertex)
		testPolygon.NewVertex(AftkHelpers::GetHVec3(Aftk::ApplyMatrix(rkNode.GetWorldMatrix(), rkNode.GetVertexPosition(rkPoly.GetPolygonVertex(iCurrentVertex).GetPositionIndex()))));
	testPolygon.CalculatePlane();

	// Classify with room BSP tree.
	Ut::Vector<RegionBSP::C_RegionId> vRegions;
	m_RoomBSPTree.ClassifyPolygon(testPolygon, vRegions);
	
	// Construct bit vector.
	Btl::BitVector roomBits;
	roomBits.Resize(m_vRooms.Size());
	roomBits.ClearBits();
	for(U32 iCurrentRegion=0; iCurrentRegion<static_cast<U32>(vRegions.Size()); ++iCurrentRegion)
	{
		U32 iIndex = GetRoomIndexByRegionId(vRegions[iCurrentRegion]);
		assert(iIndex != -1);	
		if(iIndex != -1)
		{
			roomBits.SetBitValue(iIndex, true);
		}
	}

	if(roomBits[0] && roomBits.CountBitsSet() > 1)
		roomBits[0] = false;

	Btl::Map<Btl::BitVector, U32>::Iterator findResult = m_vPrimitiveContainmentMap.Find(roomBits);
	U32 iIndex = -1;
	if(findResult != m_vPrimitiveContainmentMap.End())
		return (*findResult).GetValue();	// Found the index already.
	else
	{
		m_vPrimitiveContainment.PushBack(roomBits);
		iIndex = m_vPrimitiveContainment.Size()-1;	
		m_vPrimitiveContainmentMap.Insert(roomBits, iIndex);
	}
	return iIndex;
}

// -----------------------------------------------------------------

// Calculate the rooms that a bounding box intersects and return an index into a list of BitVectors.
const U32 C_Builder::CalculateContainmentBitVector(const Aftk::C_Bounds& rkBounds)
{
	// TODO: [Stove] Implement this properly -- could be alot more accurate.
	Aftk::Position3 vSamplePoints[3];
	vSamplePoints[0] = rkBounds.GetCentre();
	vSamplePoints[1] = rkBounds.GetMin();
	vSamplePoints[2] = rkBounds.GetMax();

	Btl::BitVector roomBits;
	roomBits.Resize(m_vRooms.Size());
	roomBits.ClearBits();
	for(U32 iCurrentPoint=0; iCurrentPoint<3; ++iCurrentPoint)
	{
		// Classify sample point with room BSP tree.
		Ut::Vector<RegionBSP::C_RegionId> vRegions;
		vRegions.Reserve(3);
		vRegions.PushBack(m_RoomBSPTree.ClassifyPoint(AftkHelpers::GetHVec3(vSamplePoints[iCurrentPoint])));

		// Construct bit vector for current sample point.
		Btl::BitVector currentPointVector;
		currentPointVector.Resize(m_vRooms.Size());
		currentPointVector.ClearBits();
		for(U32 iCurrentRegion=0; iCurrentRegion<static_cast<U32>(vRegions.Size()); ++iCurrentRegion)
		{
			U32 iIndex = GetRoomIndexByRegionId(vRegions[iCurrentRegion]);
			BIZ_WARNING_MSG(iIndex != -1, "Invalid room index.");
			if(iIndex != -1)
				currentPointVector.SetBitValue(iIndex, true);
		}
		
		roomBits |= currentPointVector;
	}

	if(roomBits[0] && roomBits.CountBitsSet() > 1)
		roomBits[0] = false;
		
	Btl::Map<Btl::BitVector, U32>::Iterator findResult = m_vPrimitiveContainmentMap.Find(roomBits);
	U32 iIndex = -1;
	if(findResult != m_vPrimitiveContainmentMap.End())
		return (*findResult).GetValue();	// Found the index already.
	else
	{
		m_vPrimitiveContainment.PushBack(roomBits);
		iIndex = m_vPrimitiveContainment.Size()-1;	
		m_vPrimitiveContainmentMap.Insert(roomBits, iIndex);
	}
	return iIndex;
}

// -----------------------------------------------------------------

// Create a new edge.
C_BuilderEdge* C_Builder::CreateEdge(Portal::C_BuilderVertex *pVert1, Portal::C_BuilderVertex *pVert2)
{
	// Find the two verts in the edge list.
	for(Btl::List<C_BuilderEdge>::Iterator edgeIterator = m_vEdges.Begin(); edgeIterator!=m_vEdges.End(); ++edgeIterator)
	{
		if((*edgeIterator).ContainsVertices(pVert1, pVert2))
			return &(*edgeIterator);
	}

	// If we got this far we didn't find the edge in our edge list, 
	// so we should create a new one.
	m_vEdges.PushBack(C_BuilderEdge(pVert1, pVert2));
	return &m_vEdges.Back();
}

// -----------------------------------------------------------------

// Create a new portal and return a pointer to it.
C_BuilderPortal* C_Builder::CreatePortal(C_BuilderRoom* pParent)
{
	BIZ_WARNING_MSG(pParent, "Invalid parent pointer."); // Invalid pointer.
	m_vPortals.PushBack(C_BuilderPortal(pParent));
	return &m_vPortals.Back();
}

// -----------------------------------------------------------------

// Create a new room and return a pointer to it.
C_BuilderRoom* C_Builder::CreateRoom(const char* pName)
{
	BIZ_WARNING_MSG(pName, "Invalid name pointer for room.");				// Invalid pointer.
	BIZ_WARNING_MSG(strlen(pName) > 0, "Name is 0 characters in length.");	// Needs a name.
	m_vRooms.PushBack(C_BuilderRoom(pName, this));

	// Create space def and add it to the room we just made -- will populate it later.
	m_vSpaces.PushBack(C_BuilderRoomContainmentSpace(&m_vRooms.Back()));
	m_vRooms.Back().SetSpace(&m_vSpaces.Back());
	return &m_vRooms.Back();
}

// -----------------------------------------------------------------

// Create a new vertex.
C_BuilderVertex* C_Builder::CreateVertex(const Maths::HVec3& rkPos)
{
	// Snap to grid.
	Maths::HVec3 newPos = rkPos * 100.0f;
	newPos.X(floor(newPos.X() + 0.5f));
	newPos.Y(floor(newPos.Y() + 0.5f));
	newPos.Z(floor(newPos.Z() + 0.5f));
	newPos = newPos * 0.01f;
	
	// Check for this vertex elsewhere.
	for(Btl::List<C_BuilderVertex>::Iterator vertIterator = m_vVertices.Begin(); vertIterator != m_vVertices.End(); ++vertIterator)
	{
		F32 fDynamicEpsilon = 0.0001f * 0.0001f;	// HACK: [Stove] Yes, it's not dynamic right now... but will be :-)
		if(newPos.DistanceSquared((*vertIterator).GetPosition()) <= fDynamicEpsilon)
		//if(FloatEqual(newPos.DistanceSquared((*vertIterator).GetPosition()), 0.0f, 0.0001f, 0.0001f))
			return &(*vertIterator);	
	}
 
	// Create a new vertex if we got this far.
	m_vVertices.PushBack(C_BuilderVertex(newPos));
	return &m_vVertices.Back();
}

// -----------------------------------------------------------------

// Find the room that contains the specified point, if the point
// cannot be found in any room in the portal system then this method
// will return NULL.
C_BuilderRoom* C_Builder::FindContainingRoom(const Maths::HVec3& rkPoint)
{
	BIZ_WARNING_MSG(m_pRoomGrid, "Room grid must be setup before using this method.");		// You need to setup the containment grid before using this method.

	U32 iCellIndex = m_pRoomGrid->ComputeCellForPosition(rkPoint);
	if(iCellIndex == 0xffffffff)
		return NULL;
	const C_BuilderRoomGridCell& gridCell = m_pRoomGrid->GetCellAtIndex(iCellIndex);
	Btl::List<C_BuilderRoom*>::ConstIterator currentRoom = gridCell.GetRoomList().Begin();
	for(; currentRoom != gridCell.GetRoomList().End(); ++currentRoom)
		if((*currentRoom)->GetSpace()->Contains(rkPoint))
			break;
	
	return (currentRoom != gridCell.GetRoomList().End()) ? *currentRoom : NULL;
}

// -----------------------------------------------------------------

// Return the index for a specified portal.
const U32 C_Builder::GetIndexForPortal(const C_BuilderPortal* pPortal) const
{
	BIZ_WARNING_MSG(pPortal, "Portal pointer is NULL.");

	U32 iIndex = 0;
	for(Btl::List<C_BuilderPortal>::ConstIterator portalIterator = m_vPortals.Begin(); portalIterator != m_vPortals.End(); ++portalIterator, ++iIndex)
		if(&(*portalIterator) == pPortal)
			return iIndex;

	BIZ_ASSERT_FAIL("Couldn't find the requested portal");	// Couldn't find the requested portal.
	return 0xd153a5ed;
}

// -----------------------------------------------------------------

// Return the index for a specified room.
const U32 C_Builder::GetIndexForRoom(const C_BuilderRoom* pRoom) const
{
	BIZ_WARNING_MSG(pRoom, "Room pointer is NULL.");

	U32 iIndex = 0;
	for(Btl::List<C_BuilderRoom>::ConstIterator roomIterator = m_vRooms.Begin(); roomIterator != m_vRooms.End(); ++roomIterator, ++iIndex)
		if(&(*roomIterator) == pRoom)
			return iIndex;

	BIZ_ASSERT_FAIL("Couldn't find the requested room");	// Couldn't find the requested room.
	return 0xd153a5ed;
}

// -----------------------------------------------------------------

// Return the index for a specified space.
const U32 C_Builder::GetIndexForSpace(const C_BuilderRoomContainmentSpace* pSpace) const
{
	BIZ_WARNING_MSG(pSpace, "Space pointer is NULL.");

	U32 iIndex = 0;
	for(Btl::List<C_BuilderRoomContainmentSpace>::ConstIterator spaceIterator = m_vSpaces.Begin(); spaceIterator != m_vSpaces.End(); ++spaceIterator, ++iIndex)
		if(&(*spaceIterator) == pSpace)
			return iIndex;

	BIZ_ASSERT_FAIL("Couldn't find requested space.");	// Couldn't find the requested space.
	return 0xd153a5ed;
}

// -----------------------------------------------------------------

// Return the region BSP tree for the room.
RegionBSP::C_Tree& C_Builder::GetRoomBSPTree()
{
	return m_RoomBSPTree;
}

// -----------------------------------------------------------------

// Return a bit vector for primitive containment at a specified index.
const Btl::BitVector& C_Builder::GetPortalBitsAtIndex(const U32 iIndex) const
{
	return m_vPrimitiveContainment[iIndex];
}

// -----------------------------------------------------------------

// Return the number of portals.
NOALIAS const U32 C_Builder::GetPortalCount() const __restrict
{
	return m_vPortals.Size();
}

// -----------------------------------------------------------------

// Return the portal list.
Btl::List<C_BuilderPortal>& C_Builder::GetPortalList()
{
	return m_vPortals;
}

// -----------------------------------------------------------------

// Return the number of rooms in the builder object.
NOALIAS const U32 C_Builder::GetRoomCount() const __restrict
{
	return m_vRooms.Size();
}

// -----------------------------------------------------------------

// Return the room grid.
C_BuilderRoomGrid* C_Builder::GetRoomGrid()
{
	return m_pRoomGrid;
}

// -----------------------------------------------------------------

// Return room index from a specified region Id.
const U32 C_Builder::GetRoomIndexByRegionId(S32 iRegionId) const
{
	U32 iIndex = 0;
	for(Btl::List<C_BuilderRoom>::ConstIterator roomIterator = m_vRooms.Begin(); roomIterator != m_vRooms.End(); ++roomIterator, ++iIndex)
		if(roomIterator->GetRegionId() == iRegionId)
			return iIndex;

	return static_cast<U32>(-1);
}

// -----------------------------------------------------------------

// Return room list.
Btl::List<C_BuilderRoom>& C_Builder::GetRoomList()
{
	return m_vRooms;
}

// -----------------------------------------------------------------

// Return the number of spaces in the builder object.
NOALIAS const U32 C_Builder::GetSpaceCount() const
{
	return static_cast<U32>(m_vSpaces.Size());
}

// -----------------------------------------------------------------

// Return the space list.
Btl::List<C_BuilderRoomContainmentSpace>& C_Builder::GetSpaceList()
{
	return m_vSpaces;
}

// -----------------------------------------------------------------

// Return the string table builder.
Util::T_StringTableBuilder<Btl::String>& C_Builder::GetStringBuilder()
{
	return m_StringTableBuilder;
}

// -----------------------------------------------------------------

// Return a bit vector which represents visible.
const U32 C_Builder::GetWorldRoomOnlyBitVector()
{
	return 0;
}

// -----------------------------------------------------------------

// Setup the room grid.
void C_Builder::SetupRoomGrid(F32 fCellSize)
{
	BIZ_WARNING_MSG(fCellSize > 0.0f, "Cell size must be larger than 0.0f");

	// Work out the extents for all rooms.
	Maths::C_AxisBox extents;
	extents.SetZeroExtents();

	// Grow to contain all verts in the rooms.
	for(Btl::List<C_BuilderRoom>::Iterator roomIterator = m_vRooms.Begin(); roomIterator != m_vRooms.End(); ++roomIterator)
	{
		for(Btl::List<C_BuilderPolygon>::Iterator polyIterator = (*roomIterator).GetPolygonList().Begin(); polyIterator != (*roomIterator).GetPolygonList().End(); ++polyIterator)
		{
			for(Btl::List<C_BuilderVertex*>::Iterator vertIterator = (*polyIterator).GetVertexList().Begin(); vertIterator != (*polyIterator).GetVertexList().End(); ++vertIterator)
				extents.UpdateExtents((*vertIterator)->GetPosition());
		}
	}

	Maths::HVec3 size = extents.Max() - extents.Min();
	U32 iCellsX = static_cast<U32>(size.X() / fCellSize);
	U32 iCellsZ = static_cast<U32>(size.Z() / fCellSize);
	m_pRoomGrid = new C_BuilderRoomGrid(extents, iCellsX, iCellsZ, fCellSize);
	return;
}
		
// -----------------------------------------------------------------

#define max(a, b) (a+b+fabs(a-b))/2

// Return true if two floating point values are equivalent within some epsilon.
const bool C_Builder::FloatEqual(const F32 fX, const F32 fY, const F32 fAbsTolerance, const F32 fRelTolerance) const
{
	return ((fabs(fX - fY) <= fAbsTolerance) ||
		    (fabs(fX - fY) <= fRelTolerance * max(fabs(fX), fabs(fY))));

}
