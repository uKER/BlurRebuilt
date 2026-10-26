#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/GameData/GameVehicleData.h>
#include <../library/kanzen/camera/HollywoodVehicleCamera.h>

C_GameVehicleData C_GameVehicleData::s_instance;

//Debug::Bool g_debugCheckForSorceryUpdatesOnDisk("Game.CheckForSorceryDataExportsWhileRunning", false, Debug::Persist());

#define AMAX_GAMEVEHICLEDATA_CALLBACK(Type, Member) \
	class Callback##Type { public: AmaxGameData<Type*>* operator()() { return &C_GameVehicleData::Instance().Member; } };

AMAX_GAMEVEHICLEDATA_CALLBACK(AmaxManufacturer, mManufacturers)
XT_TYPE(AmaxManufacturer,"AmaxManufacturer")
	XT_CALLBACK((GameDataHandler<AmaxManufacturer,CallbackAmaxManufacturer>))
	XT_BASE(AmaxGameDataObj)
	XT_FIELD(mUiTexture,"UiTexture")
	XT_FIELD(mUiSquareTexture,"UiSquareTexture")
XT_END_TYPE

AMAX_GAMEVEHICLEDATA_CALLBACK(AmaxVehicleStyle, mVehicleStyle)
XT_TYPE(AmaxVehicleStyle,"AmaxVehicleStyle")
	XT_CALLBACK((GameDataHandler<AmaxVehicleStyle,CallbackAmaxVehicleStyle>))
	XT_BASE(AmaxGameDataObj)
	XT_FIELD(style,"style")
XT_END_TYPE

XT_TYPE(AmaxVehicleWeight,"AmaxVehicleWeight")
	XT_OPTION(AmaxVehicleWeight_Light,"Light")
	XT_OPTION(AmaxVehicleWeight_Medium,"Medium")
	XT_OPTION(AmaxVehicleWeight_Heavy,"Heavy")
XT_END_TYPE

XT_TYPE(AmaxWheelType,"AmaxWheelType")
	XT_OPTION(AmaxWheelType_Standard,"Standard")
	XT_OPTION(AmaxWheelType_OffRoad,"OffRoad")
	XT_OPTION(AmaxWheelType_HotRod,"HotRod")
	XT_OPTION(AmaxWheelType_Classic,"Classic")
XT_END_TYPE

XT_TYPE(AmaxCarClass,"AmaxCarClass")
	XT_OPTION(AmaxCarClass_A,"A")
	XT_OPTION(AmaxCarClass_B,"B")
	XT_OPTION(AmaxCarClass_C,"C")
	XT_OPTION(AmaxCarClass_D,"D")
	XT_OPTION(AmaxCarClass_E,"E")
	XT_OPTION(AmaxCarClass_ANY,"ANY")
XT_END_TYPE

XT_TYPE(AmaxCarHealthGroup, "AmaxCarHealthGroup")
	XT_OPTION(AmaxCarHealthGroup_Low,		"Low")
	XT_OPTION(AmaxCarHealthGroup_Medium,	"Medium")
	XT_OPTION(AmaxCarHealthGroup_High,		"High")
	XT_OPTION(AmaxCarHealthGroup_VeryHigh,	"VeryHigh")
XT_END_TYPE

XT_TYPE(AmaxCarUiStyle,"AmaxCarUiStyle")
	XT_OPTION(AmaxCarUiStyle_Balanced,"Balanced")
	XT_OPTION(AmaxCarUiStyle_Drifty,"Drifty")
	XT_OPTION(AmaxCarUiStyle_VeryDrifty,"VeryDrifty")
	XT_OPTION(AmaxCarUiStyle_Grippy,"Grippy")
	XT_OPTION(AmaxCarUiStyle_VeryGrippy,"VeryGrippy")
	XT_OPTION(AmaxCarUiStyle_OffRoad,"OffRoad")
XT_END_TYPE

XT_TYPE(AmaxVehicleStyleType,"AmaxVehicleStyleType")
	XT_OPTION(AmaxVehicleStyleType_Stock,"Stock")
	XT_OPTION(AmaxVehicleStyleType_Smooth,"Smooth")
	XT_OPTION(AmaxVehicleStyleType_Drift,"Drift")
	XT_OPTION(AmaxVehicleStyleType_Tuner,"Tuner")
	XT_OPTION(AmaxVehicleStyleType_Race,"Race")
	XT_OPTION(AmaxVehicleStyleType_Rat,"Rat")
	XT_OPTION(AmaxVehicleStyleType_OffRoad,"OffRoad")
	XT_OPTION(AmaxVehicleStyleType_Drag,"Drag")	
