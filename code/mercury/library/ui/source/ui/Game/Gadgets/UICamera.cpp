#include "stdafx.h"

#include "ui/Game/Gadgets/UICamera.h"
#include "ui/Game/Gadgets/ButtonMessages.h"
#include "ui/Graphics/SimpleTimer.h"

#include "Display/View/View.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )	// CFG_ALLOW_OPTIMIZE_OFF_UI
#endif

#ifndef CFG_GOLD
Debug::Bool		g_UICameraReset( "UI.Camera.Reset", false );
Debug::Bool		g_UICameraDebugActive2D( "UI.Camera.DebugActive2d", false );
Debug::Float	g_UICameraSpeedOffset2D( "UI.Camera.SpeedOffset2D", 400.0f, Debug::Range( 0.0f, 1000.0f ) );
Debug::Float	g_UICameraSpeedScale2D( "UI.Camera.SpeedScale2D", 2.0f, Debug::Range( 0.0f, 10.0f ) );
Debug::Bool		g_UICameraDebugActive3D( "UI.Camera.DebugActive3d", false );
Debug::Float	g_UICameraSpeedRot3D( "UI.Camera.SpeedRot3D", 180.0f, Debug::Range( 0.0f, 360.0f ) );
Debug::Float	g_UICameraSpeedTrans3D( "UI.Camera.SpeedTrans3D", 100.0f, Debug::Range( 0.0f, 200.0f ) );
Debug::Float	g_UICameraSpeedSlow3D( "UI.Camera.SpeedSlow3D", 0.25f, Debug::Range( 0.0f, 10.0f ) );
Debug::Float	g_UICameraSpeedFast3D( "UI.Camera.SpeedFast3D", 2.0f, Debug::Range( 0.0f, 10.0f ) );
Debug::Float	g_UICameraSpeedSuperFast3D( "UI.Camera.SpeedSuperFast3D", 4.0f, Debug::Range( 0.0f, 10.0f ) );
Debug::Bool		g_UICameraInverted( "UI.Camera.InvertedY", false, Debug::Persist() );
#endif

// XT Definition
XT_TYPE( UI::E_CameraType, "UICameraType" )
	XT_OPTION( UI::E_CameraType_2D, "_2D" )
	XT_OPTION( UI::E_CameraType_3D_FromZero, "_3D" )
	XT_OPTION( UI::E_CameraType_3D_LookAt, "_3D_LookAt" )
	XT_OPTION( UI::E_CameraType_3D_Fly, "_3D_Fly" )
XT_END_TYPE

XT_TYPE( UI::UICamera, "UICamera" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_eType, "camera_type" )
	XT_FIELD( m_sSlot, "slot" )
XT_END_TYPE

