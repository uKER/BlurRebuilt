#include "stdafx.h"

#include "SceneGraph/View.h"
#include "SceneGraph/SceneGraphImpl.h"
#include "SceneGraph/Context.h"
#include "SceneGraph/GatherNode.h"

#include "SceneGraph/RenderingData/StaticRenderListNode.h"
#include "SceneGraph/RenderingData/ModelInstanceNode.h"
#include "SceneGraph/Visible/ListSorter.h"
#include "SceneGraph/RendererInterface.h"

#include "Display.h"
#include "Display/Context/Context.h"

#include "System/Debug/I_DebugRender.h"

namespace Render
{
	const Display::C_Texture2D* GetGlobalColourBuffer(const bool useAAIfAvailable = false);
	const Display::C_Texture2D* GetGlobalDepthStencilBuffer(const bool useAAIfAvailable = false);
#if PLATFORM_PC
	const Display::C_Texture2D* GetGlobalStencilBuffer();	// [Blur2PC]
#endif
}

namespace SceneGraph
{

DECLARE_CONTEXT( Context, C_Context );

extern Debug::Logger &g_Log;
extern Debug::Int	g_RenderListShowIndex;
extern Debug::Bool	g_BoundsShowRenderNodes;
extern Debug::Float g_DrawDistanceScale;

// C_View

C_View::C_View(C_SceneGraph* pSceneGraph) :
	I_View(pSceneGraph),
	m_ConstantSubtechnique(0),
	m_DistanceSubtechnique(0)
{
	m_JobQueueGroup = Scheduler::CreateGroup();

	m_ViewData.instanceDrawDistanceScale = 1.0f;
	m_ViewData.instanceDrawDistanceScaleRcp = 1.0f;
	m_ViewData.modelInstanceDrawDistanceScale = 1.0f;
	m_ViewData.modelInstanceDrawDistanceScaleRcp = 1.0f;
	m_ViewData.typeFlag = 0;
	m_ViewData.cullFlags = E_CullMethod_All;	// By default cull with all available means.

	m_ViewData.distanceSubtechniqueEnable = false;
	m_ViewData.distanceSubtechniqueDistance = 0.0f;

	m_Initialised = false;
	m_Rendering = false;
	m_Scheduling = false;

	m_useFutureCamera = false;
}

C_View::~C_View()
{
	Scheduler::DestroyGroup(m_JobQueueGroup);
}

void C_View::EnableRenderPass(const U32 passFlag)
{
	I_View::EnableRenderPass(passFlag);
	SetupLists();
}

void C_View::DisableRenderPass(const U32 passFlag)
{
	I_View::DisableRenderPass(passFlag);
	SetupLists();
}

const bool C_View::RenderListEnabled() const
{
	m_RenderListIndex++;
	if(g_RenderListShowIndex == -1 || g_RenderListShowIndex == m_RenderListIndex)
		return true;
	else
		return false;
}

void C_View::SetSubtechniqueHandle(const U32 subtechnique)
{
	m_ConstantSubtechnique = subtechnique;
}

void C_View::SetDistanceSubtechniqueHandle(const bool enable, const F32 distance, const U32 handle)
{
	m_VolatileInfoFlags = enable ? E_VolatileNodeInfo_Distance : 0;
	m_ViewData.distanceSubtechniqueEnable = enable;
	m_ViewData.distanceSubtechniqueDistance = distance;
	m_DistanceSubtechnique = handle;
}

void C_View::SetFarClip(const F32 farClip) const
{
	m_ViewData.farClip = farClip;
	m_ViewData.depthScale = 8388608.0f / farClip;	// 23 bits to store depth in
}

void C_View::SetDrawDistanceScale(F32 drawDistanceScale) const
{
	SetInstanceDrawDistanceScale(drawDistanceScale);
	SetModelInstanceDrawDistanceScale(drawDistanceScale);
}

void C_View::SetInstanceDrawDistanceScale(F32 instanceDrawDistanceScale) const
{
	instanceDrawDistanceScale *= g_DrawDistanceScale;
	m_ViewData.instanceDrawDistanceScale = instanceDrawDistanceScale;
	m_ViewData.instanceDrawDistanceScaleRcp = 1.0f / instanceDrawDistanceScale;
}

void C_View::SetModelInstanceDrawDistanceScale(F32 modelInstanceDrawDistanceScale) const
{
	modelInstanceDrawDistanceScale *= g_DrawDistanceScale;
	m_ViewData.modelInstanceDrawDistanceScale = modelInstanceDrawDistanceScale;
	m_ViewData.modelInstanceDrawDistanceScaleRcp = 1.0f / modelInstanceDrawDistanceScale;
}

void C_View::SetRenderPassSorter(const U32 renderPass, const I_VisibleNodeListSorter* pVisibleNodeListSorter)
{
	S32 i = m_PassMapping[renderPass];
	
	if(i != -1)
		m_VisibleNodeLists[i].m_pSorter = pVisibleNodeListSorter;
	else
		LOGWARN(g_Log, "SceneGraph: C_View: SetRenderPassSorter: Invalid render pass");
}

void C_View::SetViewFrustum(const Display::C_ViewFrustum& viewFrustum, const Display::C_TileSetup* pTileSetup) const
{
	m_ViewData.viewFrustum = viewFrustum;
	m_ViewData.forwardZscaled = viewFrustum.m_Forward;
	m_ViewData.forwardMultiplier = viewFrustum.m_ForwardMultiplier;
	
	// setup tiling planes on the view frustum
	if((m_ViewData.pTileSetup = pTileSetup))
		m_ViewData.viewFrustum.SetupTilingPlanes(m_ViewData.viewSettings3D.GetWorldToDevice(), *pTileSetup);
}

void C_View::SetViewData(const Display::C_ViewSettings3D& viewSettings3D) const
{
	m_ViewData.viewSettings3D = viewSettings3D;

	// set up view settings structure
	const Display::C_Matrix& viewMatrix = viewSettings3D.GetViewMatrix();
	m_ViewData.forwardZ = MathsStorage::Float4A16(viewMatrix._13, viewMatrix._23, viewMatrix._33, viewMatrix._43);
	SetFarClip(viewSettings3D.GetFarClip());
}

const Maths::HMat43& C_View::GetFutureCamera() const
{
	return m_futureCamera;
}

void C_View::SetFutureCamera(const Maths::HMat43& m)
{
	m_futureCamera = m;
	m_useFutureCamera = true;
}

void C_View::ClearNodes() const
{
	m_DeferredGatherNodes.Clear();
	for(S32 i = 0; i < m_VisibleNodeLists.Size(); ++i)
	{
		m_VisibleNodeLists[i].m_Processed = false;
		m_VisibleNodeLists[i].m_Empty = false;
		m_VisibleNodeLists[i].m_Abstract = false;
		m_VisibleNodeLists[i].m_ScreenSize = 0;
		m_VisibleNodeLists[i].m_Nodes.Clear();
		m_VisibleNodeLists[i].m_NodeBatches.Clear();
		m_VisibleNodeLists[i].m_StaticPrimitives.Clear();
		m_VisibleNodeLists[i].m_StaticInstances.Clear();
		m_VisibleNodeLists[i].m_ModelInstances.Clear();
	}
	m_VisiblePointLightList.Clear();
	m_VisibleSpotLightList.Clear();
	m_VisibleDropShadowList.Clear();

	m_LightSpheres.Clear();
	m_ViewData.pLightSpheres = 0;
	m_ViewData.lightSpheresCount = 0;

	m_RenderListIndex = -1;
}

void C_View::RendererViewsBeginGather() const
{
	// call begin gather on all derived views
	for(S32 i = 0; i < m_RendererViews.Size(); ++i)
	{
		if(m_RendererViews[i])
			m_RendererViews[i]->BeginGather(*this);
	}
}

void C_View::Initialise()
{
	if(!m_Initialised)
	{
		m_pSceneGraph->AddRendererViews(this);
		m_Initialised = true;
	}
}

void C_View::BeginFrame()
{
	if(!m_Initialised)
		Initialise();

	for(S32 i = 0; i < m_Groups.Size(); ++i)
		m_Groups[i].Reset();
}

void C_View::EndFrame()
{
	// call end gather on all derived views
	for(S32 i = 0; i < m_RendererViews.Size(); ++i)
	{
		if(m_RendererViews[i])
			m_RendererViews[i]->EndFrame(*this);
	}
}

void C_View::BeginGather() const
{
	if(m_Gathering == false)
	{
		PERF_EVENT("SceneGraph.View.Gather");

		m_Gathering = true;

		ClearNodes();
		RendererViewsBeginGather();

		m_pSceneGraph->Gather(*this);
	}
	else
		LOGWARN(g_Log, "SceneGraph: C_View:BeginGather called incorrectly");
}

void C_View::GatherDeferredNodes() const
{
	PERF_EVENT("SceneGraph.GatherDeferred");

	I_View::EndGather();

	// call end gather on all derived views
	for(S32 i = 0; i < m_RendererViews.Size(); ++i)
	{
		if(m_RendererViews[i])
			m_RendererViews[i]->EndGather(*this);
	}
}

void C_View::ProcessVisibleNodes(C_VisibleNodeList& visibleNodeList) const
{
	// first, sort all the lists somehow (eg back to front), or not at all
	SortVisibleNodes(visibleNodeList);
	
	// now, build batches of all the node types which exist in the list
	BuildNodeBatchLists(visibleNodeList);
	
	// submit batches to any relevant renders. this could replace items in the original visible abstract nodes list with compiled nodes
	BuildNodeBatches(visibleNodeList);
	
	// remove any gaps in the visible abstract nodes list
	RemoveNullNodes(visibleNodeList);
	
	// build node batch lists again
	BuildNodeBatchLists(visibleNodeList);

	visibleNodeList.m_Empty =	visibleNodeList.m_Nodes.Size() == 0 &&
								visibleNodeList.m_StaticPrimitives.Size() == 0 &&
								visibleNodeList.m_StaticInstances.Size() == 0 &&
								visibleNodeList.m_ModelInstances.Size() == 0;
	visibleNodeList.m_Processed = true;
}

void C_View::EndGather() const
{
	if(m_Gathering == true)
	{
		PERF_EVENT("SceneGraph.View.EndGather");
		
		m_Gathering = false;
		GatherDeferredNodes();
	}
	else
		LOGWARN(g_Log, "SceneGraph: C_View:EndGather called incorrectly");
}

void C_View::BeginRender() const
{
	BeginRender(*Display::Device()->GetView());
}

void C_View::BeginRender(const Display::C_View& view) const
{
	if(m_Rendering == false)
	{
		PERF_EVENT("SceneGraph.View.BeginRender");

		m_Rendering = true;
		m_pDisplayView = &view;

		// call begin view on all derived views
		for(S32 i = 0; i < m_RendererViews.Size(); ++i)
		{
			if(m_RendererViews[i])
				m_RendererViews[i]->BeginRender(*this);
		}

		m_pSceneGraph->BeginRender(view);
	}
	else
		LOGWARN(g_Log, "SceneGraph: C_View:BeginRender called incorrectly");
}

extern Debug::Bool g_Wireframe;

void C_View::RenderPass(const S32 renderPass, const RenderStates* pRenderStates, U32 subtechnique, U32 distanceSubtechnique) const
{
	if(m_Rendering == true)
	{
		PERF_EVENT("SceneGraph.View.Render");
		if(C_VisibleNodeList* pVisibleNodeList = GetVisibleNodeList(renderPass))
		{
			if(!pVisibleNodeList->m_Processed)
				ProcessVisibleNodes(*pVisibleNodeList);

			for(S32 i = 0; i < m_RendererViews.Size(); ++i)
			{
				if(m_RendererViews[i])
					m_RendererViews[i]->RenderPass(renderPass);
			}

			C_Context *pContext = m_pSceneGraph->GetContext();
			Display::C_Context* pDisplay = Display::GetContext(pContext);
			pContext->CloneState(NULL);
			pContext->Begin(NULL);
			pContext->SetView(this);

			pDisplay->SetColourDepthStencilTexture(Render::GetGlobalColourBuffer(), Render::GetGlobalDepthStencilBuffer());
			#if PLATFORM_PC
			pDisplay->SetStencilTexture(Render::GetGlobalStencilBuffer());	// [Blur2PC] retail 0x007a66e5
			#endif

			RenderSubtask subtask;
			{
				subtask.pass = renderPass;
				subtask.pRenderStates = pRenderStates;
				subtask.abstractStart = 0;
				subtask.primitiveStart = 0;
				subtask.instanceStart = 0;
				subtask.modelInstanceStart = 0;
				subtask.abstractEnd = pVisibleNodeList->m_NodeBatches.Size();
				subtask.primitiveEnd = pVisibleNodeList->m_StaticPrimitives.Size();
				subtask.instanceEnd = pVisibleNodeList->m_StaticInstances.Size();
				subtask.modelInstanceEnd = pVisibleNodeList->m_ModelInstances.Size();
				subtask.pVisibleNodeList = pVisibleNodeList;
			}

			if(g_Wireframe)
				Display::Device()->SetFillMode(Display::E_FillMode_Wireframe);

			pContext->RenderTask(&subtask, subtechnique, distanceSubtechnique, false);

			if(g_Wireframe)
				Display::Device()->SetFillMode(Display::E_FillMode_Default);

			pContext->End();
		}

		DebugRenderPass(renderPass);
	}
	else
		LOGWARN(g_Log, "SceneGraph: C_View:Render called outside of Begin/End Render");
}

void C_View::EndRender() const
{
	if(m_Rendering == true)
	{
		PERF_EVENT("SceneGraph.View.EndRender");

		m_Rendering = false;

		// call begin view on all derived views
		for(S32 i = 0; i < m_RendererViews.Size(); ++i)
		{
			if(m_RendererViews[i])
				m_RendererViews[i]->EndRender(*this);
		}
	}
	else
		LOGWARN(g_Log, "SceneGraph: C_View:EndRender called incorrectly");
}

Debug::Int	g_RenderPrimitivesMax		("Graphics.SceneGraph.PrimitivesMax", -1, Debug::Range(-1, 100));
Debug::Int	g_RenderInstancesMax		("Graphics.SceneGraph.InstancesMax", -1, Debug::Range(-1, 100));
Debug::Int	g_RenderAbstractNodesMax		("Graphics.SceneGraph.AbstractNodesMax", -1, Debug::Range(-1, 1000));
Debug::Int	g_RenderModelInstancesMax	("Graphics.SceneGraph.ModelInstancesMax", -1, Debug::Range(-1, 1000));
Debug::Int	g_RenderPrimitiveStart		("Graphics.SceneGraph.PrimitiveStart", 0, Debug::Range(0, 1000));
Debug::Int	g_RenderInstanceStart		("Graphics.SceneGraph.InstanceStart", 0, Debug::Range(0, 1000));
Debug::Int	g_RenderAbstractNodeStart		("Graphics.SceneGraph.AbstractNodeStart", 0, Debug::Range(0, 1000));
Debug::Int	g_RenderModelInstanceStart	("Graphics.SceneGraph.ModelInstanceStart", 0, Debug::Range(0, 1000));

void ClampVisibleNodes(C_VisibleNodes& nodes, S32 nodeStart, S32 maxNodes)
{
	S32 nodeEnd = nodeStart + maxNodes;

	if(nodeStart > -1 && nodeEnd >= nodeStart)
	{
		if(nodes.Size() > nodeEnd)
			nodes.Resize(nodeEnd);

		if(nodeStart < nodes.Size())
		{
			S32 size = nodes.Size() - nodeStart;
			S32 dest = nodeStart;
			for(S32 src = 0; src < size; ++src, ++dest)
				nodes[src] = nodes[dest];
			nodes.Resize(size);
		}
		else
			nodes.Resize(0);
	}
}

void C_View::SortVisibleNodes(C_VisibleNodeList& visibleNodeList) const
{
	if(const I_VisibleNodeListSorter* pListSorter = m_pSceneGraph->GetRenderPass(visibleNodeList.m_PassIndex)->GetSorter())
		pListSorter->Sort(this, &visibleNodeList);
	//else if(const I_VisibleNodeListSorter* pListSorter = m_VisibleNodeListSorters[m_VisibleNodeLists[i].m_PassIndex])
	//	pListSorter->Sort(this, &m_VisibleNodeLists[i]);

	ClampVisibleNodes(visibleNodeList.m_Nodes, g_RenderAbstractNodeStart, g_RenderAbstractNodesMax);
	ClampVisibleNodes(visibleNodeList.m_StaticPrimitives, g_RenderPrimitiveStart, g_RenderPrimitivesMax);
	ClampVisibleNodes(visibleNodeList.m_StaticInstances, g_RenderInstanceStart, g_RenderInstancesMax);
	ClampVisibleNodes(visibleNodeList.m_ModelInstances, g_RenderModelInstanceStart, g_RenderModelInstancesMax);
}

void C_View::RemoveNullNodes(C_VisibleNodeList& visibleNodeList) const
{
	C_VisibleNodes& visibleNodes = visibleNodeList.m_Nodes;

	S32 i = 0;
	S32 j = 1;

	while(j < visibleNodes.Size())
	{
		while(i < visibleNodes.Size() && visibleNodes[i].GetNode() != NULL)
		{
			++i;
			++j;
		}

		while(j < visibleNodes.Size() && visibleNodes[j].GetNode() == NULL)
		{
			++j;
		}

		if(j < visibleNodes.Size())
		{
			visibleNodes[i] = visibleNodes[j];
			visibleNodes[j].SetNode(NULL);
		}
	}

	if(i < visibleNodes.Size() - 1)
	{
		visibleNodes.Resize(i + 1);
	}
}

void C_View::BuildNodeBatchLists(C_VisibleNodeList& visibleNodeList) const
{
	// this function goes through the generic node list and builds a list of batches of the same type
	// this allows batches of nodes to be submitted to arbitrary renderers for building and renderering
	C_VisibleNodes& visibleNodes = visibleNodeList.m_Nodes;
	C_VisibleNodeBatches& visibleNodeBatches = visibleNodeList.m_NodeBatches;
	
	// clear batches
	visibleNodeBatches.Clear();

	for(S32 j = 0; j < visibleNodes.Size(); ++j)
	{
		if(const I_RenderNode* pRenderNode = visibleNodes[j].GetNode())
		{
			S32 nodeType = pRenderNode->GetType();
			S32 nodeCount = 0;

			visibleNodeBatches.PushBack();
			C_VisibleNodeBatch& visibleNodeBatch = visibleNodeBatches.Back();

			if(nodeType == E_RenderNodeType_Unknown || nodeType >= E_RenderNodeType_Count)
				visibleNodeList.m_Abstract = true;

			visibleNodeBatch.m_NodeType = nodeType;
			visibleNodeBatch.m_StartIndex = j;

			// continue through visibleNodes while they are the same type
			do
			{
				++j;
				++nodeCount;
			}
			while(	j < visibleNodes.Size() && visibleNodes[j].GetNode() && visibleNodes[j].GetNode()->GetType() == nodeType);
			
			// m_EndIndex is is inclusive so subtract 1
			visibleNodeBatch.m_NodeCount = nodeCount;
			visibleNodeBatch.m_EndIndex = --j;
		}
	}
}

void C_View::BuildNodeBatches(C_VisibleNodeList& visibleNodeList) const
{
	if(visibleNodeList.m_NodeBatches.Size() != 0)
	{
		PERF_EVENT("SceneGraph.View.Build");

		bool endBuild = false;

		// go through all the node lists
		for(S32 j = 0; j < visibleNodeList.m_NodeBatches.Size(); ++j)
		{
			C_VisibleNodeBatch& visibleNodeBatch = visibleNodeList.m_NodeBatches[j];

			// see if there is a renderer associated with this node type
			if(const I_Renderer* pRenderer = m_pSceneGraph->GetRenderer(visibleNodeBatch.m_NodeType))
			{
				endBuild = true;
				// there is so build the batch
				pRenderer->Build(*this, visibleNodeList.m_PassIndex, visibleNodeList.m_Nodes, visibleNodeBatch);
			}
		}

		if(endBuild)
		{
			// call end build on renderers
			for(Renderers::Iterator pIterator = m_pSceneGraph->m_Renderers.Begin(); pIterator != m_pSceneGraph->m_Renderers.End(); pIterator++)
			{
				I_Renderer* pRenderer = pIterator->GetValue();
				pRenderer->EndBuild(*this, visibleNodeList.m_PassIndex);
			}
		}	
	}
}

void C_View::SetupLists()
{
	// setup the node lists depending on which passes are visible in this view
	m_PassMapping.Resize(m_pSceneGraph->GetNumRenderPasses());
	m_VisibleNodeLists.Clear();

	for(S32 i = 0; i < m_PassMapping.Size(); ++i)
		m_PassMapping[i] = -1;

	S32 visibleLists = 0;

	for(S32 i = 0; i < m_pSceneGraph->GetNumRenderPasses(); ++i)
	{
		U32 renderPassFlags = m_pSceneGraph->GetRenderPass(i)->GetFlags();
		if(renderPassFlags & m_RenderPassFlags)
			++visibleLists;
	}

	m_VisibleNodeLists.Reserve(visibleLists);

	for(S32 i = 0; i < m_pSceneGraph->GetNumRenderPasses(); ++i)
	{
		U32 renderPassFlags = m_pSceneGraph->GetRenderPass(i)->GetFlags();
		if(renderPassFlags & m_RenderPassFlags)
		{
			m_PassMapping[m_pSceneGraph->GetRenderPass(i)->GetIndex()] = m_VisibleNodeLists.Size();
			m_VisibleNodeLists.PushBack();

			C_VisibleNodeList& visibleNodeList = m_VisibleNodeLists.Back();
			
			visibleNodeList.m_PassIndex = i;
			visibleNodeList.m_PassFlags = renderPassFlags;
			visibleNodeList.m_pSorter = NULL;
			visibleNodeList.m_Nodes.Reserve(50);
			visibleNodeList.m_NodeBatches.Reserve(10);
			visibleNodeList.m_StaticPrimitives.Reserve(20);
			visibleNodeList.m_StaticInstances.Reserve(20);
			visibleNodeList.m_ModelInstances.Reserve(20);
		}
	}
}

void C_View::AddVisibleNode(const I_RenderNode* pNode, const U32 renderPassType, const U32 predication, const U32 volatileInfo, const U32 screenSize) const
{
	// if render node exists in any passes
	S32 i = m_PassMapping[renderPassType];
	
	if(i != -1)
	{
		C_VisibleNodeList& visibleNodeList = m_VisibleNodeLists[i];
		visibleNodeList.m_ScreenSize += screenSize;
		visibleNodeList.m_Nodes.PushBack();
		C_VisibleNode& node = visibleNodeList.m_Nodes.Back();
		node.m_pNode = pNode;
		node.m_VolatileNodeInfo.data = volatileInfo;
	}
}

void C_View::AddVisiblePointLight(const C_VisiblePointLight& visibleLight) const
{
	m_VisiblePointLightList.PushBack(visibleLight);
}

void C_View::AddVisibleSpotLight(const C_VisibleSpotLight& visibleLight) const
{
	m_VisibleSpotLightList.PushBack(visibleLight);
}

void C_View::AddVisibleDropShadow(const C_VisibleDropShadow& visibleLight) const
{
	m_VisibleDropShadowList.PushBack(visibleLight);
}

void C_View::AddLightSphere(const MathsStorage::Float4& sphere) const
{
	// calculate the position, depth of there sphere in screen space
	Maths::HVec4 sphereW(Maths::Load<Maths::HVec4>(&sphere));
	Maths::HF32 radius(sphereW.W());
	sphereW.W(Maths::Constants<Maths::HF32>::One());

	Maths::HVec3 position(sphereW);
	Maths::HVec4 forward(Maths::Load<Maths::HVec4>(&m_ViewData.forwardZ));
	Maths::HF32 depth = (sphereW | forward) + radius;
	Maths::HVec3 unitSphere = position - m_ViewData.viewSettings3D.GetCameraPosition();
	Maths::HF32 length = Maths::Length(unitSphere);
	unitSphere /= length;
	
	m_LightSpheres.PushBack();
	LightSphere& lightSphere = m_LightSpheres.Back();

	Maths::Store(unitSphere, &lightSphere.unitSphere);
	lightSphere.unitSphere.w = radius / length;
	Maths::Store(sphereW, &lightSphere.sphere);
	lightSphere.depth.x = depth;
	
	m_ViewData.pLightSpheres = &m_LightSpheres[0];
	m_ViewData.lightSpheresCount = m_LightSpheres.Size();
}

void C_View::AddLightSphere(const MathsStorage::Float4A16& sphere) const
{
	AddLightSphere((const MathsStorage::Float4&)sphere);
}

Debug::Bool	g_BoundsLights("Graphics.SceneGraph.Bounds.Nodes.Lights", true);

void C_View::DebugRender() const
{
	if(g_BoundsShowRenderNodes && g_BoundsLights)
	{
		for(S32 i = 0; i < m_VisiblePointLightList.Size(); ++i)
			m_VisiblePointLightList[i].m_pObject->DebugRender(*this);

		for(S32 i = 0; i < m_VisibleSpotLightList.Size(); ++i)
			m_VisibleSpotLightList[i].m_pObject->DebugRender(*this);
	}
}

void C_View::DebugRenderPass(U32 iPass) const
{
	const C_VisibleNodeList* pVNL = GetVisibleNodeList(iPass);
	if (!pVNL)
		return;

	if(g_BoundsShowRenderNodes)
	{
		for(S32 i = 0; i < pVNL->m_Nodes.Size(); ++i)
		{
			if(const I_RenderNode* pNode = pVNL->m_Nodes[i].GetNode())
				pNode->DebugRender(pVNL->m_Nodes[i]);
		}

		for(S32 i = 0; i < pVNL->m_StaticPrimitives.Size(); ++i)
		{
			if(const I_RenderNode* pNode = pVNL->m_StaticPrimitives[i].GetNode())
				pNode->DebugRender(pVNL->m_StaticPrimitives[i]);
		}

		for(S32 i = 0; i < pVNL->m_StaticInstances.Size(); ++i)
		{
			if(const I_RenderNode* pNode = pVNL->m_StaticInstances[i].GetNode())
				pNode->DebugRender(pVNL->m_StaticInstances[i]);
		}

		for(S32 i = 0; i < pVNL->m_ModelInstances.Size(); ++i)
		{
			if(const I_RenderNode* pNode = pVNL->m_ModelInstances[i].GetNode())
				pNode->DebugRender(pVNL->m_ModelInstances[i]);
		}
	}
}


C_RenderGroup* C_View::BeginSchedule(const S32 groupIndex, const Display::C_View& view, const C_RenderGroupSettings* pSettings)
{
	if(m_Scheduling == false && groupIndex < m_Groups.Size())
	{
		m_Scheduling = true;
		
		m_pSceneGraph->SetSharedParameters(view);
		m_pSceneGraph->SetSharedParameters(*this);
		
		m_GroupIndex = groupIndex;
		m_pDisplayView = &view;
		m_pRenderGroupSettings = pSettings ? pSettings : m_pSceneGraph->GetRenderGroupSettings();

		return &m_Groups[groupIndex];
	}
	else
		return 0;
}

void C_View::ReserveGroups(const S32 groups)
{
	S32 prevSize = m_Groups.Size();
	if(m_Groups.Size() < groups)
	{
		m_Groups.Clear();
		m_Groups.Resize(groups);

		for(S32 i = prevSize; i < groups; ++i)
		{
			m_Groups[i].GetContext().m_pSceneGraph = m_pSceneGraph;
			m_Groups[i].GetContext().m_pView = this;
		}
	}
}

void C_View::SchedulePass(S32 pass, const RenderStates* pStates)
{
	if(m_Scheduling)
	{
		if(C_VisibleNodeList* pVisibleNodeList = GetVisibleNodeList(pass))
		{
			if(!pVisibleNodeList->m_Processed)
				ProcessVisibleNodes(*pVisibleNodeList);
			m_Groups[m_GroupIndex].SchedulePass(pass, pStates);
		}
	}
}

/// schedule tasks in the task-list
void C_View::EndSchedule()
{
	if(!m_Scheduling)
		return;

	m_Scheduling = false;
	PERF_EVENT("SceneGraph.View.Schedule");

	m_Groups[m_GroupIndex].Schedule(m_pDisplayView, m_pRenderGroupSettings);
}

/// Set the cull methods used for this view.
void C_View::SetCullMethod(const U32 cullMethodFlags)
{
	m_ViewData.cullFlags = cullMethodFlags;
	return;
}

} // namespace SceneGraph

