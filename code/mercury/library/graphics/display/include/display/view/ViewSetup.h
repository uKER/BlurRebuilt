#ifndef DISPLAY_VIEW_VIEW_SETUP_H
#define DISPLAY_VIEW_VIEW_SETUP_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

//! View setup 2d.

class C_ViewSetup2D
{
public:

	C_ViewSetup2D();
	~C_ViewSetup2D();

public:

	Maths::C_Rect			m_ViewExtents;
};

//! View setup 3d.

class C_ViewSetup3D
{
public:

	C_ViewSetup3D();
	~C_ViewSetup3D();

public:
	F32						m_WindowProportion;
	F32						m_AspectRatio; // Global aspect ratio of the screen
	F32						m_FieldOfView;
	F32						m_NearClip;
	F32						m_FarClip;
	F32						m_PerspDist;
	Maths::C_Rect			m_ViewExtents;
	Maths::HMat43			m_CameraToWorld;
};

//! View setup.

class C_ViewSetup
{
public:

	C_ViewSetup();
	~C_ViewSetup();

	static const Maths::C_Rect& GetDeviceCoords();

public:

	Maths::C_Rect			m_WindowCoords;
	C_ViewSetup2D			m_ViewSetup2D;
	C_ViewSetup3D			m_ViewSetup3D;

private:

	static Maths::C_Rect	m_DeviceCoords;
};

}; // namespace Display

#include "Display/View/ViewSetup.inl"

#endif // DISPLAY_VIEW_VIEW_SETUP_H
