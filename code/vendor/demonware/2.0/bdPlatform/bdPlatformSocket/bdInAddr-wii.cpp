// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformSocket/bdInAddr.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>

#include <revolution/soex.h>

bdInAddr::bdInAddr()
{
	inUn.m_iaddr = BD_IN_ADDR_NONE;
}

bdInAddr::bdInAddr(bdUInt32 addr)
{
	inUn.m_iaddr = addr;
}

bdInAddr::bdInAddr(const bdNChar8 * addr)
{
	fromString(addr);
}

void bdInAddr::fromString(const bdNChar8 *addr)
{
	SOInAddr soAddr;

	bdBool ok = false;
	bdInt result = SOInetAtoN(addr,&soAddr);	
	
	if(result > 0 )
	{
		m_inAddri = soAddr.addr;
	}
	else
	{
			//FIXME bdLog
	}
}

bdUWord bdInAddr::toString(bdNChar8 *const str, 
						   const bdUWord size) const
{
	SOInAddr soAddr;
	soAddr.addr = m_inAddri;

	bdBool ok = false;
	const bdNChar8 *const result = SOInetNtoA(soAddr);	

	ok = (size > bdStrlcpy(str, result, size));

	bdUWord strLength = 0;
	if(ok)
	{
		strLength = bdStrlen(str);
	}

	return strLength;
}

