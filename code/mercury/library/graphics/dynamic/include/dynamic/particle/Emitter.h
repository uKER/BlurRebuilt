#ifndef RENDER_DYNAMICEFFECT_PARTICLE_EMITTER_H
#define RENDER_DYNAMICEFFECT_PARTICLE_EMITTER_H

#ifdef _MSC_VER
#pragma once
#endif



// this is required for splines. LAME
#ifndef PLATFORM_SPU
#include <Xt/XtGame.h>
#include <Core/XtStdx.h>
#include <Xt/XtStd.h>
#include <Misc_shared/NewCurveHermite.h>
#include <Misc_Shared/NewCurve/Actor/ActorSplineTypes.h>
#else
namespace Curve
{
class C_ActorCurve3D;
}
#endif

// other includes
#include "Dynamic/Particle/Random.h"
#include "Dynamic/Particle/Block.h"
#include "Dynamic/Particle/Enums.h"
#include "Dynamic/Particle/SpawnMesh.h"
#include "Dynamic/Emitter.h"
#include "Dynamic/Atomic.h"

extern Scheduler::jqModule ParticleEmitterUpdateModule;

namespace DynamicEffect
{
namespace Particle
{

class I_ParticleEffectBase;
class C_ParticleManager;
class C_SpawnMesh;
class I_Spawner;
class ParticleBlock;
class C_EmitterList;
struct EmitterUpdateConstants;

// provides a superset of all attributes needed to create particles in any system
// as such any emitter can be used to emit into any effect, but not all parameters are necessarily used
struct ALIGN16 EmitterDefinition
{
	I_ParticleEffectBase*		pEffect;
	E_EmitterSpawn				eSpawn;
	E_EmitterSpace				eSpace;
	E_EmitterOrientation		eOrientation;
	E_EmitterDistribution		eDistribution;
	E_Collision					eCollision;
	E_EmitterColourMode			eColourMode;
	E_EmitterUV					eUV;

	F32							fConeOuter;
	F32							fConeInner;
	F32							fRate;
	F32							fRateVariation;
	F32							fRateDistance;
	F32							fTime;
	F32							fTimeVariation;
	F32							fDuration;
	F32							fDurationVariation;
	MathsStorage::Float4A16		vPosition;
	MathsStorage::Float4A16		vRotation;
	
	bool						bRepeating;

	F32							fSpeed;				// initial velocity multiplier
	F32							fSpeedRandom;		
	F32							fSpeedInherited;		// how much initial speed is taken from the emitter's velocity
	F32							fSpeedInheritedRandom;
	F32							fSize;				// initial size multiplier
	F32							fSizeRandom;
	F32							fLife;
	F32							fLifeRandom;
	F32							fForce;
	F32							fForceRandom;
	F32							fCullDistance;
	F32							fEmitRadiusSq;	// always emit within this radius, even if offscreen
	F32							fExpiryTime;	// how long before particles emitted from this emitter are offscreen before they expire
	U32							iParticlesPerBlock;	// max particles per block, for external sorting etc
	MathsStorage::Float4A16		vOrientation;
	MathsStorage::Float4A16		vOrientationRandom;
	MathsStorage::Float4A16		vAngularVelocity;
	MathsStorage::Float4A16		vAngularVelocityRandom;
	MathsStorage::Float4A16		vColour1;	// primary colour
	MathsStorage::Float4A16		vColour2;	// secondary colour
	MathsStorage::Float4A16		vSize;
	Maths::C_Sphere				cullSphere;
	MathsStorage::Float2		lodFade;
	F32							cullDistance;

	U32							id;
};

typedef Btl::Vector<EmitterDefinition> EmitterDefinitions;


class ALIGN16 C_Emitter : public DynamicEffect::AtomicPoolElement, public C_EmitterBase<ParticleBlock>
{
public:
	C_Emitter() : m_iIndex(0xffffffff) { assert(0); } // should never happen, needed for UtList though
	C_Emitter(C_ParticleManager* pRenderer, C_EmitterList* pEmitterList, const U32 iIndex);
	C_Emitter(const EmitterDefinition& settings, const U32 iIndex);
	void					Destroy() {};
	virtual ~C_Emitter();

	void						SetEmitterList(C_EmitterList* pEmitterList) { m_pEmitterList = pEmitterList; }
	void						SetEventIndex(const S32 iIndex);
	S32							GetEventIndex() const;
	U64							GetKey() const;

	void						SetCalculatePositions(bool bCalculatePositions) { m_bCalculatePositions = bCalculatePositions; }
	void						SetCalculateVelocity(bool bCalculateVelocity) { m_bCalculateVelocity = bCalculateVelocity; }
	void						SetVelocity(const Maths::HVec3& vVelocity);
	void						SetVelocity(const MathsStorage::Float4A16& vVelocity);
	void						SetPosition(const MathsStorage::Float4A16& vPosition);
	void						SetPosition(const Maths::HVec3& vPosition);
	void						SetPositionPrevious(const Maths::HVec3& vPosition);
	void						SetPositionPrevious(const MathsStorage::Float4A16& vPosition);
	void						SetPositionPreviousEmit();
	void						SetLighting(const MathsStorage::Float4A16& vLighting);
	void						SetLighting(const MathsStorage::Float4A16& vLighting, const MathsStorage::Float4A16& vTintColour);
	void						SetAlpha(F32 fLighting);
	void						SetCollisionPlane(const MathsStorage::Float4A16& vPlanePosition, const MathsStorage::Float4A16& vPlaneNormal);
	void						SetCollisionPlane(const Maths::HVec3& vPlanePosition, const Maths::HVec3& vPlaneNormal);

	void						SetLighting(const Maths::C_RGBA_F32& vLighting);
	void						SetLighting(const Maths::C_RGBA_F32& vLighting, const Maths::C_RGBA_F32& vTintColour);
	
	void						SetVisibilityFlags(U32 visibilityFlags);
	U32							GetVisibilityFlags() const { return m_VisibilityFlags;};

	void						SetDefinition(const EmitterDefinition* pDefinition, class C_EventInstance* pOwner = NULL);
	const EmitterDefinition*	GetDefinition() const { return m_pDefinition; }
			
	void						SetIntensity(F32 fIntensity);
	void						SetActive(bool bActive) { m_bActive = bActive; }
	void						SetLifeScale(F32 fLifeScale) { BIZ_ASSERT(fLifeScale > 0.0f); m_fLifeScale = fLifeScale; }

	U32							GetSpawnAmount() const { return m_nNumEmit; }

