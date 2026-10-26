#include "stdafx.h"
#include "racingphysics\vehicles\car\carsettings.h"
#include "racingphysics\vehicles\car\car.h"

#ifdef DEBUG_RACING_PHYSICS
#pragma optimize ("", off)
#endif

/////////////////////////////////////////////////////////////////////////////////////

namespace RacingPhysics
{

bool g_dynamicsChanged = false;

struct ALIGN16 DmaData
{
	XtRecord* pRecord;
	U8* pXtBase;
	U32 XtSize;
	Btl::Vector<U32> XtFixups;
};

#if !PLATFORM_SPU

void GenerateXtFixups(DmaData& dd)
{
	// xt data
	dd.pXtBase = XtGetRecordBaseAddr(dd.pRecord);
	dd.XtSize = XtGetRecordSize(dd.pRecord);
	U32 fixupCount = XtGetRecordFixupCount(dd.pRecord);
	dd.XtFixups.Resize(fixupCount);

	if (fixupCount)
		XtGenerateRecordFixups(dd.pRecord, &dd.XtFixups[0], fixupCount);
}

Btl::Vector<DmaData> gXtCarDynamics;

CarDynamicsXt::CarDynamicsXt()
{
}

Debug::Bool g_loadCarDynamics("RacingPhysics.Cars.LoadCarDynamics", true, Debug::Persist());

const XtRecord* CarDynamicsXt::LoadDynamicsXt(const char* name, bool& changed)
{
	XtRecord *xtDyn = NULL;
	changed = false;

	for (int i=0; i<gXtCarDynamics.Size(); i++)
	{
		if (!stricmp(((CarDynamicsXt*)(gXtCarDynamics[i].pXtBase))->name 
					  , g_loadCarDynamics ? name : "DefaultCar"))
		{
			xtDyn = gXtCarDynamics[i].pRecord;

			if (i == gXtCarDynamics.Size() - 1)
			{
				changed = true;
				g_dynamicsChanged = false;
			}
		}
	}

	if (xtDyn == NULL)
	{
		LOGWARN(g_RacingPhysicsLog, "No dynamics found for car (%s)", name);
		return 0;
	}

	return xtDyn;
}

void CarDynamicsXtCallback(XtEventType event, XtRecord* record, CarDynamicsXt *dyn, XtCallbackContext* context)
{
	if (event == kXtEtConstruct)
	{
		gXtCarDynamics.PushBack();
		gXtCarDynamics.Back().pRecord = record;
		GenerateXtFixups(gXtCarDynamics.Back());
		g_dynamicsChanged = true;
	}
	else if (event == kXtEtDestruct)
	{
		for (int i = 0; i < gXtCarDynamics.Size(); i++)
		{
			if (gXtCarDynamics[i].pRecord == record)
			{
				gXtCarDynamics.EraseByIndex(i);
				break;
			}
		}
	}

	XtReloadVehicles();
}

void CarDynamicsXt::GetDmaData(const XtRecord* record, void** pDmaData)
{
	for (int i = 0; i < gXtCarDynamics.Size(); i++)
	{
		if (gXtCarDynamics[i].pRecord == record)
		{
			*pDmaData = &gXtCarDynamics[i];
			return;
		}
	}

	*pDmaData = 0;
}

CarDynamicsXt* CarDynamicsXt::SpuFetch(const void* pDmaData) {return 0;}

#else

CarDynamicsXt* CarDynamicsXt::SpuFetch(const void* pDmaData)
{
	static DmaData s_dd;

	// get the dma data block
	Spu::Fetch(&s_dd, pDmaData, sizeof(DmaData));

	if (s_dd.pXtBase)
	{
		U8* pXtData = (U8*) Spu::FetchAllocAlign(s_dd.pXtBase, s_dd.XtSize);

		Btl::SpuFetch(s_dd.XtFixups);

		if (!s_dd.XtFixups.Empty())
			XtApplyRecordFixups(pXtData, s_dd.pXtBase, &s_dd.XtFixups[0], s_dd.XtFixups.Size(), s_dd.XtSize);

		return (CarDynamicsXt*) pXtData;
	}

	return 0;
}

#endif

}

#if !PLATFORM_SPU

