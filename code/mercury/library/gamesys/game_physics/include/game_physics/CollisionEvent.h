#ifndef GAMEPHYSICS_COLLISIONEVENT_H
#define GAMEPHYSICS_COLLISIONEVENT_H

#ifdef _MSC_VER
#pragma once
#endif


namespace Gravity
{
	class C_Body;
}

namespace GamePhysics
{

class C_BodyUserData;

struct C_CollisionEvent
{
	Maths::HVec3		position;
	Maths::HVec3		normal;
	Maths::HVec3		friction;
	Gravity::C_Body*	pBodyA;
	Gravity::C_Body*	pBodyB;
	C_BodyUserData*		pDataA;
	C_BodyUserData*		pDataB;
	U32					effectMaterialA;
	U32					effectMaterialB;
	U32					contactType; // really SharedPhysics::C_ContactType but didn't want to pull in that big header
	F32					impulseMagnitude;
	F32					penetrationDepth;
};

} // namespace GamePhysics


#endif // GAMEPHYSICS_COLLISIONEVENT_H
