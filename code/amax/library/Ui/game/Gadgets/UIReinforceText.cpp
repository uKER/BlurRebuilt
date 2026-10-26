#include "stdafx.h"

#include "UIReinforceText.h"
#include "UIMessages.h"

#include <ui/Graphics/SimpleTimer.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )
#endif

XT_TYPE( UI::UIReinforceText, "UIReinforceText" )
	XT_BASE( UI::UIStaticText  )
	XT_FIELD( m_ScaleRange, "scale_range" )
	XT_FIELD( m_AlphaRange, "alpha_range" )
	XT_FIELD( m_Amount, "amount" )
XT_END_TYPE

namespace UI
{

// -----------
// UIReinforceText
// -----------
void UIReinforceText::Init()
{
	UIStaticText::Init();

	m_Amount = 0;
	m_ScaleRange = 0.0f;
	m_AlphaRange = 0.0f;
}

void UIReinforceText::Render()
{
	if ( !m_Activated )
		return;

	// Render base
	UIStaticText::Render();

	if( m_Amount <= 0.0f || m_Amount >= 1.0f )
		return;

	if( m_AlphaRange <= 0.0f )
		return;

	// Render the effecty bit
	const F32 scale = (m_ScaleRange * m_Amount) + 1.0f;
	const F32 inv = (1.0f - m_Amount);
	const Maths::HVec3 joffset( gSimple2D.GetJustificationOffset( E_Justify_MiddleCentre, m_Panel ) - gSimple2D.GetJustificationOffset( m_Justify, m_Panel ) );

	// Store
	const F32 msx = m_Scale.x;
	const F32 msy = m_Scale.y;

	const F32 px = m_Position.x;
	const F32 py = m_Position.y;

	// Bang our new vars in
	m_Scale.x *= scale;
	m_Scale.y *= scale;

	// Scale in / out from centre
	F32 ox = m_Size.x * joffset.X() * msx;
	F32 oy = m_Size.y * joffset.Y() * msy;
	m_Position.x = m_Position.x - (ox*(scale-1));
	m_Position.y = m_Position.y - (oy*(scale-1));

	// Fade
	const F32 fade = Maths::Clamped( m_Alpha * inv * 1.25f, 0.0f, 1.0f );
	ResolveColourStyle( m_Intensity, m_AlphaRange * fade );

	DirtySet( E_Dirty_Matrix, true );

	UIStaticText::Render();

	// Restore
	m_Scale.x = msx;
	m_Scale.y = msy;

	m_Position.x = px;
	m_Position.y = py;

	ResolveColourStyle( m_Intensity, m_Alpha );

	// Dirty again in case someone uses us
	DirtySet( E_Dirty_Matrix, true );
}

}; // namespace UI
