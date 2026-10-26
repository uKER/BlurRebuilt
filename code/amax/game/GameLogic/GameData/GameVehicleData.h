#ifndef GAME_VEHICLEDATA_H
#define GAME_VEHICLEDATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/GameData/GameDataDefs.h>

#include <RacingPhysics/Vehicles/Car/CarSettings.h>
#include <AI_AmaxData.h>
#include <Ui\game\Multiplayer\MultiplayerDefines.h>

#include <Vehicle/Db/VehicleXt.h>

namespace Kanzen
{
	struct HollywoodVehicleParams;
}

namespace SpecialFX
{
	class C_SpecialFX;
}
//foward declarations
struct AmaxExhaustEmitter;
struct AmaxVehicleParticleEmitter;
struct AmaxVehicleParticleEmitterDesc;
struct AmaxVehicleParticleEffects;
struct AmaxPaintResprays;

namespace Audio
{
	struct XtEntityProxy;
}

namespace AmaxCam
{
	class C_VehicleFeedParam;
}

struct AmaxManufacturer : AmaxGameDataObj
{
	XtSz8 mUiTexture;
	bool	mUiSquareTexture;
};

enum AmaxVehicleWeight
{
	AmaxVehicleWeight_Light,
	AmaxVehicleWeight_Medium,
	AmaxVehicleWeight_Heavy,
	AmaxVehicleWeight_Ultra,
};

enum AmaxVehicleStyleType
{
	AmaxVehicleStyleType_Stock,
	AmaxVehicleStyleType_Smooth,
	AmaxVehicleStyleType_Drift,
	AmaxVehicleStyleType_Tuner,
	AmaxVehicleStyleType_Race,
	AmaxVehicleStyleType_Rat,
	AmaxVehicleStyleType_OffRoad,
	AmaxVehicleStyleType_Drag,
};

struct AmaxVehicleStyle : AmaxGameDataObj
{
	AmaxVehicleStyleType style;
};

enum AmaxWheelType
{	
	AmaxWheelType_Standard,
	AmaxWheelType_OffRoad,
	AmaxWheelType_HotRod,
	AmaxWheelType_Classic,	
};

enum AmaxCarClass
{
	AmaxCarClass_A = 0,
	AmaxCarClass_B,
	AmaxCarClass_C,
	AmaxCarClass_D,
	AmaxCarClass_E,
	AmaxCarClass_ANY,
	AmaxCarClass_Count = AmaxCarClass_ANY,
};

enum AmaxCarHealthGroup
{
	AmaxCarHealthGroup_Low,
	AmaxCarHealthGroup_Medium,
	AmaxCarHealthGroup_High,
	AmaxCarHealthGroup_VeryHigh,
};

enum AmaxCarUiStyle
{
	AmaxCarUiStyle_Balanced,
	AmaxCarUiStyle_Drifty,
	AmaxCarUiStyle_VeryDrifty,
	AmaxCarUiStyle_Grippy,
	AmaxCarUiStyle_VeryGrippy,
	AmaxCarUiStyle_OffRoad,
};

struct AmaxWheelModel
{
	XtSz8 fileName;
	AmaxWheelType wheelType;
};

struct AmaxFlightControl
{
	float dampingTime;
	float torque;
	bool increaseForceOverTime;

	float inFlightDownForce;
	float inFlightDownForceStartTime;
	float inFlightDownForceDuration;

	float groundedDownForce;
	float groundedDownForceDuration;
};

struct AmaxMirror
{
	XtF32									fieldOfView;
	XtVec3									offset;
};

struct AmaxVehicleDefaults : AmaxGameDataObj
{
	XtHandle<Vehicle::XtMaterialDefaults>	materials;
	AmaxFlightControl						flightControl;
	AmaxMirror								mirror;
	AmaxCam::C_VehicleFeedParam*			defaultCamera;
	XtHandle<Kanzen::HollywoodVehicleParams>		hollywoodCamDefaults;
	XtHandle<Kanzen::HollywoodVehicleParams>		hollywoodCamMotorMash;
	XtHandle<SpecialFX::C_SpecialFX>				mHighSpeedScreenEffect;
	XtHandle<SpecialFX::C_SpecialFX>				mRoughTerrainScreenEffect;
	XtHandle<SpecialFX::C_SpecialFX>				mWreckedSpecialEffect;

	F32 defaultAirFrictionCoefficient;
};

class AmaxVehicleCameras
{
public:
	XtVec3 bonnet;
	XtF32 bonnetFov;
	XtVec3 bumper;
	XtF32 bumperFov;
	XtF32 chaseHeightOverride;
};

namespace AmaxCam
{
	class C_VehicleFeedParam;
}

//#define AMAX_VEH_USE_MI

struct AmaxVehicle : AmaxGameDataObj
#ifdef AMAX_VEH_USE_MI
	, Vehicle::XtVehicle
