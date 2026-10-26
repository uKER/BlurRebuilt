#include "StdAfx.h"
#include "Samples/Section/SectionLightEffect.h"

#include "System/Misc/Random.h"

// debug
#include "Render/Debug/DebugRender.h"
#include "Render/Debug/ImmediateDebugRender.h"
#include "Render/Debug/Charts/ChartRenderer.h"
#include "Render/Debug/Charts/LineChart.h"
#include "Render/Shaders/Constants.h"

// basic rendering
#include "Render/Utils/Helper.h"
#include "Render/Render/GridRender.h"

#include "Render/Occlusion/Query.h"
#include "Render/Occlusion/Pool.h"

#include "Dynamic/Common.h"
#include "Dynamic/Renderer.h"
#include "Dynamic/TypeIndex.h"
#include "Dynamic/Light/Enums.h"
#include "Dynamic/Light/Light.h"
#include "Dynamic/Light/Manager.h"
#include "Dynamic/Light/EffectInterface.h"

#include "Dynamic/Light/Generic/XtInterface.h"


// debug
#include "Render/Debug/DebugRender.h"
#include "Render/Debug/ImmediateDebugRender.h"



#define USE_POST_EFFECTS
#define USE_POST_EFFECTS_HDR

namespace Samples
{

Debug::Float g_debugLightExpiryTime("Samples.Section.LightEffect.ExpiryTime", 0.5f, Debug::Range(0.0f, 4.0f));

C_SectionLightEffect::C_SectionLightEffect(const Util::C_String& name) : C_Section(name)
{
	Util::C_Random random;

	#ifdef USE_POST_EFFECTS
	m_pPostProcessMgr	= new PostProcess::C_Manager();
	#ifdef USE_POST_EFFECTS_HDR
	m_pPostProcessHdr	= new PostProcess::C_HighDynamicRange();
	#else
	m_pPostProcessHdr	= NULL;
	#endif
	#else
	m_pPostProcessMgr	= NULL;
	m_pPostProcessHdr	= NULL;
	#endif

	if(m_pPostProcessHdr)
	{
		m_pPostProcessHdr->SetType(PostProcess::E_Type_Final);

			m_pPostProcessHdr->SetHighDynamicRangeType(PostProcess::E_HighDynamicRangeType_ShaderU32);
	}

	m_pDynamicEffectRenderer = &DynamicEffect::C_Renderer::Instance();

	if(!Render::Debug::g_pDebugRenderSys)
	{
		m_pDebugRenderSys = new Render::Debug::C_DebugRenderSys;
		DbUtil::DefaultDebugRenderSys(m_pDebugRenderSys);
	}
	else
		m_pDebugRenderSys = NULL;

	m_pLightManager = new DynamicEffect::Light::C_Manager(m_pDynamicEffectRenderer, DynamicEffect::E_ManagerTypeIndex_Light);
	m_pLightManager->ReserveLights(256);

	DynamicEffect::XtInterface::Initialise(m_pDynamicEffectRenderer);
	DynamicEffect::Light::XtInterface::Initialise(m_pLightManager);
			
/*	for (U32 y = 0; y < 8; ++y)
	{
		for (U32 x = 0; x < 8; ++x)
		{
			C_TestLight light;
			light.m_vPosition = MathsStorage::Float3A16(x * 16.0f - 64.0f, 8.0f, y * 16.0f - 64.0f);
			light.m_fExpiryTime = 0.0f;
			light.m_vRotation = MathsStorage::Float4A16(1.0f, 0.0f, 0.0f, 1.0f);
			light.m_vColour = MathsStorage::Float4A16(random.FRand(1.0f), random.FRand(1.0f), random.FRand(1.0f), 1.0f);
			
			DynamicEffect::Light::LightDefinition* pDefinition = DynamicEffect::Light::XtInterface::FindLight("LampPost");

			Maths::HVec3 vPosition = Maths::Load<Maths::HVec3>(&light.m_vPosition);
			Maths::C_Quaternion qRotation = Maths::Load<Maths::C_Quaternion>(&light.m_vRotation);
			Maths::C_QuatTrans transform(qRotation, vPosition);

			m_pLightManager->GetSizeOfLight(pDefinition, transform, &light.m_vCentre, &light.m_vSize);
			
			m_TestLights.PushBack(light);
		}
	}
*/

	SetupPostProcess();
}

C_SectionLightEffect::~C_SectionLightEffect()
{
	DynamicEffect::Light::XtInterface::Shutdown();

	SAFE_DELETE_PTR(m_pPostProcessMgr);
	SAFE_DELETE_PTR(m_pDebugRenderSys);

	SAFE_DELETE_PTR(m_pDynamicEffectRenderer);
}

void C_SectionLightEffect::OnResize()
{
	if(!m_pPostProcessMgr)
		return;

	m_pPostProcessMgr->DestroyVolatile();

	m_pPostProcessMgr->ClearViews();

	SetupPostProcess();
	
	m_pPostProcessMgr->CreateVolatile();
}

bool C_SectionLightEffect::OnCreate()
{
	if (!m_OcclusionQueryRender.Create())
		return false;

	return true;
}

bool C_SectionLightEffect::OnCreateVolatile()
{
	return true;
}

void C_SectionLightEffect::OnDestroyVolatile()
{
}

void C_SectionLightEffect::OnDestroy()
{
	m_OcclusionQueryRender.Destroy();
}

void C_SectionLightEffect::OnUpdate(const C_Input& input, const C_Timer& timer)
{
	const S32 viewCount = Display::Device()->GetViewCount();

	DynamicEffect::Light::LightDefinition* pDefinition = DynamicEffect::Light::XtInterface::FindLight("Sun");
	Maths::C_QuatTrans transform(Maths::C_Quaternion(0.0f, 0.0f, 0.0f, 1.0f), Maths::HVec3(0.0f, 0.0f, 0.0f));
	if (pDefinition && !m_hSunLight)
	{
		m_hSunLight = m_pLightManager->CreateLight(pDefinition, transform, &m_SunNode, Maths::RGBA(0xFFFFFFFF));
	}

/*	for (S32 lightNo = 0; lightNo < m_TestLights.Size(); ++lightNo)
	{
		m_TestLights[lightNo].m_iVisibility = 0;
		for (S32 viewNo = 0; viewNo < viewCount; ++viewNo)
		{
			const Display::C_View* pView = Display::Device()->GetView(viewNo);
			if (pView->m_UserIndex < 0)
				continue;
	
			const Display::C_ViewFrustum& viewFrustum = pView->GetViewFrustum();
			
			const Maths::HVec4 vCentre = Maths::Load<Maths::HVec4>(&m_TestLights[lightNo].m_vCentre);
			const Maths::HVec4 vSize = Maths::Load<Maths::HVec4>(&m_TestLights[lightNo].m_vSize);
			const Maths::HVec3 vSize3 = Maths::HVec3(vSize);
			const Maths::HF32 vLength = vSize3.Length();

			const Display::C_CullResult cullResult = viewFrustum.CullSphere(vCentre, vLength, vSize.W());
			if (cullResult.Visible())
				m_TestLights[lightNo].m_iVisibility |= (1 << pView->m_UserIndex);
		}
	}

	for (S32 lightNo = 0; lightNo < m_TestLights.Size(); ++lightNo)
	{
		if (m_TestLights[lightNo].m_iVisibility)
		{
			if (!m_TestLights[lightNo].m_hLight)
			{
				DynamicEffect::Light::LightDefinition* pDefinition = DynamicEffect::Light::XtInterface::FindLight("LampPost");
				if (pDefinition)
				{
					const Maths::C_Quaternion qRotation;
					const Maths::HVec3 vPosition = Maths::Load<Maths::HVec3>(&m_TestLights[lightNo].m_vPosition);
					m_TestLights[lightNo].m_hLight = m_pLightManager->CreateLight(pDefinition, Maths::C_QuatTrans(qRotation, vPosition), 0, Maths::RGBA(0xFFFFFFFF));
				}
			}
			m_TestLights[lightNo].m_fExpiryTime = timer.GetTime() + g_debugLightExpiryTime;
		}
		else
		{
			if (m_TestLights[lightNo].m_hLight && m_TestLights[lightNo].m_fExpiryTime < timer.GetTime())
				m_pLightManager->DestroyLight(m_TestLights[lightNo].m_hLight);
		}
	}
*/
	DynamicEffect::Light::XtInterface::Update();
	m_pDynamicEffectRenderer->Update(timer.GetTime(), timer.GetFrameTime(), timer.IsPaused());
}

void C_SectionLightEffect::OnPreRender(const C_Input& input, const C_Timer& timer)
{
	m_pDynamicEffectRenderer->BeginFrame();
}

void C_SectionLightEffect::OnRenderGameViewWithAA(const C_Input& input, const C_Timer& timer)
{
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));
	
	m_pDynamicEffectRenderer->BeginView();

	Render::BeginRenderSplitscreenWithoutAAtates();

	Display::BasicRender()->DrawColoured3DBox(Maths::HVec3(7.f, 5.f, 8.f), Maths::HVec3(5.f, 2.f, 1.0f), Display::RED);
	Display::BasicRender()->DrawColoured3DBox(Maths::HVec3(-2.f, 1.f, -7.f), Maths::HVec3(4.f, 3.f, 3.0f), Display::GREEN);

	m_SkyDome.Render();

	Render::DrawGrid();

	m_pDynamicEffectRenderer->Build(Display::Device()->GetView()->m_Index);
	m_pDynamicEffectRenderer->Display(DynamicEffect::E_RenderPass_Opaque);

	Render::EndRenderSplitscreenWithoutAAtates();

	Util::C_String str;
	str.Printf("Post Effects Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

void C_SectionLightEffect::OnRenderGameViewWithoutAA(const C_Input& input, const C_Timer& timer)
{
	// calculate sun position in view space
	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();
	const Maths::HMat43 mView = viewSettings3D.GetWorldToCamera();
	const Maths::HMat43 mCamera = viewSettings3D.GetCameraToWorld();
	Maths::HVec3 vViewDirection = Maths::C_Normal(0.0f, 1.0f, 1.0f) * Maths::HMat33(mView);

	Maths::HF32  vFarClip(viewSettings3D.GetFarClip() - 0.1f);
	Maths::HVec3 vPosition(Maths::HVec3(vViewDirection) * vFarClip);
	vPosition = vPosition * mCamera; // world position
	Maths::HVec3 vDirection = Maths::C_Normal(0.0f, 1.0f, 1.0f);
	Maths::HMat33 mRotation = Maths::HMat33(Maths::AxisZ(Maths::C_Normal(vDirection)));
	Maths::C_Quaternion qRotation(mRotation);

	m_SunNode.SetPosition(vPosition);
	m_SunNode.SetRotation(qRotation);

	m_pLightManager->Render();

	m_pDynamicEffectRenderer->BeginView();
	m_pDynamicEffectRenderer->Display(DynamicEffect::E_RenderPass_TwoDimensional | DynamicEffect::E_RenderPass_Transparent | DynamicEffect::E_RenderPass_Debug);
}

void C_SectionLightEffect::OnBeginRenderSplitscreenWithoutAA()
{
	if(!m_pPostProcessMgr)
		return;

	const Display::C_View* pView = Display::Device()->GetView();
	m_pPostProcessMgr->Begin(pView->GetIndex(), pView->GetViewport());

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 160, 0, 255));
}

