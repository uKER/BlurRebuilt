#include "SharedActorsPch.h"
#include <shared_actors/ActorDamage.h>

#include <actor_system/ActorSerialisation.h>
#include <misc_shared/Context/ContextualStream.h>




namespace ActorDamage
{

C_DamageData::C_DamageData()
	: m_eDamageType(E_Damage_Generic)
	, m_DamageAmount(0.0f)
	, m_ShapePrimitiveIndex(-1)
	, m_PrimitiveIndex(-1)
	, m_ExplosionReceptorIndex(-1)
	, m_AmmunitionID(0)
	, m_IsFromScriptedWeapon(false)
	, m_ExplosionRadius(0.0f)
	, m_ExplosionDamage(0.0f)
	, m_ForceIndestructibleBase(false)
{
}

C_DamageData::C_DamageData(S32 messageType, U32 messageSize)
	: m_eDamageType(E_Damage_Generic)
	, m_DamageAmount(0.0f)
	, m_ShapePrimitiveIndex(-1)
	, m_PrimitiveIndex(-1)
	, m_ExplosionReceptorIndex(-1)
	, m_AmmunitionID(0)
	, m_IsFromScriptedWeapon(false)
	, m_ExplosionRadius(0.0f)
	, m_ExplosionDamage(0.0f)
	, m_ForceIndestructibleBase(false)
{
}

C_DamageData::~C_DamageData()
{
}

E_DamageClass C_DamageData::GetDamageClass(E_DamageType damageType) 
{
	switch(damageType)
	{
	case E_Damage_Generic:
		return E_DamageClass_Generic;
		break;
	case E_Damage_Bullet:	
	case E_Damage_Shrapnel:	
		return E_DamageClass_Projectile;
		break;
	case E_Damage_GenericExplosion:
	case E_Damage_GrenadeExplosion:
	case E_Damage_VehicleExplosion:
		return E_DamageClass_Explosion;
		break;
	case E_Damage_Vehicle:
	case E_Damage_WorldImpact:
		return E_DamageClass_Impact;
		break;
	case E_Damage_Electric:
		return E_DamageClass_Generic;
		break;
	case E_Damage_Melee:
	case E_Damage_MeleeGlancing:
	return E_DamageClass_Melee;
		break;
	default:
		return E_DamageClass_Generic;
	}
}

void C_DamageData::Serialise(Sys::C_ContextualStream& s)
{
	s << m_DamageAmount;
	s << m_ShapePrimitiveIndex;
	s << m_PrimitiveIndex;
	s << m_AmmunitionID;
	s << m_IsFromScriptedWeapon;
	s << m_ExplosionRadius;
	s << m_ExplosionDamage;
	s << m_CharacterHandle;
	s.WriteAs<S32>(m_eDamageType);
}

void C_DamageData::Deserialise(Sys::C_ContextualStream& s)
{
	s >> m_DamageAmount;
	s >> m_ShapePrimitiveIndex;
	s >> m_PrimitiveIndex;
	s >> m_AmmunitionID;
	s >> m_IsFromScriptedWeapon;
	s >> m_ExplosionRadius;
	s >> m_ExplosionDamage;
	s >> m_CharacterHandle;
	m_eDamageType = E_DamageType(s.ReadAs<S32>());
}



C_DamageInfo::C_DamageInfo()
{
}

C_DamageInfo::~C_DamageInfo()
{
}

} // namespace ActorDamage
