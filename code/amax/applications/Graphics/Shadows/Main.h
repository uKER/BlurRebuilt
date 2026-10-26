#ifndef MAIN_H
#define MAIN_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Framework/Application/Application.h>

//! C_GameApp

class C_TestApp : public Framework::C_Application
{
public:

	C_TestApp();
	~C_TestApp();

	HRESULT OnInitialise();

	void OnUpdate();
	void OnRender();
};

#endif // MAIN_H
