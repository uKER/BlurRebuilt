#ifndef SAMPLES_SECTION_H
#define SAMPLES_SECTION_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Utils/Renderer.h"
#include "Samples/System/Input.h"
#include "Samples/System/Timer.h"

namespace Samples
{

//! C_Section

class C_Section : public Display::I_Renderer
{
public:

	C_Section(const Util::C_String& name = "Unknown");
	virtual ~C_Section();
	
	virtual void OnInput(const C_Input& input, const C_Timer& timer);
	virtual void OnUpdate(const C_Input& input, const C_Timer& timer);
	virtual void OnRender(const C_Input& input, const C_Timer& timer);
	virtual void OnPreRender(const C_Input& input, const C_Timer& timer);
	virtual void OnPostRender(const C_Input& input, const C_Timer& timer);
	virtual void OnRenderGameViewWithAA(const C_Input& input, const C_Timer& timer);
	virtual void OnRenderGameViewWithoutAA(const C_Input& input, const C_Timer& timer);
	virtual void OnRenderHudView(const C_Input& input, const C_Timer& timer);
	virtual void OnBeginRenderSplitscreenWithoutAA();
	virtual void OnEndRenderSplitscreenWithoutAA();

	// called if the device changes size, will need to handle post effects etc if necessary
	virtual void OnResize();

protected:

	virtual bool OnCreate();
	virtual bool OnCreateVolatile();
	virtual void OnDestroyVolatile();
	virtual void OnDestroy();
};

//! C_SectionLauncherBase

class C_SectionLauncherBase
{
public:

	C_SectionLauncherBase(const Util::C_String& name);
	virtual ~C_SectionLauncherBase();

	const bool IsActive() const;

	const Util::C_String& GetName() const;

	void Create();
	void Destroy();
	
	void Input(const C_Input& input, const C_Timer& timer);
	void Update(const C_Input& input, const C_Timer& timer);
	void PreRender(const C_Input& input, const C_Timer& timer);
	void PostRender(const C_Input& input, const C_Timer& timer);
	void RenderGameViewWithAA(const C_Input& input, const C_Timer& timer);
	void RenderGameViewWithoutAA(const C_Input& input, const C_Timer& timer);
	void RenderHudView(const C_Input& input, const C_Timer& timer);

	void BeginRenderSplitscreenWithoutAA();
	void EndRenderSplitscreenWithoutAA();
	void Resize();

	virtual C_Section* CreateSection() = 0;

public:

	C_SectionLauncherBase*				m_pPrev;
	C_SectionLauncherBase*				m_pNext;

protected:

	Util::C_String				m_Name;

private:

	C_Section*					m_pSection;
};

//! C_SectionLauncher

template<class Section>
class C_SectionLauncher : public C_SectionLauncherBase
{
public:

	C_SectionLauncher(const Util::C_String& name) : C_SectionLauncherBase(name) {}
	~C_SectionLauncher() {}

	C_Section* CreateSection() { return new Section(m_Name); }
};

//! C_SectionManager

class C_SectionManager : public Display::I_Renderer
{
public:
	
	C_SectionManager();
	~C_SectionManager();

	const bool InSection() const;

	void AddSection(C_SectionLauncherBase* pLauncher);

	bool OnCreate();
	void OnDestroy();

	void InputSection(const C_Input& input, const C_Timer& timer);
	void UpdateSection(const C_Input& input, const C_Timer& timer);
	void PreRenderSection(const C_Input& input, const C_Timer& timer);
	void PostRenderSection(const C_Input& input, const C_Timer& timer);
	void GameViewWithAARenderSection(const C_Input& input, const C_Timer& timer);
	void GameViewWithoutAARenderSection(const C_Input& input, const C_Timer& timer);
	void HudViewRenderSection(const C_Input& input, const C_Timer& timer);
	
	void BeginRenderSplitscreenWithoutAA();
	void EndRenderSplitscreenWithoutAA();
	void Resize();

	void EnterSection();
	void ExitSection();

	void PrevSection();
	void NextSection();

private:

	Display::C_Texture2D		m_Texture;
	C_SectionLauncherBase*		m_pCurrLauncher;
	C_SectionLauncherBase*		m_pHeadLauncher;
	C_SectionLauncherBase*		m_pTailLauncher;
};

}; // namespace Samples

#include "Section.inl"

#endif // SAMPLES_SECTION_H
