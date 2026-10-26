#include "stdafx.h"

#include <Game/GameWorld/View/Reflection.h>
#include <Game/GameWorld/View.h>
#include <GameView/ReflectionView.h>

#define SCENEGRAPH_ALLOW_INCLUDE
#include <scenegraph/SceneGraphImpl.h>
#include <SceneGraph/MultiView.h>
#include <SceneGraph/RenderGroupSettings.h>
#include <SceneGraph/View.h>
#include <Game/GameWorld/GameWorld.h>
#include <Game/GameWorld/ViewManager.h>
#include <Game/GameWorld/RenderPass.h>
#include <Game/GameWorld/ViewType.h>

#include <Dynamic/Renderer.h>

#include <Graphics/GpuMarkers.h>

#include <Game/Game.h>
#include <GameLogic/GameData/GameData.h>
#if PLATFORM_PC
#include <GameApp.h>	// [Blur2PC] display window size for the reflection target
#endif

// for raytracing
#include <game/AmaxCam/CameraSystem.h>
#include <SchedulerConfig.h>

#include <AuxFlares/AuxFlares.h>

#include <crowd_system/Effects/CrowdFlags.h>

#include <fx/Fx.h>
#include <framework/view/View.h>

#include "Display/Render/Render.h"
#include "Display/Shaders/vsconstants.h"

Debug::Bool g_bAlphaInReflection("Amax.Views.AlphaInReflection", true);
Debug::Float g_ReflectionDrawDistance("Amax.Views.ReflectionDrawDistance", 500.f, Debug::Range(0.f, 1000.f));
Debug::Float g_ReflectionDrawDistanceLOD("Amax.Views.ReflectionDrawDistanceLOD", 150.f, Debug::Range(0.f, 1000.f));
Debug::Float g_ReflectionFovExpand("Amax.Views.ReflectionFovExpand", 0.05f, Debug::Range(0.f, 1.f));
Debug::Float g_ReflectionFrameBlackSize("Amax.Views.ReflectionFrameBlackSize", 0.04f, Debug::Range(0.f, 1.f)); 
Debug::Bool g_ReflectionFakeSunLight("Amax.Views.ReflectionFakeSunLight", true);
Debug::Float g_ReflectionAuxFlaresNearFade("Amax.Views.ReflectionAuxFlaresNearFade", 60.f, Debug::Range(0.f, 200.f)); 
Debug::Float g_ReflectionAuxFlaresFarFade("Amax.Views.ReflectionAuxFlaresFarFade", 80.f, Debug::Range(0.f, 200.f)); 

