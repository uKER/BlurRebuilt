#include "stdafx.h"

#include "ui/Game/Gadgets/ButtonMessages.h"
#include "ui/Game/UIScreenControl.h"
#include "ui/Game/Gadgets/ButtonController.h"
#include "ui/Game/UILUA.h"
#include "ui/Game/Timeline.h"
#include "ui/game/stringstore.h"
#include "ui/Graphics/SimpleTimer.h"
#include "ui/Graphics/UIParticle.h"
#include "ui/Game/UiXt.h"
#include "ui/UiInterface.h"

#ifndef CFG_GOLD
#include "ui/Graphics/Font/Font.h"
#endif

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

namespace UI
{

// Singleton for main window (possibly deprecate this
static bool g_ResetScreenDimensions = false;

#ifndef CFG_GOLD

LuaSetHookRoutineCallback gUiSetLuaHookRoutine = 0;

static bool g_ReloadGuiBank = false;
void LuaSetBool( void *pBool )
{
	*((bool*)pBool) = true;
}
Debug::Function g_DebugUiRealdTableDataFunc("UI.ScreenControl.ReloadGuiBank", LuaSetBool, &g_ReloadGuiBank);

Debug::Bool	g_debugUIShowScreens("UI.ScreenControl.ShowUiScreens", false, Debug::Persist());
Debug::Bool	g_DebugUiLogStandardMessages( "UI.ScreenControl.LuaLog.StandardMessages", false );
Debug::Bool	g_DebugUiLogPadMessages( "UI.ScreenControl.LuaLog.PadMessages", false );
Debug::Bool	g_DebugUiLogInternalMessages( "UI.ScreenControl.LuaLog.InternalMessages", false );
Debug::Bool	g_DebugUiLogExternalMessages( "UI.ScreenControl.LuaLog.ExternalMessages", false );

#endif	// CFG_GOLD

// Button ID
C_ButtonId::C_ButtonId( const U32 _context, const U32 _id )
{
	if( _context >= (1<<8) )
	{
		LOGERROR( g_Log, DBSTR "context out of range" );
	}
	if( _id >= (1<<16) )
	{
		LOGERROR( g_Log, DBSTR "id out of range" );
	}

	m_Number = (_context << 24) | (_id << 8) | 0xff;
}

// Add one line of text to the output buffer.
#define AddStr(a,b) (pstrOut += wsprintf( pstrOut, a, b ))

C_ScreenControl::C_ScreenControl( const S32 context ) :
	m_Context( context ),
	m_MessageMask( 1 << context ),
	m_State( E_State_Inactive ),
	m_PrevState( E_State_Init ),
	m_RenderEnd( false ),
	m_SuspendPending( false ),
	m_EndCurrentScreen( false ),
	m_RootRecord( NULL ),
	m_LuaHandler( context ),
	m_ButtonList(this),
	m_bBlockInputToMeDefault( false ),
	m_bBlockInputToMe( false )
{	
	m_CurrentScreen[ 0 ] = NULL;
	m_NextScreen[ 0 ] = NULL;
	m_SwitchScreen[ 0 ] = NULL;
	m_DefaultScreenName[ 0 ] = NULL;
	m_LuaScreenDir[ 0 ] = NULL;
	m_DefaultScreenName[ 0 ] = NULL;

	m_LuaHandler.SetSubscriberSystem( &gButtons.GetSubscriberSystem() );
	m_LuaHandler.AddSubscribers();

	//m_pSendMessageToLua = NULL;
}

void C_ScreenControl::AddLuaSubscriber( const Messaging::TMsgType messageType )
{
	m_LuaHandler.AddSubscriber( messageType );
	m_LuaHandler.SetSubscriberMask( messageType, m_MessageMask );
}

void C_ScreenControl::Init()
{
	char screen[256];

	m_LuaHandler.DoProcess( true );
	m_LuaHandler.Active( true );
	m_Timer = 0.0f;
	m_State = E_State_Init;
	
	m_StartTimer = 0.0f;
	m_EndTimer = 0.0f;

	m_StringsInvalid = false;

	if( strlen(m_NextScreen) > 0 )
	{
		STRCPY( m_CurrentScreen, m_uMaxScriptNameLength, m_NextScreen );
		sprintf_s(screen, lengthof(screen), "%s%s", m_LuaScreenDir, m_NextScreen);
		if( g_pUILua->LoadCallFile(screen) )
			m_LuaHandler.ScriptError( true );
	}

	m_FileReload = true;

	m_SwitchContext = -1;
	m_SwitchScreen[0] = NULL;
}

void C_ScreenControl::End( const char* str, const S32 ns_context )
{
	m_CurrentScreen[ 0 ] = NULL;
	m_NextScreen[ 0 ] = NULL;

	if(str)
	{
		STRCPY( m_SwitchScreen, m_uMaxScriptNameLength, str );
		m_SwitchContext = ns_context;
	}
	else
	{
		STRCPY( m_SwitchScreen, m_uMaxScriptNameLength, "" );
	}
	// Don't render the out loop
	if( m_State == E_State_Suspended )
		m_RenderEnd = true;

	if( m_LuaHandler.ScriptError() )
	{
		m_State = E_State_End;
		m_LuaHandler.ScriptError( false );
	}
	else
	if( m_State != E_State_Inactive )
		SetEndLoop();
}

void C_ScreenControl::SetEndLoop()
{
	C_UiEnableControllerMessage ctrl( false );
	gButtons.BroadcastMessageNow( m_MessageMask, &ctrl );

	UI::C_UiMessageBase<E_UiMsg_EnterEnd> enter_end;
	gButtons.BroadcastMessageNow( m_MessageMask, &enter_end );

	m_LuaHandler.AllowButtonMessages( false );

	m_State = E_State_EndLoop;
	m_EndCurrentScreen = true;

	// Send timeline message 'endloop'
	C_UiTimelineMessage sc_upd( "sc_update", false );
	C_UiTimelineMessage sc_end( "sc_end", true, 0.0f );
	gButtons.BroadcastMessageNow( m_MessageMask, &sc_end );
	gButtons.BroadcastMessageNow( m_MessageMask, &sc_upd );
}

void C_ScreenControl::SetEnd()
{
	UI::C_UiMessageBase< E_UiMsg_Exit > message0;
	gButtons.BroadcastMessageNow( m_MessageMask, &message0 );

	// Clear GUI table - TODO, have this store per context
	if( m_Context == 0 )
	{
		lua_State*	pState = g_pUILua->GetState();

		lua_newtable( pState );
		lua_setglobal( pState, "GUI" );
	}
}

void C_ScreenControl::Exit( )
{
	m_ButtonList.Clear();	// Free up gadgets
	m_Allocator.Free();		// Free up cloned referenced data

	m_EndCurrentScreen = (m_State != E_State_Inactive);
	m_State = E_State_Inactive;

	m_LuaHandler.Active( false );
}

const SCREEN_TIME C_ScreenControl::GetScreenTime( )
{
	switch(m_State)
	{
	case E_State_Init:
	case E_State_StartLoop:
		return SCREEN_TIME_START;

	case E_State_End:
	case E_State_EndLoop:
		return SCREEN_TIME_END;
	}

	return SCREEN_TIME_NORMAL;
}

void C_ScreenControl::SetNextScreen(const char *ns)
{
	if( ns )
	{
		STRCPY( m_NextScreen, m_uMaxScriptNameLength, ns );
		m_FileReload = true;
	}

	if( m_LuaHandler.ScriptError() )
	{
		m_State = E_State_End;
		m_LuaHandler.ScriptError( false );
	}
	else
	{
		m_EndCurrentScreen = (m_State != E_State_Inactive);
		if( m_EndCurrentScreen )
		{
			SetEndLoop();
		}
		else
		{
			m_State = E_State_End;
		}
	}

	// Try to run the LUA callback
	lua_State*	pState = g_pUILua->GetState();
	if(pState)
	{
		lua_getglobal(pState, "SetNextScreen_CB");
		if(lua_type(pState, -1) == LUA_TFUNCTION) // Check that the function exists.
			lua_pcall(pState, 0, 0, 0);
		else
			lua_pop(pState,1);
	}

	// Broadcast the change screen message
	_UI_MESSAGE1( E_UiMsg_ChangeScreen, m_Context );
}

void C_ScreenControl::SetDefaultScreenName( const char *name )
{
	assert( name );
	assert( strlen( name ) > 0 && strlen( name ) < m_uMaxScriptNameLength );

	STRCPY( m_DefaultScreenName, m_uMaxScriptNameLength, name );
}

void C_ScreenControl::ChangeScreen()
{
	char screen[256];
	int error = 0;
	
	Exit();

	if( m_NextScreen[0] == NULL )
	{
		// Change another contexts screen?
		if( m_SwitchScreen[0] )
		{
			C_ScreenControlManager::Instance().Get( m_SwitchContext ).SetNextScreen( m_SwitchScreen );
			m_SwitchScreen[0] = NULL;
		}

		return;
	}

	m_StartTimer = 0.0f;
	m_EndTimer = 0.0f;
	m_StringsInvalid = false;
	m_State = E_State_Init;
	m_bBlockInputToMe = m_bBlockInputToMeDefault;	// reset on a screen change

	m_LuaHandler.Active( true );

	gUIParticleMan.Reset();
	C_StringStore::Instance().ClearAll();

#ifndef CFG_GOLD
	bool forceFromHDD = m_NextScreen[0] == '?';
	sprintf_s(screen, lengthof(screen), "%s%s", m_LuaScreenDir, (!forceFromHDD) ? m_NextScreen : m_NextScreen+1);
#else
	bool forceFromHDD(false);
	sprintf_s(screen, lengthof(screen), "%s%s", m_LuaScreenDir, m_NextScreen);
#endif

	if( m_FileReload )
	{
		// Store current context
		S32 ctx = C_ScreenControlManager::Instance().Context();
		// Change to my context, so that any objects added in during these Lua calls
		C_ScreenControlManager::Instance().Context( m_Context );

		lua_State*	pState = g_pUILua->GetState();

		lua_getglobal(pState, "PreLoadCallFile" );
		if( lua_type( pState, -1 ) == LUA_TFUNCTION )
			m_LuaHandler.HandleError( lua_pcall(pState, 0, 0, 0) );
		else
			lua_pop(pState,1);

		error = g_pUILua->LoadCallFile(screen, forceFromHDD);
		if( error && m_DefaultScreenName[ 0 ] )
		{
#ifndef CFG_GOLD
			bool forceFromHDD = m_DefaultScreenName[0] == '?';
			sprintf_s(screen, lengthof(screen), "%s%s", m_LuaScreenDir, (!forceFromHDD) ? m_DefaultScreenName : m_DefaultScreenName+1);
#else
			bool forceFromHDD(false);
			sprintf_s(screen, lengthof(screen), "%s%s", m_LuaScreenDir, m_NextScreen);
#endif

			error = g_pUILua->LoadCallFile( screen, forceFromHDD );

			// Flag to not try and reload this file
			m_FileReload = false;
		}

		lua_getglobal(pState, "PostLoadCallFile" );
		if( lua_type( pState, -1 ) == LUA_TFUNCTION )
			m_LuaHandler.HandleError( lua_pcall(pState, 0, 0, 0) );
		else
			lua_pop(pState,1);

		// Restore context
		C_ScreenControlManager::Instance().Context( ctx );
	}

	if( error )
		m_LuaHandler.ScriptError( true );

	STRCPY( m_CurrentScreen, m_uMaxScriptNameLength, m_NextScreen );

	m_RenderEnd = true;
	Update();
}

void C_ScreenControl::Resume( )
{
	if( m_SuspendPending )
	{
		m_SuspendPending = false;
		return;
	}

	if( m_State != E_State_Suspended )
		return;

	m_State = m_PrevState;
}

void C_ScreenControl::Suspend( )
{
	if( m_State == E_State_Suspended || m_SuspendPending )
		return;

	m_PrevState = m_State;
	m_SuspendPending = true;
}

void C_ScreenControl::AddXtRecord( const char* pScreenName, const bool bForceNewTable, const bool bCreateXtElements )
{
	lua_State* pState = g_pUILua->GetState();

	const UiXt::C_Screen* pScreen = UiXt::C_ScreenList::Instance().FindByName( pScreenName );

	if( pScreen )
	{
		m_RootRecord = pScreen->m_Self.Record();

		pScreen->MakeLuaTable( pState, m_Context, bForceNewTable, m_PointerMap );
		
		if( bCreateXtElements )
			pScreen->AddElements( pState, m_RootRecord, m_Context, pScreen->m_GuiTableName, m_PointerMap );

		m_PointerMap.CreateReferencedData( m_RootRecord, m_Allocator );
	}
	else
	{
		m_RootRecord = NULL;
	}
}

void C_ScreenControl::Update()
{
	if( m_State == E_State_Inactive )
		return;

	lua_State*	pState = g_pUILua->GetState();

#ifndef CFG_GOLD
	if(g_debugUIShowScreens)
	{
		C_WideString128 str;

		// PR - can check something else too
		const char* pScreenName = NULL;
		Lua::Call( g_pUILua->GetState(), "GetCurrentScreen", "b>s", true, &pScreenName );

		if ( m_Context == 0 && pScreenName && pScreenName[ 0 ] )
			str.Format( L"%S%S~~/n~~%S%S", m_LuaScreenDir, m_NextScreen, m_LuaScreenDir, pScreenName );
		else
			str.Format( L"%d - %S%S", m_Context, m_LuaScreenDir, m_NextScreen );

		gFont.DrawDebugText( gSimple2D.TopLayer(), Maths::HVec3( 320.0f, 432.0f - (m_Context*12.0f), 0.0f ), E_Justify_TopCentre, str, (m_State == E_State_Suspended) ? 0x80808080 : 0xffffafaf, 14.0f );
	}
#endif
	if (m_Context == 0)
	{
		if ( C_ScreenControlManager::Instance().ResetContext() )
		{
			C_ScreenControlManager::Instance().Context( C_ScreenControlManager::Instance().m_sContextReset );
			C_ScreenControlManager::Instance().ResetContext( false );
		}
#ifndef CFG_GOLD
		if (gUiSetLuaHookRoutine)
			gUiSetLuaHookRoutine(pState);
#endif
	}
	if( m_LuaHandler.ScriptError() )
		return;

	if (m_Context == 0)
	{
		m_LuaHandler.DoProcess( true );
		lua_getglobal(pState, "GlobalUpdate");
		if(lua_type(pState, -1) == LUA_TFUNCTION) // Check that the function exists.
		{
			lua_pushnumber(pState, theFETimer.FrameTime() );
			if(lua_pcall(pState, 1, 1, 0))
			{
				LOGWARN( g_Log, "Lua error in GlobalUpdate function : %s\n", lua_tostring(pState,-1));
			}
			else
			{
				m_LuaHandler.DoProcess( lua_toboolean(pState, -1) ? true : false );
				lua_pop(pState,1);
			}
		}
		else
		{
			lua_pop(pState,1);
		}
	}

#ifndef CFG_GOLD
	// Sorcery update
	const S32 df = UiXt::C_ScreenList::Instance().IsDirty();
	if( df )
	{
		if((df & m_MessageMask || // Only update if my context was changed
			df & UiXt::C_ScreenList::E_DirtyFlags_Colour ||
			df & UiXt::C_ScreenList::E_DirtyFlags_Font ) )
		{
			// PR - I'm calling this to set something before we reload
			if( m_Context == 0)
				Lua::Call( pState, "DebugScreenReload" );

			SetNextScreen();

			if( df & UiXt::C_ScreenList::E_DirtyFlags_Font )
			{
				m_State = E_State_End;
				m_RenderEnd = true;
				return;
			}
		}
	}
#endif

	if( m_RenderEnd == false && m_SuspendPending == false && m_State != E_State_Suspended )
	{
		_UI_MESSAGE_MASK( E_UiMsg_StartFrame, m_MessageMask );
		_UI_MESSAGE1_MASK( E_UiMsg_FrameUpdate, (S32)m_State, m_MessageMask );
		_UI_MESSAGE_MASK( E_UiMsg_UpdateEffects, m_MessageMask );
		_UI_MESSAGE_MASK( E_UiMsg_Render, m_MessageMask );
	}

	switch( m_State )
	{
		case E_State_Init:
			m_RenderEnd = false;
			m_PointerMap.ClearBindings();

			m_LuaHandler.AllowButtonMessages( false );

#if PLATFORM_PC
			// Force garbage collection
			lua_gc(pState,LUA_GCCOLLECT,0);
#endif

			// Build the Sorcery table (if it exists)
			{
				AddXtRecord( m_NextScreen, true );
			}

			// Init
			{
				UI::C_UiMessageBase<E_UiMsg_Init> message0;
				gButtons.BroadcastMessageNow( m_MessageMask, &message0 );
			}

			// Do not move this (as Init is now over and PostInit() can check InitState() to see if PostInit() and BindTimeline() need calling on gadgets)
			m_State = E_State_StartLoop;
			
			// Bind pointers (after object have been added)
			m_PointerMap.ListRemap();

			m_ButtonList.RegenerateParentLists();
			
			// Post init
			{
				UI::C_UiMessageBase<E_UiMsg_BindTimelines> message0;
				gButtons.BroadcastMessageNow( m_MessageMask, &message0 );

				UI::C_UiMessageBase<E_UiMsg_PostInit> message1;
				gButtons.BroadcastMessageNow( m_MessageMask, &message1 );
			}
			
			m_Timer = 0.0f;

			// Disable controller input during start loop
			{
				C_UiEnableControllerMessage message( false, true );
				gButtons.BroadcastMessageNow( m_MessageMask, &message );
			}

			if( m_SuspendPending )
			{
				m_State = E_State_Suspended;
				m_PrevState = E_State_StartLoop;

				m_SuspendPending = false;
				break;
			}

		case E_State_StartLoop:
			m_Timer += theFETimer.FrameTime();
			if(m_Timer > m_StartTimer)
			{
				m_Timer=0.f;
				m_State = E_State_Update;

				m_LuaHandler.AllowButtonMessages( true );

				// Enable controller input
				C_UiEnableControllerMessage message( true );
				gButtons.BroadcastMessageNow( m_MessageMask, &message );

				// Send timeline message 'update'
				C_UiTimelineMessage nit( "sc_init", false );
				C_UiTimelineMessage upd( "sc_update", true, 0.0f, true );
				gButtons.BroadcastMessageNow( m_MessageMask, &nit );
				gButtons.BroadcastMessageNow( m_MessageMask, &upd );
			}

			break;

		case E_State_Update:
			if( m_SuspendPending )
			{
				m_SuspendPending = false;
				m_State = E_State_Suspended;
				m_PrevState = E_State_Update;
			}
			break;

		case E_State_End:
			// Exit
			if( m_EndCurrentScreen )
				SetEnd();

			ChangeScreen();
			break;

		case E_State_EndLoop:
			m_Timer += theFETimer.FrameTime();
			if(m_Timer > m_EndTimer)
			{
				m_Timer = 0.f;
				m_State = E_State_End;
			}
			break;

		default:
			break;
	}

	if( g_ResetScreenDimensions && m_Context == 0 )
	{
		g_ResetScreenDimensions = false;
		g_pUILua->LoadCallFile("Screens\\GuiBank.lua",false);
		g_pUILua->LoadCallFile("Popups\\CustomPopupData.lua",false);
		g_pUILua->LoadCallFile("Popups\\TheCustomPopup.lua",false);

		gUIPopupControl.Exit();
		SetNextScreen();
	}
#ifndef CFG_GOLD
	if( g_ReloadGuiBank && m_Context == 0 )
	{
		// PR - I'm calling this to set something before we reload
		Lua::Call( pState, "DebugScreenReload" );

		g_ReloadGuiBank = false;
		g_pUILua->LoadCallFile("Screens\\GuiBank.lua",true);
		g_pUILua->LoadCallFile("Popups\\CustomPopupData.lua",true);
		g_pUILua->LoadCallFile("Popups\\TheCustomPopup.lua",true);

		gUIPopupControl.Exit();
		SetNextScreen();
	}
#endif
}

void C_ScreenControl::ReloadLuaScripts()
{
	g_ResetScreenDimensions = true;
}

// Button lists
C_GadgetBase* C_ScreenControl::C_List::FindByName( const char *pName )
{
	for(S32 i = 0; i < m_Array.Size(); i++)
		if(m_Array[ i ]->m_Name && stricmp( m_Array[ i ]->m_Name, pName ) == 0 )
			return m_Array[ i ];

	return NULL;
}

C_GadgetBase* C_ScreenControl::C_List::Find( const C_ButtonId ID )
{
	U32	n	= ID.Number();
	int	id	= (int)ID.Id();

	if (id < m_Array.Size())
	{
		C_GadgetBase* gadget = m_Array[id];

		if (gadget && (gadget->m_ID.Number() == n))
			return gadget;
	}

	for(S32 i = 0; i < m_Array.Size(); i++)
		if( m_Array[ i ]->m_ID.Number() == n )
			return m_Array[ i ];

	return NULL;
}

namespace
{
	int FindChildrenSub(C_GadgetBase* parent, ButtonArray* children, bool all, const E_UIButtonType type, U32 context)
	{
		if (!parent)
			return 0;

		int count = 0;

		for(XtUt::LinkList<C_GadgetBase>::Iterator it = parent->ParentChildList(); it; ++it)
		{
			if (it->m_ID.Context() == context)
			{
				if ((type == E_UIButtonType_None) || (it->GetType() == type))
				{
					if (children)
						children->PushBack( it );

					count++;
				}
			}

			if (all)
				count += FindChildrenSub(it, children, all, type, context);
		}

		return count;
	}
}

const S32 C_ScreenControl::C_List::FindChildren( const C_GadgetBase* parent, ButtonArray* children, const bool all, const E_UIButtonType type )
{
	return FindChildrenSub((C_GadgetBase*)parent, children, all, type, m_ScreenControl->m_Context);
}

C_GadgetBase* C_ScreenControl::C_List::FindChildByName( C_GadgetBase* parent, const char* pName )
{
	if (!parent)
		return 0;

	for(XtUt::LinkList<C_GadgetBase>::Iterator it(parent->ParentChildList()); it; ++it)
	{
		if (it->m_ID.Context() != m_ScreenControl->m_Context)
			continue;

		const char* name = it->m_Name.Text();

		if (!name)
			continue;

		if (strcmp(name, pName) == 0)
			return it;
	}

	for(XtUt::LinkList<C_GadgetBase>::Iterator it(parent->ParentChildList()); it; ++it)
	{
		if (C_GadgetBase* gadget = FindChildByName(it, pName))
			return gadget;
	}

	return 0;
}


void C_ScreenControl::C_List::Clear( )
{
	// Delete gadgets
	while( m_Array.Size() )
	{
		SAFE_DELETE_PTR( m_Array.Back() );
		m_Array.PopBack();
	}
}

void C_ScreenControl::C_List::RegenerateParentLists()
{
	for(int i = 0; i < m_Array.Size(); i++)
	{
		C_GadgetBase* gadget = m_Array[ i ];

		if (!gadget)
			continue;

		gadget->SetParent(gadget->Parent());
	}
}

// Screen control manager container
C_ScreenControlManager C_ScreenControlManager::m_sInstance;
bool C_ScreenControlManager::m_bResetContext = false;
S32 C_ScreenControlManager::m_sContextReset = 0;

C_ScreenControlManager::C_ScreenControlManager( ) : 
	m_Context( -1 )
{

}

C_ScreenControlManager::~C_ScreenControlManager( )
{
	Exit();
}

void C_ScreenControlManager::Context( const S32 context )
{ 
	if( context >= m_List.Size() )
	{
		LOGERROR( g_Log, DBSTR " : Slot %d out of range", context );
		return;
	}

	m_Context = context; 
}

void C_ScreenControlManager::Exit( )
{
	for( S32 i = 0; i < m_List.Size(); i++ )
		SAFE_DELETE_PTR( m_List[ i ] );

	m_List.Resize(0);
}

const S32 C_ScreenControlManager::Add( )
{
	MEMORY_TRACKER_PUSH("ui.screens");

	const U32 context = m_List.Size();

#ifndef CFG_GOLD
	if( context == 0 )
	{
		// register message debug info
		U32 uNumMessagesRegistered = 0;

		_UI_MESSAGE_DEBUG( Init );
		_UI_MESSAGE_DEBUG( PostInit );
		_UI_MESSAGE_DEBUG( BindTimelines );	
		_UI_MESSAGE_DEBUG( LuaCommand );
		_UI_MESSAGE_DEBUG( StartFrame );
		_UI_MESSAGE_DEBUG( FrameUpdate );
		_UI_MESSAGE_DEBUG( UpdateEffects );
		_UI_MESSAGE_DEBUG( SetTimelineActive );
		_UI_MESSAGE_DEBUG( Render );
		_UI_MESSAGE_DEBUG( EnterEnd );
		_UI_MESSAGE_DEBUG( Exit );
		_UI_MESSAGE_DEBUG( EnableControllerInput );
		_UI_MESSAGE_DEBUG( PlaySoundEffect );
		_UI_MESSAGE_DEBUG( ChangeScreen );

		_UI_MESSAGE_DEBUG( CtrlStart );

		_UI_MESSAGE_DEBUG( CtrlButton_Up );
		_UI_MESSAGE_DEBUG( CtrlButton_Down );
		_UI_MESSAGE_DEBUG( CtrlButton_Left );
		_UI_MESSAGE_DEBUG( CtrlButton_Right );
		_UI_MESSAGE_DEBUG( CtrlButton_A );
		_UI_MESSAGE_DEBUG( CtrlButton_B );
		_UI_MESSAGE_DEBUG( CtrlButton_X );
		_UI_MESSAGE_DEBUG( CtrlButton_Y );
		_UI_MESSAGE_DEBUG( CtrlButton_Start );
		_UI_MESSAGE_DEBUG( CtrlButton_Back );
		_UI_MESSAGE_DEBUG( CtrlButton_RightTrigger );
		_UI_MESSAGE_DEBUG( CtrlButton_LeftTrigger );
		_UI_MESSAGE_DEBUG( CtrlButton_LeftShoulder );
		_UI_MESSAGE_DEBUG( CtrlButton_RightShoulder );
		_UI_MESSAGE_DEBUG( CtrlButton_LeftThumb );
		_UI_MESSAGE_DEBUG( CtrlButton_RightThumb );

		_UI_MESSAGE_DEBUG( CtrlLeftStick_X );
		_UI_MESSAGE_DEBUG( CtrlLeftStick_Y );
		_UI_MESSAGE_DEBUG( CtrlRightStick_X );
		_UI_MESSAGE_DEBUG( CtrlRightStick_Y );

		_UI_MESSAGE_DEBUG( CtrlMouse_Move_X );
		_UI_MESSAGE_DEBUG( CtrlMouse_Move_Y );
		_UI_MESSAGE_DEBUG( CtrlMouse_Wheel );
		_UI_MESSAGE_DEBUG( CtrlMouse_Pressed );
		_UI_MESSAGE_DEBUG( CtrlMouse_DebOn );
		_UI_MESSAGE_DEBUG( CtrlMouse_DebOff );
		_UI_MESSAGE_DEBUG( CtrlMouse_Click );

		_UI_MESSAGE_DEBUG( MenuBack );
		_UI_MESSAGE_DEBUG( MenuNext );

		uNumMessagesRegistered++;	// E_UiMsg_CtrlEnd doesn't need doing

		_UI_MESSAGE_DEBUG( ControllerDisconnected );
		_UI_MESSAGE_DEBUG( ProfileChanged );

		_UI_MESSAGE_DEBUG( MovieFinished );
		_UI_MESSAGE_DEBUG( TextPrintFinished );

		_UI_MESSAGE_DEBUG( KeyboardFinished );
		_UI_MESSAGE_DEBUG( KeyboardCancelled );

		_UI_MESSAGE_DEBUG( SaveLocationFound );
		_UI_MESSAGE_DEBUG( SaveLocationNotFound );
		_UI_MESSAGE_DEBUG( SaveEnumerationFinished );

		_UI_MESSAGE_DEBUG( NodeListTableFixedUpdated );

		_UI_MESSAGE_DEBUG( VerticalMenuChanged );
		_UI_MESSAGE_DEBUG( MouseInBox );
		_UI_MESSAGE_DEBUG( MouseOutBox );

		_UI_MESSAGE_DEBUG( AutomationLoadGame );

		_UI_MESSAGE_DEBUG( PopupNext );
		_UI_MESSAGE_DEBUG( PopupBack );
		_UI_MESSAGE_DEBUG( PopupEnd );

		_UI_MESSAGE_DEBUG( LegalFinished );
		_UI_MESSAGE_DEBUG( CreditsFinished );

		// if this asserts here then someone has changed E_UiMsg - you need to add some _UI_MESSAGE_DEBUG calls above
		assert( uNumMessagesRegistered == ( E_UiMsg_Count - ( E_UiMsg_Start + 1 ) ) );
	}
#endif

	m_List.PushBack( new C_ScreenControl( context ) );

	MEMORY_TRACKER_POP("ui.screens");

	return context;
}

C_ScreenControl& C_ScreenControlManager::Get( const S32 slot )
{
	const S32 index = (slot == - 1) ? m_Context : slot;

	if( m_List[ index ] == NULL )
		LOGERROR( g_Log, DBSTR " : Slot %d not added", index );

	return *m_List[ index ];
}

const S32 C_ScreenControlManager::InputMessageMask( )
{
	S32 sMask = 0;

	// pass through each active context - first active blocking one overrides
	for ( S32 i = 0; i < Count(); i++ )
	{
		C_ScreenControl& sc = Get( i );

		if ( !sc.Active() )
			continue;

		const U32 uMessageMask = sc.GetMessageMask();

		if ( sc.BlockInputToMe() )
		{
			sMask = uMessageMask;
			break;
		}

		sMask |= uMessageMask;
	}

	return sMask;
}

// Lua Message handler
C_LuaMessageHandler::C_LuaMessageHandler( const S32 context ) : 
	m_Context( context ), 
	m_MessageMask( 1 << context ),
	m_Active( true  ),
	m_DoProcess( true ),
	m_FatalScriptError( false ),
	m_AllowButtonMessages( true ),
	m_ScreenFunc( 0 )
{ 
	m_pState = g_pUILua->GetState();
	BIZ_ASSERT( m_pState );
}

void C_LuaMessageHandler::AddSubscribers( )
{
	// Controller messages
	for( S32 i = (S32)E_UiMsg_CtrlStart+1; i < (S32)E_UiMsg_CtrlEnd; i++ )
	{
		AddSubscriber( i );
		SetSubscriberMask( i, m_MessageMask );
	}

	// Standard screen messages
	const E_UiMsg screen[] = 
	{
		E_UiMsg_Init,
		E_UiMsg_PostInit,
		E_UiMsg_FrameUpdate,
		E_UiMsg_EnterEnd,
		E_UiMsg_Exit,
		E_UiMsg_LuaCommand,
	};

	for( S32 i = 0; i < lengthof( screen ); i++ )
	{
		AddSubscriber( screen[ i ] );
		SetSubscriberMask( screen[ i ], m_MessageMask );
	}

	// Additional UI messages
	for( S32 i = E_UiMsg_CtrlEnd + 1; i < E_UiMsg_Count; i++ )
	{
		AddSubscriber( i );
		SetSubscriberMask( i, m_MessageMask );
	}

}

void C_LuaMessageHandler::SetFunctionName( const E_ScreenFunc sf, const char *name )
{
	if( name )
	{
		// Set
		m_ScreenFunc |= (1<<sf);
		m_FunctionNames[ sf ] = name; 
	}
	else
	{
		// Clear
		m_ScreenFunc |= ~(1<<sf);
	}
}

const bool C_LuaMessageHandler::HandleError( const int error )
{
	if( !error )
		return false;

	if( m_FatalScriptError )
		return false;

	m_FatalScriptError = true;

	LOGERROR( g_Log, DBSTR "LUA runtime error : %s", lua_tostring(m_pState,-1));
	g_pUILua->StackDump( m_pState, "HandleError");

	lua_pop(m_pState, 1);  // pop error message from the stack

	return true;
}

const bool C_LuaMessageHandler::ScriptError( ) const
{
	return m_FatalScriptError;
}

void C_LuaMessageHandler::ScriptError( const bool error )
{
	m_FatalScriptError = error;
}

void C_LuaMessageHandler::Active( const bool a )
{
	m_Active = a;
}

void C_LuaMessageHandler::DoProcess( const bool dp )
{
	m_DoProcess = dp;
}

void C_LuaMessageHandler::AllowButtonMessages( const bool abm )
{
	m_AllowButtonMessages = abm;
}

bool C_LuaMessageHandler::MessageHandler(const Messaging::C_MessageBase *pMessage)
{
	if( !m_Active )
		return true;

	bool ret = false;

	// Store current context
	S32 ctx = C_ScreenControlManager::Instance().Context();

	// Change to my context, so that any objects added in during these Lua calls
	C_ScreenControlManager::Instance().Context( m_Context );

	if( pMessage->Type() > E_UiMsg_CtrlStart && pMessage->Type() < E_UiMsg_CtrlEnd && HandleController( pMessage ) )
	{
#ifndef CFG_GOLD
	if( g_DebugUiLogPadMessages )
		LogMessage( pMessage );
#endif
		ret = true;
	}
	else
	if( HandleStandard( pMessage ) )
	{
#ifndef CFG_GOLD
	if( g_DebugUiLogStandardMessages )
		LogMessage( pMessage );
#endif
		ret = true;
	}
	else
	if( pMessage->Type() > E_UiMsg_CtrlEnd && pMessage->Type() < E_UiMsg_Count && HandleInternal( pMessage ) )
	{
#ifndef CFG_GOLD
	if( g_DebugUiLogInternalMessages )
		LogMessage( pMessage );
#endif
		ret = true;
	}
	else
	if( HandleExternal( pMessage ) )
	{
#ifndef CFG_GOLD
	if( g_DebugUiLogExternalMessages )
		LogMessage( pMessage );
#endif
		ret = true;
	}

	// Restore context
	C_ScreenControlManager::Instance().Context( ctx );

	return ret;
}

#ifndef CFG_GOLD
void C_LuaMessageHandler::LogMessage( const Messaging::C_MessageBase *pMessage )
{
#ifndef CFG_SPEED
	if( !pMessage || !pMessage->GetDebugName() )
		return;
	
	LOGINFO( g_Log, "LuaMessage : %s (%d)", pMessage->GetDebugName(), m_Context );
#endif
}
#endif

bool C_LuaMessageHandler::HandleController( const Messaging::C_MessageBase *pMessage )
{
	if( !m_AllowButtonMessages )
		return true;

	if( !PrepScreenFunc( E_ScreenFunc_MessageUpdate ) )
		return false;
	
	lua_pushinteger( m_pState, (lua_Integer)pMessage->Type() );
	int args = C_UiMessagePadData::SendToLua( (C_UiMessagePadData*)pMessage, m_pState );
	HandleError( lua_pcall(m_pState, args + 1, 0, 0) );

	return true;
}

bool C_LuaMessageHandler::HandleInternal( const Messaging::C_MessageBase *pMessage )
{
	if( !PrepScreenFunc( E_ScreenFunc_MessageUpdate ) )
		return false;

	// Old style standard internal message (msg, arg0, arg1, arg2, arg3)
	lua_pushinteger( m_pState, (lua_Integer)pMessage->Type() );

	const C_UiMessageData* pUiMessageData = static_cast<const C_UiMessageData*>(pMessage);

	lua_pushnumber( m_pState, pUiMessageData->m_Args[ 0 ] );
	lua_pushnumber( m_pState, pUiMessageData->m_Args[ 1 ] );
	lua_pushnumber( m_pState, pUiMessageData->m_Args[ 2 ] );
	lua_pushnumber( m_pState, pUiMessageData->m_Args[ 3 ] );

	HandleError( lua_pcall(m_pState, 5, 0, 0) );

	return true;
}

bool C_LuaMessageHandler::HandleExternal( const Messaging::C_MessageBase *pMessage )
{
	if( !PrepScreenFunc( E_ScreenFunc_MessageUpdate ) )
		return false;

	lua_pushinteger( m_pState, (lua_Integer)pMessage->Type() );

	int args = UiInterface()->PushMessageLuaParams( m_pState, pMessage );

	HandleError( lua_pcall(m_pState, args + 1, 0, 0) );

	return true;
}

bool C_LuaMessageHandler::HandleStandard( const Messaging::C_MessageBase *pMessage )
{
	switch( pMessage->Type() )
	{
	// State
	case E_UiMsg_Init:
		CallScreenFunc( E_ScreenFunc_Init, 0.0 );
		return true;

	case E_UiMsg_PostInit:		
		CallScreenFunc( E_ScreenFunc_PostInit, 0.0 );
		return true;

	case E_UiMsg_EnterEnd:
		CallScreenFunc( E_ScreenFunc_EnterEnd, 0.0 );
		return true;

	case E_UiMsg_Exit:
		CallScreenFunc( E_ScreenFunc_Exit, 0.0 );
		return true;

	// Frame update
	case E_UiMsg_FrameUpdate:
		switch( (C_ScreenControl::E_State)(S32)static_cast<const C_UiMessageData*>( pMessage )->m_Args[ 0 ] )
		{
		case C_ScreenControl::E_State_StartLoop:
			CallScreenFunc( E_ScreenFunc_StartLooop, (F64)theFETimer.FrameTime() );
			break;

		case C_ScreenControl::E_State_Update:
			CallScreenFunc( E_ScreenFunc_FrameUpdate, (F64)theFETimer.FrameTime() );
			break;

		case C_ScreenControl::E_State_EndLoop:
			CallScreenFunc( E_ScreenFunc_EndLoop, (F64)theFETimer.FrameTime() );
			break;
		}
		return true;

	// Direct command
	case E_UiMsg_LuaCommand :
		HandleCommand( pMessage );
		return true;
	}

	return false;
}

bool C_LuaMessageHandler::HandleCommand( const Messaging::C_MessageBase *pMessage )
{
	const C_UiLuaCommandMessage* pMsg = Messaging::MessageCast<C_UiLuaCommandMessage>(pMessage);

	// PR - doing this from Lua now as GUI/SCUI don't seem to be set correctly if we've got more than one context running
	if( !Lua::Call( m_pState, "HandleCommand", "s", pMsg->m_Command ) )
	{
		if ( HandleError( luaL_dostring( m_pState, pMsg->m_Command ) ) )
		{
			LOGWARN( g_Log, DBSTR "Error : in command (%s)", pMsg->m_Command );
			return false;
		}
	}

	return true;
}

bool C_LuaMessageHandler::PrepScreenFunc( const E_ScreenFunc sf )
{
	if( !(m_ScreenFunc & (1<<sf) ) )
		return false;

	lua_getglobal(m_pState, m_FunctionNames[ sf ].Chars() );
	if( lua_type( m_pState, -1 ) != LUA_TFUNCTION )
	{
		lua_pop( m_pState, 1 );
		return false;
	}

	return true;
}

void C_LuaMessageHandler::CallScreenFunc( E_ScreenFunc sf, const F64 arg )
{
	if( !PrepScreenFunc( sf ) )
		return;

	lua_pushnumber( m_pState, (lua_Number)arg );
	HandleError( lua_pcall(m_pState, 1, 0, 0) );
}


}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
