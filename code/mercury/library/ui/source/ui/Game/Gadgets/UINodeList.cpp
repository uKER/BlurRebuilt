
#include "stdafx.h"

#include "Ui/Game/Gadgets/UINodeList.h"

#include <Ui/Game/Gadgets/ButtonMessages.h>
#include <Ui/Graphics/SimpleTimer.h>

#include <Ui/UiInterface.h>

#include <algorithm>

#ifndef CFG_GOLD 
Debug::Bool g_RenderNodes( "UI.NodeList.RenderNodes", false );
#endif // CFG_GOLD


#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

// XT Definition
XT_TYPE( UI::UINodeList::E_ListStyle, "UIListStyle" )
	XT_OPTION( UI::UINodeList::E_ListStyle_Horizontal,	"Horizontal" )
	XT_OPTION( UI::UINodeList::E_ListStyle_Vertical,	"Vertical" )
	XT_OPTION( UI::UINodeList::E_ListStyle_Table,		"Table" )
	XT_OPTION( UI::UINodeList::E_ListStyle_Radial,		"Radial" )
	XT_OPTION( UI::UINodeList::E_ListStyle_TableFixed,	"TableFixed" )
XT_END_TYPE

// XT Definition
XT_TYPE( UI::UINodeList::E_RadialRotation, "UIRadialRotation" )
	XT_OPTION( UI::UINodeList::E_RadialRotation_X,		"RotationX" )
	XT_OPTION( UI::UINodeList::E_RadialRotation_Y,		"RotationY" )
	XT_OPTION( UI::UINodeList::E_RadialRotation_Z,		"RotationZ" )
XT_END_TYPE

// XT Definition
XT_TYPE( UI::UINode, "UINode" )
	XT_BASE( UI::UIRenderBranch )
	XT_FIELD( m_Locked,				"locked" )
	XT_FIELD( m_Value,				"value" )
XT_END_TYPE

// XT Definition
XT_TYPE( UI::UINodeList, "UINodeList" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_ListStyle,			"style" )
	XT_FIELD( m_Padding,			"padding" )
	XT_FIELD( m_Direction,			"direction" )
	XT_FIELD( m_MoveSpeed,			"move_speed" )
	XT_FIELD( m_ItemsVisible,		"items_visible" )
	XT_FIELD( m_EndAlpha,			"end_alpha" )

	XT_FIELD( m_LoopList,			"loop_list" )
	XT_FIELD( m_FlipControls,		"flip_controls" )
	XT_FIELD( m_FlipList,			"flip_list" )
	XT_FIELD( m_HighlightLocked,	"highlight_locked" )
	XT_FIELD( m_HideLocked,			"hide_locked" )
	XT_FIELD( m_KeepSelected,		"keep_selected" )
	XT_FIELD( m_PlayToggleSound,	"play_toggle_sound" )

	// Radial settings.
	XT_FIELD( m_Radius,				"radius" )
	XT_FIELD( m_RadialRotation,		"rotation" )

	// Table settings.
	XT_FIELD( m_Columns,			"columns" )
	XT_FIELD( m_ColumnSpacing,		"column_spacing" )
	XT_FIELD( m_RowSpacing,			"row_spacing" )

	// table fixed settings
	XT_FIELD( m_uIdealRows,			"ideal_rows" )

	XT_FIELD( m_RadialAlphaFadeInnerAngle, "radial_alpha_fade_inner_angle")
	XT_FIELD( m_RadialAlphaFadeOuterAngle, "radial_alpha_fade_outer_angle")
XT_END_TYPE

namespace UI
{


/*---------------------------------------------------------
	Cakey Hackness
---------------------------------------------------------*/
namespace
{
	struct UpdateAnimAccel
	{
		F32 target;
		F32 rate;
		F32 damp;

	public:
		UpdateAnimAccel(F32 target_, F32 rate_, F32 damp_) :
			target(target_),
			rate(rate_),
			damp(damp_)
		{
		}

	public:
		inline F32 operator()(const F32& position, const F32& velocity)
		{
			return ((target - position) * rate) - (velocity * damp);
		}
	};

	template<typename Type, typename Method>
	void Integrate_LeapFrog(Type& position, Type& velocity, Method accMethod, F32 time)
	{
		F32 halfTime = time * 0.5f;

		velocity += accMethod(position, velocity) * halfTime;
		position += velocity * time;
		velocity += accMethod(position, velocity) * halfTime;
	}
}

DECLARE_DEBUG_FLOAT( g_UiNodeListRotationRate, "UI.NodeList.RotationRate", 90.0f, Debug::Range( 0.0f, 200.0f ) );
DECLARE_DEBUG_FLOAT( g_UiNodeListRotationDamp, "UI.NodeList.RotationDamp", 12.0f, Debug::Range( 0.0f, 100.0f ) );


/*-------------------------------------------------------------------
	UINode implementation
-------------------------------------------------------------------*/
void UINode::Init()
{
	UIRenderBranch::Init();

	m_Locked = false;
	m_Value = 0.0f;
	m_SortKey = -1;
}

void UINode::Select( const bool bSelect, const bool bSendTimelineMsg )
{
	UIRenderBranch::Select( bSelect, bSendTimelineMsg );

	for ( S32 i = 0; i < m_BranchChildren.Size(); i++ )
	{
		m_BranchChildren[i]->Select( bSelect, bSendTimelineMsg );
	}
}

void UINode::SendPrivateTimelineMessaage( const char* msg, const bool active, const F32 time, const bool reset )
{
	UIRenderBranch::SendPrivateTimelineMessaage( msg, active, time, reset );

	for ( S32 i = 0; i < m_BranchChildren.Size(); i++ )
	{
		m_BranchChildren[i]->SendPrivateTimelineMessaage( msg, active, time, reset );
	}
}

#ifndef CFG_GOLD
void UINode::Render()
{
	if ( !m_Activated )
		return;

	UIRenderBranch::Render();

	if ( g_RenderNodes )
	{
		C_UIAutoTransform transform( this );

		F32 size = ( Up() ) ? 6.0f : 10.0f;
		gSimple2D.AddSprite( m_Layer, m_Position, size, size, E_BasicColour_Red, m_Panel );
		gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ) );
		gSimple2D.AddJustification( E_Justify_MiddleCentre );

