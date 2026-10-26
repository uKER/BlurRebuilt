#include "stdafx.h"
#include "SceneGraph/Bounds.h"

#include "RenderingData/Bounds.h"

namespace SceneGraph
{

void C_Bounds::SetAxisBox(const RenderingData::Bounds &bounds)
{
	Maths::HVec3 vMinimum(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&bounds.aabbMin));
	Maths::HVec3 vMaximum(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&bounds.aabbMax));
	Maths::HVec3 vCentre = (vMinimum + vMaximum) * Maths::Constants<Maths::HF32>::Half();
	Maths::HVec4 vCentreW(vCentre);
	vCentreW.W(Maths::Constants<Maths::HF32>::One());
	Maths::HVec3 vSize = vMaximum - vCentre;
	Maths::HF32 radius = Maths::Length(vSize);
	Maths::HVec4 vSizeCull(vSize);
	vSizeCull.W(radius);

	Maths::Store(vCentreW, &centre);
	Maths::Store(vSizeCull, &size);

	clip.x = bounds.distanceMin;
	clip.y = bounds.distanceMax;
}

void C_Bounds::SetAxisBox(const Maths::HVec3& minPoint, const Maths::HVec3& maxPoint)
{
	Maths::HVec3 vCentre = (minPoint + maxPoint) * Maths::Constants<Maths::HF32>::Half();
	Maths::HVec3 vSize = maxPoint - vCentre;
	Maths::HF32 radius = Maths::Length(vSize);
	Maths::HVec4 vCentreOne(vCentre);
	Maths::HVec4 vSizeRadius(vSize);
	vCentreOne.W(Maths::Constants<Maths::HF32>::One());
	vSizeRadius.W(radius);

	Maths::Store(vCentreOne, &centre);
	Maths::Store(vSizeRadius, &size);
}

void C_Bounds::SetSphere(const Maths::C_Sphere& sphere)
{
	Maths::HVec4 vCentre(sphere.Centre());
	Maths::HVec4 vSize;

	vCentre.W(Maths::Constants<Maths::HF32>::One());
	vSize.W(sphere.Radius());

	Maths::Store(vCentre, &centre);
	Maths::Store(vSize, &size);
}

void C_Bounds::SetSpherePosition(const Maths::HVec3& position)
{
	Maths::HVec4 vCentre(position);
	vCentre.W(Maths::Constants<Maths::HF32>::One());
	Maths::Store(vCentre, &centre);
}

void C_Bounds::SetNearFarDistance(const F32 nearDistance, const F32 farDistance)
{
	clip.x = nearDistance;
	clip.y = farDistance;
}

// works whether bounds are being stored as aabb or sphere
Maths::C_Sphere C_Bounds::GetSphere() const
{
	Maths::HVec3 vCentre(Maths::Load<Maths::HVec3>(&centre));
	Maths::HVec4 vSize(Maths::Load<Maths::HVec4>(&size));

	return Maths::C_Sphere(vCentre, vSize.W());
}

} // namespace SceneGraph

