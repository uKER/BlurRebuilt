#include "stdafx.h"
#include "racingphysics\vehicles\motorbike\motorbikesettings.h"
#include "racingphysics\vehicles\motorbike\motorbike.h"

//#include "Resource\PAKFile.h"

//#ifndef CFG_GOLD
//#include "System\Resource\BizFile.h"
//#endif

#ifdef DEBUG_RACING_PHYSICS
#pragma optimize("", off) // DEBUG_RACING_PHYSICS
#endif

/////////////////////////////////////////////////////////////////////////////////////

namespace RacingPhysics
{

Btl::Vector<MotorbikeDynamicsXt*> gXtBikeDynamics;


MotorbikeDynamicsXt::MotorbikeDynamicsXt()
{
	// fixed settings
	//steering.steerSpringTorque_Mph.clamped = true;
}

Debug::Bool g_loadBikeDynamics("RacingPhysics.Bikes.LoadBikeDynamics", true, Debug::Persist());

const MotorbikeDynamicsXt* MotorbikeDynamicsXt::LoadDynamicsXt(const char* name, bool& changed)
{
	MotorbikeDynamicsXt *xtDyn = NULL;
	changed = false;

	for (int i=0; i<gXtBikeDynamics.Size(); i++)
	{
		if (!stricmp(gXtBikeDynamics[i]->name 
					  , g_loadBikeDynamics ? name : "DefaultBike"))
		{
			xtDyn = gXtBikeDynamics[i];

			if (i == gXtBikeDynamics.Size() - 1)
				changed = true;

			break;
		}
	}

	if (xtDyn == NULL)
	{
		LOGWARN(g_RacingPhysicsLog, "No dynamics found for Motorbike (%s)", name);
		return 0;
	}

	return xtDyn;
}

void MotorbikeDynamicsXtCallback(XtEventType event, XtRecord* record, MotorbikeDynamicsXt *dyn, XtCallbackContext* context)
{
	if (event == kXtEtConstruct)
	{
		gXtBikeDynamics.PushBack(dyn);
	}
	else if (event == kXtEtDestruct)
	{
		Btl::Remove(gXtBikeDynamics, dyn);
	}
}

}

XT_TYPE(RacingPhysics::LinearParameterXt, "RP_LinearParameter")
	XT_FIELD(x0, "x0")
	XT_FIELD(y0, "y0")
	XT_FIELD(x1, "x1")
	XT_FIELD(y1, "y1")
XT_END_TYPE

XT_TYPE(RacingPhysics::LinearFunctionXt, "RP_LinearFunction")
	XT_FIELD(x0, "x0")
	XT_FIELD(x1, "x1")
	XT_FIELD(yScale, "yScale")
	XT_FIELD(dataPoints, "dataPoints")
XT_END_TYPE

XT_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeMassXt, "RP_MotorbikeMass")
	XT_FIELD(wheelMassFraction, "wheelMassFraction")
	XT_FIELD(kerbMass, "kerbMass")
	XT_FIELD(forkMassFraction, "forkMassFraction")
	XT_FIELD(forkMIScale, "forkMIScale")
	XT_FIELD(wheelMIScaleFront, "wheelMIScaleFront")
	XT_FIELD(wheelMIScaleRear, "wheelMIScaleRear")
	XT_FIELD(MIScaleX, "MIScaleX")
	XT_FIELD(MIScaleY, "MIScaleY")
	XT_FIELD(MIScaleZ, "MIScaleZ")
	XT_FIELD(downforce, "downforce")
	XT_FIELD(constantDownforce, "constantDownforce")
XT_END_TYPE

XT_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeRiderXt, "RP_MotorbikeRider")
	XT_FIELD(riderMass, "riderMass")
	XT_FIELD(riderCofGOffsetY, "riderCofGOffsetY")
	XT_FIELD(riderCofGOffsetZ, "riderCofGOffsetZ")
	XT_FIELD(riderMovement, "riderMovement")
	XT_FIELD(riderLeanX, "riderLeanX")
	XT_FIELD(riderLeanZ, "riderLeanZ")
XT_END_TYPE