		C_WideString12 text;
		text.Format( L"%i", m_BranchChildren.Size() );
		gFont.DrawDebugText( m_Layer, m_Position, E_Justify_MiddleCentre, text, 0xffaaaaaa, size, 0.0f, m_Panel );
	}
}
#endif // CFG_GOLD

/*-------------------------------------------------------------------
	UINodeList implementation
-------------------------------------------------------------------*/
void UINodeList::Init()
{
	C_GadgetBase::Init();

	m_Pad				= -1;
	m_CurrentItem		= 0;
	m_CurrentPos		= Maths::HVec3( 0.0f, 0.0f, 0.0f );
	m_CurrentDest		= Maths::HVec3( 0.0f, 0.0f, 0.0f );

	m_Radius			= 0.0f;
	m_MoveSpeed			= 0.0f;
	m_Columns			= 0;
	m_ItemsVisible		= 0;

	m_LoopList			= false;
	m_FlipControls		= false;
	m_FlipList			= false;
	m_HighlightLocked	= false;
	m_HideLocked		= false;
	m_KeepSelected		= false;

	m_FirstTimeEnable	= true;
	m_ListStyle			= E_ListStyle_Horizontal;

	m_uNumActiveNodes	= 0;
	m_uIdealRows		= 1;

	m_ScrollOffset			= 0.0f;
	m_ScrollOffsetVel		= 0.0f;
	m_TargetScrollOffset	= 0.0f;

	m_PlayToggleSound	= false;
}


void UINodeList::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	// Register subscribers
	AddSubscriber( E_UiMsg_FrameUpdate );
	AddSubscriber( E_UiMsg_CtrlButton_Up );
	AddSubscriber( E_UiMsg_CtrlButton_Down );
	AddSubscriber( E_UiMsg_CtrlButton_Left );
	AddSubscriber( E_UiMsg_CtrlButton_Right );

	AddSubscriber( E_UiMsg_MenuNext );
	AddSubscriber( E_UiMsg_MenuBack );

	if ( m_ListStyle == E_ListStyle_TableFixed )
	{
		m_uIdealCols = m_Columns;
		m_Size.x = m_uIdealCols * m_ColumnSpacing;
		m_Size.y = m_uIdealRows * m_RowSpacing;
	}
}


void UINodeList::PostInit()
{
	C_GadgetBase::PostInit();

	if ( m_Nodes.Size() <= 0 )
		ScanForNodes();

	SelectFirstUnlocked();

	if ( m_ListStyle != E_ListStyle_Radial )
	{
		m_SortedRef.Clear(); // incase there was anything in it?
		m_SortedRef.Reserve( m_Nodes.Size() );
		
		for( S32 i = m_Nodes.Size() - 1; i >= 0; i-- )
			m_SortedRef.PushBack( i );
	}
	else
	{
		// Set all nodes to the correct position
		for( S32 i = 0; i < m_Nodes.Size(); i++ )
		{
			m_Nodes[ i ]->m_Position.x = 0.0f;
			m_Nodes[ i ]->m_Position.y = 0.0f;
			m_Nodes[ i ]->m_Position.z = m_Radius;
		}
		DepthSort();
	}
}

void UINodeList::AddNode( UINode* pNode )
{
	pNode->Scan();

	pNode->RemoveSubscriber( E_UiMsg_StartFrame );
	pNode->RemoveSubscriber( E_UiMsg_Render );
	pNode->Select( false, false );

	pNode->SetParent(this);
	pNode->CheckParentDepth();
	pNode->m_ParentAttach = E_Justify_MiddleCentre;

	pNode->m_Layer = m_Layer;
	pNode->m_Panel = m_Panel;

	m_Nodes.PushBack( pNode );
}

void UINodeList::AddNode( UINode* pNode, const bool bLocked )
{
	pNode->m_Locked = bLocked;

	AddNode( pNode );
}

void UINodeList::AddNode( UINode* pNode, const F64 fValue, const bool bLocked )
{
	pNode->m_Value = fValue;
	pNode->m_Locked = bLocked;

	AddNode( pNode );
}

