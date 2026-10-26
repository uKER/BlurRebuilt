#include "stdafx.h"
#include "Application.h"

#include <system/debuglog.h>
#include <system/debug/log/debuglog.h>
#include <system/debug/log/subscribers/consolesubscriber.h>

int main(int argc, char* argv[])
{
	C_Application app(
		"Sorcery FX",
		".FX to Sorcery schema converter",
		"tools.sorceryfx",
		"",
		false);
	
	Debug::ConsoleSubscriber *p1 = new Debug::ConsoleSubscriber();
	Debug::GetRootLogger().AddSubscriber( p1 );

	return app.Main(argc, argv, NULL);
}

