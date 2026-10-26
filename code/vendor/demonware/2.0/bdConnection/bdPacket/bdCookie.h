// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: 

#ifndef BD_COOKIE_H
#define BD_COOKIE_H

#include <bdCore/bdCore.h>

//0                   1                   2                   3
//0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                                                               |
//|                                                               |
//|                        Signed Hash                            |
//|                                                               |
//|                                                               |
//|                                                               |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                        Time stamp                             |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                        Local Tag                              |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                         Peer Tag                              |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                        Local Tie Tag                          |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|                        Peer Tie Tag                           |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 
// Signed Hash: 24 bytes
//  A Tiger-24 hash signed using the shared key.
//
// Time stamp: 4 Bytes
//  A time stamp of the time the cookie was created.

BD_REFERENCE(bdCookie);

#define BD_COOKIE_SIGNED_HASH_SIZE	(20u)

class bdCookie : public bdReferencable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdCookie();

		bdCookie(bdUInt32 localTag,
					bdUInt32 peerTag,
					bdUInt32 localTieTag = 0,
					bdUInt32 peerTieTag = 0);

		/// Destructor.
		virtual ~bdCookie();

		bdUInt32 getLocalTag() const;		

		bdUInt32 getPeerTag() const;

		bdUInt32 getLocalTieTag() const;

		bdUInt32 getPeerTieTag() const;

		bdUInt serialize(bdUByte8 *data, 
						 const bdUInt size) const;

		bdBool deserialize(const bdUByte8 *const data, 
						   const bdUInt32 size, 
						   bdUInt &offset);

		bdUInt getSerializedSize() const;

	protected:

		bdUInt32 m_localTag;
		bdUInt32 m_peerTag;
		bdUInt32 m_localTieTag;
		bdUInt32 m_peerTieTag;

		static bdUByte8 m_secret[BD_COOKIE_SIGNED_HASH_SIZE];
		static bdBool m_secretInitialized;
};

#endif // BD_COOKIE_H