void UINodeList::ScanForNodes( const bool bSnap )
{
	m_Nodes.Clear();
	m_SortedRef.Clear();

	// Find all children
	FindChildren( &m_Nodes, false, false, E_UIButtonType_Node );

	if( m_FlipList && m_Nodes.Size() )
	{
		// Flip
		const S32 size = m_Nodes.Size();
		for( S32 i = 0; i < size/2; i++ )
			Btl::Swap< C_GadgetBase* >( m_Nodes[ i ], m_Nodes[ size - (i+1) ] );
	}

	if ( m_CurrentItem >= m_Nodes.Size() )
		m_CurrentItem = 0; // reset

	for( S32 i = 0; i < m_Nodes.Size(); i++ )
	{
		((UINode*)m_Nodes[ i ])->Scan();

		m_Nodes[ i ]->Select( ( i == m_CurrentItem ), ( i == m_CurrentItem ) );
		m_Nodes[ i ]->RemoveSubscriber( E_UiMsg_StartFrame );
		m_Nodes[ i ]->RemoveSubscriber( E_UiMsg_Render );

		m_Nodes[ i ]->m_Layer = m_Layer;
		m_Nodes[ i ]->m_Panel = m_Panel;
	}
}


bool UINodeList::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( !m_Activated )
		return C_GadgetBase::MessageHandler( pMessage );

	// Forward render message.
	if ( pMessage->Type() == E_UiMsg_Render )
	{
		bool result = C_GadgetBase::MessageHandler( pMessage );

		for( S32 i = 0; i < m_SortedRef.Size(); i++ )
		{
			// Skip over locked nodes
			if ( m_HideLocked && ((UINode*)m_Nodes[ m_SortedRef[ i ] ])->m_Locked )
				continue;

			// Tell the node to render its children
			m_Nodes[ m_SortedRef[ i ] ]->MessageHandler( pMessage );
		}

		return result;
	}
	else
	if ( pMessage->Type() == E_UiMsg_StartFrame )
	{
		bool result = C_GadgetBase::MessageHandler( pMessage );

		for( S32 i = 0; i < m_Nodes.Size(); i++ )
			m_Nodes[ i ]->MessageHandler( pMessage );

		return result;
	}

	if ( !m_Selected )
		return C_GadgetBase::MessageHandler( pMessage );

	// Check if current item has changed.
	S32 item = m_CurrentItem;

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
					if ( !m_FlipControls )
						MoveDown();
					else
						MoveUp();
				}
				else
				if ( pMessage->Type() == E_UiMsg_CtrlButton_Down )
				{
					if ( !m_FlipControls )
						MoveUp();
					else
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

	// Update the items.
	if ( ( item != m_CurrentItem && m_CurrentItem < m_Nodes.Size() ) )
		ActivateCurrentItem( item, m_CurrentItem );

	return C_GadgetBase::MessageHandler( pMessage );
}


void UINodeList::MoveUp()
{
	switch( m_ListStyle )
	{
		case E_ListStyle_Table :
		case E_ListStyle_TableFixed :
		{
			if ( m_CurrentItem > (S32)m_Columns - 1 )
			{
				if ( m_HighlightLocked || !((UINode*)m_Nodes[m_CurrentItem - m_Columns])->m_Locked )
				{
					S32 sOld = m_CurrentItem;

					m_CurrentItem -= m_Columns;

					CheckCurrentItem();

					if ( m_CurrentItem != sOld )
					{
						if ( m_PlayToggleSound )
							UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Toggle ) );
						else
							UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( !m_FlipControls ? E_BaseSoundEffect_Down : E_BaseSoundEffect_Up ) );
					}
				}
			}
			break;
		}
		case E_ListStyle_Vertical :
		{
			S32 sOld = m_CurrentItem;

			if ( m_CurrentItem < m_Nodes.Size() - 1 )
			{
				m_CurrentItem++;
			}
			else
			if ( m_LoopList )
			{
				m_CurrentItem = 0;
			}

			CheckCurrentItem();

			if ( m_CurrentItem != sOld )
			{
				if ( m_PlayToggleSound )
					UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Toggle ) );
				else
					UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( !m_FlipControls ? E_BaseSoundEffect_Down : E_BaseSoundEffect_Up ) );
			}

			break;
		}
		case E_ListStyle_Radial :
		{
			if ( m_RadialRotation == E_RadialRotation_X )
			{
				S32 sOld = m_CurrentItem;

				if ( m_CurrentItem < m_Nodes.Size() - 1 )
				{
					m_CurrentItem++;
				}
				else
				if ( m_LoopList )
				{
					m_CurrentItem = 0;
				}

				CheckCurrentItem();

				if ( m_CurrentItem != sOld )
				{
					if ( m_PlayToggleSound )
						UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Toggle ) );
					else
						UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( !m_FlipControls ? E_BaseSoundEffect_Down : E_BaseSoundEffect_Up ) );
				}
			}
			break;
		}
	}
}


