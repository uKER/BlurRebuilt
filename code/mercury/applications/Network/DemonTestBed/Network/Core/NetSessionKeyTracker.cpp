#include <stdafx.h>
#include <Core/core.h>
#include <network/Core.h>
#include "../Misc/NetDebug.h"
#include "NetSessionKeyTracker.h"
#include <bdNet/bdNet.h>
#include <bdCore/bdCore.h>

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

namespace RacerNet
{	
	C_NetSessionKeyTracker::C_NetSessionKeyTracker() : m_Active(false)
	{
	}

	C_NetSessionKeyTracker::~C_NetSessionKeyTracker()
	{
		Flush();
	}

	void C_NetSessionKeyTracker::Add(const bdSecurityID& keyId)
	{
#if !defined(PLATFORM_XENON)
		if (m_Active)
		{
			// if we are already active then move the current active key to our previous list.
			if (m_Previous.Size() != m_Previous.Capacity())
				m_Previous.PushBack(m_Current);
			else
			{
				LOGFATAL(g_Log, "C_NetSessionKeyTracker::Add() - not enough storage allocated to track active secure keys.");
			}
		}

		m_Active = true;
		m_Current = keyId;
#endif 
	}

	void C_NetSessionKeyTracker::Poll(const SecurityKeyIds& inUse)
	{
#if !defined (PLATFORM_XENON)
		if (m_Previous.Size() == 0)
			return;
		SecurityKeyIds toRemove;

		for (S32 i=0; i<m_Previous.Size(); i++)
		{
			bool keyActive = false;
			for (S32 z=0; !keyActive && z<inUse.Size(); z++)
			{
				if (m_Previous[i] == inUse[z])
					keyActive = true;
			}

			if (!keyActive)
				toRemove.PushBack(m_Previous[i]);
		}

		// unregistre keys flagged as no longer in use by any connected peer.
		for (S32 i=0; i<toRemove.Size(); i++)
		{
			Unregister(toRemove[i], true);

			// find key in previous list and remove.
			bool done = false;
			for (S32 z=0; !done && z<m_Previous.Size(); z++)
			{
				if (toRemove[i] == m_Previous[z])
				{
					m_Previous.EraseByIndex(z);
					done = true;
				}
			}
		}
#endif
	}

	void C_NetSessionKeyTracker::Flush(void)
	{
#if !defined (PLATFORM_XENON)

		for (S32 i=0; i<m_Previous.Size(); i++)
			Unregister(m_Previous[i]);

		if (m_Active)
			Unregister(m_Current);

		m_Active = false;
		m_Previous.Clear();
#endif
	}	
	
	void C_NetSessionKeyTracker::Unregister(const bdSecurityID& id, const bool warn)
	{
#if !defined (PLATFORM_XENON)
		bdNetImpl *const net = bdNet::getInstance();
		if (net->getStatus() == bdNetImpl::BD_NET_DONE)
		{
			bdSocketRouter* router =  net->getSocketRouter();
			if (router)
			{
				bdSecurityKeyMap* map = router->getKeyMap();
				if (map)
				{
					map->unregisterKey(id);	

					if (warn)
					{
						LOGINFO(g_Log, " C_NetSessionKeyTracker::Unregister() - unregistering old security key as no connections use it anymore.");
					}
				}
			}
		}
#endif
	}
} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif
