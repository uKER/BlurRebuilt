#ifndef __SYSTEM_PROFILER_PS3_H__
#define __SYSTEM_PROFILER_PS3_H__

#include <core\containers\sortedbinarytree.h>

/// this header file is to be included by .cpp files that want to generate profiling data
/// .cpp files that want to read back and display generated profiling data must include ProfilerBackend.h
namespace Perf
{

/// PS3 specific part of the profiler backend	
class Profiler : public ProfilerCommon
{
public:
	/// constructor
			Profiler();

	/// destructor
	virtual ~Profiler();

	/// return a friendly name for the thread on an event
	virtual const char *ThreadGetFriendlyName(const Event &e);

	///	read trace data from libperf
	void	ReadTraceData(bool bClearOnly);

private:
	/// (re)add Lv2 OS trace buffers
	void	AddLv2OSTrace();

	/// interpret a block of trace data
	void	ProcessTraceData(U32 *pTraceCache);

	/// initialise libperf trace buffer
	void	InitialiseTraceBuffer();

	int		m_iTraceCacheSize;
	U32		*m_pSchedTraceCache;
	U32		*m_pThreadTraceCache;

	/// map thread IDs to thread names
	Ut::UtMap<U32,Util::C_String>	m_ThreadMap;
};

} // namespace Perf

#endif
