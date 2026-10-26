#include "StdAfx.h"
#include <system/Serialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <system/serialise/C_FileBlock.h>

#include "TestSectionParticles.h"
#include "Framework/Application/Application.h"
#include "Framework/Application/UserControls.h"
#include "Input/GameInput.h"

// basic rendering
#include "Display/Render/FontRender.h"
#include "Render/Render/GridRender.h"

// debug
#include "Render/Debug/DebugRender.h"
#include "Render/Debug/ImmediateDebugRender.h"
#include "Render/Debug/Charts/ChartRenderer.h"
#include "Render/Debug/Charts/LineChart.h"

// constants
#include "Render/Shaders/Constants.h"

// particles
#include "Dynamic/Renderer.h"

#include "Dynamic/Particle/Manager.h"
#include "Dynamic/Particle/Force.h"
#include "Dynamic/Particle/Emitter.h"
#include "Dynamic/Particle/Event.h"

#include "Dynamic/Particle/Tools_Shared/UserShape3DData.h"
#include "Dynamic/Particle/Tools_Shared/UserShape3DVariants.h"
#include "Dynamic/Particle/Tools_Shared/UserParticleContainer.h"
#include "Dynamic/Particle/UserParticleLibrary.h"
#include "Dynamic/Particle/Generic/XtInterface.h"
#include "Dynamic/Particle/SpawnMesh.h"

#include "Dynamic/Strip/Manager.h"
#include "Dynamic/Strip/Emitter.h"
#include "Dynamic/Strip/Generic/Beam.h"

#include "Dynamic/Minimal/Minimal.h"

#include "Dynamic/Particle/MathsUtil.h"

// scene nodes
#include "Render/Scene/SceneNode.h"

// scene graph
#include "SceneGraph/SceneGraph.h"
#include "SceneGraph/RenderNode.h"
#include "SceneGraph/Visible/BackFrontSorter.h"

// states
#include "Render/State/RenderState.h"

#include <Render/PostProcess/HDR/HDR.h>

// reduced resolution rendering

#include "Render/ReducedRender/Manager.h"

// post processing

#include "Render/PostProcess/Blur/Gaussian.h"
#include "Render/PostProcess/Blur/Box.h"
#include "Render/PostProcess/Colour/Greyscale.h"
#include "Render/PostProcess/Misc/Static.h"
#include "Render/PostProcess/Misc/ShowDepth.h"
#include "Render/PostProcess/Scale/GaussianScale.h"
#include "Render/PostProcess/HDR/HDR.h"
#include "Render/PostProcess/Colour/Tint.h"
#ifdef PLATFORM_XENON
#include "Render/PostProcess/Blur/Motion.h"
#endif

//TODO: This test section needs refactoring to use new curve class.
// curves

#ifdef PLATFORM_XENON
//#define ENABLE_CLOTH
#endif

// cloth renderer
#ifdef ENABLE_CLOTH
#include "TestDynamic/ClothRenderer.h"
#endif

// global depth texture

#include "Render/Utils/Globals.h"
#include "Render/Utils/Helper.h"

#include <system/serialise.h>
#include <system/memory.h>

Debug::Bool		g_debugSceneGraph("TestSection.Particles.UseSceneGraph", false);
Debug::Bool		g_debugRenderReduced("TestSection.Particles.RenderReduced", false);
Debug::Bool		g_debugSpawnElectricity("TestSection.Particles.SpawnElectricity", false);
Debug::Bool		g_debugRotateLight("TestSection.Particles.RotateLight", false);
Debug::Float	g_debugParticleSplineBegin("TestSection.Particles.ParticleSplineBegin", 0.0f, Debug::Range(0.f, 1.0f), Debug::Step(0.01f));
Debug::Float	g_debugParticleSplineEnd("TestSection.Particles.ParticleSplineEnd", 1.0f, Debug::Range(0.f, 1.0f), Debug::Step(0.01f));
Debug::Float	g_debugParticleFadeBegin("TestSection.Particles.ParticleFadeBegin", 0.0f, Debug::Range(0.f, 2000.0f), Debug::Step(1.0f));
Debug::Float	g_debugParticleFadeEnd("TestSection.Particles.ParticleFadeEnd", 0.1f, Debug::Range(0.f, 2000.0f), Debug::Step(1.0f));
Debug::Bool		g_debugUseSceneNodes("TestSection.Particles.UseSceneNodes", false);
Debug::Float	g_debugParticleEmissive("TestSection.Particles.ParticleEmissive", 3.0f, Debug::Range(0.f, 10.0f), Debug::Step(0.1f));
Debug::Float	g_debugParticleFreezeAmountScale("TestSection.Particles.ParticleFreezeAmountScale", 1.0f, Debug::Range(0.f, 2.0f), Debug::Step(0.1f));
Debug::Int		g_debugParticleEventIndex("TestSection.Particles.ParticleEventIndex", 0);



Debug::Float	g_debugFogDensity("Samples.FogDensity", 0.0f, Debug::Range(0.f, 0.5f), Debug::Step(0.001f));

namespace Render { namespace Sky { extern ::Debug::Float	g_debugSkyDomeIntensity; } }

