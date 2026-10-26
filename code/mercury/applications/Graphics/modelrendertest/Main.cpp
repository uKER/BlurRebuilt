#include "StdAfx.h"
#include "Main.h"
#include <TestSections/TestsectionLayer.h>
#include "source/ModelRenderSection.h"

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

	
	pTestSectionLayer->AddTestSection<C_ModelRenderSection>("Model Render Test");

	return S_OK;
}

void C_TestApp::OnUpdate()
{
	CommonUpdate();
}

void C_TestApp::OnRender()
{
	m_SplitscreenViews[0].m_ViewSetup.m_ViewSetup3D.m_FarClip = 10000.f;

	if(BeginRender())
	{
		CommonRender();

		EndRender();
	}
}

#include <Framework/Application/ApplicationMain.inl>

