#include "stdafx.h"
#include "modelrenderer.h"

#include "system/serialise/c_filesystem.h"
#include "system/serialise/c_fileblock.h"
#include "renderingdata/archive.h"

#include "scenegraph/scenegraph.h"
#include "scenegraph/visible/backfrontsorter.h"
#include "scenegraph/visible/frontbacksorter.h"
#include "scenegraph/rendernode.h"
#include "scenegraph/renderingdata/staticscene.h"
#include "world_render/worldrender.h"
#include "camera2/stock/DebugFeed.h"
#include "Display/Render/HeapRender.h"

Debug::Bool g_UseSceneGraph("ModelRenderApp.UseSceneGraph", true);
Debug::Bool g_RebuildSceneGraph("ModelRenderApp.RebuildSceneGraph", false);
Debug::Bool g_FreezeSceneGraph("ModelRenderApp.FreezeSceneGraph", false);

Debug::Bool g_GatherOpaque("ModelRenderApp.GatherOpaque", true);
Debug::Bool g_GatherOverlay("ModelRenderApp.GatherOverlay", true);
Debug::Bool g_GatherAlphaTest("ModelRenderApp.GatherAlphaTest", true);
Debug::Bool g_GatherBackFront("ModelRenderApp.GatherBackFront", true);
Debug::Bool g_GatherPrimitives("ModelRenderApp.GatherPrimitives", true);
Debug::Bool g_GatherInstances("ModelRenderApp.GatherInstances", true);

enum E_RenderPass
{
	E_RenderPass_Opaque,
	E_RenderPass_Overlay,
	E_RenderPass_AlphaTest,
	E_RenderPass_BackFront
};


bool ModelRendererApp::Create()
{
#ifdef PLATFORM_XENON
	Sys::C_FileSystem::Instance().SetWorkingDirectory("GAME:");
#endif

	ImportModel();

	m_pSceneGraph = new SceneGraph::C_SceneGraph;

	m_pSceneGraph->CreateJobQueuePolicy();

	// start the scheduler
	Scheduler::Init();
	Scheduler::Start();
	Scheduler::SetGroupPoolSize(24);

	m_pSceneGraph->CreateJobQueueGroups();

	m_pStaticScene = m_pSceneGraph->Insert(&m_scene);
	m_pSceneGraph->SetNumRenderPasses(3);

	m_pSorter = new SceneGraph::C_BackFrontSorter;
	m_pSorter2 = new SceneGraph::C_FrontBackSorter;

	m_pSceneGraph->SetRenderPassType(0, E_RenderPass_Opaque);
	m_pSceneGraph->SetRenderPassType(1, E_RenderPass_AlphaTest);
	m_pSceneGraph->SetRenderPassType(2, E_RenderPass_BackFront, m_pSorter);

	// tell scenegraph which static scene rendering passes correspond to which rendering pass type
	m_pSceneGraph->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OPAQUE, E_RenderPass_Opaque);
	m_pSceneGraph->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OVERLAY, E_RenderPass_Overlay);
	m_pSceneGraph->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_ALPHA_TEST, E_RenderPass_AlphaTest);
	m_pSceneGraph->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_BACK_FRONT, E_RenderPass_BackFront);

	m_pView = m_pSceneGraph->AddView(0);
	m_pView->EnableRenderPass(	SceneGraph::PassFlag<E_RenderPass_Opaque>() |
								SceneGraph::PassFlag<E_RenderPass_AlphaTest>() |
								SceneGraph::PassFlag<E_RenderPass_BackFront>());

	return true;
}

void ModelRendererApp::Destroy()
{
	Scheduler::Shutdown();

	delete m_pSorter;
	delete m_pSorter2;
	delete m_pSceneGraph;
}

