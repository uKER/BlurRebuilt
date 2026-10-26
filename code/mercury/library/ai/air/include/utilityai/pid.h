#ifndef _PID_H
#define _PID_H

#ifdef _MSC_VER
#pragma once
#endif

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif



namespace airacingtech
{
	class PID
	{
	public:
		PID() : m_p(0.0f), m_i(0.0f),m_d(0.0f),m_Integral(0.0f), m_timestep(0.0f), m_currentValue(0.0f),m_lastValue(0.0f), m_Output(0.0f), m_DesiredValue(0.0f){};
		
		PID(float p, float i, float d, float timestep) :m_p(p), 
														m_i(i),
														m_d(d),
														m_Integral(0.0f),
														m_timestep(timestep),
														m_currentValue(0.0f),
														m_lastValue(0.0f),
														m_Output(0.0f),
														m_DesiredValue(0.0f){};
		~PID(){};

		

	private:
		inline float Calculate()
		{
			float diff = m_currentValue - m_DesiredValue;
			m_Integral += ((diff * m_i) * m_timestep);
			m_Output = diff * m_p + ((m_currentValue - m_lastValue)/m_timestep)* m_d + m_Integral;
			return m_Output;
		}

		inline float CalculatePDonly()
		{
			float diff = m_currentValue - m_DesiredValue;
			m_Integral = ((diff * m_i) * m_timestep);
			m_Output = diff * m_p + ((m_currentValue - m_lastValue)/m_timestep)* m_d + m_Integral;
			return m_Output;
		}

		float CalculateClamped()
		{
			float diff = m_currentValue - m_DesiredValue;
			m_Integral += ((diff * m_i) * m_timestep);
			m_Integral = Maths::Clamped(m_Integral, -m_clampedValue , m_clampedValue);
			m_Output = diff * m_p + ((m_currentValue - m_lastValue)/m_timestep)* m_d + m_Integral;

			return m_Output;
		}

	public:
		inline float Update(float p, float i, float d, float timestep, float v, float dv, float clampvalue)
		{
			m_lastValue = m_currentValue;
			m_currentValue = v;
			m_p = p;
			m_i = i;
			m_d = d;
			m_timestep = timestep;
			m_DesiredValue = dv;
			m_clampedValue = clampvalue;

			return CalculateClamped();
		}

		inline float Update(float p, float i, float d, float timestep, float v, float dv)
		{
			m_lastValue = m_currentValue;
			m_currentValue = v;
			m_p = p;
			m_i = i;
			m_d = d;
			m_timestep = timestep;
			m_DesiredValue = dv;
			m_clampedValue = 0.0f;

			return Calculate();
		}

		inline float Update(float v, float dv)
		{
			m_lastValue = m_currentValue;
			m_currentValue = v;
			m_DesiredValue = dv;

			return Calculate();
		}

	//private:
		float m_p;
		float m_i;
		float m_d;
		float m_Integral;
		float m_timestep;
		float m_lastValue;
		float m_currentValue;
		float m_Output;
		float m_DesiredValue;
		float m_clampedValue;
	};
}


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif


#endif //_PID_H 
