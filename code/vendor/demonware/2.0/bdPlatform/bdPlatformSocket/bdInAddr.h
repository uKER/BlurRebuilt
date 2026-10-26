// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_INADDR_H
#define BD_INADDR_H

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>

#if defined BD_PLATFORM_PSP_ADHOC
#include <bdPlatform/bdPlatformSocket/bdInAddr-psp.h>
#else


#define BD_IN_ADDR_ANY              static_cast<bdUInt32>(0x00000000)
#define BD_IN_ADDR_BROADCAST        static_cast<bdUInt32>(0xffffffff)
#if defined BD_LITTLE_ENDIAN
# define BD_IN_ADDR_LOOPBACK         0x0100007f
# define BD_IN_ADDR_NONE             0xff00ff00
#else // big endian
# define BD_IN_ADDR_LOOPBACK         0x7f000001
# define BD_IN_ADDR_NONE             0x00ff00ff
#endif // BD_LITTLE_ENDIAN

#define BD_IN_ADDR_SIZE				(4u)

class bdInAddr
{
	public:

		bdInAddr();

		bdInAddr(bdUInt32 addr);

		bdInAddr(const bdNChar8 * addr);

		void fromString(const bdNChar8 *addr);

		bdUWord toString(bdNChar8 *const str, 
						 const bdUWord size) const;

        union 
		{
                struct 
				{ 
					bdUByte8 m_b1,m_b2,m_b3,m_b4; 
				} 
				m_caddr;

                bdUInt32 m_iaddr;
        } 
		inUn;
#define m_inAddri	inUn.m_iaddr
};

#endif //BD_PLATFORM_PSP

#endif //BD_INADDR_H
