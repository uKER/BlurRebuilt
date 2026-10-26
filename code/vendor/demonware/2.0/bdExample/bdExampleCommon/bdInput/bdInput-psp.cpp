// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC PSP-INFRA
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdCore.h>
#include <bdExample/bdExampleCommon/bdInput/bdInput.h>

#include <libgu.h>
#include <ctrlsvc.h>

#include <bdGraphics/bdGfx.h>

// The map of the BD_CONTROL values to the actual keyboard key values.
const int BD_CHAR_MAP_PSP[BD_NUM_DEFINED_CONTROLS] =
{
	SCE_CTRL_UP,		// BD_MOVE_UP
	SCE_CTRL_DOWN,		// BD_MOVE_DOWN
	SCE_CTRL_LEFT,		// BD_MOVE_LEFT
	SCE_CTRL_RIGHT,		// BD_MOVE_RIGHT
	SCE_CTRL_TRIANGLE,	// BD_BUTTON1
	SCE_CTRL_CIRCLE,	// BD_BUTTON2
	SCE_CTRL_CROSS,		// BD_BUTTON3
	SCE_CTRL_SQUARE,	// BD_BUTTON4
	SCE_CTRL_L,			// BD_BUTTON5
	SCE_CTRL_R,			// BD_BUTTON6
	SCE_CTRL_SELECT		// BD_QUIT
};

// The data structure recording the state of the controls.
static SceCtrlData ctrlData;


bdInput::bdInput()
{
	// Initialize m_buttons, so that the buttons are at their current state and m_lastButtons to all be false.
	for (bdUInt i = 0; i < BD_NUM_DEFINED_CONTROLS; i++)
	{
		m_lastButtons[i] = isKeyDown(static_cast<BD_CONTROLS>(i));
		m_pressedButtons[i] = false;
	}

	memset(&ctrlData, 0, sizeof(SceCtrlData));
}

void bdInput::startFrame()
{
	sceCtrlReadBufferPositive(&ctrlData, 1);

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
	BD_ASSERT(c < BD_NUM_DEFINED_CONTROLS, "button must be one of the values from the BD_CONTROLS enum");

	// ctrlData.Buttons is a bitfield, where each button is mapped to a bit. A 1 means the button is pressed, 0 means
	// it is not pressed. A bitwise AND operation is used to find out if the bit is set.
	if (ctrlData.Buttons & BD_CHAR_MAP_PSP[c])
	{
		return true;
	}
	else
	{
		return false;
	}
}

bdBool bdInput::wasKeyPressed(const BD_CONTROLS c)
{
	return m_pressedButtons[c];
}

void bdInput::printToScreen(const bdNChar8 *const str)
{
	bdGfx::getInstance()->setText(str);
}