void UINodeList::MoveDown()
{
	switch( m_ListStyle )
	{
		case E_ListStyle_Table :
		case E_ListStyle_TableFixed :
		{
			if ( m_CurrentItem < m_Nodes.Size() - (S32)m_Columns )
			{
				if ( m_HighlightLocked || !((UINode*)m_Nodes[m_CurrentItem + m_Columns])->m_Locked )
				{
					S32 sOld = m_CurrentItem;

					m_CurrentItem += m_Columns;

					CheckCurrentItem();

					if ( m_CurrentItem != sOld )
					{
						if ( m_PlayToggleSound )
							UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Toggle ) );
						else
							UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( !m_FlipControls ? E_BaseSoundEffect_Up : E_BaseSoundEffect_Down ) );
					}
				}
			}
			break;
		}
		case E_ListStyle_Vertical :
		{
			S32 sOld = m_CurrentItem;

			if ( m_CurrentItem > 0 )
			{
				m_CurrentItem--;
			}
			else
			if ( m_LoopList )
			{
				m_CurrentItem = m_Nodes.Size() - 1;
			}

			CheckCurrentItem( true );

			if ( m_CurrentItem != sOld )
			{
				if ( m_PlayToggleSound )
					UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Toggle ) );
				else
					UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( !m_FlipControls ? E_BaseSoundEffect_Up : E_BaseSoundEffect_Down ) );
			}

			break;
		}
		case E_ListStyle_Radial :
		{
			if ( m_RadialRotation == E_RadialRotation_X )
			{
				S32 sOld = m_CurrentItem;

				if ( m_CurrentItem > 0 )
				{
					m_CurrentItem--;
				}
				else
				if ( m_LoopList )
				{
					m_CurrentItem = m_Nodes.Size() - 1;
				}

				CheckCurrentItem( true );
				
				if ( m_CurrentItem != sOld )
				{
					if ( m_PlayToggleSound )
						UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Toggle ) );
					else
						UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( !m_FlipControls ? E_BaseSoundEffect_Up : E_BaseSoundEffect_Down ) );
				}
			}
			break;
		}
	}
}


void UINodeList::MoveLeft()
{
	switch( m_ListStyle )
	{
		case E_ListStyle_Horizontal :
		case E_ListStyle_TableFixed :
		{
			if ( m_CurrentItem > 0 )
			{
				S32 sOld = m_CurrentItem;

				m_CurrentItem--;

				CheckCurrentItem( true );

				if ( m_CurrentItem != sOld )
				{
					if ( m_PlayToggleSound )
						UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Toggle ) );
					else
						UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Left ) );
				}
			}
			break;
		}
		case E_ListStyle_Radial :
		{
			if ( m_RadialRotation == E_RadialRotation_Y )
			{
				if ( m_CurrentItem > 0 )
				{
					S32 sOld = m_CurrentItem;

					m_CurrentItem--;
					
					CheckCurrentItem();

					if ( m_CurrentItem != sOld )
					{
						if ( m_PlayToggleSound )
							UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Toggle ) );
						else
							UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Left ) );
					}
				}
			}
			break;
		}
		case E_ListStyle_Table :
		{
			if ( ( m_CurrentItem % m_Columns ) != 0 )
			{
				if ( m_CurrentItem > 0 )
				{
					S32 sOld = m_CurrentItem;

					m_CurrentItem--;

					CheckCurrentItem();

					if ( m_CurrentItem != sOld )
					{
						if ( m_PlayToggleSound )
							UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Toggle ) );
						else
							UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Left ) );
					}
				}
			}
			break;
		}
	}
}


void UINodeList::MoveRight()
{
	switch( m_ListStyle )
	{
		case E_ListStyle_Horizontal :
		case E_ListStyle_TableFixed :
		{
			if ( m_CurrentItem < m_Nodes.Size() - 1 )
			{
				S32 sOld = m_CurrentItem;

				m_CurrentItem++;

				CheckCurrentItem();

				if ( m_CurrentItem != sOld )
				{
					if ( m_PlayToggleSound )
						UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Toggle ) );
					else
						UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Right ) );
				}
			}
			break;
		}
		case E_ListStyle_Radial :
		{
			if ( m_RadialRotation == E_RadialRotation_Y )
			{
				if ( m_CurrentItem < m_Nodes.Size() - 1 )
				{
					S32 sOld = m_CurrentItem;

					m_CurrentItem++;

					CheckCurrentItem();

					if ( m_CurrentItem != sOld )
					{
						if ( m_PlayToggleSound )
							UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Toggle ) );
						else
							UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Right ) );
					}
				}
			}
			break;
		}
		case E_ListStyle_Table :
		{
			if ( ( m_CurrentItem % m_Columns ) != ( m_Columns - 1 ) )
			{
				if ( m_CurrentItem < m_Nodes.Size() - 1 )
				{
					S32 sOld = m_CurrentItem;

					m_CurrentItem++;

					CheckCurrentItem();

					if ( m_CurrentItem != sOld )
					{
						if ( m_PlayToggleSound )
							UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Toggle ) );
						else
							UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Right ) );
					}
				}
			}
			break;
		}
	}
}

bool UINodeList::CheckCurrentItemForwards()
{
	// Search forward
	for ( S32 i = m_CurrentItem; i < m_Nodes.Size(); i++ )
	{
		UINode* pNode = (UINode*)m_Nodes[i];

		if ( pNode->IsActivated() && ( m_HighlightLocked || !((UINode*)m_Nodes[i])->m_Locked ) )
		{
			m_CurrentItem = i;
			return false;
		}
	}

	return true;
}

bool UINodeList::CheckCurrentItemBackwards()
{
	// Search back
	for ( S32 i = m_CurrentItem - 1; i >= 0; i-- )
	{
		UINode* pNode = (UINode*)m_Nodes[i];

		if ( pNode->IsActivated() && ( m_HighlightLocked || !((UINode*)m_Nodes[i])->m_Locked ) )
		{
			m_CurrentItem = i;
			return false;
		}
	}

	return true;
}

