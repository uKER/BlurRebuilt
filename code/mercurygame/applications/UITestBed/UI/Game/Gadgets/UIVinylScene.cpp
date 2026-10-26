
#include "stdafx.h"

#include "UIVinylScene.h"
#include "VinylManager.h"

#include <ui/Game/Gadgets/ButtonMessages.h>
#include <ui/Graphics/SimpleTimer.h>
#include <Ui/Illustrator/UIAdobeShape.h>

// XT Definition
XT_TYPE( UI::UIVinylScene, "UIVinylScene" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_AlphaStep, "alpha_step" )
	XT_FIELD( m_RotateStep, "rotate_step" )
	XT_FIELD( m_TranslateStep, "translate_step" )
	XT_FIELD( m_ShearStep, "shear_step" )
	XT_FIELD( m_ScaleStep, "scale_step" )
	XT_FIELD( m_AlphaNudge, "alpha_nudge" )
	XT_FIELD( m_RotateNudge, "rotate_nudge" )
	XT_FIELD( m_TranslateNudge, "translate_nudge" )
	XT_FIELD( m_ShearNudge, "shear_nudge" )
	XT_FIELD( m_ScaleNudge, "scale_nudge" )
XT_END_TYPE

namespace UI
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UIVinylScene::Init()
{
	C_GadgetBase::Init();

	// Add controller subscribers
	AddSubscriber( E_UiMsg_CtrlButton_Up );
	AddSubscriber( E_UiMsg_CtrlButton_Down );
	AddSubscriber( E_UiMsg_CtrlButton_Left );
	AddSubscriber( E_UiMsg_CtrlButton_Right );
	AddSubscriber( E_UiMsg_CtrlButton_A );
	AddSubscriber( E_UiMsg_CtrlButton_B );
	AddSubscriber( E_UiMsg_CtrlButton_X );
	AddSubscriber( E_UiMsg_CtrlButton_Y );
	AddSubscriber( E_UiMsg_CtrlButton_Start );
	AddSubscriber( E_UiMsg_CtrlButton_Back );
	AddSubscriber( E_UiMsg_CtrlButton_RightTrigger );
	AddSubscriber( E_UiMsg_CtrlButton_LeftTrigger );
	AddSubscriber( E_UiMsg_CtrlButton_LeftShoulder );
	AddSubscriber( E_UiMsg_CtrlButton_RightShoulder );
	AddSubscriber( E_UiMsg_CtrlLeftStick_X );
	AddSubscriber( E_UiMsg_CtrlLeftStick_Y );
	AddSubscriber( E_UiMsg_CtrlRightStick_X );
	AddSubscriber( E_UiMsg_CtrlRightStick_Y );

	AddSubscriber( E_UiMsg_FrameUpdate );
}

bool UIVinylScene::MessageHandler(const Messaging::C_MessageBase *pMessage)
{
	if ( !m_Activated || !m_Selected )
		return C_GadgetBase::MessageHandler( pMessage );

	// Handle Pad messages
	switch( pMessage->Type() )
	{
	case E_UiMsg_CtrlButton_B:
		C_VinylManager::Instance().Save();
		return true;

	case E_UiMsg_CtrlButton_Left:
	case E_UiMsg_CtrlButton_Right:
	case E_UiMsg_CtrlLeftStick_X:
		// Edit
		{
			F32 a = 0.0f;
			bool nudge = true;
			if( pMessage->Type() == E_UiMsg_CtrlLeftStick_X )
			{
				const C_UiMessagePadData* pUIMessage = (C_UiMessagePadData*)pMessage;
				a = fabsf( pUIMessage->amount ) * pUIMessage->amount * C_VinylManager::Instance().List().m_MoveScale;
				nudge = false;
			}
			else
			if( pMessage->Type() == E_UiMsg_CtrlButton_Left )
				a =-1.0f;
			else
			if( pMessage->Type() == E_UiMsg_CtrlButton_Right )
				a = 1.0f;

			switch( C_VinylManager::Instance().EditMode() )
			{
			case C_VinylManager::E_EditMode_Translate :
				C_VinylManager::Instance().Translate( true, nudge ? (a * m_TranslateNudge) : (a * m_TranslateStep), nudge );
				break;

			case C_VinylManager::E_EditMode_Scale :
				C_VinylManager::Instance().Scale( true, nudge ? (a * m_ScaleNudge) : (a * m_ScaleStep), nudge );
				break;

			case C_VinylManager::E_EditMode_Rotate :
				C_VinylManager::Instance().Rotate( nudge ? (a * m_RotateNudge) : (a * m_RotateStep), nudge );
				break;

			case C_VinylManager::E_EditMode_Shear :
				C_VinylManager::Instance().Shear( true, nudge ? (a * m_ShearNudge) : (a * m_ShearStep), nudge );
				break;

			case C_VinylManager::E_EditMode_GradientRotate : 
				C_VinylManager::Instance().ColourRotate( nudge ? (a * m_RotateNudge) : (a * m_RotateStep), nudge );
				break;

			case C_VinylManager::E_EditMode_GradientTranslate :
				C_VinylManager::Instance().ColourTranslate( nudge ? (a * m_ScaleNudge) : (a * m_ScaleStep), nudge );
				break;

			case C_VinylManager::E_EditMode_GradientScale :
				C_VinylManager::Instance().ColourScale( nudge ? (a * m_ScaleNudge) : (a * m_ScaleStep), nudge );
				break;
			}
		}
		return true;

	case E_UiMsg_CtrlButton_Up:
	case E_UiMsg_CtrlButton_Down:
	case E_UiMsg_CtrlLeftStick_Y:
		// Edit
		{
			F32 a = 0.0f;
			bool nudge = true;
			if( pMessage->Type() == E_UiMsg_CtrlLeftStick_Y )
			{
				const C_UiMessagePadData* pUIMessage = (C_UiMessagePadData*)pMessage;
				a = fabsf( pUIMessage->amount ) * pUIMessage->amount * C_VinylManager::Instance().List().m_MoveScale;
				nudge = false;
			}
			else
			if( pMessage->Type() == E_UiMsg_CtrlButton_Up )
				a = 1.0f;
			else
			if( pMessage->Type() == E_UiMsg_CtrlButton_Down )
				a =-1.0f;

			switch( C_VinylManager::Instance().EditMode() )
			{
			case C_VinylManager::E_EditMode_Translate :
				C_VinylManager::Instance().Translate( false, nudge ? (-a * m_TranslateNudge * Up()) : (-a * m_TranslateStep * Up()), nudge );
				break;

			case C_VinylManager::E_EditMode_Scale :
				C_VinylManager::Instance().Scale( false, nudge ? (a * m_ScaleNudge) : (a * m_ScaleStep), nudge );
				break;

			case C_VinylManager::E_EditMode_Shear :
				C_VinylManager::Instance().Shear( false, nudge ? (a * m_ShearNudge) : (a * m_ShearStep), nudge );
				break;
			}
		}
		return true;

	}

	return C_GadgetBase::MessageHandler( pMessage );
}

