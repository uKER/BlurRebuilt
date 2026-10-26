#ifndef RENDER_DYNAMICEFFECT_PARTICLE_MANAGER_H
#define RENDER_DYNAMICEFFECT_PARTICLE_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

// core containers
#include <core\containers\radix.h>
#include <core\containers\bitvector.h>
#include <system/scheduler/jobqueue.h>

#include "Display/Device/Device.h"
#include "Display/Context/Context.h"
#include "Render/Debug/DebugRender.h"

// dynamic effect includes
#include "Dynamic/ManagerInterface.h"
#include "Dynamic/Defines.h"
#include "Dynamic/Light.h"
#include "Dynamic/DebugInfo.h"
#include "Dynamic/Atomic.h"


// particle specific includes
#include "Dynamic/Particle/Particle.h"
#include "Dynamic/Particle/Enums.h"
#include "Dynamic/Particle/Event.h"
#include "Dynamic/Particle/EventInstance.h"
#include "Dynamic/Particle/EmitterList.h"

#include "Dynamic/BlockManager.h"
#include <misc_shared/Types/FP32.h>

#include "Dynamic/Particle/UpdateData.h"
#include "SceneGraph/Util/RingBuffer.h"

namespace Render
{
	class C_RenderState;
}

namespace DynamicEffect
{
struct RenderStates;
class I_DynamicEffect;

namespace Particle
{

struct RenderSettings;

class C_Force;
class C_Event;
class C_EventInstance;
class C_Emitter;
class I_ParticleEffect;

// use sets and maps to track objects
typedef Ut::UtSet<C_Emitter*>				Emitters;
typedef Ut::UtSet<C_Force*>					Forces;
typedef Ut::UtSet<C_Event*>					Events;
typedef Ut::UtSet<C_EventInstance*>			EventInstances;
typedef Ut::UtMap<U32, I_ParticleEffectBase*>	EffectsMap;

typedef Ut::Vector<ParticleBlock*>			ParticleBlockPointers;
typedef Ut::Vector<ParticleTrailBlock*>		ParticleTrailBlockPointers;
typedef Ut::Vector<DeletedSuperBlock>		DeletedSuperBlocks;

//typedef Ut::Vector<ParticleCollision3D>		Collisions;



// particle manager class manages shaders and particle storage for all particle effects
class C_ParticleManager : public I_DynamicEffectManager
{
public:
	C_ParticleManager(C_Renderer* pRenderer);
	~C_ParticleManager();

	// typedefs
	typedef Ut::UtVector<Ut::ArrayVector<MathsStorage::Float4A16, 64> >		Materials;

	//
	void							ReserveEmitters(const U32 nEmitters);
	void							ReserveSuperEmitters(const U32 nEmitters);
	void							ReserveEventInstances(const U32 nEventInstances);

	bool							Create();
	bool							CreateVolatile();
	bool							Initialise();
	void							Destroy();
	void							DestroyVolatile();
	void							Shutdown();
	void							SetAutoUpdate(bool autoUpdate) { m_AutoUpdate = autoUpdate; }

	void							RecreateShaders();

	//
	C_EventInstanceHandle			Test();

	// update
	
	virtual void					Update(const F32 fFrameTime, const F32 fFrameRate);

	void							UpdateParticles(const F32 fFrameTime, const F32 fFrameRate);
	void							UpdateEmitters();
	void							UpdateSuperEmitters();
	void							UpdateSpawn();
	void							UpdateFinal();
	void							UpdateMemory();

	void							UpdateEffect(I_ParticleEffect* pEffect);
	void							UpdateSuperEffect(I_ParticleSuperEffect* pEffect);

	void							SpawnEffect( I_ParticleEffectBase* pEffect, SpawningEmitter* pSpawningEmitter, S32 count);
	void							UpdateEmitters( C_EmitterList& emitterList, Scheduler::GroupHandle group);

	// reset all active emitters/particles
	void							Reset();
	void							BeginFrame() const;

	void							BindVertexConstants(const SceneGraph::C_View& view, Display::C_Context* pContext) const;
	void							BindPixelConstants(const SceneGraph::C_View& view, Display::C_Context* pContext) const;
	void							DisplayDebugGraphics() const;
	MathsStorage::Int2				DisplayDebugInfo(MathsStorage::Int2& position) const;
	MathsStorage::Int2				DisplayDebugMemoryInfo(MathsStorage::Int2& position) const;
	

