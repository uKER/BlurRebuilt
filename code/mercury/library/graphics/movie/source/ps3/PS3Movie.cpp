#include "stdafx.h"
#include "Movie/PS3/PS3Movie.h"
#include "Movie/MovieTimings.h"

void *bink_mem_alloc( U32 num_bytes )
{
	return BizAlignedMalloc( num_bytes, 32 );
}

void bink_mem_free( void *ptr )
{
	BizAlignedFree( ptr );
}

namespace Movie
{

#if !CFG_GOLD
Debug::Bool	g_debugRenderMovieShowBinkSummary	("Graphics.Movie.ShowBinkSummary",		false);
Debug::Int	g_debugRenderMovieBinkSummaryPosX	("Graphics.Movie.BinkSummaryPosX",		50,		Debug::Range(0, 1024));
Debug::Int	g_debugRenderMovieBinkSummaryPosY	("Graphics.Movie.BinkSummaryPosY",		150,	Debug::Range(0, 768));
Debug::Int	g_debugRenderMovieBinkSummarySize	("Graphics.Movie.BinkSummarySize",		15,		Debug::Range(0, 50));
Debug::Int	g_debugRenderMovieBinkSummaryFade	("Graphics.Movie.BinkSummaryFade",		128,	Debug::Range(0, 255));
Debug::Bool g_debugMovieDrawFrameInfo			("Graphics.Movie.DrawFrameInfo",		false);
Debug::Int	g_debugMovieDrawFrameInfoPosX		("Graphics.Movie.DrawFrameInfoPosX",	50,		Debug::Range(0, 1024));
Debug::Int	g_debugMovieDrawFrameInfoPosY		("Graphics.Movie.DrawFrameInfoPosX",	250,	Debug::Range(0, 768));
Debug::Int	g_debugMovieDrawFrameInfoSize		("Graphics.Movie.DrawFrameInfoSize",	50,		Debug::Range(0, 250));
#endif

C_PS3Movie::C_PS3Movie() :
	m_FirstFrame(true),
	m_IsPlaying(false),
	m_pBink(NULL)
{
	BinkSetMemory( bink_mem_alloc, bink_mem_free );
}

C_PS3Movie::~C_PS3Movie()
{
	BIZ_ASSERT(!m_pBink);
}

bool C_PS3Movie::HasReachedEnd() const
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

void C_PS3Movie::SetVolume(const S32 volume)
{
	if(!m_pBink)
		return;

	for (U32 i = 0; i < m_Params.m_NumTracks; ++i)
	{
		BinkSetVolume(m_pBink, m_Params.m_TrackIDsToPlay[i], volume);
	}
	
	m_Volume = volume;
}

void C_PS3Movie::SetBinkIOSize(const U32 size)
{
	BinkSetIOSize(size);
}

bool C_PS3Movie::CreateBink(const C_MovieParams& params)
{
	m_IsPlaying = false;

	if(m_pBink)
		return false;

	LOGDEBUG(g_MovieLog, "Playing movie %s\n", m_FileName.Chars());

	if (m_Params.m_CreateFlags & E_MovieCreateFlags_PlayFromMemory)
	{
		Sys::C_FileInfo fileInfo;
		if (Sys::GetDefaultFileSystem()->FileInfo(m_FileName.Chars(), &fileInfo))
		{
			U32 fileSize = (U32)fileInfo.m_FileSize;
			U32 u32fileSize = (fileSize + 3) & ~3;
			m_pMovieData = new U8[u32fileSize];

			Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(m_FileName.Chars());
			if(pFile)
			{
				pFile->Read(m_pMovieData, fileSize);

				#if defined(PLATFORM_PS3)
				U32 *pData = (U32*)m_pMovieData;
				for(U32 i=0; i<u32fileSize/4; i++)
				{
					Ut::DoEndianSwap::EndianSwapD(pData);
					pData++;
				}
				#endif

				SAFE_DELETE_PTR(pFile);
			}	
			else
			{
				return false;
			}	
		}
		else
		{
			return false;
		}	
	}


	m_Params = params;

	BinkSetSoundTrack( m_Params.m_NumTracks, m_Params.m_TrackIDsToPlay );

	BinkSoundUseLibAudio( 6 );

	U32 flags = BINKSNDTRACK | BINKNOFRAMEBUFFERS | BINKALPHA;

	if(m_Params.m_CreateFlags & E_MovieCreateFlags_LoadIntoMemory)
	{
		flags |= BINKPRELOADALL;
	}

	if(m_Params.m_CreateFlags & E_MovieCreateFlags_PlayFromMemory)
	{
		flags |= BINKFROMMEMORY;
	}

	if(m_Params.m_CreateFlags & E_MovieCreateFlags_SetBufferIOSize)
	{
		flags |= BINKIOSIZE;
		//SetBinkIOSize(m_Params.m_BinkIOSize);
		SetBinkIOSize(1024 * 1024 * 8);
	}

	if(m_Params.m_CreateFlags & E_MovieCreateFlags_PlayFromMemory)
	{
		m_pBink = BinkOpen((const char*)m_pMovieData, flags);
	}
	else
	{
		if(m_FileName == "")
			return false; // Called when the renderers are created but the movie hasn't been setup yet?

		m_pBink = BinkOpen(m_FileName.Chars(), flags);
	}

	if(!m_pBink)
	{
		LOGERROR(g_MovieLog, "Failed to open bink file '%s'\n", m_FileName.Chars());
		return false;
	}

	if(!CreateShaders())
	{
		LOGERROR(g_MovieLog, "Failed to create the bink shaders for file '%s'\n", m_FileName.Chars());
		return false;
	}

	if(!SetupSpeakers())
	{
		LOGERROR(g_MovieLog, "Failed to setup the bink speakers for file '%s'\n", m_FileName.Chars());
		return false;
	}

	SetVolume(m_Volume);

	m_IsPlaying = true;

	return true;
}

void C_PS3Movie::DestroyBink()
{
	m_IsPlaying = false;

	DestroyShaders();

	if(m_pBink)
	{
		BinkClose(m_pBink);
		m_pBink = NULL;
	}
	
	if (m_Params.m_CreateFlags & E_MovieCreateFlags_PlayFromMemory)
	{
		SAFE_DELETE_ARY( m_pMovieData );
	}
}

bool C_PS3Movie::SetupSpeakers()
{
	const int kSpeakerFrontLeft		= 0;
	const int kSpeakerFrontRight	= 1;
	const int kSpeakerCentre		= 2;
	const int kSpeakerSub			= 3;
	const int kSpeakerBackLeft		= 4;
	const int kSpeakerBackRight		= 5;

	int full_volume = 32768;
	int vols[ 6 ];

	// Route channels to 5.1 speakers
	memset( vols, 0, sizeof( vols ) );
	vols[ kSpeakerCentre ] = full_volume;
	BinkSetMixBinVolumes( m_pBink, m_Params.m_TrackIDsToPlay[1], 0, vols, 6 );

	memset( vols, 0, sizeof( vols ) );
	vols[ kSpeakerFrontLeft ] = full_volume;
	vols[ kSpeakerFrontRight ] = full_volume;
	BinkSetMixBinVolumes( m_pBink, m_Params.m_TrackIDsToPlay[0], 0, vols, 6 );
	
	//memset( vols, 0, sizeof( vols ) );
	//vols[ 2 ] = full_volume;
	//BinkSetMixBinVolumes( m_pBink, 1, 0, vols, 6 );

	memset( vols, 0, sizeof( vols ) );
	vols[ kSpeakerSub ] = full_volume;
	BinkSetMixBinVolumes( m_pBink, m_Params.m_TrackIDsToPlay[2], 0, vols, 6  );

	memset( vols, 0, sizeof( vols ) );
	vols[ kSpeakerBackLeft ] = full_volume;
	vols[ kSpeakerBackRight ] = full_volume;
	BinkSetMixBinVolumes( m_pBink, m_Params.m_TrackIDsToPlay[3], 0, vols, 6 );

	// Route Mono VO channel (4) to centre speaker.
	memset( vols, 0, sizeof( vols ) );
	vols[ kSpeakerCentre ] = full_volume;
	BinkSetMixBinVolumes( m_pBink, m_Params.m_TrackIDsToPlay[4], 0, vols, 6 );

	// Route Stereo VO channel (5) to front speakers.
	memset( vols, 0, sizeof( vols ) );
	vols[ kSpeakerFrontLeft ] = full_volume;
	vols[ kSpeakerFrontRight ] = full_volume;
	BinkSetMixBinVolumes( m_pBink, m_Params.m_TrackIDsToPlay[5], 0, vols, 6 );

	return true;
}

bool C_PS3Movie::CreateShaders()
{
	if(!Create_Bink_shaders(m_pBink->Width, m_pBink->Height, m_Shaders))
		return false;

	return true;
}

void C_PS3Movie::DestroyShaders()
{
	Free_Bink_shaders(m_Shaders);
}

bool C_PS3Movie::CreateTextures()
{
	if(!m_pBink)
		return false;

	U32 flags = Display::E_Usage_Colour;

	if(m_Params.m_MemoryFlags == E_MovieMemoryFlags_System)
		flags |= Display::E_Usage_System;
	else
		flags |= Display::E_Usage_Video;

	if(!Display::Device()->CreateRenderTexture2D(m_pBink->Width, m_pBink->Height, 1, flags, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, NULL, &m_RenderTexture))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_RenderTexture, "_MovieRT");
	#endif

