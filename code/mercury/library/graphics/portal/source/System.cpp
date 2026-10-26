#include <stdafx.h>
#include <Portal/System.h>
#include <Portal/ViewPolygon.h>
#include <Display/View/ViewSettings.h>
#include <system/Perf/PerfEvent.h>
#include <system/memory/MemoryTracker.h>
#include <system/serialise/C_FileBlock.h>
#include <system/serialise/BinarySerialise.h>
#include <core/utility/endian.h>

using namespace Portal;

#ifndef PLATFORM_SPU
// Static class members.
Debug::Bool			C_System::m_bDebugShowRoomNames			("Graphics.Portal.Debug.Traversal.ShowDebugRoomsNames", false);
Debug::Int			C_System::m_iMaxRecursionLevels			("Graphics.Portal.Debug.Coder.MaxRecursionLevels", 10);
Debug::Bool			C_System::m_bDebugShowViewPoly			("Graphics.Portal.Debug.Traversal.ShowDebugViewPoly", false);
Debug::Float		C_System::m_fDebugTestPortalsBorderX	("Graphics.Portal.Debug.Coder.TestBorderX", 68.0f, Debug::Range(0.0f, 512.0f));
Debug::Float		C_System::m_fDebugTestPortalsBorderY	("Graphics.Portal.Debug.Coder.TestBorderY", 48.0f, Debug::Range(0.0f, 512.0f));
Debug::Bool			C_System::m_bDebugShowBlockers			("Graphics.Portal.Debug.Blockers.ShowBlockers", false);
Debug::Bool			C_System::m_bDebugShowCurrentRoom		("Graphics.Portal.Debug.Traversal.HightlightRoom", false);
Debug::Bool			C_System::m_bDebugShowSpaces			("Graphics.Portal.Debug.Coder.ShowSpaces", false);
Debug::Int			C_System::m_bDebugSpaceIndex			("Graphics.Portal.Debug.Coder.SpaceIndex", 0);
Debug::Bool			C_System::m_bDebugShowPortals			("Graphics.Portal.Debug.Traversal.ShowPortals", false);
Debug::Bool			C_System::m_bDebugShowRooms				("Graphics.Portal.Debug.Traversal.ShowRooms", false);
Debug::Bool			C_System::m_bDebugShowRays				("Graphics.Portal.Debug.Rays.ShowRays", false);
Debug::Bool			C_System::m_bDebugShowGrid				("Graphics.Portal.Debug.Coder.ShowGrid", false);
Debug::Bool			C_System::m_bDebugSolidRooms			("Graphics.Portal.Debug.Traversal.RenderRoomsAsSolidPolygons", false);
Debug::Bool			C_System::m_bDebugDrawOnlyVisibleRooms	("Graphics.Portal.Debug.Traversal.DrawOnlyVisibleRooms", false);
Debug::Bool			C_System::m_bDebugSetWorldRoomBit		("Graphics.Portal.Debug.Traversal.SetWorldRoomBit", true);
Debug::Bool			C_System::m_bDebugSolidPortals			("Graphics.Portal.Debug.Traversal.RenderPortalsAsSolidPolygons", false);
Debug::Int			C_System::m_iDebugRenderRoomOnly		("Graphics.Portal.Debug.Traversal.DebugRoomIndex", ~0);
Debug::Float		C_System::m_fDebugMinArea				("Graphics.Portal.Debug.Traversal.MinProjectedArea", 0.0f);
Debug::Int			C_System::m_bDebugSingleRoom			("Graphics.Portal.Debug.Traversal.RenderRoomIndex", -1);

C_DeferredBlocker	C_System::m_DeferredBlocker;
Btl::Vector<C_System*, Btl::VectorParameters_Static<PORTAL_CONFIG_MAXPORTALSYSTEMS> >
					C_System::m_vPortalSystems;

// Logging object for portal system.
Debug::Logger& Portal::g_Log = Debug::GetLogger("mercury.Graphics.Portal");