bool UINodeList::CheckCurrentItem( bool bBackwardsFirst )
{
	// Is fine
	if ( m_CurrentItem < m_Nodes.Size() )
	{
		if ( m_Nodes[m_CurrentItem]->IsActivated() && ( m_HighlightLocked || !((UINode*)m_Nodes[m_CurrentItem])->m_Locked ) )
			return true;
	}

	if ( bBackwardsFirst )
	{
		if ( !CheckCurrentItemBackwards() )
			return false;

		return CheckCurrentItemForwards();
	}
	else
	{
		if ( !CheckCurrentItemForwards() )
			return false;

		return CheckCurrentItemBackwards();
	}
	
	return true;
}


void UINodeList::FrameUpdate()
{
	C_GadgetBase::FrameUpdate();

	if ( m_Activated )
	{
		if ( m_ListStyle == E_ListStyle_TableFixed )
		{
			U32 uNumActiveNodesOld = m_uNumActiveNodes;

			if ( ( m_uNumActiveNodes = NumActiveNodes() ) != uNumActiveNodesOld )
				ScaleTableItems();
		}

		if ( m_FirstTimeEnable && m_CurrentItem < m_Nodes.Size() )
		{
			if ( !((UINode*)m_Nodes[ m_CurrentItem ])->m_Locked && m_Selected )
			{
				m_Nodes[ m_CurrentItem ]->Select( true );
			}
			m_FirstTimeEnable = false;
		}

		// Just incase a node has been deactivated at runtime.
		U32 previous = m_CurrentItem;
		if ( !CheckCurrentItem() && m_Selected )
			ActivateCurrentItem( previous, m_CurrentItem ); 
	}
}


void UINodeList::Render()
{
	// Only render if we are activated and there are nodes to display
	if ( !m_Activated || !m_Nodes.Size() )
		return;

	// Debug help me please
	C_UIAutoTransform transform( this );

	// Change render style based on list styles
	switch ( m_ListStyle )
	{
		case E_ListStyle_Horizontal:
		case E_ListStyle_Vertical:
		{
			CalculateDirectionalPositions();
			break;
		}
		case E_ListStyle_Table:
		case E_ListStyle_TableFixed:
		{
			CalculateTablePositions();
			break;
		}
		case E_ListStyle_Radial:
		{
			CalculateRadialPositions();
			break;
		}
	}
	
	DepthSort();

#ifndef CFG_GOLD
	if ( g_RenderNodes )
	{
		F32 size = ( Up() ) ? 10.0f : 20.0f;
		gSimple2D.AddSprite( m_Layer, m_Position, size, size, E_BasicColour_Green, m_Panel );
		gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ) );
		gSimple2D.AddJustification( E_Justify_MiddleCentre );

		C_WideString12 text;
		text.Format( L"%i / %i", m_CurrentItem + 1, m_Nodes.Size() );
		gFont.DrawDebugText( m_Layer, m_Position, E_Justify_MiddleCentre, text, 0xffaaaaaa, size, 0.0f, m_Panel );

		if ( m_ListStyle == E_ListStyle_TableFixed )
		{
			gSimple2D.AddSprite( m_Layer, m_Position, m_Size.x, m_Size.y, E_BasicColour_LightGrey, m_Panel );
			gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ) );
		}
	}
#endif // CFG_GOLD
}

void UINodeList::SetItemSortKey( const U32 uIndex, const S32 sSortKey )
{
	if ( uIndex < (U32)m_Nodes.Size() )
		((UINode*)m_Nodes[ uIndex ])->m_SortKey = sSortKey;
}

void UINodeList::SetItemSortKeyByValue( const F64 value, const S32 sSortKey )
{
	for ( S32 i = 0; i < m_Nodes.Size(); i++ )
	{
		if ( ((UINode*)m_Nodes[ i ])->m_Value == value )
		{
			SetItemSortKey( i, sSortKey );
			break;
		}
	}
}

static bool _NodeSort( const C_GadgetBase* pA, const C_GadgetBase* pB )
{
	return ( ((UINode*)pA)->m_SortKey < ((UINode*)pB)->m_SortKey );
}

void UINodeList::Sort()
{
	if(m_Nodes.Size() == 0)
		return;

	// make sure the old one is turned off before we re-order things
	m_Nodes[ m_CurrentItem ]->Select( false );

	std::sort( m_Nodes.Begin(), m_Nodes.End(), _NodeSort );	
}

void UINodeList::Select( const bool bSelect, const bool bSendTimelineMsg )
{
	C_GadgetBase::Select( bSelect, bSendTimelineMsg );

	if ( m_CurrentItem < m_Nodes.Size() && !m_KeepSelected )
		m_Nodes[m_CurrentItem]->Select( bSelect, bSendTimelineMsg );
}


void UINodeList::Activate( const bool bActivate, const bool bSendTimelineMsg )
{
	C_GadgetBase::Activate( bActivate, bSendTimelineMsg );

	for ( S32 i = 0; i < m_Nodes.Size(); i++ )
		m_Nodes[i]->Activate( bActivate, bSendTimelineMsg );
}


void UINodeList::SetListStyle( const E_ListStyle eStyle )
{
	m_ListStyle = eStyle;
}

