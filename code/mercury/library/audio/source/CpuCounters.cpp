#include "AudioPch.h"

bool g_bNamedCountersEnabled = false;

#ifdef NAMED_COUNTERS_ENABLED
Debug::Bool g_bNamedCountersUseInstanceNames("Audio.System.Performance.UseInstanceNamesForNamedPerfBar", false);
Debug::Bool g_bNamedCountersUseEntityNames("Audio.System.Performance.UseEntityNamesForNamedPerfBar", false);

Audio::NamedCpuCounters::~NamedCpuCounters()
{
	Clear();
}

void Audio::NamedCpuCounters::BeginEvent(const char* pchLabel, const Audio::StringHash hash) 
{
	NamedCounter* pNamedCounter = GetNamedCounter(pchLabel, hash);
	
	if (!pNamedCounter)
		return;

	pNamedCounter->Begin();
}

void Audio::NamedCpuCounters::EndEvent(const char* pchLabel, const Audio::StringHash hash) 
{
	NamedCounter* pNamedCounter = GetNamedCounter(pchLabel, hash);
	
	if (!pNamedCounter)
		return;

	pNamedCounter->End(true);
}

void Audio::NamedCpuCounters::EndAllEvents()
{
	C_NamedCounterHashMap::Iterator it = m_counterMap.Begin();
	while (it != m_counterMap.End())
	{
		it->GetValue()->EndEvents();
		++it;
	}
}

void Audio::NamedCpuCounters::Clear()
{
	C_NamedCounterHashMap::Iterator it = m_counterMap.Begin();
	while (it != m_counterMap.End())
	{
		delete it->GetValue();
		++it;
	}
	m_counterMap.Clear();
}

const Btl::String Audio::NamedCpuCounters::GetEventName(S32 index) 
{ 
	C_NamedCounterHashMap::Iterator it = m_counterMap.Begin();
	
	S32 c = 0;
	while (it != m_counterMap.End())
	{
		if (c == index)
			return it->GetValue()->strLabel;

		++it;
		c++;
	}

	return Btl::String("None");	
}

F32 Audio::NamedCpuCounters::GetEventCpuTime(S32 index) 
{
	C_NamedCounterHashMap::Iterator it = m_counterMap.Begin();
	
	S32 c = 0;
	while (it != m_counterMap.End())
	{
		if (c == index)
			return 100.f * it->GetValue()->Value() * (F32)Core::uSampleRate / (F32)Core::uFrameLength;

		++it;
		c++;
	}

	return 0.f;

}

Audio::NamedCounter* Audio::NamedCpuCounters::GetNamedCounter(const char* pchLabel, const Audio::StringHash hash) 
{ 
	NamedCounter* pNamedCounter = 0;

	C_NamedCounterHashMap::Iterator it = m_counterMap.Find(hash);
	if (it == m_counterMap.End())
	{
		// if doesnt exist then create one and add to map
		pNamedCounter = new NamedCounter(pchLabel);
		m_counterMap.Insert(StringHash(pchLabel), pNamedCounter);
	}
	else
	{
		pNamedCounter = it->GetValue();
	}
	return pNamedCounter;
}
#endif