// Return the room index for a specified point.
NOALIAS const U32 C_System::Classify(const Maths::HVec3& rkPoint) const
{
#if !PORTAL_DEBUG_DONT_USE_GRID

	const U32 iCellIndex = m_RoomGrid.CalculateCellIndexFromPosition(rkPoint);
	if(iCellIndex != C_RoomGrid::InvalidCell)
	{
		const C_RoomGridCell* pGridCell = m_RoomGrid.GetCellAtIndex(iCellIndex);
		const U32 iSubSpaceCount = pGridCell->GetSubSpaceCount();
		for(U32 iCurrentSubSpaceIndex=0; iCurrentSubSpaceIndex<iSubSpaceCount; ++iCurrentSubSpaceIndex)
		{
			bool bFailed = false;
			const C_RoomContainmentSubSpace* pCurrentSubSpace = &m_vSubSpaces[m_vCellSpaceIndices[iCurrentSubSpaceIndex]];	// Level of indirection to flatten data structures.

			// Check if we are on positive side of all the planes.
			for(U32 iCurrentPlaneIndex=pCurrentSubSpace->GetStartIndex(); iCurrentPlaneIndex<pCurrentSubSpace->GetEndIndex()+1; ++iCurrentPlaneIndex)
			{
				// Is the point on the wrong side of the plane?
				if(m_vPlanes[iCurrentPlaneIndex].Distance(rkPoint) > 0.0f)
				{
					bFailed = true;
					break;
				}
			}

			// Return the room index for the sub-space if we are contained within that sub-space.
			if(!bFailed)
				return m_vSubSpaces[iCurrentSubSpaceIndex].GetRoomIndex();
		}
	}

#else

	{
		PERF_EVENT("portal.classify");
		// Try ALL the sub-spaces, no grid optimisation.
		for(U32 iCurrentSubSpaceIndex=0, iSubSpaceCount=static_cast<U32>(m_vSubSpaces.Size()); iCurrentSubSpaceIndex<iSubSpaceCount; ++iCurrentSubSpaceIndex)
		{
			bool bFailed = false;
			const C_RoomContainmentSubSpace* pCurrentSubSpace = &m_vSubSpaces[iCurrentSubSpaceIndex];
			
			DCBT(1 * CACHELINE_SIZE, pCurrentSubSpace);	// Prefetch more sub-spaces into the L2 cache.
			DCBT(2 * CACHELINE_SIZE, pCurrentSubSpace);	// Prefetch more sub-spaces into the L2 cache.
			DCBT(3 * CACHELINE_SIZE, pCurrentSubSpace);	// Prefetch more sub-spaces into the L2 cache.
			DCBT(4 * CACHELINE_SIZE, pCurrentSubSpace);	// Prefetch more sub-spaces into the L2 cache.

			// Check if we are on positive side of all the planes.
			for(U32 iCurrentPlaneIndex=pCurrentSubSpace->GetStartIndex(), iPlaneCount=pCurrentSubSpace->GetEndIndex()+1; iCurrentPlaneIndex<iPlaneCount; ++iCurrentPlaneIndex)
			{
				// Is the point on the wrong side of the plane?
				if(m_vPlanes[iCurrentPlaneIndex].Distance(rkPoint) > 0.0f)
				{
					bFailed = true;
					break;
				}
			}

			// Return the room index for the sub-space if we are contained within that sub-space.
			if(!bFailed)
				return m_vSubSpaces[iCurrentSubSpaceIndex].GetRoomIndex();
		}
	}
	

#endif

	return 0;	// Outside of the portal system, return world room index.
	}

// -----------------------------------------------------------------

// Return the room index for a specified point, check the last room first.
NOALIAS const U32 C_System::Classify(const Maths::HVec3& rkPoint, const U32 iLastIndex) const
{
	BIZ_WARNING_MSG(iLastIndex == C_Room::InvalidRoom || iLastIndex < static_cast<U32>(m_vRooms.Size()), "Invalid index.");
	
	if(iLastIndex != C_Room::InvalidRoom)
	{
		// Try the early-out if it was a valid room.
		if(m_vRooms[iLastIndex].Contains(rkPoint) && iLastIndex)
			return iLastIndex;
	}
	
	const C_RoomContainmentSubSpace* pSubSpaces = &m_vSubSpaces[0];
	DCBT(1 * CACHELINE_SIZE, pSubSpaces);	// Prefetch more sub-spaces into the L2 cache.
	DCBT(2 * CACHELINE_SIZE, pSubSpaces);	// Prefetch more sub-spaces into the L2 cache.
	DCBT(3 * CACHELINE_SIZE, pSubSpaces);	// Prefetch more sub-spaces into the L2 cache.
	DCBT(4 * CACHELINE_SIZE, pSubSpaces);	// Prefetch more sub-spaces into the L2 cache.
	return Classify(rkPoint);
}

// -----------------------------------------------------------------

// Debug render the entire portal system.
void C_System::DebugRender(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::HMat43& rkCameraMat, const Btl::BitVector& rkVisibleRoomSet)
{
#if PORTAL_DEBUG_LOAD_DEBUG_DATA	// Only do the debug rendering if we built with debugging enabled.
	
	BIZ_WARNING_MSG(pDebugRender, "Invalid debug render pointer.");
	if(!pDebugRender)
		return;

	pDebugRender->SetBlendMode(DbUtil::E_BlendMode_Alpha);

	if(m_bDebugShowGrid.Value())
		DebugRenderRoomGrid(pDebugRender, rkCameraMat);

	if(m_bDebugShowSpaces.Value())
		DebugRenderSpaces(pDebugRender, rkCameraMat);
	
	if(m_bDebugShowRooms.Value())
		DebugRenderRooms(pDebugRender, rkCameraMat, rkVisibleRoomSet);
	
	if(m_bDebugShowPortals.Value())
		DebugRenderPortals(pDebugRender);

	if(m_bDebugShowRays.Value())
		DebugRenderRays(pDebugRender, rkCameraMat);
		
	if(m_bDebugShowBlockers.Value())
		DebugRenderBlockers(pDebugRender);

#endif // PORTAL_DEBUG_LOAD_DEBUG_DATA

	return;
}

// -----------------------------------------------------------------

// Debug render blockers.
void C_System::DebugRenderBlockers(DbUtil::I_DebugRenderPtr pDebugRender)
{
	BIZ_WARNING_MSG(pDebugRender, "Invalid debug render pointer.");
	if(!pDebugRender)
		return;

	m_DeferredBlocker.DebugRender(pDebugRender);
	return;
}

