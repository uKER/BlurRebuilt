#include <nexus_render/stdafx.h>
#include <nexus_render/Sphere.h>

using namespace NexusRender;

Sphere::Sphere()
{
}

Sphere::Sphere(const Maths::HVec3& position, F32 radiusSq)
{
	m_position = position;
	m_radiusSq = radiusSq;
}

void Sphere::SetPosition(const Maths::HVec3& position)
{
	m_position = position;
}

void Sphere::SetRadiusSq(F32 radiusSq)
{
	m_radiusSq = radiusSq;
}

F32 Sphere::RadiusSq() const
{
	return m_radiusSq;
}

const Maths::HVec3& Sphere::Position() const
{
	return m_position;
}


