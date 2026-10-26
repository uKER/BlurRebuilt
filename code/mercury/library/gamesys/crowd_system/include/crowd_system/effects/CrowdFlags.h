#ifndef CROWD_FLAGS_H
#define CROWD_FLAGS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <crowd_system/CrowdDefines.h>
#include <core/containers/string.h>

#include <display/utils/Renderer.h>
#include <display/buffer/VertexBuffer.h>
#include <display/buffer/IndexBuffer.h>
#include <display/shader/VertexShader.h>
#include <display/shader/PixelShader.h>
#include <display/buffer/VertexFormat.h>
#include <display/buffer/VertexDeclarations.h>

#include <textureResource/TextureResource.h>

#include <sceneGraph/RenderNode.h>
#include <sceneGraph/GatherNode.h>
#include <sceneGraph/RendererView.h>
#include <sceneGraph/RendererInterface.h>

#include <display/context/SamplerState.h>

#include <system/scheduler/jobqueue.h>

#include <btl/containers/bitvector.h>

namespace SceneGraph
{
	class C_SceneGraph;
}

namespace DynamicEffect
{
	namespace Cloth
	{
		class C_Sheet;
	}
}

namespace Display
{
	class C_Context;
}

namespace AnimatedCrowd
{

class C_CrowdTextureCache;

struct CrowdFlagVertex
{
	F32 x, y, z;
	F32 u, v;
	F32 nx, ny, nz;

#ifndef PLATFORM_SPU
	static const U32						m_ElementCount = 4;
	static Display2::C_VertexElement		m_Elements[m_ElementCount];
#endif
};

//! C_CrowdFlagInstance

class C_CrowdFlagInstance : public SceneGraph::I_RenderNode
{
public:
	C_CrowdFlagInstance() : SceneGraph::I_RenderNode() { }

	Maths::HMat43	Transform;
	F32				SH[16];
	S32				BaseFlagIndex;
	S32				TextureIndex;

	friend class C_CrowdFlags;
};

//! C_CrowdFlags

class C_CrowdFlags : public Display::I_Renderer, public SceneGraph::I_GatherNode, public SceneGraph::I_Renderer
{
public:
	C_CrowdFlags(SceneGraph::C_SceneGraph* pSceneGraph, const U32 prepassSubtechniqueHandle);
	virtual ~C_CrowdFlags();

	void	SetViewFlags(const U32 viewFlags);

	void	SetTextureCache(const C_CrowdTextureCache* pTextureCache) { m_pTextureCache = pTextureCache; }

	U32		GetNumBaseFlags() { return m_NumBaseFlags; }
	U32		GetBufferIndex() const { return m_BufferIndex; }

	bool	OnCreate();
	void	OnDestroy();

	bool	BuildFlagMesh(const F32 width, const F32 height, const U32 numColumns, const U32 numRows, const U32 numBaseFlags,
				const F32 stretchFactor, const F32 particleMass, const F32 windCoeff);

	void	SetRenderPass(const U32 renderPass);
	
	void	Accept(SceneGraph::I_Visitor& visitor) const {}
	void	OnGather(const SceneGraph::C_View& view) const;

	bool	IsCreated() { return m_Created; }

	void	UpdateClothPins(const U32 flagIndex, const Maths::HMat43& transform);

	void	ScheduleClothSimulationTask(const Scheduler::PolicyHandle jobQueuePolicyHandle, const F32 deltaTime);
	void	FlushClothSimulationTask() const;

	void	SetDynamicLightingTexture(const void* pLightBufferAddress);
	void	SetHalfPixelOffset(const U32 widthInPixels, const U32 heightInPixels);
	void	SetLuminance(const MathsStorage::Float4& luminance);
	void	SetFogColour(const Maths::C_RGBA_F32& fogColour);
	void	SetDepthFogSettings(const Maths::HVec4& depthFogSettings);
	
	// scene graph I_Renderer implementation
	void	Render(const SceneGraph::C_View& view, SceneGraph::C_Context& context, const SceneGraph::C_VisibleNodes& visibleNodes, const SceneGraph::C_VisibleNodeBatch& batch) const;

	void	SetFlags(const C_CrowdFlagInstance* pFlags, const U32 numFlags);// { m_pFlags = pFlags; m_NumFlags = numFlags; }

