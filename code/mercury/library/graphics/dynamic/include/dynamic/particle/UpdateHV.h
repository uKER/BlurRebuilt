#ifndef RENDER_DYNAMICEFFECT_PARTICLE_UPDATEHV_H
#define RENDER_DYNAMICEFFECT_PARTICLE_UPDATEHV_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Particle/Particle.h"
//#include "Dynamic/Particle/Force.h"
#include "Dynamic/Block.h"


namespace DynamicEffect
{
namespace Particle
{

class BlockUpdate
{
public:
	/*FORCE_INLINE void GetForces(const Forces& allForces, ParticleBlock* pBlock)
	{
		forces.Clear();
		
		U32 nIntersects = 0;
		Maths::HVec3 vBlockCentre = Maths::Load<Maths::HVec3>((MathsStorage::Float3A16*)&pBlock->m_vCentre);
		Maths::HVec3 vSize = Maths::Load<Maths::HVec3>((MathsStorage::Float3A16*)&pBlock->m_vSize);
		
		Maths::HF32 vBlockRadius = Maths::Length(vSize);

		for(Forces::ConstIterator pIterator = allForces.Begin(); pIterator != allForces.End(); pIterator++)
		{
			const C_Force* pForce = (*pIterator);
			Maths::HVec3 vForceCentre = Maths::Load<Maths::HVec3>(&pForce->m_vPosition);
			Maths::HF32 vForceRadius = pForce->m_vRadius.x;

			Maths::HVec3 vBlockToForce = vBlockCentre - vForceCentre;
			Maths::HF32 vLength = Maths::Length(vBlockToForce);

			vLength = vLength - vBlockRadius;
			vLength = vLength - vForceRadius;

			if(vLength < 0.0f)
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
	FORCE_INLINE void LoadBase(Maths::HF32& vFrameTime)
	{
		Maths::HVec4 vPositionVelocity(Maths::Load<Maths::HVec4>((MathsStorage::Float4A16*)&m_pParticle->vPosition));
		
		m_vPosition = Maths::HVec2(vPositionVelocity.X(), vPositionVelocity.Y());
		m_vVelocity = Maths::HVec2(vPositionVelocity.Z(), vPositionVelocity.W());
		
		m_vLifeSizeLife = Maths::Load<Maths::HVec3>(&m_pParticle->vLifeSizeLife);
		m_vLife = m_vLifeSizeLife.X();
		m_iFlags = m_pParticle->iFlags;
		m_vTime = vFrameTime;
	}

	FORCE_INLINE void StoreBase()
	{
		Maths::HVec4 vPositionVelocity(m_vPosition.X(), m_vPosition.Y(), m_vVelocity.X(), m_vVelocity.Y());
		Maths::Store(vPositionVelocity, (MathsStorage::Float4A16*)&m_pParticle->vPosition);
		
		m_pParticle->vLifeSizeLife.x = m_vLife;
		m_pParticle->iFlags = m_iFlags;
		
		// update bounds
		m_vSizeMax = m_vSizeMax.Max(m_vLifeSizeLife.Y());
		m_vMax = m_vMax.Max(m_vPosition);
		m_vMin = m_vMin.Min(m_vPosition);
	}

	FORCE_INLINE void SetParticle(Particle* pParticle)
	{
		m_pParticle = pParticle;
	}

	FORCE_INLINE ParticleUpdateBase2D() 
	{
		m_vSizeMax = 0.0f;
		m_vMin = Maths::HVec2(Maths::HF32(10000.0f));
		m_vMax = Maths::HVec2(Maths::HF32(-10000.0f));
	}
	FORCE_INLINE void UpdateVelocity(Maths::HF32& vResistance)
	{
		m_vVelocity = m_vVelocity * vResistance;
	}
	FORCE_INLINE void UpdateVelocity(Maths::HF32& vResistance, Maths::HVec2& vAcceleration)
	{
		m_vVelocity = m_vVelocity + vAcceleration;
		m_vVelocity = m_vVelocity * vResistance;
	}
	FORCE_INLINE void UpdateVelocity(Maths::HF32& vResistance, Maths::HVec2& vAcceleration, Maths::HVec2& vForce)
	{
		m_vVelocity = m_vVelocity + vForce * m_vLifeSizeLife.X();
		m_vVelocity = m_vVelocity + vAcceleration;
		m_vVelocity = m_vVelocity * vResistance;
	}
	FORCE_INLINE void UpdatePosition()
	{
		m_vPosition = m_vPosition + (m_vVelocity * m_vTime);
	}
	FORCE_INLINE void UpdateLife()
	{
		Maths::HF32 vAdd = m_vTime * m_vLifeSizeLife.Z();
		m_vLife = m_vLife + vAdd;
	}
	
	// creation stuff always takes Maths::HVec
	FORCE_INLINE void CreatePositionVelocity(const Maths::HVec2& vPosition, const Maths::HVec2& vVelocity)
	{
		Maths::HVec4 vPositionVelocity(vPosition.X(), vPosition.Y(), vVelocity.X(), vVelocity.Y());
		Maths::Store(vPositionVelocity, (MathsStorage::Float4A16*)&m_pParticle->vPosition);
	}

	FORCE_INLINE void CreateForceSizeLife(const Maths::HVec3& vLifeSizeLife)
	{
		// store this so collision planes can get at it
		m_vLifeSizeLife = vLifeSizeLife;

		Maths::Store(m_vLifeSizeLife, &m_pParticle->vLifeSizeLife);
	}

	FORCE_INLINE void CreateColour(const Maths::HVec4& vLighting, const Maths::HVec4& vColour)
	{
		Maths::HVec4 vColourHDR = vColour * vLighting;
		
		// store hdr colour in xyz
		Maths::Store(vColourHDR, &m_pParticle->vColour);
	}

	FORCE_INLINE S32 IsDead()
	{
		return m_vLife > 1.f;
	}

	FORCE_INLINE void StoreBounds(MathsStorage::Float4A16* pStore)
	{
		Maths::HVec2 vSizeMax(m_vSizeMax);
		m_vMax += vSizeMax;
		m_vMin -= vSizeMax;
		Maths::HVec2 vCentre = (m_vMax + m_vMin) * Maths::HF32(0.5f);
		Maths::HVec2 vSize = m_vMax - vCentre;

		Maths::HVec4 vCentre2(vCentre.X(), vCentre.Y(), 0.0f, 1.0f);
		Maths::HVec4 vSize2(vSize.X(), vSize.Y(), 0.0f, 1.0f);
		
		Maths::Store((Maths::V128)vCentre2, pStore);
		Maths::Store((Maths::V128)vSize2, (MathsStorage::Float3A16*)(pStore+1));
	}

	Particle*		m_pParticle;

	Maths::HVec2	m_vPosition;
	Maths::HVec2	m_vVelocity;
	Maths::HF32		m_vLife;
	Maths::HVec3	m_vLifeSizeLife;
	
	Maths::HF32		m_vSizeMax;
	Maths::HF32		m_vTime;
	Maths::HVec2	m_vMin;
	Maths::HVec2	m_vMax;
	
	U8				m_iFlags;
};

template<typename Particle>
class ParticleUpdate2D : public ParticleUpdateBase2D<Particle>
{
public:
	ParticleUpdate2D() : ParticleUpdateBase2D<Particle>() {}

	FORCE_INLINE void Load(Maths::HF32& vFrameTime) 
	{
		ParticleUpdateBase2D<Particle>::LoadBase(vFrameTime);
	}
	
	FORCE_INLINE void Store()
	{
		ParticleUpdateBase2D<Particle>::StoreBase();
	}
};

template<typename Particle>
class ParticleUpdateBase
{
public:
	FORCE_INLINE void LoadBase(Maths::HF32& vFrameTime)
	{
		m_vPosition = Maths::Load<Maths::HVec3>((MathsStorage::Float3A16*)&m_pParticle->vPosition);
		m_vLife = m_pParticle->fLife;
		m_vVelocity = Maths::Load<Maths::HVec3>((MathsStorage::Float3A16*)&m_pParticle->vVelocity);	
		m_vForceSizeLife = Maths::Load<Maths::HVec4>(&m_pParticle->vForceSizeLife);
		m_iFlags = m_pParticle->iFlags;
		m_vTime = vFrameTime;
	}

	FORCE_INLINE void StoreBase()
	{
		Maths::HVec4 vPositionLife(m_vPosition);
		vPositionLife.W(m_vLife);
		Maths::Store(vPositionLife, (MathsStorage::Float4A16*)&m_pParticle->vPosition);
		Maths::Store(m_vVelocity, &m_pParticle->vVelocity);
		m_pParticle->iFlags = m_iFlags;
		
		// update bounds
		m_vSizeMax = m_vSizeMax.Max(m_vForceSizeLife.Z());
		m_vMax = Maths::Max(m_vMax, m_vPosition);
		m_vMin = Maths::Min(m_vMin, m_vPosition);
	}

	FORCE_INLINE void SetParticle(Particle* pParticle)
	{
		m_pParticle = pParticle;
	}

	FORCE_INLINE ParticleUpdateBase() 
	{
		m_vSizeMax = 0.0f;
		m_vMin = Maths::HVec3(Maths::HF32(10000.0f));
		m_vMax = Maths::HVec3(Maths::HF32(-10000.0f));
	}
	FORCE_INLINE void UpdateVelocity(Maths::HF32& vResistance)
	{
		m_vVelocity = m_vVelocity * vResistance;
	}
	FORCE_INLINE void UpdateVelocity(Maths::HF32& vResistance, Maths::HVec3& vAcceleration)
	{
		m_vVelocity = m_vVelocity + vAcceleration;
		m_vVelocity = m_vVelocity * vResistance;
	}
	FORCE_INLINE void UpdateVelocity(Maths::HF32& vResistance, Maths::HVec3& vAcceleration, Maths::HVec3& vForce)
	{
		m_vVelocity = m_vVelocity + vForce * m_vForceSizeLife.X();
		m_vVelocity = m_vVelocity + vAcceleration;
		m_vVelocity = m_vVelocity * vResistance;
	}
	FORCE_INLINE void UpdatePosition()
	{
		m_vPosition = m_vPosition + (m_vVelocity * m_vTime);
	}
	FORCE_INLINE void UpdateLife()
	{
		Maths::HF32 vAdd = m_vTime * m_vForceSizeLife.W();
		m_vLife = m_vLife + vAdd;
	}
	

	// creation stuff always takes Maths::HVec
	FORCE_INLINE void CreatePositionLife(const Maths::HVec3& vPosition)
	{
		Maths::HVec4 vPositionLife(vPosition);
		vPositionLife.W(Maths::Constants<Maths::HF32>::Zero());
		Maths::Store(vPositionLife, (MathsStorage::Float4A16*)&m_pParticle->vPosition);
	}

	FORCE_INLINE void CreatePosition(const MathsStorage::Float4A16& vPosition)
	{
		m_pParticle->vPosition = vPosition;
	}

	FORCE_INLINE void CreateForceSizeLife(const Maths::HVec4& vForceSizeLife)
	{
		// store this so collision planes can get at it
		m_vForceSizeLife = vForceSizeLife;

		Maths::Store(vForceSizeLife, &m_pParticle->vForceSizeLife);
	}

	FORCE_INLINE void CreateForceSizeLife(const Maths::HVec4& vForceSizeLife, const F32* pSize)
	{
		Maths::HF32 fSize(pSize);
		// store this so collision plane can get at it
		m_vForceSizeLife = vForceSizeLife;
		m_vForceSizeLife.Z(m_vForceSizeLife.Z() * fSize);

		Maths::Store(m_vForceSizeLife, &m_pParticle->vForceSizeLife);
	} 

	FORCE_INLINE void CreateColour(const Maths::HVec4& vLighting, const Maths::HVec4& vColour)
	{
		Maths::HVec4 vColourHDR = vColour * vLighting;
		
		// store hdr colour in xyz
		Maths::Store(vColourHDR, &m_pParticle->vColour);
	}

	FORCE_INLINE void CreateVelocity(const Maths::HVec3& vVelocity)
	{
		Maths::Store(vVelocity, (MathsStorage::Float3A16*)&m_pParticle->vVelocity);
	}

	FORCE_INLINE void CreateVelocity(const MathsStorage::Float4A16& vVelocity)
	{
		m_pParticle->vVelocity = vVelocity;
	}

	FORCE_INLINE S32 IsDead()
	{
		return m_vLife > 1.f;
	}

	/*FORCE_INLINE void UpdateForces(ConstForces& currentForces)
	{
		for(S32 i = 0; i < currentForces.Size(); ++i)
		{
			const C_Force* pForce = currentForces[i];
			// get force properties
			Maths::HVec3 vCentre = Maths::Load<Maths::HVec3>(&pForce->m_vPosition);
			Maths::HF32 vRadiusRcp = pForce->m_vRadiusRcp.x;
			Maths::HF32 vStrength = pForce->m_vStrength.x;

			vStrength = vStrength * m_vTime;

			// calculate a scaling factor and direction vector based on displacement from force origin
			Maths::HVec3 vDisplacement = vCentre - m_vPosition;
			Maths::HF32 vLength = vDisplacement.Length();
			Maths::HF32 vScale = 1.0f - (vLength * vRadiusRcp);

			Maths::HVec3 vDirection;
			
			if(pForce->m_eType == E_ForceType_Point)
				vDirection = vDisplacement * vLength.Rcp();
			else
				vDirection = Maths::Load<Maths::HVec3>((MathsStorage::Float3A16*)&pForce->m_vVelocity);
			
			
			// clamp and scale
			vScale = vScale.Max(0.f);
			vScale = vScale * vStrength;

			// add to velocity
			m_vVelocity = m_vVelocity + (vDirection * vScale);
		}
	}*/

