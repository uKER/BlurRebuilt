
#include "stdafx.h"
#include "PassingShake.h"

#include "CameraSystem.h"
#include "GameLogic/RaceLogicCore/Core/RaceLogicCore.h"

#include <camera2/shake/ShakeWorld.h>

#pragma warning (push, 4)

namespace AmaxCam
{
using namespace Game::Actors;
using namespace Maths;

static Debug::Float g_PassingShakeAmount("Camera2.Shake.PassingShake.Amount",    0.05f, Debug::Range(0.0f, 0.1f));
static Debug::Float g_PassingDuration   ("Camera2.Shake.PassingShake.Duration",  1.0f,  Debug::Range(0.0f, 2.0f));
static Debug::Float g_PassingFrequency  ("Camera2.Shake.PassingShake.Frequency", 8.0f,  Debug::Range(0.0f, 20.0f));
static Debug::Float g_PassingRange      ("Camera2.Shake.PassingShake.Range",     5.0f,  Debug::Range(0.0f, 10.0f));

C_PassingShake& PassingShake()
{
	static C_PassingShake s_instance;
	return s_instance;
}

C_PassingShake::C_PassingShake()
{
	memset(&m_VelocityInDir, 0, sizeof(m_VelocityInDir));
}

C_PassingShake::~C_PassingShake()
{
}

void C_PassingShake::Update()
{
	RaceLogicCore* rlc = GetRaceLogicCore();
	
	if (!rlc)
		return;

	const Cam2::C_Camera& camera = CameraSystem().DebugFeed().Live();

	int count = Min((int)rlc->GetNumVehicles(), 20);

	int i = 0;
	for ( ; i < count; i++)
	{
		const C_VehicleActor* vehicle = rlc->GetVehicleActor((U8)i);
		if (!vehicle)
			continue;
		
		HVec3 carPos = vehicle->GetMatrix().AW();
		HVec3 velocity = GetVehicleVelocity(vehicle);
		HVec3 lookAt = carPos - camera.Position();
		if (lookAt.Length2() > g_PassingRange * g_PassingRange)
			continue;
		
		lookAt = Normalise(lookAt);
		
		float vid = velocity | lookAt;
		float prev = m_VelocityInDir[i];
		
		if (prev < 0.0f && vid >= 0.0f)
		{
			// A car just went past:
			Cam2::C_ShakeWorld::Instance().AddEvent
			(
				carPos,
				g_PassingFrequency,
				g_PassingDuration,
				velocity.Length() * g_PassingShakeAmount,
				g_PassingRange
			);
		}
	
		m_VelocityInDir[i] = vid;
	}
}

} // End of namespace

#pragma warning (pop)
