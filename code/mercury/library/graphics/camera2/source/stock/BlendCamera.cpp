
#include "stdafx.h"
#include "camera2/CameraType.h"
#include "camera2/stock/BlendCamera.h"

namespace Cam2
{

C_BlendCamera::C_BlendCamera()
{
	SetType(E_BlendCamera);
#ifndef CFG_GOLD			
	m_BlendName = "[B]";
	SetName(m_BlendName);
#endif

	m_counter = m_blendTime = 0.f;
	m_blendTo = m_blendFrom = 0;
	m_blendDelay = 0.0f;
}

void C_BlendCamera::OnUpdate()
{
	m_blendDelay -= Period();
	if (m_blendDelay > 0.0f)
		return;

	m_counter += Period();
	F32 blend = 1.f;

	if (m_blendTime > 1e-6f && m_counter < m_blendTime)
		blend = m_counter / m_blendTime;
	else
		m_counter = m_blendTime;

	if (blend >= 1.f)
	{
		const I_Feed* feed = 0;
		if (m_blendTo >= 0 && m_blendTo < m_FeedList.Size())
			feed = m_FeedList[m_blendTo];
		if (feed)
		{
			CopyFrom(feed->Live());
			SetOrient(feed->Live().Shaken());
		}
	}
	else
	{
		// Use Cos curve to smooth:
		blend = 0.5f + cos( blend * Maths::Pi ) * -0.5f;

		// Blend between whatever blend is currently active:
		S32 i = 0;
		for (i = 0; i < m_weight.Size(); i++)
			m_weight[i] = 0.f;

		if (m_blendFrom < m_weight.Size())
			m_weight[m_blendFrom] += 1.f - blend;

		if (m_blendTo < m_weight.Size())
			m_weight[m_blendTo] += blend;

		// Apply the weight and evaluate a camera:
		BlendWithWeights();
	}
	
#ifndef CFG_GOLD	
	m_BlendName = "[B]";
#endif
	
	if (m_blendTo >= 0 && m_blendTo < m_FeedList.Size())
	{
		const I_Feed* feed = m_FeedList[m_blendTo];
		if (feed)
		{
			SetObject( feed->Live().Object() );

#ifndef CFG_GOLD	
			m_BlendName += feed->Live().Name();
			SetName(m_BlendName);
#endif
		}
	}
}

// Set a camera feed into a slot:
void C_BlendCamera::SetFeedSlot(S32 n, const I_Feed* feed)
{
	n &= 0xff; // Sanity check
	
	if (m_FeedList.Size() < n + 1)
	{
		m_FeedList.Resize(n + 1);
		m_weight.Resize(n + 1);
	}
	
	m_FeedList[n] = feed;
}

void C_BlendCamera::BlendTo(S32 n, F32 blendTime, F32 blendDelay)
{
	if (n == m_blendTo)
		return;

	m_counter = 0.f;
	m_blendTime = blendTime;
	m_blendDelay = blendDelay;
	m_blendFrom = m_blendTo;
	m_blendTo = n;
}


bool C_BlendCamera::BlendActive()
{
	return m_counter < m_blendTime;
}

void C_BlendCamera::BlendWithWeights()
{
	Maths::HVec3 pos;
	Maths::HVec4 orient;
	F32 fov = 0.f, idepth = 0.f, aperture = 0.f, shutterAngle = 0.f;

	if (m_FeedList.Size() <= 0)
		return;
	
	Maths::C_Quaternion lastQuat;
	
	S32 i = 0;
	for (i = 0; i < m_FeedList.Size(); i++)
	{
		F32 weight = m_weight[i];
		if (weight <= 0.f)
			continue;
		
		const I_Feed* feed = m_FeedList[i];
		if (!feed)
			continue;
		
		const C_Camera& cam = feed->Live();

		pos += cam.Position() * weight;
		
		Maths::C_Quaternion ori = cam.Shaken();
		Maths::C_Quaternion relative = ori * !lastQuat;
		
		if (relative.Real().W() < 0.f)
			ori = -ori;
		
		lastQuat = ori;
		
		orient += (Maths::HVec4)ori * weight;
		fov    += cam.Fov() * weight;
		idepth += cam.Idepth() * weight;
		aperture += cam.Aperture() * weight;
		shutterAngle += cam.ShutterAngle() * weight;
	}

	Maths::C_Quaternion quat = (Maths::C_Quaternion)orient;
	quat.Normalise();
	
	SetPosition(pos);
	SetOrient(quat);
	SetFov(fov);
	SetShutterAngle(shutterAngle);
	SetIdepth(idepth);
	SetAperture(aperture);
}

S32 C_BlendCamera::GetIndex()
{
	return m_blendTo;
}

} // End of namespace Cam2
