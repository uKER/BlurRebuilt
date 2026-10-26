#include "stdafx.h"

#include "tools_shared\ToolsApplication.h"
#include <System\DebugLog.h>
#include <System\Debug\Log\ExecuterFeedback.h>
#include "tools_shared\MiniDumper.h"
#include "tools_shared\ToolsAssert.h"
#include "tools_shared\MemoryDbg.h"
#include <system\Debug\Log\AutoConfigure.h>
#include <system/perf/ProfilerBackend.h>
#include <system/perf/ProfilerUtil.h>
#include "tools_shared/SchedulerConfig.h"
#ifndef JAM
#include "tools_shared\VersionStrings.h"
#else
#include <system/Debug/BuildStamp.h>
#endif

Tools::C_ToolsApplication* Tools::g_pMyApplication = NULL;

using namespace Tools;

/// constructor
C_ToolsApplication::C_ToolsApplication(const char *szName, const char *szSubtitle, const char *szLogger, const char *szDmpPath,	bool bExecuter)
	: m_log(Debug::GetLogger(szLogger))
	, m_pszApplicationName(szName)
	, m_pszApplicationSubtitle(szSubtitle)
	, m_pszDmpPath(szDmpPath ? szDmpPath : "") // [Blur2PC] was the \\bizsymbol dump share; empty writes beside the working folder
	, m_bExecuter(bExecuter)
{
	g_pMyApplication = this;
	m_pExecuter = NULL;
	m_bRunLocally = true;

	SetConsoleTitle(szName);
}

/// destructor
C_ToolsApplication::~C_ToolsApplication()
{
}

#ifdef COMPILER_VC
// this handler intercepts asserts in STL and CRT
// only ever seems to pass in NULL pointers
void myinvalid_param_handler( wchar_t const* expr, wchar_t const* func, const wchar_t* file, unsigned int line, uintptr_t ) 
{ 
	assert(0 && "CRT Invalid Parameter Handler");
}
#endif

/// main startup function of the tool
S32 C_ToolsApplication::Run(U32 argc, char **argv, bool bAutoConfigDebugLog)
{
	// Initialise jobqueue scheduler
	C_SchedulerConfig::Instance().Initialise();

	// Init perf 
	Perf::ProfilerCommon::Instance().RegisterCallback(Perf::g_DumpPerfCallbackFunction);

	// turn on memory leak reporting and tracking
	InitialiseMemoryDbg();

#ifdef COMPILER_VC
	// initialise network
	WSADATA	wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

	// hook in tools assert
	::g_pCoreAssertFunction = ToolsAssert;
	SetBizAssertHandler(BizToolsAssert);

	// initialise logging system
	Debug::AutoConfigureDebugLog();

#ifndef JAM
	// this will check that the version data is correct (i.e. that version.exe ran correctly)
	if (!strcmp(g_VersionStrings.szBuildDate+9,"ABCD.EF.GH (HH:MM)"))
	{
//		LOGERROR( m_log, "You need to put svnversion and version.exe in the pre-build and pre-link step of your tool.\n");
//		LOGERROR( m_log, "See Jan or Ken for details.\n");
//		exit(-1);
	}

	// display header
	LOGINFO( m_log, " %s - %s", m_pszApplicationName, m_pszApplicationSubtitle);
	LOGINFO( m_log, " %s", g_VersionStrings.szBuildDate);
	LOGDEBUG( m_log, "%s", g_VersionStrings.szBuildMachine);
	LOGDEBUG( m_log, "%s", g_VersionStrings.szSVNVersion);
#else
	// display header
	LOGINFO( m_log, " %s - %s", m_pszApplicationName, m_pszApplicationSubtitle);
	LOGINFO( m_log, " %s(%s)", Debug::GetBuildDate(), Debug::GetBuildTime());
	LOGDEBUG( m_log, "Build PC: %s (%s)", Debug::GetBuildPc(), Debug::GetBuildUser());
	LOGDEBUG( m_log, "Revision: %s", Debug::GetBuildRevision());
#endif

#ifdef PLATFORM_PC
	char computerName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD bufferSize = MAX_COMPUTERNAME_LENGTH;
	BOOL success = GetComputerName(computerName, &bufferSize);
	LOGINFO( m_log, "Running on: %s", computerName);
#endif

	// initialise the mdump.cpp exception handler
	MiniDumper::InitialiseExceptionHandler();

#ifdef COMPILER_VC
	// put in a hook that intercepts crt asserts that would normally call _invoke_watson
	_set_invalid_parameter_handler(myinvalid_param_handler);
#endif

	S32 returnValue;
	// initialise executer feedback or network feedback
	if (m_bExecuter)
	{
		// initialise executer feedback
		Debug::ExecuterFeedback feedback(m_pszApplicationName);
		m_pExecuter = &feedback;
		returnValue = Main(argc, argv, &feedback);
	}
	else
	{
		// connect to progress server
		Progress::UpdateNetwork progress;
		progress.Connect();
		returnValue = Main(argc, argv, &progress);
	}

	//End frame and print perf data
	Perf::ProfilerCommon::Instance().EndFrame();

	//shut down jobqueue scheduler
	C_SchedulerConfig::Instance().Shutdown();

	return returnValue;
}

