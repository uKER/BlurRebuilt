#include "stdafx.h"
#include "TestSections/TestSectionLayer.h"
#include "Framework/Application/UserControls.h"
#include "Framework/Application/ApplicationBase.h"
#include "Framework/Application/TimingColours.h"
#include <camera2.h>

namespace Framework
{

C_TestSectionLayer::C_TestSectionLayer() :
	m_StraightToTestSection(false)
{
	m_pTestSectionManager = new C_TestSectionManager();
}

C_TestSectionLayer::~C_TestSectionLayer()
{
	SAFE_DELETE_PTR(m_pTestSectionManager);
}

HRESULT C_TestSectionLayer::Create()
{
	if(!m_pTestSectionManager->Create())
		return E_FAIL;

	return S_OK;
}

void C_TestSectionLayer::Destroy()
{
	m_pTestSectionManager->Destroy();
}

void C_TestSectionLayer::Update()
{
	if(Input::GetGameInput(0)->GetButtonAutoRepeat(E_Button_MenuUp, 10, 5))
	{
		m_pTestSectionManager->PrevSection();
	}
	else
	if(Input::GetGameInput(0)->GetButtonAutoRepeat(E_Button_MenuDown, 10, 5))
	{
		m_pTestSectionManager->NextSection();
	}
#ifdef USER_LLWRIGHT
	else
	if(Input::GetGameInput(0)->GetButtonAutoRepeat(E_Button_MenuCancel, 10, 5))
	{

		m_pTestSectionManager->ExitSection();
	}
#endif
	/*else
	if(Input::GetGameInput(0)->GetButtonAutoRepeat(E_Button_DebugMenuActivate, 10, 5))
	{
		#ifdef PLATFORM_XENON
		Display::Device()->BlockUntilIdle();
		#endif

		m_pTestSectionManager->ExitSection();
	}*/
	else
	if(Input::GetGameInput(0)->GetButtonAutoRepeat(E_Button_MenuConfirm, 10, 5) || m_StraightToTestSection)
	{

		if(m_pTestSectionManager->EnterSection())
		{
			for(S32 viewIndex=0; viewIndex<C_ApplicationBase::GetInstance()->GetSplitscreenViewCount(); viewIndex++)
			{
				m_pTestSectionManager->SetupUserControls(viewIndex);
			}
		}

		m_StraightToTestSection = false;
	}

	m_pTestSectionManager->Update();

	for(S32 viewIndex=0; viewIndex<C_ApplicationBase::GetInstance()->GetSplitscreenViewCount(); viewIndex++)
	{
		m_pTestSectionManager->UpdateUserControls(viewIndex);
	}
}

void C_TestSectionLayer::PreRender()
{
	m_pTestSectionManager->PreRender();
}

void C_TestSectionLayer::RenderSplitscreenWithAA()
{
	GPU_TIMING(2, g_TimingColour_RenderSplitscreenWithAA, "GPU.RenderSplitscreenWithAA");

	const S32 viewIndex = C_ApplicationBase::GetInstance()->GetCurrentSplitscreenView();
	
	m_pTestSectionManager->Render3DViewWithAA(viewIndex);
}

void C_TestSectionLayer::RenderSplitscreenWithoutAA()
{
	GPU_TIMING(2, g_TimingColour_RenderSplitscreenWithoutAA, "GPU.RenderSplitscreenWithoutAA");

	const S32 viewIndex = C_ApplicationBase::GetInstance()->GetCurrentSplitscreenView();
	
	m_pTestSectionManager->Render3DViewWithoutAA(viewIndex);
}

void C_TestSectionLayer::RenderFullscreen()
{
	GPU_TIMING(2, g_TimingColour_RenderFullscreen, "GPU.RenderFullscreen");

	m_pTestSectionManager->Render2DView();
}

void C_TestSectionLayer::PostRender()
{
	m_pTestSectionManager->PostRender();
}

void C_TestSectionLayer::BeginRenderSplitscreenWithoutAA()
{
	m_pTestSectionManager->BeginRenderSplitscreenWithoutAA();
}

void C_TestSectionLayer::EndRenderSplitscreenWithoutAA()
{
	m_pTestSectionManager->EndRenderSplitscreenWithoutAA();
}

void C_TestSectionLayer::StraightToTestSection(const Util::C_String& testSectionName)
{
	m_StraightToTestSection = true;

	m_pTestSectionManager->SelectSection(testSectionName);
}

HRESULT C_TestSectionLayer::ProcessMessage(const E_LayerMessage message)
{
	HRESULT hr = S_OK;

	switch(message)
	{
	case E_LayerMessage_Create:
		hr = Create();
		break;
	case E_LayerMessage_Destroy:
		Destroy();
		break;
	case E_LayerMessage_Update:
		Update();
		break;
	case E_LayerMessage_BeginFrame:
		PreRender();
		break;
	case E_LayerMessage_RenderSplitscreenWithAA:
		RenderSplitscreenWithAA();
		break;
	case E_LayerMessage_RenderSplitscreenWithoutAA:
		RenderSplitscreenWithoutAA();
		break;
	case E_LayerMessage_RenderFullscreen:
		RenderFullscreen();
		break;
	case E_LayerMessage_EndFrame:
		PostRender();
		break;
	case E_LayerMessage_BeginRenderSplitscreenWithoutAA:
		BeginRenderSplitscreenWithoutAA();
		break;
	case E_LayerMessage_EndRenderSplitscreenWithoutAA:
		EndRenderSplitscreenWithoutAA();
		break;
	}

	return hr;
}

}; // namespace Framework
