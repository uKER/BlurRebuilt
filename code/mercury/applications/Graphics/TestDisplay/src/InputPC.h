#ifndef INPUT_PC_H
#define INPUT_PC_H

#ifdef _MSC_VER
#pragma once
#endif

#include "InputBase.h"

struct C_InputManager;

#define VK_0 0x30
#define VK_1 0x31
#define VK_2 0x32
#define VK_3 0x33
#define VK_4 0x34
#define VK_5 0x35
#define VK_6 0x36
#define VK_7 0x37
#define VK_8 0x38
#define VK_9 0x39
#define VK_A 0x41
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C
#define VK_M 0x4D
#define VK_N 0x4E
#define VK_O 0x4F
#define VK_P 0x50
#define VK_Q 0x51
#define VK_R 0x52
#define VK_S 0x53
#define VK_T 0x54
#define VK_U 0x55
#define VK_V 0x56
#define VK_W 0x57
#define VK_X 0x58
#define VK_Y 0x59
#define VK_Z 0x5A

struct C_InputManager : public C_InputManagerBase
{
};

struct C_Keyboard : public C_Input
{
	C_Keyboard(const S32 index, C_InputManager* pManager);

	F32 GetValue(const E_InputValue eValue);
	
	bool Update();

	bool IsKeyDown(const U8 key);
	void KeyDown(const U8 key);
	void KeyUp(const U8 key);
	
	bool				m_KeyValues[UCHAR_MAX];
	bool				m_DebounceKeyValues[UCHAR_MAX];
	F32					m_DebounceKeyCounters[UCHAR_MAX];

	S32					m_Index;
	C_InputManager*		m_pManager;
};

enum E_MouseButton
{
	E_MouseButton_Left = 0,
	E_MouseButton_Right,
	E_MouseButton_Middle,
	E_MouseButton_WheelDown,
	E_MouseButton_WheelUp,
	E_MouseButton_Count,
};

struct C_Mouse : public C_Input
{
	C_Mouse(const S32 index, C_InputManager* pManager);

	F32 GetValue(const E_InputValue eValue);
	
	bool Update();

	void MouseDown(const U8 mouseButton);
	void MouseUp(const U8 mouseButton);

	void WheelDown();
	void WheelUp();

	bool IsMouseDown(const E_MouseButton button) const;
	bool IsMouseDownDebounce(const E_MouseButton button) const;
	
	HWND				m_hWnd;
	POINT				m_Cursor;
	bool				m_ButtonValues[E_MouseButton_Count];
	bool				m_DebounceButtonValues[E_MouseButton_Count];
	F32					m_DebounceButtonCounters[E_MouseButton_Count];

	F32					m_MoveX;
	F32					m_MoveY;
	F32					m_Wheel;

	S32					m_Index;
	C_InputManager*		m_pManager;
};

#endif // INPUT_PC_H
