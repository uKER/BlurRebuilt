#include "stdafx.h"

#include "system/automation/Automation.h"

#include <Game/GameActors/Vehicles/VehicleActor.h>

#include <game_resource/ModelData.h>

#include <Vehicle/VehicleModel.h>
#include <Vehicle/VehicleModelManager.h>
#include <Vehicle/Setup.h>
#include <Vehicle/Input.h>
#include <Vehicle/CodeProcessor.h>
#include <Vehicle/Damage/DamageLayer.h>
#include <Vehicle/Db/VehicleXt.h>
#include <Vehicle/Material/Material.h>

#include <RacingPhysics/RacingPhysics.h>

#include <gravity/sdk/ConstraintDescriptors.h>
#include <gravity/sdk/KinematicController.h>
#include <Gravity/Triggers/TriggerSystem.h>
#include <gravity/triggers/TriggerEvent.h>

#include "gravity/narrowphase/shapes.h"

#include <Core/XtStdX.h>

//#undef optimize
//#pragma optimize("", off)

DEBUG_ENUM(Game::Actors::E_FakeVehiclePhysicsEffect)
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_NONE,				"None")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_SPIN_360_LEFT,		"Spin 360 Left")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_SPIN_360_RIGHT,		"Spin 360 Right")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_SPIN_45_LEFT,		"Spin 45 Left")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_SPIN_45_RIGHT,		"Spin 45 Right")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_SPIN_45_LEFT_WEAK,	"Spin 45 Left Weak")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_SPIN_45_RIGHT_WEAK,	"Spin 45 Right Weak")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_SPIN_45_LEFT_MINIMINE,	"Spin 45 Left MiniMine")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_SPIN_45_RIGHT_MINIMINE,	"Spin 45 Right MiniMine")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_FLIP_FORWARD,		"Flip Forward")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_FLIP_BACKWARD,		"Flip Backward")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_ROLL_LEFT,			"Roll Left")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_ROLL_RIGHT,			"Roll Right")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_JINK_BARGE_LEFT,		"Barge Jink Left")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_JINK_BARGE_RIGHT,	"Barge Jink Right")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_JINK_BARGE_MINOR_LEFT,	"Barge Jink Minor Left")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_JINK_BARGE_MINOR_RIGHT,	"Barge Jink Minor Right")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_JINK_SHIELD_LEFT,	"Shield Jink Left")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_JINK_SHIELD_RIGHT,	"Shield Jink Right")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_JINK_NITROUS_LEFT,	"Nitrous Jink Left")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_JINK_NITROUS_RIGHT,	"Nitrous Jink Right")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_TIP_FORWARD,			"Tip Forward")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_TIP_BACKWARD,		"Tip Backward")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_BATTERING_RAM_HEAVY_LEFT,	"Battering Ram Heavy Left")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_BATTERING_RAM_HEAVY_RIGHT,	"Battering Ram Heavy Right")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_BATTERING_RAM_LIGHT_LEFT,	"Battering Ram Light Left")
	DE_OPTION(Game::Actors::E_FAKE_PHYSICS_BATTERING_RAM_LIGHT_RIGHT,	"Battering Ram Light Right")
END_DEBUG_ENUM

Debug::Vector g_Perks_FakePhysics_DefaultConstantKickForce("Perks.FakePhysics.Default.ConstantKickForce", 0.f, 0.f, 0.f);
Debug::Vector g_Perks_FakePhysics_DefaultConstantKickPos("Perks.FakePhysics.Default.ConstantKickPos", 0.f, 0.f, 0.f);

Debug::Float g_Perks_FakePhysics_Scaling_X_MaxSpeed("Perks.FakePhysics.Scaling.XMaxSpeed", 250.0f);
Debug::Float g_Perks_FakePhysics_Scaling_X_ScaleMin("Perks.FakePhysics.Scaling.XScaleMin", 0.7f);
Debug::Float g_Perks_FakePhysics_Scaling_X_ScaleMax("Perks.FakePhysics.Scaling.XScaleMax", 1.5f);

Debug::Float g_Perks_FakePhysics_Scaling_Z_MinSpeed("Perks.FakePhysics.Scaling.ZMinSpeed", 30.0f);
Debug::Float g_Perks_FakePhysics_Scaling_Z_MaxSpeed("Perks.FakePhysics.Scaling.ZMaxSpeed", 100.0f);

Debug::Float g_Perks_FakePhysics_Spin360Throttle("Perks.FakePhysics.Spin360.Throttle", 2.9f);
Debug::Float g_Perks_FakePhysics_Spin360Force("Perks.FakePhysics.Spin360.Force", 2.05f);
Debug::Float g_Perks_FakePhysics_Spin360ForceRand("Perks.FakePhysics.Spin360.ForceRand", 0.05f);
Debug::Vector g_Perks_FakePhysics_Spin360ImpulseScale("Perks.FakePhysics.Spin360.ImpulseScale", 0.0f, 0.0f, 1.0f);
Debug::Vector g_Perks_FakePhysics_Spin360ImpulseDamp("Perks.FakePhysics.Spin360.ImpulseDamp", 0.9f, 0.9f, 0.9f);
Debug::Vector g_Perks_FakePhysics_Spin360KickForce("Perks.FakePhysics.Spin360.KickForce", 0.0f, 3.25f, 0.0f);
Debug::Vector g_Perks_FakePhysics_Spin360KickPos("Perks.FakePhysics.Spin360.KickPos", 1.0f, 0.0f, 3.0f);

Debug::Float g_Perks_FakePhysics_Spin45Throttle("Perks.FakePhysics.Spin45.Throttle", 5.0f);
Debug::Float g_Perks_FakePhysics_Spin45Force("Perks.FakePhysics.Spin45.Force", 0.25f);
Debug::Float g_Perks_FakePhysics_Spin45ForceRand("Perks.FakePhysics.Spin45.ForceRand", 0.05f);
Debug::Vector g_Perks_FakePhysics_Spin45ImpulseScale("Perks.FakePhysics.Spin45.ImpulseScale", 0.0f, 0.0f, 1.0f);
Debug::Vector g_Perks_FakePhysics_Spin45ImpulseDamp("Perks.FakePhysics.Spin45.ImpulseDamp", 1.0f, 0.2f, 1.0f);
Debug::Vector g_Perks_FakePhysics_Spin45KickForce("Perks.FakePhysics.Spin45.KickForce", -1.5f, 2.0f, -7.5f);
Debug::Vector g_Perks_FakePhysics_Spin45KickForceWeak("Perks.FakePhysics.Spin45.KickForceWeak", -3.5f, 0.5f, -3.5f);
Debug::Vector g_Perks_FakePhysics_Spin45KickPos("Perks.FakePhysics.Spin45.KickPos", 0.5f, 0.0f, -3.0f);

Debug::Float g_Perks_FakePhysics_Spin45ThrottleMiniMine("Perks.FakePhysics.Spin45MiniMine.Throttle", 5.0f);
Debug::Float g_Perks_FakePhysics_Spin45ForceMiniMine("Perks.FakePhysics.Spin45MiniMine.Force", 0.25f);
Debug::Float g_Perks_FakePhysics_Spin45ForceRandMiniMine("Perks.FakePhysics.Spin45MiniMine.ForceRand", 0.05f);
Debug::Vector g_Perks_FakePhysics_Spin45ImpulseScaleMiniMine("Perks.FakePhysics.Spin45MiniMine.ImpulseScale", 0.0f, 0.0f, 1.0f);
Debug::Vector g_Perks_FakePhysics_Spin45ImpulseDampMiniMine("Perks.FakePhysics.Spin45MiniMine.ImpulseDamp", 1.0f, 0.2f, 1.0f);
Debug::Vector g_Perks_FakePhysics_Spin45KickForceMiniMine("Perks.FakePhysics.Spin45MiniMine.KickForce", -1.5f, 2.0f, -7.5f);
Debug::Vector g_Perks_FakePhysics_Spin45KickPosMiniMine("Perks.FakePhysics.Spin45MiniMine.KickPos", 0.5f, 0.0f, -3.0f);

