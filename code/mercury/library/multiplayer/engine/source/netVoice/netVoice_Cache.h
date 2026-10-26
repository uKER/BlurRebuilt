#ifndef NETVOICE_CACHE_H
#define NETVOICE_CACHE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netEnums/netEnums.h"
#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>

namespace netVoice
{
	BD_REFERENCE(Cache);
	class Cache: public bdReferencable
	{
	public:
		Cache();
		~Cache();
		
		bool unread(int bytes);
		bool read(char* buff, int limit, int& len);
		bool write(const char* buff, int len);
	
		bool clear() { m_head = 0; m_tail = 0; return true; }
		int bytes() const { return (m_head - m_tail); }
		int bytesLeft() const { return m_cacheSize - bytes(); }
		bool empty() const { return (m_head <= m_tail); }
		
		enum { MAX_CACHED_VOICE_SIZE = 32 * 20 };
		int							m_frameSize;					// voice packet from communicator.
		int							m_cacheSize;					// cache size ( a multiple of frameSize)
		int							m_head;							// cache head
		int							m_tail;							// cache tail
		unsigned char				m_cache[MAX_CACHED_VOICE_SIZE];	// cached voice data

		bool debugBytes(const char* ptr, int len, const char* header);
		bool validate();
	};
}

#endif //NETVOICE_CACHE_H