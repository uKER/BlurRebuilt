#include "stdafx.h"

#include <UI/UIHelpers.h>

#include "UICarNode.h"
#include "Game/GameActors/VehicleLogic/VehicleLogicActor.h"

// XT Definition
XT_TYPE( UI::UICarNode, "UICarNode" )
	XT_BASE( UI::C_GadgetBase )
XT_END_TYPE

namespace UI
{

void UICarNode::Init()
{
	m_sLocalPlayerIndex = -1;
	m_pLogicActor = NULL;

	AddSubscriber( E_UiMsg_FrameUpdate );
	AddSubscriber( E_UiGameMsg_SetLocalPlayerIndex );

	C_GadgetBase::Init();
}

bool UICarNode::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	switch( pMessage->Type() )
	{
	case E_UiGameMsg_SetLocalPlayerIndex:
		if ( m_sLocalPlayerIndex == -1 )
		{
			m_sLocalPlayerIndex = (S32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];
			m_Panel = E_GameSpritePanel_VP0 + m_sLocalPlayerIndex;

			m_pLogicActor = GetVehicleLogicActor( m_sLocalPlayerIndex );
			if( !m_pLogicActor )
				return true;

			return true;
		}
	}

	return C_GadgetBase::MessageHandler( pMessage );
}

void UICarNode::FrameUpdate()
{
	if( m_pLogicActor )
	{
		Maths::HMat43 mat;
		CalcTransformHierarchy( mat );

		SetMatrix(mat * m_pLogicActor->GetWorldMatrix());
#if PLATFORM_PC
		// [Blur2PC] render interpolation: this node and its children (the race start countdown) are placed at
		// the car's update pose; each rendered frame moves them with the car to the pose it is drawn at
		if( Game::Actors::C_VehicleActor* pVehicleActor = m_pLogicActor->GetGraphicalActor( m_sLocalPlayerIndex ) )
			gSimple2D.Blur2PCSetAnchor( this, m_sLocalPlayerIndex, true, pVehicleActor->Blur2PCRenderMatrix() );
#endif
	}

	return C_GadgetBase::FrameUpdate();
}

}; // namespace UI