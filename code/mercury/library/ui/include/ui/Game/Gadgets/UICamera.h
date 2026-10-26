#ifndef UICAMERA_H
#define UICAMERA_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Game/Gadgets/Buttons.h"

namespace UI
{

/// @addtogroup Camera
/// @{

enum E_CameraType
{
	E_CameraType_2D = 0,
	E_CameraType_3D_FromZero,
	E_CameraType_3D_LookAt,
	E_CameraType_3D_Fly,

	E_CameraType_Count,
};

#ifndef CFG_GOLD
class C_DebugCamUpdate2D
{
public:
	C_DebugCamUpdate2D()
		: lx( 0.0f ), ly( 0.0f ), lt( 0.0f ), rt( 0.0f )
	{}

public:
	F32 lx;
	F32 ly;
	F32 lt;
	F32 rt;
};

class C_DebugCamUpdate3D
{
public:
	C_DebugCamUpdate3D()
		: lx( 0.0f ), ly( 0.0f ), rx( 0.0f ), ry( 0.0f ),
		  lt( 0.0f ), rt( 0.0f ), lb( 0.0f ), rb( 0.0f )
	{}

public:
	F32 lx;
	F32 ly;
	F32 rx;
	F32 ry;
	F32 lt;
	F32 rt;
	F32 lb;
	F32 rb;
};
#endif


// --------------------
// C_UICameraSettings3D
// --------------------
class C_UICameraSettings3D
{
public:
	enum E_UpdateType
	{
		E_UpdateType_FromZero,
		E_UpdateType_LookAt,
		E_UpdateType_Fly,
	};

public:
	void				Init();						///< Intialise camera with default values
	void				Update();					///< update the view matrix

	void				EyePosition( const Maths::HVec3 eyePos )		{ m_Position = eyePos; }	///< @a
	Maths::HVec3&		EyePosition()									{ return m_Position; }		///< @a

	void				LookAt( const Maths::HVec3 lookAt )				{ m_LookAt = lookAt; }		///< @a
	Maths::HVec3&		LookAt()										{ return m_LookAtPrev; }	///< @a

	Maths::HVec3&		Right()											{ return m_RightPrev; }		///< @a
	Maths::HVec3&		Up()											{ return m_UpPrev; }		///< @a

	void				RotX( const F32 fRotX )							{ m_fRotX = fRotX; }		///< @a
	F32					RotX()											{ return m_fRotX; }			///< @a
	void				RotY( const F32 fRotY )							{ m_fRotY = fRotY; }		///< @a
	F32					RotY()											{ return m_fRotY; }			///< @a
	void				RotZ( const F32 fRotZ )							{ m_fRotZ = fRotZ; }		///< @a
	F32					RotZ()											{ return m_fRotZ; }			///< @a

	void				Scale( const F32 fScale )						{ m_fScale = fScale; UpdateScaleMatrix(); }		///< @a
	F32					Scale() const									{ return m_fScale; }			///< @a

	Maths::HMat44&		GetViewMatrix()									{ return m_ViewMatrix; }		///< @a
	Maths::HMat44&		GetProjMatrix()									{ return m_ProjMatrix; }		///< @a
	Maths::HMat44&		GetViewProjMatrix()								{ return m_ViewProjMatrix; }	///< @a

	void				UpdateType( E_UpdateType type )					{ m_eUpdateType = type; }	///< @a

private:
	void				Update_FromZero();
	void				Update_LookAt();

	void				UpdateScaleMatrix();
	void				UpdateViewMatrix();																						///< build view matrix from x, y and z axes
	void				UpdateViewMatrix( Maths::HVec3& pos, Maths::HVec3& right, Maths::HVec3& up, Maths::HVec3& lookAt ); 	///< build view matrix from x, y and z axes
	void				UpdateViewMatrix( Maths::HMat44& view );																///< build view matrix from matrix

private:
	E_UpdateType		m_eUpdateType;

	Maths::HMat44		m_ViewMatrix;
	Maths::HMat44		m_ProjMatrix;
	Maths::HMat44		m_ViewProjMatrix;
	Maths::HMat44		m_ScaleMatrix;

