#include <nexus_render/stdafx.h>
#include <nexus_render/Sphere.h>
#include <nexus_render/Plane.h>
#include <nexus_render/Frustum.h>

using namespace NexusRender;

void Frustum::ExtractPlanes(const Maths::HMat44& matrix)
{
	m_leftPlane.A(matrix.AX().W() + matrix.AX().X());
	m_leftPlane.B(matrix.AY().W() + matrix.AY().X());
	m_leftPlane.C(matrix.AZ().W() + matrix.AZ().X());
	m_leftPlane.D(matrix.AW().W() + matrix.AW().X());

	m_rightPlane.A(matrix.AX().W() - matrix.AX().X());
	m_rightPlane.B(matrix.AY().W() - matrix.AY().X());
	m_rightPlane.C(matrix.AZ().W() - matrix.AZ().X());
	m_rightPlane.D(matrix.AW().W() - matrix.AW().X());

	m_topPlane.A(matrix.AX().W() - matrix.AX().Y());
	m_topPlane.B(matrix.AY().W() - matrix.AY().Y());
	m_topPlane.C(matrix.AZ().W() - matrix.AZ().Y());
	m_topPlane.D(matrix.AW().W() - matrix.AW().Y());

	m_bottomPlane.A(matrix.AX().W() + matrix.AX().Y());
	m_bottomPlane.B(matrix.AY().W() + matrix.AY().Y());
	m_bottomPlane.C(matrix.AZ().W() + matrix.AZ().Y());
	m_bottomPlane.D(matrix.AW().W() + matrix.AW().Y());

	m_nearPlane.A(matrix.AX().Z());
	m_nearPlane.B(matrix.AY().Z());
	m_nearPlane.C(matrix.AZ().Z());
	m_nearPlane.D(matrix.AW().Z());

	m_farPlane.A(matrix.AX().W() - matrix.AX().Z());
	m_farPlane.B(matrix.AY().W() - matrix.AY().Z());
	m_farPlane.C(matrix.AZ().W() - matrix.AZ().Z());
	m_farPlane.D(matrix.AW().W() - matrix.AW().Z());

	m_leftPlane.Normalise();
	m_rightPlane.Normalise();
	m_topPlane.Normalise();
	m_bottomPlane.Normalise();
	m_nearPlane.Normalise();
	m_farPlane.Normalise();
}

bool Frustum::Contains(const Sphere& sphere) const
{
	F32 d;
	d = m_leftPlane.DistanceFromPoint(sphere.Position());
	if (((d*d)*Maths::Sign(d)) <= -(sphere.RadiusSq()))
		//Cull the object. It lies outside our frustum.
		return false;
	d = m_rightPlane.DistanceFromPoint(sphere.Position());
	if (((d*d)*Maths::Sign(d)) <= -(sphere.RadiusSq()))
		//Cull the object. It lies outside our frustum.
		return false;
	d = m_topPlane.DistanceFromPoint(sphere.Position());
	if (((d*d)*Maths::Sign(d)) <= -(sphere.RadiusSq()))
		//Cull the object. It lies outside our frustum.
		return false;
	d = m_bottomPlane.DistanceFromPoint(sphere.Position());
	if (((d*d)*Maths::Sign(d)) <= -(sphere.RadiusSq()))
		//Cull the object. It lies outside our frustum.
		return false;
	d = m_nearPlane.DistanceFromPoint(sphere.Position());
	if (((d*d)*Maths::Sign(d)) <= -(sphere.RadiusSq()))
		//Cull the object. It lies outside our frustum.
		return false;
	d = m_farPlane.DistanceFromPoint(sphere.Position());
	if (((d*d)*Maths::Sign(d)) <= -(sphere.RadiusSq()))
		//Cull the object. It lies outside our frustum.
		return false;

	//We're either intersecting with, or are contained by the frustum
	return true;
}

bool Frustum::Contains(const Maths::HVec3& point) const
{
	if (m_leftPlane.Collide(point) == Plane::NEGATIVE)
		return false;
	if (m_rightPlane.Collide(point) == Plane::NEGATIVE)
		return false;
	if (m_topPlane.Collide(point) == Plane::NEGATIVE)
		return false;
	if (m_bottomPlane.Collide(point) == Plane::NEGATIVE)
		return false;
	if (m_nearPlane.Collide(point) == Plane::NEGATIVE)
		return false;
	if (m_farPlane.Collide(point) == Plane::NEGATIVE)
		return false;

	return true;
}
