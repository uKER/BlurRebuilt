
#include "stdafx.h"
#include <camera2/shake/ShakeWorld.h>
#include <camera2/stock/ShakeCamera.h>
#include <system/memory/MemoryTracker.h>

namespace Cam2
{
Debug::Float ShakeAmountX("Camera2.Shake.AmountX", 0.f, Debug::Range(0.f, 5.f));
Debug::Float ShakeAmountY("Camera2.Shake.AmountY", 0.f, Debug::Range(0.f, 5.f));
Debug::Float ShakeAmountZ("Camera2.Shake.AmountZ", 0.f, Debug::Range(0.f, 5.f));
Debug::Float ShakeFreq("Camera2.Shake.Frequency", 10.f, Debug::Range(0.f, 20.f));

class C_ShakeEvent
{
public:
	Maths::HVec3 m_worldPos; // Position in world space of the shake event
	F32 m_frequency; // Frequency in cycles per second
	F32 m_duration;  // How long the event lasts for
	F32 m_magnitude; // How 'loud' the shake event is
	F32 m_range;     // Range in metres of the shake
	F32 m_timer;     // How far through the event we are
	F32 m_amplitude; // Current amplitude
	S32 m_counter;   // Counter for Perlin Noise
	F32 m_FovMagnitude; // How 'loud' the shake event is on the Field of View
	F32 m_FovAmplitude; // Current amplitude

	C_ShakeEvent();
	void Update(F32 period);
	bool Finished() { return m_timer >= m_duration; }
};

C_ShakeWorld::C_ShakeWorld()
{
	m_pGlobalShake = new C_ShakeCamera();
}

C_ShakeWorld::~C_ShakeWorld()
{
	SAFE_DELETE_PTR(m_pGlobalShake)
}

void C_ShakeWorld::Reset()
{
	m_ShakeList.Clear();
}

void C_ShakeWorld::AddEvent(const Maths::HVec3& worldPos, F32 frequency, F32 duration, F32 magnitude, F32 range, F32 fovMagnitude)
{
	MEMORY_TRACKER_CONTEXT("camera.shake");

	// Limit the amount of shake events:
	if (m_ShakeList.Size() >= 8)
		return;

	C_ShakeEvent* pe = new C_ShakeEvent;
	if (pe == 0)
		return;
	pe->m_worldPos = worldPos;
	pe->m_frequency = frequency;
	pe->m_duration  = duration;
	pe->m_magnitude = Maths::DegToRad(magnitude);
	pe->m_range     = range;
	pe->m_timer     = 0.f;
	
	pe->m_FovMagnitude = Maths::DegToRad(fovMagnitude);
	
	m_ShakeList.PushBack(pe);
}

void C_ShakeWorld::Update(F32 period)
{
	S32 i = 0;
	while (i < m_ShakeList.Size())
	{
		C_ShakeEvent*& pe = m_ShakeList[i];
		
		pe->Update(period);
		
		if (pe->Finished())
		{
			delete pe;
			pe = 0;
			m_ShakeList.EraseByIndex(i);
		}
		else
			i++;
	}

	// This is used to override the shake on any camera in C_Camera::GetCameraShake:
	if (m_pGlobalShake)
	{
		m_pGlobalShake->SetParamXYZ(ShakeAmountX, ShakeAmountY, ShakeAmountZ, ShakeFreq);
		m_pGlobalShake->Update(period);
	}
}

// Get the shake quaternion at a particular point in the world:
void C_ShakeWorld::GetCameraShake(const Maths::HVec3& position, Maths::C_Quaternion* pRetQuat, F32 *ptotalAmp, F32* pRetFovShake) const
{
	Maths::C_Quaternion quat;
	F32 totalAmp = 0.f, fovShake = 0.f;

	S32 i = 0;
	for (i = 0; i < m_ShakeList.Size(); i++)
	{
		C_ShakeEvent* pe = m_ShakeList[i];

		F32 dist = (position - pe->m_worldPos).Length();
		
		F32 scale = 1.f;
		if (pe->m_range > 0.0001f)
			scale = 1.f - dist / pe->m_range;
		
		if (scale < 0.f)
			scale = 0.f;

		totalAmp += scale * pe->m_amplitude;

		F32 ra = SoftNoise(pe->m_counter);
		F32 rb = SoftNoise(pe->m_counter + 0x234567);
		F32 rc = SoftNoise(pe->m_counter + 0x566757);
		F32 rd = SoftNoise(pe->m_counter + 0x739821);
		
		ra *= scale * pe->m_amplitude;
		rb *= scale * pe->m_amplitude;
		rc *= scale * pe->m_amplitude;
		rd *= scale * pe->m_FovAmplitude;
		
		quat = quat * Maths::C_Quaternion(sin(ra * 0.5f), 0.f, 0.f, cos(ra * 0.5f));
		quat = quat * Maths::C_Quaternion(0.f, sin(rb * 0.5f), 0.f, cos(rb * 0.5f));
		quat = quat * Maths::C_Quaternion(0.f, 0.f, sin(rc * 0.5f), cos(rc * 0.5f));
		fovShake += rd;
	}
	
	if (pRetQuat)
		*pRetQuat = quat;
	
	if (ptotalAmp)
		*ptotalAmp = totalAmp;

	if (pRetFovShake)
		*pRetFovShake = fovShake;
}

C_ShakeEvent::C_ShakeEvent()
{
	m_frequency = m_duration = m_magnitude = m_range = m_timer = m_amplitude = 0.f;
	m_FovMagnitude = m_FovAmplitude = 0.f;
	m_counter = (rand() << 12) ^ rand();
}

void C_ShakeEvent::Update(F32 period)
{
	m_timer += period;
	
	F32 amp = 1.f;
	if (m_duration > 0.0001f)
		amp = 1.f - m_timer / m_duration;

	if (amp < 0.f)
		amp = 0.f;

	m_amplitude = m_magnitude * amp; // Set amplitude amount
	m_FovAmplitude = m_FovMagnitude * amp; // Set fov amplitude amount

	m_counter += (S32)(period * 65536.f * m_frequency);
}

C_ShakeWorld& C_ShakeWorld::Instance()
{
	static C_ShakeWorld s_instance;
	return s_instance;
}

} // End of namespace

