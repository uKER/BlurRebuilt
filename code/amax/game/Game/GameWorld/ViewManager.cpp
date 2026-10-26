#include "stdafx.h"

#include <Game/GameWorld/ViewManager.h>
#include <Game/GameWorld/GameWorld.h>
#include <Game/GameWorld/View.h>
#include <Game/GameWorld/View/CubeMap.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameApp.h>
#include <Fx/Fx.h>
#include <Render/ReducedRender/Manager.h>
#include <BlobShadows/BlobShadows.h>
#include <SceneGraph/View.h>

#define SCENEGRAPH_ALLOW_INCLUDE
#include <SceneGraph/SceneGraphImpl.h>
#include <Game/GameWorld/ViewType.h>
#include <Dynamic/Renderer.h>

EXTERN_SHADER(vsFakeEnvironmentLight);
EXTERN_SHADER(psFakeEnvironmentLight);

namespace Game
{

C_ViewManager::C_ViewManager(SceneGraph::C_SceneGraph* pSceneGraph, C_GameWorld* pGameWorld) :
	m_pSceneGraph(pSceneGraph),
	m_pGameWorld(pGameWorld),
	m_pCubeMap(NULL),
	m_ReduceRenderAlpha(Render::Reduced::E_Downsample_None),
	m_ReduceRenderAlphaCycle(0.0f),
	m_DisableAACycle(0.0f),
	m_pCrowdFlagRenderer(NULL),
	m_pLightTrailsView(0)
{
	m_LODSubtechniqueHandle = FX::RegisterSubtechnique("LOD");
#if PLATFORM_PC
	m_NoDamageSubtechniqueHandle = FX::RegisterSubtechnique("NoDamage");	// [Blur2PC] retail order, Blur.exe 1.2 0x00428ff0
#endif
	m_PrepassSubtechniqueHandle = FX::RegisterSubtechnique("Prepass");
	m_LightsSubtechniqueHandle = FX::RegisterSubtechnique("Lights");
	m_LightsSplitScreenSubtechniqueHandle = FX::RegisterSubtechnique("LSplitScreen");
	m_ElectricSubtechniqueHandle = FX::RegisterSubtechnique("Electric");
	m_SolidSubtechniqueHandle = FX::RegisterSubtechnique("Solid");
	m_AlphaLuminanceSubtechniqueHandle = FX::RegisterSubtechnique("AlphaLuminance");
	m_EnvironmentSubtechniqueHandle = FX::RegisterSubtechnique("Environment");
#if PLATFORM_PC
	m_NoDepthSubtechniqueHandle = FX::RegisterSubtechnique("NoDepth");	// [Blur2PC] retail
#endif

	m_ReflectionBlurX = 1.f;
	m_ReflectionBlurY = 1.f;
	m_ReflectionBlurDeviation = 1.f;
}

C_ViewManager::~C_ViewManager()
{
}

void C_ViewManager::Create(const S32 views, const bool mirrorEnabled, const bool reflectionEnabled, const bool cubeMapEnabled)
{
	assert(views == Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount());

	m_Views.Resize(views);
	for(S32 i = 0; i < views; ++i)
	{
		
		C_View* pView = new C_View(this, i);
		pView->Create(mirrorEnabled, reflectionEnabled);

		m_Views[i] = pView;
	}

	if(cubeMapEnabled)
	{
		m_pCubeMap = new C_CubeMapView(this);
		m_pCubeMap->Create();
	}

	// LightTrails
	{
		m_pLightTrailsView = GetSceneGraph()->AddView();

		m_pLightTrailsView->SetType(Game::E_ViewType_LightTrails);

		m_pLightTrailsView->EnableRenderPass(0xFFFFFFFF);
		m_pLightTrailsView->DisableRenderNode(0xFFFFFFFF);

		m_pLightTrailsView->EnableRenderNode(m_pGameWorld->GetDynamicEffectRenderer()->GetNodeFlag());
	}

	// set up cached pointers
	Game::C_RaceBookLayer*	pRaceBookLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	m_pPostProcessManager = m_pGameWorld->GetManager();
	m_pPostProcessBloom = m_pGameWorld->GetBloom();
	m_pPostProcessHdr = m_pGameWorld->GetHDR();
	m_pAmaxWorldLayer = pRaceBookLayer->GetAmaxWorldLayer();
	m_pBlobShadows = m_pGameWorld->GetBlobShadows();
	m_pDropShadows = m_pGameWorld->GetDropShadows();
	m_pReducedRender = m_pGameWorld->GetReducedRender();
	m_pLightingPrepass = m_pGameWorld->GetLightingPrepass();
	
	// ghost depth pass
	m_GhostDepthStates.renderFlagsRD.setPass = 0;
	m_GhostDepthStates.renderFlagsFX.setBlendState = 0;

	// on xenon set a null pixel shader, otherwise use colourwrite none
	m_GhostDepthStates.colourWriteEnable = Display::E_ColourWriteEnable_None;

	m_GhostDepthStates.setBlendMode = true;
	m_GhostDepthStates.blendEnable = Display::E_Bool_False;
	m_GhostDepthStates.srcBlend = Display::E_Blend_One;
	m_GhostDepthStates.destBlend = Display::E_Blend_One;
	m_GhostDepthStates.blendOp = Display::E_BlendOp_Add;
	m_GhostDepthStates.alphaToMaskEnable = Display::E_Bool_False;
	m_GhostDepthStates.alphaTestEnable = Display::E_Bool_False;
	m_GhostDepthStates.alphaRef = 0;
	m_GhostDepthStates.setDepth = true;
	m_GhostDepthStates.depthEnable = Display::E_Bool_True;
	m_GhostDepthStates.depthWriteEnable = Display::E_Bool_True;
	m_GhostDepthStates.depthFunc = Display::E_CmpFunc_Greater;
	m_GhostDepthStates.stencilState.m_StencilEnable		= Display::E_Bool_True;
	m_GhostDepthStates.stencilState.m_StencilFunc		= Display::E_CmpFunc_Always;
	m_GhostDepthStates.stencilState.m_StencilRef		= 0xFF;
	m_GhostDepthStates.stencilState.m_StencilMask		= 0xFF;
	m_GhostDepthStates.stencilState.m_StencilWriteMask	= 0xFF;
	m_GhostDepthStates.stencilState.m_StencilFail		= Display::E_StencilOp_Keep;
	m_GhostDepthStates.stencilState.m_StencilDepthFail	= Display::E_StencilOp_Keep;
	m_GhostDepthStates.stencilState.m_StencilDepthPass	= Display::E_StencilOp_Replace;

	// ghost colour pass
	m_GhostColourStates.renderFlagsRD.setPass = 0;
	m_GhostColourStates.renderFlagsFX.setBlendState = 0;
	m_GhostColourStates.colourWriteEnable = Display::E_ColourWriteEnable_Red|Display::E_ColourWriteEnable_Green|Display::E_ColourWriteEnable_Blue;
	m_GhostColourStates.setBlendMode = true;
	m_GhostColourStates.blendEnable = Display::E_Bool_True;
	m_GhostColourStates.blendHighPrecisionEnable = Display::E_Bool_True;
	m_GhostColourStates.srcBlend = Display::E_Blend_SrcAlpha;
	m_GhostColourStates.destBlend = Display::E_Blend_InvSrcAlpha;
	m_GhostColourStates.blendOp = Display::E_BlendOp_Add;
	m_GhostColourStates.alphaToMaskEnable = Display::E_Bool_False;
	m_GhostColourStates.alphaTestEnable = Display::E_Bool_True;
	m_GhostColourStates.alphaRef = 0;
	m_GhostColourStates.setDepth = true;
	m_GhostColourStates.depthEnable = Display::E_Bool_True;
	m_GhostColourStates.depthWriteEnable = Display::E_Bool_False;
	m_GhostColourStates.depthFunc = Display::E_CmpFunc_Equal;
	m_GhostColourStates.stencilState.m_StencilEnable		= Display::E_Bool_True;
	m_GhostColourStates.stencilState.m_StencilFunc		= Display::E_CmpFunc_Always;
	m_GhostColourStates.stencilState.m_StencilRef		= 0xFF;
	m_GhostColourStates.stencilState.m_StencilMask		= 0xFF;
	m_GhostColourStates.stencilState.m_StencilWriteMask	= 0xFF;
	m_GhostColourStates.stencilState.m_StencilFail		= Display::E_StencilOp_Keep;
	m_GhostColourStates.stencilState.m_StencilDepthFail	= Display::E_StencilOp_Keep;
	m_GhostColourStates.stencilState.m_StencilDepthPass	= Display::E_StencilOp_Replace;

	//Create Fake lighting shaders
	Display::CreateVertexShaderFromFunc("Shader/vsFakeEnvironmentLight", 0, &m_FakeLightingVertexShader, REF_SHADER(vsFakeEnvironmentLight));
	Display::CreatePixelShaderFromFunc("Shader/psFakeEnvironmentLight", 0, &m_FakeLightingPixelShader, REF_SHADER(psFakeEnvironmentLight));
}

void C_ViewManager::Destroy()
{
	for(S32 i = 0; i < m_Views.Size(); ++i)
	{
		m_Views[i]->Destroy();
		delete m_Views[i];
	}

	if(m_pCubeMap)
	{
		m_pCubeMap->Destroy();
		delete m_pCubeMap;
		m_pCubeMap = NULL;
	}

	if (m_pLightTrailsView)
	{
		GetSceneGraph()->RemoveView(m_pLightTrailsView);
		m_pLightTrailsView = 0;
	}

	m_Views.Clear();

	Display::Device()->Destroy(&m_FakeLightingVertexShader);
	Display::Device()->Destroy(&m_FakeLightingPixelShader);
}

void C_ViewManager::BeginFrame()
{
	if(m_pCubeMap)
	{
		// update cubemap to use same pvs cell as main view
		m_pCubeMap->UpdateView(*m_Views[0]);
		m_pCubeMap->Gather();
	}
}

void C_ViewManager::UpdateViews()
{
	for(S32 i = 0; i < m_Views.Size(); ++i)
		m_Views[i]->UpdateView();
}

extern GameWorldFxParams	g_FxParams;

void C_ViewManager::SetLightFlareParameters()
{
	const C_LightFlareSettings levelSettings = m_pGameWorld->GetLevelLightFlareSettings();
	const C_LightFlareSettings carSettings = m_pGameWorld->GetCarLightFlareSettings();

	FX::SetParameter(g_FxParams.globalLightFlareSize, Display::C_PSConstant(levelSettings.m_GlobalSize, levelSettings.m_GlobalSizeX, levelSettings.m_GlobalSizeY, 0.0f));
	FX::SetParameter(g_FxParams.globalLightFlareColour, Display::C_PSConstant(levelSettings.m_GlobalColour.R(), levelSettings.m_GlobalColour.G(), levelSettings.m_GlobalColour.B(), levelSettings.m_GlobalColour.A()));
	FX::SetParameter(g_FxParams.carGlobalLightFlareSize, Display::C_PSConstant(carSettings.m_GlobalSize, carSettings.m_GlobalSizeX, carSettings.m_GlobalSizeY, 0.0f));
	FX::SetParameter(g_FxParams.carGlobalLightFlareColour, Display::C_PSConstant(carSettings.m_GlobalColour.R(), carSettings.m_GlobalColour.G(), carSettings.m_GlobalColour.B(), carSettings.m_GlobalColour.A()));
	FX::SetParameter(g_FxParams.nearFarLightFlare, Display::C_PSConstant(levelSettings.m_NearFadeDistant, levelSettings.m_FarFadeDistant, levelSettings.m_SizeDistantFactor / 100.f, levelSettings.m_IntensityDistantFactor / 100.f));
}


Debug::Bool g_MainReduceRenderOverride	("Amax.Views.Main.ReduceRenderOverride", false);
Debug::Bool g_MainReduceRenderDynamicEnable	("GameSys.Lod.ReduceRenderAlphaDynamicEnable", true);
Debug::Float g_MainReduceRenderDetailThreshold	("GameSys.Lod.ReduceRenderAlphaDetailThreshold", 0.5f, Debug::Range(0.0f, 1.0f));
Debug::Enum<Render::Reduced::E_Downsample> g_MainReduceRenderAlpha	("Amax.Views.Main.ReduceRenderAlpha",	Render::Reduced::E_Downsample_None);

Debug::Float g_DisableAADetailThreshold ("GameSys.Lod.DisableAAThreshold", 0.4f, Debug::Range(0.0f, 1.0f));

#if PLATFORM_PC
// [Blur2PC] Retail PC view manager dynamic detail (Blur.exe 1.2 vtable 0x00e66a78 slot 1, 0x00426c80):
// views, the cube map's half-rate toggle and detail factor, and the blob shadow quality. The reduced
// alpha render and PS3 AA parts are gone on PC.
void C_ViewManager::SetDynamicDetail(const F32 cpuDetail, const F32 gpuDetail, const bool immediate)
{
	for(S32 i = 0; i < m_Views.Size(); ++i)
		m_Views[i]->SetDynamicDetail(gpuDetail);

	if(m_pCubeMap)
		m_pCubeMap->SetDynamicDetail(gpuDetail);

	m_pBlobShadows->SetQuality(gpuDetail);
}

// [Blur2PC] Retail PC graphical detail for the views (Blur.exe 1.2 0x0063ec40): every view, then the
// cube map, which only renders at detail 3 and 4.
void C_ViewManager::SetGraphicalDetail(const U32 graphicalDetail, const bool inUI)
{
	for(S32 i = 0; i < m_Views.Size(); ++i)
		m_Views[i]->SetGraphicalDetail(graphicalDetail, inUI);

	if(m_pCubeMap)
		m_pCubeMap->SetGraphicalDetailEnabled(graphicalDetail > 2);
}
#else
void C_ViewManager::SetDynamicDetail(const F32 cpuDetail, const F32 gpuDetail, const bool immediate)
{
	for(S32 i = 0; i < m_Views.Size(); ++i)
		m_Views[i]->SetDynamicDetail(gpuDetail);

	if(m_pCubeMap)
		m_pCubeMap->SetDynamicDetail(gpuDetail);

	if(!g_MainReduceRenderOverride && g_MainReduceRenderDynamicEnable)
	{
		// kick in reduced rendering if we drop below 0.5 gpu detail
		m_ReduceRenderAlphaCycle += C_GameApp::GetInstance()->GetTimer().GetFrameTime();
		Render::Reduced::E_Downsample reduceRenderAlpha = gpuDetail < g_MainReduceRenderDetailThreshold ? Render::Reduced::E_Downsample_Quarter : Render::Reduced::E_Downsample_None;

		if(m_ReduceRenderAlphaCycle > 1.0f || immediate)
		{
			m_ReduceRenderAlpha = reduceRenderAlpha;
			m_ReduceRenderAlphaCycle = 0.0f;
		}
	}
	else
		m_ReduceRenderAlpha = g_MainReduceRenderAlpha;

	//BlobShadows quality
	m_pBlobShadows->SetQuality(gpuDetail);

#ifdef PLATFORM_PS3
	m_DisableAACycle += C_GameApp::GetInstance()->GetTimer().GetFrameTime();
	bool disableAA = gpuDetail < g_DisableAADetailThreshold;

	if(m_DisableAACycle > 1.0f || immediate)
	{
		if(disableAA)
			Display::Device()->DisableAA();
		else
			Display::Device()->EnableAA();
		m_DisableAACycle = 0.0f;
	}
#endif
}
#endif

} // namespace Game
