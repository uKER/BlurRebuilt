#include <StdAfx.h>
#include <Network\CommunicatorPacket.h>
#include <Network\CommunicatorPacketCache.h>


namespace Communicator
{
	// constructor
	C_PacketCache::C_PacketCache() : m_Initialised(false), m_pPacketStorage(NULL), m_pPacketList(NULL), m_pPacketPool(NULL)
	{
	}


	// destructor
	C_PacketCache::~C_PacketCache()
	{
		Shutdown();
	}


	// Initialise dynamic resources..
	bool C_PacketCache::Initialise(const U32 packetcCacheSize)
	{
		if (m_Initialised)
			return true;
			
		if (!packetcCacheSize)
			return false;

		m_pPacketStorage = new C_Packet[packetcCacheSize];
		m_pPacketPool	= reinterpret_cast<void*>(new C_PacketList);
		m_pPacketList	= reinterpret_cast<void*>(new C_PacketList);

		if (m_pPacketStorage && m_pPacketList && m_pPacketPool)
		{
			C_PacketList* pPool = reinterpret_cast<C_PacketList*>(m_pPacketPool);
			for (U32 i=0; i<packetcCacheSize; i++)
				pPool->PushBack(m_pPacketStorage[i]);
			m_Initialised = true;
		}
		else
			CleanUp();

		return true;
	}

	
	// totally shutdown the system.
	void C_PacketCache::Shutdown(void)
	{
		CleanUp();
	}


	// remove any cached voice packets from the list
	void C_PacketCache::Flush(void)
	{
		if (!m_Initialised)
			return;

		C_PacketList* pList = reinterpret_cast<C_PacketList*>(m_pPacketList);
		C_PacketList* pPool = reinterpret_cast<C_PacketList*>(m_pPacketPool);
		while (!pList->Empty())
		{
			C_PacketList::Iterator f = pList->Begin();
			pList->Erase(f);
			pPool->PushBack(*f);
		}
	}


	/// try and cache a set number of packets
	bool C_PacketCache::Cache(const Packet* pPackets, const U32 numPackets)
	{
		if (!m_Initialised)
			return false;
		
		if (NULL == pPackets || 0 == numPackets)
			return false;
			
		C_PacketList* pList = reinterpret_cast<C_PacketList*>(m_pPacketList);
		C_PacketList* pPool = reinterpret_cast<C_PacketList*>(m_pPacketPool);

		bool printedWrap = false;
		for (U32 i=0; i<numPackets; i++)
		{
			C_PacketList::Iterator iter;
			if (pPool->Empty())
			{
				// erase first item in main list.. ring buffer
				iter = pList->Begin();
				pList->Erase(pList->Begin());

				if (!printedWrap)
				{
					printedWrap = true;
					LOGWARN( g_Log, "C_PacketCache::Cache() - Wrapped !!\n");
				}
			}
			else
			{
				iter = pPool->Begin();
				pPool->Erase(pPool->Begin());
			}

			// copy voice packet and add to end of list.
			iter->m_VoiceData = pPackets[i];
			pList->PushBack(*iter);
		}

		return true;
	}

	
	// how many packets are curently cached..
	U16	C_PacketCache::NumCached(void)
	{
		if (!m_Initialised)
			return 0;

		C_PacketList* pList = reinterpret_cast<C_PacketList*>(m_pPacketList);
		return pList->Size();
	}


	// how many free slots we have left.
	U16 C_PacketCache::Remaining(void)
	{
		if (!m_Initialised)
			return 0;

		C_PacketList* pPool = reinterpret_cast<C_PacketList*>(m_pPacketPool);
		return pPool->Size();
	}


	// extract voice date from the cache into an external buffer.
	bool C_PacketCache::Extract(Packet* pOutPacketBuffer, U32 outBufferPacketCount, U32& numPacketsExtracted)
	{
		numPacketsExtracted = 0;
		if (!m_Initialised)
			return false;

		if (NULL == pOutPacketBuffer || 0 == outBufferPacketCount)
			return false;

		C_PacketList* pList = reinterpret_cast<C_PacketList*>(m_pPacketList);
		C_PacketList* pPool = reinterpret_cast<C_PacketList*>(m_pPacketPool);

		const U32 cached = pList->Size();
		if (cached)
		{
			numPacketsExtracted = (cached < outBufferPacketCount) ? cached : outBufferPacketCount ;
			for (U32 i=0; i<numPacketsExtracted; i++)
			{
				C_PacketList::Iterator exIt = pList->Begin();

				// copy into output buffer..
				pOutPacketBuffer[i] = exIt->m_VoiceData;

				// give item back to pool..
				C_PacketList::Iterator cur = exIt;
				exIt = pList->Erase(cur);
				pPool->PushBack(*cur);
			}
		}

		return true;
	}


	// private function to release all dunamic resources..
	void C_PacketCache::CleanUp(void)
	{
		if (m_pPacketList)
		{
			C_PacketList* p = reinterpret_cast<C_PacketList*>(m_pPacketList);
			p->Clear();
			delete p;
			m_pPacketList = NULL;
		}

		if (m_pPacketPool)
		{
			C_PacketList* p = reinterpret_cast<C_PacketList*>(m_pPacketPool);
			p->Clear();
			delete p;
			m_pPacketPool = NULL;
		}

		if (m_pPacketStorage)
		{
			delete [] m_pPacketStorage;
			m_pPacketStorage = NULL;
		}
		m_Initialised = false;
	}
}
