#include "StdAfx.h"
#include "Main.h"

C_GameApp::C_GameApp() 
{
}

C_GameApp::~C_GameApp()
{
}

HRESULT C_GameApp::OnInitialise()
{
	// TODO: Add layers here.

	return S_OK;
}

void C_GameApp::OnUpdate()
{
	CommonUpdate();
}

void C_GameApp::OnRender()
{
	if(BeginRender())
	{
		CommonRender();

		EndRender();
	}
}

#undef C_APPLICATION
#define C_APPLICATION C_GameApp
#include <Framework/Application/ApplicationMain.inl>
