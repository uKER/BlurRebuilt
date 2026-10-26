#include "stdafx.h"
#include "Dynamic/Light/Generic/Shape3dQuad.h"

namespace DynamicEffect
{
namespace Light
{

void Shape3dQuad::CreateInstance(const InstanceDefinition* pDefinition, Light* pLight)
{
	Instance3d_P* pInstance = NewInstance();

	// calculate position
	const Maths::HVec3 vLightPosition = Maths::Load<Maths::HVec3>(&pLight->m_vPosition);
	const Maths::C_Quaternion qLightRotation = Maths::Load<Maths::C_Quaternion>(&pLight->m_vRotation);
	const Maths::HVec3 vInstancePosition = Maths::Load<Maths::HVec3>(&pDefinition->m_vTransform0);
	const Maths::HVec3 vPosition = vLightPosition + vInstancePosition * qLightRotation;
	const Maths::HVec2 vSize = Maths::Load<Maths::HVec2>(&pDefinition->m_vSize);

	Maths::Store(vPosition, &pInstance->m_vPosition);
	pInstance->m_fIntensity		= pDefinition->m_fIntensity;
	pInstance->m_iTexture		= pDefinition->m_iTexture;
	pInstance->m_vDimensions	= pDefinition->m_vSize;
	pInstance->m_hLight			= LightHandle(pLight);
	pInstance->m_pNode			= pLight->m_pNode;
	pInstance->m_pEffect		= this;
	
	if (pInstance->m_pNode)
		++Base::m_nBlocksLocal;

	pInstance->SetBounds(Maths::C_Sphere(vPosition, Maths::Max(vSize.X(), vSize.Y())), m_RenderSettings.fCullDistance);
}

void Shape3dQuad::UpdateExtents(const InstanceDefinition* pDefinition, const Maths::C_QuatTrans& transform, Maths::HVec3& vMin, Maths::HVec3& vMax, Maths::HF32& vCullDistance)
{
	// calculate position
	const Maths::HVec3 vLightPosition			= transform.Translation();
	const Maths::C_Quaternion qLightRotation	= transform.Quaternion();
	const Maths::HVec3 vInstancePosition		= Maths::Load<Maths::HVec3>(&pDefinition->m_vTransform0);
	const Maths::HVec3 vPosition				= vLightPosition + vInstancePosition * qLightRotation;
	const Maths::HVec2 vSize					= Maths::Load<Maths::HVec2>(&pDefinition->m_vSize);
	const Maths::HF32 vSizeMax					= Maths::Max(vSize.X(), vSize.Y());
	const Maths::HVec3 vRadius					= Maths::HVec3(vSizeMax, vSizeMax, vSizeMax);

	vMin.MinEq(vPosition - vSize);
	vMax.MaxEq(vPosition + vSize);
	vCullDistance.MaxEq(Maths::HF32(m_RenderSettings.fCullDistance));
}

}; // namespace Light
}; // namespace DynamicEffect