	BinkGetFrameBuffersInfo(m_pBink, &m_BinkTextureSet.bink_buffers);

	if(!Create_Bink_textures(&m_BinkTextureSet))
		return false;

	BinkRegisterFrameBuffers(m_pBink, &m_BinkTextureSet.bink_buffers);

	return true;
}

void C_PS3Movie::DestroyTextures()
{
	if(!m_pBink)
		return;

	Display::Device()->Destroy(&m_RenderTexture);

	Free_Bink_textures(&m_BinkTextureSet);
}

void C_PS3Movie::Rewind()
{
	if(!m_pBink)
		return;

	m_pBink->FrameNum = 0;

	m_IsPlaying = true;
}

bool C_PS3Movie::Update()
{
	if(!m_pBink)
		return true;

	if(m_IsPlaying)
	{
		if(m_pBink->ReadError != 0)
		{
			m_IsPlaying = false; // Bink read error
		}
		//else
		//if(m_pBink->FrameNum >= m_pBink->Frames)
		//{
			//m_IsPlaying = false; // Bink has played all the frames of the movie
		//}
	}
	
	return !m_IsPlaying;
}

#if !CFG_GOLD
void C_PS3Movie::DrawFrameInfo()
{
	if(!m_pBink)
		return;

	Display::C_View view;
	Display::C_ViewSetup viewSetup;
	Display::Device()->SetupView(&view, viewSetup, m_pBink->Width, m_pBink->Height);
	Display::Device()->SetView(&view);

	F32 posx = (F32)g_debugMovieDrawFrameInfoPosX;
	F32 posy = (F32)g_debugMovieDrawFrameInfoPosY;
	F32 size = (F32)g_debugMovieDrawFrameInfoSize;

	char str[128];
	sprintf_s(str, sizeof(str), "%i / %i (%i)", m_pBink->FrameNum, m_pBink->Frames, m_Looping);
	Display::FontRender()->Draw(str, posx, posy, size, size, Maths::RGBA(255, 255, 255, 255));

	Display::Device()->SetView(NULL);
}
#endif

