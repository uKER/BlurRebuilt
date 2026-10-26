#ifndef MAIN_H
#define MAIN_H

#ifdef _MSC_VER
#pragma once
#endif

#if PLATFORM_PC
#include "Samples/System/PCApplication.h"
#elif PLATFORM_PS3
#include "Samples/System/PS3Application.h"
#elif PLATFORM_XENON
#include "Samples/System/XB2Application.h"
#endif

namespace Samples
{

#if PLATFORM_PC
class C_Sample : public C_PCApplication
#elif PLATFORM_PS3
class C_Sample : public C_PS3Application
#elif PLATFORM_XENON
class C_Sample : public C_XB2Application
#endif
{
public:

	C_Sample();
	~C_Sample();

	bool MyCreate();
};

}; // namespace Samples

#endif // MAIN_H
