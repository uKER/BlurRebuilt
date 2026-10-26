

#include "stdafx.h"

#include "ui/game/Gadgets/BaseUIList.h"
#include "ui/game/Gadgets/ButtonMessages.h"
#include "ui/game/Gadgets/UIScrollBar.h"

#include "ui/Graphics/Font/Font.h"
#include "ui/Graphics/Font/FontStyle.h"
#include "ui/Graphics/SimpleTimer.h"
#include "Ui/UiInterface.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )
#endif

/*---------------------------------------------------------------------------------------
	Constants
---------------------------------------------------------------------------------------*/
const F32 kfEpsilon = 1.0e-03f;

#ifndef CFG_GOLD
Debug::Bool gUseStencilClipping( "UI.BaseList.UseStencilClipping", true );
Debug::Bool gUseBoundsClipping( "UI.BaseList.UseBoundsClipping", true );
Debug::Bool gShowListBounds( "UI.BaseList.ShowListBounds", false );
#endif


/*---------------------------------------------------------------------------------------
	XT Annotation
---------------------------------------------------------------------------------------*/
XT_TYPE( UI::BaseUIList, "UIList" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_RowHeight,				"row_height" )
	XT_FIELD( m_RowPadding,				"row_padding" )
	XT_FIELD( m_ScrollAmount,			"scroll_amount" )
	XT_FIELD( m_ScrollSpeed,			"scroll_speed" )
	XT_FIELD( m_Padding,				"padding" )
	XT_FIELD( m_bAllowScrollingOnly,	"allow_scrolling_only" )
XT_END_TYPE


namespace UI
{

BaseUIList::BaseUIList()
: m_pScrollBar( NULL ), m_FlippedList( false ), m_SoundEffectMove( -1 )
{
}

void BaseUIList::Init()
{
	C_GadgetBase::Init();

	m_CurrentItem			= 0;

	m_RowHeight				= 0.0f;
	m_PosX					= 0.0f;
	m_PosY					= 0.0f;
	m_DestX					= 0.0f;
	m_DestY					= 0.0f;
	m_RowPadding			= 0.0f;
	m_ScrollAmount			= 5.0f;
	m_ScrollSpeed			= 0.0f;
	m_bAllowScrollingOnly	= false;

	m_StencilRenderState.m_BlendMode = Display::E_BlendMode_Additive;
}


void BaseUIList::PostInit()
{
	// Check to see if a scrollbar is present.
	ButtonArray	children;
	FindChildren( &children, false );

	// check the first level of children for a scrollbar
	for ( S32 i = 0; i < children.Size(); i++ )
	{
		if ( children[i]->GetType() == E_UIButtonType_ScrollBar )
			m_pScrollBar = (UIScrollBar*)children[i];
	}

	// Reposition the scrollbar
	if ( m_pScrollBar )
	{
		// Only allow topleft and topright
		if ( m_pScrollBar->m_Justify != E_Justify_TopLeft && m_pScrollBar->m_Justify != E_Justify_TopRight )
			m_pScrollBar->m_Justify = E_Justify_TopLeft;

		// Setup x position. Either left or right
		if ( m_pScrollBar->m_Justify == E_Justify_TopLeft )
			m_pScrollBar->m_Position.x = -( m_pScrollBar->m_Size.x );
		else
			m_pScrollBar->m_Position.x = m_Size.x + ( m_pScrollBar->m_Size.x );

		// Setup size and y pos.
		m_pScrollBar->m_Position.y = m_Padding.top * Up();
		m_pScrollBar->m_Size.y = m_Size.y - ( m_Padding.top + m_Padding.bottom );
		m_pScrollBar->m_Panel = m_Panel;
	}

	C_GadgetBase::PostInit();
}


void BaseUIList::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	// Set some default values.
	if ( m_pScrollBar )
	{
		m_pScrollBar->SetBounds( 0.0f, m_Size.y - ( m_Padding.top + m_Padding.bottom ) );
		m_pScrollBar->SetScrollArea( ( ( m_RowHeight + m_RowPadding ) * GetNumberOfItems() ) - m_RowPadding );
		m_pScrollBar->SetSliderPosition( 0.0f );
	}

