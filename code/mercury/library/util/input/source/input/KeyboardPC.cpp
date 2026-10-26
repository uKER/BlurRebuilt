#include "stdafx.h"
#include <system/debug/Blur2PCAutomation.h>	// [Blur2PC]

#include <input\DeviceManagerPC.h>
#include <input\KeyboardPC.h>
#include <DXErr.h>

namespace Input
{

const U32 ConvertToDirectInputKey( const C_KeyboardPC::E_Button button )
{
	switch(button)
	{
		default:
		assert(0 && "Unknown Key");
		break;
		
		case C_KeyboardPC::E_Button_Escape:			return DIK_ESCAPE;
		case C_KeyboardPC::E_Button_1:				return DIK_1;
		case C_KeyboardPC::E_Button_2:				return DIK_2;
		case C_KeyboardPC::E_Button_3:				return DIK_3;
		case C_KeyboardPC::E_Button_4:				return DIK_4;
		case C_KeyboardPC::E_Button_5:				return DIK_5;
		case C_KeyboardPC::E_Button_6:				return DIK_6;
		case C_KeyboardPC::E_Button_7:				return DIK_7;
		case C_KeyboardPC::E_Button_8:				return DIK_8;
		case C_KeyboardPC::E_Button_9:				return DIK_9;
		case C_KeyboardPC::E_Button_0:				return DIK_0;
		case C_KeyboardPC::E_Button_Minus:			return DIK_MINUS;
		case C_KeyboardPC::E_Button_Equals:			return DIK_EQUALS;
		case C_KeyboardPC::E_Button_Back:			return DIK_BACK;
		case C_KeyboardPC::E_Button_Tab:			return DIK_TAB;
		case C_KeyboardPC::E_Button_Q:				return DIK_Q;
		case C_KeyboardPC::E_Button_W:				return DIK_W;
		case C_KeyboardPC::E_Button_E:				return DIK_E;
		case C_KeyboardPC::E_Button_R:				return DIK_R;
		case C_KeyboardPC::E_Button_T:				return DIK_T;
		case C_KeyboardPC::E_Button_Y:				return DIK_Y;
		case C_KeyboardPC::E_Button_U:				return DIK_U;
		case C_KeyboardPC::E_Button_I:				return DIK_I;
		case C_KeyboardPC::E_Button_O:				return DIK_O;
		case C_KeyboardPC::E_Button_P:				return DIK_P;
		case C_KeyboardPC::E_Button_LBracket:		return DIK_LBRACKET;
		case C_KeyboardPC::E_Button_RBracket:		return DIK_RBRACKET;
		case C_KeyboardPC::E_Button_Return:			return DIK_RETURN;
		case C_KeyboardPC::E_Button_LControl:		return DIK_LCONTROL;
		case C_KeyboardPC::E_Button_A: 				return DIK_A;
		case C_KeyboardPC::E_Button_S: 				return DIK_S;
		case C_KeyboardPC::E_Button_D: 				return DIK_D;
		case C_KeyboardPC::E_Button_F: 				return DIK_F;
		case C_KeyboardPC::E_Button_G: 				return DIK_G;
		case C_KeyboardPC::E_Button_H: 				return DIK_H;
		case C_KeyboardPC::E_Button_J: 				return DIK_J;
		case C_KeyboardPC::E_Button_K: 				return DIK_K;
		case C_KeyboardPC::E_Button_L: 				return DIK_L;
		case C_KeyboardPC::E_Button_SemiColon:		return DIK_SEMICOLON;
		case C_KeyboardPC::E_Button_Apostrophe:		return DIK_APOSTROPHE;
		case C_KeyboardPC::E_Button_Grave:			return DIK_GRAVE;
		case C_KeyboardPC::E_Button_LShift:			return DIK_LSHIFT;
		case C_KeyboardPC::E_Button_Backslash:		return DIK_BACKSLASH;
		case C_KeyboardPC::E_Button_Z:				return DIK_Z;
		case C_KeyboardPC::E_Button_X:				return DIK_X;
		case C_KeyboardPC::E_Button_C:				return DIK_C;
		case C_KeyboardPC::E_Button_V:				return DIK_V;
		case C_KeyboardPC::E_Button_B:				return DIK_B;
		case C_KeyboardPC::E_Button_N:				return DIK_N;
		case C_KeyboardPC::E_Button_M:				return DIK_M;
		case C_KeyboardPC::E_Button_Comma:			return DIK_COMMA;
		case C_KeyboardPC::E_Button_Period:			return DIK_PERIOD;
		case C_KeyboardPC::E_Button_Slash:			return DIK_SLASH;
		case C_KeyboardPC::E_Button_RShift:			return DIK_RSHIFT;
		case C_KeyboardPC::E_Button_Multiply:		return DIK_MULTIPLY;
		case C_KeyboardPC::E_Button_LMenu:			return DIK_LMENU;
		case C_KeyboardPC::E_Button_Space:			return DIK_SPACE;
		case C_KeyboardPC::E_Button_Capital:		return DIK_CAPITAL;
		case C_KeyboardPC::E_Button_F1: 			return DIK_F1;
		case C_KeyboardPC::E_Button_F2: 			return DIK_F2;
		case C_KeyboardPC::E_Button_F3: 			return DIK_F3;
		case C_KeyboardPC::E_Button_F4: 			return DIK_F4;
		case C_KeyboardPC::E_Button_F5: 			return DIK_F5;
		case C_KeyboardPC::E_Button_F6: 			return DIK_F6;
		case C_KeyboardPC::E_Button_F7: 			return DIK_F7;
		case C_KeyboardPC::E_Button_F8: 			return DIK_F8;
		case C_KeyboardPC::E_Button_F9: 			return DIK_F9;
		case C_KeyboardPC::E_Button_F10:			return DIK_F10;
		case C_KeyboardPC::E_Button_NumLock:		return DIK_NUMLOCK;
		case C_KeyboardPC::E_Button_Scroll:			return DIK_SCROLL;
		case C_KeyboardPC::E_Button_NumPad7:		return DIK_NUMPAD7;
		case C_KeyboardPC::E_Button_NumPad8:		return DIK_NUMPAD8;
		case C_KeyboardPC::E_Button_NumPad9:		return DIK_NUMPAD9;
		case C_KeyboardPC::E_Button_Subtract:		return DIK_SUBTRACT;
		case C_KeyboardPC::E_Button_NumPad4:		return DIK_NUMPAD4;
		case C_KeyboardPC::E_Button_NumPad5:		return DIK_NUMPAD5;
		case C_KeyboardPC::E_Button_NumPad6:		return DIK_NUMPAD6;
		case C_KeyboardPC::E_Button_Add:			return DIK_ADD;
		case C_KeyboardPC::E_Button_NumPad1:		return DIK_NUMPAD1;
		case C_KeyboardPC::E_Button_NumPad2:		return DIK_NUMPAD2;
		case C_KeyboardPC::E_Button_NumPad3:		return DIK_NUMPAD3;
		case C_KeyboardPC::E_Button_NumPad0:		return DIK_NUMPAD0;
		case C_KeyboardPC::E_Button_Decimal:		return DIK_DECIMAL;
		case C_KeyboardPC::E_Button_Oem102:			return DIK_OEM_102;
		case C_KeyboardPC::E_Button_F11:			return DIK_F11;
		case C_KeyboardPC::E_Button_F12:			return DIK_F12;
		case C_KeyboardPC::E_Button_F13:    		return DIK_F13;
		case C_KeyboardPC::E_Button_F14:    		return DIK_F14;
		case C_KeyboardPC::E_Button_F15:			return DIK_F15;
		case C_KeyboardPC::E_Button_Kana:			return DIK_KANA;
		case C_KeyboardPC::E_Button_AbntC1:			return DIK_ABNT_C1;
		case C_KeyboardPC::E_Button_Convert:		return DIK_CONVERT;
		case C_KeyboardPC::E_Button_NoConvert:		return DIK_NOCONVERT;      	
		case C_KeyboardPC::E_Button_Yen:			return DIK_YEN;             
		case C_KeyboardPC::E_Button_AbntC2:			return DIK_ABNT_C2;         
		case C_KeyboardPC::E_Button_NumPadEquals:	return DIK_NUMPADEQUALS;    
		case C_KeyboardPC::E_Button_PrevTrack:		return DIK_PREVTRACK;       	
		case C_KeyboardPC::E_Button_At:				return DIK_AT;              
		case C_KeyboardPC::E_Button_Colon:			return DIK_COLON;           	
		case C_KeyboardPC::E_Button_Underline:		return DIK_UNDERLINE;       	
		case C_KeyboardPC::E_Button_Kanji:			return DIK_KANJI;          	
		case C_KeyboardPC::E_Button_Stop:			return DIK_STOP;            
		case C_KeyboardPC::E_Button_Ax:				return DIK_AX;              
		case C_KeyboardPC::E_Button_Unlabeled:		return DIK_UNLABELED;       	
		case C_KeyboardPC::E_Button_NextTrack:		return DIK_NEXTTRACK;       	
		case C_KeyboardPC::E_Button_NumPadEnter:	return DIK_NUMPADENTER;     
		case C_KeyboardPC::E_Button_RControl:		return DIK_RCONTROL;        
		case C_KeyboardPC::E_Button_Mute:			return DIK_MUTE;            
		case C_KeyboardPC::E_Button_Calculator:		return DIK_CALCULATOR;      
		case C_KeyboardPC::E_Button_PlayPause:		return DIK_PLAYPAUSE;       	
		case C_KeyboardPC::E_Button_MediaStop:		return DIK_MEDIASTOP;       	
		case C_KeyboardPC::E_Button_VolumeDown:		return DIK_VOLUMEDOWN;      
		case C_KeyboardPC::E_Button_VolumeUp:		return DIK_VOLUMEUP;        
		case C_KeyboardPC::E_Button_WebHome:		return DIK_WEBHOME;         
		case C_KeyboardPC::E_Button_NumPadComma:	return DIK_NUMPADCOMMA;     
		case C_KeyboardPC::E_Button_Divide:			return DIK_DIVIDE;          
		case C_KeyboardPC::E_Button_SysRq:			return DIK_SYSRQ;           
		case C_KeyboardPC::E_Button_RMenu:    		return DIK_RMENU;           	
		case C_KeyboardPC::E_Button_Pause:    		return DIK_PAUSE;           	
		case C_KeyboardPC::E_Button_Home:			return DIK_HOME;
		case C_KeyboardPC::E_Button_Up:				return DIK_UP;             
		case C_KeyboardPC::E_Button_Prior:			return DIK_PRIOR;           	
		case C_KeyboardPC::E_Button_Left:			return DIK_LEFT;            
		case C_KeyboardPC::E_Button_Right:			return DIK_RIGHT;           	
		case C_KeyboardPC::E_Button_End:			return DIK_END;             
		case C_KeyboardPC::E_Button_Down:			return DIK_DOWN;            
		case C_KeyboardPC::E_Button_Next:			return DIK_NEXT;            
		case C_KeyboardPC::E_Button_Insert:			return DIK_INSERT;          
		case C_KeyboardPC::E_Button_Delete:		    return DIK_DELETE;          
		case C_KeyboardPC::E_Button_LWin:			return DIK_LWIN;            
		case C_KeyboardPC::E_Button_RWin:			return DIK_RWIN;            
		case C_KeyboardPC::E_Button_Apps:			return DIK_APPS;            
		case C_KeyboardPC::E_Button_Power:			return DIK_POWER;           	
		case C_KeyboardPC::E_Button_Sleep:			return DIK_SLEEP;           	
		case C_KeyboardPC::E_Button_Wake:			return DIK_WAKE;            
		case C_KeyboardPC::E_Button_WebSearch:		return DIK_WEBSEARCH;       	
		case C_KeyboardPC::E_Button_WebFavorites:	return DIK_WEBFAVORITES;    
		case C_KeyboardPC::E_Button_WebRefresh:	    return DIK_WEBREFRESH;     
		case C_KeyboardPC::E_Button_WebStop:		return DIK_WEBSTOP;         
		case C_KeyboardPC::E_Button_WebForward:		return DIK_WEBFORWARD;
		case C_KeyboardPC::E_Button_WebBack:		return DIK_WEBBACK;     
		case C_KeyboardPC::E_Button_MyComputer:		return DIK_MYCOMPUTER;
		case C_KeyboardPC::E_Button_Mail:			return DIK_MAIL;    
		case C_KeyboardPC::E_Button_MediaSelect:	return DIK_MEDIASELECT;
	}

	return 0;
}

const S32 ConvertFromDirectInputKey( const U32 dik )
{
	switch(dik)
	{
		default:
		return -1;
		break;
		
		case DIK_ESCAPE:		return 	C_KeyboardPC::E_Button_Escape;
		case DIK_1:				return 	C_KeyboardPC::E_Button_1;				
		case DIK_2:				return 	C_KeyboardPC::E_Button_2;				
		case DIK_3:				return 	C_KeyboardPC::E_Button_3;				
		case DIK_4:				return 	C_KeyboardPC::E_Button_4;				
		case DIK_5:				return 	C_KeyboardPC::E_Button_5;				
		case DIK_6:				return 	C_KeyboardPC::E_Button_6;				
		case DIK_7:				return 	C_KeyboardPC::E_Button_7;				
		case DIK_8:				return 	C_KeyboardPC::E_Button_8;				
		case DIK_9:				return 	C_KeyboardPC::E_Button_9;				
		case DIK_0:				return 	C_KeyboardPC::E_Button_0;				
		case DIK_MINUS:			return 	C_KeyboardPC::E_Button_Minus;		
		case DIK_EQUALS:		return 	C_KeyboardPC::E_Button_Equals;	
		case DIK_BACK:			return 	C_KeyboardPC::E_Button_Back;		
		case DIK_TAB:			return 	C_KeyboardPC::E_Button_Tab;			
		case DIK_Q:				return 	C_KeyboardPC::E_Button_Q;				
		case DIK_W:				return 	C_KeyboardPC::E_Button_W;				
		case DIK_E:				return 	C_KeyboardPC::E_Button_E;				
		case DIK_R:				return 	C_KeyboardPC::E_Button_R;				
		case DIK_T:				return 	C_KeyboardPC::E_Button_T;				
		case DIK_Y:				return 	C_KeyboardPC::E_Button_Y;				
		case DIK_U:				return 	C_KeyboardPC::E_Button_U;				
		case DIK_I:				return 	C_KeyboardPC::E_Button_I;				
		case DIK_O:				return 	C_KeyboardPC::E_Button_O;				
		case DIK_P:				return 	C_KeyboardPC::E_Button_P;				
		case DIK_LBRACKET:		return 	C_KeyboardPC::E_Button_LBracket;
		case DIK_RBRACKET:		return 	C_KeyboardPC::E_Button_RBracket;
		case DIK_RETURN:		return 	C_KeyboardPC::E_Button_Return;	
		case DIK_LCONTROL:		return 	C_KeyboardPC::E_Button_LControl;
		case DIK_A:				return 	C_KeyboardPC::E_Button_A; 				
		case DIK_S:				return 	C_KeyboardPC::E_Button_S; 				
		case DIK_D:				return 	C_KeyboardPC::E_Button_D; 				
		case DIK_F:				return 	C_KeyboardPC::E_Button_F; 				
		case DIK_G:				return 	C_KeyboardPC::E_Button_G; 				
		case DIK_H:				return 	C_KeyboardPC::E_Button_H; 				
		case DIK_J:				return 	C_KeyboardPC::E_Button_J; 				
		case DIK_K:				return 	C_KeyboardPC::E_Button_K; 				
		case DIK_L:				return 	C_KeyboardPC::E_Button_L; 				
		case DIK_SEMICOLON:		return 	C_KeyboardPC::E_Button_SemiColon;		
		case DIK_APOSTROPHE:	return 	C_KeyboardPC::E_Button_Apostrophe;	
		case DIK_GRAVE:			return 	C_KeyboardPC::E_Button_Grave;			
		case DIK_LSHIFT:		return 	C_KeyboardPC::E_Button_LShift;		
		case DIK_BACKSLASH:		return 	C_KeyboardPC::E_Button_Backslash;	
		case DIK_Z:				return	C_KeyboardPC::E_Button_Z;				 
		case DIK_X:				return	C_KeyboardPC::E_Button_X;				 
		case DIK_C:				return	C_KeyboardPC::E_Button_C;				 
		case DIK_V:				return	C_KeyboardPC::E_Button_V;				 
		case DIK_B:				return	C_KeyboardPC::E_Button_B;				 
		case DIK_N:				return	C_KeyboardPC::E_Button_N;				 
		case DIK_M:				return	C_KeyboardPC::E_Button_M;				 
		case DIK_COMMA:			return 	C_KeyboardPC::E_Button_Comma;			
		case DIK_PERIOD:		return	C_KeyboardPC::E_Button_Period;		 
		case DIK_SLASH:			return 	C_KeyboardPC::E_Button_Slash;			
		case DIK_RSHIFT:		return	C_KeyboardPC::E_Button_RShift;		 
		case DIK_MULTIPLY:		return	C_KeyboardPC::E_Button_Multiply;	 
		case DIK_LMENU:			return 	C_KeyboardPC::E_Button_LMenu;			
		case DIK_SPACE:			return 	C_KeyboardPC::E_Button_Space;			
		case DIK_CAPITAL:		return 	C_KeyboardPC::E_Button_Capital;		
		case DIK_F1:			return 	C_KeyboardPC::E_Button_F1; 			
		case DIK_F2:			return 	C_KeyboardPC::E_Button_F2; 			
		case DIK_F3:			return 	C_KeyboardPC::E_Button_F3; 			
		case DIK_F4:			return 	C_KeyboardPC::E_Button_F4; 			
		case DIK_F5:			return 	C_KeyboardPC::E_Button_F5; 			
		case DIK_F6:			return 	C_KeyboardPC::E_Button_F6; 			
		case DIK_F7:			return 	C_KeyboardPC::E_Button_F7; 			
		case DIK_F8:			return 	C_KeyboardPC::E_Button_F8; 			
		case DIK_F9:			return 	C_KeyboardPC::E_Button_F9; 			
		case DIK_F10:			return 	C_KeyboardPC::E_Button_F10;			
		case DIK_NUMLOCK:		return 	C_KeyboardPC::E_Button_NumLock;	
		case DIK_SCROLL:		return 	C_KeyboardPC::E_Button_Scroll;	
		case DIK_NUMPAD7:		return 	C_KeyboardPC::E_Button_NumPad7;	
		case DIK_NUMPAD8:		return 	C_KeyboardPC::E_Button_NumPad8;	
		case DIK_NUMPAD9:		return 	C_KeyboardPC::E_Button_NumPad9;	
		case DIK_SUBTRACT:		return 	C_KeyboardPC::E_Button_Subtract;
		case DIK_NUMPAD4:		return 	C_KeyboardPC::E_Button_NumPad4;	
		case DIK_NUMPAD5:		return 	C_KeyboardPC::E_Button_NumPad5;	
		case DIK_NUMPAD6:		return 	C_KeyboardPC::E_Button_NumPad6;	
		case DIK_ADD:			return 	C_KeyboardPC::E_Button_Add;			
		case DIK_NUMPAD1:		return 	C_KeyboardPC::E_Button_NumPad1;		
		case DIK_NUMPAD2:		return 	C_KeyboardPC::E_Button_NumPad2;		
		case DIK_NUMPAD3:		return 	C_KeyboardPC::E_Button_NumPad3;		
		case DIK_NUMPAD0:		return 	C_KeyboardPC::E_Button_NumPad0;		
		case DIK_DECIMAL:		return 	C_KeyboardPC::E_Button_Decimal;		
		case DIK_OEM_102:		return 	C_KeyboardPC::E_Button_Oem102;		
		case DIK_F11:			return 	C_KeyboardPC::E_Button_F11;			
		case DIK_F12:			return 	C_KeyboardPC::E_Button_F12;			
		case DIK_F13:			return 	C_KeyboardPC::E_Button_F13;    		
		case DIK_F14:			return 	C_KeyboardPC::E_Button_F14;    		
		case DIK_F15:			return 	C_KeyboardPC::E_Button_F15;			
		case DIK_KANA:			return 	C_KeyboardPC::E_Button_Kana;		
		case DIK_ABNT_C1:		return	C_KeyboardPC::E_Button_AbntC1;	 
		case DIK_CONVERT:		return	C_KeyboardPC::E_Button_Convert;	
		case DIK_NOCONVERT:     return	C_KeyboardPC::E_Button_NoConvert;	
		case DIK_YEN:           return	C_KeyboardPC::E_Button_Yen;			
		case DIK_ABNT_C2:       return	C_KeyboardPC::E_Button_AbntC2;		
		case DIK_NUMPADEQUALS:  return	C_KeyboardPC::E_Button_NumPadEquals;
		case DIK_PREVTRACK:     return	C_KeyboardPC::E_Button_PrevTrack;		
		case DIK_AT:            return	C_KeyboardPC::E_Button_At;			
		case DIK_COLON:         return	C_KeyboardPC::E_Button_Colon;			
		case DIK_UNDERLINE:     return	C_KeyboardPC::E_Button_Underline;		
		case DIK_KANJI:         return	C_KeyboardPC::E_Button_Kanji;		
		case DIK_STOP:          return	C_KeyboardPC::E_Button_Stop;		
		case DIK_AX:            return	C_KeyboardPC::E_Button_Ax;			
		case DIK_UNLABELED:     return	C_KeyboardPC::E_Button_Unlabeled;		
		case DIK_NEXTTRACK:     return	C_KeyboardPC::E_Button_NextTrack;		
		case DIK_NUMPADENTER:   return	C_KeyboardPC::E_Button_NumPadEnter;	
		case DIK_RCONTROL:      return	C_KeyboardPC::E_Button_RControl;	
		case DIK_MUTE:          return	C_KeyboardPC::E_Button_Mute;		
		case DIK_CALCULATOR:    return	C_KeyboardPC::E_Button_Calculator;	
		case DIK_PLAYPAUSE:     return	C_KeyboardPC::E_Button_PlayPause;		
		case DIK_MEDIASTOP:     return	C_KeyboardPC::E_Button_MediaStop;		
		case DIK_VOLUMEDOWN:    return	C_KeyboardPC::E_Button_VolumeDown;	
		case DIK_VOLUMEUP:      return	C_KeyboardPC::E_Button_VolumeUp;	
		case DIK_WEBHOME:       return	C_KeyboardPC::E_Button_WebHome;		
		case DIK_NUMPADCOMMA:   return	C_KeyboardPC::E_Button_NumPadComma;	
		case DIK_DIVIDE:        return	C_KeyboardPC::E_Button_Divide;		
		case DIK_SYSRQ:         return	C_KeyboardPC::E_Button_SysRq;		
		case DIK_RMENU:         return	C_KeyboardPC::E_Button_RMenu;    		
		case DIK_PAUSE:         return	C_KeyboardPC::E_Button_Pause;    		
		case DIK_HOME:			return 	C_KeyboardPC::E_Button_Home;			
		case DIK_UP:            return	C_KeyboardPC::E_Button_Up;			
		case DIK_PRIOR:         return	C_KeyboardPC::E_Button_Prior;		 	
		case DIK_LEFT:          return	C_KeyboardPC::E_Button_Left;		 
		case DIK_RIGHT:         return	C_KeyboardPC::E_Button_Right;		 	
		case DIK_END:           return	C_KeyboardPC::E_Button_End;			
		case DIK_DOWN:          return	C_KeyboardPC::E_Button_Down;		 
		case DIK_NEXT:          return	C_KeyboardPC::E_Button_Next;		 
		case DIK_INSERT:        return	C_KeyboardPC::E_Button_Insert;		 
		case DIK_DELETE:        return	C_KeyboardPC::E_Button_Delete;		 
		case DIK_LWIN:          return	C_KeyboardPC::E_Button_LWin;		 
		case DIK_RWIN:          return	C_KeyboardPC::E_Button_RWin;		 
		case DIK_APPS:          return	C_KeyboardPC::E_Button_Apps;		 
		case DIK_POWER:         return	C_KeyboardPC::E_Button_Power;		 	
		case DIK_SLEEP:         return	C_KeyboardPC::E_Button_Sleep;		 	
		case DIK_WAKE:          return	C_KeyboardPC::E_Button_Wake;		 
		case DIK_WEBSEARCH:     return	C_KeyboardPC::E_Button_WebSearch;	 	
		case DIK_WEBFAVORITES:  return	C_KeyboardPC::E_Button_WebFavorites; 
		case DIK_WEBREFRESH:    return	C_KeyboardPC::E_Button_WebRefresh;	
		case DIK_WEBSTOP:       return	C_KeyboardPC::E_Button_WebStop;	  
		case DIK_WEBFORWARD:	return	C_KeyboardPC::E_Button_WebForward;	 
		case DIK_WEBBACK:     	return	C_KeyboardPC::E_Button_WebBack;		
		case DIK_MYCOMPUTER:	return	C_KeyboardPC::E_Button_MyComputer;	 
		case DIK_MAIL:    		return	C_KeyboardPC::E_Button_Mail;			
		case DIK_MEDIASELECT:	return	C_KeyboardPC::E_Button_MediaSelect;	
	}

	return -1;
}


C_KeyboardPC::C_KeyboardPC( C_DeviceManager* pDeviceManager, const S32 userIndex )
	: C_Keyboard( pDeviceManager, userIndex )
    , m_Acquired(false)
	, m_pDirectInputKeyboard(NULL)
{
	m_DeviceType = E_Device_Keyboard;	// [Blur2PC] retail 0x008451a0
	C_DeviceManagerPC* pDeviceManagerPC = (C_DeviceManagerPC*)pDeviceManager;
	_IDirectInput* pDirectInput = pDeviceManagerPC->GetDirectInput();
	HWND hWnd = pDeviceManagerPC->GetWindowHandle();

	if(pDirectInput == NULL || hWnd == NULL)
	{
		LOGFATAL( g_Log, DBSTR "InputDeviceManager not initialised properly\n");		
	}

    // create the DirectInput keyboard device
    HRESULT hr;
    hr = pDirectInput->CreateDevice(GUID_SysKeyboard, &m_pDirectInputKeyboard, NULL); 

    if(FAILED(hr))
    {
        LOGFATAL( g_Log, DBSTR "Failed to create DirectInput keyboard device\n");
    }

    hr = m_pDirectInputKeyboard->SetDataFormat(&c_dfDIKeyboard); 

    if(FAILED(hr))
    { 
        LOGFATAL( g_Log, DBSTR "Failed to set data format for DirectInput keyboard device\n");
    }

    hr = m_pDirectInputKeyboard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE); 

