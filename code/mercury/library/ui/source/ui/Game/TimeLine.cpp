#include "stdafx.h"

#include <misc_shared/NewCurveHermite.h>

#include "ui/UIDefines.h"
#include "ui/Game/UIXt.h"
#include "ui/Game/Timeline.h"
#include "ui/Game/Gadgets/ButtonController.h"
#include "ui/Game/Gadgets/ButtonMessages.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

#ifndef CFG_GOLD

Debug::Bool		g_UiTimelineOverideCurrent( "UI.Timeline.OverideCurrent", false );
Debug::Bool		g_UiTimelineShowLuaCommands( "UI.Timeline.ShowLuaCommands", false );
Debug::String	g_UiTimelineOverideName( "UI.Timeline.OverideName", "MAIN" );
Debug::String	g_UiTimelineTimelineMsgString( "UI.Timeline.Message.String", "" );
Debug::Bool		g_UiTimelineTimelineMsgActive( "UI.Timeline.Message.Active", true );
Debug::Float	g_UiTimelineTimelineMsgTime( "UI.Timeline.Message.Time", 0.0 );

static void g_UiTimelineMsgSendFunc(void*)
{
	UI::C_UiTimelineMessage msg( g_UiTimelineTimelineMsgString, g_UiTimelineTimelineMsgActive, g_UiTimelineTimelineMsgTime );
	UI::gButtons.BroadcastMessage( &msg, 1 );
}

Debug::Function	g_UiTimelineTimelineMsgSend( "UI.Timeline.Message.Send", g_UiTimelineMsgSendFunc, NULL );

#endif

// Timeline
XT_TYPE( UI::C_Timeline, "UITimeline" )
	XT_FIELD( m_Name, "label" )
	XT_FIELD( m_bActive, "start_active" )
	XT_FIELD( m_bDenySleep, "deny_sleep" )
	XT_FIELD( m_OnStartCmd, "on_start" )
	XT_FIELD( m_OnRestCmd, "on_rest" )
	XT_FIELD( m_Range, "time" )
	XT_FIELD( m_Type, "type" )
	XT_FIELD( m_RestType, "rest_type" )
	XT_FIELD( m_Effects, "effects" )
	XT_FIELD( m_Events, "events" )
XT_END_TYPE

XT_TYPE( UI::C_Timeline::E_Type, "UITimerType" )
	XT_OPTION( UI::C_Timeline::E_Type_Wrap, "Wrap" )
	XT_OPTION( UI::C_Timeline::E_Type_Bounce, "Bounce" )
	XT_OPTION( UI::C_Timeline::E_Type_Stop, "Stop" )
XT_END_TYPE

XT_TYPE( UI::C_Timeline::E_RestType, "UITimerRestType" )
	XT_OPTION( UI::C_Timeline::E_RestType_None, "None" )
	XT_OPTION( UI::C_Timeline::E_RestType_Start, "Start" )
	XT_OPTION( UI::C_Timeline::E_RestType_End, "End" )
XT_END_TYPE

XT_TYPE( UI::C_Timeline::C_TimeRange, "UITimelineTimeRange" )
	XT_FIELD( start, "start" )
	XT_FIELD( end, "end" )
XT_END_TYPE

// Effects
XT_TYPE( UI::C_Timeline::C_Effect, "UITimelineEffect" )
	XT_FIELD( m_ComponentName, "component" )
	XT_FIELD( m_pAmounts, "amount" )
	XT_FIELD( m_KeyFrames, "key" )
XT_END_TYPE

// Ranges
XT_TYPE( UI::C_Timeline::C_Effect::I_Range, "UITimelineRangeBase" )
XT_END_TYPE

XT_TYPE( UI::C_Timeline::C_Effect::C_Range< XtF32 >, "UIRangeNumber" )
	XT_BASE( UI::C_Timeline::C_Effect::I_Range )
	XT_CALLBACK( UI::UiXt::XtReferenceRecordCB< UI::C_Timeline::C_Effect::C_Range< XtF32 > > )
	XT_FIELD( m_From, "from" )
	XT_FIELD( m_To, "to" )
