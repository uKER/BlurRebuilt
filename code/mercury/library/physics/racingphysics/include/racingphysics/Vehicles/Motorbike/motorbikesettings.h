#ifndef RP_MOTORBIKE_SETTINGS_H
#define RP_MOTORBIKE_SETTINGS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "racingphysics\physics\parameters.h"

///////////////////////////////////////////////////////////

namespace RacingPhysics
{

class Motorbike;

struct MotorbikeDynamicsXt
{
	struct MotorbikeMassXt
	{
		float	wheelMassFraction;
		float	kerbMass;
		float	forkMassFraction;
		float	forkMIScale;
		float	wheelMIScaleFront;
		float	wheelMIScaleRear;
		float	MIScaleX;
		float	MIScaleY;
		float	MIScaleZ;
		float	downforce;
		float	constantDownforce;
	};

	struct MotorbikeRiderXt
	{
		float	riderMass;
		float	riderCofGOffsetY;
		float	riderCofGOffsetZ;
		float	riderMovement;
		float	riderLeanX;
		float	riderLeanZ;
	};

	struct MotorbikeGeometryXt
	{
		float	wheelBase;
		float	cofGOffsetY;
		float	cofGOffsetZ;
		float	frontWheelRadius;
		float	rearWheelRadius;
		float	frontTubeRadius;
		float	rearTubeRadius;
		float	casterAngle_Degrees;
		float	trail;

		float WheelRadius(int i) const {return (i == 0) ? frontWheelRadius : rearWheelRadius;}
		float TubeRadius(int i) const {return (i == 0) ? frontTubeRadius : rearTubeRadius;}
	};

	struct MotorbikeSuspensionXt
	{
		float	frontSpringStiffness;
		float	rearSpringStiffness;
		float	frontSpringDamping;
		float	rearSpringDamping;
		float	maxSuspensionMovement;
		float	tyreSpringMovement;

		float SpringConstant(int i) const {return (i == 0) ? frontSpringStiffness : rearSpringStiffness;}
		float SpringDamping(int i) const {return (i == 0) ? frontSpringDamping : rearSpringDamping;}
	};

	struct MotorbikeFeetXt
	{
		float	maxSpeedForReverse;
		float	reversingSpeed_Mph;
		float	footForceScale;
		float	footForceDamping;
		float	footForceSteer;
		float	footForceSpeed_Mph;
		float	footForceOffsetX;
		float	footForceOffsetZ;
		float	kneeForceScale;
		float	kneeForceDamping;
		float	footFriction;
	};

	struct MotorbikeSteeringXt
	{
		LinearParameterXt	steerSpringTorque_Mph;
		float				largeAngleSteerSpring;
		float				steerSpringLimit;
		float				steerDamping;
		float				steerDampingReverse;
		LinearFunctionXt	steerTorque_Mph;
		float				steerSmoothTime;
		float				leanSteerReductionPower;
		float				feedbackTorque;
		float				feedbackTorqueDamping;
		float				rollTorque;
		float				rollFeedbackRate;
		float				rollFraction;
		float				slidingRoll;
		float				maxRollTorqueFraction;
		float				lowSpeedTurnRadiusScale;
		float				lowSpeedLean;
		float				steeringLock;
	};

	struct MotorbikeBrakesXt
	{
		float			frontBrakeForce;
		float			rearBrakeForce;
		float			rearBrakingInCorner;
		bool			antiFlip;
		float			liftRatio;
		float			endoDamping;
		float			endoLiftConstant;
		float			frontBrakeMaxSlip;
		float			rearBrakeMaxSlip;
		float			rearBrakeMaxSpin;
		float			fullBrakesSmoothTime;
		float			rearBrakeSmoothTime;

		float BrakeForce(int i) const {return (i == 0) ? frontBrakeForce : rearBrakeForce;}
	};

