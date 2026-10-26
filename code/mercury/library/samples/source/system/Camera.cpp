#include "StdAfx.h"
#include "Samples/System/Camera.h"

// debug rendering
#include <System/Debug/I_DebugRender.h>

namespace Samples
{

Debug::Float g_debugCameraSpeed("Camera.Speed", 10.f, Debug::Range(0.f, 50.f));
Debug::Float g_debugCameraRotateSpeed("Camera.RotateSpeed", 10.f, Debug::Range(0.f, 50.f));
Debug::Float g_debugCameraFieldOfView("Camera.FieldOfView", 1.025f, Debug::Range(0.f, 2.f));

C_Camera::C_Camera() :
	m_Speed(1.f),
	m_RotateSpeed(.25f),
	m_AspectRatio(1.0f),
	m_FieldOfView(1.025f),
	m_Distance(0.0f),
	m_Longitude(Maths::Pi / 4.0f),
	m_Latitude(Maths::Pi / 4.0f),
	m_Type(E_CameraType_Fly)
{
	m_Eye	= Maths::HVec3(-10.f, 5.0f, -10.0f);
	m_At	= Maths::HVec3(0.0f, 0.0f, 0.0f);
	m_Up	= Maths::Constants<Maths::HVec3>::UnitAxisPositiveY();

	memset(m_Actions, 0, sizeof(C_CameraAction) * E_CameraAction_Count);
}

C_Camera::~C_Camera()
{
}

void C_Camera::Initialise()
{
	m_AspectRatio = Display::Device()->GetWideScreen() ? 16.0f / 9.0f : 4.0f / 3.0f;

	Load();
}

void C_Camera::EnableAction(const E_CameraAction action, const F32 value)
{
	assert(action < E_CameraAction_Count);
	m_Actions[action].m_State = true;
	m_Actions[action].m_Value = value;
}

void C_Camera::DisableAction(const E_CameraAction action)
{
	assert(action < E_CameraAction_Count);
	m_Actions[action].m_State = false;
}

void C_Camera::Update(const F32 frameTime)
{
	m_FieldOfView = g_debugCameraFieldOfView;
	
	m_Dir = m_At - m_Eye;
	//m_Distance = Maths::Length(m_Dir);
	//m_Dir.Normalise();

	switch(m_Type)
	{
		case E_CameraType_Fly:
			UpdateFly(frameTime);
			break;
		case E_CameraType_Orbit:
			UpdateOrbit(frameTime);
			break;
		case E_CameraType_Chase:
			UpdateChase(frameTime);
			break;
	}

	m_EyePrev = m_Eye;
	m_AtPrev = m_At;
	m_UpPrev = m_Up;
}

void C_Camera::UpdateFly(const F32 frameTime)
{
	if (m_Actions[E_CameraAction_Fw].m_State)
	{
		const F32 inc = m_Actions[E_CameraAction_Fw].m_Value * frameTime * m_Speed * g_debugCameraSpeed;
		m_Eye += m_Dir * inc;
	}

	if (m_Actions[E_CameraAction_Bw].m_State)
	{
		const F32 inc = m_Actions[E_CameraAction_Bw].m_Value * frameTime * m_Speed * g_debugCameraSpeed;
		m_Eye -= m_Dir * inc;
	}

	if (m_Actions[E_CameraAction_Lt].m_State)
	{
		const F32 inc = m_Actions[E_CameraAction_Lt].m_Value * frameTime * m_Speed * g_debugCameraSpeed;
		m_Eye += Maths::HVec3(-m_Dir.Z() * inc, 0.f, m_Dir.X() * inc);
	}

	if (m_Actions[E_CameraAction_Rt].m_State)
	{
		const F32 inc = m_Actions[E_CameraAction_Rt].m_Value * frameTime * m_Speed * g_debugCameraSpeed;
		m_Eye += Maths::HVec3(m_Dir.Z() * inc, 0.f, -m_Dir.X() * inc);
	}

	if (m_Actions[E_CameraAction_Up].m_State)
	{
		const F32 inc = m_Actions[E_CameraAction_Up].m_Value * frameTime * m_Speed * g_debugCameraSpeed;
		m_Eye += Maths::HVec3(0.f, inc, 0.f);
	}

	if (m_Actions[E_CameraAction_Dn].m_State)
	{
		const F32 inc = m_Actions[E_CameraAction_Dn].m_Value * frameTime * m_Speed * g_debugCameraSpeed;
		m_Eye -= Maths::HVec3(0.f, inc, 0.f);
	}


	// TRANSLATION AXES //

	if (m_Actions[E_CameraAction_Tx].m_State)
	{
		const F32 inc = m_Actions[E_CameraAction_Tx].m_Value * frameTime * m_Speed * g_debugCameraSpeed;
		m_Eye += Maths::HVec3(m_Dir.Z() * inc, 0.f, -m_Dir.X() * inc);
	}

	if (m_Actions[E_CameraAction_Ty].m_State)
	{
		const F32 inc = m_Actions[E_CameraAction_Ty].m_Value * frameTime * m_Speed * g_debugCameraSpeed;
		m_Eye += Maths::HVec3(0.f, inc, 0.f);
	}

	if (m_Actions[E_CameraAction_Tz].m_State)
	{
		const F32 inc = m_Actions[E_CameraAction_Tz].m_Value * frameTime * m_Speed * g_debugCameraSpeed;
		m_Eye += m_Dir * inc;
	}


	// ROTATION AXES //

	if (m_Actions[E_CameraAction_Ax].m_State)
	{
		const F32 inc = m_Actions[E_CameraAction_Ax].m_Value * frameTime * m_RotateSpeed * g_debugCameraRotateSpeed;
		UpdateDir(m_Dir, inc, m_Up);
	}

	if (m_Actions[E_CameraAction_Az].m_State)
	{
		const F32 inc = m_Actions[E_CameraAction_Az].m_Value * frameTime * m_RotateSpeed * g_debugCameraRotateSpeed;
		UpdateDir(m_Dir, inc, m_Up ^ m_Dir);
	}

	
	m_At = m_Eye + m_Dir;
	m_Distance = Maths::Length(m_At - m_Eye);
}

void C_Camera::UpdateChase(const F32 frameTime)
{
	Maths::HVec3 vForward = Maths::Normalise(m_Eye - m_At);
	Maths::HVec3 vRight = Maths::Normalise(vForward ^ m_Up);
	Maths::HVec3 vUp = Maths::Normalise(vRight ^ vForward);

	/*if (m_Actions[E_CameraAction_Tx].m_State)
	{
		m_At += m_Actions[E_CameraAction_Tx].m_Value * vRight * frameTime * m_Speed * g_debugCameraSpeed;
	}

	if (m_Actions[E_CameraAction_Ty].m_State)
	{
		m_At += m_Actions[E_CameraAction_Ty].m_Value * vUp * frameTime * m_Speed * g_debugCameraSpeed;
	}

	if (m_Actions[E_CameraAction_Tz].m_State)
	{
		m_Distance += m_Actions[E_CameraAction_Tz].m_Value * frameTime * m_Speed * g_debugCameraSpeed;

		if(m_Distance < 0.1f)
			m_Distance = 0.1f;
	}*/

	if (m_Actions[E_CameraAction_Ax].m_State)
	{
		m_Longitude += m_Actions[E_CameraAction_Ax].m_Value * frameTime * m_RotateSpeed * g_debugCameraSpeed;
	}

	if (m_Actions[E_CameraAction_Az].m_State)
	{
		m_Latitude += m_Actions[E_CameraAction_Az].m_Value * frameTime * m_RotateSpeed * g_debugCameraSpeed;
		
		if(m_Latitude > (Maths::Pi + Maths::Pi/2.0f))
			m_Latitude -= Maths::Pi * 2.0f;
		else if(m_Latitude < -(Maths::Pi + Maths::Pi/2.0f))
			m_Latitude += Maths::Pi * 2.0f;
	}

	if(m_Latitude < -Maths::Pi / 3.0f)
		m_Latitude = -Maths::Pi/3.f;
	if(m_Latitude > Maths::Pi / 3.0f)
		m_Latitude = Maths::Pi/3.f;

	m_Eye.X(m_Distance * cosf(m_Latitude) * cosf(m_Longitude) + m_At.X());
	m_Eye.Y(m_Distance * sinf(m_Latitude) + m_At.Y());
	m_Eye.Z(m_Distance * cosf(m_Latitude) * sinf(m_Longitude) + m_At.Z());

	Maths::HVec3 m_Look(m_At - m_Eye);
	m_Look.Normalise();

	m_Eye = m_At + m_Look * m_ChaseDistance;
	
}


void C_Camera::UpdateOrbit(const F32 frameTime)
{
	//Maths::HVec3 target = m_At + m_Offset;

	Maths::HVec3 vForward = Maths::Normalise(m_Eye - (m_AtPrev + m_Offset));
	Maths::HVec3 vRight = Maths::Normalise(vForward ^ m_Up);
	Maths::HVec3 vUp = Maths::Normalise(vRight ^ vForward);
	
	//m_Offset = Maths::HVec3();

	if (m_Actions[E_CameraAction_Tx].m_State)
	{
		m_Offset += m_Actions[E_CameraAction_Tx].m_Value * vRight * frameTime * m_Speed * g_debugCameraSpeed;
	}

	if (m_Actions[E_CameraAction_Ty].m_State)
	{
		m_Offset += m_Actions[E_CameraAction_Ty].m_Value * vUp * frameTime * m_Speed * g_debugCameraSpeed;
	}

	if (m_Actions[E_CameraAction_Tz].m_State)
	{
		m_Distance += m_Actions[E_CameraAction_Tz].m_Value * frameTime * m_Speed * g_debugCameraSpeed;

		if(m_Distance < 0.1f)
			m_Distance = 0.1f;
	}

	if (m_Actions[E_CameraAction_Ax].m_State)
	{
		m_Longitude += m_Actions[E_CameraAction_Ax].m_Value * frameTime * m_RotateSpeed * g_debugCameraSpeed;
	}

	if (m_Actions[E_CameraAction_Az].m_State)
	{
		m_Latitude += m_Actions[E_CameraAction_Az].m_Value * frameTime * m_RotateSpeed * g_debugCameraSpeed;
		
		if(m_Latitude > (Maths::Pi + Maths::Pi/2.0f))
			m_Latitude -= Maths::Pi * 2.0f;
		else if(m_Latitude < -(Maths::Pi + Maths::Pi/2.0f))
			m_Latitude += Maths::Pi * 2.0f;
	}

	if(m_Latitude < -Maths::Pi / 2.0f || m_Latitude > Maths::Pi / 2.0f)
		m_Up.Y(-1.0f);
	else
		m_Up.Y(1.0f);

	m_Eye.X(m_Distance * cosf(m_Latitude) * cosf(m_Longitude) + m_At.X());
	m_Eye.Y(m_Distance * sinf(m_Latitude) + m_At.Y());
	m_Eye.Z(m_Distance * cosf(m_Latitude) * sinf(m_Longitude) + m_At.Z());

	m_Eye -= m_Offset;
}

void C_Camera::UpdateDir(Maths::HVec3& dir, const F32 inc, const Maths::HVec3& axis)
{
	const F32 sinTheta = sin(inc / 2.f);
	const F32 cosTheta = cos(inc / 2.f);

	const Maths::C_Quaternion q0(axis.X() * sinTheta, axis.Y() * sinTheta, axis.Z() * sinTheta, cosTheta);
	const Maths::C_Quaternion q1(dir.X(), dir.Y(), dir.Z(), 0.f);
	const Maths::C_Quaternion q2 = q0 * q1 * !q0;

	const Maths::HVec4 q3 = q2;
	
	dir = Maths::HVec3(q3.X(), q3.Y(), q3.Z());
	dir.Normalise();
	if (dir.Y() >  0.95f) dir = Maths::HVec3(dir.X(),  0.95f, dir.Z());
	if (dir.Y() < -0.95f) dir = Maths::HVec3(dir.X(), -0.95f, dir.Z());
	dir.Normalise();
}

void C_Camera::Reset()
{
	memset(m_Actions, 0, sizeof(C_CameraAction) * E_CameraAction_Count);
}

void C_Camera::CycleType()
{
	S32 m_Current = m_Type + 1;

	if(m_Current == E_CameraType_Count)
		m_Current = 0;

	m_Type = (E_CameraType)m_Current;

	switch(m_Type)
	{
		case E_CameraType_Fly:
			break;
		case E_CameraType_Orbit:
			m_At = Maths::HVec3(0.0f, 0.0f, 0.0f);
			break;
		/*case E_CameraType_Chase:
			m_At = Maths::HVec3(0.f, 0.f, 0.f);*/
	}
}

// render any debug graphics per camera
void C_Camera::Render() const
{
	if(!DbUtil::DefaultDebugRenderSys())
		return;

	if(m_Type == E_CameraType_Orbit)
	{
		Maths::HMat43 matrix;
		matrix.Translation(m_At);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix(matrix, 2.0f);
	}
}


void C_Camera::Load()
{
	Util::C_String fileName = Display::FormatString(Sys::C_FileSystem::Instance().GetWorkingDirectory() + "/camera");

	#if defined(PLATFORM_PC)
	fileName += ".pcb";
	#elif defined(PLATFORM_PS3)
	fileName += ".p3b";
	#endif

	FILE* pFile = NULL;
	#ifdef PLATFORM_PS3
	if ((pFile = fopen(fileName.Chars(), "rb+")))
	#else
	if (fopen_s(&pFile, fileName.Chars(), "rb+") == 0)
	#endif
	{
		F32 eyex = 0.f;
		F32 eyey = 0.f;
		F32 eyez = 0.f;
		fread(&eyex, sizeof(F32), 1, pFile);
		fread(&eyey, sizeof(F32), 1, pFile);
		fread(&eyez, sizeof(F32), 1, pFile);
		m_Eye = Maths::HVec3(eyex, eyey, eyez);

		F32 atx = 0.f;
		F32 aty = 0.f;
		F32 atz = 0.f;
		fread(&atx, sizeof(F32), 1, pFile);
		fread(&aty, sizeof(F32), 1, pFile);
		fread(&atz, sizeof(F32), 1, pFile);
		m_At = Maths::HVec3(atx, aty, atz);

		fread(&m_Speed, sizeof(F32), 1, pFile);

		fclose(pFile);
	}
}

void C_Camera::Save()
{
	Util::C_String fileName = Display::FormatString(Sys::C_FileSystem::Instance().GetWorkingDirectory() + "/camera");

	#if defined(PLATFORM_PC)
	fileName += ".pcb";
	#elif defined(PLATFORM_PS3)
	fileName += ".p3b";
	#endif

	FILE* pFile = NULL;
	#ifdef PLATFORM_PS3
	if ((pFile = fopen(fileName.Chars(), "wb+")))
	#else
	if (fopen_s(&pFile, fileName.Chars(), "wb+") == 0)
	#endif
	{
		F32 eyex = m_Eye.X();
		F32 eyey = m_Eye.Y();
		F32 eyez = m_Eye.Z();
		fwrite(&eyex, sizeof(F32), 1, pFile);
		fwrite(&eyey, sizeof(F32), 1, pFile);
		fwrite(&eyez, sizeof(F32), 1, pFile);

		F32 atx = m_At.X();
		F32 aty = m_At.Y();
		F32 atz = m_At.Z();
		fwrite(&atx, sizeof(F32), 1, pFile);
		fwrite(&aty, sizeof(F32), 1, pFile);
		fwrite(&atz, sizeof(F32), 1, pFile);

		fwrite(&m_Speed, sizeof(F32), 1, pFile);

		fclose(pFile);
	}
}

}; // namespace Samples