// -----------------------------------------------------------------

// Debug render only the portals.
void C_System::DebugRenderPortals(DbUtil::I_DebugRenderPtr pDebugRender)
{
	BIZ_WARNING_MSG(pDebugRender, "Invalid debug render pointer.");
	if(!pDebugRender)
		return;

	for(U32 iCurrentPortal=0; iCurrentPortal<static_cast<U32>(m_vPortals.Size()); ++iCurrentPortal)
		m_vPortals[iCurrentPortal].DebugRender(pDebugRender, *this, m_vPortalsBlocked.GetBitValue(iCurrentPortal), iCurrentPortal);

	return;
}

// -----------------------------------------------------------------

// Debug render deferred rays.
void C_System::DebugRenderRays(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::HMat43& rkCameraMat)
{
	BIZ_WARNING_MSG(pDebugRender, "Invalid debug render pointer.");
	if(!pDebugRender)
		return;

	m_Tracer.DebugRenderTracedRays(pDebugRender, rkCameraMat);
	return;
}

// -----------------------------------------------------------------

// Debug render the room grid.
void C_System::DebugRenderRoomGrid(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::HMat43& rkCameraMat)
{
	BIZ_WARNING_MSG(pDebugRender, "Invalid debug render pointer.");
	if(!pDebugRender)
		return;

	m_RoomGrid.DebugRender(pDebugRender, rkCameraMat);
	return;
}
		
// -----------------------------------------------------------------

// Debug render only the rooms.
void C_System::DebugRenderRooms(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::HMat43& rkCameraMat, const Btl::BitVector& rkVisibleRoomSet)
{
	BIZ_WARNING_MSG(pDebugRender, "Invalid debug render pointer.");
	if(!pDebugRender)
		return;

	if(rkVisibleRoomSet.Size() < m_vRooms.Size())
		return;	// Some odd mismatch occured.

	for(U32 iCurrentRoom=0; iCurrentRoom<static_cast<U32>(m_vRooms.Size()); ++iCurrentRoom)
	{
		if(m_iDebugRenderRoomOnly.Value() == ~0)
		{
			if(!m_bDebugDrawOnlyVisibleRooms.Value())
				m_vRooms[iCurrentRoom].DebugRender(pDebugRender, rkCameraMat);
			else
			{
				if(rkVisibleRoomSet[iCurrentRoom])
					m_vRooms[iCurrentRoom].DebugRender(pDebugRender, rkCameraMat);
			}
		}
		else if (m_iDebugRenderRoomOnly == iCurrentRoom)
		{
			m_vRooms[iCurrentRoom].DebugRender(pDebugRender, rkCameraMat);
		}
	}
	return;
}

// -----------------------------------------------------------------

// Debug render only the spaces.
void C_System::DebugRenderSpaces(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::HMat43& rkCameraMat)
{
	if(!m_bDebugShowSpaces.Value())
		return;

	BIZ_WARNING_MSG(pDebugRender, "Invalid debug render pointer.");
	if(!pDebugRender)
		return;

	// Build the debug meshes the first time through.
	static bool bDone = false;
	if(!bDone)
	{
		for(U32 iCurrentSpace=0; iCurrentSpace<static_cast<U32>(m_vSpaces.Size()); ++iCurrentSpace)
			m_vSpaces[iCurrentSpace].BuildDebugMesh();

		bDone = true;
	}

	// Draw the debug meshes.
	for(U32 iCurrentSpace=0; iCurrentSpace<static_cast<U32>(m_vSpaces.Size()); ++iCurrentSpace)
		m_vSpaces[iCurrentSpace].DebugRender(pDebugRender, rkCameraMat, Maths::RGBA(0xff, 0xaa, 0xff, 0xff));

	m_vSpaces[m_bDebugSpaceIndex.Value()].DebugRender(pDebugRender, rkCameraMat, Maths::RGBA(0xff, 0x00, 0xff, 0xff));
	LOGDEBUG(g_Log, "Showing containment planes for: %s.\n", m_StringTable.GetString(m_vRooms[m_bDebugSpaceIndex.Value()].GetStringIndex()));
	
	return;
}

// -----------------------------------------------------------------

