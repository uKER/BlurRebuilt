// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE:	Container class for a single tag.

#ifndef BD_TAG_H
#define BD_TAG_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

class bdTag : public bdTaskResult
{
	public:	

		BD_DECLARE_NEW_AND_DELETE_OPERATORS
		
		bdTag();
		bdTag(bdInt priTag, bdInt secTag);
		~bdTag();
		
		virtual void serialize(bdByteBuffer &buffer) const;
		virtual bdBool deserialize(bdByteBufferRef buffer);
		virtual bdUInt sizeOf();

		void set(bdUInt32 priTag, bdUInt32 secTag);

		bdUInt32 m_priTag;
		bdUInt32 m_secTag;
};


#endif //BD_TAG_H