	void						SetVisible(bool bVisible) { m_bVisible = bVisible; }
	bool						Visible() const { return m_bVisible; }
	bool						Active() const { return m_bActive; }
	bool						Spawning() const { return m_bSpawning; }
	
	// spawn meshes
	void						SetSpawnMesh(const C_SpawnMesh* pSpawnMesh) { m_pSpawnMesh = pSpawnMesh; }
	const C_SpawnMesh*	 		GetSpawnMesh() const { return m_pSpawnMesh; }

	// spawn splines
	void							SetSpline(const Curve::C_ActorCurve3D* pSpline) { m_pSpline = pSpline; }
	const Curve::C_ActorCurve3D*	GetSpline() const { return m_pSpline; }
	void							SetSplineUsage(const F32 fBegin, const F32 fEnd);
	
	// custom spawning
	void						SetSpawner(I_Spawner* pSpawner) { m_pSpawner = pSpawner; }
	I_Spawner*					GetSpawner() { return m_pSpawner; }

	Maths::HVec3				GetWorldNormal() const; // get world normal (emission direction)
	Maths::C_Sphere				GetBoundingSphere() const; // get bounding sphere in world space

#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
	void						SetPosition(XMVECTOR vPosition);
	void						SetPositionPrevious(XMVECTOR vPosition);
	void						SetVelocity(XMVECTOR vVelocity);
	void						SetCollisionPlane(XMVECTOR vPlanePosition, XMVECTOR vPlaneNormal);

	XMVECTOR					GetSpawnPosition(C_Random& random) const;
	XMVECTOR					GetSpawnSize(C_Random& random) const;
	XMVECTOR					GetSpawnLighting() const;
	XMVECTOR					GetSpawnLifeVariance(C_Random& random) const;
	XMVECTOR					GetSpawnOrientation(C_Random& random) const;
	XMVECTOR					GetSpawnAngularVelocity(C_Random& random) const;
	XMVECTOR					GetSpawnPositionPoint() const;
	XMVECTOR					GetSpawnPositionSphere(C_Random& random) const;
	XMVECTOR					GetSpawnPositionBox(C_Random& random) const;
	XMVECTOR					GetSpawnColour(C_Random& random) const;
	XMVECTOR					GetSpawnColourSingleColour() const;
	XMVECTOR					GetSpawnColourLerpColour(C_Random& random) const;
	XMVECTOR					GetSpawnColourLerpChannels(C_Random& random) const;
	XMVECTOR					GetSpawnForceSizeLife(C_Random& random) const;
	XMVECTOR					GetSpawnColourScale() const;
	XMVECTOR					GetSpawnVelocity(C_Random& random) const;
	XMVECTOR					GetSpawnForce(C_Random& random) const;
	XMVECTOR					GetSpawnCollisionPlanePosition() const;
	XMVECTOR					GetSpawnCollisionPlaneNormal() const;
#else
	Maths::HVec3				GetSpawnPosition(C_Random& random) const;
	Maths::HF32					GetSpawnSize(C_Random& random) const;
	Maths::HF32					GetSpawnLifeVariance(C_Random& random) const;
	
	Maths::C_Quaternion			GetSpawnOrientation(C_Random& random) const;
	Maths::HVec3				GetSpawnAngularVelocity(C_Random& random) const;
	Maths::HVec3				GetSpawnPositionPoint() const;
	Maths::HVec3				GetSpawnPositionSphere(C_Random& random) const;
	Maths::HVec3				GetSpawnPositionBox(C_Random& random) const;
	// colour
	Maths::HVec4				GetSpawnLighting() const;
	Maths::HVec4				GetSpawnColour(C_Random& random) const;
	Maths::HVec4				GetSpawnColourSingleColour() const;
	Maths::HVec4				GetSpawnColourLerpColour(C_Random& random) const;
	Maths::HVec4				GetSpawnColourLerpChannels(C_Random& random) const;
	Maths::HVec4				GetSpawnForceSizeLife(C_Random& random) const;
	Maths::HF32					GetSpawnColourScale() const;
	// velocity
	Maths::HVec3				GetSpawnVelocity(C_Random& random) const;
	// collisions
	Maths::HF32					GetSpawnForce(C_Random& random) const;
	Maths::HVec3				GetSpawnCollisionPlanePosition() const;
	Maths::HVec3				GetSpawnCollisionPlaneNormal() const;
#endif

	U32							GetSpawnUV(C_Random& random) const;

	F32							GetSpawnRotation(C_Random& random) const;
	F32							GetSpawnRotationRate(C_Random& random) const;

	const MathsStorage::Float4A16&	GetPosition() const;
	const MathsStorage::Float4A16&	GetPositionPrevious() const;
	const MathsStorage::Float4A16&	GetVelocity() const;

	void						DisplayDebug() const;

	void						Free();
	void						AtomicFree(C_Emitter* pEmitterEA);
	void						Reset();
	void						Reset(C_Random& random);
	
	friend class				C_EmitterList;
	friend class				C_EventInstance;
	friend class				C_ParticleManager;
	friend class				I_ParticleEffectBase;
	friend class				I_ParticleEffect;
	friend void					EmitterUpdateTask(Scheduler::Task* pTask);

#ifndef PLATFORM_SPU
protected:
#endif

	void						Update(C_Random& random, const EmitterUpdateConstants* pUpdateConstants, C_Emitter* pEmitterEA);
	U32							GetSpawnAmountNormal(const F32 fFrameTime, const F32 fIntensityModifier);
	U32							GetSpawnAmountBurst(C_Random& random, const F32 fIntensityModifier);
	U32							GetSpawnAmountPersistent();
	void						SetDefaults();

	// timing stuff
	U32							m_nNumEmit;
	F32							m_fTime;
	F32							m_fDuration;
	F32							m_fCycle;
	F32							m_fEmitCycle;

	// aligned vectors, 128 bytes
	MathsStorage::Float4A16		m_vLighting;
	MathsStorage::Float4A16		m_vPositionPrevious;
	MathsStorage::Float4A16		m_vPositionPreviousEmit; // previous emission position
	MathsStorage::Float4A16		m_vVelocity;		// generally zero but can be set from supereffects, event instances, etc
	
	MathsStorage::Float4A16		m_vCollisionPlanePosition;	// only used by planar collision particles
	MathsStorage::Float4A16		m_vCollisionPlaneNormal;

	F32							m_fLifeScale;

