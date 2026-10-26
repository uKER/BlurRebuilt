#include "stdafx.h"

#include <Input/Device.h>
#include <Input/DeviceManager.h>
#include <Input/GameInputMapping.h>

#if defined(PLATFORM_PC)
#include <Input/Pad360.h>
#endif

#if defined(PLATFORM_PS3)
#include <Input/PadPS3.h>
#endif

#if defined(PLATFORM_PC)
#include <Input/KeyboardPC.h>
#include <Input/Mouse.h>
#endif

#include <Game/GameControls/GameControls.h>

#if defined(PLATFORM_PC)
#include <Ui/UiXtConfig.h>	// [Blur2PC] C_ControlConfig, the pad mapping of a controller configuration
#endif


namespace Game
{
using namespace Input;

#if defined ( PLATFORM_PS3 )

struct C_PadOrAxisSetup
{
      E_UiButton					m_PadOrAxis;
      C_PadPS3::E_Button     m_Pad;
      C_PadPS3::E_Axis       m_Axis;
      bool                          m_InvertAxis;
};

static const C_PadOrAxisSetup kPadOrAxisSetup [] = 
{
      {     E_UiButton_PadOrAxis_Up,      C_PadPS3::E_Button_DPadUp,         C_PadPS3::E_Axis_LeftStick_Y,      false,    },
      {     E_UiButton_PadOrAxis_Down,    C_PadPS3::E_Button_DPadDown,       C_PadPS3::E_Axis_LeftStick_Y,      true,		},
      {     E_UiButton_PadOrAxis_Left,    C_PadPS3::E_Button_DPadLeft,       C_PadPS3::E_Axis_LeftStick_X,      true,		},
      {     E_UiButton_PadOrAxis_Right,   C_PadPS3::E_Button_DPadRight,      C_PadPS3::E_Axis_LeftStick_X,      false,    },
};

#elif defined(PLATFORM_PC)

struct C_PadOrAxisSetup
{
      E_UiButton					m_PadOrAxis;
      C_Pad360::E_Button     m_Pad;
      C_Pad360::E_Axis       m_Axis;
#if defined (PLATFORM_PC)
	  C_Keyboard::E_Button	m_Key;
#endif
      bool                          m_InvertAxis;
};


#if defined(PLATFORM_PC)

static const C_PadOrAxisSetup kPadOrAxisSetup [] = 
{
      {     E_UiButton_PadOrAxis_Up,      C_Pad360::E_Button_DPadUp,     C_Pad360::E_Axis_LeftStick_Y,  C_Keyboard::E_Button_Up,		false,  },
      {     E_UiButton_PadOrAxis_Down,    C_Pad360::E_Button_DPadDown,   C_Pad360::E_Axis_LeftStick_Y,  C_Keyboard::E_Button_Down,		true,	},
      {     E_UiButton_PadOrAxis_Left,    C_Pad360::E_Button_DPadLeft,   C_Pad360::E_Axis_LeftStick_X,  C_Keyboard::E_Button_Left,		true,	},
      {     E_UiButton_PadOrAxis_Right,   C_Pad360::E_Button_DPadRight,  C_Pad360::E_Axis_LeftStick_X,  C_Keyboard::E_Button_Right,	false,  },
};

#endif

#endif




#if defined(PLATFORM_PC)

// [Blur2PC] Retail Blur PC's control setup, ported from Blur.exe 1.2 (SetupAmaxUserControls 0x00415a30 and
// its tables). Retail button and axis numbers are given where they differ from this source's enums; key
// numbers are this source's C_Keyboard enum, which retail uses unchanged. Details: docs/analysis.md,
// "Keyboard layouts (T103)".

static const S32 kNoKey = -1;

enum E_KeyDevice
{
	E_KeyDevice_None = -1,
	E_KeyDevice_Keyboard = 0,
	E_KeyDevice_Mouse,
};

// Game buttons (retail 0x00e64f98, split screen 0x00e65100): {button, key in layout 0 and 1, device of each}.
// Every button gets a combiner of its key and its pad mapping.
struct C_KeyLayoutButton
{
	S32		m_Button;
	S32		m_Key[2];
	S32		m_Device[2];
};

#define KEY(k)	C_Keyboard::E_Button_##k
#define PAD(b)	C_Pad360::E_Button_##b
#define ON_KEYBOARD	{ E_KeyDevice_Keyboard, E_KeyDevice_Keyboard }
#define NOWHERE		{ E_KeyDevice_None, E_KeyDevice_None }

static const C_KeyLayoutButton kKeyLayoutButtons[] =
{
	{ E_Button_Throttle,			{ KEY(Q),			KEY(W)			},	ON_KEYBOARD	},
	{ E_Button_Brake,				{ KEY(A),			KEY(S)			},	ON_KEYBOARD	},
	{ E_Button_Handbrake,			{ KEY(Down),		KEY(Space)		},	ON_KEYBOARD	},
	{ E_Button_ChangeView,			{ KEY(V),			KEY(Z)			},	ON_KEYBOARD	},
	{ E_Button_UsePickup,			{ KEY(RControl),	KEY(RControl)	},	ON_KEYBOARD	},
	{ E_Button_UsePickupForward,	{ KEY(W),			KEY(Up)			},	ON_KEYBOARD	},
	{ E_Button_UsePickupBackward,	{ KEY(S),			KEY(Down)		},	ON_KEYBOARD	},
	{ E_Button_DiscardPickup,		{ KEY(D),			KEY(Right)		},	ON_KEYBOARD	},
	{ E_Button_SwitchPickup,		{ KEY(Up),			KEY(Left)		},	ON_KEYBOARD	},
	{ E_Button_PhotoRoll,			{ KEY(End),			KEY(End)		},	ON_KEYBOARD	},
	{ E_Button_PhotoZoom,			{ KEY(Add),			KEY(Add)		},	ON_KEYBOARD	},
	{ E_Button_PhotoRollReset,		{ KEY(R),			KEY(R)			},	ON_KEYBOARD	},
	{ E_Button_PhotoZoomReset,		{ KEY(Q),			KEY(Q)			},	ON_KEYBOARD	},
	{ E_Button_RearView,			{ KEY(Tab),			KEY(Next)		},	ON_KEYBOARD	},
	{ E_Button_SwitchPickupLeft,	{ kNoKey,			kNoKey			},	NOWHERE		},
	{ E_Button_SwitchPickupAlt,		{ kNoKey,			kNoKey			},	NOWHERE		},
	{ E_Button_UseSecondTierPickup,	{ kNoKey,			kNoKey			},	NOWHERE		},
	{ E_Button_UsePickupAlt,		{ kNoKey,			kNoKey			},	NOWHERE		},
};

static const C_KeyLayoutButton kKeyLayoutButtonsSplitScreen[] =
{
	{ E_Button_Throttle,			{ KEY(W),			KEY(Up)				},	ON_KEYBOARD	},
	{ E_Button_Brake,				{ KEY(S),			KEY(Down)			},	ON_KEYBOARD	},
	{ E_Button_Handbrake,			{ KEY(LMenu),		KEY(RControl)		},	ON_KEYBOARD	},
	{ E_Button_ChangeView,			{ KEY(V),			KEY(Add)			},	ON_KEYBOARD	},
	{ E_Button_UsePickup,			{ KEY(Space),		KEY(NumPadEnter)	},	ON_KEYBOARD	},
	{ E_Button_UsePickupForward,	{ KEY(B),			KEY(NumPad9)		},	ON_KEYBOARD	},
	{ E_Button_UsePickupBackward,	{ KEY(N),			KEY(NumPad6)		},	ON_KEYBOARD	},
	{ E_Button_DiscardPickup,		{ KEY(LControl),	KEY(RShift)			},	ON_KEYBOARD	},
	{ E_Button_SwitchPickup,		{ KEY(F),			KEY(NumPad2)		},	ON_KEYBOARD	},
	{ E_Button_RearView,			{ KEY(LShift),		KEY(NumPad3)		},	ON_KEYBOARD	},
	{ E_Button_PhotoRoll,			{ kNoKey,			kNoKey				},	NOWHERE		},
	{ E_Button_PhotoZoom,			{ kNoKey,			kNoKey				},	NOWHERE		},
	{ E_Button_PhotoRollReset,		{ kNoKey,			kNoKey				},	NOWHERE		},
	{ E_Button_PhotoZoomReset,		{ kNoKey,			kNoKey				},	NOWHERE		},
	{ E_Button_SwitchPickupLeft,	{ kNoKey,			kNoKey				},	NOWHERE		},
	{ E_Button_SwitchPickupAlt,		{ kNoKey,			kNoKey				},	NOWHERE		},
	{ E_Button_UseSecondTierPickup,	{ kNoKey,			kNoKey				},	NOWHERE		},
	{ E_Button_UsePickupAlt,		{ kNoKey,			kNoKey				},	NOWHERE		},
};

// Game axes (retail 0x00e65268, split screen 0x00e65310): {axis, negative and positive key in layout 0 and 1,
// pad axis or negative pad button, positive pad button}. Retail axis numbers: steering 10, digital steering 11,
// pitch 12, photo move up 14.
struct C_KeyLayoutAxis
{
	S32		m_Axis;
	S32		m_KeyNegative[2];
	S32		m_KeyPositive[2];
	S32		m_Pad;
	S32		m_PadPositive;
};

static const C_KeyLayoutAxis kKeyLayoutAxes[] =
{
	{ E_Axis_Steering,			{ KEY(Left), KEY(A) },	{ KEY(Right), KEY(D) },	C_Pad360::E_Axis_LeftStick_X,	kNoKey			},
	{ E_Axis_Steering,			{ kNoKey, kNoKey },		{ kNoKey, kNoKey },		PAD(DPadLeft),					PAD(DPadRight)	},
	{ E_Axis_DigitalSteering,	{ kNoKey, kNoKey },		{ kNoKey, kNoKey },		PAD(DPadLeft),					PAD(DPadRight)	},
	{ E_Axis_Pitch,				{ kNoKey, kNoKey },		{ kNoKey, kNoKey },		C_Pad360::E_Axis_LeftStick_Y,	kNoKey			},
	{ E_Axis_Pitch,				{ kNoKey, kNoKey },		{ kNoKey, kNoKey },		PAD(DPadDown),					PAD(DPadUp)		},
	{ E_Axis_PhotoMoveUp,		{ kNoKey, kNoKey },		{ kNoKey, kNoKey },		PAD(DPadDown),					PAD(DPadUp)		},
};

static const C_KeyLayoutAxis kKeyLayoutAxesSplitScreen[] =
{
	{ E_Axis_Steering,			{ KEY(A), KEY(Left) },	{ KEY(D), KEY(Right) },	C_Pad360::E_Axis_LeftStick_X,	kNoKey			},
	{ E_Axis_Steering,			{ kNoKey, kNoKey },		{ kNoKey, kNoKey },		PAD(DPadLeft),					PAD(DPadRight)	},
	{ E_Axis_DigitalSteering,	{ kNoKey, kNoKey },		{ kNoKey, kNoKey },		PAD(DPadLeft),					PAD(DPadRight)	},
	{ E_Axis_Pitch,				{ kNoKey, kNoKey },		{ kNoKey, kNoKey },		C_Pad360::E_Axis_LeftStick_Y,	kNoKey			},
	{ E_Axis_Pitch,				{ kNoKey, kNoKey },		{ kNoKey, kNoKey },		PAD(DPadDown),					PAD(DPadUp)		},
	{ E_Axis_PhotoMoveUp,		{ kNoKey, kNoKey },		{ kNoKey, kNoKey },		PAD(DPadDown),					PAD(DPadUp)		},
};

// Menu buttons (retail 0x00e653b8): {button, key, split-screen key, pad button}. A button named twice gets both
// rows. Retail also creates inputs for the -1 entries; they are skipped here (see analysis.md).
struct C_KeyLayoutMenuButton
{
	S32		m_Button;
	S32		m_Key[2];
	S32		m_Pad;
};

static const C_KeyLayoutMenuButton kKeyLayoutMenuButtons[] =
{
	{ E_UiButton_Confirm,		{ KEY(Return),		KEY(NumPadEnter)	},	PAD(A)				},
	{ E_UiButton_Confirm,		{ KEY(Space),		kNoKey				},	kNoKey				},
	{ E_UiButton_Cancel,		{ KEY(Escape),		KEY(NumPad0)		},	PAD(B)				},
	{ E_UiButton_Option1,		{ KEY(RShift),		KEY(Subtract)		},	PAD(X)				},
	{ E_UiButton_Option2,		{ KEY(RControl),	KEY(Add)			},	PAD(Y)				},
	{ E_UiButton_LeftTrigger,	{ KEY(Prior),		kNoKey				},	PAD(LeftTrigger)	},
	{ E_UiButton_RightTrigger,	{ KEY(Next),		kNoKey				},	PAD(RightTrigger)	},
	{ E_UiButton_LeftShoulder,	{ KEY(Insert),		KEY(Decimal)		},	PAD(LeftShoulder)	},
	{ E_UiButton_RightShoulder,	{ KEY(Delete),		kNoKey				},	PAD(RightShoulder)	},
	{ E_UiButton_LeftThumb,		{ kNoKey,			kNoKey				},	PAD(LeftThumb)		},
	{ E_UiButton_RightThumb,	{ KEY(Delete),		KEY(M)				},	PAD(RightThumb)		},
	{ E_UiButton_Start,			{ KEY(F1),			kNoKey				},	PAD(Start)			},
	{ E_UiButton_Start,			{ KEY(Escape),		kNoKey				},	kNoKey				},
	{ E_UiButton_Back,			{ KEY(F2),			kNoKey				},	PAD(Back)			},
};

// Pad-or-stick menu directions (retail 0x00e64f38): {button, pad button, pad axis, key, split-screen key, invert}.
struct C_KeyLayoutPadOrAxis
{
	S32		m_Button;
	S32		m_Pad;
	S32		m_Axis;
	S32		m_Key[2];
	bool	m_InvertAxis;
};

static const C_KeyLayoutPadOrAxis kKeyLayoutPadOrAxis[] =
{
	{ E_UiButton_PadOrAxis_Up,		PAD(DPadUp),	C_Pad360::E_Axis_LeftStick_Y,	{ KEY(Up),		KEY(NumPad8) },	false	},
	{ E_UiButton_PadOrAxis_Down,	PAD(DPadDown),	C_Pad360::E_Axis_LeftStick_Y,	{ KEY(Down),	KEY(NumPad2) },	true	},
	{ E_UiButton_PadOrAxis_Left,	PAD(DPadLeft),	C_Pad360::E_Axis_LeftStick_X,	{ KEY(Left),	KEY(NumPad4) },	true	},
	{ E_UiButton_PadOrAxis_Right,	PAD(DPadRight),	C_Pad360::E_Axis_LeftStick_X,	{ KEY(Right),	KEY(NumPad6) },	false	},
};

#undef KEY
#undef PAD
#undef ON_KEYBOARD
#undef NOWHERE

static C_ButtonInput* CreatePadButtonFallback( C_Device* pPad, S32 button )
{
	// retail: the photo buttons keep their pad triggers and thumb buttons when the configuration maps none
	switch (button)
	{
		case E_Button_PhotoRoll:		return new C_ButtonInputFromDevice(pPad, C_Pad360::E_Button_LeftTrigger);
		case E_Button_PhotoZoom:		return new C_ButtonInputFromDevice(pPad, C_Pad360::E_Button_RightTrigger);
		case E_Button_PhotoRollReset:	return new C_ButtonInputFromDevice(pPad, C_Pad360::E_Button_LeftThumb);
		case E_Button_PhotoZoomReset:	return new C_ButtonInputFromDevice(pPad, C_Pad360::E_Button_RightThumb);
	}
	return NULL;
}

void SetupAmaxUserControlsPC( C_GameInput* pGameInput, C_Device* pPad, C_Device* pKeyboard, C_Device* pMouse,
							  S32 keyboardLayout, bool splitScreen, const UI::C_ControlConfig* pPadConfig, S32 menuKeyColumn )
{
	if (!pGameInput)
		return;

	assert(keyboardLayout == 0 || keyboardLayout == 1);
	assert(menuKeyColumn == 0 || menuKeyColumn == 1);

	// Clear the game and menu bindings (retail 0x01f07990: buttons 32-60 and 65-76, axes 10-22)
	for (U32 i = E_Button_Throttle; i < E_Button_Count; i++)
		pGameInput->SetButtonInput(i, NULL);
	for (U32 i = E_UiButton_Confirm; i <= E_UiButton_Back; i++)
		pGameInput->SetButtonInput(i, NULL);
	for (U32 i = E_Axis_Steering; i < E_UiAxis_Count; i++)
		pGameInput->SetAxisInput(i, NULL);

	// Game buttons
	const C_KeyLayoutButton* pButtons = splitScreen ? kKeyLayoutButtonsSplitScreen : kKeyLayoutButtons;
	for (U32 i = 0; i < lengthof(kKeyLayoutButtons); i++)
	{
		const C_KeyLayoutButton& row = pButtons[i];
		C_ButtonInputCombiner* pCombiner = new C_ButtonInputCombiner();

		if (pKeyboard && pMouse && row.m_Key[keyboardLayout] != kNoKey)
		{
			C_Device* pDevice = NULL;
			if (row.m_Device[keyboardLayout] == E_KeyDevice_Keyboard)
				pDevice = pKeyboard;
			else if (row.m_Device[keyboardLayout] == E_KeyDevice_Mouse)
				pDevice = pMouse;
			if (pDevice)
				pCombiner->AddButtonInput(new C_ButtonInputFromDevice(pDevice, row.m_Key[keyboardLayout]));
		}

		if (pPad && pPadConfig)
		{
			C_ButtonInput* pPadInput = pPadConfig->CreateButtonInput(pPad, row.m_Button);
			if (!pPadInput)
				pPadInput = CreatePadButtonFallback(pPad, row.m_Button);
			if (pPadInput)
				pCombiner->AddButtonInput(pPadInput);
		}

		pGameInput->SetButtonInput(row.m_Button, pCombiner);
	}

	// Game axes
	const C_KeyLayoutAxis* pAxes = splitScreen ? kKeyLayoutAxesSplitScreen : kKeyLayoutAxes;
	for (U32 i = 0; i < lengthof(kKeyLayoutAxes); i++)
	{
		const C_KeyLayoutAxis& row = pAxes[i];
		C_AxisInputCombiner* pCombiner = new C_AxisInputCombiner();

		if (pKeyboard && row.m_KeyNegative[keyboardLayout] != kNoKey && row.m_KeyPositive[keyboardLayout] != kNoKey)
		{
			pCombiner->AddAxisInput(new C_AxisInputFromButtons(	new C_ButtonInputFromDevice(pKeyboard, row.m_KeyNegative[keyboardLayout]),
																new C_ButtonInputFromDevice(pKeyboard, row.m_KeyPositive[keyboardLayout])));
		}

		if (pPad)
		{
			if (row.m_PadPositive == kNoKey)
				pCombiner->AddAxisInput(new C_AxisInputFromDevice(pPad, row.m_Pad));
			else
				pCombiner->AddAxisInput(new C_AxisInputFromButtons(	new C_ButtonInputFromDevice(pPad, row.m_Pad),
																	new C_ButtonInputFromDevice(pPad, row.m_PadPositive)));
		}

		if (pGameInput->GetAxisInput(row.m_Axis) == NULL)
			pGameInput->SetAxisInput(row.m_Axis, pCombiner);
		else
			Framework::AddAxisInput(pGameInput, row.m_Axis, pCombiner);
	}

	// Menu buttons
	for (U32 i = 0; i < lengthof(kKeyLayoutMenuButtons); i++)
	{
		const C_KeyLayoutMenuButton& row = kKeyLayoutMenuButtons[i];
		C_ButtonInputCombiner* pCombiner = new C_ButtonInputCombiner();

		if (pKeyboard && row.m_Key[menuKeyColumn] != kNoKey)
			pCombiner->AddButtonInput(new C_ButtonInputFromDevice(pKeyboard, row.m_Key[menuKeyColumn]));
		if (pPad && row.m_Pad != kNoKey)
			pCombiner->AddButtonInput(new C_ButtonInputFromDevice(pPad, row.m_Pad));

		if (pGameInput->GetButtonInput(row.m_Button) == NULL)
			pGameInput->SetButtonInput(row.m_Button, pCombiner);
		else
			Framework::AddButtonInput(pGameInput, row.m_Button, pCombiner);
	}

	// Pad-or-stick menu directions
	for (U32 i = 0; i < lengthof(kKeyLayoutPadOrAxis); i++)
	{
		const C_KeyLayoutPadOrAxis& row = kKeyLayoutPadOrAxis[i];
		C_ButtonInputCombiner* pCombiner = new C_ButtonInputCombiner();

		if (pPad)
		{
			pCombiner->AddButtonInput(new C_ButtonInputFromDevice(pPad, row.m_Pad));
			pCombiner->AddButtonInput(new C_ButtonInputFromAxis(new C_AxisInputFromDevice(pPad, row.m_Axis), row.m_InvertAxis, 0.5f));
		}
		if (pKeyboard)
			pCombiner->AddButtonInput(new C_ButtonInputFromDevice(pKeyboard, row.m_Key[menuKeyColumn]));

		pGameInput->SetButtonInput(row.m_Button, pCombiner);
	}

	// Menu sticks: pad only (retail axes 19-22)
	if (pPad)
	{
		pGameInput->SetAxisInput(E_UiAxis_LeftStick_UD,		new C_AxisInputFromDevice(pPad, C_Pad360::E_Axis_LeftStick_Y));
		pGameInput->SetAxisInput(E_UiAxis_LeftStick_LR,		new C_AxisInputFromDevice(pPad, C_Pad360::E_Axis_LeftStick_X));
		pGameInput->SetAxisInput(E_UiAxis_RightStick_UD,	new C_AxisInputFromDevice(pPad, C_Pad360::E_Axis_RightStick_Y));
		pGameInput->SetAxisInput(E_UiAxis_RightStick_LR,	new C_AxisInputFromDevice(pPad, C_Pad360::E_Axis_RightStick_X));
	}

	// Photo camera axes: A/D, S/W and the arrows, then the pad sticks (retail axes 13, 15, 16, 17)
	if (pKeyboard)
	{
		pGameInput->SetAxisInput(E_Axis_PhotoMoveX,		new C_AxisInputFromButtons(new C_ButtonInputFromDevice(pKeyboard, C_Keyboard::E_Button_A),		new C_ButtonInputFromDevice(pKeyboard, C_Keyboard::E_Button_D)));
		pGameInput->SetAxisInput(E_Axis_PhotoMoveZ,		new C_AxisInputFromButtons(new C_ButtonInputFromDevice(pKeyboard, C_Keyboard::E_Button_S),		new C_ButtonInputFromDevice(pKeyboard, C_Keyboard::E_Button_W)));
		pGameInput->SetAxisInput(E_Axis_PhotoRotateX,	new C_AxisInputFromButtons(new C_ButtonInputFromDevice(pKeyboard, C_Keyboard::E_Button_Left),	new C_ButtonInputFromDevice(pKeyboard, C_Keyboard::E_Button_Right)));
		pGameInput->SetAxisInput(E_Axis_PhotoRotateY,	new C_AxisInputFromButtons(new C_ButtonInputFromDevice(pKeyboard, C_Keyboard::E_Button_Down),	new C_ButtonInputFromDevice(pKeyboard, C_Keyboard::E_Button_Up)));
	}
	if (pPad)
	{
		Framework::AddAxisInput(pGameInput, E_Axis_PhotoMoveX,		new C_AxisInputFromDevice(pPad, C_Pad360::E_Axis_LeftStick_X));
		Framework::AddAxisInput(pGameInput, E_Axis_PhotoMoveZ,		new C_AxisInputFromDevice(pPad, C_Pad360::E_Axis_LeftStick_Y));
		Framework::AddAxisInput(pGameInput, E_Axis_PhotoRotateX,	new C_AxisInputFromDevice(pPad, C_Pad360::E_Axis_RightStick_X));
		Framework::AddAxisInput(pGameInput, E_Axis_PhotoRotateY,	new C_AxisInputFromDevice(pPad, C_Pad360::E_Axis_RightStick_Y));
	}
}

#endif // PLATFORM_PC


void SetupAmaxUserControls( C_GameInput* pGameInput, C_Device* pKeyboard, C_Device* pMouse, C_Device* pPad )
{
	assert(pGameInput);

#if defined(PLATFORM_PC)
	// [Blur2PC] retail Blur PC sets up every user at boot with keyboard layout 0 and no pad game buttons
	// (the application's user-control setup passes zeros, Blur.exe 1.2 0x01f13bc0); the profile's layout and
	// pad configuration are applied later by UI::SetupPlayerControls.
	SetupAmaxUserControlsPC( pGameInput, pPad, pKeyboard, pMouse, 0, false, NULL, 0 );
#else
	#if defined(PLATFORM_PS3)

	pGameInput->SetButtonInput(E_Button_Throttle,		new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_R2));
	pGameInput->SetButtonInput(E_Button_Brake,			new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_L2));
	pGameInput->SetButtonInput(E_Button_Handbrake,		new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_Circle));
	//CA - Amax doesn't currently require gear up/down buttons
	//pGameInput->SetButtonInput(E_Button_GearUp,		new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_Circle));
	//pGameInput->SetButtonInput(E_Button_GearDown,		new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_Cross));
	pGameInput->SetButtonInput(E_Button_ChangeView,		new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_R1));
	pGameInput->SetButtonInput(E_Button_ResetVehicle,	new C_ButtonInputFromDevice(pPad,    C_PadPS3::E_Button_R1));
	pGameInput->SetButtonInput(E_Button_UsePickup,		new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_Cross));
	pGameInput->SetButtonInput(E_Button_DiscardPickup,	new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_Triangle));
	pGameInput->SetButtonInput(E_Button_SwitchPickup,	new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_L1));
	pGameInput->SetButtonInput(E_Button_SwitchPickupAlt, new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_Square));
	pGameInput->SetButtonInput(E_Button_UseShield,		new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_Square));
	
	pGameInput->SetButtonInput(E_Button_UseSecondTierPickup,	new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_Triangle));
	pGameInput->SetButtonInput(E_Button_Auto_Shuffle_Perks,		new C_ButtonInputFromDevice(pPad,    C_PadPS3::E_Button_DPadUp));
	pGameInput->SetButtonInput(E_Button_Convert_To_Health,		new C_ButtonInputFromDevice(pPad,    C_PadPS3::E_Button_DPadLeft));
	pGameInput->SetButtonInput(E_Button_Clone_Selected_Perk,	new C_ButtonInputFromDevice(pPad,    C_PadPS3::E_Button_DPadRight));
	pGameInput->SetButtonInput(E_Button_Fullhouse,				new C_ButtonInputFromDevice(pPad,    C_PadPS3::E_Button_DPadDown));

	//pGameInput->SetButtonInput(E_Button_UsePickupAlt,	new C_ButtonInputFromDevice(pPad,    C_PadPS3::E_Button_R3));

	pGameInput->SetButtonInput(E_Button_PhotoRoll,		new C_ButtonInputFromDevice(pPad,    C_PadPS3::E_Button_L2));
	pGameInput->SetButtonInput(E_Button_PhotoZoom,		new C_ButtonInputFromDevice(pPad,    C_PadPS3::E_Button_R2));
	pGameInput->SetButtonInput(E_Button_PhotoRollReset,	new C_ButtonInputFromDevice(pPad,    C_PadPS3::E_Button_L3));
	pGameInput->SetButtonInput(E_Button_PhotoZoomReset,	new C_ButtonInputFromDevice(pPad,    C_PadPS3::E_Button_R3));

	C_ButtonInput* flyUp   = new C_ButtonInputFromDevice(pPad, C_PadPS3::E_Button_DPadUp);
	C_ButtonInput* flyDown = new C_ButtonInputFromDevice(pPad, C_PadPS3::E_Button_DPadDown);
	pGameInput->SetAxisInput(E_Axis_PhotoMoveUp, new C_AxisInputFromButtons(flyDown, flyUp));

	pGameInput->SetAxisInput(E_Axis_Steering,			new C_AxisInputFromDevice(pPad, 		C_PadPS3::E_Axis_LeftStick_X));
	pGameInput->SetAxisInput(E_Axis_Pitch,				new C_AxisInputFromDevice(pPad, 		C_PadPS3::E_Axis_LeftStick_Y));

	C_ButtonInput * pSteerLeftButton =									new C_ButtonInputFromDevice(pPad, 		C_PadPS3::E_Button_DPadLeft);
	C_ButtonInput * pSteerRightButton =									new C_ButtonInputFromDevice(pPad, 		C_PadPS3::E_Button_DPadRight);
	pGameInput->SetAxisInput(E_Axis_DigitalSteering,					new C_AxisInputFromDigitalButtons(pSteerLeftButton,	pSteerRightButton));

	// Ui buttons
	pGameInput->SetButtonInput(E_UiButton_Pad_Up,		new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_DPadUp));
	pGameInput->SetButtonInput(E_UiButton_Pad_Down,		new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_DPadDown));
	pGameInput->SetButtonInput(E_UiButton_Pad_Left,		new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_DPadLeft));
	pGameInput->SetButtonInput(E_UiButton_Pad_Right,	new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_DPadRight));
	pGameInput->SetButtonInput(E_UiButton_Confirm,		new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_Cross));
	pGameInput->SetButtonInput(E_UiButton_Cancel,		new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_Circle));
	pGameInput->SetButtonInput(E_UiButton_Option1,		new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_Square));
	pGameInput->SetButtonInput(E_UiButton_Option2,		new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_Triangle));
	pGameInput->SetButtonInput(E_UiButton_LeftTrigger,	new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_L2));
	pGameInput->SetButtonInput(E_UiButton_RightTrigger,	new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_R2));
	pGameInput->SetButtonInput(E_UiButton_LeftShoulder,	new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_L1));
	pGameInput->SetButtonInput(E_UiButton_RightShoulder,new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_R1));
	pGameInput->SetButtonInput(E_UiButton_LeftThumb,	new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_L3));
	pGameInput->SetButtonInput(E_UiButton_RightThumb,	new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_R3));
	pGameInput->SetButtonInput(E_UiButton_Start,		new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_Start));
	pGameInput->SetButtonInput(E_UiButton_Back,			new C_ButtonInputFromDevice(pPad, 	C_PadPS3::E_Button_Select));
	// Ui Axis
	pGameInput->SetAxisInput(E_UiAxis_LeftStick_UD,		new C_AxisInputFromDevice(pPad, 		C_PadPS3::E_Axis_LeftStick_Y));
	pGameInput->SetAxisInput(E_UiAxis_LeftStick_LR,		new C_AxisInputFromDevice(pPad, 		C_PadPS3::E_Axis_LeftStick_X));
	pGameInput->SetAxisInput(E_UiAxis_RightStick_UD,	new C_AxisInputFromDevice(pPad, 		C_PadPS3::E_Axis_RightStick_Y));
	pGameInput->SetAxisInput(E_UiAxis_RightStick_LR,	new C_AxisInputFromDevice(pPad, 		C_PadPS3::E_Axis_RightStick_X));

	#endif

	// Combine input from DPad and axis together according to kPadOrAxisSetup
	for ( U32 i = 0; i < lengthof( kPadOrAxisSetup ); i++ )
	{
		const C_PadOrAxisSetup& setup = kPadOrAxisSetup[i];
		
		// create combiner button
		C_ButtonInputCombiner* pButton = new C_ButtonInputCombiner();

		// add pad button
		pButton->AddButtonInput(new C_ButtonInputFromDevice( pPad, setup.m_Pad ));

		// add axis button
		pButton->AddButtonInput(new C_ButtonInputFromAxis(new C_AxisInputFromDevice( pPad, setup.m_Axis), setup.m_InvertAxis, 0.5f));

#if defined (PLATFORM_PC)
		// add key if keyboard present
		if(pKeyboard)
		{
			pButton->AddButtonInput(new C_ButtonInputFromDevice( pKeyboard, setup.m_Key ));
		}
#endif

		// set game input
		pGameInput->SetButtonInput( setup.m_PadOrAxis, pButton );
	}
#endif // PLATFORM_PC
}

} // namespace Game
