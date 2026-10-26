#ifndef UPDATE_MANAGER_CREATE_H
#define UPDATE_MANAGER_CREATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/SharedComponents.h>

namespace Components
{

// Active Range Update Component
// -----------------------------

class C_ActiveRangeUpdateComponentCreator : public Component::I_ComponentCreator
{
public:
	C_ActiveRangeUpdateComponentCreator():m_ActiveRange(FLT_MAX),m_StartAsleep(false){}
	C_ActiveRangeUpdateComponentCreator(F32 range):m_ActiveRange(range){}
	static Component::C_ComponentTypeId		ClassType() {return Components::E_UpdateManager_ActiveRange;}
	S32				GetComponentTypeID() const {return Components::E_UpdateManager_ActiveRange;}

public:
	F32		m_ActiveRange;
	bool	m_StartAsleep;
};

// Stagger Update Component
// ------------------------

class C_StaggerUpdateComponentCreator : public C_ActiveRangeUpdateComponentCreator
{
public:
	C_StaggerUpdateComponentCreator():C_ActiveRangeUpdateComponentCreator(FLT_MAX){}
	C_StaggerUpdateComponentCreator(F32 range):C_ActiveRangeUpdateComponentCreator(range){}
	static Component::C_ComponentTypeId		ClassType() {return Components::E_UpdateManager_Stagger;}
	S32				GetComponentTypeID() const {return Components::E_UpdateManager_Stagger;}
public:
};

// Counter Update Component
// ------------------------

class C_CounterUpdateComponentCreator : public Component::I_ComponentCreator
{
public:
	C_CounterUpdateComponentCreator():m_UpdateOrderLimit(0){}
	C_CounterUpdateComponentCreator(U32 countLimit):m_UpdateOrderLimit(countLimit){}
	static Component::C_ComponentTypeId		ClassType() {return Components::E_UpdateManager_Counter;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_UpdateManager_Counter;}
public:
	U32		m_UpdateOrderLimit;
	S32		m_GroupID;
};

// Update Seeds
// ------------

class C_UpdateSeedSingularCreator : public Component::I_ComponentCreator
{
public:
	C_UpdateSeedSingularCreator(){}
	static Component::C_ComponentTypeId		ClassType() {return Components::E_UpdateSeed_Singular;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_UpdateSeed_Singular;}
public:
};

}	// namespace Components

#endif
