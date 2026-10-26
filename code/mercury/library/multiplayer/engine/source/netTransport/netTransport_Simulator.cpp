
#include "stdafx.h"
#include "./netTransport_AddressMap.h"
#include "./netTransport_Enums.h"
#include "./netTransport_Simulator.h"
#include "netTime/netTime.h"
#include "netPlatform/netPlatform_Core.h"

#include <bdNet\bdNet.h>
#include <bdSocket\bdSocketRouter.h>
#include <bdSocket\bdAddrHandle.h>

namespace netSimulator
{
	ControlPoint::ControlPoint(F32 latency, F32 packetLoss)
	: m_latency(latency)
	, m_packetLoss(packetLoss)
	, m_time(netClock::instance()->now())
	{}

	Conditions::Conditions()
	{
		m_latencyMed = 0.0f;
		m_latencyDelta = 0.0f;
		m_packetLossMed = 0.0f;
		m_packetLossDelta = 0.0f;
		m_tickTime = (1000 * 1000);
	}

	bool Conditions::poll()
	{
		if(m_controlPoints.isEmpty())
		{
			addControlPoint();
			return true;
		}

		const ControlPoint& latest = m_controlPoints.getHead();
		
		U64 elapsed = (netClock::instance()->now() - latest.m_time);
		
		if(elapsed > m_tickTime)
		{
			addControlPoint();
			return true;
		}
		else
		{
			return false;
		}
	}

	void Conditions::setLatency(float med, float delta) 
	{ 
		m_latencyMed = med; 
		m_latencyDelta = (delta < 0.0f || delta > m_latencyMed)? med / 2 : delta; 
	}

	void Conditions::setPacketLoss(float med, float delta) 
	{ 
		m_packetLossMed = med; 
		m_packetLossDelta = (delta < 0.0f || delta > m_packetLossMed)? med / 2 : delta; 
	}

	void Conditions::addControlPoint()
	{
		float latency = m_latencyMed + netPlatform::Core::instance()->randomFloat(m_latencyDelta) - netPlatform::Core::instance()->randomFloat(m_latencyDelta);
		
		float packetLoss = m_packetLossMed + netPlatform::Core::instance()->randomFloat(m_packetLossDelta) - netPlatform::Core::instance()->randomFloat(m_packetLossDelta);

		if(m_controlPoints.getSize() >= 16)
			m_controlPoints.removeTail();

		m_controlPoints.addHead(ControlPoint(latency, packetLoss));
	}
			
	ControlPoint Conditions::getConditions()
	{
		while(poll());

		ControlPoint points[4];
		
		// http://www.mvps.org/directx/articles/catmull/
		float t = getControlPoints(points[0], points[1], points[2], points[3]);

		ControlPoint p = interpolate(points[0], points[1], points[2], points[3], t);

		if (p.m_latency < 0.0f)
			p.m_latency = 0.0f;

		if (p.m_packetLoss < 0.0f)
			p.m_packetLoss = 0.0f;

		return p;
	}

	float Conditions::getControlPoints(ControlPoint& p0, ControlPoint& p1, ControlPoint& p2, ControlPoint& p3)
	{
		if(m_controlPoints.isEmpty())
		{
			p0 = ControlPoint(0.0f, 0.0f);
			p1 = p0;
			p2 = p0;
			p3 = p0;
			return 0.0f;
		}

		bdLinkedList<ControlPoint>::Position pos = m_controlPoints.getHeadPosition();
		bdLinkedList<ControlPoint>::Position pivot = pos;

		// the time we start interpolating from.
		U64 time = netClock::instance()->now() - (m_tickTime * 4);
		
		while(pos)
		{
			const ControlPoint& point = m_controlPoints.getAt(pos);

			// that point is our pivot point.
			if(point.m_time < time)
			{
				pivot = pos;
				break;
			}
			m_controlPoints.forward(pos);
		}

		// initialise control points to pivot value.
		p0 = m_controlPoints.getAt(pivot);
		p1 = p0;
		p2 = p0;
		p3 = p0;

		// try get the control points before pivot
		pos = pivot;
		m_controlPoints.forward(pos);
		if(pos) { p0 = m_controlPoints.getAt(pos); }

		// try get the control points after pivot
		pos = pivot;
		m_controlPoints.backward(pos);
		if(pos) { p2 = p3 = m_controlPoints.getAt(pos); }
		
		// try get the second control points after pivot
		if(pos) m_controlPoints.backward(pos);
		if(pos) { p3 = m_controlPoints.getAt(pos); }

		if(p2.m_time < p1.m_time + 10) 
			return 0.0f;

		float t = (float)(time - p1.m_time) / (float)(p2.m_time - p1.m_time);
		if(t < 0.0f) t = 0.0f;
		if(t > 1.0f) t = 1.0f;
		return t;
	}

	ControlPoint Conditions::interpolate(const ControlPoint& p0, const ControlPoint& p1, const ControlPoint& p2, const ControlPoint& p3, float t)
	{
		float t2 = t*t;
		float t3 = t2*t;
		
		ControlPoint p =	(	(2*p1)							   + 
								(-p0 + p2)					* (t)  + 
								(2*p0 - 5*p1 + 4*p2 - p3)	* (t2) + 
								(-p0 + 3*p1- 3*p2 + p3)		* (t3)
							) * 0.5f;
		return p;
	}

}

