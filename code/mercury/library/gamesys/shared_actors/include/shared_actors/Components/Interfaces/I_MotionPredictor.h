#ifndef MERCURY_I_MOTIONPREDICTOR_H
#define MERCURY_I_MOTIONPREDICTOR_H

#ifdef _MSC_VER
#pragma once
#endif


#include "Shared_Actors/Components/SharedComponents.h"


namespace Components
{

class I_MotionPredictor : public Actor::I_ActorComponent
{
public:
	static Component::C_ComponentInterfaceId	InterfaceType() {return E_IF_MotionPredictor;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return E_IF_MotionPredictor;}

	virtual Maths::C_Vertex CalcFuturePosition(const F32 timeStep) const = 0;

};

} // namespace Components


#endif // I_MOTIONPREDICTOR_EEA1C048_252A_43A2_8C28_988E6FF754CF_H