    if(FAILED(hr)) 
    { 
        LOGFATAL( g_Log, DBSTR "Failed to set cooperative level for DirectInput keyboard device\n");
    } 
}

C_KeyboardPC::~C_KeyboardPC()
{
    HRESULT hr;

    if(m_Connected)
    {
        hr = m_pDirectInputKeyboard->Unacquire(); 
        if(FAILED(hr))
        {
            LOGWARN( g_Log, DBSTR "Device unacquire failed\n");
        }
		m_Connected = false;
    }

    m_pDirectInputKeyboard->Release();
    m_pDirectInputKeyboard = NULL;
}


void C_KeyboardPC::Update()
{
    HRESULT hr;
	U8 deviceState[256];

    m_Acquired = false;
	m_Connected = false;

    if (m_pDirectInputKeyboard == NULL) 
    {
        LOGWARN( g_Log, DBSTR "Device not initialised\n");
        return;
    }
    hr = m_pDirectInputKeyboard->GetDeviceState(256 * sizeof(U8), deviceState);
    if(FAILED(hr))
    {        
        switch(hr)
        {
            case DIERR_NOTACQUIRED:
            case DIERR_INPUTLOST:
				while((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
				{
					hr = m_pDirectInputKeyboard->Acquire();
				}
				if(FAILED(hr))
				{
					//LOGWARN( g_Log, DXGetErrorString(hr));
				}
				else
				{
					m_Acquired = true;
				}
			break;
            default:
				LOGWARN( g_Log, DXGetErrorString(hr));
				BIZ_ASSERT(0);
            break;
        }
        memset(deviceState, 0, 256 * sizeof(U8));
    }
	else
	{
		m_Acquired = true;
		m_Connected = true;

		const S32 buttonCount = m_ButtonValues.Size();
		for(S32 i=0; i<buttonCount; i++)
		{
			m_ButtonValues[i] = 0.0f;
		}

		for(S32 i=0; i<256; i++)
		{
			const S32 buttonIndex = ConvertFromDirectInputKey(i);
			if(buttonIndex >= 0)	// [Blur2PC] was > 0, which dropped Escape (index 0); retail Blur.exe 0x00845360 tests >= 0
			{
				if(deviceState[i] != 0)
				{
					m_ButtonValues[buttonIndex] = 1.0f;
				}
				else
				{
					m_ButtonValues[buttonIndex] = 0.0f;
				}
			}
		}
	}

#if defined(PLATFORM_PC)
	// [Blur2PC] scripted input for unattended runs (see system/debug/Blur2PCAutomation.h). Works without
	// window focus: when DirectInput has no data (window in the background) the real keys read as released.
	if (Blur2PCAutomation::Update())
	{
		m_Connected = true;
		if (FAILED(hr))
		{
			for (S32 i = 0; i < m_ButtonValues.Size(); i++)
				m_ButtonValues[i] = 0.0f;
		}
		for (S32 i = 0; i < 256; i++)
		{
			if (!Blur2PCAutomation::IsKeyDown(i))
				continue;
			const S32 buttonIndex = ConvertFromDirectInputKey(i);
			if (buttonIndex >= 0)
				m_ButtonValues[buttonIndex] = 1.0f;
		}
	}
#endif
}

} // namespace Input
