#include "stdafx.h"

#include "SceneGraph/SceneGraphImpl.h"
#include "SceneGraph/Context.h"

#include "SceneGraph/RenderStates.h"
#include "SceneGraph/RenderGroupSettings.h"

#include "SceneGraph/RenderNode.h"
#include "SceneGraph/GatherNode.h"
#include "SceneGraph/View.h"
#include "SceneGraph/MultiView.h"

#include "SceneGraph/RenderingData/StaticScene.h"
#include "SceneGraph/RenderingData/ModelInstanceNode.h"
#include "SceneGraph/RenderingData/ModelInstanceFlatList.h"
#include "SceneGraph/RenderingData/ModelInstancePortalList.h"
#include "SceneGraph/Util.h"
#include "SceneGraph/Visitor.h"
#include "SceneGraph/Util/MemoryUsageVisitor.h"
#include "SceneGraph/Util/DebugRenderVisitor.h"

#include "SceneGraph/RendererInterface.h"

#include "SceneGraph/Visible/ListSorter.h"

#include "SceneGraph/Light/Manager.h"

#include "Display/Shaders/Constants.h"

#include "RenderingData/RenderingData.h"

#include <fx/fx.h>

#include <animation/pose/PoseFormat.h>
#include <animation/pose/PoseFieldTypes.h>

#include <core/maths/hvec/new/IncludeMinimal.h>
#include <core/maths/hvec/new/IncludeMinimal.inl>

#include <system/perf.h>