Debug::Float g_Perks_FakePhysics_FlipThrottle("Perks.FakePhysics.Flip.Throttle", 2.0f);
Debug::Float g_Perks_FakePhysics_FlipForce("Perks.FakePhysics.Flip.Force", 1.4f);
Debug::Float g_Perks_FakePhysics_FlipForceRand("Perks.FakePhysics.Flip.ForceRand", 0.0f);
Debug::Vector g_Perks_FakePhysics_FlipImpulseScale("Perks.FakePhysics.Flip.ImpulseScale", 0.0f, 1.0f, 0.25f);
Debug::Vector g_Perks_FakePhysics_FlipImpulseDamp("Perks.FakePhysics.Flip.ImpulseDamp", 0.75f, 0.75f, 0.75f);
Debug::Vector g_Perks_FakePhysics_FlipKickForce("Perks.FakePhysics.Flip.KickForce", 0.0f, 8.0f, -8.0f);
Debug::Vector g_Perks_FakePhysics_FlipKickPos("Perks.FakePhysics.Flip.KickPos", 0.75f, 0.0f, 1.0f);

Debug::Float g_Perks_FakePhysics_RollThrottle("Perks.FakePhysics.Roll.Throttle", 3.0f);
Debug::Float g_Perks_FakePhysics_RollForce("Perks.FakePhysics.Roll.Force", 2.3f);
Debug::Float g_Perks_FakePhysics_RollForceRand("Perks.FakePhysics.Roll.ForceRand", 0.0f);
Debug::Vector g_Perks_FakePhysics_RollImpulseScale("Perks.FakePhysics.Roll.ImpulseScale", 1.0f, 0.0f, 0.25f);
Debug::Vector g_Perks_FakePhysics_RollImpulseDamp("Perks.FakePhysics.Roll.ImpulseDamp", 0.75f, 0.75f, 0.75f);
Debug::Vector g_Perks_FakePhysics_RollKickForce("Perks.FakePhysics.Roll.KickForce", -9.0f, 6.0f, -5.0f);
Debug::Vector g_Perks_FakePhysics_RollKickPos("Perks.FakePhysics.Roll.KickPos", 2.0f, 0.0f, 1.0f);

// Steve's amazing barge of wonder.
Debug::Float g_Perks_FakePhysics_BargeJinkThrottle("Perks.FakePhysics.JinkBarge.Throttle", 5.0f);
Debug::Float g_Perks_FakePhysics_BargeJinkForce("Perks.FakePhysics.JinkBarge.Force", 1.f);
Debug::Float g_Perks_FakePhysics_BargeJinkForceRand("Perks.FakePhysics.JinkBarge.ForceRand", 0.0f);
Debug::Vector g_Perks_FakePhysics_BargeJinkImpulseScale("Perks.FakePhysics.JinkBarge.ImpulseScale", 0.1f, 0.0f, -0.05f);
Debug::Vector g_Perks_FakePhysics_BargeJinkImpulseDamp("Perks.FakePhysics.JinkBarge.ImpulseDamp", 0.0f, 0.0f, 0.0f);
Debug::Vector g_Perks_FakePhysics_BargeJinkKickForce("Perks.FakePhysics.JinkBarge.KickForce", -17.f, 0.0f, -16.0f);
Debug::Vector g_Perks_FakePhysics_BargeJinkKickPos("Perks.FakePhysics.JinkBarge.KickPos", 1.0f, 0.0f, 0.0f);
Debug::Vector g_Perks_FakePhysics_BargeJinkConstantKickForce("Perks.FakePhysics.JinkBarge.ConstantKickForce", 0.0f, 0.0f, -0.5f);
Debug::Vector g_Perks_FakePhysics_BargeJinkConstantKickPos("Perks.FakePhysics.JinkBarge.ConstantKickPos", 1.0f, 0.0f, 0.0f);

Debug::Float g_Perks_FakePhysics_BargeMinorJinkThrottle("Perks.FakePhysics.JinkBargeMinor.Throttle", 5.0f);
Debug::Float g_Perks_FakePhysics_BargeMinorJinkForce("Perks.FakePhysics.JinkBargeMinor.Force", 1.f);
Debug::Float g_Perks_FakePhysics_BargeMinorJinkForceRand("Perks.FakePhysics.JinkBargeMinor.ForceRand", 0.0f);
Debug::Vector g_Perks_FakePhysics_BargeMinorJinkImpulseScale("Perks.FakePhysics.JinkBargeMinor.ImpulseScale", 0.1f, 0.0f, -0.035f);
Debug::Vector g_Perks_FakePhysics_BargeMinorJinkImpulseDamp("Perks.FakePhysics.JinkBargeMinor.ImpulseDamp", 0.0f, 0.0f, 0.0f);
Debug::Vector g_Perks_FakePhysics_BargeMinorJinkKickForce("Perks.FakePhysics.JinkBargeMinor.KickForce", -13.0f, 0.0f, -12.0f);
Debug::Vector g_Perks_FakePhysics_BargeMinorJinkKickPos("Perks.FakePhysics.JinkBargeMinor.KickPos", 0.75f, 0.0f, 0.0f);
Debug::Vector g_Perks_FakePhysics_BargeMinorJinkConstantKickForce("Perks.FakePhysics.JinkBargeMinor.ConstantKickForce", 0.0f, 0.0f, -0.35f);
Debug::Vector g_Perks_FakePhysics_BargeMinorJinkConstantKickPos("Perks.FakePhysics.JinkBargeMinor.ConstantKickPos", 0.75f, 0.0f, 0.0f);

Debug::Float g_Perks_FakePhysics_ShieldJinkThrottle("Perks.FakePhysics.JinkShield.Throttle", 5.0f);
Debug::Float g_Perks_FakePhysics_ShieldJinkForce("Perks.FakePhysics.JinkShield.Force", 1.f);
Debug::Float g_Perks_FakePhysics_ShieldJinkForceRand("Perks.FakePhysics.JinkShield.ForceRand", 0.0f);
Debug::Vector g_Perks_FakePhysics_ShieldJinkImpulseScale("Perks.FakePhysics.JinkShield.ImpulseScale",  0.1f, 0.0f, -0.05f);
Debug::Vector g_Perks_FakePhysics_ShieldJinkImpulseDamp("Perks.FakePhysics.JinkShield.ImpulseDamp", 0.0f, 0.0f, 0.0f);
Debug::Vector g_Perks_FakePhysics_ShieldJinkKickForce("Perks.FakePhysics.JinkShield.KickForce", -17.0f, 0.0f, -16.0f);
Debug::Vector g_Perks_FakePhysics_ShieldJinkKickPos("Perks.FakePhysics.JinkShield.KickPos", 1.0f, 0.0f, 0.0f);
Debug::Vector g_Perks_FakePhysics_ShieldJinkConstantKickForce("Perks.FakePhysics.JinkShield.ConstantKickForce", 0.0f, 0.0f, -0.5f);
Debug::Vector g_Perks_FakePhysics_ShieldJinkConstantKickPos("Perks.FakePhysics.JinkShield.ConstantKickPos", 1.0f, 0.0f, 0.0f);

Debug::Float g_Perks_FakePhysics_NitrousJinkThrottle("Perks.FakePhysics.JinkNitrous.Throttle", 5.0f);
Debug::Float g_Perks_FakePhysics_NitrousJinkForce("Perks.FakePhysics.JinkNitrous.Force", 1.f);
Debug::Float g_Perks_FakePhysics_NitrousJinkForceRand("Perks.FakePhysics.JinkNitrous.ForceRand", 0.0f);
Debug::Vector g_Perks_FakePhysics_NitrousJinkImpulseScale("Perks.FakePhysics.JinkNitrous.ImpulseScale", 0.1f, 0.0f, -0.05f);
Debug::Vector g_Perks_FakePhysics_NitrousJinkImpulseDamp("Perks.FakePhysics.JinkNitrous.ImpulseDamp", 0.0f, 0.0f, 0.0f);
Debug::Vector g_Perks_FakePhysics_NitrousJinkKickForce("Perks.FakePhysics.JinkNitrous.KickForce", -17.0f, 0.0f, -19.0f);
Debug::Vector g_Perks_FakePhysics_NitrousJinkKickPos("Perks.FakePhysics.JinkNitrous.KickPos", 1.0f, 0.0f, 0.0f);
Debug::Vector g_Perks_FakePhysics_NitrousJinkConstantKickForce("Perks.FakePhysics.JinkNitrous.ConstantKickForce", 0.0f, 0.0f, -0.5f);
Debug::Vector g_Perks_FakePhysics_NitrousJinkConstantKickPos("Perks.FakePhysics.JinkNitrous.ConstantKickPos", 1.0f, 0.0f, 0.0f);