namespace Game
{

extern Debug::Bool			g_SceneGraphRenderAlphaVehicleFlares;
extern Debug::Bool			g_EnableDeviceResets;

extern GameWorldFxParams	g_FxParams;

C_ReflectionView::C_ReflectionView(C_ViewManager* pViewManager) : I_Renderer("ReflectionView"), m_pViewManager(pViewManager), m_pView(NULL), m_pReflection(NULL)
{
	m_FakeSunLightMinSize = 0.f;
	m_FakeSunLightMaxSize = 0.f;
	m_FakeSunLightDepthTest = false;
	m_BlackBorderReflection = false;

	m_DetailFactor = 1.f;
#if PLATFORM_PC
	m_GraphicalDetailEnabled = true;	// [Blur2PC] retail ctor 0x00426d00
	m_ReflectionResolution = ReflectionResolution_Medium_512x256;	// retail leaves it unset when the city is not found
	m_TargetWidth = 0;
	m_TargetHeight = 0;
	m_TargetMultisample = Display::E_Multisample_None;
#endif
}

bool C_ReflectionView::OnCreate()
{
	// reflection view
	m_pView = m_pViewManager->GetSceneGraph()->AddView();
	m_pView->SetSubtechniqueHandle(m_pViewManager->GetLODSubtechniqueHandle());
	if (!C_Game::GetInstance()->GetLevelState().mInUI)
	{
		m_pView->EnableRenderPass(SceneGraph::Flag<E_RenderPass_Solid>() |
			//										SceneGraph::Flag<E_RenderPass_DynamicObjectSolid>() |
			SceneGraph::Flag<E_RenderPass_Punch>() |
			SceneGraph::Flag<E_RenderPass_Racification>() |
			SceneGraph::Flag<E_RenderPass_Alpha>() |
			SceneGraph::Flag<E_RenderPass_Sky>() |
			SceneGraph::Flag<E_RenderPass_AlphaVehicleFlares>()
			);
	}
	else
	{
		m_pView->EnableRenderPass(SceneGraph::Flag<E_RenderPass_Solid>() |
			//										SceneGraph::Flag<E_RenderPass_DynamicObjectSolid>() |
			SceneGraph::Flag<E_RenderPass_Punch>() |
			SceneGraph::Flag<E_RenderPass_Racification>() |
			SceneGraph::Flag<E_RenderPass_Alpha>() |
			SceneGraph::Flag<E_RenderPass_Sky>() |
			SceneGraph::Flag<E_RenderPass_AlphaVehicleFlares>() |
			SceneGraph::Flag<E_RenderPass_VehicleSolid>() | 
			SceneGraph::Flag<E_RenderPass_VehicleAlpha>() |
			SceneGraph::Flag<E_RenderPass_Ghost>()
			);
	}
	
	m_pView->DisableRenderNode(SceneGraph::Flag<SceneGraph::E_RenderNodeType_DynamicLight>());
	m_pView->DisableRenderNode(SceneGraph::Flag<SceneGraph::E_RenderNodeType_StaticInstance>());
	m_pView->DisableRenderNode(m_pViewManager->m_pDynamicEffectRenderer->GetNodeFlag());
	if (m_pViewManager->m_pCrowdFlagRenderer)
		m_pView->DisableRenderNode(m_pViewManager->m_pCrowdFlagRenderer->GetNodeFlag());

	m_pView->DisableCullMethod(SceneGraph::E_CullMethod_Portal);
	m_pView->Initialise();
	m_pView->ReserveGroups(1);
	m_pView->GetGroup(0)->SetName("ReflectionView");

#if PLATFORM_PC
	// [Blur2PC] Retail PC creates the reflection target here, once, sized from the display window (Blur.exe 1.2
	// 0x004296a0); the device recreates it at each reset with the size last handed to it.
	if (AmaxCity* pCity = C_GameData::Instance().mCities[(GAMEDATA_ID)C_Game::GetInstance()->GetLevelState().mCity])
		m_ReflectionResolution = pCity->reflectionResolution;

	m_pReflection = new GameView::C_ReflectionView();
	SizeFromDisplayWindow();
	m_pReflection->Create();

	m_RenderGroupSettings.SetNumWorkers(1);
#endif

	return true;
}

bool C_ReflectionView::OnCreateVolatile()
{
	RESOURCE_MEMORY_ID("Reflection");
	// [Blur2PC][HDR] The blurred reflection keeps retail's A8R8G8B8 under HDR output. Scenery light
	// flares are authored to grow brighter with distance and reach thousands in the sharp reflection;
	// retail's 8-bit blur capped each at 1, a small glow on the wet road. In FP16 they blurred into
	// bright coloured discs that blinked with the flares (T248). Nothing else in it exceeded 1.
	const Display::E_Format blurFormat = Display::E_Format_A8R8G8B8;

#if PLATFORM_PC
	// [Blur2PC] Retail PC (Blur.exe 1.2 0x00426500): the blur textures at half the reflection target, then the
	// black frame; the target itself belongs to OnCreate.
	Display::Device()->CreateRenderTexture2D(m_TargetWidth >> 1, m_TargetHeight >> 1, 1, Display::E_Usage_Colour, blurFormat, Display::E_Multisample_None, 0, &m_BlurAuxReflectionTexture);
	Display::Device()->CreateRenderTexture2D(m_TargetWidth >> 1, m_TargetHeight >> 1, 1, Display::E_Usage_Colour, blurFormat, Display::E_Multisample_None, 0, &m_BlurReflectionTexture);

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_BlurAuxReflectionTexture, "_ReflectionViewBlurAuxRT");
	Display2::Device()->SetRenderTexture2DName(&m_BlurReflectionTexture, "_ReflectionViewBlurRT");
	#endif

