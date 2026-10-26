#ifndef _VEHICLE_STATE_H
#define _VEHICLE_STATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <vehiclestate/wheelstate.h>
#include <utilityai/utilityai.h>

namespace airacingtech
{

	struct AmaxVehicleCarData
	{
		float	m_ConstantDownForce;
		float	m_DownForce;
		float	m_AirFriction;
		float	m_GripScale;
		float	m_FrontGripScale;
		float	m_RearGripScale;
		float	m_FrontTransverseGripScale;
		float	m_RearTransverseGripScale;
		float	m_TransverseGripScale;
		float	m_loadDependence;
		float	m_DownForceRatioZ;
		float	m_BackwardGripScale;
		float	m_tyreSlip;
		float	m_DownForceFactor;
		float	m_DownForceFriction;
	};

	struct AmaxVehicleBikeData
	{
		float	m_ConstantDownForce;
		float	m_DownForce;
		float	m_AirFriction;
		float	m_GripScale;
		float	m_TransverseGripScale;
		float	m_WheelBase;
		float	m_CofGOffsetZ;
		float	m_cofGOffsetY;
		float	m_massDownforce;

		float m_BrakingGripR;
		float m_AccelGripR;
		float m_LateralGripR;
		float m_BrakingGripF;
		float m_AccelGripF;
		float m_LateralGripF;
		float m_tyreSlip;
	
	};

	struct CollisionContactData
	{
		Maths::HVec3	m_ContactPoint;
		Maths::HVec3	m_ContactNormal;
		float			m_Dist;
	};

#define MAX_GEARS	10

	class  VehicleState 
	{
	public:
		VehicleState() : m_constantssent(false), m_TrackSpeed(0.0f) {};
		~VehicleState(){};

		

	public:
		enum WEATHER_PATCH_TYPE
		{
			NONE = 0,
			ICE,
			WATER
		};

		

	private:
		Maths::HVec3	m_pos;
		Maths::HVec3	m_velocity;
		Maths::HVec3	m_xAxis;
		Maths::HVec3	m_yAxis;
		Maths::HVec3	m_zAxis;
		Maths::HVec3	m_angularVelocity;
		Maths::HVec3	m_inertia;
		Maths::HVec3	m_cmOffset;
		Maths::HVec3	m_extents;

		Maths::HVec3	m_posLast;
		Maths::HVec3	m_velocityLast;
		Maths::HVec3	m_angularVelocityLast;
		Maths::HVec3	m_vehicleSize;

		CollisionContactData	m_BarrierContactData;
		AmaxVehicleBikeData	m_AmaxBikeData;
		AmaxVehicleCarData	m_AmaxCarData;
		

		float	m_speed;	// metres per second..
		float	m_mass;
		float	m_pitch;
		float	m_roll;
		float	m_yaw;
		float	m_radius;
		float	m_revs;
		float	m_accel;
		float	m_brake;
		float	m_steering;

		int		m_gear;
		int		m_vehicleTypeID; // rather than have an enum, could use a hash number instead
		int		m_noWheels;
		bool	m_onGround;
		bool	m_isPlayer;
		int		m_nGears;
		float	m_gearRatios[MAX_GEARS]; // 10 default value.....

		Wheels		m_wheels[4];

		std::string	m_name;

		float	m_rollBikes;
		float	m_maxrollBikes;
		float	m_acceleration;

		float	m_tyreDM;
		float	m_maxRevs;
		float	m_revLimit;
		float	m_topSpeed;
		float	m_peakTorque;
		float	m_diffRatio;
		float	m_drag;
		float	m_maxBrakingFriction;
		float	m_maxLateralFriction;

		float	m_surfaceFriction; // 1.0f is dry <1.0 is wet.
		bool	m_constantssent;
		bool	m_active;

		float	m_speedMPH;

		float	m_surfaceFrictionPuddle;
		float   m_surfaceFrictionIce;

		WEATHER_PATCH_TYPE m_weatherPatchType;

		float m_TrackSpeed;
		float m_powerWeightRatio;
		bool  m_driverPresent;

