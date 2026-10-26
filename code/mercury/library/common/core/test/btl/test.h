#ifndef BTL_TEST_H
#define BTL_TEST_H
#ifdef _MSC_VER
#pragma once
#endif
#include <core/core.h>
#ifdef PLATFORM_PC
#include <windows.h>
#endif

#define BTL_UNIT_TESTS

// todo: move into core 
#ifndef __GNUC__
#define BIZ_STATIC_ASSERT(exp) \
typedef char ERROR_[(exp)?1:-1]
#define BIZ_STATIC_ASSERT2(exp, err) \
typedef char ERROR_##err[(exp)?1:-1]
#else
#define BIZ_STATIC_ASSERT(exp) ;
#define BIZ_STATIC_ASSERT2(exp, err) ;
#endif

class Timer
{
public:
	Timer(const char* name) : m_name(name)
	{
#ifndef PLATFORM_PS3
		QueryPerformanceCounter((LARGE_INTEGER*)&t0);
#endif
	}

	~Timer()
	{
#ifndef PLATFORM_PS3
		U64 t1;
		QueryPerformanceCounter((LARGE_INTEGER*)&t1);
		U64 t = t1 - t0;

		U64 cps;
		QueryPerformanceFrequency((LARGE_INTEGER*)&cps);

		float ftime = (float) (t / cps);
		ftime += float(t % cps) / float(cps);

		OutputDebugStringA(m_name);
		OutputDebugStringA(": ");

		char buffer[_CVTBUFSIZE];
		sprintf_s(buffer, "%0.2f s", ftime);
		OutputDebugStringA(buffer);
		OutputDebugStringA("\n");
#endif
	}

private:
	U64 t0;
	const char* m_name;
};

#endif