	CreateBlackFrame();

	return true;
#else

	U32 reflectionResolutionX = 512;
	U32 reflectionResolutionY = 256;

#if defined(PLATFORM_PS3)
	Display::E_Multisample multisample = Display::E_Multisample_None;
#else
	Display::E_Multisample multisample = Display::E_Multisample_None;
#endif

	if (AmaxCity* pCity = C_GameData::Instance().mCities[(GAMEDATA_ID)C_Game::GetInstance()->GetLevelState().mCity])
	{
		switch (pCity->reflectionResolution)
		{
		case ReflectionResolution_Medium_512x256:
			reflectionResolutionX = 512;
			reflectionResolutionY = 256;
			break;
		case ReflectionResolution_VeryLow_128x64:
			reflectionResolutionX = 128;
			reflectionResolutionY = 64;
			break;
		case ReflectionResolution_Low_256x128:
			reflectionResolutionX = 256;
			reflectionResolutionY = 128;
			break;
		case ReflectionResolution_High_1024x512:
			reflectionResolutionX = 1024;
			reflectionResolutionY = 512;
			break;
		}
	}
	m_pReflection = new GameView::C_ReflectionView();
	m_pReflection->SetFormat(reflectionResolutionX, reflectionResolutionY,  multisample);

	m_pReflection->Create();

	m_RenderGroupSettings.SetNumWorkers(1);
	m_RenderGroupSettings.SetMultisample(multisample);

	Display::Device()->CreateRenderTexture2D(reflectionResolutionX/2, reflectionResolutionY/2, 1, Display::E_Usage_Colour, blurFormat, Display::E_Multisample_None, 0, &m_BlurAuxReflectionTexture);
	Display::Device()->CreateRenderTexture2D(reflectionResolutionX/2, reflectionResolutionY/2, 1, Display::E_Usage_Colour, blurFormat, Display::E_Multisample_None, 0, &m_BlurReflectionTexture);

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_BlurAuxReflectionTexture, "_ReflectionViewBlurAuxRT");
	Display2::Device()->SetRenderTexture2DName(&m_BlurReflectionTexture, "_ReflectionViewBlurRT");
	#endif

	CreateBlackFrame();

	return true;
#endif
}

void C_ReflectionView::OnDestroyVolatile()
{
	Display::Device()->Destroy(&m_BlurAuxReflectionTexture);
	Display::Device()->Destroy(&m_BlurReflectionTexture);

	DestroyBlackFrame();

#if !PLATFORM_PC
	delete m_pReflection;
#endif
}

void C_ReflectionView::OnDestroy()
{
	m_pViewManager->GetSceneGraph()->RemoveView(m_pView);

#if PLATFORM_PC
	// [Blur2PC] retail 0x00426520: the reflection target goes with the view
	delete m_pReflection;
	m_pReflection = NULL;
#endif
}

Debug::Bool g_ReflectionEnable("Amax.Views.ReflectionEnable", true);
Debug::Float g_ReflectionInstanceDrawDistanceScale("Amax.Views.ReflectionInstanceDrawDistanceScale", 0.25f, Debug::Range(0.0f, 1.0f));
Debug::Float g_ReflectionInstanceDrawDistanceScaleLOD("Amax.Views.ReflectionInstanceDrawDistanceScaleLOD", 0.05f, Debug::Range(0.0f, 1.0f));
Debug::Float g_ReflectionInstanceDrawDistanceScaleShowRoom("Amax.Views.ReflectionInstanceDrawDistanceScaleShowRoom", 0.5f, Debug::Range(0.0f, 1.0f));
Debug::Bool g_ShowReflectionPlane("Amax.Views.ShowReflectionPlane", false);
Debug::Bool g_ReflectionClipPlane("Amax.Views.ReflectionClipPlane", true);

