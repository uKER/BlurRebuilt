#include "stdafx.h"

#include "SceneGraph/Visible/Nodes.h"
#include "SceneGraph/RenderTask.h"

#include "SceneGraph/Context.h"
#include "SceneGraph/View.h"
#include "SceneGraph/SceneGraphImpl.h"
#include "SceneGraph/RendererInterface.h"
#include "SceneGraph/RenderGroupSettings.h"

#include "SceneGraph/RenderingData/StaticRenderListNode.h"
#include "SceneGraph/RenderingData/ModelInstanceNode.h"

#include "Display.h"
#include "Display/Context/Context.h"

namespace FX{ DECLARE_CONTEXT(Context, Context); }
namespace RenderingData{ namespace Target{ DECLARE_CONTEXT(Context, Context); } }

extern Scheduler::jqModule SceneGraphRenderTask, SceneGraphRenderTaskPPU, SceneGraphRenderTaskSPU;

namespace Display
{
	extern Debug::Bool g_DrawDummyPrimitiveBeforeRunningCommandBuffer;
}

namespace SceneGraph
{

#if PLATFORM_PS3 // multit-threaded rendering is on by default on Xenon and PS3. Not supported on other platforms
Debug::Bool g_MultiThreaded					("Graphics.SceneGraph.MultiThreadedRender", true);
#else
Debug::Bool g_MultiThreaded					("Graphics.SceneGraph.MultiThreadedRender", false);
#endif
Debug::Bool g_Wireframe						("Graphics.SceneGraph.Wireframe", false);

// rendering/scheduling code

Debug::Int	g_SchedulingMinimumCostPerTask	("Graphics.SceneGraph.Scheduling.MinimumCostPerTask", 32, Debug::Range(16, 1024));
Debug::Int	g_SchedulingAbstractCost		("Graphics.SceneGraph.Scheduling.AbstractCost", 1, Debug::Range(1, 10));
Debug::Int	g_SchedulingPrimitiveCost		("Graphics.SceneGraph.Scheduling.PrimitiveCost", 1, Debug::Range(1, 10));
Debug::Int	g_SchedulingInstanceCost		("Graphics.SceneGraph.Scheduling.InstanceCost", 1, Debug::Range(1, 10));
Debug::Int	g_SchedulingModelInstanceCost	("Graphics.SceneGraph.Scheduling.ModelInstanceCost", 4, Debug::Range(1, 10));

#if PLATFORM_PS3
#if !CFG_GOLD
Debug::Bool g_RunPPUForceDeferred("Graphics.SceneGraph.PS3.RunPPUForceDeferred", false);
#endif
Debug::Bool g_RunPPUTasksDeferred("Graphics.SceneGraph.PS3.RunPPUTasksDeferred", true);
Debug::Bool g_DisableSingleThreadedTasks("Graphics.SceneGraph.PS3.DisableSingleThreadedTasks", false);
Debug::Bool	g_AsyncCommandBufferKicking("Graphics.SceneGraph.PS3.AsyncCommandBufferKicking", true);
Debug::Bool g_AbstractsOnSpu("Graphics.SceneGraph.PS3.AbstractsOnSpu", true);
#endif

void C_RenderGroup::Schedule(const Display::C_View* pDisplayView, const C_RenderGroupSettings* pSettings)
{
	m_pDisplayView = pDisplayView;
	m_pSettings = pSettings;

	bool multiThreaded = g_MultiThreaded && !g_Wireframe;

	// on PS3 we always want to split tasks so we get some tasks on SPU. otherwise we can run whole task in 1 go and simplify job setup
#ifdef PLATFORM_PS3
	bool split = true;
#else
	bool split = pSettings->GetNumWorkers() != 1;
#endif

	if(multiThreaded && split)
	{
		// split all the render passes across a number of thread/tasks
		S32 abstractCost = g_SchedulingAbstractCost;
		S32 primitiveCost = g_SchedulingPrimitiveCost;
		S32 instanceCost = g_SchedulingInstanceCost;
		S32 modelInstanceCost = g_SchedulingModelInstanceCost;

		S32 totalCost = 0;
		for(S32 i = 0; i < m_Passes.Size(); ++i)
		{
			const C_VisibleNodeList* pVisibleNodeList = m_Context.GetView()->GetVisibleNodeList(m_Passes[i].first);
			totalCost += pVisibleNodeList->m_NodeBatches.Size() * abstractCost;
			totalCost += pVisibleNodeList->m_StaticPrimitives.Size() * primitiveCost;
			totalCost += pVisibleNodeList->m_StaticInstances.Size() * instanceCost;
			totalCost += pVisibleNodeList->m_ModelInstances.Size() * modelInstanceCost;
		}

		if(totalCost != 0)
		{
			m_Context.CloneState(NULL);
			Display::C_Context* pDisplay = Display::GetContext(&m_Context);
			pDisplay->SetView(*pDisplayView);

			pDisplay->SetWindowIndex(pSettings->GetWindowIndex());
			pDisplay->SetColourDepthStencilSurface(pSettings->GetColourSurface(), pSettings->GetDepthStencilSurface());
			pDisplay->SetColourDepthStencilTexture(pSettings->GetColourTexture(), pSettings->GetDepthStencilTexture());
			#if PLATFORM_PC
			pDisplay->SetStencilTexture(pSettings->GetStencilTexture());	// [Blur2PC] retail 0x007a18cc, 0x007a1e63
			#endif
			pDisplay->SetMultisample(pSettings->GetMultisample());

			S32 workers = pSettings->GetNumWorkers() == -1 ? Scheduler::GetNumWorkers() + 1 : pSettings->GetNumWorkers();

			S32 costPerTask = totalCost / workers + 1;
			costPerTask = Maths::Max(costPerTask, (S32)g_SchedulingMinimumCostPerTask);

			assert(m_Tasks.Size() == 0);
			m_Tasks.Reserve(workers + 1);

			S32 abstractStart = 0;
			S32 primitiveStart = 0;
			S32 instanceStart = 0;
			S32 modelInstanceStart = 0;

			S32 taskCost = -1;
			C_RenderTask* pTask = NULL;
			
			// split the render passes into mank tasks and sub tasks where each task has about nodesPerTask nodes
			for(S32 i = 0; i < m_Passes.Size() && totalCost; )
			{
				const C_VisibleNodeList* pVisibleNodeList = m_Context.GetView()->GetVisibleNodeList(m_Passes[i].first);

				if(pVisibleNodeList->m_Empty)
				{
					++i;
					continue;
				}

				if (pTask)
				{
					assert(!(pTask->m_bAbstract && pTask->m_bOther));
				}

				if(taskCost < 0)
				{
	#if PLATFORM_PS3
					if (pTask)
					{
						assert(pTask->m_bAbstract || pTask->m_bOther);
					}
	#endif

					m_Tasks.PushBack();
					pTask = &m_Tasks.Back();

					pTask->m_pGroup = this;
					pTask->m_MultiThreaded = true;
					pTask->m_Subtechnique = pSettings->GetSubtechnique();
					pTask->m_DistanceSubtechnique = pSettings->GetDistanceSubtechnique();
					pTask->m_pContexts = m_Context.GetSceneGraph()->GetContextList();
					pTask->m_bAbstract = false;
					pTask->m_bOther = false;

					taskCost = costPerTask;

					#if PLATFORM_PS3 && !CFG_GOLD
					sprintf_s(pTask->m_CommandBuffer.m_DebugName, sizeof(pTask->m_CommandBuffer.m_DebugName), "%s %i", m_pName, m_Passes[i].first);
					#endif
				}
				
				{
					pTask->m_Subtasks.PushBack();
					RenderSubtask& renderSubtask = pTask->m_Subtasks.Back();
					renderSubtask.pass = m_Passes[i].first;
					renderSubtask.pRenderStates = m_Passes[i].second;

					renderSubtask.abstractStart = renderSubtask.abstractEnd = abstractStart;
					renderSubtask.primitiveStart = renderSubtask.primitiveEnd = primitiveStart;
					renderSubtask.instanceStart = renderSubtask.instanceEnd = instanceStart;
					renderSubtask.modelInstanceStart = renderSubtask.modelInstanceEnd = modelInstanceStart;
					renderSubtask.pVisibleNodeList = pVisibleNodeList;

					// abstract
					{
	#if PLATFORM_PS3
						if (pVisibleNodeList->m_NodeBatches.Size() > abstractStart)
						{
							if(pTask->m_bOther)
							{
								// if theres other nodes in this task and this list is abstract then end this task
								if(pVisibleNodeList->m_Abstract || !g_AbstractsOnSpu)
								{
									taskCost = -1;
									continue;
								}
							}
							else
							{
								// say this task is abstract
								if(pVisibleNodeList->m_Abstract || !g_AbstractsOnSpu)
								{
									assert(!pTask->m_bOther);
									pTask->m_bAbstract = true;
								}
								else
								{
									// if current task is abstract then we can end it because these nodes can still be rendered
									if(pTask->m_bAbstract)
									{
										taskCost = -1;
										continue;
									}
									else
									{
										assert(!pTask->m_bAbstract);
										pTask->m_bOther = true;
									}
								}
							}
						}
	#endif

						taskCost -= pVisibleNodeList->m_NodeBatches.Size() - abstractStart;
						if(taskCost < 0)
						{
							abstractStart = pVisibleNodeList->m_NodeBatches.Size() + taskCost;
							renderSubtask.abstractEnd = abstractStart;
							continue;
						}
						else
						{
							abstractStart = pVisibleNodeList->m_NodeBatches.Size();
							renderSubtask.abstractEnd = pVisibleNodeList->m_NodeBatches.Size();
						}
					}
					
					// primitives
					{
	#if PLATFORM_PS3
						if (pVisibleNodeList->m_StaticPrimitives.Size() > primitiveStart)
						{
							if (pTask->m_bAbstract)
							{
								taskCost = -1;
								continue;
							}
							else
							{
								assert(!pTask->m_bAbstract);
								pTask->m_bOther = true;
							}
						}
	#endif

						taskCost -= pVisibleNodeList->m_StaticPrimitives.Size() - primitiveStart;
						if(taskCost < 0)
						{
							primitiveStart = pVisibleNodeList->m_StaticPrimitives.Size() + taskCost;
							renderSubtask.primitiveEnd = primitiveStart;
							continue;
						}
						else
						{
							primitiveStart = pVisibleNodeList->m_StaticPrimitives.Size();
							renderSubtask.primitiveEnd = pVisibleNodeList->m_StaticPrimitives.Size();
						}
					}

					// instances
					{
	#if PLATFORM_PS3
						if (pVisibleNodeList->m_StaticInstances.Size() > instanceStart)
						{
							if (pTask->m_bAbstract)
							{
								taskCost = -1;
								continue;
							}
							else
							{
								assert(!pTask->m_bAbstract);
								pTask->m_bOther = true;
							}
						}
	#endif

						taskCost -= pVisibleNodeList->m_StaticInstances.Size() - instanceStart;
						if(taskCost < 0)
						{
							instanceStart = pVisibleNodeList->m_StaticInstances.Size() + taskCost;
							renderSubtask.instanceEnd = instanceStart;
							continue;
						}
						else
						{
							instanceStart = pVisibleNodeList->m_StaticInstances.Size();
							renderSubtask.instanceEnd = pVisibleNodeList->m_StaticInstances.Size();
						}
					}

					// model instances
					{
	#if PLATFORM_PS3
						if (pVisibleNodeList->m_ModelInstances.Size() > modelInstanceStart)
						{
							if (pTask->m_bAbstract)
							{
								taskCost = -1;
								continue;
							}
							else
							{
								assert(!pTask->m_bAbstract);
								pTask->m_bOther = true;
							}
						}
	#endif

						S32 remainingModelInstanceCost = (pVisibleNodeList->m_ModelInstances.Size() - modelInstanceStart) * modelInstanceCost;
						S32 taskCostLeft = taskCost;
						taskCost -= remainingModelInstanceCost;
						if(taskCost < 0)
						{
							modelInstanceStart += taskCostLeft / modelInstanceCost;
							renderSubtask.modelInstanceEnd = modelInstanceStart;
							continue;
						}
						else
						{
							modelInstanceStart = pVisibleNodeList->m_ModelInstances.Size();
							renderSubtask.modelInstanceEnd = pVisibleNodeList->m_ModelInstances.Size();
						}
					}

					abstractStart = 0;
					primitiveStart = 0;
					instanceStart = 0;
					modelInstanceStart = 0;
				}
				++i;
			}

			// TODO: get rid of this at some point
			S32 totalTasksCost = 0;
			for(S32 i = 0; i < m_Tasks.Size(); ++i)
			{
				C_RenderTask& task = m_Tasks[i];
				S32 abstractNodes = 0;
				S32 primitiveNodes = 0;
				S32 instanceNodes = 0;
				S32 modelInstanceNodes = 0;

				for(S32 j = 0; j < task.m_Subtasks.Size(); ++j)
				{
					RenderSubtask& subtask = task.m_Subtasks[j];
					abstractNodes += subtask.abstractEnd -	subtask.abstractStart;
					primitiveNodes += subtask.primitiveEnd - subtask.primitiveStart;
					instanceNodes += subtask.instanceEnd -	subtask.instanceStart;
					modelInstanceNodes += subtask.modelInstanceEnd - subtask.modelInstanceStart;
				}

				S32 taskCost = abstractNodes * abstractCost + primitiveNodes * primitiveCost + instanceNodes * instanceCost + modelInstanceNodes * modelInstanceCost;

				if(taskCost > 0)
				{
					task.Schedule(m_JobQueueGroup, m_Context.GetSceneGraph()->GetJobQueuePolicyRender(pSettings->GetPriority()));
				}

				totalTasksCost += taskCost;
			}

			if(totalTasksCost != totalCost)
				int breakhere = 0;
		}
	}	
	else
	{
		S32 totalNodes = 0;
		S32 abstractNodes = 0;
		for(S32 i = 0; i < m_Passes.Size(); ++i)
		{
			const C_VisibleNodeList* pVisibleNodeList = m_Context.GetView()->GetVisibleNodeList(m_Passes[i].first);
			totalNodes += pVisibleNodeList->m_NodeBatches.Size();
			totalNodes += pVisibleNodeList->m_StaticPrimitives.Size();
			totalNodes += pVisibleNodeList->m_StaticInstances.Size();
			totalNodes += pVisibleNodeList->m_ModelInstances.Size();
			abstractNodes += pVisibleNodeList->m_Nodes.Size();
		}

		// if theres nothing to render don't bother creating a task
		if(totalNodes != 0)
		{
			// just add everything into a single task
			m_Tasks.PushBack();
			C_RenderTask* pTask = &m_Tasks.Back();

			pTask->m_pGroup = this;
			pTask->m_MultiThreaded = multiThreaded;
			pTask->m_Subtechnique = pSettings->GetSubtechnique();
			pTask->m_DistanceSubtechnique = pSettings->GetSubtechnique();
			pTask->m_pContexts = m_Context.GetSceneGraph()->GetContextList();
			pTask->m_bOther = true;
			pTask->m_bAbstract = abstractNodes != 0;
			
			for(S32 i = 0; i < m_Passes.Size(); ++i)
			{
				const C_VisibleNodeList* pVisibleNodeList = m_Context.GetView()->GetVisibleNodeList(m_Passes[i].first);
				
				if(pVisibleNodeList->m_Empty)
					continue;

				pTask->m_Subtasks.PushBack();
				RenderSubtask& renderSubtask = pTask->m_Subtasks.Back();
				renderSubtask.pass = m_Passes[i].first;
				renderSubtask.pRenderStates = m_Passes[i].second;
				renderSubtask.pVisibleNodeList = pVisibleNodeList;

				renderSubtask.abstractStart = 0;
				renderSubtask.abstractEnd = pVisibleNodeList->m_NodeBatches.Size();
				renderSubtask.primitiveStart = 0;
				renderSubtask.primitiveEnd = pVisibleNodeList->m_StaticPrimitives.Size();
				renderSubtask.instanceStart = 0;
				renderSubtask.instanceEnd = pVisibleNodeList->m_StaticInstances.Size();
				renderSubtask.modelInstanceStart = 0;
				renderSubtask.modelInstanceEnd = pVisibleNodeList->m_ModelInstances.Size();
				
			}

			// clone the global state for this group's context
			m_Context.CloneState(NULL);
			Display::C_Context* pDisplay = Display::GetContext(&m_Context);
			pDisplay->SetView(*pDisplayView);
			pDisplay->SetWindowIndex(pSettings->GetWindowIndex());
			pDisplay->SetColourDepthStencilSurface(pSettings->GetColourSurface(), pSettings->GetDepthStencilSurface());
			pDisplay->SetColourDepthStencilTexture(pSettings->GetColourTexture(), pSettings->GetDepthStencilTexture());
			#if PLATFORM_PC
			pDisplay->SetStencilTexture(pSettings->GetStencilTexture());	// [Blur2PC] retail 0x007a18cc, 0x007a1e63
			#endif
			pDisplay->SetMultisample(pSettings->GetMultisample());

			if(pTask->m_MultiThreaded)
				pTask->Schedule(m_JobQueueGroup, m_Context.GetSceneGraph()->GetJobQueuePolicyRender(pSettings->GetPriority()));
		}
	}
}

/// submit results of the rendering tasks into the main command buffer
void C_View::RenderGroup(const S32 group)
{
	// wait for all tasks to finish rendering
	if(group < m_Groups.Size())
	{
		{
			PERF_EVENT("SceneGraph.RendererViewsRenderGroup");
			
			// tell renderers which passes we are about to render
			for(S32 pass = 0; pass < m_Groups[group].GetNumPasses(); ++pass)
			{
				for(S32 view = 0; view < m_RendererViews.Size(); ++view)
				{
					if(m_RendererViews[view])
						m_RendererViews[view]->RenderPass(m_Groups[group].GetPass(pass));
				}
			}
		}

		m_Groups[group].Render();
	}
}

C_RenderGroup::C_RenderGroup() : m_pName(0)
{
	m_JobQueueGroup = Scheduler::CreateGroup(true);
}

C_RenderGroup::~C_RenderGroup()
{
	Scheduler::DestroyGroup(m_JobQueueGroup);
}

void C_RenderGroup::Reset()
{
	m_Tasks.Clear();
	m_Passes.Clear();
}

void C_RenderGroup::Render()
{
	PERF_EVENT("SceneGraph.RenderGroup");

#if PLATFORM_PS3
	// PS3 doesnt need to flush rendering tasks
	if (!g_AsyncCommandBufferKicking)
#endif
	{
		Scheduler::Flush(m_JobQueueGroup);
	}

	if(g_Wireframe)
		Display::Device()->SetFillMode(Display::E_FillMode_Wireframe);

	// run the commandbuffers
	bool invalidateState = true;
	for(S32 i = 0; i < m_Tasks.Size(); ++i)
	{
		C_RenderTask *pTask = &m_Tasks[i];
		if(pTask->m_MultiThreaded)
		{
			{
				PERF_EVENT("SceneGraph.RunCommandBuffer");

				pTask->m_CommandBuffer.Run();

				invalidateState = true;
			}
		}
		else
		{
			SceneGraph::C_Context* pContext = m_Context.GetSceneGraph()->GetContext();

			// state might need to be reset if a command buffer has been run
			if(invalidateState)
			{
				pContext->InvalidateState();
				invalidateState = false;
			}
			
			// tell display context about multisample and colour/depth textures
			Display::C_Context* pDisplay = Display::GetContext(pContext);
			pDisplay->SetMultisample(m_pSettings->GetMultisample());
			pDisplay->SetColourDepthStencilTexture(m_pSettings->GetColourTexture(), m_pSettings->GetDepthStencilTexture());

			pContext->RenderTask(pTask);
		}
	}

	if(g_Wireframe)
		Display::Device()->SetFillMode(Display::E_FillMode_Default);

	for(S32 i = 0; i < m_Passes.Size(); ++i)
		m_Context.GetView()->DebugRenderPass(m_Passes[i].first);
	m_Context.GetView()->DebugRender();

	// after we've played back the command buffers on the main thread the main thread context needs to have its state reset
	m_Context.GetSceneGraph()->GetContext()->InvalidateState();
}

/// constructor
C_RenderTask::C_RenderTask()
: m_pContexts(NULL)
{
#if PLATFORM_PS3
	m_iRenderingDataContextHandle	= RenderingData::Target::g_iContextHandleContext;
	m_iFXContextHandle				= FX::g_iContextHandleContext;
	m_iDisplayContextHandle			= Display::g_iContextHandleC_Context;
#endif
}

/// execute task
void C_RenderTask::Schedule(S32 schedulerGroup, S32 policy)
{
#if PLATFORM_PS3
	m_CommandBuffer.m_Size = 0;
	m_CommandBuffer.m_Offset = UINT_MAX;
	m_CommandBuffer.m_pAddress = NULL;
	m_CommandBuffer.m_EndOffset = UINT_MAX;
	m_CommandBuffer.m_Destination = UINT_MAX;
#endif

	if (m_MultiThreaded)
	{
#if PLATFORM_PS3
		#if !CFG_GOLD
		if (!m_bAbstract && g_RunPPUForceDeferred)
			m_bAbstract = true;
		#endif

		if (m_bAbstract)
		{
			if (g_RunPPUTasksDeferred)
			{
				//LOGINFO(Display::g_Log, "PPU size (%8i) addr (0x%.8x) start (%.8x) end (%.8x) diff (%8i) dest (%.8x) name (%s)\n",
					//m_CommandBuffer.m_Size, m_CommandBuffer.m_pAddress, m_CommandBuffer.m_Offset, m_CommandBuffer.m_EndOffset,
					//m_CommandBuffer.m_EndOffset - m_CommandBuffer.m_Offset, m_CommandBuffer.m_Destination, m_CommandBuffer.m_Name);

				Scheduler::AddTask(SceneGraphRenderTaskPPU, (void*)this, 0, schedulerGroup, policy);
			}
			else
			{
				m_MultiThreaded = false;
			}
		}
		else
#endif
		{
			//LOGINFO(Display::g_Log, "SPU size (%8i) addr (0x%.8x) start (%.8x) end (%.8x) diff (%8i) dest (%.8x) name (%s)\n",
				//m_CommandBuffer.m_Size, m_CommandBuffer.m_pAddress, m_CommandBuffer.m_Offset, m_CommandBuffer.m_EndOffset,
				//m_CommandBuffer.m_EndOffset - m_CommandBuffer.m_Offset, m_CommandBuffer.m_Destination, m_CommandBuffer.m_Name);

			Scheduler::AddTask(SceneGraphRenderTaskSPU, (void*)this, 0, schedulerGroup, policy);
		}
	}
}

} // namespace SceneGraph
