#ifndef RENDER_DYNAMICEFFECT_TRACER_EFFECTINTERFACE_H
#define RENDER_DYNAMICEFFECT_TRACER_EFFECTINTERFACE_H

#if _MSC_VER > 1000
#pragma once
#endif

#define SCENEGRAPH_ALLOW_INCLUDE // only for EffectInterface.h (need to fix this)

#include "Dynamic/Interface.h"
#include "Dynamic/Renderer.h"
#include "Dynamic/Tracer/Enums.h"
#include "Dynamic/Tracer/Manager.h"
#include "Render/View/ViewManager.h"
#include "SceneGraph/SceneGraphImpl.h"
#include "SceneGraph/View.h"

namespace DynamicEffect
{

class C_Node;

namespace Tracer
{

struct RenderSettings
{
	Display::C_RefTexture*			pTexture;
	E_PixelShader ePixelShader;
	MathsStorage::Float2 vSize;
	F32 fFade;
};

class I_EffectBase : public I_DynamicEffect
{
public:
	I_EffectBase(C_Renderer* pRenderer, I_DynamicEffectManager* pManager, const U32 iIndex, const U32 iTypeIndex);
	virtual ~I_EffectBase();
	
	// interface methods
	virtual void Destroy();

	// virtual methods
	virtual void Create(const RenderSettings& renderSettings) = 0;
	virtual void Spawn(const C_Node* pNode, const Maths::C_RGBA_F32& vColour) = 0;

	// accessors
	const RenderSettings&	GetRenderSettings() const { return m_RenderSettings; }

	S32						m_EffectPosition;
	friend class			C_Manager;

protected:
	RenderSettings			m_RenderSettings;

};

template <typename TracerType, template <typename TracerType> class ShapePolicy>
class I_Effect : public I_EffectBase
{
public:
	I_Effect(C_Renderer* pRenderer, I_DynamicEffectManager* pManager, const U32 iIndex, const U32 iTypeIndex) : I_EffectBase(pRenderer, pManager, iIndex, iTypeIndex) {}
	virtual ~I_Effect() {}

	// virtual methods
	void	Create(const RenderSettings& renderSettings);
	void	Spawn(const C_Node* pNode, const Maths::C_RGBA_F32& vColour) = 0;

	// interface methods
	void	OnRender(const C_View& view, Display::C_Context* pDisplay, const C_CompositeRenderNode& node, const U32 states) const;
	void	OnBuild(const C_View& view, C_CompositeRenderNode& node) const;

protected:

	// interface methods
	virtual void		Reset();
	virtual void		OnGather(const SceneGraph::C_View& view) const;
	virtual void		Clean(const C_Node* pNode);
	virtual void		Clean(const Display::C_Texture2D* pTexture);

protected:

	typedef Ut::Vector<TracerType> Tracers;
	Tracers m_Tracers;

	ShapePolicy<TracerType> m_ShapePolicy;

public:

	FORCE_INLINE TracerType * NewTracer()
	{
		++m_nBlocks;
		++m_nBlocksLocal;

		m_Tracers.PushBack();
		return &m_Tracers.Back();
	}

	FORCE_INLINE void FreeTracer(const U32 tracerNo)
	{
		--m_nBlocks;
		--m_nBlocksLocal;

		m_Tracers[tracerNo] = m_Tracers.Back();
		m_Tracers.PopBack();
	}