void C_ReflectionView::Gather(C_View& gameView)
{
	if(!g_ReflectionEnable)
		return;
#if PLATFORM_PC
	if(!m_GraphicalDetailEnabled)	// [Blur2PC] retail 0x004278d0
		return;
#endif

	PERF_EVENT("Render.GatherReflection");

	// setup reflction view
	const Framework::C_View* pFrameworkView = gameView.GetFrameworkView();
	Maths::C_Plane reflectionPlane = Maths::C_Plane(Maths::C_Normal(0.f, 1.f, 0.f), gameView.GetReferencePosition());
	m_pReflection->SetupView(pFrameworkView->m_ViewSetup, reflectionPlane, g_ReflectionFovExpand);
	m_ReflectionPlane = Display::C_Float4(reflectionPlane.Normal().X(), reflectionPlane.Normal().Y(), reflectionPlane.Normal().Z(), reflectionPlane.DistanceToOrigin());
	
	m_pView->SetViewFrustum(m_pReflection->GetView().GetViewFrustum());
	m_pView->SetViewData(m_pReflection->GetView().GetViewSettings3D());
	if (!C_Game::GetInstance()->GetLevelState().mInUI)
		m_pView->SetDrawDistanceScale(Maths::Lerp(g_ReflectionInstanceDrawDistanceScaleLOD, g_ReflectionInstanceDrawDistanceScale, m_DetailFactor));
	else
		m_pView->SetDrawDistanceScale(g_ReflectionInstanceDrawDistanceScaleShowRoom);
	m_pView->SetType(E_ViewType_Reflection);
	m_pView->SetFarClip(Maths::Lerp(g_ReflectionDrawDistanceLOD, g_ReflectionDrawDistance, m_DetailFactor));
	m_pView->SetPvsCell(gameView.GetMainView().GetPvsCell());

	gameView.m_pMultiView->AddView(m_pView);
}

void C_ReflectionView::Schedule(C_View& gameView)
{
	if(!g_ReflectionEnable)
		return;
#if PLATFORM_PC
	if(!m_GraphicalDetailEnabled)	// [Blur2PC] retail 0x00427ad0
		return;
#endif

	PERF_EVENT("Render.ScheduleReflection");

	{
		PERF_EVENT("Render.SetFxParams");
		const Framework::C_View* pFrameworkView = gameView.GetFrameworkView();

		//Config lightflares for reflection, with reflection interpolation
		m_CameraInc = fabsf(pFrameworkView->m_ViewSetup.m_ViewSetup3D.m_CameraToWorld.AZ().Y());
		
		const C_LightFlareSettings& levelSettings = m_pViewManager->m_pGameWorld->GetLevelLightFlareSettings();
		const C_LightFlareSettings& carSettings = m_pViewManager->m_pGameWorld->GetCarLightFlareSettings();

		F32 reflectionSizeX = Maths::Lerp(levelSettings.m_ReflectionMinSizeX, levelSettings.m_ReflectionMaxSizeX, 1.f - m_CameraInc);
		F32 reflectionSizeY = Maths::Lerp(levelSettings.m_ReflectionMinSizeY, levelSettings.m_ReflectionMaxSizeY, 1.f - m_CameraInc);
		FX::SetParameter(g_FxParams.globalLightFlareSize, Display::C_PSConstant(levelSettings.m_GlobalSize, reflectionSizeX, reflectionSizeY, 0.0f));
		FX::SetParameter(g_FxParams.globalLightFlareColour, Display::C_PSConstant(levelSettings.m_ReflectionColour.R(), levelSettings.m_ReflectionColour.G(), levelSettings.m_ReflectionColour.B(), levelSettings.m_ReflectionColour.A()));

		F32 carReflectionSizeX = Maths::Lerp(carSettings.m_ReflectionMaxSizeX, carSettings.m_ReflectionMaxSizeX, 1.f - m_CameraInc);
		F32 carReflectionSizeY = Maths::Lerp(carSettings.m_ReflectionMinSizeY, carSettings.m_ReflectionMaxSizeY, 1.f - m_CameraInc);
		FX::SetParameter(g_FxParams.carGlobalLightFlareSize, Display::C_PSConstant(carSettings.m_GlobalSize, carReflectionSizeX, carReflectionSizeY, 0.0f));
		FX::SetParameter(g_FxParams.carGlobalLightFlareColour, Display::C_PSConstant(carSettings.m_ReflectionColour.R(), carSettings.m_ReflectionColour.G(), carSettings.m_ReflectionColour.B(), carSettings.m_ReflectionColour.A()));
		F32 carFlareSizeX = carReflectionSizeX * carSettings.m_GlobalSize;
		F32 carFlareSizeY = carReflectionSizeY * carSettings.m_GlobalSize;
		FX::SetParameter(g_FxParams.nearFarLightFlare, Display::C_PSConstant(levelSettings.m_NearFadeDistant, levelSettings.m_FarFadeDistant, levelSettings.m_SizeDistantFactor / 100.f, levelSettings.m_IntensityDistantFactor / 100.f));

		// change the plane reflection to disable the reflection in the mirror and reflection (temp hack)
		Display::C_Float4 preRenderReflectionPlane(0,0,0,1000);
		FX::SetParameter(g_FxParams.planeReflection, &preRenderReflectionPlane);
		m_pViewManager->m_pGameWorld->GetAuxFlares()->SetViewSettings(E_ViewType_Reflection, carFlareSizeX, carFlareSizeY, carSettings.m_ReflectionColour);m_pViewManager->m_pGameWorld->GetAuxFlares()->SetViewSettings(E_ViewType_Reflection, carFlareSizeX, carFlareSizeY, carSettings.m_ReflectionColour);
		m_pViewManager->m_pGameWorld->GetAuxFlares()->SetViewNearFarSettings(E_ViewType_Reflection, g_ReflectionAuxFlaresNearFade, g_ReflectionAuxFlaresFarFade);

		FX::SetParameter(g_FxParams.luminanceParameter, Display::C_PSConstant(1.0f, 1.0f, 1.0f, 1.0f));

		#ifdef PLATFORM_PS3
			//Set user clip plane shared paramater to all shaders with user clip planes
			Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_USERCLIPPLANE, (F32*)&m_ReflectionPlane, 1);
		#endif
	}

	m_pView->BeginSchedule(0, m_pReflection->GetView(), &m_RenderGroupSettings);
