#include "stdafx.h"
#include "TestSections/TestSectionMovie.h"
#include <System/Memory/MemoryTracker.h>


Debug::Bool g_bDumpMemory		("Movie.DumpMemory",		false);
Debug::Bool g_bDisableAudio		("Movie.DisableAudio",		false);
Debug::Int g_iAudioVolume		("Movie.AudioVolume",		32768,		Debug::Range(0, 65536));

namespace Framework
{

C_TestSectionMovie::C_TestSectionMovie(const Util::C_String& name) : C_TestSection(name)
{
	m_iMovieVolume = -1;

}

C_TestSectionMovie::~C_TestSectionMovie()
{
}

bool C_TestSectionMovie::OnCreateVolatile()
{
	const Util::C_String fileName = "GAME:\\Movies\\BizarreSplash.bik";

	Movie::C_MovieParams movieParams;
	if(!m_Movie.Open(fileName, movieParams))
	{
		LOGWARN(Display::g_Log, "Failed to create movie '%s'\n", fileName.Chars());
		return false;
	}

	return true;
}

void C_TestSectionMovie::OnDestroyVolatile()
{
	m_Movie.Close();
}

void C_TestSectionMovie::OnUpdate()
{
	PERF_EVENT("UpdateMovie");

	if(m_iMovieVolume != g_iAudioVolume)
	{
		m_iMovieVolume = g_iAudioVolume;

		m_Movie.SetVolume(m_iMovieVolume);
	}

	m_Movie.Update();

	if(g_bDumpMemory)
	{
		Memory::Tracker::Instance().DumpNodeStats(false, false);

		g_bDumpMemory = false;
	}
}

void C_TestSectionMovie::OnPreRender()
{
	PERF_EVENT("PreRenderMovie");

	m_Movie.Render();
}

void C_TestSectionMovie::OnRender2DView()
{
	PERF_EVENT("RenderMovie");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));

	Display::BasicRender()->DrawTextured2DRect(100.f, 100.f, 1024.f-100.f-100.f, 768.f-100.f-100.f, Maths::RGBA(255, 255, 255, 255), m_Movie.GetTexture());
}

}; // namespace Framework
