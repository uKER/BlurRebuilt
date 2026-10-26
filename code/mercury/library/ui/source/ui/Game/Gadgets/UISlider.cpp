#include "stdafx.h"

#include "ui/UiInterface.h"

#include "ui/Game/UILua.h"
#include "ui/Game/UIDataFinder.h"

#include "ui/Game/Gadgets/UISlider.h"
#include "ui/Game/Gadgets/ButtonMessages.h"

#include "ui/Graphics/Font/Font.h"
#include "ui/Graphics/SimpleTimer.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

XT_TYPE( UI::UISlider::E_Type, "UISliderType" )
	XT_OPTION( UI::UISlider::E_Type_Analogue, "Analogue" )
	XT_OPTION( UI::UISlider::E_Type_Variable, "Variable" )
XT_END_TYPE

XT_TYPE( UI::UISlider, "UISlider" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_SliderType, "slider_type" )
	XT_FIELD( m_Value, "value" )
	XT_FIELD( m_Step, "step" )
	XT_FIELD( m_FontHeight, "font_height" )
	XT_FIELD( m_VariableName,	"text_string" )
	XT_FIELD( m_TextID,			"text_id" )
XT_END_TYPE

namespace UI
{

void UISlider::Init()
{
	BaseUIVariableSelect::Init();

	m_Value = 0;
	m_Step = 0;
	m_ButtonDown = false;
	m_MouseDragging = false;
	m_sSoundDown = m_sSoundUp = -1;
	m_Pad = -1;

	AddSubscriber( E_UiMsg_FrameUpdate );
	AddSubscriber( E_UiMsg_CtrlButton_Left );
	AddSubscriber( E_UiMsg_CtrlButton_Right );
	AddSubscriber( E_UiMsg_CtrlLeftStick_X );
}

void UISlider::Setup( lua_State *pState )
{
	BaseUIVariableSelect::Setup( pState );

	// Defaults
	m_fSelectorTextOffsetX = m_Size.x;
	m_fSelectorTextOffsetY = m_FontHeight * 2.0f;
	m_fSelectorIconWidth = m_Size.y * 2.0f;
	m_fSelectorIconHeight = m_Size.y * 2.0f;
	m_sAlphaAmount = 64;
	m_eSelectorTextJustify = E_Justify_TopRight;

	if( m_TextID.id != 0 )
		m_Text = m_TextID.id;

	if( pState )
	{
	//	m_Value				= ( Lua::DataMemberType( pState, "value" ) == LUA_TNUMBER )		? (F32)Lua::GetDataMember( pState, "value" ) : 0.0f;
	//	m_Step				= ( Lua::DataMemberType( pState, "step" ) != LUA_TNIL)			? (F32)Lua::GetDataMember( pState, "step" ) : 0.05f;
	//	m_SliderType		= ( Lua::DataMemberType( pState, "slider_type" ) != LUA_TNIL)	? (E_Type)(U32)Lua::GetDataMember( pState, "slider_type" ) : E_Type_Analogue;

		// text offset for selected item
		if ( Lua::DataMemberType( pState, "selectorTextOffsetX" ) == LUA_TNUMBER )	
			Lua::GetDataMember( pState, "selectorTextOffsetX", m_fSelectorTextOffsetX );

		if ( Lua::DataMemberType( pState, "selectorTextOffsetY" ) == LUA_TNUMBER )
			Lua::GetDataMember( pState, "selectorTextOffsetY", m_fSelectorTextOffsetY );
		
		// selector icon
		if ( Lua::DataMemberType( pState, "selectorIconWidth" ) == LUA_TNUMBER )
			Lua::GetDataMember( pState, "selectorIconWidth", m_fSelectorIconWidth );

		if ( Lua::DataMemberType( pState, "selectorIconHeight" ) == LUA_TNUMBER )
			Lua::GetDataMember( pState, "selectorIconHeight", m_fSelectorIconHeight );

		if ( Lua::DataMemberType( pState, "selectorTextJustify" ) == LUA_TNUMBER )
			m_eSelectorTextJustify = (E_Justify)Lua::GetDataMember<S32>( pState, "selectorTextJustify" );
		
		// de-selected alpha-ness
		if ( Lua::DataMemberType( pState, "alphaAmount" ) == LUA_TNUMBER )
			Lua::GetDataMember( pState, "alphaAmount", m_sAlphaAmount );

		// font
	//	m_FontHeight		= (F32)Lua::GetDataMember( pState, "font_height", true );

		// sounds
		Lua::GetDataMember( pState, "soundUp", m_sSoundUp, true );
		Lua::GetDataMember( pState, "soundDown", m_sSoundDown, true );
	}

	// Setup font interface
	C_FontInterfaceParam interfaceParam;

	if( interfaceParam.Setup( pState ) )
		m_RenderState.m_EffectParam = interfaceParam.GetEffectParam();

#ifdef PLATFORM_PC
	// Setup mouse bounding box stuff
	m_Mouse.clickable	= true;
	m_Mouse.bb_left		= m_Position.x - m_fSelectorIconWidth * 0.25f;
	m_Mouse.bb_right	= m_Position.x + m_Size.x + m_fSelectorIconWidth * 0.25f;
	m_Mouse.bb_top		= m_Position.y - ( m_fSelectorIconHeight * 0.25f );
	m_Mouse.bb_bottom	= m_Position.y + ( m_fSelectorIconHeight * 0.25f );
#endif
}

bool UISlider::MessageHandler(const Messaging::C_MessageBase *pMessage)
{
	if ( !m_Activated )
		return BaseUIVariableSelect::MessageHandler( pMessage );

	bool ret = false;
	switch( pMessage->Type() )
	{
	// Handle controller messages
	case E_UiMsg_CtrlButton_Left:
	case E_UiMsg_CtrlButton_Right:
	case E_UiMsg_CtrlLeftStick_X:
		if( m_Selected )
		{
			const C_UiMessagePadData* pUIMessage = (C_UiMessagePadData*)pMessage;
			if( pUIMessage && ( m_Pad == -1 || m_Pad == pUIMessage->pad ) )
			{
				if ( m_SliderType == E_Type_Analogue )
				{
					if( pMessage->Type() == E_UiMsg_CtrlButton_Left && pUIMessage->deb )
					{
						// Need to round this up.
						m_Value = (F32)((S32)(m_Value * 10.0f + 0.5f ) ) / 10 - m_Step;

						if ( m_Value >= 0.0f )
						{
							if ( m_sSoundDown != -1 )
								UiInterface()->PlaySound( m_sSoundDown );
						}
					}
					else
					if( pMessage->Type() == E_UiMsg_CtrlButton_Right && pUIMessage->deb )
					{
						// Need to round this down.
						m_Value = (F32)((S32)( m_Value * 10.0f ) ) / 10 + m_Step;

						if ( m_Value <= 1.0f )
						{
							if ( m_sSoundUp != -1 )
								UiInterface()->PlaySound( m_sSoundUp );
						}
						
					}
					else
					if( pMessage->Type() == E_UiMsg_CtrlLeftStick_X )
					{
						// deadzone for slider
						if ( Maths::Abs< F32 >( pUIMessage->amount ) < 0.015f )
							return true;

						F32 fAmount = pUIMessage->amount;

						m_Value += ( fAmount * theFETimer.FrameTime() );
						
						if ( fAmount > 0.0f )
						{
							if ( m_Value <= 1.0f )
							{
								if ( m_sSoundUp != -1 )
									UiInterface()->PlaySound( m_sSoundUp );
							}
						}
						else
						{
							if ( m_Value >= 0.0f )
							{
								if ( m_sSoundDown != -1 )
									UiInterface()->PlaySound( m_sSoundDown );
							}
						}
					}

#ifdef PLATFORM_PC
					// Handle mouse
					if ( UiInterface()->UseMouseInput() )
					{
						F32 mouse_x;
						F32 mouse_y;

						UI::UiInterface()->GetMousePos( mouse_x, mouse_y );

						mouse_x *= gSimple2D.ScreenWidth();
						mouse_x += gSimple2D.ScreenLeft();
						mouse_y *= UI_VSCREEN_SIZEY;

						// Must be dragging the slider around.
						if ( m_MouseDragging && UI::UiInterface()->GetLeftMouseDown() )
						{
							// Mouse can move left or right
							F32 denom = ( m_Position.x + m_Size.x ) - m_Position.x;
							F32 pos = ( mouse_x / denom ) - ( m_Position.x / denom );

							float tmpVal = m_Value;
							m_Value = ( mouse_x - m_Position.x ) / m_Size.x;
							m_Value = Maths::Clamped( m_Value, 0.0f, 1.0f );
							
							// Play a sound.
							if ( tmpVal < m_Value )
							{
								if ( m_sSoundUp != -1 )
									UiInterface()->PlaySound( m_sSoundUp );
							}
							if ( tmpVal > m_Value )
							{
								if ( m_sSoundDown != -1 )
									UiInterface()->PlaySound( m_sSoundDown );
							}
						}
						else if ( UI::UiInterface()->GetLeftMouseDown() )
						{
							// Get mouse over button part?
							F32 offset = ( ( m_Items.Size() - 1 ) > 0) ? ( (F32)m_CurrentItem / (F32)(m_Items.Size() - 1 ) ) : 0.5f;

							if ( ( mouse_x > ( m_Position.x + m_Size.x * offset ) ) && ( mouse_x < ( m_Position.x + m_Size.x * offset + m_fSelectorIconWidth ) ) )
							{
								m_MouseDragging = true;
							}
							// Mouse click was on the actual background
							else if ( ( mouse_x > m_Position.x ) && ( mouse_x < ( m_Position.x + m_Size.x ) ) )
							{
								// Mouse can move left or right
								F32 denom = ( m_Position.x + m_Size.x ) - m_Position.x;
								F32 pos = ( mouse_x / denom ) - ( m_Position.x / denom );

								float tmpVal = m_Value;
								m_Value = ( mouse_x - m_Position.x ) / m_Size.x;
								m_Value = Maths::Clamped( m_Value, 0.0f, 1.0f );
								
								// Play a sound.
								if ( tmpVal < m_Value )
								{
									if ( m_sSoundUp != -1 )
										UiInterface()->PlaySound( m_sSoundUp );
								}
								if ( tmpVal > m_Value )
								{
									if ( m_sSoundDown != -1 )
										UiInterface()->PlaySound( m_sSoundDown );
								}
							}
						}
						else
						{
							m_MouseDragging = false;
						}
					}
#endif
				}
				else if ( m_SliderType == E_Type_Variable )
				{
					if( ( pMessage->Type() == E_UiMsg_CtrlButton_Left && pUIMessage->deb ) || ( pMessage->Type() == E_UiMsg_CtrlLeftStick_X && pUIMessage->amount < -0.05f ) )
					{
						if ( !m_ButtonPrevDown && !m_ButtonDown )
						{
							SetPreviousItem();
						}

						m_ButtonDown = true;
					}
					else 
					if( ( pMessage->Type() == E_UiMsg_CtrlButton_Right && pUIMessage->deb ) || ( pMessage->Type() == E_UiMsg_CtrlLeftStick_X && pUIMessage->amount > 0.05f ) )
					{
						if ( !m_ButtonPrevDown && !m_ButtonDown )
						{
							SetNextItem();
						}

						m_ButtonDown = true;
					}
				}
			}
		}
		// Clamp the value
		m_Value = Maths::Clamped( m_Value, 0.0f, 1.0f );
		// Message processed okay
		ret = true;
		break;
		
	default:
		ret = BaseUIVariableSelect::MessageHandler( pMessage );
		break;
	}

	return ret;
}

void UISlider::FrameUpdate( )
{
	m_ButtonPrevDown = m_ButtonDown;
	m_ButtonDown = false;

	BaseUIVariableSelect::FrameUpdate();
}

void UISlider::Render( )
{
	if(!m_Activated)
		return;	

	BaseUIVariableSelect::Render();
		
	// auto-transform
	C_UIAutoTransform transform( this );

	C_FontStylePasses& style = (m_Selected) ? m_StyleSelected : m_StyleDeselected;
	// because fonts can have different a shader index, need a different render state for graphic
	C_RenderState graphicRenderState = m_RenderState;
	graphicRenderState.Init();

	C_UIColour colour = 0xffffffff;
	C_UIColour drawColour = m_SpriteColours[ 0 ];

	C_UISpriteColours effectColour( m_SpriteColours );

	if ( !m_Selected )
	{
		colour.A( m_sAlphaAmount );
		drawColour.A( m_sAlphaAmount );
	}

	// Top text
	if ( m_Text.IsLocString() )
	{
		gFont.DrawText( m_Layer, m_Position, m_Justify, m_Text, drawColour, m_FontHeight, style, 0, m_Panel );
	}
	else if( !m_Text.IsValid() && m_VariableName && m_VariableName[0] )
	{
		const WCHAR* pString = C_UIDataFinder::Instance().GetVariableText( m_VariableName );
		gFont.DrawText( m_Layer, m_Position, m_Justify, pString, drawColour, m_FontHeight, style, 0, m_Panel );
	}

	F32 y_offset = m_FontHeight * 1.5f;

	// Render the backing image
	gSimple2D.AddSprite( m_Layer, Maths::HVec3( m_Position.x, m_Position.y + y_offset, m_Position.z ), m_Size.x, m_Size.y, colour, m_Panel, E_SpriteType_Sprite, graphicRenderState );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
	gSimple2D.AddJustification( E_Justify_MiddleLeft );

	// Render the selected item
	const S32 textureIndex = m_Selected ? 2 : 1;

	if ( m_SliderType == E_Type_Analogue )
	{
		gSimple2D.AddSprite( m_Layer, Maths::HVec3( m_Position.x + m_Size.x * m_Value, m_Position.y + y_offset, m_Position.z ), m_fSelectorIconWidth, m_fSelectorIconHeight, colour, m_Panel, E_SpriteType_Sprite, graphicRenderState );
	}
	else
	{
		F32 offset = ( ( m_Items.Size() - 1 ) > 0 ) ? ( (F32)m_CurrentItem / (F32)( m_Items.Size() - 1 ) ) : 0.5f;
		gSimple2D.AddSprite( m_Layer, Maths::HVec3( m_Position.x + m_Size.x * offset, m_Position.y + y_offset, m_Position.z ), m_fSelectorIconWidth, m_fSelectorIconHeight, colour, m_Panel, E_SpriteType_Sprite, graphicRenderState );
	}

	gSimple2D.AddTexUVs( 0, GetTextureParam( textureIndex ) );
	gSimple2D.AddJustification( E_Justify_MiddleCentre );

	// Render the selected item's text
	if ( m_SliderType == E_Type_Variable && m_Items.Size() > 0 )
	{
		y_offset += m_Size.y;
		const WCHAR* pString = GetString( m_CurrentItem );
		gFont.DrawText( m_Layer, Maths::HVec3( m_Position.x + m_fSelectorTextOffsetX, m_Position.y + m_fSelectorTextOffsetY, m_Position.z ), m_eSelectorTextJustify, pString, drawColour, m_FontHeight, style, 0, m_Panel );
	}
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif
