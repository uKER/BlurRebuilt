
#include "stdafx.h"

#include "UIVinylStripBar.h"
#include "VinylManager.h"

#include <ui/Game/Gadgets/ButtonMessages.h>
#include <ui/Graphics/SimpleTimer.h>
#include <Ui/Illustrator/UIAdobeShape.h>

// XT Definition
XT_TYPE( UI::UIVinylStripBar, "UIVinylStripBar" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_StepSize, "step_size" )
	XT_FIELD( m_BackingSize, "backing_size" )
	XT_FIELD( m_MoveSpeedMult, "move_speed" )
	XT_FIELD( m_BackingColourName, "backing_colour" )
	XT_FIELD( m_UnavailableColourName, "unavailable_colour" )
	XT_FIELD( m_DesOutlineColourName, "deselected_outline_colour" )
	XT_FIELD( m_SelOutlineColourName, "selected_outline_colour" )
	XT_FIELD( m_IconColourName, "icon_colour" )
	XT_FIELD( m_SceneScale, "scene_scale" )
	XT_FIELD( m_StepDepth, "step_depth" )
	XT_FIELD( m_IconOffset, "icon_offset" )
	XT_FIELD( m_IconSize, "icon_size" )
	XT_FIELD( m_UnfoldScale, "unfold_scale" )
	XT_FIELD( m_SplitScale, "split_scale" )
XT_END_TYPE

namespace UI
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UIVinylStripBar::Init()
{
	C_GadgetBase::Init();

	m_CurrentShapeIndex = 0;
	m_MoveOffset = 0.0f;

	AddSubscriber( E_UiMsg_CtrlButton_Left );
	AddSubscriber( E_UiMsg_CtrlButton_Right );
	AddSubscriber( E_UiMsg_FrameUpdate );

	// Debug switch shape until shape picker is ready
	AddSubscriber( E_UiMsg_CtrlButton_LeftShoulder );
	AddSubscriber( E_UiMsg_CtrlButton_RightTrigger );
	AddSubscriber( E_UiMsg_CtrlButton_LeftTrigger );
}

void UIVinylStripBar::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	m_ShapeRs = m_RenderState;
	m_ShapeRs.m_EffectIndex = E_Effect_PolygonFlat;
	m_ShapeRs.m_EffectParam = 0;

	m_BackingColours.SetByName( m_BackingColourName, m_Alpha );
	m_UnavailableColours.SetByName( m_UnavailableColourName ); 
	m_DesOutlineColours.SetByName( m_DesOutlineColourName );
	m_SelOutlineColours.SetByName( m_SelOutlineColourName );
	m_IconColours.SetByName( m_IconColourName );
}

void UIVinylStripBar::FrameUpdate()
{
	// Move the offset
	m_MoveOffset -= (m_MoveOffset * theFETimer.FrameTime() * m_MoveSpeedMult );
}

bool UIVinylStripBar::MessageHandler(const Messaging::C_MessageBase *pMessage)
{
	if ( !m_Activated || !m_Selected )
		return C_GadgetBase::MessageHandler( pMessage );

	switch( pMessage->Type() )
	{
	// Move selection
	case E_UiMsg_CtrlButton_Left:
		m_MoveOffset -= (F32)C_VinylManager::Instance().SetSelection( -1 ) * m_StepSize;
		return true;

	// Move selection
	case E_UiMsg_CtrlButton_Right:
		m_MoveOffset += (F32)C_VinylManager::Instance().SetSelection( 1 ) * m_StepSize;
		return true;

	// Toggle highlight
	case E_UiMsg_CtrlButton_LeftShoulder:
		C_VinylManager::Instance().SelectedHighlightToggle();
		return true;
	}

	return C_GadgetBase::MessageHandler( pMessage );
}

