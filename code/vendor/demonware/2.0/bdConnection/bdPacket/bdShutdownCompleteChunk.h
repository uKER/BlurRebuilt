// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class to represent a shutdown complete chunk.

#ifndef BD_SHUTDOWN_COMPLETE_CHUNK_H
#define BD_SHUTDOWN_COMPLETE_CHUNK_H

#include <bdCore/bdCore.h>
#include <bdConnection/bdPacket/bdChunk.h>

//0                   1                   2                   3
//0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//|BD_CT_SHUTDOWN_| Cookie Ack Flags  |        Length             |
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

BD_REFERENCE(bdShutdownCompleteChunk);

class bdShutdownCompleteChunk : public bdChunk
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		enum bdShutdownCompleteFlags
		{
			BD_SCC_NO_FLAGS		= 0x00000000
		};

		bdShutdownCompleteChunk();

		/// Destructor.
		virtual ~bdShutdownCompleteChunk();

		bdShutdownCompleteFlags getFlags() const;

	protected:

		friend class bdPacket;

		virtual bdUInt serialize(bdUByte8 *data, 
								 const bdUInt size) const;

		virtual bdBool deserialize(const bdUByte8 *const data, 
								   const bdUInt size, 
								   bdUInt &offset);

	protected:

		bdShutdownCompleteFlags m_flags;
};

#endif // BD_SHUTDOWN_COMPLETE_CHUNK_H
