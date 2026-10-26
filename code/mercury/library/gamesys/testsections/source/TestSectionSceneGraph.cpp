#include "stdafx.h"
#include "TestSections/TestSectionSceneGraph.h"
#include <Render/Render/GridRender.h>
#include <Framework/Application/UserControls.h>
#include <Framework/Application/ApplicationBase.h>
#include <camera2.h>

// scene graph
#include "system/serialise/c_filesystem.h"
#include "system/serialise/c_fileblock.h"
#include "renderingdata/renderingdata.h"
#include <scenegraph/scenegraph.h>
#include <scenegraph/staticscene.h>
#include <scenegraph/visible/backfrontsorter.h>

// render
#include "Render/State/RenderState.h"

// debug
#include "Render/Debug/DebugRender.h"
#include "Render/Debug/ImmediateDebugRender.h"

// particles etc
#include "Dynamic/Renderer.h"

#include <fx/fx.h>


Debug::Bool g_SceneGraphFreezeFrustum("Graphics.SceneGraph.TestSection.FreezeFrustum", false);
Debug::Bool g_SceneGraphRenderCustom("Graphics.SceneGraph.TestSection.RenderCustom", true);
Debug::Bool g_SceneGraphRenderSolid("Graphics.SceneGraph.TestSection.RenderSolid", true);
Debug::Bool g_SceneGraphRenderPunch("Graphics.SceneGraph.TestSection.RenderPunch", true);
Debug::Bool g_SceneGraphRenderAlpha("Graphics.SceneGraph.TestSection.RenderAlpha", true);

Debug::Bool g_SceneGraphHierarchyRebuild("Graphics.SceneGraph.TestSection.Hierarchy.Rebuild", false);
Debug::Bool g_SceneGraphHierarchyBuild("Graphics.SceneGraph.TestSection.Hierarchy.Build", false);
Debug::Bool g_SceneGraphHierarchyDestroy("Graphics.SceneGraph.TestSection.Hierarchy.Destroy", false);

Debug::Bool g_SceneGraphRenderSceneWithoutGraph("Graphics.SceneGraph.TestSection.RenderSceneWithoutGraph", false);

enum E_RenderPass
{
	E_RenderPass_Custom = SceneGraph::E_RenderPass_Count,
	E_RenderPass_FocusCar,
	E_RenderPass_GhostDepth,
	E_RenderPass_GhostColour
};

// SGSkyDome

C_SGSkyDome::C_SGSkyDome() : I_GatherNode(1000)
{
	EnableRenderPass(SceneGraph::PassFlag<E_RenderPass_Custom>());
}

void C_SGSkyDome::OnRender(SceneGraph::C_RenderState& renderState) const
{
	renderState.SetRenderListNode(NULL);
	renderState.SetScene(NULL);

	C_SkyDome::Render();
}

void C_SGSkyDome::OnGather(const SceneGraph::C_View &view) const
{
	if(view.RenderPassEnabled(SceneGraph::PassFlag<E_RenderPass_Custom>()))
		view.AddVisibleNodeType(this, E_RenderPass_Custom);
}

void C_SGDynamicObject::SetPositionSize(const Maths::HVec3& position, const Maths::HVec3& size)
{
	m_Position = position;
	m_Size = size;

	Maths::HVec3 halfSize = m_Size * Maths::Constants<Maths::HF32>::Half();
	Maths::HF32 length = Maths::Length(halfSize);

	SetSphere(Maths::C_Sphere(position, length));

	ViewPassEnabled(0xFFFFFFFF);
}

void C_SGDynamicObject::OnRender(SceneGraph::C_RenderState& renderState) const
{
	renderState.SetRenderListNode(NULL);
	renderState.SetScene(NULL);

	Maths::HVec3 vSize(GetSphere().Radius(), GetSphere().Radius() * 0.5f, /*m_CullInfo.*/GetSphere().Radius());

	if(renderState.GetRenderPassType() == SceneGraph::E_RenderPass_Solid)
	{
		Maths::HVec3 vCentre = GetSphere().Centre();
		vCentre -= Maths::HVec3(0.0f, GetSphere().Radius() * 0.5f, 0.0f);

		DbUtil::DefaultDebugRenderSys()->Immediate()->SetLightDirection(Maths::HVec3(0.707f, 0.707f, 0.0f));
		DbUtil::DefaultDebugRenderSys()->Immediate()->SetBlendMode(DbUtil::E_BlendMode_Solid);
		DbUtil::DefaultDebugRenderSys()->Immediate()->Draw3dBoxLit(vCentre - vSize, vCentre + vSize, Maths::HMat43(), Maths::RGBA(255, 0, 0, 128));
	}
	else if(renderState.GetRenderPassType() == SceneGraph::E_RenderPass_Alpha)
	{
		Maths::HVec3 vCentre = GetSphere().Centre();
		vCentre += Maths::HVec3(0.0f, GetSphere().Radius() * 0.5f, 0.0f);

		DbUtil::DefaultDebugRenderSys()->Immediate()->SetLightDirection(Maths::HVec3(0.707f, 0.707f, 0.0f));
		DbUtil::DefaultDebugRenderSys()->Immediate()->SetBlendMode(DbUtil::E_BlendMode_Alpha);
		DbUtil::DefaultDebugRenderSys()->Immediate()->Draw3dBoxLit(vCentre - vSize, vCentre + vSize, Maths::HMat43(), Maths::RGBA(255, 255, 0, 128));

	}
	
}

