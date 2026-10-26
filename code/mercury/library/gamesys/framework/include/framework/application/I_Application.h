#ifndef FRAMEWORK_I_APPLICATION_H
#define FRAMEWORK_I_APPLICATION_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Framework/Application/ApplicationTypes.h"
#include "Framework/Application/ApplicationParams.h"

namespace Framework
{

//! I_Application. Interface for the application.

class I_Application
{
public:

	virtual ~I_Application() {}

	//! Is the application paused due to loss of focus?
	virtual bool IsFocusPaused() const = 0;

	//! Has the application been initialised?
	virtual bool IsInitialised() const = 0;

	//! Initialises the application.
	virtual HRESULT Initialise(const C_ApplicationParams& params) = 0;

	virtual void PollPlatformCallbacks() = 0;

	//! Shutsdown the application.
	virtual void Shutdown(bool force) = 0;

	//! Runs the application.
	virtual HRESULT Run() = 0;
};

}; // namespace Framework

#endif // FRAMEWORK_I_APPLICATION_H
