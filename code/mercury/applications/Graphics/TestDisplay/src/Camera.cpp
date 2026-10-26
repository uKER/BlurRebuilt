#include "StdAfx.h"
#include "Camera.h"

Debug::Float g_debugCameraMoveSpeed		("Camera.MoveSpeed",	10.0f,	Debug::Range(0.0f, 10.0f));
Debug::Float g_debugCameraMoveSlow		("Camera.MoveSlow",		0.5f,	Debug::Range(0.0f, 10.0f));
Debug::Float g_debugCameraMoveFast		("Camera.MoveFast",		5.0f,	Debug::Range(0.0f, 10.0f));
#if PLATFORM_PC
Debug::Float g_debugCameraLookSpeed		("Camera.LookSpeed",	3.0f,	Debug::Range(0.0f, 10.0f));
#else
Debug::Float g_debugCameraLookSpeed		("Camera.LookSpeed",	2.0f,	Debug::Range(0.0f, 10.0f));
#endif

C_Camera::C_Camera()
{
	m_AspectRatio	= 1.0f;
	m_FieldOfView	= 1.025f;
	m_NearClip		= 0.10f;
	m_FarClip		= 10000.0f;
	m_Pos			= Maths::HVec3(10.0f, 10.0f, 10.0f);
	m_Dir			= Maths::HVec3(-1.0f, -1.0f, -1.0f);
	m_Up			= Maths::HVec3(0.0f, 1.0f, 0.0f);

	m_Dir.Normalise();
}

Maths::HVec3 C_Camera::GetAt() const
{
	return m_Pos+m_Dir;
}

void C_Camera::Update(C_Input* pMoveInput, C_Input* pLookInput, const F32 elapsedTime)
{
	BIZ_ASSERT(pMoveInput);
	BIZ_ASSERT(pLookInput);

	F32 moveSpeed = g_debugCameraMoveSpeed;

	#if PLATFORM_PC
	moveSpeed *= pLookInput->GetValue(E_InputValue_CameraSpeed);
	#endif

	if(pMoveInput->GetValue(E_InputValue_CameraSlow) != 0.0f)
	{
		moveSpeed *= g_debugCameraMoveSlow;
	}

	if(pMoveInput->GetValue(E_InputValue_CameraFast) != 0.0f)
	{
		moveSpeed *= g_debugCameraMoveFast;
	}

	if(pMoveInput->GetValue(E_InputValue_CameraMoveX) != 0.0f)
	{
		const F32 inc = pMoveInput->GetValue(E_InputValue_CameraMoveX) * elapsedTime * moveSpeed;

		m_Pos += Maths::HVec3(m_Dir.Z() * inc, 0.0f, -m_Dir.X() * inc);
	}

	if(pMoveInput->GetValue(E_InputValue_CameraMoveY) != 0.0f)
	{
		const F32 inc = pMoveInput->GetValue(E_InputValue_CameraMoveY) * elapsedTime * moveSpeed;

		m_Pos += m_Dir * inc;
	}

	if(pMoveInput->GetValue(E_InputValue_CameraMoveZ) != 0.0f)
	{
		const F32 inc = pMoveInput->GetValue(E_InputValue_CameraMoveZ) * elapsedTime * moveSpeed;

		m_Pos += Maths::HVec3(0.0f, inc, 0.0f);
	}

	F32 lookSpeed = g_debugCameraLookSpeed;

	if(pLookInput->GetValue(E_InputValue_CameraLookX) != 0.0f)
	{
		const F32 inc = pLookInput->GetValue(E_InputValue_CameraLookX) * elapsedTime * lookSpeed;
		
		UpdateDir(m_Dir, -inc, m_Up);
	}

	if(pLookInput->GetValue(E_InputValue_CameraLookY) != 0.0f)
	{
		const F32 inc = pLookInput->GetValue(E_InputValue_CameraLookY) * elapsedTime * lookSpeed;
		
		Maths::HVec3 axis = m_Up^m_Dir;
		UpdateDir(m_Dir, -inc, axis);
	}
}

void C_Camera::UpdateDir(Maths::HVec3& dir, const F32 inc, const Maths::HVec3& axis)
{
	const F32 sinTheta = Maths::Sin(inc / 2.0f);
	const F32 cosTheta = Maths::Cos(inc / 2.0f);

	Maths::C_Quaternion q0(axis.X() * sinTheta, axis.Y() * sinTheta, axis.Z() * sinTheta, cosTheta);
	Maths::C_Quaternion q1(dir.X(), dir.Y(), dir.Z(), 0.0f);
	Maths::C_Quaternion q2 = q0*q1;
	Maths::C_Quaternion q3 = !q0;
	Maths::C_Quaternion q4 = q2*q3;
	
	q4.Normalise();

	if(q4.Imag().Y() >  0.95f)
		q4 = Maths::C_Quaternion(q4.Imag().X(),  0.95f, q4.Imag().Z(), 1.0f);
	else
	if(q4.Imag().Y() < -0.95f)
		q4 = Maths::C_Quaternion(q4.Imag().X(), -0.95f, q4.Imag().Z(), 1.0f);

	q4.Normalise();

	dir = Maths::HVec3(q4.Imag().X(), q4.Imag().Y(), q4.Imag().Z());
}
