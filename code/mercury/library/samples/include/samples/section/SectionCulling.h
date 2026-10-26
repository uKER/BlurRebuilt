#ifndef SAMPLES_SECTIONMOVIE_H
#define SAMPLES_SECTIONMOVIE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Movie/Movie.h"

namespace Samples
{

class C_SectionMovie : public C_Section
{
public:

	C_SectionMovie(const Util::C_String& name);
	~C_SectionMovie();

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();

	void OnInput(const C_Input& input, const C_Timer& timer);
	void OnUpdate(const C_Input& input, const C_Timer& timer);
	void OnPreRender(const C_Input& input, const C_Timer& timer);
	void OnRender(const C_Input& input, const C_Timer& timer);

private:

	Render::Movie::C_Movie m_Movie;
};

}; // namespace Samples

#endif // SAMPLES_SECTIONMOVIE_H
