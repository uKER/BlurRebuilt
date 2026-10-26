#ifndef INPUT_PS3_H
#define INPUT_PS3_H

#ifdef _MSC_VER
#pragma once
#endif

#include "InputBase.h"

struct C_InputManager;

struct C_Joypad : public C_Input
{
	C_Joypad(const S32 index, C_InputManager* pManager);
	F32 GetValue(const E_InputValue eValue);
	bool Update();
	void ApplyDeadzone(F32& x, F32& y);

	S32					m_Index;
	bool				m_Valid;
	bool				m_Connected;
	F32					m_Deadzone;
	F32					m_LeftX;
	F32					m_LeftY;
	F32					m_RightX;
	F32					m_RightY;
	CellPadData			m_PadData;
	C_InputManager*		m_pManager;
};

struct C_InputManager : public C_InputManagerBase
{
	C_InputManager();

	bool Create();
	bool Destroy();
	bool Update();

	static const S32	m_MaxPads = 4;
	CellPadInfo2		m_PadInfo;
};

#endif // INPUT_PS3_H