	FORCE_INLINE void StoreBounds(MathsStorage::Float4A16* pStore)
	{
		Maths::HVec3 vSizeMax(m_vSizeMax);
		m_vMax += vSizeMax;
		m_vMin -= vSizeMax;
		Maths::HVec3 vCentre = (m_vMax + m_vMin) * Maths::Constants<Maths::HF32>::Half();
		Maths::HVec3 vSize = m_vMax - vCentre;

		Maths::HVec4 vCentre2(vCentre);
		vCentre2.W(Maths::Constants<Maths::HF32>::One());
		
		Maths::Store(vCentre2, pStore);
		Maths::Store(vSize, (MathsStorage::Float3A16*)(pStore+1));
	}

	Particle*		m_pParticle;

	Maths::HVec3	m_vPosition;
	Maths::HF32		m_vLife;
	Maths::HVec3	m_vVelocity;
	Maths::HVec4	m_vForceSizeLife;
	
	Maths::HF32		m_vSizeMax;
	Maths::HF32		m_vTime;
	Maths::HVec3	m_vMin;
	Maths::HVec3	m_vMax;
	
	U8				m_iFlags;
};

template<typename Particle>
class ParticleUpdate : public ParticleUpdateBase<Particle>
{
public:
	ParticleUpdate() : ParticleUpdateBase<Particle>() {}

