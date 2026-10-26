#ifndef UIINPUTS_H
#define UIINPUTS_H

#ifdef _MSC_VER
#pragma once
#endif

enum E_UiAxis
{
	E_UiAxis_LeftStick_UD,
	E_UiAxis_LeftStick_LR,
	E_UiAxis_RightStick_UD,
	E_UiAxis_RightStick_LR,

	E_UiAxis_Count
};

enum E_UiButton
{
	E_UiButton_Pad_Up,
	E_UiButton_Pad_Down,
	E_UiButton_Pad_Left,
	E_UiButton_Pad_Right,
	E_UiButton_Confirm,
	E_UiButton_Cancel,
	E_UiButton_Option1,
	E_UiButton_Option2,
	E_UiButton_LeftTrigger,
	E_UiButton_RightTrigger,
	E_UiButton_LeftShoulder,
	E_UiButton_RightShoulder,
	E_UiButton_Start,
	E_UiButton_Back,
	E_UiButton_Count
};

struct UiPadState
{
	UiPadState();

	U64		m_time;
	float	m_axes[E_UiAxis_Count];
	U32		m_buttons;
};


struct UiPad
{
public:
	UiPad();

	void pushState(const UiPadState& state);
	
	bool	buttonDown(E_UiButton button) const;
	bool buttonPressed(E_UiButton button) const;
	float		  axis(E_UiAxis axis) const;
	
	enum { MAX_STATES = 16 };
	UiPadState m_stateStack[MAX_STATES];
	int m_stackPtr;
};

#include "netCore/netEnums.h"

class UiInputs
{
	DECLARE_SINGLETON(UiInputs);

public:
	void startup(int hwnd);
	void update();
	void shutdown();

		  UiPad& pad(int padNo)		  { assert(padNo >= 0 && padNo < MAX_PADS); return m_pads[padNo]; }
	const UiPad& pad(int padNo) const { assert(padNo >= 0 && padNo < MAX_PADS); return m_pads[padNo]; }
	int			 padCount()		const { return MAX_PADS; }

private:
	bool m_started;
	enum { MAX_PADS = 4 };

	UiPad m_pads[MAX_PADS];
};

#endif //UIINPUTS_H