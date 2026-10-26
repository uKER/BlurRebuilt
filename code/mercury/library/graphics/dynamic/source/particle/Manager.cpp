#include "stdafx.h"

#include "Dynamic/Particle/Manager.h"

#include "Dynamic/Particle/Emitter.h"
#include "Dynamic/Particle/Event.h"
#include "Dynamic/Particle/EventInstance.h"
#include "Dynamic/Particle/Force.h"
#include "Dynamic/Particle/EffectInterface.h"
#include "Dynamic/Particle/SuperEffect.h"
#include "Dynamic/Particle/PhysicsSettings.h"	// [Blur2PC]

#include "Dynamic/Renderer.h"
#include "Dynamic/TypeIndex.h"

#include "Dynamic/Shaders/Constants.h"

#include "Render/Utils/Globals.h"
#include "Render/State/RenderState.h"
#include "Render/State/Fog.h"

#include "SceneGraph/Util/DebugRenderVisitor.h"
#include "SceneGraph/View.h"

#include "Display/Context/Context.h"

#include <system\DebugLog.h>
#include <system\memory.h>
#include <system\Debug\I_DebugRender.h>

#include <btl/containers/radix.h>

// [Blur2PC] the frame rate particle effect data is tuned for; see PhysicsSettings.h.
F32 DynamicEffect::Particle::g_ParticleSourceFrameRate = 60.0f;

using namespace DynamicEffect;
using namespace DynamicEffect::Particle;

namespace DynamicEffect
{
	extern Debug::Logger &g_Log;

namespace Particle
{
	EXTERN_SHADER(vsParticlePointSprite);
	EXTERN_SHADER(vsParticle);
	EXTERN_SHADER(vsParticle2D);
	EXTERN_SHADER(vsParticleUVW);
	EXTERN_SHADER(vsParticleNormalMap);
	EXTERN_SHADER(vsParticleNdotL);
	EXTERN_SHADER(vsParticleSoft);
	EXTERN_SHADER(vsParticleDecal);

	EXTERN_SHADER(vsParticlePointSpritePointLights);
	EXTERN_SHADER(vsParticlePointLights);
	EXTERN_SHADER(vsParticle2DPointLights);
	EXTERN_SHADER(vsParticleUVWPointLights);
	EXTERN_SHADER(vsParticleNormalMapPointLights);
	EXTERN_SHADER(vsParticleNdotLPointLights);
	EXTERN_SHADER(vsParticleSoftPointLights);
	EXTERN_SHADER(vsParticleDecalPointLights);

	EXTERN_SHADER(psParticle);
	EXTERN_SHADER(psParticleA8);
	EXTERN_SHADER(psParticleNormalMap);
	EXTERN_SHADER(psParticlePointSprite);
	EXTERN_SHADER(psParticleSoft);
	EXTERN_SHADER(psParticleSoftA8);
	EXTERN_SHADER(psParticleDecal);

	EXTERN_SHADER(psParticleHeightFog);
	EXTERN_SHADER(psParticleA8HeightFog);
	EXTERN_SHADER(psParticleNormalMapHeightFog);
	EXTERN_SHADER(psParticlePointSpriteHeightFog);
	EXTERN_SHADER(psParticleSoftHeightFog);
	EXTERN_SHADER(psParticleSoftA8HeightFog);
	EXTERN_SHADER(psParticleDecalHeightFog);

	EXTERN_SHADER(psParticleHDRU32);
	EXTERN_SHADER(psParticleA8HDRU32);
	EXTERN_SHADER(psParticleNormalMapHDRU32);
	EXTERN_SHADER(psParticlePointSpriteHDRU32);
	EXTERN_SHADER(psParticleSoftHDRU32);
	EXTERN_SHADER(psParticleSoftA8HDRU32);
	EXTERN_SHADER(psParticleDecalHDRU32);

	EXTERN_SHADER(psParticleHDR);
	EXTERN_SHADER(psParticleA8HDR);
	EXTERN_SHADER(psParticleNormalMapHDR);
	EXTERN_SHADER(psParticlePointSpriteHDR);
	EXTERN_SHADER(psParticleSoftHDR);
	EXTERN_SHADER(psParticleSoftA8HDR);
	EXTERN_SHADER(psParticleDecalHDR);

	const U32 g_nBlocks	= 512;

	Debug::Float g_debugParticleTimeScale("Graphics.Dynamic.Particle.TimeScale", 1.0f, Debug::Range(0.f, 1.0f), Debug::Step(0.05f));

	Debug::Bool	g_Gather("Graphics.Dynamic.Particle.Gather", true);
	Debug::Bool	g_MultiThreadedEmitterUpdate("Graphics.Dynamic.Particle.MultiThreadedEmittterUpdate", true);
	Debug::Bool	g_MultiThreadedEffectUpdate("Graphics.Dynamic.Particle.MultiThreadedEffectUpdate", true);
	Debug::Bool	g_MultiThreadedSpawn("Graphics.Dynamic.Particle.MultiThreadedSpawn", true);
#ifndef CFG_SPEED
	Debug::Bool	g_MultiThreadedVertexBuild("Graphics.Dynamic.Particle.MultiThreadedVertexBuild", true);
#endif
	Debug::Bool	g_SortSpawningEmitters("Graphics.Dynamic.Particle.SortSpawningEmitters", true);
	
	Debug::Bool	g_debugParticleQualityOverride("Graphics.Dynamic.Particle.QualityOverride", false);
	Debug::Float g_debugParticleQuality("Graphics.Dynamic.Particle.Quality", 1.0f, Debug::Range(0.f, 1.0f), Debug::Step(0.05f));
	Debug::Float g_debugParticleSleepSpeedSq("Graphics.Dynamic.Particle.Physics.SleepSpeedSq", 0.01f, Debug::Range(0.0f, 1.0f));

	Debug::Float g_WindX("Graphics.Dynamic.Particle.Wind.X", 0.0f, Debug::Range(-10.f, 10.0f), Debug::Step(0.1f));
	Debug::Float g_WindY("Graphics.Dynamic.Particle.Wind.Y", 0.0f, Debug::Range(-10.f, 10.0f), Debug::Step(0.1f));
	Debug::Float g_WindZ("Graphics.Dynamic.Particle.Wind.Z", 0.0f, Debug::Range(-10.f, 10.0f), Debug::Step(0.1f));
	Debug::Float g_WindRandomX("Graphics.Dynamic.Particle.Wind.RandomX", 1.0f, Debug::Range(0.f, 2.0f), Debug::Step(0.1f));
	Debug::Float g_WindRandomY("Graphics.Dynamic.Particle.Wind.RandomY", 1.0f, Debug::Range(0.f, 2.0f), Debug::Step(0.1f));
	Debug::Float g_WindRandomZ("Graphics.Dynamic.Particle.Wind.RandomZ", 1.0f, Debug::Range(0.f, 2.0f), Debug::Step(0.1f));
	Debug::Float g_WindRandomAnimSpeed("Graphics.Dynamic.Particle.Wind.RandomAnimSpeed", 0.1f, Debug::Range(0.f, 2.0f), Debug::Step(0.1f));

	Debug::Bool g_debugParticleSortParticles("Graphics.Dynamic.Particle.Build.SortParticles", true);
	
	Debug::Bool g_debugParticleUseDiffuseMap("Graphics.Dynamic.Particle.Materials.UseDiffuseMap", true);
	Debug::Bool g_debugParticleUseNormalMap("Graphics.Dynamic.Particle.Materials.UseNormalMap", true);
	Debug::Bool g_debugParticleUseBlending("Graphics.Dynamic.Particle.Materials.UseBlending", true);
	Debug::Bool g_debugParticleUseSoft("Graphics.Dynamic.Particle.Materials.UseSoftParticles", true);
	Debug::Bool g_debugParticleUseDecal("Graphics.Dynamic.Particle.Materials.UseDecalShader", true);
	
	Debug::Float g_debugParticleDecalYOffset("Graphics.Dynamic.Particle.Materials.DecalYOffset", 0.01f, Debug::Range(-1.f, 1.0f), Debug::Step(0.001f));
	
	Debug::Float g_debugParticleSoftBias("Graphics.Dynamic.Particle.Materials.SoftParticleBias", 0.1f, Debug::Range(-1.f, 1.0f), Debug::Step(0.01f));
	Debug::Float g_debugParticleDecalClipScale1("Graphics.Dynamic.Particle.Materials.DecalClipScale1", 1.0f, Debug::Range(-10.f, 10.0f), Debug::Step(0.01f));
	Debug::Float g_debugParticleDecalClipScale2("Graphics.Dynamic.Particle.Materials.DecalClipScale2", 6.0f, Debug::Range(-10.f, 10.0f), Debug::Step(0.01f));
	Debug::Float g_debugParticleDecalClipDiffScale1("Graphics.Dynamic.Particle.Materials.DecalClipDiffScale1", 0.0f, Debug::Range(-10.f, 10.0f), Debug::Step(0.01f));
	Debug::Float g_debugParticleDecalClipDiffScale2("Graphics.Dynamic.Particle.Materials.DecalClipDiffScale2", 6.0f, Debug::Range(-10.f, 10.0f), Debug::Step(0.01f));