void UIVinylStripBar::RenderBacking( const bool isSelected, const bool isHighlighted, const bool isGroup, const Maths::HVec3& drawPos, U32 depth )
{
	if( m_TextureParams.count <= 0 )
		return;

	// Backing
	gSimple2D.AddSprite( m_Layer, drawPos, m_BackingSize.x, m_BackingSize.y, m_BackingColours, m_Panel, E_SpriteType_Sprite, m_RenderState ); 
	gSimple2D.AddJustification( m_Justify );
	gSimple2D.AddTexUVs(0, GetTextureParam( 0 ) );

	// Outline
	gSimple2D.AddSprite( m_Layer, drawPos, m_BackingSize.x, m_BackingSize.y, isSelected ? m_SelOutlineColours : m_DesOutlineColours, m_Panel, E_SpriteType_Sprite, m_RenderState ); 
	gSimple2D.AddJustification( m_Justify );
	gSimple2D.AddTexUVs(0, GetTextureParam( 1 ) );

	if( depth != 0 )
		return;

	if( isHighlighted )
	{
		// Hilite
		gSimple2D.AddSprite( m_Layer, drawPos - m_IconOffset, m_IconSize.x, m_IconSize.y, m_IconColours, m_Panel, E_SpriteType_Sprite, m_RenderState ); 
		gSimple2D.AddJustification( m_Justify );
		gSimple2D.AddTexUVs(0, GetTextureParam( 2 ) );
	}

	if( isGroup )
	{
		// Group
		gSimple2D.AddSprite( m_Layer, drawPos + m_IconOffset, m_IconSize.x, m_IconSize.y, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState ); 
		gSimple2D.AddJustification( m_Justify );
		gSimple2D.AddTexUVs(0, GetTextureParam( 3 ) );
	}
}

void UIVinylStripBar::RenderShape( const C_AdobeShape* pShape, const TransformIndex ti, const Maths::HVec3& drawPos, const F32 drawSizeX, const F32 drawSizeY, const C_UIColour& fillColour )
{
	for ( S32 i = 0; i < pShape->Fill().Size(); i++ )
	{
		// Push the sprite onto the stack
		gSimple2D.AddSprite( m_Layer, drawPos, drawSizeX, drawSizeY, fillColour, m_Panel, E_SpriteType_TriList, m_ShapeRs, (U32)pShape->Fill()[ i ] ); 
		gSimple2D.AddTexUVs(0, gUITextureManager.GetTextureByName("null"), 0.0f, 0.0f, 1.0f, 1.0f);

		gSimple2D.AddPostTransformIndex( ti );
	}
}

void UIVinylStripBar::Render()
{
	if ( !m_Activated )
		return;

	// auto-transform
	C_UIAutoTransform transform( this );

	// Render the list
	int currentItem = C_VinylManager::Instance().scCurrent();
	VinylList& list = C_VinylManager::Instance().scList();

	RenderStrip( list, currentItem, 0.0f, 0.0f, 0 );	
}

void UIVinylStripBar::RenderStripSub(VinylList* list, int root, Maths::HVec3 renderPos, U32 depth )
{
	float squareWidth	= C_VinylManager::Instance().CoordRange().X() / m_BackingSize.x / m_SceneScale.x;
	float squareHeight	= C_VinylManager::Instance().CoordRange().Y() / m_BackingSize.y / m_SceneScale.y;

	Maths::HMat43 mat;
	mat.Rotation( Maths::C_MatrixScale( 1.0f / squareWidth, 1.0f / squareHeight, 1.0f ) );
	mat.Translation( renderPos + m_Position );

	m_RenderState.m_EffectParam		= (U32)list;

	for(UI::VinylIterator it = GetChildIterator(list, root); it; ++it)
	{
		VinylItem *p = list->At( it );

		if( !p )
			continue;

		if (p->child >= 0)
		{
			RenderStripSub(list, it, renderPos, depth + 1);
			continue;
		}
		// Assign matrix
		C_VinylManager::Instance().AssignStripMatrix( p->self, mat );

		m_RenderState.m_EffectParam2 = p->self;
		gSimple2D.AddSprite( m_Layer, m_Position, m_Size.x, m_Size.y * Up(), m_SpriteColours, m_Panel, (E_SpriteType)E_GameSpriteType_VinylShape, m_RenderState, 0 );
		gSimple2D.AddPostTransformIndex( 1 );
	}
}


