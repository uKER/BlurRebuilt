#ifndef NETWORK_ASYNC_TASK_H
#define NETWORK_ASYNC_TASK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/Core.h>
#include <Network/SystemHeaders.h>

namespace NetPlay
{
	/// Wrapper class for XOVERLAPPED operations..
	/** Only allpicable if we use games for windows live for the pc version.
	 */
#if defined (LIVE_ON_WINDOWS)
	class C_AsyncTask
	{
	public:
		explicit C_AsyncTask();
		virtual ~C_AsyncTask();

		operator XOVERLAPPED() const;
		XOVERLAPPED* operator&();

		void		Close(void);
		bool		Finished(void);
		U32			ErrorCode(void);
		bool		GetBytesRead(U32& outBytes);

	private:
		// these are defined but never implemented as we dont want to allow their use.
		C_AsyncTask(const C_AsyncTask&);
		C_AsyncTask& operator=(const C_AsyncTask&); 
		XOVERLAPPED m_Task;
	};
#endif
}


#endif