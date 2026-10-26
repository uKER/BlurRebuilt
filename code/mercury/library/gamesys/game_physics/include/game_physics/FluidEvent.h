#ifndef GAMEPHYSICS_FLUIDEVENT_H
#define GAMEPHYSICS_FLUIDEVENT_H

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

struct C_FluidEvent
{
	Maths::HVec3		submergedCentroid;
	Maths::HVec3		surfacePoint;
	F32					submergedVolume;
	Gravity::C_Body*	pBody;
	C_BodyUserData*		pData;
};

} // namespace GamePhysics


#endif //  GAMEPHYSICS_FLUIDEVENT_H
