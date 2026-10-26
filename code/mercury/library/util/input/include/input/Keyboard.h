#ifndef INPUT_KEYBOARD_H
#define INPUT_KEYBOARD_H

#ifdef _MSC_VER
#pragma once
#endif

#include <input\Input.h>
#include <input\Device.h>

namespace Input
{

//! @addtogroup Input
//! @{

//! Base class for keyboard devices
class C_Keyboard : public C_Device
{
public:
	//! The available keyboard buttons
	enum E_Button
	{
		E_Button_Escape = 0,
		E_Button_1, 
		E_Button_2, 
		E_Button_3, 
		E_Button_4, 
		E_Button_5, 
		E_Button_6, 
		E_Button_7, 
		E_Button_8, 
		E_Button_9, 
		E_Button_0, 
		E_Button_Minus,				/* - on main keyboard */
		E_Button_Equals,
		E_Button_Back,				/* backspace */
		E_Button_Tab, 
		E_Button_Q, 
		E_Button_W, 
		E_Button_E, 
		E_Button_R, 
		E_Button_T, 
		E_Button_Y, 
		E_Button_U, 
		E_Button_I, 
		E_Button_O, 
		E_Button_P, 
		E_Button_LBracket, 
		E_Button_RBracket, 
		E_Button_Return,			/* Enter on main keyboard */
		E_Button_LControl, 
		E_Button_A, 	
		E_Button_S, 	
		E_Button_D, 	
		E_Button_F, 	
		E_Button_G, 	
		E_Button_H, 	
		E_Button_J, 	
		E_Button_K, 	
		E_Button_L, 	
		E_Button_SemiColon, 
		E_Button_Apostrophe,
		E_Button_Grave,				/* accent grave */
		E_Button_LShift,
		E_Button_Backslash,	
		E_Button_Z, 
		E_Button_X, 
		E_Button_C, 
		E_Button_V, 
		E_Button_B, 
		E_Button_N, 
		E_Button_M, 
		E_Button_Comma,	
		E_Button_Period,			/* . on main keyboard */
		E_Button_Slash,				/* / on main keyboard */
		E_Button_RShift,	
		E_Button_Multiply,			/* * on numeric keypad */
		E_Button_LMenu,				/* left Alt */
		E_Button_Space, 
		E_Button_Capital,	
		E_Button_F1, 	
		E_Button_F2, 	
		E_Button_F3, 	
		E_Button_F4, 	
		E_Button_F5, 	
		E_Button_F6, 	
		E_Button_F7, 	
		E_Button_F8, 	
		E_Button_F9, 	
		E_Button_F10,	
		E_Button_NumLock,	
		E_Button_Scroll,			/* Scroll Lock */
		E_Button_NumPad7,
		E_Button_NumPad8,
		E_Button_NumPad9,
		E_Button_Subtract,			/* - on numeric keypad */
		E_Button_NumPad4, 
		E_Button_NumPad5, 
		E_Button_NumPad6, 
		E_Button_Add,				/* + on numeric keypad */
		E_Button_NumPad1,
		E_Button_NumPad2,
		E_Button_NumPad3,
		E_Button_NumPad0,
		E_Button_Decimal,			/* . on numeric keypad */
		E_Button_Oem102,			/* <> or \| on RT 102-key keyboard (Non-U.S.) */
		E_Button_F11, 
		E_Button_F12, 
		E_Button_F13,    			/*                     (NEC PC98) */
		E_Button_F14,    			/*                     (NEC PC98) */
		E_Button_F15,				/*                     (NEC PC98) */
		E_Button_Kana,				/* (Japanese keyboard)            */
		E_Button_AbntC1,			/* /? on Brazilian keyboard */
		E_Button_Convert,			/* (Japanese keyboard)            */
		E_Button_NoConvert,			/* (Japanese keyboard)            */
		E_Button_Yen,				/* (Japanese keyboard)            */
		E_Button_AbntC2,			/* Numpad . on Brazilian keyboard */
		E_Button_NumPadEquals,	    /* = on numeric keypad (NEC PC98) */
		E_Button_PrevTrack,			/* Previous Track (DIK_CIRCUMFLEX on Japanese keyboard) */
		E_Button_At,				/*                     (NEC PC98) */
		E_Button_Colon,				/*                     (NEC PC98) */
		E_Button_Underline,			/*                     (NEC PC98) */
		E_Button_Kanji,				/* (Japanese keyboard)            */
		E_Button_Stop,				/*                     (NEC PC98) */
		E_Button_Ax,				/*                     (Japan AX) */
		E_Button_Unlabeled,			/*                        (J3100) */
		E_Button_NextTrack,			/* Next Track */
		E_Button_NumPadEnter,	    /* Enter on numeric keypad */
		E_Button_RControl,	
		E_Button_Mute,				/* Mute */
		E_Button_Calculator,		/* Calculator */
		E_Button_PlayPause,			/* Play / Pause */
		E_Button_MediaStop,			/* Media Stop */
		E_Button_VolumeDown,		/* Volume - */
		E_Button_VolumeUp,			/* Volume + */
		E_Button_WebHome,			/* Web home */
		E_Button_NumPadComma,		/* , on numeric keypad (NEC PC98) */
		E_Button_Divide,			/* / on numeric keypad */
		E_Button_SysRq, 
		E_Button_RMenu,    			/* right Alt */
		E_Button_Pause,    			/* Pause */
		E_Button_Home,				/* Home on arrow keypad */
		E_Button_Up,				/* UpArrow on arrow keypad */
		E_Button_Prior,				/* PgUp on arrow keypad */
		E_Button_Left,				/* LeftArrow on arrow keypad */
		E_Button_Right,				/* RightArrow on arrow keypad */
		E_Button_End,				/* End on arrow keypad */
		E_Button_Down,				/* DownArrow on arrow keypad */
		E_Button_Next,				/* PgDn on arrow keypad */
		E_Button_Insert,			/* Insert on arrow keypad */
		E_Button_Delete,		    /* Delete on arrow keypad */
		E_Button_LWin,				/* Left Windows key */
		E_Button_RWin,				/* Right Windows key */
		E_Button_Apps,				/* AppMenu key */
		E_Button_Power,				/* System Power */
		E_Button_Sleep,				/* System Sleep */
		E_Button_Wake,				/* System Wake */
		E_Button_WebSearch,			/* Web Search */
		E_Button_WebFavorites,		/* Web Favorites */
		E_Button_WebRefresh,	    /* Web Refresh */
		E_Button_WebStop,			/* Web Stop */
		E_Button_WebForward,		/* Web Forward */
		E_Button_WebBack,			/* Web Back */
		E_Button_MyComputer,		/* My Computer */
		E_Button_Mail,				/* Mail */
		E_Button_MediaSelect,		/* Media Select */

