#include "stdafx.h"
#include "Dynamic/Light/Generic/Shape2dBeam.h"

namespace DynamicEffect
{
namespace Light
{

void Shape2dBeam::CreateInstance(const InstanceDefinition* pDefinition, Light* pLight)
{
	Instance2d* pInstance = NewInstance();

	pInstance->m_vXTransform2d	= pDefinition->m_vTransform0;
	pInstance->m_vYTransform2d	= pDefinition->m_vTransform1;
	pInstance->m_fIntensity		= pDefinition->m_fIntensity;
	pInstance->m_iTexture		= pDefinition->m_iTexture;
	pInstance->m_vDimensions	= pDefinition->m_vSize;
	pInstance->m_hLight			= LightHandle(pLight);
	pInstance->m_pNode			= pLight->m_pNode;
	pInstance->m_pEffect		= this;
	
	if (pInstance->m_pNode)
		++Base::m_nBlocksLocal;

	// for the bounds, use the bounds of the light - if the light is not visible, then neither will the flare be!
	Maths::HVec3 vPosition	= Maths::Load<Maths::HVec3>(&pLight->m_vPosition);
	Maths::HVec3 vSize		= Maths::Load<Maths::HVec3>(&pLight->m_pDefinition->m_vSize);
	pInstance->SetBounds(Maths::C_Sphere(vPosition, vSize.Length()), m_RenderSettings.fCullDistance);
}

void Shape2dBeam::UpdateExtents(const InstanceDefinition* pDefinition, const Maths::C_QuatTrans& transform, Maths::HVec3& vMin, Maths::HVec3& vMax, Maths::HF32& vCullDistance)
{
}

}; // namespace Light
}; // namespace DynamicEffect
