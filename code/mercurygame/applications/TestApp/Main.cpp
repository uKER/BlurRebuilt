#include "StdAfx.h"
#include "Main.h"
#include <framework/shared_layers/DebugLayer.h>
#include <TestSections/TestSectionLayer.h>
#include <TestSections/TestSectionSkyDome.h>
#include "TestSections/TestSectionTemplate.h"
#include "TestSections/TestSectionMaths.h"

#if defined(USER_CWRIGHT) || defined(USER_LLWRIGHT)
#include <TestSections/TestSectionLivery.h>
#endif

#if defined(USER_JDENTON) || defined(USER_LLWRIGHT)
#include <TestSections/TestSectionMaths.h>
#endif

#if defined(USER_DHAMPSON) || defined(USER_LLWRIGHT)
#include <TestSections/TestSectionCamera.h>
#endif

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

	pTestSectionsLayer->AddTestSection<C_TestSectionTemplate>("Template");
	pTestSectionsLayer->AddTestSection<Framework::C_TestSectionSkyDome>("SkyDome");

	#if defined(USER_CWRIGHT) || defined(USER_LLWRIGHT)
	pTestSectionsLayer->AddTestSection<Framework::C_TestSectionLivery>("Livery");
	#endif

	//#if defined(USER_JDENTON) || defined(USER_LLWRIGHT)
	pTestSectionsLayer->AddTestSection<Framework::C_TestSectionMaths>("Maths");
	//#endif

#if defined(USER_DHAMPSON) || defined(USER_LLWRIGHT)
	pTestSectionsLayer->AddTestSection<Framework::C_TestSectionCamera>("Camera");
#endif

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
