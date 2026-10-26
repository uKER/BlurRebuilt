// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef PLAYER_INPUT_H
#define PLAYER_INPUT_H

#include <bdCore/bdCore.h>

class PlayerInput
{
	public:

		PlayerInput();
		
		/// Array to indicate which keys have been pressed
		bdBool m_keys[4];
};

#endif // PLAYER_INPUT_H