void C_PS3Movie::Render()
{
	if(!m_IsPlaying)
		return;

	if(m_pBink && !BinkWait(m_pBink) && !m_FirstFrame)
	{
		MOVIE_TIMING("Render", MOVIE_DEFAULT_THRESHOLD, m_pBink->FrameNum, m_pBink->Frames);

		{
			MOVIE_TIMING("Wait_for_Bink_textures", MOVIE_DEFAULT_THRESHOLD, m_pBink->FrameNum, m_pBink->Frames);
			Wait_for_Bink_textures(&m_BinkTextureSet);
		}

		{
			MOVIE_TIMING("BinkDoFrame", MOVIE_DEFAULT_THRESHOLD, m_pBink->FrameNum, m_pBink->Frames);
			BinkDoFrame(m_pBink);
		}

		{
			MOVIE_TIMING("BinkShouldSkip", MOVIE_DEFAULT_THRESHOLD, m_pBink->FrameNum, m_pBink->Frames);
			while(BinkShouldSkip(m_pBink))
			{
				MOVIE_TIMING("BinkSkip", MOVIE_DEFAULT_THRESHOLD, m_pBink->FrameNum, m_pBink->Frames);
				BinkNextFrame(m_pBink);
				if (m_pBink->ReadError)
				{
					// read error
					m_IsPlaying = false;
					return;
				}
				else
				{
					BinkDoFrame(m_pBink);
				}
			}
		}

		Display::Device()->SetColourDepthStencilSurface(&m_RenderTexture.GetSurface(), NULL);
		Display::Device()->Clear(Display::E_Clear_Colour, Maths::RGBA(0, 0, 0, 255));
		Display::Device()->SetAlphaTestEnable(Display::E_Bool_False);
		Display::Device()->SetDepthEnable(Display::E_Bool_False);
		
		U32 flags = Display::E_ColourWriteEnable_Red|Display::E_ColourWriteEnable_Green|Display::E_ColourWriteEnable_Blue;
		if(m_pBink->BinkType & BINKALPHA)
			flags |= Display::E_ColourWriteEnable_Alpha;
		Display::Device()->SetColourWriteEnable(flags);
		
		Display::C_Viewport viewport;
		viewport.m_W = m_pBink->Width;
		viewport.m_H = m_pBink->Height;
		Display::Device()->SetViewport(viewport);

		F32 uEnd0 = 1.f;
		F32 vEnd0 = 1.f;
		F32 uEnd1 = 1.f;
		F32 vEnd1 = 1.f;
		C_MovieBase::CalculateUVRange(m_BinkTextureSet.bink_buffers, uEnd0, vEnd0, uEnd1, vEnd1);

		{
			MOVIE_TIMING("Draw_Bink_textures", MOVIE_DEFAULT_THRESHOLD, m_pBink->FrameNum, m_pBink->Frames);
			Draw_Bink_textures(
				&m_BinkTextureSet,
				m_pBink->Width, m_pBink->Height,
				0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
				m_Shaders, uEnd1, vEnd1);
		}

		#if !CFG_GOLD
		if(g_debugMovieDrawFrameInfo)
			DrawFrameInfo();
		#endif

		Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_All);
		Display::Device()->SetColourDepthStencilSurface(NULL, NULL);
		Display::Device()->InvalidateState();

		if(m_pBink->FrameNum < m_pBink->Frames || m_Looping)
		{
			MOVIE_TIMING("BinkNextFrame", MOVIE_DEFAULT_THRESHOLD, m_pBink->FrameNum, m_pBink->Frames);
			BinkNextFrame(m_pBink);
			if (m_pBink->ReadError)
			{
				// read error
				m_IsPlaying = false;
			}
		}
		else
		{
			m_IsPlaying = false;
		}

		m_FirstFrame = false;
	}
	else
	{
		if(m_FirstFrame)
		{
			Display::Device()->SetColourDepthStencilSurface(&m_RenderTexture.GetSurface(), NULL);
			Display::Device()->Clear(Display::E_Clear_Colour, Maths::RGBA(0, 0, 0, 255));

			#if !CFG_GOLD
			if(g_debugMovieDrawFrameInfo)
				DrawFrameInfo();
			#endif

			Display::Device()->SetColourDepthStencilSurface(NULL, NULL);
			Display::Device()->InvalidateState();

			m_FirstFrame = false;
		}
	}
}

void C_PS3Movie::DebugRender(F32& x, F32& y, const F32 s)
{
#if !CFG_GOLD
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
