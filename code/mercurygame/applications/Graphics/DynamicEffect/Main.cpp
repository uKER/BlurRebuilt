#include "StdAfx.h"
#include "Main.h"
#include <TestSections/TestSectionLayer.h>
#include "TestSections/TestSectionDynamicEffect.h"
#include "TestSections/TestSectionLight.h"
#include "TestSections/TestSectionParticles.h"
#include "TestSections/TestSectionTracerFire.h"
#include "Dynamic/Generic/XtInterface.h"

#include <Xt/XtGame.h>

XT_APPLICATION
	XT_DEPENDS_ON(GraphicsDynamic)
	XT_DEPENDS_ON(GraphicsDynamicLight)
	XT_DEPENDS_ON(GraphicsDynamicParticle)
	XT_DEPENDS_ON(GraphicsDynamicTracer)
XT_END_APPLICATION

C_TestApp::C_TestApp() 
{
}

C_TestApp::~C_TestApp()
{
	DynamicEffect::XtInterface::PackClose();
}

HRESULT C_TestApp::OnInitialise()
{
	DynamicEffect::XtInterface::PackOpen(NULL, "");

	Framework::C_TestSectionLayer* pTestSectionsLayer = new Framework::C_TestSectionLayer();
	InsertLayer(pTestSectionsLayer);

	pTestSectionsLayer->AddTestSection<C_TestSectionDynamicEffect>("DynamicEffect");
	pTestSectionsLayer->AddTestSection<C_TestSectionLight>("Light");
	pTestSectionsLayer->AddTestSection<C_TestSectionParticles>("Particles");
	pTestSectionsLayer->AddTestSection<C_TestSectionTracerFire>("TracerFire");

	return S_OK;
}

void C_TestApp::OnUpdate()
{
	CommonUpdate();
	XtNetworkUpdate();
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