void UINodeList::SetItemLocked( const U32 uIndex, const bool bLocked )
{
	if ( uIndex < (U32)m_Nodes.Size() )
	{
		(( UINode*)m_Nodes[uIndex])->m_Locked = bLocked;

		if ( bLocked )
		{
			m_Nodes[ uIndex ]->Select( false );

			((UINode*)m_Nodes[ uIndex ])->SendPrivateTimelineMessaage( "gadget_locked", true );
			((UINode*)m_Nodes[ uIndex ])->SendPrivateTimelineMessaage( "gadget_unlocked", false );
		}
		else
		{
			((UINode*)m_Nodes[ uIndex ])->SendPrivateTimelineMessaage( "gadget_locked", false );
			((UINode*)m_Nodes[ uIndex ])->SendPrivateTimelineMessaage( "gadget_unlocked", true );
		}
	}
}


void UINodeList::SetItemLockedByValue( const F64 value, const bool bLocked )
{
	for ( U32 i = 0; i < (U32)m_Nodes.Size(); i++ )
	{
		if ( ((UINode*)m_Nodes[ i ])->m_Value == value )
		{
			SetItemLocked( i, bLocked );
			break;
		}
	}
}


UINode* UINodeList::GetNodeByIndex( const U32 uIndex )
{
	if ( uIndex >= (U32)m_Nodes.Size() )
		return NULL;
	
	return ((UINode*)m_Nodes[ uIndex ]);
}


void UINodeList::SelectFirstUnlocked()
{
	if ( m_CurrentItem < m_Nodes.Size() )
	{
		if ( !((UINode*)m_Nodes[m_CurrentItem])->m_Locked && m_Nodes[m_CurrentItem]->IsActivated() )
			return;
	}

	// Search forward
	for ( S32 i = m_CurrentItem; i < m_Nodes.Size(); i++ )
	{
		if ( !((UINode*)m_Nodes[i])->m_Locked )
		{
			m_CurrentItem = i;
			return;
		}
	}
	// Search back
	for ( S32 i = m_CurrentItem - 1; i >= 0; i-- )
	{
		if ( !((UINode*)m_Nodes[i])->m_Locked )
		{
			m_CurrentItem = i;
			return;
		}
	}
}


void UINodeList::CalculateDirectionalPositions()
{
	// If there is no move speed then keep the list static
	if ( m_MoveSpeed == 0.0f )
	{
		m_CurrentPos = Maths::HVec3( 0.0f, 0.0f, 0.0f );
		m_CurrentDest = Maths::HVec3( 0.0f, 0.0f, 0.0f );
	}
	else
	{
		F32 posX  = 0.0f, posY = 0.0f, posZ = 0.0f;

		// Calculate total distance
		for( S32 i = 0; i < m_CurrentItem; i++ )
		{
			posX -= ( m_Nodes[ i ]->m_Size.x + m_Padding.left + m_Padding.right ) * m_Direction.x;
			posY -= ( m_Nodes[ i ]->m_Size.y + m_Padding.top + m_Padding.bottom ) * m_Direction.y;
			posZ -= m_Nodes[ i ]->m_Size.z * m_Direction.z;
		}
		
		m_CurrentDest = Maths::HVec3( posX, posY, posZ );
	}

	// Update the items.
	F32 posX = m_CurrentPos.X() - ( ( m_MoveSpeed * ( m_CurrentPos.X() - m_CurrentDest.X() ) ) * theFETimer.FrameTime() );
	F32 posY = m_CurrentPos.Y() - ( ( m_MoveSpeed * ( m_CurrentPos.Y() - m_CurrentDest.Y() ) ) * theFETimer.FrameTime() );
	F32 posZ = m_CurrentPos.Z() - ( ( m_MoveSpeed * ( m_CurrentPos.Z() - m_CurrentDest.Z() ) ) * theFETimer.FrameTime() );

	m_CurrentPos = Maths::HVec3( posX, posY, posZ );

	for( S32 i = 0; i < m_Nodes.Size(); i++ )
	{
		// Ignore locked items if we dont want to render them so that there isnt gaps in the menu
		if ( !m_HideLocked || !((UINode*)m_Nodes[ i ])->m_Locked )
		{
			m_Nodes[ i ]->m_Position = Maths::HVec3( posX, posY, posZ );

			// Offset the next item position
			posX += ( m_Nodes[ i ]->m_Size.x + m_Padding.left + m_Padding.right ) * m_Direction.x;
			posY += ( m_Nodes[ i ]->m_Size.y + m_Padding.top + m_Padding.bottom ) * m_Direction.y;
			posZ += m_Nodes[ i ]->m_Size.z * m_Direction.z;
		}
	}
}


void UINodeList::CalculateTablePositions()
{
	if ( m_Columns == 0 )
		return;

	U32 index = 0;
	S32 rows = ( m_Nodes.Size() / m_Columns ) + 1; // incase we have an odd number

	F32 offsetY = 0.0f;

	for ( S32 row = 0; row < rows; row++ )
	{
		Maths::HVec3 offset( 0.0f, offsetY * Up(), 0.0f );

		for ( U32 column = 0; column < m_Columns; column++, index++ )
		{
			if ( (S32)index > ( m_Nodes.Size() - 1 ) )
				break;

			// Update position
			m_Nodes[ index ]->m_Position = offset;

			offset += ( Maths::HVec3( m_Direction ) * m_ColumnSpacing );
		}
		offsetY += m_RowSpacing;
	}
}