XT_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeGeometryXt, "RP_MotorbikeGeometry")
	XT_FIELD(wheelBase, "wheelBase")
	XT_FIELD(cofGOffsetY, "cofGOffsetY")
	XT_FIELD(cofGOffsetZ, "cofGOffsetZ")
	XT_FIELD(frontWheelRadius, "frontWheelRadius")
	XT_FIELD(rearWheelRadius, "rearWheelRadius")
	XT_FIELD(frontTubeRadius, "frontTubeRadius")
	XT_FIELD(rearTubeRadius, "rearTubeRadius")
	XT_FIELD(casterAngle_Degrees, "casterAngle_Degrees")
	XT_FIELD(trail, "trail")
XT_END_TYPE

XT_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeSuspensionXt, "RP_MotorbikeSuspension")
	XT_FIELD(frontSpringStiffness, "frontSpringStiffness")
	XT_FIELD(rearSpringStiffness, "rearSpringStiffness")
	XT_FIELD(frontSpringDamping, "frontSpringDamping")
	XT_FIELD(rearSpringDamping, "rearSpringDamping")
	XT_FIELD(maxSuspensionMovement, "maxSuspensionMovement")
	XT_FIELD(tyreSpringMovement, "tyreSpringMovement")
XT_END_TYPE

XT_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeFeetXt, "RP_MotorbikeFeet")
	XT_FIELD(maxSpeedForReverse, "maxSpeedForReverse")
	XT_FIELD(reversingSpeed_Mph, "reversingSpeed_Mph")
	XT_FIELD(footForceScale, "footForceScale")
	XT_FIELD(footForceDamping, "footForceDamping")
	XT_FIELD(footForceSteer, "footForceSteer")
	XT_FIELD(footForceSpeed_Mph, "footForceSpeed_Mph")
	XT_FIELD(footForceOffsetX, "footForceOffsetX")
	XT_FIELD(footForceOffsetZ, "footForceOffsetZ")
	XT_FIELD(kneeForceScale, "kneeForceScale")
	XT_FIELD(kneeForceDamping, "kneeForceDamping")
	XT_FIELD(footFriction, "footFriction")
XT_END_TYPE

XT_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeSteeringXt, "RP_MotorbikeSteering")
	XT_FIELD(steerSpringTorque_Mph, "steerSpringTorque_Mph")
	XT_FIELD(largeAngleSteerSpring, "largeAngleSteerSpring")
	XT_FIELD(steerSpringLimit, "steerSpringLimit")
	XT_FIELD(steerDamping, "steerDamping")
	XT_FIELD(steerDampingReverse, "steerDampingReverse")
	XT_FIELD(steerTorque_Mph, "steerTorque_Mph")
	XT_FIELD(steerSmoothTime, "steerSmoothTime")
	XT_FIELD(leanSteerReductionPower, "leanSteerReductionPower")
	XT_FIELD(feedbackTorque, "feedbackTorque")
	XT_FIELD(feedbackTorqueDamping, "feedbackTorqueDamping")
	XT_FIELD(rollTorque, "rollTorque")
	XT_FIELD(rollFeedbackRate, "rollFeedbackRate")
	XT_FIELD(rollFraction, "rollFraction")
	XT_FIELD(slidingRoll, "slidingRoll")
	XT_FIELD(maxRollTorqueFraction, "maxRollTorqueFraction")
	XT_FIELD(lowSpeedTurnRadiusScale, "lowSpeedTurnRadiusScale")
	XT_FIELD(lowSpeedLean, "lowSpeedLean")
	XT_FIELD(steeringLock, "steeringLock")
XT_END_TYPE

XT_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeBrakesXt, "RP_MotorbikeBrakes")
	XT_FIELD(frontBrakeForce, "frontBrakeForce")
	XT_FIELD(rearBrakeForce, "rearBrakeForce")
	XT_FIELD(rearBrakingInCorner, "rearBrakingInCorner")
	XT_FIELD(antiFlip, "antiFlip")
	XT_FIELD(liftRatio, "liftRatio")
	XT_FIELD(endoDamping, "endoDamping")
	XT_FIELD(endoLiftConstant, "endoLiftConstant")
	XT_FIELD(frontBrakeMaxSlip, "frontBrakeMaxSlip")
	XT_FIELD(rearBrakeMaxSlip, "rearBrakeMaxSlip")
	XT_FIELD(rearBrakeMaxSpin, "rearBrakeMaxSpin")
	XT_FIELD(fullBrakesSmoothTime, "fullBrakesSmoothTime")
	XT_FIELD(rearBrakeSmoothTime, "rearBrakeSmoothTime")
