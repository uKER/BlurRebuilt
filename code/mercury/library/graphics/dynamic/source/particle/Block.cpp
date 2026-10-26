#include "stdafx.h"

#include "Dynamic/Particle/Block.h"
#include "Dynamic/Particle/Particle.h"

#ifndef PLATFORM_SPU
#include "Dynamic/Particle/EffectInterface.h"
#include "Dynamic/Node.h"
#include "Dynamic/Interface.h"
#include "Dynamic/View.h"
#include "Dynamic/Renderer.h"

#define SCENEGRAPH_ALLOW_INCLUDE	// only for Block.cpp
#include "SceneGraph/SceneGraphImpl.h"
#include "SceneGraph/View.h"
#endif

namespace DynamicEffect
{
namespace Particle
{

#ifndef PLATFORM_SPU
void ParticleBlock::Gather(const SceneGraph::C_View& view) const
{
	// don't gather empty blocks
	if(m_Element.Size() == 0)
		return;
	
	if ((m_VisibilityFlags & view.GetTypeFlag()) == 0)
		return;

	Maths::HVec4 vCentreOne(Maths::Load<Maths::HVec4>(&m_vCentre));
	Maths::HVec4 vSizeCull(Maths::Load<Maths::HVec4>(&m_vSize));
	Maths::HVec3 vCentre3(vCentreOne);
	Maths::HVec3 vSize3(vSizeCull);

	if(m_pNode)
	{
		Maths::HMat43 mTransform(Maths::C_QuatTrans(*m_pNode).Matrix());
		vCentre3 = vCentre3 * mTransform;
		vSize3 = vSize3 * mTransform.Rotation();
	}
	Maths::HVec4 vCentre(vCentre3);
	Maths::HVec4 vSize(vSize3);
	Maths::HF32 fSphereRadius(Maths::Length(vSize3));
	Maths::HF32 pixelCostHalf(128.f);
	Maths::HF32 pixelCostMax(256.f);

	vCentre.W(vCentreOne.W());
	vSize.W(vSizeCull.W());

	S32 visible;

	if(m_pNode)
		visible = view.GetViewFrustum().CullSphere(vCentre, fSphereRadius, vSizeCull.W());
	else
		visible = view.GetViewFrustum().CullAxisBox(vCentre, vSize);

	if(visible)
	{
		const DynamicEffect::C_View& dynamicView = m_pEffect->GetRenderer()->GetView(view.GetIndex());

		S32 renderPassType = 0;
		const I_ParticleEffectBase* pParticleEffect = (const I_ParticleEffectBase*)m_pEffect;
		if(pParticleEffect->GetRenderSettings().blendModeTransparent)
			renderPassType = m_pEffect->GetRenderer()->GetRenderPassMapping(DynamicEffect::E_RenderPassIndex_Transparent);
		else
			renderPassType = m_pEffect->GetRenderer()->GetRenderPassMapping(DynamicEffect::E_RenderPassIndex_Opaque);

		if(renderPassType != -1)
		{
			const SceneGraph::C_RenderPass* pRenderPass = view.GetSceneGraph()->GetRenderPass(renderPassType);

			SceneGraph::VolatileNodeInfo volatileNodeInfo;
			F32 depthKey = 0;
			F32 screenSizeKey = 0;

			// calculate depth if we need it
			if(pRenderPass->GetVolatileInfoFlags() & SceneGraph::E_VolatileNodeInfo_Distance)
			{
				Maths::HF32 depthBias(pParticleEffect->GetRenderStates().fDepthBias);
				Maths::HVec4 forward(Maths::Load<Maths::HVec4>(&view.GetViewData().forwardZ));
				Maths::HF32 depthScale(view.GetViewData().depthScale);
				Maths::HF32 depth = Maths::Max(((vCentre | forward) + depthBias), Maths::Constants<Maths::HF32>::Zero());
				depth *= depthScale;

				Maths::Store(depth, &depthKey);
			}
			if(pRenderPass->GetVolatileInfoFlags() & SceneGraph::E_VolatileNodeInfo_ScreenSize)
			{
				Maths::HVec4 forward(Maths::Load<Maths::HVec4>(&view.GetViewData().forwardZ));
				Maths::HF32 depth = (vCentre | forward);
				Maths::HVec4 coord(fSphereRadius, Maths::Constants<Maths::HF32>::Zero(), depth, Maths::Constants<Maths::HF32>::One());
				coord = coord * view.GetViewData().viewSettings3D.GetCameraToDevice();
				Maths::HF32 screenSize = Maths::Abs(coord.X() / coord.W()) * pixelCostHalf;
				screenSize = Maths::Clamp(screenSize, Maths::Constants<Maths::HF32>::Zero(), pixelCostMax);
				screenSizeKey = screenSize;
			}

			volatileNodeInfo.distance = (U32)depthKey;
			view.AddVisibleNode(this, renderPassType, 0, volatileNodeInfo.data, (U32)screenSizeKey);
		}
	}
}
#endif

void ParticleBlock::CalculateExtents(F32 cullDistance, U32 vertexSize)
{
	DCBT(0, &m_Element[0]);
	DCBT(128, &m_Element[0]);

	Maths::HVec3 vMax(Maths::HF32(-10000.0f));
	Maths::HVec3 vMin(Maths::HF32(10000.0f));
	Maths::HF32 fSizeMax(0.0f);
	
	U32 iBatch = m_Element.Size() & ~1;

	U32 i = 0;
	for(i = 0; i < iBatch; i+=2)
	{
		DCBT(128, &m_Element[i]);
		DCBT(256, &m_Element[i]);

		Maths::HVec3 vPosition0(Maths::Load<Maths::HVec3>(&((C_Particle3D*)&m_Element[i])->vPosition));
		Maths::HVec3 vPosition1(Maths::Load<Maths::HVec3>(&((C_Particle3D*)&m_Element[i+1])->vPosition));
		
		fSizeMax = fSizeMax.Max(((C_Particle3D*)&m_Element[i])->vForceSizeLife.z);
		fSizeMax = fSizeMax.Max(((C_Particle3D*)&m_Element[i+1])->vForceSizeLife.z);
		
		vMin = vMin.Min(vPosition1.Min(vPosition0));
		vMax = vMax.Max(vPosition1.Max(vPosition0));
	}
	for(i = i; i < (U32)m_Element.Size(); ++i)
	{
		Maths::HVec3 vPosition(Maths::Load<Maths::HVec3>(&((C_Particle3D*)&m_Element[i])->vPosition));
		fSizeMax = fSizeMax.Max(((C_Particle3D*)&m_Element[i])->vForceSizeLife.z);

		vMax = vMax.Max(vPosition);
		vMin = vMin.Min(vPosition);
	}

	Maths::HVec3 vSizeMax(fSizeMax);
	vMax += vSizeMax;
	vMin -= vSizeMax;

	Maths::HVec3 vCentre = (vMax + vMin) * Maths::Constants<Maths::HF32>::Half();
	Maths::HVec3 vSize = vMax - vCentre;

	Maths::HVec4 vCentre2(vCentre);
	Maths::HVec4 vSize2(vSize);
	vSize2.W(cullDistance);
	vCentre2.W(Maths::Constants<Maths::HF32>::One());
	
	Maths::Store(vCentre2, &m_vCentre);
	Maths::Store(vSize2, &m_vSize);

	m_VertexDataSize = m_Element.Size() * vertexSize;
}

void ParticleBlock::FreeParticle(const U32 i)
{
	// get the last particle of the last block and copy it here
	void* pSrc = (void*)&m_Element[m_Element.Size()-1];
	void* pDest = (void*)&m_Element[i];
	memcpy(pDest, pSrc, m_Element.ElementSize());
	m_Element.PopBack();
}

} // Particle
} // DynamicEffect
