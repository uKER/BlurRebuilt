#ifndef ANIMATED_SHADER_PARAMS_CREATE_H
#define ANIMATED_SHADER_PARAMS_CREATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/SharedComponents.h>


namespace Components
{

// component creator structure
class C_AnimatedShaderParamsCreator : public Components::I_GameComponentCreator
{
public:
	C_AnimatedShaderParamsCreator() : I_GameComponentCreator(), m_ModelIndex(0) {}

public:
	static Component::C_ComponentTypeId		ClassType() {return Components::E_AnimatedShaderParams;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_AnimatedShaderParams;}

	U32		m_ModelIndex;
};

}//namespace Components

#endif
