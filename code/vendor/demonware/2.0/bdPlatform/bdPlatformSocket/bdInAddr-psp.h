// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_INADDR_PSP_H
#define BD_INADDR_PSP_H


#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>

#ifdef BD_PLATFORM_PSP_ADHOC

#include <pspnet.h>

#define BD_IN_ADDR_ANY              static_cast<bdUInt64>(0x00000000)
// Loopback addr is, in fact, invalid. It should not be used.
#define BD_IN_ADDR_LOOPBACK         0x7f000001
#define BD_IN_ADDR_BROADCAST        (0x0000ffffffffffffULL)
#define BD_IN_ADDR_NONE             (0x000000ff00ff00ffULL)

#define BD_IN_ADDR_SIZE				(6u)

class bdInAddr
{
	public:

		bdInAddr();

		bdInAddr(bdUInt64 addr);

		bdInAddr(const bdNChar8 * addr);

		void fromString(const bdNChar8 *addr);

		/// \param str[in] Buffer to receive converted address. Must be at least
		///  18 bytes long.
		/// \param size[in] Size of the buffer pointed to by \a str.
		bdUWord toString(bdNChar8 *const str, 
						 const bdUWord size) const;

		void set(bdUInt64 iaddr);
		void set(bdUByte8 addr[BD_IN_ADDR_SIZE]);

		bdUInt64 getAsUInt64() const;

		const bdUByte8* getAsPointer() const;


protected:
        union 
		{
			SceNetEtherAddr m_etherAddr;

            struct 
			{ 
				bdUByte8 m_b1,m_b2,m_b3,m_b4,m_b5,m_b6; 
			} 
			m_caddr;

			// Top two (MSB) bytes of this variable must always be zero.
            bdUInt64 m_iaddr;
        } 
		inUn;
#define m_inAddri	inUn.m_iaddr
};

#endif // #ifdef BD_PLATFORM_PSP_ADHOC

#endif //BD_INADDR_PSP_H