XT_END_TYPE

XT_TYPE( UI::C_Timeline::C_Effect::C_Array< XtF32 >, "UIArrayNumber" )
	XT_BASE( UI::C_Timeline::C_Effect::I_Range )
	XT_CALLBACK( UI::UiXt::XtReferenceRecordCB< UI::C_Timeline::C_Effect::C_Array< XtF32 > > )
	XT_FIELD( m_Items, "items" )
XT_END_TYPE

XT_TYPE( UI::C_Timeline::C_Effect::C_Array< XtSz8 >, "UIArrayString" )
	XT_BASE( UI::C_Timeline::C_Effect::I_Range )
	XT_CALLBACK( UI::UiXt::XtReferenceRecordCB< UI::C_Timeline::C_Effect::C_Array< XtSz8 > > )
	XT_FIELD( m_Items, "items" )
XT_END_TYPE

XT_TYPE( UI::C_Timeline::C_Effect::C_Array< XtLocId >, "UIArrayLoc" )
	XT_BASE( UI::C_Timeline::C_Effect::I_Range )
	XT_CALLBACK( UI::UiXt::XtReferenceRecordCB< UI::C_Timeline::C_Effect::C_Array< XtLocId > > )
	XT_FIELD( m_Items, "items" )
XT_END_TYPE

// Events
XT_TYPE( UI::C_Timeline::C_Event, "UITimelineEvent" )
	XT_FIELD( m_Command, "command" )
	XT_FIELD( m_Time, "time" )
XT_END_TYPE

