#ifndef I_MODELCUSTOMISER_6B188EDD_9ABB_4A44_B956_A3529D2F8C48_H
#define I_MODELCUSTOMISER_6B188EDD_9ABB_4A44_B956_A3529D2F8C48_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorComponent.h>
#include <shared_actors/components/SharedComponents.h>


namespace GameResource
{
	class C_Model;
}

namespace WorldRender
{
	class C_Model;
	class C_ModelInstance;
}


namespace Components
{

class I_ModelCustomiser : public Actor::I_ActorComponent
{
public:
	ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE()

public:
	I_ModelCustomiser();
	virtual ~I_ModelCustomiser();

	static Component::C_ComponentInterfaceId	InterfaceType() {return E_IF_ModelCustomiser;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return E_IF_ModelCustomiser;}

	virtual void CustomiseModel(WorldRender::C_ModelInstance* pModelInstance, const GameResource::C_Model* pModelData, const WorldRender::C_Model* pRenderData) const = 0;

};

} // namespace Components


#endif // I_MODELCUSTOMISER_6B188EDD_9ABB_4A44_B956_A3529D2F8C48_H
