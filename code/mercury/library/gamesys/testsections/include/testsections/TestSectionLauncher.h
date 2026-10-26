#ifndef TEST_SECTIONS_TEST_SECTION_LAUNCHER_H
#define TEST_SECTIONS_TEST_SECTION_LAUNCHER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "TestSections/TestSection.h"

namespace Framework
{

//! C_TestSectionLauncherBase

class C_TestSectionLauncherBase
{
public:

	C_TestSectionLauncherBase(const Util::C_String& name);
	virtual ~C_TestSectionLauncherBase();

	const Util::C_String& GetName() const;

	bool Create();
	void Destroy();
	
	void Update();
	void PreRender();
	void Render3DViewWithAA(const S32 viewIndex);
	void Render3DViewWithoutAA(const S32 viewIndex);
	void Render2DView();
	void PostRender();
	void BeginRenderSplitscreenWithoutAA();
	void EndRenderSplitscreenWithoutAA();
	void Resize();

	void SetupUserControls(const S32 viewIndex);
	void UpdateUserControls(const S32 viewIndex);

	const bool IsActive() const;

	virtual C_TestSection* CreateSection() = 0;

public:

	C_TestSectionLauncherBase*		m_pPrev;
	C_TestSectionLauncherBase*		m_pNext;

protected:

	Util::C_String					m_Name;

private:

	C_TestSection*					m_pSection;
};

//! C_TestSectionLauncher

template<class T_Section>
class C_TestSectionLauncher : public C_TestSectionLauncherBase
{
public:

	C_TestSectionLauncher(const Util::C_String& name) : C_TestSectionLauncherBase(name)
	{
	}
	
	~C_TestSectionLauncher()
	{
	}

	C_TestSection* CreateSection()
	{
		return new T_Section(m_Name);
	}
};

}; //namespace Framework

#endif // TEST_SECTIONS_TEST_SECTION_LAUNCHER_H
