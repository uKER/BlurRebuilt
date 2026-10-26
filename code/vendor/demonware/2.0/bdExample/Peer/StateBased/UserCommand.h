// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef USER_COMMAND_H
#define USER_COMMAND_H

#include <bdExample/bdExampleCommonPeer/bdExampleCommonP2P.h>

class UserCommand
{
	public:

		UserCommand();

		/// Array to indicate which keys have been pressed
		bdBool m_keys[4];
};

#endif // USER_COMMAND_H