#endif
{
#ifndef AMAX_VEH_USE_MI
	Vehicle::XtVehicle baseVehicle;
#endif
	XtLocId locStringFull;
	bool use;
	XtHandle<AmaxVehicleStyle> style;
	XtHandle<AmaxManufacturer> manufacturer;
	XtHandle<AmaxWheelModel> wheelModel;
	float wheelScaleFront;
	float wheelScaleRear;
	float wheelDepthFront;
	float wheelDepthRear;
	float frontWheelMaxTravel;
	float rearWheelMaxTravel;
	float rearAxlePivotScale;
	XtHandle<RacingPhysics::CarDynamicsXt> dynamics;
	XtHandle<AISteeringXt> aiSteering;
	bool UseAIDynamics;
	XtHandle<RacingPhysics::CarDynamicsXt> aiDynamics;
		
	// Vehicle-specific animation settings.
	XtVec3 driverOffset;
	XtVec3 steeringWheelPosition;
	F32 steeringWheelTilt;
	F32 steeringWheelRadius;
	F32 cabinFloorOffset;
	F32 cabinFootwellOffset;
	bool rightHandDrive;
	bool enableSteeringWheelIk;
	bool cannotRestArmOnDoor;		// Typically for cars with narrow or obstructed windows.
	bool cannotPutArmThroughWindow;	// Cars with even smaller windows.
	bool forceNeutralDrivingStyle;	// To avoid problems where elbows may poke through door in the aggressive driving styles.
	
	XtU16 contentPack; // 0 = shipped with game otherwise dlc pack.
	AmaxCarClass carClass;

	//lots of audio defs
	XtHandle<Audio::XtEntityProxy>  vehicleAudioDef;
	XtHandle<Audio::XtEntityProxy>  aiVehicleAudioDef; //stripped down version
	XtHandle<Audio::XtEntityProxy>  aiVehicleAudioDef_PS3; //uber stripped down version
	XtHandle<Audio::XtEntityProxy>  aiBossBattleVehicleAudioDef_PS3; //uber stripped down version
	XtHandle<Audio::XtEntityProxy>  dryWheelAudioDef;
	XtHandle<Audio::XtEntityProxy>  wetWheelAudioDef;
	XtHandle<Audio::XtEntityProxy>  aiDryWheelAudioDef;
	XtHandle<Audio::XtEntityProxy>  aiWetWheelAudioDef;

	XtHandle<Audio::XtEntityProxy>  sharedAudioDef;
	XtHandle<Audio::XtEntityProxy>  aiSharedAudioDef;

	F32 audioRPMLimit;
	F32 audioGarageID;
	bool audioForcedInduction;
	F32 audioExhaustType;
	F32 audioEngineType;

	F32 m_AirFrictionCoefficient;
	bool overwriteAirFrictionCoefficient;

	// Gameplay parameters
	AmaxCarHealthGroup	vehicleHealthGroup;
	UI::E_MultiplayerVehicleAbilities bossAbilitySlot;
	AmaxVehicleWeight	weightClass;
	float surfaceRollingFrictionScale;
		
	XtArray<AmaxExhaustEmitter> exhaustEmitters;
	XtHandle<AmaxVehicleParticleEffects> particleEffects;
	bool do_not_show_in_game;
	
	AmaxCam::C_VehicleFeedParam* m_CameraParam;

	XtSz8			m_UiIconName;
	S32				m_sUiStability;
	S32				m_sUiDifficulty;
	S32				m_sUiDrift;
	S32				m_sUiStrength;
	S32				m_sUiGrip;
	S32				m_sUiSpeed;
	S32				m_sUiAcceleration;
	S32				m_sUiMaxSpeed;
	bool			m_bUICanReSpray;
	AmaxCarUiStyle	m_eUiCarStyle;

	U16				mGameplayTopSpeed;

	XtVec3 m_ShieldSizeModifier;

	XtHandle<Kanzen::HollywoodVehicleParams> m_HollywoodVehicleParams;
	XtHandle<Kanzen::HollywoodVehicleParams> m_HollywoodVehicleParams2;

	XtHandle<AmaxVehicle> m_ParentVehicle;
	AmaxVehicle()
	{
		m_CameraParam = 0;
	}
};

class C_GameVehicleData
{
public:
	void Init();
	void Update();

	static C_GameVehicleData& Instance() {return s_instance;}

	AmaxGameData<AmaxManufacturer*> mManufacturers;
	AmaxGameData<AmaxVehicleStyle*> mVehicleStyle;
	AmaxGameData<AmaxVehicle*> mVehicles;
	AmaxGameData<AmaxVehicleDefaults*> mVehicleDefaults;

	XtPack*	mXtAmaxPack;

private:
	static C_GameVehicleData s_instance;
};

#endif // GAME_VEHICLEDATA_H