namespace UI
{

// Timeline
void C_Timeline::BindImpl( XtType* def, const U8* structPtr )
{
	// Safegaurd
	if( m_Direction.Mode() != C_Direction::E_Mode_None )
	{
		LOGERROR( g_Log, DBSTR " : Timeline already bound");
		return;
	}

	for( XtI32 i = 0; i < m_Effects.count; i++)
	{
		m_Effects[ i ].Bind( def, structPtr );
	}

	m_Direction.Mode( m_bActive ? C_Direction::E_Mode_Inc : C_Direction::E_Mode_PauseInc );

	// Move to the start - under this odd set of circumstances
	if( !m_bActive && m_RestType == E_RestType_Start )
	{
		for( XtI32 i = 0; i < m_Effects.count; i++)
			m_Effects[ i ].Reset( );

		for( XtI32 i = 0; i < m_Effects.count; i++)
			m_Effects[ i ].Apply( 0.0f );
	}
}

void C_Timeline::Reset( const bool overide )
{
	// Check for sleeping effects
	if( !overide && IsSleeping() )
		return;

	// Reset to origins
	for( XtI32 i = 0; i < m_Effects.count; i++)
		m_Effects[ i ].Reset( );
}

const bool C_Timeline::Active( )
{
#ifndef CFG_GOLD
	if ( g_UiTimelineOverideCurrent && m_Name && m_Name[ 0 ] )
	{
		return ( !stricmp( m_Name, g_UiTimelineOverideName ) );
	}
#endif
	
	return m_bActive;
}

void C_Timeline::Active( const bool bActive )
{ 
	m_bActive = bActive;
	Reset();

	if( m_bActive )
	{	
		// Grr - The go_backwards->resume_forwards case
		if( m_RestType == E_RestType_Start && m_Type != E_Type_Bounce && m_Direction.Mode() == C_Direction::E_Mode_Dec )
			m_Direction.Mode( C_Direction::E_Mode_Inc );
		else
			m_Direction.Resume();
	}
	else
	{
		if( m_RestType == E_RestType_None )	// Straight to sleep
			m_Direction.Pause();
		else
		if( m_RestType == E_RestType_Start )	// We've been switched off but need to run towards our rest point
		{
			if( m_Time > m_Range.start )
				m_Direction.Mode( C_Direction::E_Mode_Dec );
			else
				m_Direction.Pause();
		}
	}
}

void C_Timeline::SetTime( const F32 _newtime )
{
	m_Time = 0.0f;

	// Run forwards
	C_Direction dir( C_Direction::E_Mode_Inc );
	UpdateImpl( _newtime, dir, true );

	// Set current direction if active
	if( Active() )
		m_Direction = dir;
}

const bool C_Timeline::IsSleeping()
{
	if( m_bDenySleep )
		return false;

	if( m_Direction.IsPaused() )
		return true;

	if( !Active() && m_RestType == E_RestType_None )
		return true;

	return false;
}

void C_Timeline::Update( const F32 time )
{
	UpdateImpl( time, m_Direction, false );
}

void C_Timeline::UpdateImpl( F32 _time, C_Timeline::C_Direction& _dir, bool _set_time )
{
	// Check for sleeping effects
	if( IsSleeping() )
		return;

	const bool active = Active();
	const bool rest_start = (!active && m_RestType == E_RestType_Start );
	const bool rest_end = (!active && m_RestType == E_RestType_End );
	bool loop = false;
	F32 time = _time;

	const bool before_start = ( m_Time <= m_Range.start );
	const C_Direction prev_dir = _dir;

	bool run_once = _set_time;

	// Update time
	while( ( time > 0.0f && m_Range.Length() > 0.0f ) || run_once )
	{
		const F32 prevTime = m_Time;

		m_Time += (_dir.Amount() * time);

		// Check for events
		if( !_set_time )
		{
			if( _dir.Amount() > 0 )
			{
				for( XtI32 i = 0; i < m_Events.count; i++ )
				{
					// Crosed out event horizon err boundary?
					const F32 t0 = m_Events[ i ].m_Time * m_Range.Length();
					if( t0 <= prevTime || t0 > m_Time )
						continue;

					DoLuaCommand( m_Events[ i ].m_Command );
				}
			}
			else
			if( _dir.Amount() < 0 )
			{
				for( XtI32 i = m_Events.count-1; i >= 0; i-- )
				{
					const F32 t0 = m_Events[ i ].m_Time * m_Range.Length();
					if( t0 > prevTime || t0 <= m_Time )
						continue;

					DoLuaCommand( m_Events[ i ].m_Command );
				}
			}
		}

		// Process new time
		if( m_Time >= m_Range.end && ( _dir.Amount() > 0 || loop ) )
		{
			switch( m_Type )
			{
			case E_Type_Wrap :
				if( rest_start )
				{
					m_Time = m_Range.start;
					_dir.Pause();
					time = 0.0f;
				}
				else
				if( rest_end )
				{
					m_Time = m_Range.end;
					_dir.Pause();
					time = 0.0f;
				}
				else
				{
					time = (m_Time - m_Range.end);
					m_Time = m_Range.start;
					loop = true;
				}
				break;

			case E_Type_Bounce :
				if( rest_end )
				{
					m_Time = m_Range.end;
					_dir.Pause();
					time = 0.0f;
				}
				else
				{
					_dir.Reverse();
					time = (m_Time - m_Range.end);
					m_Time = m_Range.end;
					loop = true;
				}
				break;

			case E_Type_Stop :
				m_Time = m_Range.end;
				time = 0.0f;
				_dir.Pause();
				break;
			}
		}
		else
		if( m_Time <= m_Range.start && (_dir.Amount() < 0 || loop) )
		{
			switch( m_Type )
			{
				case E_Type_Wrap :
					if( rest_start )
					{
						m_Time = m_Range.start;
						_dir.Pause();
						time = 0.0f;
					}
					else
					if( rest_end )
					{
						m_Time = m_Range.end;
						_dir.Pause();
						time = 0.0f;
					}
					else
					{
						time = (m_Time - m_Range.start);
						m_Time = m_Range.end;
						loop = true;
					}
					break;

				case E_Type_Bounce :
					if( rest_start )
					{
						m_Time = m_Range.start;
						_dir.Pause();
						time = 0.0f;
					}
					else
					{
						_dir.Reverse();
						time = (m_Range.start - m_Time);
						m_Time = m_Range.start;
						loop = true;
					}
					break;

				case E_Type_Stop :
					m_Time = m_Range.start;
					time = 0.0f;
					_dir.Reverse(); // Make sure we go in the correct direction if we're ever Resumed()
					_dir.Pause();
					break;
			}
		}
		else
		{
			time = 0.0f;
			break;
		}

		run_once = false;
	}

	// Get fraction along length
	F32 delta = 0.0f;
	if( m_Range.Length() > 0.0f )
	{
		delta = Maths::Clamped< F32 >( (m_Time - m_Range.start) / m_Range.Length(), 0.0f, 1.0f );
	}

	// Apply effects
	for( XtI32 i = 0; i < m_Effects.count; i++)
	{
		m_Effects[ i ].Apply( delta );
	}

	if( !_set_time )
	{
		// Start command (only works if there range.start is > 0)
		if( before_start && m_Time >= m_Range.start )
			DoLuaCommand( m_OnStartCmd );

		// end command
		if ( _dir.IsPaused() == true && prev_dir.IsPaused() == false )
			DoLuaCommand( m_OnRestCmd );
	}

	return;
}

void C_Timeline::DoLuaCommand( const XtSz8& cmd )
{
	if( !cmd )
		return;
	if( !cmd[ 0 ] )
		return;

#ifndef CFG_GOLD
	if( g_UiTimelineShowLuaCommands )
		LOGDEBUG( g_Log, DBSTR  "%s -> [%s]", m_Name.Text(), cmd.Text() );
#endif

	C_UiLuaCommandMessage msg( cmd );
	UI::gButtons.BroadcastMessage( &msg, 1 );
}

// Effect
C_Timeline::C_Effect::C_Effect() :
	m_ComponentName( XtSz8(NULL) ), 
	m_pAmounts( NULL )
{
	m_pCurveData = new C_UIXtHermiteProxy();

	Curve::C_HermiteSplineFlags flags;
	//Set to none, as we secretly handle u values within the xt proxy.
	flags.calcKnotU = Curve::E_ProcessU_None;
	flags.calcTangents = false;
	m_pCurveEval = new C_UIXtHermite1D(flags);
	m_pCurveEval->SetData(m_pCurveData);
}

C_Timeline::C_Effect::~C_Effect()
{
	for( S32 i = 0; i < m_Modifiers.Size(); i++ )
		SAFE_DELETE_PTR( m_Modifiers[i] );

	SAFE_DELETE_PTR(m_pCurveEval);
	SAFE_DELETE_PTR(m_pCurveData);
}

void C_Timeline::C_Effect::Bind( XtType* def, const U8 * structPtr )
{
	//Initialise curve data
	m_pCurveData->SetData(&m_KeyFrames);
	//Initialise curve evaluator.
	m_pCurveEval->Process();

	// Resolve the component name
	if( m_ComponentName && m_ComponentName[ 0 ] && m_pAmounts )
	{
		char tempBuffer[256 + 1];
		const char *p = m_ComponentName;

		// Copy next 
		U32 i;
		while( *p )
		{
			for( i = 0; i < 256 && *p != ';' && *p != NULL; i++, p++ )
			{
				tempBuffer[ i ] = *p;
			}
			tempBuffer[ i ] = NULL;
			// Skip component seperator
			while( *p == ';' || *p == ' ' )
				p++;

			// Find the address of this varianble
			UiXt::C_MemberAddrLink link;
			UiXt::FindMemberAddrImpl( def, structPtr, tempBuffer, &link );

			// Get last link and store the last valid callback
			UiXt::C_MemberAddrLink *l = &link;
			XtEventCallback cb = NULL;
			void *cb_base = NULL;
			for( ; l && l->m_pNext; l = l->m_pNext )
			{
				if( l->m_pDef->event )
				{
					cb = l->m_pDef->event;
					cb_base = l->m_pComponent;
				}
			}
			if( l && l->m_pComponent )
			{
				C_HasChanged hasChanged;
				if( cb )
				{
					UiXt::C_CallbackAssignContext ctx( l );
					cb( kXtEtUser, NULL, NULL, &ctx );

					if( ctx.tag != UiXt::E_CallbackContext_Setup )
					{
						hasChanged.callback = cb;
						hasChanged.base = cb_base;
						hasChanged.type = ctx.tag;
					}
				}

				switch( m_pAmounts->Type() )
				{
				// Interpolate between two numbers
				case I_Range::E_Type_Lerp:
					switch( l->m_pDef->atom )
					{
					case kXtI8:		m_Modifiers.PushBack( new C_ModifierLerp< XtF32, XtI8 >( *( (C_Range< XtF32 > *)m_pAmounts ), (XtI8*)l->m_pComponent, hasChanged ) );	break;
					case kXtU8:		m_Modifiers.PushBack( new C_ModifierLerp< XtF32, XtU8 >( *( (C_Range< XtF32 > *)m_pAmounts ), (XtU8*)l->m_pComponent, hasChanged ) );	break;
					case kXtI16:	m_Modifiers.PushBack( new C_ModifierLerp< XtF32, XtI16 >( *( (C_Range< XtF32 > *)m_pAmounts ), (XtI16*)l->m_pComponent, hasChanged ) );	break;
					case kXtU16:	m_Modifiers.PushBack( new C_ModifierLerp< XtF32, XtU16 >( *( (C_Range< XtF32 > *)m_pAmounts ), (XtU16*)l->m_pComponent, hasChanged ) );	break;
					case kXtI32:	m_Modifiers.PushBack( new C_ModifierLerp< XtF32, XtI32 >( *( (C_Range< XtF32 > *)m_pAmounts ), (XtI32*)l->m_pComponent, hasChanged ) );	break;
					case kXtU32:	m_Modifiers.PushBack( new C_ModifierLerp< XtF32, XtU32 >( *( (C_Range< XtF32 > *)m_pAmounts ), (XtU32*)l->m_pComponent, hasChanged ) );	break;
					case kXtF32:	m_Modifiers.PushBack( new C_ModifierLerp< XtF32, XtF32 >( *( (C_Range< XtF32 > *)m_pAmounts ), (XtF32*)l->m_pComponent, hasChanged ) );	break;
					default:
						LOGWARN( g_Log, DBSTR "Type %d not supported for interpolation", l->m_pDef->behaviour );
						break;
					}
					break;
				// Switch between elements in an array
				case I_Range::E_Type_Range:
					switch( l->m_pDef->atom )
					{
					// Cast
					case kXtI8:		m_Modifiers.PushBack( new C_ModifierArray< XtF32, XtI8 >( *( (C_Array< XtF32 > *)m_pAmounts ), (XtI8*)l->m_pComponent, hasChanged ) );	break;
					case kXtU8:		m_Modifiers.PushBack( new C_ModifierArray< XtF32, XtU8 >( *( (C_Array< XtF32 > *)m_pAmounts ), (XtU8*)l->m_pComponent, hasChanged ) );	break;
					case kXtI16:	m_Modifiers.PushBack( new C_ModifierArray< XtF32, XtI16 >( *( (C_Array< XtF32 > *)m_pAmounts ), (XtI16*)l->m_pComponent, hasChanged ) );	break;
					case kXtU16:	m_Modifiers.PushBack( new C_ModifierArray< XtF32, XtU16 >( *( (C_Array< XtF32 > *)m_pAmounts ), (XtU16*)l->m_pComponent, hasChanged ) );	break;
					case kXtI32:	m_Modifiers.PushBack( new C_ModifierArray< XtF32, XtI32 >( *( (C_Array< XtF32 > *)m_pAmounts ), (XtI32*)l->m_pComponent, hasChanged ) );	break;
					case kXtU32:	m_Modifiers.PushBack( new C_ModifierArray< XtF32, XtU32 >( *( (C_Array< XtF32 > *)m_pAmounts ), (XtU32*)l->m_pComponent, hasChanged ) );	break;
					case kXtF32:	m_Modifiers.PushBack( new C_ModifierArray< XtF32, XtF32 >( *( (C_Array< XtF32 > *)m_pAmounts ), (XtF32*)l->m_pComponent, hasChanged ) );	break;
					// Specialised
					case kXtSz8:	m_Modifiers.PushBack( new C_ModifierArray< XtSz8, XtSz8 >( *( (C_Array< XtSz8 > *)m_pAmounts ), (XtSz8*)l->m_pComponent, hasChanged ) );			break;
					case kXtLocId:	m_Modifiers.PushBack( new C_ModifierArray< XtLocId, XtLocId >( *( (C_Array< XtLocId > *)m_pAmounts ), (XtLocId*)l->m_pComponent, hasChanged ) );	break;
					default:
						LOGWARN( g_Log, DBSTR "Type %d not supported for array switching", l->m_pDef->behaviour );
						break;
					}
					break;
				}
			}
		}
	}
}

/// Apply all modifiers
void C_Timeline::C_Effect::Apply( const F32 delta )
{
	F32 d = (m_KeyFrames.count > 0) ? m_pCurveEval->Evaluate( delta ) : delta;

	for( S32 i = 0; i < m_Modifiers.Size(); i++ )
	{
		assert( m_Modifiers[ i ] );
		m_Modifiers[ i ]->Apply( d );
	}
}

F32 C_Timeline::C_Effect::Evaluate( const F32 delta )
{
	return m_pCurveEval ? m_pCurveEval->Evaluate( delta ) : 0.0f;
}

// Lerp between two numbers
template < typename _RangeType, typename _ModType >
void C_Timeline::C_Effect::C_ModifierLerp< _RangeType, _ModType >::Apply( const F32 d )
{
	if( C_Modifier< _ModType >::m_pComponent == NULL )
		return;

	const _RangeType f = m_Range.m_From + ((m_Range.m_To - m_Range.m_From) * d);

	*C_Modifier< _ModType >::m_pComponent += ( _ModType )f;

	if( C_Modifier< _ModType >::m_HasChanged.callback )
	{
		XtCallbackContext ctx( C_Modifier< _ModType >::m_HasChanged.type );
		C_Modifier< _ModType >::m_HasChanged.callback( kXtEtUser, NULL, C_Modifier< _ModType >::m_HasChanged.base, &ctx );
	}
}

// Switch between an array of numbers
template < typename _RangeType, typename _ModType >
void C_Timeline::C_Effect::C_ModifierArray< _RangeType, _ModType >::Apply( const F32 d )
{
	if( C_Modifier< _ModType >::m_pComponent == NULL || m_Array.m_Items.count <= 0 )
		return;

	S32 n = (S32)Maths::Clamp< F32 >( d * (F32)m_Array.m_Items.count, 0.0f, (F32)m_Array.m_Items.count-1.0f  );

	const _RangeType f = m_Array.m_Items[ n ];

	*C_Modifier< _ModType >::m_pComponent += ( _ModType )f;

	if( C_Modifier< _ModType >::m_HasChanged.callback )
	{
		XtCallbackContext ctx( C_Modifier< _ModType >::m_HasChanged.type );
		C_Modifier< _ModType >::m_HasChanged.callback( kXtEtUser, NULL, C_Modifier< _ModType >::m_HasChanged.base, &ctx );
	}
}

template < >
void C_Timeline::C_Effect::C_Modifier< XtSz8 >::Reset( const bool overide )
{
	// Dont reset because we're not offseting our values
}

template < >
void C_Timeline::C_Effect::C_Modifier< XtLocId >::Reset( const bool overide )
{
	// Dont reset because we're not offseting our values
}

template < >
void C_Timeline::C_Effect::C_ModifierArray< XtSz8, XtSz8 >::Apply( const F32 d )
{
	if( m_pComponent == NULL || m_Array.m_Items.count <= 0 )
		return;

	S32 n = (S32)Maths::Clamp< F32 >( d * (F32)m_Array.m_Items.count, 0.0f, (F32)m_Array.m_Items.count-1.0f  );

	// Has changed? Inform the callback object
	bool hasChanged = ( *m_pComponent != m_Array.m_Items[ n ] );

	*m_pComponent = m_Array.m_Items[ n ];

	if( hasChanged && m_HasChanged.callback )
	{
		XtCallbackContext ctx( m_HasChanged.type );
		m_HasChanged.callback( kXtEtUser, NULL, m_HasChanged.base, &ctx );
	}
}

template < >
void C_Timeline::C_Effect::C_ModifierArray< XtLocId, XtLocId >::Apply( const F32 d )
{
	if( m_pComponent == NULL || m_Array.m_Items.count <= 0 )
		return;

	S32 n = (S32)Maths::Clamp< F32 >( d * m_Array.m_Items.count, 0.0f, (F32)m_Array.m_Items.count-1.0f  );

	// Has changed? Inform the callback object
	bool hasChanged = ( m_pComponent->id != m_Array.m_Items[ n ].id );
		
	*m_pComponent = m_Array.m_Items[ n ];

	if( hasChanged && m_HasChanged.callback )
	{
		XtCallbackContext ctx( m_HasChanged.type );
		m_HasChanged.callback( kXtEtUser, NULL, m_HasChanged.base, &ctx );
	}
}

// Direction mini class
const F32 C_Timeline::C_Direction::Amount() const
{
	switch( m_Mode )
	{
	case E_Mode_Inc:
		return 1.0f;

	case E_Mode_Dec:
		return -1.0f;

	case E_Mode_PauseInc:
	case E_Mode_PauseDec:
		return 0.0f;

	default:
		LOGERROR( g_Log, DBSTR " invalid mode %d - tell UI team to fix this please", m_Mode );
		break;
	}

	return 0.0f;
}

const bool C_Timeline::C_Direction::IsPaused() const
{
	switch( m_Mode )
	{
	case E_Mode_Inc:
	case E_Mode_Dec:
		return false;

	case E_Mode_PauseInc:
	case E_Mode_PauseDec:
		return true;

	default:
		LOGERROR( g_Log, DBSTR " invalid mode %d - tell UI team to fix this please", m_Mode );
		break;
	}

	return false;
}

const C_Timeline::C_Direction::E_Mode C_Timeline::C_Direction::Mode( ) const
{
	return m_Mode;
}

void C_Timeline::C_Direction::Pause( )
{
	switch( m_Mode )
	{
	case E_Mode_Inc:
		m_Mode = E_Mode_PauseInc;
		break;

	case E_Mode_Dec:
		m_Mode = E_Mode_PauseDec;
		break;

	case E_Mode_PauseInc:
	case E_Mode_PauseDec:
		break;

	default:
		LOGERROR( g_Log, DBSTR " invalid mode %d - tell UI team to fix this please", m_Mode );
		break;
	}
}

void C_Timeline::C_Direction::Resume( )
{
	switch( m_Mode )
	{
	case E_Mode_Inc:
	case E_Mode_Dec:
		break;

	case E_Mode_PauseInc:
		m_Mode = E_Mode_Inc;
		break;

	case E_Mode_PauseDec:
		m_Mode = E_Mode_Dec;
		break;

	default:
		LOGERROR( g_Log, DBSTR " invalid mode %d - tell UI team to fix this please", m_Mode );
		break;
	}
}

void C_Timeline::C_Direction::Reverse()
{
	switch( m_Mode )
	{
	case E_Mode_Inc:
		m_Mode = E_Mode_Dec;
		break;

	case E_Mode_Dec:
		m_Mode = E_Mode_Inc;
		break;

	case E_Mode_PauseInc:
		m_Mode = E_Mode_PauseDec;
		break;

	case E_Mode_PauseDec:
		m_Mode = E_Mode_PauseInc;
		break;

	default:
		LOGERROR( g_Log, DBSTR " invalid mode %d - tell UI team to fix this please", m_Mode );
		break;
	}
}

void C_Timeline::C_Direction::Mode( const C_Timeline::C_Direction::E_Mode _mode )
{
	m_Mode = _mode;
}

} // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on)
#endif
