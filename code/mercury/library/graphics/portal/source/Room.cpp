#include <stdafx.h>
#include <Portal/Room.h>
#include <Portal/System.h>

using namespace Portal;

Debug::Int		C_Room::m_iPolygonIndex("Graphics.Portal.Debug.Traversal.RoomPolygonIndex", ~0);

// Default c'tor -- disallowed.
C_Room::C_Room() :
	m_pParentSystem(NULL),
	m_iNameIndex(0xffffffff)
{
	return;
}

// -----------------------------------------------------------------

// Specified c'tor, takes parent pointer.
C_Room::C_Room(Portal::C_System *pParentSystem, U32 iStringIndex) :
	m_pParentSystem(pParentSystem),
	m_iNameIndex(iStringIndex)
{
	return;
}

// -----------------------------------------------------------------

// D'tor.
C_Room::~C_Room()
{
	// Placeholder method.
	return;
}

// -----------------------------------------------------------------

// Return true if the room contains the specified point.
NOALIAS const bool C_Room::Contains(const Maths::HVec3& rkPoint) const __restrict
{
	return m_pParentSystem->GetSpaceAtIndex(m_iSpaceIndex).Contains(rkPoint, m_pParentSystem);
}

// -----------------------------------------------------------------

// Debug render the room.
void C_Room::DebugRender(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::HMat43 &rkCameraMat)
{
	Maths::RGBA	white(1.0f, 1.0f, 1.0f, 1.0f);

	if(!C_System::GetRenderSolidRooms())
	{
		// Draw wireframe of all the polygons that form the room.
		for(U32 iCurrentPoly=0; iCurrentPoly<static_cast<U32>(m_vPolys.Size()); ++iCurrentPoly)
		{
			const C_RoomPolygon* pPolygon = &m_vPolys[iCurrentPoly];
		        
			Maths::HVec3 prevPos = pPolygon->GetVertexAtIndex(pPolygon->GetVertexCount() - 1);
			for(U32 iCurrentVertex=0; iCurrentVertex<pPolygon->GetVertexCount(); ++iCurrentVertex)
			{
				Maths::HVec3 currPos = pPolygon->GetVertexAtIndex(iCurrentVertex);
				pDebugRender->Draw3dLine(prevPos, currPos, white);                
				prevPos = currPos;    
			}
		}
	}
	else
	{
		// Draw solid polygon for each polygon in the room.
		for(U32 iCurrentPoly=0; iCurrentPoly<static_cast<U32>(m_vPolys.Size()); ++iCurrentPoly)
		{
			const C_RoomPolygon* pPolygon = &m_vPolys[iCurrentPoly];
		        
			U32 iIndex0 = 0;
			U32 iIndex1 = 0;
			U32 iIndex2 = 0;
	
			if(m_iPolygonIndex.Value() == ~0)
			{
				for(U32 iCurrentVertex=1; iCurrentVertex<pPolygon->GetVertexCount()-1; ++iCurrentVertex)
				{
					iIndex1 = iCurrentVertex;
					iIndex2 = iCurrentVertex+1;
					Maths::HVec3 currPos1 = pPolygon->GetVertexAtIndex(iIndex0);
					Maths::HVec3 currPos2 = pPolygon->GetVertexAtIndex(iIndex1);
					Maths::HVec3 currPos3 = pPolygon->GetVertexAtIndex(iIndex2);

					pDebugRender->Draw3dTriangleLit(currPos1, currPos2, currPos3, Maths::RGBA(0.0f, 0.0f, 1.0f, 0.3f));
				}

				Maths::HVec3 prevPos = pPolygon->GetVertexAtIndex(pPolygon->GetVertexCount() - 1);
				for(U32 iCurrentVertex=0; iCurrentVertex<pPolygon->GetVertexCount(); ++iCurrentVertex)
				{
					Maths::HVec3 currPos = pPolygon->GetVertexAtIndex(iCurrentVertex);
					pDebugRender->Draw3dLine(prevPos, currPos, white);                
					prevPos = currPos;    
				}
			}
			else
			{
				const C_RoomPolygon* pPolygon = &m_vPolys[m_iPolygonIndex.Value()];
				for(U32 iCurrentVertex=1; iCurrentVertex<pPolygon->GetVertexCount()-1; ++iCurrentVertex)
				{
					iIndex1 = iCurrentVertex;
					iIndex2 = iCurrentVertex+1;
					Maths::HVec3 currPos1 = pPolygon->GetVertexAtIndex(iIndex0);
					Maths::HVec3 currPos2 = pPolygon->GetVertexAtIndex(iIndex1);
					Maths::HVec3 currPos3 = pPolygon->GetVertexAtIndex(iIndex2);

					pDebugRender->Draw3dTriangleLit(currPos1, currPos2, currPos3, Maths::RGBA(0.0f, 0.0f, 1.0f, 0.3f));
				}

				Maths::HVec3 prevPos = pPolygon->GetVertexAtIndex(pPolygon->GetVertexCount() - 1);
				for(U32 iCurrentVertex=0; iCurrentVertex<pPolygon->GetVertexCount(); ++iCurrentVertex)
				{
					Maths::HVec3 currPos = pPolygon->GetVertexAtIndex(iCurrentVertex);
					pDebugRender->Draw3dLine(prevPos, currPos, white);                
					prevPos = currPos;    
				}
			}
		}
	}
		
	// If set draw the room names.
	if(Portal::C_System::GetRenderDebugRoomNames())
	{
		Maths::RGBA colour(1.0f, 1.0f, 1.0f, 1.0f);	// White by default.
		if(this == m_pParentSystem->GetLastCameraRoom()	&& C_System::GetHighlightCurrentRoom())
			colour = Maths::RGBA(1.0f, 0.0f, 1.0f, 1.0f);

		pDebugRender->Draw3dSphere(GetDebugCentre(), 1.0f, colour);
		pDebugRender->Draw3dText(GetDebugCentre(), m_pParentSystem->GetStringByIndex(m_iNameIndex).Chars());
	}

	return;
}

