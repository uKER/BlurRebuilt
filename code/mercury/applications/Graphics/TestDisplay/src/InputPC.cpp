#include "StdAfx.h"
#include "InputPC.h"

C_Keyboard::C_Keyboard(const S32 index, C_InputManager* pManager) :
	m_Index(index),
	m_pManager(pManager)
{
	BIZ_ASSERT(m_pManager);

	for(U32 i=0; i<UCHAR_MAX; i++)
	{
		m_KeyValues[i] = false;
		m_DebounceKeyValues[i] = false;
		m_DebounceKeyCounters[i] = 0.0f;
	}
}

F32 C_Keyboard::GetValue(const E_InputValue eValue)
{
	F32 fValue = 0.0f;

	switch(eValue)
	{
	case E_InputValue_CameraMoveX:
		if(m_KeyValues[VK_D] != 0.0f)
			fValue += m_KeyValues[VK_D];
		if(m_KeyValues[VK_A] != 0.0f)
			fValue -= m_KeyValues[VK_A];
		break;
	case E_InputValue_CameraMoveY:
		if(m_KeyValues[VK_W] != 0.0f)
			fValue += m_KeyValues[VK_W];
		if(m_KeyValues[VK_S] != 0.0f)
			fValue -= m_KeyValues[VK_S];
		break;
	case E_InputValue_CameraMoveZ:
		if(m_KeyValues[VK_E] != 0.0f)
			fValue += m_KeyValues[VK_E];
		if(m_KeyValues[VK_Q] != 0.0f)
			fValue -= m_KeyValues[VK_Q];
		break;
	}
	
	return fValue;
}

bool C_Keyboard::Update()
{
	F32 time = 0.0f;

	for(U32 i=0; i<UCHAR_MAX; i++)
	{
		if(m_KeyValues[i])
		{
			if(m_DebounceKeyCounters[i] > 0.0f)
			{
				m_DebounceKeyValues[i] = false;
				m_DebounceKeyCounters[i] -= time;
			}
			else
			{
				m_DebounceKeyValues[i] = true;
				m_DebounceKeyCounters[i] = 0.2f;
			}
		}
		else
		{
			m_DebounceKeyValues[i] = false;
			m_DebounceKeyCounters[i] = 0.0f;
		}
	}

	return true;
}

bool C_Keyboard::IsKeyDown(const U8 key)
{
	return m_DebounceKeyValues[key];
}

void C_Keyboard::KeyUp(const U8 key)
{
	m_KeyValues[key] = false;
}

void C_Keyboard::KeyDown(const U8 key)
{
	m_KeyValues[key] = true;
}

C_Mouse::C_Mouse(const S32 index, C_InputManager* pManager) :
	m_Index(index),
	m_pManager(pManager)
{
	BIZ_ASSERT(m_pManager);

	for(U32 i=0; i<E_MouseButton_Count; i++)
	{
		m_ButtonValues[i] = false;
		m_DebounceButtonValues[i] = false;
		m_DebounceButtonCounters[i] = 0.0f;
	}

	m_MoveX = 0.0f;
	m_MoveY = 0.0f;
	m_Wheel	= 1.0f;
}

F32 C_Mouse::GetValue(const E_InputValue eValue)
{
	F32 fValue = 0.0f;

	switch(eValue)
	{
	case E_InputValue_CameraLookX:
		fValue = m_MoveX;
		break;
	case E_InputValue_CameraLookY:
		fValue = m_MoveY;
		break;
	case E_InputValue_CameraSpeed:
		fValue = m_Wheel;
		break;
	}
	
	return fValue;
}

bool C_Mouse::Update()
{
	F32 time = 0.0f;

	for(U32 i=0; i<E_MouseButton_Count; i++)
	{
		if(m_ButtonValues[i])
		{
			if(m_DebounceButtonCounters[i] > 0.0f)
			{
				m_DebounceButtonValues[i] = false;
				m_DebounceButtonCounters[i] -= time;
			}
			else
			{
				m_DebounceButtonValues[i] = true;
				m_DebounceButtonCounters[i] = 0.2f;
			}
		}
		else
		{
			m_DebounceButtonValues[i] = false;
			m_DebounceButtonCounters[i] = 0.0f;
		}
	}

	if(m_ButtonValues[E_MouseButton_Left])
	{
		RECT rect;
		GetWindowRect(m_hWnd, &rect);

		POINT center;
		center.x = (rect.left+rect.right)/2;
		center.y = (rect.top+rect.bottom)/2;

		POINT cursor;
		GetCursorPos(&cursor);

		m_MoveX = -(F32)(center.x-cursor.x)*0.05f;
		m_MoveY = (F32)(center.y-cursor.y)*0.05f;

		SetCursorPos(center.x, center.y);
	}
	else
	{
		m_MoveX = 0.0f;
		m_MoveY = 0.0f;
	}

	return true;
}

void C_Mouse::MouseDown(const U8 mouseButton)
{
	BIZ_ASSERT(mouseButton < E_MouseButton_Count);
	m_ButtonValues[mouseButton] = true;

	if(mouseButton == E_MouseButton_Left)
	{
		GetCursorPos(&m_Cursor);

		RECT windowRect;
		GetWindowRect(m_hWnd, &windowRect);

		POINT windowCenter;
		windowCenter.x = (windowRect.left + windowRect.right) / 2;
		windowCenter.y = (windowRect.top + windowRect.bottom) / 2;

		ShowCursor(false);
		
		SetCursorPos(windowCenter.x, windowCenter.y);
	}
}

void C_Mouse::MouseUp(const U8 mouseButton)
{
	BIZ_ASSERT(mouseButton < E_MouseButton_Count);
	m_ButtonValues[mouseButton] = false;

	if(mouseButton == E_MouseButton_Left)
	{
		SetCursorPos(m_Cursor.x, m_Cursor.y);	

		ShowCursor(true);
	}
}

void C_Mouse::WheelDown()
{
	m_Wheel *= 0.9f;
}

void C_Mouse::WheelUp()
{
	m_Wheel *= 1.1f;
}

bool C_Mouse::IsMouseDown(const E_MouseButton button) const
{
	return m_ButtonValues[button];
}

bool C_Mouse::IsMouseDownDebounce(const E_MouseButton button) const
{
	return m_DebounceButtonValues[button];
}