	// Only if scrolling with a constant speed is desired
	//if ( m_ScrollSpeed > 0.0f )
	//	m_ScrollSpeed *= 100.0f;

	// Subscribe to messages
	AddSubscriber( E_UiMsg_CtrlLeftStick_Y );
	AddSubscriber( E_UiMsg_CtrlRightStick_Y );
	AddSubscriber( E_UiMsg_CtrlButton_Up );
	AddSubscriber( E_UiMsg_CtrlButton_Down );
	AddSubscriber( E_UiMsg_FrameUpdate );
}

void BaseUIList::MoveUp()
{
	if ( m_bAllowScrollingOnly )
		return;

	if ( m_CurrentItem == 0 )
		return;

	m_CurrentItem--;

	// Auto scroll to the selected item.
	ScrollToCurrentItem( ( m_ScrollSpeed <= 0.0f ) ? true : false );
}

const S32 BaseUIList::CurrentItem()
{
	return m_FlippedList ? GetNumberOfItems() - 1 - m_CurrentItem : m_CurrentItem;
}

void BaseUIList::MoveDown()
{
	if ( m_bAllowScrollingOnly )
		return;

	if ( m_CurrentItem >= GetNumberOfItems() - 1 )
		return;
	
	m_CurrentItem++;

	// Auto scroll to the selected item.
	ScrollToCurrentItem( ( m_ScrollSpeed <= 0.0f ) ? true : false );
}

bool BaseUIList::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( !m_Activated || !m_Selected )
		return C_GadgetBase::MessageHandler( pMessage );

	const S32 current_item = m_CurrentItem;
	switch ( pMessage->Type() )
	{
		case E_UiMsg_CtrlRightStick_Y:

			// Check their enough items in the view area to scroll.
			if ( CanScroll() )
			{
				const C_UiMessagePad<E_UiMsg_CtrlRightStick_Y>* pUIMessage = Messaging::MessageCast< C_UiMessagePad<E_UiMsg_CtrlRightStick_Y> >( pMessage );

				Scroll( m_ScrollAmount * ( -pUIMessage->amount ) );
				m_PosY = m_DestY; // should update straight away.
			}
			break;

		case E_UiMsg_CtrlButton_Up:

			if( m_FlippedList ) 
				MoveDown();
			else
				MoveUp();
			break;

		case E_UiMsg_CtrlButton_Down:

			if( m_FlippedList ) 
				MoveUp();
			else
				MoveDown();
			break;
	}	

	if ( current_item != m_CurrentItem )
		UI::UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Move ) );
	
	return C_GadgetBase::MessageHandler( pMessage );
}


void BaseUIList::FrameUpdate()
{
	if ( !m_Activated )
		return;

	C_GadgetBase::FrameUpdate();

	const S32 numItems = GetNumberOfItems();

	// incase an item we was on has been deleted.
	if ( numItems && m_CurrentItem > numItems - 1 )
	{
		m_CurrentItem = numItems - 1;
		// Auto scroll to the selected item.
		ScrollToCurrentItem( ( m_ScrollSpeed <= 0.0f ) ? true : false );
	}

	// Scroll up
	if ( m_ScrollSpeed > 0.0f )
	{
		if ( m_PosY > m_DestY )
		{		
			m_PosY -= ( ( m_ScrollSpeed * ( m_PosY - m_DestY ) ) * theFETimer.FrameTime() );

			if ( m_PosY - kfEpsilon < m_DestY )
				m_PosY = m_DestY;
		}
		else
		if ( m_PosY < m_DestY )
		{
			m_PosY += ( ( m_ScrollSpeed * ( m_DestY - m_PosY ) ) * theFETimer.FrameTime() );

			if ( m_PosY - kfEpsilon > m_DestY )
				m_PosY = m_DestY;
		}
	}

	// Update the scrollbar -- Do it every frame incase we get a new message.
	if ( m_pScrollBar )
	{
		// Update the scroll bar position. Takes 0 - 1 range
		F32 areaSize = ( ( m_RowHeight + m_RowPadding ) * numItems ) - m_RowPadding;
		F32 range = m_Size.y - ( m_Padding.top + m_Padding.bottom );
		F32 pos = ( m_PosY / ( areaSize - range ) ) * -1.0f;

		m_pScrollBar->SetBounds( 0.0f, m_Size.y - ( m_Padding.top + m_Padding.bottom ) );
		m_pScrollBar->SetScrollArea( areaSize );
		m_pScrollBar->SetSliderPosition( pos );

		// Fix scrollbar and scroll position if its got messed up
		if ( pos > 1.0f )
		{
			Scroll( 0.0f );
		}
	}
}


