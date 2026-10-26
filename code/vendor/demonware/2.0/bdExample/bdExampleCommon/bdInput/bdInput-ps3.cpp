// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS3
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdCore/bdCore.h>
#include <bdExample/bdExampleCommon/bdInput/bdInput.h>

#include <bdGraphics/bdGfx.h>

#include <cell/pad.h>

#define BD_INPUT_MAX_GAMEPADS		(4u)
#define BD_INPUT_DEFAULT_PAD		(0u)
#define BD_PAD_STATUS_CONNECTED_MASK	(0x01)
#define BD_LOG_LEVEL "bdExampleCommon/input"

// Firmware version 2.40 discontinued the old CellPadInfo 
// The new CellPadInfo2 does not seem to contain vendor_id nor product_id, so we 
// cannot check for 3rd party gamepads in the same way as we did before
//#define BD_INPUT_USE_UNSUPPORTED_PADS

static bdBool wasPadConnected;
static enum
{
	BD_INPUT_BAD_PAD,
	BD_INPUT_LIBUSB_PAD,
	BD_INPUT_ROCKFIRE_PAD
} padType;

static CellPadData padData;

const bdInt BD_CHAR_MAP_PS3[BD_NUM_DEFINED_CONTROLS][2] =
{
	{2, 0x10}, //	BD_MOVE_UP,
	{2, 0x40}, //	BD_MOVE_DOWN,
	{2, 0x80}, //	BD_MOVE_LEFT,
	{2, 0x20}, //	BD_MOVE_RIGHT,
	{3, 0x10}, //	BD_BUTTON1,
	{3, 0x20}, //	BD_BUTTON2,
	{3, 0x40}, //	BD_BUTTON3,
	{3, 0x80}, //	BD_BUTTON4,
	{3, 0x04}, //	BD_BUTTON5,
	{3, 0x08}, //	BD_BUTTON6,
	{2, 0x01}  //	BD_QUIT,
};

#if defined BD_INPUT_USE_UNSUPPORTED_PADS
static CellPadData storedData;
bdBool getRockfireData(bdInt padno, CellPadData *data);
#endif

bdInput::bdInput()
{
	if (cellPadInit(BD_INPUT_MAX_GAMEPADS) != CELL_PAD_OK)
	{
		// XXX: warn here.
	}
	// Initialize m_buttons, so that the buttons are at their current state and m_lastButtons to all be false.
	for (bdUInt i = 0; i < BD_NUM_DEFINED_CONTROLS; i++)
	{
		m_lastButtons[i] = isKeyDown(static_cast<BD_CONTROLS>(i));
		m_pressedButtons[i] = false;
	}
	wasPadConnected = false;
	padType = BD_INPUT_BAD_PAD;

#if defined BD_INPUT_USE_UNSUPPORTED_PADS
	bdMemset(&storedData, 0, sizeof(storedData));
#endif
}

