
#include "stdafx.h"
#pragma warning (push, 4)
#include "FatalCamera.h"
#include "Helper.h"
#include "PhysicsChecks.h"
#pragma warning (push, 3)
#include <Game/GameActors/Vehicles/VehicleActor.h>
#pragma warning (pop)

namespace AmaxCam
{

using namespace Game::Actors;
using namespace Maths;

C_FatalCamera::C_FatalCamera()
{
	SetName("Fatal");
}

C_FatalCamera::~C_FatalCamera()
{
}

void C_FatalCamera::Start(const Actor::C_ActorHandle& handle, const Cam2::C_Camera& startFrom)
{
	m_Handle = handle;
	m_Start = startFrom;
	m_Anim = 0.f;

	GetObject();

	m_End.SetPosition( Object().ToWorldSpace(HVec3(5.f, 2.f, -5.f)) );
	Update(0.f);
}

void C_FatalCamera::GetObject()
{
	const C_VehicleActor* vehicleActor = GetVehicleActor(m_Handle);
	if (vehicleActor == 0)
		return;
	Cam2::C_CamObj object = VehicleActorToCameraObject(vehicleActor);
	SetObject(object);
}

void C_FatalCamera::OnUpdate()
{
	GetObject();

	F32 lerp = m_Anim * 0.5f;
	
	if (lerp < 0.f)
		lerp = 0.f;
	if (lerp > 1.f)
		lerp = 1.f;

	// Use Cos curve to smooth:
	lerp = 0.5f + cos( lerp * Maths::Pi ) * -0.5f;
	
	m_End.LookAt( Object().ToWorldSpace(HVec3(0.f, 0.f, 2.f)) );
	m_End.SetFov(DegToRad(10.f));
	CopyFrom(m_Start.Lerp(m_End, lerp));

	// Clip position:
	HVec3 safe = Object().ToWorldSpace(HVec3(0.f, 0.6, 0.f));
	const C_VehicleActor* vehicleActor = GetVehicleActor(m_Handle);
	C_PhysicsChecks* checks = vehicleActor ? vehicleActor->GetPhysicsChecks() : 0;

	F32 trace = 1.f;
	if (checks)
		trace = checks->RayTrace(safe, Position(), 0.2f);
	SetPosition(safe + (Position() - safe) * trace);	
	
	m_Anim += Period();
}

} // End of namespace

#pragma warning (pop)
