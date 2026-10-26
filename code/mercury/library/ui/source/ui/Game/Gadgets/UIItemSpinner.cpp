
#include "stdafx.h"

#include "ui/Game/Gadgets/UIItemSpinner.h"
#include "ui/Game/Gadgets/ButtonMessages.h"
#include "ui/Graphics/SimpleTimer.h"
#include "ui/Game/Gadgets/ButtonController.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )	// CFG_ALLOW_OPTIMIZE_OFF_UI
#endif

Debug::Bool g_RenderSlotBounds( "UI.UIItemSpinner.RenderSlotBounds", true );

Debug::Int g_DisplayMode( "UI.UIItemSpinner.DisplayMode", 0, Debug::Range( 0, 3 ) );

XT_TYPE( UI::UIItemSpinner, "UIItemSpinner" ) 
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_MoveSpeed, "move_speed" )
	XT_FIELD( m_ScaleSpeed, "scale_speed" )
	XT_FIELD( m_ItemScale, "selected_scale" )
	XT_FIELD( m_Spacing, "slot_spacing" )
	XT_FIELD( m_SlotMoveDelay, "move_delay" )
	XT_FIELD( m_WrapSlotList, "wrap" )
XT_END_TYPE

namespace UI
{

void UIItemSpinner::Init()
{
	C_GadgetBase::Init();

	m_CurrentSlot	= 0;
	m_LastSlot		= 0;
	m_ChangeSlot	= 0;

	m_Spacing		= 0.0f;
	m_ItemScale		= 1.0f;

	m_WrapSlotList	= 0;

	m_MoveSpeed		= 0.1f;
	m_ScaleSpeed	= 0.01f;
	m_ItemScale		= 1.2f;

	// messages
	AddSubscriber( E_UiMsg_FrameUpdate );
	AddSubscriber( E_UiMsg_CtrlButton_Up );
	AddSubscriber( E_UiMsg_CtrlButton_Down );
	AddSubscriber( E_UiMsg_CtrlButton_Left );
	AddSubscriber( E_UiMsg_CtrlButton_Right );
}

void UIItemSpinner::Setup( lua_State *pState )
{
	C_GadgetBase::Setup( pState );

	m_DisplayMode = (E_ItemDisplayMode)(S32)g_DisplayMode;
}

bool UIItemSpinner::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( !m_Activated )
		return C_GadgetBase::MessageHandler( pMessage );

	if ( !pMessage )
		return false;


	if ( m_SlotMoveDelay > 0 )
		return false;

	switch ( pMessage->Type() )
	{
		case E_UiMsg_CtrlButton_Up:
			if ( m_DisplayMode == E_ItemDisplayMode_Vertical )
			{
				m_CurrentSlot--;

				if ( m_CurrentSlot < 0 )
				{
					if ( m_WrapSlotList )
						m_CurrentSlot = m_LastSlot;
					else
						m_CurrentSlot = 0;
				}
			}
			break;

		case E_UiMsg_CtrlButton_Down:
			if ( m_DisplayMode == E_ItemDisplayMode_Vertical )
			{
				m_CurrentSlot++;

				if ( m_CurrentSlot > m_LastSlot )
				{
					if ( m_WrapSlotList )
						m_CurrentSlot = 0;
					else
						m_CurrentSlot = m_LastSlot;
				}
			}
			break;

		case E_UiMsg_CtrlButton_Left:
			if ( m_DisplayMode != E_ItemDisplayMode_Vertical )
			{
				m_CurrentSlot--;

				if ( m_CurrentSlot < 0 )
				{
					if ( m_WrapSlotList )
						m_CurrentSlot = m_LastSlot;
					else
						m_CurrentSlot = 0;
				}
			}
			break;

		case E_UiMsg_CtrlButton_Right:
			if ( m_DisplayMode != E_ItemDisplayMode_Vertical )
			{
				m_CurrentSlot++;

				if ( m_CurrentSlot > m_LastSlot )
				{
					if ( m_WrapSlotList )
						m_CurrentSlot = 0;
					else
						m_CurrentSlot = m_LastSlot;
				}
			}
			break;

		default:
			return C_GadgetBase::MessageHandler( pMessage );
	}

