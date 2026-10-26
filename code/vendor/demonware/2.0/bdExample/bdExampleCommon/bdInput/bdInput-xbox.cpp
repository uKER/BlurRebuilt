// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE		: A class to read input from the xbox controller

#include <bdExample/bdExampleCommon/bdInput/bdInput.h>

#if defined BD_PLATFORM_XBOX && !defined BD_PLATFORM_XENON

#include <xtl.h>
#include <XbDm.h>

#include <bdGraphics/bdGfx.h>

const short INPUT_DEADZONE = (short)( 0.24f * float(0x7FFF));
//const BYTE BD_XBOX_ANALOG_THRESHOLD = 64;

struct bdGamepad : public XINPUT_GAMEPAD
{
	// The following members are inherited from XINPUT_GAMEPAD:
    // WORD    wButtons;
    // BYTE    bAnalogButtons[8];
    // SHORT   sThumbLX;
    // SHORT   sThumbLY;
    // SHORT   sThumbRX;
    // SHORT   sThumbRY;

	// Thumb stick values converted to range [-1,+1]
	bdFloat32			x1;
	bdFloat32			y1;
	bdFloat32			x2;
	bdFloat32			y2;

	// Records the state (when last updated) of the buttons.
	// These remain set as long as the button is pressed.
	bdUInt16			lastButtons;
	bdBool				lastAnalogButtons[8];

	// Records which buttons were pressed this frame - only set on
	// the frame that the button is first pressed.
	bdUInt16			pressedButtons;
	bdBool				pressedAnalogButtons[8];

    // Device properties
	XINPUT_CAPABILITIES caps;
	bdBool				connected;

    // Flags for whether game pad was just inserted or removed
	bdBool				inserted;
	bdBool				removed;

	HANDLE				handle;

	// Each bit represents a button on the gamepad. 1 means pressed, 0 means not pressed.
};

class bdXboxInput
{
public:
	static bdGamepad	gamepads[4];
	static bdGamepad	defaultGamepad;

	static HANDLE	BD_CALL initGamepad(int portNum);
	static void	BD_CALL	GetInput(bdGamepad *userGamepads = NULL);
	static bdGamepad* BD_CALL GetMergedInput();
};

bdGamepad bdXboxInput::gamepads[4];
bdGamepad bdXboxInput::defaultGamepad;

static inline float BD_CALL ConvertThumbstickValue(short thumbstickValue)
{
	if (thumbstickValue > +INPUT_DEADZONE)
	{
		return (thumbstickValue - INPUT_DEADZONE) / (32767.0f - INPUT_DEADZONE);
	}
	if (thumbstickValue < -INPUT_DEADZONE)
	{
		return (thumbstickValue + INPUT_DEADZONE + 1.0f) / (32767.0f - INPUT_DEADZONE);
	}
	return 0.0f;
}

