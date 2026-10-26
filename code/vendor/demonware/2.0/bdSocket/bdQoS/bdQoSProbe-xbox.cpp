// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdQoS/bdQoSProbe.h>
#include <bdSocket/bdQoS/bdQoSProbeListener.h>
#include <WinSockX.h>

#define BD_LOG_LEVEL "bdSocket/qos"

// Recommended Value For This Is 8
#define BD_QOS_XBOX_PROBES		(8u)

// As per the XBox 360 docs this value should be exceeded.
// Value is in bits..
#define BD_MAX_XBOX_QOS_BANDWIDTH		(65536u)

// This is a recommendation rather than an absolute.
BD_COMPILE_ASSERT(BD_QOS_MAX_BANDWIDTH_DEFAULT <= BD_MAX_XBOX_QOS_BANDWIDTH , 
					MAX_BANDWIDTH_FOR_XBOX_QOS_ABOVE_RECOMMENED_MAXIMUM);

// As per the XBox 360 docs, unless the cfgQosDataLimitDiv4 member of the 
// XNetStartupParams structure is set when calling  XNetStartup  the max
// size for the data transmitted it 512 bytes
#define BD_MAX_XBOX_QOS_PROBE_RESPONSE_DATA_SIZE (512u)

bdQoSProbe::bdQoSProbe()
: m_maxBandwidth(BD_QOS_MAX_BANDWIDTH_DEFAULT)
{
#ifdef BD_PLATFORM_XENON
	m_listenerSecIDInitialised = false;
#endif
}

void bdQoSProbe::setMaxBandwidth(const bdUInt maxBandwidth)
{
	m_maxBandwidth = maxBandwidth;
	if(m_maxBandwidth > BD_MAX_XBOX_QOS_BANDWIDTH)
	{
		BD_WARN(BD_LOG_LEVEL , "Setting the max QoS bandwidth to %u .This is higher than recommended max %u", m_maxBandwidth ,  BD_MAX_XBOX_QOS_BANDWIDTH);
	}

#ifdef BD_PLATFORM_XENON
	updateListenerBandwidth();
#endif
}

bdBool bdQoSProbe::probe(bdQoSRemoteAddr &addr, bdQoSProbeListener *listener)
{
	bdBool result = false;
	bdArray<bdQoSRemoteAddr> remoteAddrs;
	remoteAddrs.pushBack(addr);
	result =  probe(remoteAddrs, listener);
	return result;
}

bdBool bdQoSProbe::probe(const bdArray<bdQoSRemoteAddr> &addrs, bdQoSProbeListener *listener)
{
	bdBool result = false;

	const XNADDR ** xnaddrs = new const XNADDR* [addrs.getSize()];
	const XNKID **xkids = new const XNKID*[addrs.getSize()];
	const XNKEY **xkeys = new const XNKEY*[addrs.getSize()];

	BD_ASSERT(xnaddrs && xkids && xkeys, "Failed to allocate temporary buffers");

	for (bdUInt i=0; i < addrs.getSize(); i++)
	{
		xnaddrs[i] = &addrs[i].m_addr->getXNAddr();
		xkids[i] = &addrs[i].m_id;
		xkeys[i] = &addrs[i].m_key;
	}

	XNQOS *res;

	const UINT numAddrsToProbe = addrs.getSize();
	const UINT numSecurityGatewaysToProbe = 0;
	
	// Must be set to 0, as no flags are currently supported.
	const DWORD flags = 0;
	WSAEVENT* eventHandler = BD_NULL;

	const bdInt QoSInitResult = XNetQosLookup(numAddrsToProbe,
											xnaddrs,
											xkids,
											xkeys,
											numSecurityGatewaysToProbe,
											BD_NULL,
											BD_NULL,
											BD_QOS_XBOX_PROBES,
											m_maxBandwidth,
											flags,
											eventHandler,
											&res);

	delete[] xnaddrs;
	delete[] xkids;
	delete[] xkeys;

	// return value 0 indicates success.
	if(QoSInitResult == 0)
	{
		bdArray<bdCommonAddrRef> caddrs(addrs.getSize(), BD_NULL);
		for (bdUInt i=0; i < addrs.getSize(); i++)
		{
			caddrs.setGrow(i, addrs[i].m_addr);
		}
		bdQoSProbeEntryWrapper* nextEntry = new bdQoSProbeEntryWrapper(caddrs, res, listener);
		m_requests.addTail(nextEntry);
		result = true;
	}
	else
	{
		// An error has occurred.
		// There is no info in the docs on what errors to expect here :)
		BD_ERR(BD_LOG_LEVEL, "XNetQosLookup failed to start ,Error Code: %i.", QoSInitResult);
	}
	return result;
}