	return true;
}

void UIItemSpinner::ReorderRenderSlots()
{
	m_RenderList.Clear(); // Empty the list

	if ( m_DisplayMode == E_ItemDisplayMode_Horizontal || m_DisplayMode == E_ItemDisplayMode_Vertical )
	{
		for ( S32 i = m_SlotList.Size()-1; i > m_CurrentSlot; i-- )
		{
			m_RenderList.PushBack( m_SlotList[i] );
		}

		for ( S32 i = 0; i < m_CurrentSlot; i++ )
		{
			m_RenderList.PushBack( m_SlotList[i] );
		}
		// Push the current on last to fix overlap issues.
		m_RenderList.PushBack( m_SlotList[m_CurrentSlot] );
	}
	else
	if ( m_DisplayMode == E_ItemDisplayMode_Stacked )
	{
		if ( !m_WrapSlotList )
		{
			for ( S32 i = 0; i < m_SlotList.Size(); i++ )
			{
				m_RenderList.PushBack( m_SlotList[i] );
			}
		}
		else
		{
			for ( S32 i = m_CurrentSlot+1; i < m_SlotList.Size(); i++ )
			{
				m_RenderList.PushBack( m_SlotList[i] );
			}
			for ( S32 i = 0; i < m_CurrentSlot; i++ )
			{
				m_RenderList.PushBack( m_SlotList[i] );
			}
			m_RenderList.PushBack( m_SlotList[m_CurrentSlot] );
		}
	}
}


