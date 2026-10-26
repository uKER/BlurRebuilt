
#include "stdafx.h"
#pragma warning (push, 4)
#include "AmaxChaseCamera.h"
#include "AmaxChaseParam.h"
#pragma warning (push, 3)
#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Vehicle/VehicleModel.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#pragma warning (pop)
#include "PhysicsChecks.h"
#include "Helper.h"

namespace AmaxCam
{

using namespace Game::Actors;
using namespace Maths;

class C_AccelEffect
{
public:
	C_AccelEffect();
	XtF32 m_Roughness;
	XtF32 m_ZEffect, m_ZMin, m_ZMax;
	XtF32 m_FovEffect, m_FovMin, m_FovMax;

	static void SorceryCallback(XtEventType, XtRecord*, C_AccelEffect* self, XtCallbackContext*);
};

static C_AccelEffect g_AccelEffect;

C_AccelEffect::C_AccelEffect()
{
	m_Roughness = 0.f;
	m_ZEffect = m_ZMin = m_ZMax = 0.f;
	m_FovEffect = m_FovMin = m_FovMax = 0.f;
}

void C_AccelEffect::SorceryCallback(XtEventType, XtRecord*, C_AccelEffect* self, XtCallbackContext*)
{
	g_AccelEffect = *self;
}

C_AmaxChaseCamera::C_AmaxChaseCamera()
{
	VehicleReset();
	m_PrevSpeed = 0.f;
	m_SoftAccel = 0.f;
	m_EffectFov = 0.f;
	m_Raytrace = true;
}

C_AmaxChaseCamera::~C_AmaxChaseCamera()
{
}

void C_AmaxChaseCamera::VehicleReset()
{
	AddOffset(HVec3(), HVec3(), 0.f);
	Cam2::C_ChaseCamera2::Reset();
	m_AmaxReset = true;
	m_SoftAccel = 0.f;
}

void C_AmaxChaseCamera::OnUpdate()
{
	const C_VehicleActor* vehicleActor = GetVehicleActor(m_Handle);
	if (vehicleActor == 0)
		return;

	Vehicle::C_VehicleModel* model = vehicleActor->GetVehicleModel();
	Vehicle::C_ModelInput* input = model ? model->GetModelInput() : 0;
	
	HVec3 veloc;
	if (input)
		veloc = input->m_vVelocity;

	F32 speed = veloc.Length();

	F32 period = Period();
	F32 accel = 0.f;
	
	if (!m_AmaxReset)
	{
		accel = speed - m_PrevSpeed;
		if (period > 0.001f)
			accel /= period;
	}
	else
	{
		m_AmaxReset = false;
	}
			
	m_SoftAccel += (accel - m_SoftAccel) * g_AccelEffect.m_Roughness;

	m_PrevSpeed = speed;

	// Move camera forward/backwards based on acceleration:
	F32 forward = m_SoftAccel * -g_AccelEffect.m_ZEffect;
	if (forward < g_AccelEffect.m_ZMin)
		forward = g_AccelEffect.m_ZMin;
	else if (forward > g_AccelEffect.m_ZMax)
		forward = g_AccelEffect.m_ZMax;
	m_AccelOffset = HVec3(0.f, 0.f, forward);

	// Change camera fov based on acceleration:
	m_AccelFov = m_SoftAccel * g_AccelEffect.m_FovEffect;
	if (m_AccelFov < g_AccelEffect.m_FovMin)
		m_AccelFov = g_AccelEffect.m_FovMin;
	else if (m_AccelFov > g_AccelEffect.m_FovMax)
		m_AccelFov = g_AccelEffect.m_FovMax;

	if (m_Raytrace)
		SetPhysics(vehicleActor->GetPhysicsChecks());

	Maths::C_Quaternion xrot(Maths::HVec3(1.0f, 0.0f, 0.0f), DegToRad(m_EffectOffsetRot.X()));
	Maths::C_Quaternion yrot(Maths::HVec3(0.0f, 1.0f, 0.0f), DegToRad(m_EffectOffsetRot.Y()));
	Maths::C_Quaternion zrot(Maths::HVec3(0.0f, 0.0f, 1.0f), DegToRad(m_EffectOffsetRot.Z()));

	UpdateParams();
	SetOrient(m_PlainOrient);
	Cam2::C_ChaseCamera2::OnUpdate();
	m_PlainOrient = Orient();

	SetOrient(m_PlainOrient * xrot * yrot * zrot);

	SetPhysics(0); // blank physics checks pointer, just in case

	BlankParams(); // Reset the parameters so they can be accumulated next frame
}

void C_AmaxChaseCamera::Update2()
{
	const C_VehicleActor* vehicleActor = GetVehicleActor(m_Handle);
	if (!vehicleActor)
		return;

	SetPhysics(vehicleActor->GetPhysicsChecks());
	Cam2::C_ChaseCamera2::Update2();
	SetPhysics(0);
}

void C_AmaxChaseCamera::AddOffset(const Maths::HVec3& offset, const Maths::HVec3& offsetrot, F32 fov)
{
	m_EffectOffset += offset;
	m_EffectOffsetRot += offsetrot;
	m_EffectFov += fov;
}

void C_AmaxChaseCamera::UpdateParams()
{
	Cam2::C_ChaseCamera2::SetOffset(m_EffectOffset + m_AccelOffset);

	Cam2::C_ChaseParam param;
	
	param.distance = m_AmaxParam.m_Distance;
	param.height = m_AmaxParam.m_Height;
	param.fov = m_AmaxParam.m_Fov + m_AccelFov + m_EffectFov;
	param.behindSpring = m_AmaxParam.m_BehindSpring;
	param.angularLimit = m_AmaxParam.m_AngularLimit;
	param.angularFade = m_AmaxParam.m_AngularFade;	
	param.pivotZ = m_AmaxParam.m_PivotZ;
	param.fov += m_AccelFov + m_EffectFov;
	if (param.fov > 120.f)
		param.fov = 120.f;
	param.tilt = m_AmaxParam.m_Tilt;
	param.safeHeight = m_AmaxParam.m_SafeHeight;

	SetParam(param);
}

void C_AmaxChaseCamera::BlankParams()
{
	m_EffectOffset = HVec3();
	m_EffectOffsetRot = HVec3();
	m_EffectFov = 0.f;
}

void C_AmaxChaseCamera::SetMotionBlur(const HVec3& motion)
{
	SetPrevPosition(Position() - motion);
}

C_AmaxChaseParam::C_AmaxChaseParam()
{
	m_Distance = 4.65f;
	m_Height = 1.4f;
	m_Fov = 55.f;
	m_Tilt = -8.f;
	m_BehindSpring = 5.5f;
	m_AngularLimit = 180.f;
	m_AngularFade = 40.f;
	m_PivotZ = 0.f;
	m_SafeHeight = 0.7f;
}

} // End of namespace

