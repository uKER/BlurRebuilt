#ifndef BUTTONMESSAGES_H
#define BUTTONMESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/MessageBase.h>
#include <misc_shared/uid/FourCC.h>
#include <ui/uidefines.h>
#include <Ui/Game/Gadgets/ButtonController.h>

namespace UI
{

enum E_UiMsg
{
	E_UiMsg_Start = FOURCC_HI20('UIMT'), // UI Message Type

	// Gadget & screen control messages
	E_UiMsg_Init,
	E_UiMsg_BindTimelines,
	E_UiMsg_PostInit,
	E_UiMsg_StartFrame,
	E_UiMsg_FrameUpdate,
	E_UiMsg_UpdateEffects,
	E_UiMsg_SetTimelineActive,
	E_UiMsg_Render,
	E_UiMsg_EnterEnd,
	E_UiMsg_Exit,
	E_UiMsg_EnableControllerInput,
	E_UiMsg_LuaCommand,
	E_UiMsg_PlaySoundEffect,

	E_UiMsg_CtrlStart,

	// Controller button
	E_UiMsg_CtrlButton_Up,
	E_UiMsg_CtrlButton_Down,
	E_UiMsg_CtrlButton_Left,
	E_UiMsg_CtrlButton_Right,
	E_UiMsg_CtrlButton_A,
	E_UiMsg_CtrlButton_B,
	E_UiMsg_CtrlButton_X,
	E_UiMsg_CtrlButton_Y,
	E_UiMsg_CtrlButton_Start,
	E_UiMsg_CtrlButton_Back,
	E_UiMsg_CtrlButton_RightTrigger,
	E_UiMsg_CtrlButton_LeftTrigger,
	E_UiMsg_CtrlButton_LeftShoulder,
	E_UiMsg_CtrlButton_RightShoulder,
	E_UiMsg_CtrlButton_LeftThumb,
	E_UiMsg_CtrlButton_RightThumb,
	// Stick
	E_UiMsg_CtrlLeftStick_X,
	E_UiMsg_CtrlLeftStick_Y,
	E_UiMsg_CtrlRightStick_X,
	E_UiMsg_CtrlRightStick_Y,
	// Ui mouse commands
	E_UiMsg_CtrlMouse_Move_X,
	E_UiMsg_CtrlMouse_Move_Y,
	E_UiMsg_CtrlMouse_Wheel,
	E_UiMsg_CtrlMouse_Pressed,
	E_UiMsg_CtrlMouse_DebOn,
	E_UiMsg_CtrlMouse_DebOff,
	E_UiMsg_CtrlMouse_Click,
	
	// Misc
	E_UiMsg_MenuBack,
	E_UiMsg_MenuNext,

	E_UiMsg_CtrlEnd,	// don't use this one

	// More Misc	
	E_UiMsg_ChangeScreen,
	E_UiMsg_TextPrintFinished,
	E_UiMsg_ControllerDisconnected,
	E_UiMsg_ProfileChanged,

	E_UiMsg_MovieFinished,

	E_UiMsg_KeyboardFinished,
	E_UiMsg_KeyboardCancelled,

	E_UiMsg_SaveLocationFound,
	E_UiMsg_SaveLocationNotFound,
	E_UiMsg_SaveEnumerationFinished,

	E_UiMsg_NodeListTableFixedUpdated,

	// for PC mouse support
	E_UiMsg_VerticalMenuChanged,
	E_UiMsg_MouseInBox,
	E_UiMsg_MouseOutBox,

	E_UiMsg_AutomationLoadGame,

	// Popup close result
	E_UiMsg_PopupNext,
	E_UiMsg_PopupBack,
	E_UiMsg_PopupEnd,

	E_UiMsg_CreditsFinished,
	E_UiMsg_LegalFinished,

	E_UiMsg_Count
};

class C_UiMessageData : public Messaging::C_MessageBase
{
public:
	C_UiMessageData( Messaging::TMsgType TypeID, U32 messageSize ) : Messaging::C_MessageBase( TypeID, messageSize ), m_NumArgs( 0 ) { }

	// Had to be static as we can only send data in messages
	static const U32 SendToLua( const C_UiMessageData* pMsg, lua_State* pState )
	{
		for( U32 i = 0; i < pMsg->m_NumArgs; i++ )
			lua_pushnumber( pState, (lua_Number)pMsg->m_Args[ i ] );

		return pMsg->m_NumArgs;
	}

