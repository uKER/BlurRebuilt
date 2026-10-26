#ifndef MAIN_H
#define MAIN_H

#ifdef _MSC_VER
#pragma once
#endif

#if PLATFORM_PS3
#include "Samples/System/PS3Application.h"
#elif PLATFORM_XENON
#include "Samples/System/XB2Application.h"
#else // PLATFORM_PC
#include "Samples/System/PCApplication.h"
#endif

namespace Samples
{

#if PLATFORM_PS3
class C_Sample : public C_PS3Application
#elif PLATFORM_XENON
class C_Sample : public C_XB2Application
#else
class C_Sample : public C_PCApplication
#endif
{
public:

	C_Sample();
	~C_Sample();

	Display::C_Device* CreateDevice();

	bool MyCreate();
};

}; // namespace Samples

#endif // MAIN_H
