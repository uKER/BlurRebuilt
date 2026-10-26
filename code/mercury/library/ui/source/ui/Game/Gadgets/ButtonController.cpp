#include "stdafx.h"

#include <misc_shared/Messaging/Messaging.h>

#include "ui/game/UILua.h"
#include "ui/game/UIXt.h"

// Gadgets
#include "ui/Game/Gadgets/BaseUIVariableSelect.h"
#include "ui/Game/Gadgets/UIStaticGraphic.h"
#include "ui/Game/Gadgets/UIStaticLetter.h"
#include "ui/Game/Gadgets/UIStaticText.h"
#include "ui/Game/Gadgets/UIStaticScene.h"
#include "ui/Game/Gadgets/UIStaticShape.h"
#include "ui/Game/Gadgets/UIPath.h"
#include "ui/Game/Gadgets/UIBox.h"
#include "ui/Game/Gadgets/UIVerticalMenu.h"
#include "ui/Game/Gadgets/UIVariableSpinner.h"
#include "ui/Game/Gadgets/UIHorizontalMenu.h"
#include "ui/Game/Gadgets/UICredits.h"
#include "ui/Game/Gadgets/UILegal.h"
#include "ui/Game/Gadgets/UIParticleEmitter.h"
#include "ui/Game/Gadgets/UISlider.h"
#include "ui/Game/Gadgets/UILeaderboard.h"
#include "ui/Game/Gadgets/UICamera.h"
#include "ui/Game/Gadgets/UIItemSpinner.h"
#include "ui/Game/Gadgets/UIScrollBar.h"
#include "ui/Game/Gadgets/UIFriendsList.h"
#include "ui/Game/Gadgets/UIAdobeBox.h"
#include "ui/Game/Gadgets/UIAdobeTrail.h"
#include "ui/Game/Gadgets/UIAdobeAnimation.h"
#include "ui/Game/Gadgets/UIKeyboard.h"
#include "ui/Game/Gadgets/UINodeList.h"
#include "ui/Game/Gadgets/UIGraph.h"
#include "ui/Game/Gadgets/UIProgressBar.h"
#include "ui/Game/Gadgets/UITable.h"

#include "ui/Game/UIDataFinder.h"
#include "ui/Game/Gadgets/ButtonMessages.h"
#include "ui/Graphics/Font/Font.h"
#include "ui/Graphics/Font/FontStyle.h"
#include "ui/Graphics/SimpleTimer.h"

#include "ui/Game/Gadgets/ButtonController.h"

#include "ui/Game/Profiles/ProfileInterface.h"

#ifdef GAME_RACERNOTAMAX
#include "ui/Game/UIScreenControl.h"
#endif

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