	// TODO - Stick in .cpp
	void		AddParam( const bool t )	{ AddParamImpl<bool>( t ); }
	void		AddParam( const S32 t )		{ AddParamImpl<S32>( t ); }
	void		AddParam( const U32 t )		{ AddParamImpl<U32>( t ); }
	void		AddParam( const F32 t )		{ AddParamImpl<F32>( t ); }
	void		AddParam( const F64 t )		{ AddParamImpl<F64>( t ); }
private:
	template< typename T >
	void		AddParamImpl( T _type )
	{
		if( m_NumArgs >= 4 )
			return;

		m_Args[ m_NumArgs ] = (F64)_type;
		m_NumArgs++;
	}

public:
	U8		m_NumArgs;
	F64		m_Args[4];
};

// Helper for creating derived Ui messages
template <class MessageClass, Messaging::TMsgType TypeID >
class C_UiMessageDerived : public C_UiMessageData
{
public:
	static Messaging::TMsgType StaticType() {return TypeID;} ///< @returns type identifier for this message class.

protected:
	C_UiMessageDerived() : C_UiMessageData(TypeID, sizeof(MessageClass)) {}

};

// Default message : just contains space for the lua params
template <Messaging::TMsgType TypeID>
class C_UiMessageBase : public C_UiMessageDerived< C_UiMessageBase<TypeID>, TypeID> { };

// Ui message interface
// Timeline message
class C_UiTimelineMessage : public C_UiMessageDerived< C_UiTimelineMessage, E_UiMsg_SetTimelineActive >
{
public:
	C_UiTimelineMessage( const char* name, const bool set, const F32 fTime = -1.0f, const bool reset = false ) :
		m_Set( set ), m_fTime( fTime ), m_Reset( reset )
	{
		STRCPY( m_Name, lengthof( m_Name ), name );
	}

public:
	char		m_Name[48];
	bool		m_Set;
	bool		m_Reset;
	F32			m_fTime;
};

// Lua command message
class C_UiLuaCommandMessage : public C_UiMessageDerived< C_UiLuaCommandMessage, E_UiMsg_LuaCommand >
{
public:
	C_UiLuaCommandMessage( const char* command )
	{
		if( strlen( command ) >= lengthof( m_Command ) )
		{
			LOGERROR( g_Log, DBSTR " command buffer exceeded for %s", command );
			return;
		}

		STRCPY( m_Command, lengthof( m_Command ), command );
	}

public:
	char		m_Command[128];
};

class C_UiSoundEffectMessage : public C_UiMessageDerived< C_UiSoundEffectMessage, E_UiMsg_PlaySoundEffect >
{
public:
	C_UiSoundEffectMessage( const S32 sIndex, const F32 fValue1 = 0.0f, const F32 fValue2 = 0.0f )
		: m_EffectNum( sIndex ), m_Value1( fValue1 ), m_Value2( fValue2 ) {  }

public:
	S32	m_EffectNum;
	F32 m_Value1;
	F32 m_Value2;
};

class C_UiEnableControllerMessage : public C_UiMessageDerived< C_UiEnableControllerMessage, E_UiMsg_EnableControllerInput >
{
public:
	C_UiEnableControllerMessage( bool enable, bool reset_count = false ) : m_Enable( enable ), m_ResetCount( reset_count ) { }
public:
	bool	m_Enable;
	bool	m_ResetCount;
};

template <Messaging::TMsgType TypeID>
class C_UiMessagePad : public C_UiMessageDerived< C_UiMessagePad<TypeID>, TypeID >
{
public:
	C_UiMessagePad( const S8 _pad, const bool _deb, const F32 _amount, const F64 _time )
	{
		pad = _pad;
		deb = _deb;
		amount = _amount;
		time = _time;
	}

	static const U32 SendToLua( const C_UiMessagePad* pMsg, lua_State* pState )
	{
		lua_pushnumber( pState, (lua_Number) pMsg->pad );		// Pad index
		lua_pushboolean( pState, (int) pMsg->deb );				// Debounce?
		lua_pushnumber( pState, (lua_Number) pMsg->amount );	// Analogue value
		lua_pushnumber( pState, (lua_Number) pMsg->time );		// Time held

		return 4;
	}

public:
	S8		pad;
	bool	deb;
	F32		amount;
	F64		time;
};

typedef C_UiMessagePad<E_UiMsg_CtrlStart> C_UiMessagePadData;

}; // namespace UI

