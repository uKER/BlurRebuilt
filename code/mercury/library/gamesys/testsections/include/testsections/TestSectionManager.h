#ifndef TEST_SECTIONS_TEST_SECTION_MANAGER_H
#define TEST_SECTIONS_TEST_SECTION_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "TestSections/TestSectionLauncher.h"

namespace Render{ namespace Font { class C_FontRenderer;}}


namespace Framework
{

class c_QuitSectionMenuItem;

//! C_TestSectionManager

class C_TestSectionManager
{
public:
	
	C_TestSectionManager();
	~C_TestSectionManager();

	void AddSection(C_TestSectionLauncherBase* pLauncher);

	bool Create();
	void Destroy();

	void Update();
	void PreRender();
	void PostRender();
	void Render3DViewWithAA(const S32 viewIndex);
	void Render3DViewWithoutAA(const S32 viewIndex);
	void Render2DView();
	void BeginRenderSplitscreenWithoutAA();
	void EndRenderSplitscreenWithoutAA();
	void Resize();

	void SetupUserControls(const S32 viewIndex);
	void UpdateUserControls(const S32 viewIndex);

	bool EnterSection();
	void ExitSection();

	void PrevSection();
	void NextSection();

	void SelectSection(const Util::C_String& name);

	const bool InSection() const;

private:

	Util::C_String					m_ErrorStatus;
	Display::C_Texture2D			m_Texture;
	C_TestSectionLauncherBase*		m_pCurrLauncher;
	C_TestSectionLauncherBase*		m_pHeadLauncher;
	C_TestSectionLauncherBase*		m_pTailLauncher;

	//Font renderer
	Render::Font::C_FontRenderer*	m_pFontRenderer;

	//Quit section menu item
	c_QuitSectionMenuItem*			m_pQuitSectionMenuItem;
};

}; //namespace Framework

#endif // TEST_SECTIONS_TEST_SECTION_MANAGER_H
