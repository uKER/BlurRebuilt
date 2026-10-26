
#ifndef _NIGHTWINGPHYSICSBODYCOMPONENT_H
#define _NIGHTWINGPHYSICSBODYCOMPONENT_H

#include "Game/ActorComponents/GameComponents.h"
#include <shared_actors/Components/Impl/PhysicsBodyComponent.h>
#include <Game/ActorComponents/Creation/PhysicsBodyComponentCreate.h>


namespace Components
{

#if 0
//////////////////////////////////////////////////////////////////////////
//
//
class C_PlatformPhysicsBody : public C_PhysicsBody
{
public:
	// From I_ActorComponent
	static Component::C_ComponentTypeId ClassType() {return E_PlatformPhysicsBody;}
	Component::C_ComponentTypeId GetComponentTypeID() const {return E_PlatformPhysicsBody;}
	const C_PlatformPhysicsBodyCreateInfo* GetCreationData(void) const {return static_cast<const C_PlatformPhysicsBodyCreateInfo*>(GetCreator().GetData());}

private:
	virtual void CreateShape(C_PhysicsBodyShapeDesc& shapeDesc) const; // From C_PhysicsBody

};
#endif


//////////////////////////////////////////////////////////////////////////
//
//
class C_ModelPhysicsBody : public C_PhysicsBody
{
public:
	// From I_ActorComponent
	static Component::C_ComponentTypeId ClassType() {return E_ModelPhysicsBody;}
	Component::C_ComponentTypeId GetComponentTypeID() const {return E_ModelPhysicsBody;}
	const C_ModelPhysicsBodyCreateInfo* GetCreationData(void) const {return static_cast<const C_ModelPhysicsBodyCreateInfo*>(GetCreator().GetData());}

private:
	virtual void CreateShape(C_PhysicsBodyShapeDesc& shapeDesc) const; // From C_PhysicsBody

};


#if 0
//////////////////////////////////////////////////////////////////////////
//
//
class C_DynamicObjectPhysicsBody : public C_PhysicsBody
{
public:
	~C_DynamicObjectPhysicsBody();

public:
	// From I_ActorComponent
	bool InitialiseImpl();
	static Component::C_ComponentTypeId ClassType() {return E_DynObjPhysicsBody;}
	Component::C_ComponentTypeId GetComponentTypeID() const {return E_DynObjPhysicsBody;}
	const C_DynamicObjectPhysicsBodyCreateInfo* GetCreationData(void) const {return static_cast<const C_DynamicObjectPhysicsBodyCreateInfo*>(GetCreator().GetData());}
	bool OnMessage(const Messaging::C_MessageBase* pMessage);

	void SerialiseForSave(Sys::C_ContextualStream& stream) const;
	void DeserialiseForSave(Sys::C_ContextualStream& stream);
	void SerialiseForSync(Sys::C_ContextualStream& stream) const;
	void DeserialiseForSync(Sys::C_ContextualStream& stream);

protected:
	Components::C_DynamicObjectPhysicsBodyCreateInfo::TModelElemDescList m_ModelElemDesc;

private:
	virtual void CreateShape(C_PhysicsBodyShapeDesc& shapeDesc) const; // From C_PhysicsBody

};

#endif

//////////////////////////////////////////////////////////////////////////
//
//
// class C_VehiclePhysicsBody : public C_DynamicObjectPhysicsBody
// {
// public:
// 
// public:
// 	// From I_ActorComponent
// 	bool InitialiseImpl();
// 	static Component::C_ComponentTypeId ClassType() {return E_VehiclePhysicsBody;}
// 	Component::C_ComponentTypeId GetComponentTypeID() const {return E_VehiclePhysicsBody;}
// 	const C_VehiclePhysicsBodyCreateInfo* GetCreationData(void) const {return static_cast<const C_VehiclePhysicsBodyCreateInfo*>(GetCreator().GetData());}
// 
// private:
//	virtual void CreateShape(C_PhysicsBodyShapeDesc& shapeDesc) const; // From C_PhysicsBody
//
// };


} // namespace Components

#endif