XT_TYPE( UI::E_UiMsg, "UIMessage" )
	XT_OPTION( UI::E_UiMsg_FrameUpdate,				"FrameUpdate" )
	XT_OPTION( UI::E_UiMsg_Render,					"Render" )

	XT_OPTION( UI::E_UiMsg_CtrlStart,				"CtrlStart" )

	XT_OPTION( UI::E_UiMsg_CtrlButton_Up,			"ButtonUp" )
	XT_OPTION( UI::E_UiMsg_CtrlButton_Down,			"ButtonDown" )
	XT_OPTION( UI::E_UiMsg_CtrlButton_Left,			"ButtonLeft" )
	XT_OPTION( UI::E_UiMsg_CtrlButton_Right,		"ButtonRight" )
	XT_OPTION( UI::E_UiMsg_CtrlButton_A,			"ButtonA" )
	XT_OPTION( UI::E_UiMsg_CtrlButton_B,			"ButtonB" )
	XT_OPTION( UI::E_UiMsg_CtrlButton_X,			"ButtonX" )
	XT_OPTION( UI::E_UiMsg_CtrlButton_Y,			"ButtonY" )
	XT_OPTION( UI::E_UiMsg_CtrlButton_Start,		"ButtonStart" )
	XT_OPTION( UI::E_UiMsg_CtrlButton_Back,			"ButtonBack" )
	XT_OPTION( UI::E_UiMsg_CtrlButton_RightTrigger,	"ButtonRightTrigger" )
	XT_OPTION( UI::E_UiMsg_CtrlButton_LeftTrigger,	"ButtonLeftTrigger" )
	XT_OPTION( UI::E_UiMsg_CtrlButton_LeftShoulder,	"ButtonLeftShoulder" )
	XT_OPTION( UI::E_UiMsg_CtrlButton_RightShoulder,"ButtonRightShoulder" )
	XT_OPTION( UI::E_UiMsg_CtrlButton_LeftThumb,	"LeftThumb" )
	XT_OPTION( UI::E_UiMsg_CtrlButton_RightThumb,	"RightThumb" )

	XT_OPTION( UI::E_UiMsg_CtrlLeftStick_X,			"StickLeftX" )
	XT_OPTION( UI::E_UiMsg_CtrlLeftStick_Y,			"StickLeftY" )
	XT_OPTION( UI::E_UiMsg_CtrlRightStick_X,		"StickRightX" )
	XT_OPTION( UI::E_UiMsg_CtrlRightStick_Y,		"StickRightY" )

	XT_OPTION( UI::E_UiMsg_CtrlMouse_Move_X,		"MouseMoveX" )
	XT_OPTION( UI::E_UiMsg_CtrlMouse_Move_Y,		"MouseMoveY" )
	XT_OPTION( UI::E_UiMsg_CtrlMouse_Wheel,			"MouseMoveWheel" )
	XT_OPTION( UI::E_UiMsg_CtrlMouse_Pressed,		"MousePressed" )
	XT_OPTION( UI::E_UiMsg_CtrlMouse_DebOn,			"MouseDebOn" )
	XT_OPTION( UI::E_UiMsg_CtrlMouse_DebOff,		"MouseDebOff" )
	XT_OPTION( UI::E_UiMsg_CtrlMouse_Click,			"MouseClick" )

	XT_OPTION( UI::E_UiMsg_MenuNext,				"MenuNext" )
	XT_OPTION( UI::E_UiMsg_MenuBack,				"MenuBack" )

	XT_OPTION( UI::E_UiMsg_CtrlEnd,					"CtrlEnd" )

	XT_OPTION( UI::E_UiMsg_ControllerDisconnected,	"ControllerDisconnected" )
	XT_OPTION( UI::E_UiMsg_ProfileChanged,			"ProfileChanged" )

	XT_OPTION( UI::E_UiMsg_MovieFinished,			"MovieFinished" )
	XT_OPTION( UI::E_UiMsg_ChangeScreen,			"ChangeScreen" )
	XT_OPTION( UI::E_UiMsg_TextPrintFinished,		"TextPrintFinished" )

	XT_OPTION( UI::E_UiMsg_KeyboardFinished,		"KeyboardFinished" )
	XT_OPTION( UI::E_UiMsg_KeyboardCancelled,		"KeyboardCancelled" )

	XT_OPTION( UI::E_UiMsg_SaveLocationFound,		"SaveLocationFound" )
	XT_OPTION( UI::E_UiMsg_SaveLocationNotFound,	"SaveLocationNotFound" )
	XT_OPTION( UI::E_UiMsg_SaveEnumerationFinished,	"SaveEnumerationFinished" )

	XT_OPTION( UI::E_UiMsg_NodeListTableFixedUpdated,	"NodeListTableFixedUpdated" )	

	XT_OPTION( UI::E_UiMsg_VerticalMenuChanged,		"VerticalMenuChanged" )
	XT_OPTION( UI::E_UiMsg_MouseInBox,				"MouseInBox" )
	XT_OPTION( UI::E_UiMsg_MouseOutBox,				"MouseOutBox" )

	XT_OPTION( UI::E_UiMsg_AutomationLoadGame,		"AutomationLoadGame" )

	XT_OPTION( UI::E_UiMsg_PopupNext,				"PopupNext" )
	XT_OPTION( UI::E_UiMsg_PopupBack,				"PopupBack" )
	XT_OPTION( UI::E_UiMsg_PopupEnd,				"PopupEnd" )

	XT_OPTION( UI::E_UiMsg_LegalFinished,			"LegalFinished" )
	XT_OPTION( UI::E_UiMsg_CreditsFinished,			"CreditsFinished" )
