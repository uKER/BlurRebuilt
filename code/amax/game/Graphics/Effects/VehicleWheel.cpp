/////////////////////////////////////////////////////////////
// VehicleWheel
/////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <Graphics/Effects/VehicleWheel.h>

#include <RacingPhysics/RacingPhysics.h>

#include <world_render/LightSamplePointScene.h>

#include <Dynamic/Particle/Generic/XtInterface.h>
#include <Dynamic/Particle/Event.h>
#include <Dynamic/Particle/EventInstance.h>
#include <Dynamic/Common.h>
#include <Dynamic/Renderer.h>
#include <Dynamic/Particle/Manager.h>
#include <Dynamic/Track/Track.h>

#include <Game/GameWorld/ViewType.h>
#include <GameApp.h>
#include <GameLogic/GameData/GameEffectData.h>

#include <Vehicle/Damage/DamageLayer.h>
#include <framework\application\ApplicationBase.h>

#ifndef CFG_GOLD
#include <GameLogic/Replays/ReplayDirector.h>
#endif // #ifndef CFG_GOLD

#include <Game/GameUtil/ResourceBag.h>
#include "SplashCamera/SplashCamera.h"

#include "scenegraph/SceneGraph.h"

Debug::Logger &g_VehicleWheelEffectLog = Debug::GetLogger("amax.effects");

