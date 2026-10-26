// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdAddressMap.h>

#define BD_LOG_LEVEL "bdSocket/addrmap"

bdAddressMap::bdAddressMap()
{
}

bdBool bdAddressMap::getAddrHandle(const bdCommonAddrRef ca,
								   const bdSecurityID &id,
								   bdAddrHandleRef &addrHandle)
{
	bdBool found = findAddrHandle(ca, id, addrHandle);
	if(!found)
	{
		found = true;
		addrHandle = new bdAddrHandle(ca, id);
		m_addrHandles.pushBack(addrHandle);
	}

	return found;
}

bdBool bdAddressMap::findAddrHandle(const bdCommonAddrRef ca,
									const bdSecurityID &id,
									bdAddrHandleRef &addrHandle) const
{
	bdBool result = false;

#if defined(BD_DEBUG)
	BD_ASSERT(id != bdSecurityID(), "Invalid security ID");
#endif

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
				found = (handle->m_endpoint == endpoint);				
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
	return result;
}


bdBool bdAddressMap::addrToCommonAddr(	const bdAddrHandleRef &addrHandle,
										bdCommonAddrRef &ca,
										bdSecurityID &id) const
{
	bdBool result = false;
	if(addrHandle.isNull())
	{
		BD_ERR(BD_LOG_LEVEL, "A invalid addr handle ref cannot be used.");
	}
	else
	{
		ca = addrHandle->m_endpoint.getCommonAddr();
		id = addrHandle->m_endpoint.getSecID();
		result = true;

		if(ca.isNull())
		{
			BD_WARN(BD_LOG_LEVEL, "Addr handle ref has invalid common addr.");
		}
	}
	return result;
}

bdBool bdAddressMap::unregisterAddr(bdAddrHandleRef &addrHandle)
{
	bdBool result = false;
	if(addrHandle.isNull())
	{
		BD_ERR(BD_LOG_LEVEL, "A invalid addr handle ref cannot be unregistered.");
	}
	else
	{
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
		// set the handle to unresolved.
		addrHandle->setStatus(bdAddrHandle::BD_ADDR_UNRESOLVED);
	}

	return result;
}

bdBool bdAddressMap::unregisterRealAddr(const bdAddr realAddr)
{
	bdBool ok = false;
	for (bdUInt i = 0; i < m_addrHandles.getSize(); i++)
	{
		const bdAddrHandleRef handle = m_addrHandles[i];
		if(handle.notNull() && handle->getRealAddr() == realAddr)
		{
			//We just set the status and let unregisterAddr be called in cleanup
			handle->setStatus(bdAddrHandle::BD_ADDR_UNRESOLVED);
			ok = true;
		}
	}

	if(!ok)
	{
		bdNChar8 addrString[BD_ADDR_STRING_SIZE];
		bdMemset(addrString, BD_NULL, sizeof(addrString));
		realAddr.toString(addrString, sizeof(addrString));
		BD_WARN(BD_LOG_LEVEL, "No address handles found matching the real address %s.", addrString);
	}

	return ok;
}

bdUWord BD_CALL bdAddressMap::addrToString(	const bdAddrHandleRef &addrHandle,
											bdNChar8 *const str,
											const bdUWord size)
{
	bdUWord strLength = 0;

	if(addrHandle.isNull())
	{
		const bdNChar8 *const nullRefStr = "Null addr handle ref";
		bdStrlcpy(str, nullRefStr, size);

		// return the desired string length (even is strLength > size)
		strLength = bdStrlen(nullRefStr);
	}
	else if(addrHandle->getStatus() == bdAddrHandle::BD_ADDR_RESOLVED)
	{
		strLength = addrHandle->getRealAddr().toString(str, size);
	}
	else
	{
		// check if it's loopback
		if(addrHandle->m_endpoint.getCommonAddr() && addrHandle->m_endpoint.getCommonAddr()->isLoopback())
		{
			const bdNChar8 *const loopbackStr = "loopback";
			bdStrlcpy(str, loopbackStr, size);

			// return the desired string length (even is strLength > size)
			strLength = bdStrlen(loopbackStr);
		}
		else
		{
			const bdNChar8 *const notResolvedStr = "not resolved";
			bdStrlcpy(str, notResolvedStr, size);

			// return the desired string length (even is strLength > size)
			strLength = bdStrlen(notResolvedStr);
		}
	}

	return strLength;
}

void bdAddressMap::clear()
{
	m_addrHandles.clear();
}

// deprecated
bdBool bdAddressMap::commonAddrToAddr(const bdCommonAddrRef ca,
									  const bdSecurityID &id,
									  bdAddrHandleRef &addrHandle)
{
	BD_WARN(BD_LOG_LEVEL , "This function is deprecated, use getAddrHandle instead. ");
	return getAddrHandle(ca, id , addrHandle);
}

