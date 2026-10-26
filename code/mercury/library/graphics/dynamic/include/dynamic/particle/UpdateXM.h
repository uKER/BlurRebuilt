#ifndef RENDER_DYNAMICEFFECT_PARTICLE_UPDATEXM_H
#define RENDER_DYNAMICEFFECT_PARTICLE_UPDATEXM_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Particle/Particle.h"
#include "Dynamic/Particle/Force.h"
#include "Dynamic/Block.h"


namespace DynamicEffect
{
namespace Particle
{

XMGLOBALCONST XMVECTOR  g_Epsilon             = {1.192092896e-7f, 1.192092896e-7f, 1.192092896e-7f, 1.192092896e-7f};
XMGLOBALCONST XMVECTOR  g_Half             = {0.5f, 0.5f, 0.5f, 0.5f};

class BlockUpdate
{
public:
	/*FORCE_INLINE void GetForces(const Forces& allForces, ParticleBlock* pBlock)
	{
		forces.Clear();
		
		U32 nIntersects = 0;
		XMVECTOR vBlockCentre = XMLoadFloat4A16((XMFLOAT4A16*)&pBlock->m_vCentre);
		XMVECTOR vBlockRadius = XMVector3LengthEst(XMLoadFloat4A16((XMFLOAT4A16*)&pBlock->m_vSize));

		for(Forces::ConstIterator pIterator = allForces.Begin(); pIterator != allForces.End(); pIterator++)
		{
			const C_Force* pForce = (*pIterator);
			XMVECTOR vForceCentre = XMLoadFloat4A16((XMFLOAT4A16*)&pForce->m_vPosition);
			XMVECTOR vForceRadius = XMLoadFloat4A16((XMFLOAT4A16*)&pForce->m_vRadius);

			XMVECTOR vBlockToForce = XMVectorSubtract(vBlockCentre, vForceCentre);
			XMVECTOR vLength = XMVector3LengthEst(vBlockToForce);

			vLength = XMVectorSubtract(vLength, vBlockRadius);
			vLength = XMVectorSubtract(vLength, vForceRadius);

			if(XMVector4Less(vLength, XMVectorZero()))
			{
				nIntersects++;
				forces.PushBack(pForce);
			}
		}
	}

	ConstForces		forces;*/
};


template<typename Particle>
class ParticleUpdateBase2D
{
public:
	FORCE_INLINE void LoadBase(XMVECTOR vFrameTime)
	{
		XMVECTOR vPositionVelocity = XMLoadFloat4A16((XMFLOAT4A16*)&m_pParticle->vPosition);
		m_vPosition = vPositionVelocity;
		m_vVelocity = vPositionVelocity;

		m_vLifeSizeLife = XMLoadFloat4A16((XMFLOAT4A16*)&m_pParticle->vLifeSizeLife);
		m_vLife = XMVectorSplatX(m_vLifeSizeLife);
				
		m_iFlags = m_pParticle->iFlags;
		m_vTime = vFrameTime;
	}

	FORCE_INLINE void StoreBase()
	{
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vPosition, XMVectorSelect(m_vPosition, m_vVelocity, XMVectorSelectControl(0, 0, 1, 1)));
		m_vLifeSizeLife = XMVectorSelect(m_vLife, m_vLifeSizeLife, XMVectorSelectControl(0, 1, 1, 1));
		XMStoreFloat3A16((XMFLOAT3A16*)&m_pParticle->vLifeSizeLife, m_vLifeSizeLife);
		m_pParticle->iFlags = m_iFlags;
		// update bounds
		m_vSizeMax = XMVectorMax(m_vSizeMax, XMVectorSplatY(m_vLifeSizeLife));
		m_vMax = XMVectorMax(m_vMax, m_vPosition);
		m_vMin = XMVectorMin(m_vMin, m_vPosition);
	}

	FORCE_INLINE void SetParticle(Particle* pParticle)
	{
		m_pParticle = pParticle;
	}

	FORCE_INLINE ParticleUpdateBase2D() 
	{
		m_vSizeMax = XMVectorZero();
		m_vMin = XMVectorReplicate(10000.0f);
		m_vMax = XMVectorReplicate(-10000.0f);
	}

	// Update methods
	FORCE_INLINE void UpdateVelocity(XMVECTOR vResistance)
	{
		m_vVelocity = XMVectorMultiply(m_vVelocity, vResistance);
	}
	FORCE_INLINE void UpdateVelocity(XMVECTOR vResistance, XMVECTOR vAcceleration)
	{
		m_vVelocity = XMVectorAdd(m_vVelocity, vAcceleration);
		m_vVelocity = XMVectorMultiply(m_vVelocity, vResistance);
	}
	FORCE_INLINE void UpdateVelocity(XMVECTOR vResistance, XMVECTOR vAcceleration, XMVECTOR vForce)
	{
		m_vVelocity = XMVectorMultiplyAdd(vForce, XMVectorSplatX(m_vForceSizeLife), m_vVelocity);
		m_vVelocity = XMVectorAdd(m_vVelocity, vAcceleration);
		m_vVelocity = XMVectorMultiply(m_vVelocity, vResistance);
	}
	FORCE_INLINE void UpdatePosition()
	{
		m_vPosition = XMVectorAdd(XMVectorMultiply(XMVectorRotateLeft(m_vVelocity, 2), m_vTime), m_vPosition);
	}
	FORCE_INLINE void UpdateLife()
	{
		XMVECTOR vAdd = XMVectorMultiply(m_vTime, XMVectorSplatZ(m_vLifeSizeLife));
		m_vLife = XMVectorAdd(m_vLife, vAdd);
	}
	
	FORCE_INLINE void CreatePositionLife(XMVECTOR vPositionLife)
	{
		vPositionLife = XMVectorSelect(vPositionLife, XMVectorZero(), XMVectorSelectControl(0, 0, 0, 1));
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vPosition, vPositionLife);
	}

