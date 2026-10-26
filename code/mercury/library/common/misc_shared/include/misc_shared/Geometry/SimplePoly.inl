#ifndef SIMPLEPOLY_INL
#define SIMPLEPOLY_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace SimplePoly
{

inline C_Polygon::C_Polygon()
{
}

inline C_Polygon::~C_Polygon()
{
}

inline void C_Polygon::operator =( const C_Polygon& other )
{
    m_Plane = other.m_Plane;
	const S32 otherVertexCount = other.m_VertexList.Size();
    m_VertexList.Resize(otherVertexCount);
    
	for(S32 i=0; i<otherVertexCount;i++)
	{
		m_VertexList[i] = other.m_VertexList[i];
	}
}

inline void C_Polygon::NewVertex( const Maths::HVec3& pos )
{
	m_VertexList.PushBack(pos);
}

inline const Maths::C_Plane C_Polygon::Plane() const
{
    return m_Plane;
}

inline const Maths::HVec3 C_Polygon::Centroid() const
{
    return m_Centroid;
}




} // namespace SimplePoly

#endif