	FORCE_INLINE void Load(Maths::HF32& vFrameTime) 
	{
		ParticleUpdateBase<Particle>::LoadBase(vFrameTime);
	}
	
	FORCE_INLINE void Store()
	{
		ParticleUpdateBase<Particle>::StoreBase();
	}
};

template<>
class ParticleUpdate<C_Particle3D_CP> : public ParticleUpdateBase<C_Particle3D_CP>
{
public:
	ParticleUpdate() : ParticleUpdateBase<C_Particle3D_CP>() {}

	FORCE_INLINE void Load(Maths::HF32& vFrameTime) 
	{
		LoadBase(vFrameTime);
		m_vPlanePosition = Maths::Load<Maths::HVec3>((MathsStorage::Float3A16*)&m_pParticle->vPlanePosition);
		m_vPlaneNormal = Maths::Load<Maths::HVec3>(&m_pParticle->vPlaneNormal);
		m_iFlags &= ~PARTICLE_COLLISION;
		m_iFlags &= ~PARTICLE_COLLISION_FIRST;
	}
	
	FORCE_INLINE void Store()
	{
		StoreBase();
	}

	FORCE_INLINE void UpdatePosition()
	{
		m_vPosition = m_vPosition + m_vVelocityTime;
	}

	FORCE_INLINE void UpdateVelocity(Maths::HF32& vResistance, Maths::HVec3& vAcceleration)
	{
		m_vVelocity = m_vVelocity + vAcceleration;
		m_vVelocity = m_vVelocity * vResistance;
		
		// calc velocity time here cause its used in collision code
		m_vVelocityTime = m_vVelocity * m_vTime;
	}

