// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformSocket/bdInAddr-psp.h>
#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>


bdInAddr::bdInAddr()
{
	inUn.m_iaddr = BD_IN_ADDR_NONE;
}

bdInAddr::bdInAddr(bdUInt64 addr)
{
	set(addr);
}

bdInAddr::bdInAddr(const bdNChar8 * addr)
{
	fromString(addr);
}

void bdInAddr::fromString(const bdNChar8 *addr)
{
	inUn.m_iaddr = 0;
	sceNetEtherStrton(addr, &inUn.m_etherAddr);
}

bdUWord bdInAddr::toString(bdNChar8 *const str, 
						   const bdUWord size) const
{
	if (size >= 18)
	{
		bdMemset(str, BD_NULL, size);
		sceNetEtherNtostr(&inUn.m_etherAddr, str);
		const bdUWord strLength = bdStrlen(str);
		return strLength;
	}
	else
	{
		str[0] = 0;
		return 0;
	}
}

void bdInAddr::set(bdUInt64 iaddr)
{
	inUn.m_iaddr = BD_IN_ADDR_BROADCAST & iaddr;
}

void bdInAddr::set(bdUByte8 addr[BD_IN_ADDR_SIZE])
{
	inUn.m_iaddr = 0;
	bdMemcpy(&inUn.m_iaddr, addr, BD_IN_ADDR_SIZE);
}

bdUInt64 bdInAddr::getAsUInt64() const
{
	return inUn.m_iaddr;
}

const bdUByte8* bdInAddr::getAsPointer() const
{
	return reinterpret_cast<const bdUByte8*>(&inUn.m_iaddr);
}
