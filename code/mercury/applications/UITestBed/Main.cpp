#include "StdAfx.h"
#include "Main.h"

#include "Samples/Section/SectionEmpty.h"
//#include "Samples/Section/SectionSkyDome.h"
//#include "Samples/Section/SectionMovie.h"
#include "UI/SectionUI.h"

#include "Xt/XtGame.h"

#include <framework/application/LoadDebugVars.h>

XT_APPLICATION
	XT_DEPENDS_ON( UI )
	XT_DEPENDS_ON( GameUI )
XT_END_APPLICATION

#ifdef PLATFORM_XENON
#include <xtitleidbegin.h>
	XEX_TITLE_ID(0xFFFF0055)	// test ...
#include <xtitleidend.h> 
#endif

#ifdef PLATFORM_PS3
void PreStaticInit();
#endif

namespace Samples
{

C_Sample::C_Sample()
{
	#if defined (PLATFORM_PC)
	Sys::C_FileSystem::Instance().SetWorkingDirectory( "image" );
	#elif defined (PLATFORM_XENON)
	Sys::C_FileSystem::Instance().SetWorkingDirectory("GAME:\\image");
	#elif defined (PLATFORM_PS3)
	Sys::C_FileSystem::Instance().SetWorkingDirectory("/app_home/image");
	#endif

	m_Name = "UITestBed";
	
	//
}

C_Sample::~C_Sample()
{
	//
}

bool C_Sample::MyCreate()
{
	XtInitialise();

	//AddSection(new C_SectionLauncher<C_SectionMovie>("Movie"));
	//AddSection(new C_SectionLauncher<C_SectionSkyDome>("SkyDome"));
	//AddSection(new C_SectionLauncher<C_SectionEmpty>("Empty"));

	C_SectionLauncherBase* pSection = new C_SectionLauncher< C_SectionUI >( "UI" );
	AddSection( pSection );
	pSection->Create();
	//C_Section* pUISection = pSection->CreateSection();

	m_Input.Reset();
	m_Input.ButtonDn( E_Button_RtPad );

	pSection->Update( m_Input, m_Timer );

	return true;
}

void C_Sample::MyDestroy()
{
	XtShutdown();
}

void C_Sample::MyUpdate()
{
	//
}

#if defined( PLATFORM_PC )
C_Sample* g_pSample = NULL;
#endif

}; // namespace Samples

#if defined( PLATFORM_PC )
using namespace Samples;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return g_pSample->MsgProc(hWnd, message, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	g_pSample = new C_Sample();

	g_pSample->m_DeviceParams.m_pName		= "MinimalPC";
	g_pSample->m_DeviceParams.m_hInstance	= hInstance;
	g_pSample->m_DeviceParams.m_CmdShow		= nCmdShow;

	if(g_pSample->Create())
	{
		g_pSample->Run();
	}
	else
	{
		LOGERROR(Samples::g_Log, "Failed to create the sample\n");

		g_pSample->Destroy();
	}

	delete g_pSample;

	return 0;
}
#else
#if defined( PLATFORM_XENON )
void __cdecl main()
#elif defined( PLATFORM_PS3 )
int main(int argc, char *argv[])
#endif
{
#ifdef PLATFORM_PS3
	PreStaticInit();
#endif

	Samples::C_Sample sample;
	
	sample.ReadConfig("Config.ini");

	Util::C_String name( "UITestBed" );
	sample.SetName( name );

	gGameLoadDebugVariables = Framework::LoadDebugVars;
	gGameSaveDebugVariables = Framework::SaveDebugVars;

	if(sample.Create())
	{
		sample.Run();
	}
	else
	{
		LOGFATAL(Samples::g_Log, "Failed to create the sample\n");
	}
}
#endif // PLATFORM_PC
