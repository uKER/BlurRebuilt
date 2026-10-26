#ifndef DISPLAY_VIEW_VIEW_SETTINGS_INL
#define DISPLAY_VIEW_VIEW_SETTINGS_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
	
INLINE const C_Float4& C_ViewSettings2D::GetClipTransform() const
{
	return m_ClipTransform;
}

INLINE const Maths::C_Transform2d& C_ViewSettings2D::GetDeviceTransform() const
{
	return m_DeviceTransform;
}

INLINE const Maths::C_Transform2d& C_ViewSettings2D::GetShaderTransform() const
{
	return m_ShaderTransform;
}

INLINE const F32 C_ViewSettings3D::GetFieldOfView() const
{
	return m_FieldOfView;
}

INLINE const F32 C_ViewSettings3D::GetAspectRatio() const
{
	return m_AspectRatio;
}

INLINE const F32 C_ViewSettings3D::GetPerspDist() const
{
	return m_PerspDist;
}

INLINE const F32 C_ViewSettings3D::GetNearClip() const
{
	return m_NearClip;
}

INLINE const F32 C_ViewSettings3D::GetFarClip() const
{
	return m_FarClip;
}

INLINE const C_Float4& C_ViewSettings3D::GetViewPosition() const
{
	return m_ViewPosition;
}

INLINE const C_Float4& C_ViewSettings3D::GetViewPositionPrev() const
{
	return m_ViewPositionPrev;
}

INLINE const C_Matrix& C_ViewSettings3D::GetViewMatrix() const
{
	return m_ViewMatrix;
}

INLINE const C_Matrix& C_ViewSettings3D::GetViewMatrixPrev() const
{
	return m_ViewMatrixPrev;
}

INLINE const C_Matrix& C_ViewSettings3D::GetProjMatrix() const
{
	return m_ProjMatrix;
}

INLINE const C_Matrix& C_ViewSettings3D::GetProjMatrixPrev() const
{
	return m_ProjMatrixPrev;
}

INLINE const C_Matrix& C_ViewSettings3D::GetViewProjMatrix() const
{
	return m_ViewProjMatrix;
}

INLINE const C_Matrix& C_ViewSettings3D::GetViewProjMatrixPrev() const
{
	return m_ViewProjMatrixPrev;
}

INLINE const Maths::C_Rect& C_ViewSettings3D::GetViewExtents() const
{
	return m_ViewExtents;
}

INLINE const Maths::HVec3 C_ViewSettings3D::GetCameraPosition() const
{
	return m_CameraToWorld.AW();
}

INLINE const Maths::HVec3 C_ViewSettings3D::GetCameraPositionPrev() const
{
	return m_CameraToWorldPrev.AW();
}

INLINE const Maths::HMat43& C_ViewSettings3D::GetCameraToWorld() const
{
	return m_CameraToWorld;
}

INLINE const Maths::HMat43& C_ViewSettings3D::GetCameraToWorldPrev() const
{
	return m_CameraToWorldPrev;
}

INLINE const Maths::HMat44& C_ViewSettings3D::GetCameraToDevice() const
{
	return m_CameraToDevice;
}

INLINE const Maths::HMat44& C_ViewSettings3D::GetCameraToDevicePrev() const
{
	return m_CameraToDevicePrev;
}

INLINE void C_ViewSettings3D::SetCameraToDevice(Maths::HMat44& ctd)
{
	m_CameraToDevice=ctd;
	Maths::Store(m_CameraToDevice, &m_ProjMatrix);
	m_WorldToDevice = Maths::HMat44(m_WorldToCamera) * m_CameraToDevice;
	Maths::Store(m_WorldToDevice, &m_ViewProjMatrix);
}

INLINE const Maths::HMat43& C_ViewSettings3D::GetWorldToCamera() const
{
	return m_WorldToCamera;
}

INLINE const Maths::HMat43& C_ViewSettings3D::GetWorldToCameraPrev() const
{
	return m_WorldToCameraPrev;
}

INLINE const Maths::HMat44& C_ViewSettings3D::GetWorldToDevice() const
{
	return m_WorldToDevice;
}

INLINE const Maths::HMat44& C_ViewSettings3D::GetWorldToDevicePrev() const
{
	return m_WorldToDevicePrev;
}

INLINE const Maths::HMat44& C_ViewSettings3D::GetWorldToWindow() const
{
	return m_WorldToWindow;
}

}; // namespace Display

#endif // DISPLAY_VIEW_VIEW_SETTINGS_INL