	bool						m_bVisible;
	bool						m_bActive;
	bool						m_bSpawning;
	bool						m_bAlive;
	bool						m_bCalculatePositions;
	bool						m_bCalculateVelocity;
	bool                        m_bCachedAdditive;
	bool						m_bDelete;
	const U32					m_iIndex;					// global index, unique per emitter
	S32							m_iEventIndex;				// this emitters position in array of emitters in an event
	U32							m_VisibilityFlags;

	F32								m_fCullDistance;
	const C_SpawnMesh*				m_pSpawnMesh;				// optional mesh for spawning off
	const Curve::C_ActorCurve3D*	m_pSpline;					// optional spline for spawning off
	const EmitterDefinition*		m_pDefinition;

	I_Spawner*					m_pSpawner;
	
	F32							m_fSplineBegin;
	F32							m_fSplineEnd;
	F32							m_fSplineLength;
	F32							m_fSplineFraction;

	
	
	F32							m_fIntensity;	// used to scale the amount of particles emitted
	C_ParticleManager*			m_pManager;
	C_EventInstance*			m_pEventInstance;	// pointer to event instance if we are being managed by one
	C_EmitterList*				m_pEmitterList;
	// used when spawning to interpolate spawn positions from previous to current position

	mutable F32					m_fInterpolate;
	F32							m_fInterpolateDelta;
	mutable C_Node				m_CreateNode;
};


FORCE_INLINE void C_Emitter::SetEventIndex(S32 iEventIndex)
{
	m_iEventIndex = iEventIndex;
}

FORCE_INLINE S32 C_Emitter::GetEventIndex() const
{
	return m_iEventIndex;
}

FORCE_INLINE U64 C_Emitter::GetKey() const
{
	return ((U64)m_iIndex << 32) + m_iSalt;
}

FORCE_INLINE void C_Emitter::SetIntensity(F32 fIntensity)
{
	assert(fIntensity >= 0.0f);
	assert(fIntensity <= 10.0f);

	m_fIntensity = fIntensity;
}


FORCE_INLINE void C_Emitter::SetAlpha(F32 fAlpha)
{
	m_vLighting.w = fAlpha;
}

FORCE_INLINE void C_Emitter::SetLighting(const Maths::C_RGBA_F32& vLighting)
{
	Maths::HVec4 vOne(Maths::Constants<Maths::HF32>::One());
	SetLighting(vLighting, Maths::C_RGBA_F32(vOne));
}

FORCE_INLINE void C_Emitter::SetLighting(const MathsStorage::Float4A16& vLighting)
{
	SetLighting(Maths::Load<Maths::C_RGBA_F32>(&vLighting));
}

FORCE_INLINE void C_Emitter::SetLighting(const MathsStorage::Float4A16& vLighting, const MathsStorage::Float4A16& vTintColour)
{
	SetLighting(Maths::Load<Maths::C_RGBA_F32>(&vLighting), Maths::Load<Maths::C_RGBA_F32>(&vTintColour));
}

FORCE_INLINE void C_Emitter::SetCollisionPlane(const MathsStorage::Float4A16& vCollisionPlanePosition, const MathsStorage::Float4A16& vCollisionPlaneNormal)
{
	m_vCollisionPlanePosition = vCollisionPlanePosition;
	m_vCollisionPlaneNormal = vCollisionPlaneNormal;
}

FORCE_INLINE void C_Emitter::SetCollisionPlane(const Maths::HVec3& vCollisionPlanePosition, const Maths::HVec3& vCollisionPlaneNormal)
{
	Maths::Store(vCollisionPlanePosition, &m_vCollisionPlanePosition);
	Maths::Store(vCollisionPlaneNormal, &m_vCollisionPlaneNormal);
}

FORCE_INLINE void C_Emitter::SetVelocity(const MathsStorage::Float4A16& vVelocity)
{
	m_vVelocity = vVelocity;
	m_bCalculateVelocity = false;
}

FORCE_INLINE void C_Emitter::SetVelocity(const Maths::HVec3& vVelocity)
{
	Maths::Store(vVelocity, &m_vVelocity);
	m_bCalculateVelocity = false;
}

FORCE_INLINE void C_Emitter::SetPosition(const MathsStorage::Float4A16& vPosition)
{
	m_vPosition = vPosition;
}

FORCE_INLINE void C_Emitter::SetPosition(const Maths::HVec3& vPosition)
{
	Maths::Store(vPosition, &m_vPosition);
}

FORCE_INLINE void C_Emitter::SetPositionPrevious(const MathsStorage::Float4A16& vPosition)
{
	m_vPosition = vPosition;
	m_vPositionPrevious = vPosition;
	m_vPositionPreviousEmit = vPosition;
}

FORCE_INLINE void C_Emitter::SetPositionPrevious(const Maths::HVec3& vPosition)
{
	Maths::Store(vPosition, &m_vPosition);
	Maths::Store(vPosition, &m_vPositionPrevious);
	Maths::Store(vPosition, &m_vPositionPreviousEmit);
}

FORCE_INLINE void C_Emitter::SetPositionPreviousEmit()
{
	m_vPositionPreviousEmit = m_vPosition;
}

#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
FORCE_INLINE void C_Emitter::SetPosition(XMVECTOR vPosition)
{
	XMStoreFloat4A16((XMFLOAT4A16*)&m_vPosition, vPosition);
}

FORCE_INLINE void C_Emitter::SetPositionPrevious(XMVECTOR vPosition)
{
	XMStoreFloat4A16((XMFLOAT4A16*)&m_vPosition, vPosition);
	XMStoreFloat4A16((XMFLOAT4A16*)&m_vPositionPrevious, vPosition);
	XMStoreFloat4A16((XMFLOAT4A16*)&m_vPositionPreviousEmit, vPosition);
}

FORCE_INLINE void C_Emitter::SetVelocity(XMVECTOR vVelocity)
{
	XMStoreFloat4A16((XMFLOAT4A16*)&m_vVelocity, vVelocity);
}


FORCE_INLINE void C_Emitter::SetCollisionPlane(XMVECTOR vCollisionPlanePosition, XMVECTOR vCollisionPlaneNormal)
{
	XMStoreFloat4A16((XMFLOAT4A16*)&m_vCollisionPlanePosition, vCollisionPlanePosition);
	XMStoreFloat4A16((XMFLOAT4A16*)&m_vCollisionPlaneNormal, vCollisionPlaneNormal);
}

FORCE_INLINE XMVECTOR C_Emitter::GetSpawnPosition(C_Random& random) const
{
	XMVECTOR vPosition;
	XMVECTOR vPositionCentre = GetSpawnPositionPoint();

	switch(m_pDefinition->eDistribution)
	{
	case E_EmitterDistribution_Point:
	case E_EmitterDistribution_Line:
		vPosition = vPositionCentre;
		break;
	case E_EmitterDistribution_Box:
		vPosition = GetSpawnPositionBox(random);
		break;
	case E_EmitterDistribution_Spline:
	case E_EmitterDistribution_Capsule:
	case E_EmitterDistribution_Sphere:
		vPosition = GetSpawnPositionSphere(random);
		break;
	case E_EmitterDistribution_Mesh:
		if (m_pSpawnMesh)
		{
			m_CreateNode = m_pSpawnMesh->GetRandomNode(random);
			vPosition = XMLoadFloat4A16((XMFLOAT4A16*)&m_CreateNode.GetPosition());
		}
		else
			vPosition = XMVectorZero();
		break;
	default:
		vPosition = XMVectorZero();
	}

	XMMATRIX mTransform = XMMatrixAffineTransformation(XMVectorSplatOne(), XMVectorZero(), XMLoadFloat4A16((XMFLOAT4A16*)&m_Transform.GetRotation()), XMLoadFloat4A16((XMFLOAT4A16*)&m_Transform.GetPosition()));
	
	// if using external transformation for emitter position transform here
	if(m_pParentTransformLS && m_pDefinition->eSpace == E_EmitterSpace_World)
	{
		XMFLOAT4A16* pPosition = (XMFLOAT4A16*)&m_pParentTransformLS->GetRotation();
		XMFLOAT4A16* pRotation = (XMFLOAT4A16*)&m_pParentTransformLS->GetPosition();
		mTransform = XMMatrixMultiply(mTransform, XMMatrixAffineTransformation(XMVectorSplatOne(), XMVectorZero(), XMLoadFloat4A16(pPosition), XMLoadFloat4A16(pRotation)));
	}
	vPosition = XMVector3TransformCoord(vPosition, mTransform);
	vPositionCentre = XMVector3TransformCoord(vPositionCentre, mTransform);

	if(m_pDefinition->eDistribution == E_EmitterDistribution_Spline || m_pDefinition->eDistribution == E_EmitterDistribution_Line || m_pDefinition->eDistribution == E_EmitterDistribution_Capsule || m_pDefinition->fRateDistance != 0.0f)
	{
		XMVECTOR vInterpolate = XMVectorReplicate(m_fInterpolate);
		XMVECTOR vInterpolateDelta = XMVectorReplicate(m_fInterpolateDelta);

		// choose spline evaluation or linear interpolation
		if(m_pDefinition->eDistribution == E_EmitterDistribution_Spline)
		{
			XMVECTOR vSplinePosition = m_pSpline->GetCurve()->Evaluate(m_fInterpolate).GetV128();
			vPosition = XMVectorAdd(vPosition, vSplinePosition);
		}
		else
		{
			XMVECTOR vPositionPrevious = XMLoadFloat4A16((XMFLOAT4A16*)&m_vPositionPreviousEmit);
			XMVECTOR vInterpolatedPosition = XMVectorAdd(vPosition, XMVectorMultiply(XMVectorSubtract(vPositionPrevious, vPositionCentre), vInterpolate));
			vPosition = XMVectorSelect(vInterpolatedPosition, vPosition, XMVectorSelectControl(0, 0, 0, 1));
		}
		XMStoreScalar(&m_fInterpolate, XMVectorAdd(vInterpolate, vInterpolateDelta));
	}
	
	return vPosition;
}

FORCE_INLINE XMVECTOR C_Emitter::GetSpawnSize(C_Random& random) const
{
	MathsStorage::Float4A16 vSize;
	vSize.x = m_pDefinition->fSize + random.FRand(-m_pDefinition->fSizeRandom, m_pDefinition->fSizeRandom);
	return XMVectorSplatX(XMLoadFloat4A16((XMFLOAT4A16*)&vSize));
}

FORCE_INLINE XMVECTOR C_Emitter::GetSpawnLighting() const
{
	return XMLoadFloat4A16((XMFLOAT4A16*)&m_vLighting);
}

FORCE_INLINE XMVECTOR C_Emitter::GetSpawnColour(C_Random& random) const
{
	switch(m_pDefinition->eColourMode)
	{
	case E_EmitterColourMode_SingleColour:
		return GetSpawnColourSingleColour();
	case E_EmitterColourMode_LerpColour:
		return GetSpawnColourLerpColour(random);
	case E_EmitterColourMode_LerpChannels:
		return GetSpawnColourLerpChannels(random);
	default:
		return Maths::HVec4();
	}
}

FORCE_INLINE XMVECTOR C_Emitter::GetSpawnColourSingleColour() const
{
	return XMLoadFloat4A16((XMFLOAT4A16*)&m_pDefinition->vColour1);
}

FORCE_INLINE XMVECTOR C_Emitter::GetSpawnColourLerpColour(C_Random& random) const
{
	MathsStorage::Float4A16 randomLerp(random.FRand(1.0f), 0.0f, 0.0f, 0.0f);
	XMVECTOR vColour1 = XMLoadFloat4A16((XMFLOAT4A16*)&m_pDefinition->vColour1);
	XMVECTOR vColour2 = XMLoadFloat4A16((XMFLOAT4A16*)&m_pDefinition->vColour2);
	XMVECTOR vRandom = XMLoadFloat4A16((XMFLOAT4A16*)&randomLerp);
	return XMVectorLerpV(vColour1, vColour2, XMVectorSplatX(vRandom));
}

FORCE_INLINE XMVECTOR C_Emitter::GetSpawnColourLerpChannels(C_Random& random) const
{
	MathsStorage::Float4A16 randomLerp(random.FRand(1.0f), random.FRand(1.0f), random.FRand(1.0f), random.FRand(1.0f));
	XMVECTOR vColour1 = XMLoadFloat4A16((XMFLOAT4A16*)&m_pDefinition->vColour1);
	XMVECTOR vColour2 = XMLoadFloat4A16((XMFLOAT4A16*)&m_pDefinition->vColour2);
	XMVECTOR vRandom = XMLoadFloat4A16((XMFLOAT4A16*)&randomLerp);
	return XMVectorLerpV(vColour1, vColour2, vRandom);
}

FORCE_INLINE XMVECTOR C_Emitter::GetSpawnForceSizeLife(C_Random& random) const
{
	XMVECTOR vForce = GetSpawnForce(random);
	XMVECTOR vLife = GetSpawnLifeVariance(random);
	XMVECTOR vSize = GetSpawnSize(random);

	XMVECTOR vLow = XMVectorMergeXY(vForce, vSize);
	XMVECTOR vHigh = XMVectorMergeXY(vForce, vLife);
	
	return XMVectorMergeXY(vLow, vHigh);
}

FORCE_INLINE XMVECTOR C_Emitter::GetSpawnLifeVariance(C_Random& random) const
{
	F32 fLife = m_pDefinition->fLife + random.FRand(-m_pDefinition->fLifeRandom, m_pDefinition->fLifeRandom);
	fLife *= m_fLifeScale;
	return XMVectorReciprocal(XMVectorReplicate(fLife));
}

FORCE_INLINE XMVECTOR C_Emitter::GetSpawnOrientation(C_Random& random) const
{
	MathsStorage::Float4A16 vAngles(m_pDefinition->vOrientation.x + random.FRand(-m_pDefinition->vOrientationRandom.x, m_pDefinition->vOrientationRandom.x),
						m_pDefinition->vOrientation.y + random.FRand(-m_pDefinition->vOrientationRandom.y, m_pDefinition->vOrientationRandom.y),
						m_pDefinition->vOrientation.z + random.FRand(-m_pDefinition->vOrientationRandom.z, m_pDefinition->vOrientationRandom.z),
						0.0f);
	
	XMVECTOR vOrientation = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat4A16((XMFLOAT4A16*)&vAngles));