XT_TYPE(RacingPhysics::CarDynamicsXt::CarPhysicsParametersXt, "RP_CarPhysicsParameters")
	XT_FIELD(physicsCofMassOffsetY, "physicsCofMassOffsetY")
	XT_FIELD(physicsCofMassOffsetZ, "physicsCofMassOffsetZ")
	XT_FIELD(interiaTensorScaleX, "interiaTensorScaleX")
	XT_FIELD(interiaTensorScaleY, "interiaTensorScaleY")
	XT_FIELD(interiaTensorScaleZ, "interiaTensorScaleZ")
XT_END_TYPE

XT_TYPE(RacingPhysics::CarDynamicsXt::CarMassXt, "RP_CarMass")
	XT_FIELD(mass, "mass")
	XT_FIELD(MIScaleX, "MIScaleX")
	XT_FIELD(MIScaleY, "MIScaleY")
	XT_FIELD(MIScaleZ, "MIScaleZ")
	XT_FIELD(cofGOffsetY, "cofGOffsetY")
	XT_FIELD(cofGOffsetZ, "cofGOffsetZ")
	XT_FIELD(frontWheelMI, "frontWheelMI")
	XT_FIELD(rearWheelMI, "rearWheelMI")
	XT_FIELD(physicsCofMassOffsetY, "physicsCofMassOffsetY")
	XT_FIELD(physicsCofMassOffsetZ, "physicsCofMassOffsetZ")
	XT_FIELD(interiaTensorScaleX, "interiaTensorScaleX")
	XT_FIELD(interiaTensorScaleY, "interiaTensorScaleY")
	XT_FIELD(interiaTensorScaleZ, "interiaTensorScaleZ")
	XT_FIELD(physicsParameters, "physicsParameters")
XT_END_TYPE

XT_TYPE(RacingPhysics::CarDynamicsXt::CarGeometryXt, "RP_CarGeometry")
	XT_FIELD(wheelBase, "wheelBase")
	XT_FIELD(trackWidthFront, "trackWidthFront")
	XT_FIELD(trackWidthRear, "trackWidthRear")
XT_END_TYPE

XT_TYPE(RacingPhysics::CarDynamicsXt::CarTyreXt, "RP_CarTyre")
	XT_FIELD(radius, "radius")
	XT_FIELD(grip, "grip")
	XT_FIELD(loadDependence, "loadDependence")
	XT_FIELD(transverseGripScale, "transverseGripScale")
	XT_FIELD(forwardsGripScale, "forwardsGripScale")
	XT_FIELD(backwardsGripScale, "backwardsGripScale")
	XT_FIELD(transverseTyreSlipScale, "transverseTyreSlipScale")
	XT_FIELD(longitudinalTyreSlipScale, "longitudinalTyreSlipScale")
	XT_FIELD(combinedSlipBackwards, "combinedSlipBackwards")
	XT_FIELD(combinedSlipForwards_Mph, "combinedSlipForwards_Mph")
	XT_FIELD(slipAlignment_Av, "slipAlignment_Av")
	XT_FIELD(dynamicFrictionSpeed, "dynamicFrictionSpeed")
	XT_FIELD(slipSpeed, "slipSpeed")
	XT_FIELD(slipSmoothTimeInForwards, "slipSmoothTimeInForwards")
	XT_FIELD(slipSmoothTimeOutForwards, "slipSmoothTimeOutForwards")
	XT_FIELD(slipSmoothTimeInBackwards, "slipSmoothTimeInBackwards")
	XT_FIELD(slipSmoothTimeOutBackwards, "slipSmoothTimeOutBackwards")
	XT_FIELD(forwardsGripScaleInGear, "forwardsGripScaleInGear")
XT_END_TYPE

XT_TYPE(RacingPhysics::CarDynamicsXt::CarTyresXt, "RP_CarTyres")
	XT_FIELD(frontTyres, "frontTyres")
	XT_FIELD(rearTyres, "rearTyres")
XT_END_TYPE

XT_TYPE(RacingPhysics::CarDynamicsXt::CarWheelSuspensionXt, "RP_CarWheelSuspension")
	XT_FIELD(casterAngle, "casterAngle")
	XT_FIELD(camberAngle, "camberAngle")
	XT_FIELD(toeAngle, "toeAngle")
	XT_FIELD(movementDown, "movementDown")
	XT_FIELD(movementUp, "movementUp")
	XT_FIELD(springStiffness, "springStiffness")
	XT_FIELD(springProgression, "springProgression")
	XT_FIELD(springDampingCompress, "springDampingCompress")
	XT_FIELD(springDampingRelease, "springDampingRelease")
	XT_FIELD(rollStiffness, "rollStiffness")