void bdXboxInput::GetInput(bdGamepad *pGamepads)
{
	static bool initialised = false;

	if (!pGamepads)
	{
		pGamepads = gamepads;
	}

	if (!initialised)
	{
		DWORD controllerState;

		ZeroMemory(gamepads,		sizeof(gamepads));
		ZeroMemory(&defaultGamepad,	sizeof(defaultGamepad));

		controllerState = XGetDevices(XDEVICE_TYPE_GAMEPAD);

		for (int i = 0; i < 4; i++)
		{
			// Check the controller state to see if this gamepad is connected or not
			pGamepads[i].connected = ((controllerState & (1 << i)) ? true : false);

			// If it is connected => it has been inserted (nb: this is the first call to GetInput)
			pGamepads[i].inserted = pGamepads[i].connected;
			pGamepads[i].removed = false;

			if (pGamepads[i].connected)
			{
				// If the device is connected, get its handle
				pGamepads[i].handle = XInputOpen(XDEVICE_TYPE_GAMEPAD, i, XDEVICE_NO_SLOT, NULL);
				XInputGetCapabilities(pGamepads[i].handle, &pGamepads[i].caps);
			}
		}

		initialised = true;
	}
	else
	{
		DWORD insertions;
		DWORD removals;

		 if (XGetDeviceChanges(XDEVICE_TYPE_GAMEPAD, &insertions, &removals))
		 {
			 // There has been an insertion or removal of gamepad(s).
			 for (int i = 0; i < 4; i++)
			 {
				 pGamepads[i].inserted = ((insertions & (1 << i)) ? true : false);
				 pGamepads[i].removed = ((removals & (1 << i)) ? true : false);

				 if (pGamepads[i].inserted)
				 {
					 // If the gamepad has been inserted, get the handle for it
					 pGamepads[i].connected = true;
					 pGamepads[i].handle = XInputOpen(XDEVICE_TYPE_GAMEPAD, i, XDEVICE_NO_SLOT, NULL);
					 XInputGetCapabilities(pGamepads[i].handle, &pGamepads[i].caps);
				 }
				 else if (pGamepads[i].removed)
				 {
					 // If the gamepad has been removed, close the handle for it
					 pGamepads[i].connected = false;
					 XInputClose(pGamepads[i].handle);
					 pGamepads[i].handle = NULL;
					 ZeroMemory(&pGamepads[i].caps, sizeof(pGamepads[i].caps));
				 }
			 }
		 }
		 else
		 {
			 // There are no changes to the controller states, so set inserted and removed to false for all gamepads
			 for (int i = 0; i < 4; i++)
			 {
				 pGamepads[i].inserted = false;
				 pGamepads[i].removed = false;
			 }
		 }
	}

	for (int i = 0; i < 4; i++)
	{
		XINPUT_STATE input_state;

/*		bool wasConnected = pGamepads[i].connected;
		pGamepads[i].connected = ( XInputGetState(hController[i], &input_state) == ERROR_SUCCESS) ? true : false;

		pGamepads[i].removed = (wasConnected && !pGamepads[i].connected) ? true : false;
		pGamepads[i].inserted = (!wasConnected && pGamepads[i].connected) ? true : false;
*/
		if (!pGamepads[i].connected)
			continue;

		XInputGetState(pGamepads[i].handle, &input_state);

		memcpy(&pGamepads[i], &input_state.Gamepad, sizeof(XINPUT_GAMEPAD));

		pGamepads[i].x1 = ConvertThumbstickValue(pGamepads[i].sThumbLX);
		pGamepads[i].y1 = ConvertThumbstickValue(pGamepads[i].sThumbLY);
		pGamepads[i].x2 = ConvertThumbstickValue(pGamepads[i].sThumbRX);
		pGamepads[i].y2 = ConvertThumbstickValue(pGamepads[i].sThumbRY);

		pGamepads[i].pressedButtons = (pGamepads[i].lastButtons ^ pGamepads[i].wButtons) & pGamepads[i].wButtons;
		pGamepads[i].lastButtons	= pGamepads[i].wButtons;

		for (unsigned int a = 0; a < 8; a++)
		{
			bool pressed = (pGamepads[i].bAnalogButtons[a] > XINPUT_GAMEPAD_MAX_CROSSTALK);

			if (pressed)
			{
				pGamepads[i].pressedAnalogButtons[a] = !pGamepads[i].lastAnalogButtons[a];
			}
			else
			{
				pGamepads[i].pressedAnalogButtons[a] = false;
			}

			pGamepads[i].lastAnalogButtons[a] = pressed;
		}
	}
}

bdGamepad *bdXboxInput::GetMergedInput()
{
	GetInput();

	ZeroMemory(&defaultGamepad, sizeof(defaultGamepad));
	int nThumbLX = 0;
	int nThumbLY = 0;
	int nThumbRX = 0;
	int nThumbRY = 0;

	for (unsigned int i = 0; i < 4; i++)
	{
        if(gamepads[i].connected)
        {
            // Only account for thumbstick info beyond the deadzone
            if (gamepads[i].sThumbLX > INPUT_DEADZONE ||
				gamepads[i].sThumbLX < -INPUT_DEADZONE)
				nThumbLX += gamepads[i].sThumbLX;
            if (gamepads[i].sThumbLY > INPUT_DEADZONE ||
				gamepads[i].sThumbLY < -INPUT_DEADZONE)
				nThumbLY += gamepads[i].sThumbLY;
            if (gamepads[i].sThumbRX > INPUT_DEADZONE ||
				gamepads[i].sThumbRX < -INPUT_DEADZONE)
				nThumbRX += gamepads[i].sThumbRX;
            if (gamepads[i].sThumbRY > INPUT_DEADZONE ||
				gamepads[i].sThumbRY < -INPUT_DEADZONE)
				nThumbRY += gamepads[i].sThumbRY;

            defaultGamepad.x1 += gamepads[i].x1;
            defaultGamepad.y1 += gamepads[i].y1;
            defaultGamepad.x2 += gamepads[i].x2;
            defaultGamepad.y2 += gamepads[i].y2;

            defaultGamepad.wButtons			|= gamepads[i].wButtons;
            defaultGamepad.pressedButtons	|= gamepads[i].pressedButtons;
            defaultGamepad.lastButtons		|= gamepads[i].lastButtons;

			for (unsigned int a = 0; a < 8; a++)
			{
				defaultGamepad.bAnalogButtons[a]		|= gamepads[i].bAnalogButtons[a];
				defaultGamepad.pressedAnalogButtons[a]	|= gamepads[i].pressedAnalogButtons[a];
				defaultGamepad.lastAnalogButtons[a]		|= gamepads[i].lastAnalogButtons[a];
			}
        }
	}

	defaultGamepad.sThumbLX = (short)nThumbLX;
	defaultGamepad.sThumbLY = (short)nThumbLY;
	defaultGamepad.sThumbRX = (short)nThumbRX;
	defaultGamepad.sThumbRY = (short)nThumbRY;

	if ((defaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER] > 64) &&
		(defaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER] > 64) &&
		(defaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_BLACK] > 64))
	{
		DmReboot(DMBOOT_WARM);
	}

	return &defaultGamepad;
}

