#include "stdafx.h"
#include <Portal/Builder.h>
#include <Portal/BuilderRoom.h>
#include <Portal/BuilderPortal.h>
#include <Portal/RoomPolygon.h>
#include <Portal/BuilderRoomContainmentSpace.h>
#include <Process/BuildPortalData.h>
#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/SceneDescription.h>
#include <data/PortalData.h>
#include <converter_shared/helpers/AftkHelpers.h>
#include <misc_shared/geometry/SimpleBSP.h>

/// Default c'tor.
C_ProcessBuildPortalData::C_ProcessBuildPortalData() : 
	C_Process(),
	m_pWorldRoom(NULL)
{
	return;
}

// -----------------------------------------------------------------

/// Return the name, inherited from C_Process.
const Util::C_String C_ProcessBuildPortalData::GetName() const
{
	return "C_ProcessBuildPortalData";
}

// -----------------------------------------------------------------

/// Perform processing.
bool C_ProcessBuildPortalData::Process(Progress::I_ProgressBar* pProgress)
{
	Progress::UpdateSimple progress(pProgress, GetName());
	progress.SetProgress(0.0f);

	const C_DataConversionOptions* pInputConversionOptions = (const C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	const C_DataSceneDescription* pInputSceneDescription = (const C_DataSceneDescription*)GetInput( E_GraphData_SceneDescription );
	assert(pInputSceneDescription);

	C_DataPortalData* pOutputPortalData = NULL;
	pOutputPortalData = (C_DataPortalData*)GetOutput(E_GraphData_PortalRooms);
	assert(pOutputPortalData);

	const C_AftkNodeList& rkNodeList = pInputSceneDescription->GetPortalRoomsNodes();

	// Handle the case of no portals rooms.
	if(rkNodeList.Size() == 0)
	{
		progress.SetProgress(100.0f);
		return true;
	}
	
	// Build up the rooms and portals.
	Build(pProgress, rkNodeList, pOutputPortalData);
	progress.SetProgress(16.6f);

	// Report errors with mesh data.
	ReportCleanUpMeshes(pProgress, rkNodeList, pOutputPortalData);
	progress.SetProgress(33.0f);
	
	// Flatten portal polygons.
 	FlattenPortalPolygons(pProgress, pOutputPortalData);
 	progress.SetProgress(49.6f);
	
	// Create portals and match up the connecting portals to one another.
	SetupPortals(pProgress, pOutputPortalData);
	progress.SetProgress(65.0f);
	
	// Calculate containment data for rooms, break concave room volumes into smaller convex sub-spaces -- create one plane for each polygon in subspaces.
	BuildRoomContainmentData(pProgress, pOutputPortalData);
	progress.SetProgress(81.0f);
	
	// Create the room grid and figure out which rooms are in which cells.
	BuildRoomGrid(pProgress, pOutputPortalData);
	progress.SetProgress(100.0f);
	return true;	
}

// -----------------------------------------------------------------

/// Build the room and portal data from Aftk node list.
bool C_ProcessBuildPortalData::Build(Progress::I_ProgressBar* pProgress, const C_AftkNodeList& rkNodeList, C_DataPortalData* pOutputPortalData)
{
	LOGDEBUG(Tools::g_log,  "-- BUILD POLYGONAL DATA STRUCTURES --.\n");

	assert(pOutputPortalData);
	Btl::Vector<Aftk::C_SceneMeshNode*> vSeenNodes;
	
	// Create the "outside" world room -- link all "dead" portals into this room.
	m_pWorldRoom = pOutputPortalData->GetBuilder().CreateRoom("WorldRoom");

	// Loop through each room...
	for(U32 iCurrentRoom=0; iCurrentRoom<static_cast<U32>(rkNodeList.Size()); ++iCurrentRoom)
	{
		const Aftk::C_SceneNode* pCurrentRoom = rkNodeList[iCurrentRoom];
		assert(pCurrentRoom);
		
		const Aftk::E_ObjType nodeType = pCurrentRoom->GetType();			// Won't handle instances.
		if(nodeType != Aftk::E_ObjType_SceneMesh)
			continue;
		
		Aftk::C_SceneMeshNode* pAftkSceneMeshNode = (Aftk::C_SceneMeshNode*)pCurrentRoom;
		
		for(Btl::Vector<Aftk::C_SceneMeshNode*>::Iterator currentRoom = vSeenNodes.Begin(); currentRoom != vSeenNodes.End(); ++currentRoom)
			if(*currentRoom == pAftkSceneMeshNode)
				LOGERROR(Tools::g_log, "Duplicate aftk scene node found!");
				
		vSeenNodes.PushBack(pAftkSceneMeshNode);
		
		if(pAftkSceneMeshNode->GetNumLodLevels() <= 0)
			continue;

		U32 iLodLevel = 0;    
		const U32 iNumPolygons = pAftkSceneMeshNode->GetNumPolygons(iLodLevel);    
		if(iNumPolygons ==  0)    	    
			continue;
		
		Portal::C_BuilderRoom* pNewRoom = NULL;
		pNewRoom = pOutputPortalData->GetBuilder().CreateRoom(pCurrentRoom->GetName().c_str());
		assert(pNewRoom);

		// TODO: Maybe needs this at a later date?
		//pNewRoom->SetAftkNodePtr(pCurrentRoom);
		
		Maths::HMat44 nodeModelMatrix = AftkHelpers::GetHMat44(pCurrentRoom);
        
		LOGDEBUG(Tools::g_log,  "Processing room %d.\n", iCurrentRoom);
		LOGDEBUG(Tools::g_log,  "Room should contain %d polygons.", iNumPolygons); 

		// Loop through each poly in the room...
		for(U32 iCurrentPoly=0; iCurrentPoly<iNumPolygons; ++iCurrentPoly)
		{
			bool bIsAdjoiningPoly = false;
	        
            Aftk::C_Polygon& aftkPolygon = pAftkSceneMeshNode->GetPolygon(0, iCurrentPoly);
			Util::C_String textureName = AftkHelpers::GetTextureName(&aftkPolygon);
			if(textureName.ISearch("adjoiningroom") != -1)					    
                bIsAdjoiningPoly = true;                    
	
			// These base polygon indices are created during the triangulation process. The first 
			// triangle of a polygon is termed the 'base polygon'. The subsequent triangles for 
			// that polygon then have indices to that polygon (the first triangle). In this way 
			// it is possible to rebuild a polygon mesh after it has been triangulated. An index 
			// of -1 indicates that this is a base polygon.
			// From AFTK Docs.
			S32 iBasePolygonIndex = aftkPolygon.GetBasePolygonIndex();
			if(iBasePolygonIndex == -1)
			{
				LOGDEBUG(Tools::g_log,  "Setting index for -1 base index poly, index being set to %d.\n", iCurrentPoly);
				iBasePolygonIndex = iCurrentPoly;
			}
			else
				LOGDEBUG(Tools::g_log,  "Leaving base polygon index as found, found as %d.\n", iBasePolygonIndex);

			Portal::C_BuilderPolygon*	pNewPolygon = pNewRoom->CreatePolygon(iBasePolygonIndex, iCurrentPoly, pAftkSceneMeshNode);
			
			// Add it to the room irrespective of if it's a portal polygon or a room polygon.
			const U32 iVertexCount = aftkPolygon.GetNumPolygonVertices();
	        for(U32 iCurrentVertex=0; iCurrentVertex<iVertexCount; ++iCurrentVertex)
		    {
				Aftk::C_PolygonVertex& aftkPolygonVertex = aftkPolygon.GetPolygonVertex(iCurrentVertex);
				const S32 iAftkVertexPositionIndex = aftkPolygonVertex.GetPositionIndex();
				if(iAftkVertexPositionIndex >= 0)
				{
					Aftk::Position3 aftkPos = pAftkSceneMeshNode->GetVertexPosition(iAftkVertexPositionIndex);
					Maths::HVec3 newPos = Maths::HVec3(aftkPos[0], aftkPos[1], aftkPos[2]) * nodeModelMatrix;	// Transform into world space.
					Portal::C_BuilderVertex* pNewVertex = pNewPolygon->CreateVertex(newPos);
					
					// Add it to the portal if this vertex is part of an adjoining polygon.
					if(bIsAdjoiningPoly)
						pNewPolygon->MakeAdjoining();
				}
			}
            
			// Compute supporting data for the current polygon.
			pNewPolygon->Build();
		}

		// Added all the polygons, so we can compute the centroid for later.
		pNewRoom->ComputeExtents();
		
		LOGDEBUG(Tools::g_log,  "Room processing done! (%d).\n", iCurrentRoom);
		LOGDEBUG(Tools::g_log,  "Room \"%s\" actually contains %d polygons.", pNewRoom->GetName().Chars(), pNewRoom->GetPolygonCount()); 

	}

	LOGDEBUG(Tools::g_log,  "----------.\n");
	return true;
}

// -----------------------------------------------------------------

/// Clean up the portal and room geometry.
bool C_ProcessBuildPortalData::ReportCleanUpMeshes(Progress::I_ProgressBar* pProgress, const C_AftkNodeList& rkNodeList, C_DataPortalData* pOutputPortalData)
{
	LOGDEBUG(Tools::g_log,  "-- REPORT ERRORS IN ROOM MESHES --.\n");

	assert(pOutputPortalData);
	
	// Loop through all the rooms.
	Btl::List<Portal::C_BuilderRoom>::Iterator roomIterator = pOutputPortalData->GetBuilder().GetRoomList().Begin();
	for(; roomIterator!=pOutputPortalData->GetBuilder().GetRoomList().End(); ++roomIterator)
	{
		Portal::C_BuilderRoom* pCurrentRoom = &(*roomIterator);
		assert(pCurrentRoom);
		Btl::List<Portal::C_BuilderPolygon>::Iterator polyIterator = pCurrentRoom->GetPolygonList().Begin();

		// Loop through all the polygons in the current room.
		for(; polyIterator!=pCurrentRoom->GetPolygonList().End(); ++polyIterator)
		{
			Portal::C_BuilderPolygon* pTestPolygon = &(*polyIterator);	
			assert(pTestPolygon);
			
			// And check them against all other polys in the room.
			Btl::List<Portal::C_BuilderPolygon>::Iterator testPolyIterator = pCurrentRoom->GetPolygonList().Begin();
			for(; testPolyIterator!=pCurrentRoom->GetPolygonList().End(); ++testPolyIterator)
			{
				Portal::C_BuilderPolygon* pCurrentPolygon = &(*testPolyIterator);
				assert(pCurrentPolygon);

				bool bTest1 = pTestPolygon->IsCoincident(pCurrentPolygon);
				bool bTest2 = pCurrentPolygon->IsCoincident(pTestPolygon);
				if(bTest1 != bTest2)
					volatile int i = 0;
					
				bTest1 = pTestPolygon->IsCoincident(pCurrentPolygon);
				bTest2 = pCurrentPolygon->IsCoincident(pTestPolygon);

				if(pCurrentPolygon != pTestPolygon && bTest1 && bTest2)
 					LOGERROR(Tools::g_log, "Portal Error - Duplicate polygon in room %s, polygons from node %d, %d and node %d, %d.\n", pCurrentRoom->GetName().Chars(),
 																																		pCurrentPolygon->GetAftkSceneMeshNodePtr(),
 																																		pCurrentPolygon->GetAftkPolygonIndex(),
 																																		pTestPolygon->GetAftkSceneMeshNodePtr(),
 																																		pTestPolygon->GetAftkPolygonIndex());	// Log coincident polygons.
			}
		}
	}

	LOGDEBUG(Tools::g_log,  "----------.\n");
 	return true;
}

// -----------------------------------------------------------------

/// Flatten down any extraneous tesselatted geometry.
bool C_ProcessBuildPortalData::FlattenPortalPolygons(Progress::I_ProgressBar* pProgress, C_DataPortalData* pOutputPortalData)
{
	LOGDEBUG(Tools::g_log,  "-- FLATTEN PORTAL POLYGONS --.\n");
	assert(pOutputPortalData);

	while(FlattenPortalPolygonWorker(pOutputPortalData))
		;	// Null is intentional.
	
	LOGDEBUG(Tools::g_log,  "----------.\n");

	return true;
}

// -----------------------------------------------------------------

const bool C_ProcessBuildPortalData::FlattenPortalPolygonWorker(C_DataPortalData* pOutputPortalData)
{
	// Loop through each room in the portal system.
	Btl::List<Portal::C_BuilderRoom>::Iterator roomIterator = pOutputPortalData->GetBuilder().GetRoomList().Begin();
	for(; roomIterator!=pOutputPortalData->GetBuilder().GetRoomList().End(); ++roomIterator)
	{
		Portal::C_BuilderRoom* pCurrentRoom = NULL;
		pCurrentRoom = &(*roomIterator);
		assert(pCurrentRoom);								// Sanity check -- invalid room pointer returned.
		//LOGDEBUG(Tools::g_log,  "Processing %d polygons in room %s.\n", pCurrentRoom->GetPolygonCount(), pCurrentRoom->GetName().Chars());
		U32 iNumberOfPolygons = pCurrentRoom->GetPolygonCount();

		// Loop through each polygon in the room.
		Btl::List<Portal::C_BuilderPolygon>::Iterator polyIterator = pCurrentRoom->GetPolygonList().Begin();
		for(U32 iCurrentPolygon=0; polyIterator!=pCurrentRoom->GetPolygonList().End() && iCurrentPolygon<iNumberOfPolygons; ++iCurrentPolygon, ++polyIterator)
		{
			Portal::C_BuilderPolygon* pCurrentPolygon = NULL;
			pCurrentPolygon = &(*polyIterator);
			assert(pCurrentPolygon);						// Sanity check -- invalid polygon pointer returned.
			//LOGDEBUG(Tools::g_log,  "Processing polygon %d.\n", iCurrentPolygon);

			// Only bother to collapse portal polygons.
			if(pCurrentPolygon->IsAdjoining())
			{
				//LOGDEBUG(Tools::g_log,  "Polygon %d is a portal!\n", iCurrentPolygon);

				// Check each edge in the polygon.
				Btl::List<Portal::C_BuilderEdge*>::Iterator edgeIterator = pCurrentPolygon->GetEdgeList().Begin();
				for(U32 iCurrentPolygonEdge=0; edgeIterator!=pCurrentPolygon->GetEdgeList().End() && pCurrentPolygon->IsAdjoining(); ++iCurrentPolygonEdge, ++edgeIterator)
				{
					Portal::C_BuilderEdge* pCurrentEdge = NULL;
					pCurrentEdge = (*edgeIterator);
					assert(pCurrentEdge);					// Sanity check -- invalid edge pointer returned.

					// Loop through all the polygons that are used by the current edge.
					Btl::List<Portal::C_BuilderPolygon*>::Iterator edgePolygonIterator = pCurrentEdge->GetPolygonList().Begin();
					for(U32 iCurrentEdgePolygon=0; edgePolygonIterator!=pCurrentEdge->GetPolygonList().End(); ++iCurrentEdgePolygon, ++edgePolygonIterator)
					{
						Portal::C_BuilderPolygon* pCurrentEdgePolygon = NULL;
						pCurrentEdgePolygon = (*edgePolygonIterator);
						assert(pCurrentEdgePolygon);		// Sanity check -- invalid polygon pointer returned.
						//LOGDEBUG(Tools::g_log,  "Checking if polygon %d's egde-mate polygon %d is candidate for collapse!\n", iCurrentPolygon, iCurrentEdgePolygon);

						// Skip anything that we don't wish to consider, this means checking the polygon
						// against it's self, checking non-edge polygons, portal polygons that haven't
						// come from the same polygon base index (i.e.: same original primitive).
						if(pCurrentPolygon == pCurrentEdgePolygon)
						{
							//LOGDEBUG(Tools::g_log,  "Skipping polygon %d in room %s due to testing with self.\n", iCurrentEdgePolygon, pCurrentRoom->GetName().Chars());
							continue;
						}
						if(!pCurrentEdgePolygon->IsAdjoining())
						{
							//LOGDEBUG(Tools::g_log,  "Skipping polygon %d in room %s due to not being a portal polygon.\n", iCurrentEdgePolygon, pCurrentRoom->GetName().Chars());
							continue;
						}
						if(pCurrentPolygon->GetBaseIndex() != pCurrentEdgePolygon->GetBaseIndex())
						{
							//LOGDEBUG(Tools::g_log,  "Skipping polygon %d in room %s due to being of a different base index [%d vs. %d].\n", iCurrentEdgePolygon, pCurrentRoom->GetName().Chars(), pCurrentPolygon->GetBaseIndex(), pCurrentEdgePolygon->GetBaseIndex());
							continue;
						}
						if(pCurrentPolygon->GetParentRoom() != pCurrentEdgePolygon->GetParentRoom())
						{
							//LOGDEBUG(Tools::g_log,  "Skipping polygon %d in room %s due to being in a different room.", iCurrentEdgePolygon, pCurrentRoom->GetName().Chars());
							continue;
						}

						LOGDEBUG(Tools::g_log,  "Found 2 polygons to merge in room %s, [%d merged with %d].\n", pCurrentRoom->GetName().Chars(), iCurrentPolygon, iCurrentEdgePolygon);

						// Collapse the bloody polygon!
						Portal::C_BuilderPolygon* pNewPolygon = NULL;
						pNewPolygon = pCurrentPolygon->Merge(pCurrentPolygon, pCurrentEdgePolygon, pCurrentEdge);
						assert(pNewPolygon);

						pNewPolygon->MakeAdjoining();
						pCurrentPolygon->RemoveAdjoiningFlag();
						pCurrentEdgePolygon->RemoveAdjoiningFlag();
// 						pCurrentRoom->RemovePolygon(pCurrentPolygon);
// 						pCurrentRoom->RemovePolygon(pCurrentEdgePolygon);
						pNewPolygon->Build();
						return true;
						
						// TODO: Fix polygon to make it co-planar and calculate plane.
						//pNewPolygon->SnapVerticesToAveragePlane();
					}
				}
			}
		}
	}

	return false;
}

// -----------------------------------------------------------------

/// Merge portals that connect to one another.
bool C_ProcessBuildPortalData::SetupPortals(Progress::I_ProgressBar* pProgress, C_DataPortalData* pOutputPortalData)
{
	assert(pOutputPortalData);
	LOGDEBUG(Tools::g_log,  "-- MATCH PORTALS WITH COUNTERPARTS --.\n");

	// Loop through all the rooms.
	Btl::List<Portal::C_BuilderRoom>::Iterator roomIterator = pOutputPortalData->GetBuilder().GetRoomList().Begin();
	for(; roomIterator!=pOutputPortalData->GetBuilder().GetRoomList().End(); ++roomIterator)
	{
		Portal::C_BuilderRoom* pCurrentRoom = NULL;
		pCurrentRoom = &(*roomIterator);
		assert(pCurrentRoom);								// Sanity check -- invalid room pointer returned.
		LOGDEBUG(Tools::g_log,  "Processing portal polygons in room %s.\n", pCurrentRoom->GetName().Chars());

		// Loop through each polygon in the room.
		Btl::List<Portal::C_BuilderPolygon>::Iterator polyIterator = pCurrentRoom->GetPolygonList().Begin();
		for(U32 iCurrentPolygon=0; polyIterator!=pCurrentRoom->GetPolygonList().End(); ++iCurrentPolygon, ++polyIterator)
		{
			Portal::C_BuilderPolygon* pCurrentPolygon = NULL;
			pCurrentPolygon = &(*polyIterator);
			assert(pCurrentPolygon);						// Sanity check -- invalid polygon pointer returned.
			
			// Create portals for adjoining polygons (only if one not already done!).
			if(pCurrentPolygon->IsAdjoining() && !pCurrentPolygon->GetPortal())
			{
				// Counterpart should share first edge of the polygon.
				Btl::List<Portal::C_BuilderEdge*>::Iterator edgeIterator = pCurrentPolygon->GetEdgeList().Begin();
				Portal::C_BuilderEdge* pFirstEdge = NULL;
				pFirstEdge = (*edgeIterator);
				assert(pFirstEdge);							// Sanity check -- invalid polygon pointer returned.
				
				bool bFoundMatch = false;
				Btl::List<Portal::C_BuilderPolygon*>::Iterator testPolygonIterator = pFirstEdge->GetPolygonList().Begin();
				if(pFirstEdge->GetPolygonCount() <= 1)
					LOGERROR(Tools::g_log, "Problem! Edge with no shared polygons -- shouldn't happen.");

				for(; testPolygonIterator!=pFirstEdge->GetPolygonList().End(); ++testPolygonIterator)
				{
					Portal::C_BuilderPolygon* pCurrentTestPolygon = NULL;
					pCurrentTestPolygon = (*testPolygonIterator);
					assert(pCurrentTestPolygon);			// Sanity check -- invalid polygon pointer returned.

					if(pCurrentTestPolygon == pCurrentPolygon)
					{
						LOGDEBUG(Tools::g_log, "Polygon is testing against itself.");
						continue;	// Don't test with self.
					}
					if(!pCurrentTestPolygon->IsAdjoining())
					{
						LOGDEBUG(Tools::g_log, "Polygon is not adjoining.");
						continue;	// Only consider portal polygons.
					}
					if(pCurrentPolygon->GetParentRoom() == pCurrentTestPolygon->GetParentRoom())
					{
						LOGDEBUG(Tools::g_log, "Portals have same parent room.");
						continue;	// Portals in the same room.
					}
					
					if(pCurrentTestPolygon->GetPortal())
					{
						LOGDEBUG(Tools::g_log, "Portal already created for this polygon.");
						continue;	// Already setup this portal so skip it.
					}

					if(pCurrentPolygon->IsCoincident(pCurrentTestPolygon) || pCurrentTestPolygon->IsCoincident(pCurrentPolygon))
					{
						Portal::C_BuilderPortal* pNewPortal = NULL;
						Portal::C_BuilderPortal* pCounterpart = NULL;
						pNewPortal = pOutputPortalData->GetBuilder().CreatePortal(pCurrentRoom);
						pCounterpart = pOutputPortalData->GetBuilder().CreatePortal(pCurrentTestPolygon->GetParentRoom());
						assert(pNewPortal);					// Sanity check -- invalid portal polygon.
						assert(pCounterpart);				// Sanity check -- invalid portal polygon.
						pCurrentRoom->AddPortal(pNewPortal);
						pCurrentTestPolygon->GetParentRoom()->AddPortal(pCounterpart);
						pCurrentPolygon->SetPortal(pNewPortal);
						pCurrentTestPolygon->SetPortal(pCounterpart);

						// Add all the vertices from the current polygon to the portal.
						for(Btl::List<Portal::C_BuilderVertex*>::Iterator currentVertex=pCurrentPolygon->GetVertexList().Begin(); currentVertex!=pCurrentPolygon->GetVertexList().End();++currentVertex)
						{
							Maths::HVec3* pVert = pNewPortal->CreateNewPolygonVertex();
							assert(pVert);
							*pVert = (*currentVertex)->GetPosition();
						}
						pNewPortal->SetPlane(pCurrentPolygon->GetPlane());

						// Add all the vertices from the current polygon to the portal.
						for(Btl::List<Portal::C_BuilderVertex*>::Iterator currentVertex=pCurrentTestPolygon->GetVertexList().Begin(); currentVertex!=pCurrentTestPolygon->GetVertexList().End();++currentVertex)
						{
							Maths::HVec3* pVert = pCounterpart->CreateNewPolygonVertex();
							assert(pVert);
							*pVert = (*currentVertex)->GetPosition();
						}
						pCounterpart->SetPlane(pCurrentTestPolygon->GetPlane());
						
						// Link the two portals.
						pNewPortal->SetCounterpart(pCounterpart);
						pCounterpart->SetCounterpart(pNewPortal);

						LOGDEBUG(Tools::g_log,  "New portals created that link room %s to room %s.\n", pCurrentRoom->GetName().Chars(), pCurrentTestPolygon->GetParentRoom()->GetName().Chars());
						bFoundMatch = true;
					}
				}

				// Didn't find the portal's counterpart portal in another room, we can
				// safely(?) assume that it links to the outside, so special case it and
				// link into our "world room".
				if(!bFoundMatch)
				{
					LOGDEBUG(Tools::g_log,  "Making portal that links room %s to the WorldRoom.\n", pCurrentRoom->GetName().Chars());
					Portal::C_BuilderPortal* pNewPortal = NULL;
					Portal::C_BuilderPortal* pCounterpart = NULL;
					pNewPortal = pOutputPortalData->GetBuilder().CreatePortal(pCurrentRoom);
					pCounterpart = pOutputPortalData->GetBuilder().CreatePortal(m_pWorldRoom);
					assert(pNewPortal);						// Sanity check -- invalid portal polygon.
					assert(pCounterpart);					// Sanity check -- invalid portal polygon.
					pCurrentRoom->AddPortal(pNewPortal);
					m_pWorldRoom->AddPortal(pCounterpart);
					pCurrentPolygon->SetPortal(pNewPortal);

					// Set-up counterpart relationship.
					pNewPortal->SetCounterpart(pCounterpart);
					pCounterpart->SetCounterpart(pNewPortal);
					
					// Add all the vertices from the current polygon to the portal.
					for(Btl::List<Portal::C_BuilderVertex*>::Iterator currentVertex=pCurrentPolygon->GetVertexList().Begin(); currentVertex!=pCurrentPolygon->GetVertexList().End();++currentVertex)
					{
						Maths::HVec3* pVert = pNewPortal->CreateNewPolygonVertex();
						assert(pVert);
						*pVert = (*currentVertex)->GetPosition();
					}
					pNewPortal->SetPlane(pCurrentPolygon->GetPlane());
					
					// Reverse winding order for world room polygon to flip normal.
					for(Btl::List<Portal::C_BuilderVertex*>::ReverseIterator currentVertex=pCurrentPolygon->GetVertexList().ReverseBegin(); currentVertex!=pCurrentPolygon->GetVertexList().ReverseEnd(); ++currentVertex)
					{
						Maths::HVec3* pVert = pCounterpart->CreateNewPolygonVertex();
						assert(pVert);
						*pVert = (*currentVertex)->GetPosition();
					}
					Maths::C_Plane inversePlane = -(pCurrentPolygon->GetPlane());
					pCounterpart->SetPlane(inversePlane);
				}
			}
		}
	}

	LOGDEBUG(Tools::g_log,  "----------.\n");
	return true;
}

// -----------------------------------------------------------------

/// Compute containment data for rooms.
bool C_ProcessBuildPortalData::BuildRoomContainmentData(Progress::I_ProgressBar* pProgress, C_DataPortalData* pOutputPortalData)
{
	assert(pOutputPortalData);

	// Loop through all the rooms.
	Btl::List<Portal::C_BuilderRoom>::Iterator roomIterator = pOutputPortalData->GetBuilder().GetRoomList().Begin();
	for(; roomIterator!=pOutputPortalData->GetBuilder().GetRoomList().End(); ++roomIterator)
	{
		Portal::C_BuilderRoom* pCurrentRoom = NULL;
		pCurrentRoom = &(*roomIterator);
		assert(pCurrentRoom);								// Sanity check -- invalid room pointer returned.
		LOGDEBUG(Tools::g_log,  "Processing portal polygons in room %s.\n", pCurrentRoom->GetName().Chars());
		SimpleBSP::C_Tree roomBSPTree;
		
		// Loop through each polygon in the room -- these are fed in backwards as the SimplerBSP helper
		// expects normals to face outward from the box, but portal rooms have their normals facing toward
		// the box's centroid.
		//  -- STOVE, 181208
		Btl::List<Portal::C_BuilderPolygon>::Iterator polyIterator = pCurrentRoom->GetPolygonList().Begin();
		for(U32 iCurrentPolygon=0; polyIterator!=pCurrentRoom->GetPolygonList().End(); ++iCurrentPolygon, ++polyIterator)
		{
			Portal::C_BuilderPolygon* pCurrentPolygon = NULL;
			pCurrentPolygon = &(*polyIterator);
			assert(pCurrentPolygon);						// Sanity check -- invalid polygon pointer returned.

			// Create BSP polygon and add it into the current room's BSP tree.
			SimpleBSP::C_Polygon bspPolygon;
			Btl::List<Portal::C_BuilderVertex*>::ReverseIterator vertexIterator = pCurrentPolygon->GetVertexList().ReverseBegin();
			for(; vertexIterator!=pCurrentPolygon->GetVertexList().ReverseEnd(); ++vertexIterator)
			{
				Portal::C_BuilderVertex* pCurrentVertex = NULL;
				pCurrentVertex = (*vertexIterator);
				assert(pCurrentVertex);						// Sanity check -- invalid vertex pointer returned.

				bspPolygon.NewVertex(pCurrentVertex->GetPosition());
			}
			bspPolygon.CalculatePlane();
			roomBSPTree.Insert(bspPolygon);
		}

		// Compute containment information from BSP tree.
		SimpleBSP::C_Containment roomContainment(roomBSPTree);
		LOGDEBUG(Tools::g_log,  "Room '%s' has %d convex sub-spaces.\n", pCurrentRoom->GetName().Chars(), roomContainment.m_CellList.Elements());

		// Loop through all the cells, add the planes to the room containment class,
		// add the indices to sub-spaces.
		SimpleBSP::C_Cell* pCurrentCell = roomContainment.m_CellList.First();
        while(pCurrentCell)
        {
			Portal::C_BuilderRoomContainmentSpace* pContainment = pCurrentRoom->GetSpace();
			
			// Loop through all planes in the current BSP containment cell.
			SimpleBSP::C_CellPlane* pCurrentCellPlane = pCurrentCell->m_PlaneList.First();
            U32 iFirstPlane = pContainment->GetPlaneCount();
			while(pCurrentCellPlane)
            {
				pContainment->CreatePlane(pCurrentCellPlane->m_Plane);
                pCurrentCellPlane = pCurrentCellPlane->Next();
            }
            
			// Add sub-space indices.
			assert(iFirstPlane < pContainment->GetPlaneCount());	// Something went wrong here, looks like no planes in the cell.
			if(iFirstPlane >= pContainment->GetPlaneCount())
				LOGERROR(Tools::g_log, "Encountered subspace defined by 0 planes!");
			
			pContainment->CreateSubSpace(iFirstPlane, pContainment->GetPlaneCount()-1);
            pCurrentCell = pCurrentCell->Next();        
        }
	}

	return true;
}
	
// -----------------------------------------------------------------

/// Compute the room grid.
bool C_ProcessBuildPortalData::BuildRoomGrid(Progress::I_ProgressBar* pProgress, C_DataPortalData* pOutputPortalData)
{
	assert(pOutputPortalData);

	// Create the grid.
	pOutputPortalData->GetBuilder().SetupRoomGrid(16.0f);

	// Loop through all the rooms and add them all to the grid.
	Btl::List<Portal::C_BuilderRoom>::Iterator roomIterator = pOutputPortalData->GetBuilder().GetRoomList().Begin();
	for(; roomIterator!=pOutputPortalData->GetBuilder().GetRoomList().End(); ++roomIterator)
	{
		Portal::C_BuilderRoom* pCurrentRoom = NULL;
		pCurrentRoom = &(*roomIterator);
		assert(pCurrentRoom);								// Sanity check -- invalid room pointer returned.
		
		pOutputPortalData->GetBuilder().GetRoomGrid()->AddRoom(pCurrentRoom);
	}

	// Build out room BSP as well -- used to support render data queries.
	pOutputPortalData->GetBuilder().BuildRoomBSP();	
	return true;
}
