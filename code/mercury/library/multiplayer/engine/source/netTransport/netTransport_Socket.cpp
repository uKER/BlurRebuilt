
#include "stdafx.h"
#include "./netTransport_Socket.h"
#include "./netTransport_Simulator.h"
#include "netPlatform/netPlatform_Core.h"

#include <bdNet\bdNet.h>
#include <bdSocket\bdSocketRouter.h>
#include <bdSocket\bdAddrHandle.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netTransport
{
	Socket::Socket(MessageParserRef messageParser)
	{
		m_messageParser = messageParser;
		m_addressMap = new AddressMap;

		#if !defined(CFG_GOLD)
		MEMORY_TRACKER_PUSH("network.multiplayer");
		m_simulator = new Simulator(m_addressMap);
		MEMORY_TRACKER_POP("network.multiplayer");		
		#endif
	}

	Socket::~Socket()
	{}

	bool Socket::registerListener(SocketListener* listener)
	{
		m_listeners.addTail(listener);
		return true;
	}

	bool Socket::unregisterListener(SocketListener* listener)
	{
		bdLinkedList<SocketListener*>::Position pos = m_listeners.getHeadPosition();
		
		while(pos)
		{
			SocketListener* l = m_listeners.getAt(pos);
			if(l == listener)
				m_listeners.removeAt(pos);
			else
				m_listeners.forward(pos);
		}
		return true;
	}

	bool Socket::registerHandler(int port, SocketHandler* handler)
	{
		if(m_handlers.containsKey(port) || m_handlers.containsValue(handler))
		{
			LOGFATAL(Debugging::g_logger, "<netTransport::Socket::registerHandler()> already registered.");
			return false;
		}
		m_handlers.put(port, handler);
		return true;
	}

	bool Socket::unregisterHandler(int port, SocketHandler* handler)
	{
		m_handlers.remove(port);
		return true;
	}

	bool Socket::send(Message* msg, int socketPort, ConnGuid to)
	{
		SocketHandler* handler;
		if(!m_handlers.get(socketPort, handler))
			return 0;

		return handler->sendMessage(msg, to);
	}

	MessageParserRef Socket::messageParser() 
	{ 
		return m_messageParser; 
	}
	
	AddressMapRef Socket::addressMap() 
	{ 
		return m_addressMap; 
	}
	
	SimulatorRef Socket::simulator() 
	{ 
		return m_simulator; 
	}

	bool Socket::listen()
	{
		#if !defined(CFG_GOLD)
		if(m_simulator.notNull())
		{
			m_simulator->setActive(Debugging::dbg_simulatorEnabled);
			m_simulator->setLatency(Debugging::dbg_simulatorLatency / 1000.0f, Debugging::dbg_simulatorLatency / 2000.0f);
			m_simulator->setPacketLoss(Debugging::dbg_simulatorPacketLoss / 1000.0f, Debugging::dbg_simulatorPacketLoss / 2000.0f);
		}
		#endif

		if(m_simulator.notNull())
			m_simulator->poll();

		// keep reading packet from socket.
		const int maxlen = 16 * 1024; // receive a maximum of 16 KBytes per frame to avoid infinite loop.
		int len = 0;
		do
		{
			// read one packet.
			int packetlen = listenPacket();
			
			// no packet waiting. exit.
			if(packetlen == 0)
			{
				break;
			}

			// accumulate the number of bytes read this frame.
			len += packetlen;

			// If reached limit, return. Next packets will be processed next frame.
			if(len > maxlen) 
			{
				// break loop.
				LOGWARN(Debugging::g_logger, "<netTransport::Socket::listen()> Received too many packets this frame(%.2f KBytes).", (F32)(len / 1024.0f));
				break;
			}
			
		}
		while(1);
		
		return true;
	}

	// receive a packet from the socket buffer.
	// returns the size of the packet received. 
	int Socket::listenPacket()
	{
		bdNetImpl *const net = bdNet::getInstance();
		bdSocketRouter* socketRouter = net->getSocketRouter();
		if(!socketRouter) 
			return 0;
		
		if(	socketRouter->getStatus() == bdSocketRouter::BD_SOCKET_ROUTER_UNINITIALIZED ||
			socketRouter->getStatus() == bdSocketRouter::BD_SOCKET_ROUTER_ERROR)
			return 0;
		
		if(net->getStatus() != bdNetImpl::BD_NET_DONE) 
			return 0; 
		
		bdAddrHandleRef addr;		
		char data[2000];
		int len = socketRouter->receiveFrom(addr, data, sizeof(data));
		if(len <= 0) return 0;

		// endian swap first two bytes
		U16  src  = *((const U16*)data);
		U16& dest = *((U16*)data);
		bdBitBuffer::endianSwap<U16>(src, dest);

		m_packet.reset();
		m_packet.m_address = addr;
		
		if(m_packet.readFrom(data, len) <= 0)
			return 0;
		
		// process packet
		receivePacket(&m_packet);
		return len;
	}
	
	bool Socket::receivePacket(ConnPacket* packet)
	{
		// packet destination is another machine.
		if(!verifyPacket(packet))
			return true;
		
		// extract messages
		do
		{
			Message* msg = packet->readMessage(m_messageParser);
			if(msg == NULL) break;

			// process messages
			handleMessage(packet, msg);
		}
		while(1);

		return true;
	}

	bool Socket::verifyPacket(ConnPacket* packet)
	{
		// who's handling that particular port
		SocketHandler* handler;
		if(!m_handlers.get(packet->m_port, handler))
			return true;

		// sanity checks, and routing packets for forwarding.
		return handler->verifyPacket(packet);
	}
		
	bool Socket::handleMessage(ConnPacket* packet, Message* msg)
	{
		SocketHandler* handler;
		if(!m_handlers.get(packet->m_port, handler))
			return 0;

		if(!handler->handleMessage(packet, msg))
			return false;

		bdLinkedList<SocketListener*>::Position pos = m_listeners.getHeadPosition();
		while(pos)
		{
			SocketListener* entry = m_listeners.forward(pos);
			entry->recv(msg, packet->m_port, packet->m_origin);
		}
		return true;
	}

	int Socket::sendPacket(ConnPacket* packet)
	{
		if(packet->empty() || packet->error())
			return 0;

		bdNetImpl *const net = bdNet::getInstance();
		bdSocketRouter* socketRouter = net->getSocketRouter();
		if(!socketRouter) 
			return 0;
		
		if(	socketRouter->getStatus() == bdSocketRouter::BD_SOCKET_ROUTER_UNINITIALIZED ||
			socketRouter->getStatus() == bdSocketRouter::BD_SOCKET_ROUTER_ERROR)
			return 0;
		
		char data[2000];
		int len = packet->writeTo(data, sizeof(data));
		if(len == 0) return 0;

		// send the crap (endian swap first)
		U16 src = *((const U16*)data);
		U16& dest = *((U16*)data);
		bdBitBuffer::endianSwap<U16>(src, dest);
		
		// debug packet loss
		//bool block = (netPlatform::Core::instance()->randomInt(100) < 10);
		//if(block) return len;
		
		// send through internet simulator
		if(m_simulator.notNull() && m_simulator->active())
		{
			int ret = m_simulator->sendTo(packet->m_address, data, len);
			return ret;	
		}
		
		// send through socket interface
		int sent = socketRouter->sendTo(packet->m_address, (void*)data, len);
		return sent;
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

