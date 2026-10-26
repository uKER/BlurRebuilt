#include "stdafx.h"
#include "Camera.h"
#include <display/view/View.h>
#include <core/maths/hvec/new/IncludeGeneral.h>
#include <core/maths/hvec/new/IncludeGeneral.inl>

C_Camera::C_Camera():
	m_aspectRatio(1.f),
	m_fieldOfView(Maths::DegToRad(54.43f)),//1.025f),
	m_rotateSpeed(1.f),
	m_zoomSpeed(1.f),
	m_moveSpeed(0.2f),
	m_distance(3.f),
	m_long(Maths::Pi / 4.0f),
	m_lat(Maths::Pi / 4.0f),
	m_type(E_CameraType_Orbit)
{
	m_upVector = Maths::Const<Maths::Normal3>::AxisY();
	m_targetPos = Maths::Const<Maths::Point3>::Zero();
	m_targetOffset = Maths::Const<Maths::Vector3>::Zero();
	m_eyePos = Maths::Const<Maths::Point3>::Zero();
	DisableAllInputs();
}

Maths::Point3 C_Camera::GetEyePos() const
{
	Maths::Point3 eyePos;
	switch ( m_type )
	{
		case E_CameraType_Orbit:
		{
			eyePos = Maths::MakePoint3( 
				m_distance * cosf(m_lat) * cosf(m_long),
				m_distance * sinf(m_lat),
				m_distance * cosf(m_lat) * sinf(m_long)
			);
		
			eyePos += m_targetPos + m_targetOffset;
			break;
		}
		
		default:
		case E_CameraType_LookAt:
			eyePos = m_eyePos + m_targetOffset;
			break;
	}
	return eyePos;
}

Maths::Mat43 C_Camera::GetWorldMatrix() const
{
	const Maths::Point3 eyePos = GetEyePos();
	const Maths::Normal3 eyeDir = Maths::Normalise( (m_targetPos + m_targetOffset) - eyePos );
	const Maths::Normal3 eyeUp = m_upVector;
	
	Maths::Vector3 rx, ry, rz;
	rz = Maths::MakeVector3( eyeDir );
	rx = Maths::MakeVector3( Maths::Normalise( eyeUp ^ rz ) );
	ry = Maths::MakeVector3( Maths::Normalise( rz ^ rx ) );
	
	const Maths::Mat33 camToWorldRot = Maths::MakeMat33( rx, ry, rz );
	const Maths::Mat43 camToWorld = Maths::MakeMat43( camToWorldRot, eyePos );
	return camToWorld;
}

void C_Camera::ClearTargetOffset()
{
	m_targetOffset = Maths::Const<Maths::Vector3>::Zero();
}

void C_Camera::SetupView( Display::C_ViewSetup& viewSetup ) const
{
	viewSetup.m_ViewSetup3D.m_CameraToWorld = Maths::HMat43( GetWorldMatrix() );
	viewSetup.m_ViewSetup3D.m_AspectRatio = m_aspectRatio;
	viewSetup.m_ViewSetup3D.m_FieldOfView = m_fieldOfView;

	F32 height = tanf(m_fieldOfView * 0.5f) * Display::C_ViewSetup::GetDeviceCoords().Max().X();
	F32 width = height * m_aspectRatio;

	viewSetup.m_ViewSetup3D.m_ViewExtents.SetBounds(-width, -height, width, height);
}

void C_Camera::Update( F32 deltaTime )
{
	UpdateShared( deltaTime );

	switch (m_type)
	{
		case E_CameraType_Orbit:
			UpdateOrbit( deltaTime );
			break;
			
		default:
			break;
	}
}

void C_Camera::UpdateShared( F32 deltaTime )
{
	const Maths::Mat43 worldMatrix = GetWorldMatrix();
	const Maths::VF32 dt = Maths::MakeVF32(deltaTime);
	
	const Maths::VF32 moveDelta = Maths::MakeVF32(m_moveSpeed) * dt * m_distance;
	
	if ( m_inputActive[E_CameraInput_MoveLR] ) m_targetOffset -= Maths::RowX( worldMatrix ) * moveDelta * Maths::MakeVF32(m_inputValue[E_CameraInput_MoveLR]);
	if ( m_inputActive[E_CameraInput_MoveUD] ) m_targetOffset -= Maths::RowY( worldMatrix ) * moveDelta * Maths::MakeVF32(m_inputValue[E_CameraInput_MoveUD]);
}

void C_Camera::UpdateOrbit( F32 deltaTime )
{
	const F32 zoomDelta = m_zoomSpeed * deltaTime * m_distance;
	if ( m_inputActive[E_CameraInput_Zoom] ) m_distance = Maths::Max(m_distance + zoomDelta * m_inputValue[E_CameraInput_Zoom], 0.05f);
	
	const F32 rotateDelta = m_rotateSpeed * deltaTime;
	if ( m_inputActive[E_CameraInput_RotateLR] ) m_long += rotateDelta * m_inputValue[E_CameraInput_RotateLR];
	if ( m_inputActive[E_CameraInput_RotateUD] ) m_lat += rotateDelta * m_inputValue[E_CameraInput_RotateUD];
	
	const F32 k3PiOver2 = (Maths::Pi * 3.f) / 2.f;
	if ( m_lat > k3PiOver2 )
	{
		m_lat -= Maths::Pi2;
	}
	else if ( m_lat < -k3PiOver2 )
	{
		m_lat += Maths::Pi2;
	}
	
	if ( m_lat < -Maths::PiBy2 || m_lat > Maths::PiBy2 )
	{
		m_upVector = -Maths::Const<Maths::Normal3>::AxisY();		
	}
	else
	{
		m_upVector = Maths::Const<Maths::Normal3>::AxisY();
	}
}