void UINodeList::CalculateRadialPositions()
{
	// Work out how many items can fit around the circumference
	const F32 circumference = Maths::Pi * m_Radius;
	const F32 padding = m_Padding.top + m_Padding.bottom;

	F32 size = 0.0f;
	bool finished = false;

	S32 cwIndex = m_CurrentItem;
	S32 ccwIndex = m_CurrentItem-1;

	while( ( cwIndex < m_Nodes.Size() || ccwIndex >= 0 ) && !finished )
	{
		// Check clockwise
		if ( cwIndex < m_Nodes.Size() && ( !m_HideLocked || !((UINode*)m_Nodes[ cwIndex ])->m_Locked ) )
		{
			// Will it fit
			if ( size + m_Nodes[cwIndex]->m_Size.y + padding < circumference )
			{
				size += m_Nodes[cwIndex]->m_Size.y + padding;
			}
			else
				finished = true;
		}

		// Check coutner-clockwise
		if ( ccwIndex >= 0 && ( !m_HideLocked || !((UINode*)m_Nodes[ ccwIndex ])->m_Locked ) )
		{
			// Will it fit
			if ( size + m_Nodes[ccwIndex]->m_Size.y + padding < circumference )
			{
				size += m_Nodes[ccwIndex]->m_Size.y + padding;
			}
			else
				finished = true;
		}

		cwIndex++;
		ccwIndex--;
	}

	// Find scroll offset amount
	m_TargetScrollOffset = 0.0f;
	for ( S32 i = 0; i < m_CurrentItem; i++ )
	{
		// Ignore items when locked and flag is active
		if ( m_HideLocked && ((UINode*)m_Nodes[ i ])->m_Locked )
			continue;

		m_TargetScrollOffset += ( (m_Nodes[ i ]->m_Size.y + padding) / m_Radius );
	}

	// Move the offset towards the target
	Integrate_LeapFrog< F32 >( m_ScrollOffset, m_ScrollOffsetVel, UpdateAnimAccel( m_TargetScrollOffset, g_UiNodeListRotationRate, g_UiNodeListRotationDamp ), theFETimer.FrameTime() );

	// Set the starting theata to the offset amount
	F32 theta = -m_ScrollOffset;

	// Set all nodes to the correct position
	for( S32 i = 0; i < m_Nodes.Size(); i++ )
	{
		// Ignore items when locked and flag is active
		if ( m_HideLocked && ((UINode*)m_Nodes[ i ])->m_Locked )
			continue;

		switch ( m_RadialRotation )
		{
			case E_RadialRotation_X : 
			{
				m_Nodes[ i ]->m_Position.x = 0.0f;
				m_Nodes[ i ]->m_Position.y = Maths::Sin(theta) * -m_Radius;
				m_Nodes[ i ]->m_Position.z = ( Maths::Cos(theta) * -m_Radius ) + m_Radius;
				break;
			}
			case E_RadialRotation_Y : 
			{
				m_Nodes[ i ]->m_Position.x = Maths::Sin(theta) * m_Radius;
				m_Nodes[ i ]->m_Position.y = 0.0f;
				m_Nodes[ i ]->m_Position.z = Maths::Cos(theta) * -m_Radius;
				break;
			}
			case E_RadialRotation_Z : 
			{
				m_Nodes[ i ]->m_Position.x = Maths::Cos(theta) * m_Radius;
				m_Nodes[ i ]->m_Position.y = Maths::Sin(theta) * m_Radius;
				m_Nodes[ i ]->m_Position.z = 0.0f;
				break;
			}
		}

		float inner = m_RadialAlphaFadeInnerAngle * (3.14159f / 180.0f);
		float outer = m_RadialAlphaFadeOuterAngle * (3.14159f / 180.0f);

		float alpha = 1.0f - ((fabsf(theta) - inner) / (outer - inner));

		if (alpha < 0.0f)
			alpha = 0.0f;
		else if (alpha > 1.0f)
			alpha = 1.0f;

		m_Nodes[i]->m_Alpha = alpha;
		m_Nodes[i]->DirtySet( E_Dirty_Alpha, true );

		theta += ( (m_Nodes[ i ]->m_Size.y + padding) / m_Radius );
	}
}


void UINodeList::DepthSort()
{
	// Need to depth sort them. 
	m_SortedRef.Clear();

	// Either cap it at the specified max or do them all.
	S32 end = ( m_ItemsVisible ) ? ( m_ItemsVisible / 2 ) : m_Nodes.Size();

	// Search back to front.
	for ( S32 count = end; count > 0; count-- )
	{
		// Add last visible clockwise
		if ( m_CurrentItem - count >= 0 )
		{
			m_SortedRef.PushBack( m_CurrentItem - count );
		}

		// Add last visible anti clockwise
		if ( m_CurrentItem + count < m_Nodes.Size() ) 
		{
			m_SortedRef.PushBack( m_CurrentItem + count );
		}
	}

	// Add current last so that it is drawn on top
	if ( m_CurrentItem < m_Nodes.Size() )
	{
		m_SortedRef.PushBack( m_CurrentItem );
	}
}


