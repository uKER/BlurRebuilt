#include "stdafx.h"
#include "TestSections/TestSectionManager.h"
#include <Display/Render/BasicRender.h>
#include <Display/Render/FontRender.h>
#include <Render/Debug/DebugRender.h>
#include <Render/font/Font.h>
#include <DebugSystems/DebugHUDMenu.h>

namespace Framework
{

//////////////////////////////////////////////////////////////////////////
//c_QuitSectionMenuItem
//////////////////////////////////////////////////////////////////////////
class c_QuitSectionMenuItem : public DebugSystems::C_MenuHUDItem
{
public:
	c_QuitSectionMenuItem(C_TestSectionManager* pTestSectionManager): DebugSystems::C_MenuHUDItem("Quit Section")
	{
		m_pTestSectionManager = pTestSectionManager;
	};

	virtual bool Forwards()
	{

		m_pTestSectionManager->ExitSection();

		return false;
	};

private:
	C_TestSectionManager* m_pTestSectionManager;
};


C_TestSectionManager::C_TestSectionManager() :
	m_pCurrLauncher(NULL),
	m_pHeadLauncher(NULL),
	m_pTailLauncher(NULL),
	m_pFontRenderer(NULL),
	m_pQuitSectionMenuItem(NULL)
{
	m_ErrorStatus = "";
}

C_TestSectionManager::~C_TestSectionManager()
{
	C_TestSectionLauncherBase* pTempLauncher = m_pHeadLauncher;
	while(pTempLauncher)
	{
		C_TestSectionLauncherBase* pLauncher = pTempLauncher;
		pTempLauncher = pTempLauncher->m_pNext;
		SAFE_DELETE_PTR(pLauncher);
	}

	m_pCurrLauncher = m_pHeadLauncher = m_pTailLauncher = NULL;
}

bool C_TestSectionManager::Create()
{
#ifndef CFG_GOLD
	if(!Display::CreateTexture2DFromFile("Textures/Section.dds", &m_Texture))
		Display::CreateTexture2DFromFile("DontShip/Textures/Section.dds", &m_Texture);
	
	m_pFontRenderer = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");
	assert(m_pFontRenderer);
	if (m_pFontRenderer)
	{
		m_pFontRenderer->SetScaleX(30);
		m_pFontRenderer->SetScaleY(30);
		m_pFontRenderer->SetShadowEnable( true );
		m_pFontRenderer->SetColourOutline(Maths::RGBA(0,0,25,255));
		m_pFontRenderer->SetOutlineSize(0.35f);
	}
#endif

	return true;
}

void C_TestSectionManager::Destroy()
{
	Display::Device()->Destroy(&m_Texture);

	C_TestSectionLauncherBase* pTempLauncher = m_pHeadLauncher;
	while(pTempLauncher)
	{
		pTempLauncher->Destroy();
		pTempLauncher = pTempLauncher->m_pNext;
	}

	SAFE_DELETE_PTR(m_pFontRenderer);

	//Remove quit section
	if (m_pQuitSectionMenuItem)
	{
		DebugSystems::C_DebugHUDMenu::GetInstance()->RemoveItem(m_pQuitSectionMenuItem);
		SAFE_DELETE_PTR(m_pQuitSectionMenuItem);
	}
}

void C_TestSectionManager::AddSection(C_TestSectionLauncherBase* pLauncher)
{
	assert(pLauncher);
	
	if(!m_pHeadLauncher)
	{
		m_pCurrLauncher = m_pHeadLauncher = pLauncher;
	}
	
	if(m_pTailLauncher)
	{
		m_pTailLauncher->m_pNext = pLauncher;
		pLauncher->m_pPrev = m_pTailLauncher;
	}
	
	m_pTailLauncher = pLauncher;
}

void C_TestSectionManager::Update()
{
	static int oo = 0;
	oo++;
	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->Update();
	}
}

void C_TestSectionManager::PreRender()
{
	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->PreRender();
	}
}

void C_TestSectionManager::PostRender()
{
	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->PostRender();
	}
}

void C_TestSectionManager::Render3DViewWithAA(const S32 viewIndex)
{
	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->Render3DViewWithAA(viewIndex);

		if(DbUtil::DefaultDebugRenderSys())
			DbUtil::DefaultDebugRenderSys()->RenderDeferred();
	}
}

void C_TestSectionManager::Render3DViewWithoutAA(const S32 viewIndex)
{
	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->Render3DViewWithoutAA(viewIndex);

		if(DbUtil::DefaultDebugRenderSys())
			DbUtil::DefaultDebugRenderSys()->RenderDeferred();
	}
}

