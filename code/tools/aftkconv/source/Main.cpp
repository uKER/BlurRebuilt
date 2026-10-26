#include "stdafx.h"
#include <system/debug/log/logger.h>
#include <system/debug/log/subscribers/SubscriberManager.h>
#include <system/debug/log/subscribers/SubscriberCommon.h>
#include <system/debug/log/subscribers/ConsoleSubscriber.h>
#include "Application.h"

Debug::I_LogSubscriber* g_pConsoleSubscriber = 0;

S32 g_exitCode;

int main(int argc, char**argv)
{
	g_exitCode = 0;

	C_Application app(
		"AftkConv",
		"",
		"tools.aftkConv",
		"",
		false);

    g_pConsoleSubscriber = new Debug::ConsoleSubscriber();
    g_pConsoleSubscriber->SetXMLParameter("debugoutput", "true");
    g_pConsoleSubscriber->SetXMLParameter("filter", "Info|Warn|Error|Fatal");

    Debug::GetRootLogger().AddSubscriber( g_pConsoleSubscriber );
    Debug::GetRootLogger().SetLevel( DebugLevel::Warn );
    int rtn = app.RunLite(argc, argv);
    Debug::GetRootLogger().RemoveSubscriber( g_pConsoleSubscriber );
    SAFE_DELETE_PTR(g_pConsoleSubscriber);

	g_exitCode = rtn;
    return g_exitCode;
}
