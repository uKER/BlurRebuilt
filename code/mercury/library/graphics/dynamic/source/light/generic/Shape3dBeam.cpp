#include "stdafx.h"
#include "Dynamic/Light/Generic/Shape3dBeam.h"

namespace DynamicEffect
{
namespace Light
{

void Shape3dBeam::CreateInstance(const InstanceDefinition* pDefinition, Light* pLight)
{
	Instance3d_P2* pInstance = NewInstance();

	// calculate position
	const Maths::HVec3 vLightPosition = Maths::Load<Maths::HVec3>(&pLight->m_vPosition);
	const Maths::C_Quaternion qLightRotation = Maths::Load<Maths::C_Quaternion>(&pLight->m_vRotation);
	const Maths::HVec3 vInstancePosition0 = Maths::Load<Maths::HVec3>(&pDefinition->m_vTransform0);
	const Maths::HVec3 vInstancePosition1 = Maths::Load<Maths::HVec3>(&pDefinition->m_vTransform1);
	const Maths::HVec3 vPosition0 = vLightPosition + vInstancePosition0 * qLightRotation;
	const Maths::HVec3 vPosition1 = vLightPosition + vInstancePosition1 * qLightRotation;
	const Maths::HVec2 vSize = Maths::Load<Maths::HVec2>(&pDefinition->m_vSize);
	const Maths::HVec3 vRadius = Maths::HVec3(vSize.X(), vSize.X(), vSize.X());

	Maths::Store(vPosition0, &pInstance->m_vPosition0);
	Maths::Store(vPosition1, &pInstance->m_vPosition1);
	pInstance->m_fIntensity		= pDefinition->m_fIntensity;
	pInstance->m_iTexture		= pDefinition->m_iTexture;
	pInstance->m_vDimensions	= pDefinition->m_vSize;
	pInstance->m_hLight			= LightHandle(pLight);
	pInstance->m_pNode			= pLight->m_pNode;
	pInstance->m_pEffect		= this;

	if (pInstance->m_pNode)
		++Base::m_nBlocksLocal;

	Maths::HVec3 vMin = vPosition0.Min(vPosition1) - vRadius;
	Maths::HVec3 vMax = vPosition0.Max(vPosition1) + vRadius;

	pInstance->SetBounds(Maths::C_AxisBox(vMin, vMax), m_RenderSettings.fCullDistance);
}

void Shape3dBeam::UpdateExtents(const InstanceDefinition* pDefinition, const Maths::C_QuatTrans& transform, Maths::HVec3& vMin, Maths::HVec3& vMax, Maths::HF32& vCullDistance)
{
	// calculate position
	const Maths::HVec3 vLightPosition = transform.Translation();
	const Maths::C_Quaternion qLightRotation = transform.Quaternion();
	const Maths::HVec3 vInstancePosition0 = Maths::Load<Maths::HVec3>(&pDefinition->m_vTransform0);
	const Maths::HVec3 vInstancePosition1 = Maths::Load<Maths::HVec3>(&pDefinition->m_vTransform1);
	const Maths::HVec3 vPosition0 = vLightPosition + vInstancePosition0 * qLightRotation;
	const Maths::HVec3 vPosition1 = vLightPosition + vInstancePosition1 * qLightRotation;
	const Maths::HVec2 vSize = Maths::Load<Maths::HVec2>(&pDefinition->m_vSize);
	const Maths::HVec3 vRadius = Maths::HVec3(vSize.X(), vSize.X(), vSize.X());
	
	vMin.MinEq(vPosition0.Min(vPosition1) - vRadius);
	vMax.MaxEq(vPosition0.Max(vPosition1) + vRadius);
	vCullDistance.MaxEq(Maths::HF32(m_RenderSettings.fCullDistance));
}

}; // namespace Light
}; // namespace DynamicEffect
