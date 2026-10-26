#include "StdAfx.h"
#include "Main.h"
#include <TestSections/TestsectionLayer.h>
#include <TestSections/TestSectionModel.h>

C_TestApp::C_TestApp() 
{
}

C_TestApp::~C_TestApp()
{
}

HRESULT C_TestApp::OnInitialise()
{
	Framework::C_TestSectionLayer* pTestSectionLayer = new Framework::C_TestSectionLayer();
	InsertLayer(pTestSectionLayer);

	pTestSectionLayer->AddTestSection<Framework::C_TestSectionModel>("Model");
	
	return S_OK;
}

void C_TestApp::OnUpdate()
{
	CommonUpdate();
}

void C_TestApp::OnRender()
{
	if (BeginRender())
	{
		CommonRender();
		EndRender();
	}
}

#include <Framework/Application/ApplicationMain.inl>
