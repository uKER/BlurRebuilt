#include "stdafx.h"
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>	// [Blur2PC]

#include <ui/Graphics/SimpleTimer.h>

#include "UIHudDummy.h"
#include "UiHelpers.h"
#include "UIMessages.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

XT_TYPE( UI::UIHudDummy, "UIHudDummy" )
	XT_BASE( UI::C_GadgetBase )
XT_END_TYPE

namespace Game
{
extern Debug::Float gUIDebugViewportScale;
extern Debug::Float gUIDebugViewportScaleClamp;
extern Debug::Float gUIDebugViewportPosX;
extern Debug::Float gUIDebugViewportPosY;
extern Debug::Float gUIDebugViewportAngle;
}

namespace UI
{

// --------
// UIHudDummy
// --------
void UIHudDummy::Init()
{
	m_pVehicle = NULL;

	AddSubscriber( E_UiMsg_FrameUpdate );
	AddSubscriber( E_UiGameMsg_SetLocalPlayerIndex );
	RemoveSubscriber( E_UiMsg_Render );
}

void UIHudDummy::PostInit()
{
	m_OffsetPosX	= 0;
	m_OffsetPosY	= 0;
	m_OffsetScale	= 0;
	m_OffsetAngle	= 0;

	m_OriginX		= m_Position.x;	
	m_OriginY		= m_Position.y;
	m_OriginScale	= m_Scale.x;
	m_OriginAngle	= m_Rotation.z;

}

bool UIHudDummy::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( !m_Activated )
		return C_GadgetBase::MessageHandler( pMessage );

	switch ( pMessage->Type() )
	{
		case E_UiGameMsg_SetLocalPlayerIndex:
			if( !m_pVehicle )
			{
				S32 sLocalPlayerIndex = (S32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];

				Game::Actors::RaceLogicCore* pCore = GetRLC();
				if( !pCore )
					return true;
				
				S32 sVehicleIndex = pCore->GetViewportFocusVehicleIndex( sLocalPlayerIndex );
				Game::Actors::C_VehicleActor* pVehicleActor = (Game::Actors::C_VehicleActor*)pCore->GetVehicleActor( sVehicleIndex );
				
				

				if ( !pVehicleActor || !pVehicleActor->GetVehiclePhysics() )
					return true;

				m_pVehicle = pVehicleActor->GetVehiclePhysics();
			}
			break;

		default:
			return C_GadgetBase::MessageHandler( pMessage );
	}

	return true;
}


void UIHudDummy::FrameUpdate()
{
	if ( !m_Activated )
		return;

	if( !m_pVehicle )
		return;

	const F32 time = 0.1f * theFETimer.FrameTime() * 30.0f;

	Maths::HMat43 mat = RacingPhysics::ConvertHVec( m_pVehicle->GetAxis() );
	Maths::HVec3  acc = RacingPhysics::ConvertHVec( m_pVehicle->Acceleration() );
	Maths::HVec3  ang = RacingPhysics::ConvertHVec( m_pVehicle->AngularAcceleration() );

	mat.AW( Maths::HVec3( 0.0, 0.0, 0.0f ) );
	mat = mat.InverseRotTrans();

	acc = acc * mat;
	ang = ang * mat;

	acc.X( Maths::Clamp<F32>( acc.X(), -16.0f, 16.0f ) );
	acc.Y( Maths::Clamp<F32>( acc.Y(), -16.0f, 16.0f ) );
	ang.Z( Maths::Clamp<F32>( ang.Z(), -8.0f, 8.0f ) );

	F32 px( acc.X() * Game::gUIDebugViewportPosX );
	F32 py( acc.Y() * Game::gUIDebugViewportPosY );
	F32 angle( ang.Z() * Game::gUIDebugViewportAngle );
	F32 scale = ( ( acc.Z() - 0.5f ) * Game::gUIDebugViewportScale );
	scale = Maths::Clamp<F32>( scale, -Game::gUIDebugViewportScaleClamp, Game::gUIDebugViewportScaleClamp );

	F32 prevPosX	= m_OffsetPosX;
	F32 prevPosY	= m_OffsetPosY;
	F32 prevScale	= m_OffsetScale;
	F32 prevAngle	= m_OffsetAngle;

	m_OffsetPosX	+= ((px - m_OffsetPosX) * time);
	m_OffsetPosY	+= ((py - m_OffsetPosY) * time);
	m_OffsetScale	+= ((scale - m_OffsetScale) * time);
	m_OffsetAngle	+= ((angle - m_OffsetAngle) * time);

	m_Position.x	= m_OriginX		+ (m_OffsetPosX * m_Size.z);
	m_Position.y	= m_OriginY		+ (m_OffsetPosY * m_Size.z * Up());
	m_Scale.x		= m_OriginScale	+ m_OffsetScale;
	m_Rotation.z	= m_OriginAngle	+ (m_OffsetAngle * Up());

	m_Scale.y		= m_Scale.x;
}

}; // namespace UI
