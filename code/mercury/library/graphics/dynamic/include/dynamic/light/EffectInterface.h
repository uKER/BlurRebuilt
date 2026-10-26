#ifndef RENDER_DYNAMICEFFECT_LIGHT_EFFECT_H
#define RENDER_DYNAMICEFFECT_LIGHT_EFFECT_H

#if _MSC_VER > 1000
#pragma once
#endif

#define SCENEGRAPH_ALLOW_INCLUDE // only for EffectInterface.h (need to fix this)

#include "Render/Billboard/TextureAtlas.h"
#include "Dynamic/Common.h"
#include "Dynamic/Interface.h"
#include "Dynamic/Node.h"
#include "Dynamic/Renderer.h"
#include "Dynamic/Light/Enums.h"
#include "Dynamic/Light/Manager.h"
#include "Dynamic/Shaders/Constants.h"
#include "SceneGraph/SceneGraphImpl.h"
#include "SceneGraph/View.h"

namespace DynamicEffect
{
namespace Light
{

extern Debug::Float g_debugIntensityScale;
extern Debug::Float g_debugGlobalOffsetX;
extern Debug::Float g_debugGlobalOffsetY;

class Light;
class C_Manager;

struct InstanceDefinition;

struct RenderSettings
{
	Display::C_RefTexture* pTexture;
	U32 nAtlasWidth;
	U32	nAtlasHeight;
	bool bDepthEnable;
	F32 fCullDistance;
	F32 fRcpCullDistance;
	// shaders
	E_VertexShader eVertexShader;
	E_PixelShader ePixelShader;
	MathsStorage::Float4 vShaderParams;
	// lookup tables
	LookupTable	viewTable;
	LookupTable distanceTable;
	LookupTable positionTable;
};

class I_EffectBase : public I_DynamicEffect
{
public:
	I_EffectBase(C_Renderer* pRenderer, I_DynamicEffectManager* pManager, const U32 iIndex, const U32 iTypeIndex);
	virtual ~I_EffectBase();
	
	// virtual methods
	virtual void Create(const RenderSettings& renderSettings) = 0;
	virtual void CreateInstance(const InstanceDefinition* pDefinition, Light* pLight) = 0;
	virtual void UpdateExtents(const InstanceDefinition* pDefinition, const Maths::C_QuatTrans& transform, Maths::HVec3& vMin, Maths::HVec3& vMax, Maths::HF32& vCullDistance) = 0;
	virtual void Clean(const Light* pLight) = 0;

	// standard methods
	void Destroy();

	// accessors
	const RenderSettings&					GetRenderSettings()					const { return m_RenderSettings; }
	const Render::Billboard::TextureCoords&	GetTextureCoords(const U32 iIndex)	const { return m_TextureAtlas.GetTextureCoords(iIndex); }

protected:

	RenderSettings						m_RenderSettings;
	Render::Billboard::C_TextureAtlas	m_TextureAtlas;

public:

	FORCE_INLINE Maths::HF32 GetLightIntensity(const Maths::HF32& vIntensity) const
	{
		return vIntensity * g_debugIntensityScale;
	}

	FORCE_INLINE Maths::HF32 GetViewIntensity(const Maths::HF32& vViewAngle) const
	{
		return LookupValue(m_RenderSettings.viewTable, vViewAngle).X();
	}

	FORCE_INLINE Maths::HF32 GetDistanceIntensity(const Maths::HF32& vDistance) const
	{
		Maths::HF32 vScaledDistance = vDistance * m_RenderSettings.fRcpCullDistance;
		vScaledDistance.MinEq(Maths::Constants<Maths::HF32>::One());
		vScaledDistance.MaxEq(Maths::Constants<Maths::HF32>::Zero());
		return LookupValue(m_RenderSettings.distanceTable, vScaledDistance).X();
	}

	FORCE_INLINE Maths::HF32 GetPositionIntensity(const Maths::HVec2& vLight, const Maths::HVec2& vInstance) const
	{
		Maths::HF32 vLength = (vLight - vInstance).Length();
		vLength *= Maths::Constants<Maths::HF32>::Sqrt2().Rcp();
		vLength.MinEq(Maths::Constants<Maths::HF32>::One());
		vLength.MaxEq(Maths::Constants<Maths::HF32>::Zero());
		return LookupValue(m_RenderSettings.positionTable, vLength).X();
	}

	FORCE_INLINE Maths::HVec2 GetGlobalOffset() const
	{
		return Maths::HVec2((F32)g_debugGlobalOffsetX, (F32)g_debugGlobalOffsetY);
	}

};

template <typename InstanceType, template <typename InstanceType> class ShapePolicy, bool inWorldSpace>
class I_Effect : public I_EffectBase
{
public:
	I_Effect(C_Renderer* pRenderer, I_DynamicEffectManager* pManager, const U32 iIndex, const U32 iTypeIndex) : I_EffectBase(pRenderer, pManager, iIndex, iTypeIndex) {}
	virtual ~I_Effect() {}

