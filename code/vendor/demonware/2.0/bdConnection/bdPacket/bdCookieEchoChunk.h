// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class to represent a cookie echo chunk.

#ifndef BD_COOKIE_ECHO_CHUNK_H
#define BD_COOKIE_ECHO_CHUNK_H

#include <bdCore/bdCore.h>
#include <bdConnection/bdPacket/bdChunk.h>

//0                   1                   2                   3
//0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|BD_CT_COOKIE_EC| Cookie Echo Flags |        Length             |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                                                               |
//|                       Cookie Data                             |
//|                                                               |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

BD_REFERENCE(bdInitChunk);
BD_REFERENCE(bdCookie);
BD_REFERENCE(bdCookieEchoChunk);

/// A class to represent a cookie echo chunk.
class bdCookieEchoChunk : public bdChunk
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		enum bdCookieEchoFlags
		{
			BD_CE_NO_FLAGS		= 0x00000000
		};

		bdCookieEchoChunk();

		bdCookieEchoChunk(bdByteBufferRef cookieBuffer);

		/// Destructor.
		virtual ~bdCookieEchoChunk();

		bdBool getCookie(bdCookieRef &cookie);

		bdCookieEchoFlags getFlags() const;

	protected:

		/// Grant friendship to bdPacket so that it can
		/// construct an instance of this class.
		friend class bdPacket;

		virtual bdUInt serialize(bdUByte8 *data, 
								 const bdUInt32 size) const;

		virtual bdBool deserialize(const bdUByte8 *const data, 
								   const bdUInt32 size, 
								   bdUInt &usedSize);

	protected:

		bdCookieEchoFlags m_flags;

		bdCookieRef m_cookie;

		bdByteBufferRef m_cookieBuffer;
};

#endif // BD_COOKIE_ECHO_CHUNK_H