void bdInput::startFrame()
{
	// TODO: docs say to do this every 3-4 VSYNCs. Also, what about clearing the buffer?
	CellPadInfo2 padInfo;

	if (cellPadGetInfo2(&padInfo) != CELL_PAD_OK)
	{
		BD_WARN(BD_LOG_LEVEL, "Failed to get USB pad info.");
		return;
	}

	// We only care about zeroth pad.

	if ( ( padInfo.port_status[BD_INPUT_DEFAULT_PAD] & BD_PAD_STATUS_CONNECTED_MASK ) == 0 && wasPadConnected)
	{
		BD_INFO(BD_LOG_LEVEL, "Gamepad disconnected.");
		wasPadConnected = false;
		padType = BD_INPUT_BAD_PAD;
		return;
	}

	if ( ( padInfo.port_status[BD_INPUT_DEFAULT_PAD] & BD_PAD_STATUS_CONNECTED_MASK ) == 1 && !wasPadConnected)
	{
		wasPadConnected = true;
		bdInt result = cellPadGetData(BD_INPUT_DEFAULT_PAD, &padData);
		if (result == CELL_PAD_OK)
		{
//			BD_INFO(BD_LOG_LEVEL, "Detected libusbpad-supported pad. Vendor: 0x%x, product: 0x%x",
//					padInfo.vendor_id[BD_INPUT_DEFAULT_PAD], padInfo.product_id[BD_INPUT_DEFAULT_PAD]);
			wasPadConnected = true;
			padType = BD_INPUT_LIBUSB_PAD;
		}
		else
		{
#if defined BD_INPUT_USE_UNSUPPORTED_PADS

			// Rockfire psx bridge is 0583/2050
			if (padInfo.vendor_id[BD_INPUT_DEFAULT_PAD] == 0x0583 &&
				padInfo.product_id[BD_INPUT_DEFAULT_PAD] == 0x2050)
			{
				BD_INFO(BD_LOG_LEVEL, "Detected compatible Rockfire PSX bridge pad");
				padType = BD_INPUT_ROCKFIRE_PAD;
			}
			else
#endif
			{
				BD_WARN(BD_LOG_LEVEL, "Unsupported pad detected. Ignoring.");
				padType = BD_INPUT_BAD_PAD;
			}
		}
	}

#if defined BD_INPUT_USE_UNSUPPORTED_PADS
	if (padType == BD_INPUT_ROCKFIRE_PAD)
	{
		if (!getRockfireData(BD_INPUT_DEFAULT_PAD, &padData))
		{
			BD_WARN(BD_LOG_LEVEL, "Failed to get pad data.");
			return;
		}
	}
#endif // BD_INPUT_USE_UNSUPPORTED_PADS
	else if (padType == BD_INPUT_LIBUSB_PAD)
	{
		if (cellPadGetData(BD_INPUT_DEFAULT_PAD, &padData) != CELL_PAD_OK)
		{
			BD_WARN(BD_LOG_LEVEL, "Failed to get libusb pad data.");
			return;
		}
	}

	else if (padType == BD_INPUT_BAD_PAD)
	{
		return;
	}

	for (bdUInt i=0; i < BD_NUM_DEFINED_CONTROLS; i++)
	{
		bdBool keyDown = isKeyDown(static_cast<BD_CONTROLS>(i));

		m_pressedButtons[i] = !m_lastButtons[i] && keyDown;
		m_lastButtons[i] = keyDown;
	}
}

bdBool BD_CALL bdInput::isKeyDown(const BD_CONTROLS button)
{
	if (padData.button[BD_CHAR_MAP_PS3[button][0]] & BD_CHAR_MAP_PS3[button][1])
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
	return false;
}

void bdInput::printToScreen(const bdNChar8 *const str)
{
	bdGfx::getInstance()->setText(str);
}

#if defined BD_INPUT_USE_UNSUPPORTED_PADS

