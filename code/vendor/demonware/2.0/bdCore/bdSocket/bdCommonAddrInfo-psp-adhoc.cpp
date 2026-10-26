// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdCommonAddrInfo.h>

#include <bdCore/bdSocket/bdCommonAddr.h>

#define BD_LOG_LEVEL "commonaddrinfo"

//#pragma error "Not Implemented"

bdUInt BD_CALL bdCommonAddrInfo::getInfo(const bdCommonAddr& addr,
										 bdNChar8 *buf,
										 const bdUInt length)
{
	bdNChar8 *const end = buf + length;

	BD_WARN(BD_LOG_LEVEL, "Not implemented on PSP yet..");

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

	BD_WARN(BD_LOG_LEVEL, "Not implemented on PSP yet..");

	// return num bytes written (not including the null terminator)
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
