#ifndef FRAMEWORK_APPLICATION_360_H
#define FRAMEWORK_APPLICATION_360_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Framework/Application/ApplicationBase.h"
#include "Framework/application/LoadingThread.h"

namespace Framework
{

//! C_Application. 360 application.

class C_Application : public C_ApplicationBase
{
public:

	//! Constructor.
	C_Application();

	//! Destructor.
	virtual ~C_Application();

	//! Gets the 360 application instane.
	static C_Application* GetInstance()
	{
		return s_pInstance;
	}
	
	//! 360 application run.
	virtual HRESULT Run();

	virtual void ProcessLoadingScreen() OVERRIDE;

	virtual void PrePresent() OVERRIDE;

	bool LoadingScreenEnabledState() const;

private:
	
	static C_Application*		s_pInstance;

	C_LoadingThread				m_LoadingThread;

	Time::C_Time				m_startFrameTime;

	bool						m_cachedLoadingScreenEnabled; // Used by LUA GC to determine if it should run or not
};

}; // namespace Framework

#endif // FRAMEWORK_APPLICATION_360_H
