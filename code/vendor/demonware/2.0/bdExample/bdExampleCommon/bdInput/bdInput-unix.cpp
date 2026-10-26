// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: UNIX
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdCore/bdCore.h>
#include <bdExample/bdExampleCommon/bdInput/bdInput.h>


bdInput::bdInput()
{
}

void bdInput::startFrame()
{
}

bdBool BD_CALL bdInput::isKeyDown(const BD_CONTROLS button)
{
	return false;
}

bdBool bdInput::wasKeyPressed(const BD_CONTROLS button)
{
	return false;
}

void bdInput::printToScreen(const bdNChar8 *const str)
{
}
