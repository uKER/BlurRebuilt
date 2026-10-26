#ifndef MAIN_H
#define MAIN_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Framework/Application/Application.h>

//! C_TestApp

class C_TestApp : public Framework::C_Application
{
public:

	C_TestApp();
	~C_TestApp();

	HRESULT OnInitialise();
	void OnShutdown();

	HRESULT PlatformInitialise(const Framework::C_ApplicationParams& params);
	void PlatformShutdown();
	void SetupPlatformCallbacks(void);

	void OnUpdate();
	void OnRender();

private:
	bool m_NetworkStackStartedExternally;
};

#endif // MAIN_H
