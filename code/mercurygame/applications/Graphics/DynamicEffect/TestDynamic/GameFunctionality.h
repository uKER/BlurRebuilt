#ifndef GAME_FUNCTIONALITY_H
#define GAME_FUNCTIONALITY_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Dynamic/Node.h"
#include "btl/containers/list.h"
#include "btl/allocators/pool.h"

// projectile
class C_Projectile : public Btl::ListElementDouble<C_Projectile>
{
public:
	C_Projectile() {};
	~C_Projectile() {};

	void						Initialise(const Maths::HVec3& vVelocity, const DynamicEffect::C_Node& transform);
	void						Update(F32 fFrameTime);
	void						Render() const;

	const DynamicEffect::C_Node*	GetNode() { return &m_Transform; }
	Maths::HVec3				GetPosition() { return Maths::Load<Maths::HVec3>(&m_Transform.GetPosition()); }

private:
	Maths::HVec3				m_vVelocity;
	
	DynamicEffect::C_Node		m_Transform;
};

typedef Btl::List<C_Projectile>													ProjectileList;
typedef Btl::StaticPool<C_Projectile, 20>											ProjectilePool;

#endif // GAME_FUNCTIONALITY_H
