#include "stdafx.h"

#include "UIMinorScoreLog.h"
#include "UiMessages.h"
#include "UiHelpers.h"

#include <ui/Graphics/SimpleTimer.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )
#endif

XT_TYPE( UI::UIMinorScoreLog, "UIMinorScoreLog" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_FontStylePasses, "styles" )
	XT_FIELD( m_fTimerShow, "timer_show" )
	XT_FIELD( m_fTimerFade, "timer_fade" )
	XT_FIELD( m_sMaxItems, "max_items" )
	XT_FIELD( m_xtColourMajor, "colour_major" )
XT_END_TYPE

namespace UI
{

// ---------------
// UIMinorScoreLog
// ---------------
UIMinorScoreLog::~UIMinorScoreLog()
{
	while ( m_Log.Size() != 0 )
	{
		SAFE_DELETE_PTR( m_Log.Front() );
		m_Log.PopFront();
	}
}

void UIMinorScoreLog::Init()
{
	C_GadgetBase::Init();

	m_fTimerShow = 3.0f;
	m_fTimerFade = 0.25f;
	m_sMaxItems = 5;

	m_sLocalPlayerIndex = -1;
	m_sVehicleIndex = -1;

	// messages
	AddSubscriber( E_UiMsg_FrameUpdate );
	AddSubscriber( E_UiGameMsg_SetLocalPlayerIndex );
	AddSubscriber( E_UiGameMsg_ScoringEventMinor );
}

void UIMinorScoreLog::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	// resolve font styles
	m_FontStylePasses.Setup( pState );

	// resolve colour styles
	m_ColourMajor.SetByName( m_xtColourMajor );
}

bool UIMinorScoreLog::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( !m_Activated )
		return C_GadgetBase::MessageHandler( pMessage );

	switch ( pMessage->Type() )
	{
		case E_UiGameMsg_SetLocalPlayerIndex:
			if ( m_sLocalPlayerIndex == -1 )
			{
				m_sLocalPlayerIndex = (S32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];
				m_sVehicleIndex = GetPlayerVehicleIndex( m_sLocalPlayerIndex );
			}
			break;

		case E_UiGameMsg_ScoringEventMinor:
			ProcessMinor( pMessage );
			break;

		default:
			return C_GadgetBase::MessageHandler( pMessage );
	}

	return true;
}

void UIMinorScoreLog::FrameUpdate()
{
	C_GadgetBase::FrameUpdate();

	F32 fFrameTime = theFETimer.FrameTime();
	F32 fTotalTime = ( m_fTimerShow + ( m_fTimerFade * 2.0f ) );

	S32 sCount = 0;
	S32 sSize = m_Log.Size();
	S32 sLimit = sSize - m_sMaxItems;

	for ( Btl::List< C_UIMinorScoreMessage* >::Iterator it = m_Log.Begin(); it != m_Log.End(); ++it )
	{
		F32 fFadeOutTime01 = 1.0f - ( m_fTimerFade / fTotalTime );

		if ( ( sSize > m_sMaxItems ) && ( sCount++ < sLimit ) && ( (*it)->m_fTime01 < fFadeOutTime01 ) )
			(*it)->m_fTime01 = fFadeOutTime01;	// fade off straight away
		else
			(*it)->m_fTime01 += ( ( 1.0f / fTotalTime ) * fFrameTime );
	}

	// remove timeouts
	while ( ( m_Log.Size() != 0 ) && ( m_Log.Front()->m_fTime01 >= 1.0f ) )
	{
		SAFE_DELETE_PTR( m_Log.Front() );
		m_Log.PopFront();
	}
}

void UIMinorScoreLog::Render()
{
	if ( !m_Activated )
		return;

	if ( m_Log.Empty() )
		return;

	// auto-transform
	C_UIAutoTransform autoTransform( this );

	C_GadgetBase::Render();

	F32 y = m_Position.y;
	C_UIColour colBase = m_SpriteColours[ 0 ];
	C_UIColour colBaseMajor = m_ColourMajor[ 0 ];
	colBaseMajor.A( colBase.A() );

	F32 fTotalTime = m_fTimerShow + ( m_fTimerFade * 2.0f );
	F32 fFadeInTime01 = m_fTimerFade / fTotalTime;

	for ( Btl::List< C_UIMinorScoreMessage* >::Iterator it = m_Log.Begin(); it != m_Log.End(); ++it )
	{
		F32 fTime01 = (*it)->m_fTime01;

		C_UIColour col = (*it)->m_bIsMajor ? colBaseMajor : colBase;

		if ( fTime01 < fFadeInTime01 )
			col.A01( col.A01() * ( fTime01 / fFadeInTime01 ) );
		else if ( fTime01 > ( 1.0f - fFadeInTime01 ) )
			col.A01( col.A01() * Maths::Clamp01( 1.0f - ( ( fTime01 - ( 1.0f - fFadeInTime01 ) ) / fFadeInTime01 ) ) );

		if ( ( it == m_Log.Begin() ) && ( fTime01 > ( 1.0f - fFadeInTime01 ) ) )
			y -= ( m_Size.y * ( ( fTime01 - ( 1.0f - fFadeInTime01 ) ) / fFadeInTime01 ) ) * Up();

		gFont.DrawText( m_Layer, Maths::HVec3( m_Position.x, y, m_Position.z ), m_Justify, (*it)->m_String.Chars(), 
			col, m_Size.y, m_FontStylePasses, m_Size.x, m_Panel );

		y += m_Size.y * Up();
	}
}

void UIMinorScoreLog::ProcessMinor( const Messaging::C_MessageBase* pMessage )
{
	const C_UiScoringEventMinor* pMessageMinor = Messaging::MessageCast< C_UiScoringEventMinor >( pMessage );

	if ( ( pMessageMinor->m_VehicleIndex != m_sVehicleIndex ) || ( pMessageMinor->m_MinorString.Length() == 0 ) )
		return;

	C_UIMinorScoreMessage* pLogMessage = new C_UIMinorScoreMessage;

	if ( !pLogMessage )
		return;

	pLogMessage->m_String = pMessageMinor->m_MinorString;
	pLogMessage->m_bIsMajor = pMessageMinor->m_bWasMajor;

	m_Log.PushBack( pLogMessage );
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", on )
#endif