namespace SceneGraph
{

Debug::Int g_RenderListShowIndex("Graphics.SceneGraph.RenderList.ShowIndex", -1, Debug::Range(-1, 100) );

Debug::Logger &g_Log = Debug::GetLogger("graphics.scenegraph");

Debug::Bool	g_ShowMemoryUsage		("Graphics.SceneGraph.ShowMemoryUsage", false);
Debug::Bool	g_BoundsShowRenderNodes	("Graphics.SceneGraph.Bounds.ShowRenderNodes", false);
Debug::Bool	g_DebugRenderVisitor	("Graphics.SceneGraph.DebugRender.Enable", false);
Debug::Bool g_BoundsNames			("Graphics.SceneGraph.Bounds.ShowNames", false);
Debug::Bool	g_BoundsDrawDistance	("Graphics.SceneGraph.Bounds.DrawDistance", false);
Debug::Bool	g_MultiThreadedGather			("Graphics.SceneGraph.MultiThreadedGather", true);
Debug::Float g_DrawDistanceScale			("Graphics.SceneGraph.DrawDistanceScale", 1.f, Debug::Range(0.f, 1.f));

// C_CachedStaticNodeInternalMaterialData

C_CachedStaticNodeInternalMaterialData::C_CachedStaticNodeInternalMaterialData()
	: m_pMaterialList(NULL),
	m_pMaterialListDmaData(NULL),
	m_MaterialListSize(0),
	m_pPaletteMap(NULL)
{
	ClearPaletteMap();
}

C_CachedStaticNodeInternalMaterialData::~C_CachedStaticNodeInternalMaterialData()
{
	if (m_pMaterialList)
	{
		delete m_pMaterialList;
		m_pMaterialList = NULL;

		m_pMaterialListDmaData = NULL;
	}

	m_pPaletteMap = NULL;
}

void C_CachedStaticNodeInternalMaterialData::ClearPaletteMap()
{
	m_pPaletteMap = NULL;
}

FORCE_INLINE void StoreMat43(MathsStorage::MatrixRow43* dest, const Maths::Mat43& src)
{
	Maths::Store(Maths::ColX(src), &dest->v[0]);
	Maths::Store(Maths::ColY(src), &dest->v[4]);
	Maths::Store(Maths::ColZ(src), &dest->v[8]);
}

FORCE_INLINE void StoreMat43(MathsStorage::MatrixRow44* dest, const Maths::Mat43& src)
{
	Maths::Store(Maths::MakeVector4(Maths::RowX(src)), &dest->v[0]);
	Maths::Store(Maths::MakeVector4(Maths::RowY(src)), &dest->v[4]);
	Maths::Store(Maths::MakeVector4(Maths::RowZ(src)), &dest->v[8]);
	Maths::Store(Maths::MakeVector4(Maths::MakePoint3(Maths::RowW(src))), &dest->v[12]);
}

// C_SceneGraph

C_SceneGraph::C_SceneGraph()
{
	m_JobQueuePolicyGather = -1;
	for(S32 i = 0; i < E_RenderPriority_Count; ++i)
		m_JobQueuePolicyRender[i] = -1;
	m_JobQueueInitialised = false;
	m_ListType = I_ListNode::LISTNODETYPE_FLAT_LIST;
	
	// set the number of node types which the scenegraph inherently knows about
	//SetNumNodeTypes(E_RenderNodeType_Count);
	m_NodeTypeRenderer.Resize(E_RenderNodeType_Count);
	for(S32 i = 0; i < m_NodeTypeRenderer.Size(); ++i)
		m_NodeTypeRenderer[i] = 0;

	for(S32 i = 0; i < m_RenderingDataRenderPassMapping.Size(); ++i)
	{
		m_RenderingDataRenderPassMapping[i] = -1;
	}

	m_pModelInstanceFlatList = new C_ModelInstanceFlatList(this);
	m_pModelInstancePortalList = new C_ModelInstancePortalList(this);
	m_pLightManager = new C_LightManager(this);

	for (int i=0; i<m_Contexts.Size(); i++)
	{
		m_Contexts[i].m_pSceneGraph = this;
		m_Contexts[i].Initialise();
	}

	m_pDefaultRenderStates = new RenderStates;
	m_pDefaultRenderGroupSettings = new C_RenderGroupSettings;

	Insert(m_pModelInstanceFlatList);
	Insert(m_pModelInstancePortalList);
	Insert(m_pLightManager);

	m_WorldParam = FX::GetSharedParameter("g_mWorld");
	m_ViewParam = FX::GetSharedParameter("g_mView");
	m_ProjParam = FX::GetSharedParameter("g_mProj");
	m_ViewProjParam = FX::GetSharedParameter("g_mViewProj");
	m_ViewPositionParam = FX::GetSharedParameter("g_vViewPosition");
	m_LodDistanceScale = FX::GetSharedParameter("g_lodDistanceScale");

	m_pPoseFormat = NULL;

	m_pCachedStaticNodeInternalMaterialData = NULL;
	m_NumMatrixPalettes = 0;

	// reserve reasonable initial sizes for vectors
	m_NewlyVisibleList.Reserve(50);

	m_Frame = 1;
}

C_SceneGraph::~C_SceneGraph()
{
	for(S32 i = 0; i < m_Scenes.Size(); ++i)
	{
		delete m_Scenes[i];
	}

	delete m_pModelInstanceFlatList;
	delete m_pModelInstancePortalList;
	delete m_pLightManager;
	delete m_pDefaultRenderStates;
	delete m_pDefaultRenderGroupSettings;

	m_MatrixPalettes.Destroy();

	while(m_Views.Size() != 0)
		RemoveView(m_Views[0]);

	if (m_pCachedStaticNodeInternalMaterialData)
	{
		delete m_pCachedStaticNodeInternalMaterialData;
		m_pCachedStaticNodeInternalMaterialData = NULL;
	}
}

// return pointer to context to use for rendering. each thread has it's own context to allow multi-threaded rendering
C_Context *C_SceneGraph::GetContext() const
{
	int i = Scheduler::GetWorkerID();
	assert(i>=0 && i<SCENEGRAPH_MAX_CONTEXTS);

	return &m_Contexts[i];
}

bool C_SceneGraph::JobQueueActive() const
{
	return (m_JobQueueInitialised && Scheduler::Active() && g_MultiThreadedGather);
}

void C_SceneGraph::SetJobQueuePolicyGather(Scheduler::PolicyHandle policyHandle)
{
	m_JobQueuePolicyGather = policyHandle;
	SetJobQueueInitialised();
}

void C_SceneGraph::SetJobQueuePolicyRender(Scheduler::PolicyHandle lowHandle, Scheduler::PolicyHandle medHandle, Scheduler::PolicyHandle highHandle)
{
	m_JobQueuePolicyRender[E_RenderPriority_Low] = lowHandle;
	m_JobQueuePolicyRender[E_RenderPriority_Medium] = medHandle;
	m_JobQueuePolicyRender[E_RenderPriority_High] = highHandle;
	SetJobQueueInitialised();
}

void C_SceneGraph::SetJobQueuePolicyRender(Scheduler::PolicyHandle handle)
{
	m_JobQueuePolicyRender[E_RenderPriority_Low] = handle;
	m_JobQueuePolicyRender[E_RenderPriority_Medium] = handle;
	m_JobQueuePolicyRender[E_RenderPriority_High] = handle;
	SetJobQueueInitialised();
}

void C_SceneGraph::SetJobQueueInitialised()
{
	m_JobQueueInitialised = m_JobQueuePolicyGather != -1 &&
							m_JobQueuePolicyRender[E_RenderPriority_Low] != -1 &&
							m_JobQueuePolicyRender[E_RenderPriority_Medium] != -1 &&
							m_JobQueuePolicyRender[E_RenderPriority_High] != -1;
}


void C_SceneGraph::Gather(const C_View& view) const
{
	// gather static scenes
	{
		PERF_EVENT("SceneGraph.Static");
		C_LevelInstance::BeginGather1(view);
		for(S32 i = 0; i < m_Scenes.Size(); ++i)
			m_Scenes[i]->Gather(view);
		C_LevelInstance::BeginGather2(view);
	}

	// gather generics
	{
		// this could be hierachy type things. eg CachedFlatList, or stuff like particle systems
		PERF_EVENT("SceneGraph.Generic");
		for(S32 i = 0; i < m_GenericNodes.Size(); ++i)
			m_GenericNodes[i]->Gather(view);
	}
}

void C_SceneGraph::Gather(const C_MultiView& multiView) const
{
	// gather static scenes
	if(m_Scenes.Size() > 0)
	{
		PERF_EVENT("SceneGraph.Static");
		C_LevelInstance::BeginGather1(multiView);
		for(S32 i = 0; i < m_Scenes.Size(); ++i)
			m_Scenes[i]->Gather(multiView);
		C_LevelInstance::BeginGather2(multiView);
	}

	// gather generics
	{
		// this could be hierachy type things. eg CachedFlatList, or stuff like particle systems
		PERF_EVENT("SceneGraph.Generic");
		for(S32 i = 0; i < m_GenericNodes.Size(); ++i)
			m_GenericNodes[i]->Gather(multiView);
	}
}

void C_SceneGraph::Accept(I_Visitor& visitor) const
{
	for(S32 i = 0; i < m_Scenes.Size(); ++i)
	{
		m_Scenes[i]->Accept(visitor);
	}

	for(S32 i = 0; i < m_GenericNodes.Size(); ++i)
	{
		m_GenericNodes[i]->Accept(visitor);
	}

	visitor.Visit(*this);
}

C_LevelInstance* C_SceneGraph::Insert(C_LevelInstance *pStaticScene)
{
	pStaticScene->Build();
	m_Scenes.PushBack(pStaticScene);

	return pStaticScene;
}

void C_SceneGraph::Remove(C_LevelInstance *pScene)
{
	for(S32 i = 0; i < m_Scenes.Size(); ++i)
	{
		if(m_Scenes[i] == pScene)
		{
			m_Scenes.EraseByIndex(i);
			pScene->Destruct();
			break;
		}
	}
}

void Insert(C_SceneGraph *pSceneGraph, C_LevelInstance *pLevelInstance)
{
	pSceneGraph->Insert(pLevelInstance);
}

void Remove(C_SceneGraph *pSceneGraph, C_LevelInstance *pLevelInstance)
{
	pSceneGraph->Remove(pLevelInstance);
}

void Insert(C_SceneGraph *pSceneGraph, C_ModelInstance* pModelInstance)
{
	if (!pModelInstance->m_pszFriendlyName)
		pModelInstance->m_pszFriendlyName = "<Unknown>";

	pSceneGraph->Insert(pModelInstance);
}

void Remove(C_SceneGraph *pSceneGraph, C_ModelInstance* pModelInstance)
{
	pSceneGraph->Remove(pModelInstance);
}

void C_SceneGraph::Insert(C_ModelInstance* pModelInstance)
{
	switch(m_ListType)
	{
	case I_ListNode::LISTNODETYPE_FLAT_LIST:
		m_pModelInstanceFlatList->Insert(pModelInstance);
		break;

	case I_ListNode::LISTNODETYPE_PORTAL_LIST:
		m_pModelInstancePortalList->Insert(pModelInstance);
		break;

	default:
		break;
	}

	return;
}

void C_SceneGraph::Remove(C_ModelInstance* pModelInstance)
{
	switch(m_ListType)
	{
	case I_ListNode::LISTNODETYPE_FLAT_LIST:
		m_pModelInstanceFlatList->Remove(pModelInstance);
		break;

	case I_ListNode::LISTNODETYPE_PORTAL_LIST:
		m_pModelInstancePortalList->Remove(pModelInstance);
		break;

	default:
		break;
	}

	return;
}

void Insert(C_SceneGraph *pSceneGraph, I_GatherNode* pGatherNode)
{
	pSceneGraph->Insert(pGatherNode);
}

void Remove(C_SceneGraph *pSceneGraph, I_GatherNode* pGatherNode)
{
	pSceneGraph->Remove(pGatherNode);
}

void C_SceneGraph::Insert(I_GatherNode* pGatherNode)
{
	pGatherNode->OnInsert(*this);
	m_GenericNodes.PushBack(pGatherNode);
}

void C_SceneGraph::Remove(I_GatherNode* pGatherNode)
{
	for(S32 i = 0; i < m_GenericNodes.Size(); ++i)
	{
		if(m_GenericNodes[i] == pGatherNode)
		{
			pGatherNode->OnRemove(*this);
			m_GenericNodes.EraseByIndex(i);
		}
	}
}

void SetPoseFormat(C_SceneGraph *pSceneGraph, const Animation::C_PoseFormat* pPoseFormat)
{
	if (pSceneGraph)
		pSceneGraph->SetPoseFormat(pPoseFormat);
}

void SetNumMatrixPalettes(C_SceneGraph *pSceneGraph, const S32 numMatrixPalettes)
{
	if (pSceneGraph)
		pSceneGraph->SetNumMatrixPalettes(numMatrixPalettes);
}

void ClearMatrixPalettes(C_SceneGraph *pSceneGraph, const U32 offset)
{
	if (pSceneGraph)
		pSceneGraph->ClearMatrixPalettes(offset);
}

void SetMatrixPalette(C_SceneGraph *pSceneGraph, const S32 index, const Maths::Mat43* pMatrixPalette)
{
	if (pSceneGraph)
		pSceneGraph->SetMatrixPalette(index, pMatrixPalette);
	//return 0xff;
}

void SetPaletteMap(C_SceneGraph *pSceneGraph, const U8* pPaletteMap)
{
	if (pSceneGraph)
		pSceneGraph->SetPaletteMap(pPaletteMap);
}

const Btl::Vector<SceneGraph::C_VisibleDetect*> &GetNewlyVisibleList(C_SceneGraph *p)
{
	return p->GetNewlyVisibleList();
}

void C_SceneGraph::BeginFrame() const
{
	MEMORY_TRACKER_PUSH("graphics.scenegraph");

	PERF_EVENT("SceneGraph.BeginFrame");

	// call begin frame on all registered renderers
	{
		PERF_EVENT("Renderers");
		for(Renderers::ConstIterator pIterator = m_Renderers.Begin(); pIterator != m_Renderers.End(); pIterator++)
		{
			const I_Renderer* pRenderer = pIterator->GetValue();
			pRenderer->BeginFrame();
		}
	}

	{
		PERF_EVENT("Views");
		for(S32 i = 0; i < m_Views.Size(); ++i)
		{
			m_Views[i]->BeginFrame();
		}
	}

	{
		PERF_EVENT("GenericNodes");
		for(S32 i = 0; i < m_GenericNodes.Size(); ++i)
		{
			m_GenericNodes[i]->BeginFrame();
		}
	}

	{
		PERF_EVENT("StaticScenes");
		for(S32 i = 0; i < m_Scenes.Size(); ++i)
		{
			m_Scenes[i]->BeginFrame();
		}
	}

	{
		PERF_EVENT("SceneGraph.BeginFrame.MatrixPaletteParameters");
		if (m_pCachedStaticNodeInternalMaterialData)
		{
			const U32 poseFieldIndexCount = static_cast<U32>(m_pCachedStaticNodeInternalMaterialData->m_PoseFieldIndexes.Size());

			if (!poseFieldIndexCount)
			{
				const Animation::C_PoseFormat* pPoseFormat = GetPoseFormat();

				if (pPoseFormat)
					BindAnimatedParams(pPoseFormat, GetNumMatrixPalettes());
			}

			if (poseFieldIndexCount)
			{
				for (S32 i = 0, count = GetNumMatrixPalettes(); i < count; ++i)
				{
					if (GetMatrixPalette(i))
					{
						SetJointTransformParameters(m_pCachedStaticNodeInternalMaterialData->m_PoseFieldIndexes, poseFieldIndexCount,
							m_pCachedStaticNodeInternalMaterialData->m_MaterialDataList[i].m_Params.First(), GetMatrixPalette(i));
					}
				}
			}
		}
	}

	m_NewlyVisibleList.Clear();

	C_ModelInstanceFlatList::BeginFrame();

	if(g_ShowMemoryUsage)
	{
		g_ShowMemoryUsage = false;
		C_MemoryUsageVisitor memVisitor;
		Accept(memVisitor);
	}
	
	MEMORY_TRACKER_POP("graphics.scenegraph");
}

void C_SceneGraph::EndFrame() const
{
	++m_Frame;
	{
		PERF_EVENT("Views");
		for(S32 i = 0; i < m_Views.Size(); ++i)
		{
			m_Views[i]->EndFrame();
		}
	}
}

void C_SceneGraph::SetSharedParameters(const Display::C_View& view) const
{
	// setup global matrices
	Display::C_Matrix identity;
	Display::MatrixSetIdentity(&identity);
	FX::SetParameter((FX::ParameterHandle&)m_WorldParam, &identity);
	FX::SetParameter((FX::ParameterHandle&)m_ViewParam, &view.GetViewSettings3D().GetViewMatrix());
	FX::SetParameter((FX::ParameterHandle&)m_ProjParam, &view.GetViewSettings3D().GetProjMatrix());
	FX::SetParameter((FX::ParameterHandle&)m_ViewProjParam, &view.GetViewSettings3D().GetViewProjMatrix());
	FX::SetParameter((FX::ParameterHandle&)m_ViewPositionParam, (void*)&view.GetViewSettings3D().GetViewPosition());
}

void C_SceneGraph::SetSharedParameters(const C_View& view) const
{
	F32 lodDistanceScale = view.GetViewData().instanceDrawDistanceScaleRcp * view.GetViewData().forwardMultiplier;
	FX::SetParameter((FX::ParameterHandle&)m_LodDistanceScale, (void*)&lodDistanceScale);
}

void C_SceneGraph::BeginRender(const Display::C_View& view) const
{
	SetSharedParameters(view);

	if(g_DebugRenderVisitor)
	{
		C_DebugRenderVisitor debugRenderVisitor;
		Accept(debugRenderVisitor);
	}
}

const I_Renderer* C_SceneGraph::GetRenderer(const S32 nodeType) const
{
	if(nodeType > -1 && nodeType < m_NodeTypeRenderer.Size())
		return m_NodeTypeRenderer[nodeType];
	else
		return NULL;
}

void C_SceneGraph::AddRenderer(I_Renderer *pRenderer)
{
	// make sure it isn't already inserted
	assert(m_Renderers.Find(pRenderer) == m_Renderers.End());

	m_Renderers.Insert(pRenderer);

	pRenderer->m_NodeType = m_NodeTypeRenderer.Size();
	pRenderer->m_NodeFlag = 1 << pRenderer->m_NodeType;
	m_NodeTypeRenderer.PushBack(pRenderer);
}

void C_SceneGraph::RemoveRenderer(I_Renderer *pRenderer)
{
	Renderers::Iterator it = m_Renderers.Find(pRenderer);
	assert(it != m_Renderers.End());
	
	m_Renderers.Erase(it);

	for(S32 i = 0; i < m_NodeTypeRenderer.Size(); ++i)
	{
		if(m_NodeTypeRenderer[i] == pRenderer)
			m_NodeTypeRenderer[i] = NULL;
	}
}

// render passes

void C_SceneGraph::SetNumRenderPasses(const S32 renderPasses)
{
	S32 previousSize = m_RenderPasses.Size();
	m_RenderPasses.Resize(renderPasses);
	
	if(renderPasses > previousSize)
	{
		for(S32 i = previousSize; i < renderPasses; ++i)
		{
			m_RenderPasses[i].m_Flags = -1;
			m_RenderPasses[i].m_Index = -1;
			m_RenderPasses[i].m_pSorter = NULL;
		}
	}

	 // make sure all current views are set up with regards to the number of render passes which exist in this scene graph
	for(S32 i = 0; i < m_Views.Size(); ++i)
	{
		m_Views[i]->SetupLists();
	}
}

void C_SceneGraph::RegisterRenderPass(const S32 index, I_VisibleNodeListSorter* pSorter, U32 subtechniqueHandle, U32 subtechniqueMask)
{
	if(index + 1 > m_RenderPasses.Size())
		SetNumRenderPasses(index + 1);

	assert(m_RenderPasses[index].m_Flags == -1);
	assert(index < 32);
	
	m_RenderPasses[index].m_Flags = 1 << index;
	m_RenderPasses[index].m_Index = index;
	m_RenderPasses[index].m_VolatileInfoFlags = pSorter ? pSorter->GetVolatileInfoFlags() : 0;
	m_RenderPasses[index].m_RenderingDataPass = RenderingData::RENDER_PASS_MAX;
	m_RenderPasses[index].m_pSorter = pSorter;
	m_RenderPasses[index].m_Subtechnique = subtechniqueHandle;
	m_RenderPasses[index].m_SubtechniqueMask = ~subtechniqueMask;
	m_RenderPasses[index].m_pRenderStates = m_pDefaultRenderStates;
}

// sets up the default mapping between rendering data passes and arbitrary scenegraph passes
void C_SceneGraph::SetRenderingDataRenderPassMapping(const U32 renderingDataPass, const S32 passType)
{
	if(passType > -1 && passType < m_RenderPasses.Size())
	{
		m_RenderingDataRenderPassMapping[renderingDataPass] = passType;
		C_RenderPass& renderPass = m_RenderPasses[passType];
		renderPass.m_RenderingDataPass = renderingDataPass;
	}
}

S32 C_SceneGraph::GetRenderingDataRenderPassMapping(const U32 renderPass) const
{
	return m_RenderingDataRenderPassMapping[renderPass];
}

C_RenderPass* C_SceneGraph::GetRenderPass(const S32 passType)
{
	if(passType > -1 && passType < m_RenderPasses.Size())
		return &m_RenderPasses[passType];
	else
		return NULL;
}

C_View* C_SceneGraph::AddView()
{
	// create a view and add it to the list of views
	C_View*	pView = new C_View(this);
	pView->SetIndex(m_Views.Size());

	m_Views.PushBack(pView);
	
	return pView;
}

void C_SceneGraph::AddRendererViews(C_View*	pView)
{
	assert(pView->m_RendererViews.Size() == 0);

	// add derived views to registered renderers
	for(Renderers::Iterator pIterator = m_Renderers.Begin(); pIterator != m_Renderers.End(); pIterator++)
	{
		I_Renderer* pRenderer = pIterator->GetValue();
		I_RendererView* pRendererView = pRenderer->AddView(pView);
				
		pView->m_RendererViews.PushBack(pRendererView);
	}
}

void C_SceneGraph::RemoveView(C_View* pView)
{
	assert(pView);

	// erase index and update the new index of 
	S32 viewIndex = pView->GetIndex();
	m_Views.EraseByIndexNonOrdered(viewIndex);
	
	// update new view index if appropriate
	if(viewIndex < m_Views.Size())
		m_Views[viewIndex]->SetIndex(viewIndex);

	RemoveRendererViews(pView);

	delete pView;
}

void C_SceneGraph::RemoveRendererViews(C_View* pView)
{
	// remove derived views
	for(Renderers::Iterator pIterator = m_Renderers.Begin(); pIterator != m_Renderers.End(); pIterator++)
	{
		I_Renderer* pRenderer = pIterator->GetValue();
		pRenderer->RemoveView(pView, pView->GetIndex());
	}

	pView->m_RendererViews.Clear();
}

/// Reserve a specified amount of model instance flat lists, one per portal room.
void C_SceneGraph::ReserveModelInstanceFlatLists(U32 iCount)
{
	m_pModelInstancePortalList->ReserveModelInstanceFlatLists(iCount);
	return;
}

/// Add a portal system object to the scene graph.
void C_SceneGraph::AddPortalSystem(const Portal::C_System* pSystem)
{
	m_vPortalSystems.PushBack(pSystem);
	return;
}

/// Return the number of portal systems used by the scene graph.
NOALIAS const U32 C_SceneGraph::GetPortalSystemCount() const __restrict
{
	return static_cast<U32>(m_vPortalSystems.Size());
}

/// Return a pointer to array of portal system pointers.
const Btl::Vector<const Portal::C_System*>& C_SceneGraph::GetPortalSystemList() const
{
	return m_vPortalSystems;
}

void C_SceneGraph::SetPoseFormat(const Animation::C_PoseFormat *pPoseFormat)
{
	m_pPoseFormat = pPoseFormat;
}

const Animation::C_PoseFormat* C_SceneGraph::GetPoseFormat() const
{
	return m_pPoseFormat;
}

void C_SceneGraph::SetNumMatrixPalettes(const U32 numMatrixPalettes)
{
	m_MatrixPalettes.Resize(numMatrixPalettes);

	for (U32 i = 0, count = static_cast<U32>(m_MatrixPalettes.Size()); i < count; ++i)
		m_MatrixPalettes[i] = NULL;

	if (!m_pCachedStaticNodeInternalMaterialData)
		m_pCachedStaticNodeInternalMaterialData = new C_CachedStaticNodeInternalMaterialData();
}

void C_SceneGraph::ClearMatrixPalettes(const U32 offset)
{
	for (U32 i = offset, count = static_cast<U32>(m_MatrixPalettes.Size()); i < count; ++i)
		m_MatrixPalettes[i] = NULL;
}

void C_SceneGraph::SetMatrixPalette(const S32 index, const Maths::Mat43* pMatrixPalette)
{
	if (index >= 0 && index < m_MatrixPalettes.Size())
		m_MatrixPalettes[index] = pMatrixPalette;
}

const U32 C_SceneGraph::GetNumMatrixPalettes() const
{
	return static_cast<U32>(m_MatrixPalettes.Capacity());
}

const Maths::Mat43* C_SceneGraph::GetMatrixPalette(const S32 index) const
{
	BIZ_ASSERT(index < m_MatrixPalettes.Size());
	return m_MatrixPalettes[index];
}

void C_SceneGraph::SetPaletteMap(const U8* pPaletteMap) const
{
	if (m_pCachedStaticNodeInternalMaterialData && pPaletteMap)
		m_pCachedStaticNodeInternalMaterialData->m_pPaletteMap = pPaletteMap;
}

void C_SceneGraph::SetStaticNodeInternalMaterialData(C_CachedStaticNodeInternalMaterialData* pInternalData) const
{
	if (m_pCachedStaticNodeInternalMaterialData)
	{
		pInternalData->m_pPaletteMap = m_pCachedStaticNodeInternalMaterialData->m_pPaletteMap;

		delete m_pCachedStaticNodeInternalMaterialData;
		m_pCachedStaticNodeInternalMaterialData = NULL;
	}

	m_pCachedStaticNodeInternalMaterialData = pInternalData;
}

void C_SceneGraph::GetParameters(const char* pName, Btl::Vector<C_CachedStaticNodeMaterialParameter> & materialParameters, const RenderingData::Target::MaterialList* pMaterialList, const U32 numPalettes) const
{
	materialParameters.Clear();

	if (!pMaterialList)
		return;

	if (!m_pCachedStaticNodeInternalMaterialData)
		return;

	// now look in animated parameters
	for (U32 iPalette = 0, paletteCount = numPalettes + 1; iPalette < paletteCount; ++iPalette)
	{
		for (S32 i = 0; i < m_pCachedStaticNodeInternalMaterialData->m_AnimatedParamIndices.Size(); ++i)
		{
			const RenderingData::MaterialType& mtlType = pMaterialList->GetTypeOfMaterial(m_pCachedStaticNodeInternalMaterialData->m_AnimatedParamIndices[i] + (iPalette * m_pCachedStaticNodeInternalMaterialData->m_AnimatedParamIndices.Size()));

			if (!strcmp(mtlType.GetName().Chars(), pName))
			{
				C_CachedStaticNodeMaterialParameter handle;

				handle.index = m_pCachedStaticNodeInternalMaterialData->m_AnimatedParamIndices[i] + (iPalette * m_pCachedStaticNodeInternalMaterialData->m_AnimatedParamIndices.Size());
				handle.offset = mtlType.GetOffset(0, 0);
				handle.size = mtlType.GetElementDataSize(0);

				materialParameters.PushBack(handle);
			}
		}
	}
}

void C_SceneGraph::BindAnimatedParams(const Animation::C_PoseFormat* pPoseFormat, const U32 numMatrixPalettes) const
{
	if (!pPoseFormat)
		return;

	for (U32 fieldIndex = 0, count = pPoseFormat->GetFieldCount(Animation::E_PoseFieldType_JointTransform); fieldIndex < count; ++fieldIndex)
	{
		const char* fieldName = pPoseFormat->GetFieldName(Animation::E_PoseFieldType_JointTransform, fieldIndex).GetString();

		Btl::Vector<C_CachedStaticNodeMaterialParameter> materialParameters;

		GetParameters(fieldName, materialParameters, m_pCachedStaticNodeInternalMaterialData->m_pMaterialList, static_cast<U32>(numMatrixPalettes));

		for (S32 i = 0, paramCount = materialParameters.Size(); i < paramCount; ++i)
		{
			if (materialParameters[i].index >= 0)
			{
				if (materialParameters[i].size == 48)
				{
					if (i == 0)
					{
						if (m_pCachedStaticNodeInternalMaterialData->m_PoseFieldIndexes.Size() < m_pCachedStaticNodeInternalMaterialData->m_PoseFieldIndexes.Capacity())
							m_pCachedStaticNodeInternalMaterialData->m_PoseFieldIndexes.PushBack(static_cast<U16>(fieldIndex));
						/*else
							BIZ_ASSERT_FAIL("Mesh has more than bones than the upper limit of 24");*/
					}
					else
					{
						BIZ_ASSERT((i - 1) < m_pCachedStaticNodeInternalMaterialData->m_MaterialDataList.Size());

						U8* pData = m_pCachedStaticNodeInternalMaterialData->m_pMaterialList->GetMaterial(materialParameters[i].index).GetData(materialParameters[i].offset);
						m_pCachedStaticNodeInternalMaterialData->m_MaterialDataList[i - 1].m_Params.PushBack(pData);
					}
				}
			}
		}
	}
}

void C_SceneGraph::SetJointTransformParameters(const Btl::Vector<U16, Btl::VectorParameters_Static<24> > &poseFieldIndices, const U32 poseFieldIndexCount,
											   U8** __restrict ppParams, const Maths::Mat43* __restrict pMatrixPalette) const
{
	for (U32 i = 0; i < poseFieldIndexCount; ++i)
	{
		DCBT((i<<7), &poseFieldIndices[0]);
		DCBT((i<<7), &ppParams[0]);
		S16 jointIndex = poseFieldIndices[i];
		Maths::Mat43 jointMatrix = pMatrixPalette[jointIndex];
		DCBT(0, &pMatrixPalette[i+4]);
		DCBT(0, &pMatrixPalette[i+5]);
		DCBT(0, &pMatrixPalette[i+6]);
		DCBT(0, &pMatrixPalette[i+7]);
		DCBT(0, ppParams[i+2]);
		DCBT(0, ppParams[i+3]);
		DCBT(0, ppParams[i+4]);
		DCBT(0, ppParams[i+5]);

		MathsStorage::MatrixRow43* __restrict destMatrix = reinterpret_cast<MathsStorage::MatrixRow43*>(ppParams[i]);
		StoreMat43(destMatrix, jointMatrix);
	}
}

/// Model instance node movement callback.
void C_SceneGraph::OnMove(C_ModelInstance* pModelInstance) const
{
	m_pModelInstancePortalList->OnMove(pModelInstance);
	return;
}

C_PointLightHandle AddPointLight(C_SceneGraph *pSceneGraph, bool isManaged = true)
{
	return pSceneGraph->GetLightManager()->AddPointLight(isManaged);
}

void RemovePointLight(C_SceneGraph *pSceneGraph, C_PointLightHandle handle)
{
	pSceneGraph->GetLightManager()->RemovePointLight(handle);
}

C_SpotLightHandle AddSpotLight(C_SceneGraph *pSceneGraph, bool isManaged = true)
{
	return pSceneGraph->GetLightManager()->AddSpotLight(isManaged);
}

void RemoveSpotLight(C_SceneGraph *pSceneGraph, C_SpotLightHandle handle)
{
	pSceneGraph->GetLightManager()->RemoveSpotLight(handle);
}

C_DropShadowHandle AddDropShadow(C_SceneGraph *pSceneGraph, bool isManaged = true)
{
	return pSceneGraph->GetLightManager()->AddDropShadow(isManaged);
}

void RemoveDropShadow(C_SceneGraph *pSceneGraph, C_DropShadowHandle handle)
{
	pSceneGraph->GetLightManager()->RemoveDropShadow(handle);
}

Maths::C_RGBA_F32 EvaluateLighting(const C_SceneGraph* pSceneGraph, const Maths::HVec3& vPoint)
{
	return pSceneGraph->GetLightManager()->EvaluateLighting(vPoint);
}

} // namespace SceneGraph

