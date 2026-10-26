#include "stdafx.h"
#include "UI/Game/Gadgets/UITable.h"
#include "UI/Game/Gadgets/UINodeList.h"
#include "UI/Game/Gadgets/ButtonMessages.h"	// [Blur2PC] E_UiMsg_CtrlButton_Up

#include <Ui/UiInterface.h>

// XT Definition
XT_TYPE( UI::UITable, "UITable" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_RowPadding,		"row_padding" )
	XT_FIELD( m_ColumnPadding,	"col_padding" )
XT_END_TYPE

namespace UI
{

void UITable::Init()
{
	C_GadgetBase::Init();

	m_Pad = -1;
	m_Row = 0;
	m_Column = 0;

	m_RowPadding = 0.0f;
	m_ColumnPadding = 0.0f;

	m_ElementActive	= false;
	m_FirstFrame	= true;
}

void UITable::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	AddSubscriber( E_UiMsg_CtrlButton_Up );
	AddSubscriber( E_UiMsg_CtrlButton_Down );
	AddSubscriber( E_UiMsg_CtrlButton_Left );
	AddSubscriber( E_UiMsg_CtrlButton_Right );

	AddSubscriber( E_UiMsg_FrameUpdate );
}

void UITable::PostInit()
{
	C_GadgetBase::PostInit();

	PositionElements();
}

bool UITable::MessageHandler( const Messaging::C_MessageBase *pMessage )
{
	if ( !m_Activated || !m_Selected )
		return C_GadgetBase::MessageHandler( pMessage );

	// Forward render message.
	if ( pMessage->Type() == E_UiMsg_Render )
	{
		// Render children
		for ( S32 col = 0; col < m_ActiveNodes.Size(); col++ )
		{
			for ( S32 row = 0; row < m_ActiveNodes[col].Size(); row++ )
			{
				// Render current last
				if ( col == m_Column && row == m_Row )
					continue;

				m_ActiveNodes[col][row]->MessageHandler( pMessage );
			}
		}

		if ( TableValid() )
			m_ActiveNodes[m_Column][m_Row]->MessageHandler( pMessage );
	}

	// Store the last position so that we can compare movement
	const S32 previousCol = m_Column;
	const S32 previousRow = m_Row;

	if ( !m_ElementActive )
	{
		switch ( pMessage->Type() )
		{
			case E_UiMsg_CtrlButton_Up:
			case E_UiMsg_CtrlButton_Down:
			case E_UiMsg_CtrlButton_Left:
			case E_UiMsg_CtrlButton_Right:			
			{
				const C_UiMessagePadData* pUIMessage = (C_UiMessagePadData*)pMessage;

				if ( m_Pad == -1 || ( m_Pad == pUIMessage->pad ) )
				{
					if ( pMessage->Type() == E_UiMsg_CtrlButton_Up )
					{
						MoveUp();
					}
					else
					if ( pMessage->Type() == E_UiMsg_CtrlButton_Down )
					{
						MoveDown();
					}
					else
					if ( pMessage->Type() == E_UiMsg_CtrlButton_Left )
					{
						MoveLeft();
					}
					else
					if ( pMessage->Type() == E_UiMsg_CtrlButton_Right )
					{
						MoveRight();
					}
				}
				break;
			}
		}
	}

	// Has the used changed elements
	if ( TableValid() && ( previousCol != m_Column || previousRow != m_Row ) )
	{
		// Turn old off and new on
		HighLightElement( previousCol, previousRow, false );
		HighLightElement( m_Column, m_Row, true );
	}

	return C_GadgetBase::MessageHandler( pMessage );
	}

void UITable::FrameUpdate()
{
	if ( !m_Activated )
		return;

	// Build up a table list of active nodes only to simplify logic. This will 
	m_ActiveNodes.Clear();
	for ( S32 col = 0; col < m_Nodes.Size(); col++ )
	{
		m_ActiveNodes.PushBack( TRow() );
		for ( S32 row = 0; row < m_Nodes[col].Size(); row++ )
		{
			if ( m_Nodes[col][row]->IsActivated() )
				m_ActiveNodes[col].PushBack( m_Nodes[col][row] );
		}
	}

	if ( !m_Selected || !TableValid() )
		return;

	C_GadgetBase::FrameUpdate();

	if ( m_FirstFrame )
	{
		m_FirstFrame = false;
		HighLightElement( m_Column, m_Row, true );
	}
}

void UITable::ActivateElement( const bool bActivate )
{
	if ( TableValid() )
	{
		m_ElementActive = bActivate;
		SelectElement( bActivate );
	}
}

