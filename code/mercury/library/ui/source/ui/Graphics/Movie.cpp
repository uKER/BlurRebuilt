#include "stdafx.h"

#include <ui/UiInterface.h>
#include "ui/Graphics/movie.h"
#include "ui/Game/UIScreenControl.h"
#include "ui/Game/Gadgets/ButtonController.h"
#include "ui/Game/Gadgets/ButtonMessages.h"
#include <system\misc\StringUtil.h>

#ifdef PLATFORM_PS3
#include <cell/cell_fs.h>
#include <sysutil/sysutil_bgmplayback.h>
#endif

#ifdef CFG_ALLOW_OPTIMIZE_OFF_RENDER_MOVIE
#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF_RENDER_MOVIE
#endif

Debug::Bool g_debugMovieEnableSound("Graphics.Render.Movie.EnableSound", true);

#ifdef PLATFORM_PS3
extern void C_GameFileSystem_UpdateSystemMessages();
#endif

namespace UI
{

char *gHddDirectorySearchPath = NULL;

Movie gMovie[MAX_MOVIE_SLOTS];

Movie::Movie() :
		m_Active(false),
		m_bUseSound(true),
		m_bIsBackgroundMovie(false),
		m_bNeedToRestoreBackgroundMusic(false),
		m_pMovie(NULL)
{
}

Movie::~Movie()
{
	assert(!m_pMovie && "The movie has not been destroyed");
}

void Movie::Create
(
	const S32	index, 
	const char*	filename, 
	const bool	in_memory, 
	const U32	io_memory_size, 
	const bool	bStillRenderTheWorld, 
	const bool	bBackgroundMovie, 
	const S32	volume
)
{
	m_Index = index;
	m_Volume = volume;

	RESOURCE_MEMORY_ID("Movie");

	if(m_pMovie)
	{
		LOGWARN(g_Log, "Trying to create a movie %s in slot %i where movie already exists\n", filename, index);
		return;
	}

	m_pMovie = new ::Movie::C_Movie();

	m_MovieParams.Reset(); // Make sure movie params are reset to defaults

	m_MovieParams.m_MemoryFlags = ::Movie::E_MovieMemoryFlags_System; // Always allocate out of system memory

	if(io_memory_size)
	{
		m_MovieParams.m_CreateFlags |= ::Movie::E_MovieCreateFlags_SetBufferIOSize;
		m_MovieParams.m_BinkIOSize = io_memory_size;
	}
	else
	if(in_memory)
	{
		m_MovieParams.m_CreateFlags = ::Movie::E_MovieCreateFlags_LoadIntoMemory;
	}

	// PR - #23103 - different versions of the movie for 50 and 60Hz
	S32 sBufferSize = 512;

	char* pFileName = new char[ sBufferSize ];
	assert( pFileName );

	sprintf_s( pFileName, sBufferSize, "%s", filename );

	// rename for 50Hz versions?
	if ( Display::Device()->GetRefreshRate() == Display::E_RefreshRate_50HZ )
		UiInterface()->RenameMovieFor50Hz( filename, pFileName, sBufferSize );
	else
	if ( Display::C_Device::GetCpuSpeed() == Display::E_CpuSpeed_Low )
		UiInterface()->RenameMovieForSmallNTSC( filename, pFileName, sBufferSize );

#ifdef PLATFORM_PS3
	char hddFileName[256];
	if ( gHddDirectorySearchPath )
	{
		strcpy( hddFileName, gHddDirectorySearchPath );
		strcat( hddFileName, "/image/" );
		strcat( hddFileName, pFileName );
		char searchStr[] = "/USRDIR/";
		U32 lower = (U32)((char *)strstr(hddFileName, searchStr) - (char *)hddFileName) + strlen(searchStr);
//		printf("lower %d\n",lower);
		for (unsigned int w = 0 ; w < strlen(hddFileName) ; w++)
		{
			if (hddFileName[w] == '\\') hddFileName[w] = '/';
			if (w > lower) hddFileName[w] = tolower( hddFileName[w] );
		}
		
//		printf("%s\n",hddFileName);
		bool succeeded = false;
		do 
		{
			int fd = 0;
			int ret = cellFsOpen( hddFileName, CELL_FS_O_RDONLY, &fd, NULL, 0 );
			if (ret == CELL_FS_SUCCEEDED)
			{
				cellFsClose( fd );
				if(!m_pMovie->Open(hddFileName, m_MovieParams))
				{
					LOGWARN(g_Log, "Movie player failed to open %s\n", hddFileName);
					sys_timer_usleep( 30000 );

				}
				else
				{
					succeeded = true;
				}
			}
			else
			{
				Sys::C_FilePath discName = Sys::C_FileSystem::Instance().GetWorkingDirectory() + "\\" + pFileName;
				if(!m_pMovie->Open(discName.Chars(), m_MovieParams))
				{
					LOGWARN(g_Log, "Movie player failed to open %s\n", discName.Chars());
					sys_timer_usleep( 30000 );
				}
				else
				{
					succeeded = true;
				}
				
			}

			C_GameFileSystem_UpdateSystemMessages();

			if ( Display::Device()->HaveThreadOwnership() )
			{
				bool inScene = Display::Device()->InScene();
				if (inScene) Display::Device()->EndScene();
				{
					Display::Device()->BeginScene();
					UiInterface()->PollPlatformCallbacks();
					sys_timer_usleep( 10000 );
					Display::Device()->EndScene();
					Display::Device()->Present();
				}		

				if (inScene) Display::Device()->BeginScene();
			}

#if 0
			cellGcmResetFlipStatus();
			cellGcmSetFlip(0);
			cellGcmFlush();
			cellGcmSetWaitFlip();
#endif
		//	
		
		} while (!succeeded);
	}
	else
	{
		Sys::C_FilePath discName = Sys::GetDefaultFileSystem()->GetSystemFilePath( pFileName );
		if(!m_pMovie->Open(discName.Chars(), m_MovieParams))
		{
			LOGWARN(g_Log, "Movie player failed to open %s\n", discName.Chars());
		}
	}
#elif defined (PLATFORM_PC)

	m_bIsBackgroundMovie = bBackgroundMovie;

	if(!m_pMovie->Open(filename, m_MovieParams))
	{
		LOGWARN(g_Log, "Movie player failed to open %s\n", filename);
	}

#else


	Sys::C_FilePath discName = Sys::C_FileSystem::Instance().GetWorkingDirectory() + "\\" + pFileName;
	if(!m_pMovie->Open(discName.Chars(), m_MovieParams))
	{
		LOGWARN(g_Log, "Movie player failed to open %s\n", pFileName);
	}
	

#endif

	SAFE_DELETE_PTR( pFileName );

#ifndef PLATFORM_PS3
	Util::C_String myString;
	myString.Printf("MovieTarget%02d",index);
#endif // PLATFORM_PS3

	m_pMovie->SetLooping(true);

	m_Active = true;

	m_bStillRenderTheWorld = bStillRenderTheWorld;
	m_bIsBackgroundMovie = bBackgroundMovie;

	if (!m_bIsBackgroundMovie)
	{
#ifdef PLATFORM_PS3
		cellSysutilDisableBgmPlayback();
#endif
		m_bNeedToRestoreBackgroundMusic = true;
	}

	// Set volume.
	m_pMovie->SetVolume( m_Volume );
}

void Movie::Destroy()
{
	RESOURCE_MEMORY_ID("Movie");

	if(m_pMovie)
	{
		m_pMovie->Close();
		delete m_pMovie;
		m_pMovie = NULL;
	}

	m_Active = false;

	if (m_bNeedToRestoreBackgroundMusic)
	{
#ifdef PLATFORM_PS3
		cellSysutilEnableBgmPlayback();
#endif
		m_bNeedToRestoreBackgroundMusic = false;
	}
 }

void Movie::SetTrackIDsToPlay(const U32 track0, const U32 track1, const U32 track2, const U32 track3, const U32 track4, const U32 track5)
{
	m_MovieParams.m_TrackIDsToPlay[0] = track0;
	m_MovieParams.m_TrackIDsToPlay[1] = track1;
	m_MovieParams.m_TrackIDsToPlay[2] = track2;
	m_MovieParams.m_TrackIDsToPlay[3] = track3;
	m_MovieParams.m_TrackIDsToPlay[4] = track4;
	m_MovieParams.m_TrackIDsToPlay[5] = track5;
}

void Movie::Pause(const bool pause)
{
	m_Active = !pause;
}

void Movie::Looping(const bool loop)
{
	if(m_pMovie)
	{
		m_pMovie->SetLooping(loop);
	}
}

bool Movie::Update()
{
	if (!m_Active)
		return true;

	if(!m_pMovie || !m_pMovie->IsCreated())
	{
		// [Blur2PC] no Bink on PC: a movie that failed to open counts as finished, otherwise
		// screens that wait for E_UiMsg_MovieFinished (SpMovieScreen, attract) never advance.
		_UI_MESSAGE1( E_UiMsg_MovieFinished, m_Index );
		m_Active = false;
		return false;
	}

	#ifdef PLATFORM_PC
	if(!Display::Device()->GetValid())
		return true;
	#endif

	Display::Device()->AcquireThreadOwnership();

	bool wasInScene = Display::Device()->InScene();

	if(!wasInScene)
	{
		if(!Display::Device()->BeginScene())
		{
			LOGERROR(g_Log, "Failed to begin scene when rendering the movie\n");
			Display::Device()->ReleaseThreadOwnership();
			return false;
		}
	}

	m_pMovie->Render();

	if(!wasInScene)
	{
		Display::Device()->EndScene();
	}

	Display::Device()->ReleaseThreadOwnership();

	// [LloydW] If looping the video let bink seek back to beginning rather than forcing a rewind.
	// Also it seems the the movie finished message has to be sent at the end of the video even if its looping.
	if(m_pMovie->HasReachedEnd())
	{
		LOGDEBUG( g_Log,  "***** Sending message");

		_UI_MESSAGE1( E_UiMsg_MovieFinished, m_Index );

		if ( !m_pMovie->GetLooping() )
			m_Active = false;
	}

	return true;
}

void Movie::DebugRender(F32& x, F32& y, const F32 s)
{
	#if !defined(CFG_GOLD) && defined(PLATFORM_PS3)
	if(m_pMovie)
		m_pMovie->DebugRender(x, y, s);
	#endif
}

UITextureHandle Movie::GetTexture( ) const
{
#if defined( _GENESIS_ )
	return m_pMovie ? m_pMovie->GetTexture() : NULL;
#else
	return m_pMovie ? m_pMovie->GetTexture() : NULL;
#endif // _GENESIS_
}

}; // namespace UI
