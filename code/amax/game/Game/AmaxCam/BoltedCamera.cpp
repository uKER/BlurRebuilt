
#include "stdafx.h"
#pragma warning (push, 4)
#include "BoltedCamera.h"

#include "CameraSystem.h"
#include <game/AmaxCam/CameraNoiseSpecialFX.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

namespace AmaxCam
{

using namespace Maths;

C_BoltedCamera::C_BoltedCamera()
{
	SetName("Bolted");
	m_LookAngle = 0.f;
	m_PrevLookDir = 0;
	m_Offset = MathsStorage::Float3(0.0f, 0.0f, 0.0f);
	m_EffectOffset		= Maths::Constants<Maths::HVec3>::Zero();
	m_EffectOffsetRot	= Maths::Constants<Maths::HVec3>::Zero();
	m_EffectFov			= 0.0f;
}

void C_BoltedCamera::SetHandle(const Actor::C_ActorHandle& handle)
{
	m_Handle = handle;
}

void C_BoltedCamera::OnUpdate()
{
	int viewportIndex = -1;
	const Game::Actors::C_VehicleActor* vehicleActor = AmaxCam::GetVehicleActor(m_Handle);
	if(vehicleActor)
	{
		const Game::Actors::C_VehicleActorCreate *pVehicleCreate = (const Game::Actors::C_VehicleActorCreate *)(vehicleActor->GetCreationData().GetData());
		if(pVehicleCreate)
		{
			viewportIndex = pVehicleCreate->m_pRLC->GetViewportIndex((const U8)vehicleActor->GetVehicleIndex());
		}
	}
	// Query the SFX system for any noise
	const AmaxCam::C_CameraNoiseSpecialFX::ActiveNoiseList& NoiseList = AmaxCam::C_CameraNoiseSpecialFX::GetActiveNoiseList();
	for(AmaxCam::C_CameraNoiseSpecialFX::ActiveNoiseList::ConstIterator pIterator = NoiseList.Begin(); pIterator != NoiseList.End(); ++pIterator)
	{
		if( pIterator->m_ViewportID == viewportIndex )
		{
			AddOffset(pIterator->m_Position,pIterator->m_Orientation, 0.0f);
		}
	}

	HVec3 pos(m_Param.m_Position.vx+m_Offset.x, m_Param.m_Position.vy+m_Offset.y, m_Param.m_Position.vz+m_Offset.z);
	pos += m_EffectOffset;

	SetPosition(Object().ToWorldSpace(pos));
	
	// Quantise the look angle to 90 degree intervals:
	S32 lookDir = (S32)(m_LookAngle * 2.f / Pi + 2.5f); // 0 - 4
	lookDir &= 3; // 0 - 3
	m_LookAngle = (F32)(lookDir - 2) * Pi * 0.5f;
	
	C_Quaternion look(0.f, sin(m_LookAngle * -0.5f), 0.f, cos(m_LookAngle * -0.5f));
	
	SetOrient(Object().orient * look);

	Maths::C_Quaternion xrot(Maths::HVec3(1.0f, 0.0f, 0.0f), DegToRad(m_EffectOffsetRot.X()));
	Maths::C_Quaternion yrot(Maths::HVec3(0.0f, 1.0f, 0.0f), DegToRad(m_EffectOffsetRot.Y()));
	Maths::C_Quaternion zrot(Maths::HVec3(0.0f, 0.0f, 1.0f), DegToRad(m_EffectOffsetRot.Z()));
	SetOrient(Orient() * xrot * yrot * zrot);

	F32 fov = DegToRad(60.f);
	if (m_Param.m_Fov > 0.f)
		fov = DegToRad(m_Param.m_Fov);
	
	fov += Maths::DegToRad(m_EffectFov);
	F32 fovScale = CameraSystem().FovScale();
	SetFov(fov * fovScale);

	// If look direction snapped, don't have a streak of motion blur:
	if (lookDir != m_PrevLookDir)
		SetPrevShaken( Orient() );
	
	m_PrevLookDir = lookDir;

	m_EffectOffset		= Maths::Constants<Maths::HVec3>::Zero();
	m_EffectOffsetRot	= Maths::Constants<Maths::HVec3>::Zero();
	m_EffectFov			= 0.0f;
}

void 
C_BoltedCamera::AddOffset(const Maths::HVec3& offset, const Maths::HVec3& offsetrot, F32 fov)
{
	m_EffectOffset += offset;
	m_EffectOffset.Z(0.0);
	m_EffectOffsetRot += offsetrot;
	m_EffectFov += fov;
}

C_BoltedParam::C_BoltedParam()
{
	m_Fov = 58.f;
}


} // End of namespace

#pragma warning (pop)

XT_TYPE(AmaxCam::C_BoltedParam, "AmaxBoltedParam")
	XT_FIELD(m_Position, "Position")
	XT_FIELD(m_Fov,      "Fov")
XT_END_TYPE

