#include "stdafx.h"

#include "SceneGraph/View.h"
#include "SceneGraph/MultiView.h"
#include "SceneGraph/SceneGraphImpl.h"

#include "System/Perf.h"

namespace SceneGraph
{

extern Debug::Int g_RenderListsPerTask;

C_MultiView::C_MultiView(C_SceneGraph* pSceneGraph) : I_View(pSceneGraph)
{
	ClearViews();
}

bool C_MultiView::AddView(const C_View* pView)
{
	if(m_PvsCell == -1 || pView->GetPvsCell() == m_PvsCell)
	{
		m_Views.PushBack(pView);

		if(m_Views.Size() == 1)
			m_JobQueueGroup = pView->GetJobQueueGroup();

		// enable render nodes and render passes
		EnableRenderPass(pView->RenderPasses());
		EnableRenderNode(pView->RenderNodes());
		m_VolatileInfoFlags |= pView->GetVolatileInfoFlags();
		m_PvsCell = pView->GetPvsCell();
		return true;
	}
	else
	{
		// if view failed to add then clear nodes
		pView->ClearNodes();
		return false;
	}
}

void C_MultiView::ClearViews()
{
	m_RenderPassFlags = 0;
	m_RenderNodeFlags = 0;
	m_VolatileInfoFlags = 0;
	m_PvsCell = -1;

	m_Views.Clear();
}

void C_MultiView::BeginGather() const
{
	if(!m_Gathering)
	{
		if(m_Views.Size() == 0)
			return;

		MEMORY_TRACKER_PUSH("graphics.scenegraph");

		PERF_EVENT("MultiView.BeginGather");
		
		ClearNodes();
		Gather();
		
		MEMORY_TRACKER_POP("graphics.scenegraph");

		m_Gathering = true;
	}
}

void C_MultiView::Gather() const
{
	m_pSceneGraph->Gather(*this);
}

void C_MultiView::ClearNodes() const
{
	for(S32 i = 0; i < m_Views.Size(); ++i)
	{
		m_Views[i]->ClearNodes();
		m_Views[i]->RendererViewsBeginGather();
	}
	m_DeferredGatherNodes.Clear();
}

void C_MultiView::EndGather() const
{
	if(m_Gathering)
	{
		if(m_Views.Size() == 0)
			return;

		MEMORY_TRACKER_PUSH("graphics.scenegraph");

		PERF_EVENT("MultiView.EndGather");

		I_View::EndGather();

		for(S32 i = 0; i < m_Views.Size(); ++i)
		{
			PERF_EVENT("View.EndGather");
			m_Views[i]->GatherDeferredNodes();
		}
		
		MEMORY_TRACKER_POP("graphics.scenegraph");

		m_Gathering = false;
	}
}




} // namespace SceneGraph

