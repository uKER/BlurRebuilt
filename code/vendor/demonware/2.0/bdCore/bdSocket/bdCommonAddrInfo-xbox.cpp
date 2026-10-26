// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX XENON
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

	// todo : expand this implementation
	// Print public part of XNADDR 
	{
		bdNChar8 str[BD_ADDR_STRING_SIZE];
		bdUWord strLength = 0;
		const bdInt sizei = BD_ADDR_STRING_SIZE;
		if(XNetInAddrToString(addr.getXNAddr().inaOnline, str, sizei) == 0)
		{
			strLength = bdStrlen(str);

			bdInt written = bdSnprintf(buf, end - buf, "XNADDR :  %s \n", str);
			if(written > 0)
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

	// Print port & titleID 
	{
		bdUInt32 tempPort = addr.getPort();
		bdUInt32 tempTitleId = static_cast<bdUInt32>(addr.getTitleId());

		bdInt written = bdSnprintf(buf, end - buf, "Port:  %u  -- TitleId %u \n", tempPort, tempTitleId);
		if(written > 0)
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
	return getInfo(addr, buf, length);
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