XT_END_TYPE

XT_TYPE(RacingPhysics::CarDynamicsXt::CarSuspensionXt, "RP_CarSuspension")
	XT_FIELD(front, "front")
	XT_FIELD(rear, "rear")
XT_END_TYPE

XT_TYPE(RacingPhysics::CarDynamicsXt::CarSteeringXt, "RP_CarSteering")
	XT_FIELD(steerLimit, "steerLimit")
	XT_FIELD(ackerman, "ackerman")
	XT_FIELD(steerSmoothTime, "steerSmoothTime")
	XT_FIELD(steerDirect, "steerDirect")
	XT_FIELD(steerDamping, "steerDamping")
	XT_FIELD(reverseSteerDamping, "reverseSteerDamping")
	XT_FIELD(steerSpringDrift, "steerSpringDrift")
	XT_FIELD(steerSpringCentre, "steerSpringCentre")
	XT_FIELD(steerSpringLowSpeed, "steerSpringLowSpeed")
	XT_FIELD(slideRecovery, "slideRecovery")
	XT_FIELD(antiFlick, "antiflick")
	XT_FIELD(antiFlickThreshold, "antiflickThreshold")
XT_END_TYPE

XT_TYPE(RacingPhysics::CarDynamicsXt::CarBrakesXt, "RP_CarBrakes")
	XT_FIELD(frontBrakeForce, "frontBrakeForce")
	XT_FIELD(rearBrakeForce, "rearBrakeForce")
	XT_FIELD(handbrakeForce, "handbrakeForce")
	XT_FIELD(brakeSmoothTime, "brakeSmoothTime")
	XT_FIELD(handbrakeSmoothTime, "handbrakeSmoothTime")
XT_END_TYPE

XT_TYPE(RacingPhysics::CarDynamicsXt::CarEngineXt, "RP_CarEngine")
	XT_FIELD(engineTorque_Rpm, "engineTorque_Rpm")
	XT_FIELD(engineInertia, "engineInertia")
	XT_FIELD(limiterRPM, "limiterRPM")
	XT_FIELD(clutchRPMFraction, "clutchRPMFraction")
	XT_FIELD(clutchRPMFractionReverse, "clutchRPMFractionReverse")
	XT_FIELD(idleRPM, "idleRPM")
	XT_FIELD(powerCutFraction, "powerCutFraction")
	XT_FIELD(engineBraking_RpmFraction, "engineBraking_RpmFraction")
	XT_FIELD(clutchSmoothTime, "clutchSmoothTime")
	XT_FIELD(clutchDelay, "clutchDelay")
	XT_FIELD(tractionControlMaxSpin, "tractionControlMaxSpin")
	XT_FIELD(tractionControlMaxSlip, "tractionControlMaxSlip")
	XT_FIELD(wheelSpinMaxSpeed_Mph, "wheelSpinMaxSpeed_Mph")
	XT_FIELD(throttleSmoothTime, "throttleSmoothTime")
	XT_FIELD(transverse, "transverse")
XT_END_TYPE

XT_TYPE(RacingPhysics::CarDynamicsXt::CarGearsXt, "RP_CarGears")
	XT_FIELD(gearRatios, "gearRatios")
	XT_FIELD(reverseGearRatio, "reverseGearRatio")
	XT_FIELD(topGear, "topGear")
	XT_FIELD(finalRatio, "finalRatio")
	XT_FIELD(gearDownRPMFraction, "gearDownRPMFraction")
	XT_FIELD(maxSpeedForChangeToFirst_Mph, "maxSpeedForChangeToFirst_Mph")
	XT_FIELD(maxSpeedForReverse_Mph, "maxSpeedForReverse_Mph")
XT_END_TYPE

XT_TYPE(RacingPhysics::CarDynamicsXt::CarDriveTypeXt, "RP_CarDriveType")
	XT_OPTION(RacingPhysics::CarDynamicsXt::DRIVE_TYPE_REARWHEEL, "RearWheel")
	XT_OPTION(RacingPhysics::CarDynamicsXt::DRIVE_TYPE_FRONTWHEEL, "FrontWheel")
	XT_OPTION(RacingPhysics::CarDynamicsXt::DRIVE_TYPE_FOURWHEEL, "FourWheel")
XT_END_TYPE

