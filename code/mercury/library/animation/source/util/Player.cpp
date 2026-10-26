#include "stdafx.h"
#include <animation/util/Player.h>

#include <system/debug/I_DebugRender.h>

namespace Animation
{

//
// C_CycleState
//

C_CycleState::C_CycleState():
	m_salt(0),
	m_flags(0)
{
}

void C_CycleState::Create(U32 cycleId, F32 cycleDuration, U32 salt, const C_PlaybackParams& params)
{
	m_cycleId = cycleId;
	m_cycleDuration = cycleDuration;
	m_salt = salt;
	
	m_flags = E_Flag_Enabled;
	
	m_layer = params.layer;
	m_maxLoopCount = params.loopCount;
	
	if (params.autoRestart) m_flags |= E_Flag_AutoRestart;
		
	m_time = 0.f;
	m_weight = 1.f;
	m_blendWeight = 1.f;
	m_blendMode = params.blendMode;
	m_loopMode = params.loopMode;
	m_speed = params.speed;
	
	m_fadeDuration = 0.f;
	m_fadeWeight = 1.f;
	
	if (params.fadeInTime > 0.f)
	{
		Fade(params.fadeInTime, 0.f, 1.f);
	}
	else
	{
		m_fadeWeight = 1.f;
	}
}

U32 C_CycleState::GetCycleId() const
{
	return m_cycleId;
}

void C_CycleState::SetCycleDuration(F32 duration)
{
	m_cycleDuration = duration;
}

E_BlendMode	C_CycleState::GetBlendMode() const
{
	return m_blendMode;
}

void C_CycleState::SetBlendMode(E_BlendMode blendMode)
{
	m_blendMode = blendMode;
}

E_LoopMode C_CycleState::GetLoopMode() const
{
	return m_loopMode;
}

void C_CycleState::SetLoopMode(E_LoopMode loopMode)
{
	m_loopMode = loopMode;
}

F32 C_CycleState::GetSpeed() const
{
	return m_speed;
}

void C_CycleState::SetSpeed(F32 speed)
{
	m_speed = speed;
}

F32 C_CycleState::GetTime() const
{
	return m_time;
}

void C_CycleState::SetTime(F32 time)
{
	m_time = time;
}

F32 C_CycleState::GetCycleTime() const
{
	return m_cycleTime;
}

F32 C_CycleState::GetBlendWeight() const
{
	return m_blendWeight;
}

F32 C_CycleState::GetWeight() const
{
	return m_weight;
}

void C_CycleState::SetWeight(F32 weight)
{
	m_weight = weight;
}

bool C_CycleState::IsFading() const
{
	return (m_flags & E_Flag_Fading) != 0;
}

void C_CycleState::Fade(F32 duration, F32 startWeight, F32 endWeight)
{
	if (duration > 0.f)
	{
		m_flags |= E_Flag_Fading;
		m_fadeWeight = startWeight;
		m_fadeStartWeight = startWeight;
		m_fadeEndWeight = endWeight;
		m_fadeDuration = duration;
		m_fadeTime = 0.f;
	}
	else
	{
		m_flags &= ~E_Flag_Fading;
		m_fadeWeight = endWeight;
	}
}

void C_CycleState::Disable()
{
	m_flags &= ~E_Flag_Enabled;
	m_salt = 0;
}

void C_CycleState::Enable()
{
	m_flags |= E_Flag_Enabled;
}

bool C_CycleState::IsEnabled() const
{
	return (m_flags & E_Flag_Enabled) != 0;
}

void C_CycleState::Update(F32 deltaTime, F32 layerWeight)
{
	F32 prevTime = m_time;
	F32 timeStep = deltaTime * m_speed;
	m_time += timeStep;

	bool isFinished = false;

	if (m_loopMode == E_LoopMode_Once)
	{
		m_time = Maths::Clamped(m_time, 0.f, m_cycleDuration);

		if (m_time + timeStep >= m_cycleDuration)
		{
			isFinished = true;
		}
	}

	if (isFinished)
	{
		m_flags |= E_Flag_Finished;
	}
	else
	{
		m_flags &= ~E_Flag_Finished;
	}

	if (m_time == prevTime)
	{
		m_flags |= E_Flag_Paused;
	}
	else
	{
		m_flags &= ~E_Flag_Paused;
	}
		
	if (m_maxLoopCount > 0 && !IsFading())
	{
		F32 duration = m_cycleDuration;
		S32 futureLoopCount = static_cast<S32>((Maths::Abs(m_time) + m_fadeDuration) / duration);
		
		if (futureLoopCount >= S32(m_maxLoopCount))
		{
			Fade(m_fadeDuration, 1.f, 0.f);
		}
	}
		
	if (IsFading())
	{
		m_fadeTime += deltaTime;
		
		F32 t = Maths::Clamped(m_fadeTime / m_fadeDuration, 0.f, 1.f);
		F32 smoothT = Maths::SmoothStep(t);
		F32 fadeWeight = m_fadeStartWeight + (m_fadeEndWeight - m_fadeStartWeight) * smoothT;
		m_fadeWeight = fadeWeight;
		
		if (m_fadeTime > m_fadeDuration)
		{
			m_flags &= ~E_Flag_Fading;
			m_fadeWeight = m_fadeEndWeight;
		}
	}
	
	F32 localBlendWeight = m_weight * m_fadeWeight;
	
	if (localBlendWeight <= 0.f)
	{
		Disable();
	}
	
	if (layerWeight <= 0.f)
	{
		m_flags |= E_Flag_Suspended;
	}
	else
	{
		if ((m_flags & (E_Flag_Suspended | E_Flag_AutoRestart)) == (E_Flag_Suspended | E_Flag_AutoRestart))
		{
			m_time = 0.f;
			m_cycleTime = 0.f;
		}
		m_flags &= ~E_Flag_Suspended;
	}
	
	m_blendWeight = localBlendWeight * layerWeight;
}

//
// C_PlaybackParams
//
C_PlaybackParams::C_PlaybackParams():
	fadeInTime(0.f),
	speed(1.f),
	loopCount(0),
	layer(0),
	//startTime(0.f),
	//endTime(0.f),
	blendMode(Animation::E_BlendMode_Accumulate),
	loopMode(Animation::E_LoopMode_Continuous),
	autoRestart(true)
{
}

//
// C_Player
//

C_Player::C_Player():
	m_numActiveCycles(0),
	m_salt(0),
	m_isPaused(false),
	m_isFinished(false)
{
}

void C_Player::Clear()
{
	// Invalidate all currently active cycles.
	for (U32 i = 0, count = GetMaxCycleStates(); i < count; ++i)
	{
		m_cycleStates[i].Disable();
	}
	m_numActiveCycles = 0;
	m_isPaused = false;
	m_isFinished = false;
}
		
C_CycleStateHandle C_Player::Play(U32 cycleId, F32 cycleDuration, const C_PlaybackParams* params)
{
	C_PlaybackParams playbackParams;
	
	if (params)
	{
		playbackParams = *params;
	}
	
	const U32 maxCycleStates = GetMaxCycleStates();
	
	// Search for a non-enabled cycle or failing that, the one with the lowest current blend weight.
	U32 newStateIndex = 0;
	for (U32 i = 0; i < maxCycleStates; ++i)
	{
		if (!m_cycleStates[i].IsEnabled())
		{
			newStateIndex = i;
			break;
		}
		if (m_cycleStates[i].GetBlendWeight() < m_cycleStates[newStateIndex].GetBlendWeight())
		{
			newStateIndex = i;
		}
	}
		
	// Fade out currently active animations in the same layer.
	U32 numActive = 0;
	for (U32 i = 0; i < maxCycleStates; ++i)
	{
		C_CycleState& state = m_cycleStates[i];
		
		if (state.IsEnabled())
		{
			++numActive;
			if (state.GetLayer() == playbackParams.layer)
			{
				state.Fade(playbackParams.fadeInTime, 1.f, 0.f);
			}
		}
	}
	
	// Create the new animation state.
	++m_salt;
	C_CycleState& newState = m_cycleStates[newStateIndex];
	newState.Create(cycleId, cycleDuration, m_salt, playbackParams);
	
	return C_CycleStateHandle(&newState);
}
		
void C_Player::Stop(C_CycleStateHandle& cycle)
{
	if (cycle.IsValid())
	{
		cycle->Disable();
	}
}

void C_Player::Stop()
{
	for (U32 i = 0, count = GetMaxCycleStates(); i < count; ++i)
	{
		m_cycleStates[i].Disable();
	}
}
		
void C_Player::Update(F32 deltaTime)
{
	U32 numActiveCycles = 0;
	U32 maxCycleStates = GetMaxCycleStates();
	for (U32 i = 0; i < maxCycleStates; ++i)
	{
		C_CycleState& state = m_cycleStates[i];
		
		if (state.IsEnabled())
		{
			// Insert into active cycle list, sorted by descreasing layer number.
			U32 insertIndex = 0;
			for (U32 j = 0; j < numActiveCycles; ++j)
			{
				U32 activeStateIndex = m_activeCycleStates[j];
				C_CycleState& activeState = m_cycleStates[activeStateIndex];
				
				if (activeState.GetLayer() < state.GetLayer())
				{
					// state goes here.
					insertIndex = j;
					break;
				}
			}
			
			for (U32 j = numActiveCycles; j > insertIndex; --j)
			{
				m_activeCycleStates[j] = m_activeCycleStates[j-1];
			}
			m_activeCycleStates[insertIndex] = (U8)i;
			
			++numActiveCycles;
		}
	}
	m_numActiveCycles = numActiveCycles;	
	
	// Update all active cycles, calculating layer weights.
	F32 remainingLayerWeight = 1.f;
	F32 totalLayerWeight = 0.f;
	
	U32 currentLayer = (U32)-1;
	
	bool isPaused = true;
	bool isFinished = true;
	
	for (U32 i = 0; i < numActiveCycles; ++i)
	{
		U32 stateIndex = m_activeCycleStates[i];
		
		C_CycleState& state = m_cycleStates[stateIndex];
		
		BIZ_ASSERT(state.IsEnabled());
		
		if (currentLayer != state.GetLayer())
		{
			// New layer, reduce its available weight by that used on higher layers.
			remainingLayerWeight -= totalLayerWeight;
			totalLayerWeight = 0.f;
			currentLayer = state.GetLayer();
		}
		
		state.Update(deltaTime, remainingLayerWeight);
		totalLayerWeight += state.GetBlendWeight();
		
		isPaused = isPaused && state.IsPaused();
		isFinished = isFinished && state.IsFinished();
	}
	m_isPaused = isPaused;
	m_isFinished = isFinished;
}

U32 C_Player::GetNumActiveCycles() const
{
	return m_numActiveCycles;
}

C_CycleStateHandle C_Player::GetActiveCycleState(U32 activeCycleIndex) const
{
	if (activeCycleIndex >= m_numActiveCycles || activeCycleIndex >= GetMaxCycleStates())
	{
		return C_CycleStateHandle();
	}

	return C_CycleStateHandle(&m_cycleStates[m_activeCycleStates[activeCycleIndex]]);
}

S32 C_Player::GetCycleStateId(const C_CycleStateHandle& cycleState) const
{
	if (!cycleState.IsValid())
	{
		return -1;
	}
	
	S32 index = cycleState.State() - &m_cycleStates[0];
	BIZ_ASSERT(index >= 0 && index < (S32)GetMaxCycleStates());
	return index;
}

C_CycleStateHandle C_Player::GetCycleStateFromId(S32 cycleStateId) const
{
	if (cycleStateId >= 0 && cycleStateId < (S32)GetMaxCycleStates())
	{
		return C_CycleStateHandle(&m_cycleStates[cycleStateId]);
	}
	return C_CycleStateHandle();
}

} // end Animation
