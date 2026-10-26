#include <stdafx.h>

#include <aftk/AftkBounds.h>

using namespace Aftk;

C_Bounds::C_Bounds()
{
	Clear();
}

C_Bounds::C_Bounds(const C_Bounds& rhs)
{
	*this = rhs;
}

C_Bounds::~C_Bounds()
{
}

const C_Bounds& C_Bounds::operator=(const C_Bounds& rhs)
{
	m_bb[0] = rhs.m_bb[0];
	m_bb[1] = rhs.m_bb[1];
	m_bbRadiusSquared = rhs.m_bbRadiusSquared;
	m_centre = rhs.m_centre;
	m_objectRadiusSquared = rhs.m_objectRadiusSquared;

	return *this;
}

C_Bounds C_Bounds::operator +(const Position3& vertex) const
{
	C_Bounds result(*this);
	result += vertex;

	return result;
}

C_Bounds C_Bounds::operator +(const C_Bounds& rhs) const
{
	C_Bounds result(*this);
	result += rhs;
	return result;
}

const C_Bounds& C_Bounds::operator +=(const Position3& vertex)
{
	AddVertex(vertex);
	return *this;
}

const C_Bounds& C_Bounds::operator +=(const C_Bounds& rhs)
{
	AddVertex(rhs.GetMin());
	AddVertex(rhs.GetMax());
	return *this;
}

void C_Bounds::AddVertex(const Position3& vertex)
{
	bool updateBB = false;
	if (vertex[0] < m_bb[0][0])
	{
		m_bb[0][0] = vertex[0];
		updateBB = true;
	}
	if (vertex[1] < m_bb[0][1])
	{
		m_bb[0][1] = vertex[1];
		updateBB = true;
	}
	if (vertex[2] < m_bb[0][2])
	{
		m_bb[0][2] = vertex[2];
		updateBB = true;
	}

	if (vertex[0] > m_bb[1][0])
	{
		m_bb[1][0] = vertex[0];
		updateBB = true;
	}
	if (vertex[1] > m_bb[1][1])
	{
		m_bb[1][1] = vertex[1];
		updateBB = true;
	}
	if (vertex[2] > m_bb[1][2])
	{
		m_bb[1][2] = vertex[2];
		updateBB = true;
	}

	if (updateBB)
	{
		//Calc bounding box centre.
		m_centre[0] = (m_bb[1][0] - m_bb[0][0]) * 0.5f + m_bb[0][0];
		m_centre[1] = (m_bb[1][1] - m_bb[0][1]) * 0.5f + m_bb[0][1];
		m_centre[2] = (m_bb[1][2] - m_bb[0][2]) * 0.5f + m_bb[0][2];
		{
			//Let's calculate the radius^2 of the sphere enclosing this bounding box.
			F32 dx = m_bb[1][0] - m_centre[0];
			F32 dy = m_bb[1][1] - m_centre[1];
			F32 dz = m_bb[1][2] - m_centre[2];
			m_bbRadiusSquared = dx*dx + dy*dy + dz*dz;
		}
	}

	{
		//Let's calculate the radius^2 of the sphere enclosing this object.
		F32 dx = vertex[0] - m_centre[0];
		F32 dy = vertex[1] - m_centre[1];
		F32 dz = vertex[2] - m_centre[2];
		F32 currentObjectRadiusSquared = dx*dx + dy*dy + dz*dz;

		if (currentObjectRadiusSquared > m_objectRadiusSquared)
		{
			m_objectRadiusSquared = currentObjectRadiusSquared;
		}
	}
}

const Position3& C_Bounds::GetMin() const
{
	return m_bb[0];
}
		
const Position3& C_Bounds::GetMax() const
{
	return m_bb[1];
}

const Position3& C_Bounds::GetCentre() const
{
	return m_centre;
}

F32 C_Bounds::GetBoundingBoxRadiusSquared() const
{
	return m_bbRadiusSquared;
}

F32 C_Bounds::GetObjectRadiusSquared() const
{
	return m_objectRadiusSquared;
}

bool C_Bounds::operator== (const C_Bounds &rhs) const
{
	return C_Bounds::IsEqualTo(rhs);
}

bool C_Bounds::IsEqualTo(const C_Bounds &rhs, U32 flags) const
{
	return C_Bounds::IsEqualToThis(rhs, flags);
}

bool C_Bounds::IsEqualToThis(const C_Bounds &rhs, U32 flags) const
{
	if (EqualityTestFVector(m_bb[0], rhs.m_bb[0]) &&
	EqualityTestFVector(m_bb[1], rhs.m_bb[1]) &&
	EqualityTestFVector(m_centre, rhs.m_centre) &&
	(m_objectRadiusSquared == rhs.m_objectRadiusSquared) && (m_bbRadiusSquared == rhs.m_bbRadiusSquared))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void C_Bounds::Clear()
{
	m_bb[0] = Position3(100000.0f,100000.0f,100000.0f);
	m_bb[1] = Position3(-100000.0f,-100000.0f,-100000.0f);

	m_centre = Position3(0,0,0);
	m_objectRadiusSquared = 0;
	m_bbRadiusSquared = 0;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#endif
