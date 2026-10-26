#ifndef MAIN_H
#define MAIN_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Framework/Application/Application.h>

//! C_GameApp

class C_GameApp : public Framework::C_Application
{
public:

	C_GameApp();
	~C_GameApp();

	HRESULT OnInitialise();

	void OnUpdate();
	void OnRender();
};

#endif // MAIN_H
