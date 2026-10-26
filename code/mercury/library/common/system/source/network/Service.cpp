#include "SystemPch.h"

#include <core/threads.h>
#include <system/network/address.h>
#include <system/network/socket.h>
#include <system/network/connection.h>
#ifdef PLATFORM_PS3
#include <system/network/platform.h>
#endif
#include <system/network/service.h>
#include <system/time/time.h>
#include <core/utility/coredebug.h>

namespace Network
{

/// --- ServiceThread

/// Constructor
ServiceThread::ServiceThread(Service *pService)
{
	m_pMutex = new MT::Mutex;
	m_nConnections = 0;
	m_iMaxFS = 0;
	m_SignalQuit = false;

	// start the thread
	Start();
}

/// Destructor
ServiceThread::~ServiceThread()
{
	// stop thread
	m_SignalQuit = true;
	Join();
}

/// deletes itself
void ServiceThread::Destroy()
{
	delete this;
}

/// stop and delete all connections on this thread
void ServiceThread::Stop()
{
	m_Connections.Clear();
}

/// Remove a connection
void ServiceThread::Detach(Connection *p)
{
	m_pMutex->Wait();

	m_Connections.Erase( Ut::UtList<Connection>::Iterator(p) );
	delete p;

	m_nConnections--;

	m_pMutex->Release();
}

/// returns the number of connections attached to this thread
U32 ServiceThread::GetNumConnections()
{
	return m_nConnections;
}

/// Add a connection to be handled by this thread
bool ServiceThread::Attach(Connection *pConnection)
{
	m_pMutex->Wait();

	if (m_nConnections >= MAXIMUM_CONNECTIONS_PER_THREAD)
	{
		m_pMutex->Release();
		return false;
	}

	Ut::ListElementDouble<Connection> *pTest = pConnection;

	pConnection->m_pServiceThread = this;
	m_Connections.PushBack(*pConnection);
	pConnection->m_iTimeOfLastIdle = Time::GetCurrentTime();

#if !defined( COMPILER_VC)
	int x = pConnection->GetSocket()+1;
	if (x > m_iMaxFS)
		m_iMaxFS = x;
#endif

	m_nConnections++;

	m_pMutex->Release();

	return true;
}

/// thread entry point
void ServiceThread::ThreadMain()
{
#if !defined(COMPILER_VC)
	fd_set inp, out, err;
	FD_ZERO(&inp);
	FD_ZERO(&out);
	FD_ZERO(&err);
	int s;

	// Linux implementation
	for(;;)
	{
		// check for exit signal
		if (m_SignalQuit)
			return;

		// check status of all the connections
		m_pMutex->Wait();
		for( Ut::UtList<Connection>::Iterator p=m_Connections.Begin();
			p!=m_Connections.End(); ++p)
		{
			s = p->GetSocket();

			if (FD_ISSET(s, &err))
			{
				p->m_bDetectDisconnect = false;
				p->Disconnect();
				continue;
			}
			else if (FD_ISSET(s, &inp))
				p->Pending();
			else if (FD_ISSET(s, &out))
				p->Output();
		}

		int nHandles = 0;
		int iTimer = 1000;
		int iCurTime = Time::GetCurrentTime();

		// reset all flags
		for (Ut::UtList<Connection>::Iterator p=m_Connections.Begin(); p!=m_Connections.End(); ++p)
		{
			s = p->GetSocket();
			if (p->m_bDetectPending)
				FD_SET(s, &inp);

			if (p->m_bDetectOutput)
				FD_SET(s, &out);

			if (p->m_bDetectDisconnect)
				FD_SET(s, &err);

			// calculate time left until next expire for this connection
			if (p->m_iIdleTimeout != (U64)-1)
			{
				int iTimeLeft = iCurTime - (p->m_iTimeOfLastIdle + p->m_iIdleTimeout);
				if (iTimeLeft<0)
				{
					p->m_iTimeOfLastIdle = iCurTime;
					p->Idle();
				}
				if (iTimeLeft < iTimer)
					iTimer = iTimeLeft;
			}
		}
		
		m_pMutex->Release();

		// wait for a socket event
		struct timeval timeout, *tvp;
		timeout.tv_sec = iTimer / 1000;
		timeout.tv_usec = (iTimer % 1000) * 1000;
		tvp = &timeout;
#if defined( PLATFORM_PS3 )
		socketselect(m_iMaxFS, &inp, &out, &err, tvp);
#else
		select(m_iMaxFS, &inp, &out, &err, tvp);
#endif

	}
#else
	// Microsoft implementation
	HANDLE hv[MAXIMUM_WAIT_OBJECTS];
	memset(hv, 0, sizeof(hv));
	WSANETWORKEVENTS ne;

	for(;;)
	{
		// check for exit signal
		if (m_SignalQuit)
			return;

		m_pMutex->Wait();

		int nHandles = 0;
		int iTimer = 1000;
		U64 iCurTime = Time::GetCurrentTime();

		// make an array containing the socket events for all connections
		for (Ut::UtList<Connection>::Iterator iter=m_Connections.Begin(); iter!=m_Connections.End(); ++iter)
		{
			long events = 0;

			if (iter->m_bDetectDisconnect)
				events |= FD_CLOSE;
			if (iter->m_bDetectPending)
				events |= FD_READ;
			if (iter->m_bDetectOutput)
				events |= FD_WRITE;

			if (events)
			{
				assert(nHandles < MAXIMUM_WAIT_OBJECTS);

				WSAEventSelect(iter->GetSocket(),iter->m_Event,events);
				hv[nHandles] = iter->m_Event;
				nHandles++;
			}

			// calculate time left until next expire for this connection
			if (iter->m_iIdleTimeout != (U64)-1)
			{
				U64 iTimeLeft = iCurTime - (iter->m_iTimeOfLastIdle + iter->m_iIdleTimeout);
				// TODO: correctly implement the calling of the Idle function
				/*
				if (iTimeLeft<0)
				{
					iter->m_iTimeOfLastIdle = iCurTime;
					iter->Idle();
				}
				*/
				if ((int)iTimeLeft < iTimer)
					iTimer = (int)iTimeLeft;
			}
		}
		
		m_pMutex->Release();

		// if we have no sockets connected, simply sleep for a bit
		if (nHandles == 0)
		{
			Thread::Sleep(iTimer);
			continue;
		}
		// wait for a socket event
		//lint -save -e685
		DWORD res = WaitForMultipleObjects(nHandles, hv, FALSE, iTimer);
		if (res < WAIT_OBJECT_0 || res >= WAIT_OBJECT_0+MAXIMUM_WAIT_OBJECTS)
			continue;
		//lint -restore

		int iEvent = res - WAIT_OBJECT_0;

		// find connection for the event
		// (take into account that the connection may have been deleted)
		m_pMutex->Wait();
		Ut::UtList<Connection>::Iterator p=m_Connections.Begin();
		while(1)
		{
			Connection *pC = &(*p);
			if (p==m_Connections.End()) break;

			// found
			if (p->m_Event != hv[iEvent])
			{
				++p;
				continue;
			}

#if defined( PLATFORM_PC )
			WSAEnumNetworkEvents(p->GetSocket(),p->m_Event,&ne);

			if(ne.lNetworkEvents & FD_CLOSE) 
			{
				p->m_bDetectDisconnect = false;
				p->Disconnect();
			}
			else if(ne.lNetworkEvents & FD_READ)
				p->Pending();
			else if(ne.lNetworkEvents & FD_WRITE)
				p->Output();

#endif

			break;
		}

		m_pMutex->Release();
	}
#endif
}

/// --- Service

/// Constructor
Service::Service()
{
	m_pMutex = new MT::Mutex;
}

/// Destructor
Service::~Service()
{
	delete m_pMutex;
}

/// Attach a connection to be handled by this service
void Service::Attach(Connection *p)
{
	m_pMutex->Wait();

	// try to attach to an existing thread
	for (int i=0; i<m_ServiceThreads.Size(); i++)
	{
		if (m_ServiceThreads[i]->Attach(p))
		{
			m_pMutex->Release();
			return;
		}
	}

	// create a new thread
	ServiceThread *pThread = new ServiceThread(this);
	m_ServiceThreads.PushBack(pThread);
	pThread->Attach(p);

	m_pMutex->Release();
}

/// Remove a connection
void Service::Detach(Connection *p)
{
	p->m_pServiceThread->Detach(p);
}

/// Stop all connections
void Service::Stop()
{
	m_pMutex->Wait();

	// try to attach to an existing thread
	for (int i=0; i<m_ServiceThreads.Size(); i++)
	{
		m_ServiceThreads[i]->Stop();
		delete m_ServiceThreads[i];
	}

	m_pMutex->Release();
}

}
