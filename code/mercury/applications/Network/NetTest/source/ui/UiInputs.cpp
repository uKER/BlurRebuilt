#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

#include "stdafx.h"
#include "UiInputs.h"

#include <system/Time/Time.h>

IMPLEMENT_SINGLETON(UiInputs);

UiPad::UiPad()
{
	memset(this, 0, sizeof(*this));
}

UiPadComponents::UiPadState()
{
	memset(this, 0, sizeof(*this));
}

void UiPad::pushState(const UiPadState& state)
{
	m_stateStack[(m_stackPtr % MAX_STATES)] = state;
	m_stateStack[(m_stackPtr % MAX_STATES)].m_time = Time::GetPerformanceCounter();
	m_stackPtr++;
}

bool UiPad::buttonDown(E_UiButton button) const
{
	if(m_stackPtr < 1) return false;
	if(button < 0 || button >= E_UiButton_Count) return false;
	
	const UiPadState& state = m_stateStack[((m_stackPtr-1) % MAX_STATES)];
	
	return (state.m_buttons & (1 << button)) != 0;
}

bool UiPad::buttonPressed(E_UiButton button) const
{
	if(m_stackPtr < 2) return false;
	if(button < 0 || button >= E_UiButton_Count) return false;

	const UiPadState& prevState = m_stateStack[((m_stackPtr-2) % MAX_STATES)];
	const UiPadState& currState = m_stateStack[((m_stackPtr-1) % MAX_STATES)];
	
	bool wasPressed = (prevState.m_buttons & (1 << button)) != 0;
	bool nowPressed = (currState.m_buttons & (1 << button)) != 0;
	return (nowPressed && !wasPressed);
}

float UiPad::axis(E_UiAxis axis) const
{
	if(m_stackPtr < 1) return 0.0f;
	if(axis < 0 || axis >= E_UiAxis_Count) return 0.0f;

	const UiPadState& state = m_stateStack[((m_stackPtr-1) % MAX_STATES)];
	return state.m_axes[axis];
}

UiInputs::UiInputs()
{
	m_started = false;
}

UiInputs::~UiInputs()
{
	shutdown();
}


void UiInputs::startup(int hwnd)
{
	if(m_started)
	{
		assert(!"<UiInputs> alread started!");
		return;
	}
	m_started = true;

	for(int i = 0; i < MAX_PADS; i ++)
		m_pads[i] = UiPad();
}

void UiInputs::update()
{
	if(!m_started)
	{
		assert(!"<UiInputs> not started!");
		return;
	}
}

void UiInputs::shutdown()
{
	if(!m_started)
	{
		assert(!"<UiInputs> alread shutdown!");
		return;
	}
	m_started = false;
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