	// parameters
	void							SetQuality(F32 fQuality);
	void							SetParticleColour(const Maths::C_RGBA_F32& vParticleColour) { Maths::Store(vParticleColour, &m_vParticleColour); }
	void							SetParticleColour(const MathsStorage::Float4A16& vParticleColour) { m_vParticleColour = vParticleColour; }
	void							SetEmissiveColour(const Maths::C_RGBA_F32& vEmissiveColour) { Maths::Store(vEmissiveColour, &m_vEmissiveColour); m_vEmissiveColour.w = 1.0f; }
	void							SetEmissiveColour(const MathsStorage::Float4A16& vEmissiveColour) { m_vEmissiveColour = vEmissiveColour; m_vEmissiveColour.w = 1.0f; }
	void							SetExpiryTime(F32 fExpiryTime) { m_fExpiryTime = fExpiryTime; }
	void							SetMaterial(F32 fRestitution, F32 fFriction, U32 iIndex) { m_Materials[iIndex] = MathsStorage::Float4A16(1.0f + fRestitution, fFriction, 0.0f, 0.0f); }
	void							SetLightDirection(const MathsStorage::Float3& vLightDirection) { m_vLightDirection = vLightDirection; }
	void							SetCullDistance(const F32 fCullDistance) { m_fCullDistance = fCullDistance; }
	void							SetFreezeAmountScale(const F32 fFreezeAmountScale);
	void							SetGlobalForce(const Maths::HVec3& globalForce) { m_GlobalForce = globalForce; }

	void							SetDefaultSpawnDirection(const Maths::HVec3& vDirection);
	void							SetDefaultSpawnDirection(const MathsStorage::Float4A16& vDirection);

	F32								GetQuality() const { return m_fQuality; }
	F32								GetQualityRcp() const { return m_fQualityRcp; }
	const MathsStorage::Float4A16&	GetParticleColour() const { return m_vParticleColour; }
	const MathsStorage::Float4A16&	GetEmissiveColour() const { return m_vEmissiveColour; }
	F32								GetExpiryTime() const { return m_fExpiryTime; }
	F32								GetFreezeAmountScale() const { return m_fFreezeAmountScale; }
	const MathsStorage::Float4A16&	GetDefaultSpawnDirection() const { return m_vDefaultSpawnDirection; }

	void							SetVertexShader(Display::C_Context* pContext, E_VertexShader eVertexShader, E_VertexShaderVariation eVertexShaderVariation) const;
	void							SetPixelShader(Display::C_Context* pContext, E_PixelShader ePixelShader) const;
	void							SetMaterial(Display::C_Context* pContext, const RenderSettings& renderSettings) const;
	E_PixelShader					GetPixelShaderFromRenderSettings(const RenderSettings& renderSettings) const;
	
	bool							UseDiffuseMap() const;
	bool							UseNormalMap() const;
	bool							UseSoft() const;
	bool							UseDecal() const;
	bool							UseVolumetric() const;

	const Forces&					GetForces() const { return m_Forces; }

	const MathsStorage::Float4A16&	GetMaterial(U32 i) const { return m_Materials[i]; }
	const Materials&				GetMaterials() const { return m_Materials; }

	F32								GetCullDistance() const { return m_fCullDistance; }

	F32								GetSleepSpeedSq() const;
	const EffectUpdateConstants&	GetEffectUpdateConstants() const { return m_EffectUpdateConstants; }
	const EmitterUpdateConstants&	GetEmitterUpdateConstants() const { return m_EmitterUpdateConstants; }

   	AtomicVector<ParticleBlock*>&	GetCollideableBlocks() { return m_CollideableBlocks; }
	
	Scheduler::GroupHandle			GetJobQueueGroup() const { return m_JobQueueGroup; }

	VectorBlockManager<ParticleBlock>*		GetParticleBlockManager(const U32 iElementSize);
	VectorBlockManager<ParticleTrailBlock>*	GetParticleTrailBlockManager() { return &m_ParticleTrailBlockManager; }
	
	friend class I_DynamicEffect;
	friend class C_Emitter;
	friend class C_Force;
	friend class C_Event;
	friend class C_EventInstance;
	friend class C_SceneNode;
	friend class I_ParticleEffectBase;
	friend class I_ParticleEffect;
	friend class I_ParticleSuperEffect;

