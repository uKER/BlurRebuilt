#ifndef GRAVITYCARSETTING_H
#define GRAVITYCARSETTING_H

namespace Gravity
{
	class C_CarTyreXt
	{
	public:
		F32 m_CoefficientOffriction; // the coefficient of friction between tire and road
		F32 m_TireSize;
	};

	class C_CarWheelXt
	{
	public:
		F32 m_FrontWheelRadius;
		F32 m_BackWheelRadius;
		F32 m_FrontWheelWeight;
		F32 m_BackWheelWeight;
		F32 m_FrontWheelRollStiffnesses;
		F32 m_BackWheelRoolStiffnesses;
		F32 m_RollingResistanceCoefficient;
	};


	class C_CarEngineXt
	{
	public:
		//Engine data
		SharedPhysics::SortedArray<F32> m_EngineRMPToTorqueData;
		F32 m_EngineMinRMP;
		F32 m_EngienMaxRMP;
		F32 m_EngineInertia;

		//Transmission
		F32 m_TransmissionInertia;
		F32 m_TransmissionGearRatio;
		F32 m_TransmissionEfficient;

		//Final drive
		F32 m_FinalDriveInertia;
		F32 m_FinalDriveRatio;
		F32 m_FinalDriveEfficent;
	};

	class C_CarXt
	{
	public:
		
		F32 m_WheelBase; // the length between front and back wheel
		F32 m_FrontWheelBase; //the length from the CG to the front wheel
		F32 m_BackWheelBase; //the length from the CG to the back wheel
		F32 m_CGHeight;
		F32 m_TrackWidthFront;
		F32 m_TrackWidthBack;
	};
}

#endif