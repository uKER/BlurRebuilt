#ifndef ANIMATION_PLAYER_INL
#define ANIMATION_PLAYER_INL

namespace Animation
{

	//
	// C_CycleState
	//

	template <typename TStream>
	inline void C_CycleState::Serialise(TStream& stream) const
	{
		stream << m_flags;
		if (m_flags & E_Flag_Enabled)
		{
			stream << m_salt << m_layer;
			stream << m_cycleId << m_cycleDuration;
			stream << U8(m_blendMode);
			stream << U8(m_loopMode);
			stream << m_weight << m_blendWeight;
			stream << m_time << m_cycleTime;
			stream << m_speed;
			stream << m_maxLoopCount;
			stream << m_fadeTime << m_fadeDuration << m_fadeStartWeight << m_fadeEndWeight << m_fadeWeight;
		}
	}

	template <typename TStream>
	inline void C_CycleState::Deserialise(TStream& stream)
	{
		stream >> m_flags;
		
		if (m_flags & E_Flag_Enabled)
		{
			stream >> m_salt >> m_layer;
			stream >> m_cycleId >> m_cycleDuration;
			
			U8 blendMode, loopMode;
			
			stream >> blendMode >> loopMode;
			
			m_blendMode = static_cast<E_BlendMode>(blendMode);
			m_loopMode = static_cast<E_LoopMode>(loopMode);
			
			stream >> m_weight >> m_blendWeight;
			stream >> m_time >> m_cycleTime;
			stream >> m_speed;
			stream >> m_maxLoopCount;
			stream >> m_fadeTime >> m_fadeDuration >> m_fadeStartWeight >> m_fadeEndWeight >> m_fadeWeight;
		}
	}
	
	//
	// C_Player
	//

#ifndef CFG_GOLD
	template <typename TCycleNameLookup>
	void C_Player::DebugRender(DbUtil::I_DebugRender* debugRender, const Maths::Mat43& worldMatrix, TCycleNameLookup& cycleNameLookup) const
	{
		// Draw cycle name, current weight, time, layer
		
		Btl::String128 debugText;
		Btl::String128 str;
		
		Maths::HVec3 pos = Maths::HVec3(Maths::RowW(worldMatrix));
		
		U32 numActiveCycles = 0;
		U32 maxCycleStates = GetMaxCycleStates();
		for (U32 i = 0; i < maxCycleStates; ++i)
		{
			const C_CycleState& state = m_cycleStates[i];
			
			if (state.IsEnabled())
			{
				str.Printf("%d) %s T:%.2f W:%.2f", state.GetLayer(), cycleNameLookup(state.GetCycleId()), state.GetTime(), state.GetBlendWeight());
				debugText += str;
				if (state.IsPaused())
				{
					debugText += " (Paused)";
				}
				if (state.IsFinished())
				{
					debugText += " (Finished)";
				}
				
				debugText += "\n";
			}
		}
		
		if (debugText.Length() == 0)
		{
			debugText = "<no cycle playing>";
		}
			
		debugRender->Draw3dText(pos, debugText.Chars());
	}
#endif	
	

	template <typename TStream>
	inline void C_Player::Serialise(TStream& stream) const
	{
		for (U32 i = 0, count = GetMaxCycleStates(); i < count; ++i)
		{
			stream << m_cycleStates[i];
		}
		stream << m_numActiveCycles;
		stream << m_salt;
		for (U32 i = 0, count = GetMaxCycleStates(); i < count; ++i)
		{
			stream << m_activeCycleStates[i];
		}
	}

	template <typename TStream>
	inline void C_Player::Deserialise(TStream& stream)
	{
		for (U32 i = 0, count = GetMaxCycleStates(); i < count; ++i)
		{
			stream >> m_cycleStates[i];
		}
		stream >> m_numActiveCycles;
		stream >> m_salt;
		for (U32 i = 0, count = GetMaxCycleStates(); i < count; ++i)
		{
			stream >> m_activeCycleStates[i];
		}
	}
}

#endif



