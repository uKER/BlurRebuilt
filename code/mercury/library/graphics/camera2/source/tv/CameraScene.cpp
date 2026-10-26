
#include "stdafx.h"
#include <camera2/stock/AnimCamera.h>
#include <camera2/tv/CameraScene.h>
#include <Camera2/tv/CameraSceneSerialise.h>
#include <camera2/tv/AnimData.h>
#include <camera2/tv/TvZone.h>

namespace Cam2
{

C_CameraScene& CameraScene()
{
	static C_CameraScene s_CameraScene;
	return s_CameraScene;
}
	

C_CameraScene::C_CameraScene()
{
	m_AnimCamera = 0;
}

C_CameraScene::~C_CameraScene()
{
	SAFE_DELETE_PTR(m_AnimCamera)
	
	S32 i = 0;
	for (i = 0; i < m_zones.Size(); i++)
		delete m_zones[i];

	for (i = 0; i < m_cams.Size(); i++)
		delete m_cams[i];
}

void C_CameraScene::Load(Util::C_String filename)
{
	// Read camera scene from disk:
	Sys::I_FileSystem* fs = Sys::GetDefaultFileSystem();
	Sys::I_BinaryStream* file = fs->Open(filename);
	if (file == 0) return;

	Sys::BinaryDeserialiser<Ut::NativeToBigEndian> archive(*file);

	// Read header:
	S32 fileVersion = 0, pad = 0;
	archive >> fileVersion >> pad;
	file->SetVersionNumber(fileVersion);
	Debug::Logger& log = Debug::GetLogger("mercury.camera2");

	if (fileVersion == C_CameraScene::Version)
	{
		LOGINFO(log, "Camera scene '%s' (version %d/%d)", filename.Chars(), fileVersion, C_CameraScene::Version);
		archive >> *this;
	}
	else
	{
		LOGWARN(log, "Camera scene '%s' incorrect version (%d/%d), so not loading", filename.Chars(), fileVersion, C_CameraScene::Version);
	}

	delete file;
	file = 0;
}

void C_CameraScene::Update(F32 period)
{
	if (m_AnimCamera)
		m_AnimCamera->Update(period);
}

// Find out which zone a point is inside
S32 C_CameraScene::WhichZone(const Maths::HVec3& point) const
{
	S32 i = 0;
	
	for (i = 0; i < m_zones.Size(); i++)
	{
		bool ret = m_zones[i]->InsideZone(point);
		if (ret)
			return i;
	}
	
	return -1;
}

S32 C_CameraScene::FindAnimData(const char* name) const
{
	for (S32 i = 0; i < m_cams.Size(); i++)
	{
		const C_AnimData* cam = m_cams[i];
		if (stricmp(name, cam->m_name.Chars()) == 0)
			return i;
	}

	return -1;
}

const C_AnimData* C_CameraScene::GetAnimData(S32 index) const
{
	if (index < 0 || index >= m_cams.Size())
		return 0;

	return m_cams[index];
}

const C_AnimData* C_CameraScene::GetAnimData(const char* name) const
{
	return GetAnimData(FindAnimData(name));
}

bool C_CameraScene::PlayAnimation(const char* name)
{
	const C_AnimData* animData = GetAnimData(name);
	if (!animData)
		return false;

	if (!m_AnimCamera)
	{
		m_AnimCamera = new C_AnimCamera;
		if (!m_AnimCamera)
			return false;
	}
	
	m_AnimCamera->SetAnimData(animData);
	return true;
}

const C_Camera* C_CameraScene::Camera() const
{
	return m_AnimCamera;
}

} // End of namespace