	if(m_pDefinition->eDistribution == E_EmitterDistribution_Mesh)
		vOrientation = XMQuaternionMultiply(vOrientation, XMLoadFloat4A16((XMFLOAT4A16*)&m_CreateNode.GetRotation()));
	else
		vOrientation = XMQuaternionMultiply(vOrientation, XMLoadFloat4A16((XMFLOAT4A16*)&m_Transform.GetRotation()));
	
	// use global orientation if in world space
	if(m_pParentTransformLS && m_pDefinition->eSpace == E_EmitterSpace_World && m_pDefinition->eOrientation == E_EmitterOrientation_Local)
		vOrientation = XMQuaternionMultiply(vOrientation, XMLoadFloat4A16((XMFLOAT4A16*)&m_pParentTransformLS->GetRotation()));
	
	return vOrientation;
}

FORCE_INLINE XMVECTOR C_Emitter::GetSpawnAngularVelocity(C_Random& random) const
{
	MathsStorage::Float4A16 angularVelocity(	m_pDefinition->vAngularVelocity.x + random.FRand(-m_pDefinition->vAngularVelocityRandom.x, m_pDefinition->vAngularVelocityRandom.x),
						m_pDefinition->vAngularVelocity.y + random.FRand(-m_pDefinition->vAngularVelocityRandom.y, m_pDefinition->vAngularVelocityRandom.y),
						m_pDefinition->vAngularVelocity.z + random.FRand(-m_pDefinition->vAngularVelocityRandom.z, m_pDefinition->vAngularVelocityRandom.z),
						0.0f);

	return XMLoadFloat4A16((XMFLOAT4A16*)&angularVelocity);
}

FORCE_INLINE XMVECTOR C_Emitter::GetSpawnPositionPoint() const
{
	XMVECTOR vPosition = XMLoadFloat4A16((XMFLOAT4A16*)&m_pDefinition->vPosition);
	return vPosition;
}

FORCE_INLINE XMVECTOR C_Emitter::GetSpawnPositionBox(C_Random& random) const
{
	assert(m_pDefinition->eDistribution == E_EmitterDistribution_Box);
	MathsStorage::Float4A16 size(random.FRand(-0.5f, 0.5f), random.FRand(-0.5f, 0.5f), random.FRand(-0.5f, 0.5f), 0.0f);

	// get random position
	XMVECTOR vRandom = XMLoadFloat4A16((XMFLOAT4A16*)&size);
	XMVECTOR vSize = XMVectorMultiply(XMLoadFloat4A16((XMFLOAT4A16*)&m_pDefinition->vSize), vRandom);
	vSize = XMVectorAdd(XMLoadFloat4A16((XMFLOAT4A16*)&m_pDefinition->vPosition), vSize);
	return vSize;
}

FORCE_INLINE XMVECTOR C_Emitter::GetSpawnPositionSphere(C_Random& random) const
{
	// get random values
	MathsStorage::Float4A16 angles(random.FRand(Maths::Pi2), random.FRand(Maths::Pi2), 0.0f, 0.0f);
	MathsStorage::Float4A16 half(0.0f, 0.5f, 0.0f, 0.0f);
	MathsStorage::Float4A16 size(random.FRand(m_pDefinition->vSize.x), random.FRand(m_pDefinition->vSize.y), random.FRand(m_pDefinition->vSize.z), 0.0f);

	XMMATRIX mRotate = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat4A16((XMFLOAT4A16*)&angles));
	// get position within sphere
	XMVECTOR vPosition = XMVector3TransformNormal(XMLoadFloat4A16((XMFLOAT4A16*)&half), mRotate);
	// scale position by size
	vPosition = XMVectorMultiply(vPosition, XMLoadFloat4A16((XMFLOAT4A16*)&size));
	// transform by emitter's rotation
	vPosition = XMVectorAdd(XMLoadFloat4A16((XMFLOAT4A16*)&m_pDefinition->vPosition), vPosition);
	// translate to emitter position
	return vPosition;
}