	FORCE_INLINE void UpdateVelocity(Maths::HF32& vResistance, Maths::HVec3& vAcceleration, Maths::HVec3& vForce)
	{
		m_vVelocity = m_vVelocity + vForce * m_vForceSizeLife.X();
		m_vVelocity = m_vVelocity + vAcceleration;
		m_vVelocity = m_vVelocity * vResistance;
		
		// calc velocity time here cause its used in collision code
		m_vVelocityTime = m_vVelocity * m_vTime;
	}

	FORCE_INLINE void UpdateCollisions(const Maths::HF32& vRestitution, const Maths::HF32& vFriction, const bool bDieOnImpact, const Maths::HF32& vSleepSpeedSq, const MathsStorage::Float4A16* materials)
	{
		if(!(m_iFlags & PARTICLE_COLLISION_NONE))
		{	
			Maths::HVec3 vDiff(m_vPosition - m_vPlanePosition);
			Maths::HF32 vGroundDist = vDiff | m_vPlaneNormal;
			Maths::HF32 vTowards = m_vVelocity | m_vPlaneNormal;

			if(vGroundDist < Maths::Constants<Maths::HF32>::Zero() && vTowards < Maths::Constants<Maths::HF32>::Zero())
			{
				Maths::HVec4 vMaterial(Maths::Load<Maths::HVec4>(&materials[0]));
				Maths::HF32 vSurfaceRestitution = vMaterial.X();
				Maths::HF32 vSurfaceFriction = Maths::Constants<Maths::HF32>::One() - m_vTime * vMaterial.Y();

				Maths::HF32 vDot = m_vVelocityTime | m_vPlaneNormal;
				Maths::HF32 vReciprocal = vDot.Reciprocal();
				
				vGroundDist = vGroundDist - 1.192092896e-7f;
				m_vPosition = m_vPosition - (vGroundDist * m_vPlaneNormal);

				vSurfaceRestitution = (vRestitution + vSurfaceRestitution) * Maths::Constants<Maths::HF32>::Half();
				vSurfaceFriction = (vFriction + vSurfaceFriction) * Maths::Constants<Maths::HF32>::Half();

				Maths::HVec3 vNormalVelocity = m_vPlaneNormal * (m_vPlaneNormal | m_vVelocity);
				m_vVelocity = m_vVelocity - (vNormalVelocity * vSurfaceRestitution);
				m_vVelocity = m_vVelocity * vSurfaceFriction;

				if(bDieOnImpact)
					m_vLife = Maths::Constants<Maths::HF32>::One();

				// set collision
				m_iFlags |= PARTICLE_COLLISION;
				if(!(m_iFlags & PARTICLE_COLLISION_ONCE))
					m_iFlags |= PARTICLE_COLLISION_FIRST;
				m_iFlags |= PARTICLE_COLLISION_ONCE;

				if(Maths::Length2(m_vVelocity) < vSleepSpeedSq)
					m_iFlags |= PARTICLE_COLLISION_SLEEP;
			}
		}
	}

	FORCE_INLINE void CreateCollisionPlane(const Maths::HVec3& vPlanePosition, const Maths::HVec3& vPlaneNormal)
	{
		Maths::HVec3 vHvPlanePosition = vPlanePosition + vPlaneNormal * m_vForceSizeLife.Z();
		Maths::Store(vHvPlanePosition, (MathsStorage::Float4A16*)&m_pParticle->vPlanePosition);
		Maths::Store((Maths::V128)vPlaneNormal, &m_pParticle->vPlaneNormal);
	}

	FORCE_INLINE void CreateCollisionPlane(const MathsStorage::Float4A16& vPlanePosition, const MathsStorage::Float4A16& vPlaneNormal)
	{
		Maths::HVec3 v(Maths::Load<Maths::HVec3>(&vPlaneNormal));
		m_pParticle->vPlanePosition = MathsStorage::Float3(vPlanePosition.x, vPlanePosition.y, vPlanePosition.z);
		Maths::Store(v, &m_pParticle->vPlaneNormal);
	}

	Maths::HVec3	m_vVelocityTime;
	Maths::HVec3	m_vPlanePosition;
	Maths::HVec3	m_vPlaneNormal;
};

template<>
class ParticleUpdate<C_Particle3D_P> : public ParticleUpdateBase<C_Particle3D_P>
{
public:
	ParticleUpdate() : ParticleUpdateBase<C_Particle3D_P>() {}

	FORCE_INLINE void Load(Maths::HF32& vFrameTime) 
	{
		LoadBase(vFrameTime);
		m_vPosition2 = Maths::Load<Maths::HVec3>(&m_pParticle->vPosition2);
	}
	
