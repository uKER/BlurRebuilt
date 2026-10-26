#ifndef INPUT_XB2_H
#define INPUT_XB2_H

#ifdef _MSC_VER
#pragma once
#endif

#include "InputBase.h"

struct C_Gamepad : public XINPUT_GAMEPAD
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
    FLOAT      fX1;
    FLOAT      fY1;
    FLOAT      fX2;
    FLOAT      fY2;

    // Records the state (when last updated) of the buttons.
    // These remain set as long as the button is pressed.
    WORD       wLastButtons;
    BOOL       bLastLeftTrigger;
    BOOL       bLastRightTrigger;

    // Records which buttons were pressed this frame - only set on
    // the frame that the button is first pressed.
    WORD       wPressedButtons;
    BOOL       bPressedLeftTrigger;
    BOOL       bPressedRightTrigger;

    // Device properties
    XINPUT_CAPABILITIES caps;
    BOOL       bConnected;

    // Flags for whether game pad was just inserted or removed
    BOOL       bInserted;
    BOOL       bRemoved;

    // The user index associated with this gamepad
    DWORD      dwUserIndex;

    // Deadzone pseudo-constants for the thumbsticks
    static SHORT LEFT_THUMB_DEADZONE;
    static SHORT RIGHT_THUMB_DEADZONE;
};

struct C_InputManager;

struct C_Joypad : public C_Input
{
	C_Joypad(const S32 index, C_InputManager* pManager);
	F32 GetValue(const E_InputValue eValue);
	bool Update();

	S32					m_Index;
	C_Gamepad			m_Gamepad;
	C_InputManager*		m_pManager;
};

struct C_InputManager : public C_InputManagerBase
{
};

#endif // INPUT_XB2_H
