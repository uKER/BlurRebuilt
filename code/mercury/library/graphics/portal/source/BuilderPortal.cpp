#include <stdafx.h>
#include <Portal/BuilderPortal.h>
#include <Portal/BuilderRoom.h>

using namespace Portal;


// Specified c'tor -- takes parent room pointer.
C_BuilderPortal::C_BuilderPortal(C_BuilderRoom* pParent) :
	m_pCounterpart(NULL),
	m_pParent(pParent)
{
	return;
}

// -----------------------------------------------------------------

// D'tor.
C_BuilderPortal::~C_BuilderPortal()
{

}

// -----------------------------------------------------------------

// Create a new portal polygon vertex.
Maths::HVec3* C_BuilderPortal::CreateNewPolygonVertex()
{
	m_vVerts.PushBack(Maths::HVec3());
	return &m_vVerts.At(m_vVerts.Size()-1);
}

// -----------------------------------------------------------------

// Return the portal's counterpart -- portal on other side.
const C_BuilderPortal* C_BuilderPortal::GetCounterpart() const
{
	return m_pCounterpart;
}

// -----------------------------------------------------------------

// Return pointer to the room that contains this portal.
const C_BuilderRoom* C_BuilderPortal::GetParent() const
{
	return m_pParent;
}

// -----------------------------------------------------------------

// Return the plane that the polygon for this portal lies in.
const Maths::C_Plane& C_BuilderPortal::GetPlane() const
{
	return m_Plane;
}

// -----------------------------------------------------------------

// Return a vertex at a specified index.
const Maths::HVec3& C_BuilderPortal::GetVertexAtIndex(U32 iIndex) const
{
	assert(iIndex < static_cast<U32>(m_vVerts.Size()));
	return m_vVerts[iIndex];
}

// -----------------------------------------------------------------

// Return the number of vertices in the portal's polygon.
const U32 C_BuilderPortal::GetVertexCount() const
{
	return static_cast<U32>(m_vVerts.Size());
}

// -----------------------------------------------------------------

// Set the portal's counterpart.
void C_BuilderPortal::SetCounterpart(C_BuilderPortal* pCounterpart)
{
	m_pCounterpart = pCounterpart;
	return;
}

// -----------------------------------------------------------------

// Set the plane for the portal's polygon.
void C_BuilderPortal::SetPlane(const Maths::C_Plane& rkPlane)
{
	m_Plane = rkPlane;
	return;
}