XT_END_TYPE

XT_TYPE(AmaxWheelModel,"AmaxWheelModel")
	XT_FIELD(fileName,"fileName")
	XT_FIELD(wheelType,"wheelType")
XT_END_TYPE

AMAX_GAMEVEHICLEDATA_CALLBACK(AmaxVehicle, mVehicles)
XT_TYPE(AmaxVehicle,"AmaxVehicle")
	XT_CALLBACK((GameDataHandler<AmaxVehicle,CallbackAmaxVehicle>))
	XT_BASE(AmaxGameDataObj)
#ifdef AMAX_VEH_USE_MI
	XT_BASE(Vehicle::XtVehicle)
#else
	XT_FIELD(baseVehicle,"baseVehicle")
#endif
	XT_FIELD(locStringFull,"locStringFull")
	XT_FIELD(use,"use")
	XT_FIELD(contentPack,"ContentPack")
	XT_FIELD(carClass,"class")
	XT_FIELD(style,"style")
	XT_FIELD(manufacturer,"Manufacturer")
	XT_FIELD(wheelModel,"wheelModel")
	XT_FIELD(wheelScaleFront,"wheelScaleFront")
	XT_FIELD(wheelScaleRear,"wheelScaleRear")
	XT_FIELD(wheelDepthFront,"wheelDepthFront")
	XT_FIELD(wheelDepthRear,"wheelDepthRear")	
	XT_FIELD(frontWheelMaxTravel,"frontWheelMaxTravel")	
	XT_FIELD(rearWheelMaxTravel,"rearWheelMaxTravel")	
	XT_FIELD(rearAxlePivotScale,"rearAxlePivotScale")		
	XT_FIELD(dynamics,"dynamics")
	XT_FIELD(aiSteering,"aiSteering")
	XT_FIELD(UseAIDynamics,"UseAiDynamics")
	XT_FIELD(aiDynamics, "aiDynamics")
	XT_FIELD(driverOffset, "DriverOffset")
	XT_FIELD(steeringWheelPosition, "SteeringWheelPosition")
	XT_FIELD(steeringWheelTilt, "SteeringWheelTilt")
	XT_FIELD(steeringWheelRadius, "SteeringWheelRadius")
	XT_FIELD(cabinFloorOffset, "CabinFloorOffset")
	XT_FIELD(cabinFootwellOffset, "CabinFootwellOffset")
	XT_FIELD(rightHandDrive, "RightHandDrive")
	XT_FIELD(cannotRestArmOnDoor, "CannotRestArmOnDoor")
	XT_FIELD(cannotPutArmThroughWindow, "CannotPutArmThroughWindow")
	XT_FIELD(forceNeutralDrivingStyle, "ForceNeutralDrivingStyle")
	XT_FIELD(enableSteeringWheelIk, "EnableSteeringWheelIK")
	XT_FIELD(vehicleAudioDef,"vehicleAudioEntity")
	XT_FIELD(aiVehicleAudioDef,"aiVehicleAudioEntity")
	XT_FIELD(aiVehicleAudioDef_PS3,"aiVehicleAudioEntity_PS3")
	XT_FIELD(aiBossBattleVehicleAudioDef_PS3,"aiBossBattleVehicleAudioEntity_PS3")
	XT_FIELD(dryWheelAudioDef,"dryWheelAudioEntity")
	XT_FIELD(wetWheelAudioDef,"wetWheelAudioEntity")
	XT_FIELD(aiDryWheelAudioDef,"aiDryWheelAudioEntity")
	XT_FIELD(aiWetWheelAudioDef,"aiWetWheelAudioEntity")
	XT_FIELD(audioRPMLimit,"AudioRPMLimit")
	XT_FIELD(audioGarageID,"AudioGarageID")
	XT_FIELD(audioForcedInduction,"AudioForcedInduction")
	XT_FIELD(audioEngineType,"AudioEngineType")
	XT_FIELD(audioExhaustType,"AudioExhaustType")
	XT_FIELD(do_not_show_in_game,"DoNotShowInGame")
	XT_FIELD(vehicleHealthGroup,"HealthGroup")
	XT_FIELD(bossAbilitySlot,"BossAbilitySlot")
	XT_FIELD(weightClass,"weightClass")
	XT_FIELD(exhaustEmitters,"exhaustEmitters")
	XT_FIELD(particleEffects,"particleEffects")	
	XT_FIELD(m_HollywoodVehicleParams, "hollywoodVehicleParams")
	XT_FIELD(m_HollywoodVehicleParams2, "hollywoodVehicleParams2")
	XT_FIELD(m_CameraParam, "CameraParam")
	XT_FIELD(m_UiIconName, "UiIconName")
	XT_FIELD(m_sUiStability, "UiStability")
	XT_FIELD(m_sUiDifficulty, "UiDifficulty")
	XT_FIELD(m_sUiDrift, "UiDrift")
	XT_FIELD(m_sUiStrength, "UiStrength")
	XT_FIELD(m_sUiGrip, "UiGrip")
	XT_FIELD(m_sUiSpeed, "UiSpeed")
	XT_FIELD(m_sUiAcceleration, "UiAcceleration")
	XT_FIELD(m_sUiMaxSpeed, "UiMaxSpeed")
	XT_FIELD(m_bUICanReSpray, "UICanReSpray")
	XT_FIELD(m_eUiCarStyle, "UiCarStyle")
	XT_FIELD(m_ShieldSizeModifier, "ShieldSizeModifier")
	XT_FIELD(sharedAudioDef,"sharedAudioEntity")
	XT_FIELD(aiSharedAudioDef,"aiSharedAudioEntity")
	XT_FIELD(m_AirFrictionCoefficient, "AirFrictionCoefficient")
	XT_FIELD(overwriteAirFrictionCoefficient, "OverwriteAirFrictionCoefficient")
	XT_FIELD(mGameplayTopSpeed, "GameplayTopSpeed")
	XT_FIELD(surfaceRollingFrictionScale,"surfaceRollingFrictionScale")
	XT_FIELD(m_ParentVehicle, "parentVehicle")
