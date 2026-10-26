#include "stdafx.h"

#if PLATFORM_SPU
#define RENDERING_SPU_HACK 1
#endif

#include "SceneGraph/Context.h"
#include "SceneGraph/SceneGraphImpl.h"
#include "SceneGraph/RenderPass.h"
#include "SceneGraph/RenderStates.h"
#include "SceneGraph/RenderTask.h"
#include "SceneGraph/View.h"
#include "SceneGraph/RenderNode.h"
#include "SceneGraph/RendererInterface.h"
#include "SceneGraph/renderingdata/StaticRenderListNode.h"
#include "SceneGraph/renderingdata/ModelInstanceNode.h"

//#include "renderingdata/renderingdata.h"
#include "btl/utility/autopointer.h"
#include "btl/containers/vector.h"
#include "btl/containers/bitvector.h"
#include "btl/utility/string.h"
#include "btl/utility/scopedpointer.h"
#include "btl/utility/hash.h"
#include "btl/Allocators/stackallocator.h"
#include <System/Serialise.h>
#include "renderingdata/context.h"
#include <system/scheduler/jobqueue_memutil.h>
#include <system/debug/Variables.h>

#define FX_ALLOW_INCLUDE	// required for calling of InvalidateState, BeginCommandBuffer, EndCommandBuffer
#include <fx/FxContext.h>
#include <display/context/Context.h>

namespace SceneGraph { void RenderTaskFunc(Scheduler::Task* _pTask); }

JQ_DECLARE_DUAL_MODULE(SceneGraphRenderTask, SceneGraph::RenderTaskFunc, scenegraph_rendermodule, "graphics.scenegraph")
#if !PLATFORM_SPU
JQ_DECLARE_PPU_MODULE(SceneGraphRenderTaskPPU, SceneGraph::RenderTaskFunc)
JQ_DECLARE_SPU_MODULE(SceneGraphRenderTaskSPU, SceneGraph::RenderTaskFunc, scenegraph_rendermodule, "graphics.scenegraph")
#endif

#define MODEL_PVS_LONGSEGMENTS 16
#define MODEL_PVS_LATSEGMENTS 8
#define MODEL_PVS_DISTSEGMENTS 4

#if PLATFORM_SPU
namespace Maths
{
	const vector unsigned int Const< vector unsigned int >::m_AbsMask  = (vector unsigned int)( 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF );
}
#endif