FORCE_INLINE XMVECTOR C_Emitter::GetSpawnForce(C_Random& random) const
{
	MathsStorage::Float4A16 vForce;
	vForce.x = m_pDefinition->fForce + random.FRand(-m_pDefinition->fForceRandom, m_pDefinition->fForceRandom);
	return XMVectorSplatX(XMLoadFloat4A16((XMFLOAT4A16*)&vForce));
}

FORCE_INLINE XMVECTOR C_Emitter::GetSpawnVelocity(C_Random& random) const
{
	XMVECTOR qLocal;
	if(m_pDefinition->eDistribution == E_EmitterDistribution_Mesh)
		qLocal = XMLoadFloat4A16((XMFLOAT4A16*)&m_CreateNode.GetRotation());
	else
		qLocal = XMLoadFloat4A16((XMFLOAT4A16*)&m_Transform.GetRotation());
	
	// get a rotation quaterion for the cone
	// this quaternion depends on the velocity being in a Y-Up direction by default
	// any changes to this default direction (eg Z-positive) are done using m_pDefinition->vRotation
	MathsStorage::Float4A16 angles(m_pDefinition->fConeInner + random.FRand(-m_pDefinition->fConeOuter, m_pDefinition->fConeOuter), 
									 random.FRand(Maths::Pi2), 0.0f, 0.0f);
	XMVECTOR vAngles = XMLoadFloat4A16((XMFLOAT4A16*)&angles);
	XMVECTOR qRotate = XMQuaternionRotationRollPitchYawFromVector(vAngles);

	// get rotation from definition
	XMVECTOR qDefinitionRotation = XMLoadFloat4A16((XMFLOAT4A16*)&m_pDefinition->vRotation);
	qRotate = XMQuaternionMultiply(qRotate, qDefinitionRotation);

	// get cone times emitter rotation if in local space
	if(m_pDefinition->eOrientation == E_EmitterOrientation_Local)
		qRotate = XMQuaternionMultiply(qRotate, qLocal);
	
	MathsStorage::Float4A16 velocity(0.0f, m_pDefinition->fSpeed + random.FRand(-m_pDefinition->fSpeedRandom, m_pDefinition->fSpeedRandom), 0.0f, 0.0f);
	XMVECTOR vVelocity = XMLoadFloat4A16((XMFLOAT4A16*)&velocity);
	
	// get rotation from parent
	if(m_pParentTransformLS && m_pDefinition->eSpace == E_EmitterSpace_World && m_pDefinition->eOrientation == E_EmitterOrientation_Local)
	{
		XMVECTOR qParentRotation = XMLoadFloat4A16((XMFLOAT4A16*)&m_pParentTransformLS->GetRotation());
		qRotate = XMQuaternionMultiply(qRotate, qParentRotation);
	}

	XMVECTOR vVelocityEmitter = XMLoadFloat4A16((XMFLOAT4A16*)&m_vVelocity);
	XMVECTOR vSpeedInherited = XMVectorReplicate(m_pDefinition->fSpeedInherited + random.FRand(-m_pDefinition->fSpeedInheritedRandom, m_pDefinition->fSpeedInheritedRandom));

	XMMATRIX mRotate = XMMatrixRotationQuaternion(qRotate);

	// transform velocity by rotation and add inherited velocity
	return XMVectorAdd(XMVector3TransformNormal(vVelocity, mRotate), XMVectorMultiply(vVelocityEmitter, vSpeedInherited));
}

