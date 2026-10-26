#ifndef SAMPLES_GAME_FUNCTIONALITY_H
#define SAMPLES_GAME_FUNCTIONALITY_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Dynamic/Node.h"

namespace Samples
{

// projectile
class C_Projectile : public Ut::ListElementDouble<C_Projectile>
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

typedef Ut::UtList<C_Projectile, Ut::DoubleEndedList, Ut::MaintainedCounter>		ProjectileList;
typedef Ut::StaticPool<C_Projectile, 20>											ProjectilePool;

}; // namespace Samples

#endif // SAMPLES_GAME_FUNCTIONALITY_H