void C_SectionLightEffect::OnEndRenderSplitscreenWithoutAA()
{
	if(!m_pPostProcessMgr)
		return;

	if(m_pPostProcessHdr)
		m_pPostProcessHdr->Display();

	m_pPostProcessMgr->End();

	DbUtil::DefaultDebugRenderSys()->RenderDeferred();
}

void C_SectionLightEffect::SetupPostProcess()
{
	if(!m_pPostProcessMgr)
		return;

	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();

	const S32 viewCount = Display::Device()->GetViewCount();
	for(S32 viewIndex=0; viewIndex<viewCount; viewIndex++)
	{
		const Display::C_View* pView = Display::Device()->GetView(viewIndex);
		if(pView->GetUserIndex() != -1)
		{
			const Display::C_Viewport& viewport = pView->GetViewport();

			if(!m_pPostProcessHdr || m_pPostProcessHdr->GetType() == PostProcess::E_HighDynamicRangeType_ShaderU32)
			{
				m_pPostProcessMgr->AddView(
					viewport.GetW(),
					viewport.GetH(),
					pView->GetIndex(),
					PostProcess::E_RenderTargetFormat_RGBA_U8,
					Display::E_Multisample_None,
					5);
			}			
			else
			{
				m_pPostProcessMgr->AddView(
					viewport.GetW(),
					viewport.GetH(),
					pView->GetIndex(),
					PostProcess::E_RenderTargetFormat_RGBA_F10,
					Display::E_Multisample_None,
					5);
			}
		}
	}
}

} // namespace Samples