namespace SceneGraph
{

DECLARE_DEBUG_BOOL(g_RenderingDataRender,			"Graphics.SceneGraph.RenderingData.Render", true);
DECLARE_DEBUG_BOOL(g_ModelInstanceLods,				"Graphics.SceneGraph.RenderingData.ModelInstanceLods", true);
DECLARE_DEBUG_BOOL(g_bProfileNames,					"Graphics.SceneGraph.ShowNamesInProfiler", true);
DECLARE_DEBUG_BOOL(g_RenderingDynamicDataRender,	"Graphics.SceneGraph.RenderingData.DynamicRender", true);
DECLARE_DEBUG_BOOL(g_bUseModelPVS,					"Graphics.SceneGraph.UseModelPVS", true);
#if !CFG_SPEED && !PLATFORM_SPU
DECLARE_DEBUG_INT(g_iForceModelPVSLongitude,		"Graphics.SceneGraph.OverrideModelPVSLongitude", 0, Debug::Range(0, MODEL_PVS_LONGSEGMENTS));
DECLARE_DEBUG_INT(g_iForceModelPVSLatitude,			"Graphics.SceneGraph.OverrideModelPVSLatitude", 0, Debug::Range(0, MODEL_PVS_LATSEGMENTS));
DECLARE_DEBUG_INT(g_iForceModelPVSDistance,			"Graphics.SceneGraph.OverrideModelPVSDistance", 0, Debug::Range(0, MODEL_PVS_DISTSEGMENTS));
#endif
//CWright, #21623 - changed these for perf run, 70->140, 100->150
DECLARE_DEBUG_FLOAT(g_fModelPVSDistanceScale,		"Graphics.SceneGraph.ModelPVSDistanceScale", 140.f, Debug::Range(0.f, 10000.f));
DECLARE_DEBUG_FLOAT(g_fModelLODDistanceScale,		"Graphics.SceneGraph.ModelLODDistanceScale", 150.f, Debug::Range(0.f, 10000.f));
DECLARE_DEBUG_FLOAT(g_fModelPVSDisableDistance,		"Graphics.SceneGraph.ModelPVSDisableDistance", 3.f, Debug::Range(0.f, 100.f));

/// task entry point
void RenderTaskFunc(Scheduler::Task* _pTask)
{
	C_RenderTask *pTask = (C_RenderTask*)_pTask->Input;

#if !PLATFORM_SPU
	// get the context for this thread
	int i = Scheduler::GetWorkerID();
	C_Context *pContext = pTask->GetContext(i);
#else
	// on the SPU, the context is created on the stack
	C_Context context, *pContext = &context;
#endif

	// render the task
	pContext->RenderTask(pTask);
}

C_Context::C_Context()
{
	m_pRD = RenderingData::Target::GetContext(this);
	m_pFX = FX::GetContext(this);
	m_pDisplay = Display::GetContext(this);
}

C_Context::~C_Context()
{
}

void DumpMemFree()
{
#if PLATFORM_SPU
	int x;
	LOGINFO( 0, "SPU RenderModule Free memory: %d\n", (U32)&x - (U32)Scheduler::_jqThisWorker->MemBase);
#endif
}

/// clone the data in the given context into our own context
void C_Context::CloneState(const C_Context *pContext)
{
	if (pContext)
	{
	#if PLATFORM_SPU
		m_pSceneGraph = pContext->m_pSceneGraph;
	#endif
		m_pView = (const C_View*)Spu::FetchAllocAsync(pContext->m_pView, (sizeof(C_View) + 15) & ~15, 0);
		m_RenderFlags = pContext->m_RenderFlags;
	}
	else
	{
#if !PLATFORM_SPU
		m_RenderFlags = m_pSceneGraph->GetRenderFlags();
#else
		assert(0);
#endif
	}

	InvalidateState();

#if !PLATFORM_SPU
	C_DeviceContext::CloneState(pContext);
#endif
}

void C_Context::InvalidateState()
{	
	m_pRenderListNode = NULL;
}

#if !PLATFORM_SPU
void C_Context::SetRenderPassIndex(U32 renderPass)
{
	SetRenderPassIndex(renderPass, m_pSceneGraph->GetRenderPass(renderPass));
}
#endif

void C_Context::SetRenderPassIndex(U32 renderPass, const C_RenderPass *pRenderPass)
{
	m_RenderPassIndex = renderPass;
	m_RenderPassFlag = (1<<renderPass);

	RenderingData::RenderPass sceneRenderPass = (RenderingData::RenderPass)pRenderPass->GetRenderingDataPass();
	m_pRD->SetPass(sceneRenderPass);
}

void C_Context::SetAbstract(bool a)
{
	m_pRD->SetAbstract(a);
}

// begin rendering to the command buffer in this context
void C_Context::Begin(Display::C_CommandBuffer *pCommandBuffer)
{
	m_pDisplay->BeginCommandBuffer(pCommandBuffer);
}

// finish rendering to the command buffer in this context
void C_Context::End()
{
	m_pDisplay->EndCommandBuffer();
}

/// this is the entry function for rendering a single RenderTask
void C_Context::RenderTask(C_RenderTask *pTask)
{
#if PLATFORM_SPU
//	DebugBreak();
	PERF_EVENT("SceneGraph.RenderTask");
#else
	PERF_EVENT((g_bProfileNames && pTask->m_pGroup->GetName()) ? pTask->m_pGroup->GetName() : "SceneGraph.RenderTask");
#endif

	Display::C_CommandBuffer* pCommandBuffer = &pTask->m_CommandBuffer;

	// clone the state data from the group which this task is rendering for
	Spu::FetchAlloc(pTask);
	const C_Context *pContext = &pTask->GetRenderGroup()->GetContext();
	Spu::FetchAlloc(pContext);
	CloneState(pContext);

#if PLATFORM_SPU
	// on the SPU we explicitly clone the state of all of the contexts that we support
	// FIXME: need to get the source context pointer from pTask->GetRenderGroup()->GetContext() instead of local context
	m_pRD->Initialise();
	m_pRD->CloneState( pContext->GetContext(pTask->m_iRenderingDataContextHandle) );
	m_pFX->Initialise();
	m_pFX->CloneState( pContext->GetContext(pTask->m_iFXContextHandle) );
	m_pDisplay->Initialise();
	m_pDisplay->CloneState( pContext->GetContext(pTask->m_iDisplayContextHandle) );
#endif

	if (pTask->GetMultiThreaded())
	{
		Begin(pCommandBuffer);
	}
	else
		Begin(NULL);

	Spu::Wait(0);

	for(S32 i = 0; i < pTask->m_Subtasks.Size(); i++)
	{
		RenderTask(&pTask->m_Subtasks[i], pTask->m_Subtechnique, pTask->m_DistanceSubtechnique, pTask->m_MultiThreaded);
	}

	End();
}

/// this is the function for rendering a single subtask
void C_Context::RenderTask(const RenderSubtask *pTask, const U32 subtechnique, const U32 distanceSubtechnique, const bool multiThreaded)
{
	PERF_EVENT("SceneGraph.Render");

	Spu::MemBaseMarker marker;

	U32 renderPass = pTask->pass;

	const C_VisibleNodeList* pVisibleNodeList = pTask->pVisibleNodeList;
	m_pRenderPass = m_pSceneGraph->GetRenderPass(renderPass);
	Spu::FetchAlloc(m_pRenderPass);
	SetRenderPassIndex(renderPass, m_pRenderPass);

	U32 subtechniqueHandle = (m_pView->GetSubtechniqueHandle() | subtechnique | m_pRenderPass->GetSubtechniqueHandle()) & m_pRenderPass->GetSubtechniqueMask();
	U32 distanceSubtechniqueHandle = (m_pView->GetDistanceSubtechniqueHandle() | distanceSubtechnique | m_pRenderPass->GetSubtechniqueHandle()) & m_pRenderPass->GetSubtechniqueMask();

	const RenderStates* pRenderStates;
	pRenderStates = pTask->pRenderStates ? pTask->pRenderStates : m_pRenderPass->GetRenderStates();
	BIZ_ASSERT_SPEED(pRenderStates);
	Spu::FetchAlloc(pRenderStates);

	// set per pass render states eg stencil, alpha etc
	if(m_RenderFlags.setStencilState)
		m_pDisplay->SetStencilState(pRenderStates->stencilState);

	// colour writes don't work properly inside the command buffer
	if(m_RenderFlags.setColourWrite)
		m_pDisplay->SetColourWriteEnable(pRenderStates->colourWriteEnable);

	m_pFX->SetRenderFlags(pRenderStates->renderFlagsFX);
	m_pRD->SetRenderFlags(pRenderStates->renderFlagsRD);

	if(pRenderStates->setBlendMode)
	{
		// disable blending and colour writes
		m_pDisplay->SetBlendEnable(pRenderStates->blendEnable);
		m_pDisplay->SetBlendState(pRenderStates->srcBlend, pRenderStates->destBlend, pRenderStates->blendOp);
		m_pDisplay->SetAlphaTestState(pRenderStates->alphaTestEnable, pRenderStates->alphaRef);
		m_pDisplay->SetAlphaToMaskEnable(pRenderStates->alphaToMaskEnable);
	}

	if(pRenderStates->setDepth)
	{
		m_pDisplay->SetDepthWriteEnable(pRenderStates->depthWriteEnable);
		m_pDisplay->SetDepthFunc(pRenderStates->depthFunc);
	}

	if(pRenderStates->setPixelShader)
	{
		m_pDisplay->SetPixelShader(pRenderStates->pPixelShader);
	}

	bool predication = false;

	assert(pVisibleNodeList);
	Spu::FetchAlloc(pVisibleNodeList);

	RenderAbstracts(pVisibleNodeList, pTask->abstractStart, pTask->abstractEnd, subtechniqueHandle, distanceSubtechniqueHandle, predication);

	if(m_pRenderPass->GetDrawOrder() == E_DrawOrder_InstancesPrimitives)
	{
		RenderInstances(pVisibleNodeList, pTask->instanceStart, pTask->instanceEnd, subtechniqueHandle, distanceSubtechniqueHandle, predication);
		RenderPrimitives(pVisibleNodeList, pTask->primitiveStart, pTask->primitiveEnd, subtechniqueHandle, distanceSubtechniqueHandle, predication);
	}
	else
	{
		RenderPrimitives(pVisibleNodeList, pTask->primitiveStart, pTask->primitiveEnd, subtechniqueHandle, distanceSubtechniqueHandle, predication);
		RenderInstances(pVisibleNodeList, pTask->instanceStart, pTask->instanceEnd, subtechniqueHandle, distanceSubtechniqueHandle, predication);
	}
	RenderModelInstances(pVisibleNodeList, pTask->modelInstanceStart, pTask->modelInstanceEnd, subtechniqueHandle, distanceSubtechniqueHandle, predication);


	m_pFX->SetRenderFlags(FX::RenderFlags());
	m_pRD->SetRenderFlags(RenderingData::Target::RenderFlags());

	if(pRenderStates->setDepth)
		m_pDisplay->SetDepthFunc(Display::E_CmpFunc_Greater);
}

/// draw abstracts in a sub task
void C_Context::RenderAbstracts(const C_VisibleNodeList* pVisibleNodeList, const S32 start, const S32 end, U32 subtechniqueHandle, U32 distanceSubtechniqueHandle, bool predication)
{
	// abstract nodes with virtual render call - eg alpha sorted stuff
	PERF_EVENT("SceneGraph.Render.Abstracts");
	Spu::Prefetcher pf;

	SetAbstract(true);

	// abstract nodes are potentially rendered in batches - avoid needless state setting/unsetting
	for(S32 i = start; i < end; ++i)
	{
		C_VisibleNodeBatch visibleNodeBatch;
		Spu::Fetch(&visibleNodeBatch, &pVisibleNodeList->m_NodeBatches[i], sizeof(C_VisibleNodeBatch));

		// switch on node type to avoid virtuals if possible
		switch(visibleNodeBatch.m_NodeType)
		{
		case E_RenderNodeType_StaticPrimitive:
			{
				for(S32 i = visibleNodeBatch.m_StartIndex; i <= visibleNodeBatch.m_EndIndex; ++i)
				{
					const C_VisibleNode* pNode = pf.FetchNext(&pVisibleNodeList->m_Nodes[i]);
					// distance subtechnique
					if(m_pView->GetViewData().distanceSubtechniqueEnable && pNode->m_VolatileNodeInfo.distanceSubtechnique)
						subtechniqueHandle = distanceSubtechniqueHandle;

					const C_StaticPrimitive* pStaticPrimitive = (const C_StaticPrimitive*)pNode->GetNode();
					Render(pStaticPrimitive, subtechniqueHandle);
				}
			}
			break;
		case E_RenderNodeType_StaticInstance:
			{
				for(S32 i = visibleNodeBatch.m_StartIndex; i <= visibleNodeBatch.m_EndIndex; ++i)
				{
					const C_VisibleNode* pNode = pf.FetchNext(&pVisibleNodeList->m_Nodes[i]);
					// distance subtechnique
					if(m_pView->GetViewData().distanceSubtechniqueEnable && pNode->m_VolatileNodeInfo.distanceSubtechnique)
						subtechniqueHandle = distanceSubtechniqueHandle;

					const C_StaticInstance* pStaticInstance = (const C_StaticInstance*)pNode->GetNode();
					Render(pStaticInstance, subtechniqueHandle);
				}
			}
			break;
		case E_RenderNodeType_ModelInstance:
			{
				for(S32 i = visibleNodeBatch.m_StartIndex; i <= visibleNodeBatch.m_EndIndex; ++i)
				{
					const C_VisibleNode* pNode = pf.FetchNext(&pVisibleNodeList->m_Nodes[i]);
					// distance subtechnique
					if(m_pView->GetViewData().distanceSubtechniqueEnable && pNode->m_VolatileNodeInfo.distanceSubtechnique)
						subtechniqueHandle = distanceSubtechniqueHandle;

					const C_ModelInstance* pModelInstance = (const C_ModelInstance*)pNode->GetNode();
					Render(pModelInstance, subtechniqueHandle, m_pView->GetTypeFlag());
				}
			}
			break;
	#if !PLATFORM_SPU // SPU can't do virtuals...
		default:
			{
				// see if there is a renderer associated with this node type
				if(const I_Renderer* pRenderer = m_pSceneGraph->GetRenderer(visibleNodeBatch.m_NodeType))
				{
					// there is so render the whole batch
					pRenderer->Render(*m_pView, *this, pVisibleNodeList->m_Nodes, visibleNodeBatch);
				}
				else
				{
					// use a virtual call if we don't know what this is
					for(S32 i = visibleNodeBatch.m_StartIndex; i <= visibleNodeBatch.m_EndIndex; ++i)
					{
						const I_RenderNode* pNode = pVisibleNodeList->m_Nodes[i].GetNode();
						pNode->Render(*this);
					}
				}
			}
			break;
	#endif
		}
	}
	SetAbstract(false);
}

/// draw primitives in a sub task
void C_Context::RenderPrimitives(const C_VisibleNodeList* pVisibleNodeList, const S32 start, const S32 end, U32 subtechniqueHandle, U32 distanceSubtechniqueHandle, bool predication)
{
	PERF_EVENT("SceneGraph.Render.Primitives");
	Spu::Prefetcher pf;
	
	for(S32 i = start; i < end; ++i)
	{
		const C_VisibleNode *pNode = pf.FetchNext(&pVisibleNodeList->m_StaticPrimitives[i]);

		// distance subtechnique
		if(m_pView->GetViewData().distanceSubtechniqueEnable && pNode->m_VolatileNodeInfo.distanceSubtechnique)
			subtechniqueHandle = distanceSubtechniqueHandle;

		const C_StaticPrimitive* pStaticPrimitive = pNode->GetNode<C_StaticPrimitive>();
		
		Render(pStaticPrimitive, subtechniqueHandle);
	}
}

// draw instances in a sub task
void C_Context::RenderInstances(const C_VisibleNodeList* pVisibleNodeList, const S32 start, const S32 end, U32 subtechniqueHandle, U32 distanceSubtechniqueHandle, bool predication)
{
	PERF_EVENT("SceneGraph.Render.Instances");
	Spu::Prefetcher pf;
	
	for(S32 i = start; i < end; ++i)
	{
		const C_VisibleNode *pNode = pf.FetchNext(&pVisibleNodeList->m_StaticInstances[i]);

		// distance subtechnique
		if(m_pView->GetViewData().distanceSubtechniqueEnable && pNode->m_VolatileNodeInfo.distanceSubtechnique)
			subtechniqueHandle = distanceSubtechniqueHandle;

		const C_StaticInstance* pStaticInstance = pNode->GetNode<C_StaticInstance>();

		Render(pStaticInstance, subtechniqueHandle);
	}
}

// draw model instances in a sub task
void C_Context::RenderModelInstances(const C_VisibleNodeList* pVisibleNodeList, const S32 start, const S32 end, U32 subtechniqueHandle, U32 distanceSubtechniqueHandle, bool predication)
{
	PERF_EVENT("SceneGraph.Render.ModelInstances");
	Spu::Prefetcher pf;

	for(S32 i = start; i < end; ++i)
	{
		const C_VisibleNode *pNode = pf.FetchNext(&pVisibleNodeList->m_ModelInstances[i]);

		// distance subtechnique
#if !PLATFORM_SPU
		if(m_pView->GetViewData().distanceSubtechniqueEnable && pNode->m_VolatileNodeInfo.distanceSubtechnique)
			subtechniqueHandle = distanceSubtechniqueHandle;
#endif

		const C_ModelInstance* pModelInstance = pNode->GetNode<C_ModelInstance>();


		Render(pModelInstance, subtechniqueHandle, m_pView->GetTypeFlag());
	}
}

/// render a C_StaticPrimitive
void C_Context::Render(const C_StaticPrimitive *pPrim, U32 subtechniqueHandle)
{
	if(!g_RenderingDataRender)
		return;

	SetContextFlags(m_pRD->GetFlags());
//	m_pFX->UploadGlobalBindings();	

	Spu::FetchLocal<C_StaticPrimitive> prim(pPrim);
	BindRenderListNode(prim->GetNode());

	m_pRD->RenderPrimitive(prim->GetPrimitiveIndex(), subtechniqueHandle);
}

/// render a C_StaticInstance
void C_Context::Render(const C_StaticInstance *pPrim, U32 subtechniqueHandle)
{
	if (!g_RenderingDataRender)
		return;

#ifndef PLATFORM_SPU
	if (!g_RenderingDynamicDataRender)
	{
		if (pPrim->GetPaletteId() >= 0)
			return;
	}
#endif

	SetContextFlags(m_pRD->GetFlags());

	Spu::FetchLocal<C_StaticInstance> prim(pPrim);
	BindRenderListNode(prim->GetNode());

	//TODO: [rthomas] If this out for stuff that doesn't need dynamic material lists?
	{
		const RenderingData::Target::MaterialList* pMaterialList = NULL;
		const RenderingData::Target::MaterialList* pMaterialListDmaData = NULL;
		
		S32 offset = 0;
		S32 count = 0;
		
		m_pSceneGraph->GetDynamicMaterialList(&pMaterialList, &pMaterialListDmaData, prim->GetPaletteId(), offset, count);

		m_pRD->SetDynamicMaterialListData(1, pMaterialList, pMaterialListDmaData, offset, count);
		m_pRD->SetDynamicMaterialListData(0, NULL, NULL, 0, 0);
	}

	m_pRD->RenderPrimitiveInstance(prim->GetInstanceIndex(), subtechniqueHandle);
}

void C_Context::BindRenderListNode(const C_StaticRenderListNode *pNode)
{
	if (pNode == m_pRenderListNode)
		return;

	m_pRenderListNode = pNode;

	Spu::FetchLocal<C_StaticRenderListNode> node(pNode);

	// NOTE: addresses passed into SetScene and SetNode are PPU addresses
#if !PLATFORM_SPU
	m_pRD->SetScene(node->GetScene());
	m_pRD->SetNode(node->GetRenderListNode());
#else
	m_pRD->SetNode(node->GetRenderListNode(), node->GetNodeDmaData());
#endif

	m_pRD->SetDynamicMaterialListData(2, node->m_pLevelMaterialListData, node->m_pLevelMaterialListDmaData, 0, 0);
}

void C_Context::BindModelInstance(const C_ModelInstance* pModel, U32 renderPass)
{
	DCBT(0, pModel->m_pMaterialListData);

	SetContextFlags(m_pRD->GetFlags());
	m_pRenderListNode = NULL;

#if !PLATFORM_SPU
#if !CFG_SPEED
	m_pRD->SetScene(pModel->m_pScene);
#endif
	m_pRD->SetNode(pModel->m_pRenderListNode);
 	
 	// TODO: [Stove] Figure out offset.
	if(pModel->GetDynamicVertexBuffers().Size())
		m_pRD->SetDynamicVertexBuffers(pModel->GetDynamicVertexBuffers().First(), pModel->GetRenderListNode()->GetNumVertexBuffers());
 	if(pModel->GetDynamicPrimitiveStreams().Size())
		m_pRD->SetDynamicStreamData(pModel->GetDynamicPrimitiveStreams().First(), pModel->GetDynamicPrimitiveStreams().Size(), 0);

#else
	m_pRD->SetNode(pModel->m_pRenderListNode, pModel->GetNodeDmaData());

	// TODO: [Stove] Figure out the offset.
  	if(pModel->m_DynamicVertexBuffers.Size())
  	{
  		RenderingData::Target::VertexBufferData* vertexBuffers = (RenderingData::Target::VertexBufferData*)Spu::Alloc((pModel->m_DynamicVertexBuffers.Size() * sizeof(RenderingData::Target::VertexBufferData) +0xf) & ~0xf);
		Spu::Fetch((void*)vertexBuffers, (void*)pModel->m_DynamicVertexBuffers.First(), (pModel->m_DynamicVertexBuffers.Size() * sizeof(RenderingData::Target::VertexBufferData) +0xf) & ~0xf);
		m_pRD->SetDynamicVertexBuffers(vertexBuffers, pModel->m_DynamicVertexBuffers.Size());
 	}
 	if(pModel->m_DynamicVertexBufferPrimitiveStreams.Size())
  	{
  		RenderingData::Target::PrimitiveStream* primitiveStreams = (RenderingData::Target::PrimitiveStream*)Spu::Alloc((pModel->m_DynamicVertexBufferPrimitiveStreams.Size() * sizeof(RenderingData::Target::PrimitiveStream) + 0xf) & ~0xf);
		Spu::Fetch((void*)primitiveStreams, (void*)pModel->m_DynamicVertexBufferPrimitiveStreams.First(), (pModel->m_DynamicVertexBufferPrimitiveStreams.Size() * sizeof(RenderingData::Target::PrimitiveStream) +0xf) & ~0xf);
		m_pRD->SetDynamicStreamData(primitiveStreams, pModel->m_DynamicVertexBufferPrimitiveStreams.Size(), 0);
	}
#endif
	
	m_pRD->SetPass((RenderingData::RenderPass)renderPass);
	
	// bind the material output on the model instance to the render data
	{
#if !PLATFORM_SPU && !CFG_SPEED
		m_pRD->SetDynamicMaterialListTypes(0, pModel->m_pMaterialListTypes);
#endif
		m_pRD->SetDynamicMaterialListData(0, pModel->m_pMaterialListData, pModel->m_pMaterialListDmaData, 0);
		m_pRD->SetDynamicMaterialListData(1, NULL, NULL, 0);
		m_pRD->SetMaterial(pModel->m_MaterialIndex);
	}
}

/// render a C_ModelInstance
void C_Context::Render(const C_ModelInstance *pModel, U32 subtechniqueHandle, U32 typeFlag)
{

#if PLATFORM_SPU
	PERF_EVENT("SceneGraph.Render.ModelInstance");
#else
	PERF_EVENT(g_bProfileNames ? pModel->m_pszFriendlyName : "SceneGraph.Render.ModelInstance");
#endif

	Spu::MemBaseMarker marker;

	Spu::FetchLocal<C_ModelInstance> model((C_ModelInstance*)pModel);

	// pass cull
	if (!model->m_pRenderListNode || !g_RenderingDataRender)
		return;

#if PLATFORM_SPU
	RenderingData::Target::RenderListNode *pRenderListNode = m_pRD->GetLocalRenderListCopy();
#else
	const RenderingData::Target::RenderListNode *pRenderListNode = model->m_pRenderListNode;
#endif

	bool bBound = false;

	RenderingData::RenderPass renderPass = (RenderingData::RenderPass)Spu::Get(&model->m_SgToRdPassMapping[GetRenderPassIndex()]);
		
	BIZ_ASSERT(renderPass >= RenderingData::RENDER_PASS_OPAQUE && renderPass < RenderingData::RENDER_PASS_MAX);
	
	// get subtechnique handle for this model
	subtechniqueHandle = model->GetSubtechniqueHandle() | subtechniqueHandle;

	// get scaled draw distance so we can reject primitives
	F32 distanceMin = 0.0f;
	F32 distanceMax = 0.0f;
	int iLod = 0;
	
	const I_ModelInstanceCullNode *pCullNode = model->GetCullNode();

	if(pCullNode && m_pView && g_ModelInstanceLods)
	{
		Spu::FetchAlloc(pCullNode);
		Maths::HVec4 centreW(Maths::Load<Maths::HVec4>(&pCullNode->centre));
		Maths::HVec4 forward(Maths::Load<Maths::HVec4>(&m_pView->GetViewData().forwardZscaled));
		Maths::HF32 drawScale = m_pView->GetViewData().modelInstanceDrawDistanceScaleRcp;
		
		F32 distance = (centreW | forward) * drawScale;
		F32 radius = pCullNode->GetRadius() * drawScale;

		distanceMin = distance + radius;
		distanceMax = distance - radius;

		iLod = 4 - (int)(g_fModelLODDistanceScale / distanceMin);
		iLod = Maths::Max(0, iLod);
	}

	const C_ModelInstancePrimitive *pPrimitiveNodes = &model->m_PrimitiveNodes[0];
	Spu::FetchAllocArray(pPrimitiveNodes, model->m_PrimitiveNodes.Size());

#if PLATFORM_SPU
	pRenderListNode->SpuFetchRenderPassInfo(model->GetNodeDmaData());
#endif

	const RenderingData::RenderListNode::RenderPassInfo &renderPassInfo = pRenderListNode->GetRenderPassInfo(renderPass);

	const U8 *pModelPVS = NULL;
	U8 iModelPvsBit = 0;
	Maths::HMat43 *pParentMtx = (Maths::HMat43*)model->GetParentMatrixPtr();
	if (g_bUseModelPVS && m_pView && pParentMtx)
	{
		Spu::FetchAlloc(pParentMtx);

		// get view vector
		Maths::HVec3 centre = pParentMtx->AW();
		Maths::HVec3 vView = m_pView->GetViewData().viewSettings3D.GetCameraPosition() - centre;
		Maths::HF32 fDist = vView.Length();
		if (fDist > g_fModelPVSDisableDistance)
		{
#if PLATFORM_SPU
			pRenderListNode->SpuFetchPvsInfo(model->GetNodeDmaData());
#endif
			vView = vView / fDist;
			vView = vView * pParentMtx->Rotation().Transposed();

			float fLong = (F32)(atan2(vView.Z(), vView.X()) / M_PI);
			float fLat = vView.Y();

			int iLong = (int)(((fLong + 1.f) * 0.5f) * (F32)MODEL_PVS_LONGSEGMENTS);
			int iLat = (int)(((fLat + 1.f) * 0.5f) * (F32)MODEL_PVS_LATSEGMENTS);
			iLat &= (MODEL_PVS_LATSEGMENTS-1);
			iLong &= (MODEL_PVS_LONGSEGMENTS-1);
			int iDist = MODEL_PVS_DISTSEGMENTS - (int)(g_fModelPVSDistanceScale / fDist);
			iDist = Maths::Max(0, iDist);
			iDist = Maths::Min(MODEL_PVS_DISTSEGMENTS-1, iDist);

		#if !CFG_SPEED && !PLATFORM_SPU
			if (g_iForceModelPVSLongitude)
				iLong = g_iForceModelPVSLongitude-1;
			if (g_iForceModelPVSLatitude)
				iLat = g_iForceModelPVSLatitude-1;
			if (g_iForceModelPVSDistance)
				iDist = g_iForceModelPVSDistance-1;
		#endif

			int iCell = iLong + iLat*MODEL_PVS_LONGSEGMENTS + iDist*(MODEL_PVS_LONGSEGMENTS*MODEL_PVS_LATSEGMENTS);

			pModelPVS = pRenderListNode->GetPrimitivePvs(iCell);

			if (pModelPVS)
			{
				U32 iPrimMod = renderPassInfo.primitiveOffset&7;
				iModelPvsBit = 1<<(iPrimMod);
				pModelPVS += renderPassInfo.primitiveOffset/8;
				U32 modelPVsSize = Btl::AlignUp(renderPassInfo.nPrimitives + iPrimMod, 8) / 8;
				if (pModelPVS && renderPassInfo.nPrimitives)
					pModelPVS = (U8*)Spu::FetchAllocUnaligned(pModelPVS, modelPVsSize);
			}
		}
	}

	if(model->m_VisibleElements != model->m_Elements)
	{
		Btl::SpuFetch(model->m_VisibleElementFlags.GetBuffer());

		// render a subset of the list if some elements are disabled
		if(model->m_PerPrimitiveGather)
		{
			for(S32 i = renderPassInfo.primitiveOffset; i < renderPassInfo.primitiveOffset + renderPassInfo.nPrimitives; ++i)
			{
				if(	((!pModelPVS || (*pModelPVS)&iModelPvsBit) &&
					model->m_VisibleElementFlags.GetBitValue(pPrimitiveNodes[i].GetElementIndex()) &&
					pPrimitiveNodes[i].IsVisibleType(typeFlag) &&
					pPrimitiveNodes[i].IsVisibleDistance(distanceMin, distanceMax)))
				{
					if (!bBound) { BindModelInstance(model.Ptr(), renderPass); bBound = true; }
					m_pRD->RenderPrimitive(i, subtechniqueHandle, iLod);
				}

				if (pModelPVS)
				{
					iModelPvsBit<<=1; if (!iModelPvsBit) { iModelPvsBit = 1; pModelPVS++; }
				}
			}
		}
		else
		{
			for(S32 i = renderPassInfo.primitiveOffset; i < renderPassInfo.primitiveOffset + renderPassInfo.nPrimitives; ++i)
			{
				if(	((!pModelPVS || (*pModelPVS)&iModelPvsBit) &&
					model->m_VisibleElementFlags.GetBitValue(pPrimitiveNodes[i].GetElementIndex()) &&
					pPrimitiveNodes[i].IsVisibleDistance(distanceMin, distanceMax)))
				{
					if (!bBound) { BindModelInstance(model.Ptr(), renderPass); bBound = true; }
					m_pRD->RenderPrimitive(i, subtechniqueHandle, iLod);
				}

				if (pModelPVS)
				{
					iModelPvsBit<<=1; if (!iModelPvsBit) { iModelPvsBit = 1; pModelPVS++; }
				}
			}
		}
	}
	else
	{
		// if all elements are visible we can skip checking the primitive flags per primitive
		BindModelInstance(model.Ptr(), renderPass);

		// check to see if we need to do per primitive culling by comparing the gather mask on the context to the primitives
		if(model->m_PerPrimitiveGather)
		{
			for(S32 i = renderPassInfo.primitiveOffset; i < renderPassInfo.primitiveOffset + renderPassInfo.nPrimitives; ++i)
			{
				if(	((!pModelPVS || (*pModelPVS)&iModelPvsBit) &&
					pPrimitiveNodes[i].IsVisibleType(typeFlag) &&
					pPrimitiveNodes[i].IsVisibleDistance(distanceMin, distanceMax)))
					m_pRD->RenderPrimitive(i, subtechniqueHandle, iLod);

				if (pModelPVS)
				{
					iModelPvsBit<<=1; if (!iModelPvsBit) { iModelPvsBit = 1; pModelPVS++; }
				}
			}
		}
		else
		{
			for(S32 i = renderPassInfo.primitiveOffset; i < renderPassInfo.primitiveOffset + renderPassInfo.nPrimitives; ++i)
			{
				if( ((!pModelPVS || (*pModelPVS)&iModelPvsBit) &&
					pPrimitiveNodes[i].IsVisibleDistance(distanceMin, distanceMax)))
					m_pRD->RenderPrimitive(i, subtechniqueHandle, iLod);

				if (pModelPVS)
				{
					iModelPvsBit<<=1; if (!iModelPvsBit) { iModelPvsBit = 1; pModelPVS++; }
				}
			}
		}
	}
	
	// Switch off any dynamic vertex buffers/streams.
	m_pRD->SetDynamicVertexBuffers(NULL, 0);
 	m_pRD->SetDynamicStreamData(NULL, 0, 0);
}


} // namespace SceneGraph

#if PLATFORM_SPU
extern "C" void _exit() {snPause();}

void *operator new[]( unsigned long )
{
	DebugBreak();
}

void *operator new( size_t size )
{
	DebugBreak();
}

void operator delete(void*)
{
	DebugBreak();
}

void operator delete[](void*)
{
	DebugBreak();
}

#endif

