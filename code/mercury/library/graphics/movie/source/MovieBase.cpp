#include "stdafx.h"
#include "Bink.h"
#include "Movie/MovieBase.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_RENDER_MOVIE
#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF_RENDER_MOVIE
#endif

Debug::Bool g_debugMovieDisableAudio("Graphics.Render.Movie.DisableAudio", false, Debug::Persist());
Debug::Bool g_debugMovieDisableRender("Graphics.Render.Movie.DisableRender", false);

namespace Movie
{

Debug::Logger& g_MovieLog = Debug::GetLogger("mercury.movie");

// C_MovieParams

C_MovieParams::C_MovieParams()
{
	Reset();
	
	// Default track mixing params
	m_TrackIDsToPlay[0]		= 0;
	m_TrackIDsToPlay[1]		= 1;
	m_TrackIDsToPlay[2]		= 2;
	m_TrackIDsToPlay[3]		= 3;
	m_TrackIDsToPlay[4]		= 0xffffffff;	// Mono VO
	m_TrackIDsToPlay[5]		= 0xffffffff;	// Stereo VO

}

C_MovieParams::~C_MovieParams()
{
}

void C_MovieParams::Reset()
{
	m_CreateFlags			= E_MovieCreateFlags_StreamFromDisk;
	m_MemoryFlags			= E_MovieMemoryFlags_Graphics;
	m_BinkIOSize			= 0;

	m_pMovieData			= NULL;
}

// C_MovieBase

cMovieCallBackFunctor*	C_MovieBase::mPauseFileSystem = NULL;
cMovieCallBackFunctor*	C_MovieBase::mResumeFileSystem = NULL;

C_MovieBase::C_MovieBase() :
	m_Audio(false),
	m_Video(true),
	m_Paused(false),
	m_Looping(true),
	m_CurrentTime(0.0f),
	m_TotalTime(0.0f),
	m_Volume(32768),
	m_FileName("")
{
}

C_MovieBase::~C_MovieBase()
{
}

void C_MovieBase::SetFileSystemCallBacks( cMovieCallBackFunctor* PauseFunction, cMovieCallBackFunctor* ResumeFunction )
{
	mPauseFileSystem = PauseFunction;
	mResumeFileSystem = ResumeFunction;
}

void C_MovieBase::CalculateUVRange(const BINKFRAMEBUFFERS& binkBuffers, F32& uEnd0, F32& vEnd0, F32& uEnd1, F32& vEnd1)
{
	uEnd0 = 1.f;
	vEnd0 = 1.f;
	uEnd1 = 1.f;
	vEnd1 = 1.f;

	// TODO: [rthomas]	Makes assumption that cRcBBufferWidth/Height will be at near enough halfres. Flesh out to handle other possible differences in aspect ratio?

	if (binkBuffers.cRcBBufferWidth)
		uEnd1 -= ((binkBuffers.cRcBBufferWidth - (binkBuffers.YABufferWidth / 2)) / (F32)binkBuffers.cRcBBufferWidth);
	if (binkBuffers.cRcBBufferHeight)
		vEnd1 -= ((binkBuffers.cRcBBufferHeight - (binkBuffers.YABufferHeight / 2)) / (F32)binkBuffers.cRcBBufferHeight);
}

}; // namespace Movie