Debug::Float g_Perks_FakePhysics_HeavyBatteringRamJinkThrottle("Perks.FakePhysics.JinkBatteringRam.Heavy.Throttle", 5.0f);//
Debug::Float g_Perks_FakePhysics_HeavyBatteringRamJinkForce("Perks.FakePhysics.JinkBatteringRam.Heavy.Force", 1.f);//
Debug::Float g_Perks_FakePhysics_HeavyBatteringRamJinkForceRand("Perks.FakePhysics.JinkBatteringRam.Heavy.ForceRand", 0.0f);//
Debug::Vector g_Perks_FakePhysics_HeavyBatteringRamJinkImpulseScale("Perks.FakePhysics.JinkBatteringRam.Heavy.ImpulseScale", 0.1f, 0.0f, -0.05f);//
Debug::Vector g_Perks_FakePhysics_HeavyBatteringRamJinkImpulseDamp("Perks.FakePhysics.JinkBatteringRam.Heavy.ImpulseDamp", 0.0f, 0.0f, 0.0f);
Debug::Vector g_Perks_FakePhysics_HeavyBatteringRamJinkKickForce("Perks.FakePhysics.JinkBatteringRam.Heavy.KickForce", -11.f, 0.0f, -14.0f);
Debug::Vector g_Perks_FakePhysics_HeavyBatteringRamJinkKickPos("Perks.FakePhysics.JinkBatteringRam.Heavy.KickPos", 1.0f, 0.0f, 0.0f);
Debug::Vector g_Perks_FakePhysics_HeavyBatteringRamJinkConstantKickForce("Perks.FakePhysics.JinkBatteringRam.Heavy.ConstantKickForce", -0.6f, 0.0f, -0.5f);
Debug::Vector g_Perks_FakePhysics_HeavyBatteringRamJinkConstantKickPos("Perks.FakePhysics.JinkBatteringRam.Heavy.ConstantKickPos", 0.0f, 0.0f, 0.0f);

Debug::Float g_Perks_FakePhysics_LightBatteringRamJinkThrottle("Perks.FakePhysics.JinkBatteringRam.Light.Throttle", 5.0f);//
Debug::Float g_Perks_FakePhysics_LightBatteringRamJinkForce("Perks.FakePhysics.JinkBatteringRam.Light.Force", 1.f);//
Debug::Float g_Perks_FakePhysics_LightBatteringRamJinkForceRand("Perks.FakePhysics.JinkBatteringRam.Light.ForceRand", 0.0f);//
Debug::Vector g_Perks_FakePhysics_LightBatteringRamJinkImpulseScale("Perks.FakePhysics.JinkBatteringRam.Light.ImpulseScale", 0.1f, 0.0f, -0.5f);//
Debug::Vector g_Perks_FakePhysics_LightBatteringRamJinkImpulseDamp("Perks.FakePhysics.JinkBatteringRam.Light.ImpulseDamp", 0.0f, 0.0f, 0.0f);
Debug::Vector g_Perks_FakePhysics_LightBatteringRamJinkKickForce("Perks.FakePhysics.JinkBatteringRam.Light.KickForce", -11.f, 0.0f, -11.0f);
Debug::Vector g_Perks_FakePhysics_LightBatteringRamJinkKickPos("Perks.FakePhysics.JinkBatteringRam.Light.KickPos", 1.0f, 0.0f, 0.0f);
Debug::Vector g_Perks_FakePhysics_LightBatteringRamJinkConstantKickForce("Perks.FakePhysics.JinkBatteringRam.Light.ConstantKickForce", -0.3f, 0.0f, -0.5f);
Debug::Vector g_Perks_FakePhysics_LightBatteringRamJinkConstantKickPos("Perks.FakePhysics.JinkBatteringRam.Light.ConstantKickPos", 0.0f, 0.0f, 0.0f);

// Cakey's barge that's okay but not as good as Steve's
//Debug::Float g_Perks_FakePhysics_JinkThrottle("Perks.FakePhysics.Jink.Throttle", 5.0f);
//Debug::Float g_Perks_FakePhysics_JinkForce("Perks.FakePhysics.Jink.Force", 0.5f);
//Debug::Float g_Perks_FakePhysics_JinkForceRand("Perks.FakePhysics.Jink.ForceRand", 0.0f);
//Debug::Vector g_Perks_FakePhysics_JinkImpulseScale("Perks.FakePhysics.Jink.ImpulseScale", 1.0f, 0.0f, 0.25f);
//Debug::Vector g_Perks_FakePhysics_JinkImpulseDamp("Perks.FakePhysics.Jink.ImpulseDamp", 0.75f, 0.75f, 0.75f);
//Debug::Vector g_Perks_FakePhysics_JinkKickForce("Perks.FakePhysics.Jink.KickForce", -6.0f, 2.5f, -12.0f);
//Debug::Vector g_Perks_FakePhysics_JinkKickPos("Perks.FakePhysics.Jink.KickPos", 2.0f, 0.0f, 1.0f);

Debug::Float g_Perks_FakePhysics_TipThrottle("Perks.FakePhysics.Tip.Throttle", 5.0f);
Debug::Float g_Perks_FakePhysics_TipForce("Perks.FakePhysics.Tip.Force", 0.3f);
Debug::Float g_Perks_FakePhysics_TipForceRand("Perks.FakePhysics.Tip.ForceRand", 0.0f);
Debug::Vector g_Perks_FakePhysics_TipImpulseScale("Perks.FakePhysics.Tip.ImpulseScale", 0.0f, 1.0f, 0.0f);
Debug::Vector g_Perks_FakePhysics_TipImpulseDamp("Perks.FakePhysics.Tip.ImpulseDamp", 0.75f, 0.75f, 0.75f);
Debug::Vector g_Perks_FakePhysics_TipKickForce("Perks.FakePhysics.Tip.KickForce", 0.0f, 5.0f, -10.0f);
Debug::Vector g_Perks_FakePhysics_TipKickPos("Perks.FakePhysics.Tip.KickPos", 0.5f, 0.0f, 1.0f);

Debug::Float g_Perks_FakePhysics_SpeedAccelerateForce("Perks.FakePhysics.Speed.AccelerateForce", 300.0f);
Debug::Float g_Perks_FakePhysics_SpeedAccelerateStabilise("Perks.FakePhysics.Speed.AccelerateStabilise", 5.0f);
Debug::Float g_Perks_FakePhysics_SpeedAccelerateStabiliseRate("Perks.FakePhysics.Speed.AccelerateStabiliseRate", 10.0f);

Debug::Float g_Perks_FakePhysics_SpeedBrakeForce("Perks.FakePhysics.Speed.BrakeForce", 300.0f);
Debug::Float g_Perks_FakePhysics_SpeedBrakeMaxTime("Perks.FakePhysics.Speed.BrakeMaxTime", 2.0f);

Debug::Float g_Perks_FakePhysics_StickToGround_MarkA("Perks.FakePhysics.StickToGround.MarkA", 2.0f);
Debug::Float g_Perks_FakePhysics_StickToGround_MarkB("Perks.FakePhysics.StickToGround.MarkB", 3.0f);
Debug::Float g_Perks_FakePhysics_StickToGround_Force("Perks.FakePhysics.StickToGround.Force", 25.0f);
Debug::Float g_Perks_FakePhysics_StickToGround_MarkSpeed("Perks.FakePhysics.StickToGround.MarkSpeed", 100.0f);
Debug::Float g_Perks_FakePhysics_StickToGround_ForceSpeed("Perks.FakePhysics.StickToGround.ForceSpeed", 100.0f);

namespace
{
	Debug::Bool g_Perks_FakePhysics_Testing_Enabled("Perks.FakePhysics.Testing.Enabled", false);

	Debug::Int g_Perks_FakePhysics_Testing_CarNo("Perks.FakePhysics.Testing.CarNo", 0, Debug::Range(0, 19));

	const int kFpTestMaxCarStates = 40;

	U64 g_LoadCarState_TimeStamp;

	U64 g_FpTest_TimeStart;
	U64 g_FpTest_Phase;
	float g_FpTest_LastSpeed;
	SharedPhysics::C_RigidBody g_FpTest_CarState[kFpTestMaxCarStates];
	int g_FpTest_CarStateAction[kFpTestMaxCarStates];

	void FpTestStart(void*) { g_FpTest_Phase = 1; }

