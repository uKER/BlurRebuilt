#ifndef I_ELEMENT_MANAGER_H
#define I_ELEMENT_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Game/ActorComponents/GameComponents.h"

namespace Messaging
{
	class C_MessageBase;
}


namespace Components
{

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

// Render component Interface
//class I_ElementManager : public I_GameActorComponent
//{
//public:
//	static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_ElementManager;}
//	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_ElementManager;}
//	virtual bool	OnMessage(const Messaging::C_MessageBase *pMessage ) {return I_GameActorComponent::OnMessage(pMessage);}	
//
//	// interface methods
//	//virtual const C_ElementState::C_List	&GetElementList() const = 0;
//
//	virtual const S32				GetElementCount() const	= 0; 
//	virtual C_ElementState			GetElement(S32 index) const	= 0;
//
//};

// Simple Element Manager
// ----------------------
// Manages an object with many elements but doesn't have a damage functionality
// TODO: Implement!

// Damageable Element Manager
// --------------------------
// Managed an object with a list of elements that can be damaged
// eg. Dynamic Objects, Vehicles
// TODO: Implement!

}

#endif