	FORCE_INLINE void CreateForceSizeLife(XMVECTOR vLifeSizeLife)
	{
		// store this so collision plane can get at it
		m_vvLifeSizeLife = vLifeSizeLife;

		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vLifeSizeLife, vLifeSizeLife);
	}

	FORCE_INLINE void CreateColour(XMVECTOR vLighting, XMVECTOR vColour)
	{
		XMVECTOR vColourHDR = XMVectorMultiply(vLighting, vColour);
		
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vColour, vColourHDR);
	}

	// other
	FORCE_INLINE S32 IsDead()
	{
		return XMVector4Greater(m_vLife, XMVectorSplatOne());
	}

	// bounds stuff
	FORCE_INLINE void StoreBounds(MathsStorage::Float4A16* pStore)
	{
		m_vMax = XMVectorAdd(m_vMax, m_vSizeMax);
		m_vMin = XMVectorSubtract(m_vMin, m_vSizeMax);
		XMVECTOR vOne = XMVectorSplatOne();
		XMVECTOR vCentre = XMVectorMultiply(XMVectorAdd(m_vMax, m_vMin), g_Half);
		XMVECTOR vSize = XMVectorSubtract(m_vMax, vCentre);

		vCentre = XMVectorSelect(vCentre, vOne, XMVectorSelectControl(0, 0, 0, 1));
		
		XMStoreFloat4A16((XMFLOAT4A16*)pStore, vCentre);
		XMStoreFloat3A16((XMFLOAT3A16*)(pStore+1), vSize);
	}

	Particle*	m_pParticle;

	XMVECTOR	m_vPosition;
	XMVECTOR	m_vLife;
	XMVECTOR	m_vVelocity;
	XMVECTOR	m_vLifeSizeLife;
	
	XMVECTOR	m_vTime;
	XMVECTOR	m_vSizeMax;
	XMVECTOR	m_vMin;
	XMVECTOR	m_vMax;
	
	U8			m_iFlags;
};

template<typename Particle>
class ParticleUpdate2D : public ParticleUpdateBase2D<Particle>
{
public:
	ParticleUpdate2D() : ParticleUpdateBase2D() {}
	
	FORCE_INLINE void Load(XMVECTOR vFrameTime) 
	{
		LoadBase(vFrameTime);
	}
	
	FORCE_INLINE void Store()
	{
		StoreBase();
	}	
};

template<typename Particle>
class ParticleUpdateBase
{
public:
	FORCE_INLINE void LoadBase(XMVECTOR vFrameTime)
	{
		m_vPosition = XMLoadFloat4A16((XMFLOAT4A16*)&m_pParticle->vPosition);
		m_vLife = XMVectorSplatW(m_vPosition);
		m_vVelocity = XMLoadFloat4A16((XMFLOAT4A16*)&m_pParticle->vVelocity);
		m_vForceSizeLife = XMLoadFloat4A16((XMFLOAT4A16*)&m_pParticle->vForceSizeLife);
		m_iFlags = m_pParticle->iFlags;
		m_vTime = vFrameTime;
	}

