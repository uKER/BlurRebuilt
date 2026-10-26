#include "stdafx.h"
#include <system/debug/log/logger.h>
#include <system/debug/log/subscribers/SubscriberManager.h>
#include <system/debug/log/subscribers/SubscriberCommon.h>
#include <system/debug/log/subscribers/ConsoleSubscriber.h>
#include "Application.h"
/*
int main(int argc, char* argv[])
{
	C_Application app(
		"BizPak",
		"",
		"tools.bizpak",
		"",
		false);
	return app.Run(argc, argv);
}
*/

Debug::I_LogSubscriber* g_pConsoleSubscriber = 0;

int main(int argc, char**argv)
{
	C_Application app(
		"BizPak",
		"",
		"tools.bizpak",
		"",
		false);

    g_pConsoleSubscriber = new Debug::ConsoleSubscriber();
    g_pConsoleSubscriber->SetXMLParameter("debugoutput", "true");
    g_pConsoleSubscriber->SetXMLParameter("filter", "Info|Warn|Error|Fatal");

    Debug::GetRootLogger().AddSubscriber( g_pConsoleSubscriber );
    Debug::GetRootLogger().SetLevel( DebugLevel::Debug );
    Debug::GetLogger("tools.bizpak").SetLevel( DebugLevel::Info );
    int rtn = app.RunLite(argc, argv);
    Debug::GetRootLogger().RemoveSubscriber( g_pConsoleSubscriber );
    SAFE_DELETE_PTR(g_pConsoleSubscriber);
    return rtn;
}
