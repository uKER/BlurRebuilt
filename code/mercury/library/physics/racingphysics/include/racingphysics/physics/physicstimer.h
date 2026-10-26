////////////////////////////////////////////////////////////
// Physics Timer
////////////////////////////////////////////////////////////
#pragma once

#ifndef RP_PHYSICS_TIMER
#define RP_PHYSICS_TIMER

#include "system/time/time.h"

namespace RacingPhysics
{

class ALIGN16 PhysicsTimer
{
public:
	static PhysicsTimer& Instance() {return s_instance;}

	// current physics time
	double PhysicsGetTimer() {return (m_frameCount - 1) * PhysicsUpdateTime();}

	void SetPhysicsUpdateTime(float time) {m_physicsUpdateTime = time;}

	// time for each physics update
	float PhysicsUpdateTime();

	// increment timer by one frame
	void UpdatePhysicsTimer() {m_frameCount++;}

	// call when race started to store the current frames passed since we called InitialiseTimers()
	void StoreRaceStartFrameCount() {m_raceStartedFrameCount = m_frameCount;}

	// how many frames have passed since the race was flagged as started.
	int FramesSinceRaceStart(void) const {return m_frameCount - m_raceStartedFrameCount;}

	// initialize timers
	void InitialiseTimers();

#if !PLATFORM_SPU
	void NetworkCatchUp(int frames);
	void GetDmaData(void** pDmaData);

	// real time since initialise timers
	int GetFramesSinceRaceStartReal() const;
#endif
	
	void SpuFetch(void* pDmaData);

private:
	PhysicsTimer() { InitialiseTimers(); }

	static PhysicsTimer s_instance;

	int m_frameCount;
	int m_raceStartedFrameCount;
	float m_physicsUpdateTime;
	Time::C_Time m_raceStartTime;
};

}

#endif