	FORCE_INLINE void StoreBase()
	{
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vPosition, XMVectorSelect(m_vPosition, m_vLife, XMVectorSelectControl(0, 0, 0, 1)));
		XMStoreFloat3A16((XMFLOAT3A16*)&m_pParticle->vVelocity, m_vVelocity);
		m_pParticle->iFlags = m_iFlags;
		// update bounds
		m_vSizeMax = XMVectorMax(m_vSizeMax, XMVectorSplatZ(m_vForceSizeLife));
		m_vMax = XMVectorMax(m_vMax, m_vPosition);
		m_vMin = XMVectorMin(m_vMin, m_vPosition);
	}

	FORCE_INLINE void SetParticle(Particle* pParticle)
	{
		m_pParticle = pParticle;
	}

	FORCE_INLINE ParticleUpdateBase() 
	{
		m_vSizeMax = XMVectorZero();
		m_vMin = XMVectorReplicate(10000.0f);
		m_vMax = XMVectorReplicate(-10000.0f);
	}

	// Update methods
	FORCE_INLINE void UpdateVelocity(XMVECTOR vResistance)
	{
		m_vVelocity = XMVectorMultiply(m_vVelocity, vResistance);
	}
	FORCE_INLINE void UpdateVelocity(XMVECTOR vResistance, XMVECTOR vAcceleration)
	{
		m_vVelocity = XMVectorAdd(m_vVelocity, vAcceleration);
		m_vVelocity = XMVectorMultiply(m_vVelocity, vResistance);
	}
	FORCE_INLINE void UpdateVelocity(XMVECTOR vResistance, XMVECTOR vAcceleration, XMVECTOR vForce)
	{
		m_vVelocity = XMVectorMultiplyAdd(vForce, XMVectorSplatX(m_vForceSizeLife), m_vVelocity);
		m_vVelocity = XMVectorAdd(m_vVelocity, vAcceleration);
		m_vVelocity = XMVectorMultiply(m_vVelocity, vResistance);
	}
	FORCE_INLINE void UpdatePosition()
	{
		m_vPosition = XMVectorAdd(XMVectorMultiply(m_vVelocity, m_vTime), m_vPosition);
	}
	FORCE_INLINE void UpdateLife()
	{
		XMVECTOR vAdd = XMVectorMultiply(m_vTime, XMVectorSplatW(m_vForceSizeLife));
		m_vLife = XMVectorAdd(m_vLife, vAdd);
	}
	
	/*FORCE_INLINE void UpdateForces(ConstForces& currentForces)
	{
		for(S32 i = 0; i < currentForces.Size(); ++i)
		{
			// get force properties
			const C_Force* pForce = currentForces[i];
			XMVECTOR vCentre = XMLoadFloat4A16((XMFLOAT4A16*)&pForce->m_vPosition);
			XMVECTOR vRadiusRcp = XMLoadFloat4A16((XMFLOAT4A16*)&pForce->m_vRadiusRcp);
			XMVECTOR vStrength = XMLoadFloat4A16((XMFLOAT4A16*)&pForce->m_vStrength);

			vStrength = XMVectorMultiply(vStrength, m_vTime);

			// calculate a scaling factor and direction vector based on displacement from force origin
			XMVECTOR vDisplacement = XMVectorSubtract(vCentre, m_vPosition);
			XMVECTOR vLength = XMVector3LengthEst(vDisplacement);
			
			XMVECTOR vScale = XMVectorSubtract(XMVectorSplatOne(), XMVectorMultiply(vLength, vRadiusRcp));

			XMVECTOR vDirection;
			if(pForce->m_eType == E_ForceType_Point)
				vDirection = XMVectorMultiply(vDisplacement, XMVectorReciprocalEst(vLength));
			else
				vDirection = XMLoadFloat4A16((XMFLOAT4A16*)&pForce->m_vVelocity);
			
			// clamp and scale
			vScale = XMVectorMax(vScale, XMVectorZero());
			vScale = XMVectorMultiply(vScale, vStrength);

			// add to velocity
			m_vVelocity = XMVectorAdd(m_vVelocity, XMVectorMultiply(vDirection, vScale));
		}
	}*/

	FORCE_INLINE void CreatePositionLife(XMVECTOR vPositionLife)
	{
		vPositionLife = XMVectorSelect(vPositionLife, XMVectorZero(), XMVectorSelectControl(0, 0, 0, 1));
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vPosition, vPositionLife);
	}

	FORCE_INLINE void CreatePosition(const MathsStorage::Float4A16& vPosition)
	{
		m_pParticles->vPosition = vPosition;
	}

	FORCE_INLINE void CreateForceSizeLife(XMVECTOR vForceSizeLife)
	{
		// store this so collision plane can get at it
		m_vForceSizeLife = vForceSizeLife;

		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vForceSizeLife, vForceSizeLife);
	}

	FORCE_INLINE void CreateForceSizeLife(XMVECTOR vForceSizeLife, const F32* size)
	{
		XMVECTOR vSize = XMVectorReplicatePtr(size);
		// store this so collision plane can get at it
		m_vForceSizeLife = XMVectorSelect(
			vForceSizeLife, 
			XMVectorMultiply(vForceSizeLife, vSize), 
			XMVectorSelectControl(0, 0, 1, 0));

		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vForceSizeLife, m_vForceSizeLife);
	} 

	FORCE_INLINE void CreateForceSizeLife(XMVECTOR vForceSizeLife, XMVECTOR vSize)
	{
		  // store this so collision plane can get at it
		  m_vForceSizeLife = XMVectorSelect(
			  vForceSizeLife, 
			  XMVectorMultiply(vForceSizeLife, vSize), 
			  XMVectorSelectControl(0, 0, 1, 0));

		  XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vForceSizeLife, m_vForceSizeLife);
	} 

	FORCE_INLINE void CreateColour(XMVECTOR vLighting, XMVECTOR vColour)
	{
		XMVECTOR vColourHDR = XMVectorMultiply(vLighting, vColour);
		
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vColour, vColourHDR);
	}

	FORCE_INLINE void CreateVelocity(XMVECTOR vVelocity)
	{
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vVelocity, vVelocity);
	}

	FORCE_INLINE void CreateVelocity(const MathsStorage::Float4A16& vVelocity)
	{
		m_pParticles->vVelocity = vVelocity;
	}

	// other
	FORCE_INLINE S32 IsDead()
	{
		return XMVector4Greater(m_vLife, XMVectorSplatOne());
	}

	// bounds stuff
	FORCE_INLINE void StoreBounds(MathsStorage::Float4A16* pStore)
	{
		m_vMax = XMVectorAdd(m_vMax, m_vSizeMax);
		m_vMin = XMVectorSubtract(m_vMin, m_vSizeMax);
		XMVECTOR vOne = XMVectorSplatOne();
		XMVECTOR vCentre = XMVectorMultiply(XMVectorAdd(m_vMax, m_vMin), g_Half);
		XMVECTOR vSize = XMVectorSubtract(m_vMax, vCentre);

		vCentre = XMVectorSelect(vCentre, vOne, XMVectorSelectControl(0, 0, 0, 1));
		
		XMStoreFloat4A16((XMFLOAT4A16*)pStore, vCentre);
		XMStoreFloat3A16((XMFLOAT3A16*)(pStore+1), vSize);
	}

	Particle*	m_pParticle;

	XMVECTOR	m_vPosition;
	XMVECTOR	m_vLife;
	XMVECTOR	m_vVelocity;
	XMVECTOR	m_vForceSizeLife;
	
	XMVECTOR	m_vTime;
	XMVECTOR	m_vSizeMax;
	XMVECTOR	m_vMin;
	XMVECTOR	m_vMax;
	
	U8			m_iFlags;
};

template<typename Particle>
class ParticleUpdate : public ParticleUpdateBase<Particle>
{
public:
	ParticleUpdate() : ParticleUpdateBase() {}
	
	FORCE_INLINE void Load(XMVECTOR vFrameTime) 
	{
		LoadBase(vFrameTime);
	}
	
	FORCE_INLINE void Store()
	{
		StoreBase();
	}	
};

template<>
class ParticleUpdate<C_Particle3D_P> : public ParticleUpdateBase<C_Particle3D_P>
{
public:
	ParticleUpdate() : ParticleUpdateBase() {}

	FORCE_INLINE void Load(XMVECTOR vFrameTime) 
	{
		LoadBase(vFrameTime);
		m_vPosition2 = XMLoadFloat3((XMFLOAT3*)&m_pParticle->vPosition2);
	}
	
