#ifndef I_RENDER_COMPONENTS_H
#define I_RENDER_COMPONENTS_H

#ifdef _MSC_VER
#pragma once
#endif

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
#if 0
// Render component Interface
class I_RenderComponent : public I_GameActorComponent
{
public:
	static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_Render;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_Render;}
	virtual bool	OnMessage(const Messaging::C_MessageBase *pMessage ) {return I_GameActorComponent::OnMessage(pMessage);}	

	// render interface
	virtual const Maths::HMat43	&	GetRenderMatrix() const = 0;
	virtual const Maths::C_Sphere &	GetLocalBoundingSphere() const= 0;
};

// Model Resource Component Interface
class I_ModelResource : public I_GameActorComponent
{
public:
	static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_ModelResource;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_ModelResource;}
	virtual bool	OnMessage(const Messaging::C_MessageBase *pMessage ) {return I_GameActorComponent::OnMessage(pMessage);}	

	virtual const WorldRender::C_RenderModelData *	GetRenderModelData(void) const = 0;
	virtual const GameResource::C_ModelData *		GetModelData(void) const = 0;
};
#endif

#if 0
// Render component Interface
class I_PortalBlocker : public I_GameActorComponent
{
public:
	static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_PortalBlocker;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_PortalBlocker;}
	virtual bool	OnMessage(const Messaging::C_MessageBase *pMessage ) {return I_GameActorComponent::OnMessage(pMessage);}	

};
#endif
} //namespace Components

#endif