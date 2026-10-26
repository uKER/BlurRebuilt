#ifndef MAIN_H
#define MAIN_H

#ifdef _MSC_VER
#pragma once
#endif

#if defined( PLATFORM_XENON )
#include "Samples/System/XB2Application.h"
#elif defined( PLATFORM_PS3 )
#include "Samples/System/PS3Application.h"
#elif defined( PLATFORM_PC )
#include "Samples/System/PCApplication.h"
#endif

namespace Samples
{
#if defined( PLATFORM_XENON )
class C_Sample : public C_XB2Application
#elif defined( PLATFORM_PS3 )
class C_Sample : public C_PS3Application
#elif defined( PLATFORM_PC )
class C_Sample : public C_PCApplication
#endif
{
public:

	C_Sample();
	~C_Sample();

	bool MyCreate();
	void MyDestroy();
	void MyUpdate();

	void SetName( Util::C_String& name )		{ m_Name = name; }
};

}; // namespace Samples

#endif // MAIN_H
