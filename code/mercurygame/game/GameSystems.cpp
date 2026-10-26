#include "StdAfx.h"
#include <LogFs/LogFs.h>
#include <OverlayFs/OverlayFs.h>
#include <GameFs/GameHelpers.h>

#include <fx/fx.h>

#include <GameSystems.h>
#include <SchedulerConfig.h>

#include <framework/application/LoadDebugVars.h>
#include <resource/Resource.h>

#ifdef PLATFORM_PC
#include <Game/PakHash/Pc/Gamedata.h>
#elif PLATFORM_XENON
#include <Game/PakHash/360/Gamedata.h>
#elif defined(PLATFORM_PS3) || defined(PLATFORM_SPU)
#include <Game/PakHash/Ps3/Gamedata.h>
#endif

#ifdef CFILESYSTEM_CREATE_FUNC_ENABLED
Sys::C_FileSystem* CFileSystemCreateFunc()
{
	Sys::C_FileSystem* pFs = new LOGFS_WRAPPER(Sys::C_FileSystem, "CFileSystem");
	return pFs;
}
#endif

namespace Game
{

// C_GameSystems

C_GameSystems* C_GameSystems::s_pInstance = NULL;

C_GameSystems::C_GameSystems()
{
	assert(s_pInstance == NULL);
	s_pInstance = this;

#ifndef CFG_GOLD
	gGameLoadDebugVariables = Framework::LoadDebugVars;
	gGameSaveDebugVariables = Framework::SaveDebugVars;
#endif

	m_pSchedulerConfig = new C_SchedulerConfig;
}

C_GameSystems::~C_GameSystems()
{
	SAFE_DELETE_PTR( m_pSchedulerConfig );

	assert(s_pInstance == this);
	s_pInstance = NULL;
}

C_GameSystems* C_GameSystems::Instance()
{
	assert(s_pInstance);
	return s_pInstance;
}

HRESULT C_GameSystems::Initialise(const Framework::C_CoreSystemsParams& params)
{
	HRESULT hr = S_OK;
		
	hr = C_CoreSystems::Initialise( params );
	if( FAILED(hr) )
		return hr;

	hr = m_pSchedulerConfig->Initialise();
	if( FAILED(hr) )
		return hr;

	LOGFS_INIT("./user/logfs");
	Sys::I_FileSystem& nativeFs = Sys::C_FileSystem::Instance();
	{
		m_pFileSystem = new LOGFS_WRAPPER(LogicalFs::C_OverlayFs, "-OverlayFs-");
		const Sys::C_FilePath kOverlayDstPath = "/";
		const Sys::C_FilePath kOverlaySrcPath = nativeFs.GetWorkingDirectory();

		LogicalFs::C_OverlayLayer::C_Params params;
		params.flags.layerType = LogicalFs::C_OverlayLayer::E_Type_Newer;
		params.flags.writable = true;
		m_pFileSystem->AddLayer(
			kOverlayDstPath,
			&nativeFs,
			kOverlaySrcPath,
			params,
			"CFileSystem");
		m_pFileSystem->SetWorkingDirectory("/");
		SetFileSystem(m_pFileSystem);

		//Mount paks

		//Not secure retail...
		{
			char* nonSecurePaks[] =		{ "cache/root.pak",	"cache/models.pak",	"cache/shaders-speed.pak"};
			char* nonSecureDstPaths[] =	{ "/",			"/models/",		"/graphics/shaders/speed/" };
			char* nonSecureSrcPaths[] =	{ "/",			"/models/",		"/"};
			for (S32 i = 0; i < lengthof(nonSecurePaks); ++i)
				LogicalFs::GameHelpers::MountPakLayer(
					nativeFs,
					(LogicalFs::C_OverlayFs&)*m_pFileSystem,
					nonSecurePaks[i], 
					nonSecureDstPaths[i], 
					nonSecureSrcPaths[i]);
		}
		//Not secure debug
		{
			char* nonSecurePaks[] =		{ "cache/gamedata.pak",	"cache/shaders-debug.pak"};
			char* nonSecureDstPaths[] =	{ "/gamedata/",		"/graphics/shaders/debug/" };
			char* nonSecureSrcPaths[] =	{ "/gamedata/",		"/"};
			for (S32 i = 0; i < lengthof(nonSecurePaks); ++i)
				LogicalFs::GameHelpers::MountPakLayer(
					nativeFs,
					(LogicalFs::C_OverlayFs&)*m_pFileSystem,
					nonSecurePaks[i], 
					nonSecureDstPaths[i], 
					nonSecureSrcPaths[i]);
		}

		//Secure retail...
/*
		if (!LogicalFs::GameHelpers::MountPakLayer(
			nativeFs,
			(LogicalFs::C_OverlayFs&)*m_pFileSystem,
			"cache/gamedata.pak", 
			"/gamedata/",
			"/gamedata/",
			g_gamedataHash))
			return S_FALSE;
*/
	}

	FX::SetFileSystem(m_pFileSystem);
	return hr;
}

void C_GameSystems::Shutdown()
{
	SetFileSystem(&Sys::C_FileSystem::Instance());
	SAFE_DELETE_PTR(m_pFileSystem);
	m_pSchedulerConfig->Shutdown();
	C_CoreSystems::Shutdown();
}

void C_GameSystems::Update()
{
	C_CoreSystems::Update();

	Resource::C_Manager::GetInstance()->Update();
}

C_SchedulerConfig& C_GameSystems::GetSchedulerConfig()
{
	return *m_pSchedulerConfig;
}

} // namespace Game
