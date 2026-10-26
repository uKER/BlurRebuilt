#ifndef I_UPDATE_MANAGER_H
#define I_UPDATE_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/SharedComponents.h>

namespace Components
{

	// Update Manager Interface
	class I_UpdateManagerComponent : public Actor::I_ActorComponent
	{
	public:
		static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_UpdateManager;}
		virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_UpdateManager;}
		virtual bool	OnMessage(const Messaging::C_MessageBase *pMessage ) {return Actor::I_ActorComponent::OnMessage(pMessage);}	

		// Interface Methods
		virtual F32		GetDistanceFromSeed() const = 0;
	};

	// Update Seed Interface
	class I_UpdateSeedComponent : public Actor::I_ActorComponent
	{
	public:
		static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_UpdateSeed;}
		virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_UpdateSeed;}
		virtual bool	OnMessage(const Messaging::C_MessageBase *pMessage ) {return Actor::I_ActorComponent::OnMessage(pMessage);}	

	};
}

#endif