	FORCE_INLINE void Store()
	{
		StoreBase();
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vPosition2, m_vPosition2);
	}

	FORCE_INLINE void CreatePositionLife(XMVECTOR vPositionLife, XMVECTOR vPositionCurve)
	{
		vPositionLife = XMVectorSelect(vPositionLife, XMVectorZero(), XMVectorSelectControl(0, 0, 0, 1));
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vPosition2, vPositionLife);

		vPositionLife = XMVectorAdd(vPositionLife, XMVectorSelect(vPositionCurve, XMVectorZero(), XMVectorSelectControl(0, 0, 0, 1)));
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vPosition, vPositionLife);
	}

	FORCE_INLINE void UpdatePosition(XMVECTOR vPosition1)
	{
		// store the particle local position in position 2
		m_vPosition2 = XMVectorAdd(XMVectorMultiply(m_vVelocity, m_vTime), m_vPosition2);
		// store the position plus procedural position in position 1
		m_vPosition = XMVectorAdd(m_vPosition2, vPosition1);
	}

	XMVECTOR	m_vPosition2;
};

template<>
class ParticleUpdate<C_Particle3D_R1D_P> : public ParticleUpdateBase<C_Particle3D_R1D_P>
{
public:
	ParticleUpdate() : ParticleUpdateBase() {}

	FORCE_INLINE void Load(XMVECTOR vFrameTime) 
	{
		LoadBase(vFrameTime);
		m_vPosition2 = XMLoadFloat3((XMFLOAT3*)&m_pParticle->vPosition2);
		m_vRotation = XMLoadFloat4A16((XMFLOAT4A16*)&m_pParticle->vRotation);
		m_vRotationRate = XMVectorSplatY(m_vRotation);
		m_vRotation = XMVectorSplatX(m_vRotation);
	}
	
	FORCE_INLINE void Store()
	{
		StoreBase();
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vPosition2, m_vPosition2);
		m_vRotation = XMVectorMergeXY(m_vRotation, m_vRotationRate);
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vRotation, m_vRotation);
	}

	FORCE_INLINE void CreatePositionLife(XMVECTOR vPositionLife, XMVECTOR vPositionCurve)
	{
		vPositionLife = XMVectorSelect(vPositionLife, XMVectorZero(), XMVectorSelectControl(0, 0, 0, 1));
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vPosition2, vPositionLife);

		vPositionLife = XMVectorAdd(vPositionLife, XMVectorSelect(vPositionCurve, XMVectorZero(), XMVectorSelectControl(0, 0, 0, 1)));
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vPosition, vPositionLife);
	}

	FORCE_INLINE void CreateRotation(F32 fRotation, F32 fRotationRate)
	{
		m_pParticle->vRotation.x = fRotation;
		m_pParticle->vRotation.y = fRotationRate;
	}

	FORCE_INLINE void UpdateRotation()
	{
		XMVECTOR vOne = XMVectorSplatOne();
		XMVECTOR vMinusOne = XMVectorNegate(vOne);
		XMVECTOR vTwo = XMVectorAdd(vOne, vOne);

		XMVECTOR vPi = XMVectorSplatX(g_XMPiConstants0);
		XMVECTOR vNegativePi = XMVectorNegate(vPi);
		XMVECTOR vTwoPi = XMVectorSplatY(g_XMPiConstants0);

		// keep rotation between (-1,1)
		XMVECTOR vRotationRateTime = XMVectorMultiply(m_vRotationRate, m_vTime);

		m_vRotation = XMVectorAdd(m_vRotation, vRotationRateTime);
		if(XMVector4Less(m_vRotation, vNegativePi))
			m_vRotation = XMVectorAdd(m_vRotation, vTwoPi);
		else if(XMVector4Greater(m_vRotation, vPi))
			m_vRotation = XMVectorSubtract(m_vRotation, vTwoPi);
	}

	FORCE_INLINE void UpdatePosition(XMVECTOR vPosition1)
	{
		// store the particle local position in position 2
		m_vPosition2 = XMVectorAdd(XMVectorMultiply(m_vVelocity, m_vTime), m_vPosition2);
		// store the position plus procedural position in position 1
		m_vPosition = XMVectorAdd(m_vPosition2, vPosition1);
	}

	XMVECTOR	m_vPosition2;
	XMVECTOR	m_vRotation;
	XMVECTOR	m_vRotationRate;
};

template<>
class ParticleUpdate<C_Particle3D_CP> : public ParticleUpdateBase<C_Particle3D_CP>
{
public:
	ParticleUpdate() : ParticleUpdateBase() {}

	FORCE_INLINE void Load(XMVECTOR vFrameTime) 
	{
		LoadBase(vFrameTime);
		m_vPlanePosition = XMLoadFloat3((XMFLOAT3*)&m_pParticle->vPlanePosition);
		m_vPlaneNormal = XMLoadHenDN3((XMHENDN3*)&m_pParticle->vPlaneNormal);
		m_iFlags &= ~PARTICLE_COLLISION;
		m_iFlags &= ~PARTICLE_COLLISION_FIRST;
	}
	
	FORCE_INLINE void Store()
	{
		StoreBase();
	}

	FORCE_INLINE void UpdatePosition()
	{
		m_vPosition = XMVectorAdd(m_vVelocityTime, m_vPosition);
	}

	FORCE_INLINE void UpdateVelocity(XMVECTOR vResistance, XMVECTOR vAcceleration)
	{
		m_vVelocity = XMVectorAdd(m_vVelocity, vAcceleration);
		m_vVelocity = XMVectorMultiply(m_vVelocity, vResistance);
		
		// calc velocity time here cause its used in collision code
		m_vVelocityTime = XMVectorMultiply(m_vVelocity, m_vTime);
	}

	FORCE_INLINE void UpdateVelocity(XMVECTOR vResistance, XMVECTOR vAcceleration, XMVECTOR vForce)
	{
		m_vVelocity = XMVectorMultiplyAdd(vForce, XMVectorSplatX(m_vForceSizeLife), m_vVelocity);
		m_vVelocity = XMVectorAdd(m_vVelocity, vAcceleration);
		m_vVelocity = XMVectorMultiply(m_vVelocity, vResistance);
		
		// calc velocity time here cause its used in collision code
		m_vVelocityTime = XMVectorMultiply(m_vVelocity, m_vTime);
	}