void bdQoSProbe::cancelProbes()
{
	bdLinkedList<bdQoSProbeEntryWrapper*>::Position it = m_requests.getHeadPosition();

	while (it)
	{
		bdQoSProbeEntryWrapper* currentEntry = m_requests.forward(it);

		const INT QoSReleaseResult = XNetQosRelease(currentEntry->m_xnqos);
		if(QoSReleaseResult != 0)
		{
			// An error has occurred.
			// There is no info in the docs on what errors to expect here :)
			BD_ERR(BD_LOG_LEVEL, "XNetQosRelease failed ,Error Code: %i.", QoSReleaseResult);
		}
		delete currentEntry;
	}
	m_requests.clear();
}

/// \todo Need to XNetQosRelease once probe is completed to free memory...
void bdQoSProbe::pump()
{
	bdLinkedList<bdQoSProbeEntryWrapper*>::Position it = m_requests.getHeadPosition();

	while (it)
	{
		bdQoSProbeEntryWrapper *probe = m_requests.getAt(it);
		XNQOS *entry = probe->m_xnqos;
		const bdUInt completedProbes = entry->cxnqos - entry->cxnqosPending;

		if (completedProbes != probe->m_firedEvents)
		{
			BD_ASSERT(entry->cxnqos == probe->m_addrs.getSize(), "Number of stored probes and addresses don't match.");
			// We should fire some more events
			//probe->m_firedEvents = completedProbes;

			for (bdUInt j=0; j < entry->cxnqos; j++)
			{
				if ((entry->axnqosinfo[j].bFlags & XNET_XNQOSINFO_COMPLETE) != 0
					&& (entry->axnqosinfo[j].bFlags & XNET_XNQOSINFO_TARGET_CONTACTED) != 0
					&& (probe->m_addrs[j].notNull()))
				{
					// We managed to connect. Either the state is disabled, or valid QoS data
					// was returned.

					bdQoSProbeInfo info;
					
					info.m_addr = probe->m_addrs[j];
					info.m_disabled = (entry->axnqosinfo[j].bFlags & XNET_XNQOSINFO_TARGET_DISABLED) != 0;
					info.m_bandwidthDown = entry->axnqosinfo[j].dwDnBitsPerSec;
					info.m_bandwidthUp = entry->axnqosinfo[j].dwUpBitsPerSec;
					info.m_latency = ((bdFloat32)entry->axnqosinfo[j].wRttMedInMsecs)/1000.f;

					info.m_dataSize = entry->axnqosinfo[j].cbData;
					info.m_data = entry->axnqosinfo[j].pbData;

					probe->m_listener->onQoSProbeSuccess(info);

					probe->m_addrs[j] = BD_NULL;
					probe->m_firedEvents++;
				}
				else if ((entry->axnqosinfo[j].bFlags & XNET_XNQOSINFO_COMPLETE) != 0
					&& (entry->axnqosinfo[j].bFlags & XNET_XNQOSINFO_TARGET_CONTACTED) == 0
					&& probe->m_addrs[j].notNull())
				{
					// Probe failed.
					probe->m_listener->onQoSProbeFail(probe->m_addrs[j]);
					probe->m_addrs[j] = BD_NULL;
					probe->m_firedEvents++;
				}
			}

			BD_ASSERT(probe->m_firedEvents == completedProbes, "Didn't fire some events.");

			if (probe->m_firedEvents == probe->m_addrs.getSize())
			{
				// All events for this probe has been fired.
				delete probe;
				m_requests.removeAt(it);
			}
			else
			{
				m_requests.forward(it);
			}
		}
		else
		{
			m_requests.forward(it);
		}
	}
}

