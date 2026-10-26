#include "stdafx.h"
#include <animation/ik/SteeringController.h>
#include <animation/ik/IkChain.h>

#ifndef CFG_GOLD
#include <system/debug/I_DebugRender.h>
#endif

namespace Animation
{

C_SteeringController::C_SteeringController():
	m_invRefMatrix(Maths::Const<Maths::Mat43>::Identity()),
	m_targetMatrix(Maths::Const<Maths::Mat43>::Identity()),
	m_oneOverRefRadius(1.f),
	m_targetRadius(1.f),
	m_numArms(0),
	m_retargetingFalloff(0.15f),
	m_retargetingExpansionDist(0.05f),
	m_normalAxis(E_Axis_Z),
	m_reflectX(false),
	m_dirty(true)
{
}
		
void C_SteeringController::Clear()
{
	m_numArms = 0;
}

bool C_SteeringController::AddArm(C_IkChain& ikChain)
{
	if (m_numArms >= kMaxArms)
	{
		return false;
	}
	
	U32 armIndex = m_numArms;
	
	m_armIkChains[armIndex] = &ikChain;
	
	++m_numArms;
	
	return true;
}

U32 C_SteeringController::GetNumArms()
{
	return m_numArms;
}

void C_SteeringController::SetWheelNormalAxis(E_Axis normalAxis)
{
	m_normalAxis = normalAxis;
	m_dirty = true;
}

void C_SteeringController::SetReferenceWheel(const Maths::Mat43& modelMatrix, F32 radius)
{
	m_invRefMatrix = Maths::Inverse(modelMatrix);
	m_oneOverRefRadius = Maths::Reciprocal(radius);
	m_dirty = true;
}

void C_SteeringController::SetTargetWheel(const Maths::Mat43& modelMatrix, F32 radius)
{
	m_targetMatrix = modelMatrix;
	m_targetRadius = radius;
	m_dirty = true;
}

void C_SteeringController::SetDistanceFromWheelToHands(F32 dist)
{
	m_retargetingExpansionDist = dist;
	m_dirty = true;
}

void C_SteeringController::SetFalloffDistance(F32 falloffDist)
{
	m_retargetingFalloff = falloffDist;
	m_dirty = true;
}

void C_SteeringController::Update(bool reflectTargetX)
{
	if (GetNumArms() == 0)
	{
		return;
	}

	if (reflectTargetX != m_reflectX)
	{
		m_dirty = true;
		m_reflectX = reflectTargetX;
	}
	
	if (m_dirty)
	{
		F32 scale = m_targetRadius * m_oneOverRefRadius;
		F32 scale3[3] = { scale, scale, scale };
		scale3[m_normalAxis] = 1.f;
		Maths::Mat33 scaleMatrix = Maths::MakeMat33(Maths::MakeScaleMat3(scale3[0], scale3[1], scale3[2]));
		
		Maths::Mat43 targetMatrix = m_targetMatrix;
		if (reflectTargetX)
		{
			Maths::SetRowW(&targetMatrix, Maths::NegateX(Maths::RowW(targetMatrix)));
		}
		
		m_refToTargetMatrix = m_invRefMatrix * Maths::MakeMat43(scaleMatrix) * targetMatrix;
		m_refToTargetRotation = Maths::MakeUnitQuaternion(Maths::MakeMat33(m_invRefMatrix) * Maths::MakeMat33(targetMatrix));

		for (U32 i = 0; i < GetNumArms(); ++i)
		{
			C_IkChain* pIkChain = m_armIkChains[i];

			pIkChain->SetRetargetingMatrix(m_refToTargetMatrix);
			pIkChain->SetRetargetingRotation(m_refToTargetRotation);
			pIkChain->SetWorldToRetargetingRegionMatrix(m_invRefMatrix);

			F32 refWheelRadius = 1.f / m_oneOverRefRadius;	// 
			F32 expandRadius = m_retargetingExpansionDist;
			pIkChain->SetRetargetingRegionCylinder(expandRadius, -refWheelRadius, expandRadius + refWheelRadius);
			pIkChain->SetRetargetingRegionFalloff(m_retargetingFalloff);
			pIkChain->EnableRetargeting();
		}


		m_dirty = false;
	}
}

#ifndef CFG_GOLD
void C_SteeringController::DebugRender(DbUtil::I_DebugRender* debugRender, const Maths::Mat43& modelWorldMatrix, F32 rotationAngle) const
{
	// Draw reference wheel.
	debugRender->SetDepthEnable(false);
	
	Maths::Mat43 refMatrix = Maths::Inverse(m_invRefMatrix);
	
	Maths::Normal3 normal;
	switch (m_normalAxis)
	{
		case E_Axis_X: normal = Maths::Const<Maths::Normal3>::AxisX(); break;
		case E_Axis_Y: normal = Maths::Const<Maths::Normal3>::AxisY(); break;
		default:
		case E_Axis_Z: normal = Maths::Const<Maths::Normal3>::AxisZ(); break;
	}
	
	const Maths::RGBA refColour(0x80, 0x80, 0x80);
	const Maths::RGBA targetColour(0xff, 0xff, 0x80);
	
	debugRender->Draw3dCircle(
		Maths::HVec3(Maths::MakePoint3(Maths::RowW(refMatrix)) * modelWorldMatrix),
		Maths::HVec3(normal * refMatrix * modelWorldMatrix),
		Maths::Reciprocal(m_oneOverRefRadius),
		refColour
	);
		
	debugRender->Draw3dCircle(
		Maths::HVec3(Maths::MakePoint3(Maths::RowW(m_targetMatrix)) * modelWorldMatrix),
		Maths::HVec3(normal * m_targetMatrix * modelWorldMatrix),
		m_targetRadius,
		targetColour
	);

	Maths::Mat33 wheelRotation = Maths::MakeMat33_AxisAngle(normal, Maths::MakeVF32(rotationAngle));
	
	debugRender->Draw3dMatrix(
		Maths::HMat43(refMatrix * modelWorldMatrix),
		0.1f
	);
		
	debugRender->Draw3dMatrix(
		Maths::HMat43(wheelRotation * m_targetMatrix * modelWorldMatrix),
		0.1f
	);

	// pos, dir, scale, arrowHeadSize, colour

	Maths::Normal3 barAxis;
	switch (m_normalAxis)
	{
		case E_Axis_X: barAxis = Maths::Const<Maths::Normal3>::AxisY(); break;
		case E_Axis_Y: barAxis = Maths::Const<Maths::Normal3>::AxisZ(); break;
		default:
		case E_Axis_Z: barAxis = Maths::Const<Maths::Normal3>::AxisX(); break;
	}

	Maths::Vector3 bar = Maths::MakeVF32(Maths::Reciprocal(m_oneOverRefRadius)) * barAxis;

	Maths::Point3 centre = Maths::Const<Maths::Point3>::Zero();
	debugRender->Draw3dArrow(
		Maths::HVec3((centre - bar) * wheelRotation * m_targetMatrix * modelWorldMatrix),
		Maths::HVec3(bar * wheelRotation * m_targetMatrix * modelWorldMatrix),
		2.f,
		0.1f,
		targetColour
		);

	debugRender->SetDepthEnable(true);
}
#endif

} // end Animation
