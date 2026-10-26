#ifndef GRAVITYRACINGCAR_H
#define GRAVITYRACINGCAR_H

namespace Solver2
{
	enum CarType
	{
		FrontWheelDrive,
		RearWheelDrive,
		FourWheelDrive
	};

	class GravityCarWheel
	{
	public:
		Maths::HF32 m_Mass;
		Maths::HF32 m_DriveInertias;	//in-lb-sec2
		Maths::HF32 m_NonDriveInertias;
		Maths::HF32 m_Radius;	//inch
		Maths::HF32 m_RollStiffnesses;
	};

	class GravityCarTransmission
	{
	public:
		F32[5] m_Inertias;
		F32[5] m_Ratios;
		F32[5] m_Efficiencies;
	};

	class GravityCar
	{
	public:
		GravityCar(U32 type): m_Type(type)
		{
		}

		~virtual GravityCar()
		{
		}

		Maths::HF32 m_Mass; //the vehicle mass
		Maths::HF32 m_Gravity;
		Maths::HVec3 m_CenterOfGravity;
		Maths::Mat33 m_Rotation;
		Maths::HF32 m_WheelBase; //L
		Maths::HF32 m_Tread; //t
		Maths::HF32 m_CarHeight; //h (center of gravity to the ground)
		
		Maths::HF32 m_FrontRollingResistanceForce;
		Maths::HF32 m_RearRollingResistanceForce;

		Maths::HF32 m_WheelRotationalAcceleration;
		//Maths::HF32 m_WheelRadius;

		Maths::HF32 m_EngineInertia; //in-lb-sec2 0.8
		Maths::HF32 m_RPM; //revolution per minute 1 rev = 360 degree
		Maths::HF32 m_EngineTorque; // each engine torque conressponding to a revolution per minute
		U32 m_Type; //front wheel drive car, rear wheel drive car, or four wheel drive car
		//Maths::HF32 m_FrontRollStiffnesses;
		//Maths::HF32 m_RearRollStiffnesses;
		Maths::HF32 m_FrontAxleWeight;
		Maths::HF32 m_RearAxleWeight;
		Maths::HF32 m_FinalDriveRatio;

		GravityCarWheel[4] m_CarWheels;
		GravityCarTransmission m_Transmission;

		virtual Maths::HVec3 GetLongitudinalAcceralationWithLocking(Maths::HF32 coefficientOfFriction)
		{
		}

		virtual Maths::HVec3 GetLongitudinalAcceralationWithOutLocking(Maths::HF32 coefficientOfFriction)
		{
		}
		
	};

	class FrontWheelDriveCar : public GravityCar
	{
	public:
		FrontWheelDriveCar() : public GravityCar(CarType::FrontWheelDriveCar)
		{
		}

		~virtual FrontWheelDriveCar()
		{
		}

		virtual Maths::HVec3 GetLongitudinalAcceralationWithLocking(Maths::HF32 coefficientOfFriction)
		{
			const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
			const Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());

			const Maths::HF32 frontRollStiffnesses = m_CarWheels[0].m_RollStiffnesses;
			const Maths::HF32 rearRollStiffnesses  = m_CarWheels[2].m_RollStiffnesses;

			const Maths::HF32 rollStiffnesses=frontRollStiffnesses + rearRollStiffnesses; 

			const Maths::HF32 d0((m_CarHeight/m_WheelBase) * coefficientOfFriction);

			const Maths::HF32 d1((two * coefficientOfFriction * m_CarWheels[i].m_Size)/(m_FinalDriveRatio * m_Tread));

			const Maths::HF32 d2(rearRollStiffnesses/ rollStiffnesses);

			const Maths::HF32 n0(coefficientOfFriction * m_FrontAxleWeight);

			const Maths::HF32 tractiveForce=n0/(one + d0 - d1*d2);

			const Maths::HF32 vehicleWeight=m_Mass*m_Gravity;

			return tractiveForce/vehicleWeight;
		}

		virtual Maths::HVec3 GetLongitudinalAcceralationWithOutLocking(Maths::HF32 coefficientOfFriction)
		{
			const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
			const Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());

			const Maths::HF32 d0((m_CarHeight/m_WheelBase) * coefficientOfFriction);

			const Maths::HF32 n0(coefficientOfFriction * m_FrontAxleWeight);

			const Maths::HF32 tractiveForce=n0/(one + d0 );

			const Maths::HF32 vehicleWeight=m_Mass*m_Gravity;

			return tractiveForce/vehicleWeight;
		}
	};

	class RearWheelDriveCar: public GravityCar
	{
	public:
		RearWheelDriveCar() : public GravityCar(CarType::RearWheelDriveCar)
		{
		}

		~virtual RearWheelDriveCar()
		{
		}

		virtual Maths::HVec3 GetLongitudinalAcceralationWithLocking(Maths::HF32 coefficientOfFriction)
		{
			const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
			const Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());

			const Maths::HF32 frontRollStiffnesses = m_CarWheels[0].m_RollStiffnesses;
			const Maths::HF32 rearRollStiffnesses  = m_CarWheels[2].m_RollStiffnesses;
			const Maths::HF32 rollStiffnesses=frontRollStiffnesses + rearRollStiffnesses;

			const Maths::HF32 d0((m_CarHeight/m_WheelBase) * coefficientOfFriction);

			const Maths::HF32 d1((two * coefficientOfFriction * m_WheelRadius)/(m_FinalDriveRatio * m_Tread));

			const Maths::HF32 d2(frontRollStiffnesses/rollStiffnesses);

			const Maths::HF32 n0(coefficientOfFriction * m_RearAxleWeight);

			const Maths::HF32 tractiveForce=n0/(one - d0 - d1*d2);

			const Maths::HF32 vehicleWeight=m_Mass*m_Gravity;

			return tractiveForce/vehicleWeight;
		}

		virtual Maths::HVec3 GetLongitudinalAcceralationWithOutLocking(Maths::HF32 coefficientOfFriction)
		{

			const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
			const Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());

			const Maths::HF32 d0((m_CarHeight/m_WheelBase) * coefficientOfFriction);

			const Maths::HF32 n0(coefficientOfFriction * m_RearAxleWeight);

			const Maths::HF32 tractiveForce=n0/(one - d0);

			const Maths::HF32 vehicleWeight=m_Mass*m_Gravity;

			return tractiveForce/vehicleWeight;
		}
	};

	class FourWheelDriveCar: public GravityCar
	{
	public:
		FourWheelDriveCar() : public GravitGar(CarType::FourWheelDriveCar)
		{
		}

		~virtual FourWheelDriveCar()
		{
		}

		virtual Maths::HVec3 GetLongitudinalAcceralationWithLocking(Maths::HF32 coefficientOfFriction)
		{
		}

		virtual Maths::HVec3 GetLongitudinalAcceralationWithOutLocking(Maths::HF32 coefficientOfFriction)
		{
		}
	};
}
#endif