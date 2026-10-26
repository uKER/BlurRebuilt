// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef REMOTE_LOG_SUBSCRIBER_H
#define REMOTE_LOG_SUBSCRIBER_H

#include <bdCore/bdCore.h>
#include <bdCore/bdSocket/bdStreamSocket.h>

/// \todo This class does not work!
///   The best thing to do is delete it!
class RemoteLogSubscriber : public bdLogSubscriber
{
	
	public:
		
		BD_DECLARE_NEW_AND_DELETE_OPERATORS
		
		enum bdSocketType
		{
			BD_UDP_SOCKET,
			BD_UDP_BROADCAST_SOCKET,
			BD_TCP_SOCKET,
			BD_MAX_SOCKET_TYPE
		};

		RemoteLogSubscriber();

		virtual ~RemoteLogSubscriber();

		bdBool init(const bdSocketType type = BD_UDP_BROADCAST_SOCKET,	
			const bdAddr &logMachine = bdAddr("255.255.255.255:3074"));

		void quit();

		virtual void publish(const bdLogMessageType type,
							 const bdNChar8 *const fullChannelName, 
							 const bdNChar8 *const file, 
							 const bdNChar8 *const function, 
							 const bdUInt line, 
							 const bdNChar8 *const msg)const;
	protected:

		bdBool createSocket(const bdBool isBroadcast);
		bdBool createStreamSocket(const bdAddr &destination);

		void closeSocket();
		void setDestination(const bdAddr &logMachine);

		// todo ...
		// should be pointers unless we want to use both ?
		bdDefaultSocket m_socket;
		bdStreamSocket m_streamSocket;

		bdAddr m_logMachine;

		bdSocketType m_socketType;
};

#endif // REMOTE_LOG_SUBSCRIBER_H