void BaseUIList::Render()
{
	if ( !m_Activated )
		return;

	//C_UIAutoTransform transform( this );
	C_GadgetBase::Render();

	// Actual top left position.
	const Maths::HVec3 drawPos = m_Position + ( gSimple2D.GetJustifiedCorner( m_Justify, 0, m_Panel ) * m_Size );
	
	// Top left position with padding taken into account.
	const F32 posX = drawPos.X() + m_Padding.left;
	const F32 posY = drawPos.Y() + ( m_Padding.top * Up() );
	const F32 sizeX = m_Size.x - m_Padding.left - m_Padding.right;
	const F32 sizey = m_Size.y - m_Padding.top - m_Padding.bottom;

	const S32 numItems = GetNumberOfItems();

	// Clip bounds.
	const F32 top = posY - (m_RowHeight * Up()); // account for the item size
	const F32 bottom = posY + (sizey * Up());

	bool useStencilClipping = true;
	bool useBoundsClipping = true;

#ifndef CFG_GOLD
	if ( gShowListBounds )
	{
		// Total size
		gSimple2D.AddSprite( m_Layer, drawPos, m_Size.x, m_Size.y, 0x77777777, m_Panel );
		gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ) );

		// Padding size
		gSimple2D.AddSprite( m_Layer, Maths::HVec3( posX, posY, m_Position.z ), sizeX, sizey, 0x77ff7722, m_Panel );
		gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ) );
	}

	useStencilClipping = gUseStencilClipping;
	useBoundsClipping = gUseBoundsClipping;
#endif

	if ( useStencilClipping )
	{
		// render in stencil for out of bounds clipping.
		gSimple2D.AddSprite( m_Layer, m_Panel, E_SpriteType_SetRendersStateStack, 1 );
		SetupStencilBufferWrite();

		// Add the stencil.
		gSimple2D.AddSprite( m_Layer, Maths::HVec3( posX, posY, m_Position.z ), sizeX, sizey, 0xff000000, m_Panel, E_SpriteType_Sprite, m_StencilRenderState );
		gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ) );
		SetupStencilBufferRead();
	}

	// Render the items.
	F32 x = posX + m_PosX;
	F32 y = posY + m_PosY * Up();

	for ( S32 i = 0; i < numItems; i++ )
	{
		bool bVisible = ( ( Up() == 1.0f && y >= top && y <= bottom ) ||
						  ( Up() ==-1.0f && y <= top && y >= bottom ) );

		// Cull items out of the viewing bounds.
		if ( bVisible || !useBoundsClipping )
		{
			if( m_FlippedList )
			{
				//RenderItem( x - ( m_PassOffsetPosition ? (F32)drawPos.X() : 0.0f ), y - ( m_PassOffsetPosition ? (F32)drawPos.Y() : 0.0f ), numItems - 1 - i );
				RenderItem( x, y, numItems - 1 - i, drawPos );
			}
			else
				RenderItem( x, y, i, drawPos );
		}
		y += (m_RowHeight + m_RowPadding) * Up();
	}

	// Finished with the stencil so pop it.
	if ( useStencilClipping )
		gSimple2D.AddSprite( m_Layer, m_Panel, E_SpriteType_SetRendersStateStack, 0 );
}


