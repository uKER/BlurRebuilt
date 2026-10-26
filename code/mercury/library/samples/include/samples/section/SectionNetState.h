#ifndef SAMPLES_SECTIONNETSTATE_H
#define SAMPLES_SECTIONNETSTATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <list>
#include "Samples/Section/Section.h"

//namespace Samples
//{

class C_SectionNetState : public Samples::C_Section
{
public:
	C_SectionNetState(const Util::C_String& name);
	~C_SectionNetState();

	void OnUpdate(const Samples::C_Input& input, const Samples::C_Timer& timer);
	void OnRender(const Samples::C_Input& input, const Samples::C_Timer& timer);
	void OnInput(const Samples::C_Input& input, const Samples::C_Timer& timer);

protected:
	bool OnCreate();
	void OnDestroy();

private:
	bool ChangeState(class C_GameState& xGameState, class C_EntityState* xState, int uByteChangeProbability);

private:
	Display::C_Texture2D		m_Texture;
};

//}; // namespace Samples

#endif // SAMPLES_SECTIONEMPTY_H
