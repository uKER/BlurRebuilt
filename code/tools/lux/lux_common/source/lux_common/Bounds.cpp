#include "stdafx.h"

#include <lux_common/Bounds.h>

using namespace LPVS;

LPVS::Bounds::Bounds()
{
	Reset();
}

LPVS::Bounds::~Bounds()
{
}

void LPVS::Bounds::AddVertex(const Position3& vertex)
{
	bool updateBB = false;
	if (vertex.x < m_bb[0].x)
	{
		m_bb[0].x = vertex.x;
		updateBB = true;
	}
	if (vertex.y < m_bb[0].y)
	{
		m_bb[0].y = vertex.y;
		updateBB = true;
	}
	if (vertex.z < m_bb[0].z)
	{
		m_bb[0].z = vertex.z;
		updateBB = true;
	}

	if (vertex.x > m_bb[1].x)
	{
		m_bb[1].x = vertex.x;
		updateBB = true;
	}
	if (vertex.y > m_bb[1].y)
	{
		m_bb[1].y = vertex.y;
		updateBB = true;
	}
	if (vertex.z > m_bb[1].z)
	{
		m_bb[1].z = vertex.z;
		updateBB = true;
	}

	if (updateBB)
	{
		//Calc bounding box centre.
		m_centre.x = (m_bb[1].x - m_bb[0].x) * 0.5f + m_bb[0].x;
		m_centre.y = (m_bb[1].y - m_bb[0].y) * 0.5f + m_bb[0].y;
		m_centre.z = (m_bb[1].z - m_bb[0].z) * 0.5f + m_bb[0].z;
		{
			//Let's calculate the radius^2 of the sphere enclosing this bounding box.
			F32 dx = m_bb[1].x - m_centre.x;
			F32 dy = m_bb[1].y - m_centre.y;
			F32 dz = m_bb[1].z - m_centre.z;
			m_bbRadiusSquared = dx*dx + dy*dy + dz*dz;
		}
	}

	{
		//Let's calculate the radius^2 of the sphere enclosing this object.
		F32 dx = vertex.x - m_centre.x;
		F32 dy = vertex.y - m_centre.y;
		F32 dz = vertex.z - m_centre.z;
		F32 currentObjectRadiusSquared = dx*dx + dy*dy + dz*dz;

		if (currentObjectRadiusSquared > m_objectRadiusSquared)
		{
			m_objectRadiusSquared = currentObjectRadiusSquared;
		}
	}
}



const Vector3F32& LPVS::Bounds::GetMin() const
{
	return m_bb[0];
}
		
const Vector3F32& LPVS::Bounds::GetMax() const
{
	return m_bb[1];
}

const Position3& LPVS::Bounds::GetCentre() const
{
	return m_centre;
}

F32 LPVS::Bounds::GetBoundingBoxRadiusSquared() const
{
	return m_bbRadiusSquared;
}

F32 LPVS::Bounds::GetObjectRadiusSquared() const
{
	return m_objectRadiusSquared;
}

void LPVS::Bounds::Clear()
{
	Reset();
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

void LPVS::Bounds::Reset()
{
	m_bb[0] = Vector3F32(100000.0f,100000.0f,100000.0f);
	m_bb[1] = Vector3F32(-100000.0f,-100000.0f,-100000.0f);

	m_centre = Position3(0,0,0);
	m_objectRadiusSquared = 0;
	m_bbRadiusSquared = 0;
}


#endif