	FORCE_INLINE void Store()
	{
		StoreBase();
		Maths::Store(m_vPosition2, &m_pParticle->vPosition2);
	}

	FORCE_INLINE void CreatePositionLife(const Maths::HVec3& vPosition, const Maths::HVec3& vPositionCurve)
	{
		Maths::HVec4 vPositionLife(vPosition);
		vPositionLife.W(Maths::Constants<Maths::HF32>::Zero());

		Maths::Store(vPositionLife, &m_pParticle->vPosition2);

		Maths::HVec4 vPositionCurve4(vPositionCurve);

		Maths::HVec4 vPositionCurveLife = vPositionLife + vPositionCurve4;
		Maths::Store(vPositionCurveLife, (MathsStorage::Float4A16*)&m_pParticle->vPosition);
	}

	FORCE_INLINE void UpdatePosition(const Maths::HVec3& vPosition1)
	{
		// store the particle local position in position 2
		m_vPosition2 += m_vVelocity * m_vTime;
		// store the position plus procedural position in position 1
		m_vPosition = m_vPosition2 + vPosition1;
	}

	Maths::HVec3	m_vPosition2;
};

template<>
class ParticleUpdate<C_Particle3D_R1D_P> : public ParticleUpdateBase<C_Particle3D_R1D_P>
{
public:
	ParticleUpdate() : ParticleUpdateBase<C_Particle3D_R1D_P>() {}

	FORCE_INLINE void Load(Maths::HF32& vFrameTime)
	{
		LoadBase(vFrameTime);
		m_vPosition2 = Maths::Load<Maths::HVec3>(&m_pParticle->vPosition2);
		Maths::HVec4 vRotation(Maths::Load<Maths::HVec4>(&m_pParticle->vRotation));
		m_vRotationRate = vRotation.Y();
		m_vRotation = vRotation.X();
	}
	
	FORCE_INLINE void Store()
	{
		StoreBase();
		Maths::Store(m_vPosition2, &m_pParticle->vPosition2);
		Maths::HVec4 vRotation(m_vRotation, m_vRotationRate, m_vRotation, m_vRotationRate);
		Maths::Store(vRotation, &m_pParticle->vRotation);
	}

	FORCE_INLINE void CreatePositionLife(const Maths::HVec3& vPosition, const Maths::HVec3& vPositionCurve)
	{
		Maths::HVec4 vPositionLife(vPosition);
		vPositionLife.W(Maths::Constants<Maths::HF32>::Zero());

		Maths::Store(vPositionLife, &m_pParticle->vPosition2);

		Maths::HVec4 vPositionCurve4(vPositionCurve);

		Maths::HVec4 vPositionCurveLife = vPositionLife + vPositionCurve4;
		Maths::Store(vPositionCurveLife, (MathsStorage::Float4A16*)&m_pParticle->vPosition);
	}

	FORCE_INLINE void CreateRotation(const F32 fRotation, const F32 fRotationRate)
	{
		m_pParticle->vRotation.x = fRotation;
		m_pParticle->vRotation.y = fRotationRate;
	}

	FORCE_INLINE void UpdateRotation()
	{
		Maths::HF32 vRotationRateTime = m_vRotationRate * m_vTime;

		m_vRotation = m_vRotation + vRotationRateTime;

		if(m_vRotation < -Maths::Pi)
			m_vRotation += Maths::Pi2;
		else if(m_vRotation > Maths::Pi)
			m_vRotation -= Maths::Pi2;
	}

	FORCE_INLINE void UpdatePosition(const Maths::HVec3& vPosition1)
	{
		// store the particle local position in position 2
		m_vPosition2 += m_vVelocity * m_vTime;
		// store the position plus procedural position in position 1
		m_vPosition = m_vPosition2 + vPosition1;
	}

	Maths::HVec3 m_vPosition2;
	Maths::HF32	m_vRotation;
	Maths::HF32	m_vRotationRate;
};

template<>
class ParticleUpdate<C_Particle3D_R1D> : public ParticleUpdateBase<C_Particle3D_R1D>
{
public:
	ParticleUpdate() : ParticleUpdateBase<C_Particle3D_R1D>() {}
	
	FORCE_INLINE void Load(Maths::HF32& vFrameTime) 
	{
		LoadBase(vFrameTime);

		Maths::HVec4 vRotation(Maths::Load<Maths::HVec4>(&m_pParticle->vRotation));
		m_vRotationRate = vRotation.Y();
		m_vRotation = vRotation.X();
	}

	FORCE_INLINE void Store()
	{
		StoreBase();

		Maths::HVec4 vRotation(m_vRotation, m_vRotationRate, m_vRotation, m_vRotationRate);
		Maths::Store(vRotation, &m_pParticle->vRotation);
	}

	FORCE_INLINE void CreateRotation(const F32 fRotation, const F32 fRotationRate)
	{
		m_pParticle->vRotation.x = fRotation;
		m_pParticle->vRotation.y = fRotationRate;
	}

	FORCE_INLINE void UpdateRotation()
	{
		Maths::HF32 vRotationRateTime = m_vRotationRate * m_vTime;

		m_vRotation = m_vRotation + vRotationRateTime;

		if(m_vRotation < -Maths::Pi)
			m_vRotation += Maths::Pi2;
		else if(m_vRotation > Maths::Pi)
			m_vRotation -= Maths::Pi2;
	}

