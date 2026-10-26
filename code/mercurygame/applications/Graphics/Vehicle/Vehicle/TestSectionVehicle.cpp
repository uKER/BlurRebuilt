#include "stdafx.h"
#include "TestSectionVehicle.h"
#include <Render/Render/GridRender.h>
#include <Vehicle/VehicleModelManager.h>
#include <Vehicle/VehicleModel.h>
#include <Vehicle/Setup.h>
#include <Vehicle/Input.h>

#include <scenegraph/SceneGraph.h>
#include <scenegraph/Visible/BackFrontSorter.h>
#include <SceneGraph/RenderNode.h>
//#include <SchedulerConfig.h>

#include <Framework/Application/ApplicationBase.h>

#include <Render/Sky/SkyDome.h>

#include <Fx/Fx.h>

C_TestSectionVehicle::C_TestSectionVehicle(const Util::C_String& name) : Framework::C_TestSection(name)
{
	m_pSkyDome = new Render::Sky::C_SkyDome();

	m_Initialised = false;

	m_pSceneGraph = NULL;
	m_pView = NULL;
}

C_TestSectionVehicle::~C_TestSectionVehicle()
{
	SAFE_DELETE_PTR(m_pSkyDome);
}

bool C_TestSectionVehicle::OnCreate()
{
	FX::Initialise();

#ifdef SHADER_BUILDER
	Vehicle::ShaderBuild::C_ShaderBuilder::Instance().Create();
#endif

	CreateSceneGraph();

	Vehicle::C_VehicleModelManager::Instance().Create();

	Vehicle::C_VehicleModelSetup setup;
	setup.m_FilePath = "models/testmodels/";
	setup.m_FileName = "vehicle.model";
	setup.m_bCreateDamageLayer = true;
	setup.m_bCreateGameLayer = true;
	setup.m_bCreateRenderLayer = true;
	Vehicle::C_VehicleModelManager::Instance().SetSceneGraphPtr(m_pSceneGraph);

	Vehicle::C_VehicleModelManager::Instance().AddVehicleModel(&setup);	
	
	m_Initialised = false;

	return true;
}

bool C_TestSectionVehicle::CreateSceneGraph()
{
	m_pSceneGraph = new SceneGraph::C_SceneGraph();

//	m_pSceneGraph->SetJobQueueGroups( C_SchedulerConfig::GetInstance()->GetGroup(E_SchedulerGroup_SceneGraph0), C_SchedulerConfig::GetInstance()->GetGroup(E_SchedulerGroup_SceneGraph1) );
//	m_pSceneGraph->SetJobQueuePolicy( C_SchedulerConfig::GetInstance()->GetSceneGraphPolicy() );

	m_pSorter = new SceneGraph::C_BackFrontSorter;

	// set render passes and sorting strategies etc
	m_pSceneGraph->SetNumRenderPasses(3);
	m_pSceneGraph->SetRenderPassType(0, SceneGraph::E_RenderPass_Solid);
	m_pSceneGraph->SetRenderPassType(1, SceneGraph::E_RenderPass_Punch);
	m_pSceneGraph->SetRenderPassType(2, SceneGraph::E_RenderPass_Alpha, m_pSorter);

	m_pView = m_pSceneGraph->AddView(0);
	m_pView->EnableRenderPass(	SceneGraph::PassFlag<SceneGraph::E_RenderPass_Solid>() |
								SceneGraph::PassFlag<SceneGraph::E_RenderPass_Punch>() |
								SceneGraph::PassFlag<SceneGraph::E_RenderPass_Alpha>());

	return true;
}

void C_TestSectionVehicle::OnDestroy()
{
	Vehicle::C_VehicleModelManager::Instance().Destroy();

	m_Initialised = false;

	m_pSceneGraph->RemoveView(m_pView);
	SAFE_DELETE_PTR( m_pSceneGraph );
	SAFE_DELETE_PTR( m_pSorter );
}

Debug::Bool g_debugVehicleFakeInput("Vehicle.Input.FakeInput",	true);