		E_Button_Count,

		// Alternate names for keys

		E_Button_Backspace = E_Button_Back,			/* backspace */
		E_Button_NumPadStar = E_Button_Multiply,	/* * on numeric keypad */
		E_Button_LAlt = E_Button_LMenu,				/* left Alt */
		E_Button_CapsLock = E_Button_Capital,		/* CapsLock */
		E_Button_NumPadMinus = E_Button_Subtract,	/* - on numeric keypad */
		E_Button_NumPadPlus = E_Button_Add,			/* + on numeric keypad */
		E_Button_NumPadPeriod = E_Button_Decimal,	/* . on numeric keypad */
		E_Button_NumPadSlash = E_Button_Divide,		/* / on numeric keypad */
		E_Button_Ralt = E_Button_RMenu,				/* right Alt */
		E_Button_UpArrow = E_Button_Up,				/* UpArrow on arrow keypad */
		E_Button_PgUp = E_Button_Prior,				/* PgUp on arrow keypad */
		E_Button_LeftArrow = E_Button_Left,			/* LeftArrow on arrow keypad */
		E_Button_RightArrow = E_Button_Right,		/* RightArrow on arrow keypad */
		E_Button_DownArrow = E_Button_Down,			/* DownArrow on arrow keypad */
		E_Button_PgDn = E_Button_Next,				/* PgDn on arrow keypad */
	};
	
	enum E_Axis
	{
		E_Axis_Count = 0
	};

	enum E_RumbleMotor
	{
		E_RumbleMotor_Count = 0
	};

public:
	C_Keyboard( C_DeviceManager* pDeviceManager, const S32 userIndex )
		: C_Device(pDeviceManager, userIndex, E_Button_Count, E_Axis_Count, E_RumbleMotor_Count)
	{
	}

	virtual ~C_Keyboard()
	{
	}

	//! Inherited from C_Device
	virtual void Update() = 0;
};

//! @}

} // namespace Input

#endif
