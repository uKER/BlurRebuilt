
#include "stdafx.h"

#include <ui/Game/Gadgets/ButtonMessages.h>
#include <ui/Graphics/Simple2D.h>

#include "UIAmaxLight.h"

// XT Definition
XT_TYPE( UI::UIAmaxLight, "UIAmaxLight" )
	XT_BASE( UI::C_GadgetBase )
XT_END_TYPE


namespace UI
{

void UIAmaxLight::Init()
{
	RemoveSubscriber( E_UiMsg_FrameUpdate );
	AddSubscriber( E_UiMsg_Render );

	C_GadgetBase::Init();
}

void UIAmaxLight::Render()
{
	C_GadgetBase::Render();

	C_Renderer::C_Light& l = gSimple2D.GetLight( m_Panel );

	TransformIndex ti;
	CalcTransformHierarchy( ti );

	l.m_Pos			= Matrix().Translation();
	l.m_Dir			= Maths::HVec4( 0, 0, 1.0f, 0 ); // TODO transform this?
	l.m_ColourFrom	= Maths::HVec4( m_SpriteColours[0].R01(), m_SpriteColours[0].G01(), m_SpriteColours[0].B01(), m_SpriteColours[0].A01() );
	l.m_ColourTo	= Maths::HVec4( m_SpriteColours[1].R01(), m_SpriteColours[1].G01(), m_SpriteColours[1].B01(), m_SpriteColours[1].A01() );
	l.m_FallOff		= Maths::HVec4( m_Size.x, m_Size.y, m_Size.z, 1.0f );
}

}; // namespace UI