FORCE_INLINE XMVECTOR C_Emitter::GetSpawnCollisionPlanePosition() const
{
	// if using full collisions set the collision plane to be something out of the way, as the application will update the collision planes
	if(m_pDefinition->eCollision == E_Collision_Full)
		return XMVectorSet(0.0f, -100000.0f, 0.0f, 0.0f);
	else
		return XMLoadFloat4A16((XMFLOAT4A16*)&m_vCollisionPlanePosition);
}

FORCE_INLINE XMVECTOR C_Emitter::GetSpawnCollisionPlaneNormal() const
{
	if(m_pDefinition->eCollision == E_Collision_Full)
		return XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	else
		return XMLoadFloat4A16((XMFLOAT4A16*)&m_vCollisionPlaneNormal);
}
#else

FORCE_INLINE Maths::HVec3 C_Emitter::GetSpawnPosition(C_Random& random) const
{
	Maths::HVec3 vPosition(Maths::UNDEFINED);
	Maths::HVec3 vPositionCentre(GetSpawnPositionPoint());

	switch(m_pDefinition->eDistribution)
	{
	case E_EmitterDistribution_Line:
	case E_EmitterDistribution_Point:
	
		vPosition = vPositionCentre;
		break;
	case E_EmitterDistribution_Box:
		vPosition = GetSpawnPositionBox(random);
		break;
	case E_EmitterDistribution_Spline:
	case E_EmitterDistribution_Sphere:
	case E_EmitterDistribution_Capsule:
		vPosition = GetSpawnPositionSphere(random);
		break;
	case E_EmitterDistribution_Mesh:
		if (m_pSpawnMesh)
		{
			m_CreateNode = m_pSpawnMesh->GetRandomNode(random);
			vPosition = Maths::Load<Maths::HVec3>(&m_CreateNode.GetPosition());
		}
		else
			vPosition = Maths::Constants<Maths::HVec3>::Zero();		
		break;
	default:
		vPosition = Maths::HVec3(0.0f, 0.0f, 0.0f);
	}

	Maths::C_QuatTrans transform(m_Transform);
		
	// if using external transformation for emitter position transform here
	if(m_pParentTransformLS && m_pDefinition->eSpace == E_EmitterSpace_World)
	{
		Maths::C_QuatTrans parentTransform(*m_pParentTransformLS);
		transform = transform * parentTransform;
	}

	vPosition = vPosition * transform;
	vPositionCentre = vPositionCentre * transform;

	if(m_pDefinition->eDistribution == E_EmitterDistribution_Spline || m_pDefinition->eDistribution == E_EmitterDistribution_Line || m_pDefinition->eDistribution == E_EmitterDistribution_Capsule || m_pDefinition->fRateDistance != 0.0f)
	{
#ifndef PLATFORM_SPU
		if(m_pDefinition->eDistribution == E_EmitterDistribution_Spline)
		{
			Maths::HVec3 vSplinePosition = m_pSpline->GetCurve()->Evaluate(m_fInterpolate);
			vPosition = vPosition + vSplinePosition;
		}
		else
#endif
		{
			Maths::HVec3 vPositionPrevious(Maths::Load<Maths::HVec3>(&m_vPositionPreviousEmit));
			vPosition += (vPositionPrevious - vPositionCentre) * m_fInterpolate;
		}
		m_fInterpolate += m_fInterpolateDelta;
	}

	return vPosition;
}

FORCE_INLINE Maths::HF32 C_Emitter::GetSpawnSize(C_Random& random) const
{
	return m_pDefinition->fSize + random.FRand(-m_pDefinition->fSizeRandom, m_pDefinition->fSizeRandom);
}

FORCE_INLINE Maths::HVec4 C_Emitter::GetSpawnLighting() const
{
	Maths::HVec4 vLighting(Maths::Load<Maths::HVec4>(&m_vLighting));
	return vLighting;
}

FORCE_INLINE Maths::HVec4 C_Emitter::GetSpawnColour(C_Random& random) const
{
	switch(m_pDefinition->eColourMode)
	{
	case E_EmitterColourMode_SingleColour:
		return GetSpawnColourSingleColour();
	case E_EmitterColourMode_LerpColour:
		return GetSpawnColourLerpColour(random);
	case E_EmitterColourMode_LerpChannels:
		return GetSpawnColourLerpChannels(random);
	default:
		return Maths::HVec4();
	}
}

