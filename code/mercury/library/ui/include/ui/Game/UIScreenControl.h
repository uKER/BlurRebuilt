#ifndef C_ScreenControl_H
#define C_ScreenControl_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/Messaging.h>

#include "ui/Game/UiXt.h"
#include "ui/UIDefines.h"

struct XtRecord;

namespace UI
{

typedef void (*LuaSetHookRoutineCallback)(lua_State *L);

extern LuaSetHookRoutineCallback gUiSetLuaHookRoutine;

class C_GadgetBase;

typedef Btl::List< C_GadgetBase* >			ButtonList;
typedef Btl::Vector< C_GadgetBase* >		ButtonArray;

class C_ButtonId
{
public:
	C_ButtonId( ) : m_Number( m_Invalid )					{ }
	C_ButtonId( const U32 _number ) : m_Number( _number )	{ }
	C_ButtonId( const U32 _context, const U32 _id );

	const U32	Number( ) const		{ return m_Number; }
	const U32	Context( ) const	{ return (m_Number>>24) & 0xff; }
	const U32	Id( ) const			{ return (m_Number>>8) & 0xffff; }
	operator	F64( ) const		{ return (F64)m_Number; }
	const bool	Valid( ) const		{ return m_Invalid != m_Number && m_Number >= 0xff; }

	// bits 0 - 7	[ padding for error checking ]
	// bits 8 - 24	[ space of ids ]
	// bits 24 - 32	[ space for context ]

private:
	static const U32	m_Invalid = 0xffffffff;
	U32					m_Number;
};

class C_LuaMessageHandler : public Messaging::C_MessageHandler
{
public:
	// Screen functions
	enum E_ScreenFunc
	{
		E_ScreenFunc_Init,
		E_ScreenFunc_PostInit,
		E_ScreenFunc_StartLooop,
		E_ScreenFunc_FrameUpdate,
		E_ScreenFunc_MessageUpdate,
		E_ScreenFunc_EnterEnd,
		E_ScreenFunc_EndLoop,
		E_ScreenFunc_Exit,
		// End marker
		E_ScreenFunc_Count
	};

public:
	C_LuaMessageHandler( const S32 context );

public:
	// C_MessageHandler
	bool				MessageHandler(const Messaging::C_MessageBase *pMessage);
	void				AddSubscribers( );

	void				SetFunctionName( const E_ScreenFunc sf, const char *name );


	void				DoProcess( const bool dp );
	void				AllowButtonMessages( const bool abm );
	void				Active( const bool a );
	void				ScriptError( const bool error );

	const bool			ScriptError( ) const;
	const bool			HandleError( const int error );

private:
	bool				HandleCommand( const Messaging::C_MessageBase *pMessage );
	bool				HandleStandard( const Messaging::C_MessageBase *pMessage );
	bool				HandleController( const Messaging::C_MessageBase *pMessage );
	bool				HandleExternal( const Messaging::C_MessageBase *pMessage );
	bool				HandleInternal( const Messaging::C_MessageBase *pMessage );

	void				CallScreenFunc( E_ScreenFunc sf, const F64 arg );

	bool				PrepScreenFunc( E_ScreenFunc sf );

#ifndef CFG_GOLD
	void				LogMessage( const Messaging::C_MessageBase *pMessage );
#endif

private:
	Util::C_String32	m_FunctionNames[ E_ScreenFunc_Count ];
	lua_State*			m_pState;
	S32					m_Context;
	S32					m_MessageMask;
	bool				m_Active;
	bool				m_DoProcess;
	bool				m_FatalScriptError;
	bool				m_AllowButtonMessages;
	U32					m_ScreenFunc;
};

class C_ScreenControl
{
public:
	enum E_State
	{
		E_State_Inactive,
		E_State_Suspended,
		E_State_Init,
		E_State_StartLoop,
		E_State_Update,
		E_State_End,
		E_State_EndLoop,
		E_State_Max,
	};
private:
	class C_List
	{
	public:
		C_List( C_ScreenControl* self) : m_DefaultCapacity( 16 ), m_ScreenControl(self)
		{

		}

		~C_List( )
		{
			Clear();
		}

		void				Add( C_GadgetBase * pObject )					{ m_Array.PushBack( pObject ); }
		void				Clear();

