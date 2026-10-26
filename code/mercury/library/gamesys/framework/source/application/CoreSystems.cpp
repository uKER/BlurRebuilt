#include "FrameworkPch.h"
#include "Framework/Application/CoreSystems.h"
#include "Framework/Application/LoadDebugVars.h"

#include <System/Debug/log/DebugLog.h>
#include <System/Debug/Log/AutoConfigure.h>
#include <System/Debug/Log/Subscribers/DebugViewSubscriber.h>
#include <System/Artery/Artery.h>
#include <System/Automation/Automation.h>
#include <System/Serialise.h>
#include <System/Memory.h>
#include <System/Debug/BuildStamp.h>
#include <system/serialise/FsPerfLogger.h>
#include <Resource/Resource.h>

#include "System/DataLogging/DataLogging.h"

#include <System/Io/Async.h>

#ifdef PLATFORM_PS3
#include <unistd.h>
#include <sys/paths.h>
#include <cell/sysmodule.h>
#include <netex/net.h>
#include <netex/libnetctl.h>
#include <sysutil/sysutil_gamecontent.h>
#endif

#ifndef CFG_GOLD
#define GAMESYS_USE_ARTERY
#define GAMESYS_USE_DEBUGVARS
#define GAMESYS_USE_MEMORYTRACKER
#define GAMESYS_USE_AUTOMATION
#define GAMESYS_USE_DATALOG
#endif

#ifdef PLATFORM_PS3//until it finds a better home
namespace PlatformPS3
{
// Note: This may appear to be hard-coded, but it is actually populated in code, 
// if the code fails, which it will do on anything other than a disc with a valid
// PARAM.SFO, the default here will be used.
	char g_ps3_titleId[ 128 ] = { "BLUS30295\0" };
}

bool gDisablePS3InstallerFromCode = false;  // Used by code to ensure installer disabled - i.e. automation

#endif

