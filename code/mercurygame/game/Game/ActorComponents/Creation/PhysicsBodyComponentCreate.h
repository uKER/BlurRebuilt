#ifndef _NIGHTWINGPHYSICSBODYCOMPONENTCREATE_H
#define _NIGHTWINGPHYSICSBODYCOMPONENTCREATE_H

#include "Game/ActorComponents/GameComponents.h"

//#include <GameWorld/PhysicsWorldAux.h>
#include <shared_actors/Components/Creation/PhysicsBodyComponentCreate.h>
//#include <GameDef/DynamicObjectDef.h>
//#include <GameDef/VehicleActorDef.h>


namespace Components
{
#if 0
///////////////////////////////////////////////////////////////////////////////////////////////////
// Platform physics body component creation info.
//
class C_PlatformPhysicsBodyCreateInfo : public C_PhysicsBodyCreateInfo
{
public:

public: // From I_GameComponentCreator
	static Component::C_ComponentTypeId ClassType() {return E_PlatformPhysicsBody;}
	Component::C_ComponentTypeId GetComponentTypeID() const {return E_PlatformPhysicsBody;}

	void Serialise(Sys::C_ContextualStream& stream) const;
	void Deserialise(Sys::C_ContextualStream& stream);

public:
	S32 m_PlatformId;
};
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Model physics body component creation info.
//
class C_ModelPhysicsBodyCreateInfo : public C_PhysicsBodyCreateInfo
{
public: // From I_GameComponentCreator
	static Component::C_ComponentTypeId ClassType() {return E_ModelPhysicsBody;}
	Component::C_ComponentTypeId GetComponentTypeID() const {return E_ModelPhysicsBody;}

	void Serialise(Sys::C_ContextualStream& stream) const;
	void Deserialise(Sys::C_ContextualStream& stream);

public:
	//XtHandle<ResourceUtils::C_GameModel> m_Model;		// Model shape is constructed from.
	S32 m_ModelIndex;
	S32 m_ModelElementIndex;
	F32 m_Mass;
};


#if 0
///////////////////////////////////////////////////////////////////////////////////////////////////
// Dynamic object physics body component creation info.
//
class C_DynamicObjectPhysicsBodyCreateInfo : public C_PhysicsBodyCreateInfo
{
public:

public: // From I_GameComponentCreator
	static Component::C_ComponentTypeId ClassType() {return E_DynObjPhysicsBody;}
	Component::C_ComponentTypeId GetComponentTypeID() const {return E_DynObjPhysicsBody;}

	void Serialise(Sys::C_ContextualStream& stream) const;
	void Deserialise(Sys::C_ContextualStream& stream);

	const XtArray<C_DynObjectElementDef>& ElementDefs(void) const;
	const XtHandle<ResourceUtils::C_GameModel>& Model(void) const;

public:
	typedef Btl::Vector<S32> TModelElemDescList;

	XtHandle<C_DynamicObjectDef> m_DynObjDef;
	XtHandle<C_VehicleActorDef> m_VehicleActorDef;
	TModelElemDescList m_ElementDescs; // Descriptor for elements to use when creating shape.
};
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vehicle physics body component creation info.
//
// class C_VehiclePhysicsBodyCreateInfo : public C_PhysicsBodyCreateInfo
// {
// public:
// 
// public: // From I_GameComponentCreator
// 	static Component::C_ComponentTypeId ClassType() {return E_VehiclePhysicsBody;}
// 	Component::C_ComponentTypeId GetComponentTypeID() const {return E_VehiclePhysicsBody;}
// 
// 	//void Serialise(Sys::C_ContextualStream& stream) const;
// 	//void Deserialise(Sys::C_ContextualStream& stream);
// 
// public:
// 	typedef Btl::Vector<S32> TModelElemDescList;
// 	XtHandle<C_VehicleActorDef> m_VehicleDef;
// 	TModelElemDescList		m_ElementDescs; // Descriptors for model elements to use when creating shape.
// };


} // namespace Components

#endif