void UIVinylStripBar::RenderItem( VinylList& list, int item, bool selected, F32 x, U32 depth )
{
	VinylItem *p = list.At( item );
	if( !p )
		return;

	F32 fs = depth == 1 ? m_UnfoldScale : 1.0f;
	F32 d = (F32)depth * fs;

	Maths::HVec3 renderPos( x + m_MoveOffset, d * m_StepDepth.y, d * m_StepDepth.z );
//	renderPos += m_Position;

	RenderBacking( selected, p->Highlight(), (p->child >= 0), renderPos + m_Position, depth );

	if (p->child >= 0)
	{
		RenderStripSub(&list, item, renderPos, depth );
		return;
	}

	const C_AdobeShape* pShape = C_VinylManager::Instance().ResolveShape( p->Shape() );
	if( !pShape )
		return;

	F32 squareWidth	= C_VinylManager::Instance().CoordRange().X() / m_BackingSize.x / m_SceneScale.x;
//	float squareHeight	= C_VinylManager::Instance().CoordRange().Y() / m_BackingSize.y / m_SceneScale.y;

	Maths::HMat43 mat;	
	mat.Rotation( Maths::C_MatrixScale( 1.0f / squareWidth, 1.0f / squareWidth, 1.0f ) );
	mat.Translation( renderPos + m_Position );

	// Assign matrix
	C_VinylManager::Instance().AssignStripMatrix( p->self, mat );

	m_RenderState.m_EffectParam	= (U32)&list;
	m_RenderState.m_EffectParam2 = p->self;
	gSimple2D.AddSprite( m_Layer, m_Position, m_Size.x, m_Size.y * Up(), m_SpriteColours, m_Panel, (E_SpriteType)E_GameSpriteType_VinylShape, m_RenderState, 0 );
	gSimple2D.AddPostTransformIndex( 1 );
}

void UIVinylStripBar::RenderStrip( VinylList& list, int currentItem, F32 x_start, F32 x_end, U32 depth )
{
	if( currentItem < 0 )
		return;

	Maths::HMat43 mat;
	mat.SetIdentity();

	F32 fs = depth == 1 ? m_UnfoldScale : 1.0f;
	F32 d = (F32)depth * fs;
	fs = 1.0f;

	F32 ss = (depth == 0) ? m_StepSize * m_SplitScale : m_StepSize;
	
	VinylItem *p = list.At( currentItem );

	// Render parent?
	if( depth < 3 )
	{
		if( p && p->parent > 0 )
		{
			S32 first = 0;
			for(VinylIterator rev( GetSiblingIterator( &list, currentItem ) ); rev; first++, --rev )
				if((int)rev == list.GetFirstSibling( currentItem ))
					break;

			S32 last = 0;
			for(VinylIterator it( GetSiblingIterator( &list, currentItem ) ); it; last++, ++it )
				if((int)it == list.GetLastSibling( currentItem ))
					break;

			RenderStrip( list, p->parent, (ss * last) + (x_start * fs), (ss * first) + (x_end * fs), depth + 1 );
		}
	}

	VinylIterator it( GetSiblingIterator( &list, currentItem ) );
	for( S32 i = 0; i < 10; i++ )
	{
		if( i )
		{
			F32 x = (i * ss ) + (x_start * fs);

			if ( it )
				RenderItem( list, it, false, x, depth );
			else
			if( p && p->parent <= 0 )
			{
				Maths::HVec3 renderPos( x + m_MoveOffset, d * m_StepDepth.y, d * m_StepDepth.z );

				gSimple2D.AddSprite( m_Layer, renderPos + m_Position, m_BackingSize.x, m_BackingSize.y, m_BackingColours, m_Panel, E_SpriteType_Sprite, m_RenderState ); 
				gSimple2D.AddJustification( m_Justify );
				gSimple2D.AddTexUVs(0, GetTextureParam( 1 ) );
			}		
		}

		if( it )
			++it;
	}


	// Backwards
	VinylIterator rev( GetSiblingIterator( &list, currentItem ) );
	for( S32 i = 0; i < 10; i++  )
	{
		if( i )
		{
			F32 x = (ss * -i) - (x_end * fs);
			if( rev )
				RenderItem( list, rev, false, x, depth );
			else
			if( p && p->parent <= 0 )
			{
				Maths::HVec3 renderPos( x + m_MoveOffset, d * m_StepDepth.y, d * m_StepDepth.z );

				gSimple2D.AddSprite( m_Layer, renderPos + m_Position, m_BackingSize.x, m_BackingSize.y, m_BackingColours, m_Panel, E_SpriteType_Sprite, m_RenderState ); 
				gSimple2D.AddJustification( m_Justify );
				gSimple2D.AddTexUVs(0, GetTextureParam( 1 ) );
			}
		}

		if( rev )
			--rev;
	}

	// Render seleted
	if( depth == 0 )
		RenderItem( list, currentItem, true, (x_start*fs), depth );
}

}; // namespace UI
