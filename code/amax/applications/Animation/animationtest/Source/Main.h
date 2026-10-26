#ifndef GAME_APP_H
#define GAME_APP_H

#ifdef _MSC_VER
#pragma once
#endif

#include <framework/application/Application.h>

//! C_GameApp

class C_TestApp : public Framework::C_Application
{
public:

	C_TestApp();
	~C_TestApp();

	HRESULT OnInitialise();
	void OnShutdown();

	void OnUpdate();
	void OnRender();
};

#endif // GAME_APP_H
