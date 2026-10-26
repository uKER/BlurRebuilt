#ifndef INTEGRATOR_H
#define INTEGRATOR_H

namespace SharedPhysics
{
	class C_ODEState
	{
		friend class C_Integrator;
		//Num equations in terms of the number of HVec4's that are in the equations...
		U32 m_NumEquations;
		Maths::HVec4* m_pEquations;
	protected:
		C_ODEState(U32 _numEquations, Maths::HVec4* equations) : m_NumEquations(_numEquations), m_pEquations(equations)
		{
		}
	};

	class C_Integrator
	{
	public:
		template <class ODE, class DerivedODEState>
		NOALIAS inline void EulerIntegrate(ODE& ode, const Maths::C_TimeS PCREF time) const;

		template <class ODE, class DerivedODEState>
		NOALIAS inline void RK4Integrate(ODE& ode,  const Maths::C_TimeS PCREF time) const;
	};
};

#include "integrator.inl"

#endif
