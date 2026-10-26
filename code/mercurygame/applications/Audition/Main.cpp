#include "StdAfx.h"
#include "Main.h"
#include <TestSections/TestSectionLayer.h>
#include "Audio/TestSectionAudio.h"
#include "framework\shared_layers\DebugLayer.h"

//#define ENABLE_MEMORY_TRACKER
//#define MEMORY_TRACKER_CALLSTACK 0

#define ENABLE_CLEAR_MEMORY_TO_KNOWN_VALUE

C_TestApp::C_TestApp() 
{
}

C_TestApp::~C_TestApp()
{
}

HRESULT C_TestApp::OnInitialise()
{
	Framework::C_TestSectionLayer* pTestSectionsLayer = new Framework::C_TestSectionLayer();
	InsertLayer(pTestSectionsLayer);
	
#ifndef CFG_GOLD
	Framework::C_DebugLayer* pDebugLayer = new Framework::C_DebugLayer();
	InsertLayer(pDebugLayer);
#endif

	pTestSectionsLayer->AddTestSection<C_TestSectionAudio>("Audio");

	pTestSectionsLayer->StraightToTestSection("Audio");

	return S_OK;
}

void C_TestApp::OnUpdate()
{
	CommonUpdate();
}

void C_TestApp::OnRender()
{
	if(BeginRender())
	{
		CommonRender();

		EndRender();
	}
}

#include <Framework/Application/ApplicationMain.inl>
