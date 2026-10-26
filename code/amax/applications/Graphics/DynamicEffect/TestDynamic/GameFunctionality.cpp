#include "stdafx.h"
#include "TestDynamic/GameFunctionality.h"
#include "Dynamic/Common.h"
#include "Render/Debug/DebugRender.h"

void C_Projectile::Initialise(const Maths::HVec3& vVelocity, const DynamicEffect::C_Node& transform)
{
	m_vVelocity = vVelocity;
	m_Transform = transform;
}

void C_Projectile::Update(F32 fFrameTime)
{
	Maths::HVec3 vPosition(Maths::Load<Maths::HVec3>(&m_Transform.GetPosition()));
	m_vVelocity += Maths::HVec3(0.0f, -4.0f * fFrameTime, 0.0f);
	vPosition += m_vVelocity * fFrameTime;

	Maths::HVec3 vDirection = Maths::Normalise(m_vVelocity);
	Maths::C_Quaternion qRotation = DynamicEffect::GetQuaternionFromTo<Maths::C_Quaternion>(Maths::Constants<Maths::HVec3>::UnitAxisPositiveY(), vDirection);

	m_Transform.SetRotation(qRotation);
	m_Transform.SetPosition(vPosition);
}

void C_Projectile::Render() const
{
	Maths::HVec3 vPosition(Maths::Load<Maths::HVec3>(&m_Transform.GetPosition()));
	Maths::HVec3 vDirection = Maths::Normalise(m_vVelocity);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(vPosition, vDirection, 1.0f, 0.1f, Maths::RGBA(255, 255, 0, 255));
}