	void	SetNodeType(C_CrowdFlagInstance* pFlagInstance) const;

#ifndef CFG_GOLD
	const F32	GetFlagWidth() const;
	const F32	GetFlagHeight() const;
	const F32	GetFlagStretchFactor() const;
	const F32	GetFlagWindCoeff() const;
	const F32	GetFlagParticleMass() const;

	void	ResizeFlagMesh(const F32 width, const F32 height, const F32 stretchFactor, const F32 particleMass, const F32 windCoeff);
#endif

private:
	void	UpdateVertexBuffer(const U32 bufferIndex);

	void	RenderMain(Display::C_Context* pDisplay, const SceneGraph::C_View& view, const SceneGraph::C_VisibleNodes& visibleNodes, const SceneGraph::C_VisibleNodeBatch& batch) const;
	void	RenderPrepass(Display::C_Context* pDisplay, const SceneGraph::C_View& view, const SceneGraph::C_VisibleNodes& visibleNodes, const SceneGraph::C_VisibleNodeBatch& batch) const;

private:
	struct CreationParams
	{
		F32	Width;
		F32 Height;
		U32 NumColumns;
		U32 NumRows;
		F32 StretchFactor;
		F32 ParticleMass;
		F32 WindCoeff;
	};

private:
	const U32								m_PrepassSubtechniqueHandle;
	const void*								m_pLightBufferAddress;
	Maths::HVec2							m_HalfPixelOffset;
	MathsStorage::Float4					m_Luminance;
	Maths::C_RGBA_F32						m_FogColour;
	Maths::HVec4							m_DepthFogSettings;

	const C_CrowdFlagInstance*				m_pFlags;
	U32										m_NumFlags;

	CreationParams							m_CreationParams;
	U32										m_NumBaseFlags;

	U32										m_NumBaseFlagColumns;
	U32										m_NumBaseFlagRows;

	U32										m_NumBaseFlagPoints;
	U32										m_NumBaseFlagIndices;

	F32										m_DeltaU;
	F32										m_DeltaV;

	Display2::C_VertexShaderParam			m_FlagTransformParameter;
	Display2::C_VertexShaderParam			m_FlagColourParameter;
	Display2::C_VertexShaderParam			m_FlagSHParameter;
	Display2::C_VertexShaderParam			m_FlagLodFadeParameter;
	Display2::C_VertexShaderParam			m_FlagLodDistanceScaleParameter;

	Display2::C_VertexShaderParam			m_FlagPrepassTransformParameter;
	Display2::C_VertexShaderParam			m_FlagPrepassNearFarParameter;

	Display2::C_PixelShaderParam			m_FlagHalfPixelOffsetParameter;
	Display2::C_PixelShaderParam			m_FlagLuminanceParameter;
	Display2::C_PixelShaderParam			m_FlagFogColourParameter;
	Display2::C_PixelShaderParam			m_FlagDepthFogParameter;

	Display2::C_VertexShader				m_FlagVertexShader;
	Display2::C_VertexShader				m_FlagPrepassVertexShader;
	
	Display2::C_PixelShader					m_FlagPixelShader;
	Display2::C_PixelShader					m_FlagPrepassPixelShader;

	Btl::Vector<DynamicEffect::Cloth::C_Sheet>
											m_ClothSheets;
	F32										m_ParticleSeperation;

	Display::C_VertexDeclaration			m_VertexDeclaration;
	Display2::C_VertexBuffer				m_VertexBuffer[2];
	Display2::C_IndexBuffer					m_IndexBuffer;
	mutable	U32								m_BufferIndex;

	U32										m_RenderPass;
	bool									m_Created;
	bool									m_Ready;

	const C_CrowdTextureCache*				m_pTextureCache;

	Display::C_SamplerState					m_SamplerState;
	Display::C_SamplerState					m_LightingSamplerState;

	U32										m_ViewFlags;

	U32										m_NumSides;

	Btl::BitArray32							m_EnabledFlags;

	Scheduler::GroupHandle					m_ClothSimulationGroupHandle;
};

INLINE void C_CrowdFlags::SetRenderPass(const U32 renderPass)
{
	m_RenderPass = renderPass;
}

}; // namespace AnimatedCrowd

#endif // CROWD_FLAGS_H