void UITable::Render()
{
	if ( !m_Activated )
		return;

	C_UIAutoTransform transform( this );
	C_GadgetBase::Render();

	PositionElements();
}

void UITable::PositionElements()
{
	// Start from top left
	Maths::HVec3 topLeft = m_Position + ( gSimple2D.GetJustifiedCorner( m_Justify, 0, m_Panel ) * m_Size );
	F32 posX = topLeft.X();

	for ( S32 col = 0; col < m_ActiveNodes.Size(); col++ )
	{
		F32 posY = topLeft.Y();

		for ( S32 row = 0; row < m_ActiveNodes[col].Size(); row++ )
		{
			UINode* pNode = m_ActiveNodes[col][row];

			pNode->m_Position.x = posX;
			pNode->m_Position.y = posY;
			pNode->m_Position.z = m_Position.z;
	
			posY += ( m_RowPadding * Up() );
		}
		posX += m_ColumnPadding;
	}
}

void UITable::HighLightElement( const S32 sCol, const S32 sRow, const bool bHighlight )
{
	BIZ_ASSERT( m_ActiveNodes.Size() && m_ActiveNodes[sCol].Size() );

	m_ActiveNodes[sCol][sRow]->SendPrivateTimelineMessaage( "table_highlight", bHighlight );
}

void UITable::SelectElement( const bool bSelect )
{
	m_ActiveNodes[m_Column][m_Row]->Select( bSelect );
}

void UITable::AddRow( UINode* pNode, const F64 sValue )
{
	BIZ_ASSERT( pNode && ( m_Nodes.Size() - 1 ) >= 0 );
	pNode->m_Value = sValue;
	pNode->RemoveSubscriber( E_UiMsg_Render );
	pNode->Select( false, false );

	pNode->SetParent(this);
	pNode->CheckParentDepth();

	const S32 back = m_ActiveNodes.Size() - 1;

	// Add to both lists so that data is valid during init() calls
	m_Nodes[back].PushBack( pNode );
	m_ActiveNodes[back].PushBack( pNode );
}

void UITable::AddColumn()
{
	m_Nodes.PushBack( TRow() );
	m_ActiveNodes.PushBack( TRow() );
}

F64 UITable::GetSelection() const
{
	if ( !TableValid() )
		return 0;

	return m_ActiveNodes[m_Column][m_Row]->m_Value;
}

void UITable::MoveDown()
{
	if ( !m_ActiveNodes.Size() )
		return;

	if ( m_Row < m_ActiveNodes[m_Column].Size() - 1 )
	{
		m_Row++;
		UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Down ) );
	}
}

void UITable::MoveUp()
{
	if ( !m_ActiveNodes.Size() )
		return;

	if ( m_Row > 0 )
	{
		m_Row--;
		UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Up ) );
	}
}

void UITable::MoveLeft()
{
	if ( !m_ActiveNodes.Size() )
		return;

	if ( m_Column > 0 )
	{
		m_Column--;

		// Is the row availible
		if ( m_Row >= m_ActiveNodes[m_Column].Size() )
			m_Row = m_ActiveNodes[m_Column].Size()-1;

		UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Left ) );
	}
}

void UITable::MoveRight()
{
	if ( !m_ActiveNodes.Size() )
		return;

	if ( m_Column < m_ActiveNodes.Size() - 1 )
	{
		m_Column++;

		// Is the row availible
		if ( m_Row >= m_ActiveNodes[m_Column].Size() )
			m_Row = m_ActiveNodes[m_Column].Size()-1;

		UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Right ) );
	}
}

void UITable::Select( const bool bSelect, const bool bSendTimelineMsg )
{
	C_GadgetBase::Select( bSelect, bSendTimelineMsg );

	if ( TableValid() )
		HighLightElement( m_Column, m_Row, bSelect );
}

void UITable::SetSelection( const F64 fValue )
{
	for ( S32 col = 0; col < m_ActiveNodes.Size(); col++ )
	{
		for ( S32 row = 0; row < m_ActiveNodes[col].Size(); row++ )
		{
			// Is this what we are looking for
			if ( m_ActiveNodes[col][row]->m_Value == fValue )
			{
				m_Column = col;
				m_Row = row;
				return;
			}
		}
	}
}

const UINode* UITable::GetNodeByValue( const F64 fValue )
{
	for ( S32 col = 0; col < m_Nodes.Size(); col++ )
	{
		for ( S32 row = 0; row < m_Nodes[col].Size(); row++ )
		{
			// Is this what we are looking for
			if ( m_Nodes[col][row]->m_Value == fValue )
			{
				return m_Nodes[col][row];
			}
		}
	}
	return NULL;
}

}; // namespace