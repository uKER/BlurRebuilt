// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdAddressMap.h>

#define BD_LOG_LEVEL "addressmap"

bdAddressMap::bdAddressMap()
{
}

bdBool bdAddressMap::getAddrHandle(	bdCommonAddrRef ca, 
									const bdSecurityID &id, 
									bdAddrHandleRef &addrHandle)
{
	bdBool found = findAddrHandle(ca, id, addrHandle);
	if(!found)
	{
		addrHandle = new bdAddrHandle(ca, id);

		if(ca->getTitleId() > 0)
		{
			const DWORD titleid = static_cast<DWORD>(ca->getTitleId());
			IN_ADDR server;
			
			found = XNetTsAddrToInAddr(&(ca->getXNAddr()), titleid, &id, &server) == 0;

			if( found )
			{
				addrHandle->setRealAddr( server, ca->getPort() );
			}
		}
		else
		{
			IN_ADDR peer;
			
			found = XNetXnAddrToInAddr(&(ca->getXNAddr()), &id, &peer) == 0;

			if( found )
			{
				addrHandle->setRealAddr( peer, ca->getPort() );
			}
		}
		m_addrHandles.pushBack(addrHandle);
	}

	return found;
}

bdBool bdAddressMap::findAddrHandle(const bdAddr &ra,
									  bdAddrHandleRef &addrHandle) const
{
	bdBool result = false;

	const bdUInt numAddrHandles = m_addrHandles.getSize();
	bdUInt numFound = 0;

	for (bdUInt i = 0; i < numAddrHandles; i++)
	{
		bdBool found = false;
		const bdAddrHandleRef handle = m_addrHandles[i];

		if (handle.notNull())
		{
			found = (handle->getRealAddr() == ra );			
		}

		if(found)
		{
			if(numFound == 0)
			{
				result = true;
				addrHandle = handle;
			}
			numFound++;
		}
	}

	if(numFound > 1)
	{
		BD_WARN(BD_LOG_LEVEL, "Found %u matching address handles, expected 0 or 1.", numFound);
	}			


	if( result )
	{
		BD_ASSERT( addrHandle.notNull(), "Should not return null addrHandle");
	}

	return result;
}


bdBool bdAddressMap::findAddrHandle(const bdCommonAddrRef ca,
				  					  const bdSecurityID &id,
									  bdAddrHandleRef &addrHandle) const
{
	bdBool result = false;

	if(ca.isNull())
	{
		BD_ERR(BD_LOG_LEVEL, "Invalid common addr reference.");
	}
	else
	{
		const bdEndpoint endpoint(ca, id);
		const bdUInt numAddrHandles = m_addrHandles.getSize();
		bdUInt numFound = 0;

		for (bdUInt i = 0; i < numAddrHandles; i++)
		{
			bdBool found = false;
			const bdAddrHandleRef handle = m_addrHandles[i];

			if (handle.notNull())
			{
				found = (handle->m_endpoint == endpoint );			
			}

			if(found)
			{
				if(numFound == 0)
				{
					result = true;
					addrHandle = handle;
				}
				numFound++;
			}
		}

		if(numFound > 1)
		{
			BD_WARN(BD_LOG_LEVEL, "Found %u matching address handles, expected 0 or 1.", numFound);
		}			
	}

	if( result )
	{
		BD_ASSERT( addrHandle.notNull(), "Should not return null addrHandle");
	}

	return result;
}


bdBool bdAddressMap::addrToCommonAddr(const bdAddrHandleRef &addrHandle, 
										  bdCommonAddrRef &ca, 
										  bdSecurityID &id) const
{
	XNADDR xnAddr;

	const bdBool result = XNetInAddrToXnAddr(addrHandle->m_addr, &xnAddr, &id) == 0;

	if(result)
	{
		if(m_me.notNull())
		{
			ca = new bdCommonAddr(m_me, xnAddr, addrHandle->m_port);
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL,"Invalid addr.");
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL,"Invalid addr handle.");
	}

	return result;
}

bdBool bdAddressMap::unregisterAddr(bdAddrHandleRef &addrHandle)
{
	bdBool result = true;

	if( addrHandle.isNull() )
	{
		BD_ERR(BD_LOG_LEVEL, "A invalid addr handle ref cannot be unregistered.");
		result = false;
	}
	else
	{
		const bdBool isLoopback = addrHandle->m_addr.s_addr == BD_IN_ADDR_LOOPBACK;

		if(!isLoopback)
		{
			if (XNetGetConnectStatus(addrHandle->m_addr) != XNET_CONNECT_STATUS_LOST)
			{
				result = XNetUnregisterInAddr(addrHandle->m_addr) == 0;
			}
		}

		bdBool found = false;
		bdUInt numFound = 0;
		for (bdUInt i = 0; i < m_addrHandles.getSize(); i++)
		{
			const bdAddrHandleRef handle = m_addrHandles[i];

			if (handle.notNull())
			{
				found = (*handle == *addrHandle);
			}
			if (found)
			{
				m_addrHandles.removeAt(i);
				//need to recheck same slot
				i--;
				numFound++;
				result = true;
			}
		}

		if (numFound > 1)
		{
			BD_WARN(BD_LOG_LEVEL, "Found %u matching address handles, expected 0 or 1.", numFound);
		}
		else if (numFound == 0)
		{
			/// This will occur if the address handle has already been unregistered.
			BD_WARN(BD_LOG_LEVEL, "Failed to remove addr handle from address map.");
		}

	}

	return result;
}


bdUWord BD_CALL bdAddressMap::addrToString(const bdAddrHandleRef &addrHandle, 
									   bdNChar8 *const str, 
									   const bdUWord size)
{
	bdUWord strLength = 0;
	const bdInt sizei = static_cast<bdInt>(size);
	BD_ASSERT(static_cast<bdWord>(size) == sizei, "size too large.");

	if(XNetInAddrToString(addrHandle->m_addr, str, sizei) == 0)
	{
		strLength = bdStrlen(str);
	}
	return strLength;
}

void bdAddressMap::setupLocalCommonAddr(bdCommonAddrRef ca)
{
	if(ca.isNull())
	{
		BD_ERR(BD_LOG_LEVEL, "An valid local common addr is required.");
	}
	else
	{
		m_me = ca;
	}
}

// deprecated
bdBool bdAddressMap::commonAddrToAddr(bdCommonAddrRef ca, 
									  const bdSecurityID &id, 
									  bdAddrHandleRef &addrHandle)
{
	BD_WARN(BD_LOG_LEVEL , "This function is deprecated, use getAddrHandle instead. ");
	return getAddrHandle(ca, id , addrHandle);

}

void bdAddressMap::print()
{
	BD_INFO(BD_LOG_LEVEL,"addrMap print begin");
	for( bdUInt i = 0; i < m_addrHandles.getSize(); i++ )
	{
		m_addrHandles[i]->print();
	}
	BD_INFO(BD_LOG_LEVEL,"addrMap print end");
}
