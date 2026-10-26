// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE		: A class to read input from the keyboard using Win32

#include <bdExample/bdExampleCommon/bdInput/bdInput.h>

#ifdef BD_PLATFORM_WIN32

#include <bdGraphics/bdGfx.h>

// The map of the BD_CONTROL values to the actual keyboard key values.
static const int BD_CHAR_MAP_WIN32[BD_NUM_DEFINED_CONTROLS] =
{
	'W',	// BD_MOVE_UP
	'S',	// BD_MOVE_DOWN
	'A',	// BD_MOVE_LEFT
	'D',	// BD_MOVE_RIGHT
	'P',	// BD_BUTTON1
	'O',	// BD_BUTTON2
	'B',	// BD_BUTTON3
	'N',	// BD_BUTTON4
	'K',	// BD_BUTTON5
	'L',	// BD_BUTTON6
	'Q'		// BD_QUIT
};

bdInput::bdInput()
{
	// Initialize m_buttons, so that the buttons are at their current state and m_lastButtons to all be false.
	for (bdUInt i = 0; i < BD_NUM_DEFINED_CONTROLS; i++)
	{
		m_lastButtons[i] = isKeyDown(static_cast<BD_CONTROLS>(i));
		m_pressedButtons[i] = false;
	}
}

void bdInput::startFrame()
{
	for (bdUInt i = 0; i < BD_NUM_DEFINED_CONTROLS; i++)
	{
		// Get the current state of the key
		bdBool keyDown = isKeyDown(static_cast<BD_CONTROLS>(i));

		// A button is pressed this frame if it is currently down and wasn't down in the last frame.
		m_pressedButtons[i] = !m_lastButtons[i] && keyDown;

		// Store the current state of the button
		m_lastButtons[i] = keyDown;
	}
}

bdBool BD_CALL bdInput::isKeyDown(const BD_CONTROLS c)
{
	BD_ASSERT(c < BD_NUM_DEFINED_CONTROLS, "c must be one of the values from the BD_CONTROLS enum");

	// Win32 function GetAsyncKeyState() is used to determine button presses. Unfortunately, this works whether or not
	// the current application has the focus or not, so only return true if the application's window has the focus.
	if (GetFocus() == bdGfx::getInstance()->getWindowHandle())
	{
		SHORT keystate = GetAsyncKeyState(BD_CHAR_MAP_WIN32[c]);

		// Shift the most significant bit right, so it becomes the least significant bit
		keystate >>= (sizeof(SHORT) * 8) - 1;

		// Set all bits, except the least significant one to zero
		keystate &= 1;

		// return true if the least significant bit is 1 and false if it is 0
		return (keystate == 1) ? true : false;
	}
	else
	{
		return false;
	}

}

bdBool bdInput::wasKeyPressed(const BD_CONTROLS c)
{
	BD_ASSERT(c < BD_NUM_DEFINED_CONTROLS, "c must be one of the values from the BD_CONTROLS enum");

	return m_pressedButtons[c];
}

void bdInput::printToScreen(const bdNChar8 *const str)
{
	bdGfx::getInstance()->setText(str);
}

#endif // BD_PLATFORM_WIN32
