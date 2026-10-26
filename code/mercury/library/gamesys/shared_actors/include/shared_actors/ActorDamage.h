#ifndef DAMAGEDATA_H
#define DAMAGEDATA_H

#ifdef _MSC_VER
#pragma once
#endif


#include <actor_system/ActorDeclare.h>


namespace Sys
{
	class C_ContextualStream;
}


namespace ActorDamage
{

enum E_DamageClass
{
	E_DamageClass_Generic = 0,
	E_DamageClass_Projectile,
	E_DamageClass_Explosion,
	E_DamageClass_Impact,
	E_DamageClass_Melee,
	E_DamageClass_Subdue,

	E_DamageClass_Count
};

enum E_DamageType	// changed to damage type
{
	E_Damage_Generic = 0,
	E_Damage_Bullet,		// | Should these be joined through E_Damage_Projectile
	E_Damage_Shrapnel,		// | Should these be joined through E_Damage_Projectile
	E_Damage_GenericExplosion,
	E_Damage_GrenadeExplosion,
	E_Damage_VehicleExplosion,
	E_Damage_Vehicle,
	E_Damage_WorldImpact,
	E_Damage_Electric,
	E_Damage_Melee,
	E_Damage_MeleeGlancing,

	E_Damage_Fluster,	// Enemies get 'flustered'

	E_DamageType_Count,
};

//inline bool IsDamageAreaEffect(E_DamageType type) {return type==E_Damage_RadialExplosion;}


class C_DamageData
{
public:
	C_DamageData();
	C_DamageData(S32 messageType, U32 messageSize);
	virtual ~C_DamageData();
	
	static E_DamageClass	GetDamageClass(E_DamageType damageType);

	E_DamageClass	GetDamageClass(void) const { return GetDamageClass(m_eDamageType);}

	bool IsDamageAreaEffect(void) const {return GetDamageClass() == E_DamageClass_Explosion;}

	void Serialise(Sys::C_ContextualStream& s);
	void Deserialise(Sys::C_ContextualStream& s);
	
	Maths::HVec3 HitPosition() const {Maths::HVec3 hitPos(Maths::Load<Maths::HVec3>(&m_HitPosition)); return hitPos;}
	void SetHitPosition(const Maths::HVec3& hitPos) {Maths::Store(hitPos, &m_HitPosition);}

	Maths::HVec3 HitNormal() const {Maths::HVec3 hitNormal(Maths::Load<Maths::HVec3>(&m_HitNormal)); return hitNormal;}
	void SetHitNormal(const Maths::HVec3& hitNormal) {Maths::Store(hitNormal, &m_HitNormal);}

	Maths::C_ImpulseKgMpS HitImpulse() const {Maths::C_ImpulseKgMpS impulse(Maths::Load<Maths::HVec3>(&m_HitImpulse)); return impulse;}
	void SetHitImpulse(const Maths::C_ImpulseKgMpS& impulse) {Maths::Store(Maths::HVec3(impulse.Get()), &m_HitImpulse);}

public:
	//TODO expand this 
	E_DamageType			m_eDamageType;
	F32						m_DamageAmount;
	S32						m_ShapePrimitiveIndex;	// primitive of shape hit
	S32						m_PrimitiveIndex;
	S32						m_ExplosionReceptorIndex;
	U32						m_AmmunitionID;
	bool					m_IsFromScriptedWeapon;

	F32						m_ExplosionRadius;
	F32						m_ExplosionDamage;

	Actor::C_ActorHandle	m_CharacterHandle;
	Actor::C_ActorHandle	m_OtherActorHandle;		//Two objects required for a collision and therefore damage, this is the other!
	bool					m_ForceIndestructibleBase;

private:
	MathsStorage::Float3	m_HitPosition;
	MathsStorage::Float3	m_HitNormal;
	MathsStorage::Float3	m_HitImpulse;	
};


class C_DamageInfo
{
public:
	C_DamageInfo();
	~C_DamageInfo();

	template <typename Serial> 
	void Serialise(Serial& s)
	{
		s << m_AngVelocity;
		s << m_LinearMomentum;
		s << m_LinearVelocity;
	}

	template <typename Serial>
	void Deserialise(Serial& s)
	{
		s >> m_AngVelocity;
		s >> m_LinearMomentum;
		s >> m_LinearVelocity;
	}

	void SetAngularVelocity(const Maths::C_AngularVelocityRpS &angVel) {Maths::Store(angVel.Get(), &m_AngVelocity);}
	void SetLinearMomentum(const Maths::C_MomentumKgMpS &mom){Maths::Store(mom.Get(), &m_LinearMomentum);}
	void SetLinearVelocity(const Maths::C_VelocityMpS &linVel){Maths::Store(linVel.Get(), &m_LinearVelocity);}
	void SetForceIndestructibleBase(bool bIndestructibleBase);

	Maths::C_AngularVelocityRpS GetAngularVelocity() const {return Maths::Load<Maths::HVec3>(&m_AngVelocity);}
	Maths::C_MomentumKgMpS		GetLinearMomentum() const {return Maths::Load<Maths::HVec3>(&m_LinearMomentum);}
	Maths::C_VelocityMpS		GetLinearVelocity() const {return Maths::Load<Maths::HVec3>(&m_LinearVelocity);}
private:
	MathsStorage::Float3		m_AngVelocity;
	MathsStorage::Float3		m_LinearMomentum;
	MathsStorage::Float3		m_LinearVelocity;
};

} // namespace ActorDamage

#endif
