#ifndef DISPLAY_VIEW_VIEW_SETTINGS_H
#define DISPLAY_VIEW_VIEW_SETTINGS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Utils/Vector.h"
#include "Display/Utils/Matrix.h"

namespace Display
{

//! View settings 2d.

class C_ViewSettings2D
{
public:

	C_ViewSettings2D();
	~C_ViewSettings2D();

	const C_Float4& GetClipTransform() const;

	const Maths::C_Transform2d& GetDeviceTransform() const;
	const Maths::C_Transform2d& GetShaderTransform() const;

public:

	C_Float4				m_ClipTransform;

	Maths::C_Rect			m_ViewExtents;

	Maths::C_Transform2d	m_DeviceTransform;
	Maths::C_Transform2d	m_ShaderTransform;
};

//! View settings 3d.

class C_ViewSettings3D
{
public:

	C_ViewSettings3D();
	~C_ViewSettings3D();

	const F32 GetFieldOfView() const;
	const F32 GetAspectRatio() const;
	const F32 GetPerspDist() const;
	const F32 GetNearClip() const;
	const F32 GetFarClip() const;

	const C_Float4& GetViewPosition() const;
	const C_Float4& GetViewPositionPrev() const;

	const C_Matrix& GetViewMatrix() const;
	const C_Matrix& GetViewMatrixPrev() const;
	
	const C_Matrix& GetProjMatrix() const;
	const C_Matrix& GetProjMatrixPrev() const;
	
	const C_Matrix& GetViewProjMatrix() const;
	const C_Matrix& GetViewProjMatrixPrev() const;
	
	const Maths::C_Rect& GetViewExtents() const;

	const Maths::HVec3 GetCameraPosition() const;
	const Maths::HVec3 GetCameraPositionPrev() const;

	const Maths::HMat43& GetCameraToWorld() const;
	const Maths::HMat43& GetCameraToWorldPrev() const;
	
	const Maths::HMat44& GetCameraToDevice() const;
	const Maths::HMat44& GetCameraToDevicePrev() const;
	void SetCameraToDevice(Maths::HMat44& ctd);

	const Maths::HMat43& GetWorldToCamera() const;
	const Maths::HMat43& GetWorldToCameraPrev() const;

	const Maths::HMat44& GetWorldToDevice() const;
	const Maths::HMat44& GetWorldToDevicePrev() const;

	const Maths::HMat44& GetWorldToWindow() const;

	const F32 GetScreenSize(const Maths::C_Sphere& worldBoundingSphere) const;
	const F32 GetScreenSizeRelative(const Maths::C_Sphere& worldBoundingSphere) const;
	const Maths::HVec3 TransformWorldToWindow(const Maths::HVec3& worldPos, bool* isBehind=NULL) const;

	void LookAt(const Maths::HVec3& pos, const Maths::HVec3& dir);

public:

	F32						m_FieldOfView;
	F32						m_AspectRatio;
	F32						m_PerspDist;
	F32						m_NearClip;
	F32						m_FarClip;

	C_Float4				m_ViewPosition;
	C_Float4				m_ViewPositionPrev;
	
	C_Matrix				m_ViewMatrix;
	C_Matrix				m_ViewMatrixPrev;
	
	C_Matrix				m_ProjMatrix;
	C_Matrix				m_ProjMatrixPrev;
	
	C_Matrix				m_ViewProjMatrix;
	C_Matrix				m_ViewProjMatrixPrev;

	Maths::C_Rect			m_ViewExtents;

	Maths::HMat43			m_CameraToWorldPrev;
	Maths::HMat43			m_CameraToWorld;
	
	Maths::HMat44			m_CameraToDevice;
	Maths::HMat44			m_CameraToDevicePrev;

	Maths::HMat43			m_WorldToCamera;
	Maths::HMat43			m_WorldToCameraPrev;

	Maths::HMat44			m_WorldToDevice;
	Maths::HMat44			m_WorldToDevicePrev;

	Maths::HMat44			m_WorldToWindow;
};

}; // namespace Display

#include "Display/View/ViewSettings.inl"

#endif // DISPLAY_VIEW_VIEW_SETTINGS_H