DynamicEffect::Particle::C_EventInstanceHandle	g_ParticleEventHandle;
U32												g_ParticleEventIndex;
U32												g_ParticleEventId;
F32												g_ParticleCycle;

DynamicEffect::C_Node g_Node;
DynamicEffect::Strip::EmitterDefinition g_EmitterDefinition[2];
Curve2::C_Hermite3D g_Curve;

Util::C_Random g_TestParticlesRandom;

enum E_RenderPass
{
	E_RenderPass_Opaque,
	E_RenderPass_Decal,
	E_RenderPass_Transparent,
	E_RenderPass_ParticleOpaque,
	E_RenderPass_ParticleDecal,
	E_RenderPass_ParticleTransparent
};

C_TestSectionParticles::C_TestSectionParticles(const Util::C_String& name) : Framework::C_TestSection(name)
{
	// catmull rom with generated position based on estimated length
	g_Curve.ClearKeyFrames();
	g_Curve.SetType(Curve2::E_HermiteType_CatmullRom, true);

	g_ParticleEventHandle = DynamicEffect::Particle::C_EventInstanceHandle();
	g_ParticleEventIndex = 0xFFFFFFFF;
	g_ParticleEventId = 0xFFFFFFFF;
	g_ParticleCycle = 0.0f;

	Maths::HVec3 centre(2.0f, 1.0f, 2.0f);
	Maths::HF32 radius(2.0f);
	const S32 stepCount = 32;
	const F32 deltaAngle = Maths::Pi * 2.0f / (F32)stepCount;
    F32 currAngle = 0;
	for (S32 i = 0; i <= stepCount; i++)
	{        
		Maths::SinCosAlpha sc_angle(currAngle);

		Maths::HVec3 currPos;

		currPos.X(centre.X() + sc_angle.SinAlpha() * radius);
		currPos.Y(centre.Y() + sc_angle.CosAlpha() * radius);
		currPos.Z(centre.Z());
	    
		currAngle = currAngle + deltaAngle;
		g_Curve.AddKeyFrame(currPos);
	}

	g_Curve.Process();

	m_iCount = 0;
	m_fCycle = 0.0f;
	// create the renderers we use
	if(!Render::Debug::g_pDebugRenderSys)
	{
		m_pDebugRenderSys = new Render::Debug::C_DebugRenderSys;
		DbUtil::DefaultDebugRenderSys(m_pDebugRenderSys);
	}
	else
		m_pDebugRenderSys = NULL;
	
	m_pDynamicVB = new Display::C_DynamicVB();

	m_pReducedRender = new ReducedRender::C_Manager;

	m_pPostProcessing = new PostProcess::C_Manager;

	m_pHighDynamicRange = new PostProcess::C_HighDynamicRange;

#ifndef PLATFORM_XENON
	m_pHighDynamicRange->SetHighDynamicRangeType(PostProcess::E_HighDynamicRangeType_ShaderU32);
#endif

	const S32 viewCount = Render::C_ViewManager::Instance().GetViewCount();
	for (U32 viewNo = 0; viewNo < viewCount; ++viewNo)
	{
		const Render::C_View* pView = Render::C_ViewManager::Instance().GetView(viewNo);
		if (pView && pView->m_ViewFlags & Render::E_ViewFlag_EnablePostProcess)
		{
			const Display::C_Viewport& viewport = pView->m_View.GetViewport();
			if(m_pHighDynamicRange->GetType() == PostProcess::E_HighDynamicRangeType_ShaderU32)
				m_pPostProcessing->AddView(viewport.GetW(), viewport.GetH(), pView->m_ViewIndex, PostProcess::E_RenderTargetFormat_RGBA_U8, Display::E_Multisample_None, 5);
			else
				m_pPostProcessing->AddView(viewport.GetW(), viewport.GetH(), pView->m_ViewIndex, PostProcess::E_RenderTargetFormat_RGBA_F10, Display::E_Multisample_None, 5);
		}
	}

	m_pShowDepth = new PostProcess::C_ShowDepth;

	m_pTint = new PostProcess::C_Tint;
	m_pTint->SetColour(MathsStorage::Float3(1.0f, 0.0f, 0.0f));
#ifdef PLATFORM_XENON
	m_pMotionBlur = new PostProcess::C_MotionBlur;
#endif
	m_pBlur = new PostProcess::C_GaussianBlur;
	m_pBlur->SetType(PostProcess::E_Type_Final);

	m_pSceneGraph = new SceneGraph::C_SceneGraph;

	m_pBackFrontSorter = new SceneGraph::C_BackFrontSorter;

	//m_pSceneGraph->SetNumRenderPasses(1);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_Opaque);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_Decal);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_Transparent);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_ParticleOpaque);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_ParticleDecal);
	m_pSceneGraph->RegisterRenderPass(E_RenderPass_ParticleTransparent, m_pBackFrontSorter);

	// add a dynamic effect renderer and add it as a 
	m_pDynamicEffectRenderer = new DynamicEffect::C_Renderer(m_pSceneGraph);
	m_pDynamicEffectRenderer->SetSRGB(true);
	m_pDynamicEffectRenderer->SetAssetPath("Graphics/Textures/Particles/");
	m_pDynamicEffectRenderer->SetRenderPassMapping(DynamicEffect::E_RenderPassIndex_Opaque, E_RenderPass_ParticleOpaque);
	m_pDynamicEffectRenderer->SetRenderPassMapping(DynamicEffect::E_RenderPassIndex_Decal, E_RenderPass_ParticleDecal);
	m_pDynamicEffectRenderer->SetRenderPassMapping(DynamicEffect::E_RenderPassIndex_Transparent, E_RenderPass_ParticleTransparent);
	m_pDynamicEffectRenderer->ReserveVertexBuffers(512 * 1024, 64); // 64x512k buffers

	m_pParticleManager = new DynamicEffect::Particle::C_ParticleManager(m_pDynamicEffectRenderer);

	m_pParticleManager->SetParticleColour(MathsStorage::Float4A16(2.0f, 2.0f, 2.0f, 2.0f));
	m_pParticleManager->SetEmissiveColour(MathsStorage::Float4A16(g_debugParticleEmissive, g_debugParticleEmissive, g_debugParticleEmissive, g_debugParticleEmissive));
	m_pParticleManager->SetLightDirection(MathsStorage::Float3(0.707f, 0.707f, 0.0f));
	m_pParticleManager->ReserveEmitters(2048);
	m_pParticleManager->ReserveEventInstances(256);


	m_pStripManager = new DynamicEffect::Strip::C_Manager(m_pDynamicEffectRenderer);
	m_pStripManager->ReserveEmitters(256);
	m_pBeam = new DynamicEffect::Strip::C_Beam(m_pDynamicEffectRenderer, m_pStripManager, 999);

	g_EmitterDefinition[0].pEffect = m_pBeam;
	g_EmitterDefinition[0].eType = DynamicEffect::Strip::E_EmitterType_Trail;
	g_EmitterDefinition[0].fWidth = 0.4f;
	g_EmitterDefinition[0].fWidthVariation = 0.00f;
	g_EmitterDefinition[0].fSegmentLength = 0.5f;
	g_EmitterDefinition[0].fTextureRepeatLength = 0.25f;
	g_EmitterDefinition[0].fElementLife = 5.0f;
	g_EmitterDefinition[0].fElementLifeVariation = 0.0f;
	g_EmitterDefinition[0].bUpdateStrip = false;
	g_EmitterDefinition[0].bUpdateElements = true;
	g_EmitterDefinition[0].vSpawnOffset = MathsStorage::Float4A16(0.f, 0.f, 0.f, 0.f);
	g_EmitterDefinition[0].vColour1 = MathsStorage::Float4A16(1.f, 1.f, 1.f, 1.f);
	g_EmitterDefinition[0].vColour2 = MathsStorage::Float4A16(1.f, 1.f, 1.f, 1.f);
	g_EmitterDefinition[0].Validate();

	g_EmitterDefinition[1].pEffect = m_pBeam;
	g_EmitterDefinition[1].eType = DynamicEffect::Strip::E_EmitterType_Instant;
	g_EmitterDefinition[1].fWidth = 0.1f;
	g_EmitterDefinition[1].fWidthVariation = 0.05f;
	g_EmitterDefinition[1].fSegmentLength = 2.5f;
	g_EmitterDefinition[1].fStripLife = 0.1f;
	g_EmitterDefinition[1].fStripLifeVariation = 0.0f;
	g_EmitterDefinition[1].bUpdateStrip = true;
	g_EmitterDefinition[1].bUpdateElements = false;
	g_EmitterDefinition[1].vSpawnOffset = MathsStorage::Float4A16(1.f, 1.f, 1.f, 0.f);
	g_EmitterDefinition[1].vColour1 = MathsStorage::Float4A16(1.f, 2.f, 8.f, 1.f);
	g_EmitterDefinition[1].vColour2 = MathsStorage::Float4A16(1.f, 2.f, 8.f, 1.f);
	g_EmitterDefinition[1].Validate();

	::Render::Sky::g_debugSkyDomeIntensity = 3.0f;
	
	DynamicEffect::Particle::C_UserParticleLibrary::Instance().Initialise();
#if defined(PLATFORM_XENON)
	DynamicEffect::Particle::C_UserParticleLibrary::Instance().Open("game:\\data\\particles_global.xep");
#elif defined(PLATFORM_PC)
	DynamicEffect::Particle::C_UserParticleLibrary::Instance().Open("..\\data\\particles_global.pcp");
#elif defined(PLATFORM_PS3)
	DynamicEffect::Particle::C_UserParticleLibrary::Instance().Open("particles_global.p3p");
#endif

	DynamicEffect::XtInterface::Initialise(m_pDynamicEffectRenderer);
	DynamicEffect::Particle::XtInterface::Initialise(m_pParticleManager);

	m_pChartRenderer = new Render::Chart::C_ChartRenderer();
	m_pFpsChart = new Render::Chart::C_LineChart();
	
	m_pFpsChart->SetPosition(0.0f, 0.8f);
	m_pFpsChart->SetSize(1.0f, 0.2f);
	m_pFpsChart->SetAxisRange(0.0f, 60.0f);
	m_pFpsChart->SetHistorySize(512);
	m_pFpsChart->AddSeries("FPS", Maths::C_RGBA_U8(255, 0, 0, 255));

	for(S32 z = 0, i = 0; z < 8; ++z)
	{
		for(S32 x = 0; x < 8; ++x)
		{
			for(S32 y = 0; y < 1; ++y)
			{
				Maths::HMat43 matrix;
				matrix.Translation(Maths::HVec3(10.0f + x * 5.0f, y * 2.0f, z * 5.0f));

				Maths::Store(Maths::HMat44(matrix), &m_Matrices[i++]);
			}
		}
	}

	Maths::HMat43 matrix;
	matrix.Translation(Maths::HVec3(0.0f, 3.0f, 0.0f));

	Maths::Store(Maths::HMat44(matrix), &m_Matrices2[0]);

	// add minimal dynamic effect example
	{
		m_pMinimalDynamicEffect = new DynamicEffect::C_Minimal(m_pDynamicEffectRenderer, 1, 1234);

		S32 randomObjects = 0;//200;

		for(S32 i = 0; i < randomObjects; ++i)
		{
			Maths::C_MatrixRotateRPH rph(g_TestParticlesRandom.FRand(-Maths::Pi, Maths::Pi),
											g_TestParticlesRandom.FRand(-Maths::Pi, Maths::Pi),
											g_TestParticlesRandom.FRand(-Maths::Pi, Maths::Pi));
			Maths::HMat33 rotation(rph);
			Maths::HMat43 transform;
			transform.Translation(Maths::HVec3(g_TestParticlesRandom.FRand(-50.0f, 50.0f), g_TestParticlesRandom.FRand(15.0f, 0.0f), g_TestParticlesRandom.FRand(-50.0f, 50.0f)));
			transform.Rotation(rotation);
			Maths::C_RGBA_F32 colour(g_TestParticlesRandom.FRand(1.0f), g_TestParticlesRandom.FRand(1.0f), g_TestParticlesRandom.FRand(1.0f), 1.0f);

			m_pMinimalDynamicEffect->AddObject(colour, transform, false);
		}
	}

#ifdef ENABLE_CLOTH
	m_pClothRenderer = new DynamicEffect::C_SampleClothRenderer(m_pDynamicEffectRenderer, 333);
	m_pClothRenderer->AddIndexBuffer(MathsStorage::Int2(9, 7));
	m_pClothRenderer->AddIndexBuffer(MathsStorage::Int2(11, 7));
	m_pClothRenderer->ReserveInstances(0, 16, 32);
	
	Maths::HMat33 rotation;
	
	DynamicEffect::ClothSettings parameters;
	parameters.size = MathsStorage::Int2(9, 7);
	parameters.pinnedPositions.PushBack(MathsStorage::Int2(0, 0));
	parameters.pinnedPositions.PushBack(MathsStorage::Int2(0, 3));
	parameters.pinnedPositions.PushBack(MathsStorage::Int2(0, 6));
	parameters.particleMass = 0.5f;
	parameters.stretchNaturalLength = 0.25f;
	parameters.windResistanceCoeff = 15.f;
	parameters.gravity = 4.9f;

	DynamicEffect::C_Cloth* pCloth = m_pClothRenderer->AddCloth(parameters);
	m_pClothRenderer->SetNumManagedObjects(16);
	
#endif

	for(S32 i = 0; i < 8; ++i)
		m_JobQueuePolicy[i] = 7;
	m_JobQueuePolicyHandle = Scheduler::CreatePolicy(m_JobQueuePolicy);
	m_pDynamicEffectRenderer->SetJobQueuePolicyHandle(m_JobQueuePolicyHandle);

	// add the particle system to the scene graph
	m_pSceneGraph->Insert(m_pDynamicEffectRenderer);

	// start the scheduler
	Scheduler::Init();
	Scheduler::Start();
	Scheduler::SetGroupPoolSize(24);

	// enable default passes
	S32 defaultPassFlag =	SceneGraph::Flag<E_RenderPass_Opaque>() |
							SceneGraph::Flag<E_RenderPass_Decal>() |
							SceneGraph::Flag<E_RenderPass_Transparent>();

	// enable particle passes
	S32 particlePassFlag =	SceneGraph::Flag<E_RenderPass_ParticleOpaque>() |
							SceneGraph::Flag<E_RenderPass_ParticleDecal>() |
							SceneGraph::Flag<E_RenderPass_ParticleTransparent>();
	
	if(m_pSceneGraphView[0] = m_pSceneGraph->AddView())
	{
		m_pSceneGraphView[0]->EnableRenderPass(defaultPassFlag);
		m_pSceneGraphView[0]->EnableRenderPass(particlePassFlag);
		m_pSceneGraphView[0]->DisableRenderNode( SceneGraph::Flag<SceneGraph::E_RenderNodeType_ModelInstanceNode>() );
	}

	m_pSceneGraphView[1] = m_pSceneGraph->AddView();
	//m_pSceneGraphView[1]->EnableRenderPass(particlePassFlag);

	m_pSceneGraphView[2] = m_pSceneGraph->AddView();
	m_pSceneGraphView[3] = m_pSceneGraph->AddView();
}