FORCE_INLINE Maths::HVec4 C_Emitter::GetSpawnColourSingleColour() const
{
	Maths::HVec4 vColour(Maths::Load<Maths::HVec4>(&m_pDefinition->vColour1));
	return vColour;
}

FORCE_INLINE Maths::HVec4 C_Emitter::GetSpawnColourLerpColour(C_Random& random) const
{
	Maths::HVec4 vColour1(Maths::Load<Maths::HVec4>(&m_pDefinition->vColour1));
	Maths::HVec4 vColour2(Maths::Load<Maths::HVec4>(&m_pDefinition->vColour2));
	Maths::HVec4 vColour = vColour1 + (vColour2 - vColour1) * random.FRand(1.0f);
	return vColour;
}

FORCE_INLINE Maths::HVec4 C_Emitter::GetSpawnColourLerpChannels(C_Random& random) const
{
	Maths::HVec4 vColour1(Maths::Load<Maths::HVec4>(&m_pDefinition->vColour1));
	Maths::HVec4 vColour2(Maths::Load<Maths::HVec4>(&m_pDefinition->vColour2));
	Maths::HVec4 vRandom(random.FRand(1.0f), random.FRand(1.0f), random.FRand(1.0f), random.FRand(1.0f));
	Maths::HVec4 vColourDiff(vColour2 - vColour1);
	Maths::HVec4 vColour = vColour1 + vColourDiff * vRandom;
	return vColour;
}

FORCE_INLINE Maths::HVec4 C_Emitter::GetSpawnForceSizeLife(C_Random& random) const
{
	Maths::HF32 vForce = GetSpawnForce(random); 
	Maths::HF32 vLife = GetSpawnLifeVariance(random);
	Maths::HF32 vSize = GetSpawnSize(random);
	return Maths::HVec4(vForce, 0.0f, vSize, vLife);
}

FORCE_INLINE Maths::HF32 C_Emitter::GetSpawnLifeVariance(C_Random& random) const
{
	Maths::HF32 vLife = (m_pDefinition->fLife + random.FRand(-m_pDefinition->fLifeRandom, m_pDefinition->fLifeRandom)) * m_fLifeScale;
	return vLife.Reciprocal();
}

FORCE_INLINE Maths::C_Quaternion C_Emitter::GetSpawnOrientation(C_Random& random) const
{
	F32 roll = m_pDefinition->vOrientation.x + random.FRand(-m_pDefinition->vOrientationRandom.x, m_pDefinition->vOrientationRandom.x);
	F32 pitch = m_pDefinition->vOrientation.y + random.FRand(-m_pDefinition->vOrientationRandom.y, m_pDefinition->vOrientationRandom.y);
	F32 yaw = m_pDefinition->vOrientation.z + random.FRand(-m_pDefinition->vOrientationRandom.z, m_pDefinition->vOrientationRandom.z);

	Maths::HVec4 HalfAngles(roll * 0.5f, pitch * 0.5f, yaw * 0.5f, 0.0f);
	Maths::HVec4 sinAngles(Maths::Sin(HalfAngles));
	Maths::HVec4 cosAngles(Maths::Cos(HalfAngles));

	Maths::HVec4 Sign(1.0f, -1.0f, -1.0f, 1.0f);
	Maths::HVec4 P0(sinAngles.X(), cosAngles.X(), cosAngles.X(), cosAngles.X());
	Maths::HVec4 Y0(cosAngles.Y(), sinAngles.Y(), cosAngles.Y(), cosAngles.Y());
	Maths::HVec4 R0(cosAngles.Z(), cosAngles.Z(), sinAngles.Z(), cosAngles.Z());

	Maths::HVec4 P1(cosAngles.X(), sinAngles.X(), sinAngles.X(), sinAngles.X());
	Maths::HVec4 Y1(sinAngles.Y(), cosAngles.Y(), sinAngles.Y(), sinAngles.Y());
	Maths::HVec4 R1(sinAngles.Z(), sinAngles.Z(), cosAngles.Z(), sinAngles.Z());
   
	Maths::HVec4 Q1 = P1 * Sign * Y1;
    Maths::HVec4 Q0 = P0 * Y0 * R0;
    Maths::HVec4 Q = Q1 * R1 + Q0;
	
	Maths::C_Quaternion qQuaternion(Q);
	qQuaternion = !qQuaternion;
		
	// always use local emitter orientation
	if(m_pDefinition->eDistribution == E_EmitterDistribution_Mesh)
	{
		Maths::C_Quaternion qLocalRotation(Maths::Load<Maths::C_Quaternion>(&m_CreateNode.GetRotation()));
		qQuaternion = qLocalRotation * qQuaternion;
	}
	else
	{
		Maths::C_Quaternion qLocalRotation(Maths::Load<Maths::C_Quaternion>(&m_Transform.GetRotation()));
		qQuaternion = qLocalRotation * qQuaternion;
	}

	// transform orientation by global transform if in world space
	if(m_pParentTransformLS && m_pDefinition->eSpace == E_EmitterSpace_World && m_pDefinition->eOrientation == E_EmitterOrientation_Local)
	{
		Maths::C_Quaternion qGlobalRotation(Maths::Load<Maths::C_Quaternion>(&m_pParentTransformLS->GetRotation()));
		qQuaternion = qGlobalRotation * qQuaternion;
	}

	return qQuaternion;
}

FORCE_INLINE Maths::HVec3 C_Emitter::GetSpawnAngularVelocity(C_Random& random) const
{
	return Maths::HVec3(	m_pDefinition->vAngularVelocity.x + random.FRand(-m_pDefinition->vAngularVelocityRandom.x, m_pDefinition->vAngularVelocityRandom.x),
						m_pDefinition->vAngularVelocity.y + random.FRand(-m_pDefinition->vAngularVelocityRandom.y, m_pDefinition->vAngularVelocityRandom.y),
						m_pDefinition->vAngularVelocity.z + random.FRand(-m_pDefinition->vAngularVelocityRandom.z, m_pDefinition->vAngularVelocityRandom.z));
}

FORCE_INLINE Maths::HVec3 C_Emitter::GetSpawnPositionPoint() const
{
	Maths::HVec3 vPosition(Maths::Load<Maths::HVec3>(&m_pDefinition->vPosition));
	return vPosition;
}

FORCE_INLINE Maths::HVec3 C_Emitter::GetSpawnPositionBox(C_Random& random) const
{
	assert(m_pDefinition->eDistribution == E_EmitterDistribution_Box);
	Maths::HVec3 vRandom(random.FRand(-0.5f, 0.5f), random.FRand(-0.5f, 0.5f), random.FRand(-0.5f, 0.5f));
	Maths::HVec3 vSize(Maths::Load<Maths::HVec3>(&m_pDefinition->vSize));
	vSize = vSize * vRandom;
	vSize = vSize + Maths::Load<Maths::HVec3>(&m_pDefinition->vPosition);
	return vSize;
}

