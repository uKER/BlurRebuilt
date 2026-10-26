#ifndef NETTRANSPORT_ADDR_H
#define NETTRANSPORT_ADDR_H

#ifdef _MSC_VER
#pragma once
#endif


#include <bdCore/bdReference/bdReference.h>
#include <bdcore\bdsocket\bdcommonaddr.h>
#include "netStream/netStream.h"
#include "netStream/netSerialise.h"

BD_REFERENCE(bdCommonAddr);

namespace netTransport
{
	struct Addr: public netSerialisable
	{
		Addr();

		const char* toString(char* str, int len) const;
		bool toAddr(bdCommonAddrRef addr) const;	
		bool fromAddr(bdCommonAddrRef addr);
		Addr& operator=(bdCommonAddrRef addr) { fromAddr(addr); return *this; }
		
		bool operator == (bdCommonAddrRef addr) const;
		bool operator != (bdCommonAddrRef addr) const { return !(*(this) == addr); };
		
		bool operator == (const Addr& addr) const;
		bool operator != (const Addr& addr) const { return !(*(this) == addr); };
		bool isLoopback() const;

		bool serialise(netStream* stream) { stream->writeBytes(m_buffer, sizeof(m_buffer)); return stream->ok(); }
		bool deserialise(netStream* stream) { stream->readBytes(m_buffer, sizeof(m_buffer)); return stream->ok(); }

		bdUByte8 m_buffer[BD_COMMON_ADDR_SERIALIZED_SIZE];		// the address serialisation
	};
}

#endif //NETTRANSPORT_ADDR_H