// Main entry point for portal system evaluation -- debug version.
void C_System::DebugEvaluate(Btl::BitVector &rkBitfield, const Display::C_ViewSettings3D& rkViewSettings, DbUtil::I_DebugRenderPtr pDebugRenderer) const
{
	BIZ_WARNING_MSG(pDebugRenderer, "Invalid debug render pointer.");
	if(!m_vRooms.Size())
		return;		// Portal rooms to evaluate.

	// Ready bitfield for visibility flags.
	rkBitfield.Resize(m_vRooms.Size());
	rkBitfield.ClearBits();
	
	// Calculate full screen view polygon for traversal.
	C_ViewPolygon	fullScreenPolygon;
	F32 fXMin = rkViewSettings.m_ViewExtents.Min().X();
	F32 fXMax = rkViewSettings.m_ViewExtents.Max().X();
	F32 fYMin = rkViewSettings.m_ViewExtents.Min().Y();
	F32 fYMax = rkViewSettings.m_ViewExtents.Max().Y();
	F32 fPerspDist = rkViewSettings.m_PerspDist;
	Maths::HMat43 cameraMatrix = rkViewSettings.GetCameraToWorld();
	fullScreenPolygon.AddVertex(Maths::HVec3(fXMin + m_fDebugTestPortalsBorderX, fYMax - m_fDebugTestPortalsBorderY, fPerspDist) * cameraMatrix);
	fullScreenPolygon.AddVertex(Maths::HVec3(fXMax - m_fDebugTestPortalsBorderX, fYMax - m_fDebugTestPortalsBorderY, fPerspDist) * cameraMatrix);
	fullScreenPolygon.AddVertex(Maths::HVec3(fXMax - m_fDebugTestPortalsBorderX, fYMin + m_fDebugTestPortalsBorderY, fPerspDist) * cameraMatrix);
	fullScreenPolygon.AddVertex(Maths::HVec3(fXMin + m_fDebugTestPortalsBorderX, fYMin + m_fDebugTestPortalsBorderY, fPerspDist) * cameraMatrix);

	const C_Room*			pCameraRoom = NULL;
	
	if(m_pLastCameraRoom && m_pLastCameraRoom->Contains(cameraMatrix.AW()))
		pCameraRoom = m_pLastCameraRoom;	// Last room contains camera, skip containment checks.
	else
	{
		// Search for the camera.
		pCameraRoom			= FindRoomContainingCamera(cameraMatrix);

#ifdef _MSC_VER 
#pragma warning(push)
#pragma warning(disable: 4146)
#endif // _MSC_VER
		const INTPTR mask	= -(INTPTR)(!pCameraRoom);	// [Blur2PC][x64] pointer-wide select
#ifdef _MSC_VER 
#pragma warning(pop)
#endif // _MSC_VER 
		const INTPTR roomPtr	= (INTPTR)(&m_vRooms[0]);
		const INTPTR oldPtr	= (INTPTR)pCameraRoom;
		pCameraRoom			= (C_Room*)(((oldPtr^roomPtr) & mask) ^ oldPtr);
	}

	DebugEvaluate(rkBitfield, rkViewSettings, pCameraRoom, fullScreenPolygon, pDebugRenderer); 
	
	// Don't cache the world room as current.
#ifdef _MSC_VER 
#pragma warning(push)
#pragma warning(disable: 4146)
#endif // _MSC_VER
	const INTPTR mask		= -(INTPTR)(pCameraRoom != &m_vRooms[0]);	// [Blur2PC][x64] pointer-wide select
#ifdef _MSC_VER 
#pragma warning(pop)
#endif // _MSC_VER 
	const INTPTR roomPtr	= (INTPTR)(pCameraRoom);
	m_pLastCameraRoom		= (C_Room*)(((roomPtr^(INTPTR)m_pLastCameraRoom)&mask)^roomPtr);

	return;
}

// -----------------------------------------------------------------

// Evalute the portal system -- debug version.
void C_System::DebugEvaluate(Btl::BitVector& rkBitfield, const Display::C_ViewSettings3D& rkViewSettings, const C_Room* pRoom, const C_ViewPolygon& rkViewPoly, DbUtil::I_DebugRenderPtr pDebugRenderer, U32 iRecurseLevel) const
{
	if(iRecurseLevel >= static_cast<U32>(m_iMaxRecursionLevels.Value()))
	{
		LOGWARN(g_Log, "Bad portal data! Bailed out due to maximum recursion cap of %d, could potentially cause unsightly popping in distant rooms.\n", m_iMaxRecursionLevels.Value());
		return;
	}

	BIZ_WARNING_MSG(pRoom, "NULL room pointer.");

	
	// Mark current room as visible.
	U32	iRoomIndex = CalculateRoomIndex(pRoom);
	BIZ_WARNING_MSG(iRoomIndex != C_Room::InvalidRoom, "Invalid room index.");														// Sanity check -- room wasn't in the list... something seriously drastic happened.
	rkBitfield[iRoomIndex] = true;

	Maths::HVec3 cameraPos = rkViewSettings.GetCameraPosition();


	if(pRoom->GetPortalCount())
	{
		for(U32 iCurrentPortal=0; iCurrentPortal<pRoom->GetPortalCount(); ++iCurrentPortal)
		{
			BIZ_WARNING_MSG(pRoom->GetPortalAtIndex(iCurrentPortal) < static_cast<U32>(m_vPortals.Size()), "Index out of range.");	// Sanity check -- index out of range.
			const C_Portal* pCurrentPortal = &m_vPortals[pRoom->GetPortalAtIndex(iCurrentPortal)];
			BIZ_WARNING_MSG(pCurrentPortal, "Invalid portal pointer.");																// Sanity check -- invalid pointer.

			// TODO: [Stove] Range check to portal.
			if(m_vPortalsBlocked.GetBitValue(pRoom->GetPortalAtIndex(iCurrentPortal)))
				continue;	// Skip portal if it is blocked.

			if(pCurrentPortal->GetPlane().Distance(cameraPos) < 0.0f)
				continue;	// Skip portal if distance from camera is negative.

			if(pCurrentPortal->GetCounterpartPortal() != C_Portal::InvalidPortal)
			{
				const C_Portal* pOtherSide = &m_vPortals[pCurrentPortal->GetCounterpartPortal()];
				BIZ_WARNING_MSG(pOtherSide, "Invalid counterpart portal pointer.");													// Sanity check -- invalid pointer.
				BIZ_WARNING_MSG(pOtherSide->GetParentRoomIndex() != C_Room::InvalidRoom, "Invalid room index.");					// Sanity check -- invalid room index.

				// Figure out what room we are looking into through current portal.
				const C_Room* pNewRoom = &m_vRooms[pOtherSide->GetParentRoomIndex()];

				// Clip view polygon to the portal we are looking at.
				C_ViewPolygon	clippedViewPoly;
				rkViewPoly.Clip(clippedViewPoly, pCurrentPortal, cameraPos);
				
				const F32 fArea = pCurrentPortal->GetScreenSpaceArea(Maths::Load<Maths::HMat44>(&rkViewSettings.GetViewProjMatrix()));
				if(m_bDebugShowViewPoly.Value())
				{
					clippedViewPoly.DebugRender(pDebugRenderer, Maths::RGBA(0.0f, 0.0f, 1.0f, 1.0f));
					Util::C_String debugText = "";
					debugText.Printf("%f", fArea);
					pDebugRenderer->Draw3dText(pCurrentPortal->GetVertexAtIndex(0), 5.0f, debugText);
				}
				
				// Do the recursion only if there was a well-formed polygon.
				if(clippedViewPoly.GetVertexCount() >= 3 && fArea > m_fDebugMinArea.Value())
					if(pRoom != &m_vRooms[0])
						DebugEvaluate(rkBitfield, rkViewSettings, pNewRoom, clippedViewPoly, pDebugRenderer, iRecurseLevel + 1);
					else
						if(iRecurseLevel <= 1)
							DebugEvaluate(rkBitfield, rkViewSettings, pNewRoom, clippedViewPoly, pDebugRenderer, iRecurseLevel + 1);
			}
		}
	}

	return;
}

