// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE		: A class to read input from the xbox controller

#include <bdExample/bdExampleCommon/bdInput/bdInput.h>

#ifdef BD_PLATFORM_XENON

#include <xtl.h>
#include <xbdm.h>

#include <bdGraphics/bdGfx.h>

const short INPUT_DEADZONE = (short)( 0.24f * float(0x7FFF));

struct bdGamepad : public XINPUT_GAMEPAD
{
	// The following members are inherited from XINPUT_GAMEPAD:
	//    WORD    wButtons;
	//    BYTE    bLeftTrigger;
	//    BYTE    bRightTrigger;
	//    SHORT   sThumbLX;
	//    SHORT   sThumbLY;
	//    SHORT   sThumbRX;
	//    SHORT   sThumbRY;

	// Thumb stick values converted to range [-1,+1]
	float				x1;
	float				y1;
	float				x2;
	float				y2;

	// Records the state (when last updated) of the buttons.
	// These remain set as long as the button is pressed.
	unsigned short		lastButtons;
	bool				lastLeftTrigger;
	bool				lastRightTrigger;

	// Records which buttons were pressed this frame - only set on
	// the frame that the button is first pressed.
	unsigned short		pressedButtons;
	bool				pressedLeftTrigger;
	bool				pressedRightTrigger;

    // Device properties
	XINPUT_CAPABILITIES caps;
	bool				connected;

    // Flags for whether game pad was just inserted or removed
	bool				inserted;
	bool				removed;
};

class bdXenonInput
{
public:
	static bdGamepad	gamepads[4];
	static bdGamepad	defaultGamepad;

	static void BD_CALL GetInput(bdGamepad *userGamepads = NULL);
	static bdGamepad* BD_CALL GetMergedInput();
};

bdGamepad bdXenonInput::gamepads[4];
bdGamepad bdXenonInput::defaultGamepad;

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

void bdXenonInput::GetInput(bdGamepad *pGamepads)
{
	// If this is the first run, initialize the control data to zeros
	static bool initialised = false;
	if (!initialised)
	{
		ZeroMemory(gamepads,		sizeof(gamepads));
		ZeroMemory(&defaultGamepad,	sizeof(defaultGamepad));
		initialised = true;
	}

	if (!pGamepads)
	{
		pGamepads = gamepads;
	}

	// Get the input for each of the four controllers
	for (int i = 0; i < 4; i++)
	{
		XINPUT_STATE input_state;

		// Was the controller connected last time?
		bool wasConnected = pGamepads[i].connected;

		// Find the connection state of the controller and update the fields to notify the app of what's happened
		pGamepads[i].connected = ( XInputGetState(i, &input_state) == ERROR_SUCCESS) ? true : false;
		pGamepads[i].removed = (wasConnected && !pGamepads[i].connected) ? true : false;
		pGamepads[i].inserted = (!wasConnected && pGamepads[i].connected) ? true : false;

		// If this controller is not connected, there is nothing else to do with this controller
		if (!pGamepads[i].connected)
			continue;

		// If the controller has just been inserted, initialize its data structures
		if (pGamepads[i].inserted)
		{
			ZeroMemory(&pGamepads[i], sizeof(gamepads[i]));
			pGamepads[i].connected = true;
			pGamepads[i].inserted = true;
			XInputGetCapabilities(i, 0, &pGamepads[i].caps);
		}

		// Read in the controller states.
		memcpy(&pGamepads[i], &input_state.Gamepad, sizeof(XINPUT_GAMEPAD));

		// Convert the thumbstick readings to normalized floats
		pGamepads[i].x1 = ConvertThumbstickValue(pGamepads[i].sThumbLX);
		pGamepads[i].y1 = ConvertThumbstickValue(pGamepads[i].sThumbLY);
		pGamepads[i].x2 = ConvertThumbstickValue(pGamepads[i].sThumbRX);
		pGamepads[i].y2 = ConvertThumbstickValue(pGamepads[i].sThumbRY);

		// Discover which digital buttons have just been pressed since the last reading of the control pads
		pGamepads[i].pressedButtons = (pGamepads[i].lastButtons ^ pGamepads[i].wButtons) & pGamepads[i].wButtons;
		pGamepads[i].lastButtons	= pGamepads[i].wButtons;

		// Now do the same for the left and right triggers
		{
			bool pressed = (pGamepads[i].bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

			if (pressed)
			{
				pGamepads[i].pressedLeftTrigger = !pGamepads[i].lastLeftTrigger;
			}
			else
			{
				pGamepads[i].pressedLeftTrigger = false;
			}

			pGamepads[i].lastLeftTrigger = pressed;

			pressed = (pGamepads[i].bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

			if (pressed)
			{
				pGamepads[i].pressedRightTrigger = !pGamepads[i].lastRightTrigger;
			}
			else
			{
				pGamepads[i].pressedRightTrigger = false;
			}
			pGamepads[i].lastRightTrigger = pressed;
		}
	}
}

bdGamepad *bdXenonInput::GetMergedInput()
{
	// Read input from the controllers
	GetInput();

	// Zero the data structure
	ZeroMemory(&defaultGamepad, sizeof(defaultGamepad));
	int nThumbLX = 0;
	int nThumbLY = 0;
	int nThumbRX = 0;
	int nThumbRY = 0;

	// Go through each controller and merge their input, so an app can respond the same to all controllers
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

			// Merge analogue stick data
            defaultGamepad.x1 += gamepads[i].x1;
            defaultGamepad.y1 += gamepads[i].y1;
            defaultGamepad.x2 += gamepads[i].x2;
            defaultGamepad.y2 += gamepads[i].y2;

			// Merge digital button data
            defaultGamepad.wButtons			|= gamepads[i].wButtons;
            defaultGamepad.pressedButtons	|= gamepads[i].pressedButtons;
            defaultGamepad.lastButtons		|= gamepads[i].lastButtons;

			// Merge left trigger data
			defaultGamepad.bLeftTrigger			|= gamepads[i].bLeftTrigger;
			defaultGamepad.pressedLeftTrigger	|= gamepads[i].pressedLeftTrigger;
			defaultGamepad.lastLeftTrigger		|= gamepads[i].lastLeftTrigger;

			// Merge right trigger data
			defaultGamepad.bRightTrigger		|= gamepads[i].bRightTrigger;
			defaultGamepad.pressedRightTrigger	|= gamepads[i].pressedRightTrigger;
			defaultGamepad.lastRightTrigger		|= gamepads[i].lastRightTrigger;
        }
	}

	// Store thumbstick values
	defaultGamepad.sThumbLX = (short)nThumbLX;
	defaultGamepad.sThumbLY = (short)nThumbLY;
	defaultGamepad.sThumbRX = (short)nThumbRX;
	defaultGamepad.sThumbRY = (short)nThumbRY;

	// If the user presses both triggers and the right shoulder button, reset the console
	if ((defaultGamepad.bLeftTrigger > 128) &&
		(defaultGamepad.bRightTrigger > 128) &&
		(defaultGamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER))
	{
		DmReboot(DMBOOT_COLD);
	}

	// Return a pointer to the merged data structure
	return &defaultGamepad;
}

