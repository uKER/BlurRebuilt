#ifndef TEXTEFFECT_H
#define TEXTEFFECT_H

#include "ui/Graphics/Font/Font.h"

#include <btl/Containers/containers.h>

#ifdef _MSC_VER
#pragma once
#endif

namespace UI
{

class C_TextEffect : public C_FontPreRender
{
public:
	void			Init(const F32 letter_inc_time, const F32 letter_dec_speed);
	void			Reset( );
	void			Update( );

	// Process the font sprite list
	virtual void	ProcessStack( C_FontSpriteStack& stack ) = 0;
	// Set current letter count
	void			LetterCount( const U32 letter_count )							{ m_LettersCount = letter_count; }

protected:
	static const U32													m_MaxLetters = 512;
	Btl::Vector< F32, Btl::VectorParameters_Array< m_MaxLetters > >		m_LetterTimes;

	U32													m_LettersCount;			// Letter count (can be longer than string)
	U32													m_CurrentLetter;		// Current letter index
	
	F32													m_LetterIncCount;		// Current time in *this* letter
	F32													m_LetterIncTime;		// Time between each letter
	F32													m_LetterDecSpeed;		// Effect time for each letter
};

class C_TextEffectPulse : public C_TextEffect
{
public:
	void ProcessStack( C_FontSpriteStack& stack );
};

}; // namespace UI

#endif // TEXTEFFECT_H

