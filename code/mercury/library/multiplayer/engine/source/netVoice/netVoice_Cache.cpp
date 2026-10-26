
#include "stdafx.h"
#include "./netVoice_Cache.h"
#include "netDebug/netDebug.h"
#include "netPlatform/netPlatform_Core.h"
#include <Network/CommunicatorManager.h> 

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netVoice
{
	Cache::Cache()
	{
		m_frameSize = Communicator::C_Core::Instance().AudioFrameSize();
		m_cacheSize	= (MAX_CACHED_VOICE_SIZE / m_frameSize) * m_frameSize;
		m_head		= 0;
		m_tail		= 0;
	}

	Cache::~Cache()
	{}
		
	bool Cache::write(const char* buffer, int len)
	{
		if(!validate())
			return false;

		assert((len % m_frameSize) == 0);

		// sanity
		debugBytes(buffer, len, "write");

		// we'll get stutter in voice but no choice 
		// if we receive too much voice at one time.
		if(len > m_cacheSize) 
		{
			buffer += (len - m_cacheSize);
			len = m_cacheSize;
		}

		int head	= (m_head % m_cacheSize);				// head pointer, wrapped in buffer
		int split	= (m_cacheSize - head);					// max length of first split.
		int l1		= (split < len)? split : len;			// length of first split.
		int l2		= (len - l1);							// length of second split.

		// copy first split
		if(l1 > 0)
		{
			memcpy(m_cache + head, buffer, l1);
		}

		// copy second split
		if(l2 > 0)
		{
			memcpy(m_cache, buffer + l1, l2);
		}

		// update head pointer
		int bytesAppended = (l1 + l2);

		assert((bytesAppended % m_frameSize) == 0);
		
		// push head
		m_head += bytesAppended;

		// bring the tail to range (means remote talkers could miss some voice but hey...).
		if(m_tail + m_cacheSize < m_head)
		{
			LOGWARN(netDebug::g_Log, "<netVoiceCache> voice overflow (%d bytes)...", m_head - (m_tail + m_cacheSize));
			m_tail = (m_head - m_cacheSize);
		}
		return true;
	}

	bool Cache::validate()
	{
		// sanity check
		if(	(m_frameSize <= 0) ||
			(m_head < m_tail) || 
			(m_head < 0) || 
			(m_tail < 0) || 
			(m_tail < m_head - m_cacheSize) ||
			((m_head % m_frameSize) != 0) ||
			((m_tail % m_frameSize) != 0) ||
			((m_cacheSize % m_frameSize) != 0))
		{
			LOGFATAL(netDebug::g_Log, "<netVoice::Cache> cache pointers invalid.");
			clear();
			return false;
		}

		// sanity check
		if(m_head >= 0x70000000)
		{
			LOGWARN(netDebug::g_Log, "<netVoice::Cache> transmitted %.3f Gbytes of voice data! clearing buffer to avoid pointer wrap around.", m_head / (1024.0f * 1024.0f * 1024.0f));
			clear();
			return false;
		}
		return true;
	}

	bool Cache::read(char* buff, int limit, int& len)
	{
		len = 0;

		// sanity check
		if(!validate())
			return false;
				
		// no voice remaining in the buffer
		if(bytes() == 0)
			return false;

		// the total buffer size (aligned to frame size).
		limit -= (limit % m_frameSize);
		if(limit <= 0) return false;
		
		int tail	= (m_tail % m_cacheSize);											// tail pointer, wrapped in cache.
		int pending = (m_head - m_tail);												// number of bytes pending
		int length	= (pending < limit)? pending : limit;								// number of bytes we can read
		int l1		= (length < (m_cacheSize - tail))? length : (m_cacheSize - tail);	// first buffer split (cache is a ring buffer).
		int l2		= (length - l1);													// second buffer split (cache is a ring buffer)

		// buffer too small. abort.
		if(length < m_frameSize) 
			return false;
		
		len = (l1 + l2);
		assert(len <= limit);

		// write data
		if(l1 > 0)
		{
			memcpy(buff, m_cache + tail, l1);
		}

		if(l2 > 0)
		{
			memcpy(buff + l1, m_cache, l2);
		}

		// debug
		debugBytes(buff, len, "read");
		
		// move tail pointer.
		m_tail += len;
		return true;
	}

	bool Cache::debugBytes(const char* ptr, int len, const char* header)
	{
		return false;

		int step = Communicator::C_Core::Instance().AudioFrameSize();

		for(int i = 0; i < len; i += step)
		{
			int c = (step < (len - i))? step : (len - i);

			char str[128];

			netDebug::getByteInfo(ptr + i, c, str, sizeof(str));

			LOGINFO(netDebug::g_Log, "%s (%d, %d) [%s]", header, m_tail, m_head, str);
		}
		return true;
	}

	bool Cache::unread(int bytes)
	{
		m_tail -= bytes;
		return true;
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