FORCE_INLINE Maths::HVec3 C_Emitter::GetSpawnPositionSphere(C_Random& random) const
{
	Maths::C_MatrixRotateRPH rph(random.FRand(Maths::Pi2), random.FRand(Maths::Pi2), 0.0f);
	Maths::HMat33 mMatrix(rph);
	Maths::HVec3 vPosition(0.0f, 0.5f, 0.0f);
	vPosition = vPosition * mMatrix;
	Maths::HVec3 vSize(random.FRand(m_pDefinition->vSize.x), random.FRand(m_pDefinition->vSize.y), random.FRand(m_pDefinition->vSize.z));
	// scale position by size
	vPosition = vPosition * vSize;
	vPosition = vPosition + Maths::Load<Maths::HVec3>(&m_pDefinition->vPosition);
	
	return vPosition;
}

FORCE_INLINE Maths::HVec3 C_Emitter::GetSpawnVelocity(C_Random& random) const
{
	// get a rotation matrix for the cone
	Maths::HMat33 mRotation(Maths::C_MatrixRotateRPH(0.0f, m_pDefinition->fConeInner + random.FRand(-m_pDefinition->fConeOuter, m_pDefinition->fConeOuter), random.FRand(Maths::Pi2)));
	
	// get local emitter orientation
	Maths::C_Quaternion qLocalRotation(Maths::UNDEFINED);
	if(m_pDefinition->eDistribution == E_EmitterDistribution_Mesh)
		qLocalRotation = Maths::Load<Maths::C_Quaternion>(&m_CreateNode.GetRotation());
	else
		qLocalRotation = Maths::Load<Maths::C_Quaternion>(&m_Transform.GetRotation());

	// take into account spawn event's unique orientation
	Maths::C_Quaternion qSpawnEventTrans(Maths::Load<Maths::C_Quaternion>(&m_pDefinition->vRotation));
	Maths::HMat33 mSpawnEventTrans(qSpawnEventTrans);

	mRotation = mRotation * mSpawnEventTrans;
	
	Maths::HMat33 mLocalRotation(qLocalRotation);
	if(m_pDefinition->eOrientation == E_EmitterOrientation_Local)
		mRotation = mRotation * mLocalRotation;

	Maths::HF32 vScale = m_pDefinition->fSpeed + random.FRand(-m_pDefinition->fSpeedRandom, m_pDefinition->fSpeedRandom);
	Maths::HVec3 vVelocity(0.0f, vScale, 0.0f);
	
	if(m_pParentTransformLS && m_pDefinition->eSpace == E_EmitterSpace_World && m_pDefinition->eOrientation == E_EmitterOrientation_Local)
	{
		Maths::C_Quaternion qGlobalRotation(Maths::Load<Maths::C_Quaternion>(&m_pParentTransformLS->GetRotation()));
		Maths::HMat33 mGlobalRotation(qGlobalRotation);
		mRotation = mRotation * mGlobalRotation;
	}

	Maths::HVec3 vVelocityEmitter(Maths::Load<Maths::HVec3>(&m_vVelocity));
	Maths::HF32 vSpeedInherited(m_pDefinition->fSpeedInherited + random.FRand(-m_pDefinition->fSpeedInheritedRandom, m_pDefinition->fSpeedInheritedRandom));
	
	// transform velocity by rotation and add inherited velocity
	vVelocity = vVelocity * mRotation;
	vVelocity = vVelocity + (vVelocityEmitter * vSpeedInherited);
	return vVelocity;
}

FORCE_INLINE Maths::HF32 C_Emitter::GetSpawnForce(C_Random& random) const
{
	Maths::HF32 vForce(m_pDefinition->fForce + random.FRand(-m_pDefinition->fForceRandom, m_pDefinition->fForceRandom));
	return vForce;
}

FORCE_INLINE Maths::HVec3 C_Emitter::GetSpawnCollisionPlanePosition() const
{
	if(m_pDefinition->eCollision == E_Collision_Full)
		return Maths::HVec3(0.0f, -100000.0f, 0.0f);
	else
		return Maths::Load<Maths::HVec3>(&m_vCollisionPlanePosition);
}

FORCE_INLINE Maths::HVec3 C_Emitter::GetSpawnCollisionPlaneNormal() const
{
	if(m_pDefinition->eCollision == E_Collision_Full)
		return Maths::HVec3(0.0f, 1.0f, 0.0f);
	else
		return Maths::Load<Maths::HVec3>(&m_vCollisionPlaneNormal);
}
#endif

FORCE_INLINE F32 C_Emitter::GetSpawnRotation(C_Random& random) const
{
	return m_pDefinition->vOrientation.x + random.FRand(-m_pDefinition->vOrientationRandom.x, m_pDefinition->vOrientationRandom.x);
}

FORCE_INLINE F32 C_Emitter::GetSpawnRotationRate(C_Random& random) const
{
	return m_pDefinition->vAngularVelocity.x + random.FRand(-m_pDefinition->vAngularVelocityRandom.x, m_pDefinition->vAngularVelocityRandom.x);
}

FORCE_INLINE U32 C_Emitter::GetSpawnUV(C_Random& random) const
{
	U32 uv = m_pDefinition->eUV == E_EmitterUV_Random ? random.Rand(E_EmitterUV_Random) : m_pDefinition->eUV;
	assert(uv < 4);
	return uv;
}

FORCE_INLINE const MathsStorage::Float4A16& C_Emitter::GetPositionPrevious() const
{
	return m_vPositionPrevious;
}

FORCE_INLINE const MathsStorage::Float4A16& C_Emitter::GetVelocity() const
{
	return m_vVelocity;
}

FORCE_INLINE const MathsStorage::Float4A16& C_Emitter::GetPosition() const
{
	return m_vPosition;
}

FORCE_INLINE Maths::HVec3 C_Emitter::GetWorldNormal() const
{
	Maths::C_QuatTrans transform(m_Transform);
	if(m_pParentTransformLS)
	{
		Maths::C_QuatTrans globalTransform(*m_pParentTransformLS);
		transform = transform * globalTransform;
	}

	return Maths::Constants<Maths::HVec3>::UnitAxisPositiveY() * transform.Matrix();
}

}
}

#endif // RENDER_DYNAMICEFFECT_PARTICLE_EMITTER_H