void UINodeList::ActivateCurrentItem( const U32 uPrevious, const U32 uCurrent )
{
	m_Nodes[ uPrevious ]->Select( false );

	if ( ((UINode*)m_Nodes[ uPrevious ])->m_Locked )
		((UINode*)m_Nodes[ uPrevious ])->SendPrivateTimelineMessaage( "gadget_selected_locked", false );

	// only if it isnt locked.
	if ( !((UINode*)m_Nodes[uCurrent])->m_Locked )
		m_Nodes[ uCurrent ]->Select( true );

	m_CurrentItem = uCurrent;

	if ( ((UINode*)m_Nodes[ uCurrent ])->m_Locked )
		((UINode*)m_Nodes[ uCurrent ])->SendPrivateTimelineMessaage( "gadget_selected_locked", true );
}


const bool UINodeList::SetCurrentItemByValue( const F64 value )
{
	for ( S32 i = 0; i < m_Nodes.Size(); i++ )
	{
		if ( ((UINode*)m_Nodes[i])->m_Value == value )
		{
			U32 previous = m_CurrentItem;
			m_CurrentItem = i;

			// Update positions and effects
			ActivateCurrentItem( previous, m_CurrentItem );
			return true;
		}
	}
	return false;
}


const bool UINodeList::SetCurrentItemByIndex( const S32 index )
{
	if ( index >= 0 && index < m_Nodes.Size() )
	{
		U32 previous = m_CurrentItem;
		m_CurrentItem = index;

		// Update positions and effects
		ActivateCurrentItem( previous, m_CurrentItem );

		return true;
	}
	return false;
}

U32 UINodeList::NumActiveNodes()
{
	U32 uCount = 0;

	for ( S32 i = 0; i < m_Nodes.Size(); i++ )
		if ( m_Nodes[ i ] && m_Nodes[ i ]->IsActivated() )
			uCount++;

	return uCount;
}

void UINodeList::ScaleTableItems()
{
	U32 uOriginalRows = m_uIdealRows;
	U32 uOriginalCols = m_uIdealCols;

	F32 fOriginalRowSpacing = m_Size.y / uOriginalRows;	
	F32 fOriginalColSpacing = m_Size.x / uOriginalCols;

	F32 fOriginalArea = m_Size.x * m_Size.y;
	F32 fSmallestWasteArea = fOriginalArea;
	U32 uOptimalCols = uOriginalCols;
	F32 fOptimalRatio = 1.0f;

	if ( m_uNumActiveNodes < ( uOriginalRows * uOriginalCols ) )
		fSmallestWasteArea = fOriginalArea - ( fOriginalRowSpacing * fOriginalColSpacing * m_uNumActiveNodes );

	// find the optimal solution that fits inside the box - brute force
	for ( U32 i = m_uNumActiveNodes; i > 0; i-- )
	{
		U32 uRows = i;
		U32 uCols = (U32)ceilf( (F32)m_uNumActiveNodes / (F32)uRows );	// Maths::Ceil borks this on exact values

		F32 fNewColSpacing = m_Size.x / uCols;
		F32 fRatio = fNewColSpacing / fOriginalColSpacing;

		if ( fNewColSpacing > fOriginalColSpacing )
			continue;

		if ( ( ( fOriginalRowSpacing * fRatio ) * uRows ) > m_Size.y )
			continue;

		F32 fNewArea = fOriginalRowSpacing * fOriginalColSpacing * fRatio * fRatio * m_uNumActiveNodes;
		F32 fNewWasteArea = fOriginalArea - fNewArea;

		if ( fNewWasteArea < fSmallestWasteArea )
		{
			fSmallestWasteArea = fNewWasteArea;
			uOptimalCols = uCols;
			fOptimalRatio = fRatio;
		}
	}

	m_Columns = uOptimalCols;
	m_RowSpacing = fOriginalRowSpacing * fOptimalRatio;
	m_ColumnSpacing = fOriginalColSpacing * fOptimalRatio;

	for ( S32 i = 0; i < m_Nodes.Size(); i++ )
	{
		m_Nodes[ i ]->m_Scale.x = fOptimalRatio;
		m_Nodes[ i ]->m_Scale.y = fOptimalRatio;
		//UpdateItem( i, true );
	}

	// PR - this is a bit ewww, but I need to know once the scales have been updated
	// - which may not be for a while if you've got gather effects on it
	_UI_MESSAGE( E_UiMsg_NodeListTableFixedUpdated );
}


void UINodeList::OpenCurrentItem()
{
	if ( m_CurrentItem >= m_Nodes.Size() )
		return;

	// only if it isnt locked.
	if ( !((UINode*)m_Nodes[m_CurrentItem])->m_Locked )
	{
		m_Nodes[ m_CurrentItem ]->SendPrivateTimelineMessaage( "open_current_item", true );
		UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Forwards ) );
	}
	else
		UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Error ) );
}

void UINodeList::CloseCurrentItem()
{
	if ( m_CurrentItem >= m_Nodes.Size() )
		return;

	// only if it isnt locked.
	if ( !((UINode*)m_Nodes[m_CurrentItem])->m_Locked )
	{
		m_Nodes[ m_CurrentItem ]->SendPrivateTimelineMessaage( "open_current_item", false );
		UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Backwards ) );
	}
	else
		UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Error ) );
}

}; // namespace UI
