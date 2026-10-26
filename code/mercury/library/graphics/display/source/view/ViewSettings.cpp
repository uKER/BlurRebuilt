#include "stdafx.h"
#include "Display/View/ViewSettings.h"

namespace Display
{

C_ViewSettings2D::C_ViewSettings2D()
{
}

C_ViewSettings2D::~C_ViewSettings2D()
{
}

C_ViewSettings3D::C_ViewSettings3D() :
	m_FieldOfView(0.f),
	m_AspectRatio(0.f),
	m_PerspDist(0.f),
	m_NearClip(0.f),
	m_FarClip(0.f)
{
}

C_ViewSettings3D::~C_ViewSettings3D()
{
}

const F32 C_ViewSettings3D::GetScreenSize(const Maths::C_Sphere& worldBoundingSphere) const
{
	Maths::HVec3 pos = worldBoundingSphere.Centre();
    pos = pos * m_WorldToCamera;

	const F32 viewWidth = m_ViewExtents.Max().X() - m_ViewExtents.Min().X();

	if(pos.Z() <= 0.0f)
		return FLT_MAX;
	else
		return (worldBoundingSphere.Radius() * m_PerspDist * 1024.0f) / (pos.Z() * viewWidth);
}

const F32 C_ViewSettings3D::GetScreenSizeRelative(const Maths::C_Sphere& worldBoundingSphere) const
{
	Maths::HVec3 centre = worldBoundingSphere.Centre();
	Maths::HVec3 cameraToSphere = centre - m_CameraToWorld.AW();
	Maths::HVec4 coord(worldBoundingSphere.Radius(), Maths::Constants<Maths::HF32>::Zero(), cameraToSphere | m_CameraToWorld.AZ(), Maths::Constants<Maths::HF32>::One());
	coord = coord * m_CameraToDevice;
	return Maths::Abs(coord.X() / coord.W());
}

const Maths::HVec3 C_ViewSettings3D::TransformWorldToWindow(const Maths::HVec3& worldPos, bool* isBehind) const
{
	Maths::HVec4 pos4d = Maths::HVec4(worldPos);
    pos4d.W(Maths::Constants<Maths::HF32>::One());
    const Maths::HVec4 transformedPos = pos4d * m_WorldToWindow;

	Maths::HVec3 result;
	if(transformedPos.W() > 0.00001f)
	{
		result = Maths::HVec3(transformedPos) / (Maths::HF32)transformedPos.W();
		result.Z(Maths::Constants<Maths::HF32>::One());

		if (isBehind)
			*isBehind = false;
	}
	else
	{
		result = Maths::HVec3(0.0f, 0.0f, -1.0f);
		
		if (isBehind)
			*isBehind = true;
	}

    return result;
}

void C_ViewSettings3D::LookAt(const Maths::HVec3& pos, const Maths::HVec3& dir)
{
	const Maths::HVec3 up(0.f, 1.f, 0.f);
	const Maths::HVec3 at = pos + dir;

}

}; // namespace Display