C_TestSectionParticles::~C_TestSectionParticles()
{
	if(g_ParticleEventHandle.Object())
		g_ParticleEventHandle->Free();

	m_pDynamicEffectRenderer->Reset();
	DynamicEffect::Particle::XtInterface::Shutdown();

	delete m_pBackFrontSorter;
	delete m_pSceneGraph;
		
	delete m_pReducedRender;
	delete m_pPostProcessing;
	delete m_pDynamicEffectRenderer;
	delete m_pDynamicVB;
	delete m_pDebugRenderSys;
	delete m_pFpsChart;
	delete m_pChartRenderer;

	

	Render::g_pDepthStencilBuffer = NULL;

	Scheduler::Shutdown();
}

void C_TestSectionParticles::OnResize()
{
	// destroy post process and re-add the views
	m_pPostProcessing->DestroyVolatile();
	m_pPostProcessing->ClearViews();

	m_pReducedRender->DestroyVolatile();
	m_pReducedRender->ClearViews();

	const S32 viewCount = Render::C_ViewManager::Instance().GetViewCount();
	for (U32 viewNo = 0; viewNo < viewCount; ++viewNo)
	{
		const Render::C_View* pView = Render::C_ViewManager::Instance().GetView(viewNo);
		if (pView && pView->m_ViewFlags & Render::E_ViewFlag_EnablePostProcess)
		{
			const Display::C_Viewport& viewport = pView->m_View.GetViewport();
			if(m_pHighDynamicRange->GetType() == PostProcess::E_HighDynamicRangeType_ShaderU32)
				m_pPostProcessing->AddView(viewport.GetW(), viewport.GetH(), pView->m_ViewIndex, PostProcess::E_RenderTargetFormat_RGBA_U8, Display::E_Multisample_None, 5);
			else
				m_pPostProcessing->AddView(viewport.GetW(), viewport.GetH(), pView->m_ViewIndex, PostProcess::E_RenderTargetFormat_RGBA_F10, Display::E_Multisample_None, 5);
		}
	}
	
	m_pPostProcessing->CreateVolatile();
	m_pReducedRender->CreateVolatile();
}


