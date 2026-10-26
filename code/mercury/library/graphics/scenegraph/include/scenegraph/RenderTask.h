#ifndef SCENE_GRAPH_RENDER_TASK_H
#define SCENE_GRAPH_RENDER_TASK_H

#ifdef _MSC_VER
#pragma once
#endif

#if PLATFORM_PPU_SPU
#include <Display2/Display2Includes.h>
#include <Display2/Context/Context.h>
#include <Display/buffer/CommandBuffer.h>
#endif

#if !RENDERING_SPU_HACK
#include <Display/Buffer/CommandBuffer.h>
#endif
#include <SceneGraph/Context.h>
#include <SceneGraph/SceneGraphMaxContexts.h>

#include <system/scheduler/jobqueue.h>

namespace SceneGraph
{

class C_View;
class C_Context;
class C_RenderGroup;
class C_RenderGroupSettings;
struct RenderStates;

/// part of a tasklist for a scheduler task
struct RenderSubtask
{
	U32 pass;
	const RenderStates*			pRenderStates;

	S32	abstractStart;
	S32 abstractEnd;
	S32 primitiveStart;
	S32 primitiveEnd;
	S32 instanceStart;
	S32 instanceEnd;
	S32 modelInstanceStart;
	S32 modelInstanceEnd;
	const C_VisibleNodeList*	pVisibleNodeList;
};

/// a description for a single task
class ALIGN16 C_RenderTask
{
public:
	/// constructor
	C_RenderTask();

	/// schedule task to be run
	void Schedule(S32 group, S32 policy);
	
	/// get context
	C_Context *GetContext(int i) { assert(i>=0 && i<SCENEGRAPH_MAX_CONTEXTS); return &m_pContexts[i]; }

	/// get group
	const C_RenderGroup *GetRenderGroup() { return m_pGroup; }

	/// get multithreaded switch
	bool GetMultiThreaded() { return m_MultiThreaded; }

	friend class C_RenderGroup;
	friend class C_Context;

#if PLATFORM_PPU_SPU
	int																m_iRenderingDataContextHandle;
	int																m_iFXContextHandle;
	int																m_iDisplayContextHandle;
#endif

private:
	const C_RenderGroup*											m_pGroup;
	Btl::Vector<RenderSubtask, Btl::VectorParameters_Static<16> >	m_Subtasks;
	U32																m_Subtechnique;
	U32																m_DistanceSubtechnique;
	C_Context*														m_pContexts;
	bool															m_MultiThreaded;
	bool															m_bAbstract;
	bool															m_bOther;
	Display::C_CommandBuffer										m_CommandBuffer;

	const char*														m_pName;
};

typedef Btl::Pair<S32, const RenderStates*>	Pass;

// render group consists of 1 or more passes split into 1 or more render tasks
class C_RenderGroup
{
public:
	C_RenderGroup();
	~C_RenderGroup();

	void Reset();
	void Render();
	void Schedule(const Display::C_View* pDisplayView, const C_RenderGroupSettings* pSettings);

	void								SchedulePass(const S32 pass, const RenderStates* pStates) { m_Passes.PushBack(Pass(pass, pStates)); }
	
	C_Context&							GetContext() { return m_Context; }
	const C_Context&					GetContext() const { return m_Context; }

	void								SetName(const char* pName) const { m_pName = pName; }
	const char*							GetName() const { return m_pName; }
	
	const S32							GetNumPasses() const { return m_Passes.Size(); }
	const S32							GetPass(S32 i) const { return m_Passes[i].first; }

private:
	mutable Btl::Vector<Pass>			m_Passes;
	mutable Btl::Vector<C_RenderTask>	m_Tasks;
	mutable SceneGraph::C_Context		m_Context;

	const Display::C_View*				m_pDisplayView;
	const C_RenderGroupSettings*		m_pSettings;
	mutable const char*					m_pName;

	Scheduler::GroupHandle				m_JobQueueGroup;
};

// render functions
void RenderTask(C_Context *pContext, const C_View* pView, const RenderSubtask *pTask, const U32 subtechnique);

} // namespace SceneGraph

#endif // SCENE_GRAPH_RENDER_TASK_H
