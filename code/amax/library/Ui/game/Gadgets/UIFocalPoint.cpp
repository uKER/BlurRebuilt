
#include "stdafx.h"

#include <ui/Game/Gadgets/ButtonMessages.h>

#include "UIFocalPoint.h"

// XT Definition
XT_TYPE( UI::UIFocalPoint, "UIFocalPoint" )
XT_BASE( UI::C_GadgetBase )
XT_END_TYPE

Debug::Bool	g_UiDebugPostProcessDOFShowFocusZone("UI.PostProcess.DepthOfField.ShowFocusZone", false );


namespace UI
{

Maths::HVec3	UIFocalPoint::m_FocalPoint;
bool			UIFocalPoint::m_FocalPointExists;

UIFocalPoint::~UIFocalPoint()
{
	m_FocalPointExists = false;
}


void UIFocalPoint::Init()
{
	C_GadgetBase::Init();
}


void UIFocalPoint::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	m_FocalPointExists = true;

	AddSubscriber( E_UiMsg_FrameUpdate );
}


void UIFocalPoint::FrameUpdate()
{
	C_GadgetBase::FrameUpdate();
}


void UIFocalPoint::Render()
{
	C_GadgetBase::Render();

	C_UIAutoTransform transform( this );

	TransformIndex ti = gSimple2D.GetAutoTransform();
	if( ti )
		m_FocalPoint = Maths::HVec3() * gSimple2D.GetPostTransformMatrix( ti );

#ifndef CFG_GOLD
	if ( g_UiDebugPostProcessDOFShowFocusZone )
	{
		gSimple2D.AddSprite( m_Layer, m_Position, m_Size.x, m_Size.z, m_SpriteColours, m_Panel );
		gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ) );
		gSimple2D.AddJustification( E_Justify_MiddleCentre );
		gSimple2D.AddRotation( 90.0f, 0.0f, 0.0f );
	}
#endif // CFG_GOLD
}

}; // namespace UI