namespace UI
{

// static instance 
C_UICamera	C_UICamera::m_sInstance;

// --------------------
// C_UICameraSettings3D
// --------------------
void C_UICameraSettings3D::Init()
{
	// view projection settings
	Display::C_View view;

	Display::C_ViewSetup viewSetup;

	F32 height = tanf( viewSetup.m_ViewSetup3D.m_FieldOfView * 0.5f ) * gSimple2D.GetClipWidth();
	F32 width = height * Display::Device()->GetAspectRatio(); //Bug #23939   //viewSetup.m_ViewSetup3D.m_AspectRatio;

	viewSetup.m_ViewSetup3D.m_ViewExtents.SetBounds( -width, -height, width, height );

	view.Setup( viewSetup, (U32)gSimple2D.GetClipWidth(), (U32)gSimple2D.GetClipHeight() );

	m_ProjMatrix = Maths::HMat44( Maths::Load< Maths::HMat44 >( &view.GetViewSettings3D().GetProjMatrix() ) );

	// default start settings
	m_Position	= Maths::HVec3( 0.0f, 0.0f, 0.0f );
	m_Right		= Maths::HVec3( 1.0f, 0.0f, 0.0f );
	m_Up		= Maths::HVec3( 0.0f, 1.0f, 0.0f );
	m_LookAt	= Maths::HVec3( 0.0f, 0.0f, 1.0f );

	// no rotation
	m_fRotX = 0.0f;
	m_fRotY = 0.0f;
	m_fRotZ = 0.0f;

	m_fScale = 1.0f;
	UpdateScaleMatrix();

	UpdateViewMatrix();
}

void C_UICameraSettings3D::UpdateViewMatrix( Maths::HVec3& pos, Maths::HVec3& right, Maths::HVec3& up, Maths::HVec3& lookAt )
{
	lookAt.Normalise();

	right = up ^ lookAt;
	right.Normalise();

	up = lookAt ^ right;
	up.Normalise();

	Maths::HMat43 view;

	view.AX( right );
	view.AY( up );
	view.AZ( lookAt );
	view.AW( pos );

	Maths::HMat44 v( view ); // Ps3
	UpdateViewMatrix( v );
}

void C_UICameraSettings3D::UpdateViewMatrix( Maths::HMat44& view )
{
	m_RightPrev			= Maths::HVec3( m_ViewMatrix.AX() );
	m_UpPrev			= Maths::HVec3( m_ViewMatrix.AY() );
	m_LookAtPrev		= Maths::HVec3( m_ViewMatrix.AZ() );

	m_ViewMatrix		= view * m_ScaleMatrix;
	m_ViewProjMatrix	= m_ViewMatrix.InverseRotTrans() * m_ProjMatrix;
}

void C_UICameraSettings3D::UpdateViewMatrix()
{
	UpdateViewMatrix( m_Position, m_Right, m_Up, m_LookAt );
}

void C_UICameraSettings3D::UpdateScaleMatrix()
{
	m_ScaleMatrix = Maths::HMat44( Maths::C_MatrixScale( Maths::HVec3( m_fScale, m_fScale, 1.0f ) ) );
}

void C_UICameraSettings3D::Update_FromZero()
{
	// calculate new camera axes
	Maths::HVec3 right = m_Right;
	Maths::HVec3 up = m_Up;
	Maths::HVec3 lookAt = m_LookAt;

	// rot y
	Maths::C_Quaternion rotY( up, m_fRotY );
	right = right * rotY;
	lookAt = lookAt * rotY;

	// rot x
	Maths::C_Quaternion rotX( right, m_fRotX );
	up = up * rotX;
	lookAt = lookAt * rotX;

	// rot z
	Maths::C_Quaternion rotZ( lookAt, m_fRotZ );
	up = up * rotZ;
	right = right * rotZ;

	UpdateViewMatrix( m_Position, right, up, lookAt );
}

void C_UICameraSettings3D::Update_LookAt()
{
	// calculate new camera axes
	Maths::HVec3 right = m_Right;
	Maths::HVec3 up = m_Up;
	Maths::HVec3 lookAt( m_LookAt - Maths::HVec3(m_Position) );

	UpdateViewMatrix( m_Position, right, up, lookAt );
}


void C_UICameraSettings3D::Update()
{
#ifndef CFG_GOLD
	if( g_UICameraDebugActive3D )
	{
		Update_FromZero();
		return;
	}
#endif

	switch( m_eUpdateType )
	{
	case E_UpdateType_FromZero:	// Translate from p(0,0,0), rotate lookout
		Update_FromZero();
		break;

	case E_UpdateType_LookAt:	// Translate from p(0) then LookAt
		Update_LookAt();
		break;

	case E_UpdateType_Fly:		// Transform only, no lookat
		UpdateViewMatrix( m_ViewMatrix );
		break;
	}
}

// --------------------
// C_UICameraSettings2D
// --------------------
void C_UICameraSettings2D::Init()
{
	m_fOffsetX = m_fOffsetY = 0.0f;
	m_fScaleX = m_fScaleY = 1.0f;
	m_fRotX = m_fRotY = m_fRotZ = 0.0f;
}

void C_UICameraSettings2D::Update()
{
	// calculate new transform matrix
	Maths::HMat43 mat;
	mat.SetIdentity();

	// rotate about the middle
	Maths::HVec3 rotPivot( 320.0f - m_fOffsetX, 240.0f - m_fOffsetY, 0.0f );
	mat.Translation( mat.Translation() - rotPivot );
	mat = mat * Maths::HMat43( Maths::C_MatrixRotateRPH( m_fRotZ, m_fRotX, m_fRotY ) );
	mat.Translation( mat.Translation() + rotPivot );

	// scale about the middle
	Maths::HVec3 scalePivot( 320.0f - m_fOffsetX, 240.0f - m_fOffsetY, 0.0f );
	mat.Translation( mat.Translation() - scalePivot );
	mat = mat * Maths::HMat43( Maths::C_MatrixScale( Maths::HVec3( m_fScaleX, m_fScaleY, 1.0f ) ) );
	mat.Translation( mat.Translation() + scalePivot );

	// add on translation
	mat.Translation( mat.Translation() + Maths::HVec3( m_fOffsetX, m_fOffsetY, 0.0f ) );

	Maths::HMat44 transform( mat );
	m_TransformMatrix = transform * gSimple2D.GetTransformMatrix2D();
}

// ----------
// C_UICamera
// ----------
void C_UICamera::Init()
{
	for ( U32 i = 0; i < m_uNumSlots; i++ )
	{
		m_Settings3D[ i ].Init();
		m_Settings2D[ i ].Init();
	}
}

void C_UICamera::Update()
{
	for ( U32 i = 0; i < m_uNumSlots; i++ )
	{
		m_Settings3D[ i ].Update();
		m_Settings2D[ i ].Update();
	}

#ifndef CFG_GOLD
	if ( g_UICameraReset )
	{
		for ( U32 i = 0; i < m_uNumSlots; i++ )
		{
			m_Settings3D[ i ].Init();
			m_Settings2D[ i ].Init();
		}

		g_UICameraReset = false;
	}
#endif
}

#ifndef CFG_GOLD
void C_UICamera::DebugUpdate2D( const C_DebugCamUpdate2D& input )
{
	for ( U32 i = 0; i < m_uNumSlots; i++ )
	{
		C_UICameraSettings2D& settings2D = m_Settings2D[ i ];

		// offset
		F32 fOffsetAmount = g_UICameraSpeedOffset2D * theFETimer.FrameTime();

		settings2D.OffsetX( settings2D.OffsetX() + input.lx * -fOffsetAmount );
		settings2D.OffsetY( settings2D.OffsetY() + input.ly * -fOffsetAmount );

		// scale
		F32 fScaleAmount = g_UICameraSpeedScale2D * theFETimer.FrameTime();

		F32 fScale = settings2D.ScaleX();

		if ( input.lt )
			fScale -= fScaleAmount;
		if ( input.rt )
			fScale += fScaleAmount;

		settings2D.ScaleX( ( fScale < 0.0f ) ? 0.0f : fScale );
		settings2D.ScaleY( settings2D.ScaleX() );
	}
}

void C_UICamera::DebugUpdate3D( const C_DebugCamUpdate3D& input )
{
	for ( U32 i = 0; i < m_uNumSlots; i++ )
	{
		C_UICameraSettings3D& settings3D = m_Settings3D[ i ];

		// offset
		F32 fOffsetAmount = g_UICameraSpeedTrans3D * theFETimer.FrameTime();

		F32 fScale = 1.0f;

		if ( input.lt && input.rt )
			fScale = g_UICameraSpeedSuperFast3D;
		else if ( input.lt )
			fScale = g_UICameraSpeedSlow3D;
		else if ( input.rt )
			fScale = g_UICameraSpeedFast3D;

		Maths::HVec3& eyePos = settings3D.EyePosition();
		eyePos += input.lx * fOffsetAmount * fScale * settings3D.Right();
		eyePos += input.ly * fOffsetAmount * fScale * settings3D.LookAt();
		eyePos += ( -input.lb + input.rb ) * fOffsetAmount * fScale * Maths::HVec3( 0.0f, 1.0f, 0.0f );

		// rotation
		F32 fRotAmount = Maths::DegToRad( g_UICameraSpeedRot3D * theFETimer.FrameTime() );

		if ( g_UICameraInverted )
			settings3D.RotX( settings3D.RotX() - input.ry * fRotAmount );
		else
			settings3D.RotX( settings3D.RotX() + input.ry * fRotAmount );

		settings3D.RotY( settings3D.RotY() - input.rx * fRotAmount );
	}
}
#endif

// --------
// UICamera
// --------
void UICamera::Init()
{
	m_eType = E_CameraType_2D;
	m_sSlot = 0;

	C_GadgetBase::Init();
}

void UICamera::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	if ( ( m_sSlot < 0 ) || ( (U32)m_sSlot >= C_UICamera::Instance().GetNumSlots() ) )
		m_sSlot = 0;

	// set base gadget settings from global instance of the camera
	if ( m_eType == E_CameraType_2D )
	{
		C_UICameraSettings2D& settings2D = C_UICamera::Instance().GetSettings2D( m_sSlot );

		m_Position.x = settings2D.OffsetX();
		m_Position.y = settings2D.OffsetY();

		m_Scale.x = settings2D.ScaleX();
		m_Scale.y = settings2D.ScaleY();

		m_Rotation.x = Maths::RadToDeg( settings2D.RotX() );
		m_Rotation.y = Maths::RadToDeg( settings2D.RotY() );
		m_Rotation.z = Maths::RadToDeg( settings2D.RotZ() );
	}
	else if ( m_eType == E_CameraType_3D_FromZero )
	{
		C_UICameraSettings3D& settings3D = C_UICamera::Instance().GetSettings3D( m_sSlot );

		Maths::HVec3& eyePos = settings3D.EyePosition();
		m_Position.x = eyePos.X();
		m_Position.y = eyePos.Y();
		m_Position.z = eyePos.Z();

		m_Rotation.x = Maths::RadToDeg( settings3D.RotX() );
		m_Rotation.y = Maths::RadToDeg( settings3D.RotY() );
		m_Rotation.z = Maths::RadToDeg( settings3D.RotZ() );
	}
}