bool C_TestSectionParticles::OnCreateVolatile()
{
	return true;
}

bool C_TestSectionParticles::OnCreate()
{
#ifdef PLATFORM_XENON
	const Render::C_View* pView = Render::C_ViewManager::Instance().GetView(0);	
	if (!Display::Device()->CreateTexture2D(pView->m_View.GetViewport().GetW(), pView->m_View.GetViewport().GetH(), 1, 0, Display::E_Format_D24FS8, &m_DepthTexture))
		return false;
	if (!Display::Device()->CreateTexture2D(1280, 720, 1, 0, Display::E_Format_A16B16G16R16F, &m_OverlapTexture0))
		return false;
	if (!Display::Device()->CreateTexture2D(1280, 720, 1, 0, Display::E_Format_A16B16G16R16, &m_OverlapTexture1))
		return false;
#endif

	return true;
}

void C_TestSectionParticles::OnDestroy()
{
#ifdef PLATFORM_XENON
	Display::Device()->Destroy(&m_DepthTexture);
	Display::Device()->Destroy(&m_OverlapTexture0);
	Display::Device()->Destroy(&m_OverlapTexture1);
#endif
}

void C_TestSectionParticles::OnDestroyVolatile()
{
}

DynamicEffect::Particle::C_EventInstanceHandle handle;