	FORCE_INLINE void ClearTracers()
	{
		m_nBlocks = 0;
		m_nBlocksLocal = 0;
		m_Tracers.Clear();
	}

};

template<typename TracerType, template<typename TracerType> class ShapePolicy>
FORCE_INLINE void I_Effect<TracerType, ShapePolicy>::Create(const RenderSettings& renderSettings)
{
	m_RenderSettings = renderSettings;
	m_ShapePolicy.SetRenderSettings(&m_RenderSettings);
}

template<typename TracerType, template<typename TracerType> class ShapePolicy>
FORCE_INLINE void I_Effect<TracerType, ShapePolicy>::Reset()
{
	m_Tracers.Clear();
}

template<typename TracerType, template<typename TracerType> class ShapePolicy>
FORCE_INLINE void I_Effect<TracerType, ShapePolicy>::OnBuild(const C_View& view, C_CompositeRenderNode& node) const
{
	node.iVertexBuffer = 0xFFFFFFFF;
}

template<typename TracerType, template<typename TracerType> class ShapePolicy>
FORCE_INLINE void I_Effect<TracerType, ShapePolicy>::OnRender(const C_View& view, Display::C_Context* pDisplay, const C_CompositeRenderNode &node, const U32 states) const
{
	C_Manager* pManager = (C_Manager*) m_pManagerInterface;

	U32 vertexCount = node.blocks.Size() * m_ShapePolicy.GetNumPrimitives() * m_ShapePolicy.GetNumVertices();

	Display::Device()->SetTexture(0, m_RenderSettings.pTexture);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Wrap);

	Display::Device()->SetBlendMode(Display::E_BlendMode_Additive);
	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetDepthEnable(Display::E_Bool_True);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);

	Display::Device()->SetVertexDeclaration(m_ShapePolicy.GetVertexDeclaration());
	pManager->SetVertexShader();
	pManager->SetPixelShader(m_RenderSettings.ePixelShader);

	typename ShapePolicy<TracerType>::ShapeVertex* pVertices = (typename ShapePolicy<TracerType>::ShapeVertex *) Display::Device()->BeginVertices(m_ShapePolicy.GetPrimitiveType(), vertexCount, sizeof(typename ShapePolicy<TracerType>::ShapeVertex), m_ShapePolicy.GetVertexDeclaration(), true); // This can fail
	if(pVertices)
	{
		m_ShapePolicy.Build(node.blocks, pVertices);
		Display::Device()->EndVertices();
	}

	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_Default);
	Display::Device()->SetDepthEnable(Display::E_Bool_Default);
}