	Maths::HF32	m_vRotation;
	Maths::HF32	m_vRotationRate;
};

template<>
class ParticleUpdate<C_Particle3D_R1D_CP> : public ParticleUpdateBase<C_Particle3D_R1D_CP>
{
public:
	ParticleUpdate() : ParticleUpdateBase<C_Particle3D_R1D_CP>() {}
	
	FORCE_INLINE void Load(Maths::HF32& vFrameTime) 
	{
		LoadBase(vFrameTime);

		Maths::HVec4 vRotation(Maths::Load<Maths::HVec4>(&m_pParticle->vRotation));
		m_vRotationRate = vRotation.Y();
		m_vRotation = vRotation.X();
		m_vPlanePosition = Maths::Load<Maths::HVec3>((MathsStorage::Float3A16*)&m_pParticle->vPlanePosition);
		m_vPlaneNormal = Maths::Load<Maths::HVec3>(&m_pParticle->vPlaneNormal);
		m_iFlags &= ~PARTICLE_COLLISION;
		m_iFlags &= ~PARTICLE_COLLISION_FIRST;
	}

	FORCE_INLINE void Store()
	{
		StoreBase();

		Maths::HVec4 vRotation(m_vRotation, m_vRotationRate, m_vRotation, m_vRotationRate);
		Maths::Store(vRotation, &m_pParticle->vRotation);
	}

	FORCE_INLINE void CreateRotation(const F32 fRotation, const F32 fRotationRate)
	{
		m_pParticle->vRotation.x = fRotation;
		m_pParticle->vRotation.y = fRotationRate;
	}

	FORCE_INLINE void UpdateRotation()
	{
		Maths::HF32 vRotationRateTime = m_vRotationRate * m_vTime;

		m_vRotation = m_vRotation + vRotationRateTime;

		if(m_vRotation < -Maths::Pi)
			m_vRotation += Maths::Pi2;
		else if(m_vRotation > Maths::Pi)
			m_vRotation -= Maths::Pi2;
	}

	FORCE_INLINE void UpdateVelocity(Maths::HF32& vResistance, Maths::HVec3& vAcceleration)
	{
		m_vVelocity = m_vVelocity + vAcceleration;
		m_vVelocity = m_vVelocity * vResistance;
		
		// calc velocity time here cause its used in collision code
		m_vVelocityTime = m_vVelocity * m_vTime;
	}

	FORCE_INLINE void UpdateVelocity(Maths::HF32& vResistance, Maths::HVec3& vAcceleration, Maths::HVec3& vForce)
	{
		m_vVelocity = m_vVelocity + vForce * m_vForceSizeLife.X();
		m_vVelocity = m_vVelocity + vAcceleration;
		m_vVelocity = m_vVelocity * vResistance;
		
		// calc velocity time here cause its used in collision code
		m_vVelocityTime = m_vVelocity * m_vTime;
	}

	FORCE_INLINE void UpdateCollisions(const Maths::HF32& vRestitution, const Maths::HF32& vFriction, const bool bDieOnImpact, const Maths::HF32& vSleepSpeedSq, const MathsStorage::Float4A16* materials)
	{
		if(!(m_iFlags & PARTICLE_COLLISION_NONE))
		{	
			Maths::HVec3 vDiff(m_vPosition - m_vPlanePosition);
			Maths::HF32 vGroundDist = vDiff | m_vPlaneNormal;
			Maths::HF32 vTowards = m_vVelocity | m_vPlaneNormal;

			if(vGroundDist < Maths::Constants<Maths::HF32>::Zero() && vTowards < Maths::Constants<Maths::HF32>::Zero())
			{
				Maths::HVec4 vMaterial(Maths::Load<Maths::HVec4>(&materials[0]));
				Maths::HF32 vSurfaceRestitution = vMaterial.X();
				Maths::HF32 vSurfaceFriction = Maths::Constants<Maths::HF32>::One() - m_vTime * vMaterial.Y();

				Maths::HF32 vDot = m_vVelocityTime | m_vPlaneNormal;
				Maths::HF32 vReciprocal = vDot.Reciprocal();
				
				vGroundDist = vGroundDist - 1.192092896e-7f;
				m_vPosition = m_vPosition - (vGroundDist * m_vPlaneNormal);

				vSurfaceRestitution = (vRestitution + vSurfaceRestitution) * Maths::Constants<Maths::HF32>::Half();
				vSurfaceFriction = (vFriction + vSurfaceFriction) * Maths::Constants<Maths::HF32>::Half();

				Maths::HVec3 vNormalVelocity = m_vPlaneNormal * (m_vPlaneNormal | m_vVelocity);
				m_vVelocity = m_vVelocity - (vNormalVelocity * vSurfaceRestitution);
				m_vVelocity = m_vVelocity * vSurfaceFriction;

				if(bDieOnImpact)
					m_vLife = Maths::Constants<Maths::HF32>::One();

				// set collision
				m_iFlags |= PARTICLE_COLLISION;
				if(!(m_iFlags & PARTICLE_COLLISION_ONCE))
					m_iFlags |= PARTICLE_COLLISION_FIRST;
				m_iFlags |= PARTICLE_COLLISION_ONCE;

				if(Maths::Length2(m_vVelocity) < vSleepSpeedSq)
					m_iFlags |= PARTICLE_COLLISION_SLEEP;
			}
		}
	}

