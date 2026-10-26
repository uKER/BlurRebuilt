#include <core/core.h>
#include <tools_shared/ToolsApplication.h>
#include <core/containers/String.h>
#include <system/debuglog.h>
#include <system/debug/log/DebugLog.h>
#include <system/debug/log/Subscribers/LogFileSubscriber.h>
#include <system/debug/log/Subscribers/ConsoleSubscriber.h>

#include "jam.h"

int jammain( int argc, char **argv );

Debug::Logger &g_Log = Debug::GetLogger("jam");

class C_JamApp : public Tools::C_ToolsApplication
{
public:
	C_JamApp() :
		Tools::C_ToolsApplication(
			"JAM",									// application name
			"JAM Build Utility",					// application description
			"tools.jam",							// application logger
			"")
	{
	}

	/// main function of the application
	int Main(int argc, char **argv, Progress::I_ProgressBar *pProgress)
	{
		return jammain(argc, argv);
	}
};

int main(int argc, char**argv)
{
	C_JamApp myApp;

	// setup logfile
	Debug::LogFileSubscriber *p = new Debug::LogFileSubscriber("buildlog.txt");
	p->SetXMLParameter("format", "%t %c\n");
	Debug::ConsoleSubscriber *p2 = new Debug::ConsoleSubscriber();
	p2->SetXMLParameter("debugoutput", "true");
	p2->SetXMLParameter("filter", "Info|Warn|Error|Fatal");

	Debug::GetRootLogger().AddSubscriber( p );
	Debug::GetRootLogger().AddSubscriber( p2 );
	Debug::GetRootLogger().SetLevel( DebugLevel::Debug );	// if connected to a logging host, default to Debug
	Debug::GetLogger("system.scheduler").SetLevel( DebugLevel::Info );

	int rtn = myApp.RunLite(argc, argv);

	Debug::GetRootLogger().RemoveSubscriber( p );

	delete p;

	return rtn;
}