void UIVinylScene::Render()
{
	if( C_VinylManager::Instance().SceneType() == C_VinylManager::E_SceneType_Avatar )
	{
		RenderList( C_VinylManager::E_List_Avatar );
		return;
	}

	if( C_VinylManager::Instance().SceneType() == C_VinylManager::E_SceneType_Group )
	{
		RenderList( C_VinylManager::E_List_Group );
		return;
	}

	if( C_VinylManager::Instance().SceneType() == C_VinylManager::E_SceneType_Livery )
	{
		C_List* p = NULL;
		// Top
		for( S32 i = C_VinylManager::E_List_LiveryTop; i <= C_VinylManager::E_List_LiveryBack; i++ )
		{
			p = &C_VinylManager::Instance().List( (C_VinylManager::E_List)i );
			gSimple2D.AddSprite( m_Layer, Maths::HVec3( p->m_ClipX, p->m_ClipY, 0 ), p->m_ClipW, p->m_ClipH, m_SpriteColours[0], m_Panel, E_SpriteType_SetClipWindow, m_RenderState, 1 );
			RenderList( (C_VinylManager::E_List)i );
			gSimple2D.AddSprite( m_Layer, Maths::HVec3( 0, 0, 0 ), 0.0f, 0.0f, m_SpriteColours, m_Panel, E_SpriteType_SetClipWindow, m_RenderState, 0 );
		}
	}
}

void UIVinylScene::RenderList( const C_VinylManager::E_List list_id )
{
	// Render scene
	C_List& rlist = C_VinylManager::Instance().List( list_id );
	VinylList& list = rlist.m_scList;

	m_RenderState.m_EffectParam		= (U32)&rlist;
	m_RenderState.m_EffectParam2	= (U32)list.Root();

	rlist.CacheMatrices();

	C_UIClipWindow clipW = gSimple2D.AddClipWindowAuto( this );

	gSimple2D.AddSprite( m_Layer, m_Position, m_Size.x, m_Size.y, m_SpriteColours[0], m_Panel, (E_SpriteType)E_GameSpriteType_VinylList, m_RenderState, 0 );

	if( C_VinylManager::Instance().CurrentList() != list_id )
		return;

	// Render Highlighted objects
	int current = C_VinylManager::Instance().scCurrent( list_id );
	VinylItem *p = list.At( current );
	if( !p )
		return;
	if( p->Highlight() )
	{    
		// Highlight entire list
		for(VinylIterator it( GetFirstSiblingIterator( &list, current ) ); it; ++it )
		{
			p = list.At( it );
			
			if( !p || !p->Highlight() )
				continue;

			// Render the children
			if( p->child >= 0 )
			{
				m_RenderState.m_EffectParam2	= (U32)p->child;
				gSimple2D.AddSprite( m_Layer, m_Position, m_Size.x, m_Size.y, m_SpriteColours[0], m_Panel, (E_SpriteType)E_GameSpriteType_VinylList, m_RenderState, 1 );
				continue;
			}

			// Render the shape
			m_RenderState.m_EffectParam2	= (U32)p->self;
			gSimple2D.AddSprite( m_Layer, m_Position, m_Size.x, m_Size.y, m_SpriteColours[0], m_Panel, (E_SpriteType)E_GameSpriteType_VinylShape, m_RenderState, 1 );
		}
	}
	else
	{
		// Render the children
		if( p->child >= 0 )
		{
			m_RenderState.m_EffectParam2	= (U32)p->child;
			gSimple2D.AddSprite( m_Layer, m_Position, m_Size.x, m_Size.y, m_SpriteColours[0], m_Panel, (E_SpriteType)E_GameSpriteType_VinylList, m_RenderState, 1 );
		}
		else
		{
			// Render the shape
			m_RenderState.m_EffectParam2	= (U32)p->self;
			gSimple2D.AddSprite( m_Layer, m_Position, m_Size.x, m_Size.y, m_SpriteColours[0], m_Panel, (E_SpriteType)E_GameSpriteType_VinylShape, m_RenderState, 1 );
		}
	}
}

}; // namespace UI