void BaseUIList::Scroll( const F32 amount )
{
	F32	range = ( ( m_RowHeight + m_RowPadding ) * GetNumberOfItems() ) - m_RowPadding;
	F32 bottom = m_Size.y - ( m_Padding.top + m_Padding.bottom );

	// Move the draw area by a fixed amount.
	m_DestY -= amount;

	// Check we havent scrolled too far.
	if ( m_DestY > 0.0f )
		m_DestY = 0.0f;

	// Check we havent scrolled too far.
	if ( ( m_DestY + range ) < bottom )
		m_DestY = -( range - bottom );
}


const bool BaseUIList::CanScroll() const
{
	// is the size of the items visible bigger than the viewing space?
	F32	range = ( ( m_RowHeight + m_RowPadding ) * GetNumberOfItems() ) - m_RowPadding;

	return ( range > ( m_Size.y - m_Padding.top - m_Padding.bottom ) ) ? true : false;
}


void BaseUIList::ScrollToCurrentItem( const bool bSnap )
{
	// Calculate position of current item.
	F32 itemPos = m_PosY + ( ( m_RowHeight + m_RowPadding ) * CurrentItem() );
	F32	range = ( ( m_RowHeight + m_RowPadding ) * GetNumberOfItems() ) - m_RowPadding;
	F32 bottom = m_Size.y - ( m_Padding.top + m_Padding.bottom );
	
	// outside of visible area
	if ( itemPos < 0.0f )
	{
		m_DestY -= itemPos;

		if ( bSnap )
			m_PosY = m_DestY;
	}
	else
	if ( ( itemPos + m_RowHeight ) > bottom )
	{
		m_DestY -= ( itemPos - bottom ) + m_RowHeight;

		if ( bSnap )
			m_PosY = m_DestY;
	}

	// Check we havent scrolled too far.
	if ( m_DestY > 0.0f )
		m_DestY = 0.0f;

	// Check we havent scrolled too far.
	if ( ( m_DestY + range ) < bottom )
		m_DestY = -( range - bottom );
}

static S32 s_ListStencilRef = 0;

void BaseUIList::SetupStencilBufferWrite( S32 ref )
{
	if ( ref == -1 )
		ref = s_ListStencilRef;

	// Write
	m_StencilState.m_StencilEnable		= Display::E_Bool_True;
	m_StencilState.m_StencilFunc		= Display::E_CmpFunc_Always;
	m_StencilState.m_StencilDepthPass	= Display::E_StencilOp_Replace;
	m_StencilState.m_StencilDepthFail	= Display::E_StencilOp_Keep;
	m_StencilState.m_StencilFail		= Display::E_StencilOp_Keep;
	m_StencilState.m_StencilWriteMask	= 0xffffffff;
	m_StencilState.m_StencilMask		= 0xffffffff;
	m_StencilState.m_StencilRef			= ref;

	gSimple2D.AddSprite( m_Layer, m_Panel, m_StencilState );
}


void BaseUIList::SetupStencilBufferRead( S32 ref )
{
	if ( ref == -1 )
		ref = s_ListStencilRef;

	// Read
	m_StencilState.m_StencilEnable		= Display::E_Bool_True;
	m_StencilState.m_StencilFunc		= Display::E_CmpFunc_Equal;
	m_StencilState.m_StencilDepthPass	= Display::E_StencilOp_Keep;
	m_StencilState.m_StencilDepthFail	= Display::E_StencilOp_Keep;
	m_StencilState.m_StencilFail		= Display::E_StencilOp_Keep;
	m_StencilState.m_StencilWriteMask	= 0xffffffff;
	m_StencilState.m_StencilMask		= 0xffffffff;
	m_StencilState.m_StencilRef			= ref;

	gSimple2D.AddSprite( m_Layer, m_Panel, m_StencilState );
}



/*---------------------------------------------------------------------------------------
	User must supply the implementation of these functions. Cant make method abstract due to xt.
---------------------------------------------------------------------------------------*/
S32	BaseUIList::GetNumberOfItems() const
{
	assert( false );

	return 0;
}

void BaseUIList::RenderItem( const F32 fX, const F32 fY, const U32 uItem, const Maths::HVec3& vRelativePos )
{
	assert( false );
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", on )
#endif