	// virtual methods
	virtual void		Create(const RenderSettings& renderSettings);
	virtual void		CreateInstance(const InstanceDefinition* pDefinition, Light* pLight) = 0;
	virtual void		UpdateExtents(const InstanceDefinition* pDefinition, const Maths::C_QuatTrans& transform, Maths::HVec3& vMin, Maths::HVec3& vMax, Maths::HF32& vCullDistance) = 0;
	virtual void		Clean(const Light* pLight);

	// interface methods
	void	OnRender(const C_View& view, Display::C_Context* pDisplay, const C_CompositeRenderNode& node, const U32 states) const;
	void				OnBuild(const C_View& view, C_CompositeRenderNode& node) const;

protected:

	// interface methods
	virtual void		Update();
	virtual void		Reset();
	virtual void		OnGather(const SceneGraph::C_View& view) const;
	virtual void		Clean(const C_Node* pNode);
	virtual void		Clean(const Display::C_Texture2D* pTexture);

protected:

	typedef Ut::Vector<InstanceType> Instances;
	Instances m_Instances;

	ShapePolicy<InstanceType> m_ShapePolicy;

public:

	FORCE_INLINE InstanceType* NewInstance()
	{
		++m_nBlocks;

		m_Instances.PushBack();
		InstanceType& back = m_Instances.Back();
		back.SetNodeType(m_pRenderer->GetNodeType());
		back.m_pEffect = this;
		return &back;
	}

	FORCE_INLINE void FreeInstance(const U32 instanceNo)
	{
		--m_nBlocks;
		if (m_Instances[instanceNo].m_pNode)
			--m_nBlocksLocal;

		m_Instances[instanceNo] = m_Instances.Back();
		m_Instances.PopBack();
	}

	FORCE_INLINE void ClearInstances()
	{
		m_nBlocks = 0;
		m_Instances.Clear();
	}

};

template<typename InstanceType, template<typename InstanceType> class ShapePolicy, bool inWorldSpace>
FORCE_INLINE void I_Effect<InstanceType, ShapePolicy, inWorldSpace>::Create(const RenderSettings& renderSettings)
{
	m_RenderSettings = renderSettings;
	m_TextureAtlas.Resize(renderSettings.nAtlasWidth, renderSettings.nAtlasHeight);

	m_ShapePolicy.m_pEffect = this;
}

template<typename InstanceType, template<typename InstanceType> class ShapePolicy, bool inWorldSpace>
FORCE_INLINE void I_Effect<InstanceType, ShapePolicy, inWorldSpace>::Update()
{
#ifndef CFG_SPEED
	for (S32 instanceNo = 0; instanceNo < m_Instances.Size(); ++instanceNo)
	{
		const InstanceType& instance = m_Instances[instanceNo];
		if (!instance.m_hLight)
		{
			FreeInstance(instanceNo--);
		}
	}
#endif
}

template<typename InstanceType, template<typename InstanceType> class ShapePolicy, bool inWorldSpace>
FORCE_INLINE void I_Effect<InstanceType, ShapePolicy, inWorldSpace>::Reset()
{
	m_Instances.Clear();
}

template<typename InstanceType, template<typename InstanceType> class ShapePolicy, bool inWorldSpace>
FORCE_INLINE void I_Effect<InstanceType, ShapePolicy, inWorldSpace>::OnBuild(const C_View& view, C_CompositeRenderNode& node) const
{
	node.iVertexBuffer = 0xFFFFFFFF;
}

template<typename InstanceType, template<typename InstanceType> class ShapePolicy, bool inWorldSpace>
FORCE_INLINE void I_Effect<InstanceType, ShapePolicy, inWorldSpace>::OnRender(const C_View& view, Display::C_Context* pDisplay, const C_CompositeRenderNode &node, const U32 states) const
{
	C_Manager* pManager = (C_Manager*) m_pManagerInterface;

	U32 vertexCount = node.blocks.Size() * m_ShapePolicy.GetNumPrimitives() * m_ShapePolicy.GetNumVertices();

	Display::Device()->SetTexture(0, m_RenderSettings.pTexture);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Wrap);
	Display::Device()->SetTextureGamma(0, Display::E_Bool_True);

	Display::Device()->SetBlendMode(Display::E_BlendMode_AlphaPreMultiply);
	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetDepthEnable(inWorldSpace ? Display::E_Bool_True : Display::E_Bool_False);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);

	Display::Device()->SetVertexDeclaration(m_ShapePolicy.GetVertexDeclaration());
	pManager->SetVertexShader(m_RenderSettings.eVertexShader);
	pManager->SetPixelShader(m_RenderSettings.ePixelShader);
	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS, (F32 *) &m_RenderSettings.vShaderParams, 1);

	typename ShapePolicy<InstanceType>::ShapeVertex* pVertices = (typename ShapePolicy<InstanceType>::ShapeVertex *) Display::Device()->BeginVertices(m_ShapePolicy.GetPrimitiveType(), vertexCount, sizeof(typename ShapePolicy<InstanceType>::ShapeVertex), m_ShapePolicy.GetVertexDeclaration(), true); // This can fail
	if(pVertices)
	{
		m_ShapePolicy.Build(node.blocks, pVertices);
		
		Display::Device()->EndVertices();
	}

	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
	Display::Device()->SetDepthEnable(Display::E_Bool_True);
}