void C_TestSectionManager::Render2DView()
{
	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->Render2DView();
	}
	else
	{
		Display::Device()->SetPresentInterval(Display::E_PresentInterval_Two);

		Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 0, 0, 255));
		
		if(m_Texture.IsCreated())
		{
			Display::BasicRender()->DrawTextured2DRect(0.f, 0.f, 1024.f, 768.f, Maths::RGBA(255, 255, 255, 255), &m_Texture);
		}

		F32 x = 50.f, y = 50.f, w = 30.f, h = 30.f;
		m_pFontRenderer->SetScaleX(w);
		m_pFontRenderer->SetScaleY(h);
		m_pFontRenderer->SetColourTop(Maths::RGBA(255, 255, 255, 255));
		m_pFontRenderer->SetColourBottom(Maths::RGBA(150, 150, 150, 255));
		m_pFontRenderer->Print(Maths::HVec2(x,y), "TEST SECTIONS:");
		y += h * 3.f;
		
		C_TestSectionLauncherBase* pTempLauncher = m_pHeadLauncher;
		while(pTempLauncher)
		{
			char text[128];
			sprintf_s(text, 128, "- %s", pTempLauncher->GetName().Chars());
			if (pTempLauncher == m_pCurrLauncher)
			{
				m_pFontRenderer->SetColourTop(Maths::RGBA(0, 15, 0, 255));
				m_pFontRenderer->SetColourBottom(Maths::RGBA(0, 50, 0, 255));

				Maths::HVec2 rect = m_pFontRenderer->GetDimensions(1000, text);
				
				m_pFontRenderer->SetShadowEnable( false );

				Render::Debug::Immediate()->Draw2dBar(Maths::HVec2(x,y), rect, Maths::RGBA(200, 200, 200, 255));
			}
			else
			{
				m_pFontRenderer->SetColourTop(Maths::RGBA(255, 255, 255, 255));
				m_pFontRenderer->SetColourBottom(Maths::RGBA(150, 150, 150, 255));
				m_pFontRenderer->SetShadowEnable( true );
			}
			m_pFontRenderer->Print(Maths::HVec2(x,y), text);
			y += h * 2.f;
			
			pTempLauncher = pTempLauncher->m_pNext;
		}


		m_pFontRenderer->SetColourTop(Maths::RGBA(255, 255, 255, 255));
		m_pFontRenderer->SetColourBottom(Maths::RGBA(150, 150, 150, 255));
		m_pFontRenderer->SetShadowEnable( true );

		y += h;
		m_pFontRenderer->Print(Maths::HVec2(x,y), "Press A to Select");

		if(m_ErrorStatus != "")
		{
			y += h * 3;
			m_pFontRenderer->Print(Maths::HVec2(x,y), m_ErrorStatus.Chars());
		}
	}
}

void C_TestSectionManager::BeginRenderSplitscreenWithoutAA()
{
	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->BeginRenderSplitscreenWithoutAA();
	}
}

void C_TestSectionManager::EndRenderSplitscreenWithoutAA()
{
	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->EndRenderSplitscreenWithoutAA();
	}
}

void C_TestSectionManager::Resize()
{
	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->Resize();
	}
}

void C_TestSectionManager::SetupUserControls(const S32 viewIndex)
{
	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->SetupUserControls(viewIndex);
	}
}

void C_TestSectionManager::UpdateUserControls(const S32 viewIndex)
{
	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->UpdateUserControls(viewIndex);
	}
}

bool C_TestSectionManager::EnterSection()
{
	if(m_pCurrLauncher && !m_pCurrLauncher->IsActive())
	{
		if(m_pCurrLauncher->Create())
		{
			m_ErrorStatus = "";
		}
		else
		{
			m_ErrorStatus.Printf("ERROR: Failed to create test section %s", m_pCurrLauncher->GetName().Chars());
		}

		//Add quit section to debug menu
		if (DebugSystems::C_DebugHUDMenu::GetInstance())
		{
			if (m_pQuitSectionMenuItem == NULL)
				m_pQuitSectionMenuItem = new c_QuitSectionMenuItem(this);
			DebugSystems::C_DebugHUDMenu::GetInstance()->Root()->AddChild(m_pQuitSectionMenuItem);
		}
		
		return true;
	}
	else
	{
		return false;
	}
}

void C_TestSectionManager::ExitSection()
{
	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->Destroy();

		if (DebugSystems::C_DebugHUDMenu::GetInstance())
		{
			//Remove quit section menu item
			DebugSystems::C_DebugHUDMenu::GetInstance()->RemoveItem(m_pQuitSectionMenuItem);
		}
	}
}

void C_TestSectionManager::PrevSection()
{
	if(m_pCurrLauncher && !m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher = m_pCurrLauncher->m_pPrev ? m_pCurrLauncher->m_pPrev : m_pTailLauncher;
	}
}

void C_TestSectionManager::NextSection()
{
	if(m_pCurrLauncher && !m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher = m_pCurrLauncher->m_pNext ? m_pCurrLauncher->m_pNext : m_pHeadLauncher;
	}
}

void C_TestSectionManager::SelectSection(const Util::C_String& name)
{
	C_TestSectionLauncherBase* pTempLauncher = m_pHeadLauncher;
	while(pTempLauncher)
	{
		if(name == pTempLauncher->GetName())
		{
			m_pCurrLauncher = pTempLauncher;
			return;
		}

		pTempLauncher = pTempLauncher->m_pNext;
	}
}

const bool C_TestSectionManager::InSection() const
{
	return m_pCurrLauncher && m_pCurrLauncher->IsActive() ? true : false;
}

}; // namespace Framework