XT_END_TYPE

XT_TYPE(AmaxVehicleCameras, "AmaxVehicleCameras")
	XT_FIELD(bonnet, "bonnet")
	XT_FIELD(bonnetFov, "bonnetFov")
	XT_FIELD(bumper, "bumper")
	XT_FIELD(bumperFov, "bumperFov")
	XT_FIELD(chaseHeightOverride, "chaseHeightOverride")
XT_END_TYPE

XT_TYPE(AmaxFlightControl,"AmaxFlightControl")
	XT_FIELD(dampingTime,"dampingTime")
	XT_FIELD(torque,"torque")
	XT_FIELD(increaseForceOverTime,"increaseForceOverTime")

	XT_FIELD(inFlightDownForce,"inFlightDownForce")
	XT_FIELD(inFlightDownForceStartTime,"inFlightDownForceStartTime")
	XT_FIELD(inFlightDownForceDuration,"inFlightDownForceDuration")

	XT_FIELD(groundedDownForce,"groundedDownForce")
	XT_FIELD(groundedDownForceDuration,"groundedDownForceDuration")
XT_END_TYPE

XT_TYPE(AmaxMirror,"AmaxMirror")
	XT_FIELD(fieldOfView,"fieldOfView")
	XT_FIELD(offset,"offset")
XT_END_TYPE

AMAX_GAMEVEHICLEDATA_CALLBACK(AmaxVehicleDefaults, mVehicleDefaults)
XT_TYPE(AmaxVehicleDefaults,"AmaxVehicleDefaults")
	XT_CALLBACK((GameDataHandler<AmaxVehicleDefaults,CallbackAmaxVehicleDefaults>))
	XT_BASE(AmaxGameDataObj)
	XT_FIELD(materials,"materials")
	XT_FIELD(flightControl,"flightControl")
	XT_FIELD(mirror,"mirror")
	XT_FIELD(defaultCamera,"defaultCamera")
	XT_FIELD(hollywoodCamDefaults,"hollywoodCamDefaults")
	XT_FIELD(hollywoodCamMotorMash,"hollywoodCamMotorMash")
	XT_FIELD(mHighSpeedScreenEffect,"highSpeedEffect")
	XT_FIELD(mRoughTerrainScreenEffect,"roughTerrainEffect")
	XT_FIELD(mWreckedSpecialEffect,"WreckedEffect")
	XT_FIELD(defaultAirFrictionCoefficient, "defaultAirFrictionCoefficient")
XT_END_TYPE



