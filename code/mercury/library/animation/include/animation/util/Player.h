#ifndef ANIMATION_PLAYER_H
#define ANIMATION_PLAYER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <animation/common/BlendModes.h>
#include <btl/containers/vector.h>

namespace DbUtil
{
	class I_DebugRender;
}

namespace Animation
{

	class C_CycleStateHandle;
	class C_PlaybackParams;
	
	class C_CycleState
	{
	public:
		friend class C_CycleStateHandle;
		friend class C_Player;
		
		C_CycleState();
		
		//
		// Public interface.
		//
		
		void Create(U32 cycleId, F32 cycleDuration, U32 salt, const C_PlaybackParams& params);
		
		/// Returns the cycle ID.
		U32 GetCycleId() const;
		
		/// Sets the duration of the currently playing cycle.
		void SetCycleDuration(F32 duration);
		
		/// Returns the current blend mode.
		E_BlendMode	GetBlendMode() const;
		
		/// Changes the blend mode.
		void SetBlendMode(E_BlendMode blendMode);
		
		/// Returns the current loop mode.
		E_LoopMode GetLoopMode() const;
		
		/// Sets the current loop mode.
		void SetLoopMode(E_LoopMode loopMode);
		
		/// Returns the current playback speed.
		F32 GetSpeed() const;
		
		/// Sets the current playback speed.
		void SetSpeed(F32 speed);
		
		/// Returns the current elapsed playback time.
		F32 GetTime() const;
		
		/// Sets the current elapsed playback time.
		void SetTime(F32 time);
		
		/// Returns the current cycle time.
		F32 GetCycleTime() const;
		
		/// Returns the current weight to be used to blend this active cycle into a pose. Includes fading.
		F32 GetBlendWeight() const;
		
		/// Returns the cycle's weight, unaffected by fading.
		F32 GetWeight() const;
		
		/// Sets the weight for this cycle.
		void SetWeight(F32 weight);
		
		
		
		/// Sets the current evaluation time for the cycle. (modulated by duration?)
		
		bool IsFading() const;
		
		void Fade(F32 duration, F32 startWeight, F32 endWeight);
		
		void Disable();
		
		void Enable();
		
		bool IsEnabled() const;
		
		U32 GetLayer() const		{ return m_layer; }
		
		void Update(F32 deltaTime, F32 layerWeight);

		bool IsFinished() const		{ return (m_flags & E_Flag_Finished) != 0; }
		
		bool IsPaused() const		{ return (m_flags & E_Flag_Paused) != 0; }
		
		template <typename TStream>
		void Serialise(TStream& stream) const;

		template <typename TStream>
		void Deserialise(TStream& stream);
		
	private:
		enum E_Flag
		{
			E_Flag_AutoRestart	= 1 << 0,	// If true, cycle will restart after being unsuspended.
			E_Flag_Suspended	= 1 << 1,	// Zero weight, but still ready to become active when weight goes above zero.
			E_Flag_Enabled		= 1 << 2,
			E_Flag_Fading		= 1 << 3,
			E_Flag_Paused		= 1 << 4,	// If true, cycle is no longer updating its time e.g. if it is non-looping.
			E_Flag_Finished		= 1 << 5
		};
	
		U32				m_salt;
		U32				m_layer;
		U32				m_cycleId;
		F32				m_cycleDuration;
		E_BlendMode		m_blendMode;
		E_LoopMode		m_loopMode;
		F32				m_weight;
		F32				m_blendWeight;		// Calculated.
		
		F32				m_time;				// Time elapsed whilst playing.
		F32				m_cycleTime;		// Time within cycle.
		F32				m_speed;			// Playback speed.
		
		U32				m_maxLoopCount;		// Maximum loop count, at which point cycle will blend out again.
		// start time, end time?
		
		F32				m_fadeTime;			// Time elapsed since fade started.
		F32				m_fadeDuration;		// Total duration of fade
		F32				m_fadeStartWeight;	// Starting fade weight.
		F32				m_fadeEndWeight;	// Ending fade weight.
		F32				m_fadeWeight;		// Current fade weight.
		
		U8				m_flags;
	};

	class C_PlaybackParams
	{
	public:
		C_PlaybackParams();
		
		F32			fadeInTime;
		F32			speed;
		U32			loopCount;			// 0 means play continuously
		U32			layer;
		//F32			startTime;			// Used for setting duration, but this ought to be taken from cycle by default.
		//F32			endTime;
		E_BlendMode	blendMode;
		E_LoopMode	loopMode;
		bool		autoRestart;		// If true, then animation will restart after being temporarily overriden by another layer.
	};
	
	
		
	class C_Player
	{
	public:
		C_Player();
		
