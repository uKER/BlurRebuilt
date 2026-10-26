#ifndef __NET_LOADING_THREAD_H__
#define __NET_LOADING_THREAD_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/Threads/Thread.h>
#include <Core/Threads/Event.h>

namespace RacerNet
{
	/*--------------------------------------------------------------------------------------
		Class	: C_NetLoadingThread

		Desc	: Temporary thread to create while the game load blocks the main thread. This
				  allows the network layer to process messages. 

		Note	: Don't reference any network structures externally while this thread is active
	--------------------------------------------------------------------------------------*/
	class C_NetLoadingThread : private MT::Thread
	{
	public:
		C_NetLoadingThread();
		virtual ~C_NetLoadingThread();

		void		Enter(void);
		void		Leave();
		bool		IsActive(void) const {return m_Active;}
		void		ThreadMain();

	private:
		bool		m_Active;
		MT::Event	m_Event;
	};

}; // namespace RacerNet

#endif // __NET_LOADING_THREAD_H__