	FORCE_INLINE void CreateCollisionPlane(const Maths::HVec3& vPlanePosition, const Maths::HVec3& vPlaneNormal)
	{
		Maths::HVec3 vHvPlanePosition = vPlanePosition + vPlaneNormal * m_vForceSizeLife.Z();
		Maths::Store(vHvPlanePosition, (MathsStorage::Float4A16*)&m_pParticle->vPlanePosition);
		Maths::Store((Maths::V128)vPlaneNormal, &m_pParticle->vPlaneNormal);
	}

	FORCE_INLINE void CreateCollisionPlane(const MathsStorage::Float4A16& vPlanePosition, const MathsStorage::Float4A16& vPlaneNormal)
	{
		Maths::HVec3 v(Maths::Load<Maths::HVec3>(&vPlaneNormal));
		m_pParticle->vPlanePosition = MathsStorage::Float3(vPlanePosition.x, vPlanePosition.y, vPlanePosition.z);
		Maths::Store(v, &m_pParticle->vPlaneNormal);
	}

	Maths::HF32		m_vRotation;
	Maths::HF32		m_vRotationRate;
	Maths::HVec3	m_vVelocityTime;
	Maths::HVec3	m_vPlanePosition;
	Maths::HVec3	m_vPlaneNormal;
};

template<>
class ParticleUpdate<C_Particle3D_RV3D> : public ParticleUpdateBase<C_Particle3D_RV3D>
{
public:
	ParticleUpdate() : ParticleUpdateBase<C_Particle3D_RV3D>() {}
	
	FORCE_INLINE void Load(Maths::HF32& vFrameTime) 
	{
		LoadBase(vFrameTime);
		m_qRotation = Maths::C_Quaternion(Maths::Load<Maths::HVec4>(&m_pParticle->qRotation));
		m_vAngularVelocity = Maths::Load<Maths::HVec3>(&m_pParticle->vAngularVelocity);
	}

	FORCE_INLINE void Store()
	{
		StoreBase();
		Maths::Store(m_qRotation, &m_pParticle->qRotation);
		Maths::Store(m_vAngularVelocity, &m_pParticle->vAngularVelocity);
	}

	FORCE_INLINE void CreateRotation(const MathsStorage::Float4A16& qRotation, const MathsStorage::Float4A16& vAngularVelocity)
	{
		m_pParticle->qRotation = qRotation;
		m_pParticle->vAngularVelocity = vAngularVelocity;
	}

	FORCE_INLINE void CreateRotation(const Maths::C_Quaternion& qRotation, const Maths::HVec3& vAngularVelocity)
	{
		Maths::Store((Maths::V128)Maths::HVec4(qRotation), (MathsStorage::Float4A16*)&m_pParticle->qRotation);
		Maths::Store((Maths::V128)vAngularVelocity, (MathsStorage::Float4A16*)&m_pParticle->vAngularVelocity);
	}

	FORCE_INLINE void UpdateRotation()
	{
		Maths::HVec3 vAngularVelocityTime = m_vAngularVelocity * m_vTime;
		m_qRotation = m_qRotation + m_qRotation * Maths::C_Quaternion(vAngularVelocityTime, Maths::HVecW(0.0f));
		m_qRotation.Normalise();
	}

	Maths::C_Quaternion		m_qRotation;
	Maths::HVec3			m_vAngularVelocity;
};

template<>
class ParticleUpdate<C_Particle3D_RV3D_CP> : public ParticleUpdateBase<C_Particle3D_RV3D_CP>
{
public:
	ParticleUpdate() : ParticleUpdateBase<C_Particle3D_RV3D_CP>() {}

	FORCE_INLINE void Load(Maths::HF32& vFrameTime) 
	{
		LoadBase(vFrameTime);
		m_qRotation = Maths::Load<Maths::C_Quaternion>((MathsStorage::Float4A16*)&m_pParticle->qRotation);
		m_vAngularVelocity = Maths::Load<Maths::HVec3>((MathsStorage::Float4A16*)&m_pParticle->vAngularVelocity);
		m_vPlanePosition = Maths::Load<Maths::HVec3>((MathsStorage::Float4A16*)&m_pParticle->vPlanePosition);
		m_vPlaneNormal = Maths::Load<Maths::HVec3>(&m_pParticle->vPlaneNormal);
		m_iFlags &= ~PARTICLE_COLLISION;
		m_iFlags &= ~PARTICLE_COLLISION_FIRST;
	}

	FORCE_INLINE void Store()
	{
		StoreBase();
		Maths::Store(m_qRotation, (MathsStorage::Float4A16*)&m_pParticle->qRotation);
		Maths::Store(m_vAngularVelocity, (MathsStorage::Float4A16*)&m_pParticle->vAngularVelocity);
	}

	FORCE_INLINE void CreateRotation(const MathsStorage::Float4A16& qRotation, const MathsStorage::Float4A16& vAngularVelocity)
	{
		m_pParticle->qRotation = qRotation;
		m_pParticle->vAngularVelocity = vAngularVelocity;
	}

	FORCE_INLINE void CreateRotation(const Maths::C_Quaternion& qRotation, const Maths::HVec3& vAngularVelocity)
	{
		Maths::Store(Maths::HVec4(qRotation), &m_pParticle->qRotation);
		Maths::Store((Maths::V128)vAngularVelocity, &m_pParticle->vAngularVelocity);
	}

