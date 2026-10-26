// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class to represent a init chunk ack.

#ifndef BD_INIT_CHUNK_ACK_H
#define BD_INIT_CHUNK_ACK_H

#include <bdCore/bdCore.h>
#include <bdConnection/bdPacket/bdChunk.h>

//0                   1                   2                   3
//0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|   BD_CT_INIT  |  INIT Flags   |           Length              |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|							Initiate Tag						  |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|           Advertised Receiver Window Credit (a_rwnd)          |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//\                                                               /
///                         Cookie                                \.
//\                                                               /
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
//Initiate Tag: 32 bits (unsigned integer)
//
//   The receiver of the INIT (the responding end) records the value of
//   the Initiate Tag parameter.  This value MUST be placed into the
//   Verification Tag field of every SCTP packet that the receiver of
//   the INIT transmits within this association.
//
//   The Initiate Tag is allowed to have any value except 0.
//Advertised Receiver Window Credit (a_rwnd): 32 bits (unsigned
//      integer)
//
//      This value represents the dedicated buffer space, in number of
//      bytes, the sender of the INIT has reserved in association with
//      this window.  During the life of the association this buffer space
//      SHOULD not be lessened (i.e. dedicated buffers taken away from
//      this association); however, an endpoint MAY change the value of
//      a_rwnd it sends in SACK chunks.
//
// Cookie: Size specified by Length (in bytes)


BD_REFERENCE(bdInitChunk);
BD_REFERENCE(bdCookie);
BD_REFERENCE(bdInitAckChunk);

class bdInitAckChunk : public bdChunk
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		enum bdInitAckChunkFlags
		{
			BD_IC_NO_FLAGS		= 0x00000000
		};

		bdInitAckChunk();

		bdInitAckChunk(bdUInt32 initTag, 
					   bdCookieRef cookie,
					   const bdWord windowCredit,
					   bdUInt32 peerTag);

		/// Destructor.
		virtual ~bdInitAckChunk();

		bdUInt32 getInitTag() const;

		bdBool getCookie(bdByteBufferRef &cookie);

		bdWord getWindowCredit() const;

		virtual bdUInt serialize(bdUByte8 *data, 
								 const bdUInt size) const;

		virtual bdBool deserialize(const bdUByte8 *const data,
								   const bdUInt size, 
								   bdUInt &offset);

		bdUInt32 getPeerTag() const;

	protected:

		bdUInt32 m_initTag;

		bdInitAckChunkFlags m_flags;

		bdByteBufferRef m_cookieBuffer;
		
		bdCookieRef m_cookie;

		bdWord m_windowCredit;

		bdUInt32 m_peerTag;
};


#endif // BD_INIT_CHUNK_ACK_H