void UIItemSpinner::FrameUpdate()
{
	if ( !m_Activated )
		return;

	// Debuging
	m_DisplayMode = (E_ItemDisplayMode)(S32)g_DisplayMode;

	if (m_SlotMoveDelay > 0)
	{
		m_SlotMoveDelay -=(S32)(UI::theFETimer.FrameTime()*1000);
		if ( m_SlotMoveDelay < 0 )
			m_SlotMoveDelay = 0;
	}

	if ( m_ChangeSlot != m_CurrentSlot )
	{
		m_ChangeSlot = m_CurrentSlot;
		ReorderRenderSlots();
	}

	if ( m_DisplayMode == E_ItemDisplayMode_Horizontal )
	{
		// Total spacing between the items.
		F32 offsetX = m_Size.x + m_Spacing;
		F32 offsetScaleX = ( ( m_Size.x * m_ItemScale ) * 0.5f ) - ( ( offsetX - m_Spacing ) * 0.5f );

		for ( S32 slot = 0; slot < m_SlotList.Size(); slot++ )
		{
			F32 x = m_Position.x;

			if ( slot < m_CurrentSlot ) 
			{
				x -= offsetScaleX;
				x -= ( offsetX * ( m_CurrentSlot - slot ) ) ;
			}
			else
			if ( slot > m_CurrentSlot ) 
			{
				x += offsetScaleX;
				x += ( offsetX * ( slot - m_CurrentSlot ) ) ;
			}
			else
			{
				UpdateSlot( m_CurrentSlot, m_Position.x, m_Position.y, m_Position.z, m_ItemScale );
				continue;
			}
			// Update the item.
			UpdateSlot( slot, x, m_Position.y, m_Position.z, m_ItemScale );
		}
	}
	else
	if ( m_DisplayMode == E_ItemDisplayMode_Vertical )
	{
		// Total spacing between the items.
		F32 offsetY = m_Size.y + m_Spacing;
		F32 offsetScaleY = ( ( m_Size.y * m_ItemScale ) * 0.5f ) - ( ( offsetY - m_Spacing ) * 0.5f );

		for ( S32 slot = 0; slot < m_SlotList.Size(); slot++ )
		{
			F32 y = m_Position.y;

			if ( slot < m_CurrentSlot ) 
			{
				y -= offsetScaleY;
				y -= ( offsetY * ( m_CurrentSlot - slot ) ) ;
			}
			else
			if ( slot > m_CurrentSlot ) 
			{
				y += offsetScaleY;
				y += ( offsetY * ( slot - m_CurrentSlot ) ) ;
			}
			else
			{
				UpdateSlot( m_CurrentSlot, m_Position.x, m_Position.y, m_Position.z, m_ItemScale );
				continue;
			}
			// Update the item.
			UpdateSlot( slot, m_Position.x, y, m_Position.z, m_ItemScale );
		}
	}
	else
	if ( m_DisplayMode == E_ItemDisplayMode_Stacked )
	{
		if ( !m_WrapSlotList )
		{
			for ( S32 slot = 0; slot < m_RenderList.Size(); slot++ )
			{	
				F32 x = m_Position.x;
				F32 y = m_Position.y;
				F32 z = m_Position.z;

				if ( slot < m_CurrentSlot ) 
				{
					x -= m_Spacing * ( m_CurrentSlot - slot );
					y -= m_Spacing * ( m_CurrentSlot - slot );
					z -= m_Spacing * ( m_CurrentSlot - slot );
				}
				else
				if ( slot > m_CurrentSlot ) 
				{
					// Make it go off screen
					x += 640.f;
					y = m_Position.y;
					z = m_Position.z;

					if ( slot != m_CurrentSlot+1 )
					{
						UpdateSlot( slot, x, y, z, m_ItemScale );
						continue;
					}
				}
				if ( slot == m_CurrentSlot )
				{
					UpdateSlot( slot, m_Position.x, m_Position.y, m_Position.z, m_ItemScale );
				}
				else
				{
					// Update the item.
					UpdateSlot( slot, x, y, z, m_ItemScale );
				}
			}
		}
		else
		{
			for ( S32 slot = 0; slot < m_RenderList.Size(); slot++ )
			{
				F32 x = m_Position.x - ( m_Spacing * ( ( m_RenderList.Size()-1 ) - slot ) );
				F32 y = m_Position.y - ( m_Spacing * ( ( m_RenderList.Size()-1 ) - slot ) );
				F32 z = m_Position.z - ( m_Spacing * ( ( m_RenderList.Size()-1 ) - slot ) );

				UpdateSlot( slot, x, y, z, m_ItemScale, false );
			}
		}
	}
	C_GadgetBase::FrameUpdate();
}


void UIItemSpinner::Render()
{
	if ( !m_Activated )
		return;

	// Render them
	for ( S32 i = 0; i < m_RenderList.Size(); i++ )
	{
		// Render the slot bounds.
		if ( g_RenderSlotBounds )
		{
			C_GadgetBase* pSlot = m_RenderList[i]->pSlot;

			gSimple2D.AddSprite( pSlot->m_Layer, 
								 pSlot->m_Position, 
								 pSlot->m_Size.x * pSlot->m_Scale.x, 
								 pSlot->m_Size.y * pSlot->m_Scale.y, 
								 0x33888888,
								 pSlot->m_Panel );
			gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ), 0.0f, 0.0f, 1.0f, 1.0f );
			gSimple2D.AddJustification( E_Justify_MiddleCentre );

			// Just so that the slot name can be seen.
			pSlot->Render();
		}

		// Need to render the attached gadgets ourselves as they are removed from the buttons update list.
		for( S32 item = 0; item < m_RenderList[i]->AttachedList.Size(); item++ )
		{
			m_RenderList[i]->AttachedList[item]->Render();
		}

	}

	C_GadgetBase::Render();
}

