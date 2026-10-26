#ifndef COMPOSITESHAPE_H
#define	COMPOSITESHAPE_H


#include "gravity/narrowphase/Shapes.h"

namespace Collision
{
template <int MaxPatches, int MaxContacts>
bool CollideCompositeShape(C_CollisionManager& manager, const C_Shape* const  compositeShape, const C_Shape* const shape, C_CollisionResult<MaxPatches, MaxContacts>& results,
						   SharedPhysics::C_SafeStack<true>& stack, EPA& m_Epa, bool bFlip, Collision::C_ContactPatch<MaxContacts>& patch);


};

#include "gravity/narrowphase/CompositeShapeCollision.inl"

#endif