namespace Framework
{

// C_CoreSystemsParams



C_CoreSystemsParams::C_CoreSystemsParams()
{
	m_Name							= "TestApp";
#if defined(PLATFORM_PS3)
	unsigned int type;
	unsigned int attrib;
	CellGameContentSize size;
	char dirName[ CELL_GAME_DIRNAME_SIZE ];
	memset(&size, 0x00, sizeof(CellGameContentSize));
	
	
	cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
	

	int ret = cellSysmoduleLoadModule( CELL_SYSMODULE_SYSUTIL_GAME );
	if ( ret == CELL_OK ) 
	{
		char contentInfoPath[ CELL_GAME_PATH_MAX ];
		char usrdirPath[ CELL_GAME_PATH_MAX ];
		ret = cellGameBootCheck( &type, &attrib, &size, dirName );
		if(ret >= 0)
		{
			ret = cellGameGetParamString( CELL_GAME_PARAMID_TITLE_ID, PlatformPS3::g_ps3_titleId, 128 );
			ret = cellGameContentPermit( contentInfoPath, usrdirPath );
		}
		else
		{
			// Shouldn't get here, but if it does, then complete the access preparing function pair with 'cellGameBootCheck'
			// Failure to do so will prevent other access preparing functions to fail i.e. Game Save & Load.
			ret = cellGameContentPermit( contentInfoPath, usrdirPath );
		}

		bool runFromHDD = false;
		#ifndef CFG_GOLD
		{
			int numArgs			= getargc();
			char** const args	= getargv();

			for (int i=1; i < numArgs; i++)
			{
				if (!_stricmp(args[i], "--hdd"))
				{
					LOGWARN(g_Log, "--HDD detected - running from HDD with install disabled");

					gDisablePS3InstallerFromCode = true;

					runFromHDD = true;
					strcpy(dirName, PlatformPS3::g_ps3_titleId);
					break;
				}
				else if (!_stricmp(args[i], "--nohdd"))
				{
					runFromHDD = false;
					break;
				}
			}
		}
		#endif
		if (runFromHDD || strstr( usrdirPath, SYS_DEV_HDD0 ) )
		{
			//Use HDD
			m_WorkingDir = SYS_DEV_HDD0; // dev_hhd0
			m_WorkingDir += "/game/";
			m_WorkingDir += dirName;
			m_WorkingDir += "/USRDIR";
			if (runFromHDD)
				m_WorkingDir += "/hdd";
		}
		else if (strstr( usrdirPath, SYS_DEV_BDVD ) )
		{
			//Use BluRay
			m_WorkingDir = SYS_DEV_BDVD; // dev_bdvd
			m_WorkingDir += "/PS3_GAME/USRDIR";
		}
		else if (strstr( usrdirPath, SYS_APP_HOME ))
		{
			//Run over the network.
			m_WorkingDir = SYS_APP_HOME; //app_home
		}
	}
#else
	m_WorkingDir					= "";
#endif
}

C_CoreSystemsParams::~C_CoreSystemsParams()
{
}

// C_CoreSystems

C_CoreSystems* C_CoreSystems::s_pInstance = NULL;

C_CoreSystems::C_CoreSystems() :
	m_Initialised(false),
	m_ArteryStartupInitialisedNetwork(false),
	m_Name(""),
	m_pFileSystem(NULL)
{
	#ifdef GAMESYS_USE_DEBUGLOG
	m_pLogFileSubscriber = NULL;
	#endif

	assert(s_pInstance == NULL);
	s_pInstance = this;

	Sys::SetDefaultFileSystem(&Sys::C_FileSystem::Instance());
	m_pFileSystem = Sys::GetDefaultFileSystem();
	m_pMemoryFs = NULL;
}

C_CoreSystems::~C_CoreSystems()
{
	assert(s_pInstance == this);
	s_pInstance = NULL;
}

//! Has the core sytems been initialised?
const bool C_CoreSystems::IsInitialised() const
{
	return m_Initialised;
}

//! Returns the name.
const Util::C_String& C_CoreSystems::GetName() const
{
	return m_Name;
}

//! Has the network already been initialised by artery.
bool C_CoreSystems::NetworkInitialised(void) const
{
	return m_ArteryStartupInitialisedNetwork;
}

HRESULT C_CoreSystems::Initialise(const C_CoreSystemsParams& params)
{
	HRESULT hr = S_OK;

	assert(!m_Initialised);
	m_Initialised = true;

	m_Name = params.m_Name;
	
#ifdef PLATFORM_PS3
	// davidw - load file system prx module before first file access. not sure if this is the correct place for this.
	cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
#endif

	IoInitialise();

	Debug::AutoConfigureDebugLog();

	m_pFileSystem->SetWorkingDirectory(params.m_WorkingDir);

	#ifdef GAMESYS_USE_DEBUGLOG
	m_pLogFileSubscriber = new Debug::LogFileSubscriber("/DebugLog.txt");
	Debug::GetRootLogger().AddSubscriber(m_pLogFileSubscriber);
	#endif

	#ifdef GAMESYS_USE_ARTERY
	hr = StartupArtery(params);
	if(FAILED(hr))
	{
		LOGERROR(g_Log, "Failed to start up artery\n");
		return E_FAIL;
	}
	#endif

	m_pResourceManager = new Resource::C_Manager();
	
	return S_OK;
}

void C_CoreSystems::Shutdown()
{
	//assert(m_Initialised);
	m_Initialised = false;

	SAFE_DELETE_PTR( m_pResourceManager );

	// Shut down the type registry, this was created when static variables are initialised.
	Resource::C_TypeRegistry::DeleteInstance();

	#ifdef GAMESYS_USE_ARTERY
	ShutdownArtery();
	#endif

	IoShutdown();

	//Prevent shutdown order issues.
	Sys::C_FsPerfLogger::Shutdown();
}

void C_CoreSystems::Update()
{
	#ifdef GAMESYS_USE_DEBUGVARS
	Debug::NetworkUpdate();
	#endif

	#ifdef GAMESYS_USE_MEMORYTRACKER
	Memory::Update();
	#endif

	#ifdef GAMESYS_USE_AUTOMATION
	Automation::NetworkUpdate();
	#endif

	#ifdef GAMESYS_USE_DATALOG
	DataLog::NetworkUpdate();
	#endif
}

HRESULT C_CoreSystems::StartupArtery(const C_CoreSystemsParams& params)
{
#ifdef GAMESYS_USE_ARTERY
	HRESULT hr = S_OK;

	#ifdef PLATFORM_PS3
	if((hr = cellSysmoduleLoadModule(CELL_SYSMODULE_NET)) != 0)
	{
		LOGERROR(g_Log, "Failed calling cellSysmoduleLoadModule with error code %i\n", hr);
		return E_FAIL;
	}
	if((hr = sys_net_initialize_network()) != 0)
	{
		LOGERROR(g_Log, "Failed calling sys_net_initialize_network with error code %i\n", hr);
		return E_FAIL;
	}
	if((hr = cellNetCtlInit()) != 0)
	{
		LOGERROR(g_Log, "Failed calling cellNetCtlInit with error code %i\n", hr);
		sys_net_finalize_network();
		return E_FAIL;
	}
	#endif

	
	#if defined(PLATFORM_PC)
	WSADATA wsaData;
	if((hr = WSAStartup(WINSOCK_VERSION, &wsaData)) != 0)
	{
		LOGERROR(g_Log, "Failed calling WSAStartup with error code %i\n", hr);
		return E_FAIL;
	}
	#endif
	
	// artery system has started the network stack so no need for the game to do this.
	m_ArteryStartupInitialisedNetwork = true;

	Util::C_String tmp = "";
	tmp = tmp + Debug::GetBuildUser() + "-" + params.m_Name;

	if(!ArteryStartup(true, tmp.Chars()))
	{
		LOGERROR(g_Log, "Failed calling ArteryStartup\n");
		return E_FAIL;
	}
#endif

#ifdef GAMESYS_USE_DEBUGVARS
	gGameLoadDebugVariables = LoadDebugVars;
	gGameSaveDebugVariables = SaveDebugVars;
	Debug::NetworkStart();
	Debug::GetRootLogger().AddSubscriber( new Debug::DebugViewSubscriber() );
#endif

#ifdef GAMESYS_USE_AUTOMATION
	Automation::InitialiseNetworkConnection();
#endif

#ifdef GAMESYS_USE_MEMORYTRACKER
	Memory::InitialiseNetworkConnection();
#endif

#ifdef GAMESYS_USE_DATALOG
	DataLog::Initialise();
#endif

	return S_OK;
}

void C_CoreSystems::ShutdownArtery()
{
#ifdef GAMESYS_USE_ARTERY
	ArteryCleanup();

	#ifdef PLATFORM_PS3
	cellNetCtlTerm();
	sys_net_finalize_network();
	cellSysmoduleUnloadModule(CELL_SYSMODULE_NET);
	#endif

	#if defined(PLATFORM_PC)
	WSACleanup();
	#endif
#endif
}

void C_CoreSystems::SetFileSystems(Sys::I_FileSystem* pFileSystem, LogicalFs::C_MemoryFs* pMemoryFs)
{
	if (!pFileSystem)
	{
		BIZ_ASSERT_FAIL("Attempted to set null file system!");
		return;
	}

	m_pFileSystem = pFileSystem;
	m_pResourceManager->SetFileSystems(m_pFileSystem, pMemoryFs);
	Sys::SetDefaultFileSystem(m_pFileSystem);
	Display::gFileSystem = m_pFileSystem;
}

}; // namespace Framework
