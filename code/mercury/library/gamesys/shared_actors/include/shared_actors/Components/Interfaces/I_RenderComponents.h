#ifndef I_RENDER_COMPONENTS_H
#define I_RENDER_COMPONENTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorComponent.h>
#include <World_Render/MaterialParameter.h>

// Forward decs

namespace WorldRender { class C_ModelInstance; }

namespace Components
{

// Render component Interface
class I_RenderComponent : public Actor::I_ActorComponent
{
public:
	static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_Render;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_Render;}
	virtual bool	OnMessage(const Messaging::C_MessageBase *pMessage ) {return Actor::I_ActorComponent::OnMessage(pMessage);}	

	// render interface
	virtual const Maths::HMat43	&	GetRenderMatrix() const = 0;
	virtual const Maths::C_Sphere &	GetLocalBoundingSphere() const= 0;
	virtual F32						GetDrawDistance() const= 0;
	virtual WorldRender::MaterialParameter GetMaterialParameter(const Util::C_String& paramName) const=0;
	virtual WorldRender::C_ModelInstance*	GetModelInstance() const = 0;
};



// Render component Interface
class I_PortalBlocker : public Actor::I_ActorComponent
{
public:
	static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_PortalBlocker;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_PortalBlocker;}

};

} //namespace Components

#endif