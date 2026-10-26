#include "stdafx.h"

#include "ui/Graphics/SimpleTimer.h"

#ifndef CFG_GOLD
Debug::Float g_DebugUiTimeMultiplier( "UI.TimeMultiplier", 1.0f, Debug::Range( 0.0f, 4.0f ) );
#endif

namespace UI
{

SimpleTimer theFETimer;

void SimpleTimer::Update( const Maths::C_TimeS& time )
{
#if defined( GAME_RACERNOTAMAX )
	// TODO:
	thisTickCount = GetTickCount();	
	timer=float(thisTickCount - lastTickCount)/1000.0f;
	lastTickCount = thisTickCount;

	actual_timer=timer;

	if(timer > 1/15.f) timer=1/15.f;
#else
	timer = time.Get()* time_scale;
#ifndef CFG_GOLD
	timer *= g_DebugUiTimeMultiplier;
#endif
#endif

	anim_timer+=timer;

	if(anim_timer > 1.0f)
	{
		anim_timer=anim_timer-1.0f;
	}

	if(pulse_dirc)
	{
		pulse+=timer*2.0f;
		if(pulse > 1.0f)
		{
			pulse=1.0f;
			pulse_dirc=0;
		}
	}
	else
	{
		pulse-=timer*2.0f;
		if(pulse < 0.0f)
		{
			pulse=0.0f;
			pulse_dirc=1;
		}

	}


	ticktockcount+=timer*2.0f;
	if(ticktockcount > 0.75f)
	{
		ticktock=!ticktock;
		ticktockcount=0.0f;
	}


	slowticktockcount+=timer;
	if(slowticktockcount > 2.0f)
	{
		slowticktock=!slowticktock;
		slowticktockcount=0.0f;
	}
}

}; // namespace UI
