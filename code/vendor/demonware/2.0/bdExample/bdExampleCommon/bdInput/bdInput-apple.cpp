// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: OSX IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdCore/bdCore.h>
#include <bdExample/bdExampleCommon/bdInput/bdInput.h>
#include <bdGraphics/bdGfx.h>

const int bdInput::BD_CHAR_MAP_OSX[BD_NUM_DEFINED_CONTROLS] =
{
	'w',	// BD_MOVE_UP
	's',	// BD_MOVE_DOWN
	'a',	// BD_MOVE_LEFT
	'd',	// BD_MOVE_RIGHT
	'p',	// BD_BUTTON1
	'o',	// BD_BUTTON2
	'b',	// BD_BUTTON3
	'n',	// BD_BUTTON4
	'k',	// BD_BUTTON5
	'l',	// BD_BUTTON6
	'q'		// BD_QUIT
};

static 	bdBool s_pressedButtons[BD_NUM_DEFINED_CONTROLS];

bdInput::bdInput()
{
	// Initialize m_buttons, so that the buttons are at their current state and m_lastButtons to all be false.
	for (bdUInt i = 0; i < BD_NUM_DEFINED_CONTROLS; i++)
	{
		m_lastButtons[i] = isKeyDown(static_cast<BD_CONTROLS>(i));
		m_pressedButtons[i] = false;
		s_pressedButtons[i] = false;
	}
}

// Not used on OS X
void bdInput::startFrame()
{
}

bdBool BD_CALL bdInput::isKeyDown(const BD_CONTROLS button)
{
	return s_pressedButtons[button];
}

bdBool bdInput::wasKeyPressed(const BD_CONTROLS button)
{
	BD_ASSERT(button < BD_NUM_DEFINED_CONTROLS, "button must be one of the values from the BD_CONTROLS enum");
	
	return m_pressedButtons[button];
}

void bdInput::printToScreen(const bdNChar8 *const str)
{
	bdGfx::getInstance()->setText(str);
}

void bdInput::markKeyAsPressed(const BD_CONTROLS button, bdBool down)
{ 
	// A button is pressed this frame if it is currently down and wasn't down in the last frame.
	m_pressedButtons[button] = down;
	s_pressedButtons[button] = m_pressedButtons[button];
	// Store the current state of the button
	m_lastButtons[button] = down;
}
