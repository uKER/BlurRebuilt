#include "StdAfx.h"
#include "Samples/System/View.h"

namespace Samples
{

C_View::C_View()
{
}

C_View::~C_View()
{
}

void C_View::Initialise(const Display::C_Rect& windowCoords)
{
	m_WindowCoords = windowCoords;
	m_Camera.m_AspectRatio *= (m_WindowCoords.x2-m_WindowCoords.x1) / m_WindowCoords.y2-m_WindowCoords.y1;

	m_Camera.Initialise();
}

void C_View::Update(const F32 frameTime)
{
	m_Camera.Update(frameTime);

	const Maths::HVec3 position = m_Camera.m_Eye;
	const Maths::HVec3 direction = (m_Camera.m_At + m_Camera.m_Offset) - m_Camera.m_Eye;
	const Maths::HVec3 up = m_Camera.m_Up;

	Maths::HVec3 tox, toy, toz;
	Maths::HMat33 look;
	toz = direction;
	toz.Normalise();
	tox = up ^ toz;
	tox.Normalise();
	toy = toz ^ tox;
	toy.Normalise();
	look.AX(tox);
	look.AY(toy);
	look.AZ(toz);

	m_ViewSetup.m_ViewSetup3D.m_CameraToWorld = Maths::HMat43(look);
	m_ViewSetup.m_ViewSetup3D.m_CameraToWorld.AW(position);

	m_ViewSetup.m_ViewSetup3D.m_AspectRatio = m_Camera.m_AspectRatio;
	m_ViewSetup.m_ViewSetup3D.m_FieldOfView = m_Camera.m_FieldOfView;

	F32 height = tanf(m_Camera.m_FieldOfView * 0.5f) * Display::C_ViewSetup::GetDeviceCoords().Max().X();
	F32 width = height * m_Camera.m_AspectRatio;

	m_ViewSetup.m_ViewSetup3D.m_ViewExtents.SetBounds(-width, -height, width, height);
}

}; // namespace Samples
