// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 PS3 PSP-INFRA UNIX OSX IPHONE WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdCommonAddrInfo.h>
#include <bdCore/bdSocket/bdCommonAddr.h>

#include <bdPlatform/bdPlatformString/bdPlatformString.h>

#define BD_LOG_LEVEL "commonaddrinfo"

bdUInt BD_CALL bdCommonAddrInfo::getInfo(const bdCommonAddr& addr,
										 bdNChar8 *buf,
										 const bdUInt length)
{
	bdNChar8 *const end = buf + length;

	// Print NAT Type.
	{
		const bdNChar8 *const open		= "BD_NAT_OPEN";
		const bdNChar8 *const moderate	= "BD_NAT_MODERATE";
		const bdNChar8 *const strict		= "BD_NAT_STRICT";
		const bdNChar8 *const unknown	= "**UNKNOWN**";

		const bdNChar8 *natType;

		switch(addr.getNATType())
		{
			case BD_NAT_OPEN:
			{
				natType  = open;
			}
			break;
			case BD_NAT_MODERATE:
			{
				natType  = moderate;
			}
			break;
			case BD_NAT_STRICT:
			{
				natType  = strict;
			}
			break;
			default:
			{
				natType  = unknown;
			}
		}

		bdInt written = bdSnprintf(buf, static_cast<bdUInt>(end - buf), "NAT Type: %s\n", natType);
		if(written > 0 && written <= static_cast<bdInt>(end - buf))
		{
			buf += written;
		}
		else
		{
			buf = end - 1;
			if(length > 0)
			{
				*buf = '\0';
			}
		}
	}

	// Print Local Addr(s)
	{
		bdInt written = bdSnprintf(buf, static_cast<bdUInt>(end - buf), "%u Local Addrs Found: \n", addr.getLocalAddrs().getSize());
		if(written > 0 && written <= static_cast<bdInt>(end - buf))
		{
			buf += written;
		}
		else
		{
			buf = end - 1;
			if(length > 0)
			{
				*buf = '\0';
			}
		}

		for(bdUInt i =0; i < addr.getLocalAddrs().getSize(); i++)
		{
			bdNChar8 addrString[BD_ADDR_STRING_SIZE];
			addr.getLocalAddrs()[i].toString(addrString, BD_ADDR_STRING_SIZE);

			written = bdSnprintf(buf, static_cast<bdUInt>(end - buf), "Local Addr %u :  %s \n", i , addrString);
			if(written > 0 && written <= static_cast<bdInt>(end - buf))
			{
				buf += written;
			}
			else
			{
				buf = end - 1;
				if(length > 0)
				{
					*buf = '\0';
				}
			}
		}

	}

	// Print Public Addr
	{
		bdNChar8 addrString[BD_ADDR_STRING_SIZE];
		addr.getPublicAddr().toString(addrString, BD_ADDR_STRING_SIZE);

		bdInt written = bdSnprintf(buf, static_cast<bdUInt>(end - buf), "Public Addr :  %s \n", addrString);
		if(written > 0 && written <= static_cast<bdInt>(end - buf))
		{
			buf += written;
		}
		else
		{
			buf = end - 1;
			if(length > 0)
			{
				*buf = '\0';
			}
		}

	}

	//Print Hash & isLoopback
	{
		const bdNChar8 *const isLoopback		= "TRUE";
		const bdNChar8 *const isNotLoopback	= "FALSE";

		const bdNChar8 *loop;

		if(addr.isLoopback())
		{
			loop = isLoopback;
		}
		else
		{
			loop = isNotLoopback;
		}

		const bdUInt32 tempHash = addr.getHash();
		bdInt written = bdSnprintf(buf, static_cast<bdUInt>(end - buf), "Addr is loopback : %s  -- Addr Hash : %u \n", loop, tempHash);
		if(written > 0 && written <= static_cast<bdInt>(end - buf))
		{
			buf += written;
		}
		else
		{
			buf = end - 1;
			if(length > 0)
			{
				*buf = '\0';
			}
		}
	}

	// return num bytes written (not including the null terminator)
	const bdUInt bytesWritten = length - (end - buf);
	return bytesWritten;
}

bdUInt BD_CALL bdCommonAddrInfo::getInfo(const bdCommonAddrRef addr,
										 bdNChar8 *buf,
										 const bdUInt length)
{
	bdUInt bytesWritten = 0;
	if(addr.notNull())
	{
		bytesWritten = getInfo(*addr, buf, length);
	}

	return bytesWritten;
}

bdUInt BD_CALL bdCommonAddrInfo::getBriefInfo(const bdCommonAddr& addr,
											  bdNChar8 *buf,
											  const bdUInt length)
{
	bdNChar8 *const end = buf + length;
	bdInt written;

	if (addr.getPublicAddr().getAddress().isValid())
	{
		bdNChar8 addrString[BD_ADDR_STRING_SIZE];
		addr.getPublicAddr().toString(addrString, BD_ADDR_STRING_SIZE);

		written = bdSnprintf(buf, length, "Public Addr: %s\n", addrString);
	}
	else if (addr.getLocalAddrs().getSize() > 0)
	{
		const bdUInt addrsStringSize = (BD_ADDR_STRING_SIZE + 2) * BD_MAX_LOCAL_ADDRS;
		bdNChar8 addrsString[addrsStringSize];
		bdNChar8 *start = addrsString;

		BD_ASSERT(addr.getLocalAddrs().getSize() <= BD_MAX_LOCAL_ADDRS,
			"bdCommonAddr invalid.");


		for (bdUInt i=0; i < addr.getLocalAddrs().getSize(); i++)
		{
			start += addr.getLocalAddrByIndex(i).toString(start,
														   addrsStringSize - (start - addrsString));
			start += bdSnprintf(start, addrsStringSize - (start - addrsString), ", ");
		}
		
		// Remove last ", "
		start -= 2;
		*start = '\0';

		written = bdSnprintf(buf, length, "Local Addr: %s\n", addrsString);
	}
	else
	{
		written = bdSnprintf(buf, length, "Empty Common Addr\n");
	}
	if(written > 0 && written <= static_cast<bdInt>(end - buf))
	{
		buf += written;
	}
	else
	{
		buf = end - 1;
		if(length > 0)
		{
			*buf = '\0';
		}
	}

	const bdUInt bytesWritten = length - (end - buf);
	return bytesWritten;
}

bdUInt BD_CALL bdCommonAddrInfo::getBriefInfo(const bdCommonAddrRef addr,
											  bdNChar8 *buf,
											  const bdUInt length)
{
	bdUInt bytesWritten = 0;
	if(addr.notNull())
	{
		bytesWritten = getBriefInfo(*addr, buf, length);
	}

	return bytesWritten;
}

bdCommonAddrInfo::bdCommonAddrInfo()
{
}
