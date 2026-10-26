
#ifndef I_ELEMENTMANAGER_H
#define I_ELEMENTMANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorComponent.h>
#include <shared_actors/Components/SharedComponents.h>
#include <shared_actors/ActorDef/BasePhysicalObjectDef.h>

namespace Messaging { class C_MessageBase; }
namespace Gravity	{ class C_ImportShape; }
namespace GameResource	{ class C_ModelElement; }

namespace Components
{

//////////////////////////////////////////////////////////////////////////
// Basic element state type
// This is just a basic type for the render functions and can be expanded upon
class C_ElementState
{
public:
	Maths::HMat43	m_Matrix;
	S32				m_ModelElement;

	typedef Btl::Vector<C_ElementState>	C_List;
	typedef C_List::ConstIterator		C_ConstIterator;

	//const C_List &			GetChildList() const {return m_Children;}	
	S32						GetChildCount() const	{return m_Children.Size();} 
	const C_ElementState &	GetChild(S32 index) const {return m_Children[index];}
private:
	C_List	m_Children;
};


//////////////////////////////////////////////////////////////////////////
// Render component Interface
class I_ElementManager : public Actor::I_ActorComponent
{
public:
	static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_ElementManager;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_ElementManager;}
	bool OnMessage(const Messaging::C_MessageBase *pMessage ) {return Actor::I_ActorComponent::OnMessage(pMessage);}	

	virtual const S32				GetElementCount() const	= 0; 
	virtual C_ElementState			GetElement(S32 index) const	= 0;
};


//////////////////////////////////////////////////////////////////////////
// Simple Element Manager
// ----------------------
// Manages an object with many elements but doesn't have a damage functionality
// TODO: Implement!


//////////////////////////////////////////////////////////////////////////
// Damageable element manager.
// Manages an object with a list of elements that can be damaged
//
class I_DamageableElementManager : public I_ElementManager
{
public:
	static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_DamageableElementManager;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_DamageableElementManager;}

	virtual S32 ActiveElementCount() const = 0;
	virtual S32 ElementIndexFromActiveElementIndex(S32 n) const=0;
	virtual S32 ElementCount() const = 0;
	virtual const GameResource::C_ModelElement* FindModelElement(const Util::C_String& elementName) const=0;
	virtual const GameResource::C_ModelElement* FindModelElement(S32 modelElementIndex) const=0;
	virtual F32 ElementStrength(S32 elementIndex) const = 0;
	virtual const char* ElementName(S32 elementIndex) const=0;
	virtual S32 ElementDamageLevelCount(S32 elementIndex) const = 0;
	virtual S32 ElementDamageLevel(S32 elementIndex) const  = 0;
	virtual const Maths::HMat43& ElementMatrix(S32 elementIndex) const = 0;
	virtual S32 ElementIndex(const Util::C_String& elementName) const = 0;
	virtual bool IsElementIndestructible(S32 elementIndex) const  = 0;
	virtual bool GetElementCollisionLocalPos(S32 elementIndex, Maths::HVec3& localPos) const = 0;
	virtual S32 ElementIndexFromFlatPrimitiveIndex(S32 primIndex) const = 0;
	virtual S32 PrimitiveIndexFromElementIndex(S32 elementIndex) const = 0;
	virtual bool ElementIsActive(S32 elementIndex) const = 0;
	virtual const BasePhysicalObject::TElementDefList& FlatElementDefs() const = 0;
	virtual const Gravity::C_ImportShape* CollisionShape(S32 elementIndex) const = 0;
	virtual const Gravity::C_ImportShape* ActiveCollisionShape(S32 elementIndex) const = 0;
	virtual bool ContainsPivotedElement() const = 0;
	virtual F32 TotalInflictedDamage() const=0;
};

} // namespace Components

#endif
