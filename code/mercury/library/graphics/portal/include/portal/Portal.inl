// Deserialise a portal from a stream.
template <typename _T>
void C_Portal::Deserialise(_T& rkStream)
{
	rkStream >> m_Plane;
	rkStream >> m_iRoomIndex;
	rkStream >> m_iPortalIndex;

	U32 iVertexCount = 0;
	rkStream >> iVertexCount;
	BIZ_WARNING_MSG(iVertexCount, "No vertices found when deserialising a Portal.");		// Polygon with no verts!?!

	m_vVertexPos.Reserve(iVertexCount);
	
	for(U32 iCurrentVertex=0; iCurrentVertex<iVertexCount; ++iCurrentVertex)
	{
		Maths::C_Vertex currentPos;
		rkStream >> currentPos;
		m_vVertexPos.PushBack(currentPos);		
	}
	return;
}