	void							Accept(SceneGraph::I_Visitor& visitor) const;
	void							OnGather(const SceneGraph::C_View& view) const;

	// virtuals
	void							CleanEmitterDefinition(const EmitterDefinition* pDefinition);
	void							CleanNode(const C_Node* pNode);
	void							CleanEffect(const I_DynamicEffect* pEffect);

	const Events&								GetEvents() { return m_Events; }
	const AtomicActiveCache<C_EventInstance>&	GetEventInstanceList() { return m_EventInstanceList; }
	const Btl::Vector<I_ParticleEffect*>&		GetActiveEffects() { return m_ActiveEffects; }
	
	C_EmitterList*					GetEmitterList() { return &m_Emitters; }
	C_EmitterList*					GetSuperEffectEmitterList() { return &m_SuperEffectEmitters; }
	C_Random&						GetRandom(S32 index) { return m_Random[index]; }

	C_EventInstance*				NewEventInstance();
	C_EventInstance*				AtomicNewEventInstance();
	void							FreeEventInstance(C_EventInstance* pEventInstance);
	void							AtomicFreeEventInstance(C_EventInstance* pEventInstance);

	I_ParticleEffectBase*			FindEffect(const U32 id);
	C_Event*						FindEvent(const Util::C_String& name);
	void							CleanEvent(const C_Event* pEvent);

	bool							m_bUpdatingEmitters;

protected:
	// manage all objects in the particle system
	void							AddEmitter(C_Emitter* pEmitter);
	void							RemoveEmitter(C_Emitter* pEmitter);
	void							AddEvent(C_Event* pEvent);
	void							RemoveEvent(C_Event* pEvent);
	void							AddEventInstance(C_EventInstance* pEventInstance);
	void							RemoveEventInstance(C_EventInstance* pEventInstance);
	void							AddForce(C_Force* pForce);
	void							RemoveForce(C_Force* pForce);

	void							AddSuperEffect(I_ParticleSuperEffect* pSuperEffect);
	void							RemoveSuperEffect(I_ParticleSuperEffect* pSuperEffect);
	void							AddActiveSuperEffect(I_ParticleSuperEffect* pSuperEffect);
	void							RemoveActiveSuperEffect(I_ParticleSuperEffect* pSuperEffect);
	Btl::Vector<I_ParticleSuperEffect*>	m_SuperEffects;
	Btl::Vector<I_ParticleSuperEffect*>	m_ActiveSuperEffects;

	void							AddEffect(I_ParticleEffect* pEffect);
	void							RemoveEffect(I_ParticleEffect* pEffect);
	void							AddActiveEffect(I_ParticleEffect* pEffect);
	void							RemoveActiveEffect(I_ParticleEffect* pEffect);
	Btl::Vector<I_ParticleEffect*>	m_Effects;
	Btl::Vector<I_ParticleEffect*>	m_ActiveEffects;

	
	EffectsMap						m_EffectsMap;

	MathsStorage::Int2				DisplayBlockManagerDebugInfo(MathsStorage::Int2 position, const BlockManagerBase* pBlockManager) const;

	// called externally from game physics
	//void							AddCollision(const ParticleCollision3D& particle) { m_Collisions.PushBack(particle); }

	bool							CreateShaders();
	void							DestroyShaders();

	// debug stuff
	U32								m_nVisibleEmitters;
	// 

	E_PixelShaderSet				m_ePixelShaderSet;
	Display::C_PixelShader			m_PixelShader[E_PixelShader_Count];
	Display::C_VertexShader			m_VertexShader[E_VertexShader_Count][E_VertexShaderVariation_Count];

	Display::C_PixelShaderParameter	m_LuminanceParam[E_PixelShader_Count];

#ifdef PLATFORM_PS3
	Display::C_PixelShaderParameter	m_FogColourParam[E_PixelShader_Count];
	Display::C_PixelShaderParameter	m_DepthFogParam[E_PixelShader_Count];
	Display::C_PixelShaderParameter	m_HeightFogParam[E_PixelShader_Count];
	Display::C_PixelShaderParameter	m_DepthScaleParam[E_PixelShader_Count];
	Display::C_PixelShaderParameter	m_DecalScaleParam[E_PixelShader_Count];
	Display::C_PixelShaderParameter	m_ProjScaleParam[E_PixelShader_Count];
#endif
	
	Emitters						m_EmitterSet;
	Forces							m_Forces;
	Events							m_Events;
	EventInstances					m_EventInstances;

