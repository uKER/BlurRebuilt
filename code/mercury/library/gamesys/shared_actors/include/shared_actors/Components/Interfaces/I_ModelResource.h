#ifndef I_MODEL_RESOURCE_H
#define I_MODEL_RESOURCE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorComponent.h>
#include <shared_actors/Components/SharedComponents.h>

// Forward decs
namespace WorldRender
{
	class C_RenderModelData;
};

namespace GameResource
{
	class C_ModelData;
};

namespace Components
{

// Model Resource Component Interface
class I_ModelResource : public Actor::I_ActorComponent
{
public:
	ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE()

public:
	I_ModelResource();
	virtual ~I_ModelResource();

	static Component::C_ComponentInterfaceId	InterfaceType() {return E_IF_ModelResource;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return E_IF_ModelResource;}
	
	virtual bool	OnMessage(const Messaging::C_MessageBase* pMessage);

	virtual const WorldRender::C_RenderModelData *	GetRenderModelData(void) const = 0;
	virtual const GameResource::C_ModelData *		GetModelData(void) const = 0;

#ifndef CFG_GOLD
	virtual bool HasModelResourceUsageDetails() const = 0;
#endif

};

}//namespace Components

#endif