// -----------------------------------------------------------------

// Return the index of a portal at specified array index.
NOALIAS const U32 C_Room::GetPortalAtIndex(U32 iIndex) const __restrict
{
	BIZ_WARNING_MSG(iIndex < static_cast<U32>(m_vPortals.Size()), "Index out of range.");
	if(iIndex >= static_cast<U32>(m_vPortals.Size()))
		return C_Portal::InvalidPortal;

	return m_vPortals[iIndex];
}

// -----------------------------------------------------------------

// Return the number of portals in the current room.
NOALIAS const U32 C_Room::GetPortalCount() const __restrict
{
	return static_cast<U32>(m_vPortals.Size());
}

// -----------------------------------------------------------------

// Return the index of the string into the string table.
NOALIAS Util::C_StringId C_Room::GetStringIndex() const __restrict
{
	return m_iNameIndex;
}

// -----------------------------------------------------------------

// Return  room polygon at specified index.
const C_RoomPolygon& C_Room::GetRoomPolygonAtIndex(U32 iIndex) const
{
	BIZ_WARNING_MSG(iIndex < static_cast<U32>(m_vPolys.Size()), "Index out of range.");		// Index out of range.
	return m_vPolys[iIndex];
}

// -----------------------------------------------------------------

// Return the number of polygons in the room.
NOALIAS const U32 C_Room::GetRoomPolygonCount() const __restrict
{
	return static_cast<U32>(m_vPolys.Size());
}

// -----------------------------------------------------------------

// Calculate the centre of the room for debug output.
NOALIAS const Maths::HVec3 C_Room::GetDebugCentre() const __restrict
{
	Maths::HVec3 totalVert = Maths::HVec3(0.0f, 0.0f, 0.0f);
	U32			 iVertCount = 0;

    for(U32 iCurrentPoly=0; iCurrentPoly<static_cast<U32>(m_vPolys.Size()); ++iCurrentPoly)
    {
        const C_RoomPolygon* pPolygon = &m_vPolys[iCurrentPoly];

		for(U32 iCurrentVert=0; iCurrentVert<pPolygon->GetVertexCount(); ++iCurrentVert, ++iVertCount)
            totalVert += pPolygon->GetVertexAtIndex(iCurrentVert);                         
	}

	return totalVert / static_cast<F32>(iVertCount);
}
