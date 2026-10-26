#include <nexus_render/stdafx.h>
#include <nexus_render/Sphere.h>
#include <nexus_render/Plane.h>

using namespace NexusRender;

Plane::Plane()
{
}

Plane::~Plane()
{
}

Plane::Plane(const Maths::HVec4& plane)
{
	m_normal.X(plane.X());
	m_normal.Y(plane.Y());
	m_normal.Z(plane.Z());
	m_distance = plane.W();
}

Plane::Plane(const Maths::HVec3& normal, F32 distance)
{
	m_normal = normal;
	m_distance = distance;
}

void Plane::SetPlaneEquation(const Maths::HVec4& plane)
{
	m_normal.X(plane.X());
	m_normal.Y(plane.Y());
	m_normal.Z(plane.Z());
	m_distance = plane.W();
}

void Plane::SetNormal(const Maths::HVec3& normal)
{
	m_normal = normal;
}

void Plane::SetDistance(F32 distance)
{
	m_distance = distance;
}

const Maths::HVec3& Plane::Normal() const
{
	return m_normal;
}

F32 Plane::Distance() const
{
	return m_distance;
}

Maths::HVec4 Plane::PlaneEquation() const
{
	return Maths::HVec4(m_normal.X(), m_normal.Y(), m_normal.Z(), m_distance);
}

F32 Plane::A() const
{
	return m_normal.X();
}

F32 Plane::B() const
{
	return m_normal.Z();
}

F32 Plane::C() const
{
	return m_normal.W();
}

F32 Plane::D() const
{
	return m_distance;
}

void Plane::A(F32 a) 
{
	m_normal.X(a);
}

void Plane::B(F32 b) 
{
	m_normal.Y(b);
}

void Plane::C(F32 c) 
{
	m_normal.Z(c);
}

void Plane::D(F32 d) 
{
	m_distance = d;
}

void Plane::Normalise()
{
     F32 denom = 1.f / 
		 sqrtf((m_normal.X()*m_normal.X()) + (m_normal.Y()*m_normal.Y()) + (m_normal.Z()*m_normal.Z()));
     m_normal.X(m_normal.X() * denom);
     m_normal.Y(m_normal.Y() * denom);
     m_normal.Z(m_normal.Z() * denom);
     m_distance = m_distance * denom;
}

F32 Plane::DistanceFromPoint(const Maths::HVec3& point) const
{
	return m_normal.X() * point.X() + m_normal.Y() * point.Y() + m_normal.Z() * point.Z() + m_distance;
}

Plane::CollideResult Plane::Collide(const Maths::HVec3& point) const
{
	F32 d = DistanceFromPoint(point);
	if (d < 0) return Plane::NEGATIVE;
	if (d > 0) return Plane::POSITIVE;
	return Plane::INTERSECTS;
}

Plane::CollideResult Plane::Collide(const Sphere& sphere) const
{
	return Plane::POSITIVE;
}