// -----------------------------------------------------------------

// Main entry point for portal system evaluation.
void C_System::Evaluate(Btl::BitVector &rkBitfield, Btl::Vector<U16>& rkRoomIndexList, const Display::C_ViewSettings3D& rkViewSettings) const
{
	PERF_EVENT("Portal::C_System::Evaluate");
	if(!m_vRooms.Size())
		return;	// No portals to evaluate.

	// Ready bitfield for visibility flags.
	rkBitfield.Resize(m_vRooms.Size());
	rkRoomIndexList.Clear();
	rkRoomIndexList.Reserve(m_vRooms.Size());
	rkBitfield.ClearBits();
	
	// Calculate full screen view polygon for traversal starting point.
	C_ViewPolygon	fullScreenPolygon;
	Maths::HF32 fXMin			= rkViewSettings.m_ViewExtents.Min().X();
	Maths::HF32 fXMax			= rkViewSettings.m_ViewExtents.Max().X();
	Maths::HF32 fYMin			= rkViewSettings.m_ViewExtents.Min().Y();
	Maths::HF32 fYMax			= rkViewSettings.m_ViewExtents.Max().Y();
	Maths::HF32 fPerspDist		= rkViewSettings.m_PerspDist;
	Maths::HMat43 cameraMatrix	= rkViewSettings.GetCameraToWorld();
#ifndef CFG_GOLD
	fullScreenPolygon.AddVertex(Maths::HVec3(fXMin + m_fDebugTestPortalsBorderX, fYMax - m_fDebugTestPortalsBorderY, fPerspDist) * cameraMatrix);
	fullScreenPolygon.AddVertex(Maths::HVec3(fXMax - m_fDebugTestPortalsBorderX, fYMax - m_fDebugTestPortalsBorderY, fPerspDist) * cameraMatrix);
	fullScreenPolygon.AddVertex(Maths::HVec3(fXMax - m_fDebugTestPortalsBorderX, fYMin + m_fDebugTestPortalsBorderY, fPerspDist) * cameraMatrix);
	fullScreenPolygon.AddVertex(Maths::HVec3(fXMin + m_fDebugTestPortalsBorderX, fYMin + m_fDebugTestPortalsBorderY, fPerspDist) * cameraMatrix);
#else
	fullScreenPolygon.AddVertex(Maths::HVec3(fXMin, fYMax, fPerspDist) * cameraMatrix);
	fullScreenPolygon.AddVertex(Maths::HVec3(fXMax, fYMax, fPerspDist) * cameraMatrix);
	fullScreenPolygon.AddVertex(Maths::HVec3(fXMax, fYMin, fPerspDist) * cameraMatrix);
	fullScreenPolygon.AddVertex(Maths::HVec3(fXMin, fYMin, fPerspDist) * cameraMatrix);
#endif // #ifndef CFG_GOLD

	const C_Room*			pCameraRoom = NULL;
	
	const C_RoomContainmentSubSpace* pSubSpaces = &m_vSubSpaces[0];
	DCBT(1 * CACHELINE_SIZE, pSubSpaces);	// Prefetch more sub-spaces into the L2 cache.
	DCBT(2 * CACHELINE_SIZE, pSubSpaces);	// Prefetch more sub-spaces into the L2 cache.
	DCBT(3 * CACHELINE_SIZE, pSubSpaces);	// Prefetch more sub-spaces into the L2 cache.
	DCBT(4 * CACHELINE_SIZE, pSubSpaces);	// Prefetch more sub-spaces into the L2 cache.

	if(m_pLastCameraRoom && m_pLastCameraRoom->Contains(cameraMatrix.AW()))
		pCameraRoom = m_pLastCameraRoom;	// Last room contains camera, skip containment checks.
	else
	{
		// Search for the camera.
		pCameraRoom			= FindRoomContainingCamera(cameraMatrix);

#ifdef _MSC_VER 
#pragma warning(push)
#pragma warning(disable: 4146)
#endif // _MSC_VER
		const INTPTR mask	= -(INTPTR)(!pCameraRoom);	// [Blur2PC][x64] pointer-wide select
#ifdef _MSC_VER 
#pragma warning(pop)
#endif // _MSC_VER 
		const INTPTR roomPtr	= (INTPTR)(&m_vRooms[0]);
		const INTPTR oldPtr	= (INTPTR)pCameraRoom;
		pCameraRoom			= (C_Room*)(((oldPtr^roomPtr) & mask) ^ oldPtr);

	}

#ifndef CFG_GOLD
	if(m_bDebugSetWorldRoomBit.Value())
#endif
	{
		// World room is always visible.
		rkBitfield[0] = true;
		rkRoomIndexList.PushBack(0);
	}

	if(m_bDebugSingleRoom > -1 && m_bDebugSingleRoom < rkBitfield.Size())
	{
		// World room is always visible.
		rkBitfield[m_bDebugSingleRoom] = true;
		rkRoomIndexList.PushBack(m_bDebugSingleRoom);
	}
	else
	{
		Evaluate(rkBitfield, rkRoomIndexList, rkViewSettings, pCameraRoom, fullScreenPolygon); 
	}

#ifdef _MSC_VER 
#pragma warning(push)
#pragma warning(disable: 4146)
#endif // _MSC_VER
	const INTPTR mask		= -(INTPTR)(pCameraRoom != &m_vRooms[0]);	// [Blur2PC][x64] pointer-wide select
#ifdef _MSC_VER 
#pragma warning(pop)
#endif // _MSC_VER 
	const INTPTR roomPtr	= (INTPTR)(pCameraRoom);
	m_pLastCameraRoom		= (C_Room*)(((roomPtr^(INTPTR)m_pLastCameraRoom)&mask)^roomPtr);
	
	return;
}

