#include "gravity/sharedphysics/Integrator.h"

namespace SharedPhysics
{


template <class ODE, class DerivedODEState>
NOALIAS inline void C_Integrator::EulerIntegrate(ODE& ode, const Maths::C_TimeS PCREF time) const
{
	//The ode represents the object which is being solved (e.g. rigid body, particle, spring etc.)
	//DerivedODEState is the ODE state object which will be used as an intermediary

	DerivedODEState state;
	ode.Step(time, state);
	for(U32 a = 0; a < state.m_NumEquations; ++a)
	{
		state.m_pEquations[a] *= time.m_Scalar;
	}
	ode.Apply(state);
}

template <class ODE, class DerivedODEState>
NOALIAS inline void C_Integrator::RK4Integrate(ODE& ode, const Maths::C_TimeS PCREF time) const
{
	//The ode represents the object which is being solved (e.g. rigid body, particle, spring etc.)
	//DerivedODEState is the ODE state object which will be used as an intermediary
	Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
	Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
	Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());
	Maths::HF32 sixth(Maths::Constants<Maths::HF32>::Sixth());
	DerivedODEState state, state2, state3, state4;
	ode.Step(time, state);
	ode.Step(time * half, state, state2, half);
	ode.Step(time * half, state2, state3, half);
	ode.Step(time, state3, state4, one);
	const Maths::HF32 timeBySix(time.m_Scalar * sixth);

	for(U32 a =0; a < state.m_NumEquations; ++a)
	{
		const Maths::HVec4 newState = state.m_pEquations[a]  + ((state2.m_pEquations[a] + state3.m_pEquations[a]) * two) + state4.m_pEquations[a];
		state.m_pEquations[a] = newState * timeBySix;
	}

	//All derived
	ode.Apply(state);
}

};
