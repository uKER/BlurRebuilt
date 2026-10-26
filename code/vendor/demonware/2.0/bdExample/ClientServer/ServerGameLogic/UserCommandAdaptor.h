// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef USER_COMMAND_ADAPTOR_H
#define USER_COMMAND_ADAPTOR_H

#include <bdExample/bdExampleCommonClientServer/bdExampleCommonCS.h>

#include "PlayerInput.h"

class UserCommandAdaptor : public bdUserCommand
{
	public:
		
		void setUserInput(const PlayerInput& input);

		const PlayerInput& getUserInput() const;

		void clear();

		virtual void serialize(bdBitBufferRef buffer, 
							   bdUserCommand *previousCommand);

		virtual bdBool deserialize(bdBitBufferRef buffer);

	protected:

		PlayerInput m_input;
};

#endif // USER_COMMAND_ADAPTOR_H