namespace Game
{

namespace Effects
{

/* 
===============================================================
					class C_VehicleWheel

NOTES: 
===============================================================
*/

/*
===============
NAME: C_VehicleWheel

NOTE: C_VehicleWheel
===============
*/
C_VehicleWheel::C_VehicleWheel()
{
	m_pWheel		= NULL;

	m_pParticleManager	= NULL;

	m_pSceneGraph = NULL;

	m_UniqueIndex	= -1;

	m_CurrentSurfaceType	= -1;
	m_CurrentModifier		= -1;

	m_HalfWidth		= 0.15f;

	m_CurrentSurfaceStateTime = 0.f;

	m_RimPointLightFlickerPosition = 0;
	m_RimPointLightFlickerRest = 0.f;

	m_Update	= false;
	m_Refresh	= false;


	m_Update;
	m_Refresh;
	m_OnGround;
	m_RimEvent;

	memset(&m_Evaluate, 0.0f, sizeof(float)*4);

	m_TrackWidth				= 0.0f;
	m_Speed						= 0.0f;

	m_SurfaceModifier			= -1;
	m_EffectModifierMapping		= -1;
	m_TrackModifierMapping		= -1;
	m_EffectMapping				= -1;
	m_TrackMapping				= -1;
	m_SurfaceType				= -1;
	
	m_GroundPosition			= Maths::HVec3(0.0f, 0.0f, 0.0f);
	m_Position					= Maths::HVec3(0.0f, 0.0f, 0.0f);
	m_Velocity					= Maths::HVec3(0.0f, 0.0f, 0.0f);
	m_Normal					= Maths::HVec3(0.0f, 0.0f, 0.0f);

	m_ClearDeferred = false;
}

/*
===============
NAME: ~C_VehicleWheel

NOTE: Destructor
===============
*/
C_VehicleWheel::~C_VehicleWheel()
{

}

Debug::Bool g_debugUpdateWheelDirection("Vehicle.Effects.Wheel.UpdateDirection", false);

Debug::Float g_debugVehicleWheelForwardDirX("Vehicle.Effects.Wheel.ForwardDirX", 1.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugVehicleWheelForwardDirY("Vehicle.Effects.Wheel.ForwardDirY", 0.5f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugVehicleWheelForwardDirZ("Vehicle.Effects.Wheel.ForwardDirZ", -1.0f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));

Debug::Float g_debugVehicleWheelReverseDirX("Vehicle.Effects.Wheel.ReverseDirX", 1.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugVehicleWheelReverseDirY("Vehicle.Effects.Wheel.ReverseDirY", 0.5f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugVehicleWheelReverseDirZ("Vehicle.Effects.Wheel.ReverseDirZ", 1.0f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));

Debug::Float g_debugVehicleRimLightOffsetX("Vehicle.Effects.Wheel.RimLightOffsetX", 0.15f);
Debug::Float g_debugVehicleRimLightOffsetY("Vehicle.Effects.Wheel.RimLightOffsetY", 0.25f);
Debug::Float g_debugVehicleRimLightOffsetZ("Vehicle.Effects.Wheel.RimLightOffsetZ", 0.0f);
Debug::Float g_debugVehicleRimLightRadius("Vehicle.Effects.Wheel.RimLightRadius", 2.0f);
Debug::Float g_debugVehicleRimLightIntensityFactor("Vehicle.Effects.Wheel.RimLightIntensityFactor", 1.f);
Debug::Float g_debugRimPointLightCullDistant("Vehicle.Effects.Wheel.RimLightCullDistant", 25.f, Debug::Range(0.f, 128.0f));


/*
===============
NAME: Create

NOTE: Create
===============
*/
void C_VehicleWheel::Create(U32 p_Index, const RacingPhysics::CarWheel* p_pWheel,
							DynamicEffect::Particle::C_ParticleManager* p_pParticleManager,
							SceneGraph::C_SceneGraph* p_pSceneGraph,
							DynamicEffect::Track::C_TrackRenderer* p_pTrackRenderer, float p_HalfWidth)
{
	m_pWheel				= p_pWheel;

	m_pParticleManager		= p_pParticleManager;
	m_pTrackRenderer		= p_pTrackRenderer;

	m_pSceneGraph			= p_pSceneGraph;

	m_UniqueIndex			= p_Index;

	m_CurrentSurfaceType	= -1;
	m_CurrentModifier		= -1;

	m_HalfWidth				= p_HalfWidth;

	UpdateWheelDirection();
}

/*
===============
NAME: Destroy

NOTE: Destroy
===============
*/
void C_VehicleWheel::Destroy()
{
	for(S32 iParticle = 0; iParticle < m_SurfaceParticles.Size(); iParticle++)
	{
		if (m_SurfaceParticles[iParticle].pEvent != 0)
		{
			if(DynamicEffect::Particle::C_EventInstance* pEvent = m_SurfaceParticles[iParticle].particleEvent.Object())
			{
				pEvent->SetIntensity(0.0f);
				pEvent->Free();
			}
		}
	}

	if(DynamicEffect::Track::C_Emitter* pEmitter = m_SurfaceTrack.track.Object())
		pEmitter->Free();
	m_SurfaceTrack.pEvent = 0;
	
	m_pWheel		= NULL;

	m_pParticleManager = NULL;

	if (m_pSceneGraph && (m_RimPointLightHandle.Object() != NULL))
	{
		SceneGraph::RemovePointLight(m_pSceneGraph, m_RimPointLightHandle);
	}
	m_pSceneGraph = NULL;
}

Debug::Float g_debugVehicleWheelEffectSpeedScale("Vehicle.Effects.Wheel.SpeedScale", 0.037f, Debug::Range(0.01f, 1.0f), Debug::Step(.01f));
Debug::Float g_debugVehicleWheelEffectSlideFactorScale("Vehicle.Effects.Wheel.SlideFactorScale", 1.25f, Debug::Range(0.01f, 1.0f), Debug::Step(.01f));
Debug::Float g_debugVehicleWheelEffectSpinFactorScale("Vehicle.Effects.Wheel.SpinFactorScale", 0.75f, Debug::Range(0.01f, 1.0f), Debug::Step(.01f));
Debug::Float g_debugVehicleWheelEffectContactTimeScale("Vehicle.Effects.Wheel.ContactTimeScale", 2.5f, Debug::Range(0.01f, 10.0f), Debug::Step(.01f));
Debug::Float g_debugVehicleWheelEffectContactMinSpeed("Vehicle.Effects.Wheel.ContactMinSpeed", 18.f, Debug::Range(0.f, 80.0f), Debug::Step(1.f));

Debug::Float g_debugWheelRimTrackWidth("Vehicle.Damage.Code.Wheels.Particles.RimMarkWidth", 0.00625f, Debug::Range(0.0f, 0.25f), Debug::Step(0.001f));
Debug::Float g_debugWheelRimTrackOffset("Vehicle.Damage.Code.Wheels.Particles.RimTrackOffset", 0.15f, Debug::Range(0.0f, 0.25f), Debug::Step(0.001f));
Debug::Float g_debugWheelSparksMinVelocity("Vehicle.Damage.Code.Wheels.Particles.SparksMinVelocity", 5.0f, Debug::Range(0.0f, 50.0f), Debug::Step(0.001f));
Debug::Float g_debugWheelSparksMaxVelocity("Vehicle.Damage.Code.Wheels.Particles.SparksMaxVelocity", 15.0f, Debug::Range(0.0f, 50.0f), Debug::Step(0.001f));

Debug::Float g_debugRimTrackMinVelocity("Vehicle.Damage.Code.Wheels.Particles.RimTracksMinVelocity", 1.0f, Debug::Range(0.0f, 50.0f), Debug::Step(0.001f));
Debug::Float g_debugRimTrackMaxVelocity("Vehicle.Damage.Code.Wheels.Particles.RimTracksMaxVelocity", 5.0f, Debug::Range(0.0f, 50.0f), Debug::Step(0.001f));

Debug::Bool g_debugWheelEffectCarLocal("Vehicle.Effects.Wheel.CarLocal", true);

//Flickering table values for RIM light
const U32 g_FlickerTableSize = 16;
float g_FlickerTable[g_FlickerTableSize] = {0.9f, 0.3f, 0.6f, 0.4f, 0.8f, 0.5f, 0.55f, 0.2f, 0.9f, 0.7f, 0.3f, 0.1f, 0.1f, 0.8f, 0.3f, 0.4f};

/*
===============
NAME: Update

NOTE: Update
===============
*/
void C_VehicleWheel::Update(Maths::HMat43* p_pParentMatrix, Maths::HMat33* p_pYawMatrix, F32 p_fTimeStep, const Maths::C_RGBA_F32& p_Lighting, const Vehicle::C_WheelDamage* p_pWheelDamage, const bool p_Enable, float p_fEmitScale, float p_fLifeScale, S32 splashViewport, SplashCamera::C_SplashCamera* pSplashCamera, U32 visibilityFlags, Maths::HVec3 p_Velocity)
{
	UpdateCommon(p_pParentMatrix, p_pYawMatrix, p_fTimeStep, p_pWheelDamage, p_Enable, splashViewport, pSplashCamera, visibilityFlags);
	UpdateParticles(p_pParentMatrix, p_fTimeStep, p_Lighting, p_fEmitScale, p_fLifeScale, visibilityFlags, p_Velocity);
	UpdateTracks(p_Lighting, p_pWheelDamage, visibilityFlags);
}

/*
===============
NAME: Update

NOTE: Update
===============
*/
void C_VehicleWheel::UpdateCommon(Maths::HMat43* p_pParentMatrix, Maths::HMat33* p_pYawMatrix, F32 p_fTimeStep, const Vehicle::C_WheelDamage* p_pWheelDamage, const bool p_Enable, S32 splashViewport, SplashCamera::C_SplashCamera* pSplashCamera, U32 visibilityFlags)
{
	{

	PERF_EVENT("Wheel.UpdateCommon");

	m_Update = p_Enable;

	// only have particles if car is visible in the main view
	if(!m_Update || m_ClearDeferred)
	{
		m_CurrentSurfaceType = -1;
		m_CurrentModifier = -1;
		ClearParticles();
		ClearTracks();
		ClearMisc();

		if (m_ClearDeferred)
			m_ClearDeferred = false;
	}

#ifndef CFG_GOLD
	if (g_debugUpdateWheelDirection)
	{
		UpdateWheelDirection();
	}
#endif

	if (m_pWheel->OnGround())
	{
		m_SurfaceType = m_pWheel->SurfaceModifier() == 0 ? m_pWheel->SurfaceType() : m_pWheel->SurfaceModifier();
		m_Normal = Maths::HVec3(m_pWheel->GroundNormal());
		m_Position = m_pWheel->GroundPosition();
	}
	else
	{
		m_SurfaceType = 0;
		m_Normal = Maths::HVec3(0.0f, 1.0f, 0.0f);
		m_Position = Maths::HVec3(0.0f, -10000.0f, 0.0f);
	}

	m_Velocity = Maths::HVec3(m_pWheel->GetVelocity());
	m_Speed = Maths::Abs((F32)m_Velocity.Length());
	
	m_Evaluate[0] = Maths::Clamped(m_Speed * g_debugVehicleWheelEffectSpeedScale, 0.0f, 1.0f);
	m_Evaluate[1] = Maths::Clamped(Maths::Abs(m_pWheel->SlipAngle()) * g_debugVehicleWheelEffectSlideFactorScale, 0.0f, 1.0f);
	m_Evaluate[2] = Maths::Clamped(Maths::Abs(m_pWheel->WheelSpin()) * g_debugVehicleWheelEffectSpinFactorScale, 0.0f, 1.0f);
	m_Evaluate[3] = (m_Speed > g_debugVehicleWheelEffectContactMinSpeed) ? Maths::Clamped(m_CurrentSurfaceStateTime / g_debugVehicleWheelEffectContactTimeScale, 0.f, 1.f) : 1.f;

	m_SurfaceModifier = 0;

	m_EffectModifierMapping = C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceEffectsMapping[m_SurfaceModifier];
	m_TrackModifierMapping = C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceTracksMapping[m_SurfaceModifier];
	m_EffectMapping = -1;
	m_TrackMapping = -1;

	m_TrackWidth = m_HalfWidth;
	
	m_OnGround = m_pWheel->OnGround();
	if (m_OnGround)
	{
		m_GroundPosition = m_pWheel->GroundPosition();
		m_GroundPosition.Y(m_GroundPosition.Y() + m_pWheel->SurfaceModifierDepth() + 0.01f);
	}
	else
		m_GroundPosition = m_pWheel->GetWheelPosition();

	//Check for tyre damage
	m_RimEvent = false;
	if (m_Update && p_pWheelDamage && (p_pWheelDamage->m_TyreBurst || p_pWheelDamage->m_Detached))
	{
		m_TrackWidth = (float)g_debugWheelRimTrackWidth;
		m_RimEvent = true;
	}

	if (m_RimEvent)
	{
		//Create light rim, with visibility flags false
		if (m_RimPointLightHandle.Object() == NULL)
		{
			m_RimPointLightHandle = SceneGraph::AddPointLight(m_pSceneGraph);
			if (m_RimPointLightHandle.Object())
			{
				m_RimPointLightHandle->SetVisibilityFlags(0);
				m_RimPointLightHandle->SetCullDistance(g_debugRimPointLightCullDistant);
				m_RimPointLightHandle->SetFlags(visibilityFlags);
			}
		}
	}

	m_LocalSpace.SetPosition(m_GroundPosition);
	m_LocalSpaceSpin.SetPosition(m_GroundPosition);
	if (g_debugWheelEffectCarLocal)
	{
		Maths::C_Quaternion quat(p_pParentMatrix->Rotation());
		m_LocalSpace.SetRotation(quat);
		m_LocalSpaceSpin.SetRotation(Maths::C_Quaternion(*p_pYawMatrix));
	}
	else
	{
		/*Maths::HMat33 wheelMat(Maths::HVec3(m_pWheel->GetRotationMatrix().AxisX().X(), m_pWheel->GetRotationMatrix().AxisX().Y(), m_pWheel->GetRotationMatrix().AxisX().Z()), 
							Maths::HVec3(m_pWheel->GetRotationMatrix().AxisY().X(), m_pWheel->GetRotationMatrix().AxisY().Y(), m_pWheel->GetRotationMatrix().AxisY().Z()), 
							Maths::HVec3(m_pWheel->GetRotationMatrix().AxisZ().X(), m_pWheel->GetRotationMatrix().AxisZ().Y(), m_pWheel->GetRotationMatrix().AxisZ().Z()));*/

		Maths::HMat33 wheelMat(m_pWheel->GetRotationMatrix().Matrix3());
		Maths::C_Quaternion quat(wheelMat);
		m_LocalSpace.SetRotation(quat);
	}

	m_Refresh = false;

	//Update CurrentSurfaceStateTime
	if (m_SurfaceType != m_CurrentSurfaceType || m_SurfaceModifier != m_CurrentModifier)
	{
		m_CurrentSurfaceStateTime	= 0.f;
		m_CurrentSurfaceType		= m_SurfaceType;
		m_CurrentModifier			= m_SurfaceModifier;
		m_Refresh					= true;
	}
	else
		m_CurrentSurfaceStateTime += p_fTimeStep;

	if(m_EffectModifierMapping != -1)
	{
		m_EffectMapping = C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceEffects[m_EffectModifierMapping]->effectMapping[m_SurfaceType];
	}

	if(m_TrackModifierMapping != -1)
	{
		m_TrackMapping = C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceTracks[m_TrackModifierMapping]->trackMapping[m_SurfaceType];
	}

	}

	//Splash Camera
	if(splashViewport != -1 && m_EffectMapping != -1)
	{
		PERF_EVENT("Wheel.UpdateCommon.SplashCamera");

		AmaxWheelSurfaceEffect& surfaceEffect = C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceEffects[m_EffectModifierMapping]->effects[m_EffectMapping];
		for(S32 iEvent = 0; iEvent < surfaceEffect.splashEvents.count; iEvent++)
		{
			if(AmaxWheelSurfaceSplashEvent* pWheelSurfaceSplashEvent = surfaceEffect.splashEvents[iEvent].Object())
			{
				if (SplashCamera::C_SplashType* pSplashType = pWheelSurfaceSplashEvent->splashCameraType.Object())
				{
					//Calculate new splash counters
					F32 counter = pWheelSurfaceSplashEvent->splashCameraFactorCurve.Evaluate(m_Evaluate[pWheelSurfaceSplashEvent->trigger]) * pWheelSurfaceSplashEvent->splashCameraCountPerSecond * p_fTimeStep;

					pWheelSurfaceSplashEvent->splashCameraCurrentCount += counter;

					if (pWheelSurfaceSplashEvent->splashCameraCurrentCount > 1.f)
					{
						//Add splashes
						U32 splashCount = Maths::Floor(pWheelSurfaceSplashEvent->splashCameraCurrentCount);
						
						Maths::HVec3 splashVelocity = -m_Velocity;
						
						U32 viewFlags = 1 << E_ViewType_Main1;
						for (S32 i = 0; i < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount(); i++)
						{
							U32 viewType = E_ViewType_Main1 + i;
							
							pSplashCamera->AddSplash(viewType, pSplashType, splashCount, m_GroundPosition, splashVelocity, pWheelSurfaceSplashEvent->splashCameraSpread, C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr());

							viewFlags = viewFlags << 1;
						}

						pWheelSurfaceSplashEvent->splashCameraCurrentCount -= F32(splashCount);
					}
				}
			}
		}
	}

}

/*
===============
NAME: Update

NOTE: Update
===============
*/
void C_VehicleWheel::UpdateParticles(Maths::HMat43* p_pParentMatrix, F32 p_fTimeStep, const Maths::C_RGBA_F32& p_Lighting, float p_fEmitScale, float p_fLifeScale, U32 visibilityFlags, Maths::HVec3 p_Velocity)
{
	if (!m_Update)
		return;

	PERF_EVENT("Wheel.UpdateParticles");

	//Refresh if theres a different number of events (sorcery changes) get rid in speed
	#ifndef CFG_SPEED
	if(m_EffectMapping != -1 && (	(!m_RimEvent && C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceEffects[m_EffectModifierMapping]->effects[m_EffectMapping].events.count != m_SurfaceParticles.Size()) ||
								(m_RimEvent && C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceEffects[m_EffectModifierMapping]->effects[m_EffectMapping].rimEvents.count != m_SurfaceParticles.Size())))
		m_Refresh = true;

	//Refresh if any of the events have gone null
	for(S32 iParticle = 0; iParticle < m_SurfaceParticles.Size(); iParticle++)
	{
		if(!m_SurfaceParticles[iParticle].particleEvent.Object() || !m_SurfaceParticles[iParticle].event.Object())
		{
			m_SurfaceParticles[iParticle].pEvent = 0;
			m_Refresh = true;
		}
	}
	#endif

	/////////////////////////////////////////////// ####################################################

	// if the surface has changed we need to remove current emitters and request new ones
	if(m_Refresh)
	{
		// clear current effects
		SurfaceParticles previousParticles = m_SurfaceParticles;
		
		//Get the effect mapping from the surface type and modifier type
		//if we have a valid combination from socery then create the new emitters
		if(m_EffectMapping != -1)
		{
			AmaxWheelSurfaceEffect& surfaceEffect = C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceEffects[m_EffectModifierMapping]->effects[m_EffectMapping];
		
			if (m_RimEvent)
			{
				m_SurfaceParticles.Resize(surfaceEffect.rimEvents.count);

				for(S32 iParticle = 0; iParticle < surfaceEffect.rimEvents.count; iParticle++)
				{
					// set pointer to null
					m_SurfaceParticles[iParticle].pEvent = 0;

					if(AmaxWheelSurfaceParticleEvent* pWheelSurfaceParticleEvent = surfaceEffect.rimEvents[iParticle].Object())
					{
						//See if the previous effects had any of the same events
						//this allows emitters to transition surface types without being recreated
						for(S32 iPrevEffect = 0; iPrevEffect < previousParticles.Size(); iPrevEffect++)
						{
							//See if unique ids match
							if(	previousParticles[iPrevEffect].pEvent != 0 && previousParticles[iPrevEffect].pEvent == pWheelSurfaceParticleEvent)
							{
								//They match so copy previous effect directly
								m_SurfaceParticles[iParticle]			= previousParticles[iPrevEffect];
								//Set previous pointer to 0 to indicate effect was recycled
								previousParticles[iPrevEffect].pEvent	= 0;
								break;
							}
						}

						// couldn't find a previous effect, or event expired - request a new one and set up
						if(m_SurfaceParticles[iParticle].pEvent == 0 || !m_SurfaceParticles[iParticle].particleEvent.Object())
						{
							m_SurfaceParticles[iParticle].event				= surfaceEffect.rimEvents[iParticle];
							m_SurfaceParticles[iParticle].pEvent			= pWheelSurfaceParticleEvent;
							m_SurfaceParticles[iParticle].intensityCurrent	= 0.0f;
							m_SurfaceParticles[iParticle].alphaCurrent		= 0.0f;

							//Check particle event field is not null
							if(DynamicEffect::Particle::XtInterface::Event* pXtEvent = pWheelSurfaceParticleEvent->particleEvent.Object())
							{
								AmaxWheelParticleEventDescriptor* pWheel = &pWheelSurfaceParticleEvent->carFL;
								if (m_SurfaceParticles[iParticle].pEvent->trigger == 2)
									m_SurfaceParticles[iParticle].particleEvent = pXtEvent->pEvent->Create(pWheel[m_UniqueIndex].node, &m_LocalSpaceSpin);
								else
									m_SurfaceParticles[iParticle].particleEvent = pXtEvent->pEvent->Create(pWheel[m_UniqueIndex].node, &m_LocalSpace);
							}
						}
					}
				}
			}
			else
			{
				m_SurfaceParticles.Resize(surfaceEffect.events.count);

				for(S32 iParticle = 0; iParticle < surfaceEffect.events.count; iParticle++)
				{
					// set pointer to null
					m_SurfaceParticles[iParticle].pEvent = 0;

					if(AmaxWheelSurfaceParticleEvent* pWheelSurfaceParticleEvent = surfaceEffect.events[iParticle].Object())
					{
						//See if the previous effects had any of the same events
						//this allows emitters to transition surface types without being recreated
						for(S32 iPrevEffect = 0; iPrevEffect < previousParticles.Size(); iPrevEffect++)
						{
							//See if unique ids match
							if(	previousParticles[iPrevEffect].pEvent != 0 && previousParticles[iPrevEffect].pEvent == pWheelSurfaceParticleEvent)
							{
								//They match so copy previous effect directly
								m_SurfaceParticles[iParticle]			= previousParticles[iPrevEffect];
								//Set previous pointer to 0 to indicate effect was recycled
								previousParticles[iPrevEffect].pEvent	= 0;
								break;
							}
						}

						// couldn't find a previous effect, or event expired - request a new one and set up
						if(m_SurfaceParticles[iParticle].pEvent == 0 || !m_SurfaceParticles[iParticle].particleEvent.Object())
						{
							m_SurfaceParticles[iParticle].event				= surfaceEffect.events[iParticle];
							m_SurfaceParticles[iParticle].pEvent			= pWheelSurfaceParticleEvent;
							m_SurfaceParticles[iParticle].intensityCurrent	= 0.0f;
							m_SurfaceParticles[iParticle].alphaCurrent		= 0.0f;

							//Check particle event field is not null
							if(DynamicEffect::Particle::XtInterface::Event* pXtEvent = pWheelSurfaceParticleEvent->particleEvent.Object())
							{
								AmaxWheelParticleEventDescriptor* pWheel = &pWheelSurfaceParticleEvent->carFL;
								if (m_SurfaceParticles[iParticle].pEvent->trigger == 2)
									m_SurfaceParticles[iParticle].particleEvent = pXtEvent->pEvent->Create(pWheel[m_UniqueIndex].node, &m_LocalSpaceSpin);
								else
									m_SurfaceParticles[iParticle].particleEvent = pXtEvent->pEvent->Create(pWheel[m_UniqueIndex].node, &m_LocalSpace);
							}
						}
					}
				}
			}
		} // (effectMapping != -1)

		//Free up any remaining effects which weren't common between current and previous surface
		for(S32 iPrevEffect = 0; iPrevEffect < previousParticles.Size(); iPrevEffect++)
		{
			if(previousParticles[iPrevEffect].pEvent != 0)
			{
				if (DynamicEffect::Particle::C_EventInstance* pEventInstance = previousParticles[iPrevEffect].particleEvent.Object())
					pEventInstance->Free();
			}
		}
	} // (m_Refresh)
	

	//Update particle emitters
	if(m_EffectMapping != -1)
	{
		//Calculate rim light intensity if rim enabled
		AmaxWheelSurfaceEffect& surfaceWheelEffect = C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceEffects[m_EffectModifierMapping]->effects[m_EffectMapping];
		Maths::HVec3 rimLightColour = Maths::HVec3(0.f, 0.f, 0.f);

		for(S32 iParticle = 0; iParticle< m_SurfaceParticles.Size(); iParticle++)
		{
			if (m_SurfaceParticles[iParticle].pEvent != 0)
			{
				SurfaceParticle& surfaceEffect				= m_SurfaceParticles[iParticle];
				AmaxWheelParticleEventDescriptor* pWheel	= &surfaceEffect.pEvent->carFL;

				if(DynamicEffect::Particle::C_EventInstance* pEventInstance = surfaceEffect.particleEvent.Object())
				{
					float intensityTarget	= pWheel[m_UniqueIndex].intensity;
					float alphaTarget		= pWheel[m_UniqueIndex].alpha;
					
					if(surfaceEffect.pEvent->intensityCurve.Valid())
						intensityTarget		*= surfaceEffect.pEvent->intensityCurve.Evaluate(m_Evaluate[surfaceEffect.pEvent->trigger]);

					if(surfaceEffect.pEvent->alphaCurve.Valid())
						alphaTarget			*= surfaceEffect.pEvent->alphaCurve.Evaluate(m_Evaluate[surfaceEffect.pEvent->trigger]);
					
					if(surfaceEffect.pEvent->intensityRamp)
					{
						float intensityDiff = intensityTarget - surfaceEffect.intensityCurrent;
						if(Maths::Abs(intensityDiff) < surfaceEffect.pEvent->intensityStep)
							surfaceEffect.intensityCurrent	= intensityTarget;
						else
							surfaceEffect.intensityCurrent	+= Maths::Clamped(intensityDiff, -surfaceEffect.pEvent->intensityStep, surfaceEffect.pEvent->intensityStep);
					}
					else
						surfaceEffect.intensityCurrent	= intensityTarget;

					if(surfaceEffect.pEvent->alphaRamp)
					{
						float alphaDiff = alphaTarget - surfaceEffect.alphaCurrent;
						if(Maths::Abs(alphaDiff) < surfaceEffect.pEvent->alphaStep)
							surfaceEffect.alphaCurrent	= alphaTarget;
						else
							surfaceEffect.alphaCurrent	+= Maths::Clamped(alphaDiff, -surfaceEffect.pEvent->alphaStep, surfaceEffect.pEvent->alphaStep);
					}
					else
						surfaceEffect.alphaCurrent	= alphaTarget;

					surfaceEffect.intensityCurrent = Maths::Clamp(surfaceEffect.intensityCurrent, 0.0f, 1.0f);
					pEventInstance->SetVelocity(p_Velocity);
					pEventInstance->SetLighting(p_Lighting);
					pEventInstance->SetIntensity(surfaceEffect.intensityCurrent * p_fEmitScale * (m_OnGround ? 1.0f : 0.0f));
					pEventInstance->SetLifeScale(p_fLifeScale);
					pEventInstance->SetCollisionPlane(m_Position, m_Normal);
					pEventInstance->SetVisibilityFlags(visibilityFlags);

					// values from sorcery seem to be buggered so clamp here temporarily
					float alpha = Maths::Clamped(surfaceEffect.alphaCurrent, 0.0f, 1.0f);
					pEventInstance->SetAlpha(alpha);

					if (m_RimEvent && surfaceWheelEffect.rimLightEnable)
						rimLightColour += Maths::HVec3(surfaceWheelEffect.rimLightColour.r, surfaceWheelEffect.rimLightColour.g, surfaceWheelEffect.rimLightColour.b) * surfaceEffect.intensityCurrent * (m_OnGround ? 1.0f : 0.0f); 
				}
			}

			//Update intensity light
			if (m_RimEvent && m_RimPointLightHandle.Object())
			{
				if (rimLightColour.Length2() > 0.01f)
				{
					m_RimPointLightHandle->SetVisibilityFlags(visibilityFlags);

					//Calculate flicker position
					F32 currentPos = F32(m_RimPointLightFlickerPosition) + m_RimPointLightFlickerRest + surfaceWheelEffect.rimLightFrequency * m_Speed * p_fTimeStep;

					F32 currectPosFloor = Maths::Floor(currentPos);
					m_RimPointLightFlickerPosition = U32(currectPosFloor) % g_FlickerTableSize;
					m_RimPointLightFlickerRest = currentPos - currectPosFloor;

					F32 flickerValue = g_FlickerTable[(m_RimPointLightFlickerPosition + (m_UniqueIndex * g_FlickerTableSize / 2)) % g_FlickerTableSize];

					//Update colour
					m_RimPointLightHandle->SetColour(flickerValue * rimLightColour.X() * g_debugVehicleRimLightIntensityFactor, flickerValue * rimLightColour.Y() * g_debugVehicleRimLightIntensityFactor, flickerValue * rimLightColour.Z() * g_debugVehicleRimLightIntensityFactor);

					//Update position
					Maths::HVec3 rimLightPosition = m_Position + Maths::HVec3(((m_UniqueIndex % 2) == 0)?F32(-g_debugVehicleRimLightOffsetX):F32(g_debugVehicleRimLightOffsetX), F32(g_debugVehicleRimLightOffsetY), F32(g_debugVehicleRimLightOffsetZ)) * p_pParentMatrix->Rotation();
					F32	rimLightRadius = F32(g_debugVehicleRimLightRadius) + surfaceWheelEffect.rimLightRadiusRange * (flickerValue * 2.f - 1.f);
					m_RimPointLightHandle->SetDimensions(rimLightPosition, rimLightRadius);
					
				}
				else
				{
					m_RimPointLightHandle->SetVisibilityFlags(0);
				}
			}
		}
	} // (m_EffectMapping != -1)
	
	if(m_EffectMapping == -1)
		ClearParticles();
}

/*
===============
NAME: Update

NOTE: Update
===============
*/
void C_VehicleWheel::UpdateTracks(const Maths::C_RGBA_F32& p_Lighting, const Vehicle::C_WheelDamage* p_pWheelDamage, U32 visibilityFlags)
{
	if (!m_Update)
		return;

	PERF_EVENT("Wheel.UpdateTracks");
	
	float fRimTrackIntensity = Maths::Clamp((m_Speed - (float)g_debugRimTrackMinVelocity) / ((float)g_debugRimTrackMaxVelocity - (float)g_debugRimTrackMinVelocity), 0.0f, 1.0f);

	// refresh if any of the events have gone null
	// see if a valid event has expired
	if(!m_SurfaceTrack.event.Object() || !m_SurfaceTrack.track.Object())
		m_Refresh = true;

	// if the surface has changed we need to remove current emitters and request new ones
	if(m_Refresh)
	{		
		// clear current tracks
		SurfaceTrack previousTrack = m_SurfaceTrack;
		
		if(m_TrackMapping != -1)
		{
			AmaxWheelSurfaceTrack& surfaceTrack = C_GameEffectData::Instance().mAmaxVehicleWheelSurfaceTracks[m_TrackModifierMapping]->tracks[m_TrackMapping];
			m_SurfaceTrack.pEvent = 0;

			if(AmaxWheelSurfaceTrackEvent* pWheelSurfaceTrackEvent = surfaceTrack.event.Object())
			{
				bool bRequestEmitter = true;
				
				// see if unique ids match
				if(previousTrack.pEvent == pWheelSurfaceTrackEvent)
				{
					// they match so copy entire track track
					m_SurfaceTrack = previousTrack;
					previousTrack.pEvent = 0;
					bRequestEmitter = false;
				}

				// didn't match any events so see if we can at least match materials
				if(m_SurfaceTrack.pEvent == 0 && previousTrack.pEvent != 0)
				{
					if(DynamicEffect::Track::XtInterface::Material* pXtMaterial = pWheelSurfaceTrackEvent->material.Object())
					{
						if(previousTrack.materialID == pXtMaterial->pMaterial->GetIndex())
						{
							// materials match so copy track handle only, everything else needs to get setup again
							m_SurfaceTrack.track = previousTrack.track;
							// set event ID to 0 to indicate this needs to be set up again
							m_SurfaceTrack.pEvent = 0;

							previousTrack.pEvent = 0;
							bRequestEmitter = false;
						}
					}
				}

				// if we are requesting an emitter, or current emitter has gone null, get a new one
				if(bRequestEmitter || !m_SurfaceTrack.track.Object())
				{
					DynamicEffect::Track::C_Material* pMaterial;
					// if this event has a material then set it up here
					if(DynamicEffect::Track::XtInterface::Material* pXtMaterial = pWheelSurfaceTrackEvent->material.Object())
					{
						pMaterial = pXtMaterial->pMaterial;
						m_SurfaceTrack.materialID = pMaterial->GetIndex();
					}
					else
					{
						pMaterial = NULL;
						m_SurfaceTrack.materialID = 0xFFFFFFFF;
					}
					// request the emitter
					m_SurfaceTrack.track = m_pTrackRenderer->NewEmitter(pMaterial);
					// only reset alpha when we get a new track
					m_SurfaceTrack.alphaCurrent = 0.0f;
				}

				// setup the triggers and curves etc
				if(m_SurfaceTrack.pEvent == 0 && m_SurfaceTrack.track.Object())
				{
					m_SurfaceTrack.event = surfaceTrack.event;
					m_SurfaceTrack.pEvent = pWheelSurfaceTrackEvent;
					Maths::C_RGBA_F32 colour(Maths::Load<Maths::C_RGBA_F32>((MathsStorage::Float4*)&pWheelSurfaceTrackEvent->colour));
					m_SurfaceTrack.track->SetColour(colour);
				}
			}
		} // (m_TrackMapping != -1)

		// free up any remaining tracks which weren't common between current and previous surface
		if(previousTrack.pEvent != 0)
		{
			if(DynamicEffect::Track::C_Emitter* pEmitter = previousTrack.track.Object()) 
				pEmitter->Free();
		}
	} // (m_Refresh)

	// update tracks
	if(m_TrackMapping != -1)
	{
		if(DynamicEffect::Track::C_Emitter* pEmitter = m_SurfaceTrack.track.Object())
		{
			Maths::HVec3 vDirection(Maths::UNDEFINED);
			Maths::HVec3 vPosition(Maths::UNDEFINED);

			if(!m_OnGround)
			{
				vPosition = pEmitter->GetPosition();
				vDirection = pEmitter->GetDirection();
				m_SurfaceTrack.alphaCurrent = 0.0f;
			}
			else
			{
				vDirection = m_Velocity ^ m_Normal;
				vPosition = m_GroundPosition;

				F32 alphaTarget;
				if (p_pWheelDamage && (p_pWheelDamage->m_TyreBurst || p_pWheelDamage->m_Detached) && m_OnGround)
					alphaTarget = fRimTrackIntensity;
				else if(m_SurfaceTrack.pEvent->alphaCurve.Valid())
					alphaTarget = m_SurfaceTrack.pEvent->alphaCurve.Evaluate(m_Evaluate[m_SurfaceTrack.pEvent->trigger]);
				else
					alphaTarget = 1.0f;
			
				if(m_SurfaceTrack.pEvent->alphaRamp)
				{
					float alphaDiff = alphaTarget - m_SurfaceTrack.alphaCurrent;
					if(Maths::Abs(alphaDiff) < m_SurfaceTrack.pEvent->alphaStep)
						m_SurfaceTrack.alphaCurrent = alphaTarget;
					else
						m_SurfaceTrack.alphaCurrent += Maths::Clamped(alphaDiff, -m_SurfaceTrack.pEvent->alphaStep, m_SurfaceTrack.pEvent->alphaStep);
				}
				else
					m_SurfaceTrack.alphaCurrent = alphaTarget;
				
			}

			F32 trackStep = 0.4f;
			//pEmitter->SetParameters(m_SurfaceTrack.alphaCurrent != 0.0f, m_SurfaceTrack.alphaCurrent, vPosition, vDirection, m_Width * 0.5f, trackStep, 0);
#ifndef CFG_GOLD
			pEmitter->SetParameters(m_SurfaceTrack.alphaCurrent != 0.0f, m_SurfaceTrack.alphaCurrent, vPosition, vDirection, m_TrackWidth, trackStep, Game::C_ReplayDirector::Instance().GetScaledReplayTime());
#else
			pEmitter->SetParameters(m_SurfaceTrack.alphaCurrent != 0.0f, m_SurfaceTrack.alphaCurrent, vPosition, vDirection, m_TrackWidth, trackStep, 0);
#endif
			pEmitter->SetLighting(p_Lighting);

			pEmitter->SetVisibilityFlags(visibilityFlags);
		}
	} // (m_TrackMapping != -1)

	if(m_TrackMapping == -1)
		ClearTracks();
}

void C_VehicleWheel::ClearParticles()
{
	for(S32 iParticle = 0; iParticle < m_SurfaceParticles.Size(); iParticle++)
	{
		if (m_SurfaceParticles[iParticle].pEvent != 0)
		{
			if (DynamicEffect::Particle::C_EventInstance* pEventInstance = m_SurfaceParticles[iParticle].particleEvent.Object())
				pEventInstance->Free();
		}
	}
	m_SurfaceParticles.Clear();
}

void C_VehicleWheel::ClearTracks()
{
	if (DynamicEffect::Track::C_Emitter* pEmitter = m_SurfaceTrack.track.Object())
		pEmitter->Free();
	m_SurfaceTrack.pEvent = 0;	
}

void C_VehicleWheel::ClearMisc()
{
	if(DynamicEffect::Particle::C_EventInstance* pEventInstance = m_SparksEvent.Object())
		pEventInstance->Free();
}

void C_VehicleWheel::UpdateWheelDirection()
{
	const Maths::HVec3 vAxisZ = Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ();
	Maths::HVec3 vForwardDir((float)g_debugVehicleWheelForwardDirX, (float)g_debugVehicleWheelForwardDirY, (float)g_debugVehicleWheelForwardDirZ);
	Maths::HVec3 vReverseDir((float)g_debugVehicleWheelReverseDirX, (float)g_debugVehicleWheelReverseDirY, (float)g_debugVehicleWheelReverseDirZ);
	vForwardDir.SafeNormalise(vAxisZ);
	vReverseDir.SafeNormalise(vAxisZ);

	// Flip X direction for wheels on left.
	if (m_UniqueIndex == 0 || m_UniqueIndex == 2)
	{
		vForwardDir.X(-vForwardDir.X());
		vReverseDir.X(-vReverseDir.X());
	}

	m_vForwardDirectionQuat = DynamicEffect::GetQuaternionFromTo<Maths::C_Quaternion>(vAxisZ, vForwardDir);
	m_vReverseDirectionQuat = DynamicEffect::GetQuaternionFromTo<Maths::C_Quaternion>(vAxisZ, vReverseDir);
}

void C_VehicleWheel::Clear()
{
	ClearParticles();
	ClearTracks();
	ClearMisc();
}

void C_VehicleWheel::ClearDeferred()
{
	m_ClearDeferred = true;
}

} //namespace Actors

} //namespace Game