// -----------------------------------------------------------------

// Evalute the portal system.
void C_System::Evaluate(Btl::BitVector& rkBitfield, Btl::Vector<U16>& rkRoomIndexList, const Display::C_ViewSettings3D& rkViewSettings, const C_Room* pRoom, const C_ViewPolygon& rkViewPoly, U32 iRecurseLevel) const
{
	if(iRecurseLevel >= static_cast<U32>(m_iMaxRecursionLevels.Value()))
	{
		LOGWARN(g_Log, "Bad portal data! Bailed out due to maximum recursion cap of %d, could potentially cause unsightly popping in distant rooms.\n", m_iMaxRecursionLevels.Value());
		return;
	}

	BIZ_WARNING_MSG(pRoom, "NULL room pointer.");

	// Mark current room as visible.
	U32	iRoomIndex = CalculateRoomIndex(pRoom);
	BIZ_WARNING_MSG(iRoomIndex != C_Room::InvalidRoom, "Invalid room index.");														// Sanity check -- room wasn't in the list... something seriously drastic happened.
	
	if(iRoomIndex > 0 && !rkBitfield.GetBitValue(iRoomIndex))
	{
		rkRoomIndexList.PushBack((U16)iRoomIndex);
		rkBitfield[iRoomIndex] = true;
	}

	Maths::HVec3 cameraPos = rkViewSettings.GetCameraPosition();

	if(pRoom->GetPortalCount())
	{
		for(U32 iCurrentPortal=0; iCurrentPortal<pRoom->GetPortalCount(); ++iCurrentPortal)
		{
			BIZ_WARNING_MSG(pRoom->GetPortalAtIndex(iCurrentPortal) < static_cast<U32>(m_vPortals.Size()), "Index out of range.");	// Sanity check -- index out of range.
			const C_Portal* pCurrentPortal = &m_vPortals[pRoom->GetPortalAtIndex(iCurrentPortal)];
			BIZ_WARNING_MSG(pCurrentPortal, "Invalid portal pointer.");																// Sanity check -- invalid pointer.
			
			if(m_vPortalsBlocked.GetBitValue(pRoom->GetPortalAtIndex(iCurrentPortal)))
				continue;	// Skip portal if it is blocked.

			if(pCurrentPortal->GetPlane().Distance(cameraPos) < 0.0f)
				continue;	// Skip portal if distance from camera is negative.

			if(pCurrentPortal->GetCounterpartPortal() != C_Portal::InvalidPortal)
			{
				const C_Portal* pOtherSide = &m_vPortals[pCurrentPortal->GetCounterpartPortal()];
				BIZ_WARNING_MSG(pOtherSide, "Invalid counterpart portal pointer.");													// Sanity check -- invalid pointer.
				BIZ_WARNING_MSG(pOtherSide->GetParentRoomIndex() != C_Room::InvalidRoom, "Invalid room index.");					// Sanity check -- invalid room index.
				
				// Figure out what room we are looking into through current portal.
				const C_Room* pNewRoom = &m_vRooms[pOtherSide->GetParentRoomIndex()];
				
				// Clip view polygon to the portal we are looking at.
				C_ViewPolygon	clippedViewPoly;
				rkViewPoly.Clip(clippedViewPoly, pCurrentPortal, cameraPos);

				// Do the recursion only if there was a well-formed polygon.
				if(clippedViewPoly.GetVertexCount() >= 3 && pCurrentPortal->GetScreenSpaceArea(Maths::Load<Maths::HMat44>(&rkViewSettings.GetViewProjMatrix())) > m_fDebugMinArea.Value())
					if(pRoom != &m_vRooms[0])
						Evaluate(rkBitfield, rkRoomIndexList, rkViewSettings, pNewRoom, clippedViewPoly, iRecurseLevel + 1);
					else
						if(iRecurseLevel <= 1)
							Evaluate(rkBitfield, rkRoomIndexList, rkViewSettings, pNewRoom, clippedViewPoly, iRecurseLevel + 1);
			}
		}
	}

	return;
}