	void FpTestSaveCarState(void*) { g_FpTest_CarStateAction[g_Perks_FakePhysics_Testing_CarNo] = 1; }
	void FpTestLoadCarState(void*) { g_FpTest_CarStateAction[g_Perks_FakePhysics_Testing_CarNo] = 2; }

	void FpTestSaveAllCarStates(void*)
	{
		g_LoadCarState_TimeStamp = Time::GetPerformanceCounter();

		for(int i = 0; i < kFpTestMaxCarStates; i++)
			g_FpTest_CarStateAction[i] = 1;
	}

	void FpTestLoadAllCarStates(void*)
	{
		U64 currentTime = Time::GetPerformanceCounter();

		DebugLn("Time elapsed since last state: %.2f", (currentTime - g_LoadCarState_TimeStamp) / 1000000.0f);

		g_LoadCarState_TimeStamp = currentTime;

		for(int i = 0; i < kFpTestMaxCarStates; i++)
			g_FpTest_CarStateAction[i] = 2;
	}

	void FpTestLoadAndStart(void*) { FpTestStart(0); FpTestLoadCarState(0); }

	float GetFpTime() { return (Time::GetPerformanceCounter() - g_FpTest_TimeStart) / 1000000.0f; }

	Debug::Function g_Perks_FakePhysics_Testing_SaveCarPos("Perks.FakePhysics.Testing.SaveCarState", FpTestSaveCarState, 0);
	Debug::Function g_Perks_FakePhysics_Testing_LoadCarPos("Perks.FakePhysics.Testing.LoadCarState", FpTestLoadCarState, 0);

	Debug::Function g_Perks_FakePhysics_Testing_LoadAllCarPos("Perks.FakePhysics.Testing.SaveAllCarStates", FpTestSaveAllCarStates, 0);
	Debug::Function g_Perks_FakePhysics_Testing_SaveAllCarPos("Perks.FakePhysics.Testing.LoadAllCarStates", FpTestLoadAllCarStates, 0);

	Debug::Float g_Perks_FakePhysics_Testing_TargetSpeed("Perks.FakePhysics.Testing.TargetSpeed", 100.0f, Debug::Range(0.0f, 200.0f));
	Debug::Enum<Game::Actors::E_FakeVehiclePhysicsEffect> g_Perks_FakePhysics_Testing_Effect("Perks.FakePhysics.Testing.Effect", Game::Actors::E_FAKE_PHYSICS_SPIN_360_LEFT);

	Debug::Function g_Perks_FakePhysics_Testing_Start("Perks.FakePhysics.Testing.Start", FpTestStart, 0);
	Debug::Function g_Perks_FakePhysics_Testing_LoadAndStart("Perks.FakePhysics.Testing.LoadAndStart", FpTestLoadAndStart, 0);
}

namespace Game { namespace Actors
{

	//
	// FakePhys helpers
	//

	namespace
	{
		float MphToMps(float mph) { return mph * (1609.0f / 3600.0f); }
		float MpsToMph(float mps) { return mps * (3600.0f / 1609.0f); }
	}

	enum FakePhys_Flip
	{
		kFakePhys_NoFlip,
		kFakePhys_FlipBoth,
		kFakePhys_FlipVal,
		kFakePhys_FlipRandomX,
		kFakePhys_FlipKick
	};

	void FakePhys_ApplyKickForce(SharedPhysics::C_RigidBody& rb, const Debug::VectorStruct3& kickForce, const Debug::VectorStruct3& kickPos, FakePhys_Flip flip, int numHits, bool doFlipX)
	{
		Maths::HVec3 physForce(kickForce.vx, kickForce.vy, kickForce.vz);
		Maths::HVec3 physPos(kickPos.vx, kickPos.vy, kickPos.vz);

		bool flipX = (doFlipX || flip == kFakePhys_FlipBoth || flip == kFakePhys_FlipKick);

		if (flipX)
		{
			physForce	*= Maths::HVec3(-1.0f, 1.0f, 1.0f);
			physPos		*= Maths::HVec3(-1.0f, 1.0f, 1.0f);
		}

		float fwdSpeed = MpsToMph(Maths::DotProduct(rb.m_LinearVelocity, rb.m_OrientationMatrix.AZ().ZeroedY()));

		float knockBackScaling = Maths::Clamp(	
			(fwdSpeed - g_Perks_FakePhysics_Scaling_Z_MinSpeed) / (g_Perks_FakePhysics_Scaling_Z_MaxSpeed - g_Perks_FakePhysics_Scaling_Z_MinSpeed),
			0.0f, 1.0f);

		// Knockback modifier increases rapidly with multiple hits

		float recoveryTimeSpeedMod=0.1f;
		const RLPickupParametersStoredOnline* ppso=RLDatabase::Instance().GetPickupParametersStoredOnline();
		if (ppso)
		{
			recoveryTimeSpeedMod=ppso->mRecoveryTimeSpeedMod;
		}

		float knockBackMod = 0;
		for(int i = 1; i <= numHits && knockBackMod < 1.0f; i++ )
		{
			knockBackMod += (recoveryTimeSpeedMod * i);
		}

		knockBackScaling = Maths::Clamp(knockBackScaling - knockBackMod, 0.f, 1.f);

		float kickXScale = fwdSpeed / g_Perks_FakePhysics_Scaling_X_MaxSpeed;
		float kickXScaleClamped = Maths::Clamp(kickXScale + (F32)g_Perks_FakePhysics_Scaling_X_ScaleMin, (F32)g_Perks_FakePhysics_Scaling_X_ScaleMin, (F32)g_Perks_FakePhysics_Scaling_X_ScaleMax);

		physForce.X(physForce.X() * kickXScaleClamped);
		physForce.Z(physForce.Z() * knockBackScaling);

		rb.ApplyImpulseLocalSpace(physForce * rb.Mass(), physPos);
	}

	float FakePhys_SetupEffect(SharedPhysics::C_RigidBody& rb, float force, float forceRand, const Debug::VectorStruct3& kickForce, const Debug::VectorStruct3& kickPos, 
								FakePhys_Flip flip, int numHits)
	{
		float rnd	= (float)rand() / (float)RAND_MAX; // TODO: use correct rand() function
		float value	= (force + (rnd * forceRand)) * 3.14159f;

		if ((flip == kFakePhys_FlipBoth) || (flip == kFakePhys_FlipVal))
			value *= -1.0f;

		bool flipX = false;
		if ((flip == kFakePhys_FlipVal) || (flip == kFakePhys_FlipRandomX))
			flipX = rnd < 0.5f;

		FakePhys_ApplyKickForce(rb, kickForce, kickPos, flip, numHits, flipX);
		return value;
	}

	void FakePhys_UpdateEffect(SharedPhysics::C_RigidBody& rb, float dt, E_FakeVehiclePhysicsEffect& effect, float& value, float throttle, 
		const Debug::VectorStruct3& impulseScale, const Debug::VectorStruct3& impulseDamp, const Debug::VectorStruct3& constantKickForce, 
		const Debug::VectorStruct3& constantKickPos, FakePhys_Flip flip, int numHits)
	{
		value -= value * dt * throttle;

		Maths::C_MatrixRotateRPH rph;

		rph.m_Rotate = Maths::HVec3(
				impulseScale.vx * (3.14159f * dt * value),
				impulseScale.vy * (3.14159f * dt * value),
				impulseScale.vz * (3.14159f * dt * value)
			);

		// I'd rather have done this a level up; thanks a lot stupid debug vector.
		if(effect == E_FAKE_PHYSICS_JINK_NITROUS_RIGHT)
			rph.m_Rotate.Z(rph.m_Rotate.Z() * -1.f);

		rb.m_OrientationMatrix	= Maths::HMat33(rph) * rb.m_OrientationMatrix;
		rb.m_Orientation		= Maths::C_Quaternion(rb.m_OrientationMatrix);

		rb.m_AngularMomentum *= Maths::HVec3(impulseDamp.vx, impulseDamp.vy, impulseDamp.vz);
		rb.m_AngularVelocity = rb.m_AngularMomentum * rb.m_InverseInertiaTensor;

		if (fabsf(value) < 0.05f)
			effect = E_FAKE_PHYSICS_NONE;

		FakePhys_ApplyKickForce(rb, constantKickForce, constantKickPos, flip, numHits, false);
	}

	//
	// FakeVehiclePhysicsState
	//

