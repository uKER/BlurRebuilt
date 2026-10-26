#include "StdAfx.h"
#include "Main.h"
#include <TestSections/TestSectionLayer.h>
#include <TestSections/TestSectionSkyDome.h>
#include <TestSections/TestSectionNetTransport.h>
#include <TestSections/TestSectionMediatorSession.h>
#include <TestSections/TestSectionMigration.h>
#include <TestSections/TestSectionTemplate.h>
#include <TestSections/TestSectionNetState.h>
#include <system/debug/BuildStamp.h>

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

	if (!stricmp((const char*)Debug::GetBuildUser(), "orenault"))
	{
		pTestSectionsLayer->AddTestSection<C_TestSectionMediatorSession>("MediatorSession");
		pTestSectionsLayer->AddTestSection<C_TestSectionNetTransport>("NetworkTransport");
		pTestSectionsLayer->AddTestSection<C_TestMigrationSection>("NetMigration");
	}

	if (!stricmp((const char*)Debug::GetBuildUser(), "dpearce"))
	{
		pTestSectionsLayer->AddTestSection<C_TestSectionMediatorSession>("MediatorSession");
		pTestSectionsLayer->AddTestSection<C_TestStateSection>("NetState");
	}

	//pTestSectionsLayer->AddTestSection<C_TestSectionTemplate>("Template");
	//pTestSectionsLayer->AddTestSection<Framework::C_TestSectionSkyDome>("SkyDome");
	
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