void ModelRendererApp::Update(const Framework::C_View& view)
{
	PERF_EVENT("Update Streamer");

	RenderingData::Target::StreamData& streamData = m_scene.GetStreamData();

	/*while (1)
	{
		streamData.SetCacheChunk(44, true);
		streamData.SetLoadChunk(44, true);
		streamData.LoadAllRequested();
		streamData.SetLoadChunk(44, false);
		streamData.LoadAllRequested();
	}*/

	streamData.ClearRequests();
	streamData.Request(Maths::HMat44(view.DebugFeed().Mat43()), view.DebugFeed().Fov(), view.m_ViewSetup.m_ViewSetup3D.m_AspectRatio, view.m_ViewSetup.m_ViewSetup3D.m_FarClip, -1);
	streamData.UpdateMain();
	streamData.UpdateAsynch();

	{
		if (streamData.NodesChanged() || g_RebuildSceneGraph)
			m_pStaticScene->Build();

		if (streamData.BlocksChanged() || streamData.CacheChanged())
			streamData.DebugInfo();

		m_scene.UpdateResourcePointers();
	}
}

void TransformMaterials(const RenderingData::MaterialList& materialInput, RenderingData::Target::MaterialList& materialOutput)
{
	materialOutput.ClearAll();
	materialOutput.Begin();

	RenderingData::MaterialType worldMatrixMaterial;
	worldMatrixMaterial.SetName("WorldMatrix");
	RenderingData::MaterialElement element;
	element.count = 16;
	element.name = "g_mWorld";
	element.type = RenderingData::MATERIAL_ELEMENT_TYPE_F32;
	worldMatrixMaterial.AddElement(element);
	S32 worldMatrixMaterialIndex = materialOutput.AddMaterialType(worldMatrixMaterial);

	for (int i = 0; i < materialInput.GetNumMaterials(); i++)
	{
		const RenderingData::Material& input = materialInput.GetMaterial(i);
		const RenderingData::MaterialType& inputType = materialInput.GetMaterialType(input.GetTypeIndex());

		if (inputType.GetName() == "InstancePosition")
		{
			S32 outputIndex = materialOutput.AddMaterial(worldMatrixMaterialIndex);
			RenderingData::Material& output = materialOutput.GetMaterial(outputIndex);

			S32 offset = inputType.GetOffset(0, 0);
			RenderingData::DataTypes::F32_3 position = input.GetData<RenderingData::DataTypes::F32_3>(offset);
#if !PLATFORM_PS3
			MathsStorage::Matrix44 m = {	1.f, 0.f, 0.f, position.X(),
											0.f, 1.f, 0.f, position.Y(),
											0.f, 0.f, 1.f, position.Z(),
											0.f, 0.f, 0.f, 1.f };
#else
			MathsStorage::Matrix44 m = {	1.f, 0.f, 0.f, 0,
											0.f, 1.f, 0.f, 0,
											0.f, 0.f, 1.f, 0,
											position.X(), position.Y(), position.Z(), 1.f };
#endif

			output.SetData(&m, sizeof(m));
		}
	}

	materialOutput.End();
}

void TransformMaterials(RenderingData::Node& node)
{
	RenderingData::Target::RenderListNode* pRenderListNode = RenderingData::Target::RenderListNode::GetPointer(&node);

	if (!pRenderListNode)
		return;

	TransformMaterials(pRenderListNode->GetDynamicMaterialList(), pRenderListNode->GetDynamicMaterialOutputList());

	pRenderListNode->InitialiseDynamicMaterialList();
}

