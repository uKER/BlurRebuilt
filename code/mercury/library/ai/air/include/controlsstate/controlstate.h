#ifndef _CONTROL_STATE_H
#define _CONTROL_STATE_H

#ifdef _MSC_VER
#pragma once
#endif



namespace airacingtech
{
	class ControlState 
	{
	public:
		ControlState():m_gear(0), m_gearUp(false), m_gearDown(false), m_accel(0.0f),m_brake(0.0f),
						m_steering(0.0f),m_handbrake(false), m_reverse(false), m_lean(0.0f), 
						m_accelControllerInput(0.0f), m_brakeControllerInput(0.0f), m_handbrakeControllerInput(false){};

		

		inline void SetBrake(float v){m_brake = v;}
		inline void SetAccel(float v){m_accel = v;}
		inline void SetSteering(float v){m_steering = v;}
		inline void SetHandbrake(bool v){m_handbrake = v;}
		inline void SetReverse(bool v){m_reverse = v;}
		inline void SetGear(int g){m_gear = g;}
		inline void SetGearUp(bool value){m_gearUp = value;}
		inline void SetGearDown(bool value){m_gearDown = value;}
		inline void SetLean(float value){m_lean = value;}
		inline void SetPitch(float value){m_pitch = value;}

		inline float GetSteering() const {return m_steering;}
		inline float GetAccel() const {return m_accel;}
		inline float GetBrake() const {return m_brake;}
		inline bool  GetHandBrake() const {return m_handbrake;}
		inline bool GetReverse() const {return m_reverse;}
		inline int GetGear() const {return m_gear;}
		inline float GetLean() const {return m_lean;}
		inline float GetPitch() const {return m_pitch;}

		inline void ClampAllValues()
		{
			m_steering = Maths::Clamped(m_steering, -1.0f, 1.0f);
			m_pitch = Maths::Clamped(m_pitch, -1.0f, 1.0f);
			
			m_brake = Maths::Clamped(m_brake, 0.0f, 1.0f);	
			m_accel = Maths::Clamped(m_accel, 0.0f, 1.0f);
		}

		inline void Reset()
		{
			m_accel = 0.0f;
			m_brake = 0.0f;
			m_steering = 0.0f;
			m_handbrake = false;
			m_reverse = false;
			m_gearUp = false;
			m_gearDown = false;
			m_lean = 0.0f;
			m_pitch = 0.0f;
			
		}


		inline float GetAccelControllerInput() const {return m_accelControllerInput;}
		inline float GetBrakeControllerInput() const {return m_brakeControllerInput;}
		inline float GetHandbrakeControllerInput() const {return m_handbrakeControllerInput;}

		inline void SetBrakeControllerInput(float v){m_brakeControllerInput = v;}
		inline void SetAccelControllerInput(float v){m_accelControllerInput = v;}
		inline void SetHandbrakeControllerInput(bool v){m_handbrakeControllerInput = v;}

	private:
		float	m_accel;
		float	m_brake;
		float	m_steering;
		float	m_lean;
		float	m_pitch;
		int		m_gear;
		bool	m_handbrake;
		bool	m_reverse;
		bool	m_gearUp;
		bool	m_gearDown;


		// Hack....actual controller input for the player.....
		float	m_accelControllerInput;
		float	m_brakeControllerInput;
		bool	m_handbrakeControllerInput;
	};
}



#endif // _CONTROL_STATE_H
