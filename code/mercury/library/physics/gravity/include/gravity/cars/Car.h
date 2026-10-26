#ifndef GRAVITYCAR_H
#define GRAVITYCAR_H

#include "gravity/sdk/Body.h"
#include "gravity/cars/CarSetting.h"

namespace Gravity
{
	class C_Car;

	class C_CarWheel
	{
	public:
		C_CarWheel()
		{}
		~C_CarWheel(){}

		void SetId(const U32 id)
		{
			m_Id = id;
		}

		void SetConstraint(SharedPhysics::C_Constraint* pConstraint);

		Gravity::C_Body* GetBody()
		{
			return m_pBody;
		}


		void SetUpWheel(const Maths::HF32 PCREF mass, const Maths::HVec3 PCREF position, const Maths::HMat33 PCREF orientation, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height, const Gravity::C_ExternalConstants PCREF externalConstants, Collision::C_CollisionFilter* pFilter);

		friend class C_Car;
	
		Maths::HF32 m_TractiveFroce;
		Maths::HF32 m_RollingResistanceForces;
		SharedPhysics::C_Constraint* m_Constraint;
		Gravity::C_Body* m_pBody;//It has the inertial tentor of the wheel
		C_Car* m_Car;
		//C_CarTyre m_Tyre;
		U32 m_Id;
		F32 m_Radius;
		
	};

	class C_CarEngineRotor
	{
	public:

		F32 GetAngularVelocity(){ return m_AngularVelocity;}

		F32 m_AngularVelocity;
	};

	class C_CarEngine
	{
	public:

		//Get the engine rotational speed (rev per minute)
		F32 GetRPM(){return m_Rotor.GetAngularVelocity();}
		F32 GetTorque(const F32 rpm);
		C_CarEngineXt m_Xt;
		C_CarEngineRotor m_Rotor;
	};

	class C_CarController
	{
	public:
		C_CarController(){};
		~C_CarController(){};

		void Clear()
		{
			m_Throttle = m_Steer = m_Brake = m_HandBrake = m_GearUp = m_GearDown = 0.f;
		}

		F32 m_Throttle;
		F32 m_Steer;
		F32 m_Brake;
		F32 m_HandBrake;
		F32 m_GearUp;
		F32 m_GearDown;
	};

	class C_Car
	{
	public:
		C_Car(const U32 numWheels = 4) : m_NumWheels(numWheels), m_pScene(NULL)
		{
			m_Wheels = new Gravity::C_CarWheel[m_NumWheels];
		}

		~C_Car()
		{
			if(m_Wheels)
				delete[] m_Wheels;
		}

		void SetUpCarBody(Gravity::C_Body* pBody)
		{
			m_pCarBody= pBody;
		}

		void SetScene(Gravity::C_Scene* pScene)
		{
			m_pScene = pScene;
		}

		void DisplayDebug();

		//void Transform();


		Gravity::C_CarWheel* GetCarWheel(const U32 index)
		{
			BIZ_ASSERT(index <= 4);
			return &m_Wheels[index];
		}

		Gravity::C_Body* GetCarBody()
		{
			return m_pCarBody;
		}

		void InitializeCarWheel(const U32 index)
		{
			C_CarWheel* wheel = &m_Wheels[index];
			wheel->m_Radius = (index < 2) ? m_CarWheelXt.m_FrontWheelRadius : m_CarWheelXt.m_BackWheelRadius;
			wheel->m_Id = index;
			wheel->m_Car = this;
		}

		Maths::HVec3 CalculateWheelPosition(const U32 index)
		{
			using namespace Maths;
			C_CarWheel* wheel = &m_Wheels[index];
			const HF32 z(m_CarXt.m_WheelBase*0.5f);
			const F32 width = (index < 2)? m_CarXt.m_TrackWidthFront : m_CarXt.m_TrackWidthBack;
			const HF32 x(width*0.5f);

			const HF32 radius = (index < 2)? m_CarWheelXt.m_FrontWheelRadius: m_CarWheelXt.m_BackWheelRadius;
			//For Amax cars
			const HF32 y=m_pCarBody->ElementMatrix().Translation().Y() + radius;
			//For Nightwing cars
			//m_pCarBody->ElementMatrix().Translation().Y();
			const Maths::HVec3 vector(x, y, z);
			return m_pCarBody->GetBody().m_Position + vector * m_pCarBody->GetBody().m_Orientation;
			
		}


		F32 CalculateTractiveForceWithoutInertialLoss(const F32 rmp, const U32 wheelIndex);

		//Calculate the translational speed at the circumference of the tire
		F32 CalculateWheelTranslationalSpeed(const U32 wheelIndex);

		F32 CalculateLoadWeightOnFrontWheel(const F32 acceleration, const F32 theta);
		F32 CalculateLoadWeightOnBackWheel(const F32 acceleration, const F32 theta);

		F32 CalculateRollingResistance(const F32 acceleration, const F32 theta);

		C_CarController& GetController()
		{
			return m_Controller;
		}
		
		friend class C_CarWheel;
//	private:
		
		C_CarController m_Controller;
		C_CarEngine m_Engine;
		C_CarXt m_CarXt;
		C_CarWheelXt m_CarWheelXt;
		C_CarTyreXt m_CarTyreXt;

		Gravity::C_Scene* m_pScene;
		Gravity::C_Body* m_pCarBody;
		Gravity::C_CarWheel* m_Wheels;
		U32 m_NumWheels;
		
	};

}

#endif