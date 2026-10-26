#ifndef TEST_SECTION_UI_H
#define TEST_SECTION_UI_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSection.h>
#include <Input/GameInput.h>

namespace UI
{
class C_UiManager;
};

class C_SectionUI : public Framework::C_TestSection
{
public:
	C_SectionUI( const Util::C_String& name );
	~C_SectionUI();

public:
	void OnUpdate();
	void OnPreRender();
	void OnRender3DViewWithAA( const S32 viewIndex );
	void OnRender2DView();

	void OnSetupUserControls( const S32 viewIndex );

private:
	UI::C_UiManager*		m_pUiManager;
};

void SetupUiControls( Input::C_GameInput* pGameInput, Input::C_Device* pPad );

#endif // TEST_SECTION_UI_H
