#ifndef MMTIMER_H
#define MMTIMER_H


//-----------------------------------------------------------------------------
// Dependencies
//-----------------------------------------------------------------------------


#include <windows.h>
#include <mmsystem.h>
#include <list>



//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------


const DWORD MILLISECONDS_PER_SECOND = 1000;
const DWORD SECONDS_PER_MINUTE      = 60;
const DWORD MINUTES_PER_HOUR        = 60;
const DWORD HOURS_PER_DAY           = 24;


//-----------------------------------------------------------------------------
// CMMTimerListener class
//
//  Classes that derive from the CMMTimerListener class can attach themselves 
//  to the CMMTimer class for notifications when a timing event takes place.
//-----------------------------------------------------------------------------


// Forward declaration
class CMMTimer;


class CMMTimerListener
{
public:
    virtual void Update(CMMTimer &Timer) = 0;
};


//-----------------------------------------------------------------------------
// CMMTimer class
//
//  A light class wrapper for the Windows multimedia timer.
//-----------------------------------------------------------------------------


class CMMTimer
{
public:
    // Construction/Destruction
    CMMTimer();
    virtual ~CMMTimer();

    // Starts the timer. Delay is the interval in milliseconds between each 
    // timing event. Resolution is the timing resolution of this timer. A value
    // of zero gives the greatest timing resolution. Returns true if the 
    // operation was successful.
    bool Start(UINT Delay, UINT Resolution);

    // Stops this timer. Timer can be restarted by calling Start.
    void Stop();

    // Resets this timer back to zero.
    void Reset();

    // Returns true if this timer is running.
    bool IsRunning() const { return m_RunningFlag; }

    // Functions for attaching, detaching, and notifying CMMTimerListeners
    void AttachListener(CMMTimerListener &Listener);
    void DetachListener(CMMTimerListener &Listener);
    void NotifyListeners();

    // Gets the total timing values since this timer was started.
    DWORD GetTotalMilliseconds() const { return m_Milliseconds; }
    DWORD GetTotalSeconds() const;
    DWORD GetTotalMinutes() const;
    DWORD GetTotalHours() const;

    //
    // The following accessor functions return relative timing values. For 
    // example, the GetCurrentMilliseconds function returns how many 
    // milliseconds have passed since the last second has occurred. Let's say  
    // that 3.5 seconds have passed since the timer was started. 
    // GetCurrentMilliseconds would return 500 because that is how many 
    // milliseconds have passed since the last second began. 
    //

    DWORD GetCurrentMilliseconds() const;
    DWORD GetCurrentSeconds() const;
    DWORD GetCurrentMinutes() const;
    DWORD GetCurrentHours() const;

    // Gets the number of times a timing event has occurred.
    DWORD GetCount() const { return m_Count; }

    // Gets the device capabilities of this timer. Results are stored in the
    // TIMECAPS structure.
    static void GetDevCaps(LPTIMECAPS TimeCap);

private:
    // Function called by Windows when a timing event associated with this 
    // timer occurs.
    static void CALLBACK TimeProc(UINT uID, UINT uMsg, DWORD dwUser, 
                                  DWORD dw1, DWORD dw2);

    // Copying not allowed
    CMMTimer(const CMMTimer &);
    CMMTimer &operator = (const CMMTimer &);

private:
    UINT                          m_Id;
    UINT                          m_Delay;
    DWORD                         m_Milliseconds;
    DWORD                         m_Count;
    bool                          m_RunningFlag;
    std::list<CMMTimerListener *> m_Listeners;
    CRITICAL_SECTION              m_CriticalSection;
};


#endif