//	m_pView->SchedulePass(E_RenderPass_DynamicObjectSolid);
	if (C_Game::GetInstance()->GetLevelState().mInUI)
		m_pView->SchedulePass(E_RenderPass_VehicleSolid);
	m_pView->SchedulePass(E_RenderPass_Solid);
	m_pView->SchedulePass(E_RenderPass_Racification);
	m_pView->SchedulePass(E_RenderPass_Punch);
	m_pView->SchedulePass(E_RenderPass_Sky);

	if (C_Game::GetInstance()->GetLevelState().mInUI)
	{
		m_pView->SchedulePass(E_RenderPass_Ghost, &m_pViewManager->GetGhostDepthStates());
		m_pView->SchedulePass(E_RenderPass_Ghost, &m_pViewManager->GetGhostColourStates());
		m_pView->SchedulePass(E_RenderPass_VehicleAlpha);
	}
	if(g_bAlphaInReflection)
		m_pView->SchedulePass(E_RenderPass_Alpha);
	if(g_SceneGraphRenderAlphaVehicleFlares)
		m_pView->SchedulePass(E_RenderPass_AlphaVehicleFlares);
	m_pView->EndSchedule();
}

void C_ReflectionView::Render(C_View& gameView)
{
	if(!g_ReflectionEnable)
		return;
#if PLATFORM_PC
	if(!m_GraphicalDetailEnabled)	// [Blur2PC] retail 0x004297f0
		return;
#endif

	CPU_TIMING(5, Maths::RGBA(255, 255, 255, 255), "CPU.RenderReflection");
	GPU_TIMING(5, Maths::RGBA(255, 255, 255, 255), "GPU.RenderReflection");

	PERF_EVENT("Render.RenderReflection");

	// bind the current view
	m_pReflection->Bind();

	//Set clipplane

#ifdef PLATFORM_PS3
	//Enabled first user clip plane in Ps3
	cellGcmSetUserClipPlaneControl(CELL_GCM_USER_CLIP_PLANE_ENABLE_GE, CELL_GCM_USER_CLIP_PLANE_DISABLE, CELL_GCM_USER_CLIP_PLANE_DISABLE, CELL_GCM_USER_CLIP_PLANE_DISABLE, CELL_GCM_USER_CLIP_PLANE_DISABLE, CELL_GCM_USER_CLIP_PLANE_DISABLE );		
#endif

	m_pView->BeginRender();
	m_pView->RenderGroup();
	m_pView->EndRender();
	

#ifdef PLATFORM_PS3
	//Disabled first user clip plane in Ps3
	cellGcmSetUserClipPlaneControl(CELL_GCM_USER_CLIP_PLANE_DISABLE, CELL_GCM_USER_CLIP_PLANE_DISABLE, CELL_GCM_USER_CLIP_PLANE_DISABLE, CELL_GCM_USER_CLIP_PLANE_DISABLE, CELL_GCM_USER_CLIP_PLANE_DISABLE, CELL_GCM_USER_CLIP_PLANE_DISABLE );		
#endif

	Display::Device()->InvalidateState();
	Display::Device()->DefaultStates();

	if (g_ReflectionFakeSunLight)
		RenderFakeLight();

	// unbind the current view
	m_pReflection->Unbind();	

	//Display::Device()->InvalidateState();
	//Display::Device()->DefaultStates();

	m_GaussianBlur.GaussBlurX(&m_BlurAuxReflectionTexture, &m_pReflection->GetTextureRender()->GetRenderTexture(), Render::Texture::E_GaussianBlurShader_7, m_pViewManager->GetReflectionBlurX()/2, m_pViewManager->GetReflectionBlurDeviation());
	m_GaussianBlur.GaussBlurY(&m_BlurReflectionTexture, &m_BlurAuxReflectionTexture, Render::Texture::E_GaussianBlurShader_5, m_pViewManager->GetReflectionBlurY() / 2.f, m_pViewManager->GetReflectionBlurDeviation());

	if (m_BlackBorderReflection)
		RenderBackFrame();
}

