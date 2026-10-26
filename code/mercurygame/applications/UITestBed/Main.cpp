#include "StdAfx.h"
#include "Main.h"
#include <framework/shared_layers/DebugLayer.h>
#include <TestSections/TestSectionLayer.h>
#include "UI/SectionUI.h"
#include "Xt/XtGame.h"

#define ENABLE_MEMORY_TRACKER
#define MEMORY_TRACKER_CALLSTACK 1

XT_APPLICATION
	XT_DEPENDS_ON( UI )
	XT_DEPENDS_ON( GameUI )
XT_END_APPLICATION

#ifdef PLATFORM_XENON
#include <xtitleidbegin.h>
	XEX_TITLE_ID(0xFFFF0055)	// test ...
	//XEX_TITLE_ID(0x41560845)	// AMAX
#include <xtitleidend.h> 
#endif

C_TestApp::C_TestApp() 
{
	#if defined ( PLATFORM_XENON )
	Sys::C_FileSystem::Instance().SetWorkingDirectory( "GAME:" );
	#elif defined (PLATFORM_PS3)
	Sys::C_FileSystem::Instance().SetWorkingDirectory( "/app_home" );
	#elif defined (PLATFORM_PC)
	Sys::C_FileSystem::Instance().SetWorkingDirectory( "Z:\\MercuryGame\\image\\PC" );
	#endif

	//m_Name = "UITestBed";
}

C_TestApp::~C_TestApp()
{
	//
}

HRESULT C_TestApp::OnInitialise()
{
	Framework::C_TestSectionLayer* pTestSectionsLayer = new Framework::C_TestSectionLayer();
	InsertLayer( pTestSectionsLayer );

	//Framework::C_DebugLayer* pDebugLayer = new Framework::C_DebugLayer();
	//InsertLayer( pDebugLayer );

	pTestSectionsLayer->AddTestSection<C_SectionUI>( "UI" );
	pTestSectionsLayer->StraightToTestSection( "UI" );

	XtInitialise();

	return S_OK;
}

void C_TestApp::OnShutdown()
{
	XtShutdown();
}

void C_TestApp::OnUpdate()
{
	XtNetworkUpdate();
	CommonUpdate();
}

void C_TestApp::OnRender()
{
	if ( BeginRender() )
	{
		CommonRender();
		EndRender();
	}
}

#include <Framework/Application/ApplicationMain.inl>