S32 UIItemSpinner::AddSlot()
{
	if ( m_SlotList.Size() < m_MaxSlots )
	{
		Slot* pNewSlot = new Slot();

		pNewSlot->pSlot = gButtons.AddButton( E_UIButtonType_Dummy, C_ScreenControlManager::Instance().Context() );

		Util::C_String name;
		name.Printf("%sslot_%d", (const char *)m_Name, m_SlotList.Size() );	

		pNewSlot->pSlot->SetName( name.Chars() );
		pNewSlot->pSlot->m_Position = m_Position;
		pNewSlot->pSlot->m_Size = m_Size;
		pNewSlot->pSlot->m_Layer = m_Layer;
		pNewSlot->pSlot->m_Panel = m_Panel;
		pNewSlot->pSlot->m_Justify = m_Justify;

		m_LastSlot = m_SlotList.Size();
		m_SlotList.PushBack( pNewSlot );

		// Remove this from the buttons rendering list.
		pNewSlot->pSlot->RemoveSubscriber( E_UiMsg_Render );
	}
	ReorderRenderSlots();

	return m_LastSlot;
}

void UIItemSpinner::AddItem(  const S32 slot, C_GadgetBase* pData )
{
	assert( pData );

	if ( slot >= m_MaxSlots || slot < 0 )
	{
		return;
	}
	// Remove this from the buttons rendering list.
	pData->RemoveSubscriber( E_UiMsg_Render );

	pData->SetParent(m_SlotList[slot]->pSlot);
	pData->m_Layer = m_SlotList[slot]->pSlot->m_Layer;
	pData->m_Panel = m_SlotList[slot]->pSlot->m_Panel;

	// Add this to the attached list.
	m_SlotList[slot]->AttachedList.PushBack( pData );
}

void UIItemSpinner::UpdateSlot( S32 slot, F32 x, F32 y, F32 z, F32 scale, bool useSlotList )
{
	C_GadgetBase* pSlot = ( useSlotList ) ? m_SlotList[slot]->pSlot : m_RenderList[slot]->pSlot;

	pSlot->m_Position.x = Maths::Lerp( pSlot->m_Position.x, x, ( m_MoveSpeed * 60.0f ) * theFETimer.FrameTime() );
	pSlot->m_Position.y = Maths::Lerp( pSlot->m_Position.y, y, ( m_MoveSpeed * 60.0f ) * theFETimer.FrameTime() );
	pSlot->m_Position.z = Maths::Lerp( pSlot->m_Position.z, z, ( m_MoveSpeed * 60.0f ) * theFETimer.FrameTime() );

	if ( slot == ( ( useSlotList ) ? m_CurrentSlot : m_RenderList.Size()-1 ) )
	{
		pSlot->m_Scale.x += ( m_ScaleSpeed * 60.0f ) * theFETimer.FrameTime();
		pSlot->m_Scale.y += ( m_ScaleSpeed * 60.0f ) * theFETimer.FrameTime();
		pSlot->m_Scale.z += ( m_ScaleSpeed * 60.0f ) * theFETimer.FrameTime();

		if ( pSlot->m_Scale.x > scale || pSlot->m_Scale.y > scale || pSlot->m_Scale.z > scale )
		{
			pSlot->m_Scale.x = scale;
			pSlot->m_Scale.y = scale;
			pSlot->m_Scale.z = scale;
		}
	}
	else
	{
		pSlot->m_Scale.x -= ( m_ScaleSpeed * 60.0f ) * theFETimer.FrameTime();
		pSlot->m_Scale.y -= ( m_ScaleSpeed * 60.0f ) * theFETimer.FrameTime();
		pSlot->m_Scale.z -= ( m_ScaleSpeed * 60.0f ) * theFETimer.FrameTime();

		if ( pSlot->m_Scale.x < 1.0f || pSlot->m_Scale.y < 1.0f || pSlot->m_Scale.z < 1.0f )
		{
			pSlot->m_Scale.x = 1.0f;
			pSlot->m_Scale.y = 1.0f;
			pSlot->m_Scale.z = 1.0f;
		}
	}
}

void UIItemSpinner::Exit()
{
	for( S32 i = 0; i < m_SlotList.Size(); i++ )
	{
		SAFE_DELETE_PTR( m_SlotList[i] );
	}
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", on )	// CFG_ALLOW_OPTIMIZE_OFF_UI
#endif

