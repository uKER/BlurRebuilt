#ifndef TEST_SECTIONS_TEST_SECTION_MOVIE_H
#define TEST_SECTIONS_TEST_SECTION_MOVIE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "TestSections/TestSection.h"
#include <Movie/Movie.h>

namespace Framework
{

class C_TestSectionMovie : public C_TestSection
{
public:

	C_TestSectionMovie(const Util::C_String& name);
	~C_TestSectionMovie();

	bool OnCreateVolatile();
	void OnDestroyVolatile();

	void OnUpdate();
	void OnPreRender();
	void OnRender2DView();

private:

	S32				m_iMovieVolume;
	Movie::C_Movie	m_Movie;
};

}; // namespace Framework

#endif // TEST_SECTIONS_TEST_SECTION_MOVIE_H
