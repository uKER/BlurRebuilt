#include <nexus_app/stdafx.h>
#include "nexus_app/MMTimer.h"


//-----------------------------------------------------------------------------
// CMMTimer class implementation
//-----------------------------------------------------------------------------


// Default constructor
CMMTimer::CMMTimer() :
m_Milliseconds(0),
m_Delay(0),
m_Count(0),
m_RunningFlag(false)
{
    // Initialize critical section. Since we use a linked list for storing 
    // timer listeners, it's important to surround access to the list with a 
    // critical section in order for this class to be thread safe.
    ::InitializeCriticalSection(&m_CriticalSection);
}


// Destructor
CMMTimer::~CMMTimer()
{
    if(m_RunningFlag)
    {
        Stop();
    }

    ::DeleteCriticalSection(&m_CriticalSection);
}


// Starts timer
bool CMMTimer::Start(UINT Delay, UINT Resolution)
{
    bool Result = true;

    // Only begin timer if it is not already running
    if(!m_RunningFlag)
	{
		#pragma warning(push)
		#pragma warning(disable : 4311)
        // If this timer started without error
        if((m_Id = ::timeSetEvent(Delay, Resolution, TimeProc,
            reinterpret_cast<DWORD>(this), TIME_PERIODIC)) != NULL)
        {            
            // Keep track of delay interval and note that this timer is 
            // running
            m_Delay       = Delay;
            m_RunningFlag = true;
        }
        // Else an error occurred
        else
        {
            // Note error
            Result = false;
        }
		#pragma warning(pop)
    }

    return Result;
}


// Stop timer
void CMMTimer::Stop()
{
    if(m_RunningFlag)
    {
        ::timeKillEvent(m_Id);
        m_RunningFlag = false;
    }
}


// Reset timer values
void CMMTimer::Reset()
{
    m_Milliseconds = m_Count = 0;
    NotifyListeners();
}


// Attach timer listener so that it can be notified when a timing event 
// occurrs
void CMMTimer::AttachListener(CMMTimerListener &Listener)
{
    ::EnterCriticalSection(&m_CriticalSection);

    m_Listeners.push_front(&Listener);

    ::LeaveCriticalSection(&m_CriticalSection);    
}


// Detach listener
void CMMTimer::DetachListener(CMMTimerListener &Listener)
{
    ::EnterCriticalSection(&m_CriticalSection);

    m_Listeners.remove(&Listener);

    ::LeaveCriticalSection(&m_CriticalSection);    
}


// Notify listeners that a timing event has occured
void CMMTimer::NotifyListeners()
{
    ::EnterCriticalSection(&m_CriticalSection);

    for(std::list<CMMTimerListener *>::iterator i = m_Listeners.begin();
        i != m_Listeners.end(); i++)
    {
        (*i)->Update(*this);
    }

    ::LeaveCriticalSection(&m_CriticalSection);
}


// Gets total number of seconds that have passed since this timer began.
DWORD CMMTimer::GetTotalSeconds() const
{
    return m_Milliseconds / MILLISECONDS_PER_SECOND;
}


// Gets total number of minutes that have passed since this timer began.
DWORD CMMTimer::GetTotalMinutes() const
{
    return GetTotalSeconds() / SECONDS_PER_MINUTE;
}


// Gets total number of hours that have passed since this timer began.
DWORD CMMTimer::GetTotalHours() const
{
    return GetTotalMinutes() / MINUTES_PER_HOUR;
}


// Gets number of milliseconds that have passed since the last second.
DWORD CMMTimer::GetCurrentMilliseconds() const
{ 
    return m_Milliseconds - GetTotalSeconds() * MILLISECONDS_PER_SECOND;
}


// Gets number of seconds that have passed since last minute.
DWORD CMMTimer::GetCurrentSeconds() const
{
    return GetTotalSeconds() - GetTotalMinutes() * SECONDS_PER_MINUTE;
}


// Gets number of minutes that have passed since last hour.
DWORD CMMTimer::GetCurrentMinutes() const
{
    return GetTotalMinutes() - GetTotalHours() * MINUTES_PER_HOUR;
}


// Gets number of hours that have passed since last day (starts from zero 
// every 24 hours).
DWORD CMMTimer::GetCurrentHours() const
{
    DWORD CurrentHours = GetTotalHours();

    if(CurrentHours >= HOURS_PER_DAY)
    {
        CurrentHours %= HOURS_PER_DAY;
    }

    return CurrentHours;
}

    
// Gets multimedia device capabilities.
void CMMTimer::GetDevCaps(LPTIMECAPS TimeCap)
{
    ::timeGetDevCaps(TimeCap, sizeof(TIMECAPS));
}


// Function called by Windows when a timing event associated with this timer
// occurs.
void CALLBACK CMMTimer::TimeProc(UINT uID, UINT UMsg, DWORD dwUser, DWORD dw1,
                                 DWORD dw2)
{
	#pragma warning(push)
	#pragma warning(disable : 4312)
    CMMTimer *Timer = reinterpret_cast<CMMTimer *>(dwUser);
	#pragma warning(pop)

    Timer->m_Milliseconds += Timer->m_Delay;
    Timer->m_Count++;
    Timer->NotifyListeners();
}

