#include "stdafx.h"

// fog
#include "Dynamic/Fog/EffectInterface.h"
#include "Dynamic/Fog/Enums.h"
#include "Dynamic/Fog/Manager.h"

// dynamic
#include "Dynamic/Node.h"
#include "Dynamic/Renderer.h"
#include "Dynamic/RenderNode.h"

// render
#include "Render/View/ViewManager.h"

namespace DynamicEffect
{
namespace Fog
{

Debug::Float g_debugCameraInsideEpsilon("Render.Fog.Volumes.CameraInsideEpsilon", 0.2f, Debug::Range(0.0f, 1.0f));

I_VolumeEffect::I_VolumeEffect(C_Renderer* pRenderer, I_DynamicEffectManager* pManagerInterface, const U32 iIndex, const U32 iTypeIndex)
	: I_DynamicEffect(pRenderer, pManagerInterface, iIndex, iTypeIndex)
{
	C_Manager* pManager = (C_Manager*) m_pManagerInterface;
	pManager->AddEffect(this);
	pRenderer->AddEffect(this);

	m_eDraw = E_Draw_Back;
	m_eDetail = E_Detail_PerPixel;
}

I_VolumeEffect::~I_VolumeEffect()
{
	C_Manager* pManager = (C_Manager*) m_pManagerInterface;
	pManager->RemoveEffect(this);
	m_pRenderer->RemoveEffect(this);

	ClearVolumes();
}

void I_VolumeEffect::OnBuild(const C_View& view, C_CompositeRenderNode& node) const
{
	node.iVertexBuffer = 0xFFFFFFFF;
}

void I_VolumeEffect::OnGather(const SceneGraph::C_View& view) const
{

}

/*
void I_VolumeEffect::OnGatherBlocks(DynamicEffect::BlockArrays &blockArrays, const U32 iViewIndex) const
{
	Maths::HVec4 vViewDirection(Maths::Load<Maths::HVec4>(&m_pRenderer->GetViewDirection()));
	Maths::HF32 vDepthScale(Maths::Load<Maths::HF32>(&m_pRenderer->GetViewPosition().w));
	Maths::HF32 vMaxInt = (F32)0xFFFFFFFF;

	// Persistent instances will have visbits available
	for (S32 i = 0; i < m_Volumes.Size(); ++i)
	{
		const Volume* pVolume = m_Volumes[i];

		Maths::HVec4 vCentre(Maths::Load<Maths::HVec4>(&pVolume->m_vCentre));
		Maths::HVec4 vSizeCull(Maths::Load<Maths::HVec4>(&pVolume->m_vSize));
		if (pVolume->m_pNode)
		{
			vCentre = Maths::HVec3(vCentre) * Maths::C_QuatTrans(*pVolume->m_pNode).Matrix();
			vCentre.W(Maths::Constants<Maths::HF32>::One());
		}
		Maths::HF32 vLength = Maths::Length(Maths::HVec3(vSizeCull));

		S32 visible = Render::C_ViewManager::Instance().GetView(iViewIndex)->m_View.GetViewFrustum().CullSphere(vCentre, vLength, vSizeCull.W());
				
		if (visible)
		{
			C_RenderNode block(E_RenderNodeType_Fog);
			block.pBlock = pVolume;
			block.pMatrix = NULL;
			block.pInstance = NULL;

			// get a sort key for depth sorting
			Maths::HVec4 vCentre(Maths::Load<Maths::HVec4>(&pVolume->m_vCentre));
			if (pVolume->m_pNode)
			{
				vCentre = Maths::HVec3(vCentre) * Maths::C_QuatTrans(*pVolume->m_pNode).Matrix();
				vCentre.W(Maths::Constants<Maths::HF32>::One());
			}
			Maths::HF32 vDepth = (vCentre | vViewDirection);
			vDepth = vMaxInt - vDepth * vDepthScale;
			block.sortKey = (U32)vDepth;

			// push onto transparent blocks
			blockArrays.transparent.PushBack(block);
		}
	}
}*/

void I_VolumeEffect::Clean(const C_Node* pNode)
{
	for (S32 volumeNo = 0; volumeNo < m_Volumes.Size(); ++volumeNo)
	{
		if (pNode == m_Volumes[volumeNo]->m_pNode)
			FreeVolume(volumeNo--);
	}
}

void I_VolumeEffect::Update()
{
}

void I_VolumeEffect::Reset()
{
	ClearVolumes();
}

}; // namespace Fog
}; // namespace DynamicEffect
