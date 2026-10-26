#include "stdafx.h"
#include "TestSections/TestSectionLauncher.h"

namespace Framework
{

C_TestSectionLauncherBase::C_TestSectionLauncherBase(const Util::C_String& name) :
	m_Name(name),
	m_pPrev(NULL),
	m_pNext(NULL),
	m_pSection(NULL)
{
}

C_TestSectionLauncherBase::~C_TestSectionLauncherBase()
{
	assert(!m_pSection && "The section has not been destroyed");
}

const Util::C_String& C_TestSectionLauncherBase::GetName() const
{
	return m_Name;
}

bool C_TestSectionLauncherBase::Create()
{
	bool success = true;

	if(!m_pSection)
	{
		m_pSection = CreateSection();

		if(m_pSection->Create())
		{
			if(m_pSection->CreateVolatile())
			{
				Display::Device()->CreateRenderers();
				Display::Device()->CreateRenderersVolatile();
			}
			else
			{
				success = false;

				m_pSection->DestroyVolatile();
				m_pSection->Destroy();

				SAFE_DELETE_PTR(m_pSection);
			}
		}
		else
		{
			success = false;

			m_pSection->Destroy();

			SAFE_DELETE_PTR(m_pSection);
		}
	}

	return success;
}

void C_TestSectionLauncherBase::Destroy()
{
	if(m_pSection)
	{
		m_pSection->DestroyVolatile();
		m_pSection->Destroy();

		SAFE_DELETE_PTR(m_pSection);
	}
}

void C_TestSectionLauncherBase::Update()
{
	assert(m_pSection);
	m_pSection->OnUpdate();
}

void C_TestSectionLauncherBase::PreRender()
{
	assert(m_pSection);
	m_pSection->OnPreRender();
}

void C_TestSectionLauncherBase::Render3DViewWithAA(const S32 viewIndex)
{
	assert(m_pSection);
	m_pSection->OnRender3DViewWithAA(viewIndex);
}

void C_TestSectionLauncherBase::Render3DViewWithoutAA(const S32 viewIndex)
{
	assert(m_pSection);
	m_pSection->OnRender3DViewWithoutAA(viewIndex);
}

void C_TestSectionLauncherBase::Render2DView()
{
	assert(m_pSection);
	m_pSection->OnRender2DView();
}

void C_TestSectionLauncherBase::PostRender()
{
	assert(m_pSection);
	m_pSection->OnPostRender();
}

void C_TestSectionLauncherBase::BeginRenderSplitscreenWithoutAA()
{
	assert(m_pSection);
	m_pSection->OnBeginRenderSplitscreenWithoutAA();
}

void C_TestSectionLauncherBase::EndRenderSplitscreenWithoutAA()
{
	assert(m_pSection);
	m_pSection->OnEndRenderSplitscreenWithoutAA();
}

void C_TestSectionLauncherBase::Resize()
{
	assert(m_pSection);
	m_pSection->OnResize();
}

void C_TestSectionLauncherBase::SetupUserControls(const S32 viewIndex)
{
	assert(m_pSection);
	m_pSection->OnSetupUserControls(viewIndex);
}

void C_TestSectionLauncherBase::UpdateUserControls(const S32 viewIndex)
{
	assert(m_pSection);
	m_pSection->OnUpdateUserControls(viewIndex);
}

const bool C_TestSectionLauncherBase::IsActive() const
{
	return m_pSection ? true : false;
}

}; // namespace Framework
