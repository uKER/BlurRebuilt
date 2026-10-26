#include "stdafx.h"

#include <Game/GameWorld/View/Mirror.h>
#include <Game/GameWorld/View.h>
#include <Game/GameWorld/ViewManager.h>

#define SCENEGRAPH_ALLOW_INCLUDE
#include <SceneGraph/SceneGraphImpl.h>
#include <SceneGraph/View.h>
#include <SceneGraph/MultiView.h>
#include <SceneGraph/RenderNode.h>
#include <SceneGraph/RenderGroupSettings.h>
#include <Dynamic/RenderNode.h>
#include <Dynamic/Renderer.h>
#include <Render/State/RenderState.h>

#include <Fx/Fx.h>

#include <GameView/OffscreenView.h>
#include <BlobShadows/BlobShadows.h>

#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

#include <Game/GameWorld/ViewManager.h>
#include <Game/GameWorld/RenderPass.h>
#include <Game/GameWorld/ViewType.h>

#include <Game/GameActors/Vehicles/VehicleActor.h>

#include <Graphics/GpuMarkers.h>

#include <AuxFlares/AuxFlares.h>

// TEMPORARY
#include <Game/GameWorld/GameWorld.h>

#if PLATFORM_PC
#include <GameApp.h>	// [Blur2PC] display window size for the mirror target
#endif

