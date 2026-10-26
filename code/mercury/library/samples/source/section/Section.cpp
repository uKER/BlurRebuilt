#include "StdAfx.h"
#include "Samples/Section/Section.h"

namespace Samples
{

// C_Section

C_Section::C_Section(const Util::C_String& name) : Display::I_Renderer(name)
{
}

C_Section::~C_Section()
{
}

bool C_Section::OnCreate()
{
	return true;
}

bool C_Section::OnCreateVolatile()
{
	return true;
}

void C_Section::OnDestroyVolatile()
{
}

void C_Section::OnDestroy()
{
}

void C_Section::OnInput(const C_Input& input, const C_Timer& timer)
{
}

void C_Section::OnUpdate(const C_Input& input, const C_Timer& timer)
{
}

void C_Section::OnRender(const C_Input& input, const C_Timer& timer)
{
}

void C_Section::OnPreRender(const C_Input& input, const C_Timer& timer)
{
}

void C_Section::OnPostRender(const C_Input& input, const C_Timer& timer)
{
}

void C_Section::OnRenderGameViewWithAA(const C_Input& input, const C_Timer& timer)
{
}

void C_Section::OnRenderGameViewWithoutAA(const C_Input& input, const C_Timer& timer)
{
}

void C_Section::OnRenderHudView(const C_Input& input, const C_Timer& timer)
{
}

void C_Section::OnBeginRenderSplitscreenWithoutAA()
{
}

void C_Section::OnEndRenderSplitscreenWithoutAA()
{
}

void C_Section::OnResize()
{
}

// C_SectionLauncherBase

C_SectionLauncherBase::C_SectionLauncherBase(const Util::C_String& name) :
	m_Name(name),
	m_pSection(NULL),
	m_pPrev(NULL),
	m_pNext(NULL) 
{
}

C_SectionLauncherBase::~C_SectionLauncherBase()
{
	assert(!m_pSection && "The section has not been destroyed");
}

void C_SectionLauncherBase::Create()
{
	RESOURCE_MEMORY_ID("Samples");

	assert(!m_pSection);
	m_pSection = CreateSection();
	assert(m_pSection);

	Display::Device()->CreateRenderers(Display::E_RendererType_Dynamic);
	Display::Device()->CreateRenderersVolatile(Display::E_RendererType_Dynamic);
}

void C_SectionLauncherBase::Destroy()
{
	if(m_pSection)
	{

		Display::Device()->DestroyRenderersVolatile(Display::E_RendererType_Dynamic);
		Display::Device()->DestroyRenderers(Display::E_RendererType_Dynamic);

		delete m_pSection;
		m_pSection = NULL;
	}
}

void C_SectionLauncherBase::Input(const C_Input& input, const C_Timer& timer)
{
	assert(m_pSection);

	if(m_pSection->IsRendererCreated())
	{
		m_pSection->OnInput(input, timer);
	}
}

void C_SectionLauncherBase::Update(const C_Input& input, const C_Timer& timer)
{
	assert(m_pSection);

	if(m_pSection->IsRendererCreated())
	{
		m_pSection->OnUpdate(input, timer);
	}
}

void C_SectionLauncherBase::PreRender(const C_Input& input, const C_Timer& timer)
{
	assert(m_pSection);
	if(m_pSection->IsRendererCreated())
	{
		m_pSection->OnPreRender(input, timer);
	}
}

void C_SectionLauncherBase::PostRender(const C_Input& input, const C_Timer& timer)
{
	assert(m_pSection);
	if(m_pSection->IsRendererCreated())
	{
		m_pSection->OnPostRender(input, timer);
	}
}

void C_SectionLauncherBase::RenderGameViewWithAA(const C_Input& input, const C_Timer& timer)
{
	assert(m_pSection);
	if(m_pSection->IsRendererCreated())
	{
		m_pSection->OnRender(input, timer);
		m_pSection->OnRenderGameViewWithAA(input, timer);
	}
}

void C_SectionLauncherBase::RenderGameViewWithoutAA(const C_Input& input, const C_Timer& timer)
{
	assert(m_pSection);
	if(m_pSection->IsRendererCreated())
	{
		m_pSection->OnRenderGameViewWithoutAA(input, timer);
	}
}

void C_SectionLauncherBase::RenderHudView(const C_Input& input, const C_Timer& timer)
{
	assert(m_pSection);
	if(m_pSection->IsRendererCreated())
	{
		m_pSection->OnRenderHudView(input, timer);
	}
	else
	{
		Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Display::BLACK);
		char temp[128];
		sprintf_s(temp, 128, "ERROR: Failed to create %s section\n", m_Name.Chars());
		Display::FontRender()->Draw(temp, 30, 50, 20, 20, Maths::RGBA(255, 0, 0, 255));
	}
}