XT_TYPE(RacingPhysics::CarDynamicsXt::CarDifferentialTypeXt, "RP_CarDifferentialType")
	XT_OPTION(RacingPhysics::CarDynamicsXt::DIFFERENTIAL_LOCKED, "Locked")
	XT_OPTION(RacingPhysics::CarDynamicsXt::DIFFERENTIAL_TORQUE, "Torque")
	XT_OPTION(RacingPhysics::CarDynamicsXt::DIFFERENTIAL_VISCOUS, "Viscous")
XT_END_TYPE

XT_TYPE(RacingPhysics::CarDynamicsXt::CarDifferentialXt, "RP_CarDifferential")
	XT_FIELD(differentialType, "differentialType")
	XT_FIELD(forwardsTorque, "forwardsTorque")
	XT_FIELD(backwardsTorque, "backwardsTorque")
	XT_FIELD(viscosity, "viscosity")
XT_END_TYPE

XT_TYPE(RacingPhysics::CarDynamicsXt::CarTransmissionXt, "RP_CarTransmission")
	XT_FIELD(driveType, "driveType")
	XT_FIELD(frontDifferential, "frontDifferential")
	XT_FIELD(rearDifferential, "rearDifferential")
	XT_FIELD(midDifferential, "midDifferential")
XT_END_TYPE

XT_TYPE(RacingPhysics::CarDynamicsXt::CarFrictionXt, "RP_CarFriction")
	XT_FIELD(airFriction, "airFriction")
	XT_FIELD(rollingFriction, "rollingFriction")
	XT_FIELD(constantDownforce, "constantDownforce")
	XT_FIELD(downforce, "downforce")
	XT_FIELD(downforceOffset, "downforceOffset")
	XT_FIELD(rotationalAirFriction, "rotationalAirFriction")	
XT_END_TYPE

XT_TYPE(RacingPhysics::CarDynamicsXt, "RP_CarDynamics")
	XT_CALLBACK(RacingPhysics::CarDynamicsXtCallback)
	XT_FIELD(name, "name")
	XT_FIELD(mass, "mass")
	XT_FIELD(geometry, "geometry")
	XT_FIELD(suspension, "suspension")
	XT_FIELD(steering, "steering")
	XT_FIELD(brakes, "brakes")
	XT_FIELD(engine, "engine")
	XT_FIELD(gears, "gears")
	XT_FIELD(tyres, "tyres")
	XT_FIELD(transmission, "transmission")
	XT_FIELD(friction, "friction")
XT_END_TYPE

XT_LIBRARY(RacingPhysics_CarDynamics)
	XT_REGISTER_TYPE(RacingPhysics::CarDynamicsXt::CarPhysicsParametersXt)
	XT_REGISTER_TYPE(RacingPhysics::CarDynamicsXt::CarMassXt)
	XT_REGISTER_TYPE(RacingPhysics::CarDynamicsXt::CarGeometryXt)
	XT_REGISTER_TYPE(RacingPhysics::CarDynamicsXt::CarWheelSuspensionXt)
	XT_REGISTER_TYPE(RacingPhysics::CarDynamicsXt::CarSuspensionXt)
	XT_REGISTER_TYPE(RacingPhysics::CarDynamicsXt::CarSteeringXt)
	XT_REGISTER_TYPE(RacingPhysics::CarDynamicsXt::CarBrakesXt)
	XT_REGISTER_TYPE(RacingPhysics::CarDynamicsXt::CarEngineXt)
	XT_REGISTER_TYPE(RacingPhysics::CarDynamicsXt::CarGearsXt)
	XT_REGISTER_TYPE(RacingPhysics::CarDynamicsXt::CarTyreXt)
	XT_REGISTER_TYPE(RacingPhysics::CarDynamicsXt::CarTyresXt)
	XT_REGISTER_TYPE(RacingPhysics::CarDynamicsXt::CarDriveTypeXt)
	XT_REGISTER_TYPE(RacingPhysics::CarDynamicsXt::CarDifferentialTypeXt)
	XT_REGISTER_TYPE(RacingPhysics::CarDynamicsXt::CarDifferentialXt)
	XT_REGISTER_TYPE(RacingPhysics::CarDynamicsXt::CarTransmissionXt)
	XT_REGISTER_TYPE(RacingPhysics::CarDynamicsXt::CarFrictionXt)
	XT_REGISTER_TYPE(RacingPhysics::CarDynamicsXt)
XT_END_LIBRARY

#endif