template<typename InstanceType, template<typename InstanceType> class ShapePolicy, bool inWorldSpace>
FORCE_INLINE void I_Effect<InstanceType, ShapePolicy, inWorldSpace>::OnGather(const SceneGraph::C_View& view) const
{
	PERF_EVENT("DynamicEffect.GatherLights");

	S32 size = m_Instances.Size();
	S32 sizeMinusOne = m_Instances.Size() - 1;
	for (S32 instanceNo = 0; instanceNo < size; ++instanceNo)
	{
		const InstanceType& instance = m_Instances[instanceNo];
		DCBT(0x80, &instance);
		DCBT(0x100, &instance);

		if(instanceNo < sizeMinusOne)
		{
			DCBT(0, m_Instances[instanceNo+1].m_hLight.ObjectPtr());
			DCBT(0, m_Instances[instanceNo+1].m_pNode);
		}
		
		F32 fIntensity = instance.m_fIntensity;
		if (const Light* pLight = instance.m_hLight.Object())
		{
			fIntensity *= pLight->m_fIntensity;
			fIntensity *= pLight->m_fOcclusion;

			if ((pLight->m_VisibilityFlags & view.GetTypeFlag()) == 0)
				continue;
		}
		if (fIntensity < 0.0001f)
			continue;

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

			S32 renderPassType = 0;
			if (inWorldSpace)
				renderPassType = GetRenderer()->GetRenderPassMapping(DynamicEffect::E_RenderPassIndex_Transparent);
			else
				renderPassType = GetRenderer()->GetRenderPassMapping(DynamicEffect::E_RenderPassIndex_TwoDimensional);

			if(renderPassType != -1)
			{
				const SceneGraph::C_RenderPass* pRenderPass = view.GetSceneGraph()->GetRenderPass(renderPassType);

				SceneGraph::VolatileNodeInfo volatileNodeInfo;

				// calculate depth if we need it
				if(pRenderPass->GetVolatileInfoFlags() & SceneGraph::E_VolatileNodeInfo_Distance)
				{
					Maths::HVec4 forward(Maths::Load<Maths::HVec4>(&view.GetViewData().forwardZ));
					Maths::HF32 depthScale(view.GetViewData().depthScale);
					Maths::HF32 depth = (vCentreOne | forward) * depthScale;
				
					volatileNodeInfo.distance = (U32)depth;
				}
				view.AddVisibleNode(&instance, renderPassType, 0, volatileNodeInfo.data);
			}
		}
	}
}

template<typename InstanceType, template<typename InstanceType> class ShapePolicy, bool inWorldSpace>
FORCE_INLINE void I_Effect<InstanceType, ShapePolicy, inWorldSpace>::Clean(const Light* pLight)
{
	for (S32 instanceNo = 0; instanceNo < m_Instances.Size(); ++instanceNo)
	{
		if (m_Instances[instanceNo].m_hLight == pLight)
			FreeInstance(instanceNo--);
	}
}

template<typename InstanceType, template<typename InstanceType> class ShapePolicy, bool inWorldSpace>
FORCE_INLINE void I_Effect<InstanceType, ShapePolicy, inWorldSpace>::Clean(const C_Node* pNode)
{
	for (S32 instanceNo = 0; instanceNo < m_Instances.Size(); ++instanceNo)
	{
		if (m_Instances[instanceNo].m_pNode == pNode)
			FreeInstance(instanceNo--);
	}
}

template<typename InstanceType, template<typename InstanceType> class ShapePolicy, bool inWorldSpace>
FORCE_INLINE void I_Effect<InstanceType, ShapePolicy, inWorldSpace>::Clean(const Display::C_Texture2D* pTexture)
{
	if (m_RenderSettings.pTexture == pTexture)
	{
		ClearInstances();
		if (m_RenderSettings.pTexture)
		{
			m_RenderSettings.pTexture->Release();
			m_RenderSettings.pTexture = 0;
		}
	}
}

template <typename InstanceType, template <typename InstanceType> class ShapePolicy>
class I_Effect2d : public I_Effect<InstanceType, ShapePolicy, false>
{
public:
	typedef I_Effect<InstanceType, ShapePolicy, false> Base;

	I_Effect2d(C_Renderer* pRenderer, I_DynamicEffectManager* pManager, const U32 iIndex, const U32 iTypeIndex) : Base(pRenderer, pManager, iIndex, iTypeIndex) {}
	virtual ~I_Effect2d() {}
};

template <typename InstanceType, template <typename InstanceType> class ShapePolicy>
class I_Effect3d : public I_Effect<InstanceType, ShapePolicy, true>
{
public:
	typedef I_Effect<InstanceType, ShapePolicy, true> Base;

	I_Effect3d(C_Renderer* pRenderer, I_DynamicEffectManager* pManager, const U32 iIndex, const U32 iTypeIndex) : Base(pRenderer, pManager, iIndex, iTypeIndex) {}
	virtual ~I_Effect3d() {}
};

}; // namespace Light
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_LIGHT_EFFECT_H