	FakeVehiclePhysicsState::FakeVehiclePhysicsState() :
		_nextEffect(E_FAKE_PHYSICS_NONE),
		_effect(E_FAKE_PHYSICS_NONE),
		_value(0.0f),
		_recoveryTime(0.0f),
		_numConsecutiveHits(0),
		_stickToGround(0.0f)
	{
	}

	void FakeVehiclePhysicsState::Start(E_FakeVehiclePhysicsEffect effect)
	{
		if(IsInterruptable())
			Stop();

		if (_effect == E_FAKE_PHYSICS_NONE)
			_nextEffect = effect;

		IncrementConsecutiveHits();
	}

	void FakeVehiclePhysicsState::IncrementConsecutiveHits()
	{
		if(_recoveryTime > 0.f)
			_numConsecutiveHits++;
		else
		{
			float recoveryTime=6.f;
			const RLPickupParametersStoredOnline* ppso=RLDatabase::Instance().GetPickupParametersStoredOnline();
			if (ppso)
			{
				recoveryTime=ppso->mRecoveryTime;
			}

			_recoveryTime = recoveryTime;
			_numConsecutiveHits = 0;
		}
	}

	void FakeVehiclePhysicsState::Stop()
	{
		_nextEffect	= E_FAKE_PHYSICS_NONE;
		_effect		= E_FAKE_PHYSICS_NONE;
	}

	void FakeVehiclePhysicsState::Update(C_VehicleActor* self, float dt, SharedPhysics::C_RigidBody& rb)
	{
#ifndef CFG_GOLD
		if (g_Perks_FakePhysics_Testing_Enabled)
		{
			int vehNo = self->GetVehicleIndex();

			if (g_FpTest_CarStateAction[vehNo] == 1)
			{
				g_FpTest_CarState[vehNo] = rb;
				g_FpTest_CarStateAction[vehNo] = 0;
			}
			else if (g_FpTest_CarStateAction[vehNo] == 2)
			{
				rb.m_OrientationMatrix	= g_FpTest_CarState[vehNo].m_OrientationMatrix;
				rb.m_Position			= g_FpTest_CarState[vehNo].m_Position;
				rb.m_CenterOfMass		= rb.m_Position + rb.m_CenterOfMassOffset;
				rb.m_Orientation		= g_FpTest_CarState[vehNo].m_Orientation;
				rb.m_LinearMomentum		= g_FpTest_CarState[vehNo].m_LinearMomentum;
				rb.m_AngularMomentum	= g_FpTest_CarState[vehNo].m_AngularMomentum;
				rb.m_LinearVelocity		= g_FpTest_CarState[vehNo].m_LinearVelocity;
				rb.m_AngularVelocity	= g_FpTest_CarState[vehNo].m_AngularVelocity;
				rb.m_KineticEnergy		= g_FpTest_CarState[vehNo].m_KineticEnergy;
				rb.m_Force				= g_FpTest_CarState[vehNo].m_Force;
				rb.m_Torque				= g_FpTest_CarState[vehNo].m_Torque;

				self->m_PrevPosition	= rb.m_Position;

				g_FpTest_CarStateAction[vehNo] = 0;
			}

			if (vehNo == g_Perks_FakePhysics_Testing_CarNo)
			{
				float speed = MpsToMph(rb.m_LinearVelocity.Length());

				if (g_FpTest_Phase == 1)
				{
					if (speed >= g_Perks_FakePhysics_Testing_TargetSpeed)
					{
						Stop();
						Start(g_Perks_FakePhysics_Testing_Effect);

						g_FpTest_TimeStart = Time::GetPerformanceCounter();
						g_FpTest_LastSpeed = speed;

						DebugLn("FP: START %4.2f %4.2f %s", GetFpTime(), speed, g_Perks_FakePhysics_Testing_Effect.ValueAsName());

						g_FpTest_Phase++;
					}
				}
				else if (g_FpTest_Phase == 2)
				{
					if (speed < (g_Perks_FakePhysics_Testing_TargetSpeed * 0.95f))
					{
						DebugLn("FP: BRAKE %4.2f %4.2f", GetFpTime(), speed);
						g_FpTest_Phase++;
					}
				}
				else if (g_FpTest_Phase == 3)
				{
					if (speed >= (g_FpTest_LastSpeed * 1.05f))
					{
						DebugLn("FP: ACCEL %4.2f %4.2f (%.2f)", GetFpTime(), speed, g_FpTest_LastSpeed);
						g_FpTest_Phase++;
					}

					if (speed < g_FpTest_LastSpeed)
						g_FpTest_LastSpeed = speed;
				}
				else if (g_FpTest_Phase == 4)
				{
					if (speed >= g_Perks_FakePhysics_Testing_TargetSpeed)
					{
						DebugLn("FP: RECOV %4.2f %4.2f", GetFpTime(), speed);
						g_FpTest_Phase++;
					}
				}
			}
		}
#endif

		if (_nextEffect != E_FAKE_PHYSICS_NONE)
		{
			_effect		= _nextEffect;
			_nextEffect	= E_FAKE_PHYSICS_NONE;

			Maths::HVec3 physForce;
			Maths::HVec3 physPos;

			float rnd = (float)rand() / (float)RAND_MAX; // TODO: use correct rand() function

			switch(_effect)
			{
				case E_FAKE_PHYSICS_SPIN_360_LEFT:		_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_Spin360Force, g_Perks_FakePhysics_Spin360ForceRand, g_Perks_FakePhysics_Spin360KickForce, g_Perks_FakePhysics_Spin360KickPos, kFakePhys_FlipBoth, _numConsecutiveHits);	break;
				case E_FAKE_PHYSICS_SPIN_360_RIGHT:		_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_Spin360Force, g_Perks_FakePhysics_Spin360ForceRand, g_Perks_FakePhysics_Spin360KickForce, g_Perks_FakePhysics_Spin360KickPos, kFakePhys_NoFlip, _numConsecutiveHits);		break;

				case E_FAKE_PHYSICS_SPIN_45_LEFT:		_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_Spin45Force, g_Perks_FakePhysics_Spin45ForceRand, g_Perks_FakePhysics_Spin45KickForce, g_Perks_FakePhysics_Spin45KickPos, kFakePhys_FlipBoth, _numConsecutiveHits);	break;
				case E_FAKE_PHYSICS_SPIN_45_RIGHT:		_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_Spin45Force, g_Perks_FakePhysics_Spin45ForceRand, g_Perks_FakePhysics_Spin45KickForce, g_Perks_FakePhysics_Spin45KickPos, kFakePhys_NoFlip, _numConsecutiveHits);		break;

