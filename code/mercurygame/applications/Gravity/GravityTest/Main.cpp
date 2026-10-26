#include "StdAfx.h"
#include "Main.h"
#include <TestSections/TestSectionLayer.h>
#include "Physics/SectionPhysics.h"

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

	pTestSectionsLayer->AddTestSection<C_SectionPhysics>("Physics");

	pTestSectionsLayer->StraightToTestSection("Physics");
	
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