namespace netTransport
{
	SimPacket::SimPacket(bdAddrHandleRef addr, const char* data, int len, U64 time)
	{
		m_addr = addr;
		m_len = len;

		MEMORY_TRACKER_PUSH("network.multiplayer");
		m_data = new char[len];
		MEMORY_TRACKER_POP("network.multiplayer");

		m_time = time;
		memcpy(m_data, data, len);
		m_cachedTime = netClock::instance()->now();
	}

	SimPacket::~SimPacket()
	{
		delete[] m_data;
	}

	bool SimPacket::transmit()
	{
		bdNetImpl *const net = bdNet::getInstance();
		bdSocketRouter* socketRouter = net->getSocketRouter();
		if(!socketRouter) 
			return false;
		
		if(	socketRouter->getStatus() == bdSocketRouter::BD_SOCKET_ROUTER_UNINITIALIZED ||
			socketRouter->getStatus() == bdSocketRouter::BD_SOCKET_ROUTER_ERROR)
			return false;

		F32 latency = (F32)(netClock::instance()->now() - m_cachedTime) / (1000.0f);

		//LOGINFO(Debugging::g_logger, "<Simulator> sending packet len(%d), latency(%d ms).", m_len, (int)latency);

		int sent = socketRouter->sendTo(m_addr, m_data, m_len);
		return (sent >= 0);
	}

	bool SimPacket::poll()
	{
		if(m_time < netClock::instance()->now())
		{
			transmit();
			return false;
		}
		return true;
	}

	Simulator::Simulator(AddressMapRef addressMap)
	{
		m_addressMap = addressMap;
		m_active = false;
	}

	Simulator::~Simulator()
	{
		flush();
	}

	void Simulator::setLatency(float med, float delta) 
	{ 
		m_conditions.setLatency(med, delta);
	}
	
	void Simulator::setPacketLoss(float med, float delta) 
	{
		m_conditions.setPacketLoss(med, delta);
	}
	
	void Simulator::setActive(bool onOff) 
	{ 
		if(m_active && !onOff) 
			flush(); 
	
		m_active = onOff; 
	}
			
	bool Simulator::blocked(bdAddrHandleRef addrHandle) 
	{
		bdAddrHandleRef record;
		return m_blocked.get(bdAddrHandleRefWrapper(addrHandle), record);
	}

	void Simulator::blockAddress(bdAddrHandleRef addrHandle)
	{
		m_blocked.remove(bdAddrHandleRefWrapper(addrHandle));
		m_blocked.put(bdAddrHandleRefWrapper(addrHandle), addrHandle);
	}

	void Simulator::unblockAddress(bdAddrHandleRef addrHandle)
	{
		m_blocked.remove(bdAddrHandleRefWrapper(addrHandle));
	}

	bool Simulator::flush()
	{
		// send all packets
		bdLinkedList<SimPacketRef>::Position p = m_packets.getHeadPosition();
		while(p)
		{
			SimPacketRef packet = m_packets.forward(p);
			packet->transmit();
		}
		m_packets.clear();
		return true;
	}

	int Simulator::sendTo(bdAddrHandleRef addr, const char* data, int len)
	{
		// no simulation active. send directly to router.
		if(!m_active)
		{
			bdNetImpl *const net = bdNet::getInstance();
			bdSocketRouter* socketRouter = net->getSocketRouter();

			if(!socketRouter) 
				return false;
		
			if(	socketRouter->getStatus() == bdSocketRouter::BD_SOCKET_ROUTER_UNINITIALIZED ||
				socketRouter->getStatus() == bdSocketRouter::BD_SOCKET_ROUTER_ERROR)
				return false;
			
			int sent = socketRouter->sendTo(addr, (void*)data, len);
			return sent;
		}
		else
		{
			netSimulator::ControlPoint currentConditions = m_conditions.getConditions();
			float rolldice = netPlatform::Core::instance()->randomFloat(1.0f);

			// packet simulated lost.
			if(rolldice < currentConditions.m_packetLoss) 
				return len;

			// packet simulated blocked.
			if(blocked(addr)) 
				return len;

			// latency simulated time.
			U64 time = netClock::instance()->now() + (U64) (currentConditions.m_latency * (1000.0f * 1000.0f));
							
			// insert packet to cache
			MEMORY_TRACKER_PUSH("network.multiplayer");
			SimPacketRef packet = new SimPacket(addr, data, len, time);
			MEMORY_TRACKER_POP("network.multiplayer");


			bdLinkedList<SimPacketRef>::Position pos = m_packets.getHeadPosition();
			
			while(pos)
			{
				SimPacketRef cached = m_packets.getAt(pos);
				
				if(cached->m_time > packet->m_time)
					break;

				m_packets.forward(pos);
			}
			m_packets.insertBefore(pos, packet);

			// send pending packets, clean up addresses
			poll();

			return len;
		}
	}

	bool Simulator::poll()
	{
		// clear addresses no longer in use
		bdHashMap<bdAddrHandleRefWrapper, bdAddrHandleRef, bdAddrHandleRefWrapper>::Iterator it = m_blocked.getIterator();
		while(it)
		{
			bdAddrHandleRef addr = m_blocked.getValue(it);

			if(!m_addressMap->registeredAddress(addr))
			{
				m_blocked.remove(it);
			}
			else
			{
				m_blocked.next(it);
			}
		}
		m_blocked.releaseIterator(it);

		// poll packets, remove packets that have been sent
		bdLinkedList<SimPacketRef>::Position p = m_packets.getHeadPosition();
		while(p)
		{
			SimPacketRef cached = m_packets.getAt(p);

			if(!cached->poll())
			{
				m_packets.removeAt(p);
			}
			else
			{
				m_packets.forward(p);
			}
		}
		return true;
	}
}
