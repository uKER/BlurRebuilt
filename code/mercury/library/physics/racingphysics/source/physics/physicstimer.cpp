////////////////////////////////////////////////////////////
// Physics Timer
////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "racingphysics/physics/physicstimer.h"

namespace RacingPhysics
{

PhysicsTimer PhysicsTimer::s_instance;

void PhysicsTimer::InitialiseTimers()
{
	m_frameCount = 0;
	m_raceStartedFrameCount = 0;
	m_physicsUpdateTime = 1.f / 60.f;
#if !PLATFORM_SPU
	m_raceStartTime = Time::GetPerformanceCounter();
#endif
}

float PhysicsTimer::PhysicsUpdateTime()
{
	return m_physicsUpdateTime;
}

#if !PLATFORM_SPU

void PhysicsTimer::NetworkCatchUp(int frames)
{
	m_frameCount += frames;
}

void PhysicsTimer::GetDmaData(void** pDmaData)
{
	*pDmaData = this;
}

int PhysicsTimer::GetFramesSinceRaceStartReal() const
{
	const Time::C_Time currTime = Time::GetPerformanceCounter();
	return ((currTime - m_raceStartTime) * 60.f) / Time::GetPerformanceFrequency();
}

#endif

void PhysicsTimer::SpuFetch(void* pDmaData)
{
#if PLATFORM_SPU
	Spu::Fetch(this, pDmaData, sizeof(PhysicsTimer));
#endif
}

}