		// Access
		void				DefaultCapacity( const S32 default_capacity )	{ m_DefaultCapacity = default_capacity; }
		C_GadgetBase*		FindByName( const char *pName );
		C_GadgetBase*		Find( const C_ButtonId id );
		const S32			FindChildren( const C_GadgetBase *parent, ButtonArray* children, const bool all, const E_UIButtonType type );
		C_GadgetBase*		FindChildByName( C_GadgetBase* parent, const char* pName );
		
		C_GadgetBase*		GetCurrentButton( )								{ if( m_Array.Size() == 0 ) return NULL; return m_Array[ m_Array.Size()-1 ]; }
		const S32			GetCurrentId( )									{ return m_Array.Size()-1; }
		const S32			Size() const									{ return m_Array.Size(); }
		void				RegenerateParentLists();

	private:
		S32					m_DefaultCapacity;
		ButtonArray			m_Array;
		C_ScreenControl*	m_ScreenControl;
	};

public:

	/// Default constructor
	C_ScreenControl( const S32 context );

	void Init();
	void Update();
	void End( const char* = NULL, const S32 ns_context = -1 );
	void Exit();
	void ChangeScreen();
	void Suspend( );
	void Resume( );

	bool MessageHandler(const Messaging::C_MessageBase *pMessage);
	bool DefaultMessageHandler(const Messaging::C_MessageBase *pMessage);

	void AddXtRecord( const char* pScreenName, const bool bForceNewTable = false, const bool bCreateXtElements = true );

	const XtRecord* GetRootRecord()					{ return m_RootRecord; }

	bool InitState() const							{ return m_State == E_State_Init; }
	bool Active() const								{ return m_State != E_State_Inactive && m_State != E_State_Suspended; }
	bool StartLoop() const							{ return m_State == E_State_StartLoop; }
	bool EndLoop() const							{ return m_State == E_State_EndLoop; }
	bool ScreenTransitionActive() const				{ return m_State == E_State_Update ? false : true; }

	void SetDefaultScreenName( const char *name );
	void SetLuaScreenDir(const char *d)				{ STRCPY( m_LuaScreenDir, m_uMaxScriptNameLength, d ); }
	const char *GetLuaScreenDir()	const			{ return m_LuaScreenDir; }

	void SetNextScreen(const char *ns = NULL);
	const char *GetCurrentScreen() const			{ return m_CurrentScreen; }
	const char *GetNextSreen() const				{ return m_NextScreen; }

	void SetInitFunctionName(const char *nf)		{ m_LuaHandler.SetFunctionName( C_LuaMessageHandler::E_ScreenFunc_Init, nf );			}
	void SetPostInitFunctionName(const char *nf)	{ m_LuaHandler.SetFunctionName( C_LuaMessageHandler::E_ScreenFunc_PostInit, nf );		}
	void SetStartLoopName(const char* nf)			{ m_LuaHandler.SetFunctionName( C_LuaMessageHandler::E_ScreenFunc_StartLooop, nf );		}
	void SetFrameUpdateName(const char* nf)			{ m_LuaHandler.SetFunctionName( C_LuaMessageHandler::E_ScreenFunc_FrameUpdate, nf );	}
	void SetMessageUpdateName(const char * nf)		{ m_LuaHandler.SetFunctionName( C_LuaMessageHandler::E_ScreenFunc_MessageUpdate, nf );	}
	void SetEnterEndName(const char * nf)			{ m_LuaHandler.SetFunctionName( C_LuaMessageHandler::E_ScreenFunc_EnterEnd, nf );		}
	void SetEndLoopName(const char * nf)			{ m_LuaHandler.SetFunctionName( C_LuaMessageHandler::E_ScreenFunc_EndLoop, nf );	}
	void SetExitFunctionName(const char *nf)		{ m_LuaHandler.SetFunctionName( C_LuaMessageHandler::E_ScreenFunc_Exit, nf );			}

	void BlockInputToMeDefault( const bool bBlockInputToMeDefault )			{ m_bBlockInputToMeDefault = bBlockInputToMeDefault; m_bBlockInputToMe = m_bBlockInputToMeDefault; }
	void BlockInputToMe( const bool bBlockInputToMe )						{ m_bBlockInputToMe = bBlockInputToMe; }
	bool BlockInputToMe()													{ return m_bBlockInputToMe; }

