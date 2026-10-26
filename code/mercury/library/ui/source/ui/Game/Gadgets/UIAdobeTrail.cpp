#include "stdafx.h"

#include "Ui/Game/Gadgets/UIAdobeTrail.h"

#include "ui/Graphics/SimpleTimer.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

// XT Definition
XT_TYPE( UI::UIAdobeTrail, "UIAdobeTrail" )
	XT_BASE( UI::UIStaticShape )
	XT_FIELD( m_WaveSpeed,		"wave_speed" )
	XT_FIELD( m_WaveFrequency,	"wave_frequency" )
	XT_FIELD( m_WaveIntensity,	"wave_intensity" )
XT_END_TYPE

namespace UI
{

void UIAdobeTrail::Setup( lua_State* pState )
{
	UIStaticShape::Setup( pState );

	m_WaveFrequency *= Maths::Pi; // Keeps sorcery neater.
	m_Time = 0.0f;

	m_StrokeEffect = E_StrokeEffectTrail;
}


void UIAdobeTrail::Render()
{
	m_Time += m_WaveSpeed * theFETimer.FrameTime();

	gSimple2D.AddSprite( m_Layer, m_Panel, E_SpriteType_SetRendersStateStack, 1 );

	// Turn off culling as this bad boy is going to twist alot.
	gSimple2D.AddSprite( m_Layer, m_Panel, E_SpriteType_SetCullMode, Display::E_CullMode_None );

	UIStaticShape::Render();

	gSimple2D.AddSprite( m_Layer, m_Panel, E_SpriteType_SetRendersStateStack, 0 );
}


void UIAdobeTrail::RenderStroke( const C_AdobeShape* pShape, const Maths::HVec3& vPos, const F32 fSizeX, const F32 fSizeY, const TransformIndex uColourIndex )
{
	// Pack the trail settings into the sprite colours since they are unused for this effect
	U32 tPtr;
	*((F32*)&tPtr) = m_Time;

	U8 a = (tPtr & 0xff);
	U8 r = (tPtr >> 8) & 0xff;
	U8 g = (tPtr >> 16) & 0xff;
	U8 b = (tPtr >> 24) & 0xff;

	m_SpriteColours[1].A( a );
	m_SpriteColours[1].R( r );
	m_SpriteColours[1].G( g );
	m_SpriteColours[1].B( b );

	U32 iPtr;
	*((F32*)&iPtr) = m_WaveIntensity;

	a = (iPtr & 0xff);
	r = (iPtr >> 8) & 0xff;
	g = (iPtr >> 16) & 0xff;
	b = (iPtr >> 24) & 0xff;

	m_SpriteColours[2].A( a );
	m_SpriteColours[2].R( r );
	m_SpriteColours[2].G( g );
	m_SpriteColours[2].B( b );

	U32 fPtr;
	*((F32*)&fPtr) = m_WaveFrequency;

	a = (fPtr & 0xff);
	r = (fPtr >> 8) & 0xff;
	g = (fPtr >> 16) & 0xff;
	b = (fPtr >> 24) & 0xff;

	m_SpriteColours[3].A( a );
	m_SpriteColours[3].R( r );
	m_SpriteColours[3].G( g );
	m_SpriteColours[3].B( b );

	BaseUIShape::RenderStroke( pShape, vPos, fSizeX, fSizeY, uColourIndex );
}

}; // namespace UI