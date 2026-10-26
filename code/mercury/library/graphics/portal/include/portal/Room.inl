#include <Core/Maths/HVec/Geometry.h>

// Deserialise a room from a stream.
template <typename _T>
void C_Room::Deserialise(_T& rkStream)
{
	U32 iPortalCount=0;
	rkStream >> iPortalCount;
	m_vPortals.Reserve(iPortalCount);
	for(U32 iCurrentPortal=0; iCurrentPortal<iPortalCount; ++iCurrentPortal)
	{
		U32 currentPortal;
		rkStream >> currentPortal;
		m_vPortals.PushBack(currentPortal);
	}
	rkStream >> m_iNameIndex;
	rkStream >> m_iSpaceIndex;
	return;
}

// -----------------------------------------------------------------

// Read in debug polygons for the room from a stream.
template <typename _T>
void C_Room::ReadDebugPolygons(_T& rkStream)
{
	U32 iPolygonCount=0;
	rkStream >> iPolygonCount;
	m_vPolys.Reserve(iPolygonCount);
	for(U32 iCurrentPolygon=0; iCurrentPolygon<iPolygonCount; ++iCurrentPolygon)
	{
		C_RoomPolygon currentPoly;
		rkStream >> currentPoly;
		m_vPolys.PushBack(currentPoly);
	}
	return;
}