	// 
	const C_ButtonId	AddButton( C_GadgetBase *obj )	{ m_ButtonList.Add( obj ); return GetCurrentId(); }
	const C_ButtonId	GetCurrentId()					{ return C_ButtonId( m_Context, m_ButtonList.GetCurrentId() ); }
	C_GadgetBase *		GetCurrentButton()				{ return m_ButtonList.GetCurrentButton();  }
	C_GadgetBase *		FindByName( const char *pName )	{ return m_ButtonList.FindByName( pName ); }
	C_GadgetBase *		FindByID( const C_ButtonId id )	{ return m_ButtonList.Find( id ); }
	const S32			FindChildren( const C_GadgetBase *parent, ButtonArray* children, const bool all, const E_UIButtonType type )	 { return m_ButtonList.FindChildren( parent, children, all, type ); }
	C_GadgetBase*		FindChildByName( C_GadgetBase *parent, const char* pName ) { return m_ButtonList.FindChildByName( parent, pName ); }
	const S32			NumberOfButtons() const			{ return m_ButtonList.Size(); }
	UiXt::C_PointerMap&	PointerMap()					{ return m_PointerMap; }

	void SetTimers(F32 start,F32 end)				{ m_StartTimer = start; m_EndTimer = end; }
	const F32 GetStartTime( )						{ return m_StartTimer; }
	const F32 GetEndTime( )							{ return m_EndTimer; }
	const U32 GetMessageMask( )						{ return m_MessageMask; }

	const SCREEN_TIME	GetScreenTime( );
	const bool			DoRender()					{ return !m_RenderEnd; }

	void NoButtonMessages(bool s)					{ m_LuaHandler.AllowButtonMessages( !s ); }
//	bool NoButtonMessages() const					{ return m_NoButtonMessages; }

	bool InvalidStrings() const						{ return m_StringsInvalid; }
	void InvalidateStrings()						{ m_StringsInvalid = true; }

	void											AddLuaSubscriber( const Messaging::TMsgType messageType );

	void				ReloadLuaScripts();
	void				SetEndLoop();
	void				SetEnd();

private:
	void				LuaCall( const int error );

public:
	static const U32	m_uMaxScriptNameLength = 128;
	char				m_CurrentScreen[ m_uMaxScriptNameLength ];		// My Current screen
	char				m_NextScreen[ m_uMaxScriptNameLength ];			// My next screen

	char				m_SwitchScreen[ m_uMaxScriptNameLength ];		// Another contexts' next screen
	S32					m_SwitchContext;

private:
	E_State				m_State;
	E_State				m_PrevState;

	char				m_LuaScreenDir[ m_uMaxScriptNameLength ];
	char				m_DefaultScreenName[ m_uMaxScriptNameLength ];

	F32					m_Timer;
	F32					m_StartTimer;
	F32					m_EndTimer;

	U32					m_MessageMask;
	S32					m_Context;
	S8					m_IgnoreController;

	bool				m_FileReload;
	bool				m_NoButtonMessages;
	bool				m_StringsInvalid;
	bool				m_RenderEnd;
	bool				m_SuspendPending;
	bool				m_EndCurrentScreen;

	C_List				m_ButtonList;
	XtRecord *			m_RootRecord;
	UiXt::C_Allocator	m_Allocator;
	UiXt::C_PointerMap	m_PointerMap;
	C_LuaMessageHandler	m_LuaHandler;

	bool				m_bBlockInputToMeDefault;
	bool				m_bBlockInputToMe;
};

// Container for screen controllers
class C_ScreenControlManager
{
public:
	C_ScreenControlManager( );
	~C_ScreenControlManager( );

	static C_ScreenControlManager&	Instance()						{ return m_sInstance; }

	void							Context( const S32 context );
	const S32						Context( ) const				{ return m_Context; }

	void							ResetContext( const bool reset, const S32 sContext = 0 ){ m_bResetContext = reset; m_sContextReset = sContext; }
	const bool						ResetContext( )					{ return m_bResetContext; }

	const S32						MessageMask( ) const			{ return 1 << m_Context; }
	const S32						InputMessageMask( );
	
	const S32						Add( );
	C_ScreenControl&				Get( const S32 slot );
	const S32						Count( ) const					{ return m_List.Size(); }

	void							Exit( );

public:
	static S32							m_sContextReset;

private:
	static C_ScreenControlManager		m_sInstance;

	S32									m_Context;
	static bool							m_bResetContext;
	
	Btl::Vector< C_ScreenControl * >	m_List;
};

// Legacy
#define gUIScreenControl		C_ScreenControlManager::Instance().Get( 0 )
#define gUIPopupControl			C_ScreenControlManager::Instance().Get( 1 )

#ifndef CFG_GOLD
extern Debug::Bool		g_debugUIShowScreens;
#endif

}; // namespace UI

#endif // C_ScreenControl_H
