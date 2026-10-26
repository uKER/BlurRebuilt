#ifndef __SYSTEM_NETWORK_SERVICE_H__
#define __SYSTEM_NETWORK_SERVICE_H__

#pragma once

namespace MT
{
	class Mutex;
}

#if defined( COMPILER_VC )
#define MAXIMUM_CONNECTIONS_PER_THREAD MAXIMUM_WAIT_OBJECTS
#else
#define MAXIMUM_CONNECTIONS_PER_THREAD 64
#endif

namespace Network
{

class Connection;
class ServiceThread;

/// provide a mechanism for network comminication with multiple clients simultaneously using pooled threads. For an example on how to use this class, see "mercury\library\system\source\debug\log\NetworkRepeater.cpp"
class Service
{
public:
							/// Constructor
							Service();
							/// Destructor
							~Service();

							/// Attach a connection to be handled by this service
	void					Attach(Connection *p);
							/// Remove a connection
	void					Detach(Connection *p);
							/// stop and delete all connections attached to this service
	void					Stop();

protected:
	MT::Mutex				*m_pMutex;

	Ut::Vector<ServiceThread*>	m_ServiceThreads;
};

/// the worker thread code used by Network::Service. Not meant to be used directly
class ServiceThread : public MT::Thread
{
	friend class Service;
public:
							/// Constructor
							ServiceThread(Service *pService);
							/// Destructor
							~ServiceThread();

							/// thread entry point
	void					ThreadMain();

							/// Attach a connection to be handled by this thread
							/// returns false if the thread is full
	bool					Attach(Connection *p);
							/// Remove a connection
	void					Detach(Connection *p);
							/// calls delete *this
	void					Destroy();
							/// returns the number of connections attached to this thread
	U32						GetNumConnections();
							/// stop and delete all connections on this thread
	void					Stop();

protected:
	/// linked list of connections to this thread
	Ut::UtListIntrusive<Connection>	m_Connections;
	MT::Mutex				*m_pMutex;
	Service					*m_pService;
	U32						m_nConnections;
	int						m_iMaxFS;
	bool					m_SignalQuit;
};

};


#endif