template<typename TracerType, template<typename TracerType> class ShapePolicy>
FORCE_INLINE void I_Effect<TracerType, ShapePolicy>::OnGather(const SceneGraph::C_View& view) const
{
	for (S32 tracerNo = 0; tracerNo < m_Tracers.Size(); ++tracerNo)
	{
		const TracerType& instance = m_Tracers[tracerNo];

		const DynamicEffect::C_View& dynamicView = GetRenderer()->GetView(view.GetIndex());

		Maths::HVec4 vCentreOne(Maths::Load<Maths::V128>(&instance.m_vCentre));
		Maths::HVec4 vSizeCull(Maths::Load<Maths::V128>(&instance.m_vSize));

		Maths::HMat43 mTransform;
		if (instance.m_pNode)
			mTransform = Maths::C_QuatTrans(*instance.m_pNode).Matrix();

		Maths::HVec3 vCentre3(vCentreOne);
		Maths::HVec3 vSize3(vSizeCull);
		Maths::HF32 vLength = Maths::Length(vSize3);

		Maths::HVec4 vCentre(vCentre3 * mTransform);
		Maths::HVec4 vSize(vSize3 * mTransform.Rotation());

		vCentre.W(vCentreOne.W());
		vSize.W(vSizeCull.W());

		S32 visible = view.GetViewFrustum().CullSphere(vCentre, vLength);

		const S32 renderPassType = GetRenderer()->GetRenderPassMapping(DynamicEffect::E_RenderPassIndex_Transparent);
		if(renderPassType != -1)
		{
			const SceneGraph::C_RenderPass* pRenderPass = view.GetSceneGraph()->GetRenderPass(renderPassType);

			SceneGraph::VolatileNodeInfo volatileNodeInfo;

			// calculate depth if we need it
			if(pRenderPass->GetVolatileInfoFlags() & SceneGraph::E_VolatileNodeInfo_Distance)
			{
				Maths::HF32 farClip(view.GetViewData().farClip);
				const Display::C_Matrix& viewMatrix = view.GetViewSettings3D().GetViewMatrix();
				Maths::HVec4 forward(Maths::Load<Maths::HVec4>(&view.GetViewData().forwardZ));
				Maths::HF32 depthScale = 4294967296.0f / farClip;
				Maths::HF32 depth = (vCentreOne | forward) * depthScale;
				
				volatileNodeInfo.distance = (U32)depth;
			}
			view.AddVisibleNode(&instance, renderPassType, 0, volatileNodeInfo.data);
		}
	}
/*	for (S32 instanceNo = 0; instanceNo < m_Instances.Size(); ++instanceNo)
	{
		const InstanceType& instance = m_Instances[instanceNo];

		Maths::HVec4 vCentreOne(Maths::Load<Maths::V128>(&instance.m_vCentre));
		Maths::HVec4 vSizeCull(Maths::Load<Maths::V128>(&instance.m_vSize));

		Maths::HMat43 mTransform;
		if (instance.m_pNode)
			mTransform = Maths::C_QuatTrans(*instance.m_pNode).Matrix();

		Maths::HVec3 vCentre3(vCentreOne);
		Maths::HVec3 vSize3(vSizeCull);
		Maths::HF32 vLength = Maths::Length(vSize3);

		Maths::HVec4 vCentre(vCentre3 * mTransform);
		Maths::HVec4 vSize(vSize3 * mTransform.Rotation());

		vCentre.W(vCentreOne.W());
		vSize.W(vSizeCull.W());

		S32 visible = view.GetViewFrustum().CullSphere(vCentre, vLength);

		if (visible)
		{
			const DynamicEffect::C_View& dynamicView = GetRenderer()->GetView(view.GetIndex());

			// if this gets resized then any previous light pointers could be invalid
			if(dynamicView.m_RenderNodes.Size() == dynamicView.m_RenderNodes.Capacity())
				dynamicView.m_RenderNodesResize = true;
			else
			{
				C_RenderNode renderNode(m_pRenderer->GetNodeType());
				renderNode.pBlock = &instance;
				renderNode.pEffect = this;
				renderNode.pMatrix = NULL;
				renderNode.data = 0;
	
				dynamicView.m_RenderNodes.PushBack(renderNode);

				S32 renderPassType = 0;
				if (inWorldSpace)
					renderPassType = GetRenderer()->GetRenderPassMapping(DynamicEffect::E_RenderPassIndex_Transparent);
				else
					renderPassType = GetRenderer()->GetRenderPassMapping(DynamicEffect::E_RenderPassIndex_TwoDimensional);
	
				if(renderPassType != -1)
				{
					const SceneGraph::C_RenderPass* pRenderPass = view.GetSceneGraph()->GetRenderPass(renderPassType);

					S32 volatileNodeInfoIndex = -1;

					// calculate depth if we need it
					if(pRenderPass->GetVolatileInfoFlags() & SceneGraph::E_VolatileNodeInfo_Distance)
					{
						Maths::HF32 farClip(view.GetViewSettings3D().GetFarClip());
						const Display::C_Matrix& viewMatrix = view.GetViewSettings3D().GetViewMatrix();
						Maths::HVec4 forward(viewMatrix._13, viewMatrix._23, viewMatrix._33, viewMatrix._43);
						Maths::HF32 depthScale = 4294967296.0f / farClip;
						Maths::HF32 depth = (vCentreOne | forward) * depthScale;

						SceneGraph::C_VolatileNodeInfo volatileNodeInfo;
						volatileNodeInfo.distance = (U32)depth;
						volatileNodeInfoIndex = view.AddVolatileNodeInfo(volatileNodeInfo);
					}
					view.AddVisibleNode(&dynamicView.m_RenderNodes.Back(), renderPassType, volatileNodeInfoIndex);
				}
			}
		}
	} */
}