XT_END_TYPE

XT_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeEngineXt, "RP_MotorbikeEngine")
	XT_FIELD(engineTorque_Rpm, "engineTorque_Rpm")
	XT_FIELD(engineInertia, "engineInertia")
	XT_FIELD(limiterRPM, "limiterRPM")
	XT_FIELD(clutchRPMFraction, "clutchRPMFraction")
	XT_FIELD(idleRPM, "idleRPM")
	XT_FIELD(powerCutFraction, "powerCutFraction")
	XT_FIELD(engineBraking_RpmFraction, "engineBraking_RpmFraction")
	XT_FIELD(clutchSpring, "clutchSpring")
	XT_FIELD(clutchSmoothTime, "clutchSmoothTime")
	XT_FIELD(clutchDelay, "clutchDelay")
	XT_FIELD(tractionControl, "tractionControl")
	XT_FIELD(tractionControlMaxSpin, "tractionControlMaxSpin")
	XT_FIELD(tractionControlMaxSlip, "tractionControlMaxSlip")
	XT_FIELD(wheelSpinMaxSpeed_Mph, "wheelSpinMaxSpeed_Mph")
	XT_FIELD(antiFlip, "antiFlip")
	XT_FIELD(liftRatio, "liftRatio")
	XT_FIELD(wheelieDamping, "wheelieDamping")
	XT_FIELD(wheelieLiftConstant, "wheelieLiftConstant")
	XT_FIELD(throttleSmoothTime, "throttleSmoothTime")
XT_END_TYPE

XT_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeGearsXt, "RP_MotorbikeGears")
	XT_FIELD(gearRatios, "gearRatios")
	XT_FIELD(topGear, "topGear")
	XT_FIELD(finalRatio, "finalRatio")
	XT_FIELD(gearDownRPMFraction, "gearDownRPMFraction")
	XT_FIELD(maxSpeedForChangeToFirst, "maxSpeedForChangeToFirst")
XT_END_TYPE

XT_TYPE(RacingPhysics::LinearGraphXt::Point, "RP_GraphPoint")
	XT_FIELD(x, "x")
	XT_FIELD(y, "y")
XT_END_TYPE

XT_TYPE(RacingPhysics::LinearGraphXt, "RP_LinearGraph")
	XT_FIELD(yScale, "yScale")
	XT_FIELD(dataPoints, "data")
XT_END_TYPE

XT_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeTyresXt, "RP_MotorbikeTyres")
	XT_FIELD(grip, "grip")
	XT_FIELD(frontGripScale, "frontGripScale")
	XT_FIELD(rearGripScale, "rearGripScale")
	XT_FIELD(transverseGripScale, "transverseGripScale")
	XT_FIELD(forwardsGripScale, "forwardsGripScale")
	XT_FIELD(backwardsGripScale, "backwardsGripScale")
	XT_FIELD(transverseTyreSlipScale, "transverseTyreSlipScale")
	XT_FIELD(longitudinalTyreSlipScale, "longitudinalTyreSlipScale")
	XT_FIELD(combinedSlipBackwards, "combinedSlipBackwards")
	XT_FIELD(combinedSlipForwards_Mph, "combinedSlipForwards_Mph")
	XT_FIELD(frontTyreSlipScale, "frontTyreSlipScale")
	XT_FIELD(rearTyreSlipScale, "rearTyreSlipScale")
	XT_FIELD(frontCamberForceScale, "frontCamberForceScale")
	XT_FIELD(rearCamberForceScale, "rearCamberForceScale")
	XT_FIELD(slipAlignment_Av, "slipAlignment_Av")
	XT_FIELD(camberAlignment, "camberAlignment")
	XT_FIELD(camberSlipAlignmentFactor, "camberSlipAlignmentScale")
	XT_FIELD(dynamicFrictionSpeed, "dynamicFrictionSpeed")
	XT_FIELD(slipSpeed, "slipSpeed")
	XT_FIELD(slipSmoothTimeInForwards, "slipSmoothTimeInForwards")
	XT_FIELD(slipSmoothTimeOutForwards, "slipSmoothTimeOutForwards")
	XT_FIELD(slipSmoothTimeInBackwards, "slipSmoothTimeInBackwards")
	XT_FIELD(slipSmoothTimeOutBackwards, "slipSmoothTimeOutBackwards")