bdBool getRockfireData(bdInt padno, CellPadData *data)
{
	CellPadData padData;
	if (cellPadGetRawData(padno, &padData) == CELL_PAD_OK)
	{
		if (padData.len > 0)
		{
			bdMemset(&storedData, 0, sizeof(storedData));
			//	bdPrintf("length = %d, data = %x %x %x %x %x %x %x %x\n", padData.len,
			//		padData.button[0], padData.button[1], padData.button[2], padData.button[3],
			//		padData.button[4], padData.button[5], padData.button[6], padData.button[7]);
			//bdBool t = false;
			if (padData.len > 0 && padData.button[4] & 0x4)
			{
				storedData.button[BD_CHAR_MAP_PS3[BD_BUTTON4][0]] |= BD_CHAR_MAP_PS3[BD_BUTTON4][1];
				//bdPrintf("Square down "); t = true;
			}
			if (padData.len > 0 && padData.button[4] & 0x1)
			{
				storedData.button[BD_CHAR_MAP_PS3[BD_BUTTON3][0]] |= BD_CHAR_MAP_PS3[BD_BUTTON3][1];
				//bdPrintf("X down "); t = true;
			}
			if (padData.len > 0 && padData.button[4] & 0x2)
			{
				storedData.button[BD_CHAR_MAP_PS3[BD_BUTTON2][0]] |= BD_CHAR_MAP_PS3[BD_BUTTON2][1];
				//bdPrintf("O down "); t = true;
			}
			if (padData.len > 0 && padData.button[4] & 0x8)
			{
				storedData.button[BD_CHAR_MAP_PS3[BD_BUTTON1][0]] |= BD_CHAR_MAP_PS3[BD_BUTTON1][1];
				//bdPrintf("Triangle down "); t = true;
			}
			if (padData.len > 0 && (padData.button[5] & 0x1 || padData.button[5] & 0x4))
			{
				storedData.button[BD_CHAR_MAP_PS3[BD_QUIT][0]] |= BD_CHAR_MAP_PS3[BD_QUIT][1];
				//bdPrintf("Select down "); t = true;
			}
			//if (padData.len > 0 && (padData.button[5] & 0x2 || padData.button[5] & 0x8))
			//{
			//	//bdPrintf("Start down "); t = true;
			//}
			if (padData.len > 0 && padData.button[4] & 0x10)
			{
				storedData.button[BD_CHAR_MAP_PS3[BD_BUTTON5][0]] |= BD_CHAR_MAP_PS3[BD_BUTTON5][1];
				//bdPrintf("L1 down "); t = true;
			}
			//if (padData.len > 0 && padData.button[4] & 0x40)
			//{
			//	//bdPrintf("L2 down "); t = true;
			//}
			if (padData.len > 0 && padData.button[4] & 0x20)
			{
				storedData.button[BD_CHAR_MAP_PS3[BD_BUTTON6][0]] |= BD_CHAR_MAP_PS3[BD_BUTTON6][1];
				//bdPrintf("R1 down "); t = true;
			}
			//if (padData.len > 0 && padData.button[4] & 0x80)
			//{
			//	//bdPrintf("R2 down "); t = true;
			//}
			if (padData.len > 0 && (padData.button[0] == 0 || padData.button[5] == 0x70))
			{
				storedData.button[BD_CHAR_MAP_PS3[BD_MOVE_LEFT][0]] |= BD_CHAR_MAP_PS3[BD_MOVE_LEFT][1];
				//bdPrintf("LEFT down "); t = true;
			}
			if (padData.len > 0 && (padData.button[0] == 0xff || padData.button[5] == 0x30))
			{
				storedData.button[BD_CHAR_MAP_PS3[BD_MOVE_RIGHT][0]] |= BD_CHAR_MAP_PS3[BD_MOVE_RIGHT][1];
				//bdPrintf("RIGHT down "); t = true;
			}
			if (padData.len > 0 && (padData.button[1] == 0x00 || padData.button[5] == 0x10))
			{
				storedData.button[BD_CHAR_MAP_PS3[BD_MOVE_UP][0]] |= BD_CHAR_MAP_PS3[BD_MOVE_UP][1];
				//bdPrintf("UP down "); t = true;
			}
			if (padData.len > 0 && (padData.button[1] == 0xff || padData.button[5] == 0x50))
			{
				storedData.button[BD_CHAR_MAP_PS3[BD_MOVE_DOWN][0]] |= BD_CHAR_MAP_PS3[BD_MOVE_DOWN][1];
				//bdPrintf("DOWN down "); t = true;
			}
		}



	
		bdMemcpy(data, &storedData, sizeof(storedData));
		data->len = 9;
		data->button[0] = 0x0;
		data->button[1] = 0x74;
		return true;
	}
	else
	{
		return false;
	}
}

#endif // BD_INPUT_USE_UNSUPPORTED_PADS

