#include "StdAfx.h"
#include "InputPS3.h"

F32 GetPadButtonValueAbsolute(CellPadData* pPadData, const U32 padCode)
{
	return ((F32)pPadData->button[padCode]) * (1.0f / 255.0f);
}

F32 GetPadButtonValueRelative(CellPadData* pPadData, const U32 padCode, const U32 padBits)
{
	return (pPadData->button[padCode] & padBits) ? 1.0f : 0.0f;
}

F32 GetPadAxisValue(CellPadData* pPadData, const U32 padCode)
{
	return (((F32)pPadData->button[padCode] / 255.0f) * 2.0f) - 1.0f;
}

C_Joypad::C_Joypad(const S32 index, C_InputManager* pManager) :
	m_Index(index),
	m_Valid(false),
	m_Connected(false),
	m_Deadzone(0.2f),
	m_LeftX(0.0f),
	m_LeftY(0.0f),
	m_RightX(0.0f),
	m_RightY(0.0f),
	m_pManager(pManager)
{
	BIZ_ASSERT(m_pManager);
	BIZ_ASSERT(m_Index < C_InputManager::m_MaxPads);

	memset(&m_PadData, 0, sizeof(m_PadData));
}

F32 C_Joypad::GetValue(const E_InputValue eValue)
{
	F32 fValue = 0.0f;
	
	if(m_Valid)
	{
		switch(eValue)
		{
		case E_InputValue_Quit:
			fValue = GetPadButtonValueRelative(&m_PadData, CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_CIRCLE) ? 1.0f : 0.0f;
			break;
		case E_InputValue_Pause:
			fValue = GetPadButtonValueRelative(&m_PadData, CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_CROSS) ? 1.0f : 0.0f;
			break;
		case E_InputValue_Unpause:
			fValue = GetPadButtonValueRelative(&m_PadData, CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_SQUARE) ? 1.0f : 0.0f;
			break;
		case E_InputValue_CameraSlow:
			fValue = GetPadButtonValueRelative(&m_PadData, CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_L2) ? 1.0f : 0.0f;
			break;
		case E_InputValue_CameraFast:
			fValue = GetPadButtonValueRelative(&m_PadData, CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_R2) ? 1.0f : 0.0f;
			break;
		case E_InputValue_CameraMoveX:
			fValue = m_LeftX;
			break;
		case E_InputValue_CameraMoveY:
			fValue = m_LeftY;
			break;
		case E_InputValue_CameraLookX:
			fValue = m_RightX;
			break;
		case E_InputValue_CameraLookY:
			fValue = m_RightY;
			break;
		}
	}

	return fValue;
}

bool C_Joypad::Update()
{
	BIZ_ASSERT(m_pManager && m_Index < C_InputManager::m_MaxPads);

	bool connected = m_pManager->m_PadInfo.port_status[m_Index] == 1;

	if(connected != m_Connected)
	{
		int32_t result = cellPadClearBuf(m_Index);
		switch(result)
		{
		case CELL_PAD_OK:
			break;
		case CELL_PAD_ERROR_FATAL:
			LOGERROR(Display::g_Log, "Failed in cellPadClearBuf CELL_PAD_ERROR_FATAL\n");
			return false;
		case CELL_PAD_ERROR_INVALID_PARAMETER:
			LOGERROR(Display::g_Log, "Failed in cellPadClearBuf CELL_PAD_ERROR_INVALID_PARAMETER\n");
			return false;
		case CELL_PAD_ERROR_UNINITIALIZED:
			LOGERROR(Display::g_Log, "Failed in cellPadClearBuf CELL_PAD_ERROR_UNINITIALIZED\n");
			return false;
		case CELL_PAD_ERROR_NO_DEVICE:
			LOGERROR(Display::g_Log, "Failed in cellPadClearBuf CELL_PAD_ERROR_NO_DEVICE\n");
			return false;
		default:
			LOGERROR(Display::g_Log, "Failed in cellPadClearBuf %i\n", result);
			return false;
		}

		m_Connected = connected;
	}

	if(m_Connected)
	{
		int32_t result = cellPadGetData(m_Index, &m_PadData);
		if(result != 0 || m_PadData.len == 0)
		{
			return false;
		}
		else
		{
			m_Valid = true;

			m_LeftX = GetPadAxisValue(&m_PadData, CELL_PAD_BTN_OFFSET_ANALOG_LEFT_X);
			m_LeftY = -GetPadAxisValue(&m_PadData, CELL_PAD_BTN_OFFSET_ANALOG_LEFT_Y);
			ApplyDeadzone(m_LeftX, m_LeftY);

			m_RightX = GetPadAxisValue(&m_PadData, CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_X);
			m_RightY = -GetPadAxisValue(&m_PadData, CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_Y);
			ApplyDeadzone(m_RightX, m_RightY);
		}
	}
	else
	{
		m_Valid = false;
	}

	return true;
}


