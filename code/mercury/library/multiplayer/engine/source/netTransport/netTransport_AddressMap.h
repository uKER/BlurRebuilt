#ifndef NETTRANSPORT_ADDRESSMAP_H
#define NETTRANSPORT_ADDRESSMAP_H

#ifdef _MSC_VER
#pragma once
#endif

#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>
#include <bdSocket/bdAddrHandle.h>
#include <bdCore/bdSocket/bdCommonAddr.h>
#include <bdCore/bdSocket/bdSecurityKey.h>
#include <bdCore/bdSocket/bdSecurityID.h>

#include "./netTransport_Addr.h"

namespace netTransport
{
	BD_REFERENCE(Security);
	class Security: public bdReferencable
	{
	public:
		Security(bool online, const bdSecurityID& secId, const bdSecurityKey &secKey);
		~Security();

		bool m_online; 
		bool m_registered;
		bdSecurityID m_secId;
		bdSecurityKey m_secKey;
	};

	BD_REFERENCE(Address);
	class Address: public bdReferencable
	{
	public:
		Address(SecurityRef security);
		Address(const Addr& addr, SecurityRef security);
		~Address();

		bdAddrHandleRef		m_addrHandle;
		bdCommonAddrRef		m_commonAddr;
		Addr				m_addr;
		SecurityRef			m_security;
	};

	BD_REFERENCE(AddressMap);
	class AddressMap: public bdReferencable
	{
	public:
		AddressMap();
		~AddressMap();

		// local address
		netTransport::Addr myAddr();

		// key registration, not attached to an address
		bool   registerKey(bool online, const bdSecurityID& secId, const bdSecurityKey &secKey);
		bool unregisterKey(const bdSecurityID& secId, const bdSecurityKey &secKey);

		// address/key combo registration
		bdAddrHandleRef   registerAddress	(bool online, const Addr& addr, const bdSecurityID& secId, const bdSecurityKey &secKey);
		bdAddrHandleRef   updateAddress		(bdAddrHandleRef prevHandle, bdAddrHandleRef newHandle);
		bool			  registeredAddress (bdAddrHandleRef addr) { AddressRef record = findAddress(addr); return record.notNull(); }
		bool			  unregisterAddress	(bdAddrHandleRef addr);
		
		// address tools
		bool			connectAddress		(bdAddrHandleRef &addrHandle);
		bdCommonAddrRef addressCommon		(bdAddrHandleRef addrHandle);
		bdSecurityID	addressSecurityId	(bdAddrHandleRef addrHandle);

	private:
		SecurityRef findSecurity(const bdSecurityID& secId, const bdSecurityKey &secKey);
		AddressRef findAddress(bdAddrHandleRef addrHandle);
		bdLinkedList<AddressRef> m_addresses;
	};
}

#endif //NETTRANSPORT_ADDRESSMAP_H