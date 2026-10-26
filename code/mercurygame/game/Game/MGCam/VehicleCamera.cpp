
#include "stdafx.h"
#include "VehicleCamera.h"
#include "CameraSystem.h"
#include "PhysicsChecks.h"
#include <camera2/shake/ShakeWorld.h>

#include <Game/GameControls/GameControls.h>

namespace MGCam
{

C_VehicleCamera::C_VehicleCamera(Gravity::C_Scene* physicsScene, S32 groupId)
{
	SetShakeWorld(&Cam2::C_ShakeWorld::Instance());
	
	m_PhysicsChecks = new C_PhysicsChecks(physicsScene, groupId);
	SetPhysics(m_PhysicsChecks);
}

C_VehicleCamera::~C_VehicleCamera()
{
	SAFE_DELETE_PTR(m_PhysicsChecks)
}

void C_VehicleCamera::OnUpdate()
{
	F32 sx = 0.f, sy = 0.f;
	
	if (System().DebugFeed().PadUsed() != 0)
	{
		sx = Input::GetGameInput(0)->GetAxisSignedValue( Game::E_Axis_CameraLookX );
		sy = Input::GetGameInput(0)->GetAxisSignedValue( Game::E_Axis_CameraLookY );
	}
	SetLookAround(sx, sy);
	
	Cam2::C_ChaseCamera2::OnUpdate();
}

} // End of namespace
