
#ifndef _PHYSICSBODYCOMPONENT_H
#define _PHYSICSBODYCOMPONENT_H

#include <gravity/sdk/Body.h>
#include <shared_actors/Components/SharedComponents.h>
#include <shared_actors/Components/Creation/PhysicsBodyComponentCreate.h>
#include <shared_actors/Components/Interfaces/I_PhysicsBodyComponents.h>


namespace Actors
{
	class C_PBCUserData;
}

namespace Gravity
{
	class C_BodyEvent;
	class C_KinematicController;
	class C_ShapeDesc;
}


namespace Components
{

	class I_DamageableElementManager;

class C_PhysicsBodyShapeDesc
{
public:
	C_PhysicsBodyShapeDesc();
	C_PhysicsBodyShapeDesc(const C_PhysicsBodyShapeDesc& copy);

	~C_PhysicsBodyShapeDesc();

	C_PhysicsBodyShapeDesc& operator = (const C_PhysicsBodyShapeDesc& copy);

	void Clear();

	void Set(const F32 mass, Gravity::C_ShapeDesc* pShapeDesc);

	F32 GetMass() const {return m_Mass;}
	Gravity::C_ShapeDesc* GetShapeDesc() const {return m_pShapeDesc;}

	bool TakeOwnership(Gravity::C_ShapeDesc* pShapeDesc);
	void DestroyOwned();

private:
	static const U32 kMaxOwnedShapeDescs = 3; // maximum number of shape descs we've allocated

	F32						m_Mass;
	Gravity::C_ShapeDesc*	m_pShapeDesc;
	Gravity::C_ShapeDesc*	m_ppOwnedShapeDescs [kMaxOwnedShapeDescs];

};


///////////////////////////////////////////////////////////////////////////////////////////////////
// Gravity physics body component class.
//
class C_PhysicsBody : public I_PhysicsBody
{
public:
	ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE()

public:
	C_PhysicsBody();
	~C_PhysicsBody();

public:
	const Gravity::C_Body* CollideBody() const {return m_pCollideBody;} // hide this?
	bool IsBodyEnabled() const {return m_IsEnabled;}
	ActorPhysics::TBodyType BodyType() const {return ActorPhysics::BodyType(CollideBody());}
	Maths::HMat43 Matrix() const;
	Maths::HMat43 RenderMatrix() const;
	Maths::HVec3 ShapeCentroid() const;
	Maths::HF32 Mass() const {return CollideBody()->Mass();}
	Maths::HF32 ReciprocalMass() const {return CollideBody()->ReciprocalMass();}
	Maths::HVec3 LinearMomentum() const {return CollideBody()->LinearVelocity()*Mass();}
	Maths::HVec3 LinearVelocity() const {return CollideBody()->LinearVelocity();}
	Maths::HVec3 AngularVelocity() const {return CollideBody()->AngularVelocity();}
	const Collision::C_CollisionFilter& CollisionFilter() const;
	U32 CollisionGroup() const {return CollisionFilter().GetGroupId();}
	Collision::C_Sphere WorldBoundingSphere() const;
	F32 CollisionInfiniteMassRatio() const {return GetCreationData()->m_CollisionInfiniteMassRatio;}
	void DisableBuoyancy()
	{
		m_pCollideBody->GetGraphNode()->SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_NoBuoyancy);
	}
	bool SetIndexCollisionFlags(Collision::C_Shape* pShape, U32 index, U32 flags, U32 mask);
	void OverrideCollisionFlagsAndMask(Collision::C_Shape* pShape, U32 flags, U32 mask);

public:
	// From I_ActorComponent
	static Component::C_ComponentTypeId		ClassType() {return Components::E_PhysicsBody;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_PhysicsBody;} // from Component::I_GameComponent
	bool OnMessage(const Messaging::C_MessageBase* pMessage);
	const C_PhysicsBodyCreateInfo	*GetCreationData(void) const {return static_cast<const C_PhysicsBodyCreateInfo *>(GetCreator().GetData());}
#ifndef CFG_GOLD
	void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
#endif