Debug::Float g_debugVehiclePosX("Vehicle.Input.PosX",			0.0f, Debug::Range(-50.0f, 50.0f), Debug::Step(0.001f));
Debug::Float g_debugVehiclePosY("Vehicle.Input.PosX",			0.2f, Debug::Range(-50.0f, 50.0f), Debug::Step(0.001f));
Debug::Float g_debugVehiclePosZ("Vehicle.Input.PosZ",			0.0f, Debug::Range(-50.0f, 50.0f), Debug::Step(0.001f));

Debug::Int g_debugVehicleGear("Vehicle.Input.Gear",				0, Debug::Range(-1, 7), Debug::Step(1));

Debug::Float g_debugVehicleRevs("Vehicle.Input.Revs",			0.0f, Debug::Range(0.0f, 18000.0f), Debug::Step(0.001f));
Debug::Float g_debugVehicleThrottle("Vehicle.Input.Throttle",	0.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugVehicleBrake("Vehicle.Input.Brake",			0.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugVehicleSpeed("Vehicle.Input.Speed",			0.0f, Debug::Range(-50.0f, 300.0f), Debug::Step(0.001f));

Debug::Bool g_debugVehicleHandbrake("Vehicle.Input.Handbrake",	false);
Debug::Bool g_debugVehicleUpdate("Vehicle.Input.Update",		true);
Debug::Bool g_debugVehicleRender("Vehicle.Input.Render",		true);

void C_TestSectionVehicle::OnUpdate()
{
	if (!m_Initialised)
	{
		Vehicle::C_VehicleModelManager::Instance().Initialise();
		m_Initialised = true;
	}

	//Fake input values
	if (g_debugVehicleFakeInput)
	{
		for (U32 iVehicle = 0; iVehicle < Vehicle::C_VehicleModelManager::Instance().GetNumVehicles(); iVehicle++)
		{
			Vehicle::C_VehicleModel* pVehicleModel = Vehicle::C_VehicleModelManager::Instance().GetVehicleModel(iVehicle);

			if (pVehicleModel)
			{
				Vehicle::C_ModelInput* pInput = pVehicleModel->GetModelInput();

				pInput->m_mMatrix.SetIdentity();
				pInput->m_mMatrix.Translation(Maths::HVec3((F32)g_debugVehiclePosX, (F32)g_debugVehiclePosY, (F32)g_debugVehiclePosZ));
				
				pInput->m_iGear				= (S32)g_debugVehicleGear;
				pInput->m_fRevs				= (F32)g_debugVehicleRevs;
				pInput->m_fThrottle			= (F32)g_debugVehicleThrottle;
				pInput->m_fBrake			= (F32)g_debugVehicleBrake;
				pInput->m_fSpeed			= (F32)g_debugVehicleSpeed;

				pInput->m_bHandbrake		= (bool)g_debugVehicleHandbrake;

				pInput->m_bShouldUpdate		= (bool)g_debugVehicleUpdate;
				pInput->m_bShouldRender		= (bool)g_debugVehicleRender;			
			}
		}
	}

	//Update the vehicle models
	Vehicle::C_VehicleModelManager::Instance().Update();
}

void C_TestSectionVehicle::OnPreRender()
{
	Vehicle::C_VehicleModelManager::Instance().PreRender();
}

void C_TestSectionVehicle::OnRender3DViewWithAA(const S32 viewIndex)
{
	//m_pSkyDome->Render();

	Render::DrawGrid(Maths::RGBA(0, 255, 0, 64), 20);

	//Vehicle::C_VehicleModelManager::Instance().Render();	
	Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
	m_pView->SetViewFrustum(view.m_View.GetViewFrustum());
	m_pView->SetViewData(view.m_View.GetViewSettings3D());

	m_pSceneGraph->GatherNodes(*m_pView);
	m_pSceneGraph->Render(*m_pView, SceneGraph::E_RenderPass_Solid);
	m_pSceneGraph->Render(*m_pView, SceneGraph::E_RenderPass_Punch);
	m_pSceneGraph->Render(*m_pView, SceneGraph::E_RenderPass_Alpha);

	Display::Device()->ResetStates();
	Display::Device()->SetViewShaderConstants3D();

}

void C_TestSectionVehicle::OnRender2DView()
{
	//Display::FontRender()->Draw("Hello!", 1024.f/2.f, 768.f/2.f, 50.f, 50.f, Maths::RGBA(255, 255, 255, 255), Display::E_FontRenderAlign_Ct);
}