const Display::C_RenderTexture2D* C_ReflectionView::GetRenderTexture() const
{
	return &m_pReflection->GetTextureRender()->GetRenderTexture();
}

const Display::C_RenderTexture2D* C_ReflectionView::GetBlurRenderTexture() const
{
	return &m_BlurReflectionTexture;
}

const Display::C_Matrix& C_ReflectionView::GetMatrix() const
{
	return m_pReflection->GetView().GetViewSettings3D().GetViewProjMatrix();
}

void C_ReflectionView::SetDynamicDetail(F32 detail)
{
	m_DetailFactor = detail;
}

#if PLATFORM_PC
// [Blur2PC] Retail PC graphical detail for the reflection (Blur.exe 1.2 0x020bffa8): resizes the reflection
// render target to the display window (0x00427750), then renders the reflection only at detail 4.
void C_ReflectionView::SetGraphicalDetail(const U32 graphicalDetail)
{
	SizeFromDisplayWindow();
	m_GraphicalDetailEnabled = graphicalDetail >= 4;
}

// [Blur2PC] Retail PC reflection target size, from Blur.exe 1.2 0x00427750 (called from OnCreate and the
// graphical detail hook, which the settings code runs before a device reset): half of display window 0
// (truncated), then /4 (VeryLow), /2 (Low), unchanged (Medium) or x2 (High) by the city's reflection
// resolution, each rounded up to a multiple of 32, with the window's multisample. Retail hands it to the
// reflection view (0x00b20450) only when it differs from the last one and returns whether it did; the blur
// textures follow at the next volatile create.
bool C_ReflectionView::SizeFromDisplayWindow()
{
	const C_GameApp* pApp = C_GameApp::GetInstance();
	const Display::E_Multisample windowMultisample = pApp->GetDisplayWindowMultisample();

	U32 width = (U32)((F32)pApp->GetDisplayWindowWidth() * 0.5f);
	U32 height = (U32)((F32)pApp->GetDisplayWindowHeight() * 0.5f);

	switch (m_ReflectionResolution)
	{
	case ReflectionResolution_VeryLow_128x64:
		width >>= 2;
		height >>= 2;
		break;
	case ReflectionResolution_Low_256x128:
		width >>= 1;
		height >>= 1;
		break;
	case ReflectionResolution_High_1024x512:
		width *= 2;
		height *= 2;
		break;
	}

	width = (width + 0x1f) & ~0x1fu;
	height = (height + 0x1f) & ~0x1fu;

	if (m_TargetWidth == width && m_TargetHeight == height && m_TargetMultisample == windowMultisample)
		return false;

	m_TargetWidth = width;
	m_TargetHeight = height;
	m_TargetMultisample = windowMultisample;

	m_pReflection->SetFormat(width, height, windowMultisample, 1);

	LOGINFO(g_Log, "Reflection target %ux%u multisample %i (city reflection resolution %i)\n", width, height, (S32)windowMultisample, m_ReflectionResolution);	// not in retail
	return true;
}
#endif