/*
template<typename TracerType, template<typename TracerType> class ShapePolicy>
FORCE_INLINE void I_Effect<TracerType, ShapePolicy>::OnGatherBlocks(DynamicEffect::BlockArrays &blockArrays, const U32 iViewIndex) const
{
	Maths::HVec4 vViewDirection(Maths::Load<Maths::HVec4>(&m_pRenderer->GetViewDirection()));
	Maths::HF32 vDepthScale(Maths::Load<Maths::HF32>(&m_pRenderer->GetViewPosition().w));
	Maths::HF32 vMaxInt = (F32)0xFFFFFFFF;
	Maths::HF32 vZero(Maths::Constants<Maths::HF32>::Zero());
	Maths::HF32 vHalf(Maths::Constants<Maths::HF32>::Half());
	Maths::HF32 vOne(Maths::Constants<Maths::HF32>::One());

	Maths::HVec2 vSize(Maths::Load<Maths::HVec2>(&m_RenderSettings.vSize));
	Maths::HVec3 vRadius(vSize.X(), vSize.X(), vSize.X());

	for (S32 tracerNo = 0; tracerNo < m_Tracers.Size(); ++tracerNo)
	{
		const TracerType& tracer = m_Tracers[tracerNo];

		Maths::HMat43 mTransform;
		if (tracer.m_pNode)
			mTransform = Maths::C_QuatTrans(*tracer.m_pNode).Matrix();

		Maths::HVec3 vPosition1 = Maths::Load<Maths::HVec3>(&tracer.m_vPrevious);
		Maths::HVec3 vPosition0 = mTransform.AW();
		Maths::HF32  vVelocity  = Maths::HVec3(vPosition1 - vPosition0).Length();
		Maths::HF32  vFinalLength = vSize.Y() * vVelocity;

		Maths::HVec3 vCentre3(vZero, -vFinalLength * vHalf, vZero);
		Maths::HVec3 vSize3(vRadius - vCentre3);

		Maths::HVec4 vCentre(vCentre3 * mTransform);
		Maths::HF32  vLength(vSize3.Length());
		vCentre.W(vOne);
		
		S32 visible = Render::C_ViewManager::Instance().GetView(iViewIndex)->m_View.GetViewFrustum().CullSphere(vCentre, vLength);
		
		if (visible)
		{
			C_RenderNode block;
			block.pBlock = &tracer;
			block.pMatrix = 0;
			block.pInstance = 0;

			// get a sort key for depth sorting
			Maths::HVec4 vCentreOne(Maths::Load<Maths::HVec4>(&tracer.m_vCentre));
			Maths::HMat43 mTransform;
			if (tracer.m_pNode)
				mTransform = Maths::C_QuatTrans(*tracer.m_pNode).Matrix();
			
			Maths::HVec4 vCentre(Maths::HVec3(vCentreOne) * mTransform);
			vCentre.W(vCentreOne.W());
			Maths::HF32 vDepth = (vCentre | vViewDirection);
			vDepth = vMaxInt - vDepth * vDepthScale;
			block.sortKey = (U32)vDepth;

			blockArrays.transparent.PushBack(block);
		}
	}
}*/

template<typename TracerType, template<typename TracerType> class ShapePolicy>
FORCE_INLINE void I_Effect<TracerType, ShapePolicy>::Clean(const C_Node* pNode)
{
	for (S32 tracerNo = 0; tracerNo < m_Tracers.Size(); ++tracerNo)
	{
		const TracerType& tracer = m_Tracers[tracerNo];
		if (tracer.m_pNode == pNode)
		{
			FreeTracer(tracerNo--);
		}
	}
}

template<typename TracerType, template<typename TracerType> class ShapePolicy>
FORCE_INLINE void I_Effect<TracerType, ShapePolicy>::Clean(const Display::C_Texture2D* pTexture)
{
	if (m_RenderSettings.pTexture == pTexture)
	{
		ClearTracers();
		if (m_RenderSettings.pTexture)
		{
			m_RenderSettings.pTexture->Release();
			m_RenderSettings.pTexture = 0;
		}
	}
}

}; // namespace Tracer
}; // namespace DynamicEffect



#endif // RENDER_DYNAMICEFFECT_TRACER_EFFECTINTERFACE_H