// macros for sending UI messages

// Pad message
#define _UI_MESSAGE_PAD( _TYPE, _PAD, _DEB, _AMOUNT, _TIME, _MASK )			\
	do																		\
	{																		\
		UI::C_UiMessagePad< _TYPE > message( _PAD, _DEB, _AMOUNT, _TIME );	\
		UI::gButtons.BroadcastMessage( &message, _MASK );					\
	} while( 0 )

// No params
#define _UI_MESSAGE_MASK( _TYPE, _MASK )									\
	do																		\
	{																		\
		UI::C_UiMessageBase< _TYPE > message;								\
		UI::gButtons.BroadcastMessage( &message, _MASK );					\
	} while( 0 )

#define _UI_MESSAGE( _TYPE )												\
	_UI_MESSAGE_MASK( _TYPE, 0xffffffff )

// One param
#define _UI_MESSAGE1_MASK( _TYPE, _ARG0, _MASK )							\
	do																		\
	{																		\
		UI::C_UiMessageBase< _TYPE > message;								\
		message.AddParam( _ARG0 );											\
		UI::gButtons.BroadcastMessage( &message, _MASK );					\
	} while( 0 )

#define _UI_MESSAGE1( _TYPE, _ARG0 )										\
	_UI_MESSAGE1_MASK( _TYPE, _ARG0, 0xffffffff )

// Two params																
#define _UI_MESSAGE2_MASK( _TYPE, _ARG0, _ARG1, _MASK )						\
	do																		\
	{																		\
		UI::C_UiMessageBase< _TYPE > message;								\
		message.AddParam( _ARG0 );											\
		message.AddParam( _ARG1 );											\
		UI::gButtons.BroadcastMessage( &message, _MASK );					\
	} while( 0 )

#define _UI_MESSAGE2( _TYPE, _ARG0, _ARG1 )									\
	_UI_MESSAGE2_MASK( _TYPE, _ARG0, _ARG1, 0xffffffff )					
																			
// Three params																
#define _UI_MESSAGE3_MASK( _TYPE, _ARG0, _ARG1, _ARG2, _MASK )				\
	do																		\
	{																		\
		UI::C_UiMessageBase< _TYPE > message;								\
		message.AddParam( _ARG0 );											\
		message.AddParam( _ARG1 );											\
		message.AddParam( _ARG2 );											\
		UI::gButtons.BroadcastMessage( &message, _MASK );					\
	} while( 0 )

#define _UI_MESSAGE3( _TYPE, _ARG0, _ARG1, _ARG2 )							\
	_UI_MESSAGE3_MASK( _TYPE, _ARG0, _ARG1, _ARG2, 0xffffffff )

// Four params
#define _UI_MESSAGE4_MASK( _TYPE, _ARG0, _ARG1, _ARG2, _ARG3, _MASK )		\
	do																		\
	{																		\
		UI::C_UiMessageBase< _TYPE > message;								\
		message.AddParam( _ARG0 );											\
		message.AddParam( _ARG1 );											\
		message.AddParam( _ARG2 );											\
		message.AddParam( _ARG3 );											\
		UI::gButtons.BroadcastMessage( &message, _MASK );					\
	} while( 0 )

#define _UI_MESSAGE4( _TYPE, _ARG0, _ARG1, _ARG2, _ARG3 )										\
		_UI_MESSAGE4_MASK( _TYPE, (F64)_ARG0, (F64)_ARG1, (F64)_ARG2, (F64)_ARG3, 0xffffffff )

#define _UI_MESSAGE_FULL( _MESSAGE )															\
	UI::gButtons.BroadcastMessage( &_MESSAGE );

// macros for registering the debug info for UI messages

#ifndef CFG_GOLD

#define _UI_MESSAGE_DEBUG( msg ) \
	Messaging::RegisterDebugInfo( UI::E_UiMsg_##msg, "E_UiMsg_"#msg ); \
	uNumMessagesRegistered++;

#define _UI_GAME_MESSAGE_DEBUG( msg ) \
	Messaging::RegisterDebugInfo( UI::E_UiGameMsg_##msg, "E_UiGameMsg_"#msg ); \
	uNumMessagesRegistered++;

#endif

//

#endif // BUTTONMESSAGES_H

