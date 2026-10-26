#ifndef TRIGGERCREATE_65A27004_2BE0_4763_A8AE_F7C284AC796B_H
#define TRIGGERCREATE_65A27004_2BE0_4763_A8AE_F7C284AC796B_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/GameComponent.h>
#include <shared_actors/components/TriggerComponents.h>


namespace Components
{

class I_VolumeTriggerCreate : public Component::I_ComponentCreator
{
public:
	/// Flag bits
	enum E_Flag
	{
		E_Flag_HandleEnterExit =	1 << 0,		///< notify when actors enter and exit the trigger
		E_Flag_HandleCrossing =		1 << 1,		///< notify when actors cross the trigger
		E_Flag_OneShot =			1 << 2,		///< only notify for first event
		E_Flag_StartInactive =		1 << 3,		///< wait for Activate message before notifying
	};

	virtual ~I_VolumeTriggerCreate();

	bool TestFlag(const E_Flag flag) const {return (m_Flags & flag) != 0;}
	void SetFlag(const E_Flag flag) {m_Flags |= flag;}
	void ClearFlag(const E_Flag flag) {m_Flags &= ~flag;}

	virtual void Serialise(Sys::C_ContextualStream& stream) const;	// from Component::I_ComponentCreator
	virtual void Deserialise(Sys::C_ContextualStream& stream);		// from Component::I_ComponentCreator

protected:
	explicit I_VolumeTriggerCreate(const U32 flags = 0);

private:
	U32 m_Flags; ///< E_Flag bitfield
public:
	U32		m_TriggerLayerIndex;	///< Gravity Trigger Layer to use
	U32		m_TriggerMask;			///< For Gravity collision filter

};

/// Creation data for C_BoxTrigger
class C_BoxTriggerCreate : public I_VolumeTriggerCreate
{
public:
	static Component::C_ComponentTypeId ClassType() {return E_Trigger_Box;}
	virtual Component::C_ComponentTypeId GetComponentTypeID() const {return E_Trigger_Box;}

	Maths::HVec3		m_Size;
	Maths::HMat43		m_Matrix;

	C_BoxTriggerCreate();
	virtual ~C_BoxTriggerCreate();

#ifndef CFG_GOLD
	virtual void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr, Maths::HMat43 &actorMatrix) const; // from Component::I_ComponentCreator
#endif

	virtual void Serialise(Sys::C_ContextualStream& stream) const;	// from Component::I_ComponentCreator
	virtual void Deserialise(Sys::C_ContextualStream& stream);		// from Component::I_ComponentCreator

};

/// Creation data for C_SphereTrigger
class C_SphereTriggerCreate : public I_VolumeTriggerCreate
{
public:
	static Component::C_ComponentTypeId ClassType() {return E_Trigger_Sphere;}
	virtual Component::C_ComponentTypeId GetComponentTypeID() const {return E_Trigger_Sphere;}

	Maths::C_Sphere		m_Sphere;

	C_SphereTriggerCreate();
	virtual ~C_SphereTriggerCreate();

#ifndef CFG_GOLD
	virtual void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr, Maths::HMat43 &actorMatrix) const; // from Component::I_ComponentCreator
#endif

	virtual void Serialise(Sys::C_ContextualStream& stream) const;	// from Component::I_ComponentCreator
	virtual void Deserialise(Sys::C_ContextualStream& stream);		// from Component::I_ComponentCreator

};

// Trigger Source Component
// ------------------------
// This is component is used to activate triggers
// eg. It will be attached to players to make them activate box triggers

class I_TriggerSourceCreate : public Component::I_ComponentCreator
{
public:

	I_TriggerSourceCreate():m_TriggerLayerIndex(0),m_TriggerFlags(0xffffffff){}

	virtual void Serialise(Sys::C_ContextualStream& stream) const;	// from Component::I_ComponentCreator
	virtual void Deserialise(Sys::C_ContextualStream& stream);		// from Component::I_ComponentCreator

	U32		m_TriggerLayerIndex;
	U32		m_TriggerFlags;

};

// Sphere shaped trigger source
class C_TriggerSourceSphereCreate : public I_TriggerSourceCreate
{
public:
	static Component::C_ComponentTypeId ClassType() {return E_TriggerSource_Sphere;}
	virtual Component::C_ComponentTypeId GetComponentTypeID() const {return E_TriggerSource_Sphere;}

	virtual void Serialise(Sys::C_ContextualStream& stream) const;	// from Component::I_ComponentCreator
	virtual void Deserialise(Sys::C_ContextualStream& stream);		// from Component::I_ComponentCreator

	Maths::C_Sphere		m_Sphere;

};

// box shaped trigger source
class C_TriggerSourceBoxCreate : public I_TriggerSourceCreate
{
public:
	static Component::C_ComponentTypeId ClassType() {return E_TriggerSource_Box;}
	virtual Component::C_ComponentTypeId GetComponentTypeID() const {return E_TriggerSource_Box;}

	virtual void Serialise(Sys::C_ContextualStream& stream) const;	// from Component::I_ComponentCreator
	virtual void Deserialise(Sys::C_ContextualStream& stream);		// from Component::I_ComponentCreator

	Maths::HVec3		m_Size;
	Maths::HMat43		m_Matrix;
};


} // namespace Components


#endif // TRIGGERCREATE_65A27004_2BE0_4763_A8AE_F7C284AC796B_H