	public:

		inline void SetDriverPresent(bool v) {m_driverPresent = v;}
		inline bool GetDriverPresent() {return m_driverPresent;}

		inline void SetPowerWeightRatio(float v)
		{
			m_powerWeightRatio = v;
		}

		inline float GetPowerWeightRatio(){return m_powerWeightRatio;}

		inline void SetTrackSpeed(float speed)
		{
			m_TrackSpeed = speed;
		}

		inline float GetTrackSpeed()
		{
			return m_TrackSpeed;
		}


		inline void SetWeatherPatchType(WEATHER_PATCH_TYPE type)
		{
			m_weatherPatchType = type;
		}

		inline WEATHER_PATCH_TYPE GetWeatherPatchType()
		{
			return m_weatherPatchType;
		}

		inline void SetSurfaceFrictionPuddle(float v) {m_surfaceFrictionPuddle = v;}

		inline void SetSurfaceFrictionIce(float v) {m_surfaceFrictionIce = v;}

		inline float GetSurfaceFrictionPuddle() {return m_surfaceFrictionPuddle;}

		inline float GetSurfaceFrictionIce() {return m_surfaceFrictionIce;}

		inline float GetSurfaceFrictionPatch()
		{
			float friction = 1.0f;

			switch(m_weatherPatchType)
			{
				case NONE:
					friction = 1.0f;
					break;
				
				case ICE:
					friction = GetSurfaceFrictionIce();
					break;
				
				case WATER:
					friction = GetSurfaceFrictionPuddle();
					break;

				default:
					friction = 1.0f;
					break;
			}

			return friction;
		}
		

		
		inline float CalculateDownforceBike(float v)
		{
			const float MPH = 1609.f / 3600.f;
			
			m_AmaxBikeData.m_DownForce = MPHTOMS(m_AmaxBikeData.m_ConstantDownForce + m_AmaxBikeData.m_massDownforce * Maths::Abs(v) / (100.0f * MPH));
			
			//m_AmaxBikeData.m_DownForce = Maths::Max(m_AmaxBikeData.m_DownForce, 0.9f); // arguments say
			return m_AmaxBikeData.m_DownForce;
		}
	

		inline void CalculateFrictions(float v2)
		{
			if (GetNoWheels() > 2)
			{
				CalculateCarFrictions(v2);
			}
			else
			{
				CalculateBikeFrictions(v2);
			}
		}


		inline void SetDownForceFactor(float factor)
		{
			m_AmaxCarData.m_DownForceFactor = factor;
		}

		inline void CalculateCarFrictions(float v2)
		{
			float surfaceFriction = GetSurfaceFriction();
			const float speed2 = v2 * v2;
			m_AmaxCarData.m_DownForceFactor = m_AmaxCarData.m_ConstantDownForce + m_AmaxCarData.m_DownForceFriction * Maths::Abs(v2) / (100.0f * (1609.0f / 3600.0f));
			float bg = CalculateBrakingGripCar(speed2) * surfaceFriction;
			float lg = CalculateLateralGripCar(speed2) * surfaceFriction;
			SetBrakingFriction(bg);
			SetLateralFriction(lg);
		}

		inline void CalculateBikeFrictions(float v2)
		{
			float surfaceFriction = GetSurfaceFriction();

			
			const float speed2 = v2 * v2;
			float bg = CalculateBrakingGripBike(speed2) * surfaceFriction;
			float lg = CalculateLateralGripBike(speed2) * surfaceFriction;
			SetBrakingFriction(bg);
			SetLateralFriction(lg);


			// Darren...Hard codong the bike lateral friction Max to 2.6f // maybe too high
			//SetLateralFriction(Maths::Min(lg, 2.9f));
		}

		inline AmaxVehicleCarData &GetAmaxCarData()
		{
			return m_AmaxCarData;
		}

		inline AmaxVehicleBikeData &GetAmaxBikeData()
		{
			return m_AmaxBikeData;
		}

