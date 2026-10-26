#include <core/core.h>

#include <System/Memory/MemoryTracker.h>
#include <System/Memory/MemoryTrackerDefines.h>

#ifdef TOOL
#define _CXXTEST_HAVE_EH
#endif

#define _CXXTEST_HAVE_STD
#include <cxxtest/TestListener.h>
#include <cxxtest/TestTracker.h>
#include <cxxtest/TestRunner.h>
#include <cxxtest/RealDescriptions.h>
#include <customErrorPrinter.h>
#ifdef PLATFORM_PC
#include <cxxtest/Win32Gui.h>
#endif

int main(int argc, char *argv[])
{
//	Memory::Tracker::Instance().DumpStatsOnExit(false);
	Memory::Tracker::Instance().DumpBlocksOnExit(true);

	// Windows GUI version
#if 0 //defined(PLATFORM_PC)
	return CxxTest::GuiTuiRunner<CxxTest::Win32Gui, CxxTest::CustomErrorPrinter>( argc, argv ).run();
#else
	// console version
	return CxxTest::CustomErrorPrinter().run();
#endif
}

#include "overloadnew.cpp"
#include "customerrorprinter.cpp"