void C_TestSectionParticles::OnUpdate()
{
	m_fTime = Framework::C_Application::GetInstance()->GetTimer().GetElapsedTime();

	DynamicEffect::Particle::XtInterface::Update();

	m_pPostProcessing->SetFrameTime(Framework::C_Application::GetInstance()->GetTimer().GetFrameTime());

	m_pParticleManager->SetEmissiveColour(MathsStorage::Float4A16(g_debugParticleEmissive, g_debugParticleEmissive, g_debugParticleEmissive, g_debugParticleEmissive));
	m_pParticleManager->SetFreezeAmountScale(g_debugParticleFreezeAmountScale);

	Render::C_RenderState::Instance().Update();

	if(!Framework::C_Application::GetInstance()->GetTimer().IsPaused())
	{
		// update Xt once a second
		m_fCycle += Framework::C_Application::GetInstance()->GetTimer().GetFrameTime();

		if(g_ParticleEventIndex != g_debugParticleEventIndex || g_ParticleEventIndex == 0xFFFFFFFF)
		{
			if(g_ParticleEventHandle.Object())
				g_ParticleEventHandle->Free();

			g_ParticleEventIndex = g_debugParticleEventIndex;
			g_ParticleEventId = DynamicEffect::Particle::XtInterface::GetEventId(g_ParticleEventIndex);
		}
		
		DynamicEffect::Particle::C_EventInstance* pEventInstance = NULL;
		if((pEventInstance = g_ParticleEventHandle.Object()))
		{
			pEventInstance->SetSplineUsage(g_debugParticleSplineBegin, g_debugParticleSplineEnd);
		}
		else
		{
			if(g_ParticleEventId != 0xFFFFFFFF && g_ParticleCycle > 1.0f)
			{
				g_ParticleCycle = 0.0f;
				g_ParticleEventHandle = DynamicEffect::Particle::XtInterface::SpawnEvent(g_ParticleEventId, Maths::C_QuatTrans());
				if((pEventInstance = g_ParticleEventHandle.Object()))
				{
					pEventInstance->SetVelocity(Maths::HVec3(0.0f, 0.0f, 0.0f));
					pEventInstance->SetSpline(&g_Curve);
					pEventInstance->SetLighting(MathsStorage::Float4A16(1.0f, 1.0f, 1.0f, 1.0f));
					pEventInstance->SetCollisionPlane(MathsStorage::Float4A16(0.0f, 0.0f, 0.0f, 1.0f), MathsStorage::Float4A16(0.0f, 1.0f, 0.0f, 0.0f));
				}
			}
			else
				g_ParticleCycle += Framework::C_Application::GetInstance()->GetTimer().GetFrameTime();
		}

		if(m_fCycle > 1.0f && g_debugSpawnElectricity)
		{
			U32 objectCount = m_pMinimalDynamicEffect->GetObjectCount();

			U32 object1 = g_TestParticlesRandom.Rand(objectCount);
			Maths::HMat44 matrix1 = Maths::Load<Maths::HMat44>(&m_pMinimalDynamicEffect->GetObject(object1)->m_Transform);
			Maths::HMat43 transform1(matrix1);

			U32 numObjects = g_TestParticlesRandom.Rand(5) + 3;
			for(U32 j = 0; j < numObjects; ++j)
			{
				U32 object2 = g_TestParticlesRandom.Rand(objectCount);
				Maths::HMat44 matrix2 = Maths::Load<Maths::HMat44>(&m_pMinimalDynamicEffect->GetObject(object2)->m_Transform);
				Maths::HMat43 transform2(matrix2);

				//Maths::HVec3 vStart = (g_Random.FRand(20.0f), g_Random.FRand(20.0f), g_Random.FRand(20.0f));

				U32 numStrips = g_TestParticlesRandom.Rand(10) + 2;
				for(U32 i = 0; i < numStrips; ++i)
				{
					if(DynamicEffect::Strip::C_Emitter* pEmitter = m_pStripManager->NewEmitter())
					{
						pEmitter->SetDefinition(&g_EmitterDefinition[1]);

						DynamicEffect::C_Node local, dest;

						local.SetPosition(transform1.Translation());
						dest.SetPosition(transform2.Translation());

						pEmitter->SetLocalTransform(local);
						pEmitter->SetTargetTransform(dest);
					}
				}
			}
		}

		if(m_fCycle > 1.0f)
			m_fCycle = 0.0f;

		for(ProjectileList::Iterator pIterator = m_Projectiles.Begin(); pIterator != m_Projectiles.End(); )
		{
			pIterator->Update(Framework::C_Application::GetInstance()->GetTimer().GetFrameTime());

			Maths::HVec3 vPosition = pIterator->GetPosition();

			if(vPosition.Y() < 0.0f || Maths::Length(vPosition) > 100.0f)
			{
				C_Projectile* pProjectile = &(*pIterator);
				pIterator = m_Projectiles.Erase(pIterator);

				m_pDynamicEffectRenderer->CleanNode(pProjectile->GetNode());
				m_ProjectilePool.Free(pProjectile);
			}
			else
				++pIterator;
		}

		if(Input::GetGameInput(0)->GetButtonPressedDebounced(Framework::E_Button_MenuConfirm) && !m_ProjectilePool.Empty())
		{
			if(C_Projectile* pProjectile = m_ProjectilePool.Allocate())
			{
				Display::C_View* pView = &Framework::C_Application::GetInstance()->GetSplitscreenView(0).m_View;
				const Display::C_ViewSettings3D& viewSettings3D = pView->GetViewSettings3D();
				const Display::C_Matrix& viewMatrix = viewSettings3D.GetViewMatrix();

				Maths::HVec3 vForward(viewMatrix._13, viewMatrix._23, viewMatrix._33);
				Maths::HVec3 vPosition(viewSettings3D.GetCameraPosition());

				DynamicEffect::C_Node node;
				node.SetPosition(vPosition);
				node.SetRotation(DynamicEffect::GetQuaternionFromTo<MathsStorage::Float4A16>(Maths::Constants<Maths::HVec3>::UnitAxisPositiveY(), vForward));
				
				Maths::HVec3 vVelocity = vForward * 25.0f;
				pProjectile->Initialise(vVelocity, node);

				DynamicEffect::Particle::C_EventInstanceHandle handle = DynamicEffect::Particle::XtInterface::SpawnEvent(0x882F08B7, Maths::C_QuatTrans(), pProjectile->GetNode());

				if(DynamicEffect::Particle::C_EventInstance* pInstance = handle.Object())
				{
					pInstance->SetVelocity(vVelocity);
					pInstance->SetLighting(MathsStorage::Float4A16(1.0f, 1.0f, 1.0f, 1.0f));
				}


				if(DynamicEffect::Strip::C_Emitter* pEmitter = m_pStripManager->NewEmitter())
				{
					pEmitter->SetDefinition(&g_EmitterDefinition[0]);
					pEmitter->SetParentTransform(pProjectile->GetNode());
					pEmitter->SetPositionPrevious(vPosition);
				}

				m_Projectiles.PushBack(*pProjectile);
			}
		}

		MathsStorage::Float4A16 rotation;

		Maths::HMat33 mRotation(Maths::C_MatrixRotateRPH(sinf(Framework::C_Application::GetInstance()->GetTimer().GetElapsedTime() * 0.5f) * Maths::Pi * 0.5f, (Framework::C_Application::GetInstance()->GetTimer().GetElapsedTime()*0.1f/0.4f) * Maths::Pi * 2.0f, 0.0f));
		Maths::C_Quaternion qRotation(mRotation);

		Maths::HVec4 vRotation(qRotation);
		Maths::Store<Maths::HVec4>(vRotation, &rotation);

		Maths::HMat43 mTransform(mRotation, Maths::HVec3(0.0f, 0.0f, 5.0f));

		g_Node.SetPosition(MathsStorage::Float4A16(sinf(Framework::C_Application::GetInstance()->GetTimer().GetElapsedTime() * 0.25f), 0.0f, cosf(Framework::C_Application::GetInstance()->GetTimer().GetElapsedTime()* 0.25f) , 1.0f));
		g_Node.SetRotation(rotation);

		{
			Maths::HVec3 vTop(0.0f, 5.0f, 0.0f);
			Maths::HVec3 vBottom(0.0f, 5.75, 0.0f);
			vTop = vTop * mTransform;
			vBottom = vBottom * mTransform;
			//pFlag->Particle(0, 0).position = vTop;
			//pFlag->Particle(0, 5).position = vBottom;
		}
	}

	m_pDynamicEffectRenderer->Update(Framework::C_Application::GetInstance()->GetTimer().GetElapsedTime(), Framework::C_Application::GetInstance()->GetTimer().GetFrameTime(), Framework::C_Application::GetInstance()->GetTimer().IsPaused());
//	m_pDynamicEffectRenderer->UpdateEmitters();

	//m_pDynamicEffectUpdateThread->SetTime(timer.GetTime(), timer.GetFrameTime(), timer.IsPaused());
	//m_pDynamicEffectRenderer->BeginUpdate();
	
	m_pChartRenderer->ResetDeferred();

	//m_pDynamicEffectUpdateThread->Join();
}