	FORCE_INLINE void UpdateRotation()
	{
		Maths::HVec3 vAngularVelocityTime = m_vAngularVelocity * m_vTime;
		m_qRotation = m_qRotation + m_qRotation * Maths::C_Quaternion(vAngularVelocityTime, Maths::HVecW(0.0f));
		m_qRotation.Normalise();
	}

	FORCE_INLINE void UpdatePosition()
	{
		m_vPosition = m_vPosition + m_vVelocityTime;
	}

	FORCE_INLINE void UpdateVelocity(Maths::HF32& vResistance, Maths::HVec3& vAcceleration)
	{
		m_vVelocity = m_vVelocity + vAcceleration;
		m_vVelocity = m_vVelocity * vResistance;
		
		// calc velocity time here cause its used in collision code
		m_vVelocityTime = m_vVelocity * m_vTime;
	}

	FORCE_INLINE void UpdateVelocity(Maths::HF32& vResistance, Maths::HVec3& vAcceleration, Maths::HVec3& vForce)
	{
		m_vVelocity = m_vVelocity + vForce * m_vForceSizeLife.X();
		m_vVelocity = m_vVelocity + vAcceleration;
		m_vVelocity = m_vVelocity * vResistance;
		
		// calc velocity time here cause its used in collision code
		m_vVelocityTime = m_vVelocity * m_vTime;
	}

	FORCE_INLINE void UpdateCollisions(const Maths::HF32& vRestitution, const Maths::HF32& vFriction, const bool bDieOnImpact, const Maths::HF32& vSleepSpeedSq, const MathsStorage::Float4A16* materials)
	{
		if(!(m_iFlags & PARTICLE_COLLISION_NONE))
		{	
			Maths::HVec3 vDiff(m_vPosition - m_vPlanePosition);
			Maths::HF32 vGroundDist = vDiff | m_vPlaneNormal;
			Maths::HF32 vTowards = m_vVelocity | m_vPlaneNormal;

			if(vGroundDist < Maths::Constants<Maths::HF32>::Zero() && vTowards < Maths::Constants<Maths::HF32>::Zero())
			{
				Maths::HVec4 vMaterial(Maths::Load<Maths::HVec4>(&materials[0]));
				Maths::HF32 vSurfaceRestitution = vMaterial.X();
				Maths::HF32 vSurfaceFriction = Maths::Constants<Maths::HF32>::One() - m_vTime * vMaterial.Y();

				Maths::HF32 vDot = m_vVelocityTime | m_vPlaneNormal;
				Maths::HF32 vReciprocal = vDot.Reciprocal();
				
				vGroundDist = vGroundDist - 1.192092896e-7f;
				m_vPosition = m_vPosition - (vGroundDist * m_vPlaneNormal);

				vSurfaceRestitution = (vRestitution + vSurfaceRestitution) * Maths::Constants<Maths::HF32>::Half();
				vSurfaceFriction = (vFriction + vSurfaceFriction) * Maths::Constants<Maths::HF32>::Half();

				Maths::HVec3 vNormalVelocity = m_vPlaneNormal * (m_vPlaneNormal | m_vVelocity);
				m_vVelocity = m_vVelocity - (vNormalVelocity * vSurfaceRestitution);
				m_vVelocity = m_vVelocity * vSurfaceFriction;
				m_vAngularVelocity = m_vAngularVelocity * vSurfaceFriction;

				if(bDieOnImpact)
					m_vLife = Maths::Constants<Maths::HF32>::One();

				m_iFlags |= PARTICLE_COLLISION;
				if(!(m_iFlags & PARTICLE_COLLISION_ONCE))
					m_iFlags |= PARTICLE_COLLISION_FIRST;
				m_iFlags |= PARTICLE_COLLISION_ONCE;

				if(Maths::Length2(m_vVelocity) < vSleepSpeedSq)
					m_iFlags |= PARTICLE_COLLISION_SLEEP;
			}
		}
	}

	FORCE_INLINE void CreateCollisionPlane(const Maths::HVec3& vPlanePosition, const Maths::HVec3& vPlaneNormal)
	{
		Maths::HVec3 vHvPlanePosition = vPlanePosition + vPlaneNormal * m_vForceSizeLife.Z();	
		Maths::Store((Maths::V128)vHvPlanePosition, (MathsStorage::Float4A16*)&m_pParticle->vPlanePosition);
		Maths::Store((Maths::V128)vPlaneNormal, &m_pParticle->vPlaneNormal);
	}

	FORCE_INLINE void CreateCollisionPlane(const MathsStorage::Float4A16& vPlanePosition, const MathsStorage::Float4A16& vPlaneNormal)
	{
		Maths::HVec3 v(Maths::Load<Maths::HVec3>(&vPlaneNormal));
		m_pParticle->vPlanePosition = MathsStorage::Float3(vPlanePosition.x, vPlanePosition.y, vPlanePosition.z);
		Maths::Store(v, &m_pParticle->vPlaneNormal);
	}


	Maths::HVec3			m_vVelocityTime;
	Maths::HVec3			m_vPlanePosition;
	Maths::HVec3			m_vPlaneNormal;
	Maths::C_Quaternion		m_qRotation;
	Maths::HVec3			m_vAngularVelocity;
};


}
}

#endif // RENDER_DYNAMICEFFECT_PARTICLE_UPDATEXM
