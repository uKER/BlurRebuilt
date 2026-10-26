#ifndef SAMPLES_SECTIONGLOBALGRAPHICSBUFFERS_H
#define SAMPLES_SECTIONGLOBALGRAPHICSBUFFERS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"

namespace Samples
{

class C_SectionGlobalGraphicsBuffers : public C_Section
{
public:

	C_SectionGlobalGraphicsBuffers(const Util::C_String& name);
	~C_SectionGlobalGraphicsBuffers();

	bool OnCreateVolatile();
	void OnDestroyVolatile();

	void OnRender(const C_Input& input, const C_Timer& timer);
};

}; // namespace Samples

#endif // SAMPLES_SECTIONGLOBALGRAPHICSBUFFERS_H
