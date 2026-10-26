#include "stdafx.h"

#include <UI/UIHelpers.h>
#include <Game/messaging/GameflowMessages.h>

#include "UILocatorNode.h"

// XT Definition
XT_TYPE( UI::UILocatorNode, "UILocatorNode" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_LocationName, "location_name" )
XT_END_TYPE

namespace UI
{

void UILocatorNode::Init()
{
	C_GadgetBase::Init();

	AddSubscriber( E_UiMsg_FrameUpdate );
	AddSubscriber( Game::E_GameflowMessage_ClearStoredActorPtrs );
}

void UILocatorNode::ChangeLocation( const char* pLocationName )
{
	if( !pLocationName )
		return;

	if( m_LocationName )
		m_XtAllocator.Delete( const_cast<char*>( m_LocationName.Text() ) );

	m_LocationName = XtSz8( CopyCreateString( pLocationName ) );

	m_pActor = Actors::UiLocatorActor::FindByName( m_LocationName );
}

void UILocatorNode::PostInit()
{
	m_pActor = Actors::UiLocatorActor::FindByName( m_LocationName );

	C_GadgetBase::PostInit();
}

bool UILocatorNode::MessageHandler(const Messaging::C_MessageBase *pMessage)
{
	switch( pMessage->Type() )
	{
	case Game::E_GameflowMessage_ClearStoredActorPtrs:
		m_pActor = NULL;
		return true;
	}

	return C_GadgetBase::MessageHandler( pMessage );
}


void UILocatorNode::FrameUpdate()
{
	if( m_pActor )
	{
		Maths::HMat43 mat;
		CalcTransformHierarchy( mat );
		SetMatrix(mat * m_pActor->GetMatrix());
	}

	return C_GadgetBase::FrameUpdate();
}

}; // namespace UI