		inline float CalculateBrakingGripCar(float v2)
		{
#if 0
			// PGR
			float bg = m_AmaxCarData.m_GripScale * m_AmaxCarData.m_BackwardGripScale * m_AmaxCarData.m_FrontGripScale *
						(1.0f + m_AmaxCarData.m_ConstantDownForce + v2 * m_AmaxCarData.m_AirFriction * m_AmaxCarData.m_DownForce / 14000.0f);
#else
			// Amax
			float a = 1.0f + m_AmaxCarData.m_DownForceFactor;
			float bg =  a * m_AmaxCarData.m_BackwardGripScale * m_AmaxCarData.m_FrontGripScale;
#endif
			

			return bg;
		}

		inline float CalculateBrakingGripBike(float v2)
		{
			// ignore rear grip
			float BrakingGrip = m_AmaxBikeData.m_BrakingGripF;//Maths::Max(m_AmaxBikeData.m_BrakingGripR, m_AmaxBikeData.m_BrakingGripF);
			
			float brakeMult = (m_AmaxBikeData.m_WheelBase * 0.5f + m_AmaxBikeData.m_CofGOffsetZ) /
							(m_AmaxBikeData.m_cofGOffsetY + m_tyreDM);

			BrakingGrip = Maths::Min(BrakingGrip, brakeMult);
			//float AccelGrip = Maths::Min(m_AmaxBikeData.m_AccelGripR, m_AmaxBikeData.m_AccelGripF);

			//float bg = BrakingGrip * AccelGrip * (1.0f + m_AmaxBikeData.m_ConstantDownForce + v2 * m_AmaxBikeData.m_AirFriction * m_AmaxBikeData.m_DownForce / 14000.0f); 
			
			float bg = BrakingGrip * (1.0f + m_AmaxBikeData.m_DownForce);
			//float bg = BrakingGrip * m_AmaxBikeData.m_DownForce;
			
			
			// Don`t forget to multiply by the frontbrakeforce if we have it...

			return bg;
		}

		inline float CalculateLateralGripCar(float v2)
		{	
#if 0
			// PGR
			float DownForceCorrection = Maths::Min(m_AmaxCarData.m_DownForceRatioZ * 2.0f, 1.0f);				
			float downForce = m_AmaxCarData.m_ConstantDownForce + v2 * m_AmaxCarData.m_AirFriction * m_AmaxCarData.m_DownForce * DownForceCorrection / 14000.0f;
			float frontGripScale = m_AmaxCarData.m_FrontGripScale * m_AmaxCarData.m_FrontTransverseGripScale;
			float rearGripScale = m_AmaxCarData.m_RearGripScale * m_AmaxCarData.m_RearTransverseGripScale;
			float lg = m_AmaxCarData.m_GripScale * m_AmaxCarData.m_TransverseGripScale * (1.0f - m_AmaxCarData.m_loadDependence) *
						Maths::Min(frontGripScale, rearGripScale) * (1.0f + downForce);

#else

			// Amax
			// various ways to accumulate the grip here.....
			float a = 1.0f + m_AmaxCarData.m_DownForceFactor;
			float lg =  a * ((m_AmaxCarData.m_FrontTransverseGripScale + m_AmaxCarData.m_RearTransverseGripScale) * 0.5f);

#endif
			return lg;
		}

		inline float CalculateLateralGripBike(float v2)
		{
			// lateral friction.....
			float downForce = m_AmaxBikeData.m_DownForce;//m_AmaxBikeData.m_ConstantDownForce + v2 * m_AmaxBikeData.m_AirFriction * m_AmaxBikeData.m_DownForce  / 14000.0f;
			float frontGripScale = m_AmaxBikeData.m_AccelGripF * m_AmaxBikeData.m_LateralGripF;
			float rearGripScale = m_AmaxBikeData.m_AccelGripR * m_AmaxBikeData.m_LateralGripR;
			float lgs =  m_AmaxBikeData.m_TransverseGripScale * frontGripScale * (1.0f + downForce);
			return lgs;
		}

		inline void SetActive(bool v)
		{
			m_active = v;
		}

