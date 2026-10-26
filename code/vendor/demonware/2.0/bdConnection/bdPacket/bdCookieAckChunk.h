// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class to represent a cookie ack chunk.

#ifndef BD_COOKIE_ACK_CHUNK_H
#define BD_COOKIE_ACK_CHUNK_H

#include <bdCore/bdCore.h>
#include <bdConnection/bdPacket/bdChunk.h>

//0                   1                   2                   3
//0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|BD_CT_COOKIE_AC| Cookie Ack Flags  |        Length             |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

BD_REFERENCE(bdCookieAckChunk);

/// A class to represent a cookie ack chunk.
class bdCookieAckChunk : public bdChunk
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		enum bdCookieAckFlags
		{
			BD_CA_NO_FLAGS		= 0x00000000
		};

		/// Constructor
		bdCookieAckChunk();

		/// Destructor.
		virtual ~bdCookieAckChunk();

		bdCookieAckFlags getFlags() const;

	protected:

		/// Grant friendship to bdPacket so that it can
		/// construct an instance of this class.
		friend class bdPacket;

		virtual bdUInt serialize(bdUByte8 *data, 
								 const bdUInt size) const;

		virtual bdBool deserialize(const bdUByte8 *const data, 
								   const bdUInt size, 
								   bdUInt &offset);

	protected:

		bdCookieAckFlags m_flags;
};

#endif // BD_COOKIE_ACK_CHUNK_H