	FORCE_INLINE void UpdateCollisions(XMVECTOR vRestitution, XMVECTOR vFriction, const bool bDieOnImpact,  XMVECTOR vSleepSpeedSq, const MathsStorage::Float4A16* materials)
	{
		if(!(m_iFlags & PARTICLE_COLLISION_NONE))
		{
			XMVECTOR vDiff = XMVectorSubtract(m_vPosition, m_vPlanePosition);

			XMVECTOR vGroundDist = XMVector3Dot(vDiff, m_vPlaneNormal);
			XMVECTOR vTowards = XMVector3Dot(m_vVelocity, m_vPlaneNormal);
			
			if(XMVector4Less(vGroundDist, XMVectorZero()) && XMVector4Less(vTowards, XMVectorZero()))
			{
				XMVECTOR vMaterial = XMLoadFloat4A16((XMFLOAT4A16*)&materials[0]);
				XMVECTOR vSurfaceRestitution = XMVectorSplatX(vMaterial);
				XMVECTOR vSurfaceFriction = XMVectorSubtract(XMVectorSplatOne(), XMVectorMultiply(m_vTime, XMVectorSplatY(vMaterial)));
				
				XMVECTOR vDot = XMVector3Dot(m_vVelocityTime, m_vPlaneNormal);
				XMVECTOR vReciprocal = XMVectorReciprocal(vDot);

				vGroundDist = XMVectorSubtract(vGroundDist, g_Epsilon);
				m_vPosition = XMVectorSubtract(m_vPosition, XMVectorMultiply(vGroundDist, m_vPlaneNormal));
				
				XMVECTOR vNormalVelocity = XMVectorMultiply(m_vPlaneNormal, XMVector3Dot(m_vPlaneNormal, m_vVelocity));
				
				vRestitution = XMVectorMultiply(XMVectorAdd(vRestitution, vSurfaceRestitution), g_Half);
				vFriction = XMVectorMultiply(XMVectorAdd(vFriction, vSurfaceFriction), g_Half);
				
				m_vVelocity = XMVectorSubtract(m_vVelocity, XMVectorMultiply(vNormalVelocity, vRestitution));				
				m_vVelocity = XMVectorMultiply(m_vVelocity, vFriction);

				if(bDieOnImpact)
					m_vLife = XMVectorSplatOne();

				m_iFlags |= PARTICLE_COLLISION;
				if(!(m_iFlags & PARTICLE_COLLISION_ONCE))
					m_iFlags |= PARTICLE_COLLISION_FIRST;
				m_iFlags |= PARTICLE_COLLISION_ONCE;

				if(XMVector4Less(XMVector3LengthSq(m_vVelocity), vSleepSpeedSq))
					m_iFlags |= PARTICLE_COLLISION_SLEEP;
			}
		}
	}

	/*FORCE_INLINE void UpdateForces(ConstForces& currentForces)
	{
		ParticleUpdateBase::UpdateForces(currentForces);
	}*/

	FORCE_INLINE void CreateCollisionPlane(XMVECTOR vPlanePosition, XMVECTOR vPlaneNormal)
	{
		// add the size of particle to collision plane
		XMVECTOR vSize = XMVectorSplatZ(m_vForceSizeLife);
		XMStoreFloat3((XMFLOAT3*)&m_pParticle->vPlanePosition, XMVectorMultiplyAdd(vPlaneNormal, vSize, vPlanePosition));
		XMStoreHenDN3((XMHENDN3*)&m_pParticle->vPlaneNormal, vPlaneNormal);
	}

	FORCE_INLINE void CreateCollisionPlane(const MathsStorage::Float4A16& vPlanePosition,const MathsStorage::Float4A16& vPlaneNormal)
	{
		XMVECTOR vXmPlanePosition = XMLoadFloat4A16((XMFLOAT4A16*)&vPlanePosition);
		XMVECTOR vXmPlaneNormal = XMLoadFloat4A16((XMFLOAT4A16*)&vPlaneNormal);
		
		XMStoreFloat3((XMFLOAT3*)&m_pParticle->vPlanePosition, vXmPlanePosition);
		XMStoreHenDN3((XMHENDN3*)&m_pParticle->vPlaneNormal, vXmPlaneNormal);
	}

	XMVECTOR	m_vVelocityTime;
	XMVECTOR	m_vPlanePosition;
	XMVECTOR	m_vPlaneNormal;
};

template<>
class ParticleUpdate<C_Particle3D_R1D> : public ParticleUpdateBase<C_Particle3D_R1D>
{
public:
	ParticleUpdate() : ParticleUpdateBase() {}
	
	FORCE_INLINE void Load(XMVECTOR vFrameTime) 
	{
		LoadBase(vFrameTime);

		m_vRotation = XMLoadFloat4A16((XMFLOAT4A16*)&m_pParticle->vRotation);
		m_vRotationRate = XMVectorSplatY(m_vRotation);
		m_vRotation = XMVectorSplatX(m_vRotation);
	}

	FORCE_INLINE void Store()
	{
		StoreBase();

		m_vRotation = XMVectorMergeXY(m_vRotation, m_vRotationRate);
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vRotation, m_vRotation);
	}

	FORCE_INLINE void CreateRotation(F32 fRotation, F32 fRotationRate)
	{
		m_pParticle->vRotation.x = fRotation;
		m_pParticle->vRotation.y = fRotationRate;
	}

	FORCE_INLINE void UpdateRotation()
	{
		XMVECTOR vOne = XMVectorSplatOne();
		XMVECTOR vMinusOne = XMVectorNegate(vOne);
		XMVECTOR vTwo = XMVectorAdd(vOne, vOne);

		XMVECTOR vPi = XMVectorSplatX(g_XMPiConstants0);
		XMVECTOR vNegativePi = XMVectorNegate(vPi);
		XMVECTOR vTwoPi = XMVectorSplatY(g_XMPiConstants0);

		// keep rotation between (-1,1)
		XMVECTOR vRotationRateTime = XMVectorMultiply(m_vRotationRate, m_vTime);

		m_vRotation = XMVectorAdd(m_vRotation, vRotationRateTime);
		if(XMVector4Less(m_vRotation, vNegativePi))
			m_vRotation = XMVectorAdd(m_vRotation, vTwoPi);
		else if(XMVector4Greater(m_vRotation, vPi))
			m_vRotation = XMVectorSubtract(m_vRotation, vTwoPi);
	}

	XMVECTOR	m_vRotation;
	XMVECTOR	m_vRotationRate;
};