		inline void SetSurfaceFriction(float d)
		{
			m_surfaceFriction = d;
		}
		

		inline void SetBrakingFriction(float d)
		{
			m_maxBrakingFriction = d;
		}

		inline void SetLateralFriction(float d)
		{
			m_maxLateralFriction = d;
		}

		inline void SetDrag(float d)
		{
			m_drag = d;
		}

		inline void SetDiffRatio(float diffratio)
		{
			m_diffRatio = diffratio;
		}

		inline void SetHaveSentConstantsOver(bool constants)
		{
			m_constantssent = constants;
		}

		inline void SetPeakTorque(float peakTorque)
		{
			m_peakTorque = peakTorque;
		}

		inline void SetTopSpeed(float topspeed)
		{
			m_topSpeed = topspeed;
		}

		inline void SetRevLimit(float revLimit)
		{
			m_revLimit = revLimit;
		}

		inline void SetMaxRevs(float mrevs)
		{
			m_maxRevs = mrevs;
		}

		inline void SetTyreDiameter(float dm)
		{
			m_tyreDM = dm;
		}

		inline void SetGearRatios(int nGears, float gearratio)
		{
			m_nGears = nGears;
			m_gearRatios[m_nGears] = gearratio;
			m_nGears+=1;
		}

		inline void SetBarrierContactData(CollisionContactData *pData)
		{
			m_BarrierContactData.m_ContactNormal = pData->m_ContactNormal;
			m_BarrierContactData.m_ContactPoint = pData->m_ContactPoint;
			m_BarrierContactData.m_Dist = pData->m_Dist;
		}

		inline void SetMaxRollBikes(float v){m_maxrollBikes = v;}
		inline void SetRollBikes(float v){m_rollBikes = v;}

		inline void SetName(std::string name){m_name = name;}

		inline void SetPlayer(bool v){m_isPlayer = v;}

		// set

		inline void SetVehicleSize(Maths::HVec3& v)
		{
			m_vehicleSize = v;
		}

		inline void SetPosition(Maths::HVec3& pos) 
		{
			m_posLast = m_pos;
			m_pos = pos;
		}
		
		inline void SetVelocity(Maths::HVec3& vel) 
		{	
			m_velocityLast = m_velocity;
			m_velocity = vel;

			float frwdsSpeed = m_velocity | m_zAxis;
			frwdsSpeed = Maths::Max(frwdsSpeed, 0.1f);
			SetSpeed(frwdsSpeed);
			float mph = MSTOMPH(frwdsSpeed);
			SetSpeedMPH(mph);
		}

		inline void SetXaxis(Maths::HVec3& axis) {m_xAxis = axis;}
		inline void SetYaxis(Maths::HVec3& axis) {m_yAxis = axis;}
		inline void SetZaxis(Maths::HVec3& axis) {m_zAxis = axis;}
		inline void SetAngularVelocity(Maths::HVec3& vel) 
		{
			m_angularVelocityLast = m_angularVelocity;
			m_angularVelocity = vel;
		}
		inline void SetInertia(Maths::HVec3& inertia) {m_inertia = inertia;}
		inline void SetCMOffset(Maths::HVec3& cmOffset) {m_cmOffset = cmOffset;}
		inline void SetExtents(Maths::HVec3& extents) {m_extents = extents;}
		inline void SetSpeed(float speed){m_speed = speed;}
		inline void SetSpeedMPH(float speed){m_speedMPH = speed;}
		inline void SetMass(float mass){m_mass = mass;}
		inline void SetPitch(float pitch){m_pitch = pitch;}
		inline void SetRoll(float roll){m_roll = roll;}
		inline void SetYaw(float yaw){m_yaw = yaw;}
		inline void SetRadius(float radius){m_radius = radius;}
		inline void SetRevs(float revs){m_revs = revs;}
		inline void SetAccel(float accel){m_accel = accel;}
		inline void SetBrake(float brake){m_brake = brake;}
		inline void SetSteering(float steering){m_steering = steering;}
		inline void SetGear(int gear){m_gear = gear;}
		inline void SetOnGround(bool onGround) {m_onGround = onGround;}
		inline void SetVehicleType(int id){m_vehicleTypeID = id;}
		inline void SetNoWheels(int noWheels){m_noWheels = noWheels;}
		inline void SetAcceleration(float acc){m_acceleration = acc;}