void C_TestSectionParticles::OnPreRender()
{
	m_pSceneGraph->BeginFrame();
	
	//m_pDynamicEffectRenderer->BeginFrame();
}

void C_TestSectionParticles::OnBeginPostEffects()
{
	const Display::C_View* pView = Display::Device()->GetView();

	

	// do some reduced resolution rendering right now!!

	if(g_debugRenderReduced)
	{
		m_pReducedRender->Begin(pView->GetIndex(), pView->GetViewport());

		// display the transparent stuff into the reduced rendertarget
//		m_pDynamicEffectRenderer->BeginView();
//		m_pDynamicEffectRenderer->Display(DynamicEffect::E_RenderPass_Transparent);

		// end reduced rendering
		m_pReducedRender->End();
	}

	m_pPostProcessing->Begin(pView->GetIndex(), pView->GetViewport());
	//Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(96, 96, 96, 255));
}

void C_TestSectionParticles::OnEndPostEffects()
{
	m_pHighDynamicRange->Display();
	
	m_pPostProcessing->End();
	
	m_pChartRenderer->RenderDeferred();
}

void C_TestSectionParticles::OnRender3DViewWithAA(const S32 viewIndex)
{
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));

	Render::BeginPostEffectStates();

	// begin view should be called once per view, prior to gathering
	//m_pView->BeginView();

	m_pSceneGraphView[0]->SetViewFrustum(Display::Device()->GetView()->GetViewFrustum());
	m_pSceneGraphView[0]->SetViewData(Display::Device()->GetView()->GetViewSettings3D());
	m_pSceneGraphView[0]->GatherNodes();

	m_pSceneGraphView[1]->SetViewFrustum(Display::Device()->GetView()->GetViewFrustum());
	m_pSceneGraphView[1]->SetViewData(Display::Device()->GetView()->GetViewSettings3D());
	m_pSceneGraphView[1]->GatherNodes();

	m_pSceneGraphView[0]->BeginRender();
	m_pSceneGraphView[0]->Render(E_RenderPass_ParticleOpaque);

	// add cloth here
	#ifdef ENABLE_CLOTH
		DynamicEffect::ClothSettings parameters[10];

		for(S32 i = 0; i < 10; ++i)
		{
			parameters[i].size = MathsStorage::Int2(11, 7);
			parameters[i].pinnedPositions.PushBack(MathsStorage::Int2(0, 0));
			parameters[i].pinnedPositions.PushBack(MathsStorage::Int2(0, 3));
			parameters[i].pinnedPositions.PushBack(MathsStorage::Int2(0, 6));
			parameters[i].particleMass = 0.5f;
			parameters[i].stretchNaturalLength = 0.25f;
			parameters[i].windResistanceCoeff = 15.f;
			parameters[i].gravity = 4.9f;
			
			Maths::C_MatrixRotateRPH rph(0.f, 1.f, 0.f);
			Maths::HMat33 rotation(rph);

			parameters[i].worldMatrix = Maths::HMat43(rotation);
		}

		DynamicEffect::ClothInstance clothInstance;
		clothInstance.vColour = MathsStorage::Float4(1.0f, 1.0f, 1.0f, 1.0f);
		clothInstance.pDiffuse = &m_FlagTextures[1];
		clothInstance.bTransparent = false;
		clothInstance.pObject = NULL;

		S32 n = 0;
		
		for(S32 i = 0; i < 10; ++i)
		{
			for(S32 j = 0; j < 10; ++j)
			{
				Maths::HVec3 vPosition(i * 5.0f, 3.0f, j * 5.0f);
				Maths::C_Sphere sphere(vPosition, 2.0f);
				Maths::HMat43 matrix;
				matrix.Translation(vPosition);
				Maths::Store(Maths::HMat44(matrix), &clothInstance.transform);

				if(Display::Device()->GetViewFrustum().CullSphere(sphere))
					m_pClothRenderer->AddInstanceManaged(clothInstance, DynamicEffect::E_InstancedObjectType_DynamicView, 0, parameters[0]);
				
				++n;
			}
		}

	#endif

	m_SkyDome.Render();
	Render::DrawGrid();

	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_VIEWPROJ_MATRIX, (F32*)&Display::Device()->GetView()->GetViewSettings3D().GetViewProjMatrix(), 4);
	

	Render::EndPostEffectStates();
}