template<>
class ParticleUpdate<C_Particle3D_R1D_CP> : public ParticleUpdateBase<C_Particle3D_R1D_CP>
{
public:
	ParticleUpdate() : ParticleUpdateBase() {}
	
	FORCE_INLINE void Load(XMVECTOR vFrameTime) 
	{
		LoadBase(vFrameTime);

		m_vRotation = XMLoadFloat4A16((XMFLOAT4A16*)&m_pParticle->vRotation);
		m_vRotationRate = XMVectorSplatY(m_vRotation);
		m_vRotation = XMVectorSplatX(m_vRotation);
		m_vPlanePosition = XMLoadFloat3((XMFLOAT3*)&m_pParticle->vPlanePosition);
		m_vPlaneNormal = XMLoadHenDN3((XMHENDN3*)&m_pParticle->vPlaneNormal);
		m_iFlags &= ~PARTICLE_COLLISION;
		m_iFlags &= ~PARTICLE_COLLISION_FIRST;
	}

	FORCE_INLINE void Store()
	{
		StoreBase();

		m_vRotation = XMVectorMergeXY(m_vRotation, m_vRotationRate);
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vRotation, m_vRotation);
	}

	FORCE_INLINE void CreateRotation(F32 fRotation, F32 fRotationRate)
	{
		m_pParticle->vRotation.x = fRotation;
		m_pParticle->vRotation.y = fRotationRate;
	}

	FORCE_INLINE void UpdateRotation()
	{
		XMVECTOR vOne = XMVectorSplatOne();
		XMVECTOR vMinusOne = XMVectorNegate(vOne);
		XMVECTOR vTwo = XMVectorAdd(vOne, vOne);

		XMVECTOR vPi = XMVectorSplatX(g_XMPiConstants0);
		XMVECTOR vNegativePi = XMVectorNegate(vPi);
		XMVECTOR vTwoPi = XMVectorSplatY(g_XMPiConstants0);

		// keep rotation between (-1,1)
		XMVECTOR vRotationRateTime = XMVectorMultiply(m_vRotationRate, m_vTime);

		m_vRotation = XMVectorAdd(m_vRotation, vRotationRateTime);
		if(XMVector4Less(m_vRotation, vNegativePi))
			m_vRotation = XMVectorAdd(m_vRotation, vTwoPi);
		else if(XMVector4Greater(m_vRotation, vPi))
			m_vRotation = XMVectorSubtract(m_vRotation, vTwoPi);
	}

	FORCE_INLINE void UpdatePosition()
	{
		m_vPosition = XMVectorAdd(m_vVelocityTime, m_vPosition);
	}

	FORCE_INLINE void UpdateVelocity(XMVECTOR vResistance, XMVECTOR vAcceleration)
	{
		m_vVelocity = XMVectorAdd(m_vVelocity, vAcceleration);
		m_vVelocity = XMVectorMultiply(m_vVelocity, vResistance);
		
		// calc velocity time here cause its used in collision code
		m_vVelocityTime = XMVectorMultiply(m_vVelocity, m_vTime);
	}

	FORCE_INLINE void UpdateVelocity(XMVECTOR vResistance, XMVECTOR vAcceleration, XMVECTOR vForce)
	{
		m_vVelocity = XMVectorMultiplyAdd(vForce, XMVectorSplatX(m_vForceSizeLife), m_vVelocity);
		m_vVelocity = XMVectorAdd(m_vVelocity, vAcceleration);
		m_vVelocity = XMVectorMultiply(m_vVelocity, vResistance);
		
		// calc velocity time here cause its used in collision code
		m_vVelocityTime = XMVectorMultiply(m_vVelocity, m_vTime);
	}

	FORCE_INLINE void UpdateCollisions(XMVECTOR vRestitution, XMVECTOR vFriction, const bool bDieOnImpact,  XMVECTOR vSleepSpeedSq, const MathsStorage::Float4A16* materials)
	{
		if(!(m_iFlags & PARTICLE_COLLISION_NONE))
		{
			XMVECTOR vDiff = XMVectorSubtract(m_vPosition, m_vPlanePosition);

			XMVECTOR vGroundDist = XMVector3Dot(vDiff, m_vPlaneNormal);
			XMVECTOR vTowards = XMVector3Dot(m_vVelocity, m_vPlaneNormal);
			
			if(XMVector4Less(vGroundDist, XMVectorZero()) && XMVector4Less(vTowards, XMVectorZero()))
			{
				XMVECTOR vMaterial = XMLoadFloat4A16((XMFLOAT4A16*)&materials[0]);
				XMVECTOR vSurfaceRestitution = XMVectorSplatX(vMaterial);
				XMVECTOR vSurfaceFriction = XMVectorSubtract(XMVectorSplatOne(), XMVectorMultiply(m_vTime, XMVectorSplatY(vMaterial)));
				
				XMVECTOR vDot = XMVector3Dot(m_vVelocityTime, m_vPlaneNormal);
				XMVECTOR vReciprocal = XMVectorReciprocal(vDot);

				vGroundDist = XMVectorSubtract(vGroundDist, g_Epsilon);
				m_vPosition = XMVectorSubtract(m_vPosition, XMVectorMultiply(vGroundDist, m_vPlaneNormal));
				
				XMVECTOR vNormalVelocity = XMVectorMultiply(m_vPlaneNormal, XMVector3Dot(m_vPlaneNormal, m_vVelocity));
				
				vRestitution = XMVectorMultiply(XMVectorAdd(vRestitution, vSurfaceRestitution), g_Half);
				vFriction = XMVectorMultiply(XMVectorAdd(vFriction, vSurfaceFriction), g_Half);
				
				m_vVelocity = XMVectorSubtract(m_vVelocity, XMVectorMultiply(vNormalVelocity, vRestitution));				
				m_vVelocity = XMVectorMultiply(m_vVelocity, vFriction);

				if(bDieOnImpact)
					m_vLife = XMVectorSplatOne();

				m_iFlags |= PARTICLE_COLLISION;
				if(!(m_iFlags & PARTICLE_COLLISION_ONCE))
					m_iFlags |= PARTICLE_COLLISION_FIRST;
				m_iFlags |= PARTICLE_COLLISION_ONCE;

				if(XMVector4Less(XMVector3LengthSq(m_vVelocity), vSleepSpeedSq))
					m_iFlags |= PARTICLE_COLLISION_SLEEP;
			}
		}
	}

	FORCE_INLINE void CreateCollisionPlane(XMVECTOR vPlanePosition, XMVECTOR vPlaneNormal)
	{
		// add the size of particle to collision plane
		XMVECTOR vSize = XMVectorSplatZ(m_vForceSizeLife);
		XMStoreFloat3((XMFLOAT3*)&m_pParticle->vPlanePosition, XMVectorMultiplyAdd(vPlaneNormal, vSize, vPlanePosition));
		XMStoreHenDN3((XMHENDN3*)&m_pParticle->vPlaneNormal, vPlaneNormal);
	}

	FORCE_INLINE void CreateCollisionPlane(const MathsStorage::Float4A16& vPlanePosition,const MathsStorage::Float4A16& vPlaneNormal)
	{
		XMVECTOR vXmPlanePosition = XMLoadFloat4A16((XMFLOAT4A16*)&vPlanePosition);
		XMVECTOR vXmPlaneNormal = XMLoadFloat4A16((XMFLOAT4A16*)&vPlaneNormal);
		
		XMStoreFloat3((XMFLOAT3*)&m_pParticle->vPlanePosition, vXmPlanePosition);
		XMStoreHenDN3((XMHENDN3*)&m_pParticle->vPlaneNormal, vXmPlaneNormal);
	}

	XMVECTOR	m_vVelocityTime;
	XMVECTOR	m_vPlanePosition;
	XMVECTOR	m_vPlaneNormal;
	XMVECTOR	m_vRotation;
	XMVECTOR	m_vRotationRate;
};