		//get
		inline float GetAcceleration() {return m_acceleration;}
		inline const CollisionContactData &GetBarrierContactData() const {return m_BarrierContactData;}
		inline float GetMaxRollBikes() const {return m_maxrollBikes;}
		inline float GetRollBikes()const {return m_rollBikes;}
		inline std::string GetName() {return m_name;}
		inline bool GetPlayer() const {return m_isPlayer;}
		inline Maths::HVec3 GetVehicleSize(){return m_vehicleSize;}
		inline Maths::HVec3 GetPosition() const {return m_pos;}
		inline Maths::HVec3 GetVelocity() const  {return m_velocity;}
		inline Maths::HVec3 GetPositionLast() const {return m_posLast;}
		inline Maths::HVec3 GetVelocityLast() const  {return m_velocityLast;}
		inline Maths::HVec3 GetXaxis() const  {return m_xAxis;}
		inline Maths::HVec3 GetYaxis() const  {return m_yAxis;}
		inline Maths::HVec3 GetZaxis() const  {return m_zAxis;}
		inline Maths::HVec3 GetAngularVelocity() const  {return m_angularVelocity;}
		inline Maths::HVec3 GetAngularVelocityLast() const  {return m_angularVelocityLast;}
		inline Maths::HVec3 GetInertia() const  {return m_inertia;}
		inline Maths::HVec3 GetCMOffset() const  {return m_cmOffset;}
		inline Maths::HVec3 GetExtents() const  {return m_extents;}
		inline float GetSpeed() const {return m_speed;}
		inline float GetSpeedMPH() const {return m_speedMPH;}
		inline float GetSpeedForwardSpeed() const {return m_velocity | m_zAxis;}
		inline float GetMass() const {return m_mass;}
		inline float GetPitch() const {return m_pitch;}
		inline float GetRoll() const {return m_roll;}
		inline float GetYaw() const {return m_yaw;}
		inline float GetRadius() const {return m_radius;}
		inline float GetRevs() const {return m_revs;}
		inline float GetAccel() const {return m_accel;}
		inline float GetBrake() const {return m_brake;}
		inline float GetSteering() const {return m_steering;}
		inline int GetGear() const {return m_gear;}
		inline bool GetOnGround() const  {return m_onGround;}
		inline int GetNoWheels() const {return m_noWheels;}

		inline Wheels *GetWheel(int index)
		{
			assert(m_noWheels >=0 && m_noWheels <= 4); // make sure no. wheels has been set
			assert(index >= 0 && index < m_noWheels);
			return &m_wheels[index];
		}

		inline float GetGearRatios(int i)
		{
			return m_gearRatios[i];
		}

		inline float GetTopSpeed()
		{
			return m_topSpeed;
		}

		inline  float GetRevLimit()
		{
			return m_revLimit;
		}

		inline  float GetMaxRevs()
		{
			return m_maxRevs;
		}

		inline  float GetTyreDiameter()
		{
			return m_tyreDM;
		}

		inline float GetPeakTorque()
		{
			return m_peakTorque;
		}

		inline bool GetHaveSentConstantsOver()
		{
			return m_constantssent;
		}

		inline int GetNumGears(){return m_nGears;}

		inline float GetDiffRatio() // final ratio
		{
			return m_diffRatio;
		}

		inline float GetDrag()
		{
			return m_drag;
		}

		inline float GetBrakingFriction()
		{
			return m_maxBrakingFriction ;
		}

		inline float GetLateralFriction()
		{
			return m_maxLateralFriction;
		}

		inline float GetSurfaceFriction()
		{
			return m_surfaceFriction;
		}

		inline bool GetActive()
		{
			return m_active;
		}


		inline void Clear(){}
	};
}



#endif // _VEHICLE_STATE_H
