// Deserialise a room polygon from a stream.
template <typename _T>
void C_RoomPolygon::Deserialise(_T& rkStream)
{
	U32 iVertexCount = 0;
	rkStream >> iVertexCount;
	BIZ_WARNING_MSG(iVertexCount, "Room polygon with no verts, invalid data!");		// Polygon with no verts!?!

	m_vVertexData.Reserve(iVertexCount);
	
	for(U32 iCurrentVertex=0; iCurrentVertex<iVertexCount; ++iCurrentVertex)
	{
		Maths::HVec3 currentPos;
		rkStream >> currentPos;
		m_vVertexData.PushBack(currentPos);		
	}
	return;
}