	Debug::Bool g_debugParticlesMemoryInfo("Graphics.SceneGraph.DebugRender.ParticleManager.MemoryInfo", false);

C_ParticleManager::C_ParticleManager(C_Renderer* pRenderer) : I_DynamicEffectManager(pRenderer, E_ManagerTypeIndex_Particle)
{
	MEMORY_TRACKER_PUSH("graphics.dynamic.particlemanager");

	m_ParticleTrailBlockManager.SetBlockSize(32);
	m_ParticleBlockManager16.SetBlockSize(32);
	m_ParticleBlockManager32.SetBlockSize(32);
	m_ParticleBlockManager48.SetBlockSize(32);
	m_ParticleBlockManager64.SetBlockSize(32);
	m_ParticleBlockManager80.SetBlockSize(32);
	m_ParticleBlockManager96.SetBlockSize(32);
	m_ParticleBlockManager112.SetBlockSize(32);

	m_ParticleTrailBlockManager.SetElementSize(8);
	m_ParticleBlockManager16.SetElementSize(16);
	m_ParticleBlockManager32.SetElementSize(32);
	m_ParticleBlockManager48.SetElementSize(48);
	m_ParticleBlockManager64.SetElementSize(64);
	m_ParticleBlockManager80.SetElementSize(80);
	m_ParticleBlockManager96.SetElementSize(96);
	m_ParticleBlockManager112.SetElementSize(112);

	m_ParticleTrailBlockManager.SetMaxBlocks(256);
	m_ParticleBlockManager16.SetMaxBlocks(256);
	m_ParticleBlockManager32.SetMaxBlocks(256);
	m_ParticleBlockManager48.SetMaxBlocks(256);
	m_ParticleBlockManager64.SetMaxBlocks(2048);
	m_ParticleBlockManager80.SetMaxBlocks(2048);
	m_ParticleBlockManager96.SetMaxBlocks(1024);
	m_ParticleBlockManager112.SetMaxBlocks(256);

	C_Particle3D_R1D::CreateFormat();
	C_Particle3D_R1D_CP::CreateFormat();
	C_Particle3D_R1D_P::CreateFormat();
	C_Particle3D_R3D::CreateFormat();
	C_Particle3D_RV3D::CreateFormat();
	C_Particle3D_RV3D_CP::CreateFormat();
	C_Particle3D::CreateFormat();
	C_Particle3D_P::CreateFormat();
	C_Particle3D_CP::CreateFormat();

	m_JobQueueGroup = Scheduler::CreateGroup();

	m_EffectsGroup = Scheduler::CreateGroup();
	m_SuperEffectsGroup = Scheduler::CreateGroup();
	m_EmittersGroup = Scheduler::CreateGroup();
	m_SuperEmittersGroup = Scheduler::CreateGroup();

	// vectors for storing stuff from other threads
	m_DeletedBlocks.Resize(32);
	m_DeletedSuperBlocks.Resize(32);
	m_SuperEffectUpdateData.Reserve(32);

	m_SpawningEmitters.Resize(128);
	m_SpawningCustomEmitters.Resize(128);
	for(S32 i = 0; i < 7; ++i)
		m_EmitterUpdateConstants.pRandom[i] = &m_Random[i];
	//m_Collisions.Reserve(128);

	// set some default material properties
	for(U32 i = 0; i < 64; ++i)
	{
		SetMaterial(0.5f, 5.0f, i);
	}

	m_EffectUpdateConstants.frameIndex = 0;

	m_ePixelShaderSet = E_PixelShaderSet_DepthFog;

	m_nEmitters = 0;
	m_nSuperEmitters = 0;
	m_nEventInstances = 0;

	m_EventInstanceCountMax = 0;
	m_EmitterCountMax = 0;
	m_ParticleCountMax = 0;

	m_iNoiseX = 0;
	m_iNoiseY = FP32(2.0f);
	m_iNoiseZ = FP32(4.0f);

	m_GlobalForce = Maths::HVec3(Maths::Constants<Maths::HF32>::Zero());
	m_vDefaultSpawnDirection = MathsStorage::Float4A16(0.0f, 1.0f, 0.0f, 0.0f);
	m_vParticleColour = MathsStorage::Float4A16(1.f, 1.f, 1.f, 1.f);
	m_vEmissiveColour = MathsStorage::Float4A16(1.f, 1.f, 1.f, 1.f);
	m_vLightDirection = MathsStorage::Float3(0.0f, 1.0f, 0.0f);
	m_fExpiryTime = 1.0f;
	m_fQuality = 1.0f;
	m_fQualityRcp = 1.0f;
	m_fCycle = 0.0f;
	
	m_fCullDistance = 1000.0f;
	m_fFreezeAmountScale = 1.0f;
	m_AutoUpdate = true;

	m_bUpdatingEmitters = false;
	
	m_pRenderState = &Render::C_RenderState::Instance();

	MEMORY_TRACKER_POP("graphics.dynamic.particlemanager");

#ifndef CFG_GOLD
	for(int i = 0; i < m_DebugMaxAllocatedBlocks.Size(); i++)
	{
		m_DebugMaxAllocatedBlocks[i] = 0;
	}
#endif
}

C_ParticleManager::~C_ParticleManager()
{
	Scheduler::DestroyGroup(m_JobQueueGroup);
	Scheduler::DestroyGroup(m_EffectsGroup);
	Scheduler::DestroyGroup(m_SuperEffectsGroup);
	Scheduler::DestroyGroup(m_EmittersGroup);
	Scheduler::DestroyGroup(m_SuperEmittersGroup);

	m_Emitters.Free();
	m_SuperEffectEmitters.Free();
	m_EventInstanceList.Free();

	while(m_EmitterSet.Begin() != m_EmitterSet.End())
	{
		C_Emitter* pEmitter = (*m_EmitterSet.Begin());
		delete pEmitter;
	}

	while(m_Forces.Begin() != m_Forces.End())
	{
		C_Force* pForce = (*m_Forces.Begin());
		delete pForce;
	}

	while(m_Events.Begin() != m_Events.End())
	{
		C_Event* pEvent = m_Events.Begin()->GetValue();
		delete pEvent;
	}

	while(m_EventInstances.Begin() != m_EventInstances.End())
	{
		C_EventInstance* pEvent = m_EventInstances.Begin()->GetValue();
		delete pEvent;
	}

	for(S32 i = 0; i < m_SuperEffects.Size(); ++i)
		delete m_SuperEffects[i];

	for(S32 i = 0; i < m_Effects.Size(); ++i)
		delete m_Effects[i];
}

void C_ParticleManager::ReserveEmitters(const U32 nEmitters)
{
	if(nEmitters > m_nEmitters)
	{
		for(U32 i = m_nEmitters; i < nEmitters; ++i)
		{
			m_Emitters.Cache(new C_Emitter(this, &m_Emitters, i));
		}
		m_nEmitters = nEmitters;
	}
}

void C_ParticleManager::ReserveSuperEmitters(const U32 nEmitters)
{
	if(nEmitters > m_nSuperEmitters)
	{
		for(U32 i = m_nSuperEmitters; i < nEmitters; ++i)
		{
			m_SuperEffectEmitters.Cache(new C_Emitter(this, &m_SuperEffectEmitters, i));
		}
		m_nSuperEmitters = nEmitters;
	}
}

void C_ParticleManager::ReserveEventInstances(const U32 nEventInstances)
{
	if(nEventInstances > m_nEventInstances)
	{
		for(U32 i = m_nEventInstances; i < nEventInstances; ++i)
		{
			m_EventInstanceList.Cache(new C_EventInstance(this));
		}
		m_nEventInstances = nEventInstances;
	}
}

bool C_ParticleManager::Create()
{
	RESOURCE_MEMORY_ID("DynamicEffect");

	return CreateShaders();
}

bool C_ParticleManager::CreateShaders()
{
	RESOURCE_MEMORY_ID("DynamicEffect");

// [Blur2PC] Run the existing PC shader initialization on the native DX11 backend.
#if defined(DEVICE_PCD3D11) || defined(DEVICE_PS3GCM)

	// no point lights
	{
		if (!Display::CreateVertexShaderFromFunc("vsParticlePointSprite", 0, &m_VertexShader[E_VertexShader_PointSprite][E_VertexShaderVariation_Default], REF_SHADER(vsParticlePointSprite)))
			return false;
		if (!Display::CreateVertexShaderFromFunc("vsParticle", 0, &m_VertexShader[E_VertexShader_UV][E_VertexShaderVariation_Default], REF_SHADER(vsParticle)))
			return false;
		if (!Display::CreateVertexShaderFromFunc("vsParticleUVW", 0, &m_VertexShader[E_VertexShader_UVW][E_VertexShaderVariation_Default], REF_SHADER(vsParticleUVW)))
			return false;
		if (!Display::CreateVertexShaderFromFunc("vsParticleNormalMap", 0, &m_VertexShader[E_VertexShader_NormalMap][E_VertexShaderVariation_Default], REF_SHADER(vsParticleNormalMap)))
			return false;
		if (!Display::CreateVertexShaderFromFunc("vsParticleNdotL", 0, &m_VertexShader[E_VertexShader_NdotL][E_VertexShaderVariation_Default], REF_SHADER(vsParticleNdotL)))
			return false;
		if (!Display::CreateVertexShaderFromFunc("vsParticleSoft", 0, &m_VertexShader[E_VertexShader_Soft][E_VertexShaderVariation_Default], REF_SHADER(vsParticleSoft)))
			return false;
		if (!Display::CreateVertexShaderFromFunc("vsParticle2D", 0, &m_VertexShader[E_VertexShader_TwoDimensional][E_VertexShaderVariation_Default], REF_SHADER(vsParticle2D)))
			return false;
		if (!Display::CreateVertexShaderFromFunc("vsParticleDecal", 0, &m_VertexShader[E_VertexShader_Decal][E_VertexShaderVariation_Default], REF_SHADER(vsParticleDecal)))
			return false;
	}

	// point lights
	{
		if (!Display::CreateVertexShaderFromFunc("vsParticlePointSpritePointLights", 0, &m_VertexShader[E_VertexShader_PointSprite][E_VertexShaderVariation_PointLights], REF_SHADER(vsParticlePointSpritePointLights)))
			return false;
		if (!Display::CreateVertexShaderFromFunc("vsParticlePointLights", 0, &m_VertexShader[E_VertexShader_UV][E_VertexShaderVariation_PointLights], REF_SHADER(vsParticlePointLights)))
			return false;
		if (!Display::CreateVertexShaderFromFunc("vsParticleUVWPointLights", 0, &m_VertexShader[E_VertexShader_UVW][E_VertexShaderVariation_PointLights], REF_SHADER(vsParticleUVWPointLights)))
			return false;
		if (!Display::CreateVertexShaderFromFunc("vsParticleNormalMapPointLights", 0, &m_VertexShader[E_VertexShader_NormalMap][E_VertexShaderVariation_PointLights], REF_SHADER(vsParticleNormalMapPointLights)))
			return false;
		if (!Display::CreateVertexShaderFromFunc("vsParticleNdotLPointLights", 0, &m_VertexShader[E_VertexShader_NdotL][E_VertexShaderVariation_PointLights], REF_SHADER(vsParticleNdotLPointLights)))
			return false;
		if (!Display::CreateVertexShaderFromFunc("vsParticleSoftPointLights", 0, &m_VertexShader[E_VertexShader_Soft][E_VertexShaderVariation_PointLights], REF_SHADER(vsParticleSoftPointLights)))
			return false;
		if (!Display::CreateVertexShaderFromFunc("vsParticle2DPointLights", 0, &m_VertexShader[E_VertexShader_TwoDimensional][E_VertexShaderVariation_PointLights], REF_SHADER(vsParticle2DPointLights)))
			return false;
		if (!Display::CreateVertexShaderFromFunc("vsParticleDecalPointLights", 0, &m_VertexShader[E_VertexShader_Decal][E_VertexShaderVariation_PointLights], REF_SHADER(vsParticleDecalPointLights)))
			return false;
	}

	// see if we need to use shaders which do tonemapping themselves
	if(m_pRenderState->ShaderHDR())
	{
		PostProcess::E_HighDynamicRangeType eHDR = m_pRenderState->HDR()->GetType();

		// choose which set of shaders we need to use - we only have depth fog for shader tonemap at the moment
		if(eHDR == PostProcess::E_HighDynamicRangeType_ShaderU32)
			m_ePixelShaderSet = E_PixelShaderSet_DepthFog_HDRU32;
		else if(eHDR == PostProcess::E_HighDynamicRangeType_Shader)
			m_ePixelShaderSet = E_PixelShaderSet_DepthFog_HDR;
	}
	else
	{
		// choose set of shaders based on height fog settings
		const Render::C_Fog* pFog = m_pRenderState->Fog();

		if(pFog->HeightFog() == Render::E_HeightFog_Enable)
			m_ePixelShaderSet = E_PixelShaderSet_DepthHeightFog;
		else
			m_ePixelShaderSet = E_PixelShaderSet_DepthFog;
	}
	
	if(m_ePixelShaderSet == E_PixelShaderSet_DepthFog_HDRU32)
	{
		if (!Display::CreatePixelShaderFromFunc("DontShip/Shaders/psParticle", 0, &m_PixelShader[E_PixelShader_Default], REF_SHADER(psParticleHDRU32)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticleA8", 0, &m_PixelShader[E_PixelShader_A8], REF_SHADER(psParticleA8HDRU32)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticleNormalMap", 0, &m_PixelShader[E_PixelShader_NormalMap], REF_SHADER(psParticleNormalMapHDRU32)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticleSoft", 0, &m_PixelShader[E_PixelShader_Soft], REF_SHADER(psParticleSoftHDRU32)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticleSoftA8", 0, &m_PixelShader[E_PixelShader_SoftA8], REF_SHADER(psParticleSoftA8HDRU32)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticleDecal", 0, &m_PixelShader[E_PixelShader_Decal], REF_SHADER(psParticleDecalHDRU32)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticlePointSprite", 0, &m_PixelShader[E_PixelShader_PointSprite], REF_SHADER(psParticlePointSpriteHDRU32)))
			return false;
	}
	else if(m_ePixelShaderSet == E_PixelShaderSet_DepthFog_HDR)
	{
		if (!Display::CreatePixelShaderFromFunc("DontShip/Shaders/psParticle", 0, &m_PixelShader[E_PixelShader_Default], REF_SHADER(psParticleHDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticleA8", 0, &m_PixelShader[E_PixelShader_A8], REF_SHADER(psParticleA8HDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticleNormalMap", 0, &m_PixelShader[E_PixelShader_NormalMap], REF_SHADER(psParticleNormalMapHDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticleSoft", 0, &m_PixelShader[E_PixelShader_Soft], REF_SHADER(psParticleSoftHDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticleSoftA8", 0, &m_PixelShader[E_PixelShader_SoftA8], REF_SHADER(psParticleSoftA8HDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticleDecal", 0, &m_PixelShader[E_PixelShader_Decal], REF_SHADER(psParticleDecalHDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticlePointSprite", 0, &m_PixelShader[E_PixelShader_PointSprite], REF_SHADER(psParticlePointSpriteHDR)))
			return false;
	}
	else if(m_ePixelShaderSet == E_PixelShaderSet_DepthFog)
	{
		if (!Display::CreatePixelShaderFromFunc("DontShip/Shaders/psParticle", 0, &m_PixelShader[E_PixelShader_Default], REF_SHADER(psParticle)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticleA8", 0, &m_PixelShader[E_PixelShader_A8], REF_SHADER(psParticleA8)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticleNormalMap", 0, &m_PixelShader[E_PixelShader_NormalMap], REF_SHADER(psParticleNormalMap)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticleSoft", 0, &m_PixelShader[E_PixelShader_Soft], REF_SHADER(psParticleSoft)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticleSoftA8", 0, &m_PixelShader[E_PixelShader_SoftA8], REF_SHADER(psParticleSoftA8)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticleDecal", 0, &m_PixelShader[E_PixelShader_Decal], REF_SHADER(psParticleDecal)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticlePointSprite", 0, &m_PixelShader[E_PixelShader_PointSprite], REF_SHADER(psParticlePointSprite)))
			return false;
	}
	else if(m_ePixelShaderSet == E_PixelShaderSet_DepthHeightFog)
	{
		if (!Display::CreatePixelShaderFromFunc("psParticle", 0, &m_PixelShader[E_PixelShader_Default], REF_SHADER(psParticleHeightFog)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticleA8", 0, &m_PixelShader[E_PixelShader_A8], REF_SHADER(psParticleA8HeightFog)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticleNormalMap", 0, &m_PixelShader[E_PixelShader_NormalMap], REF_SHADER(psParticleNormalMapHeightFog)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticleSoft", 0, &m_PixelShader[E_PixelShader_Soft], REF_SHADER(psParticleSoftHeightFog)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticleSoftA8", 0, &m_PixelShader[E_PixelShader_SoftA8], REF_SHADER(psParticleSoftA8HeightFog)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticleDecal", 0, &m_PixelShader[E_PixelShader_Decal], REF_SHADER(psParticleDecalHeightFog)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("psParticlePointSprite", 0, &m_PixelShader[E_PixelShader_PointSprite], REF_SHADER(psParticlePointSpriteHeightFog)))
			return false;
	}
	else
	{
		LOGERROR(g_Log, "ParticleManager: Couldn't create shaders for current shader set!\n");
		return false;
	}
#endif

	// pixel shader parameters
	for(U32 i = 0; i < E_PixelShader_Count; ++i)
	{
		if(m_PixelShader[i].IsCreated())
		{
			m_LuminanceParam[i] = m_PixelShader[i].GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE, "g_vLuminance");
			
#ifdef PLATFORM_PS3
			m_FogColourParam[i] = m_PixelShader[i].GetParameter(CP_RENDER_FOG_COLOUR, "g_FogColour");
			m_DepthFogParam[i] = m_PixelShader[i].GetParameter(CP_RENDER_FOG_DEPTH_SETTINGS, "g_DepthFog");
			m_HeightFogParam[i] = m_PixelShader[i].GetParameter(CP_RENDER_FOG_HEIGHT_SETTINGS, "g_HeightFog");
			m_DepthScaleParam[i] = m_PixelShader[i].GetParameter(CP_DYNAMIC_PARTICLE_DEPTHSCALE, "g_vDepthScale");
			m_DecalScaleParam[i] = m_PixelShader[i].GetParameter(CP_DYNAMIC_PARTICLE_DECALSCALE, "g_vDecalScale");
			m_ProjScaleParam[i] = m_PixelShader[i].GetParameter(CP_DYNAMIC_PARTICLE_PROJSCALE, "g_vProjScale");
#endif
		}
	}

	return true;
}

void C_ParticleManager::Destroy()
{
	for(S32 i = 0; i < m_SuperEffects.Size(); ++i)
		m_SuperEffects[i]->Destroy();

	for(S32 i = 0; i < m_Effects.Size(); ++i)
		m_Effects[i]->Destroy();

	DestroyShaders();
}

void C_ParticleManager::DestroyShaders()
{
	for(U32 i = 0; i < E_VertexShader_Count; ++i)
	{
		for(U32 j = 0; j < E_VertexShaderVariation_Count; ++j)
		{
			if(m_VertexShader[i][j].IsCreated())
				Display::Device()->Destroy(&m_VertexShader[i][j]);
		}
	}

	for(U32 i = 0; i < E_PixelShader_Count; ++i)
	{
		if(m_PixelShader[i].IsCreated())
			Display::Device()->Destroy(&m_PixelShader[i]);
	}
}

void C_ParticleManager::RecreateShaders()
{
	DestroyShaders();
	CreateShaders();
}

bool C_ParticleManager::CreateVolatile()
{
	bool bCreated = true;
	
	return bCreated;
}

bool C_ParticleManager::Initialise()
{
	bool bInitialised = true;
	
	return bInitialised;
}

void C_ParticleManager::Shutdown()
{
	
}

void C_ParticleManager::DestroyVolatile()
{
	
}

VectorBlockManager<ParticleBlock>* C_ParticleManager::GetParticleBlockManager(const U32 iElementSize)
{
	switch(iElementSize)
	{
		case 16:
			return &m_ParticleBlockManager16;
		case 32:
			return &m_ParticleBlockManager32;
		case 48:
			return &m_ParticleBlockManager48;
		case 64:
			return &m_ParticleBlockManager64;
		case 80:
			return &m_ParticleBlockManager80;
		case 96:
			return &m_ParticleBlockManager96;
		case 112:
			return &m_ParticleBlockManager112;
		default:
			return NULL;
	}
}

void C_ParticleManager::SetQuality(F32 fQuality)
{
	fQuality = Maths::Clamped(fQuality, 0.0f, 1.0f);

	m_fQuality = fQuality;
	m_fQualityRcp = fQuality != 0.0f ? 1.0f / fQuality : 10000000.0f;
}

void C_ParticleManager::SetFreezeAmountScale(F32 fFreezeAmountScale)
{
	if(fFreezeAmountScale < 0.0f || fFreezeAmountScale > 1.0f)
		LOGWARN(g_Log, "DynamicEffect: %f is not a valid FreezeAmountScale.\n", fFreezeAmountScale);
	else
		m_fFreezeAmountScale = fFreezeAmountScale;
}

void C_ParticleManager::SetDefaultSpawnDirection(const Maths::HVec3& vDirection)
{
	Maths::HVec3 vNormalisedDirection = Maths::Normalise(vDirection);
	Maths::Store(vNormalisedDirection, &m_vDefaultSpawnDirection);
}

void C_ParticleManager::SetDefaultSpawnDirection(const MathsStorage::Float4A16& vDirection)
{
	Maths::HVec3 vNormalisedDirection = Maths::Load<Maths::HVec3>(&vDirection);
	vNormalisedDirection.Normalise();
	Maths::Store(vNormalisedDirection, &m_vDefaultSpawnDirection);
}

void C_ParticleManager::BeginFrame() const
{
	Scheduler::Flush(m_JobQueueGroup);
}

Debug::Bool	g_DebugRenderParticleManager	("Graphics.SceneGraph.DebugRender.ParticleManager", true);
Debug::Bool	g_DebugRenderParticleManagerEmitters	("Graphics.SceneGraph.DebugRender.ParticleManager.Emitters", true);
Debug::Bool	g_DebugRenderParticleManagerInfo	("Graphics.SceneGraph.DebugRender.ParticleManager.Info", true);

void C_ParticleManager::Accept(SceneGraph::I_Visitor& visitor) const
{
	if(visitor.GetType() == 1 && g_DebugRenderParticleManager)
	{
		if(g_DebugRenderParticleManagerEmitters)
			DisplayDebugGraphics();

		MathsStorage::Int2 position(10, 220);

		if(g_DebugRenderParticleManagerInfo)
		{
			DisplayDebugInfo(position);
		}
		if(g_debugParticlesMemoryInfo)
		{
			DisplayDebugMemoryInfo(position);
		}
	}
}

void C_ParticleManager::OnGather(const SceneGraph::C_View& view) const
{
	if(!g_Gather)
		return;

	PERF_EVENT("DynamicEffect.GatherParticles");

	for(S32 i = 0; i < m_ActiveSuperEffects.Size(); ++i)
		m_ActiveSuperEffects[i]->Gather(view);

	for(S32 i = 0; i < m_ActiveEffects.Size(); ++i)
		m_ActiveEffects[i]->Gather(view);
}

MathsStorage::Int2 C_ParticleManager::DisplayDebugInfo(MathsStorage::Int2& position) const
{
#ifndef CFG_GOLD
	m_EmitterCountMax = m_Emitters.m_ActiveElements > m_EmitterCountMax ? m_Emitters.m_ActiveElements : m_EmitterCountMax;
	m_EventInstanceCountMax = m_EventInstanceList.m_ActiveElements > m_EventInstanceCountMax ? m_EventInstanceList.m_ActiveElements : m_EventInstanceCountMax;

	Util::C_String128	str;
	str.Printf("Events: %d\0", m_Events.Size());
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.y += 25;
	
	str.Printf("Emitters: %d/%d (%d)\0", m_Emitters.m_ActiveElements, m_Emitters.m_ActiveElements + m_Emitters.m_CachedElements, m_EmitterCountMax);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.y += 25;

	str.Printf("Event Instances: %d/%d (%d)\0", m_EventInstanceList.m_ActiveElements, m_EventInstanceList.m_ActiveElements + m_EventInstanceList.m_CachedElements, m_EventInstanceCountMax);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.y += 25;

	S32 particleCount = 0;
	for(S32 i = 0; i < m_ActiveEffects.Size(); ++i)
		particleCount += m_ActiveEffects[i]->GetNumParticles();

	m_ParticleCountMax = particleCount > m_ParticleCountMax ? particleCount : m_ParticleCountMax;

	str.Printf("Particles: %d (%d)\0", particleCount, m_ParticleCountMax);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.y += 25;

	/*str.Printf("BlockManagers:\0");
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.y += 15;

	position = DisplayBlockManagerDebugInfo(position, &m_ParticleTrailBlockManager);
	position = DisplayBlockManagerDebugInfo(position, &m_ParticleBlockManager16);
	position = DisplayBlockManagerDebugInfo(position, &m_ParticleBlockManager32);
	position = DisplayBlockManagerDebugInfo(position, &m_ParticleBlockManager48);
	position = DisplayBlockManagerDebugInfo(position, &m_ParticleBlockManager64);
	position = DisplayBlockManagerDebugInfo(position, &m_ParticleBlockManager80);
	position = DisplayBlockManagerDebugInfo(position, &m_ParticleBlockManager96);
	position = DisplayBlockManagerDebugInfo(position, &m_ParticleBlockManager112);*/

	position.y += 15;
#endif

	return position;
}

MathsStorage::Int2 C_ParticleManager::DisplayDebugMemoryInfo(MathsStorage::Int2& position) const
{
#ifndef CFG_GOLD
	Util::C_String128	str;
	str.Printf("Trail Block Mngr: %d/%d (Max: %d)\0", m_ParticleTrailBlockManager.GetAllocatedBlocks(), m_ParticleTrailBlockManager.GetMaxBlocks(), m_DebugMaxAllocatedBlocks[0]);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.y += 25;

	str.Printf("Block Mngr 16: %d/%d (Max: %d)\0", m_ParticleBlockManager16.GetAllocatedBlocks(), m_ParticleBlockManager16.GetMaxBlocks(), m_DebugMaxAllocatedBlocks[1]);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.y += 25;

	str.Printf("Block Mngr 32: %d/%d (Max: %d)\0", m_ParticleBlockManager32.GetAllocatedBlocks(), m_ParticleBlockManager32.GetMaxBlocks(), m_DebugMaxAllocatedBlocks[2]);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.y += 25;

	str.Printf("Block Mngr 48: %d/%d (Max: %d)\0", m_ParticleBlockManager48.GetAllocatedBlocks(), m_ParticleBlockManager48.GetMaxBlocks(), m_DebugMaxAllocatedBlocks[3]);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.y += 25;

	str.Printf("Block Mngr 64: %d/%d (Max: %d)\0", m_ParticleBlockManager64.GetAllocatedBlocks(), m_ParticleBlockManager64.GetMaxBlocks(), m_DebugMaxAllocatedBlocks[4]);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.y += 25;

	str.Printf("Block Mngr 80: %d/%d (Max: %d)\0", m_ParticleBlockManager80.GetAllocatedBlocks(), m_ParticleBlockManager80.GetMaxBlocks(), m_DebugMaxAllocatedBlocks[5]);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.y += 25;

	str.Printf("Block Mngr 96: %d/%d (Max: %d)\0", m_ParticleBlockManager96.GetAllocatedBlocks(), m_ParticleBlockManager96.GetMaxBlocks(), m_DebugMaxAllocatedBlocks[6]);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.y += 25;

	str.Printf("Block Mngr 112: %d/%d (Max: %d)\0", m_ParticleBlockManager112.GetAllocatedBlocks(), m_ParticleBlockManager112.GetMaxBlocks(), m_DebugMaxAllocatedBlocks[7]);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.y += 25;
#endif

	return position;
}


MathsStorage::Int2 C_ParticleManager::DisplayBlockManagerDebugInfo(MathsStorage::Int2 position, const BlockManagerBase* pBlockManager) const
{
	Util::C_String128	str;
	str.Printf("%d:\0", pBlockManager->GetElementSize());
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y + 10), str);
	str.Printf("%d/%d\0", pBlockManager->GetAllocatedBlocks(), pBlockManager->GetMaxBlocks());
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x + 60, position.y + 10), str);
	
	position.x += 140;
	F32 maxWidth = 1024.f - (F32)position.x - 100.f;
	F32 usedWidth = ((F32)maxWidth * ((F32)pBlockManager->GetAllocatedBlocks() / (F32)pBlockManager->GetMaxBlocks()));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dBar(Maths::HVec2(position.x, position.y), Maths::HVec2(usedWidth, 20.f), Maths::RGBA(255, 0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dBar(Maths::HVec2(position.x + usedWidth, position.y), Maths::HVec2(maxWidth - usedWidth, 20.f), Maths::RGBA(0, 255, 0, 255));
	position.x -= 140;

	position.y += 25;

	return position;
}

void C_ParticleManager::CleanEmitterDefinition(const EmitterDefinition* pDefinition)
{
	for(S32 i = 0; i < m_Emitters.m_ActiveElements;)
	{
		C_Emitter* pEmitter = m_Emitters.m_Active[i];
		if(pEmitter->GetDefinition() == pDefinition)
			m_Emitters.Free(pEmitter);
		else
			++i;
	}

	for(S32 i = 0; i < m_SuperEffectEmitters.m_ActiveElements;)
	{
		C_Emitter* pEmitter = m_SuperEffectEmitters.m_Active[i];
		if(pEmitter->GetDefinition() == pDefinition)
			m_SuperEffectEmitters.Free(pEmitter);
		else
			++i;
	}
}

// virtual
void C_ParticleManager::CleanEffect(const I_DynamicEffect *pEffect)
{
	for(S32 i = 0; i < m_Emitters.m_ActiveElements;)
	{
		C_Emitter* pEmitter = m_Emitters.m_Active[i];
		if(pEmitter->GetDefinition()->pEffect == pEffect)
			m_Emitters.Free(pEmitter);
		else
			++i;
	}
}

// virtual
void C_ParticleManager::CleanNode(const C_Node* pNode)
{
	for(S32 i = 0; i < m_ActiveEffects.Size(); ++i)
	{
		if(m_ActiveEffects[i]->GetNumBlocksLocal() > 0)
			m_ActiveEffects[i]->Clean(pNode);
	}

	for(S32 i = 0; i < m_ActiveSuperEffects.Size(); ++i)
	{
		if(m_ActiveSuperEffects[i]->GetNumBlocksLocal() > 0)
			m_ActiveSuperEffects[i]->Clean(pNode);
	}

	for(S32 i = 0; i < m_Emitters.m_ActiveElements; ++i)
	{
		C_Emitter* pEmitter = m_Emitters.m_Active[i];
		if(pEmitter->GetParentTransform() == pNode)
			pEmitter->Free();
	}
}

void C_ParticleManager::AddEvent(C_Event *pEvent)
{
	// make sure this effect doesnt already exist
	m_Events.Insert(pEvent);
}

void C_ParticleManager::CleanEvent(const C_Event *pEvent)
{
	// go through any active event instances and remove them
	for(S32 i = 0; i < m_EventInstanceList.m_ActiveElements; )
	{
		C_EventInstance* pEventInstance = m_EventInstanceList.m_Active[i];
		if(pEventInstance->m_pEvent == pEvent)
			pEventInstance->FreeRemove();
		else
			++i;
	}
}

void C_ParticleManager::RemoveEvent(C_Event *pEvent)
{
	Events::Iterator it = m_Events.Find(pEvent);
	assert(it != m_Events.End());
	if(it != m_Events.End())
		m_Events.Erase(it);
}

C_Event* C_ParticleManager::FindEvent(const Util::C_String& name)
{
	for(Events::Iterator pIterator = m_Events.Begin(); pIterator != m_Events.End(); pIterator++)
	{
		C_Event* pEvent = pIterator->GetValue();
		if(pEvent->m_Definition.name == name)
			return pEvent;
	}

	return NULL;
}

void C_ParticleManager::AddEmitter(C_Emitter *pEmitter)
{
	if(pEmitter->m_pManager == NULL)
		pEmitter->m_pManager = this;

	m_EmitterSet.Insert(pEmitter);
}

void C_ParticleManager::RemoveEmitter(C_Emitter *pEffect)
{
	Emitters::Iterator it = m_EmitterSet.Find(pEffect);
	assert(it != m_EmitterSet.End());
	if(it != m_EmitterSet.End())
		m_EmitterSet.Erase(it);
}

void C_ParticleManager::AddEventInstance(C_EventInstance *pEventInstance)
{
	if(pEventInstance->m_pRenderer == NULL)
		pEventInstance->m_pRenderer = this;

	m_EventInstances.Insert(pEventInstance);
}

void C_ParticleManager::RemoveEventInstance(C_EventInstance *pEffect)
{
	EventInstances::Iterator it = m_EventInstances.Find(pEffect);
	if(it != m_EventInstances.End())
		m_EventInstances.Erase(it);
}

void C_ParticleManager::AddForce(C_Force *pForce)
{
	if(pForce->m_pRenderer == NULL)
		pForce->m_pRenderer = this;
	m_Forces.Insert(pForce);
}

void C_ParticleManager::RemoveForce(C_Force *pForce)
{
	Forces::Iterator it = m_Forces.Find(pForce);
	if(it != m_Forces.End())
		m_Forces.Erase(it);
}

void C_ParticleManager::AddEffect(I_ParticleEffect* pEffect)
{
	pEffect->m_EffectPosition = m_Effects.Size();
	m_Effects.PushBack(pEffect);

	m_ActivedEffects.Resize(m_Effects.Size());

	assert(m_EffectsMap.Find(pEffect->m_iIndex) == m_EffectsMap.End());
	m_EffectsMap.Insert(EffectsMap::DataType(pEffect->m_iIndex, pEffect));
}

void C_ParticleManager::RemoveEffect(I_ParticleEffect* pEffect)
{
	S32 effectPosition = pEffect->m_EffectPosition;
	I_ParticleEffect* pEnd = m_Effects[m_Effects.Size() - 1];
	pEnd->m_EffectPosition = pEffect->m_EffectPosition;
	m_Effects[effectPosition] = pEnd;
	m_Effects.PopBack();

	if(pEffect->m_ActivePosition != -1)
		RemoveActiveEffect(pEffect);

	EffectsMap::Iterator it = m_EffectsMap.Find(pEffect->m_iIndex);
	assert(it != m_EffectsMap.End());
	if(it != m_EffectsMap.End())
		m_EffectsMap.Erase(it);
}

void C_ParticleManager::AddActiveEffect(I_ParticleEffect* pEffect)
{
	for(S32 i = 0; i < m_ActiveEffects.Size(); ++i)
		assert(m_ActiveEffects[i] != pEffect);

	pEffect->m_ActivePosition = m_ActiveEffects.Size();
	m_ActiveEffects.PushBack(pEffect);
}

void C_ParticleManager::RemoveActiveEffect(I_ParticleEffect* pEffect)
{
	S32 effectPosition = pEffect->m_ActivePosition;
	I_ParticleEffect* pEnd = m_ActiveEffects[m_ActiveEffects.Size() - 1];
	pEnd->m_ActivePosition = pEffect->m_ActivePosition;
	pEffect->m_ActivePosition = -1;
	m_ActiveEffects[effectPosition] = pEnd;
	m_ActiveEffects.PopBack();
}

void C_ParticleManager::AddSuperEffect(I_ParticleSuperEffect* pSuperEffect)
{
	pSuperEffect->m_EffectPosition = m_SuperEffects.Size();
	m_SuperEffects.PushBack(pSuperEffect);

	m_ActivedSuperEffects.Resize(m_SuperEffects.Size());

	assert(m_EffectsMap.Find(pSuperEffect->m_iIndex) == m_EffectsMap.End());
	m_EffectsMap.Insert(EffectsMap::DataType(pSuperEffect->m_iIndex, pSuperEffect));
}

void C_ParticleManager::RemoveSuperEffect(I_ParticleSuperEffect* pSuperEffect)
{
	S32 effectPosition = pSuperEffect->m_EffectPosition;
	I_ParticleSuperEffect* pEnd = m_SuperEffects[m_SuperEffects.Size() - 1];
	pEnd->m_EffectPosition = pSuperEffect->m_EffectPosition;
	m_SuperEffects[effectPosition] = pEnd;
	m_SuperEffects.PopBack();

	if(pSuperEffect->m_ActivePosition != -1)
		RemoveActiveSuperEffect(pSuperEffect);

	EffectsMap::Iterator it = m_EffectsMap.Find(pSuperEffect->m_iIndex);
	assert(it != m_EffectsMap.End());
	if(it != m_EffectsMap.End())
		m_EffectsMap.Erase(it);
}

void C_ParticleManager::AddActiveSuperEffect(I_ParticleSuperEffect* pSuperEffect)
{
	pSuperEffect->m_ActivePosition = m_ActiveSuperEffects.Size();
	m_ActiveSuperEffects.PushBack(pSuperEffect);
}

void C_ParticleManager::RemoveActiveSuperEffect(I_ParticleSuperEffect* pSuperEffect)
{
	S32 effectPosition = pSuperEffect->m_ActivePosition;
	I_ParticleSuperEffect* pEnd = m_ActiveSuperEffects[m_ActiveSuperEffects.Size() - 1];
	pEnd->m_ActivePosition = pSuperEffect->m_ActivePosition;
	pSuperEffect->m_ActivePosition = -1;
	m_ActiveSuperEffects[effectPosition] = pEnd;
	m_ActiveSuperEffects.PopBack();
}

I_ParticleEffectBase* C_ParticleManager::FindEffect(const U32 id)
{
	EffectsMap::Iterator it = m_EffectsMap.Find(id);
	if(it !=  m_EffectsMap.End())
		return it->GetValue();
	else
		return NULL;
}

F32 C_ParticleManager::GetSleepSpeedSq() const
{
	return g_debugParticleSleepSpeedSq;
}

void C_ParticleManager::BindVertexConstants(const SceneGraph::C_View& view, Display::C_Context* pContext) const
{
	pContext->SetVertexShaderConstantF(CV_RENDER_LIGHT_DIRECTION, (F32*)&m_vLightDirection, 1);
}

void C_ParticleManager::BindPixelConstants(const SceneGraph::C_View& view, Display::C_Context* pContext) const
{
	MathsStorage::Float4 depthScaleParam, decalScaleParam, projScaleParam;
	
	const Display::C_ViewSettings3D& viewSettings = view.GetViewSettings3D();
		
	depthScaleParam = m_pRenderState->GetDepthScaleShaderParameter(viewSettings.GetNearClip(), viewSettings.GetFarClip());
			
	// decal scale
	decalScaleParam.x = g_debugParticleDecalClipDiffScale1;
	decalScaleParam.y = g_debugParticleDecalClipScale1;
	decalScaleParam.z = g_debugParticleDecalClipDiffScale2;
	decalScaleParam.w = g_debugParticleDecalClipScale2;

	const Display::C_View* pView = pContext ? pContext->GetView() : view.GetDisplayView();
	projScaleParam.x = 0.5f / pView->GetViewport().GetW();
	projScaleParam.y = 0.5f / pView->GetViewport().GetH();

	MathsStorage::Float4 luminanceParam = m_pRenderState->ShaderHDR() ? m_pRenderState->HDR()->GetShaderParameters(view.GetDisplayView()->GetUserIndex()) : m_pRenderState->GetGainShaderParameters();
	
#ifdef PLATFORM_PS3
	// patch pixel shader constants
	MathsStorage::Float4 fogColour = m_pRenderState->Fog()->GetFogColour();
	MathsStorage::Float4 depthFogParam = m_pRenderState->Fog()->GetDepthFogShaderParameters();
	MathsStorage::Float4 heightFogParam = m_pRenderState->Fog()->GetHeightFogShaderParameters();
	
	// patch pixel shaders with fog and luminance parameters
	for(S32 i = 0; i < E_PixelShader_Count; ++i)
	{
		if(m_PixelShader[i].IsCreated())
		{
			Display::Device()->SetPixelShaderParameterF(m_FogColourParam[i], (F32*)&fogColour);
			Display::Device()->SetPixelShaderParameterF(m_DepthFogParam[i], (F32*)&depthFogParam);
			Display::Device()->SetPixelShaderParameterF(m_HeightFogParam[i], (F32*)&heightFogParam);
			Display::Device()->SetPixelShaderParameterF(m_DepthScaleParam[i], (F32*)&depthScaleParam);
			Display::Device()->SetPixelShaderParameterF(m_DecalScaleParam[i], (F32*)&decalScaleParam);
			Display::Device()->SetPixelShaderParameterF(m_ProjScaleParam[i], (F32*)&projScaleParam);
			Display::Device()->SetPixelShaderParameterF(m_LuminanceParam[i], (F32*)&luminanceParam);
		}
	}
#else
	assert(pContext);
	pContext->SetPixelShaderConstantF(CP_DYNAMIC_PARTICLE_DEPTHSCALE, (F32*)&depthScaleParam, 1);
	pContext->SetPixelShaderConstantF(CP_DYNAMIC_PARTICLE_DECALSCALE, (F32*)&decalScaleParam, 1);
	pContext->SetPixelShaderConstantF(CP_DYNAMIC_PARTICLE_PROJSCALE, (F32*)&projScaleParam, 1);
	pContext->SetPixelShaderConstantF(CP_RENDER_POSTPROCESS_LUMINANCE, (F32*)&luminanceParam, 1);
#endif
}


//#include "Dynamic/Particle/MathsUtil.h"

void C_ParticleManager::DisplayDebugGraphics() const
{
#ifndef CFG_GOLD
	PERF_EVENT("Particle.DebugGraphics");
		
	for(Forces::ConstIterator pIterator = m_Forces.Begin(); pIterator != m_Forces.End(); pIterator++)
	{
		C_Force* pForce = (*pIterator);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(Maths::HVec3((F32*)&pForce->m_vPosition), pForce->m_vRadius.x, Maths::RGBA(255, 0, 0, 255));
	}

	for(S32 i = 0; i < m_Emitters.m_ActiveElements; ++i)
	{
		const C_Emitter* pEmitter = m_Emitters.m_Active[i];
		pEmitter->DisplayDebug();
	}

	for(S32 i = 0; i < m_SuperEffectEmitters.m_ActiveElements; ++i)
	{
		const C_Emitter* pEmitter = m_SuperEffectEmitters.m_Active[i];
		pEmitter->DisplayDebug();
	}
#endif
}

void C_ParticleManager::Reset()
{
	U32 nLocalBlocks = 0;
	// remove any active event instances
	m_Emitters.Update();
	m_EventInstanceList.Update();
	while(m_EventInstanceList.m_ActiveElements > 0)
	{
		C_EventInstance* pEventInstance = m_EventInstanceList.m_Active[0];
		pEventInstance->FreeRemove();
	}

	// go through all active emitters and remove them - need to do this last!
	while(m_Emitters.m_ActiveElements > 0)
	{
		C_Emitter* pEmitter = m_Emitters.m_Active[0];
		pEmitter->Free();
	}

	for(S32 i = 0; i < m_SuperEffects.Size(); ++i)
		m_SuperEffects[i]->Reset();
	for(S32 i = 0; i < m_ActiveSuperEffects.Size(); ++i)
		m_ActiveSuperEffects[i]->m_ActivePosition = -1;
	m_ActiveSuperEffects.Clear();

	for(S32 i = 0; i < m_Effects.Size(); ++i)
		m_Effects[i]->Reset();
	for(S32 i = 0; i < m_ActiveEffects.Size(); ++i)
		m_ActiveEffects[i]->m_ActivePosition = -1;
	m_ActiveEffects.Clear();

	if(nLocalBlocks > 0)
		LOGINFO(g_Log, "ParticleManager: %d blocks of local space particles were not released by their game side owners\n", nLocalBlocks);

	m_EventInstanceCountMax = 0;
	m_EmitterCountMax = 0;
	m_ParticleCountMax = 0;
}

void C_ParticleManager::UpdateMemory()
{
	m_SuperEffectUpdateData.Grow();
	m_Emitters.Update();
	m_SuperEffectEmitters.Update();

	m_ParticleTrailBlockManager.Update();
	m_ParticleBlockManager16.Update();
	m_ParticleBlockManager32.Update();
	m_ParticleBlockManager48.Update();
	m_ParticleBlockManager64.Update();
	m_ParticleBlockManager80.Update();
	m_ParticleBlockManager96.Update();
	m_ParticleBlockManager112.Update();

#ifndef CFG_GOLD
	if(m_ParticleTrailBlockManager.GetAllocatedBlocks() > m_DebugMaxAllocatedBlocks[0])
	{
		m_DebugMaxAllocatedBlocks[0] = m_ParticleTrailBlockManager.GetAllocatedBlocks();
	}

	if(m_ParticleBlockManager16.GetAllocatedBlocks() > m_DebugMaxAllocatedBlocks[1])
	{
		m_DebugMaxAllocatedBlocks[1] = m_ParticleBlockManager16.GetAllocatedBlocks();
	}

	if(m_ParticleBlockManager32.GetAllocatedBlocks() > m_DebugMaxAllocatedBlocks[2])
	{
		m_DebugMaxAllocatedBlocks[2] = m_ParticleBlockManager32.GetAllocatedBlocks();
	}

	if(m_ParticleBlockManager48.GetAllocatedBlocks() > m_DebugMaxAllocatedBlocks[3])
	{
		m_DebugMaxAllocatedBlocks[3] = m_ParticleBlockManager48.GetAllocatedBlocks();
	}

	if(m_ParticleBlockManager64.GetAllocatedBlocks() > m_DebugMaxAllocatedBlocks[4])
	{
		m_DebugMaxAllocatedBlocks[4] = m_ParticleBlockManager64.GetAllocatedBlocks();
	}

	if(m_ParticleBlockManager80.GetAllocatedBlocks() > m_DebugMaxAllocatedBlocks[5])
	{
		m_DebugMaxAllocatedBlocks[5] = m_ParticleBlockManager80.GetAllocatedBlocks();
	}

	if(m_ParticleBlockManager96.GetAllocatedBlocks() > m_DebugMaxAllocatedBlocks[6])
	{
		m_DebugMaxAllocatedBlocks[6] = m_ParticleBlockManager96.GetAllocatedBlocks();
	}

	if(m_ParticleBlockManager112.GetAllocatedBlocks() > m_DebugMaxAllocatedBlocks[7])
	{
		m_DebugMaxAllocatedBlocks[7] = m_ParticleBlockManager112.GetAllocatedBlocks();
	}
#endif
}

extern S32 g_EmitterWarnings;

void C_ParticleManager::Update(const F32 fFrameTime, const F32 fFrameRate)
{
	// generally we can just update each part in sequence, but for perf reasons we might want to do game specific stuff inbetween these updates
	// this allows us to increase performance by not flushing on the main thread until work has finished
	if(m_AutoUpdate)
	{
		UpdateParticles(fFrameTime, fFrameRate);
		UpdateEmitters();
		UpdateSuperEmitters();
		UpdateSpawn();
		UpdateFinal();
	}
}

void C_ParticleManager::UpdateParticles(const F32 fFrameTime, const F32 fFrameRate)
{
	PERF_EVENT("Particle.UpdateParticles");

#ifndef CFG_GOLD
	if(g_debugParticleQualityOverride)
		SetQuality(g_debugParticleQuality);
#endif

	g_EmitterWarnings = 0;
	
	Maths::HVec3 globalForceTime = m_GlobalForce * fFrameTime;

	Maths::Store(globalForceTime, &m_EffectUpdateConstants.globalForce);
	m_EffectUpdateConstants.timeStep = fFrameTime;
	m_EffectUpdateConstants.timeStepRcp = fFrameRate;
	m_EffectUpdateConstants.sleepSpeedSq = g_debugParticleSleepSpeedSq;
	m_EffectUpdateConstants.pMaterials = m_Materials.Size() != 0 ? &m_Materials[0] : NULL;
	++m_EffectUpdateConstants.frameIndex;
	{
		m_DeletedBlocks.Clear();

		m_EffectUpdateConstants.ppDeletedBlocks = &m_DeletedBlocks[0];
		m_EffectUpdateConstants.pDeletedBlocksCount = m_DeletedBlocks.SizePtr();
		m_EffectUpdateConstants.deletedBlocksCapacity = m_DeletedBlocks.Capacity();
	}

	{
		m_DeletedSuperBlocks.Clear();

		m_EffectUpdateConstants.pDeletedSuperBlocks = &m_DeletedSuperBlocks[0];
		m_EffectUpdateConstants.pDeletedSuperBlocksCount = m_DeletedSuperBlocks.SizePtr();
		m_EffectUpdateConstants.deletedSuperBlocksCapacity = m_DeletedSuperBlocks.Capacity();
	}

	{
		m_CollideableBlocks.Clear();

		m_EffectUpdateConstants.ppCollideableBlocks = &m_CollideableBlocks[0];
		m_EffectUpdateConstants.pCollideableBlocksCount = m_CollideableBlocks.SizePtr();
		m_EffectUpdateConstants.collideableBlocksCapacity = m_CollideableBlocks.Capacity();
	}

	m_EmitterUpdateConstants.time = m_pRenderer->GetTime();
	m_EmitterUpdateConstants.timeStep = fFrameTime;
	m_EmitterUpdateConstants.timeStepRcp = fFrameRate;
	m_EmitterUpdateConstants.intensity = m_fQuality;
	{
		m_SpawningEmitters.Clear();
		m_EmitterUpdateConstants.pSpawningEmitters = &m_SpawningEmitters[0];
		m_EmitterUpdateConstants.pSpawningEmittersCount = m_SpawningEmitters.SizePtr();
		m_EmitterUpdateConstants.spawningEmittersCapacity = m_SpawningEmitters.Capacity();

		m_SpawningCustomEmitters.Clear();
		m_EmitterUpdateConstants.pSpawningCustomEmitters = &m_SpawningCustomEmitters[0];
		m_EmitterUpdateConstants.pSpawningCustomEmittersCount = m_SpawningCustomEmitters.SizePtr();
		m_EmitterUpdateConstants.spawningCustomEmittersCapacity = m_SpawningCustomEmitters.Capacity();

		

		m_ActivedEffects.Clear();
		m_EmitterUpdateConstants.ppActivedEffects = &m_ActivedEffects[0];
		m_EmitterUpdateConstants.pActivedEffectsCount = m_ActivedEffects.SizePtr();

		m_ActivedSuperEffects.Clear();
		m_EmitterUpdateConstants.ppActivedSuperEffects = &m_ActivedSuperEffects[0];
		m_EmitterUpdateConstants.pActivedSuperEffectsCount = m_ActivedSuperEffects.SizePtr();
	}

	for(S32 i = 0; i < m_ActiveSuperEffects.Size(); ++i)
		UpdateSuperEffect(m_ActiveSuperEffects[i]);
	for(S32 i = 0; i < m_ActiveEffects.Size(); ++i)
		UpdateEffect(m_ActiveEffects[i]);
}

void C_ParticleManager::UpdateEmitters(C_EmitterList& emitterList, Scheduler::GroupHandle group)
{
	if(g_MultiThreadedEmitterUpdate)
	{
		// get blocks divided by 6 and rounded down to nearest 4
		S32 emittersDiv6 = (emitterList.m_ActiveElements / 6) & ~3;
		// if this is greater than 20 (default) use this instead
		S32 emittersPerTask = emittersDiv6 > 64 ? emittersDiv6 : 64;

		S32 numEmitters = 0;
		while(numEmitters < emitterList.m_ActiveElements)
		{
			S32 emittersRemaining = emitterList.m_ActiveElements - numEmitters;
				
			EmitterUpdateData data;
			data.pUpdateConstants = &m_EmitterUpdateConstants;
			data.ppEmitters = &emitterList.m_Active[numEmitters];
			data.numEmitters = emittersRemaining > emittersPerTask ? emittersPerTask : emittersRemaining;

			Scheduler::AddTask(ParticleEmitterUpdateModule, NULL, NULL, group, m_pRenderer->GetJobQueuePolicyHandle(), &data, sizeof(EmitterUpdateData));
			numEmitters += emittersPerTask;
		}
	}
	else
	{
		for(S32 i = 0; i < emitterList.m_ActiveElements; ++i)
		{
			C_Emitter* pEmitter = emitterList.m_Active[i];
			pEmitter->Update(m_Random[0], &m_EmitterUpdateConstants, pEmitter);
		}
	}
}

// set off emitters for update
void C_ParticleManager::UpdateEmitters()
{
	PERF_EVENT("Particle.UpdateEmitters");
	{
		m_bUpdatingEmitters = true;
		{
			static Util::C_String128 eventInstances;
			PERF_EVENT(eventInstances.Printf("Particle.EventInstances (%d)", m_EventInstanceList.m_ActiveElements));
			m_Emitters.Update();
			m_EventInstanceList.Update();
			// update all active event instances
			for(S32 i = 0; i < m_EventInstanceList.m_ActiveElements; )
			{
				C_EventInstance* pEventInstance = m_EventInstanceList.m_Active[i];
				i += pEventInstance->Update(m_EffectUpdateConstants.timeStep, m_Random[0]);
			}
		}
		// schedule all normal emitters first
		UpdateEmitters(m_Emitters, m_EmittersGroup);
	}
}

// set off super emitters for update
void C_ParticleManager::UpdateSuperEmitters()
{
	{
		PERF_EVENT("Particle.UpdateSuperEmittters");
		Scheduler::Flush(m_SuperEffectsGroup);
		m_SuperEffectEmitters.Update();
		UpdateEmitters(m_SuperEffectEmitters, m_SuperEmittersGroup);
	
		// update deleted blocks
		m_DeletedSuperBlocks.Update();
		for(S32 i = 0; i < m_DeletedSuperBlocks.Size(); ++i)
		{
			DeletedSuperBlock& deletedSuperBlock = m_DeletedSuperBlocks[i];

			I_ParticleSuperEffect* pEffect = (I_ParticleSuperEffect*)deletedSuperBlock.pBlock->m_pEffect;
			pEffect->DeleteBlock(deletedSuperBlock.pBlock);

			for(S32 j = 0; j < deletedSuperBlock.trailBlocks.Size(); ++j)
			{
				deletedSuperBlock.trailBlocks[j]->m_pEffect->DeleteTrailBlock(deletedSuperBlock.trailBlocks[j]);
			}
		}
	}

	{
		PERF_EVENT("Particle.UpdateDeleteBlocks");
		// flush normal effects
		Scheduler::Flush(m_EffectsGroup);

		m_CollideableBlocks.Update();

		m_DeletedBlocks.Update();
		for(S32 i = 0; i < m_DeletedBlocks.Size(); ++i)
		{
			I_ParticleEffect* pEffect = (I_ParticleEffect*)m_DeletedBlocks[i]->m_pEffect;
			assert(pEffect);
			pEffect->DeleteBlock(m_DeletedBlocks[i]);
		}
	}
}

// set off particle spawning
void C_ParticleManager::UpdateSpawn()
{
	PERF_EVENT("Particle.Spawn");

	// flush emitters and super emitters and do spawning
	Scheduler::Flush(m_EmittersGroup);
	m_Emitters.Update();

	Scheduler::Flush(m_SuperEmittersGroup);
	m_SuperEffectEmitters.Update();

	m_SpawningEmitters.Update();
	m_SpawningCustomEmitters.Update();


	if(g_SortSpawningEmitters && m_SpawningEmitters.Size() > 1)
	{
		// TODO: optimise this
		PERF_EVENT("Particle.SortSpawningEmitters");
		Btl::RadixSorter<Btl::Vector<SpawningEmitter> > spawningEmitterSorter;
		
		S32 originalCapacity = m_SpawningEmitters.Capacity();
		if(m_SpawningEmitters.Size() < originalCapacity)
			m_SpawningEmitters.Base::Resize(m_SpawningEmitters.Size());

		spawningEmitterSorter.Sort(m_SpawningEmitters);

		// set size of base vector back to original capacity (it might have changed during sorting)
		m_SpawningEmitters.Base::Resize(originalCapacity);
		// data has potentially moved so update the element pointer
		m_SpawningEmitters.UpdateElementPtr();
	}

	{
		static Util::C_String128 spawn;
		PERF_EVENT(spawn.Printf("Particle.Spawn (%d)", m_SpawningEmitters.Size()));
		for(S32 i = 0; i < m_SpawningEmitters.Size(); ++i)
		{
			S32 startIndex = i;
			S32 count = 1;
			I_ParticleEffectBase* pEffect = m_SpawningEmitters[i].pEffect;
			while(i + 1 < m_SpawningEmitters.Size() &&
					m_SpawningEmitters[i+1].pEffect == pEffect)
			{
				++count;
				++i;
			}
			
			// spawn many emitters at the same time
			SpawnEffect(m_SpawningEmitters[i].pEffect, &m_SpawningEmitters[startIndex], count);
		}
	}

	{
		static Util::C_String128 spawnCustom;
		PERF_EVENT(spawnCustom.Printf("Particle.SpawnCustom (%d)", m_SpawningCustomEmitters.Size()));
		for(S32 i = 0; i < m_SpawningCustomEmitters.Size(); ++i)
		{
			SpawningEmitter& spawningEmitter = m_SpawningCustomEmitters[i];
			SpawnCustom(spawningEmitter.pEmitter, &spawningEmitter.pEffect->m_SpawnSettings, m_Random[0], &m_EmitterUpdateConstants);
		}
	}
}

void C_ParticleManager::UpdateFinal()
{
	PERF_EVENT("Particle.Final");

	// flush spawning then perform necessary final maintenance before rendering
	Scheduler::Flush(m_JobQueueGroup);
	m_bUpdatingEmitters = false;

	// push actived effects onto active effects list
	{
		for(S32 i = 0; i < m_ActivedEffects.Size(); ++i)
			AddActiveEffect(m_ActivedEffects[i]);

		// push actived effects onto active effects list
		for(S32 i = 0; i < m_ActivedSuperEffects.Size(); ++i)
			AddActiveSuperEffect(m_ActivedSuperEffects[i]);
	}

	// update memory for system and all active effects before we render
	UpdateMemory();
	{
		for(S32 i = 0; i < m_ActiveSuperEffects.Size(); )
		{
			I_ParticleSuperEffect* pEffect = m_ActiveSuperEffects[i];
			pEffect->UpdateMemory();

			if(pEffect->m_nBlocks != 0)
				 ++i;
		}

		for(S32 i = 0; i < m_ActiveEffects.Size(); )
		{
			I_ParticleEffect* pEffect = m_ActiveEffects[i];
			pEffect->UpdateMemory();

			if(pEffect->m_nBlocks != 0)
				++i;
		}
	}
}

void C_ParticleManager::UpdateEffect(I_ParticleEffect* pEffect)
{
	assert(pEffect->m_nBlocks == pEffect->m_SpawnSettings.blocks.Size());

	if(g_MultiThreadedEffectUpdate)
	{
		// get blocks divided by 6 and rounded down to nearest 4
		S32 blocksDiv6 = (pEffect->m_SpawnSettings.blocks.Size() / 6) & ~3;
		// if this is greater than 20 (default) use this instead
		S32 blocksPerTask = blocksDiv6 > 20 ? blocksDiv6 : 20;

		S32 numBlocks = 0;
		while(numBlocks < pEffect->m_SpawnSettings.blocks.Size())
		{
			S32 blocksRemaining = pEffect->m_SpawnSettings.blocks.Size() - numBlocks;
				
			EffectUpdateData data;
			data.pUpdateConstants = &m_EffectUpdateConstants;
			data.pPhysicsSettings = &pEffect->GetPhysicsSettings();
			data.numBlocks = blocksRemaining > blocksPerTask ? blocksPerTask : blocksRemaining;

			Scheduler::AddTask(*pEffect->GetUpdateModule(), &pEffect->m_SpawnSettings.blocks[numBlocks], NULL, m_EffectsGroup, m_pRenderer->GetJobQueuePolicyHandle(), &data, sizeof(EffectUpdateData));
			numBlocks += blocksPerTask;
		}
	}
	else
	{
		pEffect->GetUpdateFunctor()(&pEffect->m_SpawnSettings.blocks[0], &pEffect->GetPhysicsSettings(), &m_EffectUpdateConstants, pEffect->m_SpawnSettings.blocks.Size());
	}
}

void C_ParticleManager::UpdateSuperEffect(I_ParticleSuperEffect* pSuperEffect)
{
	assert(pSuperEffect->m_nBlocks == pSuperEffect->m_SpawnSettings.blocks.Size());

	if(g_MultiThreadedEffectUpdate)
	{
		// get blocks divided by 6 and rounded down to nearest 4
		S32 blocksDiv6 = (pSuperEffect->m_SpawnSettings.blocks.Size() / 6) & ~3;
		// if this is greater than 8 (default) use this instead
		S32 blocksPerTask = blocksDiv6 > 8 ? blocksDiv6 : 8;

		S32 numBlocks = 0;
		while(numBlocks < pSuperEffect->m_SpawnSettings.blocks.Size())
		{
			S32 blocksRemaining = pSuperEffect->m_SpawnSettings.blocks.Size() - numBlocks;

			if(SuperEffectUpdateData* pUpdateData = m_SuperEffectUpdateData.Allocate())
			{
				pUpdateData->ppBlockEA = &pSuperEffect->m_SpawnSettings.blocks[numBlocks];
				pUpdateData->trailBlocksEA.Resize(pSuperEffect->m_SuperEffects.trailEffects.Size());
				for(S32 i = 0; i < pSuperEffect->m_SuperEffects.trailEffects.Size(); ++i)
				{
					pUpdateData->trailBlocksEA[i] = &pSuperEffect->m_SuperEffects.trailEffects[i].blocks[numBlocks];
				}
				pUpdateData->pSuperEffects = &pSuperEffect->m_SuperEffects;
				pUpdateData->pPhysicsSettings = &pSuperEffect->m_PhysicsSettings;
				pUpdateData->pUpdateConstants = &m_EffectUpdateConstants;
				pUpdateData->numBlocks = blocksRemaining > blocksPerTask ? blocksPerTask : blocksRemaining;
				
				Scheduler::AddTask(*pSuperEffect->GetUpdateModule(), pUpdateData, NULL, m_SuperEffectsGroup, m_pRenderer->GetJobQueuePolicyHandle());
			}
			numBlocks += blocksPerTask;
		}
	}
	else
	{
		TrailPtrPtrs trailBlocksEA;
		trailBlocksEA.Resize(pSuperEffect->m_SuperEffects.trailEffects.Size());
		for(S32 i = 0; i < pSuperEffect->m_SuperEffects.trailEffects.Size(); ++i)
		{
			trailBlocksEA[i] = &pSuperEffect->m_SuperEffects.trailEffects[i].blocks[0];
		}
		pSuperEffect->GetUpdateFunctor()(&pSuperEffect->m_SpawnSettings.blocks[0], &trailBlocksEA, &pSuperEffect->m_SuperEffects, &pSuperEffect->m_PhysicsSettings, &m_EffectUpdateConstants, pSuperEffect->m_SpawnSettings.blocks.Size());
	}
}

void C_ParticleManager::SpawnEffect( I_ParticleEffectBase* pEffect, SpawningEmitter* pSpawningEmitter, S32 count)
{
	for(S32 i = 0; i < count; ++i)
	{
		assert(pSpawningEmitter[i].pEmitter != NULL);
	}

	if(g_MultiThreadedSpawn)
	{
		// get blocks divided by 6 and rounded down to nearest 4
		S32 emittersDiv6 = (count / 6) & ~3;
		// if this is greater than 8 (default) use this instead
		S32 emittersPerTask = emittersDiv6 > 4 ? emittersDiv6 : 4;

		S32 numEmitters = 0;
		while(numEmitters < count)
		{
			S32 emittersRemaining = count - numEmitters;

			EffectSpawnData spawnData;
			spawnData.pSpawnSettings = &pEffect->m_SpawnSettings;
			spawnData.pSpawningEmitter = &pSpawningEmitter[numEmitters];
			spawnData.emitterCount = emittersRemaining > emittersPerTask ? emittersPerTask : emittersRemaining;
			Scheduler::AddTask(*pEffect->GetSpawnModule(), (void*)&m_EmitterUpdateConstants, NULL, GetJobQueueGroup(), m_pRenderer->GetJobQueuePolicyHandle(), &spawnData, sizeof(EffectSpawnData));
			numEmitters += emittersPerTask;
		}
	}
	else
	{
		pEffect->GetSpawnFunctor()(pSpawningEmitter, &pEffect->m_SpawnSettings, count, m_Random[0], &m_EmitterUpdateConstants);
	}

}


bool C_ParticleManager::UseDiffuseMap() const 
{
#ifndef CFG_SPEED
	return g_debugParticleUseDiffuseMap;
#else
	return true;
#endif
}

bool C_ParticleManager::UseNormalMap() const 
{
#ifndef CFG_SPEED
	return g_debugParticleUseNormalMap;
#else
	return true;
#endif
}

bool C_ParticleManager::UseSoft() const 
{
#ifdef PLATFORM_PC
	return false;
#endif

#ifndef CFG_SPEED
	return g_debugParticleUseSoft;
#else
	return true;
#endif
}

bool C_ParticleManager::UseDecal() const 
{
#ifndef CFG_SPEED
	return g_debugParticleUseDecal;
#else
	return true;
#endif
}

C_EventInstance* C_ParticleManager::NewEventInstance()
{
	C_EventInstance* pEventInstance = NULL;

	if((pEventInstance = m_EventInstanceList.Allocate()))
		assert(pEventInstance->m_Emitters.Size() == 0);
	else
		LOGWARN(g_Log, "ParticleManager: EventInstanceCache empty!\n");

	return pEventInstance;
}

C_EventInstance* C_ParticleManager::AtomicNewEventInstance()
{
	C_EventInstance* pEventInstance = NULL;

	if((pEventInstance = m_EventInstanceList.AtomicAllocate()))
		assert(pEventInstance->m_Emitters.Size() == 0);
	else
		LOGWARN(g_Log, "ParticleManager: EventInstanceCache empty!\n");

	return pEventInstance;
}

void C_ParticleManager::FreeEventInstance(C_EventInstance* pEventInstance)
{
	pEventInstance->m_Emitters.Clear();
	pEventInstance->m_iSalt += 2;
	m_EventInstanceList.Free(pEventInstance);
}

void C_ParticleManager::AtomicFreeEventInstance(C_EventInstance* pEventInstance)
{
	pEventInstance->m_Emitters.Clear();
	pEventInstance->m_iSalt += 2;
	m_EventInstanceList.AtomicFree(pEventInstance);
}


void C_ParticleManager::SetMaterial(Display::C_Context* pDisplay, const RenderSettings& renderSettings) const
{
	m_pRenderer->SetBlendMode(pDisplay, renderSettings.blendMode);
	
	// if no texture 
	if(renderSettings.pDiffuse && renderSettings.pDiffuse->IsCreated() && UseDiffuseMap())
		pDisplay->SetTexture(0, renderSettings.pDiffuse->GetGPUTexture());
	else
		pDisplay->SetTexture(0, m_pRenderer->GetGreyTexture().GetGPUTexture());

	if(renderSettings.pNormalMap && UseNormalMap())
		pDisplay->SetTexture(1, renderSettings.pNormalMap->GetGPUTexture());

#ifdef CFG_SPEED
	SetPixelShader(pDisplay, renderSettings.pixelShader);
#else
	SetPixelShader(pDisplay, GetPixelShaderFromRenderSettings(renderSettings));
#endif
}	

E_PixelShader C_ParticleManager::GetPixelShaderFromRenderSettings(const RenderSettings& renderSettings) const
{
	if(renderSettings.pNormalMap && UseNormalMap())
	{
		return E_PixelShader_NormalMap;
	}
	else
	{
		switch(renderSettings.blendMode)
		{
			case E_BlendMode_AlphaA8:
			case E_BlendMode_AdditiveA8:
			case E_BlendMode_SubtractiveA8:
				if(renderSettings.soft && UseSoft())
					return E_PixelShader_SoftA8;
				else
					return E_PixelShader_A8;
				break;
			default:
				if(renderSettings.vertexBuildSettings.eParticleShape == E_ParticleShape_Decal && UseDecal())
					return E_PixelShader_Decal;
				else if(renderSettings.soft && UseSoft())
					return E_PixelShader_Soft;
				else
					return E_PixelShader_Default;
				break;
		}
	}

}

}
}










		


	


