void C_TestSectionParticles::OnRender3DViewWithoutAA(const S32 viewIndex)
{
	Maths::C_AxisBox axisBox(Maths::HVec3(-20.0f, 0.0f, 0.0f), Maths::HVec3(-10, 5.0f, 10.0f));
	const Display::C_View view = *Display::Device()->GetView();

	Render::C_RenderState::Instance().Fog()->SetColour(Maths::C_RGBA_F32(1.0f, 0.0f, 0.0f, 1.0f));
	Render::C_RenderState::Instance().Fog()->SetDepth(Render::E_DepthFog_Euclidean);
	Render::C_RenderState::Instance().Fog()->SetDepthDensity(g_debugFogDensity);
	Render::C_RenderState::Instance().SetFadeIn(g_debugParticleFadeBegin, g_debugParticleFadeEnd);

	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_VIEWPROJ_MATRIX, (F32*)&Display::Device()->GetView()->GetViewSettings3D().GetViewProjMatrix(), 4);

	DbUtil::DefaultDebugRenderSys()->RenderDeferred();

	MathsStorage::Float4 lightDirection(0.707f, 0.707f, 0.0f, 0.0f);

	if(g_debugRotateLight)
	{
		lightDirection.x = 0.707f * cosf(m_fTime);
		lightDirection.z = 0.707f * sinf(m_fTime);
	}

	for(ProjectileList::ConstIterator pIterator = m_Projectiles.Begin(); pIterator != m_Projectiles.End(); ++pIterator)
	{
		pIterator->Render();
	}
	
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(Maths::HVec3(0.0f, 0.0f, 0.0f), Maths::HVec3(lightDirection.x, lightDirection.y, lightDirection.z), 10.0f, 1.0f, Maths::RGBA(255, 255, 0, 255));

	Display::Device()->SetVertexShaderConstantF(CV_RENDER_LIGHT_DIRECTION, (F32*)&lightDirection, 1);
	Display::Device()->SetPixelShaderConstantF(CP_RENDER_LIGHT_DIRECTION, (F32*)&lightDirection, 1);

