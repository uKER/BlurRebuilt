
#include "stdafx.h"
#include <camera2/debug/DebugTools.h>

#include <camera2/network/NetworkLink.h>
#include <camera2/stock/AnimCamera.h>
#include <camera2/stock/TvFeed.h>
#include <camera2/debug/PositionLogger.h>
#include <camera2/capture/CaptureService.h>

// C_DebugTools class - this is a helper class which has camera network link, Set/Get Position...

namespace Cam2
{

using namespace Maths;

static bool GetPositionNow = false;
static void GetPositionFunc(void *) { GetPositionNow = true; }
static bool SetPositionNow = false;
static void SetPositionFunc(void *) { SetPositionNow = true; }
static Debug::Int DetailLevel("Camera2.DebugTools.DetailLevel", 4, Debug::Range(4, 7));
static Debug::String PositionString("Camera2.DebugTools.PositionString", "", Debug::Persist());
static Debug::Function GetPosition("Camera2.DebugTools.GetPosition", GetPositionFunc, 0);
static Debug::Function SetPosition("Camera2.DebugTools.SetPosition", SetPositionFunc, 0);

static bool PosLogResetNow = false;
static void PosLogResetFunc(void *) { PosLogResetNow = true; }
static Debug::Function PosLogReset("Camera2.DebugTools.PositionLog.Reset", PosLogResetFunc, 0);

static bool ExportAffectEffectsNow = false;
static void ExportAffectEffectsFunc(void *) { ExportAffectEffectsNow = true; }
static Debug::Function ExportAffectEffects("Camera2.DebugTools.PositionLog.ExportAffectEffects", ExportAffectEffectsFunc, 0);

static bool ExportMayaNow = false;
static void ExportMayaFunc(void *) { ExportMayaNow = true; }
static Debug::Function ExportMaya("Camera2.DebugTools.PositionLog.ExportMaya", ExportMayaFunc, 0);

static Debug::Float MapX("Camera2.DebugTools.MapX", 0.f, Debug::Range(-2000.f, 2000.f));
static Debug::Float MapY("Camera2.DebugTools.MapY", 1000.f, Debug::Range(0.f, 2000.f));
static Debug::Float MapZ("Camera2.DebugTools.MapZ", 0.f, Debug::Range(-2000.f, 2000.f));
static Debug::Float MapSpan("Camera2.DebugTools.MapSpan", 1000.f, Debug::Range(1.f, 2000.f));
static Debug::Bool MapWizard("Camera2.DebugTools.MapWizard", false);
static Debug::String MapString("Camera2.DebugTools.MapString", "");

C_DebugTools::C_DebugTools()
{
	m_pNetworkLink = 0;
	m_pAnimCamera = new C_AnimCamera;
	m_PosLogger = 0;
	m_TvFeed = 0;
}

C_DebugTools::~C_DebugTools()
{
	Exit();
}

bool C_DebugTools::Init()
{
#ifndef CFG_GOLD
	m_pNetworkLink = &C_NetworkLink::Instance();
	m_pNetworkLink->Init();

	m_PosLogger = new C_PositionLogger;
	m_TvFeed = new C_TvFeed;
	return true;
#else
	return false;
#endif
}

void C_DebugTools::Exit()
{
#ifndef CFG_GOLD
	SAFE_DELETE_PTR(m_TvFeed)
	SAFE_DELETE_PTR(m_pAnimCamera)

	if (m_pNetworkLink)
		m_pNetworkLink->Exit();
	m_pNetworkLink = 0;

	SAFE_DELETE_PTR(m_PosLogger)
#endif
}

void C_DebugTools::Update(F32 period)
{
	(void)period;
#ifndef CFG_GOLD
	assert(m_pNetworkLink);

	m_pNetworkLink->Update();
	m_RemoteCamera = m_pNetworkLink->RemoteCamera();
	m_Notify = m_pNetworkLink->Notify();

	// Check for exporting TvScene
	if (m_Notify == E_NetNotify_Scene)
		m_TvFeed->LoadTvScene("Cams.cse");

	m_TvFeed->Update(period);

	// Check for exporting an animation:
	if (m_pAnimCamera)
	{
		if (m_Notify == E_NetNotify_Animation)
			m_pAnimCamera->SetAnimData(m_pNetworkLink->AnimData());
	
		m_pAnimCamera->Update(period);
	}

	if (GetPositionNow)
	{
		GetCameraPosition();
		GetPositionNow = false;
	}
	
	if (SetPositionNow)
	{
		SetCameraPosition();
		SetPositionNow = false;
	}

	if (PosLogResetNow)
	{
		if (m_PosLogger)
			m_PosLogger->Reset();
		PosLogResetNow = false;
	}

	if (ExportAffectEffectsNow)
	{
		if (m_PosLogger)
			m_PosLogger->WriteAfterEffects();
		ExportAffectEffectsNow = false;
	}

	if (ExportMayaNow)
	{
		if (m_PosLogger)
			m_PosLogger->WriteMaya();
		ExportMayaNow = false;
	}
	
	if (MapWizard)
		DoMapWizard();
	
#endif
}

void C_DebugTools::SetObject(const C_CamObj& object)
{
	(void)object;
#ifndef CFG_GOLD
	if (m_TvFeed)
		m_TvFeed->SetObject(object);
	if (m_pAnimCamera)
		m_pAnimCamera->SetObject(object);
#endif
}

void C_DebugTools::DebugRender()
{
#ifndef CFG_GOLD
	if (m_TvFeed)
		m_TvFeed->DebugRender();
	if (m_pAnimCamera)
		m_pAnimCamera->DebugRender(RGBA(0xff80ffff), true);
#endif
}

void C_DebugTools::SetLocalCamera(const C_Camera& cam)
{
	assert(m_pNetworkLink);
	if (!m_pNetworkLink)
		return;
	
	m_LocalCamera = cam;
	m_pNetworkLink->SetLocalCamera(m_LocalCamera);
}

E_NetNotify C_DebugTools::Notify()
{
	return m_Notify;
}

const C_Camera& C_DebugTools::RemoteCamera() const
{
	return m_RemoteCamera;
}

void C_DebugTools::LogFrame(const C_Camera& cam)
{
	if (m_PosLogger)
		m_PosLogger->AddFrame(cam);
}

void C_DebugTools::GetCameraPosition()
{
	HVec3 pos = m_LocalCamera.Position();
	F32 pitch = 0.f, yaw = 0.f, roll = 0.f;
	m_LocalCamera.GetEuler(&pitch, &yaw, &roll);

	pitch = RadToDeg(pitch);
	yaw   = RadToDeg(yaw);
	if (yaw < 0.f)
		yaw += 360.f;
	roll  = RadToDeg(roll);

	F32 fov = m_LocalCamera.Fov();
	
	if (fov >= 0.f)
		fov = RadToDeg(fov); // Degrees, perspective, else orthographic
	
	char temp[128] = {0};
	sprintf(temp, "%.3f, %.3f, %.3f", (F32)pos.X(), (F32)pos.Y(), (F32)pos.Z());
	if (DetailLevel >= 4)
		sprintf(temp + strlen(temp), ", %.2f", yaw);
	if (DetailLevel >= 5)
		sprintf(temp + strlen(temp), ", %.2f", pitch);
	if (DetailLevel >= 6)
		sprintf(temp + strlen(temp), ", %.2f", roll);
	if (DetailLevel >= 7)
		sprintf(temp + strlen(temp), ", %.2f", fov);
	
	PositionString = temp;
	GetPositionNow = false;
}

void C_DebugTools::SetCameraPosition()
{
	Util::C_String text = (const char*)PositionString;

	// Change commas and brackets to spaces:
	for (S32 i = 0; i < text.Length(); i++)
	{
		S32 c = text[i];
		if (c == ',' || c == '(' || c == ')')
			text[i] = ' ';
	}
	
	F32 x = 0.f, y = 0.f, z = 0.f;
	F32 pitch = 0.f, yaw = 0.f, roll = 0.f, fov = 60.f;
	sscanf(text.Chars(), "%f %f %f %f %f %f %f", &x, &y, &z, &yaw, &pitch, &roll, &fov);
	
	m_RemoteCamera.SetPosition(HVec3(x, y, z));

	yaw   = DegToRad(yaw);
	pitch = DegToRad(pitch);
	roll  = DegToRad(roll);
	C_Quaternion orient(0.f, sin(-yaw * 0.5f), 0.f, cos(-yaw * 0.5f));
	orient = orient * C_Quaternion(sin(pitch * 0.5f), 0.f,  0.f, cos(pitch * 0.5f));
	orient = orient * C_Quaternion( 0.f,  0.f, sin(roll * 0.5f),cos(roll * 0.5f));

	m_RemoteCamera.SetOrient(orient);
	
	if (fov >= 0)
		fov = DegToRad(fov);
	m_RemoteCamera.SetFov(fov);

	m_Notify = E_NetNotify_SetPos;
}

void C_DebugTools::DoMapWizard()
{
	F32 mapx = MapX;
	F32 mapy = MapY;
	F32 mapz = MapZ;
	m_RemoteCamera.SetPosition(HVec3(mapx, mapy, mapz));

	// Point down at world:	
	F32 sqrtHalf = 0.70710678f;
	C_Quaternion orient(-sqrtHalf, 0.f,  0.f, sqrtHalf);

	m_RemoteCamera.SetOrient(orient);
	F32 spanz = MapSpan;
	m_RemoteCamera.SetFov(-spanz);

	// Print the extents out:
	F32 spanx = spanz * 16.f / 9.f;
	char temp[128] = {0};
	sprintf(temp, "%.3f, %.3f, %.3f, %.3f", mapx - spanx, mapx + spanx, mapz - spanz, mapz + spanz);
	MapString = temp;

	m_Notify = E_NetNotify_SetPos;
}

} // End of namespace Cam2