	virtual void SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void DeserialiseForSync(Sys::C_ContextualStream& stream);

protected:
	Gravity::C_Scene* PhysicsScene() {return m_PhysicsScene;}
	bool CreateBody();
	Gravity::C_Body* CreateBody(ActorPhysics::TBodyType bodyType, C_PhysicsBodyShapeDesc& shapeDesc, const Maths::HMat43& matrix, const Collision::C_CollisionFilter& filter, const SharedPhysics::C_Deactivator* deactivation);
	void Update(const Maths::C_TimeS& timeStep);
	void OnShapeChanged(S32 overrideIndex);
	void Activate();
	Gravity::C_Body* CollideBody() {return m_pCollideBody;}
	void SetTransform(const Maths::C_QuatTrans& transform);
	bool CanAnimate() const {return m_KinematicController!=NULL;}
	void AnimateTransform(const Maths::C_QuatTrans& transform, F32 timeStep);
	Gravity::C_KinematicController* AddKinematicController(Gravity::C_Body* body);
	void ReleaseKinematicController(Gravity::C_KinematicController*& controller);
	Collision::C_CollisionFilter& CollisionFilter();
	void OnPhysicsBodyEvent(const Gravity::C_BodyEvent& bodyEvent);

	// From Component::I_GameComponent	
	bool InitialiseImpl(); 
	bool CreateImpl();
	virtual void InternalDependenciesCreatedImpl();
	virtual void ExternalDependenciesCreatedImpl();

	void DestroyBody();

	void ForceActive();
	void AllowDeactivate();
	void SetActivationThreshold(float val);

protected:
	Gravity::C_Body* m_pCollideBody;
	Maths::HMat43 m_CreationMatrix;
	Maths::HVec3 m_CreationLinVel;
	Maths::HVec3 m_CreationAngVel;
	bool m_IsEnabled;
	bool m_BodyActive;
	Gravity::C_KinematicController* m_KinematicController;
	Gravity::C_Scene* m_PhysicsScene;	// Cached scene ptr.
	ActorPhysics::TBodyType m_BodyType;

private:
	C_PhysicsBodyShapeDesc	m_ShapeDesc;
	Actors::C_PBCUserData*	m_pUserData;

	virtual void CreateShape(C_PhysicsBodyShapeDesc& shapeDesc) const {}

};


//////////////////////////////////////////////////////////////////////////
//
//
class C_PhysicalObjectPhysicsBody : public C_PhysicsBody
{
public:
	~C_PhysicalObjectPhysicsBody();

public:
	// From I_ActorComponent
	bool InitialiseImpl();
	static Component::C_ComponentTypeId ClassType() {return E_PhysObjPhysicsBody;}
	Component::C_ComponentTypeId GetComponentTypeID() const {return E_PhysObjPhysicsBody;}
	const C_BasePhysicalObjectPhysicsBodyCreateInfo* GetCreationData(void) const {return static_cast<const C_BasePhysicalObjectPhysicsBodyCreateInfo*>(GetCreator().GetData());}
	bool OnMessage(const Messaging::C_MessageBase* pMessage);

	void SerialiseForSave(Sys::C_ContextualStream& stream) const;
	void DeserialiseForSave(Sys::C_ContextualStream& stream);
	void SerialiseForSync(Sys::C_ContextualStream& stream) const;
	void DeserialiseForSync(Sys::C_ContextualStream& stream);

protected:
	Components::C_BasePhysicalObjectPhysicsBodyCreateInfo::TModelElemDescList m_ModelElemDesc;

private:
	virtual void CreateShape(C_PhysicsBodyShapeDesc& shapeDesc) const; // From C_PhysicsBody
	void UpdatePivotShapes(Collision::C_Shape* pShape, const I_DamageableElementManager* elemMgr);

};


} // namespace Components

#endif