template<>
class ParticleUpdate<C_Particle3D_RV3D> : public ParticleUpdateBase<C_Particle3D_RV3D>
{
public:
	ParticleUpdate() : ParticleUpdateBase() {}
	
	FORCE_INLINE void Load(XMVECTOR vFrameTime) 
	{
		LoadBase(vFrameTime);
		m_qRotation = XMLoadFloat4A16((XMFLOAT4A16*)&m_pParticle->qRotation);
		m_vAngularVelocity = XMLoadFloat4A16((XMFLOAT4A16*)&m_pParticle->vAngularVelocity);
	}

	FORCE_INLINE void Store()
	{
		StoreBase();
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->qRotation, m_qRotation);
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vAngularVelocity, m_vAngularVelocity);
	}

	FORCE_INLINE void CreateRotation(const MathsStorage::Float4A16& qRotation, const MathsStorage::Float4A16& vAngularVelocity)
	{
		m_pParticle->qRotation = qRotation;
		m_pParticle->vAngularVelocity = vAngularVelocity;
	}

	FORCE_INLINE void CreateRotation(XMVECTOR qRotation, XMVECTOR vAngularVelocity)
	{
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->qRotation, qRotation);
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vAngularVelocity, vAngularVelocity);
	}

	FORCE_INLINE void UpdateRotation()
	{
		XMVECTOR vAngularVelocityTime = XMVectorMultiply(m_vAngularVelocity, m_vTime);
		m_qRotation = XMVectorAdd(m_qRotation, XMQuaternionMultiply(vAngularVelocityTime, m_qRotation));
		m_qRotation = XMQuaternionNormalize(m_qRotation);
	}

	XMVECTOR m_qRotation;
	XMVECTOR m_vAngularVelocity;
};

template<>
class ParticleUpdate<C_Particle3D_RV3D_CP> : public ParticleUpdateBase<C_Particle3D_RV3D_CP>
{
public:

	FORCE_INLINE void Load(XMVECTOR vFrameTime) 
	{
		ParticleUpdateBase::LoadBase(vFrameTime);
		m_qRotation = XMLoadFloat4A16((XMFLOAT4A16*)&m_pParticle->qRotation);
		m_vAngularVelocity = XMLoadFloat4A16((XMFLOAT4A16*)&m_pParticle->vAngularVelocity);
		m_vPlanePosition = XMLoadFloat3((XMFLOAT3*)&m_pParticle->vPlanePosition);
		m_vPlaneNormal = XMLoadHenDN3((XMHENDN3*)&m_pParticle->vPlaneNormal);

		m_iFlags &= ~PARTICLE_COLLISION;
		m_iFlags &= ~PARTICLE_COLLISION_FIRST;
	}

