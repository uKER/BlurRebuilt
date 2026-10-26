#include <StdAfx.h>
#include "AsyncTask_pc.h"


namespace NetPlay
{
	U8 sTemp = 0;
#if defined (LIVE_ON_WINDOWS)
	// constructor
	C_AsyncTask::C_AsyncTask()
	{
		ZeroMemory(&m_Task, sizeof(m_Task));
	}


	// destructor.
	C_AsyncTask::~C_AsyncTask()
	{
		// MUST call close before deleting task.
		assert(Finished());		
	}


	/// Direct access to overlapped task, type conversion
	C_AsyncTask::operator XOVERLAPPED() const
	{
		return m_Task;
	}


	/// Direct access to the address of the overlapped task for passing into API functions
	XOVERLAPPED* C_AsyncTask::operator&()
	{
		return &m_Task;
	}


	// Close the overlapped operation.  
	void C_AsyncTask::Close(void)
	{
		// if task is active cancel first..
		if (!Finished())
		{
			const DWORD res = XCancelOverlapped(&m_Task);
		}
		ZeroMemory(&m_Task, sizeof(m_Task));
	}


	/// Has the overlapped operation finished.
	bool C_AsyncTask::Finished(void) 
	{
		return XHasOverlappedIoCompleted(&m_Task) ? true : false;
	}


	/// Returns the error code after we have completed an overlapped operation.
	U32 C_AsyncTask::ErrorCode(void)
	{
		return XGetOverlappedExtendedError(&m_Task);
	}


	/// Once the task has finished we can get the number of bytes read by the operation.
	//	return true	:	outBytes will have valid data in it.
	//	return	false	:	outBytes will not be valid.
	bool C_AsyncTask::GetBytesRead(U32& outBytes)
	{
		DWORD bytesRead = 0;
		const U32 res = XGetOverlappedResult(&m_Task, &bytesRead, false);
		if (ERROR_SUCCESS == res)
		{
			outBytes = bytesRead;
			return true;
		}

		outBytes = 0;
		return false;
	}
#endif
}