// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE:	Container class for key/value pairs.

#ifndef BD_KEY_VALUE_PAIR_H
#define BD_KEY_VALUE_PAIR_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

class bdKeyValuePair : public bdTaskResult
{
	public:	

		BD_DECLARE_NEW_AND_DELETE_OPERATORS
		
		enum bdArchiveUpdateType
		{
		   BD_REPLACE = 0,
		   BD_ADD = 1,
		   BD_MAX = 2,
		   BD_MIN = 3,
		   BD_AND = 4,
		   BD_OR = 5,
		   BD_XOR = 6
		};

		bdKeyValuePair();

		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();

		bdUByte8 m_updateType;
		bdUInt16 m_index;
		bdInt64 m_value;



};


#endif //BD_KEY_VALUE_PAIR_H
