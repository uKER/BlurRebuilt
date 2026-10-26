#include "StdAfx.h"
#include "Main.h"
#include <TestSections/TestSectionLayer.h>
#include "TestSections/TestSectionCompression.h"

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
	
	pTestSectionsLayer->AddTestSection<C_TestSectionCompression>("Compression");
	
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