void ModelRendererApp::Render()
{
	m_scene.ProcessNodes(&m_scene.GetRootNode(), &TransformMaterials);

	PERF_EVENT("ModelRendererApp::Render");

	const Display::C_View* pView = Display::Device()->GetView();

	if(!g_FreezeSceneGraph)
	{
		m_pView->SetViewFrustum(pView->GetViewFrustum());
		m_pView->SetViewData(pView->GetViewSettings3D());
	}

	if(g_UseSceneGraph)
	{
		if(g_GatherOpaque)
			m_pView->EnableRenderPass(SceneGraph::PassFlag<E_RenderPass_Opaque>());
		else
			m_pView->DisableRenderPass(SceneGraph::PassFlag<E_RenderPass_Opaque>());

		if(g_GatherOverlay)
			m_pView->EnableRenderPass(SceneGraph::PassFlag<E_RenderPass_Overlay>());
		else
			m_pView->DisableRenderPass(SceneGraph::PassFlag<E_RenderPass_Overlay>());

		if(g_GatherAlphaTest)
			m_pView->EnableRenderPass(SceneGraph::PassFlag<E_RenderPass_AlphaTest>());
		else
			m_pView->DisableRenderPass(SceneGraph::PassFlag<E_RenderPass_AlphaTest>());

		if(g_GatherBackFront)
			m_pView->EnableRenderPass(SceneGraph::PassFlag<E_RenderPass_BackFront>());
		else
			m_pView->DisableRenderPass(SceneGraph::PassFlag<E_RenderPass_BackFront>());

		if(g_GatherPrimitives)
			m_pView->EnableRenderNode(SceneGraph::PassFlag<SceneGraph::E_RenderNodeType_StaticRenderListPrimitiveNode>());
		else
			m_pView->DisableRenderNode(SceneGraph::PassFlag<SceneGraph::E_RenderNodeType_StaticRenderListPrimitiveNode>());

		if(g_GatherInstances)
			m_pView->EnableRenderNode(SceneGraph::PassFlag<SceneGraph::E_RenderNodeType_StaticRenderListInstanceNode>());
		else
			m_pView->DisableRenderNode(SceneGraph::PassFlag<SceneGraph::E_RenderNodeType_StaticRenderListInstanceNode>());
			

		m_pSceneGraph->GatherNodes(*m_pView);

		m_pView->BeginRender();

		m_pView->Render(E_RenderPass_Opaque);
		m_pView->Render(E_RenderPass_Overlay);
		m_pView->Render(E_RenderPass_AlphaTest);
		m_pView->Render(E_RenderPass_BackFront);
	}
	else
		m_scene.Render();

	RenderingData::Target::StreamData& streamData = m_scene.GetStreamData();
	streamData.DisplayDebug();

	m_pvsData.DebugRender(pView->GetViewSettings3D().GetCameraPosition());

/*	Display::C_HeapRender heapRender;
#if defined(PLATFORM_XENON)
	heapRender.Draw(Display::Device()->GetLargeHeap(), "");
#elif defined(PLATFORM_PS3)
	heapRender.Draw(Display::Device()->GetVideoMemory(), "");
#endif*/
}

void ModelRendererApp::DisplayDebugTriangle()
{
	Display::BasicRender()->BindColoured3D();
	Display::BasicRender()->DrawColoured3DLine(Maths::HVec3(0.f, 0.f, 0.f), Maths::HVec3(0.f, 1.f, 0.f), Maths::RGBA(1.f, 0.f, 0.f));
	Display::BasicRender()->DrawColoured3DLine(Maths::HVec3(0.f, 1.f, 0.f), Maths::HVec3(1.f, 0.f, 0.f), Maths::RGBA(1.f, 0.f, 0.f));
	Display::BasicRender()->DrawColoured3DLine(Maths::HVec3(1.f, 0.f, 0.f), Maths::HVec3(0.f, 0.f, 0.f), Maths::RGBA(1.f, 0.f, 0.f));
}

void ModelRendererApp::ImportModel()
{
	RESOURCE_MEMORY_ID("renderingdata");

	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open("model.dat", Sys::E_FileAccess_Read);

	if (!pFile)
	{
		assert(!"Could not open file.");
		return;
	}
	else
	{
		Sys::C_FileBlock rootBlock( pFile );
 
		// keep reading child blocks
		while ( rootBlock.GetPosition() < rootBlock.Length() )
		{
			Sys::C_FileBlock block( &rootBlock );

			bool loadBlocks = false;
 
			if( block.HeaderId() == "Scene" )
			{
				WorldRender::ReadScene(&block, &m_scene, &loadBlocks);

				if (!loadBlocks)
				{
					RenderingData::Target::StreamData& streamData = m_scene.GetStreamData();
					streamData.OpenStreamFile("model.stream");
				}
			}
		}
	}

	delete pFile;

	m_scene.UpdateResourcePointers();

	{
		Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open("pvs.dat", Sys::E_FileAccess_Read);

		if (pFile)
		{
			Arch::BinaryInputArchive ar(*pFile);
			ar >> m_pvsData;
		}

		delete pFile;
	}
}

