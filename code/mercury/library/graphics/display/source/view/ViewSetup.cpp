#include "stdafx.h"
#include "Display/View/ViewSetup.h"

namespace Display
{

//#ifdef USER_ECLAY
//Maths::C_Rect C_ViewSetup::m_DeviceCoords(Maths::HVec2(0.f, 0.f), Maths::HVec2(1280.f, 720.f));
//#else
Maths::C_Rect C_ViewSetup::m_DeviceCoords(Maths::HVec2(0.f, 0.f), Maths::HVec2(1024.f, 768.f)); // Default rendering coords...
//#endif

C_ViewSetup2D::C_ViewSetup2D()
{
	m_ViewExtents = C_ViewSetup::GetDeviceCoords();
}

C_ViewSetup2D::~C_ViewSetup2D()
{
}

C_ViewSetup3D::C_ViewSetup3D() :
	m_WindowProportion(1.f),
	m_AspectRatio(16.f/9.f),
	m_FieldOfView(1.025f),
	m_NearClip(0.1f),
	m_FarClip(500.f)
{
	const F32 w = C_ViewSetup::GetDeviceCoords().Size().X();
	const F32 h = C_ViewSetup::GetDeviceCoords().Size().Y();

	m_PerspDist = w;

	m_ViewExtents.SetBounds(-w/2.f, -h/2.f, w/2.f, h/2.f);
	
	m_CameraToWorld.SetIdentity();
}

C_ViewSetup3D::~C_ViewSetup3D()
{
}

C_ViewSetup::C_ViewSetup()
{
	m_WindowCoords = m_DeviceCoords;
}

C_ViewSetup::~C_ViewSetup()
{
}

}; // namespace Display
