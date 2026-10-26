#include "StdAfx.h"
#include "Samples/Section/SectionMovie.h"


Debug::Int g_debugMovieVolume	("Section.Movie.Volume",	32768,		Debug::Range(0, 65536));

#ifdef PLATFORM_PS3
Debug::Int g_debugMoviePosX		("Section.Movie.PosX",		50,			Debug::Range(0, 1024));
Debug::Int g_debugMoviePosY		("Section.Movie.PosY",		50,			Debug::Range(0, 768));
Debug::Int g_debugMovieSize		("Section.Movie.Size",		20,			Debug::Range(0, 100));
#endif

namespace Samples
{

C_SectionMovie::C_SectionMovie(const Util::C_String& name) : C_Section(name)
{
	m_MovieVolume = -1;

	#ifndef PLATFORM_PS3
	/*if(!Render::Movie::C_Movie::CreateShaders())
	{
		LOGFATAL(g_Log, "Failed to create the bink shaders\n");
	}*/
	#endif

}

C_SectionMovie::~C_SectionMovie()
{
	#ifndef PLATFORM_PS3
	//Render::Movie::C_Movie::DestroyShaders();
	#endif
}

bool C_SectionMovie::OnCreate()
{
	return true;
}

bool C_SectionMovie::OnCreateVolatile()
{
	#ifdef PLATFORM_PS3
	const Util::C_String fileName = "/app_home/Image/Movies/Outro_Seager.bik";
	#else
	const Util::C_String fileName = "Movies/Intro.bik";
	#endif

	Movie::C_MovieParams movieParams;
	if(!m_Movie.Open(fileName, movieParams))
	{
		LOGWARN(g_Log, "Failed to create movie '%s'\n", fileName.Chars());
		return false;
	}

	return true;
}

void C_SectionMovie::OnDestroyVolatile()
{
	m_Movie.Close();
}

void C_SectionMovie::OnDestroy()
{
}

void C_SectionMovie::OnInput(const C_Input& input, const C_Timer& timer)
{
	#ifdef PLATFORM_PC
	if(input.IsKeyDn('P'))
	{
		m_Movie.SetPaused(!m_Movie.GetPaused());
	}
	#endif

}

void C_SectionMovie::OnUpdate(const C_Input& input, const C_Timer& timer)
{
	m_Movie.Update();
}

void C_SectionMovie::OnPreRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("PreRenderSectionMovie");

	if(m_MovieVolume != g_debugMovieVolume)
	{
		m_MovieVolume = g_debugMovieVolume;
		m_Movie.SetVolume(m_MovieVolume);
	}

	m_Movie.Render();
}

void C_SectionMovie::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionMovie");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));

	Display::BasicRender()->DrawTextured2DRect(100.f, 100.f, 1024.f-100.f-100.f, 768.f-100.f-100.f, Maths::RGBA(255, 255, 255, 255), m_Movie.GetTexture());

	#ifdef PLATFORM_PS3
	F32 x = (F32)g_debugMoviePosX;
	F32 y = (F32)g_debugMoviePosY;
	F32 s = (F32)g_debugMovieSize;
	m_Movie.DebugRender(x, y, s);
	#endif

	Util::C_String str;
	str.Printf("Movie Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
