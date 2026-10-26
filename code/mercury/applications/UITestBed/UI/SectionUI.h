#ifndef SAMPLES_SECTIONUI_H
#define SAMPLES_SECTIONUI_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"

namespace UI
{
class C_UiManager;
};

namespace Samples
{

class C_SectionUI: public C_Section
{
public:

	C_SectionUI( const Util::C_String& name );
	~C_SectionUI();

	bool OnCreate();
	void OnDestroy();

	void OnUpdate( const C_Input& input, const C_Timer& timer );
	void OnRenderHudView( const C_Input& input, const C_Timer& timer );
	void OnRender(const C_Input& input, const C_Timer& timer);
	void OnPreRender(const C_Input& input, const C_Timer& timer);
	void OnPostRender(const C_Input& input, const C_Timer& timer);
	void OnRenderGameViewWithAA(const C_Input& input, const C_Timer& timer);
private:
	UI::C_UiManager*				m_pUiManager;
};

}; // namespace Samples

#endif // SAMPLES_SECTIONUI_H