void C_Joypad::ApplyDeadzone(F32& x, F32& y)
{
	const F32 len2 = x * x + y * y;
	const F32 m_Deadzone2 = m_Deadzone * m_Deadzone;

	if (len2 <= m_Deadzone2)
	{
		x = y = 0.0f;
	}
	else
	{
		const F32 len = sqrtf(len2);
		F32 newLen = (len - m_Deadzone) / (1.0f - m_Deadzone);

		if (newLen > 1.0f)
			newLen = 1.0f;

		F32 ratio = newLen / len;

		x *= ratio;
		y *= ratio;
	}
}

C_InputManager::C_InputManager()
{
	memset(&m_PadInfo, 0, sizeof(m_PadInfo));
}

bool C_InputManager::Create()
{
	int32_t result = cellPadInit(m_MaxPads);
	
	switch(result)
	{
	case CELL_PAD_OK:
		return true;
	case CELL_PAD_ERROR_FATAL:
		LOGERROR(Display::g_Log, "Failed in cellPadInit CELL_PAD_ERROR_FATAL\n");
		return false;
	case CELL_PAD_ERROR_INVALID_PARAMETER:
		LOGERROR(Display::g_Log, "Failed in cellPadInit CELL_PAD_ERROR_INVALID_PARAMETER\n");
		return false;
	case CELL_PAD_ERROR_ALREADY_INITIALIZED:
		LOGERROR(Display::g_Log, "Failed in cellPadInit CELL_PAD_ERROR_ALREADY_INITIALIZED\n");
		return false;
	case CELL_PAD_ERROR_RESOURCE_ALLOCATION_FAILED:
		LOGERROR(Display::g_Log, "Failed in cellPadInit CELL_PAD_ERROR_RESOURCE_ALLOCATION_FAILED\n");
		return false;
	default:
		LOGERROR(Display::g_Log, "Failed in cellPadInit %i\n", result);
		return false;
	}
	
	return true;
}

bool C_InputManager::Destroy()
{
	int32_t result = cellPadEnd();
	
	switch(result)
	{
	case CELL_PAD_OK:
		return true;
	case CELL_PAD_ERROR_FATAL:
		LOGERROR(Display::g_Log, "Failed in cellPadEnd CELL_PAD_ERROR_FATAL\n");
		return false;
	case CELL_PAD_ERROR_UNINITIALIZED:
		LOGERROR(Display::g_Log, "Failed in cellPadEnd CELL_PAD_ERROR_UNINITIALIZED\n");
		return false;
	default:
		LOGERROR(Display::g_Log, "Failed in cellPadEnd %i\n", result);
		return false;
	}
	
	return true;
}

bool C_InputManager::Update()
{
	int32_t result = cellPadGetInfo2(&m_PadInfo);

	switch(result)
	{
	case CELL_PAD_OK:
		return true;
	case CELL_PAD_ERROR_FATAL:
		LOGERROR(Display::g_Log, "Failed in cellPadGetInfo2 CELL_PAD_ERROR_FATAL\n");
		return false;
	case CELL_PAD_ERROR_INVALID_PARAMETER:
		LOGERROR(Display::g_Log, "Failed in cellPadGetInfo2 CELL_PAD_ERROR_INVALID_PARAMETER\n");
		return false;
	case CELL_PAD_ERROR_UNINITIALIZED:
		LOGERROR(Display::g_Log, "Failed in cellPadGetInfo2 CELL_PAD_ERROR_UNINITIALIZED\n");
		return false;
	default:
		LOGERROR(Display::g_Log, "Failed in cellPadGetInfo2 %i\n", result);
		return false;
	}
	
	return true;
}
