#include "StdAfx.h"
#include "Main.h"
#include <TestSections/TestSectionLayer.h>
#include "TestSections/TestSectionDynamicEffect.h"
#include "TestSections/TestSectionLight.h"
#include "TestSections/TestSectionParticles.h"
#include "TestSections/TestSectionTracerFire.h"
#include <Xt/XtGame.h>

XT_APPLICATION
	XT_DEPENDS_ON(RenderDynamicEffect)
	XT_DEPENDS_ON(RenderLightEffect)
	XT_DEPENDS_ON(RenderParticles)
	XT_DEPENDS_ON(RenderTracerEffect)
XT_END_APPLICATION

XtDb*				gXtDb;
XtPack*				gXtPack;
XtCallbackContext	gXtContext('P');

void OpenFlask()
{
	XtInitialise();

	Sys::I_File* f;

	if ((f = Sys::C_FileSystem::Instance().Open("DynamicEffect.bin")))
	{
		U32 length = (U32)f->Length();
		U8* data = new U8[length];

		if (data)
		{
			if (f->Read(data, length))
			{
				gXtDb	= XtDbCreate(XtGetApplicationSchema(), 2000, &gXtContext);
				gXtPack	= XtPackOpen(gXtDb, data, length);

				XtNetworkStart(gXtDb);
			}
		}

		delete data;
		delete f;
	}
}

C_TestApp::C_TestApp() 
{
}

C_TestApp::~C_TestApp()
{
}

HRESULT C_TestApp::OnInitialise()
{
	OpenFlask();

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
