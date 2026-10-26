#ifndef RENDER_DYNAMICEFFECT_VIEW_H
#define RENDER_DYNAMICEFFECT_VIEW_H

#ifdef _MSC_VER
#pragma once
#endif

// scene graph integration
#include "SceneGraph/RendererView.h"

// dynamic
#include "Dynamic/VertexBuildConstants.h"
#include "Dynamic/RenderNode.h"
#include "Dynamic/DynamicVertexBuffer.h"
#include "Dynamic/DebugInfo.h"

// particle
#include "Dynamic/Particle/Build/JobQueueDynamicVertexBufferBatcher.h"

// jobqueue
#include <system/scheduler/jobqueue.h>

namespace DynamicEffect
{

enum E_RenderPass
{
	E_RenderPass_Unknown			= 0x00000000,

	E_RenderPass_Opaque				= 0x00000001,
	E_RenderPass_Decal				= 0x00000002,
	E_RenderPass_Transparent		= 0x00000004,
	E_RenderPass_TwoDimensional		= 0x00000008,
	E_RenderPass_Debug				= 0x00000010,

	E_RenderPass_All				= 0xFFFFFFFF
};

enum E_RenderPassIndex
{
	E_RenderPassIndex_Unknow		= -1,

	E_RenderPassIndex_Opaque,
	E_RenderPassIndex_Decal,
	E_RenderPassIndex_Transparent,
	E_RenderPassIndex_TwoDimensional,
	
	E_RenderPassIndex_Count
};

// contains per view data required by the renderer
class C_View : public SceneGraph::I_RendererView
{
public:
	C_View() : SceneGraph::I_RendererView(), m_CompositeRenderNodesResize(false) {}
	C_View(SceneGraph::I_Renderer* pRenderer, const SceneGraph::C_View* pView) : SceneGraph::I_RendererView(pRenderer, pView), m_CompositeRenderNodesResize(false) {}
	~C_View() {}

	void	Create();
	void	Destroy();

	// SceneGraph::I_View integration
	void	BeginGather(const SceneGraph::C_View& view) const;
	void	BeginRender(const SceneGraph::C_View& view) const;
	void	EndFrame(const SceneGraph::C_View& view) const;
	void	RenderPass(S32 renderPass) const;

	void						AddDynamicVertexBufferBlock(C_DynamicVertexBufferBlock block) const { m_DynamicVertexBufferBlocks[m_RenderPassIndex].PushBack(block); }
	DynamicEffect::Particle::BuildPolicy::C_JobQueueDynamicVertexBufferBatcher&
								GetJobQueueDynamicVertexBufferBatcher() const { return m_JobQueueDynamicVertexBufferBatchers[m_RenderPassIndex]; }
	Scheduler::GroupHandle		GetJobQueueGroup() const { return m_JobQueueGroup[m_RenderPassIndex]; }
	void						JobQueueFlushPasses(S32 iRenderPasses) const;
	void						JobQueueFlushPass(S32 iRenderPass) const;
	const VertexBuildConstants&	GetVertexBuildConstants() const { return m_VertexBuildConstants; }
	
	friend class C_Renderer;

	mutable S32									m_RenderPassIndexPrev;
	mutable S32									m_RenderPassIndex;
	
private:
	mutable VertexBuildConstants				m_VertexBuildConstants;
	
	// new stuff
	mutable CompositeRenderNodes				m_CompositeRenderNodes;
	mutable bool								m_CompositeRenderNodesResize;

	mutable Ut::Vector<C_DynamicVertexBufferBlock>		
												m_DynamicVertexBufferBlocks[E_RenderPassIndex_Count];
	mutable DynamicEffect::Particle::BuildPolicy::C_JobQueueDynamicVertexBufferBatcher
												m_JobQueueDynamicVertexBufferBatchers[E_RenderPassIndex_Count];
	Scheduler::GroupHandle						m_JobQueueGroup[E_RenderPassIndex_Count];
	mutable bool								m_JobQueueFlushed[E_RenderPassIndex_Count];
	U32											m_iCullModes;
};

}

#endif // RENDER_DYNAMICEFFECT_VIEW_H
