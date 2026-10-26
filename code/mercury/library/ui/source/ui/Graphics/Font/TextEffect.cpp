#include "stdafx.h"

#include "ui/Graphics/SimpleTimer.h"
#include "ui/Graphics/Font/TextEffect.h"

namespace UI
{
// Base class
void C_TextEffect::Init(const F32 letter_inc_time, const F32 letter_dec_speed)
{
	m_LetterIncTime = letter_inc_time;
	m_LetterDecSpeed = letter_dec_speed;
	m_LettersCount = 64; // Default
	Reset( );
}

void C_TextEffect::Reset( )
{
	m_LetterIncCount = 0.0f;
	m_CurrentLetter = 0;
	for(U32 i = 0; i < m_MaxLetters; i++)
		m_LetterTimes[i] = 0.0f;
}

void C_TextEffect::Update( )
{
	// Effect each letter
	const F32 time = theFETimer.FrameTime();
	for(U32 i = 0; i < m_MaxLetters; i++)
		if(m_LetterTimes[i]>0.0f)
		{
			m_LetterTimes[i] -= time*m_LetterDecSpeed;
			if(m_LetterTimes[i] < 0.0f)
				m_LetterTimes[i] = 0.0f;
		}
	if(m_LetterIncTime>0.0f)
	{
		m_LetterIncCount+=time;
		while(m_LetterIncCount>m_LetterIncTime)
		{
			m_LetterTimes[m_CurrentLetter] = 1.0f;
			m_CurrentLetter++;
			if(m_CurrentLetter>=m_LettersCount)
				m_CurrentLetter = 0; // Wrap

			m_LetterIncCount-=m_LetterIncTime;
		}
	}
}

// Pulse effect
void C_TextEffectPulse::ProcessStack( C_FontSpriteStack& stack )
{
	if( stack.DrawEnd() == 0 || stack.DrawEnd() >= (S32)m_MaxLetters )
		return;

	for(S32 i = stack.DrawStart(); i < stack.DrawEnd(); i++)
	{
		C_FontSpriteItem& fsi = stack[ i ];
		/*
		F32 scale = 1.0f + (sinf(m_LetterTimes[i]*Maths::Pi)*g_TextScaleAmount);
//		fsi.m_Px += (fsi.m_Sx-(fsi.m_Sx*scale))*0.5f;
		fsi.m_Py += (fsi.m_Sy-(fsi.m_Sy*scale))*0.5f;
//		fsi.m_Sx *= scale;
		fsi.m_Sy *= scale;
		*/
		for(U32 j = 0; j<fsi.m_Col.m_SpriteColoursSize; j++)
		{
			/*
			fsi.m_Col[j].R( (U8)Maths::Lerp(g_UiDebugGraphicColourFrom->r*255.0f,g_UiDebugGraphicColourTo->r*255.0f,m_LetterTimes[i]) );
			fsi.m_Col[j].G( (U8)Maths::Lerp(g_UiDebugGraphicColourFrom->g*255.0f,g_UiDebugGraphicColourTo->g*255.0f,m_LetterTimes[i]) );
			fsi.m_Col[j].B( (U8)Maths::Lerp(g_UiDebugGraphicColourFrom->b*255.0f,g_UiDebugGraphicColourTo->b*255.0f,m_LetterTimes[i]) );
			fsi.m_Col[j].A( (U8)Maths::Lerp(g_UiDebugGraphicColourFrom->a*255.0f,g_UiDebugGraphicColourTo->a*255.0f,m_LetterTimes[i]) );
			*/
		}
	}
}

}; // namespace UI

