#ifndef SAMPLES_SECTIONEMPTY_H
#define SAMPLES_SECTIONEMPTY_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"

namespace Samples
{

class C_SectionEmpty : public C_Section
{
public:

	C_SectionEmpty(const Util::C_String& name);
	~C_SectionEmpty();

	void OnUpdate(const C_Input& input, const C_Timer& timer);
	void OnRender(const C_Input& input, const C_Timer& timer);

protected:

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();

private:

	Display::C_Texture2D m_Texture;
};

}; // namespace Samples

#endif // SAMPLES_SECTIONEMPTY_H
