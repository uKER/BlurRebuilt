#ifndef __MERCURY_SYSTEM_THREAD_H__
#define __MERCURY_SYSTEM_THREAD_H__

#ifdef _DEBUG
extern U32 mem_allocated;
#endif

#ifdef PLATFORM_PS3
#include "WaitForObjects.h"
#endif

namespace MT
{
#ifndef HAVE_PLATFORM_THREAD_HANDLE_TYPE
typedef void ThreadHandle;
#endif

int GetProcessID();

/// Thread class. Implement a thread by deriving from this class and implementing the ThreadMain function. Call Start() to start the thread.
#ifdef PLATFORM_PS3
class Thread : public C_EventBase
#else
class Thread
#endif
{
public:
						/// Constructor
				        Thread();

						/// Destructor
    virtual             ~Thread();

						/// release any resources
	void				Close();
    
						/// start the thread
    void                Start( const U32 stackSize = 0, const S32 priority = 0, char const * threadName = NULL );

						/// Wait for the thread to exit. (Suspend execution of the calling thread until the target thread terminates)
	void				Join();

						/// Suspends the calling thread for msec milliseconds.
    static void			Sleep( const U32 msec );

						/// returns true if the thread is running
    const bool          IsRunning();
    
						/// retrieve the platform-dependant thread identifier
	ThreadHandle		*GetHandle() const { return m_Handle; }

#if defined(PLATFORM_PC)
	U32					GetId() const { return m_Id; }
#endif
    
						/// helper function for starting the thread - for internal use only
    U32					_ThreadRunProc();

                        /// Get handle of current thread from TLS [for when you don't have a pointer to the MT::Thread object for the current thread!]
    static ThreadHandle* GetCurrentThreadHandle();
    
                        /// Set handle of current thread in TLS
    static void         SetCurrentThreadHandle( ThreadHandle* handle );


#ifdef PLATFORM_PC
						/// Fast Get Thread ID for PC, TLS
    static U32			GetCurrentThreadID();
    
                        /// Fast Set Thread ID for PC, TLS
    static void         SetCurrentThreadID( U32 threadID );
#endif
    
protected:
                        /// The thread code to execute. Implement a thread by overloading this function
    virtual void        ThreadMain() = 0;

private:
    ThreadHandle        *m_Handle; 
    S32                 m_Priority;
	bool				m_bIsRunning;
#if defined(PLATFORM_PC)
	U32					m_Id;
#endif
};

}; // namespace MT

#endif
