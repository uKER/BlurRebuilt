#ifndef TEST_SECTIONS_TEST_SECTION_H
#define TEST_SECTIONS_TEST_SECTION_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Cam2 { class C_DebugTools; }

namespace Framework
{

//! C_TestSection
class C_ApplicationBase;
class C_View;

class C_TestSection : public Display::I_Renderer
{
public:
	
	C_TestSection(const Util::C_String& name);
	virtual ~C_TestSection();

	virtual void OnUpdate();
	virtual void OnPreRender();
	virtual void OnPostRender();
	virtual void OnRender3DViewWithAA(const S32 viewIndex);
	virtual void OnRender3DViewWithoutAA(const S32 viewIndex);
	virtual void OnRender2DView();
	virtual void OnBeginRenderSplitscreenWithoutAA();
	virtual void OnEndRenderSplitscreenWithoutAA();
	virtual void OnResize();

	virtual void OnSetupUserControls(const S32 viewIndex);
	virtual void OnUpdateUserControls(const S32 viewIndex);
	// Helper functions:
	C_ApplicationBase& Application();

private:
	Cam2::C_DebugTools* m_DebugTools;
};

}; //namespace Framework

#endif // TEST_SECTIONS_TEST_SECTION_H
