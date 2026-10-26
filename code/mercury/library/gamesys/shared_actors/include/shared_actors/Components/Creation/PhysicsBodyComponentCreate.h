#ifndef PHYSICS_COMPONENT_CREATE_H
#define PHYSICS_COMPONENT_CREATE_H

#include <shared_actors/Components/SharedComponents.h>
#include <shared_actors/ActorPhysics.h>
#include <Gravity/narrowphase/CollisionFilter.h>
#include <Gravity/sharedphysics/GraphNode.h>

namespace BasePhysicalObject
{
	struct C_ElementObjectRecord;
}

namespace ResourceUtils
{
	struct C_GameModel;
}


namespace Components
{

///////////////////////////////////////////////////////////////////////////////////////////////////
// Physics body component creation info.
//
class C_PhysicsBodyCreateInfo : public I_GameComponentCreator
{
public:
	/// Level of contact events required.
	enum E_ContactEventLevel
	{
		E_ContactEventLevel_None,			///< no contact events
		E_ContactEventLevel_OnlyCollision,	///< adds direct collision contacts
		E_ContactEventLevel_NotResting,		///< adds separating and sliding contacts
		E_ContactEventLevel_All,			///< adds resting contacts
		E_ContactEventLevel_Count
	};

	C_PhysicsBodyCreateInfo();
	virtual ~C_PhysicsBodyCreateInfo();

	static Component::C_ComponentTypeId		ClassType() {return E_PhysicsBody;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return E_PhysicsBody;}

	virtual void					Serialise(Sys::C_ContextualStream& stream) const;
	virtual void					Deserialise(Sys::C_ContextualStream& stream);

public:
	Maths::HMat43					m_Matrix;	// RENDER matrix.
	Maths::HVec3					m_CentreOfMassOffset;
	Collision::C_CollisionFilter	m_CollisionFilter;
	SharedPhysics::C_Deactivator	m_Deactivator;
	ActorPhysics::TBuoyancyCoeffs	m_BuoyancyCoeffs;
	ActorPhysics::TBodyType			m_BodyType;
	S32								m_OverrideCollisionFilterIndex;
	F32								m_CollisionInfiniteMassRatio; // The heavier body is treated as having infinite mass in collisions with mass ratio >= this.
	bool							m_OverrideBodyFlagsAndMask;
	bool							m_ContinuousCollision;
	bool							m_IsConveyorBelt;
	bool							m_bFluidEvents;
	F32								m_MinCollisionImpulse;
	E_ContactEventLevel				m_ContactEventLevel;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// Physical object physics body component creation info.
//
class C_BasePhysicalObjectPhysicsBodyCreateInfo : public C_PhysicsBodyCreateInfo
{
public: // From I_GameComponentCreator
	C_BasePhysicalObjectPhysicsBodyCreateInfo() {}
	static Component::C_ComponentTypeId ClassType() {return E_PhysObjPhysicsBody;}
	Component::C_ComponentTypeId GetComponentTypeID() const {return E_PhysObjPhysicsBody;}

	void Serialise(Sys::C_ContextualStream& stream) const {}
	void Deserialise(Sys::C_ContextualStream& stream) {}

public:
	typedef Btl::Vector<S32> TModelElemDescList;

	XtHandle<BasePhysicalObject::C_ElementObjectRecord> m_ElementObject;
	XtHandle<ResourceUtils::C_GameModel> m_Model;
	TModelElemDescList m_ElementDescs; // Descriptor for elements to use when creating shape.
};


} // namespace Components


#endif
