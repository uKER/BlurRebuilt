#ifndef __SYSTEM_PROFILER_NETWORK_CODE_H__
#define __SYSTEM_PROFILER_NETWORK_CODE_H__

class ArteryMessageReader;
class ArteryConnection;

namespace Perf
{

/// initialise server that allows the PC profiler client to connect to this app
int InitialiseNetworkConnection();

template<typename Archive>
void SerialiseProfilerCapture(ProfilerCommon *pProfiler, Archive ar, EventList *pEventList = NULL);

//Callback function to capture the perf data to a raw file
extern void g_CapturePerfCallbackFunction(Perf::ProfilerCommon *pProfiler, void* pParam);

/// internal list of counters that the profiling app is interested in
struct SelectedCounter
{
	float fValue;
	Util::C_String sName;

	// serialise a SelectedCounter structure
	template<typename Stream>
	Stream& Serialise(Stream& out)
	{
		Util::C_String tmp = sName + " ";
		out << tmp << fValue;
		return out;
	}
};

/// this class keeps track of what counters the profiling app is interested in
class SelectedCounters : public Ut::Singleton<SelectedCounters>
{
public:
	/// add counters from an artery counter list message
	void AddSelectedCountersFromArteryMessage(ArteryMessageReader &msgReader);

	/// callback to collect the values of a selection of counters and then send them to the profiling client app via artery
	void ArterySendCounterList(ProfilerCommon *pProfiler);

private:
	/// clear the list of selected counters
	void ClearSelectedCounters();

	/// add a selected counter to the list
	void AddSelectedCounter(const char *pszName);

	/// update selected counter list with a particular counter value
	void UpdateSelectedCounterList(float fValue, const char *pszName);

	Ut::Vector<SelectedCounter>	m_SelectedCounters;

	MT::Mutex m_Mutex;
};


} // namespace Perf

#endif
