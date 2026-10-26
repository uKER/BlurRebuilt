#include "stdafx.h"

// header
#include "Dynamic/Particle/Emitter.h"

// particle system dependant
#ifndef PLATFORM_SPU
#include "Dynamic/Particle/Manager.h"
#include "Dynamic/Particle/Event.h"
#include "Dynamic/Particle/EffectInterface.h"
#endif

#include "Dynamic/Particle/EmitterList.h"
#include "Dynamic/Particle/EventInstance.h"
#include "Dynamic/Particle/SpawnerInterface.h"
#include "Dynamic/Particle/SpawnSettings.h"
#include "Dynamic/Particle/UpdateData.h"

#ifdef PLATFORM_SPU
namespace Maths
{ 
	const vector unsigned int Const< vector unsigned int >::m_SignMask = (vector unsigned int)( 0x80000000, 0x80000000, 0x80000000, 0x80000000 );
	const vector unsigned int Const< vector unsigned int >::m_AbsMask  = (vector unsigned int)( 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF );
}
#endif

namespace DynamicEffect
{

extern Debug::Logger& g_Log;

namespace Particle
{

#ifndef PLATFORM_SPU

S32	g_EmitterWarnings = 0;

Debug::Bool g_debugParticleEmitterCull("Graphics.Dynamic.Particle.Emitters.Cull", true);
Debug::Bool g_debugParticleEmitterSpawn("Graphics.Dynamic.Particle.Emitters.Spawn", true);
Debug::Bool g_debugParticleEmitterDeferredSpawn("Graphics.Dynamic.Particle.Emitters.DeferredSpawn", true);

C_Emitter::C_Emitter(const EmitterDefinition& settings, const U32 iIndex) : m_iIndex(iIndex), m_VisibilityFlags(0xFFFFFFFF)
{
	SetDefaults();
	assert(settings.pEffect);
	SetDefinition(new EmitterDefinition(settings));

	m_pManager = settings.pEffect->m_pManager;
	m_pManager->AddEmitter(this);
}

C_Emitter::C_Emitter(C_ParticleManager* pRenderer, C_EmitterList* pEmitterList, const U32 iIndex) : m_iIndex(iIndex), m_pEmitterList(pEmitterList), m_VisibilityFlags(0xFFFFFFFF)
{
	SetDefaults();

	m_pManager = pRenderer;
	m_pManager->AddEmitter(this);
}


C_Emitter::~C_Emitter()
{
	assert(m_pManager);
	m_pManager->RemoveEmitter(this);
}

void C_Emitter::SetDefaults()
{
	m_bVisible = true;
	m_bCalculatePositions = true;
	m_bCalculateVelocity = true;
	m_bCachedAdditive = false;
	m_bDelete = false;
	m_pDefinition = NULL;
	m_pBlockEA = NULL;
	m_iBlockSalt = 0;
	m_pParentTransformLS = NULL;
	m_pParentTransformEA = NULL;
	m_pSpawnMesh = NULL;
	m_pSpline = NULL;
	m_pSpawner = NULL;
	SetSplineUsage(0.0f, 1.0f);
	m_vVelocity = MathsStorage::Float4A16(0.0f, 0.0f, 0.0f, 0.0f);
	m_vLighting = MathsStorage::Float4A16(1.0f, 1.0f, 1.0f, 1.0f);
	m_fIntensity = 1.0f;
	m_fLifeScale = 1.0f;
	m_iEventIndex = -1;
	m_iSalt = 0;
}


void C_Emitter::SetDefinition(const EmitterDefinition* pDefinition, C_EventInstance* pEventInstance)
{
	m_pEventInstance = pEventInstance;
	if(pEventInstance)
		m_fCullDistance = pEventInstance->GetEvent()->GetDefinition().fCullDistance;
	else
		m_fCullDistance = pDefinition->fCullDistance;
	m_pDefinition = pDefinition;
	m_pBlockEA = NULL;
	m_pParentTransformLS = NULL;
	m_pParentTransformEA = NULL;
	m_pSpawnMesh = NULL;
	m_pSpawner = NULL;
	m_pSpline = NULL;
	m_fIntensity = 1.0f;
	m_fLifeScale = 1.0f;
	m_bVisible = true;
	m_bActive = true;
	m_bCachedAdditive = pDefinition->pEffect->GetRenderSettings().additive;
	m_bDelete = false;
	m_iEventIndex = -1;
	m_nNumEmit = 0;
	Reset();
}

void C_Emitter::SetSplineUsage(const F32 fBegin, const F32 fEnd)
{
	if(fBegin >= 0.0f && fBegin < 1.0f && fBegin < fEnd && fEnd > 0.0f && fEnd <= 1.0f)
	{
		m_fSplineBegin = fBegin;
		m_fSplineEnd = fEnd;
		m_fSplineLength = fEnd - fBegin;
		m_fSplineFraction = m_fSplineLength / 1.0f;
	}
}

#endif

// resets animation of spawn events, ie looping animations
void C_Emitter::Reset(C_Random& random)
{
	m_bAlive = true;
	m_fCycle = 0.0f;
	m_fEmitCycle = 0.0f;
	
	F32 fTime = m_pDefinition->fTime;
	F32 fTimeVariation = m_pDefinition->fTime;
	F32 fDuration = m_pDefinition->fDuration;
	F32 fDurationVariation = m_pDefinition->fDurationVariation;
	m_fTime = fTime + random.FRand(-fTimeVariation, fTimeVariation);
	m_fDuration = m_fTime + fDuration + random.FRand(-fDurationVariation, fDurationVariation);
}

void C_Emitter::Reset()
{
	m_bAlive = true;
	m_fCycle = 0.0f;
	m_fEmitCycle = 0.0f;
	
	F32 fTime = m_pDefinition->fTime;
	F32 fDuration = m_pDefinition->fDuration;
	m_fTime = fTime;
	m_fDuration = fTime + fDuration;
}

void C_Emitter::Free()
{
	// tell owner (event instance) that we are freeing ourself
	if(m_pEventInstance)
	{
		m_pEventInstance->RemoveEmitter(m_iEventIndex);
		m_pEventInstance = NULL;
		m_iEventIndex = -1;
	}
	m_pEmitterList->Free(this);
}

void C_Emitter::AtomicFree(C_Emitter* pEmitterEA)
{
	// tell owner (event instance) that we are freeing ourself
	if(m_pEventInstance)
	{
		m_pEventInstance->RemoveEmitter(m_iEventIndex);
		m_pEventInstance = NULL;
		m_iEventIndex = -1;
	}
	m_pEmitterList->AtomicFree(pEmitterEA);
}


U32 C_Emitter::GetSpawnAmountNormal(const F32 fFrameTime, const F32 fIntensityModifier)
{
	U32 nNumEmit = 0;
	F32 fEmitRate = m_pDefinition->fRate * fIntensityModifier;
	F32 fOneOverEmitRate = (fEmitRate != 0.f) ? (1.0f / fEmitRate) : 10000.f;

	if(m_fCycle >= m_pDefinition->fTime)
	{
		if(m_fCycle > m_fDuration)
			m_bAlive = false;	// emitter goes inactive
		
		m_fEmitCycle += fFrameTime;
		if(m_fEmitCycle > fOneOverEmitRate && fEmitRate != 0.0f)
		{
			// get number of emissions in F32
			F32 fEmit = (m_fEmitCycle / fOneOverEmitRate);

			assert(fEmit >= 0.0f);

			// get number of particles to emit
			nNumEmit = (U32)fEmit;

			// subtract emitted particle time from emit timer
			m_fEmitCycle -= nNumEmit * fOneOverEmitRate;
		}
	}

	assert(nNumEmit < 10000);
	return nNumEmit;
}

U32 C_Emitter::GetSpawnAmountBurst(C_Random& random, const F32 fIntensityModifier)
{
	U32 nNumEmit = 0;
	F32 fCeil;
	if(m_fCycle >= m_pDefinition->fTime)
	{
		m_bAlive = false;	// emitter goes inactive
		fCeil = ceilf((m_pDefinition->fRate + random.FRand(-m_pDefinition->fRateVariation, m_pDefinition->fRateVariation)) * fIntensityModifier);
		assert(fCeil >= 0.0f);
		
		nNumEmit = (U32)fCeil;
	}

	assert(nNumEmit < 10000);
	return nNumEmit;
}

U32 C_Emitter::GetSpawnAmountPersistent()
{
	U32 nNumEmit = 0;
	ParticleBlock* pBlock = GetBlock();

	if(!pBlock)
		nNumEmit = (U32)m_pDefinition->fRate;
	else
	{
		if(pBlock->m_pEffect == (I_DynamicEffect*)m_pDefinition->pEffect)
			nNumEmit = (U32)Maths::Max(m_pDefinition->fRate - pBlock->m_Element.Size(), 0.0f);
		else
			nNumEmit = (U32)m_pDefinition->fRate;
	}

	assert(nNumEmit < 10000);
	return nNumEmit;
}

void C_Emitter::Update(C_Random& random, const EmitterUpdateConstants* pUpdateConstants, C_Emitter* pEmitterEA)
{
	assert(m_pDefinition);

	BIZ_ASSERT(m_fLifeScale > 0.0f);

	// if flagged for delete then remove now
	if(m_bDelete)
	{
		AtomicFree(pEmitterEA);
		return;
	}

	if(m_bCalculatePositions || m_bCalculateVelocity)
	{
		// get current and previous position taking into account external transformations
		Maths::HVec3 vPositionPrevious(Maths::Load<Maths::HVec3>(&m_vPosition));
		Maths::HVec3 vPosition(Maths::Load<Maths::HVec3>(&m_Transform.GetPosition()));
		if(m_pParentTransformLS && m_pDefinition->eSpace == E_EmitterSpace_World)
		{
			Maths::C_QuatTrans transform(*GetParentTransform());
			vPosition = vPosition * transform.Matrix();
		}
		if(m_bCalculatePositions)
		{
			Maths::Store(vPosition, &m_vPosition);
			Maths::Store(vPositionPrevious, &m_vPositionPrevious);
		}
	
		if(m_bCalculateVelocity)
		{
			Maths::HVec3 vVelocity = (vPosition - vPositionPrevious) * pUpdateConstants->timeStepRcp;
			Maths::Store(vVelocity, &m_vVelocity);
		}

		// if the emitter is deactivated or not visible (ie not spawning) set the previous emission position
		// or else we'll get lots of particles emitted when it reactivates if it emits/distance
		if(!m_bVisible || !m_bActive)
			Maths::Store(vPosition, &m_vPositionPreviousEmit);
	}
	else
	{
		int a = 0;
	}

	m_bSpawning = false;
	m_bCalculatePositions = true;
	m_bCalculateVelocity = true;

	m_fCycle += pUpdateConstants->timeStep;

	// only bother spawning if emitter is visible on screen, active (from intensities etc) and alive (expired etc)
	if(m_bVisible && m_bActive && m_bAlive)
	{
		Maths::HVec3 vPosition(Maths::Load<Maths::HVec3>(&m_vPosition));

		F32 fIntensityModifier;

		// if we are using a mesh for spawning off modify the intensity by the area of the spawn mesh
		if(m_pDefinition->eDistribution == E_EmitterDistribution_Mesh && m_pSpawnMesh)
			fIntensityModifier = m_fIntensity * pUpdateConstants->intensity * m_pSpawnMesh->GetArea();
		else
			fIntensityModifier = m_fIntensity * pUpdateConstants->intensity;

		F32 fIntensityModifierRcp = 1.0f / fIntensityModifier;

	#ifndef CFG_GOLD
	#ifndef PLATFORM_SPU
		// check for run time errors where spawn meshes/splines etc haven't been set
		if(m_pDefinition->eDistribution == E_EmitterDistribution_Mesh && !m_pSpawnMesh)
		{
			if (MT::AtomicIncrement(&g_EmitterWarnings) < 3)
				LOGERROR(g_Log, "DynamicEffect: Emitter (%x) has mesh distribution but NULL m_pSpawnMesh!", m_pDefinition->id);
			return;
		}
		
		if(m_pDefinition->eDistribution == E_EmitterDistribution_Spline && !m_pSpline)
		{
			if (MT::AtomicIncrement(&g_EmitterWarnings) < 5)
				LOGERROR(g_Log, "DynamicEffect: Emitter (%x) has spline distribution but NULL m_pSpline!", m_pDefinition->id);
			return;
		}

		if(m_pDefinition->pEffect->GetPhysicsSettings().eBehaviour == E_PhysicsBehaviour_SplineFollower && !m_pSpline)
		{
			if (MT::AtomicIncrement(&g_EmitterWarnings) < 3)
				LOGERROR(g_Log, "DynamicEffect: Emitter (%x) trying to emit spline following particles with NULL m_pSpline!", m_pDefinition->id);
			return;
		}
	#endif
	#endif

		switch(m_pDefinition->eSpawn)
		{
		case E_EmitterSpawn_Burst:
			m_nNumEmit = GetSpawnAmountBurst(random, fIntensityModifier);
			break;
		case E_EmitterSpawn_Normal:
			m_nNumEmit = GetSpawnAmountNormal(pUpdateConstants->timeStep, fIntensityModifier);
			break;
		case E_EmitterSpawn_Persistent:
			m_nNumEmit = GetSpawnAmountPersistent();
			break;
		}

		#ifndef PLATFORM_SPU
		if(m_nNumEmit > 5000)
		{
			LOGERROR(g_Log, "DynamicEffect: Emitter (%x) is trying to spawn %d particles. This is too many so stop it.", m_pDefinition->id, m_nNumEmit);
			m_nNumEmit = 0;
		}
		#endif

		// if this emitter emits per distance then work out how much to add here
		if(m_pDefinition->fRateDistance != 0.0f)
		{
			Maths::HVec3 vPositionPreviousEmit(Maths::Load<Maths::HVec3>(&m_vPositionPreviousEmit));
			Maths::HF32 fRateDistance = m_pDefinition->fRateDistance * fIntensityModifierRcp;
			Maths::HF32 fRateDistanceSq = fRateDistance * fRateDistance;

			Maths::HVec3 vDifference = vPosition - vPositionPreviousEmit;
			Maths::HF32 fLengthSq = Maths::Length2(vDifference);

			if(fLengthSq > fRateDistanceSq)
				m_nNumEmit = (U32)(fLengthSq.Sqrt()/fRateDistance);

			// work out interpolation factors for spawning between previous and current positions
			if (m_nNumEmit > 0)
				m_fInterpolateDelta = 1.0f/(F32)m_nNumEmit;
			m_fInterpolate = 0.0f;
		}

		if(m_pDefinition->eDistribution == E_EmitterDistribution_Line || m_pDefinition->eDistribution == E_EmitterDistribution_Capsule)
		{
			Maths::HVec3 vStart(Maths::Load<Maths::HVec3>(&m_Transform.GetPosition()));
			Maths::HVec3 vEnd(Maths::Load<Maths::HVec3>(&m_Target.GetPosition()));

			if(m_pDefinition->fRateDistance != 0.0f)
			{
				Maths::HF32 fRateDistance = m_pDefinition->fRateDistance * fIntensityModifierRcp;
				Maths::HF32 fRateDistanceSq = fRateDistance * fRateDistance;

				Maths::HVec3 vDifference = vStart - vEnd;
				Maths::HF32 fLengthSq = Maths::Length2(vDifference);

				if(fLengthSq > fRateDistanceSq)
					m_nNumEmit = (U32)(fLengthSq.Sqrt()/fRateDistance);
				else
					m_nNumEmit = 0;
			}

			// store end position into previous position
			Maths::Store(vEnd, &m_vPositionPrevious);
		
			// work out interpolation factors for spawning between previous and current positions
		
			F32 fLineOffset = 0.0f;
			if (m_nNumEmit > 0)
			{
				F32 fNumEmit = (F32)(m_nNumEmit);
				F32 fInterpolateDelta = 1.0f/fNumEmit;
				fLineOffset = random.FRand(fInterpolateDelta);
				m_fInterpolateDelta = fInterpolateDelta;
			}
			m_fInterpolate = fLineOffset;
		}

		#ifndef PLATFORM_SPU
		if(m_pDefinition->eDistribution == E_EmitterDistribution_Spline)
		{
			Maths::HF32 fLength = m_pSpline->GetCurve()->GetArcLengthLinear() * m_fSplineFraction;

			if(m_pDefinition->fRateDistance != 0.0f)
			{
				Maths::HF32 fRateDistance = m_pDefinition->fRateDistance * fIntensityModifierRcp;
				
				if(fLength > fRateDistance)
					m_nNumEmit = (U32)(fLength/fRateDistance);
				else
					m_nNumEmit = 0;
			}

			F32 fLineOffset = 0.0f;
			if (m_nNumEmit > 0)
			{
				F32 fNumEmit = (F32)(m_nNumEmit);
				F32 fInterpolateDelta = m_fSplineLength/fNumEmit;
				fLineOffset = random.FRand(fInterpolateDelta) + m_fSplineBegin;

				m_fInterpolateDelta = fInterpolateDelta;
			}
			m_fInterpolate = fLineOffset;
		}
		#endif
		
		if(m_nNumEmit > 0)
		{
			// atomically add to appropriate spawning emitters queue
			if(m_pSpawner)
			{
				S32 queuePosition = MT::AtomicIncrement(pUpdateConstants->pSpawningCustomEmittersCount);
				if(queuePosition < pUpdateConstants->spawningCustomEmittersCapacity)
				{
					// SPU this
					SpawningEmitter* pSpawningEmitter = &pUpdateConstants->pSpawningCustomEmitters[queuePosition];
					Spu::Set((INTPTR*)&pSpawningEmitter->pEmitter, (INTPTR)pEmitterEA);	// [Blur2PC][x64] pointer-wide
					Spu::Set((INTPTR*)&pSpawningEmitter->pEffect, (INTPTR)m_pDefinition->pEffect);	// [Blur2PC][x64] pointer-wide
				}
			}
			else
			{
				S32 queuePosition = MT::AtomicIncrement(pUpdateConstants->pSpawningEmittersCount);
				if(queuePosition < pUpdateConstants->spawningEmittersCapacity)
				{
					// SPU this
					SpawningEmitter* pSpawningEmitter = &pUpdateConstants->pSpawningEmitters[queuePosition];
					Spu::Set((INTPTR*)&pSpawningEmitter->pEmitter, (INTPTR)pEmitterEA);	// [Blur2PC][x64] pointer-wide
					Spu::Set((INTPTR*)&pSpawningEmitter->pEffect, (INTPTR)m_pDefinition->pEffect);	// [Blur2PC][x64] pointer-wide
				}
			}

			m_bSpawning = true;
		}
	}
	else
		m_nNumEmit = 0;

	// set alive to false if past duration
	if(m_fCycle > m_fDuration)
	{
		// if we are a repeating emitter then reset cycle & time and generate a new duration
		if(m_pDefinition->bRepeating)
		{
			Reset(random);
		}
		else
		{
			// if we have an owner and its repeating then sit tight cause it will reset us soon
			if(m_pEventInstance && m_pEventInstance->Repeating())
				m_fCycle = 0.0f;	// do nothing
			else
			{
				if(m_nNumEmit == 0)
					AtomicFree(pEmitterEA);			// we aren't managed so free ourselves
				else
					m_bDelete = true;
				return;
			}
		}
	}
}

#ifndef PLATFORM_SPU

void C_Emitter::SetLighting(const Maths::C_RGBA_F32& vLighting, const Maths::C_RGBA_F32& vTintColour)
{
	Maths::C_RGBA_F32 vColour = vLighting;

	if(m_bCachedAdditive)
	{
		Maths::C_RGBA_F32 vEmissiveColour(Maths::Load<Maths::C_RGBA_F32>(&m_pManager->GetEmissiveColour()));
		vColour = vEmissiveColour;
	}
	vColour = vColour * vTintColour;
	
	Maths::Store(vColour, &m_vLighting);
}

void C_Emitter::SetVisibilityFlags(U32 visibilityFlags)
{
	ParticleBlock* pBlock = GetBlock();
	if (pBlock)
		pBlock->SetVisibilityFlags(visibilityFlags);
	m_VisibilityFlags = visibilityFlags;
}

void C_Emitter::DisplayDebug() const
{
	Maths::C_QuatTrans transform(GetWorldTransform());
	Maths::HMat43 matrix = transform.Matrix();
	
	/*Maths::HVec3 vPosition(Maths::Load<Maths::HVec3>(&GetPosition()));
	Maths::C_Sphere sphere = GetBoundingSphere();
	Maths::HVec3 vVelocity(Maths::Load<Maths::HVec3>(&GetVelocity()));

	if(m_pDefinition->eDistribution != E_EmitterDistribution_Point)
	{
		Maths::HVec3 vSize(Maths::Load<Maths::HVec3>(&m_pDefinition->vSize));
		Maths::HVec3 vHalfSize = vSize * 0.5f;
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(-vHalfSize, vHalfSize, matrix, Maths::RGBA(255, 255, 0, 255));
	}

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(sphere.Centre(), sphere.Radius(), Maths::RGBA(255, 255, 255, 255));*/
	
	Maths::HF32 size = m_bActive ? 1.0f : 0.5f;
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix(matrix, size);
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(vPosition, vVelocity, 0.5f, 0.1f, Maths::RGBA(255, 255, 0, 255));
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText( vPosition + Maths::HVec3(0.0f, -1.0f, 0.0f), Util::C_String().Printf("%d", m_nNumEmit));

	/*if(m_pSpawnMesh)
	{
		Maths::HMat43 parentMatrix;
		if(GetParentTransform())
		{
			Maths::C_QuatTrans parentTransform(GetWorldTransform());
			parentMatrix = parentTransform.Matrix();
		}
		
		m_pSpawnMesh->Display(parentMatrix);
	}*/
}

Maths::C_Sphere C_Emitter::GetBoundingSphere() const
{
	Maths::C_QuatTrans transform(m_Transform);
	Maths::C_QuatTrans globalTransform;
	Maths::C_Sphere sphere;

	if(m_pParentTransformLS)
	{
		globalTransform = Maths::C_QuatTrans(*m_pParentTransformLS);
		transform = transform * globalTransform;
	}

	if(m_pDefinition->eDistribution == E_EmitterDistribution_Line)
	{
		// get a sphere which encompasses start and end points of line
		Maths::C_QuatTrans target(m_Target);
		if(m_pParentTransformLS)
			target = target * globalTransform;

		Maths::HVec3 position1 = m_pDefinition->cullSphere.Centre() * transform.Matrix();
		Maths::HVec3 position2 = m_pDefinition->cullSphere.Centre() * target.Matrix();
		Maths::HVec3 centre = (position1 + position2) * Maths::Constants<Maths::HF32>::Half();
		Maths::HF32 radius = Maths::Length(position1 - position2) * Maths::Constants<Maths::HF32>::Half() + m_pDefinition->cullSphere.Radius();

		sphere = Maths::C_Sphere(centre, radius);
	}
	else if((m_pDefinition->eDistribution == E_EmitterDistribution_Spline || m_pDefinition->pEffect->GetPhysicsSettings().eBehaviour == E_PhysicsBehaviour_SplineFollower) && m_pSpline)
	{
		sphere = m_pSpline->GetAabb().BoundingSphere() * transform.Matrix();
	}
	else
		sphere = Maths::C_Sphere(m_pDefinition->cullSphere.Centre() * transform.Matrix(), m_pDefinition->cullSphere.Radius());


	return sphere;
}

#endif

// update task
void EmitterUpdateTask(Scheduler::Task* pTask)
{
	PERF_EVENT("Particle.EmitterUpdate");

	EmitterUpdateData* pUpdateData = (EmitterUpdateData*)pTask->ParamData;

#ifdef PLATFORM_SPU
	// fetch emitter pointers
	C_Emitter** ppEmitters = (C_Emitter**)Spu::FetchAllocUnaligned(pUpdateData->ppEmitters, sizeof(C_Emitter*) * pUpdateData->numEmitters);
	pUpdateData->pUpdateConstants = (EmitterUpdateConstants*)Spu::FetchAlloc(pUpdateData->pUpdateConstants, sizeof(EmitterUpdateConstants));
	
	C_Random* pRandomEA = pUpdateData->pUpdateConstants->pRandom[Scheduler::GetWorkerID()];
	C_Random* pRandom = (C_Random*)Spu::FetchAlloc(pRandomEA, sizeof(C_Random));

	C_EmitterList* pEmitterListLS = (C_EmitterList*)Spu::Alloc(sizeof(C_EmitterList));
	C_EmitterList* pEmitterListEA = NULL;

	for(U32 i = 0; i < pUpdateData->numEmitters; ++i)
	{
		Spu::MemBaseMarker marker;
		C_Emitter* pEmitter = ppEmitters[i];

		// fetch emitter
		C_Emitter* pEmitterEA = pEmitter;
		pEmitter = (C_Emitter*)Spu::FetchAlloc(pEmitterEA, sizeof(C_Emitter));

		// fetch emitter list
		if(!pEmitterListEA)
		{
			pEmitterListEA = pEmitter->m_pEmitterList;
			Spu::FetchAsync(pEmitterListLS, pEmitterListEA, sizeof(C_EmitterList), 0);
		}
		pEmitter->m_pEmitterList = pEmitterListLS;

		// fetch the parent transformation
		if(pEmitter->m_pParentTransformEA)
			pEmitter->m_pParentTransformLS = (C_Node*)Spu::FetchAllocAsync(pEmitter->m_pParentTransformEA, sizeof(C_Node), 0);

		// fetch spawn mesh
		const C_SpawnMesh* pSpawnMeshEA = pEmitter->m_pSpawnMesh;
		if(pEmitter->m_pSpawnMesh)
		{
			pEmitter->m_pSpawnMesh = (C_SpawnMesh*)Spu::FetchAlloc(pEmitter->m_pSpawnMesh, sizeof(C_SpawnMesh));
			pEmitter->m_pSpawnMesh->SpuFetch();
		}

		// fetch emitter definition
		const EmitterDefinition* pDefinitionEA = pEmitter->m_pDefinition;
		pEmitter->m_pDefinition = (EmitterDefinition*)Spu::FetchAllocAsync(pDefinitionEA, sizeof(EmitterDefinition), 0);

		// fetch event instance
		C_EventInstance* pEventInstanceEA = pEmitter->m_pEventInstance;
		if(pEventInstanceEA)
			pEmitter->m_pEventInstance = (C_EventInstance*)Spu::FetchAllocAsync(pEmitter->m_pEventInstance, sizeof(C_EventInstance), 0);

		// fetch block if there is one
		pEmitter->m_pBlockLS = (ParticleBlock*)Spu::Alloc(sizeof(ParticleBlock));
		if(pEmitter->m_pBlockEA)
			Spu::FetchAsync(pEmitter->m_pBlockLS, pEmitter->m_pBlockEA, sizeof(ParticleBlock), 0);
		Spu::Wait(0);

		// update emitter
		pEmitter->Update(*pRandom, pUpdateData->pUpdateConstants, pEmitterEA);

		if(pEmitter->m_pSpawnMesh)
			pEmitter->m_pSpawnMesh = pSpawnMeshEA;
		// dma back updated data
		pEmitter->m_pParentTransformLS = pEmitter->m_pParentTransformEA;
		pEmitter->m_pDefinition = pDefinitionEA;
		pEmitter->m_pEventInstance = pEventInstanceEA;
		pEmitter->m_pEmitterList = pEmitterListEA;
		Spu::Store(pEmitterEA, pEmitter, sizeof(C_Emitter));
	}

	// store random back to main
	Spu::Store(pRandomEA, pRandom, sizeof(C_Random));
#else
	C_Random& random = *pUpdateData->pUpdateConstants->pRandom[Scheduler::GetWorkerID()];
	for(U32 i = 0; i < pUpdateData->numEmitters - 1; ++i)
	{
		C_Emitter* pEmitter = pUpdateData->ppEmitters[i];
		C_Emitter* pNext = pUpdateData->ppEmitters[i+1];
		DCBT(0, pNext);
		DCBT(128, pNext);
		DCBT(256, pNext);

		pEmitter->Update(random, pUpdateData->pUpdateConstants, pEmitter);
	}
	C_Emitter* pEmitter = pUpdateData->ppEmitters[pUpdateData->numEmitters - 1];
	pEmitter->Update(random, pUpdateData->pUpdateConstants, pEmitter);
#endif
}

} // Particle
} // DynamicEffect

JQ_DECLARE_SPU_MODULE(ParticleEmitterUpdateModule, DynamicEffect::Particle::EmitterUpdateTask, dynamic_particle_emitterupdate, "dynamic.scheduler");