XT_END_TYPE

namespace UI
{

C_ButtonController gButtons;

C_ButtonController::C_ButtonController( ) : m_pMessageSubscriberSystem( new Messaging::C_MessageSubscriberSystem() )
{
	m_pCurrentDeferredMessageQueue=&m_DeferredMessageQueue1;
}

C_ButtonController::~C_ButtonController()
{
	delete m_pMessageSubscriberSystem;
}

void C_ButtonController::InitCreator( const U32 maxButtonTypes, const bool createDefaults )
{
	m_ButtonCreator.Resize( maxButtonTypes );

	if(createDefaults)
	{
		if( maxButtonTypes < E_UIButtonType_EndMarker )
		{
			LOGFATAL( g_Log,  "maxButtonTypes exceeds E_UIButtonType_EndMarker when trying to create default button types");
			return;
		}
		else
		{
			// Register default types
			Register<C_GadgetBase>();
			Register<UISafeArea>();
			Register<UIRenderState>();
			Register<UIRenderBranch>();
			Register<UIDummyLerp>();
			Register<UIStaticGraphic>();
			Register<UIStaticLetter>();
			Register<UIStaticText>();
			Register<UIStaticScene>();
			Register<UIStaticShape>();
			Register<UIPath>();
			Register<UIStaticBox>();
			Register<UIVerticalMenu>();
			Register<UIVariableSpinner>();
			Register<UIHorizontalMenu>();
			Register<UISlider>();
			Register<UIParticleEmitter>();
			Register<UICredits>();
			Register<UILegal>();
			Register<UICamera>();
			Register<UIItemSpinner>();
			Register<UIScrollBar>();
			Register<UIFriendsList>();
			Register<UIAdobeBox>();
			Register<UIAdobeTrail>();
			Register<UIAdobeAnimation>();
			Register<UIKeyboard>();
			Register<UINodeList>();
			Register<UINode>();
			Register<UITable>();
			Register<UIGraph>();
			Register<UIProgressBar>();
		}
	}
}

void C_ButtonController::ExitCreator( )
{
	for(S32 i = 0; i < m_ButtonCreator.Capacity(); i++)
		m_ButtonCreator[i].m_Creator = NULL;
}

void C_ButtonController::CreateLuaEnums( lua_State *pState )
{
	lua_getglobal(pState, LUA_ENUM_TABLE_NAME);

	if(lua_isnil(pState,-1))
	{
		LOGFATAL( g_Log,  "%s table not found", LUA_ENUM_TABLE_NAME );
		lua_pop(pState,1);
		return;
	}

	// Add the registered buttons
	lua_pushstring(pState, "ButtonTypes");
	lua_newtable(pState);
		const U32 capacity = m_ButtonCreator.Capacity();
		for(U32 i = 0; i < capacity; i++)
			Lua::AddDataMember(pState,m_ButtonCreator[i].m_Name,i);
	lua_rawset(pState,-3);

	// Add effect types
	lua_pushstring(pState, "Effect");
	lua_newtable(pState);
		Lua::AddDataMember(pState,"MOVE",				EFFECT_TYPE_MOVE);
		Lua::AddDataMember(pState,"MOVEX",				EFFECT_TYPE_MOVEX);
		Lua::AddDataMember(pState,"MOVEY",				EFFECT_TYPE_MOVEY);
		Lua::AddDataMember(pState,"MOVEZ",				EFFECT_TYPE_MOVEZ);
		Lua::AddDataMember(pState,"COLOUR",				EFFECT_TYPE_COLOUR);
		Lua::AddDataMember(pState,"ALPHA",				EFFECT_TYPE_ALPHA);
		Lua::AddDataMember(pState,"SIZE",				EFFECT_TYPE_SIZE);
		Lua::AddDataMember(pState,"SIZEX",				EFFECT_TYPE_SIZEX);
		Lua::AddDataMember(pState,"SIZEY",				EFFECT_TYPE_SIZEY);
		Lua::AddDataMember(pState,"SCALE",				EFFECT_TYPE_SCALE);
		Lua::AddDataMember(pState,"SCALEX",				EFFECT_TYPE_SCALEX);
		Lua::AddDataMember(pState,"SCALEY",				EFFECT_TYPE_SCALEY);
		Lua::AddDataMember(pState,"TEXTUREMOVE0",		EFFECT_TYPE_TEXTUREMOVE0);
		Lua::AddDataMember(pState,"TEXTUREMOVE1",		EFFECT_TYPE_TEXTUREMOVE1);
		Lua::AddDataMember(pState,"TEXTUREMOVE2",		EFFECT_TYPE_TEXTUREMOVE2);
		Lua::AddDataMember(pState,"TEXTURESIZE0",		EFFECT_TYPE_TEXTURESIZE0);
		Lua::AddDataMember(pState,"ROTATE",				EFFECT_TYPE_ROTATE);
		Lua::AddDataMember(pState,"ANIMATE",			EFFECT_TYPE_ANIMATE);
		Lua::AddDataMember(pState,"COLOUR_TL",			EFFECT_TYPE_COLOUR0);
		Lua::AddDataMember(pState,"COLOUR_TR",			EFFECT_TYPE_COLOUR1);
		Lua::AddDataMember(pState,"COLOUR_BL",			EFFECT_TYPE_COLOUR2);
		Lua::AddDataMember(pState,"COLOUR_BR",			EFFECT_TYPE_COLOUR3);
	lua_rawset(pState,-3);

	// Add effect times
	lua_pushstring(pState, "EffectTime");
	lua_newtable(pState);
		Lua::AddDataMember(pState,"START",				EFFECT_WHEN_START);
		Lua::AddDataMember(pState,"END",				EFFECT_WHEN_END);
		Lua::AddDataMember(pState,"MAINLOOP",			EFFECT_WHEN_MAINLOOP);
		Lua::AddDataMember(pState,"VARIABLE",			EFFECT_WHEN_VARIABLE);
		Lua::AddDataMember(pState,"SELECTED",			EFFECT_WHEN_SELECTED);
		Lua::AddDataMember(pState,"LOCKED",				EFFECT_WHEN_LOCKED);
		Lua::AddDataMember(pState,"LOCKED_AND_SELECTED",EFFECT_WHEN_LOCKED_AND_SELECTED);
		Lua::AddDataMember(pState,"SB_TOP",				EFFECT_WHEN_SB_TOP);
		Lua::AddDataMember(pState,"SB_YOU",				EFFECT_WHEN_SB_YOU);
	lua_rawset(pState,-3);

	// Timer do type times
	lua_pushstring(pState, "TimerType");
	lua_newtable(pState);
		Lua::AddDataMember(pState,"WRAP",				UITimer::E_Type_Wrap);
		Lua::AddDataMember(pState,"BOUNCE",				UITimer::E_Type_Bounce);
		Lua::AddDataMember(pState,"STOP",				UITimer::E_Type_Stop);
	lua_rawset(pState,-3);

	// Group types
	lua_pushstring(pState, "GroupType");
	lua_newtable(pState);
		Lua::AddDataMember(pState,"OFFSET",				UI_GROUP_TYPES_OFFSET);
		Lua::AddDataMember(pState,"PIVOT_OFFSET",		UI_GROUP_TYPES_PIVOT_OFFSET);
		Lua::AddDataMember(pState,"SIZE",				UI_GROUP_TYPES_SIZE);
		Lua::AddDataMember(pState,"SCALE",				UI_GROUP_TYPES_SCALE);
		Lua::AddDataMember(pState,"ROTATE",				UI_GROUP_TYPES_ROTATE);
	lua_rawset(pState,-3);

	// Scoreboard column types
	lua_pushstring(pState, LUA_COLUMNTYPE_NAME);
	lua_newtable(pState);
		Lua::AddDataMember(pState,"STRING",				SB_COLUMN_TYPE_STRING);
		Lua::AddDataMember(pState,"POS",				SB_COLUMN_TYPE_POS);
		Lua::AddDataMember(pState,"NUMBER",				SB_COLUMN_TYPE_NUMBER);
		Lua::AddDataMember(pState,"WIDE_STRING",		SB_COLUMN_TYPE_WIDE_STRING);
		Lua::AddDataMember(pState,"TIME",				SB_COLUMN_TYPE_TIME);
		Lua::AddDataMember(pState,"TIME_AND_DATE",		SB_COLUMN_TYPE_TIME_AND_DATE);
		Lua::AddDataMember(pState,"CASH",				SB_COLUMN_TYPE_CASH);
	lua_rawset(pState,-3);

	// Text and graphic justification
	UiXt::CreateLuaTableFromEnumType( pState, "UIJustify", "Justify" );

	// Display stencil operations
	UiXt::CreateLuaTableFromEnumType( pState, "DisplayStencilOp", "StencilOp" );

	// Display comparison functions
	UiXt::CreateLuaTableFromEnumType( pState, "DisplayCompareFunction", "CompareFunction" );
 	
	// Blend modes
	UiXt::CreateLuaTableFromEnumType( pState, "UIBlendMode", "BlendMode" );

	// Effects (shader pairs)
	UiXt::CreateLuaTableFromEnumType( pState, "UIEffect", "EffectIndex" );

	// Timeline timer types 
	UiXt::CreateLuaTableFromEnumType( pState, "UITimerType", "TimerType" );
	UiXt::CreateLuaTableFromEnumType( pState, "UITimerRestType", "TimerRestType" );

	// Font fixed space modes
	UiXt::CreateLuaTableFromEnumType( pState, "UIFixedSpaceType", "FixedSpaceType" );

	// Font texture mapping modes
	lua_pushstring( pState, "FontMapping" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "AsLetter",			C_FontStyleTextureInfo::E_FontMappingMode_AsLetter );
		Lua::AddDataMember( pState, "ZeroToOne",		C_FontStyleTextureInfo::E_FontMappingMode_ZeroToOne );
		Lua::AddDataMember( pState, "Shifted",			C_FontStyleTextureInfo::E_FontMappingMode_Shifted );
		Lua::AddDataMember( pState, "RandShifted",		C_FontStyleTextureInfo::E_FontMappingMode_RndShifted );
	lua_rawset( pState, -3 );		

	// xbox keyboard enums
	lua_pushstring( pState, "XboxKeyboardType" );
	lua_newtable( pState );
#if defined(LIVE_ON_WINDOWS)
		// live for windows doesn't have the same keyboard enums
		Lua::AddDataMember( pState, "Gamertag",				VKBD_DEFAULT );
		Lua::AddDataMember( pState, "GamertagHighlight",	VKBD_DEFAULT | VKBD_HIGHLIGHT_TEXT );
#else
		Lua::AddDataMember( pState, "Gamertag",				0 );
		Lua::AddDataMember( pState, "GamertagHighlight",	0 );
#endif
	lua_rawset( pState, -3 );


	// platforms
	lua_pushstring( pState, "Platform" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Unknown",	E_UiPlatform_Unknown );
		Lua::AddDataMember( pState, "Xenon",	E_UiPlatform_Xenon );
		Lua::AddDataMember( pState, "PC",		E_UiPlatform_PC );
		Lua::AddDataMember( pState, "PS3",		E_UiPlatform_PS3 );
	lua_rawset( pState, -3 );

	// current platform
#if defined( PLATFORM_PC )
	Lua::AddDataMember( pState, "CurrentPlatform",	E_UiPlatform_PC );
#elif defined( PLATFORM_PS3 )
	Lua::AddDataMember( pState, "CurrentPlatform",	E_UiPlatform_PS3 );
#else
	Lua::AddDataMember( pState, "CurrentPlatform",	E_UiPlatform_Unknown );
#endif

	// profile/save system errors
	lua_pushstring( pState, "ProfileError" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Okay",					E_UiGameProfile_Error_Okay );
		Lua::AddDataMember( pState, "FileDoesNotExist",		E_UiGameProfile_Error_FileDoesNotExist );
		Lua::AddDataMember( pState, "FileExists",			E_UiGameProfile_Error_FileExists );
		Lua::AddDataMember( pState, "CannotEnumerateFiles",	E_UiGameProfile_Error_CannotEnumerateFiles );
		Lua::AddDataMember( pState, "CannotReadFile",		E_UiGameProfile_Error_CannotReadFile );
		Lua::AddDataMember( pState, "CannotWriteFile",		E_UiGameProfile_Error_CannotWriteFile );
		Lua::AddDataMember( pState, "CannotDeleteFile",		E_UiGameProfile_Error_CannotDeleteFile );
		Lua::AddDataMember( pState, "DeviceNotConnected",	E_UiGameProfile_Error_DeviceNotConnected );
		Lua::AddDataMember( pState, "NoFreeSpace",			E_UiGameProfile_Error_NoFreeSpace );
		Lua::AddDataMember( pState, "UserCancelled",		E_UiGameProfile_Error_UserCancelled );
		Lua::AddDataMember( pState, "DifferentOwner",		E_UiGameProfile_Error_DifferentOwner );
	lua_rawset( pState, -3 );

	// game invite modes
	lua_pushstring( pState, "AcceptGameInviteMode" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Accept",	E_AcceptGameInviteMode_Accept );
		Lua::AddDataMember( pState, "Delay",	E_AcceptGameInviteMode_Delay );
		Lua::AddDataMember( pState, "Discard",	E_AcceptGameInviteMode_Discard );
	lua_rawset( pState, -3 );

	// slider types
	UiXt::CreateLuaTableFromEnumType( pState, "UISliderType", "SliderType" );

	// leaderboard data type
	lua_pushstring( pState, "DataType" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "WCHAR",	E_UIDataType_WCHAR );
		Lua::AddDataMember( pState, "S64",		E_UIDataType_S64 );
		Lua::AddDataMember( pState, "U64",		E_UIDataType_U64 );
		Lua::AddDataMember( pState, "F64",		E_UIDataType_F64 );
	lua_rawset( pState, -3 );

	UiXt::CreateLuaTableFromEnumType( pState, "UIMessage", "Message" );

	// camera types
	UiXt::CreateLuaTableFromEnumType( pState, "UICameraType", "CameraType" );

	// language and region
	UiXt::CreateLuaTableFromEnumType( pState, "UILanguage", "Language" );
	UiXt::CreateLuaTableFromEnumType( pState, "UIRegion", "Region" );

	lua_pop(pState,1);
}


const U32 C_ButtonController::NumberOfGadgets() const
{
	U32 size = 0;
	for ( S32 i = 0; i < C_ScreenControlManager::Instance().Count(); i++ )
	{
		size += C_ScreenControlManager::Instance().Get( i ).NumberOfButtons();
	}
	return size;
}


C_GadgetBase *C_ButtonController::AddButton(const E_UIButtonType type, const S32 context )
{
	MEMORY_TRACKER_PUSH("ui.gadgets");

	// Create a new object
	C_GadgetBase *obj = CreateButton(type);
	if( !obj )
	{
		MEMORY_TRACKER_POP("ui.gadgets");
		return NULL;
	}

	m_CurrentType = type;
	m_CurrentObject = obj;

	obj->m_ID = C_ScreenControlManager::Instance().Get( context ).AddButton( obj );
	obj->StoreXtType();

	obj->SetSubscriberSystem( &GetSubscriberSystem() );
	obj->AddSubscriber( E_UiMsg_Render );
	obj->AddSubscriber( E_UiMsg_StartFrame );
	obj->AddSubscriber( E_UiMsg_SetTimelineActive );
	obj->AddSubscriber( E_UiMsg_UpdateEffects );
	obj->AddSubscriber( E_UiMsg_BindTimelines );
	obj->AddSubscriber( E_UiMsg_PostInit );
	obj->AddSubscriber( E_UiMsg_Exit );
	obj->AddSubscriber( E_UiMsg_EnableControllerInput );

	obj->Init();

	MEMORY_TRACKER_POP("ui.gadgets");

	return obj;
}

C_GadgetBase* C_ButtonController::GetCurrentObject( const S32 context )
{
	return C_ScreenControlManager::Instance().Get( context ).GetCurrentButton(); 
}

C_GadgetBase *C_ButtonController::FindByName( const char *pName )
{
	if( !pName )
		return NULL;

	C_GadgetBase *p = NULL;
	for(S32 i = 0; i < C_ScreenControlManager::Instance().Count(); i++ )
	{
		p = C_ScreenControlManager::Instance().Get( i ).FindByName( pName );
		if( p )
			return p;
	}

	return p;
}

C_GadgetBase *C_ButtonController::Find( const U32 _id )
{
	C_ButtonId id( _id );
	if( id.Valid() == false )
	{
		LOGERROR(g_Log,  "C_ButtonController::Find - Id(%d) is invalid", _id );
		return NULL;
	}

	C_GadgetBase *p = C_ScreenControlManager::Instance().Get( id.Context() ).FindByID( id );
	if( p )
		return p;

	LOGERROR( g_Log,  "C_ButtonController::Find - Can't find button in list(%d)", id.Id() );
	return NULL;
}

void C_ButtonController::BroadcastMessageNow( const U32 mask, const Messaging::C_MessageBase* pMessage )
{
	m_pMessageSubscriberSystem->BroadcastMessage(pMessage, mask);
}

void C_ButtonController::SendMessageNow( const C_ButtonHandle& handle, const Messaging::C_MessageBase* pMessage )
{
	C_GadgetBase *pDest = Find( handle );
	
	// if actor doesn't exist or has been killed return 0
	if(pDest == NULL)
		return;

	pDest->MessageHandler( pMessage );
}

#if 1
// Ensure the deferred message queue is thread safe for loading
void C_ButtonController::BroadcastMessage(const Messaging::C_MessageBase* pMessage, U32 mask)
{
	m_CriticalSecion.Enter();
	
	m_pCurrentDeferredMessageQueue->BroadcastMessage(pMessage, mask);

	m_CriticalSecion.Exit();
}

void C_ButtonController::ProcessDeferredMessages()
{
	bool first = true;
	U32 numMessages = 0;

	do
	{
		C_DeferredUiMessageQueue* queueToProcess=m_pCurrentDeferredMessageQueue;
		
		m_CriticalSecion.Enter();

		if (m_pCurrentDeferredMessageQueue==&m_DeferredMessageQueue1)
		{
			m_pCurrentDeferredMessageQueue=&m_DeferredMessageQueue2;
		}
		else
		{
			m_pCurrentDeferredMessageQueue=&m_DeferredMessageQueue1;
		}

		m_CriticalSecion.Exit();

		numMessages = queueToProcess->ProcessMessages( this );

//		if( !first && numMessages > 0 )
//			LOGDEBUG( g_Log, "flushing %d",  numMessages );

		first = false;

	} while( numMessages );
}
#else
// Ensure the deferred message queue is thread safe for loading
void C_ButtonController::BroadcastMessage(const Messaging::C_MessageBase* pMessage, U32 mask)
{
	m_CriticalSecion.Enter();
	
	m_DeferredMessageQueue.BroadcastMessage(pMessage, mask);

	m_CriticalSecion.Exit();
}

void C_ButtonController::ProcessDeferredMessages()
{
	m_CriticalSecion.Enter();

	m_DeferredMessageQueue.ProcessMessages( this ); 

	m_CriticalSecion.Exit();
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Button list controller

// Process list functions
/*
void C_ButtonController::C_List::FrameUpdate( )
{
	for(ButtonList::Iterator it(m_ProcessList.Begin()); it != m_ProcessList.End(); ++it)
		(*(it))->FrameUpdate();
}

void C_ButtonController::C_List::Render( )
{
	for(ButtonList::Iterator it(m_ProcessList.Begin()); it != m_ProcessList.End(); ++it)
	{
		(*(it))->UpdateEffects( );
		(*(it))->Render( );
	}
}

// Array functions
void C_ButtonController::C_List::ResetTimers( SCREEN_TIME stime )
{
	for(S32 i = 0; i < m_Array.Size(); i++)
		m_Array[ i ]->ResetTimer( stime );
}

void C_ButtonController::C_List::Exit( )
{
	for(S32 i = 0; i < m_Array.Size(); i++)
		m_Array[ i ]->Exit( );
}

void C_ButtonController::C_List::Update(SCREEN_TIME time,UIMessage *Message)
{
//	for(S32 i = 0; i < m_Array.Size(); i++)
//		m_Array[ i ]->Update( time, Message );
}

C_GadgetBase* C_ButtonController::C_List::FindByName( const char *pName )
{
	for(S32 i = 0; i < m_Array.Size(); i++)
		if(m_Array[ i ]->m_Name && stricmp( m_Array[ i ]->m_Name, pName ) == 0 )
			return m_Array[ i ];

	return NULL;
}

C_GadgetBase* C_ButtonController::C_List::Find( const S32 ID )
{
	for(S32 i = 0; i < m_Array.Size(); i++)
		if( m_Array[ i ]->m_ID == ID )
			return m_Array[ i ];

	return NULL;
}

void C_ButtonController::C_List::Clear( )
{
	// Clear process list
	m_ProcessList.Clear();

	// Delete gadgets
	while( m_Array.Size() )
	{
		SAFE_DELETE_PTR( m_Array.Back() );
		m_Array.PopBack();
	}

	// Shrink
	if(m_Array.Capacity() > m_DefaultCapacity * 3 / 2 )
		m_Array.Reserve( m_DefaultCapacity, 0 );
}

void C_ButtonController::C_List::Add( C_GadgetBase * pObject )
{
	m_ProcessList.PushBack( pObject );
	m_Array.PushBack( pObject );
}

void C_ButtonController::C_List::BindTimelines( )
{
	for ( S32 i = 0; i < m_Array.Size(); i++ )
	{
		C_GadgetBase* pObject = m_Array[ i ];

		if ( !pObject )
			continue;

		if( pObject->m_Timeline.count > 0)
			pObject->BindTimeline();
		else
		if( pObject->m_EffectsActive == 0 ) // No effects
			pObject->RemoveSubscriber( E_UiMsg_UpdateEffects );
	}
}

void C_ButtonController::C_List::SetTimelineActive( const char* pName, const bool bActive )
{
	for ( S32 i = 0; i < m_Array.Size(); i++ )
	{
		C_GadgetBase* pObject = m_Array[ i ];

		if ( !pObject )
			continue;

		for ( S32 j = 0; j < pObject->m_Timeline.count; j++ )
		{
			if ( !stricmp( pName, pObject->m_Timeline[ j ].m_Name ) )
				pObject->m_Timeline[ j ].Active( bActive );
		}
	}
}
*/

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
