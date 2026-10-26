#ifndef __SYSTEM_PROFILER_H__
#define __SYSTEM_PROFILER_H__

#include "system/Perf/ProfilerEnable.h"

/// this header file is to be included by .cpp files that want to generate profiling data
/// .cpp files that want to read back and display generated profiling data must include ProfilerBackend.h
namespace Perf
{

struct Event;

/// mark the start of a perf event
void BeginNamedEvent(const char *pszName);

/// mark the end of a perf event
void EndNamedEvent();

/// emit a custom event
void EmitEvent(const char *pszName, U64 time, U32 iThread, U8 iCore, U8 iEnd);

/// set the value of a named counter
void AddNamedCounter(float fValue, const char *pszName);

#ifdef PLATFORM_SPU
/// flush the local SPU event buffer
void FlushEventBuffer();
#endif

#if PLATFORM_PC
void BeginTrace();
void EndTrace();
#else
inline void BeginTrace() {}
inline void EndTrace() {}
#endif

//Dump to Debug.Perf log the perf data
void DumpPerfData();

//Write a capture file to dontship/captures folder or captureFileName 
void CapturePerfData(const char* captureFileName = NULL);

#ifndef ENABLE_MERCURY_PROFILER

inline void BeginNamedEvent(const char *pszName) {}
inline void EndNamedEvent() {}
inline void EmitEvent(const char *pszName, U64 time, U32 iThread, U8 iCore, U8 iEnd) {}
inline void AddNamedCounter(float fValue, const char *pszName) {}
inline void DumpPerfData() {}
inline void CapturePerfData(const char* captureFileName) {}
#ifdef PLATFORM_SPU
/// flush the local SPU event buffer
inline void FlushEventBuffer() {}
#endif

#endif

} // namespace Perf

#endif