XT_END_TYPE

XT_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeFrictionXt, "RP_MotorbikeFriction")
	XT_FIELD(airFriction, "airFriction")
	XT_FIELD(riderFrictionScale, "riderFrictionScale")
	XT_FIELD(rollingFriction, "rollingFriction")
	XT_FIELD(wheelieFriction, "wheelieFriction")
XT_END_TYPE

XT_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeRumbleXt, "RP_MotorbikeRumble")
	XT_FIELD(suspensionCutOff, "suspensionCutOff")
	XT_FIELD(suspensionScale, "suspensionScale")
	XT_FIELD(suspensionLRBalance, "suspensionLRBalance")
	XT_FIELD(wheelSpinCutOff, "wheelSpinCutOff")
	XT_FIELD(wheelSpinScale, "wheelSpinScale")
	XT_FIELD(wheelSpinLRBalance, "wheelSpinLRBalance")
	XT_FIELD(engineCutOff, "engineCutOff")
	XT_FIELD(engineScale, "engineScale")
	XT_FIELD(engineLRBalance, "engineLRBalance")
	XT_FIELD(engineClutchScale, "engineClutchScale")
	XT_FIELD(surfaceSpeedCutOff, "surfaceSpeedCutOff")
	XT_FIELD(surfaceSpeedScale, "surfaceSpeedScale")
	XT_FIELD(surfaceSpeedLRBalance, "surfaceSpeedLRBalance")
	XT_FIELD(rumbleStripCutOff, "rumbleStripCutOff")
	XT_FIELD(rumbleStripScale, "rumbleStripScale")
	XT_FIELD(rumbleStripLRBalance, "rumbleStripLRBalance")
	XT_FIELD(rumbleStripPeriod, "rumbleStripPeriod")
	XT_FIELD(collisionDecay, "collisionDecay")
	XT_FIELD(collisionLRBalance, "collisionLRBalance")
	XT_FIELD(collisionScale, "collisionScale")
	XT_FIELD(gearDecay, "gearDecay")
	XT_FIELD(gearLRBalance, "gearLRBalance")
	XT_FIELD(gearScale, "gearScale")
	XT_FIELD(shallowPuddleScale, "shallowPuddleScale")
	XT_FIELD(shallowPuddleBalance, "shallowPuddleBalance")
	XT_FIELD(deepPuddleScale, "deepPuddleScale")
	XT_FIELD(deepPuddleBalance, "deepPuddleBalance")
	XT_FIELD(testLeft, "testLeft")
	XT_FIELD(testRight, "testRight")
	XT_FIELD(singleEffect, "singleEffect")
XT_END_TYPE

XT_TYPE(RacingPhysics::MotorbikeDynamicsXt, "RP_MotorbikeDynamics")
	XT_CALLBACK(RacingPhysics::MotorbikeDynamicsXtCallback)
	XT_FIELD(name, "name")
	XT_FIELD(mass, "mass")
	XT_FIELD(rider, "rider")
	XT_FIELD(geometry, "geometry")
	XT_FIELD(suspension, "suspension")
	XT_FIELD(feet, "feet")
	XT_FIELD(steering, "steering")
	XT_FIELD(brakes, "brakes")
	XT_FIELD(engine, "engine")
	XT_FIELD(gears, "gears")
	XT_FIELD(tyres, "tyres")
	XT_FIELD(friction, "friction")
	XT_FIELD(rumble, "rumble")
XT_END_TYPE

XT_LIBRARY(RacingPhysics_MotorbikeDynamics)
	XT_REGISTER_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeMassXt)
	XT_REGISTER_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeRiderXt)
	XT_REGISTER_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeGeometryXt)
	XT_REGISTER_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeSuspensionXt)
	XT_REGISTER_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeFeetXt)
	XT_REGISTER_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeSteeringXt)
	XT_REGISTER_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeBrakesXt)
	XT_REGISTER_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeEngineXt)
	XT_REGISTER_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeGearsXt)
	XT_REGISTER_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeTyresXt)
	XT_REGISTER_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeFrictionXt)
	XT_REGISTER_TYPE(RacingPhysics::MotorbikeDynamicsXt::MotorbikeRumbleXt)
	XT_REGISTER_TYPE(RacingPhysics::MotorbikeDynamicsXt)
XT_END_LIBRARY
