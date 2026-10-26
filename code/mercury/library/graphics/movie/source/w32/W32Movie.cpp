#include "stdafx.h"
#include "Movie/W32/W32Movie.h"
#include <d3d9.h>

extern Debug::Bool g_debugMovieDisableRender;

#ifndef CFG_GOLD
Debug::Bool	g_debugRenderMovieShowBinkSummary	("Graphics.Render.Movie.ShowBinkSummary",	false);
Debug::Int	g_debugRenderMovieBinkSummaryPosX	("Graphics.Render.Movie.BinkSummaryPosX",	50, Debug::Range(0, 1024));
Debug::Int	g_debugRenderMovieBinkSummaryPosY	("Graphics.Render.Movie.BinkSummaryPosY",	150, Debug::Range(0, 768));
Debug::Int	g_debugRenderMovieBinkSummarySize	("Graphics.Render.Movie.BinkSummarySize",	15, Debug::Range(0, 50));
Debug::Int	g_debugRenderMovieBinkSummaryFade	("Graphics.Render.Movie.BinkSummaryFade",	128, Debug::Range(0, 255));
#endif

namespace Movie
{

C_W32Movie::C_W32Movie() :
	m_BinkIsDirty(false),
	m_BinkIsPlaying(false),
	m_pBink(NULL)
{
}

C_W32Movie::~C_W32Movie()
{
	BIZ_ASSERT(!m_pBink);
}

bool C_W32Movie::HasReachedEnd() const
{
	if(m_pBink)
	{
		return m_pBink->FrameNum >= m_pBink->Frames ? true : false;
	}
	else
	{
		return true;
	}
}

void C_W32Movie::SetPaused(const bool paused)
{
	m_Paused = paused;

	if(m_pBink)
		BinkPause(m_pBink, m_Paused ? 1 : 0);
}

void C_W32Movie::SetVolume(const S32 volume)
{
	if(!m_pBink)
		return;

	for (U32 i = 0; i < m_Params.m_NumTracks; ++i)
	{
		BinkSetVolume(m_pBink, m_Params.m_TrackIDsToPlay[i], volume);
	}
	
	m_Volume = volume;
}

void C_W32Movie::SetBinkIOSize(U32 size)
{
	BinkSetIOSize(size);
}

bool C_W32Movie::CreateBink(const C_MovieParams& params)
{
	if(m_pBink)
		return false;

	m_BinkIsPlaying = false;

	if(!CreateShaders())
		return false;

	//const Util::C_String filePath = Display::FormatString(Sys::C_FileSystem::Instance().GetWorkingDirectory() + "/" + m_FileName);
	const Util::C_String filePath = m_FileName;

	m_Params = params;
	BinkSetSoundTrack(m_Params.m_NumTracks, m_Params.m_TrackIDsToPlay);

	U32 flags = BINKSNDTRACK | BINKNOFRAMEBUFFERS | BINKALPHA;

	if(m_Params.m_CreateFlags == E_MovieCreateFlags_LoadIntoMemory)
		flags |= BINKPRELOADALL;
	if(m_Params.m_CreateFlags == E_MovieCreateFlags_PlayFromMemory)
		flags |= BINKFROMMEMORY;
	if(m_Params.m_CreateFlags & E_MovieCreateFlags_SetBufferIOSize)
		flags |= BINKIOSIZE;

	BinkSoundUseWaveOut();

	if(m_Params.m_CreateFlags == E_MovieCreateFlags_PlayFromMemory)
	{
		m_pBink = BinkOpen((const char*)m_Params.m_pMovieData, flags);
	}
	else
	{
		m_pBink = BinkOpen(filePath.Chars(), flags);
	}

#ifdef MILES//PLATFORM_PC
	U32 volumes[ 6 ]; // 6 channels for 5.1

	// turn on the front left and right for the first Bink track
	memset( volumes, 0, sizeof( volumes ) );
	volumes[ 0 ] = 0;
	volumes[ 1 ] = 0;
	BinkSetMixBinVolumes( m_pBink, m_Params.m_TrackIDsToPlay[0], 0, (S32*)&volumes, 6 ); 

	// turn on the center for the second Bink track
	memset( volumes, 0, sizeof( volumes ) );
	volumes[ 2 ] = 0;
	BinkSetMixBinVolumes( m_pBink, m_Params.m_TrackIDsToPlay[1], 0, (S32*)&volumes, 6 ); 

	// turn on the sub woofer for the third Bink track
	memset( volumes, 0, sizeof( volumes ) );
	volumes[ 3 ] = 0;
	BinkSetMixBinVolumes( m_pBink, m_Params.m_TrackIDsToPlay[2], 0, (S32*)&volumes, 6 ); 

	// turn on the back left and right for the fourth Bink track
	memset( volumes, 0, sizeof( volumes ) );
	volumes[ 4 ] = 0;
	volumes[ 5 ] = 0;
	BinkSetMixBinVolumes( m_pBink, m_Params.m_TrackIDsToPlay[3], 0, (S32*)&volumes, 6 ); 

	// route fifth Bink track to centre speaker (mono VO channel)
	memset( volumes, 0, sizeof( volumes ) );
	volumes[ 2 ] = 0;
	BinkSetMixBinVolumes( m_pBink, m_Params.m_TrackIDsToPlay[4], 0, (S32*)&volumes, 6 ); 

	// route sixth and final Bink track to front speakers (stereo VO channel)
	memset( volumes, 0, sizeof( volumes ) );
	volumes[ 4 ] = 0;
	volumes[ 5 ] = 0;
	BinkSetMixBinVolumes( m_pBink, m_Params.m_TrackIDsToPlay[5], 0, (S32*)&volumes, 6 ); 
#endif

	if(!m_pBink)
	{
		LOGWARN(Render::g_Log, "Failed to open file '%s'\n", filePath.Chars());
		return false;
	}

	if(m_pBink->FrameRateDiv>0)
	{
		F32 frame_rate = (F32)m_pBink->FrameRate / (F32)m_pBink->FrameRateDiv;
		m_TotalTime = (F32)m_pBink->Frames / frame_rate;
	}

	m_BinkIsPlaying = true;

	return true;
}

void C_W32Movie::DestroyBink()
{
	//DestroyShaders();

	if(m_pBink)
	{
		BinkClose(m_pBink);
		m_pBink = NULL;
	}

	m_BinkIsPlaying = false;
}

bool C_W32Movie::CreateShaders()
{
	// [Blur1PCDX11] BinkOpen is a stub in phase one and returns null.
	return true;
}

void C_W32Movie::DestroyShaders()
{
}


bool C_W32Movie::CreateTextures()
{
	return false; // The installed Bink stub cannot supply a decoded frame.
}

void C_W32Movie::DestroyTextures()
{
}

void C_W32Movie::Rewind()
{
	if(!m_pBink)
		return;

	m_pBink->FrameNum = 0;

	m_BinkIsPlaying = true;
}

bool C_W32Movie::Update()
{
	if(!m_pBink)
		return true;

	if(m_BinkIsPlaying)
	{
		if(m_pBink->ReadError != 0)
		{
			m_BinkIsPlaying = false; // Bink read error
		}
		else
		if(m_pBink->FrameNum >= m_pBink->Frames)
		{
			m_BinkIsPlaying = false; // Bink has played all the frames of the movie
		}
	}

	if(m_pBink->FrameRateDiv>0)
	{
		F32 frame_rate = (F32)m_pBink->FrameRate / (F32)m_pBink->FrameRateDiv;
		m_CurrentTime = (F32)m_pBink->FrameNum / frame_rate;
	}

	return !m_BinkIsPlaying;
}

void C_W32Movie::Render()
{
	return; // BinkOpen returns null with the phase-one SDK stub.
}

void C_W32Movie::DebugRender(F32& x, F32& y, const F32 s)
{
	#ifndef CFG_GOLD
	if(!m_pBink)
		return;

	Display::BasicRender()->DrawTextured2DRect(x, y, s, s, Maths::RGBA(255, 255, 255, 255), &m_RenderTexture.GetTexture());
	x += s;
	
	if(g_debugRenderMovieShowBinkSummary)
	{
		BINKSUMMARY summary;
		BinkGetSummary(m_pBink, &summary);

		Display::BasicRender()->DrawColoured2DRect(20.f, 20.f, 984.f, 728.f, Maths::RGBA(0, 0, 0, g_debugRenderMovieBinkSummaryFade));

		F32 x = (F32)g_debugRenderMovieBinkSummaryPosX;
		F32 y = (F32)g_debugRenderMovieBinkSummaryPosY;
		F32 s = (F32)g_debugRenderMovieBinkSummarySize;

		Util::C_String temp;

		temp.Printf("Width %i Height %i TotalTime %i",
			summary.Width,
			summary.Height,
			summary.TotalTime);
		Display::FontRender()->Draw(temp.Chars(), x, y, s, s, Maths::RGBA(255, 255, 255, 255));
		y += s*1.5f;

		temp.Printf("FileFrameRate %i FileFrameRateDiv %i",
			summary.FileFrameRate,
			summary.FileFrameRateDiv);
		Display::FontRender()->Draw(temp.Chars(), x, y, s, s, Maths::RGBA(255, 255, 255, 255));
		y += s*1.5f;

		temp.Printf("FrameRate % FrameRateDiv %i",
			summary.FrameRate,
			summary.FrameRateDiv);
		Display::FontRender()->Draw(temp.Chars(), x, y, s, s, Maths::RGBA(255, 255, 255, 255));
		y += s*1.5f;

		temp.Printf("TotalOpenTime %i TotalFrames %i TotalPlayedFrames %i",
			summary.TotalOpenTime,
			summary.TotalFrames,
			summary.TotalPlayedFrames);
		Display::FontRender()->Draw(temp.Chars(), x, y, s, s, Maths::RGBA(255, 255, 255, 255));
		y += s*1.5f;

		temp.Printf("SkippedFrames %i SkippedBlits %i SoundSkips %i",
			summary.SkippedFrames,
			summary.SkippedBlits,
			summary.SoundSkips);
		Display::FontRender()->Draw(temp.Chars(), x, y, s, s, Maths::RGBA(255, 255, 255, 255));
		y += s*1.5f;

		temp.Printf("SoundSkips %i TotalBlitTime %i TotalReadTime %i",
			summary.SoundSkips,
			summary.TotalBlitTime,
			summary.TotalReadTime);
		Display::FontRender()->Draw(temp.Chars(), x, y, s, s, Maths::RGBA(255, 255, 255, 255));
		y += s*1.5f;

		temp.Printf("TotalVideoDecompTime %i TotalAudioDecompTime %i",
			summary.TotalVideoDecompTime,
			summary.TotalAudioDecompTime);
		Display::FontRender()->Draw(temp.Chars(), x, y, s, s, Maths::RGBA(255, 255, 255, 255));
		y += s*1.5f;

		temp.Printf("TotalIdleReadTime %i TotalBackReadTime %i TotalReadSpeed %i",
			summary.TotalIdleReadTime,
			summary.TotalBackReadTime,
			summary.TotalReadSpeed);
		Display::FontRender()->Draw(temp.Chars(), x, y, s, s, Maths::RGBA(255, 255, 255, 255));
		y += s*1.5f;

		temp.Printf("SlowestFrameTime %i Slowest2FrameTime %i",
			summary.SlowestFrameTime,
			summary.Slowest2FrameTime);
		Display::FontRender()->Draw(temp.Chars(), x, y, s, s, Maths::RGBA(255, 255, 255, 255));
		y += s*1.5f;

		temp.Printf("SlowestFrameNum %i Slowest2FrameNum %i",
			summary.SlowestFrameNum,
			summary.Slowest2FrameNum);
		Display::FontRender()->Draw(temp.Chars(), x, y, s, s, Maths::RGBA(255, 255, 255, 255));
		y += s*1.5f;

		temp.Printf("AverageDataRate %i AverageFrameSize %i",
			summary.AverageDataRate,
			summary.AverageFrameSize);
		Display::FontRender()->Draw(temp.Chars(), x, y, s, s, Maths::RGBA(255, 255, 255, 255));
		y += s*1.5f;

		temp.Printf("HighestMemAmount %i TotalIOMemory %i",
			summary.HighestMemAmount,
			summary.TotalIOMemory);
		Display::FontRender()->Draw(temp.Chars(), x, y, s, s, Maths::RGBA(255, 255, 255, 255));
		y += s*1.5f;

		temp.Printf("HighestIOUsed %i Highest1SecRate %i Highest1SecFrame %i",
			summary.HighestIOUsed,
			summary.Highest1SecRate,
			summary.Highest1SecFrame);
		Display::FontRender()->Draw(temp.Chars(), x, y, s, s, Maths::RGBA(255, 255, 255, 255));
		y += s*1.5f;

		static U32 TotalVideoDecompTime = 0;
		static U32 TotalAudioDecompTime = 0;

		temp.Printf("LastVideoDecompTime %i LastAudioDecompTime %i",
			summary.TotalVideoDecompTime-TotalVideoDecompTime,
			summary.TotalAudioDecompTime-TotalAudioDecompTime);
		Display::FontRender()->Draw(temp.Chars(), x, y, s, s, Maths::RGBA(255, 255, 255, 255));
		y += s*1.5f;

		TotalVideoDecompTime = summary.TotalVideoDecompTime;
		TotalAudioDecompTime = summary.TotalAudioDecompTime;
	}
	#endif
}

}; // namespace Movie