// The map of the BD_CONTROL values to the game pad buttons.
static const int BD_CHAR_MAP_XBOX[BD_NUM_DEFINED_CONTROLS] =
{
	XINPUT_GAMEPAD_DPAD_UP,			// BD_MOVE_UP
	XINPUT_GAMEPAD_DPAD_DOWN,		// BD_MOVE_DOWN
	XINPUT_GAMEPAD_DPAD_LEFT,		// BD_MOVE_LEFT
	XINPUT_GAMEPAD_DPAD_RIGHT,		// BD_MOVE_RIGHT
	XINPUT_GAMEPAD_Y,				// BD_BUTTON1
	XINPUT_GAMEPAD_B,				// BD_BUTTON2
	XINPUT_GAMEPAD_A,				// BD_BUTTON3
	XINPUT_GAMEPAD_X,				// BD_BUTTON4
	XINPUT_GAMEPAD_LEFT_TRIGGER,	// BD_BUTTON5
	XINPUT_GAMEPAD_RIGHT_TRIGGER,	// BD_BUTTON6
	XINPUT_GAMEPAD_BACK				// BD_QUIT
};

bdInput::bdInput()
{
}

void bdInput::startFrame()
{
	bdXboxInput::GetMergedInput();
}

bdBool bdInput::wasKeyPressed(const BD_CONTROLS button)
{
	bdBool pressed = false;

	BD_ASSERT(button < BD_NUM_DEFINED_CONTROLS, "button must be one of the values from the BD_CONTROLS enum");

	switch (button)
	{
	case BD_MOVE_UP:
	case BD_MOVE_DOWN:
	case BD_MOVE_LEFT:
	case BD_MOVE_RIGHT:
	case BD_QUIT:
		// These are the digital buttons on the XBox controller
		pressed = (bdXboxInput::defaultGamepad.pressedButtons & BD_CHAR_MAP_XBOX[button]) ? true : false;
		break;
	case BD_BUTTON1:
	case BD_BUTTON2:
	case BD_BUTTON3:
	case BD_BUTTON4:
	case BD_BUTTON5:
	case BD_BUTTON6:
		// These are the analog buttons on the XBox controller
		pressed = bdXboxInput::defaultGamepad.pressedAnalogButtons[BD_CHAR_MAP_XBOX[button]];
		break;
	}

	return pressed;
}

bdBool BD_CALL bdInput::isKeyDown(const BD_CONTROLS button)
{
	bdBool pressed = false;

	BD_ASSERT(button < BD_NUM_DEFINED_CONTROLS, "button must be one of the values from the BD_CONTROLS enum");

	switch (button)
	{
	case BD_MOVE_UP:
	case BD_MOVE_DOWN:
	case BD_MOVE_LEFT:
	case BD_MOVE_RIGHT:
	case BD_QUIT:
		// These are the digital buttons on the XBox controller
		pressed = (bdXboxInput::defaultGamepad.wButtons & BD_CHAR_MAP_XBOX[button]) ? true : false;
		break;
	case BD_BUTTON1:
	case BD_BUTTON2:
	case BD_BUTTON3:
	case BD_BUTTON4:
	case BD_BUTTON5:
	case BD_BUTTON6:
		// These are the analog buttons on the XBox controller
		pressed = bdXboxInput::defaultGamepad.lastAnalogButtons[BD_CHAR_MAP_XBOX[button]];
		break;
	}

	return pressed;
}

void bdInput::printToScreen(const bdNChar8 *const str)
{
	bdGfx::getInstance()->setText(str);
}

#endif // BD_PLATFORM_XBOX && !BD_PLATFORM_XENON
