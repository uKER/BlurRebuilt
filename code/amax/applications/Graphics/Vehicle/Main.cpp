#include "StdAfx.h"
#include "Main.h"
#include <framework/shared_layers/DebugLayer.h>
#include <TestSections/TestSectionLayer.h>
#include <TestSections/TestSectionLivery.h>
#include "Vehicle/TestSectionVehicle.h"

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

	Framework::C_DebugLayer* pDebugLayer = new Framework::C_DebugLayer();
	InsertLayer(pDebugLayer);

	pTestSectionsLayer->AddTestSection<C_TestSectionVehicle>("Vehicle");
	pTestSectionsLayer->AddTestSection<Framework::C_TestSectionLivery>("Livery");

	//pTestSectionsLayer->StraightToTestSection("Vehicle");

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
