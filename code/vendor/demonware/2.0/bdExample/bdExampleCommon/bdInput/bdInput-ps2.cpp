// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS2
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdCore/bdCore.h>
#include <bdExample/bdExampleCommon/bdInput/bdInput.h>

#include <external/ps2shell/include/shellCore.h>

#include <bdGraphics/bdGfx.h>

const u_int BD_CHAR_MAP_PS2[BD_NUM_DEFINED_CONTROLS] =
{
	SCE_PADLup,		// BD_MOVE_UP
	SCE_PADLdown,	// BD_MOVE_DOWN
	SCE_PADLleft,	// BD_MOVE_LEFT
	SCE_PADLright,	// BD_MOVE_RIGHT
	SCE_PADRup,		// BD_BUTTON1
	SCE_PADRright,	// BD_BUTTON2
	SCE_PADRdown,	// BD_BUTTON3
	SCE_PADRleft,	// BD_BUTTON4
	SCE_PADL1,		// BD_BUTTON5
	SCE_PADR1,		// BD_BUTTON6
	SCE_PADselect	// BD_QUIT
};

ShellPadData padData;

bdInput::bdInput()
{
	shellFileInit(	SHELLFILE_MEDIUM_HOST0,	// Where to serve files from: SHELLFILE_MEDIUM_HOST0 = host0.
		0,								// Flag: Do not reboot IOP.
		"../../../../iop/modules",	// String of directories to search for IOP modules, separated by semicolons.
		NULL);							// Parameter specific to choice of medium.

	shellPadInit(0);

	shellPadInitPort(&padData,
		0,
		0,
		32);

	// Initialize m_buttons, so that the buttons are at their current state and m_lastButtons to all be false.
	for (bdUInt i = 0; i < BD_NUM_DEFINED_CONTROLS; i++)
	{
		m_lastButtons[i] = isKeyDown(static_cast<BD_CONTROLS>(i));
		m_pressedButtons[i] = false;
	}

	memset(&padData, 0, sizeof(ShellPadData));
}

void bdInput::startFrame()
{
	if (shellPadRead(&padData) == SHELLPAD_DISCONNECT)
	{
		memset(m_lastButtons, 0, sizeof(m_lastButtons));
		memset(m_pressedButtons, 0, sizeof(m_pressedButtons));
	}
	else
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
}

bdBool BD_CALL bdInput::isKeyDown(const BD_CONTROLS button)
{
	if (padData.button & BD_CHAR_MAP_PS2[button])
	{
		return true;
	}
	else
	{
		return false;
	}
}

bdBool bdInput::wasKeyPressed(const BD_CONTROLS button)
{
	return m_pressedButtons[button];
}

void bdInput::printToScreen(const bdNChar8 *const str)
{
	bdGfx::getInstance()->setText(str);
}
