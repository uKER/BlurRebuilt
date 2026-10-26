#include "stdafx.h"

#include <Ui\Graphics\UIColourClasses.h>

#include "UIAmaxGraphic.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )
#endif

// XT Definition
XT_TYPE( UI::UIAmaxGraphic, "UIAmaxGraphic" )
	XT_BASE( UI::UIStaticGraphic )
	XT_FIELD( m_LerpColFromName, "lerp_from" )
	XT_FIELD( m_LerpColToName, "lerp_to" )
XT_END_TYPE

namespace UI
{

void UIAmaxGraphic::Setup( lua_State* pState )
{
	UIStaticGraphic::Setup( pState );

	if( m_RenderState.m_EffectIndex == E_Effect_IntensityLerp )
	{
		// Resolve colour styles
		C_UISpriteColours from;
		C_UISpriteColours to;

		from.SetByName( m_LerpColFromName );
		to.SetByName( m_LerpColToName );

		C_UIColourARGB16 f( from[0] );
		C_UIColourARGB16 t( to[0] );

		// Pack the render state param
		m_RenderState.m_EffectParam = (U16)f | ((U16)t)<<16;
	}
}

}; // namespace UI