// This array stores a mapping from values in the BD_CONTROLS enum to their respective XINPUT_* values
static const int BD_CHAR_MAP_XENON[BD_NUM_DEFINED_CONTROLS] =
{
	XINPUT_GAMEPAD_DPAD_UP,		// BD_MOVE_UP
	XINPUT_GAMEPAD_DPAD_DOWN,	// BD_MOVE_DOWN
	XINPUT_GAMEPAD_DPAD_LEFT,	// BD_MOVE_LEFT
	XINPUT_GAMEPAD_DPAD_RIGHT,	// BD_MOVE_RIGHT
	XINPUT_GAMEPAD_Y,			// BD_BUTTON1
	XINPUT_GAMEPAD_B,			// BD_BUTTON2
	XINPUT_GAMEPAD_A,			// BD_BUTTON3
	XINPUT_GAMEPAD_X,			// BD_BUTTON4
	0,							// BD_BUTTON5
	0,							// BD_BUTTON6
	XINPUT_GAMEPAD_BACK			// BD_QUIT
};

bdInput::bdInput()
{
}

void bdInput::startFrame()
{
	// Read the controllers' state at the beginning of the frame and only once per frame
	bdXenonInput::GetMergedInput();
}

bdBool BD_CALL bdInput::isKeyDown(const BD_CONTROLS button)
{
	bdBool pressed = false;

	// Make sure that button is a valid value
	BD_ASSERT(button < BD_NUM_DEFINED_CONTROLS, "button must be one of the values from the BD_CONTROLS enum");

	switch (button)
	{
	case BD_MOVE_UP:
	case BD_MOVE_DOWN:
	case BD_MOVE_LEFT:
	case BD_MOVE_RIGHT:
	case BD_BUTTON1:
	case BD_BUTTON2:
	case BD_BUTTON3:
	case BD_BUTTON4:
	case BD_QUIT:
		// button is a digital button
		pressed = (bdXenonInput::defaultGamepad.wButtons & BD_CHAR_MAP_XENON[button]) ? true : false;
		break;
	case BD_BUTTON5:
		// button is Left Trigger
		pressed = bdXenonInput::defaultGamepad.lastLeftTrigger;
	case BD_BUTTON6:
		// button is Right Trigger
		pressed = bdXenonInput::defaultGamepad.lastRightTrigger;
		break;
	}

	// Return the state of the button
	return pressed;
}

bdBool bdInput::wasKeyPressed(const BD_CONTROLS button)
{
	bdBool pressed = false;

	// Make sure that button is a valid value
	BD_ASSERT(button < BD_NUM_DEFINED_CONTROLS, "button must be one of the values from the BD_CONTROLS enum");

	switch (button)
	{
	case BD_MOVE_UP:
	case BD_MOVE_DOWN:
	case BD_MOVE_LEFT:
	case BD_MOVE_RIGHT:
	case BD_BUTTON1:
	case BD_BUTTON2:
	case BD_BUTTON3:
	case BD_BUTTON4:
	case BD_QUIT:
		// button is a digital button
		pressed = (bdXenonInput::defaultGamepad.pressedButtons & BD_CHAR_MAP_XENON[button]) ? true : false;
		break;
	case BD_BUTTON5:
		// button is left trigger
		pressed = bdXenonInput::defaultGamepad.pressedLeftTrigger;
	case BD_BUTTON6:
		// button is right trigger
		pressed = bdXenonInput::defaultGamepad.pressedRightTrigger;
		break;
	}

	return pressed;
}

void bdInput::printToScreen(const bdNChar8 *const str)
{
	bdGfx::getInstance()->setText(str);
}

#endif // BD_PLATFORM_XENON