void C_SGDynamicObject::OnGather(const SceneGraph::C_View& view) const
{
	if(view.RenderPassEnabled(SceneGraph::PassFlag<SceneGraph::E_RenderPass_Solid>() | SceneGraph::PassFlag<SceneGraph::E_RenderPass_Alpha>()))
	{
		view.AddVisibleNodeType(this, SceneGraph::E_RenderPass_Solid);
		view.AddVisibleNodeType(this, SceneGraph::E_RenderPass_Alpha);
	}

}

namespace Framework
{

C_TestSectionSceneGraph::C_TestSectionSceneGraph(const Util::C_String& name) : C_TestSectionHDR(name)
{
	m_pSkyDome = new C_SGSkyDome();

	SceneGraph::C_GroupNode* pGroupNode = new SceneGraph::C_GroupNode();
	pGroupNode->AddChild(m_pSkyDome);
	
	m_pSceneGraph = new SceneGraph::C_SceneGraph();

	m_pScene = new RenderingData::Target::Scene();

	FX::Initialise();

	LoadScene();

	for(S32 i = 0; i < 1; ++i)
	{
		for(S32 j = 0; j < 1; ++j)
		{
			for(S32 k = 0; k < 1; ++k)
			{
				C_SGDynamicObject* pObject = new C_SGDynamicObject;
				pObject->SetPositionSize(Maths::HVec3(i * 3.0f, k * 3.0f + 5.0f, j * 3.0f), Maths::HVec3(1.0f, 1.0f, 1.0f));
				
				pObject->/*GetPassInfo().*/EnableRenderPass(SceneGraph::PassFlag<SceneGraph::E_RenderPass_Solid>() | SceneGraph::PassFlag<SceneGraph::E_RenderPass_Alpha>());

				m_DynamicObects.PushBack(pObject);
			}
		}
	}

	m_pDeRenderer = new DynamicEffect::C_Renderer;

	// insert the scene
	m_pStaticScene = m_pSceneGraph->Insert(m_pScene);
	m_pSceneGraph->SetRootNode(pGroupNode);

	m_pSorter = new SceneGraph::C_BackFrontSorter;

	//for(S32 i = 0; i < m_DynamicObects.Size(); ++i)
	{
		//m_pSceneGraph->Insert(m_DynamicObects[i]);
	}

	m_pSceneGraph->SetNumRenderPasses(4);

	// todo - set sorting strategies etc per remder pass
	m_pSceneGraph->SetRenderPassType(0, E_RenderPass_Custom);
	m_pSceneGraph->SetRenderPassType(1, SceneGraph::E_RenderPass_Solid);
	m_pSceneGraph->SetRenderPassType(2, SceneGraph::E_RenderPass_Punch);
	m_pSceneGraph->SetRenderPassType(3, SceneGraph::E_RenderPass_Alpha, m_pSorter);
	m_pSceneGraph->SetRenderPassType(4, E_RenderPass_GhostDepth);
	m_pSceneGraph->SetRenderPassType(5, E_RenderPass_GhostColour);

	m_pView = m_pSceneGraph->AddView(0);
	m_pView->EnableRenderPass(	SceneGraph::PassFlag<E_RenderPass_Custom>() |
								SceneGraph::PassFlag<SceneGraph::E_RenderPass_Solid>() |
								SceneGraph::PassFlag<SceneGraph::E_RenderPass_Punch>() |
								SceneGraph::PassFlag<SceneGraph::E_RenderPass_Alpha>());
	
	// Set the debug feed to fly camera:
	C_View& view = C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
	view.DebugFeed().SetDebugType(Cam2::E_FlyCamera);
	
	// Position the fly camera:
	Cam2::C_Camera& camera = view.DebugFeed().Live();
	camera.SetPosition(Maths::HVec3(-30.f, 20.f, -30.f));
	camera.LookAt(Maths::HVec3());



	// debug renderer

	if(!Render::Debug::g_pDebugRenderSys)
	{
		m_pDebugRenderSys = new Render::Debug::C_DebugRenderSys;
		DbUtil::DefaultDebugRenderSys(m_pDebugRenderSys);
	}
	else
		m_pDebugRenderSys = NULL;
	
	m_pSceneGraph->CreateJobQueuePolicy();

	// start the scheduler
	Scheduler::Init();
	Scheduler::Start();
	Scheduler::SetGroupPoolSize(24);

	m_pSceneGraph->CreateJobQueueGroups();
}

C_TestSectionSceneGraph::~C_TestSectionSceneGraph()
{
	SAFE_DELETE_PTR(m_pSkyDome);
	SAFE_DELETE_PTR(m_pSorter);
	SAFE_DELETE_PTR(m_pSceneGraph);
	SAFE_DELETE_PTR(m_pScene);
	SAFE_DELETE_PTR(m_pDeRenderer);

	Scheduler::Shutdown();
}

void C_TestSectionSceneGraph::OnUpdate()
{
	C_View& view = C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
	view.DebugFeed().SetInput( GetFlyCameraControls(0) );

	if(g_SceneGraphHierarchyRebuild)
		m_pStaticScene->Build();

	if(g_SceneGraphHierarchyDestroy)
	{
		m_pStaticScene->Destroy();
		g_SceneGraphHierarchyDestroy = false;
	}

	if(g_SceneGraphHierarchyBuild)
	{
		m_pStaticScene->Build();
		g_SceneGraphHierarchyBuild = false;
	}
}

void TransformMaterials(const RenderingData::MaterialList& materialInput, RenderingData::Target::MaterialList& materialOutput)
{
	materialOutput.ClearAll();

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
			MathsStorage::Matrix44 m = {	1.f, 0.f, 0.f, position.X(),
											0.f, 1.f, 0.f, position.Y(),
											0.f, 0.f, 1.f, position.Z(),
											0.f, 0.f, 0.f, 1.f };

			output.SetData(&m, sizeof(m));
		}
	}
}