	Maths::HVec3		m_Position;
	Maths::HVec3		m_Right;
	Maths::HVec3		m_Up;
	Maths::HVec3		m_LookAt;

	Maths::HVec3		m_RightPrev;
	Maths::HVec3		m_UpPrev;
	Maths::HVec3		m_LookAtPrev;

	F32					m_fRotX;
	F32					m_fRotY;
	F32					m_fRotZ;

	F32					m_fScale;
};

// --------------------
// C_UICameraSettings2D
// --------------------
class C_UICameraSettings2D
{
public:
	void				Init();		///< set default values
	void				Update();	///< @a

	void				OffsetX( const F32 fOffsetX )	{ m_fOffsetX = fOffsetX; }		///< @a
	F32					OffsetX() const					{ return m_fOffsetX; }			///< @a
	void				OffsetY( const F32 fOffsetY )	{ m_fOffsetY = fOffsetY; }		///< @a
	F32					OffsetY() const					{ return m_fOffsetY; }			///< @a

	void				ScaleX( const F32 fScaleX )		{ m_fScaleX = fScaleX; }		///< @a
	F32					ScaleX() const					{ return m_fScaleX; }			///< @a
	void				ScaleY( const F32 fScaleY )		{ m_fScaleY = fScaleY; }		///< @a
	F32					ScaleY() const					{ return m_fScaleY; }			///< @a

	void				RotX( const F32 fRotX )			{ m_fRotX = fRotX; }			///< @a
	F32					RotX() const					{ return m_fRotX; }				///< @a
	void				RotY( const F32 fRotY )			{ m_fRotY = fRotY; }			///< @a
	F32					RotY() const					{ return m_fRotY; }				///< @a
	void				RotZ( const F32 fRotZ )			{ m_fRotZ = fRotZ; }			///< @a
	F32					RotZ() const					{ return m_fRotZ; }				///< @a

	Maths::HMat44&		GetTransformMatrix()			{ return m_TransformMatrix; }	///< @a

private:
	F32					m_fOffsetX;
	F32					m_fOffsetY;
	F32					m_fScaleX;
	F32					m_fScaleY;	
	F32					m_fRotX;
	F32					m_fRotY;
	F32					m_fRotZ;

	Maths::HMat44		m_TransformMatrix;
};

// ----------
// C_UICamera
// ----------
/// basic camera for the UI - has 3D and 2D settings and values
class C_UICamera
{
public:
	static C_UICamera&		Instance()					{ return m_sInstance; }	///< @a

public:
	void					Init();		///< Intialise camera with default values
	void					Update();	///< @a

#ifndef CFG_GOLD
	void					DebugUpdate2D( const C_DebugCamUpdate2D& input );	///< @a
	void					DebugUpdate3D( const C_DebugCamUpdate3D& input );	///< @a
#endif

	U32						GetNumSlots()							{ return m_uNumSlots; }

	C_UICameraSettings3D&	GetSettings3D( const S32 sSlot = 0 )	{ return m_Settings3D[ sSlot ]; }	///< @a
	C_UICameraSettings2D&	GetSettings2D( const S32 sSlot = 0 )	{ return m_Settings2D[ sSlot ]; }	///< @a

private:
	static C_UICamera		m_sInstance;

private:
	static const U32		m_uNumSlots = 2;

	Btl::Vector< C_UICameraSettings3D, Btl::VectorParameters_Array< m_uNumSlots > >		m_Settings3D;
	Btl::Vector< C_UICameraSettings2D, Btl::VectorParameters_Array< m_uNumSlots > >		m_Settings2D;
};

/// @}

// --------
// UICamera
// --------
class UICamera : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIButtonType_Camera, "CAMERA" )

public:
	void 	Init();
	void	Setup( lua_State* pState );
	void 	Render();

public:	// exposed for Xt
	E_CameraType		m_eType;
	S32					m_sSlot;

private:
	//
};

};	// namespace UI

#endif	// UICAMERA_H