void C_SectionLauncherBase::BeginRenderSplitscreenWithoutAA()
{
	assert(m_pSection);
	if(m_pSection->IsRendererCreated())
	{
		m_pSection->OnBeginRenderSplitscreenWithoutAA();
	}
}

void C_SectionLauncherBase::EndRenderSplitscreenWithoutAA()
{
	assert(m_pSection);
	if(m_pSection->IsRendererCreated())
	{
		m_pSection->OnEndRenderSplitscreenWithoutAA();
	}
}

void C_SectionLauncherBase::Resize()
{
	assert(m_pSection);
	// don't do check since renderer will be in an incomplete state
	m_pSection->OnResize();
}

// C_SectionManager

C_SectionManager::C_SectionManager() : Display::I_Renderer("SectionManager"),
	m_pCurrLauncher(NULL),
	m_pHeadLauncher(NULL),
	m_pTailLauncher(NULL)
{
}

C_SectionManager::~C_SectionManager()
{
	C_SectionLauncherBase* pTempLauncher = m_pHeadLauncher;

	while(pTempLauncher)
	{
		C_SectionLauncherBase* pLauncher = pTempLauncher;
		pTempLauncher = pTempLauncher->m_pNext;
		pLauncher->Destroy();
		delete pLauncher;
	}

	m_pCurrLauncher = m_pHeadLauncher = m_pTailLauncher = NULL;
}

bool C_SectionManager::OnCreate()
{
	RESOURCE_MEMORY_ID("Samples");

	Display::CreateTexture2DFromFile("Textures/Section.dds", &m_Texture);

	return true;
}

void C_SectionManager::OnDestroy()
{
	Display::Device()->Destroy(&m_Texture);
}

void C_SectionManager::AddSection(C_SectionLauncherBase* pLauncher)
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

void C_SectionManager::InputSection(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("InputSection");

	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->Input(input, timer);
	}
}

void C_SectionManager::UpdateSection(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("UpdateSection");

	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->Update(input, timer);
	}
}

void C_SectionManager::PreRenderSection(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("PreRenderSection");

	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->PreRender(input, timer);
	}
}

void C_SectionManager::PostRenderSection(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("PostRenderSection");

	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->PostRender(input, timer);
	}
}

void C_SectionManager::GameViewWithAARenderSection(const C_Input& input, const C_Timer& timer)
{
	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->RenderGameViewWithAA(input, timer);
	}
}

void C_SectionManager::GameViewWithoutAARenderSection(const C_Input& input, const C_Timer& timer)
{
	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->RenderGameViewWithoutAA(input, timer);
	}
}

void C_SectionManager::HudViewRenderSection(const C_Input& input, const C_Timer& timer)
{
	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->RenderHudView(input, timer);
	}
	else
	{
		Display::Device()->SetPresentInterval(Display::E_PresentInterval_Default);
		Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));
		
		if(m_Texture.IsCreated())
		{
			Display::BasicRender()->DrawTextured2DRect(0.f, 0.f, 1024.f, 768.f, Maths::RGBA(255, 255, 255, 255), &m_Texture);
		}

		F32 x = 50.f, y = 50.f, w = 30.f, h = 30.f;
		Display::FontRender()->Draw("SECTIONS:", x, y, w, h, Maths::RGBA(255, 255, 255, 255));
		y += h * 3.f;

		C_SectionLauncherBase* pTempLauncher = m_pHeadLauncher;
		while(pTempLauncher)
		{
			static char text[128];
			sprintf_s(text, 128, "- %s", pTempLauncher->GetName().Chars());
			Display::FontRender()->Draw(text, x, y, w, h, pTempLauncher == m_pCurrLauncher ? Maths::RGBA(0, 255, 0, 255) : Maths::RGBA(255, 255, 255, 255));
			y += h * 2.f;

			pTempLauncher = pTempLauncher->m_pNext;
		}
	}
}

void C_SectionManager::BeginRenderSplitscreenWithoutAA()
{
	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->BeginRenderSplitscreenWithoutAA();
	}
}

void C_SectionManager::EndRenderSplitscreenWithoutAA()
{
	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->EndRenderSplitscreenWithoutAA();
	}
}

void C_SectionManager::Resize()
{
	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->Resize();
	}
}

void C_SectionManager::EnterSection()
{
	if(m_pCurrLauncher && !m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->Create();
	}
}

void C_SectionManager::ExitSection()
{
	if(m_pCurrLauncher && m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher->Destroy();
	}
}

void C_SectionManager::PrevSection()
{
	if(m_pCurrLauncher && !m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher = m_pCurrLauncher->m_pPrev ? m_pCurrLauncher->m_pPrev : m_pTailLauncher;
	}
}

void C_SectionManager::NextSection()
{
	if(m_pCurrLauncher && !m_pCurrLauncher->IsActive())
	{
		m_pCurrLauncher = m_pCurrLauncher->m_pNext ? m_pCurrLauncher->m_pNext : m_pHeadLauncher;
	}
}

}; // namespace Samples
