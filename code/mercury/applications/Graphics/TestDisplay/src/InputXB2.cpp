#include "StdAfx.h"
#include "InputXB2.h"

SHORT C_Gamepad::LEFT_THUMB_DEADZONE  = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
SHORT C_Gamepad::RIGHT_THUMB_DEADZONE = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;

FLOAT ConvertThumbstickValue( SHORT sThumbstickValue, SHORT sDeadZone )
{
    if( sThumbstickValue > +sDeadZone )
    {
        return (sThumbstickValue-sDeadZone) / (32767.0f-sDeadZone);
    }
    if( sThumbstickValue < -sDeadZone )
    {
        return (sThumbstickValue+sDeadZone+1.0f) / (32767.0f-sDeadZone);
    }
    return 0.0f;
}

C_Joypad::C_Joypad(const S32 index, C_InputManager* pManager) :
	m_Index(index),
	m_pManager(pManager)
{
	BIZ_ASSERT(m_pManager);
	BIZ_ASSERT(m_Index < XUSER_MAX_COUNT);
}

F32 C_Joypad::GetValue(const E_InputValue eValue)
{
	F32 fValue = 0.0f;
	
	if(m_Gamepad.bConnected)
	{
		switch(eValue)
		{
		case E_InputValue_CameraSlow:
			fValue = m_Gamepad.bLeftTrigger ? 1.0f : 0.0f;
			break;
		case E_InputValue_CameraFast:
			fValue = m_Gamepad.bRightTrigger ? 1.0f : 0.0f;
			break;
		case E_InputValue_CameraMoveX:
			fValue = m_Gamepad.fX1;
			break;
		case E_InputValue_CameraMoveY:
			fValue = m_Gamepad.fY1;
			break;
		case E_InputValue_CameraLookX:
			fValue = m_Gamepad.fX2;
			break;
		case E_InputValue_CameraLookY:
			fValue = m_Gamepad.fY2;
			break;
		}
	}
	
	return fValue;
}

bool C_Joypad::Update()
{
	BIZ_ASSERT(m_Index < XUSER_MAX_COUNT);

	// Read the input state
	XINPUT_STATE InputState;
	BOOL bWasConnected = m_Gamepad.bConnected;
	m_Gamepad.bConnected = ( XInputGetState( m_Index, &InputState ) == ERROR_SUCCESS ) ? TRUE : FALSE;

	// Track insertion and removals
	m_Gamepad.bRemoved  = (  bWasConnected && !m_Gamepad.bConnected ) ? TRUE : FALSE;
	m_Gamepad.bInserted = ( !bWasConnected &&  m_Gamepad.bConnected ) ? TRUE : FALSE;

	if( FALSE == m_Gamepad.bConnected )
		return false;

	// Store the capabilities of the device
	if( TRUE == m_Gamepad.bInserted )
	{
		ZeroMemory( &m_Gamepad, sizeof(C_Gamepad) );
		m_Gamepad.bConnected = TRUE;
		m_Gamepad.bInserted  = TRUE;
		XInputGetCapabilities( m_Index, XINPUT_FLAG_GAMEPAD, &m_Gamepad.caps );
	}

	// Copy gamepad to local structure
	memcpy( &m_Gamepad, &InputState.Gamepad, sizeof(XINPUT_GAMEPAD) );

	// Put Xbox device input for the gamepad into our custom format
	m_Gamepad.fX1 = ConvertThumbstickValue( m_Gamepad.sThumbLX, C_Gamepad::LEFT_THUMB_DEADZONE );
	m_Gamepad.fY1 = ConvertThumbstickValue( m_Gamepad.sThumbLY, C_Gamepad::LEFT_THUMB_DEADZONE );
	m_Gamepad.fX2 = ConvertThumbstickValue( m_Gamepad.sThumbRX, C_Gamepad::RIGHT_THUMB_DEADZONE );
	m_Gamepad.fY2 = ConvertThumbstickValue( m_Gamepad.sThumbRY, C_Gamepad::RIGHT_THUMB_DEADZONE );

	// Get the boolean buttons that have been pressed since the last
	// call. Each button is represented by one bit.
	m_Gamepad.wPressedButtons = ( m_Gamepad.wLastButtons ^ m_Gamepad.wButtons ) & m_Gamepad.wButtons;
	m_Gamepad.wLastButtons    = m_Gamepad.wButtons;

	// Figure out if the left trigger has been pressed or released
	BOOL bPressed = ( m_Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD );

	if( bPressed )
		m_Gamepad.bPressedLeftTrigger = !m_Gamepad.bLastLeftTrigger;
	else
		m_Gamepad.bPressedLeftTrigger = FALSE;

	// Store the state for next time
	m_Gamepad.bLastLeftTrigger = bPressed;

	// Figure out if the right trigger has been pressed or released
	bPressed = ( m_Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD );

	if( bPressed )
		m_Gamepad.bPressedRightTrigger = !m_Gamepad.bLastRightTrigger;
	else
		m_Gamepad.bPressedRightTrigger = FALSE;

	// Store the state for next time
	m_Gamepad.bLastRightTrigger = bPressed;

	// Set the user index for this gamepad
	m_Gamepad.dwUserIndex = m_Index;
	
	return true;
}