XT_TYPE(Kanzen::HollywoodVehicleParams,"HollywoodVehicleParams")
	XT_FIELD(NormalFOV,"NormalFOV")	

	XT_FIELD(IdealDistanceNormal,"IdealDistanceNormal")
	XT_FIELD(IdealDistanceDampingNormal,"IdealDistanceDampingNormal")
	XT_FIELD(IdealDistanceAtFullAcceleration,"IdealDistanceAtFullAcceleration")
	XT_FIELD(IdealDistanceAtFullDeceleration,"IdealDistanceAtFullDeceleration")
	XT_FIELD(IdealDistanceAirbourne,"IdealDistanceAirbourne")
	XT_FIELD(IdealDistanceSmoothing,"IdealDistanceSmoothing")
	XT_FIELD(IdealDistanceSmoothingAccelerating,"IdealDistanceSmoothingAccelerating")
	XT_FIELD(IdealDistanceSmoothingDecelerating,"IdealDistanceSmoothingDecelerating")
	XT_FIELD(IdealDistanceSmoothingAirbourne,"IdealDistanceSmoothingAirbourne")
	XT_FIELD(MaxAcceleration,"MaxAcceleration")
	XT_FIELD(MaxDeceleration,"MaxDeceleration")

	XT_FIELD(IdealPitchAirbourneUp,"IdealPitchAirbourneUp")
	XT_FIELD(IdealPitchAirbourneDown,"IdealPitchAirbourneDown")
	XT_FIELD(IdealPitch,"IdealPitch")
	XT_FIELD(IdealBase,"IdealBase")
		
	XT_FIELD(PitchDampingNormal,"PitchDampingNormal")
	XT_FIELD(PitchDampingAirbourneUp,"PitchDampingAirbourneUp")
	XT_FIELD(PitchDampingAirbourneDown,"PitchDampingAirbourneDown")
	XT_FIELD(MaxUpwardsVelocity,"MaxUpwardsVelocity")	
	XT_FIELD(MaxDownwardsVelocity,"MaxDownwardsVelocity")


	XT_FIELD(YawDampingNormal,"YawDampingNormal")
	XT_FIELD(YawDampingSmoothValue,"YawDampingSmoothValue")
	XT_FIELD(CameraCarRollDiffMax,"CameraCarRollDiffMax")
	XT_FIELD(LostControlRollSmoothing,"LostControlRollSmoothing")
	XT_FIELD(UprightRollSmoothing,"UprightRollSmoothing")
	XT_FIELD(BarrelRollSmoothing,"BarrelRollSmoothing")
	XT_FIELD(PitchMaxSpeedNormal,"PitchMaxSpeedNormal")
	XT_FIELD(PitchMaxErrorNormal,"PitchMaxErrorNormal")

	XT_FIELD(YawMaxSpeedNormal,"YawMaxSpeedNormal")
	XT_FIELD(YawMaxErrorNormal,"YawMaxErrorNormal")

	XT_FIELD(YawRollFwdsVelThreshold,"YawRollFwdsVelThreshold")
	XT_FIELD(YawRollMinYaw,"YawRollMinYaw")
	XT_FIELD(YawRollMaxYaw,"YawRollMaxYaw")
	XT_FIELD(YawRollMax,"YawRollMax")


	XT_FIELD(LookSideMax,"LookSideMax")
	XT_FIELD(LookSideUpDownMultiplier,"LookSideUpDownMultiplier")
	XT_FIELD(ForwardsVelocityUpDownMultiplier,"ForwardsVelocityUpDownMultiplier")
	XT_FIELD(LookAtSmoothingValue,"LookAtSmoothingValue")
	XT_FIELD(LookAtMaxAngVel,"LookAtMaxAngVel")
	XT_FIELD(LookAtMinAngVel,"LookAtMinAngVel")
	XT_FIELD(LookAtPos,"LookAtPos")

	XT_FIELD(WheelNoContactThreshold,"WheelNoContactThreshold")
	XT_FIELD(UpsideDownDotProductThreshold,"UpsideDownDotProductThreshold")
	XT_FIELD(RegainedControlAngVelThreshold,"RegainedControlAngVelThreshold")
	XT_FIELD(RegainedControlThrottleThreshold,"RegainedControlThrottleThreshold")
	XT_FIELD(TargetTransformSmooth,"TargetTransformSmooth")
 
	XT_FIELD(RotationalUpperThreshold,"RotationalUpperThreshold")
	XT_FIELD(RotationalLowerThreshold,"RotationalLowerThreshold")
	XT_FIELD(BurnoutTimerThreshold,"BurnoutTimerThreshold")

	XT_FIELD(HighSpeedLowerVelThreshold,"HighSpeedLowerVelThreshold")
	XT_FIELD(HighSpeedUpperVelThreshold,"HighSpeedUpperVelThreshold")
	XT_FIELD(HighSpeedFovMultiplier,"HighSpeedFovMultiplier")

XT_END_TYPE