// -----------------------------------------------------------------

// Return a vector of active portal system pointers.
const Btl::Vector<C_System*, Btl::VectorParameters_Static<PORTAL_CONFIG_MAXPORTALSYSTEMS> >& C_System::GetActivePortalSystems()
{
	return m_vPortalSystems;
}

// -----------------------------------------------------------------

// Return the deferred blocker.
C_DeferredBlocker& C_System::GetDeferredBlocker()
{
	return m_DeferredBlocker;
}

// -----------------------------------------------------------------

// Return string from the table by Id.
Btl::String C_System::GetStringByIndex(Util::C_StringId iStringId) const
{
	BIZ_WARNING_MSG(m_StringTable.GetStringCount() > 0, "No strings to look-up from.");	// No strings to look-up from.
	if(m_StringTable.GetStringCount() == 0)
		return Btl::String("");
		
	return Btl::String(m_StringTable.GetString(iStringId));
}

// -----------------------------------------------------------------

// Kick the jobs off for the deferred ray tracer.
void C_System::KickDeferredRayTraceJobs()
{
	m_Tracer.StartJobs();
	return;
}

// -----------------------------------------------------------------

// Load C_System object from a file.
bool C_System::LoadFromFile(Sys::C_FileBlock& rkBlock)
{
	LOGINFO(g_Log, "Starting portal system object.");

	U32 iNumberOfRooms		= 0;
	U32 iNumberOfPortals	= 0;
	U32 iNumberOfPlanes		= 0;
	U32 iNumberOfSubSpaces	= 0;
	U32 iNumberOfIndices	= 0;
	U32 iFileVersion		= 0;

	// Read the header chunk.
	{
		Sys::C_FileBlock headerBlock(&rkBlock);
		if(headerBlock.HeaderId() != "Header")
		{
			LOGWARN(g_Log, "Expected a 'Header' block.");
			return false;
		}
		Sys::BinaryDeserialiser<Ut::NoEndianSwap> deserialiser(headerBlock);

		// Read the block data.
		deserialiser >> iFileVersion;
		deserialiser >> iNumberOfRooms;
		deserialiser >> iNumberOfPortals;
		deserialiser >> iNumberOfPlanes;
		deserialiser >> iNumberOfSubSpaces;
	
		if(iFileVersion != E_PortalFileVersion_Current)
		{
			LOGERROR(g_Log, "Bad file version, expected version %d read %d, not loading any portal data!", E_PortalFileVersion_Current, iFileVersion);
			return false;
		}

		if(!iNumberOfRooms || !iNumberOfPortals)
		{
			LOGERROR(g_Log, "0 rooms or 0 portals. Not loading any portal data!");
			return false;
		}
	}

	
	// Read the rooms chunk.
	m_vRooms.Reserve(iNumberOfRooms);
	{
		Sys::C_FileBlock roomsBlock(&rkBlock);
		if(roomsBlock.HeaderId() != "Rooms")
		{
			LOGWARN(g_Log, "Expected a 'Room' block.");
			return false;
		}
		Sys::BinaryDeserialiser<Ut::NoEndianSwap> deserialiser(roomsBlock);
		
		// Read in all the rooms.
		for(U32 iCurrentRoom=0; iCurrentRoom<iNumberOfRooms; ++iCurrentRoom)
		{
			m_vRooms.PushBack(C_Room(this));	
			C_Room* pCurrentRoom = &m_vRooms[m_vRooms.Size()-1];
			pCurrentRoom->Deserialise(deserialiser);
		}
	}

	// Reserve the number of bits we need to represent each portal's visibilty,
	// make all portals un-blocked initially.
	m_vPortalsBlocked.Resize(iNumberOfPortals);
	m_vPortals.Reserve(iNumberOfPortals);
	{
		Sys::C_FileBlock portalsBlock(&rkBlock);
		if(portalsBlock.HeaderId() != "Portals")
		{
			LOGWARN(g_Log, "Expected a 'Portals' block.");
			return false;
		}
		Sys::BinaryDeserialiser<Ut::NoEndianSwap> deserialiser(portalsBlock);
		
		// Read in all the portals.
		for(U32 iCurrentPortal=0; iCurrentPortal<iNumberOfPortals; ++iCurrentPortal)
		{
			m_vPortalsBlocked.ClearBit(iCurrentPortal);
			m_vPortals.PushBack(C_Portal());	
			C_Portal* pCurrentPortal = &m_vPortals[m_vPortals.Size()-1];
			pCurrentPortal->Deserialise(deserialiser);
		}
	}

	// Deserialise the spaces chunk.
	m_vPlanes.Reserve(iNumberOfPlanes);
	m_vSubSpaces.Reserve(iNumberOfSubSpaces);
	m_vSpaces.Reserve(iNumberOfRooms);
	{
		Sys::C_FileBlock spacesBlock(&rkBlock);
		if(spacesBlock.HeaderId() != "Spaces")
		{
			LOGWARN(g_Log, "Expected a 'Spaces' block.");
			return false;
		}
		Sys::BinaryDeserialiser<Ut::NoEndianSwap> deserialiser(spacesBlock);

		// Read in all the planes.
		for(U32 iCurrentSpace=0; iCurrentSpace<iNumberOfPlanes; ++iCurrentSpace)
		{
			Maths::C_Plane currentPlane;
			deserialiser >> currentPlane;
			m_vPlanes.PushBack(currentPlane);
		}

		// Read in all the sub-spaces.
		for(U32 iCurrentSubSpace=0; iCurrentSubSpace<iNumberOfSubSpaces; ++iCurrentSubSpace)
		{
			U32 iStartPlaneIndex = 0;
			U32 iEndPlaneIndex = 0;
			U32 iRoomIndex = 0;
			deserialiser >> iStartPlaneIndex;
			deserialiser >> iEndPlaneIndex;
			deserialiser >> iRoomIndex;
			m_vSubSpaces.PushBack(C_RoomContainmentSubSpace(iStartPlaneIndex, iEndPlaneIndex, iRoomIndex));
		}

		// Read in all the spaces.
		for(U32 iCurrentSpace=0; iCurrentSpace<iNumberOfRooms; ++iCurrentSpace)
		{
			m_vSpaces.PushBack(C_RoomContainmentSpace());	
			C_RoomContainmentSpace* pCurrentSpace = &m_vSpaces[m_vSpaces.Size()-1];
			pCurrentSpace->Deserialise(deserialiser);
		}
	}

	// Deserialise the grid chunk.
	{
		Sys::C_FileBlock gridBlock(&rkBlock);
		if(gridBlock.HeaderId() != "Grid")
		{
			LOGWARN(g_Log, "Expected a 'Grid' block.");
			return false;
		}
		Sys::BinaryDeserialiser<Ut::NoEndianSwap> deserialiser(gridBlock);

		const U32 iLength = static_cast<U32>(gridBlock.Length());
		gridBlock.SetPosition(iLength);
	}	

	// Deserialise the BSP chunk for deferred raytracer.
	{
		Sys::C_FileBlock bspBlock(&rkBlock);
		if(bspBlock.HeaderId() != "BSP")
		{
			LOGWARN(g_Log, "Expected a 'BSP' block.");
			return false;
		}
		Sys::BinaryDeserialiser<Ut::NoEndianSwap> deserialiser(bspBlock);
		deserialiser >> m_Tracer.GetBSPTree();
	}

// Only need to bother with this chunk when debugging, uses by far the most
// memory in the entire portal system. This chunk contains string table for
// room names and polygonal data for rooms.
//  -- STOVE, 191208
#if PORTAL_DEBUG_LOAD_DEBUG_DATA
	// Read the debug chunk.
	{
		Sys::C_FileBlock debugBlock(&rkBlock);
		if(debugBlock.HeaderId() != "Debug")
		{
			LOGWARN(g_Log, "Expected a 'Debug' block.");
			return false;
		}
		Sys::BinaryDeserialiser<Ut::NoEndianSwap> deserialiser(debugBlock);
		deserialiser >> m_StringTable;
		
		for(U32 iCurrentRoom=0; iCurrentRoom<static_cast<U32>(m_vRooms.Size()); ++iCurrentRoom)
			m_vRooms[iCurrentRoom].ReadDebugPolygons(deserialiser);
	}
#endif // PORTAL_DEBUG_LOAD_DEBUG_DATA
	
	LOGINFO(g_Log, "Loaded portal system object.");
	return true;
}

// -----------------------------------------------------------------

// Process deferred ray trace jobs.
void C_System::ProcessDeferredRayTraceJobs()
{
	m_Tracer.EndJobs();
	m_DeferredBlocker.Update();
	return;
}

// -----------------------------------------------------------------

// Reset the portal system object.
void C_System::Reset()
{
	m_vRooms.Clear();
	m_vPortals.Clear();
	m_vSpaces.Clear();
	return;
}

#endif // PLATFORM_SPU

