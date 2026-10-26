
#include "stdafx.h"
#include "netTransport_Peer.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netTransport
{

	static int comparePeers(const void* ptr1, const void* ptr2)
	{
		const Peer* peer1 = (const Peer*)(ptr1);
		const Peer* peer2 = (const Peer*)(ptr2);

		return (peer1->m_guid - peer2->m_guid);
	}
		
	void Peer::sortPeers(Peer* peers, int peerCount)
	{
		qsort(peers, peerCount, sizeof(peers[0]), comparePeers);
	}

	const Peer* Peer::findPeer(const ConnGuid& guid, const Peer* peers, int peerCount)
	{
		for(int i = 0; i < peerCount; i ++)
		{
			if(peers[i].m_guid == guid)
				return &peers[i];
		}
		return NULL;
	}
}
	
#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