#pragma warning (pop)

XT_TYPE(AmaxCam::C_AccelEffect, "AmaxAccelEffect")
	XT_CALLBACK(AmaxCam::C_AccelEffect::SorceryCallback)
	XT_FIELD(m_Roughness,    "Roughness")
	XT_FIELD(m_ZEffect,      "ZEffect")
	XT_FIELD(m_ZMin,         "ZMin")
	XT_FIELD(m_ZMax,         "ZMax")
	XT_FIELD(m_FovEffect,    "FovEffect")
	XT_FIELD(m_FovMin,       "FovMin")
	XT_FIELD(m_FovMax,       "FovMax")
XT_END_TYPE

XT_TYPE(AmaxCam::C_AmaxChaseParam, "AmaxChaseParam")
	XT_FIELD(m_Distance,		"Distance")
	XT_FIELD(m_Height,			"Height")
	XT_FIELD(m_Fov,				"Fov")
	XT_FIELD(m_Tilt,			"Tilt")
	XT_FIELD(m_BehindSpring,    "BehindSpring")
	XT_FIELD(m_AngularLimit,    "AngularLimit")
	XT_FIELD(m_AngularFade,     "AngularFade")
	XT_FIELD(m_PivotZ,          "PivotZ")
	XT_FIELD(m_SafeHeight,      "SafeHeight")
XT_END_TYPE
