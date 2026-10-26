#include "stdafx.h"
#include "Application.h"

#include <system/debug/log/debuglog.h>
#include <system/debug/log/AutoConfigure.h>
#include <system/debug/log/Subscribers/ConsoleSubscriber.h>
#include <system/debug/log/Subscribers/SocketSubscriber.h>


int main(int argc, char* argv[])
{
	C_Application app(
		"FxExporter",
		"Bizarre Creations .FX file compiler",
		"tools.fxb",
		"",
		false);

	return app.Main(argc, argv, NULL);
}

