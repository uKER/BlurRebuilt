#include "StdAfx.h"
#include "Main.h"
#include <TestSections/TestSectionLayer.h>
#include <TestSections/TestSectionSkyDome.h>
#include "TestSections/TestSectionTemplate.h"
//#include "TestSections/TestSectionDisplay2.h"

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

	pTestSectionsLayer->AddTestSection<C_TestSectionTemplate>("Template");
	pTestSectionsLayer->AddTestSection<Framework::C_TestSectionSkyDome>("SkyDome");
	//pTestSectionsLayer->AddTestSection<C_TestSectionDisplay2>("Display2");

	//pTestSectionsLayer->StraightToTestSection("Display2");
	
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