	SceneGraph::DynamicRingBuffer<SuperEffectUpdateData>	m_SuperEffectUpdateData;

	AtomicVector<ParticleBlock*>			m_DeletedBlocks;
	AtomicVector<DeletedSuperBlock>			m_DeletedSuperBlocks;
	AtomicVector<ParticleBlock*>			m_CollideableBlocks;
	AtomicVector<SpawningEmitter>			m_SpawningEmitters;
	AtomicVector<SpawningEmitter>			m_SpawningCustomEmitters;
	AtomicVector<I_ParticleEffect*>			m_ActivedEffects;
	AtomicVector<I_ParticleSuperEffect*>	m_ActivedSuperEffects;
	
	C_EmitterList							m_Emitters;
	C_EmitterList							m_SuperEffectEmitters;
	AtomicActiveCache<C_EventInstance>		m_EventInstanceList;
	C_Random								m_Random[7];

	VectorBlockManager<ParticleTrailBlock>	m_ParticleTrailBlockManager;
	VectorBlockManager<ParticleBlock>		m_ParticleBlockManager16;
	VectorBlockManager<ParticleBlock>		m_ParticleBlockManager32;
	VectorBlockManager<ParticleBlock>		m_ParticleBlockManager48;
	VectorBlockManager<ParticleBlock>		m_ParticleBlockManager64;
	VectorBlockManager<ParticleBlock>		m_ParticleBlockManager80;
	VectorBlockManager<ParticleBlock>		m_ParticleBlockManager96;
	VectorBlockManager<ParticleBlock>		m_ParticleBlockManager112;



	Scheduler::GroupHandle					m_JobQueueGroup;
	
	Scheduler::GroupHandle					m_EffectsGroup;
	Scheduler::GroupHandle					m_SuperEffectsGroup;
	Scheduler::GroupHandle					m_EmittersGroup;
	Scheduler::GroupHandle					m_SuperEmittersGroup;
	
	EffectUpdateConstants			m_EffectUpdateConstants;
	EmitterUpdateConstants			m_EmitterUpdateConstants;

	Materials						m_Materials;	
	
	U32								m_nEmitters;
	U32								m_nSuperEmitters;
	U32								m_nEventInstances;
	
	FP32							m_iNoiseX;
	FP32							m_iNoiseY;
	FP32							m_iNoiseZ;

	Maths::HVec3					m_GlobalForce;

	MathsStorage::Float4A16			m_vDefaultSpawnDirection;
	MathsStorage::Float4A16			m_vParticleColour;
	MathsStorage::Float4A16			m_vEmissiveColour;
	MathsStorage::Float3			m_vLightDirection;
	F32								m_fExpiryTime;
	F32								m_fQuality;
	F32								m_fQualityRcp;
	F32								m_fCycle;
	
	mutable S32						m_EmitterCountMax;
	mutable S32						m_EventInstanceCountMax;
	mutable S32						m_ParticleCountMax;
	
	F32								m_fCullDistance;
	F32								m_fFreezeAmountScale;
	bool							m_AutoUpdate;

	const Render::C_RenderState*	m_pRenderState;

#ifndef CFG_GOLD
	Btl::Vector< U32, Btl::VectorParameters_Array< 8 > >		m_DebugMaxAllocatedBlocks;
#endif
};

INLINE void C_ParticleManager::SetPixelShader(Display::C_Context* pDisplay, E_PixelShader ePixelShader) const
{
	pDisplay->SetPixelShader(&m_PixelShader[ePixelShader]);
}

INLINE void C_ParticleManager::SetVertexShader(Display::C_Context* pDisplay, E_VertexShader eVertexShader, E_VertexShaderVariation eVertexShaderVariation) const
{
	pDisplay->SetVertexShader(&m_VertexShader[eVertexShader][eVertexShaderVariation]);
}

/*INLINE VertexBuildData* C_ParticleManager::AddVertexBuildData() const
{
	if(m_VertexBuildData.Size() + 1 == m_VertexBuildData.Capacity())
	{
		m_VertexBuildDataGrow = true;
		return NULL;
	}
	else
	{
		m_VertexBuildData.PushBack();
		return &m_VertexBuildData.Back();
	}
}*/

} // Particle
} // DynamicEffect

#endif // RENDER_DYNAMICEFFECT_PARTICLE_RENDERER_H
