#ifndef CAMERA_H
#define CAMERA_H

#include <core/maths/hvec/new/IncludeMinimal.h>

namespace Display
{
	class C_ViewSetup;
}

enum E_CameraType
{
	E_CameraType_Orbit	= 0,
	E_CameraType_LookAt
};
	

enum E_CameraInput
{
	E_CameraInput_MoveLR	= 0,
	E_CameraInput_MoveUD,
	
	E_CameraInput_Zoom,
	
	E_CameraInput_RotateLR,
	E_CameraInput_RotateUD,
	
	E_CameraInput_Count
};

class C_Camera
{
public:
	C_Camera();
	
	void SetType( E_CameraType type )	{ m_type = type; }

	Maths::Point3	GetEyePos() const;

	Maths::Mat43 GetWorldMatrix() const;
	void SetWorldMatrix( const Maths::Mat43& worldMatrix );
	
	void SetDistance( F32 d )				{ m_distance = d; }
	
	void SetTargetPos( const Maths::Point3& targetPos )	{ m_targetPos = targetPos;	}
	Maths::Point3 GetTarget() const							{ return m_targetPos + m_targetOffset; }
	void ClearTargetOffset();
	void SetTargetOffset(const Maths::Vector3& offset)		{ m_targetOffset = offset; }
	void SetEyePos( const Maths::Point3& p )				{ m_eyePos = p; }
	void SetUpVector( const Maths::Normal3& up )			{ m_upVector = up; }
	
	void SetAspectRatio( F32 aspect )		{ m_aspectRatio = aspect; }
	void SetFieldOfView( F32 fov )			{ m_fieldOfView = fov; }
	
	void DisableAllInputs()									{ for ( U32 i = 0; i < E_CameraInput_Count; ++i ) m_inputActive[i] = false; }
	
	void EnableInput( E_CameraInput i, F32 value = 0.f )	{ m_inputActive[i] = true; m_inputValue[i] = value; }
	void DisableInput( E_CameraInput i)						{ m_inputActive[i] = false; }
	
	void SetupView( Display::C_ViewSetup& viewSetup ) const;
	
	void Update( F32 deltaTime );

private:

	void UpdateShared( F32 deltaTime );
	void UpdateOrbit( F32 deltaTime );

	// Parameters.
	Maths::Point3	m_eyePos;
	Maths::Point3	m_targetPos;
	Maths::Normal3	m_upVector;
	F32				m_aspectRatio;
	F32				m_fieldOfView;
	F32				m_rotateSpeed;
	F32				m_zoomSpeed;
	F32				m_moveSpeed;
		
	// State.
	bool			m_inputActive[ E_CameraInput_Count ];
	F32				m_inputValue[ E_CameraInput_Count ];
	
	Maths::Vector3	m_targetOffset;
	
	F32				m_distance;
	F32				m_long;			// 0 - 360.
	F32				m_lat;			// -90 (looking up) to +90 (looking down)
	
	E_CameraType	m_type;
	
};


#endif