#ifdef PLATFORM_XENON
	Render::g_pDepthStencilBuffer = &m_DepthTexture;
	D3DRECT rect;
	rect.x1 = 0;
	rect.y1 = 0;
	rect.x2 = Display::Device()->GetView()->GetViewport().GetW();
	rect.y2 = Display::Device()->GetView()->GetViewport().GetH();
	Display::Device()->GetD3DDevice()->Resolve(D3DRESOLVE_DEPTHSTENCIL, &rect, m_DepthTexture.m_pd3dTexture, NULL, 0, 0, NULL, 1.f, 0, NULL);
#endif

	m_pSceneGraphView[0]->Render(E_RenderPass_ParticleDecal);
	m_pSceneGraphView[0]->Render(E_RenderPass_ParticleTransparent);
	

	Curve2::RenderHermite3d(&g_Curve, 0.025f);

	Util::C_String str;
	str.Printf("FPS:%.1f", Framework::C_Application::GetInstance()->GetTimer().GetFrameRate());
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(50, 50), str);

	m_pFpsChart->SetValue(0, Framework::C_Application::GetInstance()->GetTimer().GetFrameRate());
	m_pFpsChart->RenderDeferred();

	m_pSceneGraphView[0]->EndRender();

	// test second view
	m_pSceneGraphView[1]->BeginRender();
	m_pSceneGraphView[1]->EndRender();
}