				case E_FAKE_PHYSICS_SPIN_45_LEFT_WEAK:	_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_Spin45Force, g_Perks_FakePhysics_Spin45ForceRand, g_Perks_FakePhysics_Spin45KickForceWeak, g_Perks_FakePhysics_Spin45KickPos, kFakePhys_FlipBoth, _numConsecutiveHits);	break;
				case E_FAKE_PHYSICS_SPIN_45_RIGHT_WEAK:	_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_Spin45Force, g_Perks_FakePhysics_Spin45ForceRand, g_Perks_FakePhysics_Spin45KickForceWeak, g_Perks_FakePhysics_Spin45KickPos, kFakePhys_NoFlip, _numConsecutiveHits);		break;

				case E_FAKE_PHYSICS_SPIN_45_LEFT_MINIMINE:	_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_Spin45ForceMiniMine, g_Perks_FakePhysics_Spin45ForceRandMiniMine, g_Perks_FakePhysics_Spin45KickForceMiniMine, g_Perks_FakePhysics_Spin45KickPosMiniMine, kFakePhys_FlipBoth, _numConsecutiveHits);	break;
				case E_FAKE_PHYSICS_SPIN_45_RIGHT_MINIMINE:	_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_Spin45ForceMiniMine, g_Perks_FakePhysics_Spin45ForceRandMiniMine, g_Perks_FakePhysics_Spin45KickForceMiniMine, g_Perks_FakePhysics_Spin45KickPosMiniMine, kFakePhys_NoFlip, _numConsecutiveHits);		break;

				case E_FAKE_PHYSICS_FLIP_FORWARD:		_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_FlipForce, g_Perks_FakePhysics_FlipForceRand, g_Perks_FakePhysics_FlipKickForce, g_Perks_FakePhysics_FlipKickPos, kFakePhys_FlipRandomX, _numConsecutiveHits);	break;
				case E_FAKE_PHYSICS_FLIP_BACKWARD:		_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_FlipForce, g_Perks_FakePhysics_FlipForceRand, g_Perks_FakePhysics_FlipKickForce, g_Perks_FakePhysics_FlipKickPos, kFakePhys_FlipVal, _numConsecutiveHits);		break;

				case E_FAKE_PHYSICS_ROLL_LEFT:			_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_RollForce, g_Perks_FakePhysics_RollForceRand, g_Perks_FakePhysics_RollKickForce, g_Perks_FakePhysics_RollKickPos, kFakePhys_NoFlip, _numConsecutiveHits);		break;
				case E_FAKE_PHYSICS_ROLL_RIGHT:			_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_RollForce, g_Perks_FakePhysics_RollForceRand, g_Perks_FakePhysics_RollKickForce, g_Perks_FakePhysics_RollKickPos, kFakePhys_FlipBoth, _numConsecutiveHits);	break;
	
				case E_FAKE_PHYSICS_JINK_BARGE_LEFT:	_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_BargeJinkForce, g_Perks_FakePhysics_BargeJinkForceRand, g_Perks_FakePhysics_BargeJinkKickForce, g_Perks_FakePhysics_BargeJinkKickPos, kFakePhys_NoFlip, _numConsecutiveHits);		break;
				case E_FAKE_PHYSICS_JINK_BARGE_RIGHT:	_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_BargeJinkForce, g_Perks_FakePhysics_BargeJinkForceRand, g_Perks_FakePhysics_BargeJinkKickForce, g_Perks_FakePhysics_BargeJinkKickPos, kFakePhys_FlipBoth, _numConsecutiveHits);	break;
				
				case E_FAKE_PHYSICS_JINK_BARGE_MINOR_LEFT:	_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_BargeMinorJinkForce, g_Perks_FakePhysics_BargeJinkForceRand, g_Perks_FakePhysics_BargeMinorJinkKickForce, g_Perks_FakePhysics_BargeMinorJinkKickPos, kFakePhys_NoFlip, _numConsecutiveHits);		break;
				case E_FAKE_PHYSICS_JINK_BARGE_MINOR_RIGHT:	_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_BargeMinorJinkForce, g_Perks_FakePhysics_BargeJinkForceRand, g_Perks_FakePhysics_BargeMinorJinkKickForce, g_Perks_FakePhysics_BargeMinorJinkKickPos, kFakePhys_FlipBoth, _numConsecutiveHits);	break;

				case E_FAKE_PHYSICS_JINK_SHIELD_LEFT:	_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_ShieldJinkForce, g_Perks_FakePhysics_ShieldJinkForceRand, g_Perks_FakePhysics_ShieldJinkKickForce, g_Perks_FakePhysics_ShieldJinkKickPos, kFakePhys_NoFlip, _numConsecutiveHits);		break;
				case E_FAKE_PHYSICS_JINK_SHIELD_RIGHT:	_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_ShieldJinkForce, g_Perks_FakePhysics_ShieldJinkForceRand, g_Perks_FakePhysics_ShieldJinkKickForce, g_Perks_FakePhysics_ShieldJinkKickPos, kFakePhys_FlipBoth, _numConsecutiveHits);	break;

				case E_FAKE_PHYSICS_JINK_NITROUS_LEFT:	_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_NitrousJinkForce, g_Perks_FakePhysics_NitrousJinkForceRand, g_Perks_FakePhysics_NitrousJinkKickForce, g_Perks_FakePhysics_NitrousJinkKickPos, kFakePhys_NoFlip, _numConsecutiveHits);			break;
				case E_FAKE_PHYSICS_JINK_NITROUS_RIGHT:	_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_NitrousJinkForce, g_Perks_FakePhysics_NitrousJinkForceRand, g_Perks_FakePhysics_NitrousJinkKickForce, g_Perks_FakePhysics_NitrousJinkKickPos, kFakePhys_FlipKick, _numConsecutiveHits);	break;
				
				case E_FAKE_PHYSICS_TIP_FORWARD:		_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_TipForce, g_Perks_FakePhysics_TipForceRand, g_Perks_FakePhysics_TipKickForce, g_Perks_FakePhysics_TipKickPos, kFakePhys_FlipRandomX, _numConsecutiveHits);	break;
				case E_FAKE_PHYSICS_TIP_BACKWARD:		_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_TipForce, g_Perks_FakePhysics_TipForceRand, g_Perks_FakePhysics_TipKickForce, g_Perks_FakePhysics_TipKickPos, kFakePhys_FlipVal, _numConsecutiveHits);		break;

				case E_FAKE_PHYSICS_BATTERING_RAM_HEAVY_LEFT:		_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_HeavyBatteringRamJinkForce, g_Perks_FakePhysics_HeavyBatteringRamJinkForceRand, g_Perks_FakePhysics_HeavyBatteringRamJinkKickForce, g_Perks_FakePhysics_HeavyBatteringRamJinkKickPos, kFakePhys_NoFlip, _numConsecutiveHits);		break;
				case E_FAKE_PHYSICS_BATTERING_RAM_HEAVY_RIGHT:		_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_HeavyBatteringRamJinkForce, g_Perks_FakePhysics_HeavyBatteringRamJinkForceRand, g_Perks_FakePhysics_HeavyBatteringRamJinkKickForce, g_Perks_FakePhysics_HeavyBatteringRamJinkKickPos, kFakePhys_FlipBoth, _numConsecutiveHits);	break;
				case E_FAKE_PHYSICS_BATTERING_RAM_LIGHT_LEFT:		_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_LightBatteringRamJinkForce, g_Perks_FakePhysics_LightBatteringRamJinkForceRand, g_Perks_FakePhysics_LightBatteringRamJinkKickForce, g_Perks_FakePhysics_LightBatteringRamJinkKickPos, kFakePhys_NoFlip, _numConsecutiveHits);		break;
				case E_FAKE_PHYSICS_BATTERING_RAM_LIGHT_RIGHT:		_value = FakePhys_SetupEffect(rb, g_Perks_FakePhysics_LightBatteringRamJinkForce, g_Perks_FakePhysics_LightBatteringRamJinkForceRand, g_Perks_FakePhysics_LightBatteringRamJinkKickForce, g_Perks_FakePhysics_LightBatteringRamJinkKickPos, kFakePhys_FlipBoth, _numConsecutiveHits);	break;

	
				default: _value = 0.0f; break;
			}
		}

		/*
			airacingtech::ISplineTracker *vehicleDataSplineTracker = AIAmax::AmaxAIR::Instance().GetAISplineTracker(mVehicleIndex);

			if( vehicleDataSplineTracker->GetCurrentRoute() && vehicleDataSplineTracker->GetCurrentRoute()->m_pRoute )
			{
				vehicleDataSplineTracker->GetCurrentRoute()->m_pRoute->GetDirection(vehicleDataSplineTracker->GetCurrentRoute()->m_index, splineDir);
		*/

		switch(_effect)
		{
			case E_FAKE_PHYSICS_SPIN_360_LEFT:		FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_Spin360Throttle, g_Perks_FakePhysics_Spin360ImpulseScale, g_Perks_FakePhysics_Spin360ImpulseDamp, g_Perks_FakePhysics_DefaultConstantKickForce, g_Perks_FakePhysics_DefaultConstantKickPos, kFakePhys_FlipBoth, _numConsecutiveHits);					break;
			case E_FAKE_PHYSICS_SPIN_360_RIGHT:		FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_Spin360Throttle, g_Perks_FakePhysics_Spin360ImpulseScale, g_Perks_FakePhysics_Spin360ImpulseDamp, g_Perks_FakePhysics_DefaultConstantKickForce, g_Perks_FakePhysics_DefaultConstantKickPos, kFakePhys_NoFlip, _numConsecutiveHits);						break;

			case E_FAKE_PHYSICS_SPIN_45_LEFT:		FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_Spin45Throttle, g_Perks_FakePhysics_Spin45ImpulseScale, g_Perks_FakePhysics_Spin45ImpulseDamp, g_Perks_FakePhysics_DefaultConstantKickForce, g_Perks_FakePhysics_DefaultConstantKickPos, kFakePhys_FlipBoth, _numConsecutiveHits);						break;
			case E_FAKE_PHYSICS_SPIN_45_RIGHT:		FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_Spin45Throttle, g_Perks_FakePhysics_Spin45ImpulseScale, g_Perks_FakePhysics_Spin45ImpulseDamp, g_Perks_FakePhysics_DefaultConstantKickForce, g_Perks_FakePhysics_DefaultConstantKickPos, kFakePhys_NoFlip, _numConsecutiveHits);							break;

			case E_FAKE_PHYSICS_SPIN_45_LEFT_WEAK:	FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_Spin45Throttle, g_Perks_FakePhysics_Spin45ImpulseScale, g_Perks_FakePhysics_Spin45ImpulseDamp, g_Perks_FakePhysics_DefaultConstantKickForce, g_Perks_FakePhysics_DefaultConstantKickPos, kFakePhys_FlipBoth, _numConsecutiveHits);						break;
			case E_FAKE_PHYSICS_SPIN_45_RIGHT_WEAK:	FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_Spin45Throttle, g_Perks_FakePhysics_Spin45ImpulseScale, g_Perks_FakePhysics_Spin45ImpulseDamp, g_Perks_FakePhysics_DefaultConstantKickForce, g_Perks_FakePhysics_DefaultConstantKickPos, kFakePhys_NoFlip, _numConsecutiveHits);							break;

			case E_FAKE_PHYSICS_SPIN_45_LEFT_MINIMINE:	FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_Spin45ThrottleMiniMine, g_Perks_FakePhysics_Spin45ImpulseScale, g_Perks_FakePhysics_Spin45ImpulseDampMiniMine, g_Perks_FakePhysics_DefaultConstantKickForce, g_Perks_FakePhysics_DefaultConstantKickPos, kFakePhys_FlipBoth, _numConsecutiveHits);						break;
			case E_FAKE_PHYSICS_SPIN_45_RIGHT_MINIMINE:	FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_Spin45ThrottleMiniMine, g_Perks_FakePhysics_Spin45ImpulseScale, g_Perks_FakePhysics_Spin45ImpulseDampMiniMine, g_Perks_FakePhysics_DefaultConstantKickForce, g_Perks_FakePhysics_DefaultConstantKickPos, kFakePhys_NoFlip, _numConsecutiveHits);							break;

			case E_FAKE_PHYSICS_FLIP_FORWARD:		FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_FlipThrottle, g_Perks_FakePhysics_FlipImpulseScale, g_Perks_FakePhysics_FlipImpulseDamp, g_Perks_FakePhysics_DefaultConstantKickForce, g_Perks_FakePhysics_DefaultConstantKickPos, kFakePhys_FlipRandomX, _numConsecutiveHits);							break;
			case E_FAKE_PHYSICS_FLIP_BACKWARD:		FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_FlipThrottle, g_Perks_FakePhysics_FlipImpulseScale, g_Perks_FakePhysics_FlipImpulseDamp, g_Perks_FakePhysics_DefaultConstantKickForce, g_Perks_FakePhysics_DefaultConstantKickPos, kFakePhys_FlipVal, _numConsecutiveHits);								break;

			case E_FAKE_PHYSICS_ROLL_LEFT:			FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_RollThrottle, g_Perks_FakePhysics_RollImpulseScale, g_Perks_FakePhysics_RollImpulseDamp, g_Perks_FakePhysics_DefaultConstantKickForce, g_Perks_FakePhysics_DefaultConstantKickPos, kFakePhys_NoFlip, _numConsecutiveHits);								break;
			case E_FAKE_PHYSICS_ROLL_RIGHT:			FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_RollThrottle, g_Perks_FakePhysics_RollImpulseScale, g_Perks_FakePhysics_RollImpulseDamp, g_Perks_FakePhysics_DefaultConstantKickForce, g_Perks_FakePhysics_DefaultConstantKickPos, kFakePhys_FlipBoth, _numConsecutiveHits);								break;

			case E_FAKE_PHYSICS_JINK_BARGE_LEFT:	FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_BargeJinkThrottle, g_Perks_FakePhysics_BargeJinkImpulseScale, g_Perks_FakePhysics_BargeJinkImpulseDamp, g_Perks_FakePhysics_BargeJinkConstantKickForce, g_Perks_FakePhysics_BargeJinkConstantKickPos, kFakePhys_NoFlip, _numConsecutiveHits);			break;
			case E_FAKE_PHYSICS_JINK_BARGE_RIGHT:	FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_BargeJinkThrottle, g_Perks_FakePhysics_BargeJinkImpulseScale, g_Perks_FakePhysics_BargeJinkImpulseDamp, g_Perks_FakePhysics_BargeJinkConstantKickForce, g_Perks_FakePhysics_BargeJinkConstantKickPos, kFakePhys_FlipBoth, _numConsecutiveHits);			break;

			case E_FAKE_PHYSICS_JINK_BARGE_MINOR_LEFT:	FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_BargeMinorJinkThrottle, g_Perks_FakePhysics_BargeMinorJinkImpulseScale, g_Perks_FakePhysics_BargeJinkImpulseDamp, g_Perks_FakePhysics_BargeMinorJinkConstantKickForce, g_Perks_FakePhysics_BargeMinorJinkConstantKickPos, kFakePhys_NoFlip, _numConsecutiveHits);			break;
			case E_FAKE_PHYSICS_JINK_BARGE_MINOR_RIGHT:	FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_BargeMinorJinkThrottle, g_Perks_FakePhysics_BargeMinorJinkImpulseScale, g_Perks_FakePhysics_BargeJinkImpulseDamp, g_Perks_FakePhysics_BargeMinorJinkConstantKickForce, g_Perks_FakePhysics_BargeMinorJinkConstantKickPos, kFakePhys_FlipBoth, _numConsecutiveHits);			break;

			case E_FAKE_PHYSICS_JINK_SHIELD_LEFT:	FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_ShieldJinkThrottle, g_Perks_FakePhysics_ShieldJinkImpulseScale, g_Perks_FakePhysics_ShieldJinkImpulseDamp, g_Perks_FakePhysics_ShieldJinkConstantKickForce, g_Perks_FakePhysics_ShieldJinkConstantKickPos, kFakePhys_NoFlip, _numConsecutiveHits);		break;
			case E_FAKE_PHYSICS_JINK_SHIELD_RIGHT:	FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_ShieldJinkThrottle, g_Perks_FakePhysics_ShieldJinkImpulseScale, g_Perks_FakePhysics_ShieldJinkImpulseDamp, g_Perks_FakePhysics_ShieldJinkConstantKickForce, g_Perks_FakePhysics_ShieldJinkConstantKickPos, kFakePhys_FlipBoth, _numConsecutiveHits);		break;

			case E_FAKE_PHYSICS_JINK_NITROUS_LEFT:	FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_NitrousJinkThrottle, g_Perks_FakePhysics_NitrousJinkImpulseScale, g_Perks_FakePhysics_NitrousJinkImpulseDamp, g_Perks_FakePhysics_NitrousJinkConstantKickForce, g_Perks_FakePhysics_NitrousJinkConstantKickPos, kFakePhys_NoFlip, _numConsecutiveHits);	break;
			case E_FAKE_PHYSICS_JINK_NITROUS_RIGHT:	FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_NitrousJinkThrottle, g_Perks_FakePhysics_NitrousJinkImpulseScale, g_Perks_FakePhysics_NitrousJinkImpulseDamp, g_Perks_FakePhysics_NitrousJinkConstantKickForce, g_Perks_FakePhysics_NitrousJinkConstantKickPos, kFakePhys_FlipBoth, _numConsecutiveHits);break;	

			case E_FAKE_PHYSICS_TIP_FORWARD:		FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_TipThrottle, g_Perks_FakePhysics_TipImpulseScale, g_Perks_FakePhysics_TipImpulseDamp, g_Perks_FakePhysics_DefaultConstantKickForce, g_Perks_FakePhysics_DefaultConstantKickPos, kFakePhys_FlipRandomX, _numConsecutiveHits);								break;
			case E_FAKE_PHYSICS_TIP_BACKWARD:		FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_TipThrottle, g_Perks_FakePhysics_TipImpulseScale, g_Perks_FakePhysics_TipImpulseDamp, g_Perks_FakePhysics_DefaultConstantKickForce, g_Perks_FakePhysics_DefaultConstantKickPos, kFakePhys_FlipVal, _numConsecutiveHits);									break;

			case E_FAKE_PHYSICS_BATTERING_RAM_HEAVY_LEFT:	FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_HeavyBatteringRamJinkThrottle, g_Perks_FakePhysics_HeavyBatteringRamJinkImpulseScale, g_Perks_FakePhysics_HeavyBatteringRamJinkImpulseDamp, g_Perks_FakePhysics_HeavyBatteringRamJinkConstantKickForce, g_Perks_FakePhysics_HeavyBatteringRamJinkConstantKickPos, kFakePhys_NoFlip, _numConsecutiveHits);			break;
			case E_FAKE_PHYSICS_BATTERING_RAM_HEAVY_RIGHT:	FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_HeavyBatteringRamJinkThrottle, g_Perks_FakePhysics_HeavyBatteringRamJinkImpulseScale, g_Perks_FakePhysics_HeavyBatteringRamJinkImpulseDamp, g_Perks_FakePhysics_HeavyBatteringRamJinkConstantKickForce, g_Perks_FakePhysics_HeavyBatteringRamJinkConstantKickPos, kFakePhys_FlipBoth, _numConsecutiveHits);		break;
			case E_FAKE_PHYSICS_BATTERING_RAM_LIGHT_LEFT:	FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_LightBatteringRamJinkThrottle, g_Perks_FakePhysics_LightBatteringRamJinkImpulseScale, g_Perks_FakePhysics_LightBatteringRamJinkImpulseDamp, g_Perks_FakePhysics_LightBatteringRamJinkConstantKickForce, g_Perks_FakePhysics_LightBatteringRamJinkConstantKickPos, kFakePhys_NoFlip, _numConsecutiveHits);			break;
			case E_FAKE_PHYSICS_BATTERING_RAM_LIGHT_RIGHT:	FakePhys_UpdateEffect(rb, dt, _effect, _value, g_Perks_FakePhysics_LightBatteringRamJinkThrottle, g_Perks_FakePhysics_LightBatteringRamJinkImpulseScale, g_Perks_FakePhysics_LightBatteringRamJinkImpulseDamp, g_Perks_FakePhysics_LightBatteringRamJinkConstantKickForce, g_Perks_FakePhysics_LightBatteringRamJinkConstantKickPos, kFakePhys_FlipBoth, _numConsecutiveHits);		break;

		}

		if(_recoveryTime > 0.f)
		{
			float timeStep = C_GameApp::GetInstance()->GetTimeStep();
			_recoveryTime -= timeStep;
		}

		if (_effect == E_FAKE_PHYSICS_NONE)
		{
			int count = 0;

			for(int i = 0; i < 4; i++)
				count += self->GetVehicleInput()->m_Wheels[i].m_bOnGround != 0;

			if (count == 0)
				_stickToGround += dt;
			else
				_stickToGround = 0.0f;

			float markA = g_Perks_FakePhysics_StickToGround_MarkA;
			float markB = g_Perks_FakePhysics_StickToGround_MarkB;
			float speed = MpsToMph(rb.m_LinearVelocity.Length());

			float markScale = g_Perks_FakePhysics_StickToGround_MarkSpeed / Maths::Max(speed, (float)g_Perks_FakePhysics_StickToGround_MarkSpeed); 

			markA *= markScale;
			markB *= markScale;

			if (_stickToGround > markA)
			{
				float v = _stickToGround - markA;

				if (v > markB)
					v = markB;

				v /= markB - markA;

				v *= speed / (float)g_Perks_FakePhysics_StickToGround_ForceSpeed;

				rb.AddForce(Maths::HVec3(0.0f, -v * rb.Mass() * (float)g_Perks_FakePhysics_StickToGround_Force, 0.0f));
			}
		}
		else
			_stickToGround = 0.0f;
	}

	bool FakeVehiclePhysicsState::IsActive() const
	{
		return _effect != Game::Actors::E_FAKE_PHYSICS_NONE;
	}

	bool FakeVehiclePhysicsState::IsInterruptable() const
	{
		bool isInterrupt = (
			_effect == Game::Actors::E_FAKE_PHYSICS_SPIN_45_LEFT || _effect == Game::Actors::E_FAKE_PHYSICS_SPIN_45_RIGHT ||
			_effect == Game::Actors::E_FAKE_PHYSICS_SPIN_45_LEFT_WEAK || _effect == Game::Actors::E_FAKE_PHYSICS_SPIN_45_RIGHT_WEAK ||
			_effect == Game::Actors::E_FAKE_PHYSICS_SPIN_45_LEFT_MINIMINE || _effect == Game::Actors::E_FAKE_PHYSICS_SPIN_45_RIGHT_MINIMINE );

		return isInterrupt;
	}

	//
	// FakeVehicleSpeedState
	//

	FakeVehicleSpeedState::FakeVehicleSpeedState() :
		_nextEffect(E_FAKE_SPEED_NONE),
		_nextTarget(0.0f),
		_effect(E_FAKE_SPEED_NONE),
		_target(0.0f), 
		_value(0.0f),
		_slowdownRate(0.0f)
	{
	}

	void FakeVehicleSpeedState::Start(E_FakeVehicleSpeedEffect effect, float target, float rate)
	{
		_nextEffect		= effect;
		_nextTarget		= target;
		_slowdownRate	= (rate > 0.f) ? rate : g_Perks_FakePhysics_SpeedBrakeForce;
	}

	void FakeVehicleSpeedState::Stop()
	{
		_nextEffect	= E_FAKE_SPEED_NONE;
		_nextTarget	= 0.0f;
		_effect		= E_FAKE_SPEED_NONE;
		_target		= 0.0f;
	}

	void FakeVehicleSpeedState::Update(C_VehicleActor* self, float dt, SharedPhysics::C_RigidBody& rb)
	{
		if (_nextEffect != E_FAKE_SPEED_NONE)
		{
			_effect			= _nextEffect;
			_target			= _nextTarget;
			_nextEffect		= E_FAKE_SPEED_NONE;
			_nextTarget		= 0.0f;

			switch(_effect)
			{
				case E_FAKE_SPEED_ACCELERATE:	_value = 0.5f;	break;
				case E_FAKE_SPEED_BRAKE:		_value = g_Perks_FakePhysics_SpeedBrakeMaxTime;	break;

				default: _value = 0.0f; break;
			}
		}

		switch(_effect)
		{
			case E_FAKE_SPEED_ACCELERATE:
			{
				float fwdSpeed		= Maths::DotProduct(rb.m_LinearVelocity, rb.m_OrientationMatrix.AZ());
				float hrzSpeed		= Maths::DotProduct(rb.m_LinearVelocity, rb.m_OrientationMatrix.AX());
				float desiredSpeed	= MphToMps(_target);

				if (fwdSpeed < desiredSpeed)
				{
					float adjust = (fwdSpeed < (desiredSpeed * 0.5f)) ? 2.0f : 1.0f;

					Maths::HVec3 force = rb.m_OrientationMatrix.AZ() * (g_Perks_FakePhysics_SpeedAccelerateForce * rb.Mass() * adjust);

					force.Y(0.0f);

					rb.AddForce(force);
				}

				if (hrzSpeed > MphToMps(g_Perks_FakePhysics_SpeedAccelerateStabilise))
				{
					Maths::HVec3 force;

					force = rb.m_OrientationMatrix.AX() * -(hrzSpeed * g_Perks_FakePhysics_SpeedAccelerateStabiliseRate * rb.Mass());

					force.Y(0.0f);

					rb.AddForce(force);
				}

				_value -= dt;

				if (_value <= 0.0f)
					_effect = E_FAKE_SPEED_NONE;
			}
			break;

			case E_FAKE_SPEED_BRAKE:
			{
				float speed			= rb.m_LinearVelocity.Length();
				float desiredSpeed	= MphToMps(_target);

				if (speed > desiredSpeed)
				{
					float adjust = (speed > (desiredSpeed * 0.5f)) ? 2.0f : 1.0f;

					Maths::HVec3 force = rb.m_LinearVelocity * -((_slowdownRate * rb.Mass() * adjust) / speed);

					rb.AddForce(force);

					_value -= dt;

					if (_value <= 0.0f)
						_effect = E_FAKE_SPEED_NONE;
				}
				else
					_effect = E_FAKE_SPEED_NONE;
			}
			break;
		}
	}

	bool FakeVehicleSpeedState::IsActive() const
	{
		return _effect != Game::Actors::E_FAKE_SPEED_NONE;
	}

} }
