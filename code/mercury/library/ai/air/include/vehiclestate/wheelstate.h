#ifndef _WHEEL_STATE_H
#define _WHEEL_STATE_H

#ifdef _MSC_VER
#pragma once
#endif





namespace airacingtech
{
	struct Wheels
	{
	private:
		Maths::HVec3	m_pos;		// world pos
		Maths::HVec3	m_localpos;
		Maths::HVec3	m_velocity;
		Maths::HVec3	m_headingVec;


		bool	m_onGround;	

		float	m_radius;
		float	m_angularRotation;
		float   m_longSlip;
		float   m_slipRatio;
		float	m_slipAngle;
		float	m_lateralSlip;
		float	m_slideFactor;
		float	m_heading;
		float	m_wheelspin;
		bool	m_driven;




	public:
		// Set
		inline void SetWheelSpin(float v) {m_wheelspin = v;}
		inline void SetDriven(bool d) {m_driven = d;}
		inline void SetPosition(Maths::HVec3& pos) {m_pos = pos;}
		inline void SetLocalPosition(Maths::HVec3& pos) {m_localpos = pos;}
		inline void SetVelocity(Maths::HVec3& vel) {m_velocity = vel;}
		inline void SetOnGround(bool onGround) {m_onGround = onGround;}
		inline void SetRadius(float radius){m_radius = radius;}
		inline void SetAngularRotation(float rotation){m_angularRotation = rotation;}
		inline void SetLongSlip(float slip){m_longSlip = slip;}
		inline void SetLateralSlip(float slip){m_lateralSlip = slip;}
		inline void SetSlipRatio(float ratio){m_slipRatio = ratio;}
		inline void SetSlipAngle(float angle){m_slipAngle = angle;}
		inline void SetHeading(float heading){	m_heading = heading;}
		inline void SetHeadingVec(Maths::HVec3& heading){	m_headingVec = heading;}
		inline void SetSlideFactorAmax(float slide){m_slideFactor = slide;}

		// Get
		inline float GetWheelSpin() {return m_wheelspin;}
		inline bool IsDriven() {return m_driven;}
		inline Maths::HVec3 GetPosition() const {return m_pos;}
		inline Maths::HVec3 GetLocalPosition() const {return m_localpos;}
		inline Maths::HVec3 GetVelocity() const {return m_velocity;}
		inline bool GetOnGround() const {return m_onGround;}
		inline float GetRadius() const {return m_radius;}
		inline float GetAngularRotation() const {return m_angularRotation;}
		inline float GetLongSlip() const {return m_longSlip;}
		inline float GetLateralSlip() const {return m_lateralSlip;}
		inline float GetSlipRatio() const {return m_slipRatio;}
		inline float GetSlipAngle() const {return m_slipAngle;}
		inline Maths::HVec3 GetHeadingVector() const {return m_headingVec;}
		inline float GetHeadingAngle() const {return m_heading;}
		inline float GetSlideFactorAmax(){return m_slideFactor;}

		

	};

}




#endif //_WHEEL_STATE_H
