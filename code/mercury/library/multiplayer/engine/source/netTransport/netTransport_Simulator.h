#ifndef NETTRANSPORT_SIMULATOR_H
#define NETTRANSPORT_SIMULATOR_H

#ifdef _MSC_VER
#pragma once
#endif


#include <bdSocket/bdAddrHandleRefWrapper.h>
#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>


namespace netSimulator
{
	struct ControlPoint
	{
		ControlPoint(F32 latency=0.0f, F32 packetLoss=0.0f);

		ControlPoint& operator-=(const ControlPoint& other) { m_latency -= other.m_latency; m_packetLoss -= other.m_packetLoss; return *this; }
		ControlPoint& operator+=(const ControlPoint& other) { m_latency += other.m_latency; m_packetLoss += other.m_packetLoss; return *this; }
		ControlPoint  operator- (const ControlPoint& other) const { ControlPoint temp(*this); return temp -= other; }
		ControlPoint  operator+ (const ControlPoint& other) const { ControlPoint temp(*this); return temp += other; }
		
		ControlPoint& operator*=(const float k) { m_latency *= k; m_packetLoss *= k; return *this; }
		ControlPoint& operator/=(const float k) { m_latency /= k; m_packetLoss /= k; return *this; }
		ControlPoint  operator* (const float k) const { ControlPoint temp(*this); return temp *= k; }
		ControlPoint  operator/ (const float k) const { ControlPoint temp(*this); return temp /= k; }
		
		friend ControlPoint operator * (const float k, const ControlPoint& point) { return point * k; }
		ControlPoint operator-() const { return ControlPoint(-m_latency, -m_packetLoss); }
	
		F32 m_latency; 
		F32 m_packetLoss;
		U64 m_time;
	};

	struct Conditions
	{
		Conditions();

		ControlPoint getConditions();
		void setLatency(float med, float delta=-1.0f);
		void setPacketLoss(float med, float delta=-1.0f);
	
	private:
		bool poll();
		float getControlPoints(ControlPoint& p0, ControlPoint& p1, ControlPoint& p2, ControlPoint& p3);
		ControlPoint interpolate(const ControlPoint& p0, const ControlPoint& p1, const ControlPoint& p2, const ControlPoint& p3, float t);
		void addControlPoint();
		
		float m_latencyMed;
		float m_latencyDelta;
		float m_packetLossMed;
		float m_packetLossDelta;
		U64	  m_tickTime;
		bdLinkedList<ControlPoint> m_controlPoints;
	};
};

namespace netTransport
{
	BD_REFERENCE(SimPacket);
	class SimPacket: public bdReferencable
	{
	public:
		SimPacket(bdAddrHandleRef addr, const char* data, int len, U64 time);
		~SimPacket();
		bool transmit();
		bool poll();
	
	public:
		bdAddrHandleRef m_addr;
		char* m_data;
		int m_len;
		U64 m_time;
		U64 m_cachedTime;
	};

	BD_REFERENCE(AddressMap);
	BD_REFERENCE(Simulator);
	class Simulator: public bdReferencable
	{
	public:
		Simulator(AddressMapRef addressMap);
		virtual ~Simulator();

		void unblockAddress(bdAddrHandleRef addrHandle);
		void blockAddress(bdAddrHandleRef addrHandle);

		void setLatency(float med, float delta=-1.0f);
		void setPacketLoss(float med, float delta=-1.0f);
		void setActive(bool onOff);
				
		bool active() const { return m_active; }
		bool blocked(bdAddrHandleRef addrHandle);

		bool poll();
		bool flush();
		int sendTo(bdAddrHandleRef addr, const char* data, int len);

		bool						m_active;
		netSimulator::Conditions	m_conditions;
		AddressMapRef				m_addressMap;
		bdLinkedList<SimPacketRef>	m_packets;
		bdHashMap<bdAddrHandleRefWrapper, bdAddrHandleRef, bdAddrHandleRefWrapper> m_blocked;
	};
}

#endif //NETTRANSPORT_SIMULATOR_H