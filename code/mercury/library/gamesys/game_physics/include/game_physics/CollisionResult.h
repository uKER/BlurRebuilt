#ifndef GAMEPHYSICS_COLLISIONRESULT_H
#define GAMEPHYSICS_COLLISIONRESULT_H

#ifdef _MSC_VER
#pragma once
#endif


namespace Gravity
{
	class C_Body;
}

namespace SharedPhysics
{
	class C_ContactStreamIterator;

	struct C_ContactStreamResult;
}


namespace GamePhysics
{

class C_BodyUserData;

struct C_CollisionResult
{
	Gravity::C_Body*						pBodyA;
	Gravity::C_Body*						pBodyB;
	C_BodyUserData*							pDataA;
	C_BodyUserData*							pDataB;
	SharedPhysics::C_ContactStreamIterator*	pIter;
	SharedPhysics::C_ContactStreamResult*	pResult;
};

} // namespace GamePhysics


#endif // GAMEPHYSICS_COLLISIONRESULT_H
