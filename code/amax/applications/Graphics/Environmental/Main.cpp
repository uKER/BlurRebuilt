#include "StdAfx.h"
#include "Main.h"
#include <TestSections/TestSectionLayer.h>
#include "TestSections/TestSectionFog.h"
#include "TestSections/TestSectionOcean.h"
#include "TestSections/TestSectionRain.h"
#include "TestSections/TestSectionSky.h"
#include "TestSections/TestSectionSplash.h"

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
	
	pTestSectionsLayer->AddTestSection<C_TestSectionFog>("Fog");
	pTestSectionsLayer->AddTestSection<C_TestSectionOcean>("Ocean");
	pTestSectionsLayer->AddTestSection<C_TestSectionRain>("Rain");
	pTestSectionsLayer->AddTestSection<C_TestSectionSky>("Sky");
	pTestSectionsLayer->AddTestSection<C_TestSectionSplash>("Splash");

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