XT_LIBRARY(GAMEVEHICLEDATA)
	XT_REGISTER_TYPE(AmaxVehicleWeight)
	XT_REGISTER_TYPE(AmaxManufacturer)
	XT_REGISTER_TYPE(AmaxWheelType)
	XT_REGISTER_TYPE(AmaxCarClass)
	XT_REGISTER_TYPE(AmaxCarUiStyle)
	XT_REGISTER_TYPE(AmaxCarHealthGroup)
	XT_REGISTER_TYPE(AmaxWheelModel)
	XT_REGISTER_TYPE(AmaxVehicle)
	XT_REGISTER_TYPE(Kanzen::HollywoodVehicleParams)
	XT_REGISTER_TYPE(AmaxVehicleStyle)
	XT_REGISTER_TYPE(AmaxVehicleStyleType)
	XT_REGISTER_TYPE(AmaxVehicleDefaults)
	XT_REGISTER_TYPE(AmaxVehicleCameras)
	XT_REGISTER_TYPE(AmaxFlightControl)
	XT_REGISTER_TYPE(AmaxMirror)
XT_END_LIBRARY

void C_GameVehicleData::Init()
{
	//this needs to move somewhere nice
	mManufacturers.AlphaSort();
	mVehicles.AlphaSort();

	//Debug Code to print out the Sorcery IDs for Agora
	//printf("Vehicles\n");
	//for(int a=0;a<mVehicles.Number();a++)
	//{
	//	if(mVehicles[a]->style->style == AmaxVehicleStyleType_Drag)
	//		printf("ID %x = %S %S (Drag)\n",mVehicles[a]->ID,UI::C_TextStringID(mVehicles[a]->manufacturer->loc_string.id).String(),UI::C_TextStringID(mVehicles[a]->loc_string.id).String());

	//	if(mVehicles[a]->style->style == AmaxVehicleStyleType_Drift)
	//		printf("ID %x = %S %S (Drift)\n",mVehicles[a]->ID,UI::C_TextStringID(mVehicles[a]->manufacturer->loc_string.id).String(),UI::C_TextStringID(mVehicles[a]->loc_string.id).String());

	//	if(mVehicles[a]->style->style == AmaxVehicleStyleType_OffRoad)
	//		printf("ID %x = %S %S (OffRoad)\n",mVehicles[a]->ID,UI::C_TextStringID(mVehicles[a]->manufacturer->loc_string.id).String(),UI::C_TextStringID(mVehicles[a]->loc_string.id).String());

	//	if(mVehicles[a]->style->style == AmaxVehicleStyleType_Race)
	//		printf("ID %x = %S %S (Race)\n",mVehicles[a]->ID,UI::C_TextStringID(mVehicles[a]->manufacturer->loc_string.id).String(),UI::C_TextStringID(mVehicles[a]->loc_string.id).String());

	//	if(mVehicles[a]->style->style == AmaxVehicleStyleType_Rat)
	//		printf("ID %x = %S %S (Rat)\n",mVehicles[a]->ID,UI::C_TextStringID(mVehicles[a]->manufacturer->loc_string.id).String(),UI::C_TextStringID(mVehicles[a]->loc_string.id).String());

	//	if(mVehicles[a]->style->style == AmaxVehicleStyleType_Smooth)
	//		printf("ID %x = %S %S (Smooth)\n",mVehicles[a]->ID,UI::C_TextStringID(mVehicles[a]->manufacturer->loc_string.id).String(),UI::C_TextStringID(mVehicles[a]->loc_string.id).String());

	//	if(mVehicles[a]->style->style == AmaxVehicleStyleType_Stock)
	//		printf("ID %x = %S %S (Stock)\n",mVehicles[a]->ID,UI::C_TextStringID(mVehicles[a]->manufacturer->loc_string.id).String(),UI::C_TextStringID(mVehicles[a]->loc_string.id).String());

	//	if(mVehicles[a]->style->style == AmaxVehicleStyleType_Tuner)
	//		printf("ID %x = %S %S (Tuner)\n",mVehicles[a]->ID,UI::C_TextStringID(mVehicles[a]->manufacturer->loc_string.id).String(),UI::C_TextStringID(mVehicles[a]->loc_string.id).String());
	//}
	
	//printf("Manufacturers\n");
	//for(int a=0;a<mManufacturers.Number();a++)
	//{
	//	printf("ID %x = %S\n",mManufacturers[mManufacturers.mSortedData[a]]->ID,UI::C_TextStringID(mManufacturers[mManufacturers.mSortedData[a]]->loc_string.id).String());
	//}
}


