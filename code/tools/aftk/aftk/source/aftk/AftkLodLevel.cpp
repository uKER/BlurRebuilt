#include <stdafx.h>

using namespace Aftk;

C_LodLevel::C_LodLevel()
{
}

C_LodLevel::C_LodLevel(const C_LodLevel& rhs)
{
	*this = rhs;
}

C_LodLevel::~C_LodLevel()
{
	Clear();
}

const C_LodLevel& C_LodLevel::operator= (const C_LodLevel& rhs)
{
	m_info = rhs.m_info;

	//Clear polygon array.
	Ut::MoveRaw(m_polygons, (PolygonArray&)(PolygonArray const&)PolygonArray());
	m_polygons = rhs.m_polygons;

	return *this;
}

void C_LodLevel::SetInfo(const C_MeshLodLevelInfo& info)
{
	m_info = info;
}

C_MeshLodLevelInfo& C_LodLevel::GetInfo()
{
	return m_info;
}

const C_MeshLodLevelInfo& C_LodLevel::GetInfo() const
{
	return m_info;
}

void C_LodLevel::SetPolygonInitialCapacity(U32 capacity)
{
	if (capacity > (U32)m_polygons.Capacity())
	{
		m_polygons.Reserve(capacity);
	}
}

S32 C_LodLevel::AddPolygon(const Aftk::C_Polygon& polygon)
{
	U32 newPolyIndex = m_polygons.Size();
	m_polygons.PushBack(polygon);

	return newPolyIndex;
}

void C_LodLevel::DeletePolygon(U32 index)
{
	m_polygons.EraseByIndex(index);
}

void C_LodLevel::SetPolygon(U32 instance, const Aftk::C_Polygon& polygon)
{
	m_polygons[instance] = polygon;
}

Aftk::C_Polygon& C_LodLevel::GetPolygon(U32 polyIndex)
{
	AFTK_ASSERT(polyIndex < (U32)m_polygons.Size());
	return m_polygons[polyIndex];
}

const Aftk::C_Polygon& C_LodLevel::GetPolygon(U32 polyIndex) const
{
	AFTK_ASSERT(polyIndex < (U32)m_polygons.Size());
	return m_polygons[polyIndex];
}

U32 C_LodLevel::GetNumPolygons() const
{
	return (U32)m_polygons.Size();
}

PolygonArray& C_LodLevel::GetPolygonArray()
{
	return m_polygons;
}

const PolygonArray& C_LodLevel::GetPolygonArray() const
{
	return m_polygons;
}

bool C_LodLevel::operator== (const C_LodLevel& rhs) const
{
	if (!EqualityTestVector(m_polygons, rhs.m_polygons))
	{
		return false;
	}
	
	if (!(m_info == rhs.m_info))
	{
		return false;
	}

	return true;
}


void C_LodLevel::Clear()
{
	m_polygons.Clear();
}

// protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS


#endif
