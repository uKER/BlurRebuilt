#ifndef _PID_CONTROLLER_C_H
#define _PID_CONTROLLER_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <pidcontrollerai/ipidcontrollerai.h>




namespace airacingtech
{
	

	BEGIN_COMPONENT_DEC(CPIDControllerAI, IPIDControllerAI)

	public:

		void QuickRestart();
		// get	
		inline float GetError()
		{
			if (m_NumErrorsRecorded >= 1)
			{
				assert((m_CurrentIndex >= 0) && (m_CurrentIndex < m_NUM_SLOTS));

				return m_Error[m_CurrentIndex];
			}
			else
			{
				return 0.0f;
			}
		}

		inline float GetErrorIntegral()
		{
			return m_CurrentIntegral;
		}

		inline float GetErrorDerivative()
		{
			if (m_NumErrorsRecorded >= 2)  
			{
				assert((m_CurrentIndex	>= 0)	&& (m_CurrentIndex	< m_NUM_SLOTS));
				assert((m_PreviousIndex	>= 0)	&& (m_PreviousIndex	< m_NUM_SLOTS));

				float difference	= m_Error[m_CurrentIndex] - m_Error[m_PreviousIndex];
				float time_interval	= m_Timestep;

				if (time_interval > 0.00001f)
				{
					return (difference / time_interval);
				}
				else
				{
					return 999999.0f;
				}
			}
			else
			{
				return 0.0f;
			}
		}

		inline float GetOutput()
		{
			return	(m_P * GetError() + m_I * GetErrorIntegral()	+  m_D * GetErrorDerivative() );
		}

		inline float GetTimeStep() const
		{
			return m_Timestep;
		}


		// set

		inline void	SetCoefficients(float p, float i, float d) { m_P = p; m_I = i; m_D = d; }

		inline void SetNumSlots(int numSlots) {m_NUM_SLOTS = numSlots;}

		// go
		inline bool	IsFull()
		{
			return (m_NumErrorsRecorded == m_NUM_SLOTS);
		}

		inline void Clear()
		{
			m_CurrentIndex		= -1;
			m_PreviousIndex		= -1;
			m_NumErrorsRecorded	= 0;

			m_CurrentIntegral	= 0.0f;

			// not bothered about efficiency here as it is called only on initialisation.
			for(int i=0;i<m_NUM_SLOTS; i++)
			{
				m_Error[i]		= 0.0f;
			}
		}

		inline void Record(float error, float timestep)
		{
			m_PreviousIndex				= m_CurrentIndex;
			m_CurrentIndex				= (m_CurrentIndex + 1) % m_NUM_SLOTS;

			if (IsFull())
			{
				m_CurrentIntegral		-= m_Error[m_CurrentIndex] * m_Timestep;
			}

			m_Error[m_CurrentIndex]		= error;
			m_Timestep	= timestep;

			m_CurrentIntegral			+= error * timestep;

			m_NumErrorsRecorded			= Maths::Min(m_NumErrorsRecorded + 1, m_NUM_SLOTS);
		}



		inline float CalculateIntegralDirectly()
		{
			float integral = 0.0f;

			for (int i = 0; i < m_NumErrorsRecorded; i++)
			{
				integral += (m_Error[i] * m_Timestep);
			}

			return integral;
		}

		
	private:
		float	m_P;				
		float	m_I;				
		float	m_D;	
		float	m_CurrentIntegral;
		float	m_Timestep;

		int		m_CurrentIndex;					
		int		m_PreviousIndex;				
		int		m_NumErrorsRecorded;
		int		m_NUM_SLOTS;

		Btl::Vector<float>  m_Error;


		void CopyData(PIDControllerInitialiseParams *pData);
		
	END_COMPONENT_DEC
}




#endif //_PID_CONTROLLER_C_H