namespace Game
{

extern Debug::Bool			g_SceneGraphRenderAlphaVehicleFlares;

extern GameWorldFxParams	g_FxParams;

void C_MirrorView::Create()
{
	// mirror view
	m_pView = m_pViewManager->GetSceneGraph()->AddView();
	m_pView->SetSubtechniqueHandle(m_pViewManager->GetLODSubtechniqueHandle());
	m_pView->EnableRenderPass(SceneGraph::Flag<E_RenderPass_Solid>() |
									SceneGraph::Flag<E_RenderPass_DynamicObjectSolid>() |
									SceneGraph::Flag<E_RenderPass_Punch>() |
//									SceneGraph::Flag<E_RenderPass_Decal>() |
									SceneGraph::Flag<E_RenderPass_Racification>() |
									SceneGraph::Flag<E_RenderPass_Alpha>() |
									SceneGraph::Flag<E_RenderPass_VehicleSolid>() |
									SceneGraph::Flag<E_RenderPass_Sky>() |
									SceneGraph::Flag<E_RenderPass_VehicleAlpha>() |
									SceneGraph::Flag<E_RenderPass_AlphaVehicleFlares>() |
									SceneGraph::Flag<E_RenderPass_RefractAlpha>()
									);
	m_pView->DisableRenderNode(SceneGraph::Flag<SceneGraph::E_RenderNodeType_DynamicLight>());
	m_pView->DisableRenderNode(m_pViewManager->m_pDynamicEffectRenderer->GetNodeFlag());
	m_pView->DisableCullMethod(SceneGraph::E_CullMethod_Portal);
	m_pView->Initialise();
	m_pView->ReserveGroups(2);
	m_pView->GetGroup(0)->SetName("MirrorView");
	m_pView->GetGroup(1)->SetName("MirrorViewAlpha");

#if defined(PLATFORM_PS3)
	Display::E_Multisample multisample = Display::E_Multisample_2;
#else
	Display::E_Multisample multisample = Display::E_Multisample_None;
#endif
	
	m_pMirror = new GameView::C_OffscreenView();
#if PLATFORM_PC
	// [Blur2PC] Retail PC sizes the mirror from the display window (Blur.exe 1.2 0x00429480 -> 0x00427660).
	SizeFromDisplayWindow();
#else
	m_pMirror->SetFormat(512, 64, multisample);
#endif
	
	
	m_renderStates.renderFlagsFX.setAlphaToMask = false;
	m_renderStates.renderFlagsFX.setBlendState = false;
	m_renderStates.setBlendMode = true;
	m_renderStates.alphaToMaskEnable = Display::E_Bool_False;
	m_renderStates.alphaRef = 0x80;
	m_renderStates.alphaTestEnable = Display::E_Bool_True;
	m_renderStates.srcBlend = Display::E_Blend_One;
	m_renderStates.destBlend = Display::E_Blend_Zero;
	m_renderStates.blendOp = Display::E_BlendOp_Add;
	m_renderStates.blendEnable = Display::E_Bool_False;

	m_MirrorRefractAlphaRenderStates.renderFlagsRD.setPass = 0;
	m_MirrorRefractAlphaRenderStates.renderFlagsFX.setBlendState = 0;
	m_MirrorRefractAlphaRenderStates.colourWriteEnable = Display::E_ColourWriteEnable_Red|Display::E_ColourWriteEnable_Green|Display::E_ColourWriteEnable_Blue;
	m_MirrorRefractAlphaRenderStates.setBlendMode = true;
	m_MirrorRefractAlphaRenderStates.blendEnable = Display::E_Bool_True;
	m_MirrorRefractAlphaRenderStates.blendHighPrecisionEnable = Display::E_Bool_True;
	m_MirrorRefractAlphaRenderStates.srcBlend = Display::E_Blend_One;
	m_MirrorRefractAlphaRenderStates.destBlend = Display::E_Blend_InvSrcAlpha;
	m_MirrorRefractAlphaRenderStates.blendOp = Display::E_BlendOp_Add;
	m_MirrorRefractAlphaRenderStates.alphaToMaskEnable = Display::E_Bool_False;
	m_MirrorRefractAlphaRenderStates.alphaTestEnable = Display::E_Bool_True;
	m_MirrorRefractAlphaRenderStates.alphaRef = 0;
	m_MirrorRefractAlphaRenderStates.setDepth = true;
	m_MirrorRefractAlphaRenderStates.depthEnable = Display::E_Bool_False;
	m_MirrorRefractAlphaRenderStates.depthWriteEnable = Display::E_Bool_False;
	m_MirrorRefractAlphaRenderStates.depthFunc = Display::E_CmpFunc_GreaterEqual;


	m_RenderGroupSettings.SetNumWorkers(1);
#if !PLATFORM_PC
	m_RenderGroupSettings.SetMultisample(multisample);	// [Blur2PC] PC: set by SizeFromDisplayWindow (retail 0x00427660)
#endif

	m_pMirror->Create();

	m_DetailFactor = 1.f;
}

void C_MirrorView::Destroy()
{
	m_pViewManager->GetSceneGraph()->RemoveView(m_pView);

	m_pMirror->Destroy();
	delete m_pMirror;
}

Debug::Bool g_MirrorEnable("Amax.Views.MirrorEnableTheReturn", true);
#if PLATFORM_PC
// [Blur2PC] Retail PC names these Max/Min (Blur.exe 1.2 0x0111e828/0x0111e860, same defaults); the
// graphical detail preset sets them.
Debug::Float g_MirrorInstanceDrawDistanceScale("Amax.Views.MirrorInstanceDrawDistanceScaleMax", 0.25f, Debug::Range(0.0f, 1.0f));
Debug::Float g_MirrorInstanceDrawDistanceScaleLOD("Amax.Views.MirrorInstanceDrawDistanceScaleMin", 0.05f, Debug::Range(0.0f, 1.0f));
#else
Debug::Float g_MirrorInstanceDrawDistanceScale("Amax.Views.MirrorInstanceDrawDistanceScale", 0.25f, Debug::Range(0.0f, 1.0f));
Debug::Float g_MirrorInstanceDrawDistanceScaleLOD("Amax.Views.MirrorInstanceDrawDistanceScaleLOD", 0.05f, Debug::Range(0.0f, 1.0f));
#endif
Debug::Float g_MirrorShadowDrawDistance("Amax.Views.MirrorShadowDrawInstance", 30.f);
Debug::Float g_MirrorDrawDistance("Amax.Views.MirrorDrawDistance", 500.f, Debug::Range(0.f, 1000.f));
Debug::Float g_MirrorDrawDistanceLOD("Amax.Views.MirrorDrawDistanceLOD", 150.f, Debug::Range(0.f, 1000.f));

void C_MirrorView::Gather(C_View& gameView)
{
	if(!g_MirrorEnable)
		return;

	PERF_EVENT("Render.GatherMirror");

	m_pMirror->SetupView();
	
	//Setup scenegraph view
	m_pView->SetViewFrustum(m_pMirror->GetView().GetViewFrustum());
	m_pView->SetViewData(m_pMirror->GetView().GetViewSettings3D());
	m_pView->SetDrawDistanceScale(Maths::Lerp(g_MirrorInstanceDrawDistanceScaleLOD, g_MirrorInstanceDrawDistanceScale, m_DetailFactor));
	m_pView->SetType(E_ViewType_Mirror);
	m_pView->SetFarClip(Maths::Lerp(g_MirrorDrawDistanceLOD, g_MirrorDrawDistance, m_DetailFactor));
	m_pView->SetPvsCell(gameView.GetMainView().GetPvsCell());
	
	gameView.m_pMultiView->AddView(m_pView);
}

void C_MirrorView::Schedule(C_View& gameView)
{
	PERF_EVENT("Render.ScheduleMirror");
	{
		PERF_EVENT("Render.SetFxParams");
		m_pViewManager->SetLightFlareParameters();

		const S32 viewIndex = Framework::C_ApplicationBase::GetInstance()->GetCurrentSplitscreenView();
		if (Render::C_RenderState::Instance().ShaderHDR())
		{
			const Display::C_Float4	psLuminance = Render::C_RenderState::Instance().HDR()->GetShaderParameters(viewIndex);
			FX::SetParameter(g_FxParams.luminanceParameter, &psLuminance);
		}
	}

	if(!g_MirrorEnable)
		return;

	const C_LightFlareSettings carSettings = m_pViewManager->m_pGameWorld->GetCarLightFlareSettings();
	m_pViewManager->m_pGameWorld->GetAuxFlares()->SetViewSettings(E_ViewType_Mirror, carSettings.m_GlobalSize * carSettings.m_GlobalSizeX, carSettings.m_GlobalSize * carSettings.m_GlobalSizeY, carSettings.m_GlobalColour);

	m_pView->BeginSchedule(0, m_pMirror->GetView(), &m_RenderGroupSettings);
	m_pView->SchedulePass(E_RenderPass_VehicleSolid);
	m_pView->SchedulePass(E_RenderPass_DynamicObjectSolid);
	m_pView->SchedulePass(E_RenderPass_Solid);
	m_pView->SchedulePass(E_RenderPass_Punch, &m_renderStates);
	m_pView->SchedulePass(E_RenderPass_Sky);
	//	m_pView->SchedulePass(E_RenderPass_Decal, &m_renderStates);
	m_pView->SchedulePass(E_RenderPass_Racification);
	m_pView->SchedulePass(E_RenderPass_VehicleAlpha);
	m_pView->EndSchedule();

	m_pView->BeginSchedule(1, m_pMirror->GetView(), &m_RenderGroupSettings);
	m_pView->SchedulePass(E_RenderPass_RefractAlpha, &m_MirrorRefractAlphaRenderStates); //Force alpha render pass settings for mirror
	m_pView->SchedulePass(E_RenderPass_Alpha);
	if(g_SceneGraphRenderAlphaVehicleFlares)
		m_pView->SchedulePass(E_RenderPass_AlphaVehicleFlares);
	m_pView->EndSchedule();
}

void C_MirrorView::Render(C_View& gameView)
{
	if(!g_MirrorEnable)
		return;

	CPU_TIMING(5, Maths::RGBA(255, 255, 255, 255), "CPU.RenderMirror");
	GPU_TIMING(5, Maths::RGBA(255, 255, 255, 255), "GPU.RenderMirror");
	
	PERF_EVENT("Render.RenderMirror");

	// bind the current view
	m_pMirror->Bind();

#if defined(PLATFORM_PS3)
	Display::Device()->SetDepthEnable(Display::E_Bool_True);
#endif

	m_pView->BeginRender();
	m_pView->RenderGroup(0);

	// TODO: these need to go into the scenegraph
	{
		PERF_EVENT("Render.MirrorShadows");
		Display::Device()->InvalidateState();

		//Render shadows in mirror
		for (U32 iVehicle = 0; iVehicle < m_pViewManager->m_pAmaxWorldLayer->GetRaceLogicCore()->GetNumVehicles(); iVehicle++)
		{
			Game::Actors::C_VehicleActor* pVehicleActor = const_cast<Game::Actors::C_VehicleActor*>(m_pViewManager->m_pAmaxWorldLayer->GetRaceLogicCore()->GetVehicleActor(iVehicle));
			if (pVehicleActor && pVehicleActor->RenderShadowInMirror())
			if ((m_pMirror->GetPosition() - pVehicleActor->GetMatrix().AW()).Length() < (g_MirrorShadowDrawDistance * m_DetailFactor))
			{
				const Btl::Vector<Maths::HVec3>& collisionVertices = pVehicleActor->GetCollisionCacheVertices();
				if (collisionVertices.Size() > 0)
					m_pViewManager->m_pBlobShadows->RenderCollisionMeshShadow(pVehicleActor->GetMatrix(), pVehicleActor->GetGroundPosition(), 1.f, &collisionVertices[0], collisionVertices.Size());
			}
		}
	}

	m_pView->RenderGroup(1);
	m_pView->EndRender();

	// unbind the current view
	m_pMirror->Unbind();
}

void C_MirrorView::SetupMirror(const Maths::HVec3 position, const Maths::HVec3 view, const Maths::HVec3 up, const F32 fieldOfView)
{
	m_pMirror->SetCamera(position, view, up);
	m_pMirror->SetupView(6.4f, fieldOfView, 500.f);
}

const Display::C_RenderTexture2D* C_MirrorView::GetRenderTexture() const
{
	return &m_pMirror->GetTextureRender()->GetRenderTexture();
}

void C_MirrorView::SetDynamicDetail(F32 detail)
{
	m_DetailFactor = detail;
}

#if PLATFORM_PC
// [Blur2PC] Retail PC graphical detail for the mirror, from Blur.exe 1.2 0x004295f0. It ends by resizing the
// mirror render target to the display window (0x00427660).
void C_MirrorView::SetGraphicalDetail(const U32 graphicalDetail)
{
	static const F32 s_Max[4] = { 0.15f, 0.2f, 0.3f, 0.5f };
	static const F32 s_Min[4] = { 0.075f, 0.1f, 0.15f, 0.25f };

	if(graphicalDetail >= 1 && graphicalDetail <= 4)
	{
		g_MirrorInstanceDrawDistanceScale = s_Max[graphicalDetail - 1];
		g_MirrorInstanceDrawDistanceScaleLOD = s_Min[graphicalDetail - 1];
	}

	SizeFromDisplayWindow();
}

// [Blur2PC] Retail PC mirror render target size, from Blur.exe 1.2 0x00427660 (called when the mirror is
// created and from the graphical detail hook, which the settings code runs before a device reset): 0.45 x
// 0.125 of display window 0, each truncated and rounded up to a multiple of 32, with the window's multisample.
// Retail hands the size to the offscreen view (0x00b1fd20) only when it differs from the last one.
// The render target and the mirror's render group take the window's multisample; the device renders into a
// multisampled render target and resolves it into the texture on unbind (0x01f01680, 0x005adff0).
void C_MirrorView::SizeFromDisplayWindow()
{
	const C_GameApp* pApp = C_GameApp::GetInstance();
	const U32 windowWidth = pApp->GetDisplayWindowWidth();
	const U32 windowHeight = pApp->GetDisplayWindowHeight();
	const Display::E_Multisample windowMultisample = pApp->GetDisplayWindowMultisample();

	// retail: (0x1f - (S32)(width * -0.45f)) & ~0x1f, the float-to-int conversion truncating
	const U32 width = (U32)(0x1f - (S32)((F32)windowWidth * -0.45f)) & ~0x1fu;
	const U32 height = (U32)(0x1f - (S32)((F32)windowHeight * -0.125f)) & ~0x1fu;

	if(m_TargetWidth != width || m_TargetHeight != height || m_TargetMultisample != windowMultisample)
	{
		m_TargetWidth = width;
		m_TargetHeight = height;
		m_TargetMultisample = windowMultisample;

		m_RenderGroupSettings.SetMultisample(windowMultisample);
		m_pMirror->SetFormat(width, height, windowMultisample);

		LOGINFO(g_Log, "Mirror target %ux%u multisample %i for display window %ux%u\n", width, height, (S32)windowMultisample, windowWidth, windowHeight);	// not in retail
	}
}
#endif


} // namespace Game
