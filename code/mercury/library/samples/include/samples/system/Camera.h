#ifndef SAMPLES_CAMERA_H
#define SAMPLES_CAMERA_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Samples
{

extern Debug::Float g_debugCameraSpeed;
extern Debug::Float g_debugCameraRotateSpeed;

enum E_CameraType
{
	E_CameraType_Fly = 0,
	E_CameraType_Orbit,
	E_CameraType_Chase,

	E_CameraType_Count
};

enum E_CameraAction
{
	E_CameraAction_Fw = 0,
	E_CameraAction_Bw,
	E_CameraAction_Lt,
	E_CameraAction_Rt,
	E_CameraAction_Up,
	E_CameraAction_Dn,

	E_CameraAction_Tx,
	E_CameraAction_Ty,
	E_CameraAction_Tz,

	E_CameraAction_Ax,
	E_CameraAction_Az,

	E_CameraAction_Count,
};

struct C_CameraAction
{
	bool	m_State;
	F32		m_Value;
};

class C_Camera
{
public:

	C_Camera();
	~C_Camera();

	void			EnableAction(const E_CameraAction action, const F32 value);
	void			DisableAction(const E_CameraAction action);

	void			Initialise();

	void			Update(const F32 frameTime);
	void			CycleType();
	void			SetType(const E_CameraType type);
	E_CameraType	GetType() const;

	void			Render() const;

	void			Reset();

	void			Load();
	void			Save();

private:

	void			UpdateDir(Maths::HVec3& dir, const F32 inc, const Maths::HVec3& axis);

public:

	F32				m_Speed;
	F32				m_RotateSpeed;
	F32				m_AspectRatio;
	F32				m_FieldOfView;
	F32				m_Distance;
	F32				m_Longitude;
	F32				m_Latitude;
	Maths::HVec3	m_Dir;
	Maths::HVec3	m_Eye;
	Maths::HVec3	m_At;
	Maths::HVec3	m_Up;
	Maths::HVec3	m_EyePrev;
	Maths::HVec3	m_AtPrev;
	Maths::HVec3	m_UpPrev;
	Maths::HVec3	m_Offset;

	Maths::HF32		m_ChaseDistance;

private:
	void			UpdateFly(const F32 frameTime);
	void			UpdateOrbit(const F32 frameTime);
	void			UpdateChase(const F32 frameTime);

	E_CameraType	m_Type;
	C_CameraAction	m_Actions[E_CameraAction_Count];
};

INLINE void C_Camera::SetType(const E_CameraType type)
{
	m_Type = type;
}

INLINE E_CameraType C_Camera::GetType() const
{
	return m_Type;
}

}; // namespace Samples

#endif // SAMPLES_CAMERA_H