void C_ReflectionView::CreateBlackFrame()
{
	if (!Display::Device()->CreateVertexBuffer(sizeof(Display::Vertex_XY_C) * 10, 0, &m_FrameVertexBuffer))
		return;

	F32 offset = 0.0f;

	Display::Vertex_XY_C vertex[8] = 
	{
		{ -1.f - offset, -1.f - offset, GPU_U8_4_TO_U32(0,0,0,0)},
		{ -1.f + g_ReflectionFrameBlackSize, -1.f + g_ReflectionFrameBlackSize * 2.f, GPU_U8_4_TO_U32(255,255,255,0)},
		{ 1.f + offset, -1.f - offset, GPU_U8_4_TO_U32(0,0,0,0)},
		{ 1.f - g_ReflectionFrameBlackSize, -1.f + g_ReflectionFrameBlackSize * 2.f, GPU_U8_4_TO_U32(255,255,255,0)},
		{ 1.f + offset, 1.f + offset, GPU_U8_4_TO_U32(0,0,0,0)},
		{ 1.f - g_ReflectionFrameBlackSize, 1.f - g_ReflectionFrameBlackSize * 2.f, GPU_U8_4_TO_U32(255,255,255,0)},
		{ -1.f - offset, 1.f + offset, GPU_U8_4_TO_U32(0,0,0,0)},
		{ -1.f + g_ReflectionFrameBlackSize, 1.f - g_ReflectionFrameBlackSize * 2.f, GPU_U8_4_TO_U32(255,255,255,0)},
	};

	Display::Vertex_XY_C* pVertex = (Display::Vertex_XY_C*)m_FrameVertexBuffer.Lock();
	if (pVertex)
	{
		*pVertex++ = vertex[0];
		*pVertex++ = vertex[1];
		*pVertex++ = vertex[2];
		*pVertex++ = vertex[3];
		*pVertex++ = vertex[4];
		*pVertex++ = vertex[5];
		*pVertex++ = vertex[6];
		*pVertex++ = vertex[7];
		*pVertex++ = vertex[0];
		*pVertex++ = vertex[1];
		m_FrameVertexBuffer.Unlock();
	}
}
void C_ReflectionView::DestroyBlackFrame()
{
	Display::Device()->DestroyVertexBuffer(&m_FrameVertexBuffer);
}

void C_ReflectionView::RenderBackFrame()
{
	//Set RenderBuffer
	Display::Device()->SetColourDepthStencilSurface(&m_BlurReflectionTexture.GetSurface(), 0);

	Display::C_Viewport viewport;
	viewport.m_X = 0;
	viewport.m_Y = 0;
	viewport.m_W = m_BlurReflectionTexture.GetSurface().GetWidth();
	viewport.m_H = m_BlurReflectionTexture.GetSurface().GetHeight();
	Display::Device()->SetViewport(viewport);

	//Set States
	Display::Device()->SetBlendMode(Display::E_BlendMode_Multiply);
	Display::Device()->SetDepthEnable(Display::E_Bool_False);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
	Display::Device()->SetCullMode(Display::E_CullMode_None);

	//Set Vertex and Shaders
	Display::Device()->SetVertexShader(&Display::VertexShader_XY::m_VertexShaderVertexColour);
	Display::Device()->SetPixelShader(&Display::PixelShader::m_PixelShaderVertexColour);
	//Draw
	Display::Device()->SetVertexBuffer(0, &m_FrameVertexBuffer, 0, sizeof(Display::Vertex_XY_C));
	Display::Device()->SetVertexDeclaration( Display::Vertex_XY_C::m_pDeclaration);

	Display::Device()->DrawPrimitive(Display::E_PrimitiveType_TriangleStrip, 0, 10);

	m_BlurReflectionTexture.Resolve();

	Display::Device()->SetVertexShader(NULL);
	Display::Device()->SetPixelShader(NULL);

	Display::Device()->SetBlendMode(Display::E_BlendMode_Default);
	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
	Display::Device()->SetDepthEnable(Display::E_Bool_True);

}

void C_ReflectionView::RenderFakeLight()
{

	if (m_FakeSunLightMaxSize > 0.01f && m_FakeSunLightMinSize > 0.01f)
	{

		//Setup states
		Display::Device()->SetBlendMode(Display::E_BlendMode_AlphaPreMultiply);
		Display::Device()->SetCullMode(Display::E_CullMode_None);
		if (m_FakeSunLightDepthTest)
			Display::Device()->SetDepthEnable(Display::E_Bool_True);
		else
			Display::Device()->SetDepthEnable(Display::E_Bool_False);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);	

		Display::Device()->SetVertexShader(&m_pViewManager->GetFakeLightingVertexShader());
		Display::Device()->SetPixelShader(&m_pViewManager->GetFakeLightingPixelShader());

		// knock out until we have a begin vertices method for PC
	#ifndef PLATFORM_PC

		Display::Vertex_XYZ_HDR_UV* pVertices = (Display::Vertex_XYZ_HDR_UV*)Display::Device()->BeginVertices(Display::E_PrimitiveType_QuadList, 4, sizeof(Display::Vertex_XYZ_HDR_UV), Display::Vertex_XYZ_HDR_UV::m_pDeclaration);

		if(pVertices)
		{
			Maths::HVec3 position = m_FakeSunLightPosition;
			Maths::HVec4 colour = Maths::HVec4(m_FakeSunLightColour.R(), m_FakeSunLightColour.G(), m_FakeSunLightColour.B(), F32(Maths::Lerp(m_FakeSunLightMaxSize, m_FakeSunLightMinSize, m_CameraInc)));


			Maths::Store(position, (MathsStorage::Float3*)&pVertices[0].x);
			Maths::Store(colour, (MathsStorage::Float4*)&pVertices[0].r);
			pVertices[0].u = -1.f;
			pVertices[0].v = 1.f;

			Maths::Store(position, (MathsStorage::Float3*)&pVertices[1].x);
			Maths::Store(colour, (MathsStorage::Float4*)&pVertices[1].r);
			pVertices[1].u = 1.f;
			pVertices[1].v = 1.f;

			Maths::Store(position, (MathsStorage::Float3*)&pVertices[2].x);
			Maths::Store(colour, (MathsStorage::Float4*)&pVertices[2].r);
			pVertices[2].u = 1.f;
			pVertices[2].v = -1.f;

			Maths::Store(position, (MathsStorage::Float3*)&pVertices[3].x);
			Maths::Store(colour, (MathsStorage::Float4*)&pVertices[3].r);
			pVertices[3].u = -1.f;
			pVertices[3].v = -1.f;
		}
		Display::Device()->EndVertices();

	#endif

		Display::Device()->SetVertexShader(NULL);
		Display::Device()->SetPixelShader(NULL);

		Display::Device()->SetBlendMode(Display::E_BlendMode_Default);
		Display::Device()->SetCullMode(Display::E_CullMode_Default);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
		Display::Device()->SetDepthEnable(Display::E_Bool_True);
	}
}

void C_ReflectionView::UpdateFakeSunLight( const Maths::C_RGBA_F32& colour, const Maths::HVec3& position, const F32& minSize, const F32& maxSize, const bool& depthTest)
{
	m_FakeSunLightColour = colour;
	m_FakeSunLightPosition = position;
	m_FakeSunLightMinSize = minSize;
	m_FakeSunLightMaxSize = maxSize;
	m_FakeSunLightDepthTest = depthTest;
}

void C_ReflectionView::EnableBlackBorderReflection( const bool blackBorderReflection )
{
	m_BlackBorderReflection = blackBorderReflection;
}
} // namespace Game