bdBool bdQoSProbe::listen(const bdSecurityID &secid, bdUByte8 *data, bdUInt dataSize)
{
	bdBool result = false;

	if(dataSize > BD_MAX_XBOX_QOS_PROBE_RESPONSE_DATA_SIZE)
	{
		// See comment on BD_MAX_XBOX_QOS_PROBE_RESPONSE_DATA_SIZE
		BD_WARN(BD_LOG_LEVEL, "dataSize greater than default data size");
	}

	DWORD flags = XNET_QOS_LISTEN_ENABLE | XNET_QOS_LISTEN_SET_DATA | XNET_QOS_LISTEN_SET_BITSPERSEC;
	const HRESULT qosListenResult = XNetQosListen(&secid, 
													data, 
													dataSize, 
													m_maxBandwidth,
													flags);

	if (qosListenResult == 0)
	{
		m_id = secid;
		m_listenerSecIDInitialised = true;
		result = true;
	}
	else if(0x00002726 == qosListenResult)
	{
		// An invalid argument was supplied
		// This may be caused by:
		//  - XNetRegisterKey has not been called.
		BD_ERR(BD_LOG_LEVEL, "An invalid argument was supplied to XNetQosListen.");
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL, "XNetQosListen failed with %i.", qosListenResult);
	}

	return result;
}

bdBool bdQoSProbe::setData(bdUByte8 *data, bdUInt dataSize)
{
	bdBool result = false;

	if(dataSize > BD_MAX_XBOX_QOS_PROBE_RESPONSE_DATA_SIZE)
	{
		// See comment on BD_MAX_XBOX_QOS_PROBE_RESPONSE_DATA_SIZE
		BD_WARN(BD_LOG_LEVEL, "dataSize greater than default data size");
	}

	if(true == m_listenerSecIDInitialised)
	{
		const DWORD flags = XNET_QOS_LISTEN_SET_DATA;
		const bdInt qosListenResult = XNetQosListen(&m_id, 
													data, 
													dataSize, 
													0,
													flags);

		if (qosListenResult == 0)
		{
			result = true;
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "XNetQosListen setData failed with %i.", qosListenResult);
		}
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL, "SecID for listener not initialised.");
	}
	return result;
}

void bdQoSProbe::disableListener()
{
	bdBool result = false;
	if(true == m_listenerSecIDInitialised)
	{
		const DWORD flags = XNET_QOS_LISTEN_DISABLE;
		const bdInt qosListenResult = XNetQosListen(&m_id, 
													NULL, 
													0, 
													0, 
													flags);
		if (qosListenResult == 0)
		{
			result = true;
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "XNetQosListen disable  failed with %i.", qosListenResult);
		}
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL, "SecID for listener not initialised.");
	}

}

void bdQoSProbe::enableListener()
{
	bdBool result = false;
	if(true == m_listenerSecIDInitialised)
	{
		const DWORD flags = XNET_QOS_LISTEN_ENABLE;
		const bdInt qosListenResult = XNetQosListen(&m_id, 
													NULL, 
													0, 
													0, 
													flags);
		if (qosListenResult == 0)
		{
			result = true;
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "XNetQosListen disable  failed with %i.", qosListenResult);
		}
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL, "SecID for listener not initialised.");
	}

}

void bdQoSProbe::shutdownListener()
{
	bdBool result = false;
	if(true == m_listenerSecIDInitialised)
	{
		const DWORD flags = XNET_QOS_LISTEN_RELEASE;
		const bdInt qosListenResult = XNetQosListen(&m_id, 
													NULL, 
													0, 
													0, 
													flags);
		if (qosListenResult == 0)
		{
			result = true;
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "XNetQosListen release failed with %i.", qosListenResult);
		}
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL, "SecID for listener not initialised.");
	}
}

bdBool bdQoSProbe::isProbing() const
{
	const bdBool activeRequests = (m_requests.getSize() > 0);
	return activeRequests;
}

bdBool bdQoSProbe::updateListenerBandwidth()
{
	bdBool result = false;
	if(true == m_listenerSecIDInitialised)
	{
		const DWORD flags = XNET_QOS_LISTEN_SET_BITSPERSEC;
		const bdInt qosListenResult = XNetQosListen(&m_id, 
													NULL, 
													0, 
													m_maxBandwidth, 
													flags);
		if (qosListenResult == 0)
		{
			result = true;
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "XNetQosListen setBandwidth failed with %i.", qosListenResult);
		}
	}
	else
	{
		BD_ERR(BD_LOG_LEVEL, "SecID for listener not initialised.");
	}
	return result;
}
