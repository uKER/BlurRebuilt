#ifndef I_ANIMATEDSHADERPARAMS_H
#define I_ANIMATEDSHADERPARAMS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorComponent.h>
#include <shared_actors/Components/SharedComponents.h>

namespace WorldRender
{
	class C_ModelInstance;
}


namespace Components
{

class I_AnimatedShaderParams : public Actor::I_ActorComponent
{
public:
	static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_AnimatedShaderParams;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_AnimatedShaderParams;}
	virtual bool	OnMessage(const Messaging::C_MessageBase *pMessage ) {return Actor::I_ActorComponent::OnMessage(pMessage);}	
		
	virtual void SetMaterialParameters(WorldRender::C_ModelInstance* pModelInstance) const = 0;
};

}

#endif