void UICamera::Render()
{
	if ( !m_Activated )
		return;

	// update the global instance of the UI camera with our variables
#ifndef CFG_GOLD
	if ( !g_UICameraDebugActive2D )
#endif
	{
		if ( m_eType == E_CameraType_2D )
		{
			C_UICameraSettings2D& settings2D = C_UICamera::Instance().GetSettings2D( m_sSlot );

			settings2D.OffsetX( m_Position.x );
			settings2D.OffsetY( m_Position.y );

			settings2D.ScaleX( m_Scale.x );
			settings2D.ScaleY( m_Scale.y );

			settings2D.RotX( Maths::DegToRad( m_Rotation.x ) );
			settings2D.RotY( Maths::DegToRad( m_Rotation.y ) );
			settings2D.RotZ( Maths::DegToRad( m_Rotation.z ) );
		}
	}
#ifndef CFG_GOLD
	if ( !g_UICameraDebugActive3D )
#endif
	{
		C_UICameraSettings3D& settings3D = C_UICamera::Instance().GetSettings3D( m_sSlot );
		if ( m_eType == E_CameraType_3D_FromZero ) // GeordieCam (tm)
		{
			settings3D.EyePosition( m_Position );
			settings3D.RotX( Maths::DegToRad( m_Rotation.x ) );
			settings3D.RotY( Maths::DegToRad( m_Rotation.y ) );
			settings3D.RotZ( Maths::DegToRad( m_Rotation.z ) );
			settings3D.UpdateType( C_UICameraSettings3D::E_UpdateType_FromZero );
		}
		else
		if( m_eType == E_CameraType_3D_Fly )
		{
			TransformIndex ti;
			CalcTransformHierarchy( ti );

			Maths::HMat43 mat = Matrix();

			if( ti == 0 )
			{
				mat.Translation( m_Position );
				SetMatrix(mat);
			}

			settings3D.EyePosition( mat.AW() );

#ifndef CFG_GOLD
			// Not the correct decomposition but better than nothing for debug
			settings3D.RotX( Maths::DegToRad( m_Rotation.x ) );
			settings3D.RotY( Maths::DegToRad( m_Rotation.y ) );
			settings3D.RotZ( Maths::DegToRad( m_Rotation.z ) );
#endif
			settings3D.GetViewMatrix() = mat;
			settings3D.UpdateType( C_UICameraSettings3D::E_UpdateType_Fly );
		}
		else
		if( m_eType == E_CameraType_3D_LookAt )
		{
			settings3D.EyePosition( m_Position );
			settings3D.LookAt( m_Size ); // Meh!
			settings3D.UpdateType( C_UICameraSettings3D::E_UpdateType_LookAt );
		}
	}

	C_GadgetBase::FrameUpdate();
}

};	// namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", on )	// CFG_ALLOW_OPTIMIZE_OFF_UI
#endif
