#include "stdafx.h"
#include "Main.h"
#include <testsections/TestSectionLayer.h>
#include <testsections/TestSectionEmpty.h>
//#include <testsections/TestSectionMaths.h>
#include <framework/application/CoreSystems.h>
#include <system/artery/Artery.h>
#include <system/memory/MemoryTracker.h>
#include <system/perf/ProfilerBackend.h>
#include <system/perf/Profiler.h>
#include "TestSectionAnimation.h"

C_TestApp::C_TestApp() 
{
}

C_TestApp::~C_TestApp()
{
}

static Scheduler::Policy g_allCpuPolicy[ 8 ];
static Scheduler::Policy g_noPpuPolicy[ 8 ];

namespace Framework
{
	C_TestSectionEmpty::C_TestSectionEmpty(const Util::C_String& name) : C_TestSection(name) {}
	C_TestSectionEmpty::~C_TestSectionEmpty() {}
}

HRESULT C_TestApp::OnInitialise()
{
	Framework::C_TestSectionLayer* pTestSectionsLayer = new Framework::C_TestSectionLayer();
	pTestSectionsLayer->AddTestSection< Framework::C_TestSectionAnimation >( "Animation" );
	pTestSectionsLayer->AddTestSection< Framework::C_TestSectionEmpty >( "Empty" );
	
	pTestSectionsLayer->StraightToTestSection( "Animation" );
	//pTestSectionsLayer->StraightToTestSection( "Empty" );
	
	InsertLayer(pTestSectionsLayer);
		
	for ( U32 i = 0; i < 8; ++i )
	{
		g_allCpuPolicy[ i ] = 7;
		g_noPpuPolicy[ i ] = 7;
	}
	
	// Don't let the PPU process big tasks.
	g_noPpuPolicy[ 0 ] = 0;
	g_noPpuPolicy[ 1 ] = 0;
	
	Scheduler::PolicyHandle policyHandle1 = Scheduler::CreatePolicy( g_allCpuPolicy );
	Scheduler::PolicyHandle policyHandle2 = Scheduler::CreatePolicy( g_noPpuPolicy );
	
	assert( policyHandle1 == 0 );
	assert( policyHandle2 == 1 );
	
	
	Scheduler::Init();
	Scheduler::SetGroupPoolSize( 32 );
	Scheduler::jqEnableWorkers( Scheduler::JQ_CORE_ALL );
	
	Scheduler::Start();
	
	//ArteryStartup( true, "animationtestapp" );
	
	//Debug::NetworkStart();
	
	Perf::InitialiseNetworkConnection();
		
	return S_OK;
}

void C_TestApp::OnShutdown()
{
	Scheduler::Shutdown();
	
	//ArteryCleanup();
}

void C_TestApp::OnUpdate()
{
	//Framework::C_CoreSystems::Instance()->Update();
	
	

	CommonUpdate();
	
}

void C_TestApp::OnRender()
{
	if(IsFocusPaused())
	{
		Perf::Profiler::Instance().EndFrame();
		return;
	}

	if(BeginRender())
	{
		m_pDisplayDevice->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 128, 0, 255));

		CommonRender();

		EndRender();
	}
	Perf::Profiler::Instance().EndFrame();
}

#include <framework/application/ApplicationMain.inl>

