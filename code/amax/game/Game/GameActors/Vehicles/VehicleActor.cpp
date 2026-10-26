  /////////////////////////////////////////////////////////////
// VehicleActor
/////////////////////////////////////////////////////////////

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

#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/PhysicsMessages.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>
#include <shared_actors/SharedActorUtils.h>
#include <shared_actors/Utility/VisibilityDetection.h>

#include <GameLogic/ActorStudioActors/DynamicObjectActor.h>
#include <GameLogic/ActorStudioActors/PhysicalObjectActor.h>

#include <Game/Game.h>
#include <Game/GameControls/GameControls.h>
#include <camera2/stock/ChaseCamera2.h>
#include <camera2/stock/DebugFeed.h>
#include <game/AmaxCam/CameraSystem.h>
#include <game/AmaxCam/PhysicsChecks.h>
#include <game/AmaxCam/AmaxChaseCamera.h>
#include <GameApp.h>
#include <Game/GameDef/PhysicsConstants.h>
#include <framework/application/ApplicationBase.h>

#include <Game\Messaging\GameMessages.h>
#include <Game\Messaging\VehicleActorMessages.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>
#include <Game/Messaging/GameflowMessages.h>

#include <GameLogic/GameData/GameEffectData.h>
#include <GameLogic/GameData/GameVehicleData.h>

#include <GameLogic\RaceLogicCore\Core\RaceLogicCore.h>
#include <GameLogic\Layers\RaceLogicLayer.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>

#include <GameLogic\Replays\ReplayPacket.h>

#include <gravity/sdk/CompositeDescriptor.h>
#include <gravity/sdk/ConstraintDescriptors.h>
#include <gravity/sdk/KinematicController.h>
#include <Gravity/Triggers/TriggerSystem.h>
#include <gravity/triggers/TriggerEvent.h>

#include <Render/Utils/Helper.h>
#include <resource_utils/ResourceBagUtils.h>

#include "Game/GameActors/Components/Creation/AudioComponentsCreate.h"
#include "gravity/narrowphase/shapes.h"

#include <SpecialFX/SpecialFX.h>

#include <World_Render/Scene.h>

#include <Dynamic/Particle/EventInstance.h>
#include <Dynamic/Particle/EventInstanceHandle.h>
#include <Dynamic/Particle/Generic/XtInterface.h>
#include <Dynamic/Common.h>
#include <Dynamic/Renderer.h>
#include <Dynamic/Particle/Manager.h>

#include <SceneGraph/SceneGraph.h>
#include <SceneGraph/View.h>

#include <system/Memory.h>
#include <world_render/ModelInstance.h>
#include <game_resource/ModelData.h>
#include <Game/GameWorld/GameWorld.h>
#include <Game/GameWorld/ViewType.h>
#include <Game/GameWorld/RenderPass.h>
#include <Game/GameWorld/ViewManager.h>
#include <Game/GameWorld/View.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>

#include <System/Misc/Random.h>

#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>
#include <GameLogic/ActorStudioActors/CutSceneMessages.h>
#include <UI/UiMessages.h>
#include <UI/UiLayer.h>
#include <Ui/UiXtConfig.h>

//#define TURN_OFF_VEHICLE_OPTIMISATIONS
#include <Gravity/SDK/ShapeDescriptors.h>
#include "gravity/narrowphase/LineCollisionCache.h"

#include <Display/Buffer/VertexDeclarations.h>
#include <BlobShadows/BlobShadows.h>

#include <GameLogic/GameData/GameEffectData.h>

#include <GameLogic\RaceLogicCore\Database\RLDatabase.h>

#include <System/Misc/Random.h>
#include <Game/GameUtil/ResourceBag.h>

#include <../library/kanzen/camera/HollywoodVehicleCamera.h>
#include <../library/kanzen/camera/Utility.h>

#include <AuxFlares/AuxFlares.h>

#include "Misc_Shared/Noise/Perlin.h"
#include <Game/GameActors/Drivers/DriverManager.h>
#include <Game/GameActors/Crowds/CrowdGameActor.h>

#include <GameLogic\Profile\GameProfileManager.h>
#include <GameLogic\StickerManager.h>

#include <Core/XtStdX.h>
#include "Game/AmaxCam/VehicleFeed.h"
#include <AI_Messages.h>
#include <ai/AI_Amax.h>

#include <lod/lodManager.h>

#include <LightTrails/LightTrails.h>

//#define TURN_OFF_VEHICLE_OPTIMISATIONS
#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif


#if defined(USER_CASPINALL)
#ifdef _MSC_VER
#pragma optimize ("", off) //RLC_DEBUG
#endif
#endif

namespace Game { extern bool g_SorceryRecordsModified; }

Debug::Logger	&g_VehicleActorLog( Debug::GetLogger("amax.actors.vehicle") );
Debug::Logger	&g_VehicleActorEffectsLog( Debug::GetLogger("amax.actors.vehicle.Effects") );
Debug::Bool		g_VehicleActorEffectsLogEnabled("Vehicle.Effects.EnableLog", false);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////DEBUG VARIABLES//////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Debug::Bool g_dbDrawTriangleCache("Vehicle.Debug.DrawTriangleCache", false);

//The AI data used to reset the vehicles is at ground level (obviously) but this means that we need to add a little Y() to make sure the car spawns above the ground
Debug::Float g_AutoResetHeightAdjustmentHack("RLC.TrackPosition.AutoResetHeightAdjustmentHack_NonPersist", 0.0f, Debug::Range(0.f, 2.5f)); 

Debug::Bool g_debugUpdateDynamicMaterials("Vehicle.Render.Material.UpdateDynamicMaterials", false);

Debug::Bool g_debugUpdatePaintFromXt("Vehicle.Render.Material.Paint.UpdateFromXt", false);
Debug::Int g_debugPaintIndex("Vehicle.Render.Material.Paint.PaintJobIndex", 0, Debug::Range(0, 20), Debug::Step(1));

Debug::Bool g_debugSetupDamageMap("Vehicle.Damage.SetupNodes", false);
Debug::Bool g_debugVehicleDisplayDebugModels("Vehicle.Render.Debug.DisplayModels", false);

Debug::Bool g_debugFlightControlEnable("Vehicle.FlightControl.Enabled", false);
Debug::Bool g_debugForceFlightControl("Vehicle.FlightControl.ForceOn", false);
Debug::Bool g_debugFlightControlInWorldSpace("Vehicle.FlightControl.WorldSpace", false);

Debug::Bool g_debugDrawVehicleHud("Vehicle.Hud.Draw", false, Debug::Persist());
Debug::Bool g_debugUseMultiplayerHollywoodCamera("Kanzen.Camera.HollywoodVehicleCamera.UseMultiplayerCamera", false);
Debug::Float g_debugHudNeedleLength("Vehicle.Hud.NeedleLength", 92.0f, Debug::Range(0.0f, 256.0f), Debug::Step(0.001f), Debug::Persist());
Debug::Float g_debugHudNeedleCentreX("Vehicle.Hud.CentreX", 800.0f, Debug::Range(0.0f, 1024.0f), Debug::Step(1.0f), Debug::Persist());
Debug::Float g_debugHudNeedleCentreY("Vehicle.Hud.CentreY", 620.0f, Debug::Range(0.0f, 768.0f), Debug::Step(1.0f), Debug::Persist());

Debug::Float g_debugHudNeedleRestAngle("Vehicle.Hud.NeedleRestAngle", 2.5f, Debug::Range(-4.0f, 4.0f), Debug::Step(0.001f), Debug::Persist());
Debug::Float g_debugHudNeedleMaxAngle("Vehicle.Hud.NeedleMaxAngle", -1.57f, Debug::Range(-4.0f, 4.0f), Debug::Step(0.001f), Debug::Persist());

Debug::Float g_HeadlightIntensity("Vehicle.Effects.Headlight.GlobalIntensityScale", 10.0f, Debug::Range(0.0f, 10.0f));
Debug::Float g_HeadlightCullDistance("Vehicle.Effects.Headlight.GlobalCullDistance", 128.0f, Debug::Range(0.0f, 1000.0f));
Debug::Float g_HeadlightNearCull("Vehicle.Effects.Headlight.GlobalNearCull", -4.0f, Debug::Range(-32.0f, 8.0f));
Debug::Float g_HeadlightCullDistanceLOD("Vehicle.Effects.Headlight.GlobalCullDistanceLOD", 70.0f, Debug::Range(0.0f, 1000.0f));
Debug::Float g_HeadlightNearCullLOD("Vehicle.Effects.Headlight.GlobalNearCullLOD", 3.0f, Debug::Range(-32.0f, 8.0f));

Debug::Float g_CutSceneHeadlightNearCull("Vehicle.Effects.Headlight.CutsceneNearCull", -100.0f, Debug::Range(-100.0f, 8.0f));
Debug::Bool g_UseCutSceneHeadlightNearCull("Vehicle.Effects.Headlight.UseCutsceneNearCull", false);

Debug::Bool g_debugTriggerEmp("Vehicle.Effects.Emp.DebugTrigger", false);
Debug::Float g_debugEmpDuration("Vehicle.Effects.Emp.Duration", 4.0f, Debug::Range(0.0f, 10.0f), Debug::Step(0.001f));
Debug::Float g_debugEmpProbability("Vehicle.Effects.Emp.Probability", 0.35f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Bool g_SmallImpactAccumulatorShow("Vehicle.Effects.Impacts.SmallImpactAccumulatorShow", false);
Debug::Float g_SmallImpactAccumulatorValue("Vehicle.Effects.Impacts.SmallImpactAccumulatorValue", 0.0f, Debug::Range(0.0f, 10.0f));
Debug::Float g_SmallImpactEffectValue("Vehicle.Effects.Impacts.SmallImpactEffectValue", 0.0f, Debug::Range(0.0f, 10.0f));
Debug::Float g_SmallImpactImpulseMin("Vehicle.Effects.Impacts.SmallImpactImpulseMin", 0.0f, Debug::Range(0.0f, 10.0f));
Debug::Float g_SmallImpactImpulseMax("Vehicle.Effects.Impacts.SmallImpactImpulseMax", 0.1f, Debug::Range(0.0f, 10.0f));
Debug::Float g_SmallImpactAccumulatorMax("Vehicle.Effects.Impacts.SmallImpactAccumulatorMax", 1.0f, Debug::Range(0.0f, 100.0f));
Debug::Float g_SmallImpactAccumulatorDecayRate("Vehicle.Effects.Impacts.SmallImpactAccumulatorDecayRate", 35.0f, Debug::Range(0.0f, 100.0f));
Debug::Float g_SmallImpactAccumulatorThreshold("Vehicle.Effects.Impacts.SmallImpactAccumulatorThreshold", 0.1f, Debug::Range(0.0f, 100.0f));
Debug::Float g_SmallImpactAccumulatorScale("Vehicle.Effects.Impacts.SmallImpactAccumulatorScale", 0.7f, Debug::Range(0.0f, 100.0f));
Debug::Float g_SmallImpactMinVelocity("Vehicle.Effects.Impacts.SmallImpactMinVelocity", 1.0f, Debug::Range(0.0f, 100.0f));
Debug::Float g_SmallImpactMaxVelocity("Vehicle.Effects.Impacts.SmallImpactMaxVelocity", 5.0f, Debug::Range(0.0f, 100.0f));
Debug::Float g_SmallImpactEffectDamping("Vehicle.Effects.Impacts.SmallImpactDamping", 0.8f, Debug::Range(0.0f, 1.0f));
Debug::Float g_SmallImpactAccumulatorStep("Vehicle.Effects.Impacts.SmallImpactAccumulatorStep", 0.015f, Debug::Range(0.0f, 1.0f));
Debug::Float g_SmallImpactDecayDelay("Vehicle.Effects.Impacts.SmallImpactDecayDelay", 0.18f, Debug::Range(0.0f, 10.0f));

Debug::Bool g_bDeferredVehicleRaytraces("Vehicle.DeferredRaytraces", true);

Debug::Bool g_bDeferredVehicleUpdate("Vehicle.DeferredUpdate", true);

JQ_DECLARE_PPU_MODULE(DeferredVehicleUpdateTask, Game::Actors::DeferredVehicleUpdate);

Debug::Float g_debugPerkShockedBrakeForce("Perks.ShockedBrakeForce", 0.5f, Debug::Range(0.0f, 1.0f));

Debug::Float g_debugAudioPartsMessageThreshold("Audio.MessageHandler.Vehicles.PartCollisionThreshold", 0.01f, Debug::Range(0.0f, 1.0f));

Debug::Float g_dbAngularDamping("Vehicle.AngularDampingCoefficient", 0.995f, Debug::Range(0.f, 1.f));

Debug::Float g_debugVehicleBaumgarte("Gravity.VehicleBaumgarte", 1.25f, Debug::Range(0.0f, 5.0f), Debug::Step(0.001f), Debug::Persist());
Debug::Float g_dbSlopValue("Gravity.VehicleSlop", 0.00005f, Debug::Range(1.f, 0.f));
Debug::Bool g_ConstraintCar("Vehicle.ConstraintCar", true); 
Debug::Float g_dbConstraintAngle("Vehicle.ConstraintCarAngle", 30.f, Debug::Range(0.f, 361.f));
Debug::Colour g_DashBoardDriverLightColour("Vehicle.DashBoardDriverLightColour", 0.1f, 0.1f, 0.25f, 1.f);

Debug::Float g_debugAbsoluteMaxAVehicleCanEverGo("Vehicle.MaxLinearSpeedMph", 250.0f, Debug::Range(0.0f, 500.0f), Debug::Step(0.001f));

Debug::Bool g_debugHighSpeedReset("Vehicle.Effects.HighSpeed.Reset", false);
Debug::Float g_debugHighSpeedEffect("Vehicle.Effects.HighSpeed.Value", -1.0f, Debug::Range(-1.0f, 1.0f));
Debug::Float g_debugHighSpeedEffectMaxVel("Vehicle.Effects.HighSpeed.MaxVel", 50.0f, Debug::Range(0.0f, 500.0f));
Debug::Float g_debugHighSpeedEffectDamping("Vehicle.Effects.HighSpeed.Damping", 0.975f, Debug::Range(0.0f, 1.0f));

Debug::Bool g_debugRoughTerrainEffectDisplay("Vehicle.Effects.RoughTerrain.Display", false);
Debug::Bool g_debugRoughTerrainReset("Vehicle.Effects.RoughTerrain.Reset", false);
Debug::Float g_debugRoughTerrainEffect("Vehicle.Effects.RoughTerrain.Value", -1.0f, Debug::Range(-1.0f, 1.0f));
Debug::Float g_debugRoughTerrainEffectDamping("Vehicle.Effects.RoughTerrain.Damping", 0.90f, Debug::Range(0.0f, 1.0f));
Debug::Float g_debugRoughTerrainEffectMaxRoughness("Vehicle.Effects.RoughTerrain.Scale", 2.0f, Debug::Range(0.0f, 10.0f));
Debug::Float g_debugRoughTerrainEffectMaxSpeed("Vehicle.Effects.RoughTerrain.MaxSpeed", 25.0f, Debug::Range(0.0f, 100.0f));

Debug::Bool g_applyRollingFrictionScale("Vehicle.Racing.ApplyRollingFrictionScale", true);

//temp
extern Debug::Float g_PerkGravGunSlot1_ForceX;
extern Debug::Float g_PerkGravGunSlot1_ForceY;
extern Debug::Float g_PerkGravGunSlot1_ForceZ;
extern Debug::Float g_PerkGravGunSlot1_PosX;
extern Debug::Float g_PerkGravGunSlot1_PosY;
extern Debug::Float g_PerkGravGunSlot1_PosZ;

extern Debug::Float g_PerkGravGunSlot2_ForceX;
extern Debug::Float g_PerkGravGunSlot2_ForceY;
extern Debug::Float g_PerkGravGunSlot2_ForceZ;
extern Debug::Float g_PerkGravGunSlot2_PosX;
extern Debug::Float g_PerkGravGunSlot2_PosY;
extern Debug::Float g_PerkGravGunSlot2_PosZ;

extern Debug::Float g_PerkGravGunSlot3_ForceX;
extern Debug::Float g_PerkGravGunSlot3_ForceY;
extern Debug::Float g_PerkGravGunSlot3_ForceZ;
extern Debug::Float g_PerkGravGunSlot3_PosX;
extern Debug::Float g_PerkGravGunSlot3_PosY;
extern Debug::Float g_PerkGravGunSlot3_PosZ;

extern Debug::Float g_PerkGravGunSlot4_ForceX;
extern Debug::Float g_PerkGravGunSlot4_ForceY;
extern Debug::Float g_PerkGravGunSlot4_ForceZ;
extern Debug::Float g_PerkGravGunSlot4_PosX;
extern Debug::Float g_PerkGravGunSlot4_PosY;
extern Debug::Float g_PerkGravGunSlot4_PosZ;

extern Debug::Bool g_debugFakeArtDamage;
extern Debug::Float g_debugFakeArtScratchLevel;
extern Debug::Float g_debugFakeArtDeformLevel;
extern Debug::Bool g_debugActuallyBurstTyre;
extern Debug::Bool g_debugActuallyLoseWheel;
extern Debug::Bool g_debugFakeFatalDamage;

extern Debug::Bool g_bDisableSPUDamage;

Debug::Float g_BlobShadowUnderwaterFactor("Vehicle.BlobShadowUnderwaterFactor", 1.0f, Debug::Range(0.0f, 5.0f));

Debug::Float g_DefaultHeroLightRadius("Vehicle.HeroLight.DefaultRadius", 6.0f, Debug::Range(0.0f, 10.0f));
Debug::Colour g_DefaultHeroLightColour("Vehicle.HeroLight.DefaultColour", 0.737603, 0.937906, 1.0f, 1.00f);
Debug::Float g_DefaultHeroLightOffsetX("Vehicle.HeroLight.DefaultOffsetX", 0.f, Debug::Range(-5.0f, 5.0f));
Debug::Float g_DefaultHeroLightOffsetY("Vehicle.HeroLight.DefaultOffsetY", 0.25f, Debug::Range(-5.0f, 5.0f));
Debug::Float g_DefaultHeroLightOffsetZ("Vehicle.HeroLight.DefaultOffsetZ", 1.5f, Debug::Range(-5.0f, 5.0f));
Debug::Bool g_HeroLightEnable("Vehicle.HeroLight.Enable", true);

Debug::Float g_DefaultLightRadius("Vehicle.Lights.DefaultRadius", 1.5f, Debug::Range(0.0f, 10.0f));

Debug::Float g_DefaultLightOffsetZ("Vehicle.Lights.DefaultBreakOffsetZ", -0.5f, Debug::Range(-5.0f, 5.0f));
Debug::Float g_DefaultLightOffsetY("Vehicle.Lights.DefaultBreakOffsetY", 0.25f, Debug::Range(-5.0f, 5.0f));

Debug::Colour g_DefaultBreakLightColour("Vehicle.Lights.DefaultBreakColour", 1.f, 0.1f, 0.0f, 0.2f);
Debug::Colour g_DefaultRearLightColour("Vehicle.Lights.DefaultRearColour", 1.f, 0.1f, 0.0f, 0.1f);
Debug::Colour g_DefaultReverseLightColour("Vehicle.Lights.DefaultReverseColour", 1.f, 1.0f, 1.0f, 0.25f);

Debug::Float g_DynamicLightCullDistant("Vehicle.Lights.DynamicLightCullDistant", 25.f, Debug::Range(0.f, 128.0f));

Debug::Colour g_DefaultBreakSelfRearLightColour("Vehicle.Lights.SelfRearLights.BreakColour", 0.6f, 0.1f, 0.0f, 0.2f);
Debug::Colour g_DefaultReverseSelfRearLightColour("Vehicle.Lights.SelfRearLights.ReverseColour", 0.5f, 0.5f, 0.5f, 0.2f);

Debug::Float g_DefaultLightFlareFarDistant("Vehicle.Lights.LightFlareFarDistant", 35.f);
Debug::Float g_DefaultLightFlareFarDistantBadPerf("Vehicle.Lights.LightFlareFarDistantBadPerf", 8.f);
Debug::Float g_DefaultLightFlareExtraAngle("Vehicle.Lights.LightFlareExtraAngle", 0.2f);

Debug::Bool g_wreckedImpulseForceApply("Vehicle.Logic.Impulse.Apply", false);

Debug::Vector3 g_WreckedImpulseMultiplier("Vehicle.Logic.Impulse.AngularVelMultiplier",0.0f,0.5f,3.0f);
Debug::Vector3 g_WreckedImpulseMin("Vehicle.Logic.Impulse.AngularVelMin",0.0f,0.5f,6.0f);
Debug::Float g_WreckedImpulseUpVel("Vehicle.Logic.Impulse.UpVel",2.0f);

Debug::Float g_SpeedBoostUpgade("Multiplayer.Upgrades.SpeedBoost", 1.03f, Debug::Range(0.2f, 5.f));
Debug::Float g_SpeedBoostUpgadeAirFriction("Multiplayer.Upgrades.SpeedBoostAirFriction", 0.97f, Debug::Range(0.2f, 5.f));

Debug::Bool g_DefaultRearLightTrailsUpdate("Vehicle.LightTrails.Update", false);

namespace Game
{
	extern Debug::Bool g_FixedReflectionNormal;
}

Util::C_Random g_VehicleRandom;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////DEBUG VARIABLES//////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Maths::HMat43 RPToMaths(const RacingPhysics::Axis* p_pAxis)
{
	Maths::HVec3 ax(p_pAxis->AxisX().X(), p_pAxis->AxisX().Y(), p_pAxis->AxisX().Z());
	Maths::HVec3 ay(p_pAxis->AxisY().X(), p_pAxis->AxisY().Y(), p_pAxis->AxisY().Z());
	Maths::HVec3 az(p_pAxis->AxisZ().X(), p_pAxis->AxisZ().Y(), p_pAxis->AxisZ().Z());
	Maths::HVec3 aw(p_pAxis->Position().X(), p_pAxis->Position().Y(),	p_pAxis->Position().Z());
	return Maths::HMat43(ax, ay, az, aw);	
}

Maths::HVec3 RPToMaths(const RacingPhysics::Vector3R* p_pVector)
{
	return Maths::HVec3(p_pVector->X(), p_pVector->Y(), p_pVector->Z());
}

Maths::HVec3 RPToMaths(const RacingPhysics::Point3R* p_pPoint)
{
	return Maths::HVec3(p_pPoint->X(), p_pPoint->Y(), p_pPoint->Z());
}

Maths::HVec3 RPToMaths(const RacingPhysics::Normal3R* p_pNormal)
{
	return Maths::HVec3(p_pNormal->X(), p_pNormal->Y(), p_pNormal->Z());
}

namespace Game
{

C_VehicleParticleEmitter::C_VehicleParticleEmitter()
{
	m_Intensity		= 0.0f;
	m_Activate		= false;
	m_Deactivate	= false;
	m_Created		= false;
}

void C_VehicleParticleEmitter::Setup(Maths::HVec3 p_Position, Maths::HVec3 p_Direction, XtHandle<DynamicEffect::Particle::XtInterface::Event> p_EventIdle,
																						XtHandle<DynamicEffect::Particle::XtInterface::Event> p_EventSpeed)
{
	if (!p_EventIdle || !p_EventIdle->pEvent || !p_EventSpeed || !p_EventSpeed->pEvent)
		return;

	m_Intensity	= 0.0f;
	m_Emitters.PushBack();
	Emitter* pEmitter = &m_Emitters.Back();
	pEmitter->m_Position = p_Position;
	p_Direction.Normalise();
	pEmitter->m_Orientation = DynamicEffect::GetQuaternionFromTo<Maths::C_Quaternion>(Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ(), p_Direction);
	pEmitter->m_XtEventIdle = p_EventIdle;
	pEmitter->m_XtEventSpeed = p_EventSpeed;
}

void C_VehicleParticleEmitter::Create(DynamicEffect::C_Node* p_pLocalSpace, float p_Intensity)
{
	m_Intensity	= p_Intensity;

	for (S32 iEmitter = 0; iEmitter < m_Emitters.Size(); iEmitter++)
	{
		Emitter* pEmitter = &m_Emitters[iEmitter];
		if (!pEmitter->m_XtEventIdle || !pEmitter->m_XtEventIdle->pEvent || !pEmitter->m_XtEventSpeed || !pEmitter->m_XtEventSpeed->pEvent)
			continue;

		DynamicEffect::C_Node node;
		node.SetPosition(pEmitter->m_Position);
		node.SetRotation(pEmitter->m_Orientation);
		pEmitter->m_ParticleEventIdle = pEmitter->m_XtEventIdle->pEvent->Create(node, p_pLocalSpace);
		if(DynamicEffect::Particle::C_EventInstance* pEventInstance = pEmitter->m_ParticleEventIdle.Object())
			pEventInstance->SetIntensity(m_Intensity);
		pEmitter->m_ParticleEventSpeed = pEmitter->m_XtEventSpeed->pEvent->Create(node, p_pLocalSpace);
		if(DynamicEffect::Particle::C_EventInstance* pEventInstance = pEmitter->m_ParticleEventSpeed.Object())
			pEventInstance->SetIntensity(m_Intensity);
	}

	m_Activate = false;
	m_Created = true;
}

void C_VehicleParticleEmitter::Destroy()
{
	m_Intensity = 0.0f;

	for (S32 iEmitter = 0; iEmitter < m_Emitters.Size(); iEmitter++)
	{
		if(DynamicEffect::Particle::C_EventInstance* pEventInstance = m_Emitters[iEmitter].m_ParticleEventIdle.Object())
			pEventInstance->Free();
		if(DynamicEffect::Particle::C_EventInstance* pEventInstance = m_Emitters[iEmitter].m_ParticleEventSpeed.Object())
			pEventInstance->Free();
	}

	m_Created = false;
}

void C_VehicleParticleEmitter::Shutdown()
{
	Destroy();

	m_Emitters.Clear();
}

Debug::Float g_debugDamageParticleSpeedMin("Vehicle.Damage.Effects.EngineSmokeSpeedBlendMin", 15.0f);
Debug::Float g_debugDamageParticleSpeedRange("Vehicle.Damage.Effects.EngineSmokeSpeedBlendRange", 15.0f);


void C_VehicleParticleEmitter::Update(DynamicEffect::C_Node* p_pLocalSpace, const Maths::C_RGBA_F32& p_Lighting, float p_Quality, float p_Speed, Maths::HVec3 p_Velocity, U32 visibilityFlags)
{
	bool recreate = false;

	float fSpeedScaleIdle = Maths::Clamp((p_Speed - (float)g_debugDamageParticleSpeedMin) / (float)g_debugDamageParticleSpeedRange, 0.0f, 1.0f);
	float fSpeedScaleSpeed = Maths::Clamp(p_Speed / (float)g_debugDamageParticleSpeedRange, 0.0f, 1.0f);
	for (S32 iEmitter = 0; iEmitter < m_Emitters.Size(); iEmitter++)
	{
		if(DynamicEffect::Particle::C_EventInstance* pEventInstance = m_Emitters[iEmitter].m_ParticleEventIdle.Object())
		{
			float fIntensityScale = (1.0f - fSpeedScaleIdle) * m_Intensity * p_Quality;
			float fLifeScale = 0.5f + fIntensityScale * 0.5f;
			pEventInstance->SetIntensity(fIntensityScale);
			pEventInstance->SetAlpha(fIntensityScale);
			pEventInstance->SetLifeScale(fLifeScale);
			pEventInstance->SetLighting(p_Lighting);
			pEventInstance->SetVelocity(p_Velocity);
			pEventInstance->SetVisibilityFlags(visibilityFlags);
		}
		else
			recreate = true;

		if(DynamicEffect::Particle::C_EventInstance* pEventInstance = m_Emitters[iEmitter].m_ParticleEventSpeed.Object())
		{
			float fIntensityScale = fSpeedScaleSpeed * m_Intensity * p_Quality;
			float fLifeScale = 0.5f + fIntensityScale * 0.5f;
			pEventInstance->SetIntensity(fIntensityScale);
			pEventInstance->SetAlpha(fIntensityScale);
			pEventInstance->SetLifeScale(fLifeScale);
			pEventInstance->SetLighting(p_Lighting);
			pEventInstance->SetVelocity(p_Velocity);
			pEventInstance->SetVisibilityFlags(visibilityFlags);
		}
		else
			recreate = true;
	}

	if(m_Created && recreate)
	{
		Destroy();
		Create(p_pLocalSpace, 1.0f);
	}
}

namespace Actors
{

/* 
===============================================================
					class C_VehicleActor

NOTES: 
===============================================================
*/

/*
===============
NAME: C_VehicleActor

NOTE: Constructor
===============
*/
C_VehicleActor::C_VehicleActor()
{
	m_pVehicleModel		= NULL;
	m_pVehiclePhysics	= NULL;
	m_pPhysicsChecks	= NULL;
	m_pCollisionBody	= NULL;
	m_pModelResource	= NULL;
	m_pPickupTriggerActor		= NULL;
	m_pTriggerRegionTriggerActor		= NULL;
	
	m_pRoughTerrainSpecialFX = NULL;
	m_pHighSpeedSpecialFX = NULL;
	m_pLandedSpecialFX	= NULL;
	m_pImpactSpecialFX	= NULL;
	m_pHealthSpecialFX	= NULL;
	m_pConstraint		= NULL;
	m_pAlphaTransitionCurve = NULL;
	m_pAuxFlareModelResource = NULL;
#if PLATFORM_PC
	m_Blur2PCHookAdded = false;	// [Blur2PC]
	m_Blur2PCLightsMoved = false;
#endif
	m_pPendingImpactEffect	= NULL;

	mPickupSFXArray.Clear();

	m_VehicleModelResourceIndex	= -1;
	m_WheelModelResourceIndex	= -1;
#ifdef _WHEEL_LOD
	m_WheelLodModelResourceIndex	= -1;
#endif
	m_VehicleIndex				= -1;
	m_PhysicsGroupId			= 0;
	m_XtIndex					= 0;
	m_ObjectColliderIndex		= -1;
	m_CarColliderIndex			= -1;
	m_MainElementIndex			= -1;
	m_HealthPoints				= 0;

	m_ReplayUpdateTimeStep		= 1.0f;				//	Default to sending state once every second
	m_fSmallImpactAccumulator	= 0.0f;
	m_fSmallImpactTimeSinceImpact = 0.0f;
	m_fSmallImpactEffectValue   = 0.0f;

	m_fPitch					= 0.0f;
	m_fRoll						= 0.0f;
	//m_RadiatorSmokeEmitter.m_Intensity		= 0.0f;
	m_EngineSmokeLightEmitter.m_Intensity	= 0.0f;
	m_EngineSmokeHeavyEmitter.m_Intensity	= 0.0f;
	//m_EngineFireEmitter.m_Intensity			= 0.0f;
	m_fAirTime					= 0.0f;
	m_FrameTime					= 0.0f;
	m_KillPlaneHeight			= 0.0f;


	m_GhostTransitionDuration	= 0.0f;
	m_GhostTransitionTime		= 0.0f;

	SetAlpha(E_VAC_TYPE_ALL, 1.f, E_VAC_TARGET_TYPE_ALL);

	m_LoadSeparateWheelModel	= false;
	m_RaceStarted				= false;
	m_ControlsEnabled			= true;
	m_Eliminated				= false;
	m_IsFocusVehicle			= false;
	m_LostControl				= false;
	m_FatallyDamaged			= false;
	m_ResetifyDriver			= false;
	m_tempHaveNewVehicleToVehicleMesh	= false;
	m_OnlyUnloadResources		= false;
	m_ResourcesUnloaded			= false;
	m_ConstraintFatallyDamaged	= false;
	m_HeavyCollision			= false;
	m_FreezeConstrainedParts	= false;

	m_PerformanceModifiers.Initialise();

	m_ShadowVertexCount = 0;
	m_ShadowIndexCount = 0;

	m_pBlobShadowInstance = NULL;
	m_Created	= false;
	m_Enabled	= false;

	m_pGroundRay = new Collision::C_Ray;
	m_pCollisionRay = new Collision::C_Ray;

	m_pVisDetect = NULL;
	m_pReflectionVisDetect = NULL;

	m_NeedUpdateLightCubeMap = true;
	m_UpdateLightCubeMapFrecuency = 1;
	m_UpdateLightCubeMapCount = 0;

	m_DeferredResetType = E_ResetType_None;
//	m_pProxyActor = NULL;

	m_overide_controls = false;

	m_corneringAssist = true;
	m_brakeAssist = true;

	m_pDashBoardNode = NULL;

	m_accelControllerInput = 0.0f;
	m_brakeControllerInput = 0.0f;
	m_handbrakeControllerInput = 0.0f;

	m_IsBumperCamera = false;
	m_EnableParticles = false;
	m_EnableDeferredEventParticles = false;
	m_hasGoneOffTrack = false;
	m_ViewportID = -1;
	m_ScreenSize = 0.0f;

	m_fRoughTerrainPercentage = 0.0f;
	m_fHighSpeedPercentage = 0.0f;

	m_WeightClass	= AmaxVehicleWeight_Medium;
	m_CollisionsEnabled = false;

	m_EffectsVisiblityFlags = 0xFFFFFFFF;

	m_hUpdateSchedulerGroup = Scheduler::JQ_GROUP_NONE;
}

/*
===============
NAME: ~C_VehicleActor

NOTE: Destructor
===============
*/
C_VehicleActor::~C_VehicleActor()
{
#if PLATFORM_PC
	if (m_Blur2PCHookAdded)	// [Blur2PC]
		WorldRender::Blur2PCInterpRemoveRenderHook(&C_VehicleActor::Blur2PCRenderHook, this);
#endif
	Destroy();

	delete m_pGroundRay;
	delete m_pCollisionRay;
}

Debug::Bool g_debugVehicleUpdateCentreOfMass("RacingPhysics.Cars.UpdateCentreOfMass", false, Debug::Persist());
Debug::Bool g_debugVehicleEditConstraints("Vehicle.DebugRender.DrawSorceryConstraints", false, Debug::Persist());
Debug::Float g_debugImpactEffectDuration("Vehicle.Impact.Duration", 0.75f, Debug::Range(0.0f, 10.0f), Debug::Step(0.001f));

Debug::Bool  g_debugParticleScaleEnable("Vehicle.Particle.EnableScaling", true);
Debug::Bool  g_debugParticleShowScales("Vehicle.Particle.ShowScales", false);
Debug::Float g_debugParticleFullEmitScreenSize("Vehicle.Particle.FullEmitScreenSize", 0.075f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugParticleFullLifeScreenSize("Vehicle.Particle.FullLifeScreenSize", 0.075f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugParticleDisableAllScreenSize("Vehicle.Particle.DisableAllScreenSize", 0.02f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugParticleDisableFrontScreenSize("Vehicle.Particle.DisableFrontScreenSize", 0.04f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Float g_debugParticleDisableDeferredEventsScreenSize("Vehicle.Particle.DisableDefferedEventsScreenSize", 0.05f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Float g_debugParticleReducedEmitScaleTimer("Vehicle.Particle.ReducedEmitScaleTimer", 2.0f);
Debug::Float g_debugParticleReducedEmitScale("Vehicle.Particle.ReducedEmitScale", 0.5f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Bool g_debugVehicleUpdateHeadlights("Vehicle.Effects.Headlights.UpdateFromSorcery", false);
Debug::Bool g_debugDrawExhaustEmitters("Vehicle.Effects.Exhaust.DrawEmitters", false);

Debug::Bool g_debugEnableParticleTrails("Vehicle.Effects.LightTrails.Enable", true);
Debug::Bool g_debugDrawParticleTrailEmitters("Vehicle.Effects.LightTrails.DrawEmitters", false);

Debug::Bool g_debugFakeDropOut("Vehicle.Debug.Network.FakeDropOut", false);
Debug::Bool g_debugVehicleStatus("Vehicle.Debug.Network.VehicleStatus", false);

Debug::Bool g_debugUpdateVehicleDrawDistances("Vehicle.DrawDistances.Update", false);



Debug::Bool g_debugUpdateWheelScale("Vehicle.Wheels.UpdateScale", false);

Debug::Bool g_debugSetCollisions("Vehicle.Collisions.SetCollisions", false);
Debug::Bool g_debugVehicleCollisions("Vehicle.Collisions.VehicleCollisions", false);

Debug::Float g_debugVehicleBuoyancyCoefficient("Vehicle.Collisions.BuoyancyCoefficient", 1.0f, Debug::Range(0.0f, 2.0f), Debug::Step(0.001f));
Debug::Float g_debugVehicleMaxAngularVelocity("Vehicle.Collisions.MaxAngularVelocity", 10.0f, Debug::Range(0.0f, 100.0f), Debug::Step(0.001f));


Debug::Bool g_debugVehicleUpdateDirt("Vehicle.Effects.DirtLayer.UpdateFromSorcery", false);
Debug::Bool g_debugUpdateDamageInfo("Vehicle.Damage.Code.UpdateDeformationInformationFromSorcery", false);

Debug::Bool g_debugTryAndHandleNetworkJump("Vehicle.TryToHandleNetworkJumpInVehicleActor", false);

// deferred part of the vehicle update
void C_VehicleActor::DeferredUpdate()
{
	m_pVehicleModel->UpdateDeferred();

	switch (m_DeferredResetType)
	{
	case E_ResetType_Position:
		Reset(E_ResetType_Position, m_ResetOnTrackMsg.mNewWorldMatrix, m_ResetOnTrackMsg.mVelocity);
		m_DeferredResetType = E_ResetType_None;
		break;
	case E_ResetType_Spawn:
		Reset(E_ResetType_Spawn);
		m_DeferredResetType = E_ResetType_None;
		break;
	case E_ResetType_Camera:
		Reset(E_ResetType_Camera);
		m_DeferredResetType = E_ResetType_None;
		break;
	default:
		break;
	}

	if (m_pVehicleModel->GetDamageLayer())
	{
		if (m_pVehicleModel->GetDamageLayer()->GetWheel(0)->m_TyreBurst || m_pVehicleModel->GetDamageLayer()->GetWheel(1)->m_TyreBurst)
		{		
			//Bug #12384 - pivot car render matrix about rear-axle for 
			Maths::HVec3 rearAxlePosWS = Maths::Lerp(RPToMaths(&m_pVehiclePhysics->Wheel(2).GetWheelPosition()), RPToMaths(&m_pVehiclePhysics->Wheel(3).GetWheelPosition()), 0.5f);
			RacingPhysics::Axis axis = m_pVehiclePhysics->GetAxis();
			RacingPhysics::Point3R rearAxlePosLS = axis.Local(RacingPhysics::Point3R(rearAxlePosWS.X(), rearAxlePosWS.Y(), rearAxlePosWS.Z()));
			RacingPhysics::Axis axisTransformed = axis.Rotated(axis.AxisX(), RacingPhysics::FloatVR(m_fRearAxlePivotAngle));
			RacingPhysics::Point3R rearAxlePosTransformedLS = axisTransformed.Local(RacingPhysics::Point3R(rearAxlePosWS.X(), rearAxlePosWS.Y(), rearAxlePosWS.Z()));
			RacingPhysics::Vector3R offsetVec = rearAxlePosTransformedLS - rearAxlePosLS;
			offsetVec = axis.Global(offsetVec);
			axisTransformed.SetPosition(axisTransformed.Position() + offsetVec);
			m_Matrix = RPToMaths(&axisTransformed);			
		}
	}

	//m_pVehicleModel->UpdateDeferred();
	if (g_debugTryAndHandleNetworkJump && m_pVehiclePhysics->CheckNetworkJumped())
	{
		RacingPhysics::Axis axis = m_pVehiclePhysics->GetAxis();
		RacingPhysics::Vector3R vel = m_pVehiclePhysics->GetVelocity();
		Reset(E_ResetType_Position, RPToMaths(&axis), RPToMaths(&vel), true);
	}

	// get data in right form for copying to material list
	MathsStorage::MatrixRow44 matrix;
	Maths::Store(Maths::HMat44(m_Matrix), &matrix);

	{
		PERF_EVENT("C_VehicleActor.Update.SetWorldSpheres");
		
		for (U32 iInstance = 0; iInstance < m_pVehicleModel->GetNumModelInstances(); iInstance++)
		{				
			WorldRender::C_ModelInstance* pModelInstance = m_pVehicleModel->GetModelInstance(iInstance);
			pModelInstance->SetWorldSpherePosition(m_Matrix.Translation());
			//pModelInstance->SetElementMatrix(-1, m_Matrix);
		}

	}

	{
		PERF_EVENT("C_VehicleActor.Update.SetMatrices");

		S32 i;
		for(i = 0; i < m_WorldMatrices.Size() - 1; ++i)
		{
			DCBT(0x0, m_WorldMatrices[i+1]);
			DCBT(0x10, m_WorldMatrices[i+1]);
			*m_WorldMatrices[i] = matrix;
			DCBF(0x0, m_WorldMatrices[i]);
			DCBF(0x10, m_WorldMatrices[i]);
		}
		*m_WorldMatrices[i] = matrix;
#if PLATFORM_PC
		if (!m_Blur2PCHookAdded)
		{	// [Blur2PC] render hook for the flares and rear light (see Blur2PCPlaceLights)
			WorldRender::Blur2PCInterpAddRenderHook(&C_VehicleActor::Blur2PCRenderHook, this);
			m_Blur2PCHookAdded = true;
			m_Blur2PCLightsMoved = false;
		}
		{	// [Blur2PC] keep the last two tick matrices for render-time readers (Blur2PCRenderMatrix)
			const U32 tick = WorldRender::Blur2PCInterpCurrentTick();
			if (tick != m_Blur2PCMatrix.tick)
			{
				m_Blur2PCMatrix.prev = (m_Blur2PCMatrix.tick + 1 == tick) ? m_Blur2PCMatrix.cur : m_Matrix;
				m_Blur2PCMatrix.tick = tick;
			}
			m_Blur2PCMatrix.cur = m_Matrix;
		}
		// [Blur2PC] render interpolation: these stores bypass SetElementMatrix, record them too
		for (i = 0; i < m_WorldMatrices.Size(); ++i)
			WorldRender::Blur2PCInterpRecordStore(m_Blur2PCWorldMatrixOwners[i], m_WorldMatrices[i], m_Matrix);
#endif
	}

	{
		PERF_EVENT("C_VehicleActor.Update.UpdateMatrices");
		m_pVehicleModel->GetComponentList().UpdateMatrices(m_Matrix, m_pVisDetect, false, &m_RawMatrix);		
	}


	if (m_IsFocusVehicle || !m_Visible )
		m_pVehicleModel->DisableVisibilityFlags(Game::E_ViewTypeFlag_Mirror);
	else
		m_pVehicleModel->EnableVisibilityFlags(Game::E_ViewTypeFlag_Mirror);


	m_pVehicleModel->SetDynamicShaderParameters();

	SynchData(&m_Matrix, Maths::HVec3());

#ifdef _WHEEL_LOD
	for (S32 iLod = 0; iLod < m_LodNodes.Size(); iLod++)
	{
		m_LodNodes[iLod].Update(AmaxCam::CameraSystem().DebugFeed(0).Live().Position());
	}
#endif

	//Update light flares
	if (!m_IsSplitscreen)
		UpdateLightFlares();

	//Update dynamic lights
	UpdateDynamicLights();

	// Update headlights
	UpdateHeadlights();

	//Update Aux flares
	UpdateAuxFlares();

	UpdateSizeOnScreen();

	const Maths::C_RGBA_F32& lighting = m_pVehicleModel->GetLighting();

	m_ExhaustEffects.Update(m_Matrix, m_FrameTime, lighting, m_ParticleEmitScale, m_EffectsVisiblityFlags);

	//Update engine smoke and fire
	UpdateDamageEffects(lighting);

	//Update trails
	UpdateLightTrails(lighting);

	{
		PERF_EVENT("C_VehicleActor.Update.WheelEffects");

		S32 splashViewport = 0;
		if (m_IsFocusVehicle)
			splashViewport = m_ViewportID;
		else
			splashViewport = -1;

		if (!m_ResetifyDriver)
		for (S32 iWheel = 0; iWheel < m_WheelEffects.Size(); iWheel++)
		{
			const Vehicle::C_WheelDamage* pWheelDamage = m_pVehicleModel->GetDamageLayer()->GetWheel((U32)iWheel);
			bool enableParticles = m_EnableParticles && m_WheelParticleState[iWheel].first;
			float fPScale = m_WheelParticleState[iWheel].second;

			m_WheelEffects[iWheel].UpdateParticles(&m_Matrix, m_FrameTime, lighting, m_ParticleEmitScale * fPScale, m_ParticleLifeScale * fPScale, m_EffectsVisiblityFlags, m_pVehicleModel->GetModelInput()->m_vVelocity);
		}
	}	
}


void DeferredVehicleUpdate(Scheduler::Task* pTask)
{
	PERF_EVENT("VehicleUpdateTask");
	C_VehicleActor *p = *(C_VehicleActor**)pTask->ParamData;
	p->DeferredUpdate();
}

/*
===============
NAME: UpdateBlobShadow

NOTE: UpdateBlobShadow
===============
*/
void C_VehicleActor::UpdateBlobShadow(const Cam2::C_Camera& camera)
{
	PERF_EVENT("UpdateBlobShadow");

	//Update shadows
	if (m_pBlobShadowInstance)
	{	
		//Check if available better ground position from deferred raytrace to improve the blobshadows
		if (g_bDeferredVehicleRaytraces)
		if (m_pGroundRay->m_result.Hit())
		{
			m_GroundPosition = m_pGroundRay->m_result.m_contact;
			U32 numPositions = 1;
			//Get normals from the wheels		
			for (S32 iWheel = 0; iWheel < m_pVehicleModel->GetModelInput()->m_Wheels.Size(); iWheel++)
			{
				if (!m_pVehicleModel->GetModelInput()->m_Wheels[iWheel].m_bOnGround)
					continue;

				m_GroundPosition.Y(m_GroundPosition.Y() + m_pVehicleModel->GetModelInput()->m_Wheels[iWheel].m_vGroundPosition.Y());
				numPositions++;
			}
			m_GroundPosition.Y(m_GroundPosition.Y() / F32(numPositions));
		}

		//Check camera and calculate priority
		Maths::HVec3 cameraRefPosition = camera.ToCameraSpace(m_GroundPosition);
		F32 priority = (cameraRefPosition.Z()) + (Maths::Abs(cameraRefPosition.X()) + Maths::Abs(cameraRefPosition.Y()));
		if (m_IsFocusVehicle)
			priority -= 1000.f;

		//Under water
		F32 underWater = 0.f;
		F32 overallVehicleAlpha = GetAlpha(E_VAC_TYPE_VEHICLE);
		for (S32 iWheel = 0; iWheel < GetVehiclePhysics()->GetNumWheels(); iWheel++)
		{
			underWater += GetVehiclePhysics()->Wheel(iWheel).SurfaceModifierDepth();
		}

		F32 shadowIntensity = Maths::Clamp(1.f - underWater * g_BlobShadowUnderwaterFactor, 0.f, 1.f);
		shadowIntensity *= overallVehicleAlpha;

		m_pBlobShadowInstance->UpdatePosition(m_Matrix, m_GroundPosition, m_GroundNormal, priority, cameraRefPosition.Z(), shadowIntensity);
	}
}

/*
===============
NAME: PausedUpdate

NOTE: PausedUpdate
===============
*/
void C_VehicleActor::PausedUpdate()
{
	if (m_IsFocusVehicle)
		CheckCamera();	

	//Update light flares (visibility)
	if (!m_IsSplitscreen)
		UpdateLightFlares();

	if (m_pVehicleModel)
		m_pVehicleModel->SetDynamicShaderParameters(true);

	/*if(m_EnableParticles && g_debugParticleShowScales)
	{
		Util::C_String text;
		text.Printf("%0.2f %0.1f %0.1f", m_ScreenSize, m_ParticleEmitScale, m_ParticleLifeScale);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(m_Matrix.AW(), text);
	}*/
}

/*
===============
NAME: PostUpdate

NOTE: PostUpdate
===============
*/
void C_VehicleActor::PostUpdate()
{
	HandleDefferedEvents();
}

Scheduler::GroupHandle C_VehicleActor::m_hAudioSchedulerGroup = 0;
Scheduler::GroupHandle C_VehicleActor::m_hRaytraceGroup = 0;

void C_VehicleActor::SetRaytraceGroup( Scheduler::GroupHandle handle )
{
	m_hRaytraceGroup = handle;
}
void C_VehicleActor::SetAudioSchedulerGroup(Scheduler::GroupHandle handle)
{
	m_hAudioSchedulerGroup = handle;
}

Debug::Bool g_bDeferredVehicleAudioUpdate("Audio.Optimisations.VehicleAudioDeferredUpdate", true);

JQ_DECLARE_PPU_MODULE(DeferredVehicleAudioUpdateTask, Game::Actors::DeferredVehicleAudioUpdate);

void DeferredVehicleAudioUpdate(Scheduler::Task* pTask)
{
	PERF_EVENT("DeferredVehicleAudioUpdate");
	Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

	if (!pRLC)
		return;
	
	for (U32 iVehicle = 0; iVehicle < pRLC->GetNumVehicles(); iVehicle++)
	{
		Game::Actors::C_VehicleActor* pActor = pRLC->GetVehicleActor(iVehicle);
		
		if (!pActor)
			continue;

		pActor->AudioUpdate();
	}
}

void C_VehicleActor::UpdateAudio()
{
	if (g_bDeferredVehicleAudioUpdate)
	{
		BIZ_ASSERT(m_hAudioSchedulerGroup != 0); // make sure you set the scheduler group before running the update
		Scheduler::AddTask(DeferredVehicleAudioUpdateTask, 0, 0, m_hAudioSchedulerGroup, 0, NULL, 0);				
	}
	else
	{
		Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

		if (!pRLC)
			return;
		
		for (U32 iVehicle = 0; iVehicle < pRLC->GetNumVehicles(); iVehicle++)
		{
			Game::Actors::C_VehicleActor* pActor = pRLC->GetVehicleActor(iVehicle);
			
			if (!pActor)
				continue;

			pActor->AudioUpdate();
		}
	}	
}

void C_VehicleActor::AudioUpdate() const
{
	PERF_EVENT("AudioUpdate");
	
	Messaging::C_MessageBase msg(E_GameflowMessage_UpdateDeferredAudio, sizeof(Messaging::C_MessageBase));
	for (U32 iComponent = 0; iComponent < GetNumComponents(); iComponent++)
	{
		Actor::I_ActorComponent* pComponent = const_cast<Actor::I_ActorComponent*>(GetComponent(iComponent));
		if (!pComponent)
			continue;
		pComponent->OnMessage(&msg);
	}
	//SendMessageToActor(GetHandle(), E_GameflowMessage_UpdateDeferredAudio);
}

static Util::C_Random g_EMPRandom;
static int g_NumFrameBeforeCreateConstraints = 2;
static int g_ConstraintsFrameCount = 0;
Debug::Bool g_debugVehiclePrintPhysicsParts("Vehicle.Debug.Output.PrintPhysicsParts", false);
Debug::Bool g_debugVehicleRecreateConstraints("Vehicle.Parts.RecreateConstraints", false);
Debug::Bool g_debugCreateConstraints("Vehicle.DoNotTouchMe.CreateConstraints", false);

Debug::Float g_debugVehicleGravity("Vehicle.Gravity", -9.8f, Debug::Range(-20.0f, 20.0f), Debug::Step(0.001f));

Debug::Int g_debugFramesUntilPhysicsCanIgnoreCollision("Gravity.CollisionDamping.FramesUntilCanIgnoreCollision", 10, Debug::Range(0, 60), Debug::Step(1));

Debug::Bool g_debugTestCollisionDamping("Vehicle.TestCollisionDamping", false);

void C_VehicleActor::SanityCheckPosition()
{
	//SANITY CHECK
	if (!m_Matrix.AW().Finite())
	{
		BIZ_ASSERT_FAIL("Vehicle position is INFINITE!");
		LOGERROR(g_VehicleActorLog, "Vehicle position is INFINITE! Resetting at spawn position");
		Reset(E_ResetType_Spawn);
	}
	else if (!Maths::Abs(m_Matrix.AW()).All_LT(Maths::HVec3(10000.0f, 10000.0f, 10000.0f)))
	{
		BIZ_ASSERT_FAIL("Vehicle position is MASSIVE!");
		LOGERROR(g_VehicleActorLog, "Vehicle position is MASSIVE! Resetting at spawn position");
		Reset(E_ResetType_Spawn);
	}
	else if (m_Matrix.AW().Y().LT(m_KillPlaneHeight))
	{
		BIZ_ASSERT_FAIL("Vehicle fell out of the WORLD!\n(%.1f,%.1f,%.1f)", (float)m_LastKnownGroundPosition.X(), (float)m_LastKnownGroundPosition.Y(), (float)m_LastKnownGroundPosition.Z());
		LOGERROR(g_VehicleActorLog, "Vehicle fell out of the WORLD! (%.1f,%.1f,%.1f) Resetting at spawn position", (float)m_LastKnownGroundPosition.X(), (float)m_LastKnownGroundPosition.Y(), (float)m_LastKnownGroundPosition.Z());
		Reset(E_ResetType_Spawn);
	}
}

Debug::Bool g_debugVehicleActorMasterDebug("Vehicle.Debug.MasterDebugSwitch_Actor", false, Debug::Persist());
Debug::Int g_debugVehicleActorMasterDebugVehicle("Vehicle.Debug.MasterDebugSwitch_ActorNumber", -1, Debug::Range(-1, 19));

Debug::Bool g_debugBodyShakeGo("Vehicle.BodyShake.Go", false);
Debug::Float g_debugBodyShakeScaleX("Vehicle.BodyShake.RangeX", 0.025f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugBodyShakeScaleY("Vehicle.BodyShake.RangeY", 0.01f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugBodyShakeScaleZ("Vehicle.BodyShake.RangeZ", 0.05f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugBodyShakeNoiseScale("Vehicle.BodyShake.Frequency", 20.0f, Debug::Range(0.0f, 5.0f), Debug::Step(0.001f));
Debug::Float g_debugBodyShakeDuration("Vehicle.BodyShake.Duration", 3.0f, Debug::Range(0.0f, 5.0f), Debug::Step(0.001f));

Debug::Bool g_debugDamageBodyOverride("Vehicle.BodyShake.Damage.Override", false);
Debug::Float g_debugDamageBodyShakeScale("Vehicle.BodyShake.Damage.DamageShakeScale", 0.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugDamageBodyShakeScaleX("Vehicle.BodyShake.Damage.RangeX", 0.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugDamageBodyShakeScaleY("Vehicle.BodyShake.Damage.RangeY", 0.005f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugDamageBodyShakeScaleZ("Vehicle.BodyShake.Damage.RangeZ", 0.02f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugDamageBodyShakeNoiseScale("Vehicle.BodyShake.Damage.Frequency", 15.0f, Debug::Range(0.0f, 5.0f), Debug::Step(0.001f));

Debug::Float g_debugDamageBodyShakeMinSpeed("Vehicle.BodyShake.Damage.MinVehicleSpeed", 20.0f, Debug::Range(0.0f, 100.0f), Debug::Step(0.001f));
Debug::Float g_debugDamageBodyShakeSpeedRange("Vehicle.BodyShake.Damage.MaxVehicleSpeedRange", 130.0f, Debug::Range(0.0f, 100.0f), Debug::Step(0.001f));

Debug::Bool g_debugPartShakeOverride("Vehicle.BodyShake.Damage.Parts.OverrideParams", false);
Debug::Float g_debugPartVibrationMinSpeed("Vehicle.BodyShake.Damage.Parts.MinVehicleSpeed", 10.0f, Debug::Range(0.0f, 100.0f), Debug::Step(0.001f));
Debug::Float g_debugPartVibrationSpeedRange("Vehicle.BodyShake.Damage.Parts.MaxVehicleSpeedRange", 80.0f, Debug::Range(0.0f, 100.0f), Debug::Step(0.001f));
Debug::Float g_debugPartAmbientTorque("Vehicle.BodyShake.Damage.Parts.AmbientTorque", 100.0f, Debug::Range(0.0f, 500.0f), Debug::Step(0.001f));
Debug::Float g_debugPartSurfaceTorqueScale("Vehicle.BodyShake.Damage.Parts.SurfaceTorqueScale", 65.0f, Debug::Range(0.0f, 500.0f), Debug::Step(0.001f));

void C_VehicleActor::BodyShake()
{
	PERF_EVENT("C_VehicleActor.BodyShake");

	m_RawMatrix = m_Matrix;

	C_RaceLogicLayer* pRaceLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	if (!pRaceLogicLayer)
		return;
	const C_VehicleLogicActor* pLogicActor = pRaceLogicLayer->GetLogicVehicleActor(m_VehicleIndex);

	if (!pLogicActor)
		return;

	const RLHealthLevelZone* pHealthZone = pLogicActor->GetCurrentHealthZone();

	if (!m_BodyShakeDamage.m_bShaking)
		m_BodyShakeDamage.Start();

	if (g_debugDamageBodyOverride)
	{
		m_BodyShakeDamage.m_fFrequency	= g_debugDamageBodyShakeNoiseScale;
		m_BodyShakeDamage.m_vRange		= MathsStorage::Float3(g_debugDamageBodyShakeScaleX, g_debugDamageBodyShakeScaleY, g_debugDamageBodyShakeScaleZ);
	}
	else
	{
		m_BodyShakeDamage.m_fScale		= pHealthZone->mBodyShake.mSpeedRange > 0.0f ? Maths::Clamp((m_pVehicleModel->GetModelInput()->m_fSpeed - pHealthZone->mBodyShake.mMinSpeed) / pHealthZone->mBodyShake.mSpeedRange, 0.0f, 1.0f) : 0.0f;
		m_BodyShakeDamage.m_fFrequency	= pHealthZone->mBodyShake.mFrequency;
		m_BodyShakeDamage.m_vRange		= MathsStorage::Float3(pHealthZone->mBodyShake.mShakeRange.vx, pHealthZone->mBodyShake.mShakeRange.vy, pHealthZone->mBodyShake.mShakeRange.vz);
	}

	/*if (g_debugPartShakeOverride)
	{
		m_pVehicleModel->GetModelInput()->m_PartShakeParams.m_MinSpeed		= (float)g_debugPartVibrationMinSpeed;
		m_pVehicleModel->GetModelInput()->m_PartShakeParams.m_SpeedRange	= (float)g_debugPartVibrationSpeedRange;
		m_pVehicleModel->GetModelInput()->m_PartShakeParams.m_Torque		= (float)g_debugPartVibrateTorque;
	}
	else
	{
		m_pVehicleModel->GetModelInput()->m_PartShakeParams.m_MinSpeed		= pHealthZone->mPartShake.mMinSpeed;
		m_pVehicleModel->GetModelInput()->m_PartShakeParams.m_SpeedRange	= pHealthZone->mPartShake.mSpeedRange > 0.0f ? pHealthZone->mPartShake.mSpeedRange : 0.0f;
		m_pVehicleModel->GetModelInput()->m_PartShakeParams.m_Torque		= pHealthZone->mPartShake.mTorque;
	}*/

	F32 surfaceRoughness = 0.f;
	for(U32 a = 0; a < 4; ++a)
	{
		const RacingPhysics::CarWheel& wheel = m_pVehiclePhysics->Wheel(0);
		if(wheel.OnGround())
		{
			surfaceRoughness = Maths::Max(wheel.SurfaceRoughnessScale(), surfaceRoughness);	
		}
	}
	F32 ambientTorque = g_debugPartAmbientTorque;
	F32 surfaceTorqueScale = g_debugPartSurfaceTorqueScale;
	m_pVehicleModel->GetModelInput()->m_PartShakeParams.m_MinSpeed		= pHealthZone->mPartShake.mMinSpeed;
	m_pVehicleModel->GetModelInput()->m_PartShakeParams.m_SpeedRange	= pHealthZone->mPartShake.mSpeedRange > 0.0f ? pHealthZone->mPartShake.mSpeedRange : 0.0f;
	m_pVehicleModel->GetModelInput()->m_PartShakeParams.m_Torque		= ambientTorque + surfaceTorqueScale * surfaceRoughness;


	if (g_debugDamageBodyShakeScale > 0.0f)
		m_BodyShakeDamage.m_fScale		= g_debugDamageBodyShakeScale;
	
	if (g_debugBodyShakeGo)
	{
		g_debugBodyShakeGo	= false;

		if (m_BodyShake.m_bShaking)
			m_BodyShake.Extend(g_debugBodyShakeDuration);
		else
		{
			m_BodyShake.m_fDuration		= g_debugBodyShakeDuration;
			m_BodyShake.m_fFrequency	= g_debugBodyShakeNoiseScale;
			m_BodyShake.m_vRange		= MathsStorage::Float3(g_debugBodyShakeScaleX, g_debugBodyShakeScaleY, g_debugBodyShakeScaleZ);
			m_BodyShake.Start();
		}
	}
	
	m_BodyShakeDamage.Update(m_Matrix, m_FrameTime);
	m_BodyShake.Update(m_Matrix, m_FrameTime);	
}


Debug::Float g_debugGripLossScale2("Vehicle.GripLoss.Scale", 0.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugGripRecoverTimer("Vehicle.GripLoss.GripRecoverTimer", 0.75f, Debug::Range(0.0f, 5.0f), Debug::Step(0.001f));
Debug::Float g_debugGripLossTimer("Vehicle.GripLoss.GripLossTimer", 0.25f, Debug::Range(0.0f, 5.0f), Debug::Step(0.001f));

Debug::Bool g_debugDrawNoGripLoss2("Vehicle.GripLoss.DrawNoGripLoss", false, Debug::Persist());
Debug::Bool g_debugDrawGripLoss2("Vehicle.GripLoss.DrawGripLoss", false, Debug::Persist());


void C_VehicleActor::UpdateGripLoss()
{
	const float gripLossScale = (float)g_debugGripLossScale2;
	const float gripRecoverTimer = (float)g_debugGripRecoverTimer;
	const float gripLossTimer = (float)g_debugGripLossTimer;
	const float gripLossDuration = gripRecoverTimer + gripLossTimer;

	if (m_bLostGrip)
	{
		m_bLostGrip	= false;
		m_fLostGripTime = 0.f;
		m_pVehiclePhysics->PerformanceModifiers().transverseGripScale = gripLossScale;
	}

	m_fLostGripTime += m_FrameTime;
	
	if (m_fLostGripTime > gripLossDuration)
	{
		//No grip loss any more, reset the transversegripscale
		m_pVehiclePhysics->PerformanceModifiers().transverseGripScale =1.f;
	}
	else
	{

		//grip loss based on the scale
		if(m_fLostGripTime < gripLossTimer  && m_pVehiclePhysics->PerformanceModifiers().transverseGripScale < 1.f)
		{
			m_pVehiclePhysics->PerformanceModifiers().transverseGripScale = gripLossScale;
			if (g_debugDrawGripLoss2)
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(m_Matrix.AW(), Maths::HVec3(0.0f, 1.0f, 0.0f), 2.0f, Maths::RGBA(255, 0, 0));
		}
		else
		{
			//Going to the grip recovery 
			if(m_pVehiclePhysics->PerformanceModifiers().transverseGripScale < 1.f)
			{
				const float ratio = m_fLostGripTime/gripLossDuration;
				m_pVehiclePhysics->PerformanceModifiers().transverseGripScale = gripLossScale +  (1.f - gripLossScale)*ratio;
				if (g_debugDrawGripLoss2)
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(m_Matrix.AW(), Maths::HVec3(0.0f, 1.0f, 0.0f), 2.0f, Maths::RGBA(255, 255, 0));
			}
		}
	}

	if (g_debugDrawNoGripLoss2 && m_pVehiclePhysics->PerformanceModifiers().transverseGripScale == 1.f)
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(m_Matrix.AW(), Maths::HVec3(0.0f, 1.0f, 0.0f), 2.0f, Maths::RGBA(0, 255, 0));
	
}


AmaxCam::E_VFCamera	C_VehicleActor::GetCameraType(int viewport) const
{
	return AmaxCam::CameraSystem().VehicleFeed(viewport)->CurrentType();
}

//Debug::Bool g_debugSetHideCar("Vehicle.SetVisible", false);
//Debug::Bool g_debugCarVisible("Vehicle.Visible", false);

Debug::Bool g_UpdateGraphicalActorGhostiness("Vehicle.Ghost.GraphicalActor.UpdateGhostiness", false);
Debug::Bool g_GraphicalActorGhost("Vehicle.Ghost.GraphicalActor.Ghost", false);
Debug::Int g_GraphicalActorVehicleIndexToGhost("Vehicle.Ghost.GraphicalActor.IndexToGhost", 0, Debug::Range(0, 19), Debug::Step(1));
Debug::Float g_GhostedAlpha("Vehicle.Ghost.GraphicalActor.GhostedAlpha", 0.6f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_DeGhostedAlpha("Vehicle.Ghost.GraphicalActor.DeGhostedAlpha", 1.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_GhostTransitionTime("Vehicle.Ghost.GraphicalActor.TransitionTime", 0.50f, Debug::Range(0.0f, 5.0f), Debug::Step(0.1f));
Debug::Bool g_MoveFullyGhostedCarOutOfAlphaPass("Vehicle.Ghost.GraphicalActor.MoveFullyGhostedCarOutOfAlphaPass", true);
Debug::Bool g_HideFullyGhostedCar("Vehicle.Ghost.GraphicalActor.HideFullyGhostedCar", true);

Debug::Bool g_Perks_FakePhysics_Spin360Left("Perks.FakePhysics.Spin360.SpinLeft", false);
Debug::Bool g_Perks_FakePhysics_Spin360Right("Perks.FakePhysics.Spin360.SpinRight", false);

Debug::Bool g_Perks_FakePhysics_Spin45Left("Perks.FakePhysics.Spin45.SpinLeft", false);
Debug::Bool g_Perks_FakePhysics_Spin45Right("Perks.FakePhysics.Spin45.SpinRight", false);

Debug::Bool g_Perks_FakePhysics_FlipForward("Perks.FakePhysics.Flip.FlipForward", false);
Debug::Bool g_Perks_FakePhysics_FlipBackward("Perks.FakePhysics.Flip.FlipBackward", false);

Debug::Bool g_Perks_FakePhysics_RollLeft("Perks.FakePhysics.Roll.RollLeft", false);
Debug::Bool g_Perks_FakePhysics_RollRight("Perks.FakePhysics.Roll.RollRight", false);

Debug::Bool g_Perks_FakePhysics_BargeJinkLeft("Perks.FakePhysics.JinkBarge.JinkLeft", false);
Debug::Bool g_Perks_FakePhysics_BargeJinkRight("Perks.FakePhysics.JinkBarge.JinkRight", false);

Debug::Bool g_Perks_FakePhysics_ShieldJinkLeft("Perks.FakePhysics.JinkShield.JinkLeft", false);
Debug::Bool g_Perks_FakePhysics_ShieldJinkRight("Perks.FakePhysics.JinkShield.JinkRight", false);

Debug::Bool g_Perks_FakePhysics_NitrousJinkLeft("Perks.FakePhysics.JinkNitrous.JinkLeft", false);
Debug::Bool g_Perks_FakePhysics_NitrousJinkRight("Perks.FakePhysics.JinkNitrous.JinkRight", false);

Debug::Bool g_Perks_FakePhysics_TipForward("Perks.FakePhysics.Tip.TipForward", false);
Debug::Bool g_Perks_FakePhysics_TipBackward("Perks.FakePhysics.Tip.TipBackward", false);

Debug::Bool g_Perks_FakePhysics_SpeedUp("Perks.FakePhysics.Speed.SpeedUp", false);
Debug::Bool g_Perks_FakePhysics_SlowDown("Perks.FakePhysics.Speed.SlowDown", false);

Debug::Bool g_debugLandingTriggerEffect		("Vehicle.Effects.Landing.TriggerEffect",		false);
Debug::Bool g_debugLandingEffectDust		("Vehicle.Effects.Landing.TriggerDust",			false);
Debug::Bool g_debugLandingEffectSparks		("Vehicle.Effects.Landing.TriggerSparks",		false);
Debug::Bool g_debugLandingEffectWaterSplash	("Vehicle.Effects.Landing.TriggerWaterSplash",	false);
Debug::Bool g_debugLandingEffectWaterBob	("Vehicle.Effects.Landing.TriggerWaterBob",		false);
Debug::Bool g_debugLandingTriggerCameraEffect("Vehicle.Effects.Landing.TriggerCameraEffect",false);
Debug::Float g_debugLandingCameraImpulseValue("Vehicle.Effects.Landing.CameraImpulse",		1.0f, Debug::Range(0.0f, 1.0f));


Debug::Float g_debugLandingEffectSuspensionDistance	("Vehicle.Effects.Landing.SuspensionDistanceToTrigger", 0.2f,		Debug::Range(0.0f, 0.5f),		Debug::Step(0.001f));
Debug::Float g_debugLandingEffectGroundGorce		("Vehicle.Effects.Landing.GroundForceToTrigger",		9000.0f,	Debug::Range(0.0f, 15000.0f),	Debug::Step(1.0f));
Debug::Float g_debugLandingEffectYOffset			("Vehicle.Effects.Landing.YOffset",						0.1f,		Debug::Range(0.0f, 0.5f),		Debug::Step(0.001f));

#ifndef CFG_GOLD
Debug::Bool g_debugVehicleCollisionDrawMainShape	("Vehicle.Collision.DrawMainShape",		false);
Debug::Bool g_debugVehicleCollisionDrawVehicleShape	("Vehicle.Collision.DrawVehicleShape",	false);
Debug::Bool g_debugVehicleCollisionDrawObjectShape	("Vehicle.Collision.DrawObjectShape",	false);
Debug::Bool g_debugVehicleCollisionDrawPartShape    ("Vehicle.Collision.DrawPartShape",     false);
#endif

void C_VehicleActor::CheckCamera(bool force)
{
	if (!C_Game::GetInstance()->GetLevelState().mInUI)
	{
		const C_VehicleActorCreate *pVehicleCreate = (const C_VehicleActorCreate *)(GetCreationData().GetData());
		S32 viewportIndex = pVehicleCreate->m_pRLC->GetViewportIndex(m_VehicleIndex);

		const bool ghosted = Ghosted();
		const float vehicleAlpha = GetAlpha(E_VAC_TYPE_VEHICLE);

		m_ViewportID = viewportIndex;
		U32 viewportFlags;
		if (m_ViewportID == -1)
			viewportFlags = 0x0F;
		else
			viewportFlags = 1 << m_ViewportID;

		if (AmaxCam::CameraSystem().GameViewIsInsideCar(viewportIndex))
		{
			m_IsBumperCamera = true;

			//Check to see if the vehicle isn't hidden, if so hide it
			if (((m_Visible & viewportFlags) && !m_VisibilityOverride)  || force)
			{
				DisplayDriver(false, viewportIndex);
				SetVisibility(false, viewportIndex, force);
			}
			m_EffectsVisiblityFlags = 0xFFFFFFFF & (~(1 << (E_ViewType_Main1 + viewportIndex)));
		}
		else
		{
			m_IsBumperCamera = false;

			//Check to see if the vehicle is hidden (and a valid alpha), if so show it
			if ( (!(m_Visible & viewportFlags) && (vehicleAlpha > 0.f) && !m_VisibilityOverride) || force)
			{
				if (!ghosted)
					DisplayDriver(true);

				SetVisibility(true, viewportIndex, force);
			}
			m_EffectsVisiblityFlags = 0xFFFFFFFF;
		}
	}
	else
		m_EffectsVisiblityFlags = 0xFFFFFFFF;
}

Debug::Bool g_debugHandleTunneling("Vehicle.Collisions.HandleTunneling", true);
Debug::Float g_dbConstraintTimeOnGround("Vehicle.GamePhysicsMechanics.Constraint.ConstraintTimeOnGround", 2.f, Debug::Range(0.f, 10.f));
Debug::Bool g_dbDrawConstraintState("Vehicle.GamePhysicsMechanics.Constraint.DrawConstraint", false);
Debug::Float g_dbConstraintLowSpeedThreshold("Vehicle.GamePhysicsMechanics.Constraint.LowSpeedThreshold", 10.f, Debug::Range(0.f, 100.f));

void C_VehicleActor::UpdateConstraint(const F32 timeStep)
{
	PERF_EVENT("C_VehicleActor.UpdateConstraint");

	if(m_pConstraint)
	{
		SharedPhysics::C_RigidBody& rigid(m_pCollisionBody->GetBody());
		const Maths::HF32 velocity(rigid.m_LinearVelocity|rigid.m_LinearVelocity);
		const Maths::HF32 lowSpeed((float)g_dbConstraintLowSpeedThreshold);
		//We have a constraint that needs updating...
		if(m_FakeVehiclePhysics.IsActive() || m_ConstraintFatallyDamaged || m_HeavyCollision)
		{
			m_HeavyCollision = false;
			m_pConstraint->SetScale(0.f);
			m_GroundedTimer = 0.f;
		}
		else  if(m_pVehiclePhysics->NumWheelsOnGround() >= 2)
		{
			m_GroundedTimer += timeStep;
		}
		else
		{
			m_GroundedTimer = 0.f;
		}

		if(velocity < (lowSpeed * lowSpeed))
		{
			m_pConstraint->SetScale(0.f);
		}
		else if(m_GroundedTimer >= g_dbConstraintTimeOnGround)
		{
			m_pConstraint->SetScale(1.f);
		}

		if(g_dbDrawConstraintState)
		{
			if(m_pConstraint->m_Scale > 0.f)
			{
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(m_Matrix.AW(), Maths::HVec3(0.0f, 1.0f, 0.0f), 2.0f, Maths::RGBA(0, 255, 0));
			}
			else
			{
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(m_Matrix.AW(), Maths::HVec3(0.0f, 1.0f, 0.0f), 2.0f, Maths::RGBA(255, 0, 0));
			}
		}
	}
}


void C_VehicleActor::UpdateVehicleAirFrictionCoefficient(const LevelStateCar& levelStateCar)
{
	PERF_EVENT("C_VehicleActor.UpdateVehicleAirFrictionCoefficient");

	if(m_pVehiclePhysics->NumWheelsOnGround() == 0 && 
		m_pConstraint != NULL && m_pConstraint->m_Scale > 0.f)
	{
		AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];

		//float airFrictionCoefficient = Maths::Select(pVehicle->overwriteAirFrictionCoefficient, pVehicle->m_AirFrictionCoefficient, g_debugAirFrictionValue);
		float airFrictionCoefficient = Maths::Select(pVehicle->overwriteAirFrictionCoefficient, pVehicle->m_AirFrictionCoefficient, C_GameVehicleData::Instance().mVehicleDefaults[0]->defaultAirFrictionCoefficient);
		if(levelStateCar.mUpgrades.AdditionalHP())
		{
			airFrictionCoefficient *= g_SpeedBoostUpgadeAirFriction;
		}
		const Maths::HVec3 angVel = m_pCollisionBody->GetBody().m_AngularMomentum;
		const Maths::HVec3 torque(-airFrictionCoefficient *  angVel);
		m_pCollisionBody->AddTorque(torque); 
		
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(m_pCollisionBody->GetBody().m_Position, Maths::Constants<Maths::HVec3>::UnitAxisPositiveY(), 2.f, Maths::RGBA(0, 0, 255));
	}
}

void C_VehicleActor::Update()
{
	if (!m_Created || !m_pVehicleModel)
		return;

	PERF_EVENT("VehicleActor.Update"); 

	m_pVehicleModel->Update(m_FrameTime);

	HandleCollisionStates();

#if !defined (PLATFORM_SPU)
	//Do tunneling recovery!!!
	if (g_debugHandleTunneling)
		this->HandleTunneling();
#endif

#if !defined(PLATFORM_SPU) && !defined(CFG_GOLD)
	if (Game::g_SorceryRecordsModified && m_pVehiclePhysics)
	{
		AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];		
		if (pVehicle && pVehicle->dynamics)
		{
			Btl::String currentName = m_pVehiclePhysics->GetFilename();
			Btl::String newName = pVehicle->dynamics->name.Text();

			if (stricmp(currentName.Chars(), newName.Chars()) != 0)
			{
				m_pVehiclePhysics->SetFilename(pVehicle->dynamics->name.Text());
				RacingPhysics::XtReloadVehicles();
			}
		}
	}
#endif

#ifndef CFG_GOLD
	if( g_wreckedImpulseForceApply )
	{
		g_wreckedImpulseForceApply = false;
		ApplyWreckedImpulse();
	}
#endif

	m_overide_controls = false;						// Clear any cutscene control overrides

	if (m_pVehiclePhysics)
	{
		AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];

		float rollingFrictionScale = (g_applyRollingFrictionScale) ? pVehicle->surfaceRollingFrictionScale : 1.f;

		m_pVehiclePhysics->PerformanceModifiers().surfaceRollingFrictionScale = rollingFrictionScale;
	}

	if (m_pVehiclePhysics && g_debugVehicleUpdateCentreOfMass)
	{
		m_pVehiclePhysics->SetCopyMassVariablesToGravity();
		m_pVehiclePhysics->CopyToGravity();		
		m_pVehiclePhysics->UnsetCopyMassVariablesToGravity();
	}

	U8 data[1024 * sizeof(void*)];
	SharedPhysics::C_SafeStack<true> physicsStack(data, 1024);

	m_FramesInWater--;
	if (m_FramesInWater == 0)
	{
		if(DynamicEffect::Particle::C_EventInstance* pEvent = m_WaterBobParticleEffect.Object())
		{
			pEvent->SetIntensity(0.0f);
			pEvent->Free();
		}
	}
	else if (m_FramesInWater < 0)
		m_FramesInWater = 0;

	if (m_pVehiclePhysics && m_pVehiclePhysics->NumWheelsOnGround() == 4)
	{
		m_LastKnownSafeMatrix = m_Matrix;
#ifndef CFG_GOLD
		m_LastKnownGroundPosition = m_Matrix.AW();
#endif
	}	

	if (g_UpdateGraphicalActorGhostiness)
	{
		if (m_VehicleIndex == g_GraphicalActorVehicleIndexToGhost)
		{
			if (g_GraphicalActorGhost)
			{
				//Ghosting
				Ghost(g_GhostTransitionTime, g_GhostedAlpha);
			}
			else
			{
				//Deghosting
				Deghost(g_GhostTransitionTime, g_DeGhostedAlpha);
			}

			g_UpdateGraphicalActorGhostiness = false;
		}
	}

#ifndef CFG_GOLD
	if (m_pCollisionBody)
	{		
		Collision::C_CollisionFilter filter;
		Collision::C_CompositeShape* pComp = (Collision::C_CompositeShape*)m_pCollisionBody->GetShape();
		
		if (g_debugVehicleCollisionDrawMainShape)
		{
			Collision::C_CompositeShapeIterator iter(pComp);
			iter.NextShape();
			Gravity::DrawCompositeShape(DbUtil::DefaultDebugRenderSys()->Deferred(), iter.GetCurrentShape<Collision::C_CompositeShape>(), physicsStack,	&filter);
		}
		if (g_debugVehicleCollisionDrawVehicleShape && m_CarColliderIndex != -1)
		{
			Collision::C_CompositeShapeIterator iter(pComp);
			iter.NextShape();
			for(U32 a = 0; a < (U32)m_CarColliderIndex; ++a)
			{
				iter.NextShape();
			}
			Gravity::DrawCompositeShape(DbUtil::DefaultDebugRenderSys()->Deferred(), iter.GetCurrentShape<Collision::C_CompositeShape>(), physicsStack,	&filter);
		}
		if (g_debugVehicleCollisionDrawObjectShape && m_ObjectColliderIndex != -1)
		{
			Collision::C_CompositeShapeIterator iter(pComp);
			iter.NextShape();
			for(U32 a = 0; a < (U32)m_ObjectColliderIndex; ++a)
			{
				iter.NextShape();
			}
			Gravity::DrawCompositeShape(DbUtil::DefaultDebugRenderSys()->Deferred(), iter.GetCurrentShape<Collision::C_CompositeShape>(), physicsStack,	&filter);
		}

		if(g_debugVehicleCollisionDrawPartShape)
		{
			Collision::C_CollisionFilter allSolid;
			Vehicle::C_PartList& list = this->GetVehicleModel()->GetComponentList();
			for(int i=0; i<list.m_Parts.Size(); ++i)
			{
				Vehicle::C_Part* part = list.m_ActiveParts[i].m_pNode;
				
				if(part->m_pPhysicsBody && part->m_pPhysicsBody->GetShape() && part->m_pPhysicsBody->GetShape()->m_ShapeType == Collision::E_Composite_Shape)
				{
					Collision::C_CompositeShape* pShape = (Collision::C_CompositeShape*)part->m_pPhysicsBody->GetShape();
					if(pShape->m_NumShapes > 0)
					{
						Gravity::DrawCompositeShape(DbUtil::DefaultDebugRenderSys()->Deferred(),pShape, physicsStack,	&allSolid);
					}
				}
				
			}
		}

	}
#endif

	m_GravityConstants.m_Gravity = Maths::HVec3(0.f, (float)g_debugVehicleGravity, 0.f);

	if(m_VehicleIndex == 0)
	{
		if(g_Perks_FakePhysics_Spin360Left)		{ g_Perks_FakePhysics_Spin360Left = false;		GetFakePhysics()->Start(Actors::E_FAKE_PHYSICS_SPIN_360_LEFT);		}
		if(g_Perks_FakePhysics_Spin360Right)	{ g_Perks_FakePhysics_Spin360Right = false;		GetFakePhysics()->Start(Actors::E_FAKE_PHYSICS_SPIN_360_RIGHT);		}
		if(g_Perks_FakePhysics_Spin45Left)		{ g_Perks_FakePhysics_Spin45Left = false;		GetFakePhysics()->Start(Actors::E_FAKE_PHYSICS_SPIN_45_LEFT);		}
		if(g_Perks_FakePhysics_Spin45Right)		{ g_Perks_FakePhysics_Spin45Right = false;		GetFakePhysics()->Start(Actors::E_FAKE_PHYSICS_SPIN_45_RIGHT);		}
		if(g_Perks_FakePhysics_FlipForward)		{ g_Perks_FakePhysics_FlipForward = false;		GetFakePhysics()->Start(Actors::E_FAKE_PHYSICS_FLIP_FORWARD);		}
		if(g_Perks_FakePhysics_FlipBackward)	{ g_Perks_FakePhysics_FlipBackward = false;		GetFakePhysics()->Start(Actors::E_FAKE_PHYSICS_FLIP_BACKWARD);		}
		if(g_Perks_FakePhysics_RollLeft)		{ g_Perks_FakePhysics_RollLeft = false;			GetFakePhysics()->Start(Actors::E_FAKE_PHYSICS_ROLL_LEFT);			}
		if(g_Perks_FakePhysics_RollRight)		{ g_Perks_FakePhysics_RollRight = false;		GetFakePhysics()->Start(Actors::E_FAKE_PHYSICS_ROLL_RIGHT);			}
		if(g_Perks_FakePhysics_BargeJinkLeft)	{ g_Perks_FakePhysics_BargeJinkLeft = false;	GetFakePhysics()->Start(Actors::E_FAKE_PHYSICS_JINK_BARGE_LEFT);	}
		if(g_Perks_FakePhysics_BargeJinkRight)	{ g_Perks_FakePhysics_BargeJinkRight = false;	GetFakePhysics()->Start(Actors::E_FAKE_PHYSICS_JINK_BARGE_RIGHT);	}
		if(g_Perks_FakePhysics_ShieldJinkLeft)	{ g_Perks_FakePhysics_ShieldJinkLeft = false;	GetFakePhysics()->Start(Actors::E_FAKE_PHYSICS_JINK_SHIELD_LEFT);	}
		if(g_Perks_FakePhysics_ShieldJinkRight)	{ g_Perks_FakePhysics_ShieldJinkRight = false;	GetFakePhysics()->Start(Actors::E_FAKE_PHYSICS_JINK_SHIELD_RIGHT);	}
		if(g_Perks_FakePhysics_NitrousJinkLeft)	{ g_Perks_FakePhysics_NitrousJinkLeft = false;	GetFakePhysics()->Start(Actors::E_FAKE_PHYSICS_JINK_NITROUS_LEFT);	}
		if(g_Perks_FakePhysics_NitrousJinkRight){ g_Perks_FakePhysics_NitrousJinkRight = false;	GetFakePhysics()->Start(Actors::E_FAKE_PHYSICS_JINK_NITROUS_RIGHT);	}
		if(g_Perks_FakePhysics_TipForward)		{ g_Perks_FakePhysics_TipForward = false;		GetFakePhysics()->Start(Actors::E_FAKE_PHYSICS_TIP_FORWARD);		}
		if(g_Perks_FakePhysics_TipBackward)		{ g_Perks_FakePhysics_TipBackward = false;		GetFakePhysics()->Start(Actors::E_FAKE_PHYSICS_TIP_BACKWARD);		}
		if(g_Perks_FakePhysics_SpeedUp)			{ g_Perks_FakePhysics_SpeedUp = false;			GetFakeSpeed()->Start(Actors::E_FAKE_SPEED_ACCELERATE, 200.0f);		}
		if(g_Perks_FakePhysics_SlowDown)		{ g_Perks_FakePhysics_SlowDown = false;			GetFakeSpeed()->Start(Actors::E_FAKE_SPEED_BRAKE, 25.0f);			}
	}	

	m_IsFocusVehicle = false;
	const C_VehicleActorCreate *pVehicleCreate = (const C_VehicleActorCreate *)(GetCreationData().GetData());
	{
		S32 viewportIndex = pVehicleCreate->m_pRLC->GetViewportIndex(m_VehicleIndex);
		m_IsFocusVehicle = viewportIndex != -1;
	}

	// disable all particles if smaller than this
	m_EnableParticles = (m_ScreenSize > g_debugParticleDisableAllScreenSize);
	// disable front wheels if smaller than this
	if(m_ScreenSize < g_debugParticleDisableFrontScreenSize)
		SetWheelParticleLod(false, false, true, true, 1.0f, 1.0f, 1.0f, 1.0f);
	else
		SetWheelParticleLod(true, true, true, true, 1.0f, 1.0f, 1.0f, 1.0f);
	// disable deferred particles if smaller than this
	m_EnableDeferredEventParticles = m_ScreenSize > g_debugParticleDisableDeferredEventsScreenSize;

	// calculate scales based on screen size on screen
	if(g_debugParticleScaleEnable)
	{
		// we calculate an emission rate modifier and particle lifetime modifier based on the vehicles size on screen
		F32 screenSizeMinusDisable = m_ScreenSize - g_debugParticleDisableAllScreenSize;
		// particle emit scale goes from 1.f to 0.f as screensize goes from g_debugParticleDisableAllScreenSize to g_debugParticleFullEmitScreenSize
		m_ParticleEmitScale = Maths::Clamp(screenSizeMinusDisable / g_debugParticleFullEmitScreenSize, 0.0f, 1.0f);
		m_ParticleEmitScale = Maths::Min(m_ParticleEmitScale, m_pGameWorld->GetVehicleParticleEmitScale());
		// particle life scale goes from 1.f to 0.f as screensize goes from g_debugParticleDisableAllScreenSize to g_debugParticleFullLifeScreenSize
		m_ParticleLifeScale = Maths::Clamp(screenSizeMinusDisable / g_debugParticleFullLifeScreenSize, 0.5f, 1.0f);
		m_ParticleLifeScale = Maths::Min(m_ParticleLifeScale, m_pGameWorld->GetVehicleParticleLifeScale());
	}
	else
	{
		m_ParticleEmitScale = 1.0f;
		m_ParticleLifeScale = 1.0f;
	}

	/*if(m_EnableParticles && g_debugParticleShowScales)
	{
		Util::C_String text;
		text.Printf("%0.2f %0.1f %0.1f", m_ScreenSize, m_ParticleEmitScale, m_ParticleLifeScale);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(m_Matrix.AW(), text);
	}*/

	m_pVehicleModel->m_FocusVehicle = m_IsFocusVehicle;

	if(m_ParticleReducedEmitScaleTimer > 0.f)
	{
		m_ParticleReducedEmitScaleTimer -= m_FrameTime;
		m_ParticleEmitScale *= (float)g_debugParticleReducedEmitScale;
	}

	if (m_IsFocusVehicle)
	{
		m_pVehicleModel->SetPartVisibility(Vehicle::E_PartCode_WindowInterior, false);
		//Set "m_VisibilityOverride = true" if some external mechanic (i.e. cut-scenes) needs to override this
		CheckCamera();	
		UpdateScreenSFX();
	}

	if (!m_pVehiclePhysics)
		return;

	bool updateMe = m_pVehicleModel->GetModelInput()->m_bShouldUpdate;// && m_pVehicleModel->GetModelInput()->m_bShouldRender;

	Maths::C_RGBA_F32 lighting = m_pVehicleModel->GetLighting();

	if (updateMe)
	{
		UpdateGripLoss();
		
		MathsStorage::Matrix44 matrix44 = m_pVehiclePhysics->GetAxis().GetMatrix();

		Maths::HMat44 hMat44;
		hMat44 = Maths::Load<Maths::HMat44>(&matrix44);
		m_Matrix = (Maths::HMat43)hMat44;
		m_Velocity = RPToMaths(&m_pVehiclePhysics->GetVelocity());

		SanityCheckPosition();

		m_RotationQuaternion = Maths::C_Quaternion(m_Matrix.Rotation());
		
		m_FakeVehiclePhysics.Update(this, m_FrameTime, GetCollisionBody()->GetBody());
		m_FakeVehicleSpeed.Update(this, m_FrameTime, GetCollisionBody()->GetBody());

		HandleConstraintFreezing();

		UpdateConstraint(m_FrameTime);
		UpdateVehicleAirFrictionCoefficient(C_Game::GetInstance()->GetLevelState().mCars[m_VehicleIndex]);

		BodyShake();

		m_LocalSpacePosition.SetPosition(m_Matrix.AW());
		m_LocalSpace.SetPosition(m_Matrix.AW());
		m_LocalSpace.SetRotation(m_RotationQuaternion);

		m_pVehicleModel->GetModelInput()->m_mMatrix = m_Matrix;
	
		if (m_pVehicleModel)
		{
			m_pVehicleModel->UpdateHeadLightMatrix();
		}

		
		// Notify driver of vehicle reset.
		if (m_ResetifyDriver)
		{
			C_DriverManager::Instance()->ResetDriver(m_VehicleIndex);
			m_ResetifyDriver = false;
		}

		S32 splashViewport = 0;
		if (m_IsFocusVehicle)
			splashViewport = m_ViewportID;
		else
			splashViewport = -1;
		
		if (!m_ResetifyDriver)
		for (S32 iWheel = 0; iWheel < m_WheelEffects.Size(); iWheel++)
		{
			const Vehicle::C_WheelDamage* pWheelDamage = m_pVehicleModel->GetDamageLayer()->GetWheel((U32)iWheel);
			bool enableParticles = m_EnableParticles && m_WheelParticleState[iWheel].first;
			float fPScale = m_WheelParticleState[iWheel].second;

			m_WheelEffects[iWheel].UpdateCommon(&m_Matrix, &m_pVehicleModel->GetModelInput()->m_Wheels[iWheel].m_mYawMatrix, m_FrameTime, pWheelDamage, enableParticles, splashViewport, m_pGameWorld->GetSplashCamera(), m_EffectsVisiblityFlags);
			m_WheelEffects[iWheel].UpdateTracks(lighting, pWheelDamage, m_EffectsVisiblityFlags);
		}

		if (g_bDeferredVehicleUpdate)
		{
			C_VehicleActor *p = this;
			BIZ_ASSERT(m_hUpdateSchedulerGroup != 0); // make sure you set the scheduler group before running the update
			Scheduler::AddTask(::DeferredVehicleUpdateTask, 0, 0, m_hUpdateSchedulerGroup, 0, &p, sizeof(p));
		}
		else
		{
			DeferredUpdate();
		}

		if (m_pPickupTriggerActor && pVehicleCreate->m_pGameWorld->GetTriggerLayer(Game::E_TriggerLayer_Pickup))
		{
			m_pPickupTriggerActor->SetMatrix(m_Matrix, physicsStack);
		}
		if (m_pTriggerRegionTriggerActor && pVehicleCreate->m_pGameWorld->GetTriggerLayer(Game::E_TriggerLayer_TriggerRegion))
		{
			Gravity::C_TriggerLayer* pLayer = pVehicleCreate->m_pGameWorld->GetTriggerLayer(Game::E_TriggerLayer_TriggerRegion);
			m_pTriggerRegionTriggerActor->SetMatrix(m_Matrix, physicsStack);
		}
		
		if (m_EmpState.m_bEmp)
		{
			if (m_EmpState.m_fEmpTime > m_EmpState.m_fEmpMaxTime)
			{
				m_EmpState.m_fEmpMaxTime	= 0.0f;
				m_EmpState.m_fEmpTime		= 0.0f;
				m_EmpState.m_bEmp			= false;
				m_pVehicleModel->GetModelInput()->m_fBrake					= m_EmpState.m_fBrake;
				m_pVehicleModel->GetModelInput()->m_bRearLights.m_Current	= m_EmpState.m_bRearLights;
				m_pVehicleModel->GetModelInput()->m_bHeadLights.m_Current	= m_EmpState.m_bHeadLights;
				m_pVehicleModel->GetModelInput()->m_bFogLights.m_Current	= m_EmpState.m_bFogLights;
			}
			else
			{
				m_EmpState.m_fEmpTime += m_FrameTime;

				float rpercent = (float)g_EMPRandom.FRand(0.0f, 1.0f);
				
				if (rpercent < (float)g_debugEmpProbability)
				{
					m_pVehicleModel->GetModelInput()->m_fBrake = 1.0f - m_pVehicleModel->GetModelInput()->m_fBrake;
					m_pVehicleModel->GetModelInput()->m_bHeadLights.m_Current = !m_pVehicleModel->GetModelInput()->m_bHeadLights.m_Current;
					m_pVehicleModel->GetModelInput()->m_bRearLights.m_Current = !m_pVehicleModel->GetModelInput()->m_bRearLights.m_Current;
					m_pVehicleModel->GetModelInput()->m_bFogLights.m_Current = !m_pVehicleModel->GetModelInput()->m_bFogLights.m_Current;
				}
			}
		}
	}
	
	if (!C_Game::GetInstance()->GetLevelState().mInUI)
		CheckForLandingEffects();
	

	if (updateMe && !m_LostControl)
	{		
		UpdateFlightControl();
		UpdateDownForce();
		DoRayTraces();
	}	

	//Update any pickup effects
	UpdatePickupSFX(m_FrameTime);
	
	m_Window.Update(m_FrameTime);

	//Send messages
	{
		PERF_EVENT("VehicleActor.BroadcastPositionMsg");
		C_VehicleWorldPositionMsg positionMsg;
		positionMsg.mActorHandle = GetCreationData()->GetActorHandle();
		positionMsg.mVehicleIndex = m_VehicleIndex;
		positionMsg.mWorldMatrix = m_pVehicleModel->GetModelInput()->m_mMatrix;
		positionMsg.mWorldPosition = m_pVehicleModel->GetModelInput()->m_mMatrix.AW();
		BroadcastMessage(&positionMsg);
		Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&positionMsg);
	}

	{
		// Update driver parameters (full driver update is performed later).
		U32 driverIndex = m_VehicleIndex;
		C_DriverManager* pDriverManager = C_DriverManager::Instance();
		pDriverManager->SetVehicleModelInput(driverIndex, *m_pVehicleModel->GetModelInput());
		pDriverManager->SetVehicleLightSampler(driverIndex, m_pVehicleModel->GetLightSampler());
		pDriverManager->SetDriverVisibilityDetectionFlags(driverIndex, m_pVisDetect->GetVisibleFlags());
	}

	m_CollisionCacheVertices.Clear();
	if (RenderShadowInMirror())
	{		
		for (unsigned int iTri = m_pVehiclePhysics->GetLineCollisionCache().m_Low; iTri < m_pVehiclePhysics->GetLineCollisionCache().m_High; iTri++)
		{
			const Collision::C_CachedTriangle& tri = m_pVehiclePhysics->GetLineCollisionCache().m_pTriangles[iTri];
			m_CollisionCacheVertices.PushBack(tri.m_VertexA);
			m_CollisionCacheVertices.PushBack(tri.m_VertexB);
			m_CollisionCacheVertices.PushBack(tri.m_VertexC);
		}
	}

	if( m_VisibleHealthDamageEnabled )
	{


		// Keep the visuals in sync with damage - seems easier than having to keep checking this per potential damage event.
		C_RaceLogicLayer* pRaceLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		if (pRaceLogicLayer)
		{
			const C_VehicleLogicActor* pLogicActor = pRaceLogicLayer->GetLogicVehicleActor(m_VehicleIndex);
			if (pLogicActor)
			{
				//E_HEALTH_LEVEL healthLevel = pLogicActor->GetCurrentHealthLevel();
				m_HealthPoints		= pLogicActor->GetCurrentHealth();

				E_HEALTH_LEVEL healthLevel = pLogicActor->GetCurrentHealthLevel();

				if (m_HealthPoints <= 200)
					healthLevel = E_HEALTH_LEVEL_LOW;
				else if (m_HealthPoints <= 300)
					healthLevel = E_HEALTH_LEVEL_MEDIUM;
				
				switch (healthLevel)
				{
				case E_HEALTH_LEVEL_HIGH:
					{
						if (m_EngineSmokeLightEmitter.m_Intensity != 0.0f)
						{
							m_EngineSmokeLightEmitter.m_Deactivate	= true;
							m_EngineSmokeLightEmitter.m_Intensity	= 0.0f;
						}
						if (m_EngineSmokeHeavyEmitter.m_Intensity != 0.0f)
						{
							m_EngineSmokeHeavyEmitter.m_Deactivate	= true;
							m_EngineSmokeHeavyEmitter.m_Intensity	= 0.0f;
						}
					}
					break;
				case E_HEALTH_LEVEL_MEDIUM:
					{
						if (m_EngineSmokeLightEmitter.m_Intensity == 0.0f)
							m_EngineSmokeLightEmitter.m_Activate = true;
						if (m_EngineSmokeHeavyEmitter.m_Intensity != 0.0f)
						{
							m_EngineSmokeHeavyEmitter.m_Deactivate	= true;
							m_EngineSmokeHeavyEmitter.m_Intensity	= 0.0f;
						}
					}
					break;
				case E_HEALTH_LEVEL_LOW:
				case E_HEALTH_LEVEL_WRECKED:
					{
						if (m_EngineSmokeHeavyEmitter.m_Intensity == 0.0f)
							m_EngineSmokeHeavyEmitter.m_Activate = true;
						if (m_EngineSmokeLightEmitter.m_Intensity != 0.0f)
						{
							m_EngineSmokeLightEmitter.m_Deactivate	= true;
							m_EngineSmokeLightEmitter.m_Intensity	= 0.0f;
						}
					}
					break;
				}
			}
		}
	}
	else
	{
		m_EngineSmokeLightEmitter.Destroy();
		m_EngineSmokeHeavyEmitter.Destroy();
	}

	UpdateAlphaTransitions(m_FrameTime);


	if (g_debugVehicleActorMasterDebug && (g_debugVehicleActorMasterDebugVehicle == m_VehicleIndex || g_debugVehicleActorMasterDebugVehicle == -1))
	{
		Update_Debug();
	}
}

void C_VehicleActor::HandleConstraintFreezing()
{
	bool shouldFreezeForEffect = false;

	switch (m_FakeVehiclePhysics._effect)
	{
	case E_FAKE_PHYSICS_SPIN_360_LEFT:
	case E_FAKE_PHYSICS_SPIN_360_RIGHT:
		{
			shouldFreezeForEffect = true;

			if (!m_FreezeConstrainedParts && m_FakeVehiclePhysics.IsActive())
			{
				m_FreezeConstrainedParts = true;

				m_pVehicleModel->GetComponentList().FreezeParts(true, m_Matrix);
				return;
			}
		}
		break;
	default:
		break;
	}

	if (m_FreezeConstrainedParts && (!m_FakeVehiclePhysics.IsActive() || (m_FakeVehiclePhysics.IsActive() && !shouldFreezeForEffect)))
	{
		m_FreezeConstrainedParts = false;
		m_pVehicleModel->GetComponentList().FreezeParts(false, m_Matrix);
	}
}

void C_VehicleActor::SetWheelParticleLod(bool p_Wheel0Enable, bool p_Wheel1Enable, bool p_Wheel2Enable, bool p_Wheel3Enable, 
					  					 float p_Wheel0Quality, float p_Wheel1Quality, float p_Wheel2Quality, float p_Wheel3Quality)
{
	m_WheelParticleState[0].first = p_Wheel0Enable;
	m_WheelParticleState[1].first = p_Wheel1Enable;
	m_WheelParticleState[2].first = p_Wheel2Enable;
	m_WheelParticleState[3].first = p_Wheel3Enable;

	m_WheelParticleState[0].second = p_Wheel0Quality;
	m_WheelParticleState[1].second = p_Wheel1Quality;
	m_WheelParticleState[2].second = p_Wheel2Quality;
	m_WheelParticleState[3].second = p_Wheel3Quality;
}

void C_VehicleActor::UpdateScreenSFX()
{
	if (m_pImpactSpecialFX)
	{
		m_fImpactEffectTime += m_FrameTime;
		if (m_fImpactEffectTime > m_pImpactSpecialFX->GetDuration())//(float)g_debugImpactEffectDuration)
		{
			m_fImpactEffectTime = 0.0f;
			m_pImpactSpecialFX->Stop();
			SAFE_DELETE_PTR(m_pImpactSpecialFX);
		}
		else
		{
			m_pImpactSpecialFX->Update(m_fImpactEffectTime);
		}
	}


	UpdateSmallImpactEffects(m_FrameTime);
	UpdateRoughTerrainEffects(m_FrameTime);
	UpdateHighSpeedEffects(m_FrameTime);

	if (m_pLandedSpecialFX)
	{	
		m_fLandedEffectTime += m_FrameTime;
		if (m_fLandedEffectTime > m_pLandedSpecialFX->GetDuration())
		{
			m_fLandedEffectTime = 0.0f;
			m_pLandedSpecialFX->Stop();
			SAFE_DELETE_PTR(m_pLandedSpecialFX);
		}
		else
		{
			m_pLandedSpecialFX->Update(m_fLandedEffectTime);
		}
	}
	
	if (m_pHealthSpecialFX)
	{
		m_fHealthEffectTime += m_FrameTime;
		if (m_fHealthEffectTime > m_pHealthSpecialFX->GetDuration())//(float)g_debugImpactEffectDuration)
		{
			m_fHealthEffectTime = 0.0f;
			m_pHealthSpecialFX->Stop();
			m_pHealthSpecialFX->Start();
		}
		else
		{
			m_pHealthSpecialFX->Update(m_fHealthEffectTime);
		}
	}
}

void C_VehicleActor::UpdateAlphaTransitions(const float frameTime)
{
	if(m_GhostTransitionTime > 0.0f)
	{
		m_GhostTransitionTime = Maths::Clamp(m_GhostTransitionTime - frameTime, 0.0f, m_GhostTransitionDuration);

		const float initialAlpha = GetAlpha(E_VAC_TYPE_VEHICLE, E_VAC_TARGET_TYPE_INITIAL);
		const float targetAlpha = GetAlpha(E_VAC_TYPE_VEHICLE, E_VAC_TARGET_TYPE_DESIRED);
		bool deGhosting = initialAlpha < targetAlpha;
	
		float lerpValue = 1.0f - Maths::Clamp(m_GhostTransitionTime / m_GhostTransitionDuration, 0.0f, 1.0f);
		float alpha = CalculateAlphaValue(lerpValue);
		
		//Apply the alpha, its seriously expensive (and pointless) to render the vehicle with an alpha of 0.f
		UpdateAlpha(alpha);
		//UpdateAlpha(alpha, lerpValue);

		if (m_GhostTransitionTime <= 0.0f)
		{
			//If we're deghost
			if(deGhosting)
			{
				//Snap to the final alpha value
				Deghost(0.f, targetAlpha);
			}

			//Get rid of the curve
			m_pAlphaTransitionCurve = NULL;
		}
	}
}

const float C_VehicleActor::CalculateAlphaValue(const float lerpValue, const enum E_VAC_TYPE eVehicleAlphaComponentType) const
{
	float alpha = -1.f;

	const float initialAlpha = GetAlpha(eVehicleAlphaComponentType, E_VAC_TARGET_TYPE_INITIAL);
	const float targetAlpha = GetAlpha(eVehicleAlphaComponentType, E_VAC_TARGET_TYPE_DESIRED);
	
	//See if we've got a curve to follow for this 
	if(!m_pAlphaTransitionCurve || !m_pAlphaTransitionCurve->Valid())
	{
		alpha = Maths::Clamp(Maths::Lerp(initialAlpha, targetAlpha, lerpValue), 0.0f, 1.0f);
	}
	else
	{	
		float alphaRange = 0.f;
		float alphaMin = 0.f;
		const bool deGhosting = (initialAlpha < targetAlpha);
		if(deGhosting)
		{
			alphaRange = (targetAlpha - initialAlpha);
			alphaMin = initialAlpha;
		}
		else
		{
			alphaRange = (initialAlpha - targetAlpha);
			alphaMin = targetAlpha;
		}

		alpha = m_pAlphaTransitionCurve->Evaluate( lerpValue );
		if(alphaRange != 0.f)
		{
			alpha = (alpha * alphaRange) + alphaMin;
		}
	}

	return alpha;
}

void C_VehicleActor::SetAlphaTransitionCurve(const Curve::C_HermiteCurve *pAlphaTransitionCurve)
{
	m_pAlphaTransitionCurve = pAlphaTransitionCurve;
}


Game::Effects::E_MiscParticleEventType C_VehicleActor::GetLandingEffectType(S32 p_SurfaceType)
{
	switch (p_SurfaceType)
	{
	case Game::E_EffectMaterial_Dirt:
	case Game::E_EffectMaterial_Mud:
	case Game::E_EffectMaterial_Grass:
	case Game::E_EffectMaterial_Sand:
	case Game::E_EffectMaterial_WetSand:
	case Game::E_EffectMaterial_Vegetation:
	case Game::E_EffectMaterial_Wood:
	case Game::E_EffectMaterial_DirtLightRough:
	case Game::E_EffectMaterial_DirtHeavyRough:
		return Game::Effects::E_MiscParticleEventType_LandingDust;
	}
	return Game::Effects::E_MiscParticleEventType_LandingSparks;
}

void C_VehicleActor::UpdateSmallImpactEffects(F32 elapsedTime)
{
	PERF_EVENT("C_VehicleActor.UpdateSmallImpactEffects");
	m_fSmallImpactTimeSinceImpact += elapsedTime;
	if(m_fSmallImpactTimeSinceImpact>=0.0f)
	{
		
		m_fSmallImpactTimeSinceImpact = Maths::Clamped<F32>(m_fSmallImpactTimeSinceImpact,0.0f,1.0f);
		m_fSmallImpactAccumulator -= elapsedTime * g_SmallImpactAccumulatorDecayRate * m_fSmallImpactTimeSinceImpact;
	}
	
	if( m_fSmallImpactAccumulator < 0.0f )
	{
		m_fSmallImpactAccumulator = 0.0f;
	}

	// Scale by a component of forwards velocity
	RacingPhysics::Car* physics = GetVehiclePhysics();
	Maths::HVec3 linVel = physics->GetVelocity();
	Maths::HVec3 linVelLS = linVel * Maths::HMat33(GetMatrix().InverseRotTrans());
	F32 velocityScaling = (fabs(linVelLS.Z()) - g_SmallImpactMinVelocity) / (g_SmallImpactMaxVelocity - g_SmallImpactMinVelocity);
	velocityScaling = Maths::Clamped<F32>(velocityScaling,0.0f,1.0f);

	F32 smallImpactAccumulator = m_fSmallImpactAccumulator * velocityScaling;


	
	F32 val = (smallImpactAccumulator - g_SmallImpactAccumulatorThreshold)/( g_SmallImpactAccumulatorMax - g_SmallImpactAccumulatorThreshold);
	F32 normalised = Maths::Clamped<F32>(val, 0.0f, 1.0f);
	normalised *= velocityScaling;

	m_fSmallImpactEffectValue += (normalised - m_fSmallImpactEffectValue) * Kanzen::DampingAdjust( g_SmallImpactEffectDamping, elapsedTime);
	
	
#ifndef CFG_GOLD
	if( g_SmallImpactAccumulatorShow )
	{
		g_SmallImpactEffectValue = m_fSmallImpactEffectValue;
	}
#endif
	

#ifndef CFG_GOLD
	if( g_SmallImpactAccumulatorShow )
	{
		g_SmallImpactAccumulatorValue = smallImpactAccumulator;
	}
#endif
}

void C_VehicleActor::UpdateRoughTerrainEffects(F32 elapsedTime)
{
	PERF_EVENT("C_VehicleActor.UpdateRoughTerrainEffects");
	const C_VehicleActorCreate *pVehicleCreate = (const C_VehicleActorCreate *)(GetCreationData().GetData());

	if(!pVehicleCreate)
		return;

	S32 viewIndex = pVehicleCreate->m_pRLC->GetViewportIndex(m_VehicleIndex);

	// First of all, if we don't have a rough-terrain effect started
	// and we should (because we are now in an active viewport) so
	// create and start one.
	if(!m_pRoughTerrainSpecialFX)
	{
		if(viewIndex!=-1)
		{
			const SpecialFX::C_SpecialFX* p_pScreenEffect = NULL;

			if(C_GameVehicleData::Instance().mVehicleDefaults.Number() > 0)
			{
				p_pScreenEffect = C_GameVehicleData::Instance().mVehicleDefaults[0]->mRoughTerrainScreenEffect;
			}
			if(p_pScreenEffect)
			{
				m_pRoughTerrainSpecialFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(*p_pScreenEffect, GetActorList()->GetDefaultContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr());

				if (!m_pRoughTerrainSpecialFX)
					return;

				m_pRoughTerrainSpecialFX->SetParameter(SpecialFX::E_ParamID_SetCameraViewport, (void *)viewIndex);
				m_pRoughTerrainSpecialFX->Start();
			}
			// create and start
		}
	}
	// Also check for turning off unnecessary effects 
	else
	{
		if(viewIndex==-1 || g_debugRoughTerrainReset )
		{
			g_debugRoughTerrainReset = false;
			m_pRoughTerrainSpecialFX->Stop();
			SAFE_DELETE_PTR(m_pRoughTerrainSpecialFX);
		}
	}

	// Now if we have a valid rough terrain effect, update it
	if(m_pRoughTerrainSpecialFX)
	{
		F32 desiredRoughTerrainPercentage = 0.0f;
		if( g_debugRoughTerrainEffect >= 0.0f && g_debugRoughTerrainEffectDisplay == false)
		{
			desiredRoughTerrainPercentage = g_debugRoughTerrainEffect;
		}
		else
		{
			const RacingPhysics::Car* physics = GetVehiclePhysics();

			desiredRoughTerrainPercentage = 0.0f;
			for(int i=0; i<physics->GetNumWheels(); ++i)
			{
				const RacingPhysics::CarWheel& wheel = physics->Wheel(i);
				if(wheel.OnGround())
				{
					desiredRoughTerrainPercentage += wheel.SurfaceRoughnessScale();
				}
			}
			desiredRoughTerrainPercentage /= physics->GetNumWheels();

			Maths::HVec3 LinVelWS = physics->GetVelocity();
			Maths::HVec3 LinVelLS = LinVelWS * Maths::HMat33(GetMatrix().InverseRotTrans());
			LinVelLS.Y(0.0f);

			// Make the desired terrain effect a function of velocity as well. So we don't get rumble when we are still
			desiredRoughTerrainPercentage *= Maths::Clamped<F32>(LinVelLS.Length()/g_debugRoughTerrainEffectMaxSpeed, 0.0f,1.0f );
			desiredRoughTerrainPercentage *= (1.0f / g_debugRoughTerrainEffectMaxRoughness);
			desiredRoughTerrainPercentage = Maths::Clamped<F32>(desiredRoughTerrainPercentage,0.0f,1.0f);
		}
		
		m_fRoughTerrainPercentage += ((desiredRoughTerrainPercentage - m_fRoughTerrainPercentage) * Kanzen::DampingAdjust(g_debugRoughTerrainEffectDamping,elapsedTime));
		
		F32 input = Maths::Max(m_fSmallImpactEffectValue * g_SmallImpactAccumulatorScale,m_fRoughTerrainPercentage);
		F32 roughTerrainInput = Maths::Clamped<F32>(input,0.0f,0.99f) * m_pRoughTerrainSpecialFX->GetDuration();
		if(g_debugRoughTerrainEffectDisplay)
		{
			if(roughTerrainInput<0.01f)
			{
				roughTerrainInput=0.0f;
			}
			g_debugRoughTerrainEffect.SetValue(roughTerrainInput);
		}
		m_pRoughTerrainSpecialFX->Update(roughTerrainInput);
	}
}

void C_VehicleActor::UpdateHighSpeedEffects(F32 elapsedTime)
{
	PERF_EVENT("C_VehicleActor.UpdateHighSpeedEffects");
	const C_VehicleActorCreate *pVehicleCreate = (const C_VehicleActorCreate *)(GetCreationData().GetData());

	if(!pVehicleCreate)
		return;

	S32 viewIndex = pVehicleCreate->m_pRLC->GetViewportIndex(m_VehicleIndex);

	// First of all, if we don't have a highspeed effect started
	// and we should (because we are now in an active viewport
	// create and start one.
	if(!m_pHighSpeedSpecialFX)
	{
		if(viewIndex!=-1)
		{
			const SpecialFX::C_SpecialFX* p_pScreenEffect = NULL;

			if(C_GameVehicleData::Instance().mVehicleDefaults.Number() > 0)
			{
				p_pScreenEffect = C_GameVehicleData::Instance().mVehicleDefaults[0]->mHighSpeedScreenEffect;
			}
			if(p_pScreenEffect)
			{
				m_pHighSpeedSpecialFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(*p_pScreenEffect, GetActorList()->GetDefaultContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr());

				if (!m_pHighSpeedSpecialFX)
					return;

				m_pHighSpeedSpecialFX->SetParameter(SpecialFX::E_ParamID_SetCameraViewport, (void *)viewIndex);
				m_pHighSpeedSpecialFX->Start();
			}
			// create and start
		}
	}
	// Also check for turning off unnecessary effects 
	else
	{
		if(viewIndex==-1 || g_debugHighSpeedReset )
		{
			g_debugHighSpeedReset = false;
			m_pHighSpeedSpecialFX->Stop();
			SAFE_DELETE_PTR(m_pHighSpeedSpecialFX);
		}
	}

	// Now if we have a valid high speed effect, update it
	if(m_pHighSpeedSpecialFX)
	{
		F32 desiredHighSpeedPercentage = 0.0f;
		if( g_debugHighSpeedEffect >= 0.0f )
		{
			desiredHighSpeedPercentage = g_debugHighSpeedEffect;
		}
		else
		{
			const RacingPhysics::Car* physics = GetVehiclePhysics();

			Maths::HVec3 LinVelWS = physics->GetVelocity();
			Maths::HVec3 LinVelLS = LinVelWS * Maths::HMat33(GetMatrix().InverseRotTrans());
			
			desiredHighSpeedPercentage = Maths::Clamped<F32>(LinVelLS.Z()/g_debugHighSpeedEffectMaxVel, 0.0f,1.0f );
		}
		m_fHighSpeedPercentage += ((desiredHighSpeedPercentage - m_fHighSpeedPercentage) * Kanzen::DampingAdjust(g_debugHighSpeedEffectDamping,elapsedTime));
		m_pHighSpeedSpecialFX->Update(m_pHighSpeedSpecialFX->GetDuration() * m_fHighSpeedPercentage);
	}
}
void C_VehicleActor::PlayWreckedEffect()
{
	// Play a wrecked special effect
	//// Play a special FX
	RLPickupSpecialFX sfx;



	sfx.mExternalSpecialFX=C_GameVehicleData::Instance().mVehicleDefaults[0]->mWreckedSpecialEffect;
	sfx.mDurationLoop.mStart=sfx.mDurationLoop.mEnd=0;
	sfx.mDurationIntro.mStart=0.0f;sfx.mDurationIntro.mEnd=5;
	sfx.mDurationOutro.mStart=sfx.mDurationOutro.mEnd=0;
	sfx.mFrequency=1.0f;
	sfx.mSpecialFXLocation = E_PICKUP_SPECIALFX_LOCATION_ACTINGCAR;

	Game::C_InGameSpecialFXManager::Instance().SetUpVehicleSFX(sfx,GetMatrix(),m_VehicleIndex,-1,Game::E_InGameSpecialFX_Trigger_Wrecked, -1.f, Maths::HVec3(1,1,1), Maths::HVec3());

}

void C_VehicleActor::CheckForLandingEffects()
{
	PERF_EVENT("CheckForLandingEffects");

	m_landing.Update(*this,m_FrameTime);
	m_tumbling.Update(*this,m_FrameTime);

#ifndef CFG_GOLD
	// Conditionally enable the debug messages we are logging. Off by default
	if(g_VehicleActorEffectsLogEnabled)
	{
		if(g_VehicleActorEffectsLog.GetTrueLevel() != DebugLevel::Debug )
		{
			g_VehicleActorEffectsLog.SetLevel(DebugLevel::Debug);
		}
	}
	else
	{
		if(g_VehicleActorEffectsLog.GetTrueLevel() == DebugLevel::Debug )
		{
			g_VehicleActorEffectsLog.SetLevel(DebugLevel::Info);
		}
	}
#endif
	if(IsFocusVehicle())
	{	
		F32 landingImpact = m_landing.GetLandedImpact();
		if (landingImpact>0.01f)
		{
			C_RaceLogicLayer* pRaceLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			const RLLandingType* pLandingType = pRaceLogicLayer->CalculateLandingType(landingImpact);
			if (pLandingType)
			{
				LOGDEBUG( g_VehicleActorEffectsLog, "Triggered Vehicle Landing FX: Level - %d  [%f]\n", pLandingType->meImpactLevel, landingImpact);
				SetLandedEffect(pLandingType->mScreenEffect);
			}
		}
	}

#ifndef CFG_GOLD
	if (g_debugLandingTriggerCameraEffect)
	{
		g_debugLandingTriggerCameraEffect = false;
	
		C_RaceLogicLayer* pRaceLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		const RLLandingType* pLandingType = pRaceLogicLayer->CalculateLandingType(g_debugLandingCameraImpulseValue);
		if (pLandingType)
		{
			if (IsFocusVehicle())
			{	
				SetLandedEffect(pLandingType->mScreenEffect);
			}
		}
	}
#endif

	using namespace Maths;
	if (!m_EnableParticles || !m_pVehiclePhysics || m_pVehiclePhysics->GetNumWheels() != 4 || !m_pVehicleModel || !m_pVehicleModel->GetDamageLayer())
		return;

	//Left
	//m_pVehiclePhysics->Wheel(0).SuspensionDistance()
	float fWidth = m_pVehicleModel->GetDamageLayer()->GetVehicleInfo()->m_Extents.X();
	HVec3 positiveY(Constants<HVec3>::UnitAxisPositiveY());
		
	const RacingPhysics::CarWheel& wheel0 = m_pVehiclePhysics->Wheel(0);
	const RacingPhysics::CarWheel& wheel2 = m_pVehiclePhysics->Wheel(2);
	if ((wheel0.OnGround() && wheel0.SuspensionDistance() > (float)g_debugLandingEffectSuspensionDistance && wheel0.GroundForce().Y() > (float)g_debugLandingEffectGroundGorce) ||
		(wheel2.OnGround() && wheel2.SuspensionDistance() > (float)g_debugLandingEffectSuspensionDistance && wheel2.GroundForce().Y() > (float)g_debugLandingEffectGroundGorce))
	{
		SpawnParticleEffect(GetLandingEffectType(wheel0.OnGround() ? wheel0.SurfaceType() : wheel2.SurfaceType()), Maths::HVec3(-fWidth*0.9f, -(float)g_debugLandingEffectYOffset, 0.0f), positiveY, m_pVehiclePhysics->GetVelocity(), 1.0f, &m_LocalSpace);
	}

	const RacingPhysics::CarWheel& wheel1 = m_pVehiclePhysics->Wheel(1);
	const RacingPhysics::CarWheel& wheel3 = m_pVehiclePhysics->Wheel(3);
	if ((wheel1.OnGround() && wheel1.SuspensionDistance() > (float)g_debugLandingEffectSuspensionDistance && wheel1.GroundForce().Y() > (float)g_debugLandingEffectGroundGorce) ||
		(wheel3.OnGround() && wheel3.SuspensionDistance() > (float)g_debugLandingEffectSuspensionDistance && wheel3.GroundForce().Y() > (float)g_debugLandingEffectGroundGorce))
	{
		SpawnParticleEffect(GetLandingEffectType(wheel1.OnGround() ? wheel1.SurfaceType() : wheel3.SurfaceType()), Maths::HVec3(fWidth*0.9f, -(float)g_debugLandingEffectYOffset, 0.0f), positiveY, m_pVehiclePhysics->GetVelocity(), 1.0f, &m_LocalSpace);
	}

#ifndef CFG_GOLD
	if (g_debugLandingTriggerEffect)
	{
		g_debugLandingTriggerEffect = false;
		if (g_debugLandingEffectDust)
		{
			SpawnParticleEffect(Game::Effects::E_MiscParticleEventType_LandingDust, Maths::HVec3(-fWidth*0.9f, -0.1f, 0.0f), positiveY, m_pVehiclePhysics->GetVelocity(), 1.0f, &m_LocalSpace);
			SpawnParticleEffect(Game::Effects::E_MiscParticleEventType_LandingDust, Maths::HVec3(fWidth*0.9f, -0.1f, 0.0f), positiveY, m_pVehiclePhysics->GetVelocity(), 1.0f, &m_LocalSpace);
		}
		if (g_debugLandingEffectSparks)
		{
			SpawnParticleEffect(Game::Effects::E_MiscParticleEventType_LandingSparks, Maths::HVec3(-fWidth*0.9f, -0.1f, 0.0f), positiveY, m_pVehiclePhysics->GetVelocity(), 1.0f, &m_LocalSpace);
			SpawnParticleEffect(Game::Effects::E_MiscParticleEventType_LandingSparks, Maths::HVec3(fWidth*0.9f, -0.1f, 0.0f), positiveY, m_pVehiclePhysics->GetVelocity(), 1.0f, &m_LocalSpace);
		}
		if (g_debugLandingEffectWaterSplash)
		{
			SpawnParticleEffect(Game::Effects::E_MiscParticleEventType_WaterBigSplash, Constants<HVec3>::Zero(), positiveY, m_pVehiclePhysics->GetVelocity(), 1.0f, &m_LocalSpacePosition);
		}
		if (g_debugLandingEffectWaterBob)
		{
			SpawnParticleEffect(Game::Effects::E_MiscParticleEventType_WaterBobbing, Constants<HVec3>::Zero(), positiveY, m_pVehiclePhysics->GetVelocity(), 1.0f, &m_LocalSpacePosition);
		}
		
	}
#endif
}

void C_VehicleActor::RegisterWaterCollision(const MathsStorage::Float3& p_Position)
{
	if(m_EnableParticles)
	{
		using namespace Maths;
		HVec3 vPosition = Load<HVec3>(&p_Position);
		const HVec3 zero(Constants<HVec3>::Zero());
		const HVec3 positiveY(Constants<HVec3>::UnitAxisPositiveY());
		m_LocalSpacePositionWater = m_LocalSpacePosition;
		m_LocalSpacePositionWater.m_vPosition.y = p_Position.y;

		if (m_FramesInWater == 0)
		{
			SpawnParticleEffect(Game::Effects::E_MiscParticleEventType_WaterBigSplash, vPosition, positiveY, m_pVehiclePhysics->GetVelocity(), 1.0f, NULL, NULL);
			if (!m_WaterBobParticleEffect.Object())// || (m_WaterBobParticleEffect.Object() && !m_WaterBobParticleEffect->Active()))
				SpawnParticleEffect(Game::Effects::E_MiscParticleEventType_WaterBobbing, zero, positiveY, m_pVehiclePhysics->GetVelocity(), 1.0f, &m_LocalSpacePositionWater, NULL, &m_WaterBobParticleEffect);

			//send audio message
			//if player car
			if(m_IsFocusVehicle)
			{
				Game::Actors::C_VehicleAudioWaterSplashMsg pMsg;
				C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&pMsg);
			}

		}
		m_FramesInWater = Maths::Clamp(m_FramesInWater+10, 0, 10);
	}
}

void C_VehicleActor::DoRayTraces()
{
	PERF_EVENT("C_VehicleActor.GroundRaytrace");

	//If car is in the air, we'll need to ray-trace
	//if (!m_pVehicleModel->GetModelInput()->m_bAllWheelsOnGround)

	using namespace Maths;
	//Vector of normals
	HVec3 groundNormals[5];
	U32	numNormals = 1;
	U32 numPositions = 1;

	Collision::C_CollisionFilter filter(Game::PolyFlags::kWheelSolid, Game::PolyFlags::kWheelSolid, m_PhysicsGroupId, 0, 0);
	const HVec3 rayPos( m_Centre * m_Matrix );
	const HVec3 rayEndPos(rayPos + HVec3(0.f, -10.f, 0.f));
	
	if (!g_bDeferredVehicleRaytraces)
	{
		//Get ground position and normal
		//Collision::C_CollisionFilter filter(Game::PolyFlags::kWheelSolid, Game::PolyFlags::kWheelSolid, m_PhysicsGroupId, 0, 0);
		//Collision::C_CollisionFilter filter(0xffffffff, Game::PolyFlags::kAllSolid, m_PhysicsGroupId, 0, 0);
		U8 data[1024 * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> physicsStack(data, 1024);
		Collision::C_Ray ray;
		ray.InitialiseRay(rayPos, rayEndPos, &filter);
		GetPhysicsScene()->DoRayTrace(ray, physicsStack);
	
		const Scalar_Bool con(ray.m_result.Hit());
		m_GroundPosition = Select(con, ray.m_result.m_contact, Maths::HVec3(rayPos.X(), m_GroundPosition.Y(), rayPos.Z()));
		groundNormals[0] = Select(con, ray.m_result.m_TriangleNormal, m_GroundNormal);
	}
	else
	{
		const Scalar_Bool con(m_pGroundRay->m_result.Hit());
		m_GroundPosition = Select(con, HVec3(rayPos.X(), m_pGroundRay->m_result.m_contact.Y(), rayPos.Z()), Maths::HVec3(rayPos.X(), m_GroundPosition.Y(), rayPos.Z()));
		groundNormals[0]=Select(con, m_pGroundRay->m_result.m_TriangleNormal, m_GroundNormal);
		m_pGroundRay->InitialiseRay(rayPos, rayEndPos, &filter);
		GetPhysicsScene()->DoDeferredRayTrace(m_pGroundRay, NULL, GetRaytraceGroup(), m_iDeferredRaytracePolicy);
	}

	//Get normals from the wheels		
	for (S32 iWheel = 0; iWheel < m_pVehicleModel->GetModelInput()->m_Wheels.Size(); iWheel++)
	{
		if (!m_pVehicleModel->GetModelInput()->m_Wheels[iWheel].m_bOnGround)
			continue;
		groundNormals[numNormals] =  m_pVehicleModel->GetModelInput()->m_Wheels[iWheel].m_vGroundNormal;
		numNormals++;
		m_GroundPosition.Y(m_GroundPosition.Y() + m_pVehicleModel->GetModelInput()->m_Wheels[iWheel].m_vGroundPosition.Y());
		numPositions++;
	}

	BIZ_ASSERT(numNormals > 0);
	
	//Get all except the less important (to avoid small jumps)
	U32 lessImportant = 0;
	HF32 maxZValue(Constants<HF32>::One());
	HVec3 accumulatedGroundNormal(Constants<HVec3>::Zero());
	for (U32 i = 0 ; i < numNormals; i++ )
	{
		if(groundNormals[i].Y().LT(maxZValue))
		{
			lessImportant = i;
			maxZValue = groundNormals[i].Y();
		}

		accumulatedGroundNormal +=groundNormals[i];
	}

	accumulatedGroundNormal -=groundNormals[lessImportant];
	accumulatedGroundNormal.Normalise();
 
	//Average normals
	m_GroundNormal = Select(Scalar_Bool(numNormals == 1), groundNormals[0], accumulatedGroundNormal);

	//Average height position
	m_GroundPosition.Y(m_GroundPosition.Y() / F32(numPositions));
}

Debug::Bool g_debugFakeFatalDamageParticles("Vehicle.Damage.Fatal.FakeFatalDamageParticles", false);

Debug::Bool g_debugSetBonnetPosition("Vehicle.Functional.Bonnet.SetPosition", false);
Debug::Float g_debugBonnetAngle("Vehicle.Functional.Bonnet.Angle", 45.0f, Debug::Range(0.0f, 180.0f), Debug::Step(0.001f));

Debug::Bool g_debugSetDoorPosition("Vehicle.Functional.Door.SetPosition", false);
Debug::Float g_debugDoorLeftAngle("Vehicle.Functional.Door.LeftAngle", 45.0f, Debug::Range(0.0f, 180.0f), Debug::Step(0.001f));
Debug::Float g_debugDoorRightAngle("Vehicle.Functional.Door.RightAngle", 45.0f, Debug::Range(0.0f, 180.0f), Debug::Step(0.001f));

Debug::Bool g_debugUpdateWheelTravel("Vehicle.Wheels.UpdateMaxTravel", false);
Debug::Bool g_debugDisplayLastKnownResetMatrix("Vehicle.Components.HandOfGod.DrawLastKnownResetMatrix", false);

Debug::Bool g_debugToggleCarSkirt("Vehicle.Collisions.ToggleSkirt", false);
Debug::Bool g_debugCarSkirtEnabled("Vehicle.Collisions.EnableSkirt", false);

Debug::Bool g_debugWindowUpdateFromSorcery("Vehicle.Windows.UpdateFromSorcery", false);

void C_VehicleActor::Update_Debug()
{
	if (g_debugWindowUpdateFromSorcery)
	{
		Vehicle::XtVehicle* pXtVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex] ? &C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex]->baseVehicle : NULL;

		if (pXtVehicle && pXtVehicle->m_FunctionalWindow.m_Part)
		{
			Vehicle::C_Part* pWindow = m_pVehicleModel->GetComponent(pXtVehicle->m_FunctionalWindow.m_Part->m_PartCode, pXtVehicle->m_FunctionalWindow.m_Part->m_PartSubCode);
			if (pWindow && pWindow->m_pModelInstance)
			{
				m_Window.Initialise(&pXtVehicle->m_FunctionalWindow, pWindow);
				pWindow->SetPermanentVisibility(1);
			}
		}		
	}

#if 0 //KierS - get things compiling - we already know we don't want to toggle this on now
	if (g_debugToggleCarSkirt)
	{
		g_debugToggleCarSkirt = false;
		if (g_debugCarSkirtEnabled)
		{			
			if(m_ObjectColliderIndex >= 0)
			{
				Collision::C_CompositeShape* pComposite = (Collision::C_CompositeShape*)m_pCollisionBody->GetShape();
				Collision::C_Shape* pSkirtShape = pComposite->m_pShapes[m_ObjectColliderIndex].m_pShape;
				pSkirtShape->m_Filter.m_CollisionFlags = m_ObjectCollisionFilter.m_CollisionFlags;
				pSkirtShape->m_Filter.m_CollisionMask = m_ObjectCollisionFilter.m_CollisionMask;
			}
		}
		else
		{
			if(m_ObjectColliderIndex >= 0)
			{
				Collision::C_CollisionFilter nullFilter(0, 0, 0, 0, 0, 0, 0);
				Collision::C_CompositeShape* pComposite = (Collision::C_CompositeShape*)m_pCollisionBody->GetShape();
				Collision::C_Shape* pSkirtShape = pComposite->m_pShapes[m_ObjectColliderIndex].m_pShape;
				pSkirtShape->m_Filter.m_CollisionFlags = nullFilter.m_CollisionFlags;
				pSkirtShape->m_Filter.m_CollisionMask = nullFilter.m_CollisionMask;				
			}
		}
	}
#endif
	if (g_debugDisplayLastKnownResetMatrix)
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix(m_LastKnownResetMatrix);

	if (g_debugSetCollisions)
		SetCollisions(g_debugVehicleCollisions);

	if (g_debugSetBonnetPosition)
	{
		SetBonnetPosition((float)g_debugBonnetAngle);
		g_debugSetBonnetPosition = false;
	}

	if (g_debugSetDoorPosition)
	{
		SetDoorPosition((float)g_debugDoorLeftAngle, (float)g_debugDoorRightAngle);
		g_debugSetDoorPosition = false;
	}

	//REMOVE_DIRT
	//if (g_debugVehicleUpdateDirt)
	//	SetupDirtLayer();

	if (g_debugUpdateWheelTravel)
	{
		AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];
		if (pVehicle)
		{
			m_fMaxWheelTravelFront	= pVehicle->frontWheelMaxTravel;
			m_fMaxWheelTravelRear	= pVehicle->rearWheelMaxTravel;
			float fAngle = Maths::ArcTan(m_fMaxWheelTravelFront / m_pVehiclePhysics->Dynamics().geometry.wheelBase);
			m_fRearAxlePivotAngle = fAngle * (pVehicle->rearAxlePivotScale == 0.0f ? 1.0f : pVehicle->rearAxlePivotScale);			
		}
	}

	if (g_debugUpdateVehicleDrawDistances)
	{
		for (U32 iModel = 0; iModel < m_pVehicleModel->GetNumModelInstances(); iModel++)
		{
			Util::C_String name = "";
			m_pVehicleModel->GetModelInstanceName(iModel, name);
			SetDrawDistance(m_pVehicleModel->GetModelInstance(iModel), name);
		}
	}

	if (g_debugFakeDropOut)
		SetVehicleStatus((bool)g_debugVehicleStatus);

	if (g_debugUpdateDamageInfo)
	{
#ifdef AMAX_VEH_USE_MI
		Vehicle::XtVehicle* pXtVehicle = (Vehicle::XtVehicle*)C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];
#else
		Vehicle::XtVehicle* pXtVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex] ? &C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex]->baseVehicle : NULL;
#endif
		if (pXtVehicle && m_pVehicleModel && m_pVehicleModel->GetDamageLayer())
		{
			m_pVehicleModel->GetDamageLayer()->SetupDeformRanges(pXtVehicle->m_DeformationSetup);
			m_pVehicleModel->GetDamageLayer()->RenderDefomationInformation(m_Matrix);
		}
	}

	if (g_debugVehicleUpdateHeadlights && m_pVehicleModel)
	{
#ifdef AMAX_VEH_USE_MI
		Vehicle::XtVehicle* pXtVehicle = (Vehicle::XtVehicle*)C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];
#else
		Vehicle::XtVehicle* pXtVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex] ? &C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex]->baseVehicle : NULL;
#endif

		if (pXtVehicle->m_Headlight)
		{
			m_pVehicleModel->GetHeadlight()->m_ZOffset = pXtVehicle->m_Headlight->m_ZOffset;
			m_pVehicleModel->GetHeadlight()->m_Width = pXtVehicle->m_Headlight->m_Width;
			m_pVehicleModel->GetHeadlight()->m_Length = pXtVehicle->m_Headlight->m_Length;
			m_pVehicleModel->GetHeadlight()->m_Height = pXtVehicle->m_Headlight->m_Height;
			m_pVehicleModel->GetHeadlight()->m_Intensity = pXtVehicle->m_Headlight->m_Intensity;
			m_pVehicleModel->GetHeadlight()->m_Dip = pXtVehicle->m_Headlight->m_Dip;
			m_pVehicleModel->GetHeadlight()->m_Colour = Maths::C_RGBA_F32(pXtVehicle->m_Headlight->m_Colour.r, pXtVehicle->m_Headlight->m_Colour.g, pXtVehicle->m_Headlight->m_Colour.b, pXtVehicle->m_Headlight->m_Intensity);
		}
	}


	if (g_debugVehicleEditConstraints)
	{
		if (m_pVehicleModel && m_pVehicleModel->GetDamageLayer())
		{
#ifdef AMAX_VEH_USE_MI
			Vehicle::XtVehicle* pXtVehicle = (Vehicle::XtVehicle*)C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];
#else
			Vehicle::XtVehicle* pXtVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex] ? &C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex]->baseVehicle : NULL;
#endif
			m_pVehicleModel->GetDamageLayer()->EditConstraints(pXtVehicle, m_pCollisionBody, GetPhysicsScene());
		}		
	}

	DrawDebugHud();

	if (g_debugTriggerEmp && !m_EmpState.m_bEmp)
	{
		g_debugTriggerEmp = false;
		m_EmpState.m_fEmpMaxTime	= (float)g_debugEmpDuration;
		m_EmpState.m_fEmpTime		= 0.0f;
		m_EmpState.m_bEmp			= true;
		m_EmpState.m_fBrake			= m_pVehicleModel->GetModelInput()->m_fBrake;
		m_EmpState.m_bRearLights	= m_pVehicleModel->GetModelInput()->m_bRearLights.m_Current;
		m_EmpState.m_bHeadLights	= m_pVehicleModel->GetModelInput()->m_bHeadLights.m_Current;
		m_EmpState.m_bFogLights		= m_pVehicleModel->GetModelInput()->m_bFogLights.m_Current;
	}

	if (g_debugUpdateWheelScale)
	{
		AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];
		if (pVehicle)
		{
			//Alloy
			Vehicle::C_PartGroup* pComponent = &m_pVehicleModel->GetComponentList()[Vehicle::E_PartCode_Alloy];		
			
		
			//Front Left
			Maths::HMat33 rotMat;
			rotMat.SetIdentity();
			Vehicle::C_Part* pNode = &pComponent->m_Nodes[Vehicle::E_PartSubCode_FrontLeft];
			if (pNode->m_pModel->GetElement(0))
			{
				Maths::HMat43 wheelMat = pNode->m_pModel->GetElement(0)->GetMatrix();//pNode->m_pModel->GetMatrix();//pNode->m_RestMatrix;
				wheelMat.SetScaleRotTrans(Maths::HVec3(pVehicle->wheelScaleFront,pVehicle->wheelScaleFront,pVehicle->wheelScaleFront),rotMat,Maths::HVec3(wheelMat.AW().X()+pVehicle->wheelDepthFront*-1.0f,wheelMat.AW().Y(),wheelMat.AW().Z()));
				wheelMat = wheelMat * m_pVehicleModel->GetModelInput()->m_Wheels[0].m_mMatrix;
				pNode->m_pModelInstance->SetElementMatrix(-1, wheelMat);
			}
			
			//Front Right
			pNode = &pComponent->m_Nodes[Vehicle::E_PartSubCode_FrontRight];
			rotMat.SetIdentity();
			rotMat = Maths::HMat33(Maths::RotY(Maths::DegToRad(180.0f)));
			if (pNode->m_pModel->GetElement(0))
			{
				Maths::HMat43 wheelMat = pNode->m_pModel->GetElement(0)->GetMatrix();//pNode->m_pModel->GetMatrix();//pNode->m_RestMatrix;
				float err = wheelMat.AW().X() * -2.0f;
				wheelMat.SetScaleRotTrans(Maths::HVec3(pVehicle->wheelScaleFront,pVehicle->wheelScaleFront,pVehicle->wheelScaleFront),rotMat,Maths::HVec3(wheelMat.AW().X()+pVehicle->wheelDepthFront+err,wheelMat.AW().Y(),wheelMat.AW().Z()));
				wheelMat = wheelMat * m_pVehicleModel->GetModelInput()->m_Wheels[1].m_mMatrix;
				pNode->m_pModelInstance->SetElementMatrix(-1, wheelMat);
			}
			
			//Back Left
			pNode = &pComponent->m_Nodes[Vehicle::E_PartSubCode_BackLeft];
			rotMat.SetIdentity();
			if (pNode->m_pModel->GetElement(0))
			{
				Maths::HMat43 wheelMat = pNode->m_pModel->GetElement(0)->GetMatrix();//pNode->m_pModel->GetMatrix();//pNode->m_RestMatrix;			
				wheelMat.SetScaleRotTrans(Maths::HVec3(pVehicle->wheelScaleRear,pVehicle->wheelScaleRear,pVehicle->wheelScaleRear),rotMat,Maths::HVec3(wheelMat.AW().X()+pVehicle->wheelDepthRear*-1.0f,wheelMat.AW().Y(),wheelMat.AW().Z()));
				wheelMat = wheelMat * m_pVehicleModel->GetModelInput()->m_Wheels[2].m_mMatrix;
				pNode->m_pModelInstance->SetElementMatrix(-1, wheelMat);
			}
			
			//Back Right
			pNode = &pComponent->m_Nodes[Vehicle::E_PartSubCode_BackRight];
			rotMat.SetIdentity();
			rotMat = Maths::HMat33(Maths::RotY(Maths::DegToRad(180.0f)));
			if (pNode->m_pModel->GetElement(0))
			{
				Maths::HMat43 wheelMat = pNode->m_pModel->GetElement(0)->GetMatrix();//pNode->m_pModel->GetMatrix();//pNode->m_RestMatrix;
				float err = wheelMat.AW().X() * -2.0f;
				wheelMat.SetScaleRotTrans(Maths::HVec3(pVehicle->wheelScaleRear,pVehicle->wheelScaleRear,pVehicle->wheelScaleRear),rotMat,Maths::HVec3(wheelMat.AW().X()+pVehicle->wheelDepthRear+err,wheelMat.AW().Y(),wheelMat.AW().Z()));
				wheelMat = wheelMat * m_pVehicleModel->GetModelInput()->m_Wheels[3].m_mMatrix;
				pNode->m_pModelInstance->SetElementMatrix(-1, wheelMat);
			}
		}	
	}

	if (g_debugUpdateDynamicMaterials)
	{
#ifdef AMAX_VEH_USE_MI
		Vehicle::XtVehicle* pVehicle = (Vehicle::XtVehicle*)C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];
#else
		Vehicle::XtVehicle* pVehicle = &C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex]->baseVehicle;
#endif
		Vehicle::XtMaterialDefaults* pMaterialDefaults = NULL;
		if (C_GameVehicleData::Instance().mVehicleDefaults.Number() > 0)
			pMaterialDefaults = C_GameVehicleData::Instance().mVehicleDefaults[0]->materials;

		m_pVehicleModel->UpdateDynamicMaterials(pVehicle, pMaterialDefaults);
	}

	if (g_debugUpdatePaintFromXt)
	{
#ifdef AMAX_VEH_USE_MI
		Vehicle::XtVehicle* pXtVehicle = (Vehicle::XtVehicle*)C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];
#else
		Vehicle::XtVehicle* pXtVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex] ? &C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex]->baseVehicle : NULL;
#endif
		Vehicle::XtMaterialDefaults* pMaterialDefaults = NULL;
		if (C_GameVehicleData::Instance().mVehicleDefaults.Number() > 0)
			pMaterialDefaults = C_GameVehicleData::Instance().mVehicleDefaults[0]->materials;
		
		XtId id = 0;
		if (pXtVehicle && (int)g_debugPaintIndex < pXtVehicle->m_PaintJobs.count && pXtVehicle->m_PaintJobs[(int)g_debugPaintIndex])
			id = pXtVehicle->m_PaintJobs[(int)g_debugPaintIndex].Id();

		m_pVehicleModel->GetPaintJob()->UpdateMaterial(pXtVehicle, pMaterialDefaults, id);
	}

	if (g_debugSetupDamageMap)
	{
		if (m_pVehicleModel->GetDamageLayer())
		{
#ifdef AMAX_VEH_USE_MI
			Vehicle::XtVehicle* pXtVehicle = (Vehicle::XtVehicle*)C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];
#else
			Vehicle::XtVehicle* pXtVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex] ? &C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex]->baseVehicle : NULL;
#endif
			m_pVehicleModel->GetDamageLayer()->Setup(pXtVehicle);
		}
	}

	///UpdateLightCubeMap();

	if (g_debugTestCollisionDamping)
	{
		if (m_FramesSinceCollision >= (int)g_debugFramesUntilPhysicsCanIgnoreCollision)
		{
			m_FramesSinceCollision = 0;
			m_CanPhysicsIgnoreCollision = true;
		}
		else if (m_CollisionThisFrame)
		{
			m_CanPhysicsIgnoreCollision = false;
		}
		m_FramesSinceCollision++;
	}

	if (g_debugVehiclePrintPhysicsParts)
	{
		m_pVehicleModel->GetComponentList().PrintPhysicsParts();
		g_debugVehiclePrintPhysicsParts = false;
	}

#ifndef CFG_GOLD
	if (g_debugCreateConstraints)
	{
		if (g_ConstraintsFrameCount == 1)
		{
			Vehicle::XtVehicle* pXtVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex] ? &C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex]->baseVehicle : NULL;
			if (pXtVehicle)
				m_pVehicleModel->GetComponentList().Debug_CreateConstraints(pXtVehicle, m_pCollisionBody, m_Matrix);			
		}
		else if (g_ConstraintsFrameCount == 0)
		{
			for (S32 iPart = 0; iPart < m_pVehicleModel->GetComponentList().m_ActiveParts.Size(); iPart++)
				m_pVehicleModel->GetComponentList().m_ActiveParts[iPart].m_pNode->ActivateConstraints();
			g_debugCreateConstraints = false;
		}
		
		g_ConstraintsFrameCount--;
	} 
	else if (g_debugVehicleRecreateConstraints && !g_debugCreateConstraints)
	{
		m_pVehicleModel->GetComponentList().Debug_DestroyConstraints();
		g_debugCreateConstraints = true;
		g_ConstraintsFrameCount = g_NumFrameBeforeCreateConstraints;
		g_debugVehicleRecreateConstraints = false;
	}

	if (g_debugFakeFatalDamageParticles)
	{
		g_debugFakeFatalDamageParticles = false;

		//if (m_RadiatorSmokeEmitter.m_Intensity == 0.0f)
		//	m_RadiatorSmokeEmitter.m_Activate = true;
		//else if (m_RadiatorSmokeEmitter.m_Intensity == 1.0f)
		//	m_RadiatorSmokeEmitter.m_Deactivate = true;

		if (m_EngineSmokeLightEmitter.m_Intensity == 0.0f)
			m_EngineSmokeLightEmitter.m_Activate = true;
		else if (m_EngineSmokeLightEmitter.m_Intensity == 1.0f)
			m_EngineSmokeLightEmitter.m_Deactivate = true;
		
		if (m_EngineSmokeHeavyEmitter.m_Intensity == 0.0f)
			m_EngineSmokeHeavyEmitter.m_Activate = true;
		else if (m_EngineSmokeHeavyEmitter.m_Intensity == 1.0f)
			m_EngineSmokeHeavyEmitter.m_Deactivate = true;
		
		//if (m_EngineFireEmitter.m_Intensity == 0.0f)
		//	m_EngineFireEmitter.m_Activate = true;
		//else if (m_EngineFireEmitter.m_Intensity == 1.0f)
		//	m_EngineFireEmitter.m_Deactivate = true;
	}

#endif
}

void C_VehicleActor::HandleDefferedEvents()
{
	PERF_EVENT("C_VehicleActor.HandleDefferedEvents");

	if (!m_pVehicleModel)
		return;

	U32 numEffects = m_pVehicleModel->GetDamageLayer() ? m_pVehicleModel->GetDamageLayer()->GetNumDefferedEvents() : 0;
	if (numEffects == 0)
		return;

	//Process deffered effects			
	for (U32 iEffect = 0; iEffect < numEffects; iEffect++)
	{
		Vehicle::C_DefferedEvent* pEvent = m_pVehicleModel->GetDamageLayer()->GetDefferedEvent(iEffect);
		if (!pEvent)
			continue;

		if (pEvent->m_PartCode < Vehicle::E_PartCode_None || pEvent->m_PartCode >= Vehicle::E_PartCode_Max)
		{
			BIZ_ASSERT_FAIL("pEvent->m_PartCode(%d) is out of range! iEffect(%d), numEffects(%d)", pEvent->m_PartCode, iEffect, numEffects);
			continue;
		}

		if (pEvent->m_PartSubCode < Vehicle::E_PartSubCode_None || pEvent->m_PartSubCode >= Vehicle::E_PartSubCode_Max)
		{
			BIZ_ASSERT_FAIL("pEvent->m_PartSubCode(%d) is out of range! iEffect(%d), numEffects(%d)", pEvent->m_PartSubCode, iEffect, numEffects);
			continue;
		}

		C_VehicleAudioImpactMsg audioMsg;
		audioMsg.m_Direction	= pEvent->m_Direction;
		audioMsg.m_Position		= pEvent->m_Position;
		audioMsg.m_Event		= pEvent->m_Event;
		audioMsg.m_Scale		= pEvent->m_Scale;
		audioMsg.m_PartCode		= pEvent->m_PartCode;
		audioMsg.m_PartSubCode	= pEvent->m_PartSubCode;
		audioMsg.m_Attached		= true;

		//if (pEvent->m_Event >= Vehicle::E_DefferedEvent_WindowCrack && pEvent->m_Event <= Vehicle::E_DefferedEvent_LightSmash)
		//{
		//	pEvent->m_Scale = 1.0f;
		//}		

		//send to audio component
		if(QueryInterface(Components::E_AudioEntity_AdditionalSpawned)) //if present
		{
			if(audioMsg.m_Scale > g_debugAudioPartsMessageThreshold)
				m_pGameWorld->GetActorList()->SendMessageToActorComponent(GetCreationData()->GetActorHandle(),Components::E_AudioEntity_AdditionalSpawned,&audioMsg);
		}

		Vehicle::C_Part* pPart = m_pVehicleModel->GetComponent(pEvent->m_PartCode, pEvent->m_PartSubCode);

		if(m_EnableDeferredEventParticles)
		{
			switch (pEvent->m_Event)
			{
			case Vehicle::E_DefferedEvent_BurstTyre:
				SpawnParticleEffect(Game::Effects::E_MiscParticleEventType_BurstTyre, pEvent->m_Position, pEvent->m_Direction, pEvent->m_PointVelocity, pEvent->m_Scale, &m_LocalSpace, pPart);
				break;
			case Vehicle::E_DefferedEvent_WindowCrack:
				SpawnParticleEffect(Game::Effects::E_MiscParticleEventType_GlassCrack, pEvent->m_Position, pEvent->m_Direction, pEvent->m_PointVelocity, pEvent->m_Scale, &m_LocalSpace, pPart);
				break;
			case Vehicle::E_DefferedEvent_WindowSmash:
				SpawnParticleEffect(Game::Effects::E_MiscParticleEventType_GlassSmash, pEvent->m_Position, pEvent->m_Direction, pEvent->m_PointVelocity, pEvent->m_Scale, &m_LocalSpace, pPart);
				break;
			case Vehicle::E_DefferedEvent_LightCrack:
				SpawnParticleEffect(Game::Effects::E_MiscParticleEventType_GlassCrack, pEvent->m_Position, pEvent->m_Direction, pEvent->m_PointVelocity, pEvent->m_Scale, &m_LocalSpace, pPart);
				break;
			case Vehicle::E_DefferedEvent_LightSmash:
				SpawnParticleEffect(Game::Effects::E_MiscParticleEventType_GlassSmash, pEvent->m_Position, pEvent->m_Direction, pEvent->m_PointVelocity, pEvent->m_Scale, &m_LocalSpace, pPart);
				break;
			case Vehicle::E_DefferedEvent_ScrapeStatic:
				SpawnParticleEffect(Game::Effects::E_MiscParticleEventType_ScrapeStatic, pEvent->m_Position, pEvent->m_Direction, pEvent->m_PointVelocity, pEvent->m_Scale, &m_LocalSpace, NULL);
				break;
			case Vehicle::E_DefferedEvent_ScrapeDynamic:
				SpawnParticleEffect(Game::Effects::E_MiscParticleEventType_ScrapeDynamic, pEvent->m_Position, pEvent->m_Direction, pEvent->m_PointVelocity, pEvent->m_Scale, &m_LocalSpace, NULL);
				break;
			case Vehicle::E_DefferedEvent_PartOpened:
				//Audio: part just fully opened - use: pEvent->m_PartCode, pEvent->m_PartSubCode
				break;
			case Vehicle::E_DefferedEvent_PartClosed:
				//Audio: part just fully closed - use: pEvent->m_PartCode, pEvent->m_PartSubCode
				break;
			case Vehicle::E_DefferedEvent_LicensePlateFront:
			case Vehicle::E_DefferedEvent_LicensePlateBack:
				SpawnParticleEffect(Game::Effects::E_MiscParticleEventType_LicensePlate, pEvent->m_Position, pEvent->m_Direction, pEvent->m_PointVelocity, pEvent->m_Scale, &m_LocalSpace, NULL);
				break;
			default:
				continue;
				break;	
			}
		}
	}

	m_pVehicleModel->GetDamageLayer()->ClearDefferedEvents();

	HandleCollisionStates();
	
}

void C_VehicleActor::HandleCollisionStates()
{
	if(m_Enabled)
	{
		if (m_pCollisionBody && m_pCollisionBody->TestBodyFlags(Gravity::C_Body::E_TemporarilyRemoved))
		{
			GetPhysicsScene()->AddBody(m_pCollisionBody);
		}
	}
	else
	{
		if( m_pCollisionBody && m_pCollisionBody->TestBodyFlags(Gravity::C_Body::E_InsertedIntoWorld) &&
			!m_pCollisionBody->TestBodyFlags(Gravity::C_Body::E_TemporarilyRemoved))
		{
			GetPhysicsScene()->TemporarilyRemoveBody(m_pCollisionBody);
		}
	}	
}
/*
===============
NAME: SpawnParticleEffect

NOTE: Spawns a particle effect
===============
*/
void C_VehicleActor::SpawnParticleEffect(Game::Effects::E_MiscParticleEventType p_EventType, 
										Maths::HVec3 p_Position, Maths::HVec3 p_Direction, Maths::HVec3 p_Velocity,
										float p_Scale, 
										DynamicEffect::C_Node* p_pLocalSpace,
										Vehicle::C_Part* p_pPart, 										
										DynamicEffect::Particle::C_EventInstanceHandle* p_pHandle)
{
	PERF_EVENT("C_VehicleActor.SpawnParticleEffect");

	if (C_GameEffectData::Instance().mAmaxMiscEffects.Number() == 0)
		return;

	int mapping = C_GameEffectData::Instance().mAmaxMiscEffects[0]->effectMapping[p_EventType];
		
	if (mapping > -1 && C_GameEffectData::Instance().mAmaxMiscEffects[0]->effects[mapping].particleEvent)
	{
		DynamicEffect::C_Node node;
		if (!p_pPart || (p_pPart && !p_pPart->m_pSpawnMesh))
		{
			node.SetPosition(p_Position);
			node.SetRotation(DynamicEffect::GetQuaternionFromTo<Maths::C_Quaternion>(Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ(), p_Direction));
		}

		DynamicEffect::Particle::C_EventInstanceHandle emitter = C_GameEffectData::Instance().mAmaxMiscEffects[0]->effects[mapping].particleEvent->pEvent->Create(node, p_pLocalSpace);
		if(DynamicEffect::Particle::C_EventInstance* pEvent = emitter.Object())
		{
			if (p_pPart)
//			{
				pEvent->SetSpawnMesh(p_pPart->m_pSpawnMesh);
//
//				for (S32 bob = 0; bob < C_GameEffectData::Instance().mAmaxMiscEffects[0]->effects[mapping].particleEvent->emitters.count; bob++)
//				{
//					if (!p_pPart->m_pSpawnMesh && C_GameEffectData::Instance().mAmaxMiscEffects[0]->effects[mapping].particleEvent->emitters[bob].spawnParams.distributionType == DynamicEffect::Particle::E_EmitterDistribution_Mesh)
//						bool wtf = true;
//				}
//			}
			pEvent->SetLighting(m_pVehicleModel->GetLighting());
			pEvent->SetIntensity(p_Scale * m_ParticleEmitScale);
			pEvent->SetVelocity(p_Velocity);
			pEvent->SetCollisionPlane(m_GroundPosition, m_GroundNormal);
			pEvent->SetLifeScale(p_Scale);
			pEvent->SetAlpha(p_Scale);
			pEvent->SetVisibilityFlags(m_EffectsVisiblityFlags);

			if (p_pHandle)
				*p_pHandle = emitter;
		}
	}
}

//Debug::Bool g_debugOverrideRadiatorSmoke("Vehicle.Damage.Effects.RadiatorSmoke.OverrideIntensity", false);
//Debug::Float g_debugRadiatorSmokeLevel("Vehicle.Damage.Effects.RadiatorSmoke.Intensity", 1.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Bool g_debugOverrideEngineSmokeLight("Vehicle.Damage.Effects.EngineSmokeLight.OverrideIntensity", false);
Debug::Float g_debugEngineSmokeLightLevel("Vehicle.Damage.Effects.EngineSmokeLight.Intensity", 1.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Bool g_debugOverrideEngineSmokeHeavy("Vehicle.Damage.Effects.EngineSmokeHeavy.OverrideIntensity", false);
Debug::Float g_debugEngineSmokeHeavyLevel("Vehicle.Damage.Effects.EngineSmokeHeavy.Intensity", 1.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

//Debug::Bool g_debugOverrideEngineFire("Vehicle.Damage.Effects.EngineFire.Override", false);

Debug::Bool g_debugReloadLightTrails("Vehicle.Effects.LightTrails.Reload", false);

/*
===============
NAME: UpdateLightTrails

NOTE: Updates light trail effects
===============
*/
void C_VehicleActor::UpdateLightTrails(const Maths::C_RGBA_F32& lighting)
{
	if(m_EnableParticles && !m_ResetifyDriver)
	{
		PERF_EVENT("UpdateLightTrails");

		BIZ_ASSERT(C_GameEffectData::Instance().mAmaxMiscEffects.Number() > 0);
		
		float fMinSpeed = C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailMinSpeed;
		float fRangeRcp = C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailRangeRcp;
			
		float fSpeed = m_pVehicleModel->GetModelInput()->m_fSpeed;
		float fSpeedScale = Maths::Clamp((fSpeed - fMinSpeed) * fRangeRcp, 0.0f, 1.0f);

		// if visible in main view then update trails
		for (S32 iTrail = 0; iTrail < m_LightTrails.Size(); iTrail++)
		{
// 			if (!m_ParticleTrails[iTrail].m_ParticleEvent.Object() || g_debugReloadLightTrails)
// 				m_ParticleTrails[iTrail].Initialise(&m_LocalSpace, 1.0f);
			
			float fIntensity = 0.0f;

// 			#ifndef CFG_SPEED
// 			if (g_debugEnableParticleTrails)
// 			#endif
// 			{
// 				if (m_ParticleTrails[iTrail].m_PartCode == Vehicle::E_PartCode_BrakeLight)
// 					fIntensity = m_pVehicleModel->GetModelInput()->m_fBrake * fSpeedScale;
// 				else// if (m_ParticleTrails[iTrail].m_PartCode == Vehicle::E_PartCode_RearLight)
// 					fIntensity = m_pVehicleModel->GetModelInput()->m_bRearLights.m_Current ? fSpeedScale : 0.0f;
// 			}
// 
// 			// only use the life scale for the trails, not emit scale
// 			m_ParticleTrails[iTrail].Update(m_Matrix, lighting, fIntensity > 0.01f, m_ParticleLifeScale, 1.0f);
#ifndef CFG_GOLD
			if (g_DefaultRearLightTrailsUpdate)
			{
				if (m_LightTrails[iTrail].m_PartCode == Vehicle::E_PartCode_BrakeLight)
					m_LightTrails[iTrail].m_pLightTrailInstance->SetColour(Maths::C_RGBA_F32(C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailBreakColour.r, C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailBreakColour.g, C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailBreakColour.b, C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailBreakColour.a));
				else
					m_LightTrails[iTrail].m_pLightTrailInstance->SetColour(Maths::C_RGBA_F32(C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailRearColour.r, C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailRearColour.g, C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailRearColour.b, C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailRearColour.a));
				m_LightTrails[iTrail].m_pLightTrailInstance->SetCameraFadeFactor(C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailCameraFactor);
			}
#endif
			if (m_LightTrails[iTrail].m_PartCode == Vehicle::E_PartCode_BrakeLight)
				fIntensity = m_pVehicleModel->GetModelInput()->m_fBrake * fSpeedScale;
			else// if (m_ParticleTrails[iTrail].m_PartCode == Vehicle::E_PartCode_RearLight)
				fIntensity = m_pVehicleModel->GetModelInput()->m_bRearLights.m_Current ? fSpeedScale : 0.0f;

			fIntensity *= GetAlpha(E_VAC_TYPE_LIGHT_TRAILS);
			m_LightTrails[iTrail].m_pLightTrailInstance->SetViewFlags(m_EffectsVisiblityFlags);
			m_LightTrails[iTrail].m_pLightTrailInstance->Update(m_LightTrails[iTrail].m_Position * m_Matrix, m_Matrix.AX(), m_Matrix.AY(), fIntensity);

// 			#ifndef CFG_SPEED
// 			if (g_debugDrawParticleTrailEmitters)
// 				m_ParticleTrails[iTrail].DebugDraw(m_Matrix);
// 			#endif
		}

		#ifndef CFG_SPEED
		if (g_debugDrawExhaustEmitters)
		{		
			AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];
			if (pVehicle)
			{
				S32 iPos = 0;
				for (iPos = 0; iPos < m_ExhaustEffects.m_EmitPositions.Size(); iPos++)
					m_ExhaustEffects.m_EmitPositions[iPos].m_Exists = false;
				for (iPos = 0; iPos < pVehicle->exhaustEmitters.count; iPos++)
				{
					Maths::HVec3 pos(Load(pVehicle->exhaustEmitters[iPos].position));
					Maths::HVec3 dir(Load(pVehicle->exhaustEmitters[iPos].direction));
					dir.Normalise();
					m_ExhaustEffects.AddEmitPosition(pVehicle->exhaustEmitters[iPos].exhaust, pos, dir);
				}
			}
			m_ExhaustEffects.DebugDraw(m_Matrix);
		}
		#endif
	}
	else
	{
		// not visible so give back
// 		for (S32 iTrail = 0; iTrail < m_ParticleTrails.Size(); iTrail++)
// 			m_ParticleTrails[iTrail].Clear();

		for (S32 iTrail = 0; iTrail < m_LightTrails.Size(); iTrail++)
		{
			m_LightTrails[iTrail].m_pLightTrailInstance->Update(m_LightTrails[iTrail].m_Position * m_Matrix, m_Matrix.AX(), m_Matrix.AY(), 0.f);
		}
	}
}

//Debug::Bool g_debugReloadRadiatorSmoke("Vehicle.Damage.Effects.RadiatorSmoke.Reload", false);
Debug::Bool g_debugReloadEngineSmokeLight("Vehicle.Damage.Effects.EngineSmokeLight.Reload", false);
Debug::Bool g_debugReloadEngineSmokeHeavy("Vehicle.Damage.Effects.EngineSmokeHeavy.Reload", false);
//Debug::Bool g_debugReloadEngineFire("Vehicle.Damage.Effects.EngineFire.Reload", false);

///*
//===============
//NAME: CreateRadiatorSmoke
//
//NOTE: Creates radiator smoke
//===============
//*/
//void C_VehicleActor::CreateRadiatorSmoke()
//{
//	AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];
//	if (!pVehicle || !pVehicle->particleEffects)
//		return;
//	
//	m_RadiatorSmokeEmitter.Shutdown();
//
//	for (S32 iEmitter = 0; iEmitter < pVehicle->particleEffects->radiatorSmoke.emitters.count; iEmitter++)
//	{
//
//		m_RadiatorSmokeEmitter.Setup(Load(pVehicle->particleEffects->radiatorSmoke.emitters[iEmitter].position),
//									Load(pVehicle->particleEffects->radiatorSmoke.emitters[iEmitter].direction),
//									pVehicle->particleEffects->radiatorSmoke.emitters[iEmitter].particleEvent);
//	}
//}

/*
===============
NAME: CreateEngineSmoke

NOTE: Creates engine smoke
===============
*/
void C_VehicleActor::CreateEngineSmokeLight()
{
	AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];
	if (!pVehicle || !pVehicle->particleEffects)
		return;
	
	m_EngineSmokeLightEmitter.Shutdown();

	for (S32 iEmitter = 0; iEmitter < pVehicle->particleEffects->engineSmokeLight.emitters.count; iEmitter++)
	{

		m_EngineSmokeLightEmitter.Setup(Load(pVehicle->particleEffects->engineSmokeLight.emitters[iEmitter].position), 	
										Load(pVehicle->particleEffects->engineSmokeLight.emitters[iEmitter].direction),
										pVehicle->particleEffects->engineSmokeLight.emitters[iEmitter].particleEventIdle,
										pVehicle->particleEffects->engineSmokeLight.emitters[iEmitter].particleEventSpeed);
	}
}

/*
===============
NAME: CreateEngineSmoke

NOTE: Creates engine smoke
===============
*/
void C_VehicleActor::CreateEngineSmokeHeavy()
{
	AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];
	if (!pVehicle || !pVehicle->particleEffects)
		return;
	
	m_EngineSmokeHeavyEmitter.Shutdown();

	for (S32 iEmitter = 0; iEmitter < pVehicle->particleEffects->engineSmokeHeavy.emitters.count; iEmitter++)
	{

		m_EngineSmokeHeavyEmitter.Setup(Load(pVehicle->particleEffects->engineSmokeHeavy.emitters[iEmitter].position), 			
										Load(pVehicle->particleEffects->engineSmokeHeavy.emitters[iEmitter].direction),
										pVehicle->particleEffects->engineSmokeHeavy.emitters[iEmitter].particleEventIdle,
										pVehicle->particleEffects->engineSmokeHeavy.emitters[iEmitter].particleEventSpeed);
	}
}

///*
//===============
//NAME: CreateEngineFire
//
//NOTE: Creates engine fire
//===============
//*/
//void C_VehicleActor::CreateEngineFire()
//{	
//	//AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];
//	//if (!pVehicle || !pVehicle->particleEffects)
//	//	return;
//	//
//	//m_EngineFireEmitter.Shutdown();
//
//	//for (S32 iEmitter = 0; iEmitter < pVehicle->particleEffects->engineFire.emitters.count; iEmitter++)
//	//{
//
//	//	m_EngineFireEmitter.Setup(Load(pVehicle->particleEffects->engineFire.emitters[iEmitter].position), 			
//	//							  Load(pVehicle->particleEffects->engineFire.emitters[iEmitter].direction), 
//	//							  pVehicle->particleEffects->engineFire.emitters[iEmitter].particleEvent);
//	//}
//}

//Debug::Bool g_debugRenderRadiatorSmokeEmitters("Vehicle.Damage.Effects.RadiatorSmoke.DrawEmitters", false);
#ifndef CFG_SPEED
Debug::Bool g_debugRenderEngineSmokeLightEmitters("Vehicle.Damage.Effects.EngineSmokeLight.DrawEmitters", false);
Debug::Bool g_debugRenderEngineSmokeHeavyEmitters("Vehicle.Damage.Effects.EngineSmokeHeavy.DrawEmitters", false);
#endif
//Debug::Bool g_debugRenderEngineFireEmitters("Vehicle.Damage.Effects.EngineFire.DrawEmitters", false);

/*
===============
NAME: UpdateDamageEffects

NOTE: Update damage effects
===============
*/
void C_VehicleActor::UpdateDamageEffects(const Maths::C_RGBA_F32& lighting)
{
	if(!m_EnableParticles)
	{
		// remove the emitters rather than just not update them
		if(m_EngineSmokeLightEmitter.m_Created)
			m_EngineSmokeLightEmitter.Destroy();
		if(m_EngineSmokeHeavyEmitter.m_Created)
			m_EngineSmokeHeavyEmitter.Destroy();
		return;
	}

	//Engine damage smoke light
	{
#ifndef CFG_SPEED
		if (g_debugRenderEngineSmokeLightEmitters)
		{
			AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];
			if (pVehicle && pVehicle->particleEffects)
			{
				for (S32 iEmitter = 0; iEmitter < pVehicle->particleEffects->engineSmokeLight.emitters.count; iEmitter++)
				{	
					const Maths::HVec3 pos = Load(pVehicle->particleEffects->engineSmokeLight.emitters[iEmitter].position) * m_Matrix;
					const Maths::HVec3 dir = Load(pVehicle->particleEffects->engineSmokeLight.emitters[iEmitter].direction) * m_Matrix.Rotation();
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(pos, dir, 0.5f, 0.1f, Maths::RGBA(0, 255, 255));
				}
			}
		}

		PERF_EVENT("UpdateDamageEffects::Smoke");
#ifndef CFG_GOLD
		if (g_debugReloadEngineSmokeLight)
			CreateEngineSmokeLight();
		else 
#endif
		if (g_debugOverrideEngineSmokeLight)
		{
			m_EngineSmokeLightEmitter.m_Intensity = (float)g_debugEngineSmokeLightLevel;
			if (!m_EngineSmokeLightEmitter.m_Created)
				m_EngineSmokeLightEmitter.Create(&m_LocalSpace, 1.0f);
		}
#endif
		if (m_EngineSmokeLightEmitter.m_Activate)
			m_EngineSmokeLightEmitter.Create(&m_LocalSpace, 1.0f);		
		m_EngineSmokeLightEmitter.Update(&m_LocalSpace, lighting, m_ParticleEmitScale, m_pVehicleModel->GetModelInput()->m_fSpeed, m_pVehicleModel->GetModelInput()->m_vVelocity, m_EffectsVisiblityFlags);
	}

	//Engine damage smoke heavy
	{		
#ifndef CFG_SPEED
		if (g_debugRenderEngineSmokeHeavyEmitters)
		{
			AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];
			if (pVehicle && pVehicle->particleEffects)
			{
				for (S32 iEmitter = 0; iEmitter < pVehicle->particleEffects->engineSmokeHeavy.emitters.count; iEmitter++)
				{
			
					const Maths::HVec3 pos = Load(pVehicle->particleEffects->engineSmokeHeavy.emitters[iEmitter].position) * m_Matrix;
					const Maths::HVec3 dir = Load(pVehicle->particleEffects->engineSmokeHeavy.emitters[iEmitter].direction) * m_Matrix.Rotation();
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(pos, dir, 0.5f, 0.1f, Maths::RGBA(0, 0, 255));
				}
			}
		}

		PERF_EVENT("UpdateDamageEffects::Smoke");
#ifndef CFG_GOLD
		if (g_debugReloadEngineSmokeHeavy)
			CreateEngineSmokeHeavy();
		else 
#endif
		if (g_debugOverrideEngineSmokeHeavy)
		{
			m_EngineSmokeHeavyEmitter.m_Intensity = (float)g_debugEngineSmokeHeavyLevel;
			if (!m_EngineSmokeHeavyEmitter.m_Created)
				m_EngineSmokeHeavyEmitter.Create(&m_LocalSpace, 1.0f);
		}
#endif
		if (m_EngineSmokeHeavyEmitter.m_Activate)
			m_EngineSmokeHeavyEmitter.Create(&m_LocalSpace, 1.0f);		
		m_EngineSmokeHeavyEmitter.Update(&m_LocalSpace, lighting, m_ParticleEmitScale, m_pVehicleModel->GetModelInput()->m_fSpeed, m_pVehicleModel->GetModelInput()->m_vVelocity, m_EffectsVisiblityFlags);
	}

	////Engine damage fire
	//{	
	//	if (g_debugRenderEngineFireEmitters)
	//	{
	//		AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];
	//		if (pVehicle && pVehicle->particleEffects)
	//		{
	//			for (S32 iEmitter = 0; iEmitter < pVehicle->particleEffects->engineFire.emitters.count; iEmitter++)
	//			{
	//		
	//				const Maths::HVec3 pos = Load(pVehicle->particleEffects->engineFire.emitters[iEmitter].position) * m_Matrix;
	//				const Maths::HVec3 dir = Load(pVehicle->particleEffects->engineFire.emitters[iEmitter].direction) * m_Matrix.Rotation();
	//				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(pos, dir, 0.5f, 0.1f, Maths::RGBA(255, 0, 0));
	//			}
	//		}
	//	}

//		PERF_EVENT("UpdateDamageEffects::Fire");
//#ifndef CFG_GOLD
//		if (g_debugReloadEngineFire)
//			CreateEngineFire();
//		else
//#endif
//		if (g_debugOverrideEngineFire)
//		{
//			m_EngineFireEmitter.m_Intensity =  1.0f;
//			if (!m_EngineFireEmitter.m_Created)
//				m_EngineFireEmitter.Create(&m_LocalSpace, 1.0f);
//		}
//		if (m_EngineFireEmitter.m_Activate)
//			m_EngineFireEmitter.Create(&m_LocalSpace, 1.0f);		
//		m_EngineFireEmitter.Update(lighting, m_ParticleEmitScale);
	//}
}


/*
===============
NAME: UpdateHeadlights

NOTE: Updates positions of headlights owned by this vehicle
===============
*/

void C_VehicleActor::UpdateHeadlights()
{
	PERF_EVENT("UpdateHeadlights");

	if (SceneGraph::C_SpotLight* pSpotLight = m_HeadlightHandle)
	{
		if (m_pVehicleModel && m_pVehicleModel->GetHeadlight())
		{
			
			m_pVehicleModel->GetHeadlight()->m_IntensityModifier = m_pVehicleModel->GetHeadLightIntensity();

			Vehicle::C_Headlight* pHeadlight = m_pVehicleModel->GetHeadlight();

			F32 headlightIntensity = pHeadlight->GetIntensity();
			F32 bumperCameraHeadlightIntensityOffset = 0.f;
			if (AmaxCity* pCity = C_GameData::Instance().mCities[(GAMEDATA_ID)C_Game::GetInstance()->GetLevelState().mCity])
			{
				headlightIntensity *= pCity->headlightIntensityScale;
				bumperCameraHeadlightIntensityOffset = pCity->bumperCameraHeadlightIntensityOffset;
			}

			if (!m_pVehicleModel->GetModelInput()->m_bHeadLights.m_Current || !m_pVehicleModel->GetModelInput()->m_bHeadLightCones.m_Current)
				headlightIntensity = 0.0f;

			if(!m_Enabled)
				headlightIntensity = 0.0f;

			const Maths::HF32 intensity(headlightIntensity * g_HeadlightIntensity * GetAlpha(E_VAC_TYPE_DYNAMIC_LIGHTS));

			const F32 radius = pHeadlight->m_Length;
			const F32 fieldOfViewX = atanf(0.5f * pHeadlight->m_Width / pHeadlight->m_Length);
			const F32 fieldOfViewY = atanf(0.5f * pHeadlight->m_Height / pHeadlight->m_Length);

			pSpotLight->SetColour(pHeadlight->m_Colour * intensity);
			pSpotLight->SetCullDistance(Maths::Lerp(g_HeadlightCullDistanceLOD, g_HeadlightCullDistance, Lod::C_LodManager::Instance().GetGpuDetail()));

			if (g_UseCutSceneHeadlightNearCull)
				pSpotLight->SetNearCull(g_CutSceneHeadlightNearCull);
			else
				pSpotLight->SetNearCull(Maths::Lerp(g_HeadlightNearCullLOD, g_HeadlightNearCull, Lod::C_LodManager::Instance().GetGpuDetail()));

			pSpotLight->SetDimensions(pHeadlight->m_WorldMatrix, radius, fieldOfViewX, fieldOfViewY);
			pSpotLight->SetFlags(1<<m_VehicleIndex);

			pSpotLight->m_pData = m_pHeadlightTexture;
			pSpotLight->m_RenderFlags = SceneGraph::E_SpotLightType_Volume;
			if (m_IsBumperCamera)
				pSpotLight->SetPerViewIntensity(E_ViewType_Main1 + m_ViewportID, bumperCameraHeadlightIntensityOffset + 1.f);
			else
				pSpotLight->SetPerViewIntensity(E_ViewType_Main1 + m_ViewportID, 1.f);

			if (g_HeroLightEnable && m_IsFocusVehicle && !m_IsSplitscreen)
			{
				//Added if null
				SceneGraph::C_PointLight* pHeroLight = m_HeroLightHandle;
				if (pHeroLight)
				{
					pHeroLight->SetVisibilityFlags(0xFFFFFFFF);
					Vehicle::C_DynamicLight* pVehicleHeadlight = m_pVehicleModel->GetHeroLight();

					//Distant from floor
					F32 distant = m_Matrix.AW().Y() - m_GroundPosition.Y();
					F32 factor = Maths::Clamp(2.f - distant, 0.f, 1.f);

 					m_pVehicleModel->GetHeroLight()->m_Offset = Maths::HVec3(F32(g_DefaultHeroLightOffsetX), F32(g_DefaultHeroLightOffsetY), F32(g_DefaultHeroLightOffsetZ));
 					m_pVehicleModel->GetHeroLight()->m_Colour = MathsStorage::Float3(g_DefaultHeroLightColour->r * g_DefaultHeroLightColour->a/2.f, g_DefaultHeroLightColour->g * g_DefaultHeroLightColour->a/2.f, g_DefaultHeroLightColour->b * g_DefaultHeroLightColour->a/2.f);
 					m_pVehicleModel->GetHeroLight()->m_Radius =g_DefaultHeroLightRadius;

 					pHeroLight->SetColour(pVehicleHeadlight->m_Colour.x * intensity * factor, pVehicleHeadlight->m_Colour.y * intensity * factor, pVehicleHeadlight->m_Colour.z * intensity * factor);
 					pHeroLight->SetCullDistance(g_HeadlightCullDistance);
 					pHeroLight->SetNearCull(g_HeadlightNearCull);
 					pHeroLight->SetDimensions(pVehicleHeadlight->m_Offset * pHeadlight->m_WorldMatrix, pVehicleHeadlight->m_Radius);
 					pHeroLight->SetFlags(1<<m_VehicleIndex);

					if (m_IsBumperCamera)
						pHeroLight->SetPerViewIntensity(E_ViewType_Main1 + m_ViewportID, bumperCameraHeadlightIntensityOffset + 1.f);
					else
						pHeroLight->SetPerViewIntensity(E_ViewType_Main1 + m_ViewportID, 1.f);
				}
			}
			else
			{
				//Removed if needed
				SceneGraph::C_PointLight* pHeroLight = m_HeroLightHandle;
				if (pHeroLight)
				{
					pHeroLight->SetVisibilityFlags(0);
				}
			}
		}
	}
}

/*
===============
NAME: UpdatePickupSFX

NOTE: Updates any effects of being hit by pickups on the car
===============
*/
void C_VehicleActor::UpdatePickupSFX(const float frameTime)
{
	PERF_EVENT("UpdatePickupSFX");

	Btl::Vector<VehiclePickupEffect>			newSFXArray;

	//Go through each pickup SFX
	float effectTime = 0.f;
	for(int effectIndex = 0; effectIndex < mPickupSFXArray.Size(); effectIndex++)
	{
		mPickupSFXArray[effectIndex].mTime += frameTime;
		mPickupSFXArray[effectIndex].m_pSFX->UpdateWorldMatrix( m_Matrix );

		
		if(mPickupSFXArray[effectIndex].m_pSFX->Update( mPickupSFXArray[effectIndex].mTime ))
			newSFXArray.PushBack(mPickupSFXArray[effectIndex]);
		else
		{
			mPickupSFXArray[effectIndex].m_pSFX->Stop();
			SAFE_DELETE_PTR (mPickupSFXArray[effectIndex].m_pSFX);
		}
	}

	mPickupSFXArray = newSFXArray;
}

/*
===============
NAME: AddPickupSFX

NOTE: Adds a new pickup SFX
===============
*/
void C_VehicleActor::AddPickupSFX(const VehiclePickupEffect newPickupSFX)
{
	mPickupSFXArray.PushBack(newPickupSFX);
}

Debug::Bool g_debugEnableInAirDownForce("Vehicle.GamePhysicsMechanics.InFlightDownForce.Enable", true);
			
/*
===============
NAME: UpdateFlightControl

NOTE: Updates the flight control mechanic
===============
*/
void C_VehicleActor::UpdateFlightControl()
{
	if (!m_pVehicleModel)
		return;
	else if(!g_debugFlightControlEnable)					//CA - To allow us to disable flight control so we don't mess up the shunt/shock effects
		return;
	//else if(m_PerformanceModifiers.mDisableHandOfGod)		//CA - Make sure that if we've been attacked by a pickup
	//	return;

	PERF_EVENT("C_VehicleActor.UpdateFlightControl");

	S32 numWheels = m_pVehicleModel->GetModelInput()->m_Wheels.Size();

	if (numWheels < 2)
		return;

	bool engage = false;

	if (!m_pVehicleModel->GetModelInput()->m_Wheels[0].m_bOnGround &&
		!m_pVehicleModel->GetModelInput()->m_Wheels[1].m_bOnGround)
		engage = true;

	if (!engage)
	{
		m_fAirTime = 0.0f;
		return;
	}

	float fDampTime = C_GameVehicleData::Instance().mVehicleDefaults[0]->flightControl.dampingTime;
	float fScale = Maths::Clamp((m_fAirTime / fDampTime), 0.0f, 1.0f);
	if (!C_GameVehicleData::Instance().mVehicleDefaults[0]->flightControl.increaseForceOverTime)
		fScale = 1.0f - fScale;

	float fTorque = C_GameVehicleData::Instance().mVehicleDefaults[0]->flightControl.torque;

	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	const Maths::HMat33 rot(m_pCollisionBody->Orientation());
	const Maths::HVec3 angMom = m_pCollisionBody->GetBody().m_AngularMomentum;
	const Maths::HVec3 angTorque(angMom/m_FrameTime);
	const Maths::HF32 angTorqueX(angTorque | rot.AX());
	
	Maths::HVec3 torque(-Maths::Max(Maths::Min(Maths::HF32(fTorque * fScale), angTorqueX), -Maths::HF32(fTorque * fScale)), 0.0f, 0.0f);

	torque = torque * m_pCollisionBody->Orientation();

	m_pCollisionBody->AddTorque(torque);
		
	m_fAirTime += m_FrameTime;
}

Debug::Bool g_debugGroundedDownForceEnable("Vehicle.GamePhysicsMechanics.GroundedDownForce.Enable", false);
Debug::Bool g_debugGroundedDownForceAllWheelsOnGround("Vehicle.GamePhysicsMechanics.GroundedDownForce.NeedsAllWheelsOnGroundToApply", false);

//Debug::Float g_debugGroundedDownForce("Vehicle.GamePhysicsMechanics.GroundedDownForce.Force", 1000000.0f, Debug::Range(0.0f, 1000000.0f), Debug::Step(0.001f));
//Debug::Float g_debugGroundedDownForceTime("Vehicle.GamePhysicsMechanics.GroundedDownForce.Duration", 2.0f, Debug::Range(0.0f, 5.0f), Debug::Step(0.001f));

/*
===============
NAME: UpdateDownForce

NOTE: Updates the down-force mechanic
===============
*/
void C_VehicleActor::UpdateDownForce()
{
	if (!m_pVehicleModel)
		return;
	//else if(m_PerformanceModifiers.mDisableHandOfGod)		//CA - Make sure that if we've been attacked by a pickup
	//	return;

	PERF_EVENT("C_VehicleActor.UpdateDownForce");

	S32 numWheels = m_pVehicleModel->GetModelInput()->m_Wheels.Size();

	if (numWheels != 4)
		return;

	if (C_GameVehicleData::Instance().mVehicleDefaults.Number() == 0 || !C_GameVehicleData::Instance().mVehicleDefaults[0])
		return;

	float fInFlightDownForce			= C_GameVehicleData::Instance().mVehicleDefaults[0]->flightControl.inFlightDownForce;
	float fInFlightDownForceDuration	= C_GameVehicleData::Instance().mVehicleDefaults[0]->flightControl.inFlightDownForceDuration;
	float fInFlightDownForceStart		= C_GameVehicleData::Instance().mVehicleDefaults[0]->flightControl.inFlightDownForceStartTime;

	float fGroundedDownForce			= C_GameVehicleData::Instance().mVehicleDefaults[0]->flightControl.groundedDownForce;
	float fGroundedDownForceDuration	= C_GameVehicleData::Instance().mVehicleDefaults[0]->flightControl.groundedDownForceDuration;

	//Grounded down-force
	if (g_debugGroundedDownForceEnable)
	{
		//Have we just grounded?
		if (g_debugGroundedDownForceAllWheelsOnGround)
		{
			if (m_pVehicleModel->GetModelInput()->m_bAllWheelsOnGround && m_bCatchingAir)
			{
				m_bCatchingAir	= false;
				m_fGroundedDownForceTime = fGroundedDownForceDuration;
			}
		}
		else
		{
			if (m_bCatchingAir && (	m_pVehicleModel->GetModelInput()->m_Wheels[0].m_bOnGround ||
									m_pVehicleModel->GetModelInput()->m_Wheels[1].m_bOnGround ||
									m_pVehicleModel->GetModelInput()->m_Wheels[2].m_bOnGround ||
									m_pVehicleModel->GetModelInput()->m_Wheels[3].m_bOnGround))
			{
				m_bCatchingAir	= false;
				m_fGroundedDownForceTime = fGroundedDownForceDuration;
			}			
		}

		//If we are applying grounded downforce, do it!
		if (m_fGroundedDownForceTime >= 0.0f)
		{
			float fGroundedDownForceScale = Maths::Clamp(m_fGroundedDownForceTime / fGroundedDownForceDuration, 0.0f, 1.0f);
		
			Maths::HVec3 vGroundedDownForce(0.0f, -fGroundedDownForce * m_FrameTime * fGroundedDownForceScale, 0.0f);
			m_pCollisionBody->AddForce(vGroundedDownForce);		
		}

		//If applying force, continue countdown
		if (m_fGroundedDownForceTime >= 0.0f)
			m_fGroundedDownForceTime -= m_FrameTime;
		//We're not applying any down-force, so we'd better check if we're catching any air for the next bout of fixage
		else if (!m_pVehicleModel->GetModelInput()->m_Wheels[0].m_bOnGround &&
				!m_pVehicleModel->GetModelInput()->m_Wheels[1].m_bOnGround &&
				!m_pVehicleModel->GetModelInput()->m_Wheels[2].m_bOnGround &&
				!m_pVehicleModel->GetModelInput()->m_Wheels[3].m_bOnGround)
		{
			m_bCatchingAir = true;
		}
	}

	if (g_debugEnableInAirDownForce)
	{
		bool engage = false;

		if (!m_pVehicleModel->GetModelInput()->m_Wheels[0].m_bOnGround &&
			!m_pVehicleModel->GetModelInput()->m_Wheels[1].m_bOnGround)
			engage = true;
		else
			m_bInAirDownForceEngaged = false;

		if (engage && !m_bInAirDownForceEngaged)
		{
			m_bInAirDownForceEngaged	= true;
			m_fAirDownForceTime			= 0.0f;
		}
			
		if (m_bInAirDownForceEngaged)
		{
			if (m_fAirDownForceTime > fInFlightDownForceStart && m_fAirDownForceTime <= (fInFlightDownForceStart + fInFlightDownForceDuration))
			{
				float fDownForceScale = 1.0f - Maths::Clamp((m_fAirDownForceTime - fInFlightDownForceStart) / fInFlightDownForceDuration, 0.0f, 1.0f);
				Maths::HVec3 vDownForce(0.0f, -fInFlightDownForce * m_FrameTime * fDownForceScale, 0.0f);
				m_pCollisionBody->AddForce(vDownForce);		
			}

			m_fAirDownForceTime += m_FrameTime;
		}
	}
}

Debug::Bool g_debugDisablePerformanceModifiers("Vehicle.DisablePerformanceModifiers", false);

Debug::Bool g_debugEnableSteeringPull("Vehicle.Damage.Wheels.PerformanceModifiers.EnableSteeringPull", false);
Debug::Float g_debugWheelLossPullScale("Vehicle.Damage.Wheels.PerformanceModifiers.WheelLossSteeringPullScale", 1.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugTyreLossPullScale("Vehicle.Damage.Wheels.PerformanceModifiers.TyreLossSteeringPullScale", 0.18f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugWheelDamagePullAmount("Vehicle.Damage.Wheels.PerformanceModifiers.PullAmount", 0.55f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugWheelDamagePullNoise("Vehicle.Damage.Wheels.PerformanceModifiers.PullNoise", 0.45f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Float g_debugRPMLimiterScale("Vehicle.RPMScale", 1.0f, Debug::Range(0.0f, 50.0f), Debug::Step(0.001f));

static Util::C_Random g_SteeringPullRandom;

Debug::Bool g_debugBoostHack("Vehicle.Lol.BoostHackGO", false);
Debug::Bool g_debugDisableBoostHackWhenBraking("Vehicle.Lol.DisableBoostWhenBraking", true);
Debug::Float g_debugBoostHackMaxSteer("Vehicle.Lol.MinSteer", 0.25f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugBoostHackMinDot("Vehicle.Lol.MinDot", 0.5f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugBoostHackMinSpeed("Vehicle.Lol.MinSpeed", 15.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugBoostHackMaxSpeed("Vehicle.Lol.MaxSpeed", 40.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Logger &g_AssistBoostLog(Debug::GetLogger("Amax.Multiplayer.Balance2"));

Debug::Float g_debugVehicleTransverseGripScale("Vehicle.TransverseGripScale", -1.0f, Debug::Range(0.0f, 2.0f), Debug::Step(0.001f));

/*
===============
NAME: ApplyPerformanceModifiers

NOTE: Effects the controls of the vehicle based on pickup effects
===============
*/
void C_VehicleActor::ApplyPerformanceModifiers(RacingPhysics::CarControls* pControls)
{
	if (g_debugDisablePerformanceModifiers)
		return;

	Gravity::C_Body* pBody = GetCollisionBody();

	if (g_debugVehicleTransverseGripScale >= 0.0f)
		m_pVehiclePhysics->PerformanceModifiers().transverseGripScale = (float)g_debugVehicleTransverseGripScale;

	if(m_PerformanceModifiers.mForce.mModifier.Length())
	{
		pBody->ApplyForce(m_PerformanceModifiers.mForce.mModifier,m_PerformanceModifiers.mPosition.mModifier);

		m_PerformanceModifiers.mForce.Set(true, Maths::HVec3());
		m_PerformanceModifiers.mPosition.Set(true, Maths::HVec3());
	}

	bool aiControlled = false;
	if( m_pVehiclePhysics->GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_AI)
		aiControlled = true;

	bool allWheelsOnGround = m_pVehicleModel->GetModelInput()->m_bAllWheelsOnGround;
	float speedInMetresPerSecond = m_pVehicleModel->GetModelInput()->m_fMagVelocity;
	float mass = pBody->Mass(),
		torque = 0.f;

	Maths::HVec3 centreOfMassPosition = pBody->GetBody().m_CenterOfMassOffset;
	Maths::HVec3 z = Maths::HVec3(0.f, 0.f, 1.f);
	Maths::HVec3 angularVelocity = pBody->AngularVelocity();
	
	Maths::HVec3 rotationDirection,vTorque;
	
	if(m_PerformanceModifiers.mBoost.mModifier != 0.f)
	{
		float fForce = m_PerformanceModifiers.mBoost.mModifier * m_FrameTime;
		Maths::HVec3 vForce = z * fForce;
		float fMaxForce = 1000.f * GetCollisionBody()->GetBody().Mass();
		if (fForce > fMaxForce)
		{
			LOGERROR(g_VehicleActorLog, "mBoost is too large! (%f)", fForce);
			vForce = z * fMaxForce * 0.75f;
		}
		GetCollisionBody()->ApplyImpulseLocalSpace(vForce , centreOfMassPosition);
		
		m_PerformanceModifiers.mBoost.Set(true,0.f);
	}

	if(m_PerformanceModifiers.mDraftBoost.mModifier > 0.f)
	{
		float fForce = m_PerformanceModifiers.mDraftBoost.mModifier * m_FrameTime;
		Maths::HVec3 vForce = z * fForce;
		float fMaxForce = 1000.f * GetCollisionBody()->GetBody().Mass();
		if (fForce > fMaxForce)
		{
			LOGERROR(g_VehicleActorLog, "mDraftBoost is too large! (%f)", fForce);
			vForce = z * fMaxForce * 0.75f;
		}
		GetCollisionBody()->ApplyImpulseLocalSpace(vForce , centreOfMassPosition);
	}

	//	Apply assisted boost (Multiplayer rubber-banding)
	if(m_PerformanceModifiers.mAssistBoost.mModifier && m_pVehicleModel->GetModelInput()->m_bAllWheelsOnGround)
	{
		float fBrakeScale = 1.0f;
		if (g_debugDisableBoostHackWhenBraking)
		{
			if (pControls->brake > 0.0f || pControls->handbrake)
				fBrakeScale = 0.0f;
		}
		float fSteerScale = Maths::Abs(pControls->steer);
		fSteerScale = Maths::Clamp(((float)g_debugBoostHackMaxSteer - fSteerScale) / (float)g_debugBoostHackMaxSteer, 0.0f, 1.0f);
		float fBoost = m_PerformanceModifiers.mAssistBoost.mModifier;
		Maths::HVec3 vDir = m_pVehicleModel->GetModelInput()->m_vVelocity * m_Matrix.Rotation().Inverse();
		vDir.Normalise();
		float fDot = z | vDir; 
		fDot = Maths::Clamp(fDot, 0.0f, 1.0f);
		float fSpeedScale = Maths::Clamp((m_pVehicleModel->GetModelInput()->m_fSpeed - (float)g_debugBoostHackMinSpeed) / ((float)g_debugBoostHackMaxSpeed - (float)g_debugBoostHackMinSpeed), 0.0f, 1.0f);
		float fDotScale = Maths::Clamp((fDot - (float)g_debugBoostHackMinDot) / (1.0f - (float)g_debugBoostHackMinDot), 0.0f, 1.0f);
		fBoost *= fSteerScale * fBrakeScale * fSpeedScale * fDotScale;

/*		if(fBoost > 0)
			LOGWARN(g_AssistBoostLog, "Applying assist boost of %d", (int)fBoost);*/

		GetCollisionBody()->ApplyImpulseLocalSpace( z * fBoost * m_FrameTime, centreOfMassPosition);
	}

	//if (m_PerformanceModifiers.mBoostSteering && !aiControlled)
	//{
	//	// Darren : Handling nitro
	//	//			1. We need to increase the amount of turning the car can achieve, use torque...
	//	//			2. We need to add in a lateral force, similar principle to a basic wheel,  but applied to the body as a whole,
	//	//				this essentially improves cornering grip.
	//	//			3. Using 1.0f / 60.0f as a time constant.
	//	//			4. Might need to be tuned for all vehicles ???
	//	//			5. Need to make sure the AI does not use this, it has it`s own version.
	//	//			6. Might want to include angular velocity damping in the equation...

	//	const float t = 1.0f / 60.0f;
	//	if(allWheelsOnGround)
	//	{
	//		if (0)
	//		{
	//			// steering torque...
	//			if (Maths::Abs(pControls->steer) > 0.01f)
	//			{
	//				Maths::HVec3 torque = pBody->GetBody().m_OrientationMatrix.AY();
	//				float minSpeed = Maths::Max(speedInMetresPerSecond, 20.0f);
	//				// we probably want to cap the speed, set at 100 mps
	//				minSpeed = Maths::Min(minSpeed, 100.0f);
	//				torque *= 12.0f * -pControls->steer * mass * minSpeed; 
	//				pBody->GetBody().AddTorque(torque * t);
	//			}

	//			// Cornering grip
	//			Maths::HVec3 offset = centreOfMassPosition;			
	//			Maths::HVec3 rightVelDir = pBody->GetBody().m_OrientationMatrix.AX() * (pBody->GetBody().m_OrientationMatrix.AX() | pBody->GetBody().m_LinearVelocity);
	//			Maths::HVec3 force = rightVelDir * -30000.0f; // 10000 - 50000, might be the limit.
	//			const Maths::HMat33 inverse(pBody->GetBody().m_OrientationMatrix.InverseRotation());
	//			Maths::HVec3 f = force * inverse;  
	//			pBody->GetBody().ApplyImpulseLocalSpace( f * t, offset);
	//		}
	//		else
	//		{
	//			Maths::HVec3 offset = centreOfMassPosition;	
	//			const Maths::HMat33 inverse(pBody->GetBody().m_OrientationMatrix.InverseRotation());
	//			Maths::HVec3 angVel = pBody->GetBody().m_AngularVelocity;
	//			float speed =  pBody->GetBody().m_LinearVelocity | pBody->GetBody().m_OrientationMatrix.AZ();
	//			float cfAccel;
	//			cfAccel = (angVel | pBody->GetBody().m_OrientationMatrix.AY()) * speed * 1.5f;
	//			Maths::HVec3 force = -pBody->GetBody().m_OrientationMatrix.AX() * cfAccel * mass ;
	//			Maths::HVec3 f = force * inverse;  
	//			pBody->GetBody().ApplyImpulseLocalSpace( f * t, offset);
	//		}
	//	}
	//}

	if (g_debugEnableSteeringPull && !aiControlled)
	{	
		float fPull = 0.0f;
		bool bDoPull = false;
		for (S32 iWheel = 0; iWheel < m_pVehicleModel->GetModelInput()->m_Wheels.Size(); iWheel++)
		{
			Vehicle::C_WheelDamage* pWheelDamage = m_pVehicleModel->GetDamageLayer()->GetWheel(iWheel);

			if ((iWheel == 0 || iWheel == 1) && pWheelDamage)
			{
				bool isLeft = iWheel == 0 ? true : false;
				if (pWheelDamage->m_Detached)
				{
					fPull += isLeft ? -(float)g_debugWheelLossPullScale : (float)g_debugWheelLossPullScale;
					bDoPull = true;
				}
				else if (pWheelDamage->m_TyreBurst)
				{
					fPull += isLeft ? -(float)g_debugTyreLossPullScale : (float)g_debugTyreLossPullScale;
					bDoPull = true;
				}
			}
		}

		if (g_debugEnableSteeringPull && bDoPull)
		{
			float fPullAmount = (float)g_debugWheelDamagePullAmount;
			if ((float)g_debugWheelDamagePullNoise != 0.0f)
			{
				float fNoise = (float)g_debugWheelDamagePullNoise;
				fNoise = g_SteeringPullRandom.FRand(-fNoise, fNoise);
				fPull += fNoise;
			}
			float fSteer = Maths::Clamp(pControls->steer + fPull * fPullAmount, -1.0f, 1.0f);
			if (pControls->steer > 0.0f)
				pControls->steer = Maths::Lerp(fSteer, 1.0f, pControls->steer);
			else if (pControls->steer < 0.0f)
				pControls->steer = Maths::Lerp(-1.0f, fSteer, 1.0f - Maths::Abs(pControls->steer));
			else
				pControls->steer = fSteer;
		}
	}

/*		
		//if(pVehicle->mPerformanceModifiers.mLever.LengthSquared() > 0.f)
		{
			//See if we're going to be rotating the spinning vehicle 
			if(abs(m_PerformanceModifiers.mRotateY) > 0.f)
			{
				rotationDirection = m_Matrix.AY();
				torque = m_PerformanceModifiers.mRotateY * m_FrameTime;
				pBody->GetBody().AddTorque( rotationDirection * torque);
			}

			//See if we're going to be flipping and/or rolling the vehicle
			if( abs(m_PerformanceModifiers.mRotateXZ) > 0.f)
			{
				rotationDirection = m_PerformanceModifiers.mLever;
				torque = m_PerformanceModifiers.mRotateXZ * m_FrameTime;
				pBody->GetBody().AddTorque( rotationDirection * torque);
			}
		}
		

		if( (m_PerformanceModifiers.mSlowDownTime - m_FrameTime) > 0.f)
			m_PerformanceModifiers.mSlowDownTime -= m_FrameTime;
		else
			m_PerformanceModifiers.mSlowDownTime = 0.f;

		if( (m_PerformanceModifiers.mRotationTime - m_FrameTime) > 0.f)
		{
			vTorque = Maths::HVec3();

			//Difference in percent between the desired angular velocity and the actual
			Maths::HVec3 percentageError = Maths::HVec3();

			//Get the current angular rotation velocity
			float currentAngularVelocityX = abs(angularVelocity.X());
			float currentAngularVelocityY = abs(angularVelocity.Y());
			float currentAngularVelocityZ = abs(angularVelocity.Z());

			float desiredAngularVelocityX = abs(m_PerformanceModifiers.mDesiredAngularVelocity.X());
			float desiredAngularVelocityY = abs(m_PerformanceModifiers.mDesiredAngularVelocity.Y());
			float desiredAngularVelocityZ = abs(m_PerformanceModifiers.mDesiredAngularVelocity.Z());


			if(desiredAngularVelocityY > 0.f)
			{
				percentageError.Y( 1.f - Maths::Clamped( (currentAngularVelocityY / desiredAngularVelocityY), -1.f, 1.f) );
				vTorque.Y(m_PerformanceModifiers.mAccelerationTorque * m_FrameTime * percentageError.Y());
			}

			if(desiredAngularVelocityX > 0.f)
			{
				percentageError.X( 1.f - Maths::Clamped( (currentAngularVelocityX / desiredAngularVelocityX), -1.f, 1.f) );
				vTorque.X(m_PerformanceModifiers.mAccelerationTorque * m_FrameTime * percentageError.X());
			}
			
			if(desiredAngularVelocityZ > 0.f)
			{
				percentageError.Z( 1.f - Maths::Clamped( (currentAngularVelocityZ / desiredAngularVelocityZ), -1.f, 1.f) );
				vTorque.Z(m_PerformanceModifiers.mAccelerationTorque * m_FrameTime * percentageError.Z());
			}

			pBody->GetBody().AddTorque(m_Matrix.AX() * vTorque.X());
			pBody->GetBody().AddTorque(m_Matrix.AY() * vTorque.Y());
			pBody->GetBody().AddTorque(m_Matrix.AZ() * vTorque.Z());

			m_PerformanceModifiers.mRotationTime -= m_FrameTime;
		}
		else
		{
			m_PerformanceModifiers.mRotationTime = 0.f;
		}
	}
	*/
}



/*
===============
NAME: HaltAndRemoveFromRaceLogic

NOTE: Called when vehicle is wrecked/eliminated/leaves a MP race.
===============
*/
void C_VehicleActor::HaltAndRemoveFromRaceLogic()
{
	m_Eliminated = true;	//	Set this so Darren's AI stuff doesn't include these vehicles when calculating positions.
	m_FatallyDamaged = true;	//	Better set this just in case, it's really just a check used with m_Eliminated though. 

	m_ControlsEnabled = false;

	if(!m_pVehiclePhysics) 
		return;

	RacingPhysics::CarControls& controls = ((RacingPhysics::Car*)m_pVehiclePhysics)->Controls();
	if (&controls)
	{
		controls.Clear();
		controls.handbrake = 1.0f;
	}
}

/*
===============
NAME: Shutdown

NOTE: Shutsdown vehicle, but doesn't destroy everything
===============
*/
void C_VehicleActor::Shutdown()
{
#if PLATFORM_PC
	if (m_Blur2PCHookAdded)
	{	// [Blur2PC]
		WorldRender::Blur2PCInterpRemoveRenderHook(&C_VehicleActor::Blur2PCRenderHook, this);
		m_Blur2PCHookAdded = false;
	}
#endif
	//Destroy flushes and sets group to NONE
	if (m_hUpdateSchedulerGroup != Scheduler::JQ_GROUP_NONE)
		Scheduler::DestroyGroup(m_hUpdateSchedulerGroup);

	for (S32 iWheel = 0; iWheel < m_WheelEffects.Size(); iWheel++)
		m_WheelEffects[iWheel].Destroy();
	m_WheelEffects.Clear();

	//Destroy effects
	for (S32 i = 0; i < mPickupSFXArray.Size(); i++)
	{
		if (mPickupSFXArray[i].m_pSFX)
		{
			mPickupSFXArray[i].m_pSFX->Stop();
			SAFE_DELETE_PTR(mPickupSFXArray[i].m_pSFX);
		}	
	}

	if (m_pRoughTerrainSpecialFX)
	{
		m_pRoughTerrainSpecialFX->Stop();
		SAFE_DELETE_PTR(m_pRoughTerrainSpecialFX);
	}
	if (m_pHighSpeedSpecialFX)
	{
		m_pHighSpeedSpecialFX->Stop();
		SAFE_DELETE_PTR(m_pHighSpeedSpecialFX);
	}
	if (m_pLandedSpecialFX)
	{
		m_pLandedSpecialFX->Stop();
		SAFE_DELETE_PTR(m_pLandedSpecialFX);
	}
	if (m_pImpactSpecialFX)
	{
		m_pImpactSpecialFX->Stop();
		SAFE_DELETE_PTR(m_pImpactSpecialFX);
	}
	if (m_pHealthSpecialFX)
	{
		m_pHealthSpecialFX->Stop();
		SAFE_DELETE_PTR(m_pHealthSpecialFX);
	}	

	//Destroy dynamic lights
	for (int i = 0; i < m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Size(); i++)
	{
		Vehicle::C_DynamicLight& dynamicLight = m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector[i];
		if (dynamicLight.m_LightHandle.Object())
			SceneGraph::RemovePointLight(GetSceneGraph(), dynamicLight.m_LightHandle);

	}

	//Destroy flares
	for (int i = 0; i < m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Size(); i++)
	{
		Vehicle::C_Flare& flare = m_pVehicleModel->GetLightFlares()->m_LightFlareVector[i];
		if (flare.m_Handle)
		{
			m_pLightEffectManager->DestroyLight(flare.m_Handle);
		}
	}

	SceneGraph::C_SceneGraph* pSceneGraph = GetSceneGraph();
	if (m_HeadlightHandle.Object())
		SceneGraph::RemoveSpotLight(pSceneGraph, m_HeadlightHandle);
	if (m_HeroLightHandle.Object())
		SceneGraph::RemovePointLight(pSceneGraph, m_HeroLightHandle);
	if (m_RearLightHandle.Object())
		SceneGraph::RemovePointLight(pSceneGraph, m_RearLightHandle);

	const C_VehicleActorCreate *pVehicleCreate = (const C_VehicleActorCreate *)(GetCreationData().GetData());

	if (m_pPickupTriggerActor && pVehicleCreate && pVehicleCreate->m_pGameWorld && pVehicleCreate->m_pGameWorld->GetTriggerLayer(Game::E_TriggerLayer_Pickup))
	{
		m_pTriggerRegionTriggerActor->SetUserData((void*)&m_UserData);
		pVehicleCreate->m_pGameWorld->GetTriggerLayer(Game::E_TriggerLayer_Pickup)->RemoveTriggerActor(m_pPickupTriggerActor);
		SAFE_DELETE_PTR(m_pPickupTriggerActor);
	}
	if (m_pTriggerRegionTriggerActor && pVehicleCreate && pVehicleCreate->m_pGameWorld && pVehicleCreate->m_pGameWorld->GetTriggerLayer(Game::E_TriggerLayer_TriggerRegion))
	{
		m_pTriggerRegionTriggerActor->SetUserData((void*)&m_UserData);
		pVehicleCreate->m_pGameWorld->GetTriggerLayer(Game::E_TriggerLayer_TriggerRegion)->RemoveTriggerActor(m_pTriggerRegionTriggerActor);
		SAFE_DELETE_PTR(m_pTriggerRegionTriggerActor);
	}
}

/*
===============
NAME: RemoveFromRace

NOTE: Shuts off updates and stops rendering vehicle/driver etc.
===============
*/
void C_VehicleActor::RemoveFromRace()
{
	if (m_hUpdateSchedulerGroup != Scheduler::JQ_GROUP_NONE)
		Scheduler::Flush(m_hUpdateSchedulerGroup);

	DisableVehicle();

	if (m_pCollisionBody && m_pCollisionBody->TestBodyFlags(Gravity::C_Body::E_InsertedIntoWorld) &&
							!m_pCollisionBody->TestBodyFlags(Gravity::C_Body::E_TemporarilyRemoved))
		GetPhysicsScene()->TemporarilyRemoveBody(m_pCollisionBody);
	// Disable in the mirror. 
	RemoveModelFromSceneGraph();  // BUG: 29306
	DisableHeadlights();

	Shutdown();
}

Debug::Float g_debugDeghostDuration("Amax.Garage.DeghostDuration", 1.0f);

void C_VehicleActor::EnableVehicle()
{
	// If the vehicle is eliminated or wrecked, resurrect it for cutscenes.
	Game::C_RaceLogicLayer *pLayer	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	if (pLayer)
	{
		C_VehicleLogicActor* pActor		= pLayer->GetLogicVehicleActor(m_VehicleIndex);
		if (pActor)
			pActor->mCurrentHealthLevel		= E_HEALTH_LEVEL_HIGH;
	}

	if (C_Game::GetInstance()->GetLevelState().mInUI)
	{
		Ghost();
		UpdateAlpha(0.f);
		Deghost((float)g_debugDeghostDuration);
	}
	if (m_pCollisionBody && m_pCollisionBody->TestBodyFlags(Gravity::C_Body::E_TemporarilyRemoved))
	{
		GetPhysicsScene()->AddBody(m_pCollisionBody);
	}
	m_Eliminated = false;	
	m_Enabled = true;
	m_EnableParticles = true;
	SetState(true);
	SetCollisions(true);
	SetVisibility(true,-1,!C_Game::GetInstance()->GetLevelState().mInUI,false);
	DisplayDriver(true);
	AddSubscriber( GameMessaging::E_GameMsg_PreUpdate );
	AddSubscriber( GameMessaging::E_GameMsg_Update );
	AddSubscriber( Game::E_VehicleMessage_PostUpdate );
	AddSubscriber( ActorMsg::E_GameActorMsg_Update2ndPass );
	AddSubscriber( Game::E_GameflowMessage_RaceLoaded );
	AddSubscriber( Game::E_GameflowMessage_RaceStarted );
}

void C_VehicleActor::DisableVehicle()
{
//	m_RadiatorSmokeEmitter.Destroy();
	m_EngineSmokeLightEmitter.Destroy();
	m_EngineSmokeHeavyEmitter.Destroy();
	//m_EngineFireEmitter.Destroy();

	SetState(false);
	SetCollisions(false);
	SetVisibility(false,-1,true,false);
	DisplayDriver(false);
	DisableHeadlights();
	m_Enabled = false;
	if (m_pCollisionBody && m_pCollisionBody->TestBodyFlags(Gravity::C_Body::E_InsertedIntoWorld) &&
	   !m_pCollisionBody->TestBodyFlags(Gravity::C_Body::E_TemporarilyRemoved))
	{
		GetPhysicsScene()->TemporarilyRemoveBody(m_pCollisionBody);
	}

	m_EnableParticles = false;
	for (S32 iWheel = 0; iWheel < m_WheelEffects.Size(); iWheel++)
	{
		float fPScale = m_WheelParticleState[iWheel].second;
		bool enableParticles = m_EnableParticles && m_WheelParticleState[iWheel].first;
		m_WheelEffects[iWheel].Update(&m_Matrix, &m_pVehicleModel->GetModelInput()->m_Wheels[iWheel].m_mYawMatrix, m_FrameTime, m_pVehicleModel->GetLighting(), NULL, enableParticles, m_ParticleEmitScale * fPScale, m_ParticleLifeScale * fPScale, -1, m_pGameWorld->GetSplashCamera(), m_EffectsVisiblityFlags, m_pVehicleModel->GetModelInput()->m_vVelocity);
	}
	RemoveSubscriber( GameMessaging::E_GameMsg_PreUpdate );
	RemoveSubscriber( GameMessaging::E_GameMsg_Update );
	AddSubscriber( Game::E_VehicleMessage_PostUpdate );
	RemoveSubscriber( ActorMsg::E_GameActorMsg_Update2ndPass );
	RemoveSubscriber( Game::E_GameflowMessage_RaceLoaded );
	RemoveSubscriber( Game::E_GameflowMessage_RaceStarted );
}

void C_VehicleActor::RemoveModelFromSceneGraph()
{
	for (int iModel = 0; iModel < m_ModelInstances.Size(); iModel++)
	{
		SceneGraph::Remove(GetSceneGraph(), m_ModelInstances[iModel]);
	}
}

void C_VehicleActor::DisableHeadlights()
{
	//Hide aux flares
	for (int i = 0; i < m_pVehicleModel->GetAuxFlares()->m_AuxFlareVector.Size(); i++)
	{
		Vehicle::C_AuxFlare& flare = m_pVehicleModel->GetAuxFlares()->m_AuxFlareVector[i];
		flare.m_pAuxFlareInstance->SetViewFlags(0); 
	}
}

//So the idea here is that we can override the alpha returned by this function so that 
//the particles are rendered even when the vehicle's not (SilentRunning)
const float C_VehicleActor::GetAlpha(	const enum E_VAC_TYPE eVehicleAlphaComponentType, 
										const enum E_VAC_TARGET_TYPE eVehicleAlphaComponentTargetType) const
{
	const bool validComponent = (eVehicleAlphaComponentType != E_VAC_TYPE_NONE && eVehicleAlphaComponentType != E_VAC_TYPE_MAX);
	BIZ_ASSERT_MSG( validComponent, "C_VehicleActor::GetAlpha() > Invalid component type");

	const bool validTarget = (eVehicleAlphaComponentTargetType != E_VAC_TARGET_TYPE_NONE && eVehicleAlphaComponentTargetType != E_VAC_TARGET_TYPE_MAX);
	BIZ_ASSERT_MSG( validTarget, "C_VehicleActor::GetAlpha() > Invalid target type");

	return m_ComponentAlpha[eVehicleAlphaComponentTargetType][eVehicleAlphaComponentType];
}

void C_VehicleActor::SetAlpha(	const enum E_VAC_TYPE eVehicleAlphaComponentType, 
								const float alpha,
								const enum E_VAC_TARGET_TYPE eVehicleAlphaComponentTargetType)
{
	if(eVehicleAlphaComponentType == E_VAC_TYPE_ALL)
	{
		//If we're setting all then go off overwrite them all here
		for(int i = 0; i < E_VAC_TYPE_MAX; i++)
		{
			if(eVehicleAlphaComponentTargetType == E_VAC_TARGET_TYPE_ALL)
			{
				for(int j = 0; j < E_VAC_TARGET_TYPE_MAX; j++)
				{
					m_ComponentAlpha[j][i] = alpha;
				}
			}
			else 
			{
				m_ComponentAlpha[eVehicleAlphaComponentTargetType][i] = alpha;
			}
		}
	}
	else
	{
		//Just do the specific one
		if(eVehicleAlphaComponentTargetType == E_VAC_TARGET_TYPE_ALL)
		{
			for(int j = 0; j < E_VAC_TARGET_TYPE_MAX; j++)
			{
				m_ComponentAlpha[j][eVehicleAlphaComponentType] = alpha;
			}
		}
		else 
		{
			m_ComponentAlpha[eVehicleAlphaComponentTargetType][eVehicleAlphaComponentType] = alpha;
		}
	}
}

const bool C_VehicleActor::Ghosted() const
{
	const float currentAlpha = GetAlpha(E_VAC_TYPE_VEHICLE);

	//We can say that we're ghosted if we're not at full alpha
	return (currentAlpha < 1.f);
}

/*
===============
NAME: Ghosting

NOTE: Is in its own function so we can later 
      add effects (such as translucency)
===============
*/

void C_VehicleActor::Ghost(const float duration, const float targetAlpha)
{
	m_GhostTransitionDuration = duration;

	//Set the alpha of all the components
	SetAlpha(E_VAC_TYPE_ALL, targetAlpha, E_VAC_TARGET_TYPE_DESIRED);

	//See if we're going to be doing this transition over time or instantly
	if (m_GhostTransitionDuration > 0.0f)
	{
		//We're going to do this over time
		m_GhostTransitionTime = m_GhostTransitionDuration;

		//Set the initial alpha value to the alpha of the car
		const float initialAlpha = GetAlpha(E_VAC_TYPE_VEHICLE);
		SetAlpha(E_VAC_TYPE_ALL, initialAlpha, E_VAC_TARGET_TYPE_INITIAL);
	}
	else
	{
		//Instantly
		UpdateAlpha(targetAlpha);
	}
}

void C_VehicleActor::Deghost(const float transitionDuration, const float targetAlpha)
{
	if (GetAlpha(E_VAC_TYPE_VEHICLE) == targetAlpha)
		return;

	m_GhostTransitionDuration = transitionDuration;
	SetAlpha(E_VAC_TYPE_VEHICLE, targetAlpha, E_VAC_TARGET_TYPE_DESIRED);
	if (m_GhostTransitionDuration > 0.0f)
	{
		//Set the initial alpha value to the alpha of the car
		const float initialAlpha = GetAlpha(E_VAC_TYPE_VEHICLE);
		SetAlpha(E_VAC_TYPE_ALL, initialAlpha, E_VAC_TARGET_TYPE_INITIAL);

		UpdateAlpha(initialAlpha);
		m_GhostTransitionTime = m_GhostTransitionDuration;
		return;
	}

	// Do not deghost eliminated cars. They cannot be resurrected!
	if(!m_Eliminated)
	{
		const float vehicleTargetAlpha = GetAlpha(E_VAC_TYPE_VEHICLE, E_VAC_TARGET_TYPE_DESIRED);
		UpdateAlpha( vehicleTargetAlpha );
	}
}

void C_VehicleActor::UpdateAlpha(const float alpha)
{
	//See if we're not needing alpha (fully invisible or fully visible)
	if( (alpha <= 0.f && g_MoveFullyGhostedCarOutOfAlphaPass) || alpha >= 1.f)
	{
		//Take the vehicle out of the ghost render pass
		ChangeModelRenderPass( E_RenderPass_VehicleSolid );
	}
	else 
	{
		ChangeModelRenderPass( E_RenderPass_Ghost );
	}

	if(alpha <= 0.f && !m_pVehicleModel->m_Hidden && g_HideFullyGhostedCar)
	{
		SetVisibility(false);
		m_VisibilityOverride = true;
	}
	else if(alpha > 0.f && m_pVehicleModel->m_Hidden)
	{
		SetVisibility(true);
		CheckCamera(true);
		m_VisibilityOverride = false;
	}

	//We only want to show the driver if we're at full alpha
	if(alpha < 1.f)
	{
		DisplayDriver(false);
	}
	else if(!m_IsBumperCamera)
	{
		DisplayDriver(true);	
	}
	else
	{
		//Display in all views except focus
		for (U32 i = 0; i < 4; i++)
		{
			if (i != m_ViewportID)
				DisplayDriver(true, i);
		}
	}

	//Now update the component alphas
	for(int i = 0; i < E_VAC_TYPE_MAX; i++)
	{
		UpdateComponentAlpha(alpha, (const enum E_VAC_TYPE)i);
	}

	//Finally we can update the actual vehicle's alpha
	m_pVehicleModel->SetAlpha( m_ComponentAlpha[E_VAC_TARGET_TYPE_CURRENT][E_VAC_TYPE_VEHICLE] );
}

void C_VehicleActor::UpdateComponentAlpha(const float alpha, const enum E_VAC_TYPE eVehicleAlphaComponentType)
{
	//Work out if we're ghosting or deghosting
	bool componentDeGhosting = m_ComponentAlpha[E_VAC_TARGET_TYPE_DESIRED][eVehicleAlphaComponentType] > m_ComponentAlpha[E_VAC_TARGET_TYPE_INITIAL][eVehicleAlphaComponentType];
	if(componentDeGhosting)
	{
		//Alpha increasing
		if(m_ComponentAlpha[E_VAC_TARGET_TYPE_DESIRED][eVehicleAlphaComponentType] < alpha)
		{
			//Don't let the alpha go above the target
			m_ComponentAlpha[E_VAC_TARGET_TYPE_CURRENT][eVehicleAlphaComponentType] = m_ComponentAlpha[E_VAC_TARGET_TYPE_DESIRED][eVehicleAlphaComponentType];
		}
		else
		{
			m_ComponentAlpha[E_VAC_TARGET_TYPE_CURRENT][eVehicleAlphaComponentType] = alpha;
		}
	}
	else
	{
		//Alpha decreasing
		if(m_ComponentAlpha[E_VAC_TARGET_TYPE_DESIRED][eVehicleAlphaComponentType] > alpha)
		{
			//Don't let the alpha drop below the target
			m_ComponentAlpha[E_VAC_TARGET_TYPE_CURRENT][eVehicleAlphaComponentType] = m_ComponentAlpha[E_VAC_TARGET_TYPE_DESIRED][eVehicleAlphaComponentType];
		}
		else
		{
			m_ComponentAlpha[E_VAC_TARGET_TYPE_CURRENT][eVehicleAlphaComponentType] = alpha;
		}
	}

}


void C_VehicleActor::ChangeModelRenderPass(const enum Game::E_RenderPass eRenderPass)
{
	//Go through the wheels and change the blur models 
	for (S32 iBlur = 0; iBlur < m_pVehicleModel->m_WheelBlur.Size(); iBlur++)
	{
		m_pVehicleModel->m_WheelBlur[iBlur].m_SolidPass = eRenderPass;
		m_pVehicleModel->m_WheelBlur[iBlur].m_bUpdateMe	= true;

		m_pVehicleModel->m_WheelBlur[iBlur].Update();
	}

	//Now change the exterior and wheel models themselves
	for(U32 i = 0; i < m_pVehicleModel->GetNumModelInstances(); ++i)
	{
		WorldRender::C_ModelInstance* pModelInstance = m_pVehicleModel->GetModelInstance(i);

		if (m_pVehicleModel->GetModelInstanceType(i) == Vehicle::E_ModelInstanceType_Exterior)
		{
			pModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OPAQUE, eRenderPass);
			pModelInstance->Build();
		}
	}

}


Debug::Float g_debugDeghostInitialDist2Check("Vehicle.Components.HandOfGod.DeghostInitialDistCheckSquared", 64, Debug::Range(0, 100), Debug::Step(0.001f));

void C_VehicleActor::CheckForPotentialCollisions(Btl::Vector<int>& vehiclesUnderneath)
{
	C_RaceLogicLayer* pRaceLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	Collision::C_CRAxisBox vehTransformBox	= GetCollisionBody()->GetObject()->m_TransformedBox;
	//const Maths::HF32 deghostDist((F32)g_debugDeghostInitialDist2Check);

	for( int i = 0; i < pRaceLogicLayer->GetNumVehicles(); i++)
	{
		if( i == m_VehicleIndex )
			continue;

		if (!pRaceLogicLayer->GetLogicVehicleActor(i))
			continue;

		C_VehicleActor* pOtherVeh			= pRaceLogicLayer->GetLogicVehicleActor(i)->GetGraphicalActor(i);
		Collision::C_CRAxisBox otherVehTransformBox	= pOtherVeh->GetCollisionBody()->GetObject()->m_TransformedBox;
		
		if( vehTransformBox.Intersects(otherVehTransformBox) )
			vehiclesUnderneath.PushBack(pOtherVeh->GetVehicleIndex());
	}
}

/*
===============
NAME: SetMatrices

NOTE: Sets vehicle matrix and updates parts
===============
*/

void C_VehicleActor::SetMatrices(const Maths::HMat43 newWorldMatrix)
{
	if (!m_pVehicleModel)
		return;

	m_Matrix = newWorldMatrix;
	m_pVehicleModel->GetModelInput()->m_mMatrix = m_Matrix;

	//DeferredUpdate();
	
	for (U32 iInstance = 0; iInstance < m_pVehicleModel->GetNumModelInstances(); iInstance++)
	{				
		WorldRender::C_ModelInstance* pModelInstance = m_pVehicleModel->GetModelInstance(iInstance);
		pModelInstance->SetWorldSpherePosition(m_Matrix.Translation());
		pModelInstance->SetElementMatrix(-1, m_Matrix);
	}

	m_pVehicleModel->GetComponentList().UpdateMatrices(m_Matrix, m_pVisDetect, false, NULL, true);		
	
	m_pVehicleModel->SetDynamicShaderParameters();

	SynchData(&m_Matrix, Maths::HVec3(0.0f, 0.0f, 0.0f));
}


Debug::Bool g_debugSetMatricesOnReset("Vehicle.HoGResetMatrices", true);

Debug::Float g_debugExtraWheelRadius("Vehicle.WheelRadiusPaddingForReset", 0.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

/*
===============
NAME: Reset

NOTE: Resets vehicle actor
===============
*/

void C_VehicleActor::Reset(const enum E_ResetType eResetType, const Maths::HMat43 newWorldMatrix, Maths::HVec3 velocity, bool networkJump)
{
	if (!m_pCollisionBody || !m_pVehiclePhysics || !m_pVehicleModel)
		return;

	Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
	Maths::HVec3 newVelocity(zero);
	m_pCollisionBody->AngularVelocity(newVelocity);

	switch(eResetType)
	{
	case E_ResetType_Spawn:
		{
			
				//SetMatrices(m_BirthPlace);
			RacingPhysics::Axis axis((Maths::HMat44)m_BirthPlace);
			//RacingPhysics::Axis axis((Maths::HMat44)m_LastKnownSafeMatrix);
			axis.SetPosition(RacingPhysics::Point3R(axis.Position().X(), axis.Position().Y(), axis.Position().Z()));

			m_pVehiclePhysics->SetAxis(axis, ((RacingPhysics::Car*)m_pVehiclePhysics)->Controls().steer);
			m_pVehiclePhysics->Stop();
			m_landing.Reset();
			m_tumbling.Reset();
			m_fHighSpeedPercentage = 0.0f;
			m_fRoughTerrainPercentage = 0.0f;
			m_fSmallImpactAccumulator = 0.0f;
			m_fSmallImpactTimeSinceImpact = 0.0f;
			m_fSmallImpactEffectValue = 0.0f;

			Maths::C_Quaternion quat((Maths::HMat33)m_BirthPlace);
			//Maths::C_Quaternion quat((Maths::HMat33)m_LastKnownSafeMatrix);
			m_pCollisionBody->GetBody().StoreVelocitiesAndPosition(	zero, 
																	zero,
																	quat, 
																	Maths::HVec3(axis.Position().X(), axis.Position().Y(), axis.Position().Z())); 
			m_pCollisionBody->ZeroExternalForces();
			m_pCollisionBody->ClearExternalForces();
			m_pCollisionBody->SynchroniseState();
			m_pCollisionBody->SynchronisePosRot(); 

			//m_PrevPosition = ((Collision::C_CompositeShape*)m_pCollisionBody->GetShape())->GetShape(this->m_MainElementIndex)->m_PosRot.Translation();
			m_PrevPosition = m_pCollisionBody->GetShape()->m_PosRot.Translation();



			if (g_debugSetMatricesOnReset)
				SetMatrices(m_BirthPlace);
				//SetMatrices(m_LastKnownSafeMatrix);

			m_pVehicleModel->GetComponentList().ZeroPartsLinearVelocity();
			m_pVehicleModel->GetComponentList().ZeroPartsAngularVelocity();

			AIAmax::AIStopMsg	aiStopMessage;
			aiStopMessage.m_pVehicleActor = this;
			C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&aiStopMessage);
			if(m_pConstraint)
			{
				m_pConstraint->SetScale(0.f);
			}

			break;
		}
	case E_ResetType_Position:
		{
			float fWheelRadius = 0.0f;
			if (!networkJump && m_pVehiclePhysics->GetNumWheels() == 4)
				fWheelRadius = Maths::Max(m_pVehiclePhysics->Wheel(0).Radius(), m_pVehiclePhysics->Wheel(2).Radius()) + g_debugExtraWheelRadius;

			Maths::HMat43 resetMatrix = newWorldMatrix;
			Maths::HVec3 newPos = resetMatrix.AW() + resetMatrix.AY() * fWheelRadius * 2.0f;
			resetMatrix.AW(newPos);

			m_LastKnownResetMatrix = resetMatrix;
			m_landing.Reset();
			m_tumbling.Reset();
			m_fHighSpeedPercentage = 0.0f;
			m_fRoughTerrainPercentage = 0.0f;
			m_fSmallImpactAccumulator = 0.0f;
			m_fSmallImpactTimeSinceImpact = 0.0f;
			m_fSmallImpactEffectValue = 0.0f;

			RacingPhysics::Axis axis((Maths::HMat44)resetMatrix);
			axis.SetPosition(RacingPhysics::Point3R(axis.Position().X(), axis.Position().Y(), axis.Position().Z()));
			
			m_pVehiclePhysics->SetAxis(axis, ((RacingPhysics::Car*)m_pVehiclePhysics)->Controls().steer);
			Maths::C_Quaternion quat(axis.Matrix3());
			m_pVehiclePhysics->Stop();
			
			m_pCollisionBody->GetBody().StoreVelocitiesAndPosition(	velocity, 
																	zero,
																	quat, 
																	Maths::HVec3(axis.Position().X(), axis.Position().Y(), axis.Position().Z())); 
			m_pCollisionBody->ZeroExternalForces();
			m_pCollisionBody->ClearExternalForces();
			m_pCollisionBody->SynchroniseState();
			m_pCollisionBody->SynchronisePosRot(); 

			m_PrevPosition = m_pCollisionBody->GetShape()->m_PosRot.Translation();


			if (g_debugSetMatricesOnReset)
				SetMatrices(resetMatrix);

	
			m_pVehicleModel->GetComponentList().SetPartsLinearVelocity(velocity);
			m_pVehicleModel->GetComponentList().ZeroPartsAngularVelocity();


			AIAmax::AIStopMsg	aiStopMessage;
			aiStopMessage.m_pVehicleActor = this;
			C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&aiStopMessage);
			if(m_pConstraint)
			{
				m_pConstraint->SetScale(0.f);
			}


			// As this message is deferred, we cannot set the ai until the physics has the correct postion, etc, etc
			// so reset the ai here.....
			AIAmax::AmaxAIR::Instance().ResetAfterHOG(m_VehicleIndex);

			break;
		}
	case E_ResetType_Camera:
		{
			const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
			Maths::HMat43 cameraMatrix = view.DebugFeed().Mat43();
			Maths::HMat44 mat(cameraMatrix);
			RacingPhysics::Axis axis(mat);
			m_pVehiclePhysics->SetAxis(axis, ((RacingPhysics::Car*)m_pVehiclePhysics)->Controls().steer);
			Maths::C_Quaternion quat(axis.Matrix3());
			m_pCollisionBody->GetBody().StoreVelocitiesAndPosition(	zero, 
																	zero,
																	quat, 
																	Maths::HVec3(axis.Position().X(), axis.Position().Y(), axis.Position().Z())); 
			m_pCollisionBody->ZeroExternalForces();
			m_pCollisionBody->ClearExternalForces();
			m_pCollisionBody->SynchroniseState(); 
			m_pCollisionBody->SynchronisePosRot(); 

			m_PrevPosition = m_pCollisionBody->GetShape()->m_PosRot.Translation();

			if (g_debugSetMatricesOnReset)
				SetMatrices(cameraMatrix);

			m_pVehicleModel->GetComponentList().ZeroPartsLinearVelocity();
			m_pVehicleModel->GetComponentList().ZeroPartsAngularVelocity();
			if(m_pConstraint)
			{
				m_pConstraint->SetScale(0.f);
			}
			break;
		}
	};

	//Reset lighttrails
	for (S32 i = 0; i < m_LightTrails.Size(); i++)
	{
		m_LightTrails[i].m_pLightTrailInstance->Reset(m_LightTrails[i].m_Position * newWorldMatrix);
	}

	//Reset particles
	for (U32 i = 0; i < 4; i++)
	{
		m_WheelEffects[i].ClearDeferred();
	}

	m_pVehicleModel->Reset();

	m_ResetifyDriver = true;
}

void C_VehicleActor::ResetVehicleAlphaComponents()
{
	SetAlpha(E_VAC_TYPE_ALL, 1.f, E_VAC_TARGET_TYPE_ALL);
}

Debug::Bool g_debugOverrideRepairAmount("Vehicle.Damage.Repair.OverrideAmount", false);
Debug::Float g_debugRepairAmount("Vehicle.Damage.Repair.Amount", 1.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugRepairTransition("Vehicle.Damage.Repair.TransitionPeriod", 2.0f, Debug::Range(0.0f, 10.0f), Debug::Step(0.001f));
Debug::Bool g_debugLetFatalDamageKnockWheelsOff("Vehicle.Damage.Parts.FatalDamageKnockWheelsOff", true);

Debug::Bool g_debugDisableVisualDamage("Game.DisableVehicleVisualDamage", false);

/*
===============
NAME: HandleCollision

NOTE: Handle vehicle collisions
===============
*/
void C_VehicleActor::HandleCollision(const Game::C_ImpactMsg* p_pMsg)
{
	m_landing.HandleImpact(*this);

	if (g_bDisableSPUDamage) return;
	
	if (g_debugDisableVisualDamage || !m_pVehicleModel || !m_pVehicleModel->GetDamageLayer() || !p_pMsg || m_Eliminated)
		return;

	if (p_pMsg->mCollisionInfo.m_GameplayImpulse <= 0.0f)
		return;

	C_RaceLogicLayer* pRaceLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	if(pRaceLogicLayer->GetLogicVehicleActor(m_VehicleIndex)->mShieldTime > 0.f)
		return;

	//LOGDEBUG(g_VehicleActorLog, "C_VehicleActor::HandleCollision()");
	
	//////////////////if (m_RadiatorSmokeEmitter.m_Intensity == 0.0f && p_pMsg->mDamageFilter.m_RadiatorSmoke)
	//////////////////	m_RadiatorSmokeEmitter.m_Activate = true;
	//////////////////else if (!p_pMsg->mDamageFilter.m_RadiatorSmoke && m_RadiatorSmokeEmitter.m_Intensity == 1.0f)
	//////////////////	m_RadiatorSmokeEmitter.m_Deactivate = true;

	//////////////////if (m_EngineSmokeLightEmitter.m_Intensity == 0.0f && p_pMsg->mDamageFilter.m_EngineSmokeLight)
	//////////////////	m_EngineSmokeLightEmitter.m_Activate = true;
	//////////////////else if (!p_pMsg->mDamageFilter.m_EngineSmokeLight && m_EngineSmokeLightEmitter.m_Intensity == 1.0f)
	//////////////////	m_EngineSmokeLightEmitter.m_Deactivate = true;
	//////////////////
	//////////////////if (m_EngineSmokeHeavyEmitter.m_Intensity == 0.0f && p_pMsg->mDamageFilter.m_EngineSmokeHeavy)
	//////////////////	m_EngineSmokeHeavyEmitter.m_Activate = true;
	//////////////////else if (!p_pMsg->mDamageFilter.m_EngineSmokeHeavy && m_EngineSmokeHeavyEmitter.m_Intensity == 1.0f)
	//////////////////	m_EngineSmokeHeavyEmitter.m_Deactivate = true;
	
	//if (m_EngineFireEmitter.m_Intensity == 0.0f && p_pMsg->mDamageFilter.m_EngineFire)
	//	m_EngineFireEmitter.m_Activate = true;
	//else if (!p_pMsg->mDamageFilter.m_EngineFire && m_EngineFireEmitter.m_Intensity == 1.0f)
	//	m_EngineFireEmitter.m_Deactivate = true;

	Vehicle::C_ImpactEvent* pImpactEvent = m_pVehicleModel->GetDamageLayer()->ProcessCollision(p_pMsg->mMatrix, p_pMsg->mCollisionInfo, p_pMsg->mDamageFilter);

	if (pImpactEvent)
	{
		m_CollisionThisFrame = true;

		if(m_EnableParticles)
		{
			SpawnImpactEffects(	pImpactEvent->m_CollisionInfo.m_EffectMaterial, pImpactEvent->m_GlobalPosition,
								pImpactEvent->m_GlobalNormal, Maths::HVec3(0.0f, 0.0f, 0.0f), 
								p_pMsg->mCollisionInfo.m_EffectImpulse);//pImpactEvent->m_ImpactSize);
		}
	}

	const Actor::C_Actor* actorA = GetActorList()->GetActorPtr(p_pMsg->mActorHandleA);
	const Actor::C_Actor* actorB = GetActorList()->GetActorPtr(p_pMsg->mActorHandleB);

	Actors::C_VehicleLogicActor* pLogicActor = pRaceLogicLayer->GetLogicVehicleActor(m_VehicleIndex);

	if(pLogicActor && actorA && actorA->GetActorTypeId() == E_ActorTypeId_DynamicObject)
	{
		const ::Actors::DynamicObjectActor* dynActor = (::Actors::DynamicObjectActor*)actorA;
		pLogicActor->mStatistics.mCriticalObjectsHit.Insert(dynActor->GetCreationData()->m_RaceLogicGroup);
	}
	
	if (pLogicActor)
	{
		if(actorB && actorB->GetActorTypeId() == E_ActorTypeId_DynamicObject)
		{
			const ::Actors::DynamicObjectActor* dynActor = (::Actors::DynamicObjectActor*)actorB;
			pLogicActor->mStatistics.mCriticalObjectsHit.Insert(dynActor->GetCreationData()->m_RaceLogicGroup);
		}

		if(actorA && actorA->GetActorTypeId() == E_ActorTypeId_PhysicalObject)
		{
			const ::Actors::C_PhysicalObject* physActor = (::Actors::C_PhysicalObject*)actorA;
			pLogicActor->mStatistics.mCriticalObjectsHit.Insert(physActor->GetCreationData()->m_RaceLogicGroup);
		}

		if(actorB && actorB->GetActorTypeId() == E_ActorTypeId_PhysicalObject)
		{
			const ::Actors::C_PhysicalObject* physActor = (::Actors::C_PhysicalObject*)actorB;
			pLogicActor->mStatistics.mCriticalObjectsHit.Insert(physActor->GetCreationData()->m_RaceLogicGroup);
		} 
	}

	const float gameplayImpulse	 = p_pMsg->mCollisionInfo.m_GameplayImpulse;

	if(gameplayImpulse >= g_SmallImpactImpulseMin &&
	   gameplayImpulse <= g_SmallImpactImpulseMax)
	{
		// Only collect impacts with static geometry
		if( !actorA || !actorB )
		{
			m_fSmallImpactAccumulator += g_SmallImpactAccumulatorStep;
			m_fSmallImpactTimeSinceImpact = -g_SmallImpactDecayDelay;
			if( m_fSmallImpactAccumulator > g_SmallImpactAccumulatorMax )
			{
				m_fSmallImpactAccumulator = g_SmallImpactAccumulatorMax;
				
			}
		}
	}
}

//Debug::Float g_debugVehicleTunnelingThreshold("Vehicle.GamePhysicsMechanics.TunnelingThreshold", 200.0f);

void C_VehicleActor::HandleTunneling()
{
	PERF_EVENT("C_VehicleActor::HandleTunneling");
	//Simple algorithm really - 
	//(1)	Fire a ray from the old position to the new position of the centre of the vehicle
	//(2)	If said ray goes through a wall, then clamp position to impact point + triangle normal * some 
	//		arbitrary positive number (probably smallest axis in AABB)

	//Need to update this here, otherwise weird stuff will happen if we ever do get through this 
	//early rejection test.
	const Maths::HVec3 prevPosition = m_PrevPosition;
	m_PrevPosition = m_pCollisionBody->GetShape()->m_PosRot.Translation();
	
	if (m_pVehicleModel->GetModelInput()->m_iControlType != E_VEHICLE_CONTROL_TYPE_PLAYER)
		return;

	/*if (m_pVehicleModel->GetModelInput()->m_iControlType == E_VEHICLE_CONTROL_TYPE_PLAYER)
	{
		if (m_pVehicleModel->GetModelInput()->m_fSpeed < (float)g_debugVehicleTunnelingThreshold)
			return;
	}
	else if (m_pVehicleModel->GetModelInput()->m_iControlType == E_VEHICLE_CONTROL_TYPE_AI)
	{
		if (m_pVehicleModel->GetModelInput()->m_fSpeed < (float)g_debugVehicleTunnelingThresholdAI)
			return;
		else if (!GetFakeSpeed()->IsActive() && !GetFakePhysics()->IsActive())
			return;
	}
	else
		return;*/

	if(m_MainElementIndex >= 0 && m_pPhysicsChecks != NULL)
	{
		
		Collision::C_CompositeShape* pShape = (Collision::C_CompositeShape*)m_pCollisionBody->GetShape();
		Collision::C_Shape* pSubShape = pShape;

		Gravity::C_Scene* pScene = m_pPhysicsChecks->GetScene();
		if(pScene != NULL)
		{
			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			void* data[1024];
			SharedPhysics::C_SafeStack<true> stack(data, 1024);
			Collision::C_CollisionFilter rayFilter(0xffffffff, Game::PolyFlags::kVehicleSolid, 0, 0, 0,
				0xffffffff, Game::PolyFlags::kbWorld);

			Collision::C_Ray ray;
			ray.InitialiseRay(prevPosition, pSubShape->m_PosRot.Translation(), &rayFilter);
			pScene->DoRayTraceMesh(ray, stack, false);
			if(ray.GetResult().Hit())
			{
				const Maths::HVec3 contactPoint(ray.GetResult().HitPoint());
				const Maths::HF32 projection(Maths::Min(pShape->m_LocalBoundingBox.extents.X(), Maths::Min(pShape->m_LocalBoundingBox.extents.Y(),
					pShape->m_LocalBoundingBox.extents.Z())) * 0.75f);
				//Now set this as the position...
				const Maths::HVec3 oldPosition(m_pCollisionBody->ElementMatrix().Translation());
				const Maths::HVec3 offset(oldPosition - pSubShape->m_PosRot.Translation());
				const Maths::HVec3 position((contactPoint + offset) + ray.GetResult().Normal() * projection);
				//Set this as the position in the body/render state and voila - done!

				
				SharedPhysics::C_RigidBody& rigid = m_pCollisionBody->GetBody();

				//Problem - if we were through the wall, we've already deferred the collision detection
				//and will have not received the contact point. We need to add in an impulse to bounce us
				//off the wall...

				rigid.ApplyImpulseWorldSpace(ray.m_result.Normal() * -Maths::Min((rigid.m_LinearMomentum | ray.m_result.Normal()), zero), rigid.CenterOfRotation()); 

				rigid.StoreVelocitiesAndPosition(	rigid.m_LinearVelocity, rigid.m_AngularVelocity,
					rigid.m_Orientation, position); 

				m_pCollisionBody->SynchroniseState();
				m_pCollisionBody->SynchronisePosRot(); 
			}
		}		
	}
}


static float g_MsToMph = 3600.f / 1609.f;
static float g_MphToMs = 1609.f / 3600.f;

/*
===============
NAME: SynchData

NOTE: Syncs racing physics data with model data - done in a thread-safe happy place
===============
*/
#if PLATFORM_PC
void C_VehicleActor::Blur2PCRenderHook(void* ctx, bool begin)	// [Blur2PC]
{
	C_VehicleActor* pThis = (C_VehicleActor*)ctx;
	if (begin)
	{
		if (WorldRender::Blur2PCInterpRenderAlpha() >= 1.0f)
			return;
		pThis->Blur2PCPlaceLights(pThis->Blur2PCRenderMatrix(), true);
	}
	else if (pThis->m_Blur2PCLightsMoved)
		pThis->Blur2PCPlaceLights(pThis->m_Matrix, false);
}

// [Blur2PC] Re-places what UpdateLightFlares, UpdateAuxFlares and UpdateDynamicLights positioned from
// m_Matrix, using 'matrix'. begin: save the update values and move to 'matrix'; !begin: restore.
void C_VehicleActor::Blur2PCPlaceLights(const Maths::HMat43& matrix, bool begin)
{
	if (!m_pVehicleModel)
		return;
	const Maths::HMat43 delta = m_Matrix.InverseRotTrans() * matrix;

	Btl::Vector<Vehicle::C_Flare>& flares = m_pVehicleModel->GetLightFlares()->m_LightFlareVector;
	if (begin)
		m_Blur2PCSavedFlarePos.Resize(flares.Size());
	for (int i = 0; i < flares.Size() && i < m_Blur2PCSavedFlarePos.Size(); i++)
	{
		DynamicEffect::C_Node& node = flares[i].m_Node;
		if (begin)
		{
			m_Blur2PCSavedFlarePos[i] = node.GetPosition();
			const MathsStorage::Float4A16& p = m_Blur2PCSavedFlarePos[i];
			node.SetPosition(Maths::HVec3(p.x, p.y, p.z) * delta);
		}
		else
			node.SetPosition(m_Blur2PCSavedFlarePos[i]);
	}

	for (int i = 0; i < m_pVehicleModel->GetAuxFlares()->m_AuxFlareVector.Size(); i++)
	{
		Vehicle::C_AuxFlare& auxFlare = m_pVehicleModel->GetAuxFlares()->m_AuxFlareVector[i];
		if (!auxFlare.m_pAuxFlareInstance)
			continue;
		const Maths::HMat43 flareMatrix = auxFlare.m_Matrix * matrix;	// as UpdateAuxFlares
		auxFlare.m_pAuxFlareInstance->UpdatePosition(flareMatrix.Translation());
		auxFlare.m_pAuxFlareInstance->UpdateDirection(flareMatrix.AZ());
	}

	if (SceneGraph::C_PointLight* pLight = m_RearLightHandle.Object())
	{	// as UpdateDynamicLights
		const Maths::HVec3 position(Maths::HVec3(0.f, F32(g_DefaultLightOffsetY), -m_pVehicleModel->GetDamageLayer()->GetVehicleInfo()->m_Extents.Z() + g_DefaultLightOffsetZ) * matrix);
		pLight->SetDimensions(position, g_DefaultLightRadius);
	}

	m_Blur2PCLightsMoved = begin;
}

Maths::HMat43 C_VehicleActor::Blur2PCRenderMatrix() const	// [Blur2PC]
{
	const F32 alpha = WorldRender::Blur2PCInterpRenderAlpha();
	if (alpha >= 1.0f || m_Blur2PCMatrix.tick == 0 || m_Blur2PCMatrix.tick != WorldRender::Blur2PCInterpCurrentTick()
		|| (F32)(m_Blur2PCMatrix.cur.AW() - m_Blur2PCMatrix.prev.AW()).Length() >= 20.0f)	// teleport: no blend
		return m_Matrix;
	return WorldRender::Blur2PCBlendMatrix(m_Blur2PCMatrix.prev, m_Blur2PCMatrix.cur, alpha);
}
#endif

void C_VehicleActor::SynchData(Maths::HMat43* p_pMatrix, const Maths::HVec3& photoOffset)
{	
	if (!m_pVehicleModel->GetModelInput()->m_bShouldUpdate)// || !m_pVehicleModel->GetModelInput()->m_bShouldRender)
		return;

	PERF_EVENT("C_VehicleActor.SynchData");
	{
	const RacingPhysics::Vector3R& tempVelocity = m_pVehiclePhysics->GetVelocity();
	m_pVehicleModel->GetModelInput()->m_vVelocity				= RPToMaths(&tempVelocity);
	m_pVehicleModel->GetModelInput()->m_vAcceleration			= RPToMaths(&m_pVehiclePhysics->Acceleration());
	const RacingPhysics::Vector3R& tempAngluarVelocity = m_pVehiclePhysics->GetAngularVelocity();
	m_pVehicleModel->GetModelInput()->m_vAngularVelocity		= RPToMaths(&tempAngluarVelocity);
	//m_pVehicleModel->GetModelInput()->m_vAngularAcceleration	= RPToMaths(&m_pVehiclePhysics->AngularAcceleration());

	m_pVehicleModel->GetModelInput()->m_iGear					= m_pVehiclePhysics->GetGear();

	m_pVehicleModel->GetModelInput()->m_fRevs					= m_pVehiclePhysics->Engine().RPM();
	m_pVehicleModel->GetModelInput()->m_fRedLine				= m_pVehiclePhysics->Engine().LimiterRPM();
	m_pVehicleModel->GetModelInput()->m_fThrottle				= m_pVehiclePhysics->Throttle();
	m_pVehicleModel->GetModelInput()->m_fBrake					= m_pVehiclePhysics->Brake();	
	m_pVehicleModel->GetModelInput()->m_fMagVelocity			= Magnitude(m_pVehicleModel->GetModelInput()->m_vVelocity);
	m_pVehicleModel->GetModelInput()->m_fSpeed					= m_pVehicleModel->GetModelInput()->m_fMagVelocity * g_MsToMph;
	m_pVehicleModel->GetModelInput()->m_fSteering				= m_pVehiclePhysics->Controls().steer;
	
	m_pVehicleModel->GetModelInput()->m_bHandbrake				= m_pVehiclePhysics->Handbrake() > 0.f ? true : false;

	m_pVehicleModel->GetModelInput()->m_iControlType			= m_pVehiclePhysics->GetControlType();

	} //Synch part 1
	
	m_pVehicleModel->GetModelInput()->m_bAllWheelsOnGround = true;
	if (m_pVehiclePhysics->GetNumWheels() != m_pVehicleModel->GetModelInput()->m_Wheels.Size())
		return;

	for (S32 iWheel = 0; iWheel < m_pVehiclePhysics->GetNumWheels(); iWheel++)
	{
		Vehicle::C_WheelInput* pWheelInput	= &m_pVehicleModel->GetModelInput()->m_Wheels[iWheel];

		Vehicle::E_PartCode ePartCode		= Vehicle::E_PartCode_Wheel;
		Vehicle::E_PartSubCode eLocCode	= (Vehicle::E_PartSubCode)(iWheel+1);

		const RacingPhysics::CarWheel& wheel = ((RacingPhysics::Car*)m_pVehiclePhysics)->Wheel(iWheel);
		if(!wheel.OnGround())
			m_pVehicleModel->GetModelInput()->m_bAllWheelsOnGround = false;

		RacingPhysics::Axis axis;
		axis.SetRotation(wheel.GetRotationMatrix());
		axis.SetPosition(wheel.GetWheelPosition());
		
		Maths::HMat43 wMat = RPToMaths(&axis);

		if (m_pVehicleModel->GetDamageLayer())
		{
			Vehicle::C_WheelDamage* pWheelDamage = m_pVehicleModel->GetDamageLayer()->GetWheel(iWheel);
			if (pWheelDamage && pWheelDamage->m_TyreBurst)
			{
				Maths::HVec3 vPos = axis.Position();
/////				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCross(axis.Position(), 0.01f, Maths::RGBA(0xff0000ff));
				//RacingPhysics::Vector3R vUp = MathsToRP(&m_RawMatrix.AY());
				float fOffset = m_pVehiclePhysics->Dynamics().geometry.wheelBase * Maths::Tan(m_fRearAxlePivotAngle);
				axis.SetPosition(axis.Position() - m_pVehiclePhysics->GetAxis().AxisY() * (pWheelDamage->m_TyreSize - fOffset));				
				//axis.SetPosition(axis.Position() - vUp * (pWheelDamage->m_TyreSize - fOffset));
//////				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCross(axis.Position(), 0.01f, Maths::RGBA(0xff00ff00));
//////				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(vPos, axis.Position(), Maths::RGBA(0xffff0000));

//////				{
//////					DbUtil::I_DebugRender *pDebugRenderer = DbUtil::DefaultDebugRenderSys()->Deferred();
//////
//////					Vehicle::C_Part* pFrontTyre = &m_pVehicleModel->GetComponentList()[Vehicle::E_PartCode_Tyre].m_Nodes[Vehicle::E_PartSubCode_FrontLeft];
//////					Vehicle::C_Part* pFrontRim = &m_pVehicleModel->GetComponentList()[Vehicle::E_PartCode_AlloyRim].m_Nodes[Vehicle::E_PartSubCode_FrontLeft];
//////					if (pFrontRim->m_pElement && pFrontTyre->m_pElement)
//////					{			
//////						pDebugRenderer->Draw3dBox(pFrontRim->m_pElement->GetLocalBounds().Min(), pFrontRim->m_pElement->GetLocalBounds().Max(), wMat, Maths::RGBA(0xff0000ff));
//////						pDebugRenderer->Draw3dBox(pFrontTyre->m_pElement->GetLocalBounds().Min(), pFrontTyre->m_pElement->GetLocalBounds().Max(), wMat, Maths::RGBA(0xff00ff00));
//////					}
//////				}
//////			}
//////			else
//////			{
//////				DbUtil::I_DebugRender *pDebugRenderer = DbUtil::DefaultDebugRenderSys()->Deferred();
//////
//////				Vehicle::C_Part* pFrontTyre = &m_pVehicleModel->GetComponentList()[Vehicle::E_PartCode_Tyre].m_Nodes[Vehicle::E_PartSubCode_FrontLeft];
//////				Vehicle::C_Part* pFrontRim = &m_pVehicleModel->GetComponentList()[Vehicle::E_PartCode_AlloyRim].m_Nodes[Vehicle::E_PartSubCode_FrontLeft];
//////				if (pFrontRim->m_pElement && pFrontTyre->m_pElement)
//////				{			
//////					pDebugRenderer->Draw3dBox(pFrontRim->m_pElement->GetLocalBounds().Min(), pFrontRim->m_pElement->GetLocalBounds().Max(), wMat, Maths::RGBA(0xff0000ff));
//////					pDebugRenderer->Draw3dBox(pFrontTyre->m_pElement->GetLocalBounds().Min(), pFrontTyre->m_pElement->GetLocalBounds().Max(), wMat, Maths::RGBA(0xff00ff00));
//////				}
			}
		}
		RacingPhysics::Point3R vLocalPos = m_pVehiclePhysics->GetAxis().Local(axis.Position());
		float fSuspensionDist = wheel.SuspensionDistance();
		float fMaxTravel = (iWheel == 0 || iWheel == 1) ? m_fMaxWheelTravelFront : m_fMaxWheelTravelRear;
		if (fSuspensionDist > fMaxTravel)
		{
			RacingPhysics::Point3R vPos = axis.Position();
			RacingPhysics::Normal3R vUp	= m_pVehiclePhysics->GetAxis().AxisY();
			vPos -= vUp * (fSuspensionDist - fMaxTravel);
			axis.SetPosition(vPos);
		}
		else if (vLocalPos.Y() > fMaxTravel)
		{
			RacingPhysics::Point3R vPos = axis.Position();
			RacingPhysics::Normal3R vUp	= m_pVehiclePhysics->GetAxis().AxisY();
			vPos -= vUp * (vLocalPos.Y() - fMaxTravel);
			axis.SetPosition(vPos);
		}

		if (m_pVehicleModel->GetModelInput()->m_bShouldUpdate)
			pWheelInput->m_mMatrix				= RPToMaths(&axis);

		//if (!m_pVehicleModel->GetModelInput()->m_bShouldRender)
		//	pWheelInput->m_mMatrix.SetIdentity();
	
		if (Vehicle::C_Part* pNode = &m_pVehicleModel->GetComponentList()[ePartCode].m_Nodes[eLocCode])
		{
			if (pNode->m_Damage.m_Detached && pNode->m_pPhysicsBody)
				pWheelInput->m_mMatrix = pNode->m_pPhysicsBody->ElementMatrix();
		}

		//Wheel matrix
		Maths::HMat43 wheelMatrix = pWheelInput->m_mMatrix;
		wheelMatrix.AW(wheelMatrix.AW() + photoOffset);
#if PLATFORM_PC
		// [Blur2PC] render interpolation: the rotor's spin about the wheel's axle (its local X), so renders
		// between ticks turn the tyre, rim, disc and alloy the whole way (Blur2PCInterpRecordWheelStore)
		F32 blur2pcSpinRate = 0.0f;
		{
			const RacingPhysics::Vector3R rotorAngVel = wheel.GetRotor().AngularVelocity();
			const Maths::HVec3 axle = wheelMatrix.AX();
			const F32 axleLength = (F32)axle.Length();
			if (axleLength > 1e-6f)
				blur2pcSpinRate = (F32)Maths::DotProduct(Maths::HVec3(RPToMaths(&rotorAngVel)), axle) / axleLength;
		}
#endif

		//Update matrices for wheel elements
		Vehicle::C_Part* pPart = &m_pVehicleModel->GetComponentList()[Vehicle::E_PartCode_Tyre].m_Nodes[eLocCode];
		if(pPart->m_pWorldMatrix)
		{
			Maths::Store((Maths::HMat44)wheelMatrix, pPart->m_pWorldMatrix);
#if PLATFORM_PC
			WorldRender::Blur2PCInterpRecordWheelStore(pPart->m_pModelInstance, pPart->m_pWorldMatrix, wheelMatrix, wheelMatrix, blur2pcSpinRate);	// [Blur2PC] render interpolation
#endif
		}

		pPart = &m_pVehicleModel->GetComponentList()[Vehicle::E_PartCode_AlloyRim].m_Nodes[eLocCode];		
		if(pPart->m_pWorldMatrix)
		{
			Maths::Store((Maths::HMat44)wheelMatrix, pPart->m_pWorldMatrix);
#if PLATFORM_PC
			WorldRender::Blur2PCInterpRecordWheelStore(pPart->m_pModelInstance, pPart->m_pWorldMatrix, wheelMatrix, wheelMatrix, blur2pcSpinRate);	// [Blur2PC] render interpolation
#endif
		}
		
		pPart = &m_pVehicleModel->GetComponentList()[Vehicle::E_PartCode_BrakeDisc].m_Nodes[eLocCode];
		if(pPart->m_pWorldMatrix)
		{
			Maths::Store((Maths::HMat44)wheelMatrix, pPart->m_pWorldMatrix);
#if PLATFORM_PC
			WorldRender::Blur2PCInterpRecordWheelStore(pPart->m_pModelInstance, pPart->m_pWorldMatrix, wheelMatrix, wheelMatrix, blur2pcSpinRate);	// [Blur2PC] render interpolation
#endif
		}
		
		//Brake calipers mustn't rotate
		//if (pPart && pPart->m_pModelInstance && pPart->m_pElement)
		pPart = m_pVehicleModel->GetComponent(Vehicle::E_PartCode_BrakeCaliper, eLocCode);
		if(pPart->m_pWorldMatrix)
		{
			Maths::HMat43 wheelMat = pWheelInput->m_mMatrix;
			wheelMat.Rotation(Maths::HMat33(wheel.GetRotationMatrix().AxisX(), m_pVehiclePhysics->GetAxis().AxisY(), wheel.GetRotationMatrix().AxisX() ^ m_pVehiclePhysics->GetAxis().AxisY()));
			pWheelInput->m_mYawMatrix = wheelMat.Rotation();
			pPart->m_Matrix = pPart->m_RestMatrix * wheelMat;
			pPart->m_Matrix.AW((pPart->m_Matrix.AW() - (pPart->m_Matrix.AW() - pWheelInput->m_mMatrix.AW())) + photoOffset);
			Maths::Store((Maths::HMat44)pPart->m_Matrix, pPart->m_pWorldMatrix);
#if PLATFORM_PC
			WorldRender::Blur2PCInterpRecordStore(pPart->m_pModelInstance, pPart->m_pWorldMatrix, pPart->m_Matrix);	// [Blur2PC] render interpolation
#endif
		}

		//Now must do alloys...
		WorldRender::C_ModelInstance* pAlloyModelInstance = NULL;
		if (m_pWheelModelResource)
			pAlloyModelInstance = m_pVehicleModel->GetComponentList().GetModelInstance(Vehicle::E_PartCode_Alloy, eLocCode);
		if (pAlloyModelInstance && !g_debugUpdateWheelScale)
		{
			pPart = &m_pVehicleModel->GetComponentList()[Vehicle::E_PartCode_Alloy].m_Nodes[eLocCode];
			pPart->m_Matrix = pPart->m_RestMatrix * pWheelInput->m_mMatrix;

			pAlloyModelInstance->SetWorldSpherePosition(pPart->m_Matrix.Translation());

			Maths::HMat43 mat = pPart->m_Matrix;
			mat.AW(mat.AW() + photoOffset);
#if PLATFORM_PC
			pAlloyModelInstance->Blur2PCSetWheelElementMatrix(-1, mat, wheelMatrix, blur2pcSpinRate);	// [Blur2PC] render interpolation
#else
			pAlloyModelInstance->SetElementMatrix(-1, mat);
#endif								
		}

		pWheelInput->m_bOnGround			= wheel.OnGround();

		if (pWheelInput->m_bOnGround)
		{
			pWheelInput->m_iSurfaceType			= (U32)wheel.SurfaceType();
			pWheelInput->m_iSurfaceModifier		= (U32)wheel.SurfaceModifier();

			pWheelInput->m_vGroundPosition		= RPToMaths(&wheel.GroundPosition());
			pWheelInput->m_vGroundNormal		= RPToMaths(&wheel.GroundNormal());
		}
		
		RacingPhysics::Vector3R angVel			= wheel.GetRotor().AngularVelocity();
		pWheelInput->m_vAngularVelocity			= RPToMaths(&angVel);
		
		pWheelInput->m_fSlipAngle				= wheel.SlipAngle();
	}	
}

Debug::Bool g_debugVehicleNightLights("Vehicle.InitialState.NightLights", true);
Debug::Float g_debugInitialScratchLevel("Vehicle.InitialState.ScratchLevel", 0.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugInitialDamageLevel("Vehicle.InitialState.DamageLevel", 0.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Float g_debugVehicleInstanceMinDraw("Vehicle.DrawDistance.DefuaultMinDraw", 500.0f, Debug::Range(0.0f, 4000.0f), Debug::Step(1.0f));
Debug::Float g_debugVehicleInstanceDrawFactor("Vehicle.DrawDistance.DefaultFactor", 200.0f, Debug::Range(0.0f, 1000.0f), Debug::Step(1.0f));

Debug::Float g_debugExteriorMinDraw("Vehicle.DrawDistance.ExteriorMinDraw", 500.0f, Debug::Range(0.0f, 4000.0f), Debug::Step(1.0f));
Debug::Float g_debugExteriorDrawFactor("Vehicle.DrawDistance.ExteriorFactor", 200.0f, Debug::Range(0.0f, 1000.0f), Debug::Step(1.0f));
Debug::Float g_debugInteriorMinDraw("Vehicle.DrawDistance.InteriorMinDraw", 500.0f, Debug::Range(0.0f, 4000.0f), Debug::Step(1.0f));
Debug::Float g_debugInteriorDrawFactor("Vehicle.DrawDistance.InteriorFactor", 50.0f, Debug::Range(0.0f, 1000.0f), Debug::Step(1.0f));
Debug::Float g_debugWheelMinDraw("Vehicle.DrawDistance.WheelMinDraw", 500.0f, Debug::Range(0.0f, 4000.0f), Debug::Step(1.0f));
Debug::Float g_debugWheelDrawFactor("Vehicle.DrawDistance.WheelFactor", 350.0f, Debug::Range(0.0f, 1000.0f), Debug::Step(1.0f));
Debug::Float g_debugBonnetMinDraw("Vehicle.DrawDistance.BonnetMinDraw", 500.0f, Debug::Range(0.0f, 4000.0f), Debug::Step(1.0f));
Debug::Float g_debugBonnetDrawFactor("Vehicle.DrawDistance.BonnetFactor", 100.0f, Debug::Range(0.0f, 1000.0f), Debug::Step(1.0f));
Debug::Float g_debugBootMinDraw("Vehicle.DrawDistance.BootMinDraw", 500.0f, Debug::Range(0.0f, 4000.0f), Debug::Step(1.0f));
Debug::Float g_debugBootDrawFactor("Vehicle.DrawDistance.BootFactor", 100.0f, Debug::Range(0.0f, 1000.0f), Debug::Step(1.0f));
Debug::Float g_debugWingMinDraw("Vehicle.DrawDistance.WingMinDraw", 500.0f, Debug::Range(0.0f, 4000.0f), Debug::Step(1.0f));
Debug::Float g_debugWingDrawFactor("Vehicle.DrawDistance.WingFactor", 100.0f, Debug::Range(0.0f, 1000.0f), Debug::Step(1.0f));
Debug::Float g_debugDoorMinDraw("Vehicle.DrawDistance.DoorMinDraw", 500.0f, Debug::Range(0.0f, 4000.0f), Debug::Step(1.0f));
Debug::Float g_debugDoorDrawFactor("Vehicle.DrawDistance.DoorFactor", 150.0f, Debug::Range(0.0f, 1000.0f), Debug::Step(1.0f));
Debug::Float g_debugMirrorMinDraw("Vehicle.DrawDistance.MirrorMinDraw", 500.0f, Debug::Range(0.0f, 4000.0f), Debug::Step(1.0f));
Debug::Float g_debugMirrorDrawFactor("Vehicle.DrawDistance.MirrorFactor", 250.0f, Debug::Range(0.0f, 1000.0f), Debug::Step(1.0f));
Debug::Float g_debugWindowMinDraw("Vehicle.DrawDistance.WindowMinDraw", 500.0f, Debug::Range(0.0f, 4000.0f), Debug::Step(1.0f));
Debug::Float g_debugWindowDrawFactor("Vehicle.DrawDistance.WindowFactor", 175.0f, Debug::Range(0.0f, 1000.0f), Debug::Step(1.0f));
Debug::Float g_debugBumperFrontMinDraw("Vehicle.DrawDistance.BumperFrontMinDraw", 200.0f, Debug::Range(0.0f, 4000.0f), Debug::Step(1.0f));
Debug::Float g_debugBumperFrontDrawFactor("Vehicle.DrawDistance.BumperFrontFactor", 100.0f, Debug::Range(0.0f, 1000.0f), Debug::Step(1.0f));
Debug::Float g_debugBumperBackMinDraw("Vehicle.DrawDistance.BumperBackMinDraw", 500.0f, Debug::Range(0.0f, 4000.0f), Debug::Step(1.0f));
Debug::Float g_debugBumperBackDrawFactor("Vehicle.DrawDistance.BumperBackFactor", 200.0f, Debug::Range(0.0f, 1000.0f), Debug::Step(1.0f));
Debug::Float g_debugLightMinDraw("Vehicle.DrawDistance.LightMinDraw", 500.0f, Debug::Range(0.0f, 4000.0f), Debug::Step(1.0f));
Debug::Float g_debugLightDrawFactor("Vehicle.DrawDistance.LightFactor", 1500.0f, Debug::Range(0.0f, 5000.0f), Debug::Step(1.0f));

/*
===============
NAME: SetDrawDistance

NOTE: SetDrawDistance
===============
*/
void C_VehicleActor::SetDrawDistance(WorldRender::C_ModelInstance* p_pModelInstance, const Util::C_String p_Name)
{
	if (!p_pModelInstance)
		return;
	
	Maths::C_Sphere sphere = p_pModelInstance->GetWorldSphere();

	float minDraw = g_debugVehicleInstanceMinDraw;
	float drawFactor = g_debugVehicleInstanceDrawFactor;

	if (p_Name.ISearch("_exterior") != -1)
	{
		minDraw		= g_debugExteriorMinDraw;
		drawFactor	= g_debugExteriorDrawFactor;
	}
	else if (p_Name.ISearch("_interior") != -1)
	{
		minDraw		= g_debugInteriorMinDraw;
		drawFactor	= g_debugInteriorDrawFactor;
	}
	else if (p_Name.ISearch("_wheel_") != -1 || p_Name.ISearch("e_wheel") != -1 || p_Name.ISearch("m_wheel") != -1 || p_Name.ISearch("_alloy") != -1)
	{
		minDraw		= g_debugWheelMinDraw;
		drawFactor	= g_debugWheelDrawFactor;
	}
	else if (p_Name.ISearch("_bonnet") != -1)
	{
		minDraw		= g_debugBonnetMinDraw;
		drawFactor	= g_debugBonnetDrawFactor;
	}
	else if (p_Name.ISearch("_boot") != -1)
	{
		minDraw		= g_debugBootMinDraw;
		drawFactor	= g_debugBootDrawFactor;
	}
	else if (p_Name.ISearch("_wing") != -1)
	{
		minDraw		= g_debugWingMinDraw;
		drawFactor	= g_debugWingDrawFactor;
	}
	else if (p_Name.ISearch("_door") != -1)
	{
		minDraw		= g_debugDoorMinDraw;
		drawFactor	= g_debugDoorDrawFactor;
	}
	else if (p_Name.ISearch("_mirror") != -1)
	{
		minDraw		= g_debugMirrorMinDraw;
		drawFactor	= g_debugMirrorDrawFactor;
	}
	else if (p_Name.ISearch("_window") != -1)
	{
		minDraw		= g_debugWindowMinDraw;
		drawFactor	= g_debugWindowDrawFactor;
	}
	else if (p_Name.ISearch("_bumper_front") != -1)
	{
		minDraw		= g_debugBumperFrontMinDraw;
		drawFactor	= g_debugBumperFrontDrawFactor;
	}
	else if (p_Name.ISearch("_bumper_back") != -1)
	{
		minDraw		= g_debugBumperBackMinDraw;
		drawFactor	= g_debugBumperBackDrawFactor;
	}
	else if (p_Name.ISearch("_headlight") != -1 || p_Name.ISearch("_foglight") != -1 ||
			p_Name.ISearch("_brakelight") != -1 || p_Name.ISearch("_rearlight") != -1 ||
			p_Name.ISearch("_reverselight") != -1)
	{
		minDraw		= g_debugLightMinDraw;
		drawFactor	= g_debugLightDrawFactor;
	}

	F32 farDist = Maths::Min(minDraw, drawFactor * sphere.Radius());
	p_pModelInstance->SetNearFarDistance(0.0f, farDist);	
}

//#pragma optimize("", off)

/*
===============
NAME: Create

NOTE: Create
===============
*/
void C_VehicleActor::SetupDirtLayer()
{
	//REMOVE_DIRT

	//Game::C_LevelState& levelState  = Game::C_Game::GetInstance()->GetLevelState();
	//if (!C_GameData::Instance().mCities[(GAMEDATA_ID)levelState.mCity])
	//{
	//	m_pVehicleModel->GetDirtLayer()->Reset();
	//	return;
	//}

	//AmaxVehicleDirtSettings* pSettings = C_GameData::Instance().mCities[(GAMEDATA_ID)levelState.mCity]->vehicleDirtSettings;
	//if (!pSettings)
	//{
	//	m_pVehicleModel->GetDirtLayer()->Reset();
	//	return;
	//}

	//m_pVehicleModel->GetDirtLayer()->Enable();

	//m_pVehicleModel->GetDirtLayer()->m_vBaseLayerColour = MathsStorage::Float4(pSettings->baseColour.r, pSettings->baseColour.g, pSettings->baseColour.b, 0.0f);
	//m_pVehicleModel->GetDirtLayer()->m_fBaseStartTime	= pSettings->baseStartTime;
	//m_pVehicleModel->GetDirtLayer()->m_fBaseDuration	= pSettings->baseDuration;
	//m_pVehicleModel->GetDirtLayer()->m_fMinSpeedForBase	= pSettings->baseSpeedThreshold;

	//m_pVehicleModel->GetDirtLayer()->m_vDetailLayerColour = MathsStorage::Float4(pSettings->detailColour.r, pSettings->detailColour.g, pSettings->detailColour.b, 0.0f);
	//m_pVehicleModel->GetDirtLayer()->m_fDetailStartTime	= pSettings->detailStartTime;
	//m_pVehicleModel->GetDirtLayer()->m_fDetailDuration	= pSettings->detailDuration;
	//m_pVehicleModel->GetDirtLayer()->m_fMinSpeedForDetail	= pSettings->detailSpeedThreshold;
}

Debug::Bool g_debugVehiclePartsEnablePhysics("Vehicle.Damage.Parts.EnablePhysics", true);
Debug::Bool g_debugDisableIntWindows("Vehicle.DisableIntWindows", true);
Debug::Bool g_debugDisableHeadlightCovers("Vehicle.DisableHeadlightCovers", false);
Debug::Bool g_debugVehicleComponentCollisionOn("Vehicle.Component.Parts.Collision", false);

Debug::Float g_debugWheelClipRenderHackSuspensionHack("Vehicle.Wheels.RenderClipHackMaxSuspensionTravel", 0.05f, Debug::Range(0.0f, 0.5f), Debug::Step(0.001f));

Debug::Float g_debugVehicleKILLPLANE("Vehicle.KillPlane", -500.0f, Debug::Range(-1000.0f, 1000.0f), Debug::Step(0.001f));

Debug::Float g_debugGarageSteerAngle("Vehicle.ShowRoom.SteerAngle", 0.48f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));

//Chad WiP - collision stuff
Debug::Bool g_debugEnableDeformableCollision("Vehicle.Collision.EnableDeformableCollisionMesh", false);
Debug::Float g_debugCarCollisionMargin("Vehicle.Collision.Margin", 0.04f, Debug::Range(0.0f, 0.1f), Debug::Step(0.001f));

#ifndef CFG_GOLD
Debug::Bool g_RoadsweeperPickups("Multiplayer.Upgrades.RoadsweeperPickups", false, Debug::Persist());
#endif

/*
===============
NAME: Create

NOTE: Create
===============
*/
bool C_VehicleActor::Create()
{
	MEMORY_TRACKER_PUSH("graphics.vehicle.actor");

	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	if( m_pRoughTerrainSpecialFX)
	{
		m_pRoughTerrainSpecialFX->Stop();
		SAFE_DELETE_PTR(m_pRoughTerrainSpecialFX);
	}
	if( m_pHighSpeedSpecialFX)
	{
		m_pHighSpeedSpecialFX->Stop();
		SAFE_DELETE_PTR(m_pHighSpeedSpecialFX);
	}

	m_fLandedEffectTime = 0.0f;
	if (m_pLandedSpecialFX)
	{
		m_pLandedSpecialFX->Stop();
		SAFE_DELETE_PTR(m_pLandedSpecialFX)
	}

	m_fImpactEffectTime = 0.0f;
	if (m_pImpactSpecialFX)
	{
		m_pImpactSpecialFX->Stop();
		SAFE_DELETE_PTR(m_pImpactSpecialFX)
	}

	m_fHealthEffectTime = 0.0f;
	if (m_pHealthSpecialFX)
	{
		m_pHealthSpecialFX->Stop();
		SAFE_DELETE_PTR(m_pHealthSpecialFX)
	}	

	m_hUpdateSchedulerGroup = Scheduler::CreateGroup();

#ifndef CFG_GOLD
	m_LastKnownGroundPosition = Maths::HVec3(0.0f, 0.0f, 0.0f);
#endif

	const C_VehicleActorCreate *pVehicleCreate = (const C_VehicleActorCreate *)(GetCreationData().GetData());

	m_VisibilityOverride = false;
	m_Visible = 0x0F;
	m_HasLogicActor = pVehicleCreate->m_HasLogicActor;
	
	m_pVisDetect = ::Actors::C_VisibilityDetector::CreateVisDetect(GetHandle());
	m_pReflectionVisDetect = ::Actors::C_VisibilityDetector::CreateVisDetect(GetHandle());

	AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];

	if (pVehicle)
		m_WeightClass = pVehicle->weightClass;

#ifdef AMAX_VEH_USE_MI
	Vehicle::XtVehicle* pXtVehicle = (Vehicle::XtVehicle*)pVehicle;
#else
	Vehicle::XtVehicle* pXtVehicle = pVehicle ? &pVehicle->baseVehicle : NULL;
#endif
	Vehicle::C_VehicleModelSetup setup;
	setup.m_bCreateDamageLayer	= true;
	setup.m_bCreateGameLayer	= true;
	setup.m_bCreateRenderLayer	= true;
	setup.m_pXtVehicle			= pXtVehicle;
	setup.m_InitialState.m_NightLights	= (bool)g_debugVehicleNightLights;
	setup.m_InitialState.m_ScratchLevel	= (float)g_debugInitialScratchLevel;
	setup.m_InitialState.m_DamageLevel	= (float)g_debugInitialDamageLevel;
	setup.m_pLightSamplePointScene = &GetWorldRenderScene()->GetLightSamplePointScene();
	//setup.m_Index	= Vehicle::C_VehicleModelManager::Instance().GetNumVehicles();
	setup.m_Index	= pVehicleCreate->m_VehicleIndex;
	//m_pVehicleModel = Vehicle::C_VehicleModelManager::Instance().AddVehicleModel(&setup);
	m_pVehicleModel = Vehicle::C_VehicleModelManager::Instance().CreateVehicleModel(&setup);

	m_PhysicsGroupId = Game::PhysicsConstants::E_PhysicsGroupID_VehicleStart + setup.m_Index;

	if (!m_pVehicleModel)
		return false;

	m_fMaxWheelTravelFront	= pVehicle->frontWheelMaxTravel == 0.0f ? (float)g_debugWheelClipRenderHackSuspensionHack : pVehicle->frontWheelMaxTravel;
	m_fMaxWheelTravelRear	= pVehicle->rearWheelMaxTravel == 0.0f ? (float)g_debugWheelClipRenderHackSuspensionHack : pVehicle->rearWheelMaxTravel;
	
	m_ParticleEmitScale = 1.0f;
	m_ParticleLifeScale = 1.0f;
	m_ParticleReducedEmitScaleTimer = 0.f;
	
	m_FramesInWater		= 0;

	m_pLightEffectManager	= NULL;
	m_pHeadlightTexture		= NULL;
	m_pGameWorld			= pVehicleCreate->m_pGameWorld;
	m_pLightEffectManager	= m_pGameWorld->GetLightEffectManager();
	m_pHeadlightTexture		= m_pGameWorld->GetHeadlightTexture();
	m_pViewManager			= m_pGameWorld->GetViewManager();		

	//Grr
	m_pVehicleModel->GetModelInput()->m_Wheels.Resize(4);
	
	m_pVehicleModel->SetModelResource(m_pVehicleModelResource);
	m_pVehicleModel->SetVehicleModelResource(m_pVehicleModelResource);
	m_pVehicleModel->SetWheelModelResource(m_pWheelModelResource);
	m_pVehicleModel->GetComponentList().Clear();
	m_pVehicleModel->SetVisDetect(m_pVisDetect);
	
	m_Matrix.SetAxes(	Maths::HVec4(pVehicleCreate->m_WorldMatrix.ax.vx, pVehicleCreate->m_WorldMatrix.ax.vy, pVehicleCreate->m_WorldMatrix.ax.vz, 0.0f),
						Maths::HVec4(pVehicleCreate->m_WorldMatrix.ay.vx, pVehicleCreate->m_WorldMatrix.ay.vy, pVehicleCreate->m_WorldMatrix.ay.vz, 0.0f),
						Maths::HVec4(pVehicleCreate->m_WorldMatrix.az.vx, pVehicleCreate->m_WorldMatrix.az.vy, pVehicleCreate->m_WorldMatrix.az.vz, 0.0f),
						Maths::HVec4(pVehicleCreate->m_WorldMatrix.aw.vx, pVehicleCreate->m_WorldMatrix.aw.vy, pVehicleCreate->m_WorldMatrix.aw.vz, 1.0f));
	
	const GameResource::C_Model* pShadowCasterModel = NULL;
	
	int foundCollisionMeshFlags = 0;
	bool foundShadowCaster = false;
	m_tempHaveNewVehicleToVehicleMesh = false;

	for (S32 iModel = 0; iModel < m_pVehicleModelResource->GetModelData()->GetModelCount(); iModel++)
	{
		const GameResource::C_Model* pModel = m_pVehicleModelResource->GetModelData()->GetModel(iModel);
		
		Util::C_String modelName = pModel->GetName();
		modelName.ConvertToLower();

		if (modelName.ISearch("_bone") != -1)
		{
			CreateBones(pModel);
			continue;
		}

		Maths::HMat33 identity;
		identity.SetIdentity();

		if (modelName == "m_collisionmesh")
		{
			foundCollisionMeshFlags |= CreateCollisionMesh(pModel, levelState.mCars[m_VehicleIndex]);
			continue;
		}

		if (modelName == "m_shadowcaster")
		{
			//#ifndef PLATFORM_PS3
			pShadowCasterModel = pModel;
			foundShadowCaster = true;
			//#endif
			continue;
		}

		SceneGraph::C_SceneGraph* pSceneGraph = GetSceneGraph();

		//MEMORY_TRACKER_PUSH("graphics.vehicle.actor.modelinstances");
		WorldRender::C_ModelInstance* pModelInstance = new WorldRender::C_ModelInstance(m_pVehicleModelResource->GetRenderModelData(), pModel->GetIndex(), pSceneGraph);
		//MEMORY_TRACKER_POP("graphics.vehicle.actor.modelinstances");		
		bool added = false;
		pModelInstance->SetVisDetect(m_pVisDetect);
		pModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OPAQUE, E_RenderPass_VehicleSolid);
		pModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_BACK_FRONT, E_RenderPass_VehicleAlpha);
		if (!C_Game::GetInstance()->GetLevelState().mInUI)
			pModelInstance->DisableVisibilityFlags(E_ViewTypeFlag_CubeMap | E_ViewTypeFlag_Reflection);
		else
			pModelInstance->DisableVisibilityFlags(E_ViewTypeFlag_CubeMap);

		if (modelName == "m_interior" ||
			modelName == "m_bumper_back" ||
			modelName == "m_boot")
		{
			pModelInstance->DisableVisibilityFlags(E_ViewTypeFlag_Mirror);
		}
		SceneGraph::Insert(pSceneGraph, pModelInstance);
		m_ModelInstances.PushBack(pModelInstance);
		//Maths::C_Sphere sphere = pModel->GetLocalBounds().BoundingSphere();
		Maths::C_Sphere sphere = pModel->GetBoundingSphere();
		
		if (pModelInstance->GetRenderListNode() && pModelInstance->GetCullNode())
		{		
			pModelInstance->SetWorldSphere(sphere);
			SetDrawDistance(pModelInstance, pModel->GetName());
			m_pVehicleModel->AddModelInstance(pModelInstance, pModel->GetName());
			// set the parent matrix pointer. this is used during rendering to calculate the PVS viewing direction
			pModelInstance->SetParentMatrixPtr(&m_Matrix);
			added = true;
		}
		
		if (!added)
		{
			Util::C_String FFS = pModel->GetName();
			LOGERROR(g_VehicleActorLog, "::Create - ModelInstance created but no added, so deleting: '%s'", FFS.Chars());
			SAFE_DELETE_PTR(pModelInstance);
			continue;
		}

		for (S32 iElement = 0; iElement < pModel->GetElementCount(); iElement++)
		{
			const GameResource::C_ModelElement* pModelElement = pModel->GetElement(iElement);
			pModelInstance->SetElementMatrix(iElement, pModelElement->GetMatrix() * m_Matrix );

			Util::C_String elementName = pModelElement->GetName();
			elementName.ConvertToLower();

			if (elementName == "e_wheel_frontleft" ||
				elementName == "e_wheel_frontright" ||
				elementName == "e_wheel_backleft" ||
				elementName == "e_wheel_backright")
				{
					CreateWheelInstance(pModelElement);
				}

			//Get collision mesh, if there is any
			//Collision::C_CollisionFilter filter = PhysicsConstants::kDynamicObjectCollisionFilter;
			Collision::C_CollisionFilter filter = PhysicsConstants::kVehicleBlockCollisionFilter;
			if(!g_debugVehicleComponentCollisionOn)
				filter.SetCollisionMask(0);
			//////Collision::C_CollisionFilter filter = PhysicsConstants::kDynamicObjectCollisionFilter;
			////////Collision::C_CollisionFilter filter = PhysicsConstants::kVehicleBlockCollisionFilter;
			////////if(!g_debugVehicleComponentCollisionOn)
			////////	filter.m_CollisionMask = 0;
			filter.SetGroupId(m_PhysicsGroupId);
			filter.SetGroupFlags(0);
			filter.SetGroupMask(0);
			Gravity::C_Body* pPhysicsBody = NULL;
			Gravity::C_CompositeDesc compDesc;

			if (g_debugVehiclePartsEnablePhysics)
			{
				MEMORY_TRACKER_PUSH("graphics.vehicle.actor.gravityobjects");
				Gravity::C_ImportShape* pImportShape = const_cast<Gravity::C_ImportShape*>( pModelElement->GetPhysicsImportShape() );

				if (pImportShape && pImportShape->GetShapeDesc() && (	pImportShape->GetShapeDesc()->m_Type != Collision::E_TriangleMesh_Shape &&
																		pImportShape->GetShapeDesc()->m_Type != Collision::E_InstancedTriangleMesh_Shape && 
																		pImportShape->GetShapeDesc()->m_Type != Collision::E_TriangleMeshBase_Shape))
				{
					if (!pImportShape->GetShapeDesc()->m_RenderOffset.Finite())
					{
						LOGERROR(g_VehicleActorLog, "TEMP HACK FIX: pImportShape->GetShapeDesc()->m_RenderOffset is INFINITE!!!");
						pImportShape->GetShapeDesc()->m_RenderOffset = Maths::Constants<Maths::HVec3>::Zero();
					}
					if (!pImportShape->GetImportIneriaTensor().Finite())
					{
						LOGERROR(g_VehicleActorLog, "TEMP HACK FIX: pImportShape->GetImportIneriaTensor() is INFINITE!!!");
						pImportShape->SetImportIneria(Maths::Constants<Maths::HMat33>::Identity());
					}
					if (!Maths::HF32(pImportShape->GetShapeDesc()->m_Mass).Finite())
					{
						LOGERROR(g_VehicleActorLog, "TEMP HACK FIX: pImportShape->GetShapeDesc()->m_Mass is INFINITE!!!");
						pImportShape->GetShapeDesc()->m_Mass = Maths::Constants<Maths::HF32>::One();
					}

					pImportShape->GetShapeDesc()->SetPhysicsMaterial(Game::E_PhysicsMaterial_Vehicle);

					compDesc.AddShape(	pImportShape->GetShapeDesc(), Maths::HMat43(pImportShape->GetImportRotation(),
										pImportShape->GetImportPosition()), 1.f, &filter);

					compDesc.m_HasCollision = g_debugVehicleComponentCollisionOn;

					
					compDesc.RemapShape();
					compDesc.SetRemapped();

					SharedPhysics::C_Deactivator deactivator;
					deactivator.m_ActivateThresholdSq = 0.005f;
					deactivator.m_SleepThresholdSq = 0.001f;

					pPhysicsBody = GetPhysicsScene()->CreateBody();
									
					//pPhysicsBody->SetExternalConstants(GetPhysicsExternalConstants());
					pPhysicsBody->SetExternalConstants(&m_GravityConstants);					
					pPhysicsBody->Slop(PhysicsConstants::kDefaultSlop);
					pPhysicsBody->Baumgarte(PhysicsConstants::kDefaultBaumgarte);
					
					Maths::HMat43 mat = pModelElement->GetMatrix() * m_Matrix;
					Gravity::C_BodyHelper bodyHelper(&compDesc);
					bodyHelper.CreateBody(100.0f, mat.AW(), (Maths::HMat33)mat, pPhysicsBody, &filter, 0, 0);

					pPhysicsBody->SetDeactivator(deactivator);
					pPhysicsBody->Deactivate();
					float fMaxLinVel = (float)g_debugAbsoluteMaxAVehicleCanEverGo * g_MphToMs;
					pPhysicsBody->SetMaxLinearVelocity(fMaxLinVel);
					pPhysicsBody->SetMaxAngularVelocity(Maths::Constants<Maths::HF32>::Three() * Maths::Constants<Maths::HF32>::Pi() * Maths::Constants<Maths::HF32>::Two());
					//pPhysicsBody->SetMaxAngularVelocity(60.f);
					
					pPhysicsBody->LinearVelocity(Maths::HVec3(0.0f, 0.0f, 0.0f));						
				}				
				MEMORY_TRACKER_POP("graphics.vehicle.actor.gravityobjects");
			}

			if (pPhysicsBody)
			{
				const Maths::HMat43 matrix(pModelElement->GetMatrix() * m_Matrix);
				pPhysicsBody->ElementMatrix(matrix);
				this->m_PrevPosition = matrix.Translation();
			}
			
			Vehicle::C_Part* pNode = m_pVehicleModel->SetComponent(pModel, pModelElement, m_pVehicleModelResource, pModelInstance, pPhysicsBody, GetPhysicsScene());
				
			// if no part code (ie its just body or whatever) or its a part which isn't wheels then store world matrix pointer for later
			if(!pNode || (pNode && pNode->m_PartCode > Vehicle::E_PartCode_WheelLod))
			{
				const WorldRender::MaterialParameter* pElementMatrixParam = pModelInstance->GetRenderModel()->GetWorldMatrixParameter(iElement);
				if(pElementMatrixParam && pElementMatrixParam->index >= 0)
				{
					m_WorldMatrices.PushBack((MathsStorage::MatrixRow44*)pModelInstance->GetParameter(*pElementMatrixParam));
#if PLATFORM_PC
					m_Blur2PCWorldMatrixOwners.PushBack(pModelInstance);	// [Blur2PC]
#endif
				}
			}

			if (pNode && pNode->m_PartCode == Vehicle::E_PartCode_HeadlightCover && g_debugDisableHeadlightCovers)
				pNode->SetPermanentVisibility(0);
			else if (pNode && pNode->m_PartCode == Vehicle::E_PartCode_Window && !C_Game::GetInstance()->GetLevelState().mInUI)
			{
				if ((pNode->m_PartSubCode == Vehicle::E_PartSubCode_FrontRight || pNode->m_PartSubCode == Vehicle::E_PartSubCode_Right) && pVehicle->rightHandDrive)
					pNode->SetPermanentVisibility(0);
				else if ((pNode->m_PartSubCode == Vehicle::E_PartSubCode_FrontLeft || pNode->m_PartSubCode == Vehicle::E_PartSubCode_Left) && !pVehicle->rightHandDrive)
					pNode->SetPermanentVisibility(0);
			}
			else if (pNode && (pNode->m_PartCode == Vehicle::E_PartCode_BrakeLight || pNode->m_PartCode == Vehicle::E_PartCode_RearLight || pNode->m_PartCode == Vehicle::E_PartCode_RearAndBrakeLight))
			{
				LightTrails::C_LightTrailInstance* pLightTrailIntance = pVehicleCreate->m_pGameWorld->GetLightTrails()->AllocLightTrailInstance();

				if (pNode->m_PartCode == Vehicle::E_PartCode_BrakeLight)
					pLightTrailIntance->SetColour(Maths::C_RGBA_F32(C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailBreakColour.r, C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailBreakColour.g, C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailBreakColour.b, C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailBreakColour.a));
				else
					pLightTrailIntance->SetColour(Maths::C_RGBA_F32(C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailRearColour.r, C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailRearColour.g, C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailRearColour.b, C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailRearColour.a));
				pLightTrailIntance->SetCameraFadeFactor(C_GameEffectData::Instance().mAmaxMiscEffects[0]->lightTrailCameraFactor);
				
				pLightTrailIntance->SetSize(Maths::Max(0.10f, pModelElement->GetLocalBounds().Diagonal().X() * 1.3f / 2.f), Maths::Max(0.10f, pModelElement->GetLocalBounds().Diagonal().Y() * 1.3f / 2.f));
				pLightTrailIntance->Reset(pNode->GetCentre() * m_Matrix);
				
				m_LightTrails.PushBack();
				
				m_LightTrails.Back().m_pLightTrailInstance = pLightTrailIntance;
				m_LightTrails.Back().m_Position = pNode->GetCentre();
				m_LightTrails.Back().m_PartCode = pNode->m_PartCode;

			}

			//For now, turn off interior windows until rendering is sorted
			if (pNode && pNode->m_PartCode == Vehicle::E_PartCode_WindowInterior)
			{
				if (g_debugDisableIntWindows)
					pNode->SetVisible(false);
				else
					m_pVehicleModel->SetPartVisibility(Vehicle::E_PartCode_WindowInterior, false);					
			}
			
			bool bAllowLightFlares = true;

			//Add the light flares
			if (pNode && (pNode->m_PartCode == Vehicle::E_PartCode_HeadLight) && setup.m_pXtVehicle->m_LightFlares)
			{
				XtHandle<DynamicEffect::Light::XtInterface::Light> xtLight;
				if (!C_Game::GetInstance()->GetLevelState().mInUI) //Special headlights for showroom
					xtLight = setup.m_pXtVehicle->m_LightFlares->m_HeadLight;
				else
				{
					if (setup.m_pXtVehicle->m_LightFlares->m_ShowRoomHeadLight)
						xtLight = setup.m_pXtVehicle->m_LightFlares->m_ShowRoomHeadLight;
					else
						xtLight = setup.m_pXtVehicle->m_LightFlares->m_HeadLight;
				}
				if (xtLight && xtLight->pDefinition && bAllowLightFlares)
				{
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.PushBack();	
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_pNode = pNode;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_XtHandle = xtLight;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_Inverted = false;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_pLightDefinition = xtLight->pDefinition;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_RenderOffset = setup.m_pXtVehicle->m_FrontLightRenderOffset;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_pDamageNode = NULL;

					//Add aux flare
					AddAuxFlare(pNode, Maths::HMat33() ,Maths::HVec3(0.f, 0.f, 0.10f), 2.f, 1.f, Vehicle::E_AuxFlareColour_White);
				}
 				const Vehicle::XtDynamicLight& dynamicLight =  setup.m_pXtVehicle->m_DynamicLights.m_HeadDynamicLight;
 				//if (dynamicLight.m_Enable)
 				{
 					//m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.PushBack();	
// 					m_pVehicleModel->GetHeroLight()->m_Offset = Maths::HVec3(F32(g_DefaultHeroLightOffsetX), F32(g_DefaultHeroLightOffsetY), F32(g_DefaultHeroLightOffsetZ));
// 					m_pVehicleModel->GetHeroLight()->m_Colour = MathsStorage::Float3(g_DefaultHeroLightColour->r, g_DefaultHeroLightColour->g, g_DefaultHeroLightColour->b);
// 					m_pVehicleModel->GetHeroLight()->m_Radius =g_DefaultHeroLightRadius;
// 					m_pVehicleModel->GetHeroLight()->m_Flags = 1<<m_VehicleIndex;
//  					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_pNode = pNode;
//  					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Offset = Maths::HVec3((pNode->m_PartSubCode == Vehicle::E_PartSubCode_Left || pNode->m_PartSubCode == Vehicle::E_PartSubCode_FrontLeft)?(F32(g_DefaultHeroLightOffsetX)):-(F32(g_DefaultHeroLightOffsetX)), F32(g_DefaultHeroLightOffsetY), F32(g_DefaultHeroLightOffsetZ));
//  					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Colour = MathsStorage::Float3(g_DefaultHeroLightColour->r * g_DefaultHeroLightColour->a , g_DefaultHeroLightColour->g * g_DefaultHeroLightColour->a, g_DefaultHeroLightColour->b * g_DefaultHeroLightColour->a);
//  					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Radius = g_DefaultHeroLightRadius;
//  					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Flags = 1<<m_VehicleIndex;
 				}
			}

			if (pNode && (pNode->m_PartCode == Vehicle::E_PartCode_RearLight) && setup.m_pXtVehicle->m_LightFlares)
			{
				bool centreRearLight = false;
				if ((pNode->m_PartSubCode == Vehicle::E_PartSubCode_Top) || (pNode->m_PartSubCode == Vehicle::E_PartSubCode_Back) || (pNode->m_PartSubCode == Vehicle::E_PartSubCode_Centre))
				{
					centreRearLight = true;
				}
				XtHandle<DynamicEffect::Light::XtInterface::Light> xtLight = setup.m_pXtVehicle->m_LightFlares->m_RearLight;
				if (xtLight && xtLight->pDefinition && bAllowLightFlares)
				{
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.PushBack();	
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_pNode = pNode;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_XtHandle = xtLight;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_Inverted = true;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_pLightDefinition = xtLight->pDefinition;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_RenderOffset = -setup.m_pXtVehicle->m_RearLightRenderOffset;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_pDamageNode = NULL;

					//Add aux flare
					AddAuxFlare(pNode, Maths::RotY(Maths::DegToRad(180.f)), Maths::HVec3(0.f, 0.f, -0.01f), 1.0f, 0.25f, Vehicle::E_AuxFlareColour_Red);
				}
// 				if (!centreRearLight)
// 				{
// 					const Vehicle::XtDynamicLight& dynamicLight =  setup.m_pXtVehicle->m_DynamicLights.m_RearDynamicLight;
// 					if (dynamicLight.m_Enable)
// 					{
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.PushBack();	
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_pNode = pNode;
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Offset = Maths::HVec3((pNode->m_PartSubCode == Vehicle::E_PartSubCode_Left || pNode->m_PartSubCode == Vehicle::E_PartSubCode_BackLeft)?dynamicLight.m_Offset.vx:-dynamicLight.m_Offset.vx, dynamicLight.m_Offset.vy, dynamicLight.m_Offset.vz);
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Colour = MathsStorage::Float3(dynamicLight.m_Colour.r, dynamicLight.m_Colour.g, dynamicLight.m_Colour.b);
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Radius = dynamicLight.m_Radius;
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Flags = 1<<m_VehicleIndex;
// 					}
// 					else
// 					{
// 						//Add default break light
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.PushBack();	
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_pNode = pNode;
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Offset = Maths::HVec3(F32(g_DefaultRearLightOffsetX), F32(g_DefaultRearLightOffsetY), F32(g_DefaultRearLightOffsetZ));
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Colour = MathsStorage::Float3(g_DefaultRearLightColour->r * g_DefaultRearLightColour->a, g_DefaultRearLightColour->g * g_DefaultRearLightColour->a, g_DefaultRearLightColour->b * g_DefaultRearLightColour->a);
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Radius = g_DefaultRearLightRadius;
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Flags = 1<<m_VehicleIndex;
// 					}
// 				}
			}

			if (pNode && (pNode->m_PartCode == Vehicle::E_PartCode_FogLight) && setup.m_pXtVehicle->m_LightFlares)
			{
				XtHandle<DynamicEffect::Light::XtInterface::Light> xtLight = setup.m_pXtVehicle->m_LightFlares->m_FogLight;
				if (xtLight && xtLight->pDefinition && bAllowLightFlares)
				{
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.PushBack();	
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_pNode = pNode;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_XtHandle = xtLight;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_Inverted = false;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_pLightDefinition = xtLight->pDefinition;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_RenderOffset = setup.m_pXtVehicle->m_FrontLightRenderOffset;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_pDamageNode = NULL;
				}
// 				const Vehicle::XtDynamicLight& dynamicLight =  setup.m_pXtVehicle->m_DynamicLights.m_FogDynamicLight;
// 				if (dynamicLight.m_Enable)
// 				{
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.PushBack();	
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_pNode = pNode;
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Offset = Maths::HVec3((pNode->m_PartSubCode == Vehicle::E_PartSubCode_Left || pNode->m_PartSubCode == Vehicle::E_PartSubCode_FrontLeft)?dynamicLight.m_Offset.vx:-dynamicLight.m_Offset.vx, dynamicLight.m_Offset.vy, dynamicLight.m_Offset.vz);
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Colour = MathsStorage::Float3(dynamicLight.m_Colour.r, dynamicLight.m_Colour.g, dynamicLight.m_Colour.b);
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Radius = dynamicLight.m_Radius;
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Flags = 1<<m_VehicleIndex;
// 				}
			}

			if (pNode && (pNode->m_PartCode == Vehicle::E_PartCode_BrakeLight || pNode->m_PartCode == Vehicle::E_PartCode_RearAndBrakeLight) && setup.m_pXtVehicle->m_LightFlares)
			{
				//bool centreBreakLight = false;
				XtHandle<DynamicEffect::Light::XtInterface::Light> xtLight = setup.m_pXtVehicle->m_LightFlares->m_BrakeLight;
				if ((pNode->m_PartSubCode == Vehicle::E_PartSubCode_Top) || (pNode->m_PartSubCode == Vehicle::E_PartSubCode_Back) || (pNode->m_PartSubCode == Vehicle::E_PartSubCode_Centre))
				{
					//centreBreakLight = true;
					//if (setup.m_pXtVehicle->m_LightFlares->m_CentreBrakeLight.Object() != NULL)
					//{
						xtLight = setup.m_pXtVehicle->m_LightFlares->m_CentreBrakeLight;
					//}
					//else 
					//	xtLight = XtHandle<DynamicEffect::Light::XtInterface::Light>();
				}
				 
				if (xtLight && xtLight->pDefinition && bAllowLightFlares)
				{
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.PushBack();	
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_pNode = pNode;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_XtHandle = xtLight;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_Inverted = true;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_pLightDefinition = xtLight->pDefinition;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_RenderOffset = -setup.m_pXtVehicle->m_RearLightRenderOffset;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_pDamageNode = NULL;

					//Add aux flare
					AddAuxFlare(pNode, Maths::RotY(Maths::DegToRad(180.f)), Maths::HVec3(0.f, 0.f, -0.01f), 1.f, 1.f, Vehicle::E_AuxFlareColour_Red);
				}
// 				if (!centreBreakLight)
// 				{
// 					const Vehicle::XtDynamicLight& dynamicLight =  setup.m_pXtVehicle->m_DynamicLights.m_BrakeDynamicLight;
// 					if (dynamicLight.m_Enable)
// 					{
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.PushBack();	
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_pNode = pNode;
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Offset = Maths::HVec3((pNode->m_PartSubCode == Vehicle::E_PartSubCode_Left || pNode->m_PartSubCode == Vehicle::E_PartSubCode_BackLeft)?dynamicLight.m_Offset.vx:-dynamicLight.m_Offset.vx, dynamicLight.m_Offset.vy, dynamicLight.m_Offset.vz);
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Colour = MathsStorage::Float3(dynamicLight.m_Colour.r, dynamicLight.m_Colour.g, dynamicLight.m_Colour.b);
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Radius = dynamicLight.m_Radius;
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Flags = 1<<m_VehicleIndex;
// 					}
// 					else
// 					{
// 						//Add default break light
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.PushBack();	
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_pNode = pNode;
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Offset = Maths::HVec3(F32(g_DefaultBreakLightOffsetX), F32(g_DefaultBreakLightOffsetY), F32(g_DefaultBreakLightOffsetZ));
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Colour = MathsStorage::Float3(g_DefaultBreakLightColour->r * g_DefaultBreakLightColour->a, g_DefaultBreakLightColour->g * g_DefaultBreakLightColour->a, g_DefaultBreakLightColour->b * g_DefaultBreakLightColour->a);
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Radius = g_DefaultBreakLightRadius;
// 						m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Flags = 1<<m_VehicleIndex;
// 					}
// 				}
			}

			if (pNode && (pNode->m_PartCode == Vehicle::E_PartCode_ReverseLight) && setup.m_pXtVehicle->m_LightFlares)
			{
				XtHandle<DynamicEffect::Light::XtInterface::Light> xtLight = setup.m_pXtVehicle->m_LightFlares->m_ReverseLight;
				if (xtLight && xtLight->pDefinition && bAllowLightFlares)
				{
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.PushBack();	
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_pNode = pNode;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_XtHandle = xtLight;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_Inverted = true;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_pLightDefinition = xtLight->pDefinition;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_RenderOffset = -setup.m_pXtVehicle->m_RearLightRenderOffset;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_pDamageNode = NULL;

					//Add aux flare
					AddAuxFlare(pNode, Maths::RotY(Maths::DegToRad(180.f)), Maths::HVec3(0.f, 0.f, -0.01f), 1.f, 1.f, Vehicle::E_AuxFlareColour_White);
				}
// 				const Vehicle::XtDynamicLight& dynamicLight =  setup.m_pXtVehicle->m_DynamicLights.m_ReverseDynamicLight;
// 				if (dynamicLight.m_Enable)
// 				{
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.PushBack();	
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_pNode = pNode;
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Offset = Maths::HVec3((pNode->m_PartSubCode == Vehicle::E_PartSubCode_Left || pNode->m_PartSubCode == Vehicle::E_PartSubCode_BackLeft)?dynamicLight.m_Offset.vx:-dynamicLight.m_Offset.vx, dynamicLight.m_Offset.vy, dynamicLight.m_Offset.vz);
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Colour = MathsStorage::Float3(dynamicLight.m_Colour.r, dynamicLight.m_Colour.g, dynamicLight.m_Colour.b);
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Radius = dynamicLight.m_Radius;
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Flags = 1<<m_VehicleIndex;
// 				}
// 				else
// 				{
// 					//Add default break light
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.PushBack();	
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_pNode = pNode;
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Offset = Maths::HVec3(F32(g_DefaultReverseLightOffsetX), F32(g_DefaultReverseLightOffsetY), F32(g_DefaultReverseLightOffsetZ));
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Colour = MathsStorage::Float3(g_DefaultReverseLightColour->r * g_DefaultReverseLightColour->a, g_DefaultReverseLightColour->g * g_DefaultReverseLightColour->a, g_DefaultReverseLightColour->b * g_DefaultReverseLightColour->a);
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Radius = g_DefaultReverseLightRadius;
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Flags = 1<<m_VehicleIndex;
// 				}
			}
			if (pNode && (pNode->m_PartCode == Vehicle::E_PartCode_LicensePlate))
			{
				XtHandle<DynamicEffect::Light::XtInterface::Light> xtLight = setup.m_pXtVehicle->m_LightFlares->m_PlateLight;
				if (xtLight && xtLight->pDefinition && bAllowLightFlares)
				{
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.PushBack();	
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_pNode = pNode;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_XtHandle = xtLight;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_Inverted = true;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_pLightDefinition = xtLight->pDefinition;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_RenderOffset = 0.0f;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_pDamageNode = NULL;
				}
				//Add aux flare
				//AddAuxFlare(pNode, Maths::RotY(Maths::DegToRad(180.f)), Maths::HVec3(0.f, 0.f, -0.01f), 0.5f, 0.05f, Maths::C_RGBA_F32(1.f,1.f,1.f,1));
				m_pDashBoardNode = pNode;
			}
			if (pNode && (pNode->m_PartCode == Vehicle::E_PartCode_SteeringWheel))
			{
				XtHandle<DynamicEffect::Light::XtInterface::Light> xtLight = setup.m_pXtVehicle->m_LightFlares->m_DashBoardLight;
				if (xtLight && xtLight->pDefinition && bAllowLightFlares)
				{
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.PushBack();	
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_pNode = pNode;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_XtHandle = xtLight;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_Inverted = true;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_pLightDefinition = xtLight->pDefinition;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_RenderOffset = 0.0f;
					m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Back().m_pDamageNode = NULL;
				}
// 				const Vehicle::XtDynamicLight& dynamicLight =  setup.m_pXtVehicle->m_DynamicLights.m_DashBoardDynamicLight;
// 				if (dynamicLight.m_Enable)
// 				{
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.PushBack();	
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_pNode = pNode;
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Offset = Maths::HVec3(dynamicLight.m_Offset.vx, dynamicLight.m_Offset.vy, dynamicLight.m_Offset.vz);
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Colour = MathsStorage::Float3(dynamicLight.m_Colour.r, dynamicLight.m_Colour.g, dynamicLight.m_Colour.b);
// 					m_pVehicleModel->GetDynamicLights()->m_DynamicLightVector.Back().m_Radius = dynamicLight.m_Radius;
// 				}

			}
		}	
	}

#ifdef _WHEEL_LOD
	for (S32 iLod = 0; iLod < 4; iLod++)
	{
		Vehicle::C_Part* pLodNode = m_pVehicleModel->GetComponent(Vehicle::E_PartCode_WheelLod, (Vehicle::E_PartSubCode)(iLod+1));
		if (pLodNode->m_pElement)
		{
			m_LodNodes.PushBack();
			m_LodNodes.Back().m_pControllingPart = m_pVehicleModel->GetComponent(Vehicle::E_PartCode_Tyre, (Vehicle::E_PartSubCode)(iLod+1));
			m_LodNodes.Back().m_pLowLod = m_pVehicleModel->GetComponent(Vehicle::E_PartCode_WheelLod, (Vehicle::E_PartSubCode)(iLod+1));
			if (m_LodNodes.Back().m_pLowLod)
			{
				//m_LodNodes.Back().m_pLowLod->SetVisible(false, true);
				m_LodNodes.Back().m_pLowLod->Hide(true, true);
			}
			m_LodNodes.Back().m_pHighLods.PushBack(m_pVehicleModel->GetComponent(Vehicle::E_PartCode_Tyre, (Vehicle::E_PartSubCode)(iLod+1)));
			m_LodNodes.Back().m_pHighLods.PushBack(m_pVehicleModel->GetComponent(Vehicle::E_PartCode_AlloyRim, (Vehicle::E_PartSubCode)(iLod+1)));
			m_LodNodes.Back().m_pHighLods.PushBack(m_pVehicleModel->GetComponent(Vehicle::E_PartCode_Alloy, (Vehicle::E_PartSubCode)(iLod+1)));
			m_LodNodes.Back().m_pHighLods.PushBack(m_pVehicleModel->GetComponent(Vehicle::E_PartCode_AlloyBlur, (Vehicle::E_PartSubCode)(iLod+1)));
			m_LodNodes.Back().m_pHighLods.PushBack(m_pVehicleModel->GetComponent(Vehicle::E_PartCode_BrakeDisc, (Vehicle::E_PartSubCode)(iLod+1)));
			m_LodNodes.Back().m_pHighLods.PushBack(m_pVehicleModel->GetComponent(Vehicle::E_PartCode_BrakeCaliper, (Vehicle::E_PartSubCode)(iLod+1)));
		}
	}
#endif

#ifndef CFG_GOLD
	// For automation, check whether we found certain parts of the car
	{
		if ((foundCollisionMeshFlags & 1<<0) == 0)
		{
			Automation::SendProblemFoundMsg("Vehicle", m_VehicleIndex, "NoColMesh");
		}
		
		if ((foundCollisionMeshFlags & 1<<1) == 0)// && !m_tempHaveNewVehicleToVehicleMesh)
		{
			Automation::SendProblemFoundMsg("Vehicle", m_VehicleIndex, "NoDObjMesh");
		}

		if ((foundCollisionMeshFlags & 1<<2) == 0)
		{
			Automation::SendProblemFoundMsg("Vehicle", m_VehicleIndex, "NoCarMesh");
		}

		if (!foundShadowCaster)
		{
			Automation::SendProblemFoundMsg("Vehicle", m_VehicleIndex, "NoShadCast");
		}
	}
#endif

	if (m_pVehiclePhysics)
	{
		m_WheelEffects.Resize(m_pVehiclePhysics->GetNumWheels());
		for (S32 iWheel = 0; iWheel < m_pVehiclePhysics->GetNumWheels(); iWheel++)
		{
			float halfWidth = 0.15f;
			Vehicle::C_Part* pTyre = m_pVehicleModel->GetComponent(Vehicle::E_PartCode_Tyre, (Vehicle::E_PartSubCode)(Vehicle::E_PartSubCode_FrontLeft + iWheel));
			if (pTyre && pTyre->m_pElement)
				halfWidth = 0.95f * (pTyre->m_pElement->GetLocalBounds().Max().X() - pTyre->m_pElement->GetLocalBounds().Min().X()) * 0.5f;
			m_WheelEffects[iWheel].Create(iWheel, 
										&m_pVehiclePhysics->Wheel(iWheel), 
										pVehicleCreate->m_pGameWorld->GetParticleManager(), 
										GetSceneGraph(),
										pVehicleCreate->m_pGameWorld->GetTrackRenderer(),
										halfWidth);
		}

		m_ControlsEnabled = false;
		RacingPhysics::CarControls& controls = ((RacingPhysics::Car*)m_pVehiclePhysics)->Controls();
		controls.Clear();
		controls.Hold();
	}
	else
		bool breakHere = true;

	Vehicle::XtMaterialDefaults* pMaterialDefaults = NULL;
	if (C_GameVehicleData::Instance().mVehicleDefaults.Number() > 0)
		pMaterialDefaults = C_GameVehicleData::Instance().mVehicleDefaults[0]->materials;

//	CreateRadiatorSmoke();
	CreateEngineSmokeLight();
	CreateEngineSmokeHeavy();
//	CreateEngineFire();

	if (pVehicle && !pVehicle->particleEffects)
		Automation::SendProblemFoundMsg("Vehicle", m_VehicleIndex, "NoParEff");

	m_pVehicleModel->GetComponentList().Initialise();

	m_pVehicleModel->Initialise(pXtVehicle);

	for (S32 iBlur = 0; iBlur < m_pVehicleModel->m_WheelBlur.Size(); iBlur++)
	{
		m_pVehicleModel->m_WheelBlur[iBlur].m_SolidPass = E_RenderPass_VehicleSolid;
		m_pVehicleModel->m_WheelBlur[iBlur].m_AlphaPass = E_RenderPass_VehicleAlpha;
	}

	// respray override
	if ( pVehicleCreate->m_XtResprayJobId && UI::C_UiXtConfig::Instance() )
	{
		Vehicle::XtMaterialDefaults resprayDefaults;
		resprayDefaults.m_DefaultPaintJob = pMaterialDefaults->m_DefaultPaintJob;

		XtHandle< Vehicle::XtPaintJob > resprayOverride = UI::C_UiXtConfig::Instance()->m_GarageResprays.Find( pVehicleCreate->m_XtResprayJobId );

		if ( resprayOverride )
			resprayDefaults.m_DefaultPaintJob = resprayOverride;

		m_pVehicleModel->GetPaintJob()->Create(NULL, &resprayDefaults, 0, m_VehicleIndex);
	}
	else
	{
		m_pVehicleModel->GetPaintJob()->Create(pXtVehicle, pMaterialDefaults, pVehicleCreate->m_XtPaintJobId, m_VehicleIndex);
	}

	if ((pVehicleCreate->m_ResprayColour >> 24) & 0xff)
		m_pVehicleModel->GetPaintJob()->Respray(pVehicleCreate->m_ResprayColour);

	if (m_pVehicleModel->GetDamageLayer())
	{		
		m_pVehicleModel->GetDamageLayer()->SetupDamagedParts(	pXtVehicle, 
																m_pCollisionBody, GetPhysicsScene(), m_Matrix);
		m_pVehicleModel->GetDamageLayer()->SetPhysicsScene(GetPhysicsScene());
	}

	m_pVehicleModel->SetStaticShaderParameters();

	if (Vehicle::C_VehicleModelManager::Instance().m_DEBUG_RegisterCarModelType)
		m_pVehicleModel->SetupDynamicMaterials(pXtVehicle, pMaterialDefaults, m_pVehicleModelResource);

	if (Vehicle::C_VehicleModelManager::Instance().m_DEBUG_RegisterWheelModelType)
		m_pVehicleModel->SetupDynamicMaterialsWheel(pXtVehicle, pMaterialDefaults);

	if (Vehicle::C_VehicleModelManager::Instance().m_DEBUG_RegisterCarModelType || Vehicle::C_VehicleModelManager::Instance().m_DEBUG_RegisterWheelModelType)
		m_pVehicleModel->GetComponentList().BuildDynamicMaterials();

	//REMOVE_DIRT
	//SetupDirtLayer();
	
	for (S32 iFlare = 0; iFlare < m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Size(); iFlare++)
	{
		Vehicle::C_Flare& flare = m_pVehicleModel->GetLightFlares()->m_LightFlareVector[iFlare];
		//flare.m_pDamageNode = m_pVehicleModel->GetDamageLayer()->GetClosestNode(flare.m_pNode->m_pElement->GetLocalBounds().Centre());		
		if (m_pVehicleModel->GetDamageLayer())
			flare.m_pDamageNode = m_pVehicleModel->GetDamageLayer()->GetClosestNode(flare.m_pNode->GetCentre());		
		else
			flare.m_pDamageNode = NULL;
	}
	if (pXtVehicle->m_Headlight)
	{
		m_pVehicleModel->GetHeadlight()->m_ZOffset = pXtVehicle->m_Headlight->m_ZOffset;
		m_pVehicleModel->GetHeadlight()->m_Width = pXtVehicle->m_Headlight->m_Width;
		m_pVehicleModel->GetHeadlight()->m_Length = pXtVehicle->m_Headlight->m_Length;
		m_pVehicleModel->GetHeadlight()->m_Height = pXtVehicle->m_Headlight->m_Height;
		m_pVehicleModel->GetHeadlight()->m_Intensity = pXtVehicle->m_Headlight->m_Intensity;
		m_pVehicleModel->GetHeadlight()->m_Colour = Maths::C_RGBA_F32(pXtVehicle->m_Headlight->m_Colour.r, pXtVehicle->m_Headlight->m_Colour.g, pXtVehicle->m_Headlight->m_Colour.b, pXtVehicle->m_Headlight->m_Intensity);
	}

#ifndef CFG_GOLD
	if(g_RoadsweeperPickups)
	{
		C_Game::GetInstance()->GetMultiplayerAbilities().ActivateBossSlot(UI::E_MultiplayerVehicleAbilities_Road_Sweeper, levelState.mCars[m_VehicleIndex]);
	}
#endif

	//Trigger stuff
	if (pVehicleCreate->m_pGameWorld)
	{
		U8 data[1024 * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> stack(data, 1024);				
		if (pVehicleCreate->m_pGameWorld->GetTriggerLayer(Game::E_TriggerLayer_Pickup))
		{
			m_pPickupTriggerActor = new Gravity::C_TriggerActor;
			Collision::C_CollisionFilter filter;
			Maths::HVec3 extents = m_pCollisionBody->GetLocalBoundingBox().extents;
			extents.X(extents.X()*CheckRoadSweeperMultiplier(levelState)); //road sweeper mod means you can pick up pickups "nearby" you
			m_pPickupTriggerActor->m_pShape = new Gravity::C_TriggerBox(extents, &filter);//m_BodyCollisionFilter);
			m_pPickupTriggerActor->SetUserData((void*)&m_UserData);
			pVehicleCreate->m_pGameWorld->GetTriggerLayer(Game::E_TriggerLayer_Pickup)->AddTriggerActor(m_pPickupTriggerActor);
			m_pPickupTriggerActor->SetMatrix(m_Matrix, stack);

		}
		if (pVehicleCreate->m_pGameWorld->GetTriggerLayer(Game::E_TriggerLayer_TriggerRegion))
		{
			m_pTriggerRegionTriggerActor = new Gravity::C_TriggerActor;
			Collision::C_CollisionFilter filter;
			m_pTriggerRegionTriggerActor->m_pShape = new Gravity::C_TriggerBox(m_pCollisionBody->GetLocalBoundingBox().extents, &filter);//m_BodyCollisionFilter);
			m_pTriggerRegionTriggerActor->SetUserData((void*)&m_UserData);
			pVehicleCreate->m_pGameWorld->GetTriggerLayer(Game::E_TriggerLayer_TriggerRegion)->AddTriggerActor(m_pTriggerRegionTriggerActor);
			m_pTriggerRegionTriggerActor->SetMatrix(m_Matrix, stack);
		}
	}

	m_IsSplitscreen = (pVehicleCreate->m_pGameWorld->GetViewManager()->GetNumViews() != 1);

	m_pPhysicsChecks = new AmaxCam::C_PhysicsChecks(GetPhysicsScene(), m_PhysicsGroupId);

	//Add exhaust effect emitters
	for (S32 iPos = 0; iPos < pVehicle->exhaustEmitters.count; iPos++)
	{
		Maths::HVec3 pos(pVehicle->exhaustEmitters[iPos].position.vx, pVehicle->exhaustEmitters[iPos].position.vy, pVehicle->exhaustEmitters[iPos].position.vz);
		Maths::HVec3 dir(pVehicle->exhaustEmitters[iPos].direction.vx, pVehicle->exhaustEmitters[iPos].direction.vy, pVehicle->exhaustEmitters[iPos].direction.vz);
		dir.Normalise();
		m_ExhaustEffects.AddEmitPosition(pVehicle->exhaustEmitters[iPos].exhaust, pos, dir);
	}
	m_ExhaustEffects.Initialise(&m_LocalSpace);

	if (pVehicle && pVehicle->exhaustEmitters.count == 0)
		Automation::SendProblemFoundMsg("Vehicle", m_VehicleIndex, "NoExEmit");

	m_GroundPosition = Maths::HVec3(0.0f, 0.0f, 0.0f);
	m_GroundNormal = Maths::C_Normal(0.0f, 1.0f, 0.0f);

	if (pShadowCasterModel == NULL)
	{
		//Create a shadow from the colision mesh
		FindShadowMesh();

		//If shadows
		if (m_ShadowVertexCount != 0)
		{
			m_pBlobShadowInstance = pVehicleCreate->m_pGameWorld->GetBlobShadows()->CreateInstance(&m_ShadowVertexBuffer, m_ShadowVertexCount, &m_ShadowIndexBuffer, m_ShadowIndexCount);
			LOGWARN(g_VehicleActorLog, "Using physic model as shadow caster in vehicle %s", m_ModelFileName.Chars());
		}
	}
	else
	{
		//Create a shadow from the model
		//MEMORY_TRACKER_PUSH("graphics.vehicle.actor.modelinstances");
		WorldRender::C_ModelInstance* pShadowModelInstance = new WorldRender::C_ModelInstance(m_pVehicleModelResource->GetRenderModelData(), pShadowCasterModel->GetIndex(), GetSceneGraph());
		if (pShadowModelInstance->GetRenderListNode() != NULL)
		{
			pShadowModelInstance->Build();
		}
		else
		{
			LOGERROR(g_VehicleActorLog, "Found Shadow Mesh but it doesn't have a RenderListNode for '%s'", m_ModelFileName.Chars());
			Automation::SendProblemFoundMsg("Vehicle", m_VehicleIndex, "NoShadCast");
		}
		//MEMORY_TRACKER_POP("graphics.vehicle.actor.modelinstances");
		//m_pVehicleModel->AddModelInstance(pShadowModelInstance, "m_shadowCaster");

		//Create blob shadow instance
		m_pBlobShadowInstance = pVehicleCreate->m_pGameWorld->GetBlobShadows()->CreateInstance(pShadowModelInstance);

		//Add to scenegraph and model instance to render the reflection
		//pShadowModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OPAQUE, E_RenderPass_);
		//pShadowModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OVERLAY, E_RenderPass_Solid);
		//pShadowModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_ALPHA_TEST, E_RenderPass_Solid);
		//pShadowModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_POST_RESOLVE, E_RenderPass_Solid);
		//pShadowModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_BACK_FRONT, E_RenderPass_Solid);

		SceneGraph::Insert(GetSceneGraph(), pShadowModelInstance);
		m_ModelInstances.PushBack(pShadowModelInstance);

		//Maths::C_Sphere sphere = pShadowCasterModel->GetLocalBounds().BoundingSphere();

		Maths::C_Sphere sphere = pShadowCasterModel->GetBoundingSphere();
		if (pShadowModelInstance->GetRenderListNode() && pShadowModelInstance->GetCullNode())
		{	
			pShadowModelInstance->DisableVisibilityFlags(E_ViewTypeFlag_CubeMap | E_ViewTypeFlag_Main | E_ViewTypeFlag_Mirror);
			if (!C_Game::GetInstance()->GetLevelState().mInUI)
				pShadowModelInstance->EnableVisibilityFlags(E_ViewTypeFlag_Reflection);
			else
				pShadowModelInstance->DisableVisibilityFlags(E_ViewTypeFlag_Reflection);

			pShadowModelInstance->SetVisDetect(m_pReflectionVisDetect);
			pShadowModelInstance->SetWorldSphere(sphere);
			SetDrawDistance(pShadowModelInstance, pShadowCasterModel->GetName());
			m_pVehicleModel->AddModelInstance(pShadowModelInstance, pShadowCasterModel->GetName());
			
		}
	}

	{
		SceneGraph::C_SceneGraph* pSceneGraph = GetSceneGraph();
		m_HeadlightHandle = SceneGraph::AddSpotLight(pSceneGraph);
		if (m_HeadlightHandle.Object())
			m_HeadlightHandle->AllocatePerViewIntensities(E_ViewType_Count); //Allocate per view intensities
		m_HeroLightHandle = SceneGraph::AddPointLight(pSceneGraph);
		if (m_HeroLightHandle.Object())
			m_HeroLightHandle->AllocatePerViewIntensities(E_ViewType_Count); //Allocate per view intensities
#ifdef PLATFORM_PS3		
		if (!m_IsSplitscreen)
#endif
			m_RearLightHandle = SceneGraph::AddPointLight(pSceneGraph);
	}

	m_CurrentLightCubeMapIndex = 0;
	//Get Light Cubes from GameWorld
	m_LightCubeMapTexture[0] = pVehicleCreate->m_pGameWorld->GetVehicleLightCubeMap(m_VehicleIndex, 0);
	m_LightCubeMapTexture[1] = pVehicleCreate->m_pGameWorld->GetVehicleLightCubeMap(m_VehicleIndex, 1);

	
	//Set first cubemap
	m_pVehicleModel->SetLightEnvironmentTexture(m_LightCubeMapTexture[0]);
	
	if (m_pVehiclePhysics)
	{
		float fAngle = Maths::ArcTan(m_fMaxWheelTravelFront / m_pVehiclePhysics->Dynamics().geometry.wheelBase);
		m_fRearAxlePivotAngle = fAngle * (pVehicle->rearAxlePivotScale == 0.0f ? 1.0f : pVehicle->rearAxlePivotScale);
	}

	m_VisibleHealthDamageEnabled					= true;
	m_HealthPoints = 0;
//	m_RadiatorSmokeEmitter.m_Intensity		= 0.0f;
	m_EngineSmokeLightEmitter.m_Intensity	= 0.0f;
	m_EngineSmokeHeavyEmitter.m_Intensity	= 0.0f;
//	m_EngineFireEmitter.m_Intensity			= 0.0f;
	m_FatallyDamaged = false;	

	if (pVehicleCreate->m_Dormant)
	{
		SetState(false);
		SetCollisions(false);
		SetVisibility(false);
	}

	if (pXtVehicle && pXtVehicle->m_FunctionalWindow.m_Part)
	{
		Vehicle::C_Part* pWindow = m_pVehicleModel->GetComponent(pXtVehicle->m_FunctionalWindow.m_Part->m_PartCode, pXtVehicle->m_FunctionalWindow.m_Part->m_PartSubCode);
		if (pWindow && pWindow->m_pModelInstance)
			m_Window.Initialise(&pXtVehicle->m_FunctionalWindow, pWindow);
	}

	m_KillPlaneHeight = g_debugVehicleKILLPLANE;
	AmaxRoute *pRoute  = C_GameData::Instance().mRoutes[(GAMEDATA_ID)C_GameApp::GetInstance()->GetGame()->GetLevelState().mRoute];
	if (pRoute)
	{
		if (pRoute->minYPosition != 0)
			m_KillPlaneHeight = pRoute->minYPosition;
	}

	if (m_pVehiclePhysics && C_Game::GetInstance()->GetLevelState().mInUI)
		((RacingPhysics::Car*)m_pVehiclePhysics)->Controls().steer = (float)g_debugGarageSteerAngle;

#if 1 //Spawn() vs. Create()
	AddSubscribers();
#endif

	//Create Light Flares (visibility false)
	if (!m_IsSplitscreen)
	{
		for (int i = 0; i < m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Size(); i++)
		{
			Vehicle::C_Flare& flare = m_pVehicleModel->GetLightFlares()->m_LightFlareVector[i];
			//Create flare
			if (flare.m_XtHandle)
			{
				flare.m_Handle = m_pLightEffectManager->CreateLight(flare.m_XtHandle->pDefinition, Maths::C_QuatTrans(), &flare.m_Node, Maths::RGBA(255, 255, 255, 255), 0.0f, 0.f);
				if (!flare.m_Handle)
					continue;
				flare.m_Handle->m_VisibilityFlags = 0;
			}
		}
	}

	//Chad WiP - collision stuff
	if (g_debugEnableDeformableCollision)
		GetVehicleModel()->GetDamageLayer()->MapCollisionVertices(m_pCollisionBody);

#if PLATFORM_PS3
	GetVehicleModel()->GetDamageLayer()->InitSPUDamage(GetVehicleModel()->GetComponentList().m_ControlPointHandler.m_Data);
#endif

MEMORY_TRACKER_POP("graphics.vehicle.actor");

	SetWheelParticleLod(true, true, true, true, 1.0f, 1.0f, 1.0f, 1.0f);

	m_EmpState.m_fEmpMaxTime	= 0.0f;
	m_EmpState.m_fEmpTime		= 0.0f;
	m_EmpState.m_fBrake			= 0.0f;
	m_EmpState.m_bEmp			= false;
	m_EmpState.m_bRearLights	= false;
	m_EmpState.m_bHeadLights	= false;
	m_EmpState.m_bFogLights		= false;

	m_bCatchingAir				= false;
	m_bInAirDownForceEngaged	= false;
	m_fGroundedDownForceTime	= 0.0f;
	m_fAirDownForceTime			= 0.0f;
	m_CanPhysicsIgnoreCollision	= true;
	m_CollisionThisFrame		= false;
	m_FramesSinceCollision		= 0;

	m_fLostGripTime				= 0.0f;
	m_bLostGrip					= false;
	m_fCantLoseGripTime			= 0.0f;
	m_bCanLoseGrip				= true;

	m_Created					= true;
	m_Enabled					= true;
	return true;
}

/*
===============
NAME: AddSubscribers

NOTE: Subscribe to messages
===============
*/
void C_VehicleActor::AddSubscribers()
{
	//AddSubscriber( GameMessaging::E_GameMsg_PreUpdate );
	//AddSubscriber( GameMessaging::E_GameMsg_Update );	//CA - Just used to get the frame time for the pickup effects...is there a better way?!?
	AddSubscriber( ActorMsg::E_GameActorMsg_Update2ndPass );
	
	AddSubscriber( GameMessaging::E_GameMsg_PausedUpdate );
	
	AddSubscriber( Game::E_VehicleMessage_UpdatePlayerControls );
	AddSubscriber( Game::E_VehicleMessage_UpdateAIControls );
	AddSubscriber( Game::E_VehicleMessage_UpdateNetworkControls );
	AddSubscriber( Game::E_VehicleMessage_UpdatePerformanceModifiers );

	AddSubscriber( Game::E_VehicleMessage_Eliminated );

	AddSubscriber( E_VehicleLogicMsg_HitByPerk);
	AddSubscriber( Game::E_VehicleMessage_ForceAIControl );
	AddSubscriber( Game::E_VehicleMessage_ForceHumanControl );

	AddSubscriber( Game::E_VehicleMessage_DebugRender );

	AddSubscriber( Game::E_VehicleMessage_ResetAtSpawn );
	
	AddSubscriber( Game::E_VehicleMessage_ResetAtWorldPosition );
	AddSubscriber( Game::E_VehicleMessage_ResetAtCameraPosition );

	AddSubscriber( Game::E_VehicleMessage_ControlsEnable );
	AddSubscriber( Game::E_VehicleMessage_ControlsDisable );

	AddSubscriber( Game::E_VehicleMessage_FatallyDamaged );
	AddSubscriber( Game::E_VehicleMessage_ResetFatalDamage );
	AddSubscriber( Game::E_VehicleMessage_RepairDamage );
	AddSubscriber( Game::E_VehicleMessage_RaceStarted );
	AddSubscriber( ActorMsg::E_CutSceneMsg_Start321 );

	AddSubscriber( Game::E_VehicleMessage_ExhaustPopEvent );
	AddSubscriber( Game::E_VehicleMessage_LeftRace );

	AddSubscriber( Game::E_VehicleMessage_ApplyShockedEffect );

	AddSubscriber( Game::E_VehicleMessage_Freeze );
	AddSubscriber( Game::E_VehicleMessage_LoseGrip );
	AddSubscriber( Game::E_VehicleMessage_DontLoseGrip );		 
		
	AddSubscriber( Game::E_ReplayMessage_UpdateVehicleState );
	AddSubscriber( Game::E_ReplayMessage_UpdateControls );
	AddSubscriber( Game::E_ReplayMessage_SetVehicleDamage );
	AddSubscriber( Game::E_GameLogicMessage_Impact );

	AddSubscriber( Game::E_VehicleMessage_StopWreckedEffect );
	AddSubscriber( Game::E_VehicleMessage_KillNitro );

	AddSubscriber( Game::E_VehicleMessage_ApplyDebugDamage );	

	AddSubscriber( Game::E_VehicleMessage_Respray );

	AddSubscriber( Game::E_GameflowMessage_RaceLoaded );

	AddSubscriber( Game::E_GameflowMessage_RaceStarted );

	AddSubscriber( Game::E_VehicleMessage_SetVisibility );
}

/*
===============
NAME: CreateBones

NOTE: Create bone
===============
*/
void C_VehicleActor::CreateBones(const GameResource::C_Model* p_pModel)
{
	Util::C_String modelName = Util::C_String(p_pModel->GetName()).Lower();

	Ut::Vector<Util::C_String> strings;

	modelName.Tokenise(strings, '_');

	//if (strings.Size() > 3)
	if (strings.Size() == 3)
		modelName = strings[0] + "_" + strings[1];
	else if (strings.Size() > 3)
		modelName = strings[0] + "_" + strings[1] + "_" + strings[2];

	Vehicle::E_PartCode ePartCode = Vehicle::E_PartCode_None;
	Vehicle::E_PartSubCode ePartSubCode = Vehicle::E_PartSubCode_None;
	Vehicle::C_CodeProcessor::Instance().GetCodes(modelName, ePartCode, ePartSubCode);

	if (ePartCode == Vehicle::E_PartCode_None)
		return;

	Vehicle::C_Part* pNode = m_pVehicleModel->GetComponent(ePartCode, ePartSubCode);

	if (!pNode)
		return;

	m_pVehicleModel->GetComponentList().m_ControlPointHandler.AddControlPoint(Util::C_String(p_pModel->GetName()).Lower(), p_pModel->GetMatrix().AW(), pNode);
}

/*
===============
NAME: CreateWheelInstance

NOTE: Create a wheel instance
===============
*/
void C_VehicleActor::CreateWheelInstance(const GameResource::C_ModelElement* p_pModelElement)
{
	if (!m_pWheelModelResource)
		return;

	Util::C_String modelName = p_pModelElement->GetName();
	modelName.ConvertToLower();

	Vehicle::E_PartCode eWheelPartCode		= Vehicle::E_PartCode_None;
	Vehicle::E_PartSubCode eWheelLocCode	= Vehicle::E_PartSubCode_None;

	//Get part and location codes for the wheel instance
	Vehicle::C_CodeProcessor::Instance().GetCodes(modelName, eWheelPartCode, eWheelLocCode);

	const GameResource::C_Model* pSpokesModel = m_pWheelModelResource->GetModelData()->GetModel("m_alloy");
	if (!pSpokesModel)
		return;

	//Create a new model instance for the spokes
	SceneGraph::C_SceneGraph* pSceneGraph = GetSceneGraph();

	//Spokes
	WorldRender::C_ModelInstance* pSpokesModelInstance = new WorldRender::C_ModelInstance(m_pWheelModelResource->GetRenderModelData(), pSpokesModel->GetIndex(), pSceneGraph);

	pSpokesModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OPAQUE, E_RenderPass_VehicleAlpha);
	//pSpokesModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_BACK_FRONT, E_RenderPass_VehicleAlpha);
	if (!C_Game::GetInstance()->GetLevelState().mInUI)
		pSpokesModelInstance->DisableVisibilityFlags(Game::E_ViewTypeFlag_CubeMap | Game::E_ViewTypeFlag_Reflection | Game::E_ViewTypeFlag_Mirror);
	else
		pSpokesModelInstance->DisableVisibilityFlags(Game::E_ViewTypeFlag_CubeMap | Game::E_ViewTypeFlag_Mirror);
	//pSpokesModelInstance->Build();

	SceneGraph::Insert(pSceneGraph, pSpokesModelInstance);
	m_ModelInstances.PushBack(pSpokesModelInstance);
	bool added = false;
	if (pSpokesModelInstance->GetRenderListNode() && pSpokesModelInstance->GetCullNode())
	{
		//Wheel index
		S32 wheelIndex = -1;
		switch (eWheelLocCode)
		{
		case Vehicle::E_PartSubCode_FrontLeft:
			wheelIndex = 0;
			break;
		case Vehicle::E_PartSubCode_FrontRight:
			wheelIndex = 1;
			break;
		case Vehicle::E_PartSubCode_BackLeft:
			wheelIndex = 2;
			break;
		case Vehicle::E_PartSubCode_BackRight:
			wheelIndex = 3;
			break;
		}
		m_pVehicleModel->AddModelInstance(pSpokesModelInstance, pSpokesModel->GetName(), wheelIndex);
		added = true;		
	}
	if (!added)
	{
		Util::C_String FFS = pSpokesModel->GetName();
		LOGERROR(g_VehicleActorLog, "::CreateWheelInstance(element) - ModelInstance created but no added, so deleting: '%s'", FFS.Chars());
		SAFE_DELETE_PTR(pSpokesModelInstance);
		return;
	}

	float fScale = 1.0f;
	float fOffset = 0.0f;
	AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];
	//Get transformation matrix for the instanced wheel
	Maths::HMat43 wheelMatrix = pSpokesModel->GetMatrix();
	Maths::HVec3 vScale(fScale, fScale, fScale);
	Maths::HVec3 vOffset(fOffset, 0.0f, 0.0f);
	if (eWheelLocCode == Vehicle::E_PartSubCode_FrontLeft || eWheelLocCode == Vehicle::E_PartSubCode_FrontRight)
	{
		if (pVehicle)
		{
			fScale = pVehicle->wheelScaleFront;	
			fOffset = pVehicle->wheelDepthFront;
			if (eWheelLocCode == Vehicle::E_PartSubCode_FrontLeft)
				fOffset *= -1.0f;
		}
	}
	else if (eWheelLocCode == Vehicle::E_PartSubCode_BackLeft || eWheelLocCode == Vehicle::E_PartSubCode_BackRight)
	{
		if (pVehicle)
		{
			fScale = pVehicle->wheelScaleRear;	
			fOffset = pVehicle->wheelDepthRear;
			if (eWheelLocCode == Vehicle::E_PartSubCode_BackLeft)
				fOffset *= -1.0f;
		}
	}
	vScale = Maths::HVec3(fScale, fScale, fScale);

	Maths::HMat33 rotMat;
	rotMat.SetIdentity();
	Maths::HVec3 vTrans(wheelMatrix.AW().X() + fOffset, wheelMatrix.AW().Y(), wheelMatrix.AW().Z());
	if (eWheelLocCode == Vehicle::E_PartSubCode_BackRight || eWheelLocCode == Vehicle::E_PartSubCode_FrontRight)
	{		
		rotMat = Maths::HMat33(Maths::RotY(Maths::DegToRad(180.0f)));
		vTrans = Maths::HVec3(vTrans.X() + pSpokesModel->GetMatrix().AW().X() * -2.0f, vTrans.Y(), vTrans.Z());
	}
	//Maths::HVec3 vTrans(wheelMatrix.AW().X() + fOffset, wheelMatrix.AW().Y(), wheelMatrix.AW().Z());
	wheelMatrix.SetScaleRotTrans(vScale, rotMat, vTrans);

	/*Maths::C_Sphere sphere = pSpokesModel->GetLocalBounds().BoundingSphere();*/
	Maths::C_Sphere sphere = pSpokesModel->GetBoundingSphere();
	pSpokesModelInstance->SetWorldSphere(Maths::C_Sphere(sphere.Centre(), sphere.Radius() * fScale));
	SetDrawDistance(pSpokesModelInstance, pSpokesModel->GetName());
	
	Vehicle::C_Part* pUnblurredNode = NULL;
	Vehicle::C_Part* pBlurredNode = NULL;

	for (S32 iElement = 0; iElement < pSpokesModel->GetElementCount(); iElement++)
	{
		const GameResource::C_ModelElement* pModelElement = pSpokesModel->GetElement(iElement);
		pSpokesModelInstance->SetElementMatrix(iElement, pModelElement->GetMatrix() * wheelMatrix * m_Matrix );

		Util::C_String elementName = pModelElement->GetName();
		elementName.ConvertToLower();

		Vehicle::E_PartCode eElementPartCode	= Vehicle::E_PartCode_None;
		Vehicle::E_PartSubCode eElementLocCode	= Vehicle::E_PartSubCode_None;

		//Get part and location codes for the wheel instance
		Vehicle::C_CodeProcessor::Instance().GetCodes(elementName, eElementPartCode, eElementLocCode);

		Vehicle::C_Part* pNode = m_pVehicleModel->SetComponent(pSpokesModel, pModelElement, m_pWheelModelResource, pSpokesModelInstance, NULL, GetPhysicsScene(), &wheelMatrix, eElementPartCode, eWheelLocCode);

		if (eElementPartCode == Vehicle::E_PartCode_AlloyBlur)
			pBlurredNode = pNode;
		else if (eElementPartCode == Vehicle::E_PartCode_Alloy)
			pUnblurredNode = pNode;
	}

//#ifdef _WHEEL_LOD
//	//Lod
//	const GameResource::C_Model* pLodModel = m_pWheelLodModelResource->GetModelData()->GetModel("m_wheellod");
//	if (pLodModel)
//	{
//		WorldRender::C_ModelInstance* pLodModelInstance = new WorldRender::C_ModelInstance(m_pWheelLodModelResource->GetRenderModelData(), 1, pSceneGraph);
//		SceneGraph::Insert(pSceneGraph, pLodModelInstance);
//		m_pVehicleModel->AddModelInstance(pLodModelInstance, pLodModel->GetName());
//
//		sphere = pLodModel->GetLocalBounds().BoundingSphere();
//		pLodModelInstance->SetWorldSphere(sphere);
//		SetDrawDistance(pLodModelInstance, pLodModel->GetName());
//
//		for (S32 iElement = 0; iElement < pLodModel->GetElementCount(); iElement++)
//		{
//			const GameResource::C_ModelElement* pModelElement = pLodModel->GetElement(iElement);
//			pLodModelInstance->SetElementMatrix(iElement, pModelElement->GetMatrix() * wheelMatrix * m_Matrix );
//
//			Util::C_String elementName = pModelElement->GetName();
//			elementName.ConvertToLower();
//
//			Vehicle::E_PartCode eElementPartCode	= Vehicle::E_PartCode_None;
//			Vehicle::E_PartSubCode eElementLocCode	= Vehicle::E_PartSubCode_None;
//
//			//Get part and location codes for the wheel instance
//			Vehicle::C_CodeProcessor::Instance().GetCodes(elementName, eElementPartCode, eElementLocCode);
//
//			Vehicle::C_Part* pNode = m_pVehicleModel->SetComponent(pLodModel, pModelElement, m_pWheelLodModelResource, pLodModelInstance, NULL, GetPhysicsScene(), &wheelMatrix, eElementPartCode, eWheelLocCode);
//		}
//	}
//#endif

	if (pUnblurredNode && pBlurredNode)		
		m_pVehicleModel->AddWheelBlur(pUnblurredNode, pBlurredNode);
}

Debug::Bool g_debugEnableNewVehicleToVehicleCollisionMesh("Vehicle.Collisions.EnableNewVehicleToVehicleCollisionMesh", true);

/*
===============
NAME: CreateCollisionMesh

NOTE: Create vehicle physics body
===============
*/

int C_VehicleActor::CreateCollisionMesh(const GameResource::C_Model* p_pPhysicsModel, const LevelStateCar& levelStateCar)
{
	int foundFlags = 0;

	m_Centre = p_pPhysicsModel->GetLocalBounds().Centre();

	const C_VehicleActorCreate *pVehicleCreate = (const C_VehicleActorCreate *)(GetCreationData().GetData());

	m_pVehiclePhysics = new RacingPhysics::Car();

	RacingPhysics::VehicleInitialiseInfo info;
	info.filename = m_DynamicsFileName.Chars();
	info.autoGears = true;
	
	switch(pVehicleCreate->m_ControlType)
	{
	case E_VEHICLE_CONTROL_TYPE_PLAYER:
		{
			info.controlType = RacingPhysics::Vehicle::CONTROL_TYPE_PLAYER;
			ForcePlayerControl(false);
			break;
		}
	case E_VEHICLE_CONTROL_TYPE_AI:
		{
			info.controlType = RacingPhysics::Vehicle::CONTROL_TYPE_AI;			
			ForceAIControl(false);
			break;
		}
	case E_VEHICLE_CONTROL_TYPE_NETWORK:
		{
			info.controlType = RacingPhysics::Vehicle::CONTROL_TYPE_NETWORK;			
			ForcePlayerControl(false);
			break;
		}
	case E_VEHICLE_CONTROL_TYPE_SHOWROOM:
		{
			info.controlType = RacingPhysics::Vehicle::CONTROL_TYPE_SHOWROOM;			
			ForcePlayerControl(false);
			break;
		}
	case E_VEHICLE_CONTROL_TYPE_REPLAY:
		{
			info.controlType = RacingPhysics::Vehicle::CONTROL_TYPE_REPLAY;			
			ForcePlayerControl(false);
			break;
		}
	default:
		{
			BIZ_PANIC_FAIL("C_VehicleActor::CreateCollisionMesh() > Unknown control type...we can't really continue");
			break;
		}
	}
	
	m_pVehiclePhysics->Initialise(info);

	Collision::C_CollisionFilter worldFilter(PhysicsConstants::kVehicleBodyCollisionFilter);
	Collision::C_CollisionFilter objectFilter(PhysicsConstants::kVehicleBlockCollisionFilter);
	Collision::C_CollisionFilter vehicleFilter(PhysicsConstants::kVehicleVehicleCollisionFilter);
	worldFilter.SetGroupId(m_PhysicsGroupId);
	worldFilter.SetGroupFlags(0);
	worldFilter.SetGroupMask(worldFilter.GetCollisionMask());
	m_ObjectColliderIndex = -1;
	m_CarColliderIndex = -1;
	m_MainElementIndex = -1;
	
	if(m_pCollisionBody == NULL)
	{
		Gravity::C_ImportShape* pMainImportShape = NULL;

		
		Gravity::C_CompositeDesc compDesc;

		if (g_debugEnableNewVehicleToVehicleCollisionMesh)
		{
			for (S32 iElement = 0; iElement < p_pPhysicsModel->GetElementCount(); iElement++)
			{
				const GameResource::C_ModelElement* pElement = p_pPhysicsModel->GetElement(iElement);

				Util::C_String elementName = pElement->GetName();
				elementName.ConvertToLower();

				if (elementName == "e_carcollider")
					m_tempHaveNewVehicleToVehicleMesh = true;
			}
		}
		for (S32 iElement = 0; iElement < p_pPhysicsModel->GetElementCount(); iElement++)
		{
			const GameResource::C_ModelElement* pElement = p_pPhysicsModel->GetElement(iElement);

			Util::C_String elementName = pElement->GetName();
			elementName.ConvertToLower();
			
			Gravity::C_ImportShape* pImportShape = const_cast<Gravity::C_ImportShape*>( pElement->GetPhysicsImportShape() );

			if (!pImportShape)
				continue;

			if (!pImportShape->GetShapeDesc()->m_RenderOffset.Finite())
			{
				LOGERROR(g_VehicleActorLog, "TEMP HACK FIX: pImportShape->GetShapeDesc()->m_RenderOffset is INFINITE!!!");
				pImportShape->GetShapeDesc()->m_RenderOffset = Maths::Constants<Maths::HVec3>::Zero();
			}
			if (!pImportShape->GetImportIneriaTensor().Finite())
			{
				LOGERROR(g_VehicleActorLog, "TEMP HACK FIX: pImportShape->GetImportIneriaTensor() is INFINITE!!!");
				pImportShape->SetImportIneria(Maths::Constants<Maths::HMat33>::Identity());
			}
			if (!Maths::HF32(pImportShape->GetShapeDesc()->m_Mass).Finite())
			{
				LOGERROR(g_VehicleActorLog, "TEMP HACK FIX: pImportShape->GetShapeDesc()->m_Mass is INFINITE!!!");
				pImportShape->GetShapeDesc()->m_Mass = Maths::Constants<Maths::HF32>::One();
			}

			if (elementName == "m_collisionmesh")
			{
				if (pImportShape)
				{
					pMainImportShape = pImportShape;

					worldFilter.SetGroupFlags(pImportShape->GetShapeDesc()->GetCollisionFlags());
					pImportShape->GetShapeDesc()->SetPhysicsMaterial(Game::E_PhysicsMaterial_Vehicle);
					compDesc.AddShape(pImportShape->GetShapeDesc(), Maths::HMat43(pImportShape->GetImportRotation(),
						pImportShape->GetImportPosition()), 1.f, &worldFilter);
					
					if (m_tempHaveNewVehicleToVehicleMesh)
						pImportShape->GetShapeDesc()->m_CollisionFlag &= ~(Game::PolyFlags::kVehicleSolid | Game::PolyFlags::kDynamicObjectSolid);
					//else
					//	worldFilter.m_CollisionFlags &= Game::PolyFlags::kVehicleBlocker;

					if (pImportShape->GetShapeDesc()->ValidShapeDescriptor())
					{
						foundFlags |= 1<<0;
						m_MainElementIndex = iElement;
					}
				}
			} 
			//else if (elementName == "e_objectcollider")// && !m_tempHaveNewVehicleToVehicleMesh)
			//{
			//	if (pImportShape)
			//	{
			//		objectFilter.m_GroupFlags = pImportShape->GetShapeDesc()->GetCollisionFlags();
			//		pImportShape->GetShapeDesc()->SetPhysicsMaterial(Game::E_PhysicsMaterial_Vehicle);
			//		compDesc.AddShape(pImportShape->GetShapeDesc(), Maths::HMat43(pImportShape->GetImportRotation(),
			//			pImportShape->GetImportPosition()), 1.f, &objectFilter);
			//		m_ObjectColliderIndex = compDesc.shapes.m_numElements - 1;
			//		
			//		if (pImportShape->GetShapeDesc()->ValidShapeDescriptor())
			//			foundFlags |= 1<<1;
			//	}
			//}
			else if (m_tempHaveNewVehicleToVehicleMesh && elementName == "e_carcollider")
			{
				if (pImportShape)
				{
					vehicleFilter.SetGroupFlags(pImportShape->GetShapeDesc()->GetCollisionFlags());
					pImportShape->GetShapeDesc()->SetPhysicsMaterial(Game::E_PhysicsMaterial_Vehicle);
					compDesc.AddShape(pImportShape->GetShapeDesc(), Maths::HMat43(pImportShape->GetImportRotation(),
						pImportShape->GetImportPosition()), 1.f, &vehicleFilter);
					m_CarColliderIndex = compDesc.shapes.m_numElements - 1;
					
					//if (pMainImportShape)
					//	pMainImportShape->GetShapeDesc()->m_CollisionFlag &= ~(Game::PolyFlags::kVehicleSolid | Game::PolyFlags::kDynamicObjectSolid);

					if (pImportShape->GetShapeDesc()->ValidShapeDescriptor())
						foundFlags |= 1<<2;
				}
			}
		}		

		for (U32 iShape = 0; iShape < compDesc.shapes.Size(); iShape++)
		{
			if (!compDesc.shapes[iShape].m_IneriaTensor.Finite())
			{
				LOGERROR(g_VehicleActorLog, "TEMP HACK FIX: compDesc.shapes[iShape].m_IneriaTensor is INFINITE!!!");
				compDesc.shapes[iShape].m_IneriaTensor = Maths::Constants<Maths::HMat33>::Identity();
			}
			if (!compDesc.shapes[iShape].m_Mass.Finite())
			{
				LOGERROR(g_VehicleActorLog, "TEMP HACK FIX: compDesc.shapes[iShape].m_Mass is INFINITE!!!");
				compDesc.shapes[iShape].m_Mass = Maths::Constants<Maths::HF32>::One();
			}
		}
		compDesc.RemapShape();
		compDesc.SetRemapped();

		m_pCollisionBody = GetPhysicsScene()->CreateBody();
		m_UserData.m_hActorHandle = GetCreationData()->GetActorHandle();
		m_pCollisionBody->SetUserData((void*)&m_UserData);

		Gravity::C_ExternalConstants* pExternal = GetPhysicsExternalConstants();
		m_GravityConstants = *pExternal;
		m_GravityConstants.m_Gravity = Maths::HVec3(0.f, (float)g_debugVehicleGravity, 0.f);
		//pExternal->m_Gravity = Maths::HVec3(0.f, -9.8f, 0.f);
		m_pCollisionBody->SetExternalConstants(&m_GravityConstants);
		m_pCollisionBody->Baumgarte(g_debugVehicleBaumgarte);//PhysicsConstants::kVehicleBaumgarte);
		m_pCollisionBody->Slop(g_dbSlopValue);

		//const Maths::HMat33 inertiaTensor(pImportShape->GetImportIneriaTensor());
		Maths::HMat33 identity;
		identity.SetIdentity();
		Gravity::C_BodyHelper helper(&compDesc);
		helper.CreateBody(m_pVehiclePhysics->Dynamics().mass.mass, m_Matrix.AW(), m_Matrix.Rotation(), m_pCollisionBody, &worldFilter);

		//Go through the shape we've made and set vehicle-vehicle filter in the vehicle-vehicle/vehicle/object shape.

		m_pCollisionBody->SetMaxAngularVelocity((float)g_debugVehicleMaxAngularVelocity);
		float fMaxLinVel = (float)g_debugAbsoluteMaxAVehicleCanEverGo * g_MphToMs;
		m_pCollisionBody->SetMaxLinearVelocity(fMaxLinVel);
		helper.MakeFloat(300.f, 25.f, 6.f, m_pCollisionBody, 1.f, 0.5f);

		SharedPhysics::C_Deactivator deactivator;
		deactivator.m_ActivateThresholdSq = -1.f;
		deactivator.m_SleepThresholdSq = -1.f;
		deactivator.m_sSleepCountdown = 15;

		m_pCollisionBody->SetDeactivator(deactivator);

		{
			Collision::C_Shape* pOrigShape = m_pCollisionBody->GetShape();

			Maths::HVec3 size(pOrigShape->m_LocalBoundingBox.extents.X(), 0.25f, pOrigShape->m_LocalBoundingBox.extents.Z());
			Maths::HVec3 localBoundsOffset; 
			if(pMainImportShape)
			{
				Collision::C_CRAxisBox axisBox(pMainImportShape->GetShapeDesc()->GetBoundingBox());
				size = axisBox.extents;
				localBoundsOffset = axisBox.centre;
			}

			
#if 0		//KierS - no need - the collision detection now automatically deflates the collision primitive by margin!
			if(m_MainElementIndex >= 0)
			{
				Collision::C_CompositeShape* pCompositeShape = (Collision::C_CompositeShape*)m_pCollisionBody->GetShape();
				Collision::C_Shape* pShape = pCompositeShape->GetShape(m_MainElementIndex);
				pShape->SetMargin((float)g_debugCarCollisionMargin);

				//Work out offset from parent shape to main shape...
				//localBoundsOffset = pCompositeShape->m_ElementOffset + (pShape->m_PosRot.Translation() - pCompositeShape->m_PosRot.Translation());
				localBoundsOffset = pCompositeShape->m_ElementOffset + (pShape->m_LocalBoundingBox.centre * pCompositeShape->m_pShapes[m_MainElementIndex].m_RelativeTransformation);
			}
#endif
			m_pVehiclePhysics->SetGravityBody(m_pCollisionBody, Maths::HVec3(0.f, 0.f, 0.f), size, localBoundsOffset);
		}

		if(g_ConstraintCar)
		{
			Gravity::RotationalLimitDescriptor rotDesc;
			const F32 angle = (F32)g_dbConstraintAngle;
			rotDesc.DefineLimits(Maths::HVec3(-angle, -361.f, -angle), Maths::HVec3(angle, 361.f, angle));
			rotDesc.m_AxisOrder = SharedPhysics::C_Constraint::E_AxisOrder_ZXY;
			SharedPhysics::C_Constraint* pConstraint = rotDesc.CreateConstraint(*GetPhysicsScene()->m_pGraphManager, m_pCollisionBody, GetPhysicsScene()->GetStaticWorldBody());
			GetPhysicsScene()->AddDebugConstraint(pConstraint);
			m_pConstraint = pConstraint;
		}

		Collision::C_CompositeShape* pComposite = (Collision::C_CompositeShape*)m_pCollisionBody->GetShape();
		pComposite->m_Filter.SetCollisionFlags(pComposite->m_Filter.GetCollisionFlags() | worldFilter.GetCollisionFlags() | vehicleFilter.GetCollisionFlags()/*Mask*/);
		pComposite->m_Filter.SetBodyMask(pComposite->m_Filter.GetBodyMask() | worldFilter.GetBodyMask() | vehicleFilter.GetBodyMask());
		pComposite->m_Filter.SetCollisionMask(worldFilter.GetCollisionMask() | vehicleFilter.GetCollisionMask());

		if(m_ObjectColliderIndex >= 0)
		{
			
			Collision::C_CompositeShapeIterator iter(pComposite);
			iter.NextShape();
			for(U32 a = 0; a < (U32)m_ObjectColliderIndex; ++a)
			{
				iter.NextShape();
			}
			Collision::C_Shape* pSkirtShape = iter.GetCurrentShape<Collision::C_Shape>();
			pSkirtShape->m_Filter.SetCollisionFlags(objectFilter.GetCollisionFlags());
			pSkirtShape->m_Filter.SetCollisionMask(objectFilter.GetCollisionMask());
			m_ObjectCollisionFilter = pSkirtShape->m_Filter;
			//pSkirtShape->SetMargin((float)g_debugCarCollisionMargin);
		}
		if(m_CarColliderIndex >= 0)
		{
			Collision::C_CompositeShapeIterator iter(pComposite);
			iter.NextShape();
			for(U32 a = 0; a < (U32)m_CarColliderIndex; ++a)
			{
				iter.NextShape();
			}
			Collision::C_Shape* pCarShape = iter.GetCurrentShape<Collision::C_Shape>();
			//pCarShape->SetMargin((float)g_debugCarCollisionMargin);
			//pCarShape->SetFilter(&vehicleFilter);
			pCarShape->m_Filter.SetCollisionFlags(pCarShape->m_Filter.GetCollisionFlags() | vehicleFilter.GetCollisionFlags());
			pCarShape->m_Filter.SetCollisionMask(vehicleFilter.GetCollisionMask());
			pCarShape->m_Filter.SetBodyMask(vehicleFilter.GetBodyMask());
			pCarShape->SetFilter(&pCarShape->m_Filter);

			m_VehicleCollisionFilter = pComposite->m_Filter;
			Collision::C_CRAxisBox box = pCarShape->m_LocalBoundingBox;
			m_pVehicleModel->GetModelInput()->m_vCentre = box.centre;
			m_pVehicleModel->GetModelInput()->m_vExtents = box.extents;
		}
		else
		{
			Collision::C_CRAxisBox box = m_pCollisionBody->GetLocalBoundingBox();
			m_pVehicleModel->GetModelInput()->m_vCentre = box.centre;
			m_pVehicleModel->GetModelInput()->m_vExtents = box.extents;
		}
		
		GetPhysicsScene()->AddBody(m_pCollisionBody);

		m_BodyCollisionFilter = m_pCollisionBody->GetShape()->m_Filter;
		m_CollisionsEnabled = true;
	}

	if (m_pVehicleModel->GetDamageLayer())
		m_pVehicleModel->GetDamageLayer()->SetVehiclePhysics(m_pVehiclePhysics);

	
	RacingPhysics::Axis axis( (Maths::HMat44)m_Matrix );
	m_pVehiclePhysics->SetAxis( axis, ((RacingPhysics::Car*)m_pVehiclePhysics)->Controls().steer );
	m_pVehiclePhysics->SetCopyMassVariablesToGravity();
	m_pVehiclePhysics->CopyToGravity();		
	m_pVehiclePhysics->UnsetCopyMassVariablesToGravity();
	m_pVehiclePhysics->SetGravityScene(GetPhysicsScene());
	m_pVehiclePhysics->SetWheelGroundFilter( &PhysicsConstants::kVehicleWheelCollisionFilter );
	m_pVehiclePhysics->SetWheelOverlayFilter( &PhysicsConstants::kVehicleOverlayCollisionFilter );

	float torqueScale=levelStateCar.mUpgrades.AdditionalHP()?g_SpeedBoostUpgade:1.0f;
	m_pVehiclePhysics->PerformanceModifiers().peakTorqueScale=torqueScale;

	return foundFlags;
}


/*
===============
NAME: KillImpl

NOTE: KillImpl
===============
*/
void C_VehicleActor::KillImpl()
{
	Destroy();
}

/*
===============
NAME: Destroy

NOTE: Destroy
===============
*/
void C_VehicleActor::Destroy()
{
	if (!m_pVehicleModel)
		return;

	Shutdown();

#ifdef _WHEEL_LOD
	for (S32 iLod = 0; iLod < m_LodNodes.Size(); iLod++)
		m_LodNodes[iLod].Clear();
#endif

	m_CollisionCacheVertices.Clear();

	if (m_pVisDetect)
	{
		::Actors::C_VisibilityDetector::ReleaseVisDetect(m_pVisDetect);
		m_pVisDetect = NULL;
	}

	if (m_pReflectionVisDetect)
	{
		::Actors::C_VisibilityDetector::ReleaseVisDetect(m_pReflectionVisDetect);
		m_pReflectionVisDetect = NULL;
	}

//	m_RadiatorSmokeEmitter.Shutdown();
	m_EngineSmokeLightEmitter.Shutdown();
	m_EngineSmokeHeavyEmitter.Shutdown();
//	m_EngineFireEmitter.Shutdown();

	m_ExhaustEffects.Shutdown();

// 	for (S32 iTrail = 0; iTrail < m_ParticleTrails.Size(); iTrail++)
// 		m_ParticleTrails[iTrail].Shutdown();
// 	m_ParticleTrails.Clear();

	m_LightTrails.Clear();

	//for (U32 iModel = 0; iModel < m_pVehicleModel->GetNumModelInstances(); iModel++)
	//{
	//	WorldRender::C_ModelInstance* pModelInstance = m_pVehicleModel->GetModelInstance(iModel);
	//	if (pModelInstance)
	//	{
	//		SceneGraph::Remove(GetSceneGraph(), pModelInstance);
	//		delete pModelInstance;
	//	}
	//}
	for (int iModel = 0; iModel < m_ModelInstances.Size(); iModel++)
	{
		SceneGraph::Remove(GetSceneGraph(), m_ModelInstances[iModel]);
		delete m_ModelInstances[iModel];
	}
	m_ModelInstances.Clear();
	

	SAFE_DELETE_PTR(m_pVehiclePhysics);

	m_pVehicleModel->Destroy();

	SAFE_DELETE_PTR(m_pVehicleModel);

	GetPhysicsScene()->RemoveBody(m_pCollisionBody);
	m_pCollisionBody	= NULL;
	
	const C_VehicleActorCreate *pVehicleCreate = (const C_VehicleActorCreate *)(GetCreationData().GetData());
		
	// Blank focus handles:
	for (S32 i = 0; i < 4 ; i++)
		AmaxCam::CameraSystem().SetFocusHandle(i, Actor::C_ActorHandle());
	SAFE_DELETE_PTR(m_pPhysicsChecks);

	//If shadows
	if (m_pBlobShadowInstance)
	{
		pVehicleCreate->m_pGameWorld->GetBlobShadows()->DestroyInstance(m_pBlobShadowInstance);
		m_pBlobShadowInstance = NULL;
	}
	if (m_ShadowVertexCount)
		Display::Device()->DestroyVertexBuffer(&m_ShadowVertexBuffer);
	if (m_ShadowIndexCount)
		Display::Device()->DestroyIndexBuffer(&m_ShadowIndexBuffer);

	m_pConstraint = NULL;

//#ifdef NOT_USING_AUDIO_COMPONENETS
//	//TODO send these through processmessage 
//	m_vehicleAudio.ReleaseEntityInstance();
//
//	m_vehicleAudio.ReleaseEntityType();
//#endif
}

/*
===============
NAME: CreateImpl

NOTE: CreateImpl
===============
*/
bool C_VehicleActor::CreateImpl()
{
MEMORY_TRACKER_PUSH("graphics.vehicle.actor");
	const C_VehicleActorCreate * pVehicleCreate = (const C_VehicleActorCreate *)(GetCreationData().GetData());

	// load resources
	ActorMsg::C_ResourceLoadMsg tempResourceLogMsg( m_VehicleModelResourceIndex );
	SendMessageToComponent( Components::E_ResourceIF_Loader, &tempResourceLogMsg);
	if (m_LoadSeparateWheelModel)
	{
		ActorMsg::C_ResourceLoadMsg tempWheelResourceLoadMsg( m_WheelModelResourceIndex );
		SendMessageToComponent( Components::E_ResourceIF_Loader, &tempWheelResourceLoadMsg);
	}
	
//#ifdef _WHEEL_LOD
//	ActorMsg::C_ResourceLoadMsg tempWheelLodResourceLogMsg( m_WheelLodModelResourceIndex );
//	SendMessageToComponent( Components::E_ResourceIF_Loader, &tempWheelLodResourceLogMsg);
//#endif

	//ActorMsg::C_ResourceLoadMsg tempAuxFlareResourceLogMsg( m_AuxFlareResourceIndex );
	//SendMessageToComponent( Components::E_ResourceIF_Loader, &tempAuxFlareResourceLogMsg);

	//SendMessageToComponent( Components::E_ResourceIF_Loader, ActorMsg::E_ResourceMsg_LoadAll );
MEMORY_TRACKER_POP("graphics.vehicle.actor");
	return true;
}

bool C_VehicleActor::PrimeImpl()
{
	bool primed = true; // C_GameActor::PrimeImpl();

	//m_hResourceBag = C_ResourceBag::Create();

	//m_hResourceBag = C_ResourceBag::Create();
	//m_hResourceBag->AddListener(this);
	//m_hResourceBag->LockListeners();

	//const C_VehicleActorCreate * pVehicleCreate = (const C_VehicleActorCreate *)(GetCreationData().GetData());
	//
	//m_hResourceBag->AddResource(MakeResourceBagTag(pVehicleCreate->m_ModelFilename), pVehicleCreate->m_ModelFilename, GameResource::C_ModelResource::StaticGetResourceTypeId());
	//m_hResourceBag->AddResource(MakeResourceBagTag(pVehicleCreate->m_WheelFilename), pVehicleCreate->m_WheelFilename, GameResource::C_ModelResource::StaticGetResourceTypeId());
	//m_hResourceBag->AddResource(MakeResourceBagTag("models/system/CarLightFlare.model"), "models/system/CarLightFlare.model", GameResource::C_ModelResource::StaticGetResourceTypeId());
	//	
	//m_hResourceBag->UnlockListeners();
	//primed &= !m_hResourceBag->IsWaitingForLoad();

	return primed;
}

void C_VehicleActor::OnResourceBagLoaded(const C_ResourceBag& bag)
{
	//if (GetLifetimeState() == ActorSys::E_LifetimeState_Priming)
	//	SetPrimed();
}

/*
===============
NAME: InitialiseImpl

NOTE: InitialiseImpl
===============
*/
bool C_VehicleActor::InitialiseImpl()
{
MEMORY_TRACKER_PUSH("graphics.vehicle.actor");
	const C_VehicleActorCreate * pVehicleCreate = (const C_VehicleActorCreate *)(GetCreationData().GetData());

	m_Matrix = pVehicleCreate->GetCreationMatrix();
	m_BirthPlace = m_Matrix;
	m_LastKnownResetMatrix = m_BirthPlace;
	
	m_ModelFileName = pVehicleCreate->m_ModelFilename;
	m_WheelFileName = pVehicleCreate->m_WheelFilename;
	m_DynamicsFileName = pVehicleCreate->m_DynamicsFilename;
	m_XtIndex = pVehicleCreate->m_XtIndex;
	m_VehicleIndex = pVehicleCreate->m_VehicleIndex;

	mPickupSFXArray.Clear();
	
	AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];

	m_AudioRPMLimit = pVehicle->audioRPMLimit;
	m_AudioGarageID = pVehicle->audioGarageID;
	m_AudioForcedInduction = pVehicle->audioForcedInduction;
	m_AudioEngineType = pVehicle->audioEngineType;
	m_AudioExhaustType = pVehicle->audioExhaustType;
	if (pVehicle && pVehicle->wheelModel)
		m_LoadSeparateWheelModel = true;
	else
		m_LoadSeparateWheelModel = false;

	// create resource loader
	{
		Component::C_ComponentCreatorFactoryHelper<Components::C_ResourceListCreate> pCreator;
		m_VehicleModelResourceIndex = pCreator->AddItem(pVehicleCreate->m_ModelFilename, GameResource::C_ModelResource::StaticGetResourceTypeId());
		if (m_LoadSeparateWheelModel)
			m_WheelModelResourceIndex	= pCreator->AddItem(pVehicleCreate->m_WheelFilename, GameResource::C_ModelResource::StaticGetResourceTypeId());
//#ifdef _WHEEL_LOD
//		m_WheelLodModelResourceIndex = pCreator->AddItem("models/vehicles/wheels/wheellod.model", GameResource::C_ModelResource::StaticGetResourceTypeId());
//#endif
		//m_AuxFlareResourceIndex = pCreator->AddItem("models/system/CarLightFlare.model", GameResource::C_ModelResource::StaticGetResourceTypeId());

		if( InitialiseComponent( pCreator.Get() ) == NULL )
			return false;
	}
MEMORY_TRACKER_POP("graphics.vehicle.actor");


	m_corneringAssist = false;
	m_brakeAssist = false;


	//	This will have to do until we have Multiplayer profiles:
	if ( C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork() )
	{
		m_corneringAssist = false;
		m_brakeAssist = false;
	}
	
	m_pAbilities = C_Game::GetInstance()->GetLevelState().mCars[m_VehicleIndex].mUpgrades.GetMultiplayerAbilities();
	//// add a logic component
	//{
	//	Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_Pickup_CreateInfo> pCreator;
	//	pCreator->mParentActorHandle=this->GetHandle();
	//	pCreator->mVehicleIndex=m_VehicleIndex;
	//	if( InitialiseComponent( pCreator.Get() ) == NULL )
	//		return false;
	//}

	//{
	//	if( GetRLC()->IsVehicleAIControlled(m_VehicleIndex) )
	//	{
	//		Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_Pickup_AI_CreateInfo> pCreator;
	//		pCreator->mParentActorHandle=this->GetHandle();
	//		pCreator->mVehicleIndex=m_VehicleIndex;
	//		if( InitialiseComponent( pCreator.Get() ) == NULL )
	//			return false;
	//	}
	//}

	//{
	//	if( GetRLC()->IsVehicleAIControlled(m_VehicleIndex) )
	//	{
	//		Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_AIController_CreateInfo> pCreator;
	//		pCreator->mParentActorHandle=this->GetHandle();
	//		pCreator->mVehicleIndex=m_VehicleIndex;
	//		if( InitialiseComponent( pCreator.Get() ) == NULL )
	//			return false;
	//	}
	//}

	//{
	//	if( GetRLC()->GetPlayerVehicleIndex() == m_VehicleIndex )
	//	{
	//		Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_Pickup_Targetting_CreateInfo> pCreator;
	//		pCreator->mParentActorHandle=this->GetHandle();
	//		pCreator->mVehicleIndex=m_VehicleIndex;
	//		if( InitialiseComponent( pCreator.Get() ) == NULL )
	//			return false;
	//	}
	//}
	//
	//{
	//	Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_Pickup_SFX_CreateInfo> pCreator;
	//	pCreator->mParentActorHandle=this->GetHandle();
	//	pCreator->mVehicleIndex=m_VehicleIndex;
	//	if( InitialiseComponent( pCreator.Get() ) == NULL )
	//		return false;
	//}

	//{
	//	Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_HandOfGod_CreateInfo> pCreator;
	//	pCreator->mParentActorHandle=this->GetHandle();
	//	pCreator->mVehicleIndex=m_VehicleIndex;
	//	if( InitialiseComponent( pCreator.Get() ) == NULL )
	//		return false;
	//}

	//{
	//	Component::C_ComponentCreatorFactoryHelper<Components::I_VehicleComponent_Statistics_CreateInfo> pCreator;
	//	pCreator->mParentActorHandle=this->GetHandle();
	//	pCreator->mVehicleIndex=m_VehicleIndex;
	//	if( InitialiseComponent( pCreator.Get() ) == NULL )
	//		return false;
	//}

bool raining = false;

	//if(pVehicle->vehicleAudioDef)
	//	pVehicle->vehicleAudioDef->SetupEntityType();
//MEMORY_TRACKER_PUSH("graphics.vehicle.actor.audio");
#ifndef NOT_USING_AUDIO_COMPONENETS
		// CreateAudio Component
	m_ControlType = pVehicleCreate->m_ControlType;
//	switch(pVehicleCreate->m_VehicleIndex)
	if (!C_Game::GetInstance()->GetLevelState().mInUI)
	{

		//////if splitscreen, load the non-AI cars as player cars with AI wheels
		////	if(C_Game::GetInstance()->GetLevelState().mNumViewports > 1 && m_ControlType == E_VEHICLE_CONTROL_TYPE_PLAYER)
		////	{
		////		if (pVehicle->vehicleAudioDef)
		////		{						
		////			//use AI wheel
		////			if(pVehicle->aiDryWheelAudioDef)
		////			{
		////				Component::C_ComponentCreatorFactoryHelper<Components::C_AudioEntity_SpawnedCreateInfo> pCreator;

		////				pCreator->m_AudioEntityDef = pVehicle->dryWheelAudioDef; 

		////				if (InitialiseComponent(pCreator.Get()) == NULL)
		////				{
		////					LOGERROR(Game::g_Log, "Failed to create audio component for vehicle actor");
		////					return false;
		////				}
		////			}
		////			

		////			Component::C_ComponentCreatorFactoryHelper<Components::C_AudioEntity_AdditionalSpawnedCreateInfo> pCreator_Additional;

		////			//Player Vehicle
		////			pCreator_Additional->m_AudioEntityDef = pVehicle->vehicleAudioDef; 

		////			//set the new shared entity in the car .. Use AI Shared
		////			if(pVehicle->aiSharedAudioDef)
		////				pCreator_Additional->SetXtSharedEntityDef( pVehicle->aiSharedAudioDef);

		////			if (InitialiseComponent(pCreator_Additional.Get()) == NULL)
		////			{
		////				LOGERROR(Game::g_Log, "Failed to create audio component for vehicle actor");
		////				return false;
		////			}

		////		}
		////	}
		////	else
			//only load for focus car/ first vehicle in single screen games
			if(pVehicleCreate->m_VehicleIndex == 0 && C_Game::GetInstance()->GetLevelState().mNumViewports == 1)
			{
				if (pVehicle->vehicleAudioDef)
				{

					
					if(!raining)
					{
						if(pVehicle->dryWheelAudioDef)
						{
							Component::C_ComponentCreatorFactoryHelper<Components::C_AudioEntity_SpawnedCreateInfo> pCreator;

							pCreator->m_AudioEntityDef = pVehicle->dryWheelAudioDef; 

							if (InitialiseComponent(pCreator.Get()) == NULL)
							{
								LOGERROR(Game::g_Log, "Failed to create audio component for vehicle actor");
								return false;
							}
						}
					}
					else
					{
						if(pVehicle->wetWheelAudioDef)
						{
							Component::C_ComponentCreatorFactoryHelper<Components::C_AudioEntity_SpawnedCreateInfo> pCreator;

							pCreator->m_AudioEntityDef = pVehicle->dryWheelAudioDef; 

							if (InitialiseComponent(pCreator.Get()) == NULL)
							{
								LOGERROR(Game::g_Log, "Failed to create audio component for vehicle actor");
								return false;
							}
						}
					}

					Component::C_ComponentCreatorFactoryHelper<Components::C_AudioEntity_AdditionalSpawnedCreateInfo> pCreator_Additional;

					pCreator_Additional->m_AudioEntityDef = pVehicle->vehicleAudioDef; 

					//set the new shared entity in the car
					if(pVehicle->sharedAudioDef)
						pCreator_Additional->SetXtSharedEntityDef( pVehicle->sharedAudioDef);

					if (InitialiseComponent(pCreator_Additional.Get()) == NULL)
					{
						LOGERROR(Game::g_Log, "Failed to create audio component for vehicle actor");
						return false;
					}

				}

				//if (pVehicle->vehicleAudioDef)
				{


				}
			  }
			 // break;
			else
			
		//case E_VEHICLE_CONTROL_TYPE_AI:
		//case E_VEHICLE_CONTROL_TYPE_NETWORK:
		//default:
			  {
#ifndef PLATFORM_PS3
				if (pVehicle->aiVehicleAudioDef)
#else
				  if (pVehicle->aiVehicleAudioDef_PS3)
#endif
				{

					if(!raining)
					{
						if(pVehicle->aiDryWheelAudioDef)
						{
							Component::C_ComponentCreatorFactoryHelper<Components::C_AudioEntity_SpawnedCreateInfo> pCreator;

							pCreator->m_AudioEntityDef = pVehicle->aiDryWheelAudioDef; 

							if (InitialiseComponent(pCreator.Get()) == NULL)
							{
								LOGERROR(Game::g_Log, "Failed to create audio component for vehicle actor");
								return false;
							}
						}
					}
					else
					{
						if(pVehicle->aiWetWheelAudioDef)
						{
							Component::C_ComponentCreatorFactoryHelper<Components::C_AudioEntity_SpawnedCreateInfo> pCreator;

							pCreator->m_AudioEntityDef = pVehicle->aiWetWheelAudioDef; 

							if (InitialiseComponent(pCreator.Get()) == NULL)
							{
								LOGERROR(Game::g_Log, "Failed to create audio component for vehicle actor");
								return false;
							}
						}
					}
					Component::C_ComponentCreatorFactoryHelper<Components::C_AudioEntity_AdditionalSpawnedCreateInfo> pCreator_Additional;

#ifndef PLATFORM_PS3
					pCreator_Additional->m_AudioEntityDef = pVehicle->aiVehicleAudioDef; 
#else
					//we load a special Bosscar for PS3 boss events
					if(((C_Game::GetInstance()->GetLevelState().mNumViewports > 1 && m_ControlType == E_VEHICLE_CONTROL_TYPE_PLAYER) || Game::C_Game::GetInstance()->GetLevelState().mRaceMode == Game::E_RaceMode_BossBattle) && pVehicle->aiBossBattleVehicleAudioDef_PS3)
						pCreator_Additional->m_AudioEntityDef = pVehicle->aiBossBattleVehicleAudioDef_PS3; 
					else
						pCreator_Additional->m_AudioEntityDef = pVehicle->aiVehicleAudioDef_PS3; 
#endif

					//set the new shared entity in the ai car
					if(pVehicle->aiSharedAudioDef)
					pCreator_Additional->SetXtSharedEntityDef( pVehicle->aiSharedAudioDef);

					if (InitialiseComponent(pCreator_Additional.Get()) == NULL)
					{
						//LOGERROR(Actors::g_Log, "Failed to create audio component for weapon actor");
						return false;
					}
				}
			}
			//  break;
	}
#else

	//AUDIO SETUP
	m_vehicleAudio.SetAudioDefinition(pVehicle->vehicleAudioDef);
	m_vehicleAudio.SetVehicleIndex(m_VehicleIndex);
#endif
//MEMORY_TRACKER_POP("graphics.vehicle.actor.audio");

	ResetVehicleAlphaComponents();

	return true;
}

/*
===============
NAME: DrawDebugHud

NOTE: Draw debug hud
===============
*/
void C_VehicleActor::DrawDebugHud()
{
	if (!g_debugDrawVehicleHud || !m_pVehiclePhysics)
		return;

	Maths::HVec3 centre((float)g_debugHudNeedleCentreX, (float)g_debugHudNeedleCentreY, 0.0f);
	Maths::HVec3 end(0.0f, -(float)g_debugHudNeedleLength, 0.0f);

	float minRevs = 0.0f;
	float redLine = m_pVehiclePhysics->Engine().LimiterRPM();
	float maxRevs = m_pVehiclePhysics->Engine().LimiterRPM() + 2500.0f;
	if (maxRevs == 0.0f)
		maxRevs = 1.0f;

	float fRevs = m_pVehiclePhysics->Engine().RPM();

	float fRevStep = Maths::Clamp(fRevs / maxRevs, 0.0f, 1.0f);
	float angle = Maths::Lerp((float)g_debugHudNeedleRestAngle, (float)g_debugHudNeedleMaxAngle, fRevStep);
	
	float fNumSteps = maxRevs / 500.0f;
	float fRedLinePos = Maths::Clamp(redLine / maxRevs, 0.0f, 1.0f);

	Maths::RGBA colour(0, 128, 0, 255);
	
	if (fRevs >= redLine)
		colour = Maths::RGBA(255, 0, 0, 255);

	Maths::C_Quaternion rot(Maths::HVec3(0.0f, 0.0f, 1.0f), Maths::HF32(angle));

	end = end * rot;

	end += centre;
	
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dLine(Maths::HVec2(centre.X(), centre.Y()), Maths::HVec2(end.X(), end.Y()), colour);

	for (U32 iStep = 0; iStep <= (U32)fNumSteps; iStep++)
	{
		float lineLength = 16.0f;

		bool drawText = false;

		//Big line
		if (fmod((float)iStep, 2.0f) == 0.0f)
		{
			lineLength = 32.0f;
			drawText = true;
		}

		Maths::HVec3 stepStart(0.0f, -(float)g_debugHudNeedleLength, 0.0f);
		Maths::HVec3 stepEnd(0.0f, -(float)g_debugHudNeedleLength - lineLength, 0.0f);

		float fStep = Maths::Clamp((float)iStep / fNumSteps, 0.0f, 1.0f);
		float fAngle = Maths::Lerp((float)g_debugHudNeedleRestAngle, (float)g_debugHudNeedleMaxAngle, fStep);

		Maths::RGBA colour(0, 0, 255, 255);

		if (fStep >= fRedLinePos)
			colour = Maths::RGBA(255, 0, 0, 255);
	
		Maths::C_Quaternion stepRot(Maths::HVec3(0.0f, 0.0f, 1.0f), Maths::HF32(fAngle));
		stepStart = stepStart * stepRot;
		stepEnd = stepEnd * stepRot;
		stepEnd += centre;
		stepStart += centre;
		
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dLine(Maths::HVec2(stepStart.X(), stepStart.Y()), Maths::HVec2(stepEnd.X(), stepEnd.Y()), colour);

		if (drawText)
		{
			float fStepRevs = fStep * maxRevs;
			char text[260];
			_snprintf(text, sizeof(text), "%.0f", fStepRevs / 1000.0f);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(stepEnd.X(), stepEnd.Y() - 16.0f), Util::C_String(text));
		}
	}

	if (fNumSteps == 0.0f)
		fNumSteps = 1.0f;

	//Quick hack - draw last big line
	{
		float lineLength = 32.0f;

		Maths::HVec3 stepStart(0.0f, -(float)g_debugHudNeedleLength, 0.0f);
		Maths::HVec3 stepEnd(0.0f, -(float)g_debugHudNeedleLength - lineLength, 0.0f);

		float fAngle = (float)g_debugHudNeedleMaxAngle;
		Maths::RGBA colour(255, 0, 0, 255);
		Maths::C_Quaternion stepRot(Maths::HVec3(0.0f, 0.0f, 1.0f), Maths::HF32(fAngle));
		stepStart = stepStart * stepRot;
		stepEnd = stepEnd * stepRot;
		stepEnd += centre;
		stepStart += centre;		
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dLine(Maths::HVec2(stepStart.X(), stepStart.Y()), Maths::HVec2(stepEnd.X(), stepEnd.Y()), colour);
	}
	//Draw last red line
	{
		float lineLength = 32.0f;

		Maths::HVec3 stepStart(0.0f, -(float)g_debugHudNeedleLength, 0.0f);
		Maths::HVec3 stepEnd(0.0f, -(float)g_debugHudNeedleLength - lineLength, 0.0f);

		float fAngle = Maths::Lerp((float)g_debugHudNeedleRestAngle, (float)g_debugHudNeedleMaxAngle, fRedLinePos);
		Maths::RGBA colour(255, 0, 0, 255);
		Maths::C_Quaternion stepRot(Maths::HVec3(0.0f, 0.0f, 1.0f), Maths::HF32(fAngle));
		stepStart = stepStart * stepRot;
		stepEnd = stepEnd * stepRot;
		stepEnd += centre;
		stepStart += centre;		
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dLine(Maths::HVec2(stepStart.X(), stepStart.Y()), Maths::HVec2(stepEnd.X(), stepEnd.Y()), colour);
	}
}

void C_VehicleActor::DisplayDebug()
{
	if (!m_pVehiclePhysics)
		return;

	m_pVehiclePhysics->DisplayDebug();
	if(g_dbDrawTriangleCache)
		m_pVehiclePhysics->DebugRenderTriangleCache(DbUtil::DefaultDebugRenderSys()->Deferred());
}


/*
===============
NAME: GetVehicleInput

NOTE: Return vehicle input
===============
*/
Vehicle::C_ModelInput* C_VehicleActor::GetVehicleInput() const
{
	if(m_pVehicleModel)
	{
		return m_pVehicleModel->GetModelInput();
	}

	return NULL;
}
/*
===============
NAME: GetVehicleEnabled

NOTE: Is this vehicle going to respond to control messages
===============
*/
const bool C_VehicleActor::GetVehicleControlsEnabled() const
{
	return m_ControlsEnabled;
}

const int C_VehicleActor::GetVehicleIndex() const
{
	return m_VehicleIndex;
}

void C_VehicleActor::GetNetworkControlsAndState(RacingPhysics::Vehicle::CompressedNetworkControls* controls, RacingPhysics::Vehicle::CompressedNetworkState* state, const C_VehicleActor* pTarget)
{
	m_pVehiclePhysics->GetNetworkControls(*controls);
	m_pVehiclePhysics->GetNetworkState(state, pTarget ? pTarget->m_pVehiclePhysics : 0);
}

Game::Actors::RaceLogicCore *C_VehicleActor::GetRLC() const
{
	const C_VehicleActorCreate *pVehicleCreate = (const C_VehicleActorCreate *)(GetCreationData().GetData());
	return pVehicleCreate->m_pRLC;
}

void C_VehicleActor::SetLandedEffect(const SpecialFX::C_SpecialFX* p_pScreenEffect)
{
	if (m_pLandedSpecialFX || !p_pScreenEffect)
		return;

	if(GetFakePhysics()->IsActive())
	{
		return;
	}
	if (m_pLandedSpecialFX)
	{
		m_pLandedSpecialFX->Stop();
		SAFE_DELETE_PTR(m_pLandedSpecialFX)
	}

	m_fLandedEffectTime = 0.0f;

	m_pLandedSpecialFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(*p_pScreenEffect, GetActorList()->GetDefaultContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr());

	if (!m_pLandedSpecialFX)
		return;

	const C_VehicleActorCreate *pVehicleCreate = (const C_VehicleActorCreate *)(GetCreationData().GetData());

	if(!pVehicleCreate)
		return;

	S32 viewIndex = pVehicleCreate->m_pRLC->GetViewportIndex(m_VehicleIndex);
	if(viewIndex != -1)
	{
		m_pLandedSpecialFX->SetParameter(SpecialFX::E_ParamID_SetCameraViewport, (void *)viewIndex);
		m_pLandedSpecialFX->Start();
	}
}

void C_VehicleActor::SetImpactEffect(const SpecialFX::C_SpecialFX* p_pScreenEffect,const Maths::HMat33& impactTransform, bool immediate)
{
	if (m_pImpactSpecialFX || !p_pScreenEffect)
		return;

	m_pPendingImpactEffect		= p_pScreenEffect;
	m_PendingImpactTransform	= impactTransform;
	
	if (!immediate)
		return;

	if (m_pImpactSpecialFX)
	{
		m_pImpactSpecialFX->Stop();
		SAFE_DELETE_PTR(m_pImpactSpecialFX)
	}

	m_fImpactEffectTime = 0.0f;

	m_pImpactSpecialFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(*m_pPendingImpactEffect, GetActorList()->GetDefaultContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr());

	if (!m_pImpactSpecialFX)
	{
		m_pPendingImpactEffect	= NULL;
		return;
	}

	const C_VehicleActorCreate *pVehicleCreate = (const C_VehicleActorCreate *)(GetCreationData().GetData());

	S32 viewIndex = pVehicleCreate->m_pRLC->GetViewportIndex(m_VehicleIndex);
	if(viewIndex != -1)
	{
		m_pImpactSpecialFX->SetParameter(SpecialFX::E_ParamID_EnableTransformUpdate, (void*)1);
		m_pImpactSpecialFX->SetParameter(SpecialFX::E_ParamID_SetCameraViewport, (void *)viewIndex);
		m_pImpactSpecialFX->Start();
		m_pImpactSpecialFX->UpdateWorldMatrix(Maths::HMat43(m_PendingImpactTransform));
	}

	m_pPendingImpactEffect	= NULL;
}

void C_VehicleActor::SetHealthEffect(const SpecialFX::C_SpecialFX* p_pHealthEffect)
{
	if (m_pHealthSpecialFX)
	{
		m_pHealthSpecialFX->Stop();
		SAFE_DELETE_PTR(m_pHealthSpecialFX)
	}

	if (!p_pHealthEffect)
		return;

	m_fHealthEffectTime = 0.0f;

	m_pHealthSpecialFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(*p_pHealthEffect, GetActorList()->GetDefaultContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr());

	if (!m_pHealthSpecialFX)
		return;

	const C_VehicleActorCreate *pVehicleCreate = (const C_VehicleActorCreate *)(GetCreationData().GetData());

	if(!pVehicleCreate)
		return;

	S32 viewIndex = pVehicleCreate->m_pRLC->GetViewportIndex(m_VehicleIndex);
	if(viewIndex != -1)
	{
		m_pHealthSpecialFX->SetParameter(SpecialFX::E_ParamID_SetCameraViewport, (void *)viewIndex);
		m_pHealthSpecialFX->Start();
	}
}

/*
===============
NAME: SpawnImpactEffects

NOTE: Spawns impact effects
===============
*/
void C_VehicleActor::SpawnImpactEffects(U32 p_SurfaceType, 
										Maths::HVec3 p_Position, 
										Maths::HVec3 p_Direction, 
										Maths::HVec3 p_Velocity, 
										float p_Intensity)
{
	Maths::C_QuatTrans transform;
	DynamicEffect::C_Node node;
	Maths::HVec3 vDirection = p_Direction;
	Maths::HVec3 vVelocity = p_Velocity;
	vDirection = Maths::Normalise(vDirection);
	
	//Set up the position of the emitter
	node.SetPosition(p_Position);
	node.SetRotation(DynamicEffect::GetQuaternionFromTo<Maths::C_Quaternion>(Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ(), vDirection));

	//Surface specific stuff
	for (S32 iEffect = 0; iEffect < C_GameEffectData::Instance().mAmaxImpactSurfaceEffects.Number(); iEffect++)
	{
		AmaxImpactSurfaceEffect* pEffect = C_GameEffectData::Instance().mAmaxImpactSurfaceEffects[iEffect];
		
		bool surfaceValid = false;
		for (S32 iSurface = 0; iSurface < pEffect->m_SurfaceTypes.count && !surfaceValid; iSurface++)
			if (p_SurfaceType == pEffect->m_SurfaceTypes[iSurface])
				surfaceValid = true;

		if (surfaceValid && p_Intensity >= pEffect->m_MinImpact && p_Intensity <= pEffect->m_MaxImpact)
		{
			float intensity = Maths::Clamp((p_Intensity - pEffect->m_MinImpact) / (pEffect->m_MaxImpact - pEffect->m_MinImpact), 0.0f, 1.0f);
			intensity = Maths::Lerp(pEffect->m_MinIntensity, pEffect->m_MaxIntensity, intensity);

			DynamicEffect::Particle::C_EventInstanceHandle handle = pEffect->m_ParticleEvent->pEvent->Create(node);//, &m_LocalSpace);
			if(DynamicEffect::Particle::C_EventInstance* pEventInstance = handle.Object())
			{
				//Maths::C_RGBA_F32 colour(Maths::Load<Maths::C_RGBA_F32>(&m_pModel->EvaluatedSHSampleColour()));
				//pEventInstance->SetLighting(colour);
				pEventInstance->SetLighting(m_pVehicleModel->GetLighting());
				pEventInstance->SetIntensity(intensity * m_ParticleEmitScale);
				pEventInstance->SetLifeScale(m_ParticleLifeScale);
				pEventInstance->SetVelocity(vVelocity);
				pEventInstance->SetCollisionPlane(m_GroundPosition, m_GroundNormal);
			}	
		}
	}
}

/*
===============
NAME: SetVehicleStatus

NOTE: Sets the rendering/physics status of the vehicle
===============
*/
void C_VehicleActor::SetVehicleStatus(bool p_Status)
{
	if (p_Status == m_pVehicleModel->GetModelInput()->m_bShouldUpdate &&
		p_Status == m_pVehicleModel->GetModelInput()->m_bShouldRender)
		return;

	m_ControlsEnabled									= p_Status;
	m_pVehicleModel->GetModelInput()->m_bShouldRender	= p_Status;
	m_pVehicleModel->GetModelInput()->m_bShouldUpdate	= p_Status;

	if (!p_Status)
	{
		RacingPhysics::Axis axis;
		axis.SetIdentityMatrix();
		axis.SetPosition(RacingPhysics::Point3R(0.0f, 0.0f, 0.0f));
		m_Matrix.SetIdentity();

		for (U32 iInstance = 0; iInstance < m_pVehicleModel->GetNumModelInstances(); iInstance++)
		{				
			WorldRender::C_ModelInstance* pModelInstance = m_pVehicleModel->GetModelInstance(iInstance);
			pModelInstance->SetWorldSpherePosition(m_Matrix.Translation());
			pModelInstance->SetElementMatrix(-1, m_Matrix);
		}

		m_pVehiclePhysics->SetAxis(axis, ((RacingPhysics::Car*)m_pVehiclePhysics)->Controls().steer, true);
		m_pVehicleModel->GetModelInput()->Clear();
		m_pVehicleModel->GetComponentList().DisableAllParts();
		m_pVehicleModel->GetComponentList().UpdateMatrices(m_Matrix, m_pVisDetect, true);
		GetPhysicsScene()->TemporarilyRemoveBody(m_pCollisionBody);
	}
}

Debug::Bool g_debugDisableAITorqueForces("Vehicle.DisableAITorqueForces", false);

void C_VehicleActor::AIApplyTorquesForces()
{
	if (g_debugDisableAITorqueForces)
		return;

	//if(m_PerformanceModifiers.mDisableHandOfGod)
	//	return;

	Maths::HVec3 offset = m_pCollisionBody->GetBody().m_CenterOfMassOffset;
	// [Jan] Changed the ApplyImpulseLocalSpace to be immediate rather than deferred, to avoid a mutex contention
	// [Kier] This will mean that this impulse is unchecked - i.e. doesn't test whether it results in an invalid velocity (in case of infinities)
	m_pCollisionBody->GetBody().ApplyImpulseLocalSpace(m_AILocalForce, offset);
	m_pCollisionBody->AddTorque(m_AITorque);
}

void C_VehicleActor::SetLinearVelocityFromSpeed(float speed)
{
	m_pCollisionBody->GetBody().m_LinearVelocity = m_pVehiclePhysics->GetAxis().AxisZ() * speed;
}

Maths::HVec3 C_VehicleActor::GetExtents() const
{
	return m_pVehicleModel->GetModelInput()->m_vExtents;
}

void C_VehicleActor::NetworkTests()
{
	static int count = 0;
	static int max = 3;

	static RacingPhysics::Vehicle::CompressedNetworkControls controls;
	static RacingPhysics::Vehicle::CompressedNetworkState state;

	RacingPhysics::Car* pCar = (RacingPhysics::Car*) m_pVehiclePhysics;

	if (GetVehicleIndex() == 0)
	{
		if (count == 0)
		{
			pCar->GetNetworkControls(controls);
			pCar->GetNetworkState(&state);
		}

		count++;
	}
	else if (GetVehicleIndex() == 1)
	{
		if (count >= max)
		{
			count = 0;
			
			pCar->ProcessNetworkState(state, false);
		}

		pCar->SetControlType(RacingPhysics::Vehicle::CONTROL_TYPE_NETWORK);
		pCar->SetNetworkControls(controls);
	}
}

Debug::Bool g_debugDeferredVehicleReset("Vehicle.DeferredVehicleReset", true);
Debug::Bool g_debugEnableFatalDamageCam("Vehicle.Damage.Fatal.EnableFatalDamageCamera", false);

/*
===============
NAME: MessageHandler

NOTE: Handles messages
===============
*/
bool C_VehicleActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	//PERF_EVENT("C_VehicleActor::MessageHandler");	
	//send messages to audio component
//	#ifdef NOT_USING_AUDIO_COMPONENETS
//	m_vehicleAudio.ProcessMessage(pMessage);
//#endif

	//Testing something...
	if (m_ResourcesUnloaded && pMessage->Type() != ActorMsg::E_ResourceMsg_AllUnloaded)
		return true;

	E_ReplayMode replayState = C_Game::GetInstance()->GetLevelState().GetReplayMode();

	switch(pMessage->Type())
	{

	case Game::E_GameflowMessage_RaceLoaded:
		{
			m_ControlsEnabled = true;
			break;		
		}

	case Game::E_GameflowMessage_RaceStarted:
		{
			m_BirthPlace = m_Matrix;
			break;
		}

	case GameMessaging::E_GameMsg_PreUpdate:
		{		
			const GameMessaging::C_PreUpdateMsg* pMsg = Messaging::MessageCast<GameMessaging::C_PreUpdateMsg>(pMessage);
			m_FrameTime = pMsg->GetValue();
			if (m_pVehicleModel)
				m_pVehicleModel->SetFrameTime(m_FrameTime);
			
			Update();
			
			if (g_debugVehicleDisplayDebugModels)
			{
				if (m_pVehicleModelResource)
					m_pVehicleModelResource->GetModelData()->DebugRender(DbUtil::DefaultDebugRenderSys()->Deferred(), m_Matrix);
				if (m_pWheelModelResource)
					m_pWheelModelResource->GetModelData()->DebugRender(DbUtil::DefaultDebugRenderSys()->Deferred(), m_Matrix);
			}
		}
		return true;

	case GameMessaging::E_GameMsg_Update:
		{
			const GameMessaging::C_UpdateMsg* pMsg = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage);
			m_FrameTime = pMsg->GetValue();		
			if (m_pVehicleModel)
				m_pVehicleModel->SetFrameTime(m_FrameTime);

			if (m_pPendingImpactEffect)
				SetImpactEffect(m_pPendingImpactEffect, m_PendingImpactTransform, true);
		}
		return true;
	case Game::E_VehicleMessage_PostUpdate:
		{
			PostUpdate();
		}
		return true;
	case GameMessaging::E_GameMsg_PausedUpdate:
		{
			PausedUpdate();
		}
		return true;
	case ActorMsg::E_GameActorMsg_Update2ndPass:
		{
		}
		return true;
	case Game::E_VehicleMessage_Enable:
		{
			EnableVehicle();
		}
		return true;
	case Game::E_VehicleMessage_Disable:
		{
			DisableVehicle();
		}
		return true;
	case Game::E_VehicleMessage_SetVisibility:
		{
			const C_SetVehicleVisibility* pMsg = Messaging::MessageCast<C_SetVehicleVisibility>(pMessage);
			SetVisibility(pMsg->m_Visibility, pMsg->m_ViewportIndex, true, false);
			DisplayDriver(pMsg->m_Visibility, pMsg->m_ViewportIndex);
		}
		return true;
	case Game::E_VehicleMessage_UpdatePlayerControls:
		{
			if(!m_pVehiclePhysics || /*!m_RaceStarted ||*/ m_pVehiclePhysics->GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_AI || m_pVehiclePhysics->GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_NETWORK)
			{

			}
			else
			{
				const Game::Actors::C_VehicleUpdateControlsMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleUpdateControlsMsg>(pMessage);

				//Check that this message is for this actor
				RacingPhysics::CarControls& controls = ((RacingPhysics::Car*)m_pVehiclePhysics)->Controls();
				controls.Clear();

				if(m_ControlsEnabled && !m_Eliminated && !m_FatallyDamaged)
				{
					//If the vehicle's enabled just pass the control messages through as per normal
					controls.throttle	= pMsg->m_fThrottle;
					controls.steer		= pMsg->m_fSteering;
					controls.brake		= pMsg->m_fBrake;
					controls.handbrake	= pMsg->m_bHandbrake;
					controls.gearUp		= pMsg->m_bGearUp;
					controls.gearDown	= pMsg->m_bGearDown;

					m_accelControllerInput = controls.throttle;
					m_brakeControllerInput = controls.brake;
					m_handbrakeControllerInput = controls.handbrake;

					CheckOverideControls(controls);				// Handles any perk or cutscene specific control overrides

					SetFlightControl(Maths::Clamp(pMsg->m_fSteering, -1.0f, 1.0f), Maths::Clamp(pMsg->m_fPitch, -1.0f, 1.0f));
				
					BIZ_PANIC(controls.throttle >= 0.0f && controls.throttle <= 1.0f);
				
				}
				else
				{
					controls.brake		= 0.f;
					controls.handbrake	= 1.f;
				}

				ApplyPerformanceModifiers(&controls);

				BIZ_PANIC(controls.throttle >= 0.0f && controls.throttle <= 1.0f);
				//NetworkTests();
			}
		}
		return true;
	case Game::E_VehicleMessage_UpdateAIControls:
		{
			// NOTE : Keep this and the above in sync please... player assists come through this code...

			//if (m_pVehiclePhysics->GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_AI)
			{
				const Game::Actors::C_VehicleUpdateAIControlsMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleUpdateAIControlsMsg>(pMessage);
				RacingPhysics::CarControls& controls = ((RacingPhysics::Car*)m_pVehiclePhysics)->Controls();
				controls.Clear();

				if(m_ControlsEnabled && !m_Eliminated && !m_FatallyDamaged)
				{
					controls.throttle	= pMsg->m_fThrottle;
					controls.steer		= pMsg->m_fSteering;
					controls.brake		= pMsg->m_fBrake;
					controls.handbrake	= pMsg->m_bHandbrake;
					controls.gearUp		= pMsg->m_bGearUp;
					controls.gearDown	= pMsg->m_bGearDown;

					CheckOverideControls(controls);				// Handles any perk or cutscene specific control overrides

					SetFlightControl(Maths::Clamp(pMsg->m_fSteering, -1.0f, 1.0f), Maths::Clamp(pMsg->m_fPitch, -1.0f, 1.0f));

					// AI torques and forces.....
					m_AILocalForce = pMsg->m_localForce;
					m_AITorque = pMsg->m_torque;

					if (!controls.handbrake)
					{
						if(!m_PerformanceModifiers.mCutThrottle.mModifier && !m_PerformanceModifiers.mClampSteering.mModifier && !m_PerformanceModifiers.mForce.mModifier.Length())
						{
							AIApplyTorquesForces();
						}
					}

					BIZ_PANIC(controls.throttle >= 0.0f && controls.throttle <= 1.0f);
				}
				else
				{
					controls.brake		= 0.f;
					controls.handbrake	= 1.f;					
				}

				// problem with the performance modifiers being called at the beginning of the frame and then inside the physics loop
				if (m_pVehiclePhysics->GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_AI)
				{
					ApplyPerformanceModifiers(&controls);
				}

				BIZ_PANIC(controls.throttle >= 0.0f && controls.throttle <= 1.0f);
			}
		}
		return true;
	case Game::E_VehicleMessage_UpdateNetworkControls:
		{
			if(!m_Eliminated)
			{
				const Game::Actors::C_VehicleUpdateNetworkControlsMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleUpdateNetworkControlsMsg>(pMessage);

				m_pVehiclePhysics->SetNetworkControls(pMsg->m_CarControls);
				m_pVehiclePhysics->ProcessNetworkState(pMsg->m_CarState, true);
			}
		}
		return true;
	case Game::E_VehicleMessage_UpdatePerformanceModifiers:
		{
			const Game::Actors::C_VehicleUpdatePerformanceModifiersMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleUpdatePerformanceModifiersMsg>(pMessage);

			if(m_VehicleIndex == pMsg->mVehicleIndex)
			{
				//Boost
				if(pMsg->mPerformanceModifiers.mBoost.mModify)
				{
					m_PerformanceModifiers.mBoost.Set(true,pMsg->mPerformanceModifiers.mBoost.mModifier);
				}

				//Draft Boost
				if(pMsg->mPerformanceModifiers.mDraftBoost.mModify)
				{
					m_PerformanceModifiers.mDraftBoost.Set(true,pMsg->mPerformanceModifiers.mDraftBoost.mModifier);
				}

				//Assist Boost
				if(pMsg->mPerformanceModifiers.mAssistBoost.mModify)
				{
					m_PerformanceModifiers.mAssistBoost.Set(true,pMsg->mPerformanceModifiers.mAssistBoost.mModifier);
				}

				//XP Muliplier
				if(pMsg->mPerformanceModifiers.mXPMultiplier.mModify)
				{
					m_PerformanceModifiers.mAssistBoost.Set(true,pMsg->mPerformanceModifiers.mXPMultiplier.mModifier);
				}

				//Force and Position
				if(pMsg->mPerformanceModifiers.mForce.mModify && pMsg->mPerformanceModifiers.mPosition.mModify)
				{
					m_PerformanceModifiers.mForce.Set(true,pMsg->mPerformanceModifiers.mForce.mModifier);
					m_PerformanceModifiers.mPosition.Set(true,pMsg->mPerformanceModifiers.mPosition.mModifier);
				}

				//CutThrottle
				if(pMsg->mPerformanceModifiers.mCutThrottle.mModify)
				{
					m_PerformanceModifiers.mCutThrottle.Set(true,pMsg->mPerformanceModifiers.mCutThrottle.mModifier);
				}

				//Clamp Steering and Angle
				if(pMsg->mPerformanceModifiers.mClampSteering.mModify && pMsg->mPerformanceModifiers.mClampedSteeringAngle.mModify)
				{
					m_PerformanceModifiers.mClampSteering.Set(true,pMsg->mPerformanceModifiers.mClampSteering.mModifier);
					m_PerformanceModifiers.mClampedSteeringAngle.Set(true,pMsg->mPerformanceModifiers.mClampedSteeringAngle.mModifier);
				}
			}
		}
		return true;

	case E_VehicleLogicMsg_HitByPerk:
		{
			const C_VehicleLogicMsg_HitByPerkMsg* msg	= Messaging::MessageCast< C_VehicleLogicMsg_HitByPerkMsg >(pMessage);
			if(msg->mVehicleIndex == m_VehicleIndex)
				m_ParticleReducedEmitScaleTimer = g_debugParticleReducedEmitScaleTimer;

			return true;
		}

	case Game::E_VehicleMessage_ForceAIControl:
		{
			const Game::Actors::C_VehicleForceAIControlMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleForceAIControlMsg>(pMessage);
			if(m_VehicleIndex == pMsg->m_VehicleIndex)
				ForceAIControl();
		}
		return true;
	case Game::E_VehicleMessage_ForceHumanControl:
		{
			const Game::Actors::C_VehicleForceHumanControlMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleForceHumanControlMsg>(pMessage);
			if(m_VehicleIndex == pMsg->m_VehicleIndex)
				ForcePlayerControl();
		}
		return true;

	case Game::E_VehicleMessage_ResetAtSpawn:
		{
			const Game::Actors::C_VehicleResetAtSpawnPositionMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleResetAtSpawnPositionMsg>(pMessage);
			if(m_VehicleIndex == pMsg->mVehicleIndex || pMsg->mVehicleIndex == -1)
			{
				if (g_debugDeferredVehicleReset)
				{
					m_DeferredResetType = E_ResetType_Spawn;
				}
				else
				{
					Reset(E_ResetType_Spawn);
					m_DeferredResetType = E_ResetType_None;
				}
			}
		}
		return true;
	case Game::E_VehicleMessage_ResetAtWorldPosition:
		{
			const Game::Actors::C_VehicleResetAtWorldPositionMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleResetAtWorldPositionMsg>(pMessage);
			if(GetCreationData()->GetActorHandle() == pMsg->mActorHandle && m_VehicleIndex == pMsg->mVehicleIndex)
			{
				if (g_debugDeferredVehicleReset && !C_Game::GetInstance()->GetLevelState().mInUI)
				{
					m_DeferredResetType = E_ResetType_Position;
					m_ResetOnTrackMsg = *pMsg;
				}
				else
				{
					Reset(E_ResetType_Position, pMsg->mNewWorldMatrix, pMsg->mVelocity);
					m_DeferredResetType = E_ResetType_None;

					if (C_Game::GetInstance()->GetLevelState().mInUI)
					{
						if (m_pVehicleModel)
							m_pVehicleModel->SetDynamicShaderParameters(true);
					}
				}
			}
		}
		return true;
	
	case Game::E_VehicleMessage_ResetAtCameraPosition:
		{
			const Game::Actors::C_VehicleResetAtCameraPositionMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleResetAtCameraPositionMsg>(pMessage);
			if(GetCreationData()->GetActorHandle() == pMsg->mActorHandle && m_VehicleIndex == pMsg->mVehicleIndex)
			{
				if (g_debugDeferredVehicleReset)
				{
					m_DeferredResetType = E_ResetType_Camera;
				}
				else
				{
					Reset(E_ResetType_Camera);
					m_DeferredResetType = E_ResetType_None;
				}
			}
		}
		return true;
	case Game::E_VehicleMessage_ExhaustPopEvent:
		{
	//		const Game::Actors::C_VehicleExhaustPopMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleExhaustPopMsg>(pMessage);
	//		if (pMsg->m_VehicleIndex == m_VehicleIndex && m_EnableParticles)
	//			m_ExhaustEffects.SpawnEffect((Game::Effects::E_ExhuastEffectType)pMsg->m_ExhaustPopType, m_ParticleEmitScale);
		}
		return true;

	case Game::E_VehicleMessage_Freeze:
		{
			const Game::Actors::C_VehicleFreezeMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleFreezeMsg>(pMessage);
			if (pMsg->m_VehicleIndex == m_VehicleIndex)
			{
				if (pMsg->m_Freeze)
					Freeze();
				else 
					Resume();
			}
		}
		return true;
	case Game::E_VehicleMessage_LoseGrip:
		{
			if (!m_bLostGrip && m_bCanLoseGrip)
			{
				m_bLostGrip = true;
				m_fLostGripTime = 0.0f;
			}
		}
		return true;	
	case Game::E_VehicleMessage_DontLoseGrip:
		{
			if (!m_bLostGrip)				
			{
				m_bCanLoseGrip		= false;
				m_fCantLoseGripTime = 0.0f;
			}
		}
		return true;
	case Game::E_VehicleMessage_ApplyDebugDamage:
		{
			const Game::Actors::C_VehicleDebugDamageMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleDebugDamageMsg>(pMessage);
			FireDamageEvent(pMsg->m_Position, pMsg->m_Direction);
		}
		return true;
	case Game::E_VehicleMessage_Respray:
		{
			const Game::Actors::C_VehicleResprayMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleResprayMsg>(pMessage);
			if (pMsg->m_VehicleIndex == m_VehicleIndex && m_pVehicleModel)
			{
				const C_VehicleActorCreate *pVehicleCreate = (const C_VehicleActorCreate *)(GetCreationData().GetData());

				Vehicle::XtMaterialDefaults* pMaterialDefaults = NULL;
				if (C_GameVehicleData::Instance().mVehicleDefaults.Number() > 0)
					pMaterialDefaults = C_GameVehicleData::Instance().mVehicleDefaults[0]->materials;

				Vehicle::XtPaintJob* pPaintJob = NULL;

				if ( UI::C_UiXtConfig::Instance() )
					pPaintJob = UI::C_UiXtConfig::Instance()->m_GarageResprays.Find( pMsg->m_PaintJobXtId );

				AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];
				Vehicle::XtVehicle* pXtVehicle = pVehicle ? &pVehicle->baseVehicle : NULL;
				m_pVehicleModel->GetPaintJob()->UpdateMaterial(pXtVehicle, pMaterialDefaults, 0, pPaintJob);
				m_pVehicleModel->GetPaintJob()->UpdateMaterialTexture();
			}
		}
		return true;		

/*
	case Game::E_GameflowMessage_RaceLoaded:
		{
			m_ControlsEnabled = true;
			return true;		
		}
*/

	case ActorMsg::E_ResourceMsg_AllLoaded:
		{
			m_pVehicleModelResource = QueryInterface<Components::I_ResourceLoader>()->GetResource<GameResource::C_ModelResource>( m_VehicleModelResourceIndex );
			m_pWheelModelResource = QueryInterface<Components::I_ResourceLoader>()->GetResource<GameResource::C_ModelResource>( m_WheelModelResourceIndex );
			//m_pAuxFlareModelResource = QueryInterface<Components::I_ResourceLoader>()->GetResource<GameResource::C_ModelResource>( m_AuxFlareResourceIndex );
//#ifdef _WHEEL_LOD
//			m_pWheelLodModelResource = QueryInterface<Components::I_ResourceLoader>()->GetResource<GameResource::C_ModelResource>( m_WheelLodModelResourceIndex );
//#endif

			bool loadStatus = false;

			if (m_pVehicleModelResource)
			{
				if (m_pWheelModelResource || (!m_pWheelModelResource && !m_LoadSeparateWheelModel))
					loadStatus = Create();
			}

			if (C_Game::GetInstance()->GetLevelState().mInUI)
				GetRLC()->SetVehicleLoadingHandle(GetHandle());

			C_VehicleLoadedMsg loadedMsg(GetCreationData()->GetActorHandle(), m_VehicleIndex, loadStatus);
			BroadcastMessage(&loadedMsg);

			AddSubscribers();
		}
		return true;
	//case ActorMsg::E_ResourceMsg_AllUnloaded:
	//	{
	//		if (m_OnlyUnloadResources)
	//		{
	//			SetState(false);
	//			SetCollisions(false);
	//			RemoveSubscriber( GameMessaging::E_GameMsg_PreUpdate );
	//			RemoveSubscriber( GameMessaging::E_GameMsg_Update );
	//			m_ResourcesUnloaded = true;
	//		}
	//		else
	//		{
	//			Destroy();
	//		}
	//		
	//	}
	//	return true;

	case Game::E_VehicleMessage_DebugRender:
		{
			DisplayDebug();
		}
		return true;

	case Game::E_VehicleMessage_RaceStarted:
		{
			ResetInitialStates();
			m_RaceStarted = true;
			m_ControlsEnabled = true;
		}
		return true;

	case ActorMsg::E_CutSceneMsg_Start321:
		{
			ResetInitialStates();
			m_ControlsEnabled = true;
		}
		return true;

	case Game::E_VehicleMessage_ControlsDisable:			
		{
			const Game::Actors::C_VehicleControlsDisableMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleControlsDisableMsg>(pMessage);
			if(m_VehicleIndex == pMsg->m_VehicleIndex || pMsg->m_VehicleIndex == -1)
			{
				m_ControlsEnabled = false;
			}
		}
		return true;

	case Game::E_VehicleMessage_ControlsEnable:			
		{
			const Game::Actors::C_VehicleControlsEnableMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleControlsEnableMsg>(pMessage);
			if(GetCreationData()->GetActorHandle() == pMsg->m_ActorHandle && m_VehicleIndex == pMsg->m_VehicleIndex)
			{
				m_ControlsEnabled = true;
			}
		}
		return true;

	/*case Game::E_VehicleMessage_FatallyDamaged:
		{
			this->m_pConstraint->SetScale(0.f);
			this->m_GroundedTimer = 0.f;
			return true;
		}*/

	case Game::E_VehicleMessage_ResetFatalDamage:
		{
			const Game::Actors::C_VehicleResetFatalDamageMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleResetFatalDamageMsg>(pMessage);
			if(GetCreationData()->GetActorHandle() == pMsg->m_ActorHandle && m_VehicleIndex == pMsg->m_VehicleIndex && m_pVehicleModel && m_pVehicleModel->GetDamageLayer())
			{
				m_ControlsEnabled = true;

				m_pVehicleModel->GetDamageLayer()->Repair(pMsg->m_RepairAmount, 0.0f, (float)g_debugRepairTransition);		

//				m_RadiatorSmokeEmitter.Destroy();
				m_EngineSmokeLightEmitter.Destroy();
				m_EngineSmokeHeavyEmitter.Destroy();
//				m_EngineFireEmitter.Destroy();

				m_FatallyDamaged	= false;

				m_pVehicleModel->GetDamageLayer()->SetFatalDamageFlag(m_FatallyDamaged);
			}

			C_DriverManager::Instance()->ResetDriver(m_VehicleIndex);

			if (pMsg->m_ActorHandle == AmaxCam::CameraSystem().FocusHandle())
			{
				AmaxCam::CameraSystem().StopFatalCamera();
				AmaxCam::CameraSystem().ResetVehicleFeed();
			}
		}
		return true;

	case Game::E_VehicleMessage_RepairDamage:
		{
			const Game::Actors::C_VehicleRepairDamageMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleRepairDamageMsg>(pMessage);
			if(GetCreationData()->GetActorHandle() == pMsg->m_ActorHandle && m_VehicleIndex == pMsg->m_VehicleIndex && m_pVehicleModel && m_pVehicleModel->GetDamageLayer())
			{
				m_pVehicleModel->GetDamageLayer()->Repair(pMsg->m_RepairAmount, pMsg->m_CurrentDamagePercentage, pMsg->m_RepairDuration);

//				m_RadiatorSmokeEmitter.Destroy();
				m_EngineSmokeLightEmitter.Destroy();
				m_EngineSmokeHeavyEmitter.Destroy();
//				m_EngineFireEmitter.Destroy();
			}
		}
		return true;

	case Game::E_VehicleMessage_ApplyShockedEffect:
		{
			const Game::Actors::C_VehicleApplyShockedEffectMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleApplyShockedEffectMsg>(pMessage);
			if(m_VehicleIndex == pMsg->mVehicleIndex && !m_EmpState.m_bEmp)
			{
				m_EmpState.m_fEmpMaxTime	= pMsg->mMaxTime;
				m_EmpState.m_fEmpTime		= 0.0f;
				m_EmpState.m_bEmp			= true;
				m_EmpState.m_fBrake			= m_pVehicleModel->GetModelInput()->m_fBrake;
				m_EmpState.m_bRearLights	= m_pVehicleModel->GetModelInput()->m_bRearLights.m_Current;
				m_EmpState.m_bHeadLights	= m_pVehicleModel->GetModelInput()->m_bHeadLights.m_Current;
				m_EmpState.m_bFogLights		= m_pVehicleModel->GetModelInput()->m_bFogLights.m_Current;
			}
		}
		return true;


	case Actor::E_ActorMsg_PhysicsBodyEvent:
		return true;

//	Replay messages

	case Game::E_ReplayMessage_UpdateVehicleState:
		{
			//const Game::C_ReplayPacketVehicleState* pMsg = Messaging::MessageCast<Game::C_ReplayPacketVehicleState>(pMessage);
			const Game::C_ReplayMsg_VehicleStateMsg* pMsg = Messaging::MessageCast<Game::C_ReplayMsg_VehicleStateMsg>(pMessage);
			m_pVehiclePhysics->ProcessNetworkState(pMsg->m_CompressedNetworkState, true);
			//m_pVehiclePhysics->ProcessReplayState(pMsg->m_CompressedNetworkState);
		}
		return true;

	case Game::E_ReplayMessage_UpdateControls:
		{
			//const Game::C_ReplayPacketControls* pMsg = Messaging::MessageCast<Game::C_ReplayPacketControls>(pMessage);
			const Game::C_ReplayMsg_ControlsMsg* pMsg = Messaging::MessageCast<Game::C_ReplayMsg_ControlsMsg>(pMessage);
/*
			// Make sure applying the brake doesn't rev the engine
			const RacingPhysics::CarNetworkControls& networkControls = (const RacingPhysics::CarNetworkControls&) pMsg->m_CompressedControls;
			F32 brake = UncompressFloat<U32, 8>(networkControls.brake, 0.f, 1.f);
			if (brake > 0.05f)
				brake = 0.05f;
			networkControls.brake = CompressFloat<U32, 8>(brake, 0.f, 1.f);
*/

			m_pVehiclePhysics->SetNetworkControls(pMsg->m_CompressedControls);
			m_pVehiclePhysics->EnsureReducedBrakeValue();
		}
		return true;

	case Game::E_ReplayMessage_SetVehicleDamage:
		{
			const Game::C_ReplayMsg_SetVehicleDamageMsg* pMsg = Messaging::MessageCast<Game::C_ReplayMsg_SetVehicleDamageMsg>(pMessage);
			if(m_VehicleIndex == pMsg->m_VehicleIndex)
			{
				SetDamageState(pMsg->m_DamageState);			
			}
		}
		return true;


	case ActorMsg::E_DynamicObjMsg_VehicleEnteredTrigger:
		{
			const ActorMsg::C_VehicleEnteredTriggerMsg* pMsg = Messaging::MessageCast<ActorMsg::C_VehicleEnteredTriggerMsg>(pMessage);

			if(pMsg->m_TriggerType == E_TRIGGER_TYPE_RACERULES)
			{
				C_RaceLogicLayer* pRaceLogicLayer	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
				C_VehicleLogicActor* pLogicActor	= pRaceLogicLayer->GetLogicVehicleActor(m_VehicleIndex);
				if (pLogicActor)
				{
					pLogicActor->mPositionInfo.mCurrentRaceRuleArea = pMsg->m_RefNumber;
					pLogicActor->mPositionInfo.mLastRaceRuleArea	= pMsg->m_RefNumber;
				}
			}
#if 0
			else
			{
				// Pass on to the UI
				UI::C_UiPlayerDirectionBoxTrigger msg( m_VehicleIndex, true, pMsg->m_TriggerType );
				_UI_MESSAGE_FULL( msg );
			}
#endif
		}
		return true;

	case ActorMsg::E_DynamicObjMsg_VehicleExitedTrigger:
		{
			const ActorMsg::C_VehicleExitedTriggerMsg* pMsg = Messaging::MessageCast<ActorMsg::C_VehicleExitedTriggerMsg>(pMessage);

			if(pMsg->m_TriggerType == E_TRIGGER_TYPE_RACERULES)
			{
				C_RaceLogicLayer* pRaceLogicLayer	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
				C_VehicleLogicActor* pLogicActor	= pRaceLogicLayer->GetLogicVehicleActor(m_VehicleIndex);
				if (pLogicActor)
					pLogicActor->mPositionInfo.mCurrentRaceRuleArea = -1;
			}
#if 0
			else
			{
				// Pass on to the UI
				UI::C_UiPlayerDirectionBoxTrigger msg( m_VehicleIndex, false, pMsg->m_TriggerType );
				_UI_MESSAGE_FULL( msg );
			}
#endif
		}
		return true;

	case Game::E_GameLogicMessage_Impact:
		{
			Game::C_ImpactMsg* pMsg = const_cast<Game::C_ImpactMsg*>(Messaging::MessageCast<Game::C_ImpactMsg>(pMessage));

			HandleCollision(pMsg);

			C_DriverManager::Instance()->OnCollision(m_VehicleIndex, pMsg);

			if (Game::C_CrowdGameManager::Instance())
				Game::C_CrowdGameManager::Instance()->OnImpact(m_VehicleIndex, pMsg);
		}
		return true;

	case ActorMsg::E_DynamicObjMsg_VehicleHandOfGodCameraEnteredArea:
		{
			const ActorMsg::C_VehicleHandOfGodResetCameraEnteredArea* pMsg = Messaging::MessageCast<ActorMsg::C_VehicleHandOfGodResetCameraEnteredArea>(pMessage);

			C_RaceLogicLayer* pRaceLogicLayer		= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			const C_VehicleLogicActor* pLogicActor	= pRaceLogicLayer->GetLogicVehicleActor(m_VehicleIndex);

			if (!pLogicActor)
				return true;

			C_VehicleResetEnteredCameraZone vehicleResetCameraEnterMsg(pMsg->mCameraFollow);

			if( pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_HandOfGod) )
				GetActorList()->SendMessageToActorComponent( pLogicActor->GetHandle(), Components::E_IF_VehicleComponent_HandOfGod, &vehicleResetCameraEnterMsg );
		}
		return true;

	case ActorMsg::E_DynamicObjMsg_VehicleHandOfGodForceResetArea:
		{
			return HandleHoGForceResetArea(pMessage);
		}

	case ActorMsg::E_DynamicObjMsg_VehicleHandOfGodCameraForceArea:
		{
			const ActorMsg::C_VehicleHandOfGodResetCameraForceArea* pMsg = Messaging::MessageCast<ActorMsg::C_VehicleHandOfGodResetCameraForceArea>(pMessage);

			C_RaceLogicLayer* pRaceLogicLayer		= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			const C_VehicleLogicActor* pLogicActor	= pRaceLogicLayer->GetLogicVehicleActor(m_VehicleIndex);

			if (!pLogicActor)
				return true;

			C_VehicleResetForceCameraZone vehicleResetCameraForceMsg(pMsg->mCameraFollow);

			if( pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_HandOfGod) )
				GetActorList()->SendMessageToActorComponent( pLogicActor->GetHandle(), Components::E_IF_VehicleComponent_HandOfGod, &vehicleResetCameraForceMsg );
		}
		return true;

	case ActorMsg::E_DynamicObjMsg_VehicleHandOfGodCameraExitedArea:
		{
			C_VehicleResetExitedCameraZone vehicleResetCameraExitMsg;

			C_RaceLogicLayer* pRaceLogicLayer		= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			const C_VehicleLogicActor* pLogicActor	= pRaceLogicLayer->GetLogicVehicleActor(m_VehicleIndex);

			if (!pLogicActor)
				return true;

			if( pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_HandOfGod) )
				GetActorList()->SendMessageToActorComponent( pLogicActor->GetHandle(), Components::E_IF_VehicleComponent_HandOfGod, &vehicleResetCameraExitMsg );
		}
		return true;

	case ActorMsg::E_DynamicObjMsg_VehicleHandOfGodTwoWayArea:
		{
			const ActorMsg::C_VehicleHandOfGodResetTwoWayArea* pMsg = Messaging::MessageCast<ActorMsg::C_VehicleHandOfGodResetTwoWayArea>(pMessage);

			C_RaceLogicLayer* pRaceLogicLayer		= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			const C_VehicleLogicActor* pLogicActor	= pRaceLogicLayer->GetLogicVehicleActor(m_VehicleIndex);

			if (pLogicActor && pMsg->mResetPositionActor)
			{
				C_VehicleResetTwoWayZone vehicleResetTwoWayMsg( pMsg->mTimer, pMsg->mResetPositionActor->GetCreationMatrix().AW(), 
					pMsg->mResetPositionActor->GetCreationMatrix().AZ(), pMsg->mZoneId, pMsg->mIndivId );

				if( pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_HandOfGod) )
					GetActorList()->SendMessageToActorComponent( pLogicActor->GetHandle(), Components::E_IF_VehicleComponent_HandOfGod, &vehicleResetTwoWayMsg );
			}
		}
		return true;

	case ActorMsg::E_DynamicObjMsg_VehicleHandOfGodEntryArea:
		{
			const ActorMsg::C_VehicleHandOfGodResetEntryArea* pMsg = Messaging::MessageCast<ActorMsg::C_VehicleHandOfGodResetEntryArea>(pMessage);

			C_RaceLogicLayer* pRaceLogicLayer		= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			const C_VehicleLogicActor* pLogicActor	= pRaceLogicLayer->GetLogicVehicleActor(m_VehicleIndex);
			if (pLogicActor && pMsg->mResetPositionActor)
			{
				C_VehicleResetEntryZone vehicleResetEntryMsg( pMsg->mTimer, pMsg->mResetPositionActor->GetCreationMatrix().AW(), 
															  pMsg->mResetPositionActor->GetCreationMatrix().AZ(), pMsg->mZoneId );

				if( pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_HandOfGod) )
					GetActorList()->SendMessageToActorComponent( pLogicActor->GetHandle(), Components::E_IF_VehicleComponent_HandOfGod, &vehicleResetEntryMsg );
			}
		}
		return true;

	case ActorMsg::E_DynamicObjMsg_VehicleHandOfGodExitArea:
		{
			const ActorMsg::C_VehicleHandOfGodResetExitArea* pMsg = Messaging::MessageCast<ActorMsg::C_VehicleHandOfGodResetExitArea>(pMessage);

			C_RaceLogicLayer* pRaceLogicLayer		= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			const C_VehicleLogicActor* pLogicActor	= pRaceLogicLayer->GetLogicVehicleActor(m_VehicleIndex);
			if (pLogicActor)
			{
				C_VehicleResetExitZone vehicleResetExitMsg( pMsg->mZoneId );

				if( pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_HandOfGod) )
					GetActorList()->SendMessageToActorComponent( pLogicActor->GetHandle(), Components::E_IF_VehicleComponent_HandOfGod, &vehicleResetExitMsg );
			}
		}
		return true;

	case ActorMsg::E_DynamicObjMsg_VehicleSloMoEntry:
		{
			//	[GavinY] - I regret nothing! Apart from this horrible hack that will disappear once MP has separate actor lists... (promise)
			if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() != Game::E_GameMode_SinglePlayer && 
			   C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() != Game::E_GameMode_Debug)
				return true;

			const ActorMsg::C_VehicleSloMoEntryMsg* pMsg = Messaging::MessageCast<ActorMsg::C_VehicleSloMoEntryMsg>(pMessage);
			if (pMsg && !pMsg->m_Disable && !pMsg->m_Manual)
			{
				if (pMsg->m_MinSpeed != pMsg->m_MaxSpeed)
				{
					float fSpeed = m_pVehicleModel->GetModelInput()->m_fSpeed;
					if (fSpeed < pMsg->m_MinSpeed || fSpeed > pMsg->m_MaxSpeed)
						return true;
				}				

				
				C_RaceLogicLayer* pRaceLogicLayer		= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
				const C_VehicleLogicActor* pLogicActor	= pRaceLogicLayer->GetLogicVehicleActor(m_VehicleIndex);
				if (pLogicActor)
				{
					if (pMsg->m_Nitro && pLogicActor->mPickupInfo.mCurrentlyUsingType != E_PICKUP_TYPE_NITROUS)
						return true;		

					C_GameApp::GetInstance()->SetTimeScale(pMsg->m_TimeScale);
					C_GameApp::GetInstance()->SetTimeScaleTimeOut(pMsg->m_Duration);
					AmaxCam::CameraSystem().StartSetPieceCamera(pMsg->m_CameraPosition, pMsg->m_CameraLookAt, pMsg->m_CameraFOV, pMsg->m_TrackCar);

					C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(pMsg);
					
					if(QueryInterface(Components::E_AudioEntity_AdditionalSpawned)) //if present
						GetActorList()->SendMessageToActorComponent(GetHandle(),Components::E_AudioEntity_AdditionalSpawned, ActorMsg::E_DynamicObjMsg_VehicleAudioSloMoEntry);
				}
			}
		}
		return true;

	case ActorMsg::E_DynamicObjMsg_VehicleSloMoExit:
		{
			//	[GavinY] - I regret nothing! Apart from this horrible hack that will disappear once MP has separate actor lists... (promise)
			if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() != Game::E_GameMode_SinglePlayer && 
			   C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() != Game::E_GameMode_Debug)
				return true;

			const ActorMsg::C_VehicleSloMoExitMsg* pMsg = Messaging::MessageCast<ActorMsg::C_VehicleSloMoExitMsg>(pMessage);
			if (pMsg && !pMsg->m_Disable)
			{
				if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer())
					C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->ExitSloMo();
				C_GameApp::GetInstance()->SetTimeScale(1.0f);
			}
		}
		return true;

	case ActorMsg::E_DynamicObjMsg_VehicleAchieveAmboyOilfields:
		{
			Game::C_StickerManager::Instance().CheckAmboyOilfieldsTriggerRegion(m_VehicleIndex);
		}
		return true;

	case ActorMsg::E_DynamicObjMsg_VehicleAchieveGasStation:
		{
			Game::C_StickerManager::Instance().CheckGasStationTriggerRegion(m_VehicleIndex);
		}
		return true;

	case Game::E_VehicleMessage_StopWreckedEffect:
		{
			m_fHealthEffectTime = 0.0f;
			if (m_pHealthSpecialFX)
			{
				m_pHealthSpecialFX->Stop();
				SAFE_DELETE_PTR(m_pHealthSpecialFX)
			}				
		}
		return true;

	case Game::E_VehicleMessage_KillNitro:
		{
			this->m_FakeVehicleSpeed.Stop();
			return true;
		}

	case ::Actors::E_VisibilityMsg_OnHidden:
		return true;
	case ::Actors::E_VisibilityMsg_OnVisible:
		return true;

	case ::ActorMsg::E_ResourceMsg_Loaded:
		return true;

	default:				// Send any messages that aren't handled to the vehicle proxy (if it has one)
		SendMessageToActor(m_pProxyActor, pMessage);
		break;

	}

	return C_GameActor::OnMessage(pMessage);
}

void C_VehicleActor::ForceAIControl(bool bChangeDynamics)
{
	if (!m_pVehiclePhysics)
		return;

	if (bChangeDynamics)
		m_pVehiclePhysics->ForceAIControl();

	AIAmax::AmaxAIR::Instance().AITakeControl(m_VehicleIndex);
}

void C_VehicleActor::ForcePlayerControl(bool bChangeDynamics)
{
	if (!m_pVehiclePhysics)
		return;

	if (bChangeDynamics)
		m_pVehiclePhysics->ForcePlayerControl();

	AIAmax::AmaxAIR::Instance().AIRelinquishControl(m_VehicleIndex);
}

void C_VehicleActor::TriggerExhaustPop(Game::Actors::C_VehicleExhaustPopMsg* p_pMsg)
{
	const Game::Actors::C_VehicleExhaustPopMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleExhaustPopMsg>(p_pMsg);
	if (p_pMsg->m_VehicleIndex == m_VehicleIndex && m_EnableParticles)
		m_ExhaustEffects.SpawnEffect((Game::Effects::E_ExhuastEffectType)p_pMsg->m_ExhaustPopType, m_ParticleEmitScale, m_EffectsVisiblityFlags);
}

const char*	 C_VehicleActor::GetActorName() const
{
	static Util::C_String32 name;
	name.Printf("VehicleActor %d", m_VehicleIndex);
	return name.Chars();
}

	void										SetOverideControls(RacingPhysics::CarControls& controls);


void C_VehicleActor::SetOverideControls(RacingPhysics::CarControls& controls)				// Stores overide controls
{
	m_overide_controls = true;
	m_new_controls = controls;
}

F32 C_VehicleActor::CheckRoadSweeperMultiplier(Game::C_LevelState& levelState) const
{
	if(!levelState.mCars.Empty())
	{
		Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = levelState.mCars[m_VehicleIndex].mUpgrades.GetMultiplayerAbilities();
		BIZ_ASSERT_MSG(pAbilities, "C_VehicleActor::CheckRoadSweeperMultiplier > Could not find multiplayer abilities, Road Sweeper mod skipped");
		Game::C_MultiplayerVehicleAbility* roadSweeper = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Road_Sweeper);
		BIZ_ASSERT_MSG(roadSweeper, "C_VehicleActor::CheckRoadSweeperMultiplier > Could not find multiplayer ability, Road Sweeper mod skipped");
		if(roadSweeper && roadSweeper->GetActive())
			return roadSweeper->GetValue();
	}

	return 1.f; //it's a multiplier, remember!
}

void C_VehicleActor::CheckOverideControls(RacingPhysics::CarControls& controls)				// Handles any perk or cutscene specific control overrides
{
	// Perk overides
	if (m_PerformanceModifiers.mCutThrottle.mModifier)
	{
		controls.throttle	= 0.f;
		controls.brake		= g_debugPerkShockedBrakeForce;
	}

	if (m_PerformanceModifiers.mClampSteering.mModifier)
	{
		controls.steer = m_PerformanceModifiers.mClampedSteeringAngle.mModifier;
	}

	// Cutscene overides
	if (m_overide_controls)
	{
		controls.brake = m_new_controls.brake;
		controls.handbrake = m_new_controls.handbrake;
		controls.steer = m_new_controls.steer;
		controls.throttle = m_new_controls.throttle;

//		m_overide_controls = false;
	}
}

const AmaxCam::C_VehicleFeedParam* C_VehicleActor::GetCameraParam() const
{
	AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];
	if (!pVehicle)
		return 0;

	if (C_GameVehicleData::Instance().mVehicleDefaults.Number() > 0 && !pVehicle->m_CameraParam)
		return C_GameVehicleData::Instance().mVehicleDefaults[0]->defaultCamera;

	return pVehicle->m_CameraParam;
}

#ifndef CFG_SPEED
Debug::Bool g_debugUpdateLightFlares("Vehicle.Lights.UpdateFromSorcery", false);
#endif
Debug::Bool g_debugDisableLightFlares("Vehicle.Lights.DisableFlares", false);
// knock this out since it seems were sticking with flares on focus car only?
//Debug::Bool g_debugOnlyFlaresForHumans("Vehicle.Lights.OnlyHumansHaveFlares", true);

void C_VehicleActor::UpdateLightFlares()
{
	PERF_EVENT("UpdateLightFlares");

	if (!m_pVehicleModel)
		return;

#ifndef CFG_SPEED
	if (g_debugDisableLightFlares)
		return;
#endif
	
#ifndef CFG_SPEED
	Vehicle::XtVehicle* pVehicle = NULL;
	if (g_debugUpdateLightFlares)
		pVehicle = &C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex]->baseVehicle;		
#endif

	BIZ_ASSERT(!m_IsSplitscreen);

	//Calculate distant to main camera
	Maths::HVec3 cameraCarVector = AmaxCam::CameraSystem().DebugFeed(0).Live().Position() - m_Matrix.Translation();
	F32 distant = (cameraCarVector).Length();
	cameraCarVector *= 1.f / distant;

	bool flareVisible;
	flareVisible = (distant < Maths::Lerp(g_DefaultLightFlareFarDistantBadPerf, g_DefaultLightFlareFarDistant, Lod::C_LodManager::Instance().GetCpuDetail())) && m_EnableParticles;

	//Calculate orientation with camera
	F32 cameraOrientation = m_Matrix.AZ() | cameraCarVector;  //Positive->Front flares

	F32 fDefaultOcclusion = m_IsFocusVehicle ? 1.0f : 0.0f;

	//Fill matrix position and intensity
	for (int i = 0; i < m_pVehicleModel->GetLightFlares()->m_LightFlareVector.Size(); i++)
	{
		Vehicle::C_Flare& flare = m_pVehicleModel->GetLightFlares()->m_LightFlareVector[i];

		//Calculate orientation and kill the effect
		if (flare.m_Handle && flareVisible && !(flare.m_Inverted && cameraOrientation > g_DefaultLightFlareExtraAngle) && !(!flare.m_Inverted && cameraOrientation < -g_DefaultLightFlareExtraAngle))
		{
			float intensity = 1.0f;
			if (flare.m_pNode && flare.m_pNode->m_pMaterialParameters && 
				flare.m_pNode->m_pMaterialParameters->m_Type == Vehicle::E_MaterialType_Light)
			{
				Vehicle::C_MaterialLight* pMaterial = (Vehicle::C_MaterialLight*)flare.m_pNode->m_pMaterialParameters;
				intensity = pMaterial->FlareIntensity();
			}

			if (flare.m_pNode->GetVisible() && (intensity > 0.05f))
			{
				flare.m_Handle->m_VisibilityFlags = 0xFFFFFFFF;

				//Set position and rotation
				Maths::HVec3 vPosition = flare.m_pNode->GetCentre();
				
				if (flare.m_pNode->m_pParent && flare.m_pNode->m_pParent->m_bActive && flare.m_pNode->m_pParent->m_pPhysicsBody)
				{
					vPosition = vPosition * flare.m_pNode->m_pParent->m_pPhysicsBody->ElementMatrix();
					vPosition = vPosition * m_Matrix.InverseRotTrans();				
				}

				if (flare.m_pDamageNode)
					vPosition += flare.m_pDamageNode->m_OffsetVector;
				
				Maths::C_Quaternion qRotation = (!flare.m_Inverted) ?	Maths::C_Quaternion(0.707106769f, Maths::Constants<Maths::HF32>::Zero(), Maths::Constants<Maths::HF32>::Zero(), 0.707106769f) :
																	Maths::C_Quaternion(0.707106769f, Maths::Constants<Maths::HF32>::Zero(), Maths::Constants<Maths::HF32>::Zero(), -0.707106769f);;

			#ifndef CFG_SPEED
				if (pVehicle)
					flare.m_RenderOffset = flare.m_Inverted ? pVehicle->m_RearLightRenderOffset : pVehicle->m_FrontLightRenderOffset;
			#endif

				Maths::HVec3 vOffset(Maths::Constants<Maths::HF32>::Zero(), Maths::Constants<Maths::HF32>::Zero(), flare.m_RenderOffset);

				qRotation = m_RotationQuaternion * qRotation;
				vPosition = vPosition + vOffset;
				vPosition = vPosition * m_Matrix;

				flare.m_Node.SetPosition(vPosition);
				flare.m_Node.SetRotation(qRotation);
				
				flare.m_Handle->m_fIntensity = intensity * GetAlpha(E_VAC_TYPE_FLARES);
				flare.m_Handle->m_fDefaultOcclusion = fDefaultOcclusion;

				continue;
			}
		}

		//Destroy
		if (flare.m_Handle)
			flare.m_Handle->m_VisibilityFlags = 0;
	}
}

void C_VehicleActor::UpdateDynamicLights()
{
	PERF_EVENT("UpdateDynamicLights");
	
#ifdef PLATFORM_PS3
	if (m_IsSplitscreen)
		return;
#endif
	
	F32 fVehicleAlpha = GetAlpha(E_VAC_TYPE_DYNAMIC_LIGHTS);
	F32 fBreakIntensity = m_pVehicleModel->GetBreakLightIntensity() * fVehicleAlpha;
	F32 fRearIntensity = m_pVehicleModel->GetRearLightIntensity() * fVehicleAlpha;
	F32 fReverseIntensity = m_pVehicleModel->GetReverseLightIntensity() * fVehicleAlpha;


	//Update Rear Light
	Maths::HVec3 rearLightColour(0.f, 0.f, 0.f);
	rearLightColour += Maths::HVec3(g_DefaultRearLightColour->r * g_DefaultRearLightColour->a * fRearIntensity, g_DefaultRearLightColour->g * g_DefaultRearLightColour->a * fRearIntensity, g_DefaultRearLightColour->b * g_DefaultRearLightColour->a * fRearIntensity);
	rearLightColour += Maths::HVec3(g_DefaultBreakLightColour->r * g_DefaultBreakLightColour->a * fBreakIntensity, g_DefaultBreakLightColour->g * g_DefaultBreakLightColour->a * fBreakIntensity, g_DefaultBreakLightColour->b * g_DefaultBreakLightColour->a * fBreakIntensity);
	rearLightColour += Maths::HVec3(g_DefaultReverseLightColour->r * g_DefaultReverseLightColour->a * fReverseIntensity, g_DefaultReverseLightColour->g * g_DefaultReverseLightColour->a * fReverseIntensity, g_DefaultReverseLightColour->b * g_DefaultReverseLightColour->a * fReverseIntensity);

	if ( SceneGraph::C_PointLight* pLight = m_RearLightHandle.Object())
	{
		pLight->SetColour(rearLightColour);
		pLight->SetCullDistance(g_DynamicLightCullDistant);
		pLight->SetFlags(1<<m_VehicleIndex);
		const Maths::HVec3 position(Maths::HVec3(0.f, F32(g_DefaultLightOffsetY), -m_pVehicleModel->GetDamageLayer()->GetVehicleInfo()->m_Extents.Z() + g_DefaultLightOffsetZ) * m_Matrix);

		pLight->SetDimensions(position, g_DefaultLightRadius);
		pLight->SetVisibilityFlags(m_EffectsVisiblityFlags);
	}
	
}

Debug::Float g_debugAuxFlareIntensity("Vehicle.Lights.AuxFlareIntensity", 2.0f, Debug::Range(0.0f, 2.0f), Debug::Step(0.001f));

void C_VehicleActor::UpdateAuxFlares()
{
	PERF_EVENT("UpdateAuxFlares");


	//Get Colours
	const C_VehicleActorCreate *pVehicleCreate = (const C_VehicleActorCreate *)(GetCreationData().GetData());
	Maths::C_RGBA_F32 whiteFlareColour = pVehicleCreate->m_pGameWorld->GetCarLightFlareSettings().m_CarWhiteFlaresColour;
	Maths::C_RGBA_F32 redFlareColour = pVehicleCreate->m_pGameWorld->GetCarLightFlareSettings().m_CarRedFlaresColour;

	for (int i = 0; i < m_pVehicleModel->GetAuxFlares()->m_AuxFlareVector.Size(); i++)
	{
		Vehicle::C_AuxFlare& auxFlare = m_pVehicleModel->GetAuxFlares()->m_AuxFlareVector[i];

		if ((m_pVisDetect->IsVisible() || m_pReflectionVisDetect->IsVisible()) && auxFlare.m_pNode->GetVisible())
		{
			float intensity = 1.0f;
			if (auxFlare.m_pNode->m_pMaterialParameters && 
				auxFlare.m_pNode->m_pMaterialParameters->m_Type == Vehicle::E_MaterialType_Light)
			{
				Vehicle::C_MaterialLight* pMaterial = (Vehicle::C_MaterialLight*)auxFlare.m_pNode->m_pMaterialParameters;
				intensity = Maths::Min(pMaterial->Intensity(), g_debugAuxFlareIntensity) * GetAlpha(E_VAC_TYPE_FLARES);
			}
			
			if(intensity > 0.05f)
			{
				//Update flare
				Maths::HMat43 flareMatrix = auxFlare.m_Matrix * m_Matrix;
				auxFlare.m_pAuxFlareInstance->UpdatePosition(flareMatrix.Translation());
				auxFlare.m_pAuxFlareInstance->UpdateDirection(flareMatrix.AZ());
				auxFlare.m_pAuxFlareInstance->UpdateIntensity(intensity * auxFlare.m_Intensity);
				
				switch (auxFlare.m_FlareColour)
				{
				case Vehicle::E_AuxFlareColour_White:
					auxFlare.m_pAuxFlareInstance->UpdateColour(whiteFlareColour);
					break;
				case Vehicle::E_AuxFlareColour_Red:
					auxFlare.m_pAuxFlareInstance->UpdateColour(redFlareColour);
					break;
				}

				if(m_pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Silent_Running)->GetActive())
					auxFlare.m_pAuxFlareInstance->SetViewFlags((m_pVisDetect->GetVisibleFlags() & ~Game::E_ViewTypeFlag_Main & ~Game::E_ViewTypeFlag_Mirror) | m_pReflectionVisDetect->GetVisibleFlags());
				else if (!m_IsSplitscreen)
					auxFlare.m_pAuxFlareInstance->SetViewFlags((m_pVisDetect->GetVisibleFlags() & ~Game::E_ViewTypeFlag_Main) | m_pReflectionVisDetect->GetVisibleFlags());
				else
					auxFlare.m_pAuxFlareInstance->SetViewFlags(m_pVisDetect->GetVisibleFlags() | m_pReflectionVisDetect->GetVisibleFlags());
			}
			else
				auxFlare.m_pAuxFlareInstance->SetViewFlags(0);
		}
		else
			auxFlare.m_pAuxFlareInstance->SetViewFlags(0);
	}
}

void C_VehicleActor::UpdateSizeOnScreen()
{
	F32 size = 0.0f;
	S32 numViews = m_pViewManager->GetNumViews();
	PERF_EVENT("UpdateSizeOnScreen");
	if (m_IsBumperCamera && !m_IsSplitscreen)
		size = 1.f;
	else 
	for(S32 i = 0; i < numViews; ++i)
	{
		const SceneGraph::C_View& view = m_pViewManager->GetView(i)->GetMainView();
		if ((m_Visible & (1 << i)) == 0)
		{
			//Not visible, use frustum
			for (U32 m = 0; m < m_pVehicleModel->GetNumModelInstances(); ++m)
			{
				if(m_pVehicleModel->GetModelInstanceType(m) == Vehicle::E_ModelInstanceType_Exterior)
				{
					if (view.GetViewFrustum().CullSphere(m_pVehicleModel->GetModelInstance(m)->GetWorldSphere()))
						size = Maths::Max(size, view.GetViewSettings3D().GetScreenSizeRelative(m_pVehicleModel->GetModelInstance(m)->GetWorldSphere()));
				}
			}
		}
		else
		//It is visible, use visibility detect
		if(m_pVisDetect->IsVisible(g_MainViewFlags[i]))
		{
			for (U32 m = 0; m < m_pVehicleModel->GetNumModelInstances(); ++m)
			{
				if(m_pVehicleModel->GetModelInstanceType(m) == Vehicle::E_ModelInstanceType_Exterior)
				{
					size = Maths::Max(size, view.GetViewSettings3D().GetScreenSizeRelative(m_pVehicleModel->GetModelInstance(m)->GetWorldSphere()));
				}
			}
		}
	}
	m_ScreenSize = size;
}

bool C_VehicleActor::IsVisible()
{
	if (!m_pVisDetect)
		return false;

	return (m_pVisDetect->IsVisible() > 0);
}

void C_VehicleActor::UpdateLightCubeMap(const Cam2::C_Camera& camera)
{
	if (!m_pVisDetect)
		return;

	//If visible
	if (m_pVisDetect->IsVisible(E_ViewTypeFlag_Main))
	{
		//Check camera distant
		Maths::HVec3 cameraRefPosition = camera.Position();
		F32 distant = (cameraRefPosition - m_Matrix.AW()).Length();

		if (distant < 60.f)
		{

			//Check frecuency update
			m_UpdateLightCubeMapCount++;

			if (m_UpdateLightCubeMapCount >= m_UpdateLightCubeMapFrecuency)
			{
				//Double buffer, inc and set the new to render instance data
				m_CurrentLightCubeMapIndex = (m_CurrentLightCubeMapIndex + 1) % 2;

				//Set cubemap to render instance data
				m_pVehicleModel->SetLightEnvironmentTexture(m_LightCubeMapTexture[m_CurrentLightCubeMapIndex]);

				m_NeedUpdateLightCubeMap = true;
				m_UpdateLightCubeMapCount = 0;
				return;
			}
		}
	}

	m_NeedUpdateLightCubeMap = false;
}

void C_VehicleActor::ApplyWreckedImpulse( F32 scale )
{
	// Stop any fake physics
	GetFakePhysics()->Stop();

	// Apply some rotation
	Gravity::C_Body* pBody = GetCollisionBody();

	F32 randX = g_VehicleRandom.SignedFRand();
	F32 randY = g_VehicleRandom.SignedFRand();
	F32 randZ = g_VehicleRandom.SignedFRand();
	F32 angularVelX = (g_WreckedImpulseMultiplier->vx * randX) + (g_WreckedImpulseMin->vx * (randX<0.0f?-1.0f:1.0f));
	F32 angularVelY = (g_WreckedImpulseMultiplier->vy * randY) + (g_WreckedImpulseMin->vy * (randY<0.0f?-1.0f:1.0f));
	F32 angularVelZ = (g_WreckedImpulseMultiplier->vz * randZ) + (g_WreckedImpulseMin->vz * (randZ<0.0f?-1.0f:1.0f));

	//Last Gasp Mod
	Game::C_LevelState& levelState	= Game::C_Game::GetInstance()->GetLevelState();	
	Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = levelState.mCars[m_VehicleIndex].mUpgrades.GetMultiplayerAbilities();
	BIZ_ASSERT_MSG(pAbilities, "Could Not Get C_LevelStateMultiplayerVehicleAbilities when trying to access the last gasp mod");
	Game::C_MultiplayerVehicleAbility* LastGaspAbility = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Last_Gasp);
	BIZ_ASSERT_MSG(LastGaspAbility, "Could Not Get C_MultiplayerVehicleAbility when trying to access the last gasp mod");
	bool lastGaspActive = LastGaspAbility->GetActive();

	MultiplayerConfig* pMultiplayerConfig = C_MultiplayerData::Instance().m_pConfig;
	BIZ_ASSERT_MSG(pMultiplayerConfig, "Could Not Get Multiplayer Config for Last Gasp Mod");
	LastGaspModSettings& modSettings = pMultiplayerConfig->m_LastGaspModSettings;

	Maths::HVec3 angVelLS = pBody->AngularVelocity() * Maths::HMat33(GetMatrix().InverseRotTrans());
	angVelLS += Maths::HVec3(angularVelX, angularVelY, angularVelZ);

	if( lastGaspActive)// If we have the last gasp mod
	{	
		angVelLS *= modSettings.m_wreckAngularVelocityModifier;
	}
	
	Maths::HVec3 angVelWS = angVelLS *  Maths::HMat33(GetMatrix());
	pBody->AngularVelocity(angVelWS * scale);

	float UpVelocity = g_WreckedImpulseUpVel.Value();


	if(lastGaspActive)// If we have the last gasp mod
	{
		UpVelocity *= modSettings.m_wreckUpVelocityModifier;			
	}

	Maths::HVec3 linVelWS = pBody->LinearVelocity() + Maths::HVec3(0.0f,UpVelocity,0.0f);
	pBody->LinearVelocity(linVelWS * scale);
	
}

Display::C_TextureCube* C_VehicleActor::GetCurrentLightCubeMap()
{
	return m_LightCubeMapTexture[m_CurrentLightCubeMapIndex];
}

bool C_VehicleActor::NeedUpdateLightCubeMap()
{
	return m_NeedUpdateLightCubeMap;
}

void C_VehicleActor::UpdateSHLightData( const F32 additiveSHLightData[16] )
{
	//Calculate rear light
	
	F32 fBreak = (m_pVehicleModel->GetModelInput()->m_fBrake > 0.01f) ? m_pVehicleModel->GetBreakLightIntensity() : 0.f;
	F32 fReverse = m_pVehicleModel->GetReverseLightIntensity();
	
	MathsStorage::Float4 rearLightColour (g_DefaultBreakSelfRearLightColour->r * g_DefaultBreakSelfRearLightColour->a * fBreak + g_DefaultReverseSelfRearLightColour->r * g_DefaultReverseSelfRearLightColour->a * fReverse,
										  g_DefaultBreakSelfRearLightColour->g * g_DefaultBreakSelfRearLightColour->a * fBreak + g_DefaultReverseSelfRearLightColour->g * g_DefaultReverseSelfRearLightColour->a * fReverse,
										  g_DefaultBreakSelfRearLightColour->b * g_DefaultBreakSelfRearLightColour->a * fBreak + g_DefaultReverseSelfRearLightColour->b * g_DefaultReverseSelfRearLightColour->a * fReverse, 0.f);
	
	//Vehicle
	m_pVehicleModel->SetDynamicSHLightShaderParameters(additiveSHLightData, m_Matrix, rearLightColour);

	//Driver, TODO: Add Dashboard lighting
	if (GetActorList())
	{
		//Add Dashboard light
		
		float x = 0.f;
		float y = -1.f;
		float z = -0.f;
		static const float a = 0.5f*sqrtf(1.0f/(float)M_PI);
		static const float b = 0.5f*sqrtf(3.0f/(float)M_PI);

		Maths::HVec4 shPreVector;
		shPreVector.X( a );
		shPreVector.Y( -b*y);
		shPreVector.Z( b*z);
		shPreVector.W( -b*x);

		//Transform the shVector for normal dot product
		Maths::HVec4 shVector;
		const float fC0 = 0.282094777; //1.0f / (2.0f * sqrtf(PI));
		const float fC1 = 0.325734973; //sqrt(3.0f) / (3.0f * sqrtf(PI));
		shVector.W(fC0*shPreVector.X());
		shVector.Y(-fC1*shPreVector.Y());
		shVector.Z(fC1*shPreVector.Z());
		shVector.X(-fC1*shPreVector.W());

		F32 intensity = m_pVehicleModel->GetHeadlight()->m_IntensityModifier;
// 			if (m_pDashBoardNode && (m_pDashBoardNode->m_pMaterialParameters) && (m_pDashBoardNode->m_pMaterialParameters->m_Type == Vehicle::E_MaterialType_Light))
// 			{
// 				Vehicle::C_MaterialLight* pMaterial = (Vehicle::C_MaterialLight*)m_pDashBoardNode->m_pMaterialParameters;
// 				intensity = pMaterial->Intensity();
// 			}

		//Calculate Colour
		Maths::C_RGBA_F32 dashBoardLightColour = Maths::C_RGBA_F32(g_DashBoardDriverLightColour->r * intensity, g_DashBoardDriverLightColour->g * intensity, g_DashBoardDriverLightColour->b * intensity, 1.f);

		//Add to shData
		F32 shData[16];

		//Red
		shData[0] = additiveSHLightData[0] + shVector.X() * dashBoardLightColour.R() ;
		shData[1] = additiveSHLightData[1] + shVector.Y() * dashBoardLightColour.R() ;
		shData[2] = additiveSHLightData[2] + shVector.Z() * dashBoardLightColour.R() ;
		shData[3] = additiveSHLightData[3] + shVector.W() * dashBoardLightColour.R() ;

		//Green
		shData[4] = additiveSHLightData[4] + shVector.X() * dashBoardLightColour.G() ;
		shData[5] = additiveSHLightData[5] + shVector.Y() * dashBoardLightColour.G() ;
		shData[6] = additiveSHLightData[6] + shVector.Z() * dashBoardLightColour.G() ;
		shData[7] = additiveSHLightData[7] + shVector.W() * dashBoardLightColour.G() ;

		//Blue
		shData[8]  = additiveSHLightData[8] + shVector.X() * dashBoardLightColour.B() ;
		shData[9]  = additiveSHLightData[9] + shVector.Y() * dashBoardLightColour.B() ;
		shData[10] = additiveSHLightData[10] + shVector.Z() * dashBoardLightColour.B() ;
		shData[11] = additiveSHLightData[11] + shVector.W() * dashBoardLightColour.B() ;

		C_DriverManager::Instance()->SetDynamicSHLightShaderParameters(m_VehicleIndex, shData);
	}
}

void C_VehicleActor::FindShadowMesh()
{
	//Look for the shadow mesh in the colision
	bool foundShadowCaster = false;

	const Maths::HVec3* shadowPositionVector = NULL;
	U32 shadowPositionSize = 0;
	const U32* shadowIndexVector = NULL;
	U32 shadowIndexSize = 0;

	for (S32 iModel = 0; iModel < m_pVehicleModelResource->GetModelData()->GetModelCount(); iModel++)
	{
		const GameResource::C_Model* pModel = m_pVehicleModelResource->GetModelData()->GetModel(iModel);

		Util::C_String modelName = pModel->GetName();
		modelName.ConvertToLower();
		Maths::HMat33 identity;
		identity.SetIdentity();

		if (modelName == "m_collisionmesh")
		{
			Collision::C_CollisionFilter filter(PhysicsConstants::kVehicleBodyCollisionFilter);
			filter.SetGroupId(m_PhysicsGroupId);
			filter.SetGroupFlags( 0);
			filter.SetGroupMask(filter.GetCollisionMask());

			Gravity::C_CompositeDesc compDesc;
			
			for (S32 iElement = 0; iElement < pModel->GetElementCount(); iElement++)
			{
				const GameResource::C_ModelElement* pElement = pModel->GetElement(iElement);

				Util::C_String elementName = pElement->GetName();
				elementName.ConvertToLower();

				Gravity::C_ImportShape* pImportShape = const_cast<Gravity::C_ImportShape*>( pElement->GetPhysicsImportShape() );

				if (elementName == "m_collisionmesh")
				{
					if (pImportShape)
					{
						if(pImportShape->GetShapeDesc()->m_Type == Collision::E_Composite_Shape)
						{
							Gravity::C_CompositeDesc* pCompositeDesc = (Gravity::C_CompositeDesc*)pImportShape->GetShapeDesc();
							
							if (pCompositeDesc->shapes.Size() > 0)
							{
								//We only support one
								Gravity::C_ShapeDesc* pShape = pCompositeDesc->shapes[0].m_pShapeDesc;

								if(pShape->m_Type == Collision::E_ConvexHull_Shape && !foundShadowCaster)
								{
									Gravity::C_ConvexHullDesc* pHullDesc = (Gravity::C_ConvexHullDesc*)pShape;

									//Found physic mesh, possible colision
									shadowPositionVector = &pHullDesc->m_Points[0];
									shadowPositionSize = pHullDesc->m_Points.Size();
									shadowIndexVector = &pHullDesc->m_Triangles[0].m_IndexA;
									shadowIndexSize = pHullDesc->m_Triangles.Size() * 3;
								}
							}

						}
					}
				}
			}
		}
		else if (modelName == "m_shadowcaster")
		{
			//Only one element
			if (pModel->GetElementCount() == 1)
			{
				const GameResource::C_ModelElement* pElement = pModel->GetElement(0);

				Gravity::C_ImportShape* pImportShape = const_cast<Gravity::C_ImportShape*>( pElement->GetPhysicsImportShape() );
				if (pImportShape)
				{
					if( pImportShape->GetShapeDesc()->m_Type == Collision::E_ConvexHull_Shape)
					{
						Gravity::C_ConvexHullDesc* pHullDesc = (Gravity::C_ConvexHullDesc*)pImportShape->GetShapeDesc();
						foundShadowCaster = true;

						shadowPositionVector = &pHullDesc->m_Points[0];
						shadowPositionSize = pHullDesc->m_Points.Size();
						shadowIndexVector = &pHullDesc->m_Triangles[0].m_IndexA;
						shadowIndexSize = pHullDesc->m_Triangles.Size() * 3;

					}
				}
			}
		}
	}
	
	//If shadow find
	if (shadowPositionVector!= NULL)
	{
		CreateShadowMesh(shadowPositionVector, shadowPositionSize, shadowIndexVector, shadowIndexSize);
	}
	else
	{
		LOGWARN(g_VehicleActorLog, "Car %s doesn't have any shadow mesh", m_ModelFileName.Chars());
	}

}

void C_VehicleActor::CreateShadowMesh( const Maths::HVec3* shadowPositionVector, U32 shadowPositionSize, const U32* shadowIndexVector, U32 shadowIndexSize )
{
//#ifndef PLATFORM_PC
	RESOURCE_MEMORY_ID("BlobShadows");

	m_ShadowVertexCount = shadowPositionSize;
	if (!Display::Device()->CreateVertexBuffer(m_ShadowVertexCount * sizeof(Display::Vertex_XYZ), 0, &m_ShadowVertexBuffer))
		return ;

	volatile Display::Vertex_XYZ * pVertices = (Display::Vertex_XYZ *) m_ShadowVertexBuffer.Lock();
	if (pVertices)
	{
		for (U32 i = 0; i < m_ShadowVertexCount; ++i)
		{
			pVertices[i].x = shadowPositionVector[i].X();
			pVertices[i].y = shadowPositionVector[i].Y();
			pVertices[i].z = shadowPositionVector[i].Z();
		}
		m_ShadowVertexBuffer.Unlock();
	}

	m_ShadowIndexCount = shadowIndexSize;
	if (!Display::Device()->CreateIndexBuffer(m_ShadowIndexCount * sizeof(U16), 0, Display::E_Format_Index16, &m_ShadowIndexBuffer))
		return;

	volatile U16 * pIndices = (U16 *) m_ShadowIndexBuffer.Lock();
	if (pIndices)
	{
		for (U32 i = 0; i < m_ShadowIndexCount; ++i)
		{
			pIndices[i] = (U16)shadowIndexVector[i];
		}
		m_ShadowIndexBuffer.Unlock();
	}
//#endif
}

/*
===============
NAME: Collisions

NOTE: Sets collisions on/off
===============
*/
void C_VehicleActor::SetCollisions(bool p_Status)
{
	if (!m_pCollisionBody || !m_pCollisionBody->GetShape())
		return;

	if (p_Status)
	{
		Collision::C_CompositeShape* pComposite = (Collision::C_CompositeShape*)m_pCollisionBody->GetShape();
		pComposite->m_Filter.SetCollisionFlags(m_BodyCollisionFilter.GetCollisionFlags());	
		pComposite->m_Filter.SetCollisionMask( m_BodyCollisionFilter.GetCollisionMask());	
		pComposite->m_Filter.SetBodyMask(m_BodyCollisionFilter.GetBodyMask()); 
	}
	else
	{
		Collision::C_CompositeShape* pComposite = (Collision::C_CompositeShape*)m_pCollisionBody->GetShape();
		
		pComposite->m_Filter.SetCollisionFlags(pComposite->m_Filter.GetCollisionFlags() & (~(PolyFlags::kCameraSolid | PolyFlags::kVehicleSolid | PolyFlags::kVehicleBlocker | PolyFlags::kDynamicObjectSolid)));
		pComposite->m_Filter.SetCollisionMask(PolyFlags::kCameraSolid);
	}	

	m_CollisionsEnabled = p_Status;
}

/*
===============
NAME: IsCollisionEnabled

NOTE: Returns the state of the collisions on/off
===============
*/
bool C_VehicleActor::IsCollisionEnabled() const
{
	return m_CollisionsEnabled;
}
/*
===============
NAME: SetState

NOTE: Sets collisions + rendering on/off
===============
*/
void C_VehicleActor::SetState(bool p_Status)
{
	if (!m_pCollisionBody)
		return;

	if (p_Status)
		m_pCollisionBody->Activate();
	else
		m_pCollisionBody->Deactivate();
}

void C_VehicleActor::SetDeferredRaytracePolicy(int iPolicy)
{
	m_iDeferredRaytracePolicy = iPolicy;
}

void C_VehicleActor::AddAuxFlare( Vehicle::C_Part* pNode, Maths::HMat33 rotation, Maths::HVec3 offset, const F32 size, F32 intensity, Vehicle::E_AuxFlareColour flareColour )
{
	const C_VehicleActorCreate *pVehicleCreate = (const C_VehicleActorCreate *)(GetCreationData().GetData());

	m_pVehicleModel->GetAuxFlares()->m_AuxFlareVector.PushBack();
	Vehicle::C_AuxFlare& auxFlare = m_pVehicleModel->GetAuxFlares()->m_AuxFlareVector.Back();

	auxFlare.m_pNode = pNode;

	auxFlare.m_FlareColour = flareColour;

	//Allocate AuxFlare
	auxFlare.m_pAuxFlareInstance = pVehicleCreate->m_pGameWorld->GetAuxFlares()->AllocAuxFlareInstance();

	//Create matrix
	//auxFlare.m_Matrix = Maths::HMat43(rotation, offset + pNode->m_pElement->GetLocalBounds().Centre());
	auxFlare.m_Matrix = Maths::HMat43(rotation, offset + pNode->GetCentre());
	
	//Size
	auxFlare.m_pAuxFlareInstance->UpdateSize(size);
	
	//Colour
	Maths::C_RGBA_F32 whiteFlareColour = pVehicleCreate->m_pGameWorld->GetCarLightFlareSettings().m_CarWhiteFlaresColour;
	Maths::C_RGBA_F32 redFlareColour = pVehicleCreate->m_pGameWorld->GetCarLightFlareSettings().m_CarRedFlaresColour;
	switch (auxFlare.m_FlareColour)
	{
	case Vehicle::E_AuxFlareColour_White:
		auxFlare.m_pAuxFlareInstance->UpdateColour(whiteFlareColour);
		break;
	case Vehicle::E_AuxFlareColour_Red:
		auxFlare.m_pAuxFlareInstance->UpdateColour(redFlareColour);
		break;
	}

	//Intensity
	auxFlare.m_Intensity = intensity;
	auxFlare.m_pAuxFlareInstance->UpdateIntensity(intensity);

	//Enable true
	auxFlare.m_pAuxFlareInstance->SetViewFlags(0xFFFFFFFF);
}

int C_VehicleActor::m_iDeferredRaytracePolicy = 0;

void C_VehicleActor::GetDamageState(Vehicle::C_DamageState& p_DamageState) const
{
	if (!m_pVehicleModel || !m_pVehicleModel->GetDamageLayer())
		return;

	memcpy(&p_DamageState, m_pVehicleModel->GetDamageLayer()->GetDamageState(), sizeof(Vehicle::C_DamageState));
	p_DamageState.m_GamePlayPoints = m_HealthPoints;
	p_DamageState.m_Padding = 0;
}

void C_VehicleActor::SetDamageState(const Vehicle::C_DamageState& p_DamageState)
{
	if (!m_pVehicleModel || !m_pVehicleModel->GetDamageLayer())
		return;

	m_pVehicleModel->GetDamageLayer()->RestoreDamageState(&p_DamageState);

	const C_VehicleActorCreate *pVehicleCreate = (const C_VehicleActorCreate *)(GetCreationData().GetData());
	if (!pVehicleCreate || !pVehicleCreate->m_pRLC)
		return;
	
	// Is this obsolete? It doesn't live in RLVehicleData any more either way.
	//pVehicleCreate->m_pRLC->GetVehicleData(m_VehicleIndex).mDamagePoints = p_DamageState.m_GamePlayPoints;

	m_HealthPoints = p_DamageState.m_GamePlayPoints;
}

bool C_VehicleActor::RenderShadowInMirror()
{
	if (!m_pVisDetect)
		return false;

	return m_pVisDetect->IsVisible(Game::E_ViewTypeFlag_Mirror) != 0 ? true : false;
}

void C_VehicleActor::PhotoMoveCar(const Maths::HVec3& offset)
{
	Maths::HMat43 matrix = m_Matrix;
	matrix.AW( matrix.AW() + offset );

	if (!m_pVehicleModel)
		return;

	//Set everything for now, don't worry about wheels to start off with
	for (U32 iInstance = 0; iInstance < m_pVehicleModel->GetNumModelInstances(); iInstance++)
	{				
		WorldRender::C_ModelInstance* pModelInstance = m_pVehicleModel->GetModelInstance(iInstance);
		pModelInstance->SetWorldSpherePosition(matrix.Translation());
		pModelInstance->SetElementMatrix(-1, matrix);
	}

	m_pVehicleModel->GetComponentList().UpdateMatrices(matrix, m_pVisDetect, false, &m_Matrix);		

	m_pVehicleModel->SetDynamicShaderParameters();

	SynchData(&m_Matrix, offset);	

	// Move the light flares too:
	Maths::HMat43 oldMatrix = m_Matrix;
	m_Matrix = matrix;
	if (!m_IsSplitscreen)
		UpdateLightFlares();
	UpdateAuxFlares();
	UpdateBlobShadow(AmaxCam::CameraSystem().DebugFeed().Live());
	m_Matrix = oldMatrix;

}

float C_VehicleActor::GetChaseCamHeightBase() const
{
	if (!m_pVehicleModel)
		return 0.0f;

	return m_pVehicleModel->GetModelInput()->m_vCentre.Y() + m_pVehicleModel->GetModelInput()->m_vExtents.Y();
}

void C_VehicleActor::Freeze()
{
	RemoveSubscriber( GameMessaging::E_GameMsg_PreUpdate );
	RemoveSubscriber( GameMessaging::E_GameMsg_Update );	
	RemoveSubscriber( Game::E_VehicleMessage_PostUpdate );		

	m_pCollisionBody->MakeStatic();

	m_pVehicleModel->Freeze(true);
}

void C_VehicleActor::Resume()
{
	AddSubscriber( GameMessaging::E_GameMsg_PreUpdate );
	AddSubscriber( GameMessaging::E_GameMsg_Update );
	AddSubscriber( Game::E_VehicleMessage_PostUpdate );

	m_pCollisionBody->Activate();

	m_pVehicleModel->Freeze(false);
}

Kanzen::HollywoodVehicleParams	C_VehicleActor::GetHollywoodCameraParams(S32 debugIndex) const
{
	Game::C_LevelState& LevelState = Game::C_Game::GetInstance()->GetLevelState();

	Kanzen::HollywoodVehicleParams* pParams = NULL;

	AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)m_XtIndex];
	BIZ_ASSERT(pVehicle);
	
	pParams = debugIndex == 0 ? pVehicle->m_HollywoodVehicleParams : pVehicle->m_HollywoodVehicleParams2;

	if (!pParams && C_GameVehicleData::Instance().mVehicleDefaults.Number() > 0)
	{
		pParams = C_GameVehicleData::Instance().mVehicleDefaults[0]->hollywoodCamDefaults;
	}
	BIZ_ASSERT(pParams);

	// Override for multiplayer motormash games
	if(LevelState.mRaceMode == Game::E_RaceMode_Multiplayer_Destruction ||
		g_debugUseMultiplayerHollywoodCamera )
	{
		if(C_GameVehicleData::Instance().mVehicleDefaults.Number() > 0)
		{

			Kanzen::HollywoodVehicleParams* pMotorMashDefaults = C_GameVehicleData::Instance().mVehicleDefaults[0]->hollywoodCamMotorMash;
			Kanzen::HollywoodVehicleParams MotorMashCombined = *pMotorMashDefaults;

			// Take certain parameters from the motormash camera and override
			MotorMashCombined.IdealDistanceAirbourne = pParams->IdealDistanceAirbourne;
			MotorMashCombined.IdealBase = pParams->IdealBase;
			MotorMashCombined.IdealDistanceAtFullAcceleration = pParams->IdealDistanceAtFullAcceleration;
			MotorMashCombined.IdealDistanceAtFullDeceleration = pParams->IdealDistanceAtFullDeceleration;
			MotorMashCombined.IdealDistanceNormal = pParams->IdealDistanceNormal;
			MotorMashCombined.IdealPitch = pParams->IdealPitch;
			MotorMashCombined.IdealPitchAirbourneDown = pParams->IdealPitchAirbourneDown;
			MotorMashCombined.IdealPitchAirbourneDown = pParams->IdealPitchAirbourneDown;
			MotorMashCombined.NormalFOV = pParams->NormalFOV;
			MotorMashCombined.LookAtPos = pParams->LookAtPos;


			return MotorMashCombined;
		}
	}
	
	return *pParams;
	
}

void C_VehicleActor::SetVisibility(bool p_Visible, S32 viewportIndex, bool force, bool forceTransition)
{
	if (!m_pVehicleModel)
		return;

	if ((viewportIndex == -1) && (m_Visible == 0xFF) && p_Visible && !force)
		return;
	if ((viewportIndex == -1) && (m_Visible == 0x00) && !p_Visible && !force)
		return;
	if ((viewportIndex != -1) && (m_Visible & (1 << viewportIndex))  && p_Visible && !force)
		return;
	if ((viewportIndex != -1) && !(m_Visible & (1 << viewportIndex)) && !p_Visible && !force)
		return;

	if (force && (viewportIndex == -1))
	{
		if (p_Visible)
		{
			if (forceTransition)
			{
				SetAlpha(E_VAC_TYPE_ALL, 0.0f, E_VAC_TARGET_TYPE_CURRENT);
				SetAlpha(E_VAC_TYPE_ALL, 1.0f, E_VAC_TARGET_TYPE_DESIRED);
			}
			else
				SetAlpha(E_VAC_TYPE_ALL, 1.0f, E_VAC_TARGET_TYPE_ALL);
		}
		else
		{
			if (forceTransition)
			{
				SetAlpha(E_VAC_TYPE_ALL, 1.0f, E_VAC_TARGET_TYPE_CURRENT);
				SetAlpha(E_VAC_TYPE_ALL, 0.0f, E_VAC_TARGET_TYPE_DESIRED);
			}
			else
				SetAlpha(E_VAC_TYPE_ALL, 0.0f, E_VAC_TARGET_TYPE_ALL);
		}
	}

	if (viewportIndex == -1)
		m_Visible = (p_Visible) ? 0x0F : 0;
	else
	{
		U32 viewPortFlag = 1 << viewportIndex;
		m_Visible = (m_Visible & ~viewPortFlag) | ((p_Visible) ? viewPortFlag : 0);  
	}

	m_pVehicleModel->GetModelInput()->m_bShouldRender = (m_Visible != 0);
	m_pVehicleModel->m_Hidden = (m_Visible == 0);

	for (U32 iModelInstance = 0; iModelInstance < m_pVehicleModel->GetNumModelInstances(); iModelInstance++)
	{
		if(m_pVehicleModel->GetModelInstanceType(iModelInstance) != Vehicle::E_ModelInstanceType_Shadow)
		{
			U32 flags = m_pVehicleModel->GetModelInstance(iModelInstance)->GetVisibilityFlags();

			// if visible then set visible flag, else clear view flag
			if (viewportIndex == -1)
			{
				for (U32 iView = 0; iView < 4; iView++)
				{
					if(p_Visible)
						flags |= g_MainViewFlags[iView];
					else
						flags &= ~g_MainViewFlags[iView];
				}
				if (C_Game::GetInstance()->GetLevelState().mInUI)
				{
					if(p_Visible)
						flags |= Game::E_ViewTypeFlag_Reflection;
					else
						flags &= ~Game::E_ViewTypeFlag_Reflection;
				}
			}
			else
			{
				if(p_Visible)
					flags |= g_MainViewFlags[viewportIndex];
				else
					flags &= ~g_MainViewFlags[viewportIndex];
			}

			m_pVehicleModel->GetModelInstance(iModelInstance)->SetVisibilityFlags(flags);
		}
	}

	if (m_IsFocusVehicle || !m_Visible )
		m_pVehicleModel->DisableVisibilityFlags(Game::E_ViewTypeFlag_Mirror);
	else
		m_pVehicleModel->EnableVisibilityFlags(Game::E_ViewTypeFlag_Mirror);

	if (m_Visible)
	{
		for (int iPart = 0; iPart < m_pVehicleModel->GetComponentList().m_ActiveParts.Size(); iPart++)
		{
			if (!m_pVehicleModel->GetComponentList().m_ActiveParts[iPart].m_pNode->GetVisible(true))
				m_pVehicleModel->GetComponentList().m_ActiveParts[iPart].m_pNode->SetVisible(false);
		}
	}	

	//Update the vehicle's blobby shadow
	if (m_pBlobShadowInstance)
	{
		m_pBlobShadowInstance->SetVisibilityFlags(m_Visible << E_ViewType_Main1);
	}
}

//================== Vehicle Proxy Utility Functions ============================

void C_VehicleActor::ResetInitialStates()
{
	SetBonnetPosition(0.0f);
	SetDoorPosition(0.0f, 0.0f);
	SetWindowPosition(1.0f, 1.0f);

	if (m_Window.m_pPart)
		m_Window.m_pPart->SetPermanentVisibility(0);

	GetVehicleModel()->ResetInitialStates();
}

void C_VehicleActor::Damage_EnableDisable(bool recieveDamage)
{
	// TODO - Chad
	// Option to disable vehicle damage
}

void C_VehicleActor::Damage_Apply(S32 damagestate)
{
	// TODO - Chad
	// Needs further discussion on how we do this
#ifndef CFG_GOLD
	switch(damagestate)
	{
		case 0:						// No damage
			g_debugFakeArtScratchLevel = 0.0f;
			g_debugFakeArtDeformLevel = 0.0f;
			g_debugFakeArtDamage = false;
			g_debugActuallyBurstTyre = false;
			g_debugActuallyLoseWheel = false;
			g_debugFakeFatalDamage = false;
			break;

		case 1:						// Full scratches
			g_debugFakeArtScratchLevel = 1.0f;
			g_debugFakeArtDeformLevel = 1.0f;
			g_debugFakeArtDamage = true;
			g_debugActuallyBurstTyre = false;
			g_debugActuallyLoseWheel = false;
			g_debugFakeFatalDamage = false;
			break;

		case 2:						// Lost tyre
			g_debugFakeArtScratchLevel = 1.0f;
			g_debugFakeArtDeformLevel = 1.0f;
			g_debugFakeArtDamage = true;
			g_debugActuallyBurstTyre = true;
			g_debugActuallyLoseWheel = false;
			g_debugFakeFatalDamage = false;
			break;

		case 3:						// Lost wheel
			g_debugFakeArtScratchLevel = 1.0f;
			g_debugFakeArtDeformLevel = 1.0f;
			g_debugFakeArtDamage = true;
			g_debugActuallyBurstTyre = true;
			g_debugActuallyLoseWheel = true;
			g_debugFakeFatalDamage = false;
			break;

		case 4:						// Wrecked
			g_debugFakeArtScratchLevel = 1.0f;
			g_debugFakeArtDeformLevel = 1.0f;
			g_debugFakeArtDamage = true;
			g_debugActuallyBurstTyre = true;
			g_debugActuallyLoseWheel = true;
			g_debugFakeFatalDamage = true;
			break;
	}
#endif
}

void C_VehicleActor::Damage_Repair()
{
	// TODO - Chad
	// Remove all vehicle damage
}

void C_VehicleActor::SetDoorPosition(F32 leftDoorState, F32 rightDoorState)
{
	if (!m_pVehicleModel)
		return;
	
	Vehicle::C_Part* pLeftPart = m_pVehicleModel->GetComponent(Vehicle::E_PartCode_Door, Vehicle::E_PartSubCode_FrontLeft);
	if (pLeftPart && !pLeftPart->m_pPhysicsBody)
		pLeftPart = m_pVehicleModel->GetComponent(Vehicle::E_PartCode_Door, Vehicle::E_PartSubCode_Left);
	if (pLeftPart)
	{
		if (leftDoorState != 0.0f)
			pLeftPart->OverrideConstraintLimits(Maths::DegToRad(leftDoorState), Maths::DegToRad(leftDoorState));
		else
			pLeftPart->RestoreConstraintLimits();
	}

	Vehicle::C_Part* pRightPart = m_pVehicleModel->GetComponent(Vehicle::E_PartCode_Door, Vehicle::E_PartSubCode_FrontRight);
	if (pRightPart && !pRightPart->m_pPhysicsBody)
		pRightPart = m_pVehicleModel->GetComponent(Vehicle::E_PartCode_Door, Vehicle::E_PartSubCode_Right);
	if (pRightPart)
	{
		if (rightDoorState != 0.0f)
			pRightPart->OverrideConstraintLimits(Maths::DegToRad(rightDoorState), Maths::DegToRad(rightDoorState));
		else
			pRightPart->RestoreConstraintLimits();
	}
}

void C_VehicleActor::SetWindowPosition(F32 leftWindowState, F32 rightWindowState)
{
	if (!m_Window.m_pPart)
		return;

	if (m_Window.m_pPart->m_PartSubCode == Vehicle::E_PartSubCode_FrontLeft || 
		m_Window.m_pPart->m_PartSubCode == Vehicle::E_PartSubCode_Left)
	{
		m_Window.SetPosition(1.0f - leftWindowState);
	}
	else
	{
		m_Window.SetPosition(1.0f - rightWindowState);
	}
}

void C_VehicleActor::SetBonnetPosition(F32 bonnetState)
{
	if (!m_pVehicleModel)
		return;
	
	Vehicle::C_Part* pPart = m_pVehicleModel->GetComponent(Vehicle::E_PartCode_Bonnet, Vehicle::E_PartSubCode_None);
	if (pPart && !pPart->m_pPhysicsBody)
		pPart = m_pVehicleModel->GetComponent(Vehicle::E_PartCode_Bonnet, Vehicle::E_PartSubCode_Front);
	if (pPart)
	{
		if (bonnetState != 0.0f)
			pPart->OverrideConstraintLimits(Maths::DegToRad(bonnetState), Maths::DegToRad(bonnetState));
		else
			pPart->RestoreConstraintLimits();
	}
}

void C_VehicleActor::SetLightState(bool leftIndicator, bool rightIndicator, bool headLights, bool brakeLights, bool reverseLights)
{
	// TODO - Chad
	// Pure graphical effect (don't know if this conflicts with setting vehicle speeds etc?)
}

void C_VehicleActor::DisplayVehicle(bool displayVehicle, bool fromMessage)
{
	if (fromMessage)
	{
		m_VisibilityOverride = !displayVehicle;
		CheckCamera(true);
	}

	if (displayVehicle)
	{
		SetState(true);
		SetCollisions(true);
		SetVisibility(true);
		DisplayDriver(true);
		AddSubscriber( GameMessaging::E_GameMsg_PreUpdate );
		AddSubscriber( GameMessaging::E_GameMsg_Update );
		AddSubscriber( Game::E_VehicleMessage_PostUpdate );
	}
	else
	{
		SetState(false);
		SetCollisions(false);
		SetVisibility(false);
		DisplayDriver(false);
		RemoveSubscriber( GameMessaging::E_GameMsg_PreUpdate );
		RemoveSubscriber( GameMessaging::E_GameMsg_Update );
		RemoveSubscriber( Game::E_VehicleMessage_PostUpdate );
	}	
}

void C_VehicleActor::DisplayDriver(bool displayDriver, S32 viewportIndex, bool fromMessage)
{
	// We might want some vehicles to be empty or need HiRes driver animations instead
	// (HiRes animated drivers will be parented to the vehicle actors in actor studio)

	if (fromMessage)
	{
		m_VisibilityOverride = !displayDriver;
		CheckCamera(true);
	}

	if (viewportIndex == -1)
	{
		for (U32 iView = 0; iView < 4; iView++)
			C_DriverManager::Instance()->SetDriverVisibility(m_VehicleIndex, displayDriver, iView);
	}
	else
		C_DriverManager::Instance()->SetDriverVisibility(m_VehicleIndex, displayDriver, viewportIndex);	
}

void C_VehicleActor::CreateVehicleEffects()
{
	// TODO - Chad
	// Not sure what the requirements are for this - To be discussed
}

void C_VehicleActor::SwapVehicleModel()
{
	// TODO - Chad
	// Some cutscenes need realistic police cars. We might use existing vehicles but swap their render model
}

Debug::Float g_debugDebugCollisionImpulse("Vehicle.Damage.Impact.DamageImpactSize", 0.25f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f), Debug::Persist());
Debug::Float g_debugDebugCollisionEffectImpulse("Vehicle.Damage.Impact.EffectImpactSize", 0.25f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f), Debug::Persist());
Debug::Float g_debugFilterCodeDeform("Vehicle.Damage.Impact.FilterCodeDeformScale", 0.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f), Debug::Persist());
Debug::Float g_debugFilterArtDeform("Vehicle.Damage.Impact.FilterArtDeformScale", 1.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f), Debug::Persist());
Debug::Bool g_debugFilterScratch("Vehicle.Damage.Impact.Scratch", true, Debug::Persist());
Debug::Bool g_debugFilterBurstTyres("Vehicle.Damage.Impact.BurstTyres", true, Debug::Persist());
Debug::Bool g_debugFilterDeformParts("Vehicle.Damage.Impact.DeformPanels", true, Debug::Persist());
Debug::Bool g_debugFilterDetachParts("Vehicle.Damage.Impact.DetachParts", false, Debug::Persist());
Debug::Bool g_debugFilterDetachWheels("Vehicle.Damage.Impact.DetachWheels", false, Debug::Persist());
Debug::Bool g_debugFilterDislodgeParts("Vehicle.Damage.Impact.DislodgeParts", true, Debug::Persist());
Debug::Bool g_debugFilterLightBreak("Vehicle.Damage.Impact.BreakLights", true, Debug::Persist());
Debug::Bool g_debugFilterLightKnockOut("Vehicle.Damage.Impact.KnockOutLights", false, Debug::Persist());
Debug::Bool g_debugFilterFatalDamage("Vehicle.Damage.Impact.FatalDamage", false, Debug::Persist());

/*
===============
NAME: FireDamageEvent

NOTE: 
===============
*/
void C_VehicleActor::FireDamageEvent(Maths::HVec3 p_ImpactPosition, Maths::HVec3 p_ImpactVector)
{
	if (!m_pVehicleModel || !m_pVehicleModel->GetDamageLayer())
		return;

	Vehicle::CollisionInfo collisionInfo;
	collisionInfo.m_ContactPoints.PushBack();
	collisionInfo.m_ContactPoints.Back().m_ContactType		= 2;
	collisionInfo.m_ContactPoints.Back().m_ContactPoint		= p_ImpactPosition;
	collisionInfo.m_ContactPoints.Back().m_PenetrationDepth = 0.0f;
	collisionInfo.m_ContactPoints.Back().m_Friction			= Maths::HVec3(0.0f, 0.0f, 0.0f);
	//collisionInfo.m_ContactPoints.Back().m_Impulse			= (float)g_debugDebugCollisionImpulse;
	collisionInfo.m_ContactPoints.Back().m_LocalContactPoint= collisionInfo.m_ContactPoints.Back().m_ContactPoint * m_pVehicleModel->GetModelInput()->m_mMatrix;

	collisionInfo.m_ContactNormal		= -p_ImpactVector;
	collisionInfo.m_MaxImpulse			= (float)g_debugDebugCollisionImpulse;
	collisionInfo.m_GameplayImpulse		= (float)g_debugDebugCollisionImpulse;
	collisionInfo.m_DamageImpulse		= (float)g_debugDebugCollisionImpulse;
	collisionInfo.m_EffectImpulse		= (float)g_debugDebugCollisionEffectImpulse;
	collisionInfo.m_PenerationDepth		= 0.0f;

	Vehicle::C_DamageFilter filter;

	filter.m_CodeDeformAmount	= (float)g_debugFilterCodeDeform;
	filter.m_ArtDeformAmount	= (float)g_debugFilterArtDeform;
	filter.m_ScratchDamage		= g_debugFilterScratch;
	filter.m_BurstTyres			= g_debugFilterBurstTyres;
	filter.m_DeformParts		= g_debugFilterDeformParts;
	filter.m_DetachParts		= g_debugFilterDetachParts;
	filter.m_DetachWheels		= g_debugFilterDetachWheels;
	filter.m_DislodgeParts		= g_debugFilterDislodgeParts;
	filter.m_LightBreak			= g_debugFilterLightBreak;
	filter.m_LightKnockOut		= g_debugFilterLightKnockOut;
	filter.m_FatalDamage		= g_debugFilterFatalDamage;
	
	m_pVehicleModel->GetDamageLayer()->ProcessCollision(m_pVehicleModel->GetModelInput()->m_mMatrix, collisionInfo, filter);
}

void C_VehicleActor::SetEvictDelay(S32 p_EvictDelay)
{
	if (m_pModelResource)
		m_pModelResource->SetEvictDelay(p_EvictDelay);
	if (m_pVehicleModelResource)
		m_pVehicleModelResource->SetEvictDelay(p_EvictDelay);
	if (m_pWheelModelResource)
		m_pWheelModelResource->SetEvictDelay(p_EvictDelay);
	if (m_pAuxFlareModelResource)
		m_pAuxFlareModelResource->SetEvictDelay(p_EvictDelay);
}

bool C_VehicleActor::HandleHoGForceResetArea(const Messaging::C_MessageBase *pMessage)
{
	C_RaceLogicLayer* pRaceLogicLayer		= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	const C_VehicleLogicActor* pLogicActor	= pRaceLogicLayer->GetLogicVehicleActor(m_VehicleIndex);

	if (!pLogicActor)
		return true;

	const ActorMsg::C_VehicleHandOfGodForceResetArea* pMsg = Messaging::MessageCast<const ActorMsg::C_VehicleHandOfGodForceResetArea>(pMessage);

	C_VehicleResetForceZone resetMsg;
	resetMsg.mResetPositions=pMsg->mResetPositions;
	resetMsg.mDelay=pMsg->mDelay;

	if( pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_HandOfGod) )
		GetActorList()->SendMessageToActorComponent( pLogicActor->GetHandle(), Components::E_IF_VehicleComponent_HandOfGod, &resetMsg );

	return true;
}


void C_VehicleActor::RepairForFinish()
{
	if (!m_pVehicleModel || !m_pVehicleModel->GetDamageLayer())
		return;

	m_pVehicleModel->GetDamageLayer()->RepairPart(Vehicle::E_PartCode_Window, Vehicle::E_PartSubCode_None, true);
}

bool C_VehicleActor::IsVisibleInMainView() const
{
	if (m_pVisDetect)
		return (m_pVisDetect->GetVisibleFlags() & E_ViewTypeFlag_Main) != 0;
	else
		return false;
}

C_BodyShake::C_BodyShake()
{
	m_bShaking				= false;
	
	m_vRange				= MathsStorage::Float3(0.0f, 0.0f, 0.0f);
	
	m_fTime					= 0.0f;
	m_fDuration				= 0.0f;
	m_fFrequency			= 0.0f;
	m_fNoise				= 0.0f;
}

void C_BodyShake::Start()
{
	m_bShaking	= true;

	m_fTime		= 0.0f;
	m_fNoise	= 0.0f;
}

void C_BodyShake::Extend(float p_fDuration)
{
	m_fDuration += p_fDuration;
}

void C_BodyShake::Update(Maths::HMat43& p_Matrix, float p_fTimeStep)
{
	if (!m_bShaking || m_fScale == 0.0f)
		return;
	
	float fTimeScale = 1.0f;
	if (m_fDuration > 0.0f)
		fTimeScale = m_fTime / m_fDuration;
	
	C_PerlinNoiseDefault noise;
	float fNoiseX	= m_vRange.x * fTimeScale * noise.SoftNoise1D(m_fNoise) * m_fScale;
	float fNoiseY	= m_vRange.y * fTimeScale * noise.SoftNoise1D(m_fNoise) * m_fScale;
	float fNoiseZ	= m_vRange.z * fTimeScale * noise.SoftNoise1D(m_fNoise) * m_fScale;

	m_fNoise		+= p_fTimeStep * m_fFrequency * m_fScale;
	
	Maths::C_Quaternion quatX(p_Matrix.AX(), Maths::HF32(fNoiseX));
	Maths::C_Quaternion quatY(p_Matrix.AY(), Maths::HF32(fNoiseY));
	Maths::C_Quaternion quatZ(p_Matrix.AZ(), Maths::HF32(fNoiseZ));

	p_Matrix.Rotation(p_Matrix.Rotation() * quatX);
	p_Matrix.Rotation(p_Matrix.Rotation() * quatY);
	p_Matrix.Rotation(p_Matrix.Rotation() * quatZ);	

	m_fTime			+= p_fTimeStep;

	if (m_fDuration > 0.0f && m_fTime > m_fDuration)
	{
		m_fTime			= 0.0f;
		m_bShaking		= false;
	}
}

//=======================================================================================
// Landing Detection
//=======================================================================================
const F32 LandingDetection::MIN_TIME_OFF_GROUND_BEFORE_AIRBOURNE = 0.1f;
const F32 LandingDetection::YVELOCITY_FOR_MAX_IMPACT = 7.0f;
const F32 LandingDetection::YDISTANCE_FOR_MAX_IMPACT = 10.0f;
const F32 LandingDetection::XZDISTANCE_FOR_MAX_IMPACT = 35.0f;
const F32 LandingDetection::AIRBOURNE_TIME_FOR_MAX_IMPACT = 4.0f;
const F32 LandingDetection::LANDING_IMPACT_LOWER_DOT = 0.75f;
const F32 LandingDetection::LANDING_IMPACT_UPPER_DOT = 0.99f;
const F32 LandingDetection::IMPACT_SCALE_FACTOR = 0.7f;

LandingDetection::LandingDetection()
{
	Reset();
}

void		
LandingDetection::Reset()
{
	m_LandedImpact = 0.0f;
	m_AirbourneTime = 0.0f;
	m_Airbourne = false;
	m_MaxYDistance = 0.0f;
	m_MaxYVelocity = 0.0f;
	m_Impact = false;
	for(int i=LANDING_MAX_WHEELS-1;i>=0;--i)
	{
		m_WheelOffGroundTime[i]=0.0f;
	}
}

void
LandingDetection::HandleImpact(
	const C_VehicleActor& vehicleActor
)
{
	F32 dot = vehicleActor.GetMatrix().AY().Y();
	if( dot < LANDING_IMPACT_LOWER_DOT  )
	{
		// Abort Abort
		m_Airbourne = false;
	}
	else
	{
		m_Impact = true;
	}
}

F32 
LandingDetection::GetLandedImpact() const
{
	return m_LandedImpact;
}

bool		
LandingDetection::IsAirbourne() const
{
	return m_Airbourne;
}

void
LandingDetection::Update(
	const C_VehicleActor& vehicleActor,
	F32 timeElapsed
) 
{
	const RacingPhysics::Car* physics = vehicleActor.GetVehiclePhysics();
	BIZ_ASSERT(physics);
	BIZ_ASSERT(physics->GetNumWheels() <= LANDING_MAX_WHEELS);

	// Data collection
	// collect together all the relevant data
	// to use in determining our current state
	// and an impact level when we do land.
	int wheelsOnGround = 0;
	bool anyOnGround = false;

	if(!m_Airbourne)
	{
		m_AirbourneTime = Maths::Constants<F32>::Max();
	}
	
	for(int i=physics->GetNumWheels()-1;i>=0;--i)
	{
		bool onGround = physics->WheelOnGround(i);

		wheelsOnGround += onGround?1:0;

		m_WheelOffGroundTime[i] = onGround ? 0.0f: m_WheelOffGroundTime[i] + timeElapsed;
		if(!m_Airbourne)
		{
			m_AirbourneTime = Maths::Min(m_AirbourneTime,m_WheelOffGroundTime[i]);
		}
		else
		{
			m_AirbourneTime += timeElapsed;
		}
	}

	m_LandedImpact = 0.0f;
	// State Management
	// Work out if we are airbourne or on ground and 
	// act on any change when they occur.
	if(!m_Airbourne)
	{
		if(m_AirbourneTime>MIN_TIME_OFF_GROUND_BEFORE_AIRBOURNE)
		{
			m_Airbourne = true;
			m_PositionAtTakeOff = vehicleActor.GetMatrix().AW();
			m_MaxYDistance = m_PositionAtTakeOff.Y();
			m_MaxYVelocity = Maths::Constants<F32>::Max();
			m_Impact = false;
		}
		
	}
	else
	{
		if(wheelsOnGround>2)
		{
			// Landed, so work out a landing severity
			// 1.0f = Harshest possible landing
			// 0.0f = Incredibly light landing
			
			// YDistanceTravelled. +ve if we have landed lower than we took off. -ve otherwise.
			F32 currentYPos = vehicleActor.GetMatrix().AW().Y();
			F32 YDistanceTravelled = m_PositionAtTakeOff.Y() - currentYPos;
			
			// XZDistanceTravelled. Always +ve. The magnitude of the distance travelled in the XZ plane.
			F32 XZDistanceTravelled;
			Maths::HVec3 DistanceTravelledVec = vehicleActor.GetMatrix().AW() - m_PositionAtTakeOff;
			Maths::HVec3 DistanceTravelledVecXZ( DistanceTravelledVec.X(), 0.0f, DistanceTravelledVec.Z());
			XZDistanceTravelled = DistanceTravelledVecXZ.Length();

			// YVelocityAtLanding. Should always be <0. The world y velocity just before touchdown
			F32 YVelocityAtLanding = Maths::Clamped<F32>(m_MaxYVelocity,-Maths::Constants<F32>::Max(),0.0f);
			YVelocityAtLanding = -YVelocityAtLanding;

			
			m_LandedImpact  = Maths::Clamped<F32>(YVelocityAtLanding / YVELOCITY_FOR_MAX_IMPACT,0.0f,1.0f);
			m_LandedImpact += Maths::Clamped<F32>(YDistanceTravelled / YDISTANCE_FOR_MAX_IMPACT,0.0f,1.0f);
			m_LandedImpact *= Maths::Clamped<F32>(XZDistanceTravelled/ XZDISTANCE_FOR_MAX_IMPACT,0.0f,1.0f);
			m_LandedImpact *= Maths::Clamped<F32>(m_AirbourneTime/ AIRBOURNE_TIME_FOR_MAX_IMPACT,0.0f,1.0f);
			m_LandedImpact *= m_Impact?IMPACT_SCALE_FACTOR:1.0f;
			m_LandedImpact = Maths::Clamped<F32>(m_LandedImpact,0.0f,1.0f);
			
			
			m_Airbourne = false;
		}
		else
		{
			F32 currentYPos = vehicleActor.GetMatrix().AW().Y();
			if( currentYPos > m_MaxYDistance)
			{
				m_MaxYDistance = currentYPos;
			}

			F32 currentYVel = physics->GetVelocity().Y();;
			if( currentYVel < m_MaxYVelocity)
			{
				m_MaxYVelocity = currentYVel;
			}
		}
	}

	m_prevVelocity = physics->GetVelocity();
}

//=======================================================================================
// Tumbling Detection
//=======================================================================================
const F32 TumblingDetection::MIN_TIME_OFF_GROUND_BEFORE_TUMBLING = 0.5f;
const F32 TumblingDetection::UPSIDE_DOWN_DOT = 0.85f;
const F32 TumblingDetection::TUMBLING_REGAINED_CONTROL_ANG_VELOCITY_MAG = 0.4f;

TumblingDetection::TumblingDetection()
{
	Reset();
}

void		
TumblingDetection::Reset()
{
	m_tumbling = false;
	for(int i=TUMBLING_MAX_WHEELS-1;i>=0;--i)
	{
		m_WheelOffGroundTime[i]=0.0f;
	}
}

bool
TumblingDetection::GetTumbling() const
{
	return m_tumbling;
}

void
TumblingDetection::Update(
	const C_VehicleActor& vehicleActor,
	F32 timeElapsed
) 
{
	const RacingPhysics::Car* physics = vehicleActor.GetVehiclePhysics();
	BIZ_ASSERT(physics);
	BIZ_ASSERT(physics->GetNumWheels() <= TUMBLING_MAX_WHEELS);

	bool offGround = true;
	int wheelsOnGround = 0;
	for(int i=physics->GetNumWheels()-1;i>=0;--i)
	{
		bool onGround = physics->WheelOnGround(i);

		wheelsOnGround += onGround?1:0;

		m_WheelOffGroundTime[i] = onGround ? 0.0f: m_WheelOffGroundTime[i] + timeElapsed;
		if( m_WheelOffGroundTime[i] < MIN_TIME_OFF_GROUND_BEFORE_TUMBLING )
		{
			offGround = false;
		}
	}

	F32 worldDotUp = vehicleActor.GetMatrix().AY() | Maths::Constants<Maths::HVec3>::UnitAxisPositiveY();
	bool upsideDown = worldDotUp < UPSIDE_DOWN_DOT;
	if(!m_tumbling)
	{
		if( offGround && upsideDown)
		{
			m_tumbling = true;
		}
	}
	else
	{
		RacingPhysics::Vector3R angVel = physics->GetAngularVelocity();

		if( !upsideDown &&												// upright again?
			(physics->GetNumWheels() - wheelsOnGround <= 1) &&			// At most 1 wheel still off the ground
			angVel.Length() < TUMBLING_REGAINED_CONTROL_ANG_VELOCITY_MAG)
		{
			m_tumbling = false;
		}
	}

}

} //namespace Actors

} //namespace Game

