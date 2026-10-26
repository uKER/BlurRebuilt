
#include "stdafx.h"

#include <Camera2/CameraType.h>
#include <Camera2/stock/TvFeed.h>
#include <Camera2/stock/AnimCamera.h>

#include <camera2/tv/AnimData.h>
#include <Camera2/tv/CameraScene.h>
#include <Camera2/tv/TvZone.h>
#include <system/debug/I_DebugRender.h>

namespace Cam2
{

//static Debug::Int CameraChoice("Camera2.TvFeed.CameraChoice", -1, Debug::Range(-1, 10));

C_TvFeed::C_TvFeed()
{
	SetType(E_TvFeed);
	m_pScene = 0;
	m_liveZone = m_liveCam = 0;
	m_prevCam = -1;
}

C_TvFeed::~C_TvFeed()
{
	FreeTvScene();
	m_pScene = 0;
}

void C_TvFeed::Update(F32 period)
{
	F32 progress = -100.f;

	if (m_pScene == 0)
		return;

	// Find out which zone the object is in:
	S32 zone = m_pScene->WhichZone(Object().position);

	if (zone >= 0)
		m_liveZone = zone;

	if (m_liveZone >= 0 && m_liveZone < m_pScene->m_zones.Size())
	{
		const C_TvZone* zone = m_pScene->m_zones[m_liveZone];
		assert(zone);
		
		// Pick the live camera:
		m_liveCam = zone->m_cameraStart;

		progress = zone->m_spline.Progress(Object().position);
	}

	//if (CameraChoice >= 0)
	//	m_liveCam = CameraChoice;

	// Update all cameras:
	S32 i = 0;
	for (i = 0; i < m_tvcam.Size(); i++)
	{
		C_AnimCamera* anim = m_tvcam[i];
		
		anim->SetObject(Object());

		if (i == m_liveCam)
		{
			if (m_prevCam != m_liveCam)
				anim->SetCounter(0.f);

			// Spline driven:
			if (progress >= 0.f)
				anim->SetCounter(progress * 10.f);
		}

		anim->Update(period);
	}
	
	m_prevCam = m_liveCam;
}

const C_Camera& C_TvFeed::Live() const
{
	if (m_liveCam >= 0 && m_liveCam < m_tvcam.Size())
	{
		assert(m_tvcam[m_liveCam]);
		return *m_tvcam[m_liveCam];
	}
	else
		return NullCamera;
}

void C_TvFeed::LoadTvScene(Util::C_String filename)
{
	FreeTvScene();

	m_pScene = new C_CameraScene;
	if (m_pScene == 0)
		return;
	m_pScene->Load(filename);

	// Init cameras:
	S32 max = m_pScene->m_cams.Size();
	S32 i = 0;
	for (i = 0; i < max; i++)
	{
		C_AnimCamera* panim = new C_AnimCamera;
		if (panim == 0) continue;
		
		panim->SetAnimData(m_pScene->m_cams[i]);
		m_tvcam.PushBack(panim);
	}
}

void C_TvFeed::FreeTvScene()
{
	S32 i = 0;
	for (i = 0; i < m_tvcam.Size(); i++)
		delete m_tvcam[i];
	m_tvcam.Clear();
	
	delete m_pScene;
	m_pScene = 0;
}

void C_TvFeed::DebugRender() const
{
#ifndef CFG_GOLD
	if (m_pScene == 0)
		return;

	S32 i = 0;

	DbUtil::I_DebugRender* debugRender = GetDebugRender();

	// Go through all the zones:
	for (i = 0; i < m_pScene->m_zones.Size(); i++)
	{
		const C_TvZone& z = *m_pScene->m_zones[i];
		assert(&z);
		
		Maths::RGBA colour(0, 192, 192, 128);

		if (i == m_liveZone) colour = Maths::RGBA(255, 0, 0, 128);

		// Go through all the points in all the faces of the zone
		S32 n = 0;
		for (n = 0; n < z.m_face.Size(); n++)
		{
			// m = wraps back to first point in each face:
			S32 m = (n & ~3) | ((n + 1) & 3);
			
			if (m >= z.m_face.Size())
				continue;

			debugRender->Draw3dLine(z.m_point[z.m_face[n]], z.m_point[z.m_face[m]], colour);
		}

		const C_ZoneSpline& spline = z.m_spline;
		assert(&spline);

		S32 j = 0;
		for (j = 1; j < spline.m_cvs.Size(); j++)
		{
			debugRender->Draw3dLine(
				spline.m_cvs[j    ].v,
				spline.m_cvs[j - 1].v,
				colour
			);
		}
	}

	// Render all of the cameras:
	for (i = 0; i < m_tvcam.Size(); i++)
	{
		Maths::RGBA colour(0, 255, 255);
		bool frustrum = false;
		
		// Render the live one in red:		
		if (i == m_liveCam)
		{
			colour = Maths::RGBA(255, 0, 0);
			frustrum = true;
		}
		
		m_tvcam[i]->DebugRender(colour, frustrum);
	}
#endif
}

S32 C_TvFeed::GetCameraCount()
{
	return m_tvcam.Size();
}

const C_Camera* C_TvFeed::GetCamera(S32 index)
{
	if (index >= 0 && index < m_tvcam.Size())
		return m_tvcam[index];
	
	return 0;
}

Util::C_String C_TvFeed::CameraName()
{
#ifndef CFG_GOLD
	if (m_pScene)
	{
		if (m_liveCam >= 0 && m_liveCam < m_pScene->m_cams.Size())
		{
			const C_AnimData* tvc = m_pScene->m_cams[m_liveCam];
			
			return tvc->m_name;
		}
	}
#endif
	return Util::C_String();
}

} // End of namespace Cam2