		/// Removes all active cycles.
		void Clear();
			
		// Ignore if currently playing cycle is played again?
		C_CycleStateHandle Play(U32 cycleId, F32 cycleDuration, const C_PlaybackParams* params = NULL);
		
		//C_CycleStateHandle PlayQueued(const Animation::C_Cycle* cycle, const C_PlaybackParams* params = NULL);
		
		/// Stops a specific cycle. The cycle's handle will be invalid when this function returns.
		void Stop(C_CycleStateHandle& cycle);
		
		/// Stops all currently playing cycles, invalidating their handles.
		void Stop();
		
		/// Updates the weights and times of all active cycles.
		void Update(F32 deltaTime);

		// Query active cycles.
		U32 GetNumActiveCycles() const;
		
		C_CycleStateHandle GetActiveCycleState(U32 activeCycleIndex) const;
		
		S32 GetCycleStateId(const C_CycleStateHandle& cycleState) const;
		
		C_CycleStateHandle GetCycleStateFromId(S32 cycleStateId) const;
		
		/// Returns true if all currently active cycles are paused, i.e. their time hasn't changed since the last update.
		/// Will also return true if there are no currently active cycles.
		bool IsPaused() const			{ return m_isPaused; }

		/// Returns true if all currently active cycles have reached the end of their animations.
		bool IsFinished() const			{ return m_isFinished; }
		
		template <typename TStream>
		void Serialise(TStream& stream) const;

		template <typename TStream>
		void Deserialise(TStream& stream);
		
	#ifndef CFG_GOLD
		template <typename TCycleNameLookup>
		void DebugRender(DbUtil::I_DebugRender* debugRender, const Maths::Mat43& worldMatrix, TCycleNameLookup& cycleNameLookup) const;
	#endif
	
	private:
		static const U32 kMaxCycleStates = 3;	// could have lots more
		typedef Btl::Vector<C_CycleState, Btl::VectorParameters_Array<kMaxCycleStates> > CycleStateVector;
		//typedef Btl::Vector<U8, Btl::VectorParameters_Static<kMaxCycleStates> > ActiveCycleStateIndexVector;
		
		U32 GetMaxCycleStates() const		{ return kMaxCycleStates; }
		
		CycleStateVector	m_cycleStates;
		U32					m_numActiveCycles;
		U32					m_salt;
		U8					m_activeCycleStates[kMaxCycleStates];
		bool				m_isPaused;
		bool				m_isFinished;
	};
	
	class C_CycleStateHandle
	{
	public:
		C_CycleStateHandle();
		
		C_CycleStateHandle(C_CycleState* state);
		
		C_CycleStateHandle(const C_CycleState* state);
	
		void operator = (C_CycleState* newState);
		
		void Clear();
		
		C_CycleState* State();
		const C_CycleState* State() const;

		operator C_CycleState* ()					{ return State(); }
		operator const C_CycleState* ()			{ return State(); }
		C_CycleState* operator->()					{ return State(); }
		const C_CycleState* operator->() const	{ return State(); }
		
		bool IsValid() const						{ return State() != NULL; }

	private:
		C_CycleState*	m_state;
		U32				m_salt;			// Used to check for out of date state.
	};
	
	inline C_CycleStateHandle::C_CycleStateHandle():
		m_state(NULL),
		m_salt(0)
	{
	}
	
	inline C_CycleStateHandle::C_CycleStateHandle(C_CycleState* state):
		m_state(state),
		m_salt(0)
	{
		if (state != NULL)
		{
			m_salt = state->m_salt;
		}
	}
	
	inline C_CycleStateHandle::C_CycleStateHandle(const C_CycleState* state):
		m_state(const_cast<C_CycleState*>(state)),	// tut, tut...
		m_salt(0)
	{
		if (state != NULL)
		{
			m_salt = state->m_salt;
		}
	}	
	
	inline void C_CycleStateHandle::Clear()
	{
		m_state = NULL;
		m_salt = 0;
	}
	
	inline C_CycleState* C_CycleStateHandle::State()
	{
		if (m_state == NULL) return NULL;
		if (m_state->m_salt != m_salt) return NULL;
		return m_state;
	}
	
	inline const C_CycleState* C_CycleStateHandle::State() const
	{
		if (m_state == NULL) return NULL;
		if (m_state->m_salt != m_salt) return NULL;
		return m_state;
	}
	
	inline void C_CycleStateHandle::operator = (C_CycleState* newState)
	{
		m_state = newState;
		if (newState != NULL)
		{
			m_salt = newState->m_salt;
		}
		else
		{
			m_salt = 0;
		}
	}
	

	
	
	
}

#endif
