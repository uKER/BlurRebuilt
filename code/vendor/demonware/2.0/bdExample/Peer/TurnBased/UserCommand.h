// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef USER_COMMAND_H
#define USER_COMMAND_H

#define BD_PEER_EXAMPLES
#include <bdExample/bdExampleCommonPeer/bdExampleCommonP2P.h>

BD_REFERENCE(UserCommand);

class UserCommand : public bdCommand
{
	public:

		UserCommand();

		virtual void serialize(bdBitBufferRef buffer);

		virtual bdBool deserialize(bdBitBufferRef buffer);

		/// Array to indicate which keys have been pressed
		bdBool m_keys[4];
};

#endif // USER_COMMAND_H