	struct MotorbikeEngineXt
	{
		LinearFunctionXt	engineTorque_Rpm;
		float				engineInertia;
		float				limiterRPM;
		float				clutchRPMFraction;
		float				idleRPM;
		float				powerCutFraction;
		LinearParameterXt	engineBraking_RpmFraction;
		float				clutchSpring;
		float				clutchSmoothTime;
		float				clutchDelay;
		bool				tractionControl;
		float				tractionControlMaxSpin;
		float				tractionControlMaxSlip;
		float				wheelSpinMaxSpeed_Mph;
		bool				antiFlip;
		float				liftRatio;
		float				wheelieDamping;
		float				wheelieLiftConstant;
		float				throttleSmoothTime;
	};

	struct MotorbikeGearsXt
	{
		XtArray<float>	gearRatios;
		int				topGear;
		float			finalRatio;
		float			gearDownRPMFraction;
		float			maxSpeedForChangeToFirst;
	};

	struct MotorbikeFrictionXt
	{
		float	airFriction;
		float	riderFrictionScale;
		float	rollingFriction;
		float	wheelieFriction;
	};

	struct MotorbikeTyresXt
	{
		LinearGraphXt		grip;
		float				frontGripScale;
		float				rearGripScale;
		float				transverseGripScale;
		float				forwardsGripScale;
		float				backwardsGripScale;
		float				transverseTyreSlipScale;
		float				longitudinalTyreSlipScale;
		float				combinedSlipBackwards;
		LinearParameterXt	combinedSlipForwards_Mph;
		float				frontTyreSlipScale;
		float				rearTyreSlipScale;
		float				frontCamberForceScale;
		float				rearCamberForceScale;
		LinearParameterXt	slipAlignment_Av;
		float				camberAlignment;
		float				camberSlipAlignmentFactor;
		float				dynamicFrictionSpeed;
		float				slipSpeed;
		float				slipSmoothTimeInForwards;
		float				slipSmoothTimeOutForwards;
		float				slipSmoothTimeInBackwards;
		float				slipSmoothTimeOutBackwards;

		float GripScale(int i) const {return (i == 0) ? frontGripScale : rearGripScale;}
		float TyreSlipScale(int i) const {return (i == 0) ? frontTyreSlipScale : rearTyreSlipScale;}
		float CamberForceScale(int i) const {return (i == 1) ? frontCamberForceScale : rearCamberForceScale;}
	};

	struct MotorbikeRumbleXt
	{
		float suspensionCutOff;
		float suspensionScale;
		float suspensionLRBalance;

		float wheelSpinCutOff;
		float wheelSpinScale;
		float wheelSpinLRBalance;

		float engineCutOff;
		float engineScale;
		float engineLRBalance;
		float engineClutchScale;

		float surfaceSpeedCutOff;
		float surfaceSpeedScale;
		float surfaceSpeedLRBalance;

		float rumbleStripCutOff;
		float rumbleStripScale;
		float rumbleStripLRBalance;
		float rumbleStripPeriod;

		float collisionDecay;
		float collisionLRBalance;
		float collisionScale;

		float gearDecay;
		float gearLRBalance;
		float gearScale;

		float shallowPuddleScale;
		float shallowPuddleBalance;

		float deepPuddleScale;
		float deepPuddleBalance;

		float testLeft;
		float testRight;

		int singleEffect;
	};

	XtSz8					name;
	MotorbikeMassXt			mass;
	MotorbikeRiderXt		rider;
	MotorbikeGeometryXt		geometry;
	MotorbikeSuspensionXt	suspension;
	MotorbikeFeetXt			feet;
	MotorbikeSteeringXt		steering;
	MotorbikeBrakesXt		brakes;
	MotorbikeEngineXt		engine;
	MotorbikeGearsXt		gears;
	MotorbikeTyresXt		tyres;
	MotorbikeFrictionXt		friction;
	MotorbikeRumbleXt		rumble;

	MotorbikeDynamicsXt();

	static const MotorbikeDynamicsXt* LoadDynamicsXt(const char* name, bool& changed);
};

}

#endif