/// main startup function of the tool (lite version which starts more quickly)
S32 C_ToolsApplication::RunLite(U32 argc, char **argv)
{
	// Initialise jobqueue scheduler
	C_SchedulerConfig::Instance().Initialise();

	// Init perf 
	Perf::ProfilerCommon::Instance().RegisterCallback(Perf::g_DumpPerfCallbackFunction);

	// turn on memory leak reporting and tracking
//	InitialiseMemoryDbg();

	// hook in tools assert
	::g_pCoreAssertFunction = ToolsAssert;
	SetBizAssertHandler(BizToolsAssert);

	// initialise logging system
	Debug::AutoConfigureDebugLog("", false);

	// display header
	LOGDEBUG( m_log, " %s - %s", m_pszApplicationName, m_pszApplicationSubtitle);
	LOGDEBUG( m_log, " %s(%s)", Debug::GetBuildDate(), Debug::GetBuildTime());
	LOGDEBUG( m_log, "Build PC: %s (%s)", Debug::GetBuildPc(), Debug::GetBuildUser());
	LOGDEBUG( m_log, "Revision: %s", Debug::GetBuildRevision());

#ifdef PLATFORM_PC
	char computerName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD bufferSize = MAX_COMPUTERNAME_LENGTH;
	BOOL success = GetComputerName(computerName, &bufferSize);
	LOGDEBUG( m_log, "Running on: %s", computerName);
#endif

	// initialise the mdump.cpp exception handler
	MiniDumper::InitialiseExceptionHandler();

#ifdef COMPILER_VC
	// put in a hook that intercepts crt asserts that would normally call _invoke_watson
	_set_invalid_parameter_handler(myinvalid_param_handler);
#endif

	S32 returnValue = Main(argc, argv, NULL);

	//End frame and print perf data
	Perf::ProfilerCommon::Instance().EndFrame();

	//shut down jobqueue scheduler
	C_SchedulerConfig::Instance().Shutdown();

	return returnValue;
}

Debug::ExecuterFeedback* C_ToolsApplication::GetExecuterFeedbackPtr()
{
	return m_pExecuter;
}

bool C_ToolsApplication::IsRunLocally() const
{
	return m_bRunLocally;
}

bool C_ToolsApplication::IsUsingExecuter() const
{
	return m_bExecuter;
}

const char* C_ToolsApplication::GetName() const
{
	return m_pszApplicationName;
}

const char* C_ToolsApplication::GetDumpPath() const
{
	return m_pszDmpPath;
}

const char* C_ToolsApplication::GetSubtitle() const
{
	return m_pszApplicationSubtitle;
}

Debug::Logger& C_ToolsApplication::GetLogger()
{
	return m_log;
}
