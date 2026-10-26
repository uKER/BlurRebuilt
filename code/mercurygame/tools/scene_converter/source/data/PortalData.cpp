#include <stdafx.h>
#include <Data/PortalData.h>
#include <Btl\Containers\algorithm.h>
#include <Misc_shared\Geometry\RegionBSP.h>
#include <Misc_shared\Geometry\RegionBSP.inl>


/// Default c'tor.
C_DataPortalData::C_DataPortalData()
{
	return;
}

// -----------------------------------------------------------------

const Util::C_String C_DataPortalData::GetName() const
{
	return Util::C_String("C_DataPortalData");
}

// -----------------------------------------------------------------

/// Export data to a file.
bool C_DataPortalData::Export(Sys::I_File* pFile, const Util::E_Platform& rkPlatform) const
{
	// No portal data to serialise.
	if(m_PortalBuilder.GetRoomCount() == 0)
		return true;

	bool bSuccess = true;
	Sys::C_FileBlock rootBlock(pFile, "Portals");

	// Header Block.
	{
		Sys::C_FileBlock headerBlock(&rootBlock, "Header");
		Sys::BinarySerialiserAutoEndian serialiser(headerBlock);
		serialiser << static_cast<U32>(Portal::E_PortalFileVersion_Current);
		serialiser << m_PortalBuilder.GetRoomCount();
		serialiser << m_PortalBuilder.GetPortalCount();
		
		U32 iNumberOfPlanes = 0;
		U32 iNumberOfSubSpaces = 0;
		Btl::List<Portal::C_BuilderRoom>::ConstIterator roomIterator = m_PortalBuilder.GetRoomList().Begin();
		for(; roomIterator!=m_PortalBuilder.GetRoomList().End(); ++roomIterator)
		{
			const Portal::C_BuilderRoom* pCurrentRoom = NULL;
			pCurrentRoom = &(*roomIterator);
			BIZ_WARNING_MSG(pCurrentRoom, "Sanity check failed -- invalid pointer.");			
			iNumberOfSubSpaces		+= pCurrentRoom->GetSubSpaceCount();
			iNumberOfPlanes			+= pCurrentRoom->GetSpace()->GetPlaneCount();
		}
		serialiser << iNumberOfPlanes;
		serialiser << iNumberOfSubSpaces;
		LOGINFO(Tools::g_log, "Number of room[%d], portals[%d], spaces[%d], planes[%d] and sub-spaces[%d].\n", m_PortalBuilder.GetRoomCount(), 
																											   m_PortalBuilder.GetPortalCount(), 
																											   m_PortalBuilder.GetSpaceCount(),
																											   iNumberOfPlanes,
																											   iNumberOfSubSpaces);
	}

	// Builder the string table.
	Util::C_StringTable stringTable;
	
	// Room data block.
	{
		Sys::C_FileBlock roomsBlock(&rootBlock, "Rooms");
		Sys::BinarySerialiserAutoEndian serialiser(roomsBlock);
			
		// Loop through each room.
		Btl::List<Portal::C_BuilderRoom>::ConstIterator roomIterator = m_PortalBuilder.GetRoomList().Begin();
		for(U32 iCurrentRoom=0; roomIterator!=m_PortalBuilder.GetRoomList().End(); ++iCurrentRoom, ++roomIterator)
		{
			const Portal::C_BuilderRoom* pCurrentRoom = NULL;
			pCurrentRoom = &(*roomIterator);
			BIZ_WARNING_MSG(pCurrentRoom, "Sanity check failed -- invalid pointer.");						// Sanity check -- invalid room pointer returned.

			// Write out the room to stream.
			serialiser << pCurrentRoom->GetPortalCount();
			Btl::List<Portal::C_BuilderPortal*>::ConstIterator portalIterator = pCurrentRoom->GetPortalList().Begin();
			for(; portalIterator != pCurrentRoom->GetPortalList().End(); ++portalIterator)
				serialiser << m_PortalBuilder.GetIndexForPortal((*portalIterator));
			serialiser << m_PortalBuilder.GetStringBuilder().AddString(pCurrentRoom->GetName());
			serialiser << m_PortalBuilder.GetIndexForSpace(pCurrentRoom->GetSpace());
		}
	}

	// Portal data block.
	{
		Sys::C_FileBlock portalsBlock(&rootBlock, "Portals");
		Sys::BinarySerialiserAutoEndian serialiser(portalsBlock);
		
		// Loop through each portal.
		Btl::List<Portal::C_BuilderPortal>::ConstIterator portalIterator = m_PortalBuilder.GetPortalList().Begin();
		for(U32 iCurrentPortal=0; portalIterator!=m_PortalBuilder.GetPortalList().End(); ++iCurrentPortal, ++portalIterator)
		{
			const Portal::C_BuilderPortal* pCurrentPortal = NULL;
			pCurrentPortal = &(*portalIterator);
			BIZ_WARNING_MSG(pCurrentPortal, "Sanity check failed -- invalid pointer.");						// Sanity check -- invalid portal pointer returned.
	
			// Write out the portal to stream.
			serialiser << pCurrentPortal->GetPlane();
			serialiser << m_PortalBuilder.GetIndexForRoom(pCurrentPortal->GetParent());
			serialiser << m_PortalBuilder.GetIndexForPortal(pCurrentPortal->GetCounterpart());
			serialiser << pCurrentPortal->GetVertexCount();
			for(U32 iCurrentVertex=0; iCurrentVertex<pCurrentPortal->GetVertexCount(); ++iCurrentVertex)
				serialiser << pCurrentPortal->GetVertexAtIndex(iCurrentVertex);
		}
	}

	Btl::Map<const Portal::C_BuilderRoomContainmentSubSpace*, U32> subSpaceMap;	// Map sub-space pointers to their indices.

	// Space definitions.
	{
		Sys::C_FileBlock spaceBlock(&rootBlock, "Spaces");
		Sys::BinarySerialiserAutoEndian serialiser(spaceBlock);
		
		Btl::Map<const Maths::C_Plane*, U32>	planeIndexMap;

		// Write out all the planes.
		Btl::List<Portal::C_BuilderRoom>::ConstIterator roomIterator = m_PortalBuilder.GetRoomList().Begin();
		U32 iPlaneIndex=0;
		for(; roomIterator!=m_PortalBuilder.GetRoomList().End(); ++roomIterator)
		{
			const Portal::C_BuilderRoom* pCurrentRoom = NULL;
			pCurrentRoom = &(*roomIterator);
			BIZ_WARNING_MSG(pCurrentRoom, "Sanity check failed -- invalid pointer.");			
			Btl::List<Maths::C_Plane>::ConstIterator planeIterator = pCurrentRoom->GetSpace()->GetPlaneList().Begin();
			for(; planeIterator != pCurrentRoom->GetSpace()->GetPlaneList().End(); ++planeIterator, ++iPlaneIndex)
			{
				const Maths::C_Plane* pCurrentPlane = &(*planeIterator);
				planeIndexMap.Insert(pCurrentPlane, iPlaneIndex);
				serialiser << (*pCurrentPlane);		// Write all the planes for this sub-space.
			}
		}

		// Write out all the sub-spaces.
		Btl::List<Portal::C_BuilderRoomContainmentSpace>::ConstIterator spaceIterator = m_PortalBuilder.GetSpaceList().Begin();
		U32 iSubSpaceIndex = 0;
		for(U32 iCurrentSpace=0; spaceIterator!=m_PortalBuilder.GetSpaceList().End(); ++iCurrentSpace, ++spaceIterator)
		{
			const Portal::C_BuilderRoomContainmentSpace* pCurrentSpace = NULL;
			pCurrentSpace = &(*spaceIterator);
			BIZ_WARNING_MSG(pCurrentSpace, "Sanity check failed -- invalid pointer.");						// Sanity check -- invalid space pointer returned.

			// Fire out all the sub-spaces.
			//serialiser << pCurrentSpace->GetSubSpaceCount();
			Btl::List<Portal::C_BuilderRoomContainmentSubSpace>::ConstIterator subSpaceIterator = pCurrentSpace->GetSubSpaceList().Begin();
			for(; subSpaceIterator!=pCurrentSpace->GetSubSpaceList().End(); ++subSpaceIterator, ++iSubSpaceIndex)
			{
				const Portal::C_BuilderRoomContainmentSubSpace* pCurrentSubSpace = &(*subSpaceIterator);
				subSpaceMap.Insert(pCurrentSubSpace, iSubSpaceIndex);

				// Write out the plane and room indices for the sub-space.
				const Btl::List<Maths::C_Plane>& planeList = pCurrentSpace->GetPlaneList();
				Btl::List<Maths::C_Plane>::ConstIterator firstPlane = planeList.Begin();
				Btl::List<Maths::C_Plane>::ConstIterator secondPlane = planeList.Begin();
				Btl::Advance(firstPlane, pCurrentSubSpace->GetStartIndex());
				Btl::Advance(secondPlane, pCurrentSubSpace->GetEndIndex());
				BIZ_WARNING(planeIndexMap.Find(&(*firstPlane)) != planeIndexMap.End());
				BIZ_WARNING(planeIndexMap.Find(&(*secondPlane)) != planeIndexMap.End());
				serialiser << planeIndexMap[&(*firstPlane)];
				serialiser << planeIndexMap[&(*secondPlane)];
				BIZ_WARNING(m_PortalBuilder.GetIndexForRoom(pCurrentSpace->GetParentRoom()) != 0xd153a5ed);
				serialiser << m_PortalBuilder.GetIndexForRoom(pCurrentSpace->GetParentRoom());
			}
		}

		// Write out all the spaces.
		spaceIterator = m_PortalBuilder.GetSpaceList().Begin();
		for(U32 iCurrentSpace=0; spaceIterator!=m_PortalBuilder.GetSpaceList().End(); ++iCurrentSpace, ++spaceIterator)
		{
			const Portal::C_BuilderRoomContainmentSpace* pCurrentSpace = NULL;
			pCurrentSpace = &(*spaceIterator);
			BIZ_WARNING_MSG(pCurrentSpace, "Sanity check failed -- Invalid pointer.");						// Sanity check -- invalid space pointer returned.
	
			if(pCurrentSpace->GetSubSpaceCount())
			{
				serialiser << subSpaceMap[&(*pCurrentSpace->GetSubSpaceList().Begin())];
				serialiser << subSpaceMap[&pCurrentSpace->GetSubSpaceList().Back()];
			}
			else
			{
				// No sub-spaces in the space -- still write indices.
				serialiser << -1;
				serialiser << -1;
			}
		}
	}

	// Grid data block.
	{
		Sys::C_FileBlock gridBlock(&rootBlock, "Grid");
		Sys::BinarySerialiserAutoEndian serialiser(gridBlock);
		
		// Calculate size of index list -- don't think there's a nicer way :-(.
		U32 iNumberOfIndices = 0;
		Btl::Vector<Portal::C_BuilderRoomGridCell>::ConstIterator cellIterator;
		for(cellIterator = m_PortalBuilder.GetRoomGrid()->GetCellList().Begin(); cellIterator!=m_PortalBuilder.GetRoomGrid()->GetCellList().End(); ++cellIterator)
			for(Btl::List<Portal::C_BuilderRoom*>::ConstIterator roomIterator = (*cellIterator).GetRoomList().Begin(); roomIterator!=(*cellIterator).GetRoomList().End(); ++roomIterator)
				for(Btl::List<Portal::C_BuilderRoomContainmentSubSpace>::ConstIterator subSpaceIterator = (*roomIterator)->GetSpace()->GetSubSpaceList().Begin(); subSpaceIterator != (*roomIterator)->GetSpace()->GetSubSpaceList().End(); ++subSpaceIterator, ++iNumberOfIndices)
					;

		serialiser << iNumberOfIndices;

		// Write out cell/sub-space mapping indices -- build cell/spaces map.
		Btl::Map<U32, Btl::Pair<U32, U32> > gridCellRoomMap;
		U32 iIndexListSize = 0;
		cellIterator = m_PortalBuilder.GetRoomGrid()->GetCellList().Begin();
		for(U32 iCurrentCell=0; cellIterator!=m_PortalBuilder.GetRoomGrid()->GetCellList().End(); ++iCurrentCell, ++cellIterator)
		{
			const Portal::C_BuilderRoomGridCell* pCurrentCell = NULL;
			pCurrentCell = &(*cellIterator);
			BIZ_WARNING_MSG(pCurrentCell, "Sanity check failed -- Invalid pointer.");						// Sanity check -- invalid cell pointer returned.
			U32 iStartIndex = iIndexListSize;

			// Loop through each room.
			Btl::List<Portal::C_BuilderRoom*>::ConstIterator roomIterator = pCurrentCell->GetRoomList().Begin();
			for(U32 iCurrentRoom=0; roomIterator!=pCurrentCell->GetRoomList().End(); ++iCurrentRoom, ++roomIterator)
			{
				const Portal::C_BuilderRoom* pCurrentRoom = NULL;
				pCurrentRoom = (*roomIterator);
				BIZ_WARNING_MSG(pCurrentRoom, "Sanity check failed -- Invalid pointer.");					// Sanity check -- invalid room pointer returned.

				Btl::List<Portal::C_BuilderRoomContainmentSubSpace>::ConstIterator subSpaceIterator = pCurrentRoom->GetSpace()->GetSubSpaceList().Begin();
				for(; subSpaceIterator != pCurrentRoom->GetSpace()->GetSubSpaceList().End(); ++subSpaceIterator, ++iIndexListSize)
				{
					BIZ_WARNING_MSG(subSpaceMap.Find(&(*subSpaceIterator)) != subSpaceMap.End(), "Sub-Space pointer not mapped to index -- this would be bad.");	// Sub-Space pointer not mapped to index -- this would be bad.
					serialiser << subSpaceMap[&(*subSpaceIterator)];
				}
			}

			U32 iEndIndex = (iIndexListSize>iStartIndex)?(iIndexListSize-1):iIndexListSize;
			gridCellRoomMap.Insert(iCurrentCell, Btl::MakePair(iStartIndex, iEndIndex));
		}
		BIZ_WARNING_MSG(iIndexListSize == iNumberOfIndices, "Something went crazy wrong y'all!");											// Something went wrong :-(

		// Write out the settings for grid.
		serialiser << m_PortalBuilder.GetRoomGrid()->GetExtents();
		serialiser << m_PortalBuilder.GetRoomGrid()->GetCellSize();
		serialiser << m_PortalBuilder.GetRoomGrid()->GetCellsX();
		serialiser << m_PortalBuilder.GetRoomGrid()->GetCellsZ();
		serialiser << m_PortalBuilder.GetRoomGrid()->GetCellCount();
		
		// Loop through each grid cell.
		cellIterator = m_PortalBuilder.GetRoomGrid()->GetCellList().Begin();
		for(U32 iCurrentCell=0; cellIterator!=m_PortalBuilder.GetRoomGrid()->GetCellList().End(); ++iCurrentCell, ++cellIterator)
		{
			const Portal::C_BuilderRoomGridCell* pCurrentCell = NULL;
			pCurrentCell = &(*cellIterator);
			BIZ_WARNING_MSG(pCurrentCell, "Sanity check failed -- Invalid cell pointer returned.");						// Sanity check -- invalid cell pointer returned.
		
			BIZ_WARNING_MSG(gridCellRoomMap.Find(iCurrentCell) != gridCellRoomMap.End(), "Cell index not mapped to a start/end index pair." );
			Btl::Pair<U32, U32> value = gridCellRoomMap[iCurrentCell];
			serialiser << value.first;
			serialiser << value.second;
		}
	}

	{
		Sys::C_FileBlock bspBlock(&rootBlock, "BSP");
		Sys::BinarySerialiserAutoEndian serialiser(bspBlock);
		serialiser << m_PortalBuilder.GetRoomBSPTree();

// 		C_DataPortalData::NodeCounter		counter;
// 		m_PortalBuilder.GetRoomBSPTree().Traverse(counter);
// 		C_DataPortalData::NodeEnumerator	enumerator(counter.GetCount(), m_PortalBuilder);
// 		m_PortalBuilder.GetRoomBSPTree().Traverse(enumerator);
// 		
// 		// Serialise all the nodes.
// 		serialiser << enumerator.GetMappedIndex(m_PortalBuilder.GetRoomBSPTree().GetSplitPlaneIndex(m_PortalBuilder.GetRoomBSPTree().GetRootRegion()->GetSplitPlane()));
// 		serialiser << (U32)0xd15ea5ed;	// TODO: [Stove] Remove test value before committing.
// 		serialiser << counter.GetCount();
// 		for(U32 iCurrentNode=0; iCurrentNode < counter.GetCount(); ++iCurrentNode)
// 		{
// 			C_DataPortalData::Node* pNode = enumerator.GetNodePtrAt(iCurrentNode);
// 			serialiser << enumerator.GetMappedIndex(pNode->m_iBackIndex);
// 			serialiser << enumerator.GetMappedIndex(pNode->m_iFrontIndex);
// 			serialiser << pNode->m_iPlaneIndex;
// 			serialiser << pNode->m_iRoomIndex;
// 		}
// 
// 		// Serialise all the planes.
// 		serialiser << m_PortalBuilder.GetRoomBSPTree().GetSplitPlaneCount();
// 		for(U32 iCurrentPlane=0; iCurrentPlane<m_PortalBuilder.GetRoomBSPTree().GetSplitPlaneCount(); ++iCurrentPlane)
// 			serialiser << m_PortalBuilder.GetRoomBSPTree().GetPlanes()[iCurrentPlane].GetPlane();
	}		

	// Debug block -- doesn't need to be loaded unless debugging.
	{
		Sys::C_FileBlock debugBlock(&rootBlock, "Debug");
		Sys::BinarySerialiserAutoEndian serialiser(debugBlock);

		stringTable.Build(m_PortalBuilder.GetStringBuilder());
		stringTable.Serialise(serialiser);

		// Loop through each room.
		Btl::List<Portal::C_BuilderRoom>::ConstIterator roomIterator = m_PortalBuilder.GetRoomList().Begin();
		for(U32 iCurrentRoom=0; roomIterator!=m_PortalBuilder.GetRoomList().End(); ++iCurrentRoom, ++roomIterator)
		{
			const Portal::C_BuilderRoom* pCurrentRoom = NULL;
			pCurrentRoom = &(*roomIterator);
			BIZ_WARNING_MSG(pCurrentRoom, "Sanity check failed -- Invalid pointer.");						// Sanity check -- invalid room pointer returned.


			// Loop through each polygon in the room count non-adjoining ones.
			U32 iPolygonCount = 0;
			Btl::List<Portal::C_BuilderPolygon>::ConstIterator polyIterator = pCurrentRoom->GetPolygonList().Begin();
			for(; polyIterator != pCurrentRoom->GetPolygonList().End(); ++polyIterator)
			{
				const Portal::C_BuilderPolygon* pCurrentPoly = NULL;
				pCurrentPoly = &(*polyIterator);
				BIZ_WARNING_MSG(pCurrentPoly, "Sanity check failed -- Invalid pointer.");					// Sanity check -- invalid polygon pointer.

				// Skip portal polygons.
				if(!pCurrentPoly->IsAdjoining())
					iPolygonCount++;
			}

			serialiser << iPolygonCount;
			
			// Loop through each polygon in the room -- write out non-adjoining ones.
			polyIterator = pCurrentRoom->GetPolygonList().Begin();
			for(; polyIterator != pCurrentRoom->GetPolygonList().End(); ++polyIterator)
			{
				const Portal::C_BuilderPolygon* pCurrentPoly = NULL;
				pCurrentPoly = &(*polyIterator);
				assert(pCurrentPoly);					// Sanity check -- invalid polygon pointer.

				// Skip portal polygons.
				if(pCurrentPoly->IsAdjoining())
					continue;

				serialiser << pCurrentPoly->GetVertexCount();
			
				Btl::List<Portal::C_BuilderVertex*>::ConstIterator vertexIterator = pCurrentPoly->GetVertexList().Begin();
				for(; vertexIterator != pCurrentPoly->GetVertexList().End(); ++vertexIterator)
				{
					const Portal::C_BuilderVertex* pCurrentVert = NULL;
					pCurrentVert = (*vertexIterator);
					BIZ_WARNING_MSG(pCurrentVert, "Sanity check failed -- Invalid pointer.");				// Sanity check -- invalid vertex pointer.

					serialiser << pCurrentVert->GetPosition();
				}
			}
		}
	}
	
	return bSuccess;
}

// -----------------------------------------------------------------

/// Get the portal builder object.
Portal::C_Builder& C_DataPortalData::GetBuilder()
{
	return m_PortalBuilder;
}
