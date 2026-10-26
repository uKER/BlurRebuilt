#ifndef RP_CAR_SETTINGS_H_
#define RP_CAR_SETTINGS_H_
#ifdef _MSC_VER
#pragma once
#endif

#include "racingphysics\physics\parameters.h"

namespace RacingPhysics
{

extern bool g_dynamicsChanged;

struct CarDynamicsXt
{
	struct CarPhysicsParametersXt
	{
		float		physicsCofMassOffsetY;
		float		physicsCofMassOffsetZ;
		float		interiaTensorScaleX;
		float		interiaTensorScaleY;
		float		interiaTensorScaleZ;
	};

	struct CarMassXt
	{
		float	mass;
		float	MIScaleX;
		float	MIScaleY;
		float	MIScaleZ;
		float	cofGOffsetY;
		float	cofGOffsetZ;
		float	frontWheelMI;
		float	rearWheelMI;
		float	physicsCofMassOffsetY;
		float	physicsCofMassOffsetZ;
		float	interiaTensorScaleX;
		float	interiaTensorScaleY;
		float	interiaTensorScaleZ;
		XtHandle<CarPhysicsParametersXt> physicsParameters;
	};

	struct CarGeometryXt
	{
		float	wheelBase;
		float	trackWidthFront;
		float	trackWidthRear;
	};

	struct CarTyreXt
	{
		float				radius;
		LinearGraphXt		grip;
		float				loadDependence;
		float				transverseGripScale;
		float				forwardsGripScale;
		float				backwardsGripScale;
		float				transverseTyreSlipScale;
		float				longitudinalTyreSlipScale;
		float				combinedSlipBackwards;
		LinearParameterXt	combinedSlipForwards_Mph;
		LinearParameterXt	slipAlignment_Av;
		float				dynamicFrictionSpeed;
		float				slipSpeed;
		float				slipSmoothTimeInForwards;
		float				slipSmoothTimeOutForwards;
		float				slipSmoothTimeInBackwards;
		float				slipSmoothTimeOutBackwards;
		XtArray<float>		forwardsGripScaleInGear;
	};

	struct CarTyresXt
	{
		CarTyreXt frontTyres;
		CarTyreXt rearTyres;

		const CarTyreXt& GetTyre(int i) const {return (i < 2) ? frontTyres : rearTyres;}
	};

	struct CarWheelSuspensionXt
	{
		float casterAngle;
		float camberAngle;
		float toeAngle;
		float movementDown;
		float movementUp;
		float springStiffness;
		float springProgression;
		float springDampingCompress;
		float springDampingRelease;
		float rollStiffness;
	};

	struct CarSuspensionXt
	{
		CarWheelSuspensionXt front;
		CarWheelSuspensionXt rear;

		const CarWheelSuspensionXt& GetWheel(int i) const {return (i < 2) ? front : rear;}
	};

	struct CarSteeringXt
	{
		float steerLimit;
		float ackerman;
		float steerSmoothTime;
		float steerDirect;
		float steerDamping;
		float reverseSteerDamping;
		float steerSpringDrift;
		float steerSpringCentre;
		float steerSpringLowSpeed;
		float slideRecovery;
		float antiFlick;
		float antiFlickThreshold;
	};

	struct CarBrakesXt
	{
		float frontBrakeForce;
		float rearBrakeForce;
		float handbrakeForce;
		float brakeSmoothTime;
		float handbrakeSmoothTime;

		const float& BrakeForce(int i) const {return (i < 2) ? frontBrakeForce : rearBrakeForce;}
	};

	struct CarEngineXt
	{
		LinearFunctionXt	engineTorque_Rpm;
		float				engineInertia;
		float				limiterRPM;
		float				clutchRPMFraction;
		float				clutchRPMFractionReverse;
		float				idleRPM;
		float				powerCutFraction;
		LinearParameterXt	engineBraking_RpmFraction;
		float				clutchSmoothTime;
		float				clutchDelay;
		float				tractionControlMaxSpin;
		float				tractionControlMaxSlip;
		float				wheelSpinMaxSpeed_Mph;
		float				throttleSmoothTime;
		bool				transverse;
	};

	struct CarGearsXt
	{
		XtArray<float>	gearRatios;
		float			reverseGearRatio;
		int				topGear;
		float			finalRatio;
		float			gearDownRPMFraction;
		float			maxSpeedForChangeToFirst_Mph;
		float			maxSpeedForReverse_Mph;
	};

	enum CarDriveTypeXt
	{
		DRIVE_TYPE_REARWHEEL,
		DRIVE_TYPE_FRONTWHEEL,
		DRIVE_TYPE_FOURWHEEL,
	};

	enum CarDifferentialTypeXt
	{
		DIFFERENTIAL_LOCKED,
		DIFFERENTIAL_TORQUE,
		DIFFERENTIAL_VISCOUS,
	};

	struct CarDifferentialXt
	{
		CarDifferentialTypeXt differentialType;
		float forwardsTorque;
		float backwardsTorque;
		float viscosity;
	};

	struct CarTransmissionXt
	{
		CarDriveTypeXt driveType;
		CarDifferentialXt frontDifferential;
		CarDifferentialXt rearDifferential;
		CarDifferentialXt midDifferential;
	};

	struct CarFrictionXt
	{
		float airFriction;
		float rollingFriction;
		float constantDownforce;
		float downforce;
		float downforceOffset;
		float rotationalAirFriction;
	};

	XtSz8 name;
	CarMassXt mass;
	CarGeometryXt geometry;
	CarSuspensionXt suspension;
	CarSteeringXt steering;
	CarBrakesXt brakes;
	CarEngineXt engine;
	CarGearsXt gears;
	CarTyresXt tyres;
	CarTransmissionXt transmission;
	CarFrictionXt friction;

	CarDynamicsXt();

	static const XtRecord* LoadDynamicsXt(const char* name, bool& changed);
	static void GetDmaData(const XtRecord* record, void** pDmaData);
	static CarDynamicsXt* SpuFetch(const void* pDmaData);
	static bool DynamicsChanged();
};

}

#endif
