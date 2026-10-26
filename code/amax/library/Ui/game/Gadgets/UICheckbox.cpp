
#include "stdafx.h"

#include "UICheckbox.h"

#include "Ui/SoundEffects.h"

#include <Ui/Game/Gadgets/ButtonMessages.h>

// XT Definition
XT_TYPE( UI::UICheckbox, "UICheckBox" )
	XT_BASE( UI::UIStaticText )
	XT_FIELD( m_IconSize, "icon_size" )
	XT_FIELD( m_Checked, "checked" )
XT_END_TYPE


namespace UI
{

void UICheckbox::Init()
{
	UIStaticText::Init();

	m_Checked = false;
	m_IconSize = 2.0f;

	m_CallbackName[0] = '\0';
}

void UICheckbox::Setup( lua_State* pState )
{
	UIStaticText::Setup( pState );

	AddSubscriber( E_UiMsg_MenuNext );
	AddSubscriber( E_UiMsg_CtrlButton_Left );
	AddSubscriber( E_UiMsg_CtrlButton_Right );
}

void UICheckbox::PostInit()
{
	UIStaticText::PostInit();
}

bool UICheckbox::MessageHandler( const Messaging::C_MessageBase *pMessage )
{
	if ( !m_Activated || !m_Selected )
		return UIStaticText::MessageHandler( pMessage );

	if ( pMessage->Type() == E_UiMsg_CtrlButton_Left || pMessage->Type() == E_UiMsg_CtrlButton_Right )
	{
//	if ( pMessage->Type() == E_UiMsg_MenuNext )
//	{
		m_Checked = !m_Checked;

		UiInterface()->PlaySound( E_UiSoundEffect_Toggle );

		// Call function assigned to this gadget in lua
		lua_State*	pState = g_pUILua->GetState();
		if( pState && strlen( m_CallbackName ) )
		{
			Lua::Call( pState, m_CallbackName, "b", m_Checked );
		}
	}
	return UIStaticText::MessageHandler( pMessage );
}


void UICheckbox::Render()
{
	if ( !m_Activated )
		return;

	C_UIAutoTransform transform( this );
	C_GadgetBase::Render();

	// Set colour green or red. Yeah i hardcoded it. So what
	m_SpriteColours.SetByName( ( m_Checked ) ? "Support_0" : "Support_3", m_AlphaInherit, m_IntensityInherit );

	// Draw tick box
	E_Justify justify = E_Justify_None;

	switch ( m_Justify )
	{
		case E_Justify_TopLeft :		justify = E_Justify_TopRight; 		break;
		case E_Justify_MiddleLeft :		justify = E_Justify_MiddleRight; 	break;
		case E_Justify_BottomLeft :		justify = E_Justify_BottomRight; 	break;

		case E_Justify_TopRight :		justify = E_Justify_TopLeft; 		break;
		case E_Justify_MiddleRight :	justify = E_Justify_MiddleLeft; 	break;
		case E_Justify_BottomRight :	justify = E_Justify_BottomLeft; 	break;
	}

	gSimple2D.AddSprite( m_Layer, m_Position, m_IconSize, m_IconSize, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddTexUVs( 0, GetTextureParam( m_Checked ) );
	gSimple2D.AddJustification( justify );
}


void UICheckbox::AssignCallback( const char* pName )
{
	if ( pName )
		sprintf_s( m_CallbackName, m_MaxName, pName );
}

}; // namespace UI