void TransformMaterials(RenderingData::Node& node)
{
	RenderingData::Target::RenderListNode* pRenderListNode = RenderingData::Target::RenderListNode::GetPointer(&node);

	if (!pRenderListNode)
		return;

	TransformMaterials(pRenderListNode->GetDynamicMaterialList(), pRenderListNode->GetDynamicMaterialOutputList());

	pRenderListNode->InitialiseDynamicMaterialList();
}

void C_TestSectionSceneGraph::OnPreRender()
{
	m_pDeRenderer->BeginFrame();
}

void C_TestSectionSceneGraph::OnRenderHdr3DViewWithAA(const S32 viewIndex)
{
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 75, 150, 225));

	m_pScene->ProcessNodes(&m_pScene->GetRootNode(), &TransformMaterials);

	// set hdr parameters
	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		FX::Parameter*	pVectorParameter	= FX::GetSharedParameter("g_vLuminance");
		FX::Parameter*	pTextureParameter	= FX::GetSharedParameter("g_tLuminance");
		
		const Display::C_Float4	pVector		= Render::C_RenderState::Instance().HDR()->GetShaderParameters();
		const void*				pTexture	= Render::C_RenderState::Instance().HDR()->GetAdaptedLuminanceTexture()->GetGPUTexture();
		
		FX::SetParameter(pVectorParameter, &pVector);
		FX::SetParameter(pTextureParameter, &pTexture);
		FX::SetSamplerState(pTextureParameter,
							Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp,
							Display::E_TextureFilter_Point, Display::E_TextureFilter_Point, Display::E_TextureFilter_Point,
							1, 0, 0.0f);
	}

	C_View& view = C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
	
	if(!g_SceneGraphFreezeFrustum)
	{
		m_pView->SetViewFrustum(view.m_View.GetViewFrustum());
		m_pView->SetViewData(view.m_View.GetViewSettings3D());
	}

	{
		PERF_EVENT("SceneGraph::Render");
		m_pSceneGraph->GatherNodes(*m_pView);

		if(g_SceneGraphRenderSolid)
			m_pSceneGraph->Render(*m_pView, SceneGraph::E_RenderPass_Solid);
		
		if(g_SceneGraphRenderPunch)
			m_pSceneGraph->Render(*m_pView, SceneGraph::E_RenderPass_Punch);

		// render pass 0
		if(g_SceneGraphRenderCustom)
			m_pSceneGraph->Render(*m_pView, E_RenderPass_Custom);

		if(g_SceneGraphRenderAlpha)
			m_pSceneGraph->Render(*m_pView, SceneGraph::E_RenderPass_Alpha);
	}

	if(g_SceneGraphRenderSceneWithoutGraph)
	{
		PERF_EVENT("Scene::Render");
		m_pScene->Render();
	}

	// reset states since world renderer will have trashed them
	Display::Device()->ResetStates();
	Display::Device()->DefaultStates();
	Display::Device()->SetVertexShader(NULL);
	Display::Device()->SetPixelShader(NULL);

	m_pDeRenderer->BeginView();
	m_pDeRenderer->Build(Display::Device()->GetView()->m_Index);
	m_pDeRenderer->Display(DynamicEffect::E_RenderPass_Opaque);
	m_pDeRenderer->Display(DynamicEffect::E_RenderPass_TwoDimensional | DynamicEffect::E_RenderPass_Transparent | DynamicEffect::E_RenderPass_Debug);


	DbUtil::DefaultDebugRenderSys()->RenderDeferred();
}

void C_TestSectionSceneGraph::LoadScene()
{
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
 
			if( block.HeaderId() == "Scene" )
			{
				Arch::BinaryInputArchive ar(block);
				ar >> *m_pScene;

				for (int i = 0; i < m_pScene->GetNumResourceBlocks(); i++)
				{
					ar >> m_pScene->GetResourceBlock(i);
				}
			}
		}
	}

	delete pFile;

	m_pScene->UpdateResourcePointers();
}

}; // namespace Framework
