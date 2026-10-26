#ifndef NETTRANSPORT_SOCKET_H
#define NETTRANSPORT_SOCKET_H

#ifdef _MSC_VER
#pragma once
#endif


#include "netTransport_AddressMap.h"
#include "netTransport_Message.h"
#include "netTransport_ConnPacket.h"

#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>

namespace netTransport
{
	class SocketListener
	{
	public:
		virtual ~SocketListener(){}
		virtual bool recv(Message* msg, int socketPort, ConnGuid from)=0;
	};

	class SocketHandler
	{
	protected:
		virtual ~SocketHandler(){}
		virtual bool sendMessage(Message* msg, ConnGuid to)=0;
		virtual bool verifyPacket(ConnPacket* packet)=0;
		virtual bool handleMessage(ConnPacket* packet, Message* msg)=0;
		friend class Socket;
	};

	BD_REFERENCE(Simulator);
	BD_REFERENCE(Socket);
	class Socket: public bdReferencable
	{
	public:
		Socket(MessageParserRef messageParser);
		~Socket();
		
		bool listen();
		bool send(Message* msg, int socketPort, ConnGuid to);

		// packet message parser
		MessageParserRef messageParser();
		AddressMapRef	 addressMap();
		SimulatorRef	 simulator();

		// handler & listener registration
		bool registerListener(SocketListener* listener);
		bool unregisterListener(SocketListener* listener);	

		bool registerHandler(int port, SocketHandler* handler);
		bool unregisterHandler(int port, SocketHandler* handler);
		int  sendPacket(ConnPacket* packet);
		
	private:
		int  listenPacket();
		bool verifyPacket(ConnPacket* packet);
		bool receivePacket(ConnPacket* packet);
		bool handleMessage(ConnPacket* packet, Message* msg);

		ConnPacket						m_packet;
		bdHashMap<int, SocketHandler*>	m_handlers;
		bdLinkedList<SocketListener*>	m_listeners;
		MessageParserRef				m_messageParser;
		AddressMapRef					m_addressMap;
		SimulatorRef					m_simulator;
	};
}

#endif //NETTRANSPORT_SOCKET_H