#ifndef GRAVITY_ACTION_H
#define GRAVITY_ACTION_H
namespace Gravity
{

	/*
	 * An action is a user-defined operation which is performed at a given frequency in the physics update loop.
	 * You can define what frequency you want to run it at (in ms). If the time is -1, then it is run at
	 * the same rate as the physics update...
	 */
	class C_Action
	{
	public:

		C_Action(Maths::HF32 PCREF frequency) : m_Frequency(frequency), m_NextRun(Maths::Constants<Maths::HF32>::Zero()), m_ActionIndex(numActions++)
		{
		}

		virtual ~C_Action()
		{
		}

		Maths::C_TimeS Apply(Maths::HF32 PCREF dt)
		{
			//const Maths::HF32 dif(simulationTime - m_LastRun);
			//m_Frequency states how long until we run again!!!
			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			if(m_NextRun <= zero)
			{
				//We have reached the time to apply the action.
				//How long does the action last for?
				const Maths::HF32 actionLength(Select(m_Frequency.LE(zero), dt, Maths::Min(m_Frequency, dt)));
				ApplyAction(actionLength);
				m_NextRun = Select(actionLength.GE(m_Frequency), zero, m_Frequency - actionLength);
				return actionLength;
			}
			//Next run is scheduled, but not reached yet! Move forward to the next time the action is performed
			const Maths::HF32 futureTime(dt.LE(m_NextRun), dt, m_NextRun);
			m_NextRun -= futureTime;
			return futureTime;
		}

		//virtual void ApplyAction(Maths::HF32 dt) = 0; // error C2719: 'dt': formal parameter with __declspec(align('16')) won't be aligned
		virtual void ApplyAction( const Maths::HF32 PCREF dt) = 0;

		Maths::HF32 m_Frequency;
		Maths::HF32 m_NextRun;
		U32 m_ActionIndex;
		static U32 numActions;
	};

	/*class C_ActionWrapper
	{
	public:
		C_Action* m_pAction;

		C_ActionWrapper(C_Action* pAction) : m_pAction(pAction)
		{
		}

		C_ActionWrapper() : m_pAction(NULL)
		{
		}

		NOALIAS __forceinline bool operator < (const C_ActionWrapper& wrapper)
		{
			return (m_pAction->m_NextRun < wrapper.m_pAction->m_NextRun) != 0;
		}

		NOALIAS __forceinline bool operator > (const C_ActionWrapper& wrapper)
		{
			return (m_pAction->m_NextRun > wrapper.m_pAction->m_NextRun) != 0;
		}

		NOALIAS __forceinline bool operator <= (const C_ActionWrapper& wrapper)
		{
			return (m_pAction->m_NextRun <= wrapper.m_pAction->m_NextRun) != 0;
		}

		NOALIAS __forceinline bool operator >= (const C_ActionWrapper& wrapper)
		{
			return (m_pAction->m_NextRun >= wrapper.m_pAction->m_NextRun) != 0;
		}

		NOALIAS __forceinline bool operator == (const C_ActionWrapper& wrapper)
		{
			return (m_pAction->m_NextRun == wrapper.m_pAction->m_NextRun) != 0;
		}

		NOALIAS __forceinline bool operator != (const C_ActionWrapper& wrapper)
		{
			return (m_pAction->m_NextRun != wrapper.m_pAction->m_NextRun) != 0;
		}
	};*/

};

#endif

