#include "stdafx.h"

#include "Dynamic/View.h"
#include "Dynamic/Renderer.h"
#include "Dynamic/TextureCoords.h"

#include "SceneGraph/View.h"

using namespace DynamicEffect;

#ifdef PLATFORM_PS3
namespace Display
{
	extern S32 g_VertexBuildingJobs;
}
#endif

namespace DynamicEffect
{
	namespace Particle
	{
		extern	Debug::Bool g_debugParticleSortParticles;
	}
}

void C_View::Create()
{
	if(Scheduler::Active())
	{
		for(S32 i = 0; i < E_RenderPassIndex_Count; ++i)
			m_JobQueueGroup[i] = Scheduler::CreateGroup();
	}
	else
	{
		for(S32 i = 0; i < E_RenderPassIndex_Count; ++i)
			m_JobQueueGroup[i] = Scheduler::JQ_GROUP_NONE;
	}

	m_VertexBuildConstants.pTexCoords = &C_TextureCoords::Instance();
#ifdef PLATFORM_PS3
	m_VertexBuildConstants.pVertexBuildingJobs = &Display::g_VertexBuildingJobs;
#else
	m_VertexBuildConstants.pVertexBuildingJobs = NULL;
#endif

	m_CompositeRenderNodes.Reserve(512);
}

void C_View::Destroy()
{
	if(Scheduler::Active())
	{
		for(S32 i = 0; i < E_RenderPassIndex_Count; ++i)
			Scheduler::DestroyGroup(m_JobQueueGroup[i]);
	}
	else
	{
		// if scheduler is not active assert that it wasn't active when we created groups either
		for(S32 i = 0; i < E_RenderPassIndex_Count; ++i)
			BIZ_ASSERT(m_JobQueueGroup[i] == Scheduler::JQ_GROUP_NONE);
	}

}

void C_View::BeginRender(const SceneGraph::C_View& view) const
{
	// patch pixel shaders on PS3
#ifdef PLATFORM_PS3
	C_Renderer* pRenderer = (C_Renderer*)m_pRenderer;
	pRenderer->BindPixelConstants(view, NULL);
#endif
}

void C_View::EndFrame(const SceneGraph::C_View& view) const
{
	// flush any outstanding passes
	JobQueueFlushPasses(0xFFFFFFFF);
}

// because particles start getting built as soon as the gather has finished, these need to be up to date during
void C_View::BeginGather(const SceneGraph::C_View& view) const
{
	const Display::C_ViewSettings3D& viewSettings3D = view.GetViewSettings3D();
	const Display::C_Matrix& viewMatrix = viewSettings3D.GetViewMatrix();
	const Display::C_Matrix& viewProjectionMatrix = viewSettings3D.GetViewProjMatrix();
	const Maths::HMat44 projMatrix = Maths::Load<Maths::HMat44>(&viewSettings3D.GetProjMatrix());
	const Maths::HVec3 cameraPosition = viewSettings3D.GetCameraPosition();

	F32 farClip = viewSettings3D.GetFarClip() > 0.f ? viewSettings3D.GetFarClip() : 100.f;
	F32 depthScale = 8388608.0f / farClip;

	m_VertexBuildConstants.vRight = MathsStorage::Float4A16(viewMatrix._11, viewMatrix._21, viewMatrix._31, viewMatrix._41);
	m_VertexBuildConstants.vUp = MathsStorage::Float4A16(viewMatrix._12, viewMatrix._22, viewMatrix._32, viewMatrix._42);
	m_VertexBuildConstants.vForward = MathsStorage::Float4A16(viewMatrix._13, viewMatrix._23, viewMatrix._33, viewMatrix._43);
	m_VertexBuildConstants.vPosition = MathsStorage::Float4A16(cameraPosition.X(), cameraPosition.Y(), cameraPosition.Z(), depthScale);
	m_VertexBuildConstants.fFrameTime = ((C_Renderer*)m_pRenderer)->GetFrameTimeRender();
	m_VertexBuildConstants.sort = true;

	// store matrix in platform specific transposition
	Maths::Store<Maths::HMat44>(projMatrix, &m_VertexBuildConstants.mProj);

	m_CompositeRenderNodes.Clear();
	if(m_CompositeRenderNodesResize)
	{
		m_CompositeRenderNodes.Reserve(m_CompositeRenderNodes.Capacity() + 32);
		m_CompositeRenderNodesResize = false;
	}

	for(S32 i = 0; i < E_RenderPassIndex_Count; ++i)
	{
		m_DynamicVertexBufferBlocks[i].Clear();
		m_JobQueueFlushed[i] = false;
	}

	m_RenderPassIndexPrev = -1;
	m_RenderPassIndex = -1;
}

// [Blur1PCDX11] Retail registers this default as true (0x00e189a0).
// RetailDynamicEffectView_RenderPass (recovered, 0x0062cb80) flushes the
// mapped pass before drawing, completing jobs and unlocking/uploading VBs.
Debug::Bool g_debugDynamicEffectFlushJobs("Graphics.Dynamic.FlushJobs", true);

void C_View::RenderPass(S32 renderPass) const
{
	if(g_debugDynamicEffectFlushJobs)
	{
		// we need to run this in release to avoid corruption
		const C_Renderer* pRenderer = (const C_Renderer*)m_pRenderer;

		S32 pass = pRenderer->GetInverseRenderPassMapping(renderPass);
		if(pass != -1)
			JobQueueFlushPass(pass);
	}
}

void C_View::JobQueueFlushPass(S32 iRenderPass) const
{
	if(!m_JobQueueFlushed[iRenderPass])
	{
		m_JobQueueFlushed[iRenderPass] = true;

		// flush all jobs needed for this render call
		if(Scheduler::Active())
			Scheduler::Flush(m_JobQueueGroup[iRenderPass]);

		m_JobQueueDynamicVertexBufferBatchers[iRenderPass].EndFrame();

		// tell DynamicVertexBuffer we are done with the blocks used for this draw call
		Ut::Vector<C_DynamicVertexBufferBlock>& dynamicVertexBufferBlocks = m_DynamicVertexBufferBlocks[iRenderPass];
		for(S32 j = 0; j < dynamicVertexBufferBlocks.Size(); ++j)
		{
			C_Renderer* pRenderer = (C_Renderer*)m_pRenderer;
			pRenderer->GetDynamicVertexBuffer()->End(dynamicVertexBufferBlocks[j]);
		}
	}
}

void C_View::JobQueueFlushPasses(S32 iRenderPasses) const
{
	// flush any outstanding jobs which are required for this render pass and viewport
	PERF_EVENT("DynamicEffect.SchedulerFlush");

	for(S32 i = 0; i < E_RenderPassIndex_Count; ++i)
	{
		// check to see if this pass being rendered and if its already been flushed
		if(iRenderPasses & (1 << i))
		{
			JobQueueFlushPass(i);
		}
	}
}
