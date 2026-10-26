
#include "StdAfx.h"
#include "Main.h"
#include "Samples/Section/SectionEmpty.h"
#include "Samples/Section/SectionNetState.h"
//#include "Samples/Section/SectionNetGame.h"
#include "Samples/Section/SectionNetConnect.h"
#include "netServices/netServices.h"
#include "netApp/netAppServices/netAppServices_Components.h"
#include "netApp/netAppServices/netAppServices_Manager.h"

#include <Xt/xtGame.h>
XT_APPLICATION
	XT_DEPENDS_ON( UI )
XT_END_APPLICATION

namespace Samples
{

C_Sample::C_Sample()
{
	Sys::C_FileSystem::Instance().SetWorkingDirectory("GAME:\\Image");
	m_Name = "NetTest";
	
	netServices::initialise();
}

C_Sample::~C_Sample()
{
	netAppServices::Components::destroyInstance();
	netAppServices::Manager::destroyInstance();
	netServices::shutdown();
}

bool C_Sample::MyCreate()
{	
	XtInitialise();

	AddSection(new C_SectionLauncher<C_SectionNetConnect>("NetConnect"));
	AddSection(new C_SectionLauncher<C_SectionNetState>("NetState"));
	//m_pSection->EnterSection();

	return true;
}

void C_Sample::MyDestroy()
{
	XtShutdown();
}

}; // namespace Samples

void __cdecl main()
{
	Samples::C_Sample sample;
	
	sample.ReadConfig("Config.ini");

	if(sample.Create())
	{
		sample.Run();
	}
	else
	{
		LOGFATAL(Samples::g_Log, "Failed to create the sample\n");
	}
}
