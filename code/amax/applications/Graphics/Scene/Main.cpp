#include "StdAfx.h"
#include "Main.h"
#include <TestSections/TestsectionLayer.h>
#include <TestSections/TestSectionSkyDome.h>
#include <TestSections/TestSectionSceneGraph.h>

Debug::Float g_DrawDistance("Render.DrawDistance", 500.0f, Debug::Range(0.0f, 10000.0f));

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

	

	pTestSectionLayer->AddTestSection<Framework::C_TestSectionSkyDome>("SkyDome");

	pTestSectionLayer->AddTestSection<Framework::C_TestSectionSceneGraph>("SceneGraph");
	
	return S_OK;
}

void C_TestApp::OnUpdate()
{
	CommonUpdate();
}

void C_TestApp::OnRender()
{
	m_SplitscreenViews[0].m_ViewSetup.m_ViewSetup3D.m_FarClip = g_DrawDistance;

	if(BeginRender())
	{
		CommonRender();

		EndRender();
	}
}

#include <Framework/Application/ApplicationMain.inl>

