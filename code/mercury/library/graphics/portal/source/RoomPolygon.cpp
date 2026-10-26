#include <stdafx.h>
#include <Portal/RoomPolygon.h>

using namespace Portal;


// Default c'tor.
C_RoomPolygon::C_RoomPolygon()
{
	return;
}

// -----------------------------------------------------------------

// D'tor.
C_RoomPolygon::~C_RoomPolygon()
{
	return;
}

// -----------------------------------------------------------------

// Debug render the room polygon.
void C_RoomPolygon::DebugRender(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::RGBA &rkColour) const
{
	const Maths::RGBA	blue(0.0f, 0.0f, 1.0f, 1.0f);
	
	// Draw wireframe of the room polygon.
	Maths::HVec3 prevPos = m_vVertexData.At(m_vVertexData.Size() - 1);
	for(U32 iCurrentVertex=0; iCurrentVertex<static_cast<U32>(m_vVertexData.Size()); ++iCurrentVertex)
	{
		Maths::HVec3 currPos = m_vVertexData.At(iCurrentVertex);
		pDebugRender->Draw3dLine(prevPos, currPos, blue);
		prevPos = currPos;
	}
	return;
}

// -----------------------------------------------------------------

// Return the vertex at the specified index.
const Maths::HVec3& C_RoomPolygon::GetVertexAtIndex(U32 iIndex) const
{
	return m_vVertexData.At(iIndex);
}

// -----------------------------------------------------------------

// Return the number of vertices in the polygon.
const U32 C_RoomPolygon::GetVertexCount() const
{
	return static_cast<U32>(m_vVertexData.Size());
}
	