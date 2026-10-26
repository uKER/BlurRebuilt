
#include "stdafx.h"
#include "./netTransport_Enums.h"
#include "./netTransport_AddressMap.h"
#include "netPlatform/netPlatform_Core.h"

#include <bdNet\bdNet.h>
#include <bdSocket\bdSocketRouter.h>
#include <bdSocket\bdAddrHandle.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netTransport
{
	Security::Security(bool online, const bdSecurityID& secId, const bdSecurityKey &secKey)
	: m_secId(secId)
	, m_secKey(secKey)
	, m_registered(false)
	, m_online(online)
	{
		m_registered = netPlatform::Core::instance()->registerKey(online, secId, secKey);
	}

	Security::~Security()
	{
		netPlatform::Core::instance()->unregisterKey(m_online, m_secId, m_secKey);
	}

	// NULL address used to register stand-alone keys.
	Address::Address(SecurityRef security)
	: m_security(security)
	{}

	Address::Address(const Addr& addr, SecurityRef security)
	: m_addr(addr)
	, m_security(security)
	{
		m_commonAddr = new bdCommonAddr;
		m_addr.toAddr(m_commonAddr);
		m_addrHandle = netPlatform::Core::instance()->registerAddress(m_commonAddr, security->m_secId);
	}

	Address::~Address()
	{
		if(m_addrHandle.notNull())
			netPlatform::Core::instance()->unregisterAddress(m_addrHandle);
	}

	AddressMap::AddressMap()
	{}

	AddressMap::~AddressMap()
	{
		m_addresses.clear();
	}

	Addr AddressMap::myAddr()
	{
		netTransport::Addr addr;
		bdCommonAddrRef commonAddr = netPlatform::Core::instance()->myCommonAddr();
		if(commonAddr.isNull()) return addr;
		addr.fromAddr(commonAddr);
		return addr;
	}

	SecurityRef AddressMap::findSecurity(const bdSecurityID& secId, const bdSecurityKey &secKey)
	{
		bdLinkedList<AddressRef>::Position pos = m_addresses.getHeadPosition();
		while(pos)
		{
			AddressRef address = m_addresses.forward(pos);
			
			if(	netPlatform::Core::instance()->sameSecurityId(address->m_security->m_secId, secId) && 
				netPlatform::Core::instance()->sameSecurityKey(address->m_security->m_secKey, secKey))
			{
				return address->m_security;
			}
		}
		return NULL;
	}

	// key registration, not attached to an address
	bool AddressMap::registerKey(bool online, const bdSecurityID& secId, const bdSecurityKey &secKey)
	{
		SecurityRef security = findSecurity(secId, secKey);
		if(security.notNull())
		{
			LOGERROR(Debugging::g_logger, "<netTransport::AddressMap> key already registered???");
			return false;
		}

		security = new Security(online, secId, secKey);
		if(!security->m_registered)
		{
			LOGFATAL(Debugging::g_logger, "<netTransport::AddressMap> key registration failed???");
			return false;
		}

		LOGINFO(Debugging::g_logger, "<netTransport::AddressMap> dangling key added.");
		
		AddressRef address = new Address(security);
		m_addresses.addTail(address);
		return true;
	}

	bool AddressMap::unregisterKey(const bdSecurityID& secId, const bdSecurityKey &secKey)
	{
		// security reference
		SecurityRef security = findSecurity(secId, secKey);
		
		if(security.isNull())
			return false;

		bdLinkedList<AddressRef>::Position pos = m_addresses.getHeadPosition();
		while(pos)
		{
			AddressRef address = m_addresses.getAt(pos);

			// this is the dangling key we are looking for
			if(address->m_addrHandle.isNull() && (address->m_security == security))
			{
				m_addresses.removeAt(pos);
				return true;
			}
			else
			{
				m_addresses.forward(pos);
			}
		}
		LOGERROR(Debugging::g_logger, "<netTransport::AddressMap> failed to find dangling key.");
		return false;
	}
	


	// address registration
	bdAddrHandleRef AddressMap::registerAddress(bool online, const Addr& addr, const bdSecurityID& secId, const bdSecurityKey &secKey)
	{
		SecurityRef security = findSecurity(secId, secKey);
		if(security.isNull())
		{
			security = new Security(online, secId, secKey);
			if(!security->m_registered)
				return NULL;
		}

		AddressRef address = new Address(addr, security);
		if(address->m_addrHandle.isNull())
			return NULL;

		m_addresses.addTail(address);
		return address->m_addrHandle;
	}

	bdAddrHandleRef AddressMap::updateAddress(bdAddrHandleRef prevHandle, bdAddrHandleRef newHandle)
	{
		AddressRef address = findAddress(prevHandle);
		if(address.isNull())
			return prevHandle;

		address->m_addrHandle = newHandle;
		return newHandle;
	}
		
	AddressRef AddressMap::findAddress(bdAddrHandleRef addrHandle)
	{
		if(addrHandle.isNull())
			return NULL;
				
		bdLinkedList<AddressRef>::Position pos = m_addresses.getHeadPosition();
		while(pos)
		{
			AddressRef address = m_addresses.forward(pos);

			if(address->m_addrHandle == addrHandle)
			{
				return address;
			}
		}
		return NULL;
	}
	
	bool AddressMap::unregisterAddress(bdAddrHandleRef addrHandle)
	{
		bdLinkedList<AddressRef>::Position pos = m_addresses.getHeadPosition();
		while(pos)
		{
			AddressRef address = m_addresses.getAt(pos);

			if(address->m_addrHandle == addrHandle)
			{
				m_addresses.removeAt(pos);
				return true;
			}
			else
			{
				m_addresses.forward(pos);
			}
		}
		LOGERROR(Debugging::g_logger, "<netTransport::AddressMap> unknown address.");
		return false;
	}

	bool AddressMap::connectAddress(bdAddrHandleRef &addrHandle)
	{
		AddressRef address = findAddress(addrHandle);
		
		if(address.isNull())
		{
			LOGERROR(Debugging::g_logger, "<netTransport::AddressMap> unknown address.");
			return false;
		}

		return netPlatform::Core::instance()->connectAddress(address->m_commonAddr, addrHandle);
	}

	bdCommonAddrRef AddressMap::addressCommon(bdAddrHandleRef addrHandle)
	{
		AddressRef address = findAddress(addrHandle);
		
		if(address.isNull())
		{
			LOGERROR(Debugging::g_logger, "<netTransport::AddressMap> unknown address.");
			return false;
		}

		return address->m_commonAddr;
	}

	bdSecurityID AddressMap::addressSecurityId(bdAddrHandleRef addrHandle)
	{
		AddressRef address = findAddress(addrHandle);
		
		if(address.isNull())
		{
			LOGERROR(Debugging::g_logger, "<netTransport::AddressMap> unknown address.");
			return bdSecurityID();
		}

		return address->m_security->m_secId;
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

