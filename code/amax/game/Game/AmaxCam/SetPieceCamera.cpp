
#include "stdafx.h"
#pragma warning (push, 4)
#include "SetPieceCamera.h"
#include "Helper.h"
#include "PhysicsChecks.h"
#pragma warning (push, 3)
#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Vehicle/VehicleModel.h>
#pragma warning (pop)

namespace AmaxCam
{

using namespace Game::Actors;
using namespace Maths;

C_SetPieceCamera::C_SetPieceCamera()
{
	SetName("SetPiece");
}

C_SetPieceCamera::~C_SetPieceCamera()
{
}

void C_SetPieceCamera::Start(const Actor::C_ActorHandle& handle, Maths::HVec3 p_Position, Maths::HVec3 p_LookAt, float p_Fov, bool p_TrackObject)
{
	m_Handle = handle;

	GetObject();

	m_TrackObject	= p_TrackObject;
	
	SetPosition(p_Position);

	m_LookAt = p_LookAt;

	m_Fov = p_Fov;

	SetFov(DegToRad(m_Fov));

	Update(0.f);
}

void C_SetPieceCamera::GetObject()
{
	const C_VehicleActor* vehicleActor = GetVehicleActor(m_Handle);
	if (vehicleActor == 0)
		return;
	Cam2::C_CamObj object = VehicleActorToCameraObject(vehicleActor);
	SetObject(object);
}

void C_SetPieceCamera::OnUpdate()
{
	GetObject();

	if (m_TrackObject)
	{
		const C_VehicleActor* vehicleActor = GetVehicleActor(m_Handle);
		if (vehicleActor)
		{
			Maths::HVec3 vehiclePos = vehicleActor->GetMatrix().AW();
			Maths::HVec3 direction = vehicleActor->GetVehicleModel()->GetModelInput()->m_vVelocity;
			direction.Normalise();
			Maths::HVec3 vForward = direction * m_LookAt.Z();
			Maths::HVec3 vUp = Maths::HVec3(0.0f, m_LookAt.Y(), 0.0f);
			Maths::HVec3 lookAt = vehiclePos + vForward + vUp;
			vForward.Normalise();
			vUp.Normalise();
			Maths::HVec3 vSide = vForward ^ vUp;
			lookAt = lookAt + vSide * m_LookAt.X();
			LookAt(lookAt);
		}
	}
	else
		LookAt(m_LookAt);

	SetFov(DegToRad(m_Fov));
}

} // End of namespace

#pragma warning (pop)