	FORCE_INLINE void Store()
	{
		StoreBase();
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->qRotation, m_qRotation);
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vAngularVelocity, m_vAngularVelocity);
	}

	FORCE_INLINE void CreateRotation(const MathsStorage::Float4A16& qRotation, const MathsStorage::Float4A16& vAngularVelocity)
	{
		m_pParticle->qRotation = qRotation;
		m_pParticle->vAngularVelocity = vAngularVelocity;
	}

	FORCE_INLINE void CreateRotation(XMVECTOR qRotation, XMVECTOR vAngularVelocity)
	{
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->qRotation, qRotation);
		XMStoreFloat4A16((XMFLOAT4A16*)&m_pParticle->vAngularVelocity, vAngularVelocity);
	}

	FORCE_INLINE void UpdateRotation()
	{
		XMVECTOR vAngularVelocityTime = XMVectorMultiply(m_vAngularVelocity, m_vTime);
		m_qRotation = XMVectorAdd(m_qRotation, XMQuaternionMultiply(vAngularVelocityTime, m_qRotation));
		m_qRotation = XMQuaternionNormalize(m_qRotation);
	}

	FORCE_INLINE void UpdatePosition()
	{
		m_vPosition = XMVectorAdd(m_vVelocityTime, m_vPosition);
	}

	FORCE_INLINE void UpdateVelocity(XMVECTOR vResistance, XMVECTOR vAcceleration)
	{
		m_vVelocity = XMVectorAdd(m_vVelocity, vAcceleration);
		m_vVelocity = XMVectorMultiply(m_vVelocity, vResistance);
		
		// calc velocity time here cause its used in collision code
		m_vVelocityTime = XMVectorMultiply(m_vVelocity, m_vTime);
	}

	FORCE_INLINE void UpdateVelocity(XMVECTOR vResistance, XMVECTOR vAcceleration, XMVECTOR vForce)
	{
		m_vVelocity = XMVectorMultiplyAdd(vForce, XMVectorSplatX(m_vForceSizeLife), m_vVelocity);
		m_vVelocity = XMVectorAdd(m_vVelocity, vAcceleration);
		m_vVelocity = XMVectorMultiply(m_vVelocity, vResistance);
		
		// calc velocity time here cause its used in collision code
		m_vVelocityTime = XMVectorMultiply(m_vVelocity, m_vTime);
	}

	FORCE_INLINE void UpdateCollisions(XMVECTOR vRestitution, XMVECTOR vFriction, const bool bDieOnImpact, XMVECTOR vSleepSpeedSq, const MathsStorage::Float4A16* materials)
	{
		if(!(m_iFlags & PARTICLE_COLLISION_NONE))
		{
			XMVECTOR vDiff = XMVectorSubtract(m_vPosition, m_vPlanePosition);
			XMVECTOR vGroundDist = XMVector3Dot(vDiff, m_vPlaneNormal);
			XMVECTOR vTowards = XMVector3Dot(m_vVelocity, m_vPlaneNormal);
		
			if(XMVector4Less(vGroundDist, XMVectorZero()) && XMVector4Less(vTowards, XMVectorZero()))
			{
				XMVECTOR vMaterial = XMLoadFloat4A16((XMFLOAT4A16*)&materials[0]);
				XMVECTOR vSurfaceRestitution = XMVectorSplatX(vMaterial);
				XMVECTOR vSurfaceFriction = XMVectorSubtract(XMVectorSplatOne(), XMVectorMultiply(m_vTime, XMVectorSplatY(vMaterial)));
				
				XMVECTOR vDot = XMVector3Dot(m_vVelocityTime, m_vPlaneNormal);
				XMVECTOR vReciprocal = XMVectorReciprocal(vDot);

				vGroundDist = XMVectorSubtract(vGroundDist, g_Epsilon);
				m_vPosition = XMVectorSubtract(m_vPosition, XMVectorMultiply(vGroundDist, m_vPlaneNormal));
				
				XMVECTOR vNormalVelocity = XMVectorMultiply(m_vPlaneNormal, XMVector3Dot(m_vPlaneNormal, m_vVelocity));
				
				vRestitution = XMVectorMultiply(XMVectorAdd(vRestitution, vSurfaceRestitution), g_Half);
				vFriction = XMVectorMultiply(XMVectorAdd(vFriction, vSurfaceFriction), g_Half);
				
				m_vVelocity = XMVectorSubtract(m_vVelocity, XMVectorMultiply(vNormalVelocity, vRestitution));				
				m_vVelocity = XMVectorMultiply(m_vVelocity, vFriction);

				m_vAngularVelocity = XMVectorMultiply(m_vAngularVelocity, vFriction);

				if(bDieOnImpact)
					m_vLife = XMVectorSplatOne();

				m_iFlags |= PARTICLE_COLLISION;
				if(!(m_iFlags & PARTICLE_COLLISION_ONCE))
					m_iFlags |= PARTICLE_COLLISION_FIRST;
				m_iFlags |= PARTICLE_COLLISION_ONCE;

				if(XMVector4Less(XMVector3LengthSq(m_vVelocity), vSleepSpeedSq))
					m_iFlags |= PARTICLE_COLLISION_SLEEP;
			}
		}
	}

	/*FORCE_INLINE void UpdateForces(ConstForces& currentForces)
	{
		ParticleUpdateBase::UpdateForces(currentForces);
	}*/

	FORCE_INLINE void CreateCollisionPlane(XMVECTOR vPlanePosition, XMVECTOR vPlaneNormal)
	{
		// add the size of particle to collision plane
		XMVECTOR vSize = XMVectorSplatZ(m_vForceSizeLife);
		XMStoreFloat3((XMFLOAT3*)&m_pParticle->vPlanePosition, XMVectorMultiplyAdd(vPlaneNormal, vSize, vPlanePosition));
		XMStoreHenDN3((XMHENDN3*)&m_pParticle->vPlaneNormal, vPlaneNormal);
	}

	FORCE_INLINE void CreateCollisionPlane(const MathsStorage::Float4A16& vPlanePosition,const MathsStorage::Float4A16& vPlaneNormal)
	{
		XMVECTOR vXmPlanePosition = XMLoadFloat4A16((XMFLOAT4A16*)&vPlanePosition);
		XMVECTOR vXmPlaneNormal = XMLoadFloat4A16((XMFLOAT4A16*)&vPlaneNormal);
		
		XMStoreFloat3((XMFLOAT3*)&m_pParticle->vPlanePosition, vXmPlanePosition);
		XMStoreHenDN3((XMHENDN3*)&m_pParticle->vPlaneNormal, vXmPlaneNormal);
	}

	XMVECTOR	m_vVelocityTime;
	XMVECTOR	m_vPlanePosition;
	XMVECTOR	m_vPlaneNormal;
	XMVECTOR	m_qRotation;
	XMVECTOR	m_vAngularVelocity;
};

}
}

#endif // RENDER_DYNAMICEFFECT_PARTICLE_UPDATEXM_H
